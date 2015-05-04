#include "bootpack.h"

//初始化这个图层管理块儿
LAYER_MANAGE *layer_man_init(MEMMANAGE *memman, unsigned char *vram, int scrnx, int scrny)
{
	LAYER_MANAGE *layer_man;
	int i;
	layer_man = (LAYER_MANAGE *)memmanage_alloc_4K(memman, sizeof(LAYER_MANAGE));
	if(0 == layer_man)
	{
		return 0;
	}
	layer_man->map = (unsigned char *)memmanage_alloc_4K(memman, scrnx * scrny);
	if(0 == layer_man->map)
	{
		memmanage_free_4K(memman, (int) layer_man, sizeof(LAYER_MANAGE));		//如果map的内存没有分配成功，则把图层管理结构的内存也释放掉
		return 0;
	}
	layer_man->vram = vram;
	layer_man->scrnx = scrnx;
	layer_man->scrny = scrny;
	layer_man->top = -1;				//表示现在一个图层也没有
	for(i = 0; i < MAX_LAYER; i++)
	{
		layer_man->layers[i].flags = LAYER_AVAILABLE;	//初始化每个图层的使用标记为0，即未使用状态
		layer_man->layers[i].layer_man = layer_man;		//把图层管理结构地址保存到每个图层中（感觉有点儿浪费空间啊）
	}
	return layer_man;
}

//为某个图层分配管理块儿
LAYER *layer_alloc(LAYER_MANAGE *layer_man)
{
	LAYER *layer;
	int i;
	for(i = 0; i < MAX_LAYER; i++)
	{
		if(LAYER_AVAILABLE == layer_man->layers[i].flags)			//flags为0，说明这个layer没有人用，可以分配给这个图层
		{
			layer = &layer_man->layers[i];
			layer->flags = LAYER_USING;
			layer->height = LAYER_HIDE;						//分配给图层之后，还没有把它显示在屏幕上呢，先把高度设置为-1，等以后显示之后再从新设置高度
			return layer;
		}
	}
	return 0;					//如果返回0，则说明所有图层都在被使用中
}

//为layer图层设定具体的图形，即把图形与其对应的管理块儿结合在一起
void layer_set(LAYER *layer, unsigned char *buf, int length0,int width0, int col_luc)
{
	layer->buffer = buf;
	layer->length = length0;
	layer->width = width0;
	layer->color_luc = col_luc;			
	return ;
}

//图层高度上的移动，即前后显示切换
void layer_switch(LAYER *layer, int height)
{
	LAYER_MANAGE *layer_man = layer->layer_man;
	int h, old = layer->height;			//记录这个图层切换前的高度
	/*1.如果指定的高度不合理，过低或者过高都要进行修正，然后对图层高度赋值*/
	if(height > layer_man->top + 1)
	{
		height = layer_man->top + 1;
	}
	if(height < -1)
	{
		height = 1;
	}
	layer->height = height;		//把新的要设置的高度赋值给这个图层
	/*2.对layers_order进行重新排序*/
	if(old > height)			//如果把图层向下调的情况
	{
		if(height >= 0)			//依旧处于屏幕内，处于显示状态下
		{
			/*需要把这个图层切换高度中间夹的图层的高度向上调整，其他高度的图层不受影响*/
			for(h = old; h > height; h--)
			{
				layer_man->layers_order[h] = layer_man->layers_order[h - 1];
				layer_man->layers_order[h]->height = h;
			}
			layer_man->layers_order[height] = layer;			//把其他图层高度调好之后，再把自己的高度调整到指定高度位置
			//将土层向下调之后，因为没有平移，所以自己的图层就不用在map表中刷新了，直接刷新下调到的高度之上的图层就行了
			layer_refresh_map(layer_man, layer->x, layer->y, layer->x + layer->length, layer->y + layer->width, height + 1);
			//这里只需要刷新两个移动的高度层之间的图层即可，其他的图层不会受到影响
			layer_part_refresh(layer_man, layer->x, layer->y, layer->x + layer->length, layer->y + layer->width, height + 1, old);
		}
		else					//height < 0,不处于显示状态下，即隐藏该图层
		{
			if(layer_man->top > old)		//只有当这个图层处于非最上层的时候才需要调整它上面的图层高度，如果是最上层，其他的图层就不需要调整了
			{
				for(h = old; h < layer_man->top; h++)
				{
					layer_man->layers_order[h] = layer_man->layers_order[h + 1];
					layer_man->layers_order[h]->height = h;
				}
			}
			layer_man->top--;				//图层减少一个，所以最高高度减1
		}
		layer_refresh_map(layer_man, layer->x, layer->y, layer->x + layer->length, layer->y + layer->width, 0);
		layer_part_refresh(layer_man, layer->x, layer->y, layer->x + layer->length, layer->y + layer->width, 0, old - 1);			
	}
	else if(old < height)			//如果把图层向上调的情况
	{
		if(old >= 0)
		{
			for(h = old; h < height; h++)
			{
				layer_man->layers_order[h] = layer_man->layers_order[h + 1];
				layer_man->layers_order[h]->height = h;
			}
			layer_man->layers_order[height] = layer;
		}
		else				//有可能就是刚刚创建的图层，height还是0,；或者是像上面的情况，已经被隐藏起来的图层，height是-1
		{
			for(h = layer_man->top; h >= height; h--)
			{
				layer_man->layers_order[h + 1] = layer_man->layers_order[h];
				layer_man->layers_order[h + 1]->height = h + 1;
			}
			layer_man->layers_order[height] = layer;
			layer_man->top++;
		}
		layer_refresh_map(layer_man, layer->x, layer->y, layer->x + layer->length, layer->y + layer->width, height);
		layer_part_refresh(layer_man, layer->x, layer->y, layer->x + layer->length, layer->y + layer->width, height, height);	
	}
	return ;
}

//图层平移，不切换
void layer_slide(LAYER *layer, int x0, int y0)
{
	LAYER_MANAGE *layer_man = layer->layer_man;
	int old_x0 = layer->x, old_y0 = layer->y;
	layer->x = x0;
	layer->y = y0;
	if(layer->height >= 0)			//只要图层位于显示区域（没有隐藏），就可以进行平移
	{
		//图层移动之后，原来位置上的所有图层都需要刷新
		layer_refresh_map(layer_man, old_x0, old_y0, old_x0 + layer->length, old_y0 + layer->width, 0);
		//图层移动的新的位置上，只刷新该图层到最顶层的地图，下面的图层即时刷新也会被上层掩盖，所以不用白费功夫
		layer_refresh_map(layer_man, x0, y0, x0 + layer->length, y0 + layer->width, layer->height);
		//原来的位置需要从最底层开始刷新，因为移动之后最底层的图像有可能会显示出来
		layer_part_refresh(layer_man, old_x0, old_y0, old_x0 + layer->length, old_y0 + layer->width, 0, layer->height - 1);					//刷新图层原来的位置
		//移动到新位置后，只刷新自己这一图层就行了，因为它不会影响更高图层的显示，刷新高层图层也是浪费时间，做无用功
		layer_part_refresh(layer_man, x0, y0, x0 + layer->length, y0 + layer->width, layer->height, layer->height);						//刷新图层现在所在的位置
	}
}

//判断指定图层是否与指定区域重合
int is_layer_overlap(LAYER *layer, int x0, int y0, int x1, int y1)
{
	/*1.图层代表的图形是否有顶点存在于指定区域*/
	if(
		((x0 <= layer->x && layer->x <= x1) && (y0 <= layer->y && layer->y <= y1)) ||
		((x0 <= (layer->x + layer->length) && (layer->x + layer->length) <= x1) && (y0 <= layer->y && layer->y <= y1)) ||
		((x0 <= layer->x && layer->x <= x1) && (y0 <= (layer->y + layer->width) && (layer->y + layer->width) <= y1)) ||
		((x0 <= (layer->x + layer->length) && (layer->x + layer->length) <= x1) && (y0 <= (layer->y + layer->width) && (layer->y + layer->width) <= y1))
	  )
	{
		return 1;
	}
	/*2.指定区域是否有顶点存在于图层代表的图形*/
	//layer->x <= x0 && layer->y <= y0 && (layer->x + layer->length) >= x1 && (layer->y + layer->width) >= y1
	if(
		(layer->x <= x0 && x0 <= (layer->x + layer->length) && layer->y <= y0 && y0 <= (layer->y + layer->width)) ||
		(layer->x <= x0 && x0 <= (layer->x + layer->length) && layer->y <= y1 && y1 <= (layer->y + layer->width)) ||
		(layer->x <= x1 && x1 <= (layer->x + layer->length) && layer->y <= y0 && y0 <= (layer->y + layer->width)) ||
		(layer->x <= x1 && x1 <= (layer->x + layer->length) && layer->y <= y1 && y1 <= (layer->y + layer->width))
	  )
	{
		return 1;
	}
	return 0;		//没有重叠的情况下返回0
}

//刷新高度表地图(只有图层高度改变和平移的时候才需要刷新高度地图表)
void layer_refresh_map(LAYER_MANAGE *layer_man, int x0, int y0, int x1, int y1, int height)
{
	int h, width0, length0, x, y, overlap_startX, overlap_startY, overlap_endX, overlap_endY;
	unsigned char *buf, color, layerID, *map = layer_man->map;
	LAYER *layer;
	/*超出屏幕范围的区域不刷新*/
	if(x0 < 0)
	{
		x0 = 0;
	}
	if(y0 < 0)
	{
		y0 = 0;
	}
	if(x1 > layer_man->scrnx)
	{
		x1 = layer_man->scrnx;
	}
	if(y1 > layer_man->scrny)
	{
		y1 = layer_man->scrny;
	}
	/*对各个图层的在指定区域中的部分进行刷新*/
	for(h = height; h <= layer_man->top; h++)
	{
		layer = layer_man->layers_order[h];
		//判断这一图层是否与指定区域有重合，如果没有重叠，就不用继续下面的步骤了，直接进入下一个图层的判断
		/*
		if(0 == is_layer_overlap(layer, x0, y0, x1, y1))
		{
			continue;
		}
		*/
		layerID = layer - layer_man->layers_order[0];//layer->height;				//用图层的高度作为图层ID
		buf = layer->buffer;
		overlap_startX = x0 - layer->x;
		overlap_startY = y0 - layer->y;
		overlap_endX = x1 - layer->x;
		overlap_endY = y1 - layer->y;
		//请看书P199
		if(overlap_startX < 0)
		{
			overlap_startX = 0;
		}
		if(overlap_startY < 0)
		{
			overlap_startY = 0;
		}
		if(overlap_endX > layer->length)
		{
			overlap_endX = layer->length;
		}
		if(overlap_endY > layer->width)
		{
			overlap_endY = layer->width;
		}
		for(width0 = overlap_startY; width0 < overlap_endY; width0++)
		{
			y = layer->y + width0;
			for(length0 = overlap_startX; length0 < overlap_endX; length0++)
			{
				x = layer->x + length0;
				if((x0 <= x && x <= x1) && (y0 <= y && y <= y1))		//在这个矩形范围内的才会重绘
				{
					color = buf[width0 * layer->length + length0];
					//这里的判断就判断这个颜色是不是当初设计的要画成透明的颜色设定值，这个值是随便设定的，只要能跟自己判断出来就行，每个图层都可以不一样
					if(color != layer->color_luc)				
					{
						map[y * layer_man->scrnx + x] = layerID;
					}
				}
			}
		}
	}
	return ;
}

//屏幕局部刷新,在指定矩形(x0,y0)和(x1,y1)范围内重绘图层
void layer_part_refresh(LAYER_MANAGE *layer_man, int x0, int y0, int x1, int y1, int height1, int height2)		
{
	int h, width0, length0, x, y, overlap_startX, overlap_startY, overlap_endX, overlap_endY;
	unsigned char *buf, color, *vram = layer_man->vram, *map = layer_man->map, layerID;
	LAYER *layer;
	/*超出屏幕范围的区域不刷新*/
	if(x0 < 0)
	{
		x0 = 0;
	}
	if(y0 < 0)
	{
		y0 = 0;
	}
	if(x1 > layer_man->scrnx)
	{
		x1 = layer_man->scrnx;
	}
	if(y1 > layer_man->scrny)
	{
		y1 = layer_man->scrny;
	}
	/*对各个图层的在指定区域中的部分进行刷新*/
	for(h = height1; h <= height2; h++)
	{
		layer = layer_man->layers_order[h];
		buf = layer->buffer;
		layerID = layer - layer_man->layers_order[0];		//layer->height;
		overlap_startX = x0 - layer->x;
		overlap_startY = y0 - layer->y;
		overlap_endX = x1 - layer->x;
		overlap_endY = y1 - layer->y;
		//请看书P199
		if(overlap_startX < 0)
		{
			overlap_startX = 0;
		}
		if(overlap_startY < 0)
		{
			overlap_startY = 0;
		}
		if(overlap_endX > layer->length)
		{
			overlap_endX = layer->length;
		}
		if(overlap_endY > layer->width)
		{
			overlap_endY = layer->width;
		}
		for(width0 = overlap_startY; width0 < overlap_endY; width0++)
		{
			y = layer->y + width0;
			for(length0 = overlap_startX; length0 < overlap_endX; length0++)
			{
				x = layer->x + length0;
				if((x0 <= x && x <= x1) && (y0 <= y && y <= y1))		//在这个矩形范围内的才会重绘
				{
					color = buf[width0 * layer->length + length0];
					//这里的判断就判断这个颜色是不是当初设计的要画成透明的颜色设定值，这个值是随便设定的，只要能跟自己判断出来就行，每个图层都可以不一样
					if(layerID == map[y * layer_man->scrnx + x])				
					{
						vram[y * layer_man->scrnx + x] = color;
					}
				}
			}
		}
	}
	return ;
}

//图层局部刷新，根据图层自己的坐标系为基础对指定位置刷新（这个只是单个图层的刷新，不造成高度的改变，所以不需要刷新高度地图表）
void layer_refresh(LAYER *layer, int length0, int width0, int length1, int width1)
{
	LAYER_MANAGE *layer_man = layer->layer_man;
	if(layer->height >= 0)
	{
		layer_part_refresh(layer_man, layer->x + length0, layer->y + width0, layer->x + length1, layer->y + width1, layer->height, layer->height);
	}
	return ;
}

//图层释放
void layer_free(LAYER *layer)
{
	LAYER_MANAGE *layer_man = layer->layer_man;
	if(layer->height >= 0)
	{
		layer_switch(layer, LAYER_HIDE);
	}
	layer->flags = LAYER_AVAILABLE;
	return ;
}














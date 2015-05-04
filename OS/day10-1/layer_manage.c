#include "bootpack.h"

//初始化这个图层管理块儿
LAYER_MANAGE *layer_man_init(MEMMANAGE *memman, unsigned char *vram, int scrnx, int scrny)
{
	LAYER_MANAGE *layer_man;
	int i;
	layer_man = (LAYER_MANAGE *)memmanage_alloc_4K(memman, sizeof(LAYER_MANAGE));
	if(0 == layer_man)
	{
		return layer_man;
	}
	layer_man->vram = vram;
	layer_man->scrnx = scrnx;
	layer_man->scrny = scrny;
	layer_man->top = -1;				//表示现在一个图层也没有
	for(i = 0; i < MAX_LAYER; i++)
	{
		layer_man->layers[i].flags = LAYER_AVAILABLE;	//初始化每个图层的使用标记为0，即未使用状态
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
			layer = &(layer_man->layers[i]);
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
void layer_switch(LAYER_MANAGE *layer_man, LAYER *layer, int height)
{
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
		}
		else					//height < 0,不处于显示状态下
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
		layer_all_refresh(layer_man);			
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
		layer_all_refresh(layer_man);
	}
	return ;
}

//图层平移，不切换
void layer_slide(LAYER_MANAGE *layer_man, LAYER *layer, int x0, int y0)
{
	layer->x = x0;
	layer->y = y0;
	if(layer->height >= 0)			//只要图层位于显示区域（没有隐藏），就可以进行平移
	{
		layer_all_refresh(layer_man);
	}
}

//全局刷新屏幕
void layer_all_refresh(LAYER_MANAGE *layer_man)
{
	int h, width0, length0, x0, y0;
	unsigned char *buf, color, *vram = layer_man->vram;
	LAYER *layer;
	for(h = 0; h <= layer_man->top; h++)
	{
		layer = layer_man->layers_order[h];
		buf = layer->buffer;
		for(width0 = 0; width0 < layer->width; width0++)
		{
			y0 = layer->y + width0;
			for(length0 = 0; length0 < layer->length; length0++)
			{
				x0 = layer->x + length0;
				color = buf[width0 * layer->length + length0];
				if(color != layer->color_luc)
				{
					vram[y0 * layer_man->scrnx + x0] = color;
				}
			}
		}
	}
	return ;
}

//图层释放
void layer_free(LAYER_MANAGE *layer_man, LAYER *layer)
{
	if(layer->height >= 0)
	{
		layer_switch(layer_man, layer, LAYER_HIDE);
	}
	layer->flags = LAYER_AVAILABLE;
	return ;
}














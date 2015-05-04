#include "bootpack.h"

void init_palette(void)
{
	//这个rgb表是为16色系统准备的
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,    //黑色
		0xff, 0x00, 0x00,    //亮红色
		0x00, 0xff, 0x00,    //亮绿色
		0xff, 0xff, 0x00,    //亮黄色
		0x00, 0x00, 0xff,    //亮蓝色
		0xff, 0x00, 0xff,    //亮紫色
		0x00, 0xff, 0xff,    //浅亮蓝
		0xff, 0xff, 0xff,    //白色
		0xc6, 0xc6, 0xc6,    //亮灰色
		0x84, 0x00, 0x00,    //暗红色
		0x00, 0x84, 0x00,    //暗绿色
		0x84, 0x84, 0x00,    //暗黄色
		0x00, 0x00, 0x84,    //暗青色
		0x84, 0x00, 0x84,    //暗紫色
		0x00, 0x84, 0x84,    //浅暗蓝
		0x84, 0x84, 0x84,    //暗灰色
	};
	//这个rgb表是为216色系统准备的(其实就是rgb每个颜色分成6阶，所以是6*6*6共216种颜色)
	unsigned char table_rgb2[216 * 3];		//以r为例，r分的6阶为：0、51、102、153、204、255
	int r, g, b;
	for(b = 0; b < 6; b++)
	{
		for(g = 0; g < 6; g++)
		{
			for(r = 0; r < 6; r++)
			{
				table_rgb2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;			//rgb都是以51为差度递增的
				table_rgb2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table_rgb2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
		}
	}
	//从0号到15号共16种色调
	set_palette(0, 15, table_rgb);
	//从16号到231号共216种色调
	set_palette(16, 231, table_rgb2);
	return ;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int eflags,i;
	eflags = io_load_eflags();	
	io_cli();
	io_out8(0x03c8, start);					//这里的0x03c8和下面的0x03c9是设定好的，不能随便写，需要调制调色板就得这样写
	for(i = start;i <= end;i++)
	{
		io_out8(0x03c9,rgb[0] / 4);			//设置颜色RGB的R部分
		io_out8(0x03c9,rgb[1] / 4);			//设置颜色RGB的G部分
		io_out8(0x03c9,rgb[2] / 4);			//设置颜色RGB的B部分
		rgb += 3;
	}
	io_store_eflags(eflags);
	return ;
}

void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int length, int width)
{
	int iX,iY;
	for(iY = y; iY < y + width; iY++)
	{
		for(iX = x; iX < x + length; iX++)
		{
			vram[iX + iY * xSize] = color;
		}
	}
}

void init_screen(unsigned char *vram, int x, int y)
{
	drawRectangle(vram, x, COL8_008484, 0, 0, x, y - 28); 			//画最大的背景矩形，下方留出空位给任务栏的矩形
	drawRectangle(vram, x, COL8_C6C6C6, 0, y - 28, x, 1); 			//描出一条很细很细的线来
	drawRectangle(vram, x, COL8_FFFFFF, 0, y - 27, x, 1); 			//同上，这样会有种立体的效果
	drawRectangle(vram, x, COL8_C6C6C6, 0, y - 26, x, 26); 			//画出最下方的灰色任务栏
	
	drawRectangle(vram, x, COL8_FFFFFF, 3, y - 24, 57, 1); 			//画出开始按钮的上方的立体白线
	drawRectangle(vram, x, COL8_FFFFFF, 2, y - 24, 1, 21); 			//画出开始按钮的左方的立体白线
	drawRectangle(vram, x, COL8_848484, 3, y - 4, 57, 1); 			//画出开始按钮的下方的立体黑线
	drawRectangle(vram, x, COL8_848484, 59, y - 23, 1, 19); 		//画出开始按钮的下方的立体黑线
	drawRectangle(vram, x, COL8_000000, 2, y - 3, 58, 1); 			//
	drawRectangle(vram, x, COL8_000000, 60, y - 24, 1, 22);			//
	
	drawRectangle(vram, x, COL8_848484, x - 47, y - 24, 44, 1); 		//
	drawRectangle(vram, x, COL8_848484, x - 47, y - 23, 1, 20); 		//
	drawRectangle(vram, x, COL8_FFFFFF, x - 47, y - 3, 44, 1); 			//
	drawRectangle(vram, x, COL8_FFFFFF, x - 3, y - 24, 1, 22); 			//
}

//每一个字体都占用16个字节，因为是8 X 16的方阵。所以这里的font共有16字节
void displayfont(unsigned char *vram, int scrnx, int x, int y, char color, char *font)
{
	int i;
	char *position, row;
	for(i = 0; i < 16; i++)
	{
		//字体的特定位置所对应的显示内存的地址的计算公式
		position = vram + (y + i) * scrnx + x;			//position代表屏幕上某一个像素对应的内存中的地址
		row = font[i];		//每个font[i]占用一个字节8位，row代表字体的每一行的点阵
		if(0 != (row & 0x80))
		{
			position[0] = color;
		}
		if(0 != (row & 0x40))
		{
			position[1] = color;
		}
		if(0 != (row & 0x20))
		{
			position[2] = color;
		}
		if(0 != (row & 0x10))
		{
			position[3] = color;
		}
		if(0 != (row & 0x08))
		{
			position[4] = color;
		}
		if(0 != (row & 0x04))
		{
			position[5] = color;
		}
		if(0 != (row & 0x02))
		{
			position[6] = color;
		}
		if(0 != (row & 0x01))
		{
			position[7] = color;
		}
	}
}

void displayStrings_CS(unsigned char *vram, int scrnx, int x, int y, char color, unsigned char *strings)
{
	extern char charset[4096];
	for(; *strings != 0x00; strings++)
	{
		displayfont(vram, scrnx, x, y, color, charset + *strings * 16);			//charset中有256个字符，每个字符占16个字节，编码顺序与ASCII码一样，所以如果要显示字母“A”，只需要在charset这个地址基础上加上65*16即可，也可以写成"A"*16
		x += 8;			//一个字符占用八个像素的宽度
		//这个部分是自己写的，如果像素数超过了scrnx，即屏宽，那么要向下移动16个像素，并把x归0
		if(x + 8 > scrnx)
		{
			y += 16;
			x = 0;
		}
	}
}

//在指定图层的指定位置显示指定颜色的指定文字，这样就不用管这个图层在哪里显示了，只需要在它上面显示好文字就行
void displayStrings_atLayer(LAYER *layer, int x, int y, int color, int bg_color, char *strings)
{
	int length = strlen(strings);
	drawRectangle(layer->buffer, layer->length, bg_color, x, y, 8 * length,15);
	displayStrings_CS(layer->buffer, layer->length, x, y, color,strings);
	layer_part_refresh(layer, x, y, x + length * 8, y + 16);
}

//第一个参数需要一个16*16的内存来存储鼠标每个像素的颜色，第二个是指定背景色
void init_mouse_cursor(unsigned char *mouse, char back_color)
{
	static char cursor[mCursorWidth][mCursorHeight] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int i, j;
	for(i = 0; i < mCursorWidth; i++)
	{
		for(j = 0; j < mCursorHeight; j++)
		{
			if(cursor[i][j] == '*')
			{
				mouse[i * mCursorWidth + j] = COL8_000000;			//鼠标光标的周围是黑色线条
			}
			if(cursor[i][j] == 'O')
			{
				mouse[i * mCursorWidth + j] = COL8_FFFFFF;			//鼠标光标主体是白色的
			}
			if(cursor[i][j] == '.')
			{
				mouse[i * mCursorWidth + j] = back_color;				//鼠标光标的背景颜色设置为主背景色
			}
		}
	}
}

//这个函数可以针对一个已经填充好颜色数值的内存进行显示，由于这块儿内存不一定是在显示内存区域，所以需要复制到显示内存区的特定位置才行
void displayShape(unsigned char *vram, int scrnx, int length, int width, int x, int y,unsigned  char *shapeBuffer, int xPixel)
{
	int i, j;
	for(i = 0; i < width; i++)
	{
		for(j = 0; j < length; j++)
		{
			vram[(y + i) * scrnx + (x + j)] = shapeBuffer[i * xPixel + j];		//将保存图形颜色值的那块儿区域对应的值存到相应的显示内存区中
		}
	}
}
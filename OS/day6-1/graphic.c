#include "bootpack.h"

void init_palette(void)
{
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
	set_palette(0, 15, table_rgb);
	return ;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int eflags,i;
	eflags = io_load_eflags();	
	io_cli();
	io_out8(0x03c8,start);
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

void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int height)
{
	int iX,iY;
	for(iY = y; iY <= y + height; iY++)
	{
		for(iX = x; iX <= x + width; iX++)
		{
			vram[iX + iY * xSize] = color;
		}
	}
}

void init_screen(char *vram, int x, int y)
{
	drawRectangle(vram, x, COL8_008484, 0, 0, x - 1, y - 29); 			//画最大的背景矩形，下方留出空位给任务栏的矩形
	drawRectangle(vram, x, COL8_C6C6C6, 0, y - 28, x - 1, 0); 			//描出一条很细很细的线来
	drawRectangle(vram, x, COL8_FFFFFF, 0, y - 27, x - 1, 0); 			//同上，这样会有种立体的效果
	drawRectangle(vram, x, COL8_C6C6C6, 0, y - 26, x - 1, 25); 			//画出最下方的灰色任务栏
	
	drawRectangle(vram, x, COL8_FFFFFF, 3, y - 24, 56, 0); 			//画出开始按钮的上方的立体白线
	drawRectangle(vram, x, COL8_FFFFFF, 2, y - 24, 0, 20); 			//画出开始按钮的左方的立体白线
	drawRectangle(vram, x, COL8_848484, 3, y - 4, 56, 0); 			//画出开始按钮的下方的立体黑线
	drawRectangle(vram, x, COL8_848484, 59, y - 23, 0, 18); 		//画出开始按钮的下方的立体黑线
	drawRectangle(vram, x, COL8_000000, 2, y - 3, 57, 0); 			//
	drawRectangle(vram, x, COL8_000000, 60, y - 24, 0, 21);			//
	
	drawRectangle(vram, x, COL8_848484, x - 47, y - 24, 43, 0); 		//
	drawRectangle(vram, x, COL8_848484, x - 47, y - 23, 0, 19); 		//
	drawRectangle(vram, x, COL8_FFFFFF, x - 47, y - 3, 43, 0); 			//
	drawRectangle(vram, x, COL8_FFFFFF, x - 3, y - 24, 0, 21); 			//
}

void displayfont(char *vram, int scrnx, int x, int y, char color, char *font)
{
	int i;
	char *p, d;
	for(i = 0; i < 16; i++)
	{
		//字体的特定位置所对应的显示内存的地址的计算公式
		p = vram + (y + i) * scrnx + x;
		d = font[i];
		if(0 != (d & 0x80))
		{
			p[0] = color;
		}
		if(0 != (d & 0x40))
		{
			p[1] = color;
		}
		if(0 != (d & 0x20))
		{
			p[2] = color;
		}
		if(0 != (d & 0x10))
		{
			p[3] = color;
		}
		if(0 != (d & 0x08))
		{
			p[4] = color;
		}
		if(0 != (d & 0x04))
		{
			p[5] = color;
		}
		if(0 != (d & 0x02))
		{
			p[6] = color;
		}
		if(0 != (d & 0x01))
		{
			p[7] = color;
		}
	}
}

void displayStrings_CS(char *vram, int scrnx, int x, int y, char color, unsigned char *strings)
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

//第一个参数需要一个16*16的内存来存储鼠标每个像素的颜色，第二个是指定背景色
void init_mouse_cursor(char *mouse, char back_color)
{
	static char cursor[16][16] = {
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
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 16; j++)
		{
			if(cursor[i][j] == '*')
			{
				mouse[i * 16 + j] = COL8_000000;			//鼠标光标的周围是黑色线条
			}
			if(cursor[i][j] == 'O')
			{
				mouse[i * 16 + j] = COL8_FFFFFF;			//鼠标光标主体是白色的
			}
			if(cursor[i][j] == '.')
			{
				mouse[i * 16 + j] = back_color;				//鼠标光标的背景颜色设置为主背景色
			}
		}
	}
}

//这个函数可以针对一个已经填充好颜色数值的内存进行显示，由于这块儿内存不一定是在显示内存区域，所以需要复制到显示内存区的特定位置才行
void displayShape(char *vram, int scrnx, int width, int height, int x, int y, char *shapeBuffer, int xPixel)
{
	int i, j;
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			vram[(y + i) * scrnx + (x + j)] = shapeBuffer[i * xPixel + j];		//将保存图形颜色值的那块儿区域对应的值存到相应的显示内存区中
		}
	}
}
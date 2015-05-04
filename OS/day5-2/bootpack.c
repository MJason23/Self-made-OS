//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

#define NULL 0x00

//结构体声明
typedef struct{
	char cyls, leds, vmode, reserve;
	short scrnx,scrny;
	char *vram;
}BOOTINFO;

void io_hlt(void);
void io_cli(void);						//将中断许可标志置为0，禁止中断，这一部分代码的解释在书上P80
void io_out8(int port,int data);		//向指定装置发送信息
int io_load_eflags(void);					//记录中断许可标志的值
void io_store_eflags(int eflags);		//恢复中断许可标志的值
//void write_mem8(int addr,int data);
void init_palette(void);          		//初始化调色板，这个调色板是用来规定各个数字所对应的颜色的
void set_palette(int start,int end,unsigned char *rgb);  			//设置调色板
//参数vram：代表显示内存的起始地址；参数xSize：代表横屏的像素数；参数color：代表画出的矩形的颜色；参数x和y：代表矩形的坐标；参数width和length：代表矩形的宽高
void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int length);
void init_screen(char *vram, int x, int y);			//把生成屏幕背景的步骤总结成一个函数
void displayfont(char *vram, int scrnx, int x, int y, char color, char *font);						//在指定位置显示指定颜色的某个字体的函数
void displayStrings_CS(char *vram, int scrnx, int x, int y, char color, unsigned char *strings);	//用来在指定位置显示指定颜色的字符串的函数，使用的编码集是charset.txt

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *)0x0ff0;
	
	init_palette();
	
	//画出一个windows界面
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//显示一行字符串
	displayStrings_CS(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "It's the first time that I made an OS myself!!");
	
	for(;;)
	{
		io_hlt();						//执行完成启动系统之后就让CPU停下来
	}	
}

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

void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int length)
{
	int iX,iY;
	for(iY = y; iY <= y + length; iY++)
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
	for(; *strings != NULL; strings++)
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












//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

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
	char *vram;
	int xSize,ySize;
	
	init_palette();
	
	vram = (char *)0xa0000;
	xSize = 320;
	ySize = 200;
	//画出一个windows界面
	drawRectangle(vram, xSize, COL8_008484, 0, 0, xSize - 1, ySize - 29); 			//画最大的背景矩形，下方留出空位给任务栏的矩形
	drawRectangle(vram, xSize, COL8_C6C6C6, 0, ySize - 28, xSize - 1, 0); 			//描出一条很细很细的线来
	drawRectangle(vram, xSize, COL8_FFFFFF, 0, ySize - 27, xSize - 1, 0); 			//同上，这样会有种立体的效果
	drawRectangle(vram, xSize, COL8_C6C6C6, 0, ySize - 26, xSize - 1, 25); 			//画出最下方的灰色任务栏
	
	drawRectangle(vram, xSize, COL8_FFFFFF, 3, ySize - 24, 56, 0); 			//画出开始按钮的上方的立体白线
	drawRectangle(vram, xSize, COL8_FFFFFF, 2, ySize - 24, 0, 20); 			//画出开始按钮的左方的立体白线
	drawRectangle(vram, xSize, COL8_848484, 3, ySize - 4, 56, 0); 			//画出开始按钮的下方的立体黑线
	drawRectangle(vram, xSize, COL8_848484, 59, ySize - 23, 0, 18); 		//画出开始按钮的下方的立体黑线
	drawRectangle(vram, xSize, COL8_000000, 2, ySize - 3, 57, 0); 			//
	drawRectangle(vram, xSize, COL8_000000, 60, ySize - 24, 0, 21);			//
	
	drawRectangle(vram, xSize, COL8_848484, xSize - 47, ySize - 24, 43, 0); 		//
	drawRectangle(vram, xSize, COL8_848484, xSize - 47, ySize - 23, 0, 19); 		//
	drawRectangle(vram, xSize, COL8_FFFFFF, xSize - 47, ySize - 3, 43, 0); 			//
	drawRectangle(vram, xSize, COL8_FFFFFF, xSize - 3, ySize - 24, 0, 21); 			//
	
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






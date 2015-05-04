//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

//由于用到了sprintf这个函数，所以需要这个头文件，但是这个函数不需要调用任何操作系统的功能，所以可以这么写（因为我们本来就是在做操作系统，所以不能有依赖操作系统的函数）
#include <stdio.h>
#include "bootpack.h"

extern FIFO keyFIFO;
extern FIFO mouseFIFO;
extern TIMERCTL timerctl;

void HariMain(void)
{
	/*
	**最初的这部分变量没有通过内存管理来分配，它们本身属于操作系统的一部分，存在于bootpack.hrb所在的那块儿内存空间中
	*/
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER_MANAGE *layer_manage;
	LAYER *layer_bg, *layer_mouse, *layer_window;
	TIMER *timer1, *timer2, *timer3;
	FIFO timerfifo1, timerfifo2, timerfifo3;
	char buf_cursor[mCursorWidth * mCursorHeight], timerbuf1[8], timerbuf2[8], timerbuf3[8];
	unsigned char *buf_bg, *buf_window;			//屏幕的大背景会在init_screen的时候画出来，这里只需要一个指向它的指针即可
	unsigned char i, strings[40], keyBuffer[32], mouseBuffer[128];
	unsigned int memory_total, count = 0;
	int j;
	int mCursorX, mCursorY;				//鼠标光标显示位置的横纵坐标
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//减去下方任务栏的高度
	
	
	/*内存检查*/
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);		//i的单位是MB
	
	/*内存管理*/
	memmanage_init(memmanage);
	memory_total = i * 1024 * 1024;
	memmanage_free_4K(memmanage, 0x00001000, 0x0009e000);
	memmanage_free_4K(memmanage, 0x00400000, memory_total - 0x00400000);
	
	/*初始化接收中断的缓冲区*/
	init_fifo(&keyFIFO, 32, keyBuffer);				//初始化keyFIFO缓冲区
	init_fifo(&mouseFIFO, 128, mouseBuffer);		//初始化mouseFIFO缓冲区
	/*初始化GDT和IDT表以及PIC板的数据*/
	init_GDTandIDT();				//初始化GDT和IDT表
	init_pic();						//初始化PIC主从板数据，除了IRQ2禁止了全部中断
	io_sti();						//开始接收中断
	/*初始化PIT中断控制*/
	init_PIT();
		/*若要接收鼠标中断需要两个步骤，首先必须使鼠标控制电路（就是键盘控制电路的一部分）有效，然后要使鼠标本身有效*/
	init_keyboard();						//初始化键盘控制器电路
	enable_mouse(&mouse_dec);				//激活鼠标
	/*开放各种中断*/
	io_out8(PIC0_IMR, 0xf8);				//PIC0开发IRQ(11111000)，开放IRQ0、IRQ1和IRQ2，定时器、键盘中断和从PIC板
	io_out8(PIC1_IMR, 0xef); 				//PIC1开放IRQ(11101111)， 开放鼠标中断
	/*初始化调色板，为图形界面做准备*/
	init_palette();					//初始化调色板
	/*初始化图层管理，并且初始化鼠标光标和背景的图层*/
	layer_manage = layer_man_init(memmanage, binfo->vram, binfo->scrnx, binfo->scrny);
	
	layer_bg = layer_alloc(layer_manage);			//为背景分配图层
	layer_mouse = layer_alloc(layer_manage);		//为鼠标分配图层
	layer_window = layer_alloc(layer_manage);		//为窗口分配图层
	
	buf_bg = (unsigned char *)memmanage_alloc_4K(memmanage, binfo->scrnx * binfo->scrny);		//为背景图形的内容分配内存
	buf_window = (unsigned char *)memmanage_alloc_4K(memmanage, 160 * 52);						//为窗口图形的内容分配内存
	/*为各个图形的图层内容进行设定*/
	layer_set(layer_bg, buf_bg, binfo->scrnx, binfo->scrny, -1);			
	layer_set(layer_mouse, buf_cursor, 16, 16 ,99);
	layer_set(layer_window, buf_window, 160, 52, -1);
	/*初始化整个桌面背景*/
	init_screen(buf_bg, binfo->scrnx, binfo->scrny);				//这个时候的init_screen不再是直接画出背景，而是在mBg内存地址中填写好背景内容
	layer_slide(layer_bg, 0, 0);					//把背景图层从(0,0)坐标开始画
	/*初始化鼠标图标*/
	init_mouse_cursor(buf_cursor, 99);								//初始化鼠标光标
	layer_slide(layer_mouse, mCursorX, mCursorY);	//现在显示图形不需要再用displayShape函数了，直接用这个图层管理的绘图函数就行
	/*初始化窗口*/
	create_window(buf_window, 160, 52, "counter");	//制作窗口
	layer_slide(layer_window, 80, 72);				//在指定位置显示出窗口
	/*设置好各个图层的高度*/
	layer_switch(layer_bg, 0);						//把背景图层调为最底层，高度为0
	layer_switch(layer_window, 1);					//窗口图层调节为第二层，高度为1
	layer_switch(layer_mouse, 2);					//鼠标图层调为最高层，高度为2
	/*定时器的初始化及其设置*/
	init_fifo(&timerfifo1, 8, timerbuf1);
	timer1 = timer_alloc();
	timer_init(timer1, &timerfifo1, 1);
	timer_set(timer1, 1000);
	init_fifo(&timerfifo2, 8, timerbuf2);
	timer2 = timer_alloc();
	timer_init(timer2, &timerfifo2, 1);
	timer_set(timer2, 300);
	init_fifo(&timerfifo3, 8, timerbuf3);
	timer3 = timer_alloc();
	timer_init(timer3, &timerfifo3, 1);
	timer_set(timer3, 50);
	/*在屏幕上显示一些内存、鼠标和键盘等信息*/
	sprintf(strings, "Memory has %dMB", i);
	displayStrings_CS(buf_bg, binfo->scrnx, 0, 48, COL8_FFFFFF,strings);
	layer_refresh(layer_bg, 0, 48, binfo->scrnx, 80);
	sprintf(strings, "free memory:%dKB",memmanage_total(memmanage) / 1024);
	displayStrings_CS(buf_bg, binfo->scrnx, 120, 48, COL8_FFFFFF,strings);			//用字体显示当前内存容量
	layer_refresh(layer_bg, 120, 48, binfo->scrnx, 80);
	
	
	for(;;)
	{
		sprintf(strings, "%10d", timerctl.count);
		drawRectangle(buf_window, 160, COL8_C6C6C6, 40, 28, 79,15);
		displayStrings_CS(buf_window, 160, 40, 28, COL8_000000,strings);
		layer_refresh(layer_window, 40, 28, 120, 44);
		
		/*只有在从中断返回的缓冲区中读取数据的时候才需要禁止中断，因为如果这个时候来了中断而没有禁止的话，
		**有可能读脏数据，即把还没有读出的数据的给抹掉或换成别的数据
		*/
		io_cli();		
		if(0 == fifo_status(&keyFIFO) + fifo_status(&mouseFIFO) + fifo_status(&timerfifo1) + fifo_status(&timerfifo2)
			 + fifo_status(&timerfifo3))		//当前没有中断产生
		{
			io_sti();			//当CPU执行hlt指令之后只有外部中断等之情况才会再次唤醒CPU继续工作
		}
		else
		{
			if(0 != fifo_status(&keyFIFO))
			{
				i = fifo_get(&keyFIFO);
				io_sti();
				sprintf(strings,"%2X",i);
				drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 0, 0, 16,16);		//这里的字体不再是卸载vram中，而是写到mBg这个背景内存中，与背景成为一个图层
				displayStrings_CS(buf_bg, binfo->scrnx, 0, 0, COL8_FFFFFF,strings);
				layer_refresh(layer_bg, 0, 0, 16, 16);								//由于向背景图层中添加了新东西，需要重绘各个图层
			}
			else if(0 != fifo_status(&mouseFIFO))
			{
				i = fifo_get(&mouseFIFO);
				io_sti();
				if(0 != mouse_decode(&mouse_dec, i))		//只有返回值为1的时候才说明成功接收完成一次鼠标的中断
				{
					/*显示鼠标的信息*/
					sprintf(strings,"[lcr %3d,%3d]",mouse_dec.x, mouse_dec.y);
					if(0 != (mouse_dec.btn & 0x01))		//按下了左键
					{
						strings[1] = 'L';
					}
					if(0 != (mouse_dec.btn & 0x02))		//按下了右键
					{
						strings[3] = 'R';
					}
					if(0 != (mouse_dec.btn & 0x04))		//按下了中键
					{
						strings[2] = 'C';
					}
					drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 0, 16, 15 * 8, 16);		//用背景色覆盖上次的坐标
					displayStrings_CS(buf_bg, binfo->scrnx, 0, 16, COL8_FFFFFF,strings);		//显示新的坐标
					layer_refresh(layer_bg,0, 16, 15 * 8, 32);
					/*鼠标的移动*/
					//根据mouse_dec里存储的鼠标信息画出新的鼠标图像
					mCursorX += mouse_dec.x;
					mCursorY += mouse_dec.y;
					//不能让鼠标移出画面
					if(mCursorX < 0)
					{
						mCursorX = 0;
					}
					if(mCursorY < 0)
					{
						mCursorY = 0;
					}
					if(mCursorX > binfo->scrnx - 1)
					{
						mCursorX = binfo->scrnx - 1;
					}
					if(mCursorY > binfo->scrny - 1)
					{
						mCursorY = binfo->scrny - 1;
					}
					sprintf(strings, "(%3d,%3d)", mCursorX, mCursorY);
					drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 40, 32, 20 * 8,16);		//用背景色覆盖上次的坐标
					displayStrings_CS(buf_bg, binfo->scrnx, 40, 32, COL8_FFFFFF,strings);		//显示新的坐标
					layer_refresh(layer_bg, 40, 32, 20 * 8,48);
					layer_slide(layer_mouse, mCursorX, mCursorY);
				}
			}
			else if(0 != fifo_status(&timerfifo1))			//10秒定时器倒计时结束后在屏幕上显示"10[sec]"
			{
				i = fifo_get(&timerfifo1);
				io_sti();
				displayStrings_CS(buf_bg, binfo->scrnx, 0, 64, COL8_FFFFFF,"10[sec]");
				layer_refresh(layer_bg, 0, 64, 7 * 8, 80);
			}
			else if(0 != fifo_status(&timerfifo2))			//3秒定时器倒计时结束后在屏幕上显示"3[sec]"
			{
				i = fifo_get(&timerfifo2);
				io_sti();
				displayStrings_CS(buf_bg, binfo->scrnx, 0, 80, COL8_FFFFFF,"3[sec]");
				layer_refresh(layer_bg, 0, 80, 6 * 8, 96);
			}
			else if(0 != fifo_status(&timerfifo3))			//利用0.5秒定时器倒计时结束的特性，在屏幕上显示闪烁光标
			{
				i = fifo_get(&timerfifo3);
				io_sti();
				if(0 != i)				//这个timer的数据为1的时候显示光标
				{
					timer_init(timer3, &timerfifo3, 0);
					drawRectangle(buf_bg, binfo->scrnx, COL8_FFFFFF, 8, 96, 3, 16);
				}
				else					//这个timer的数据为0的时候不显示光标
				{
					timer_init(timer3, &timerfifo3, 1);
					drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 8, 96, 3, 16);
				}
				timer_set(timer3, 50);
				layer_refresh(layer_bg, 8, 96, 11, 112);
			}
		}
	}	
}






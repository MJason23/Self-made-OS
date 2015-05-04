//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

//由于用到了sprintf这个函数，所以需要这个头文件，但是这个函数不需要调用任何操作系统的功能，所以可以这么写（因为我们本来就是在做操作系统，所以不能有依赖操作系统的函数）
#include <stdio.h>
#include "bootpack.h"

extern FIFO keyFIFO;
extern FIFO mouseFIFO;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	char mCursor[mCursorWidth * mCursorHeight];
	unsigned char strings[4], keyBuffer[32], mouseBuffer[128],i;
	int j;
	int mCursorX, mCursorY;				//鼠标光标显示位置的横纵坐标
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//减去下方任务栏的高度
	
	
	
	init_fifo(&keyFIFO, 32, keyBuffer);				//初始化FIFO缓冲区
	init_fifo(&mouseFIFO, 128, mouseBuffer);		//初始化FIFO缓冲区
	
	init_GDTandIDT();				//初始化GDT和IDT表
	init_pic();						//初始化PIC主从板数据
	io_sti();						//开始接收中断
	
	init_palette();					//初始化调色板
	
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);				//画出一个windows界面
	
	/*若要接收鼠标中断需要两个步骤，首先必须使鼠标控制电路（就是键盘控制电路的一部分）有效，然后要使鼠标本身有效*/
	init_keyboard();						//初始化键盘控制器电路
	enable_mouse(&mouse_dec);				//激活鼠标
	
	init_mouse_cursor(mCursor, COL8_008484);							//初始化鼠标光标
	displayShape(binfo->vram, binfo->scrnx, mCursorWidth, mCursorHeight, mCursorX, mCursorY, mCursor, mCursorWidth);		//显示鼠标光标
	
	io_out8(PIC0_IMR, 0xf9); 						/* PIC0开发IRQ(11111001) */
	io_out8(PIC1_IMR, 0xef); 						/* PIC1开放IRQ(11101111) */
	
	for(;;)
	{
		io_cli();
		if(0 == fifo_status(&keyFIFO) + fifo_status(&mouseFIFO))		//当前没有中断产生
		{
			io_stihlt();			//当CPU执行hlt指令之后只有外部中断等之情况才会再次唤醒CPU继续工作
		}
		else
		{
			if(0 != fifo_status(&keyFIFO))
			{
				i = fifo_get(&keyFIFO);
				io_sti();
				sprintf(strings,"%2X",i);
				drawRectangle(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 15,31);
				displayStrings_CS(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF,strings);
			}
			else if(0 != fifo_status(&mouseFIFO))
			{
				i = fifo_get(&mouseFIFO);
				io_sti();
				if(0 != mouse_decode(&mouse_dec, i))		//只有返回值为1的时候才说明成功接收完成一次鼠标的中断
				{
					/*显示鼠标的信息*/
					sprintf(strings,"[lcr %4d %4d]",mouse_dec.x, mouse_dec.y);
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
					drawRectangle(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15 * 8,16);		//用背景色覆盖上次的坐标
					displayStrings_CS(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF,strings);		//显示新的坐标
					/*鼠标的移动*/
					//先隐藏当前鼠标图像
					drawRectangle(binfo->vram, binfo->scrnx, COL8_008484, mCursorX, mCursorY, 16,16);
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
					if(mCursorX > binfo->scrnx - 16)
					{
						mCursorX = binfo->scrnx - 16;
					}
					if(mCursorY > binfo->scrny - 16)
					{
						mCursorY = binfo->scrny - 16;
					}
					sprintf(strings, "[%4d %4d]", mCursorX, mCursorY);
					drawRectangle(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15 * 8,16);		//用背景色覆盖上次的坐标
					displayStrings_CS(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF,strings);		//显示新的坐标
					displayShape(binfo->vram, binfo->scrnx, 16, 16, mCursorX, mCursorY,mCursor, 16);//显示新的鼠标图形
				}
					
			}
		}
	}	
}






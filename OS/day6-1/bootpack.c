//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

//由于用到了sprintf这个函数，所以需要这个头文件，但是这个函数不需要调用任何操作系统的功能，所以可以这么写（因为我们本来就是在做操作系统，所以不能有依赖操作系统的函数）
#include <stdio.h>
#include "bootpack.h"


void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *)0x0ff0;
	//char string[40];
	char mCursor[mCursorWidth * mCursorHeight];
	int mCursorX, mCursorY;				//鼠标光标显示位置的横纵坐标
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//减去下方任务栏的高度
	
	init_GDTandIDT();
	init_palette();
	
	//画出一个windows界面
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//显示一行字符串
	displayStrings_CS(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "This is the first time that I made an OS myself!!");
	//sprintf(string, "scrnx = %d", binfo->scrnx);
	//displayStrings_CS(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, string);
	init_mouse_cursor(mCursor, COL8_008484);
	displayShape(binfo->vram, binfo->scrnx, mCursorWidth, mCursorHeight, mCursorX, mCursorY, mCursor, mCursorWidth);
	
	for(;;)
	{
		io_hlt();						//执行完成启动系统之后就让CPU停下来
	}	
}






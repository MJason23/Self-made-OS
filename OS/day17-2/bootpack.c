//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

//由于用到了sprintf这个函数，所以需要这个头文件，但是这个函数不需要调用任何操作系统的功能，所以可以这么写（因为我们本来就是在做操作系统，所以不能有依赖操作系统的函数）
#include <stdio.h>
#include "bootpack.h"

extern TIMERCTL timerctl;
extern TIMER *task_timer;

void HariMain(void)
{
	/*
	**最初的这部分变量没有通过内存管理来分配，它们本身属于操作系统的一部分，存在于bootpack.hrb所在的那块儿内存空间中
	*/
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER_MANAGE *layer_manage;
	LAYER *layer_bg, *layer_mouse, *layer_window, *layer_console;
	TIMER *timer;
	FIFO fifo_mutual;									//缓冲区管理，所有中断公用的
	TASK *task_a, *task_console;
	int fifobuf[128];									//缓冲区
	int cursor_x = 8, cursor_color = COL8_FFFFFF;		//分别代表输入字符后的那个闪烁光标的横坐标和颜色
	int mCursorX, mCursorY;				//鼠标光标显示位置的横纵坐标
	int key_to = 0;						//标志键盘输入到哪个窗口中
	unsigned int memory_total, i;
	char buf_cursor[mCursorWidth * mCursorHeight];
	unsigned char *buf_bg, *buf_window, *buf_console;			//屏幕的大背景会在init_screen的时候画出来，这里只需要一个指向它的指针即可
	unsigned char strings[40];
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
	init_fifo(&fifo_mutual, 128, fifobuf, 0);		//初始化mouseFIFO缓冲区，现在还没任务，先指定为0
	/*初始化GDT和IDT表以及PIC板的数据*/
	init_GDTandIDT();				//初始化GDT和IDT表
	init_pic();						//初始化PIC主从板数据，除了IRQ2禁止了全部中断
	io_sti();						//开始接收中断
	/*初始化PIT中断控制*/
	init_PIT();
		/*若要接收鼠标中断需要两个步骤，首先必须使鼠标控制电路（就是键盘控制电路的一部分）有效，然后要使鼠标本身有效*/
	init_keyboard(&fifo_mutual, 256);						//初始化键盘控制器电路
	enable_mouse(&fifo_mutual, 512, &mouse_dec);				//激活鼠标
	/*开放各种中断*/
	io_out8(PIC0_IMR, 0xf8);				//PIC0开发IRQ(11111000)，开放IRQ0、IRQ1和IRQ2，定时器、键盘中断和从PIC板
	io_out8(PIC1_IMR, 0xef); 				//PIC1开放IRQ(11101111)， 开放鼠标中断
	/*初始化任务切换管理*/
	task_a = task_init(memmanage);		//这个task_a其实代表的就是这些鼠标键盘等的任务
	fifo_mutual.task = task_a;						//为鼠标键盘等的缓冲区指定唤醒任务为task_a
	task_run(task_a, 1, 2);
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
	create_window(buf_window, 160, 52, "task_a", ACTIVE);					//制作窗口
	layer_slide(layer_window, 80, 100);								//在指定位置显示出窗口
	create_textbox(layer_window, 8, 28, 144, 16, COL8_FFFFFF);		//在这个窗口中创建一个输入框
	/*执行一个命令行窗口任务*/
	layer_console = layer_alloc(layer_manage);
	buf_console = (unsigned char *)memmanage_alloc_4K(memmanage, 256 * 165);	
	layer_set(layer_console, buf_console, 256, 165, -1);
	create_window(buf_console, 256, 165, "console", INACTIVE);
	create_textbox(layer_console, 8, 28, 240, 128, COL8_000000);
	task_console = task_alloc();
	task_console->tss.esp = memmanage_alloc_4K(memmanage, 64 * 1024) + 64 * 1024 - 8;
	task_console->tss.eip = (int) &console_task;
	task_console->tss.es = 1 * 8;
	task_console->tss.cs = 2 * 8;
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
	*((int *) (task_console->tss.esp + 4)) = (int) layer_console;
	task_run(task_console, 2, 2); /* level=2, priority=2 */
	
	
	layer_slide(layer_console, 32, 60);
	/*设置好各个图层的高度*/
	layer_switch(layer_bg, 0);						//把背景图层调为最底层，高度为0
	layer_switch(layer_console, 1);					//命令行窗口图层调节为第三层，高度为2
	layer_switch(layer_window, 2);					//窗口图层调节为第二层，高度为1
	layer_switch(layer_mouse, 3);					//鼠标图层调为最高层，高度为3
	/*定时器的初始化及其设置*/
	timer = timer_alloc();
	timer_init(timer, &fifo_mutual, 1);
	timer_set(timer, 50);
	/*在屏幕上显示一些内存、鼠标和键盘等信息*/
	sprintf(strings, "Memory has %dMB", i);
	displayStrings_CS(buf_bg, binfo->scrnx, 0, 48, COL8_FFFFFF,strings);
	layer_part_refresh(layer_bg, 0, 48, binfo->scrnx, 80);
	sprintf(strings, "free memory:%dKB",memmanage_total(memmanage) / 1024);
	displayStrings_CS(buf_bg, binfo->scrnx, 120, 48, COL8_FFFFFF,strings);			//用字体显示当前内存容量
	layer_part_refresh(layer_bg, 120, 48, binfo->scrnx, 80);
	
	
	
	for(;;)
	{
		/*只有在从中断返回的缓冲区中读取数据的时候才需要禁止中断，因为如果这个时候来了中断而没有禁止的话，
		**有可能读脏数据，即把还没有读出的数据的给抹掉或换成别的数据
		*/
		io_cli();		
		if(0 == fifo_status(&fifo_mutual))		//当前没有中断产生
		{
			task_sleep(task_a);	//如果没有需要处理的数据，就自己让自己休眠
			/*上面的那个函数直接就跳转到另一个任务中去了，不过，因为每个人物的默认eflags都是开启中断的，
			**所以不用担心，这个任务还是能被唤醒的，耳环醒后的第一个动作就是为了以防万一先开启中断
			*/
			io_sti();		//万一只有自己在运行的话，则无法睡眠，那么就执行hlt指令好了，这样省电嘛	
		}
		else
		{
			i = fifo_get(&fifo_mutual);
			io_sti();
			//如果是键盘中断的话，需要处理键盘发送来的中断数据
			if((keyboard_offset <= i) && (i <= keyboard_offset + 255))			
			{
				sprintf(strings, "%2X", i - keyboard_offset);
				displayStrings_atLayer(layer_bg, 0, 0, COL8_FFFFFF, COL8_008484, strings);					//由于向背景图层中添加了新东西，需要重绘各个图层
				//判断按下的字符是否为一般字符，0对应的字符不处理
				if(i < keyboard_offset + 0x54 && 0 != key_table[i - keyboard_offset])				
				{
					//发送给task_a任务的窗口
					if(0 == key_to)
					{
						//所有可打印的字符都需要打印出来
						if(cursor_x < 144)
						{
							strings[0] = key_table[i - keyboard_offset];
							strings[1] = 0;
							displayStrings_atLayer(layer_window, cursor_x, 28, COL8_000000, COL8_FFFFFF, strings);
							cursor_x += 8;			//光标的位置随着每一个字符的输入向后移动一个字符宽度的位置即8像素
						}
					}
					//发送给命令行窗口
					else
					{
						fifo_put(&task_console->fifo, key_table[i - keyboard_offset] + 256);
					}
				}
				/*这些都是那些不能打印的特殊按键，它们对应的都是0*/
				//按下了退格键的情况，退格键的号码是0x0e
				if((keyboard_offset + 0x0e == i))				
				{
					if(0 == key_to)
					{
						//只有当前的输入位置上有字符了才能向前删除
						if(cursor_x > 8)
						{
							cursor_x -= 8;
							/*本来退格键应该只填充一个空格就行的，但是这样的话没办法刷新光标所在的那块儿区域了，
							**会留下光标的黑色痕迹，所以直接填充两个空格，刷新之后就不会有痕迹了
							*/
							displayStrings_atLayer(layer_window, cursor_x, 28, COL8_000000, COL8_FFFFFF, "  ");
						}
					}
					else
					{
						fifo_put(&task_console->fifo, 8 + keyboard_offset);
					}
				}
				//按下TAB键的处理情况，切换输入窗口
				if(keyboard_offset + 0x0f == i)					
				{
					if(0 == key_to)			//当前最前面窗口为task_a，则让console窗口成为最前端窗口
					{
						key_to = 1;
						create_titlebar(buf_window, layer_window->length, "task_a", INACTIVE);
						create_titlebar(buf_console, layer_console->length, "console", ACTIVE);
					}
					else					//task_a任务的窗口成为最前端窗口
					{
						key_to = 0;
						create_titlebar(buf_window, layer_window->length, "task_a", ACTIVE);
						create_titlebar(buf_console, layer_console->length, "console", INACTIVE);
					}
					layer_part_refresh(layer_window, 0, 0, layer_window->length, 21);
					layer_part_refresh(layer_console, 0, 0, layer_console->length, 21);
				}
				//由于光标坐标后退了，为了及时显示它，需要立即重绘光标
				drawRectangle(layer_window->buffer, layer_window->length, cursor_color, cursor_x, 28, 2, 15);
				layer_part_refresh(layer_window, cursor_x, 28, cursor_x + 8, 44);
			}
			//如果是鼠标中断的话，需要处理鼠标发送来的中断数据
			else if((mouse_offset <= i) && (i <= mouse_offset + 255))		
			{
				if(0 != mouse_decode(&mouse_dec, i - mouse_offset))		//只有返回值为1的时候才说明成功接收完成一次鼠标的中断
				{
				/*显示鼠标的信息*/
					sprintf(strings,"[lcr %3d,%3d]",mouse_dec.x, mouse_dec.y);
					if(0 != (mouse_dec.btn & 0x01))		//按下了左键
					{
						strings[1] = 'L';
						layer_slide(layer_window, mCursorX - 80, mCursorY -8);		//这行代码是移动窗口的
					}
					if(0 != (mouse_dec.btn & 0x02))		//按下了右键
					{
						strings[3] = 'R';
					}
					if(0 != (mouse_dec.btn & 0x04))		//按下了中键
					{
						strings[2] = 'C';
					}
					displayStrings_atLayer(layer_bg, 0, 16, COL8_FFFFFF, COL8_008484, strings);	
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
					displayStrings_atLayer(layer_bg, 40, 32, COL8_FFFFFF, COL8_008484, strings);
					layer_slide(layer_mouse, mCursorX, mCursorY);
				}
			}
			//如果是定时器中断的话，需要处理定时器对应的任务
			else if(1 == i || 0 == i)				
			{
				if(0 != i)				//这个timer的数据为1的时候显示光标
				{
					timer_init(timer, &fifo_mutual, 0);
					cursor_color = COL8_000000;
				}
				else					//这个timer的数据为0的时候不显示光标
				{
					timer_init(timer, &fifo_mutual, 1);
					cursor_color = COL8_FFFFFF;
				}
				timer_set(timer, 50);
				drawRectangle(layer_window->buffer, layer_window->length, cursor_color, cursor_x, 28, 2, 15);
				layer_part_refresh(layer_window, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}	
}








//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

//由于用到了sprintf这个函数，所以需要这个头文件，但是这个函数不需要调用任何操作系统的功能，所以可以这么写（因为我们本来就是在做操作系统，所以不能有依赖操作系统的函数）
#include <stdio.h>
#include <string.h>
#include "bootpack.h"

extern TIMERCTL timerctl;
extern TIMER *task_timer;

void keywindow_on(LAYER *key_to_window);
void keywindow_off(LAYER *key_to_window);

void HariMain(void)
{
	/*
	**最初的这部分变量没有通过内存管理来分配，它们本身属于操作系统的一部分，存在于bootpack.hrb所在的那块儿内存空间中
	*/
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER_MANAGE *layer_manage;
	LAYER *layer_bg, *layer_mouse, *layer_console[2];
	FIFO fifo_mutual;									//缓冲区管理，所有中断公用的
	FIFO keycmd_fifo;									
	TASK *task_a, *task_console[2], *task;				//后面的那个task只是作为一个临时变量来用的
	CONSOLE *console;
	int fifobuf[128], keycmd_buf[32], *console_fifo[2];											//缓冲区
	int mCursorX, mCursorY;				//鼠标光标显示位置的横纵坐标
	int key_shift = 0, key_ctrl = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;			//标志键盘输入到哪个窗口中
	int j, x, y, last_mcursorX = -1, last_mcursorY= -1, layerX = 0;			//last_mcursorX这两个变量记录的是鼠标移动之前的坐标
	LAYER *layer = 0, *key_to_window;							//key_to_window这个变量记录当前的键盘输入到哪个窗口中
	unsigned int memory_total, i;
	char buf_cursor[mCursorWidth * mCursorHeight];
	unsigned char *buf_bg, *buf_console[2];		//屏幕的大背景会在init_screen的时候画出来，这里只需要一个指向它的指针即可
	unsigned char strings[40];
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//减去下方任务栏的高度
	
	
	/*内存检查*/
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);	//i的单位是MB
	
	/*内存管理*/
	memmanage_init(memmanage);
	memory_total = i * 1024 * 1024;
	memmanage_free_4K(memmanage, 0x00001000, 0x0009e000);
	memmanage_free_4K(memmanage, 0x00400000, memory_total - 0x00400000);
	
	init_fifo(&keycmd_fifo, 32, keycmd_buf, 0);
	/*初始化接收中断的缓冲区*/
	init_fifo(&fifo_mutual, 128, fifobuf, 0);				//初始化mouseFIFO缓冲区，现在还没任务，先指定为0
	/*初始化GDT和IDT表以及PIC板的数据*/
	init_GDTandIDT();				//初始化GDT和IDT表
	init_pic();						//初始化PIC主从板数据，除了IRQ2禁止了全部中断
	io_sti();						//开始接收中断
	/*初始化PIT中断控制*/
	init_PIT();
		/*若要接收鼠标中断需要两个步骤，首先必须使鼠标控制电路（就是键盘控制电路的一部分）有效，然后要使鼠标本身有效*/
	init_keyboard(&fifo_mutual, 256);						//初始化键盘控制器电路
	enable_mouse(&fifo_mutual, 512, &mouse_dec);			//激活鼠标
	/*开放各种中断*/
	io_out8(PIC0_IMR, 0xf8);			//PIC0开发IRQ(11111000)，开放IRQ0、IRQ1和IRQ2，定时器、键盘中断和从PIC板
	io_out8(PIC1_IMR, 0xef); 			//PIC1开放IRQ(11101111)， 开放鼠标中断
	/*初始化任务切换管理*/
	task_a = task_init(memmanage);		//这个task_a其实代表的就是这些鼠标键盘等的任务
	fifo_mutual.task = task_a;			//为鼠标键盘等的缓冲区指定唤醒任务为task_a
	task_run(task_a, 1, 2);
	/*初始化调色板，为图形界面做准备*/
	init_palette();						//初始化调色板
	/*初始化图层管理，并且初始化鼠标光标和背景的图层*/
	layer_manage = layer_man_init(memmanage, binfo->vram, binfo->scrnx, binfo->scrny);
	*((int *) 0x0fe4) = (int) layer_manage;
	
	layer_bg = layer_alloc(layer_manage);			//为背景分配图层
	layer_mouse = layer_alloc(layer_manage);		//为鼠标分配图层
	
	buf_bg = (unsigned char *)memmanage_alloc_4K(memmanage, binfo->scrnx * binfo->scrny);		//为背景图形的内容分配内存
	/*为各个图形的图层内容进行设定*/
	layer_set(layer_bg, buf_bg, binfo->scrnx, binfo->scrny, -1);			
	layer_set(layer_mouse, buf_cursor, 16, 16 ,99);
	/*初始化整个桌面背景*/
	init_screen(buf_bg, binfo->scrnx, binfo->scrny);				//这个时候的init_screen不再是直接画出背景，而是在mBg内存地址中填写好背景内容
	layer_slide(layer_bg, 0, 0);									//把背景图层从(0,0)坐标开始画
	/*初始化鼠标图标*/
	init_mouse_cursor(buf_cursor, 99);								//初始化鼠标光标
	layer_slide(layer_mouse, mCursorX, mCursorY);					//现在显示图形不需要再用displayShape函数了，直接用这个图层管理的绘图函数就行

	/*执行两个命令行窗口任务*/
	for(i = 0; i < 2; i++)
	{
		layer_console[i] = layer_alloc(layer_manage);
		buf_console[i] = (unsigned char *)memmanage_alloc_4K(memmanage, 256 * 165);	
		layer_set(layer_console[i], buf_console[i], 256, 165, -1);
		create_window(buf_console[i], 256, 165, "console", INACTIVE);
		create_textbox(layer_console[i], 8, 28, 240, 128, COL8_000000);
		task_console[i] = task_alloc();
		task_console[i]->tss.esp = memmanage_alloc_4K(memmanage, 64 * 1024) + 64 * 1024 - 12;		//由于这里要传入两个参数，所以减去了12
		task_console[i]->tss.eip = (int) &console_task;		//eip指向的是运行console的函数首地址
		task_console[i]->tss.es = 1 * 8;
		task_console[i]->tss.cs = 2 * 8;
		task_console[i]->tss.ss = 1 * 8;
		task_console[i]->tss.ds = 1 * 8;
		task_console[i]->tss.fs = 1 * 8;
		task_console[i]->tss.gs = 1 * 8;
		*((int *) (task_console[i]->tss.esp + 4)) = (int) layer_console[i];
		*((int *) (task_console[i]->tss.esp + 8)) = memory_total;	
		task_run(task_console[i], 2, 2); 			/* level=2, priority=2 */
		layer_console[i]->task = task_console[i];
		layer_console[i]->flags |= 0x20;			//flags的0x20代表当前窗口的光标ON
		console_fifo[i] = (int *)memmanage_alloc_4K(memmanage, 128 * 4);
		init_fifo(&task_console[i]->fifo, 128, console_fifo[i], task_console[i]);
	}
	
	
	key_to_window = layer_console[0];					//最初的时候由第一个console接收键盘输入
	
	layer_slide(layer_console[0], 8, 2);
	layer_slide(layer_console[1], 300, 2);
	/*设置好各个图层的高度*/
	layer_switch(layer_bg, 0);						//把背景图层调为最底层，高度为0
	layer_switch(layer_console[0], 1);					//命令行窗口图层调节为第三层，高度为2
	layer_switch(layer_console[1], 2);
	layer_switch(layer_mouse, 3);					//鼠标图层调为最高层，高度为3	
	keywindow_on(key_to_window);
	
	//
	fifo_put(&keycmd_fifo, KEYCMD_LED);
	fifo_put(&keycmd_fifo, key_leds);
	
	for(;;)
	{ 
		if(fifo_status(&keycmd_fifo) > 0 && keycmd_wait < 0)
		{
			keycmd_wait = fifo_get(&keycmd_fifo);
			wait_KBC_sendready();
			io_out8(PORT_KEYDATA, keycmd_wait);
		}
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
			//窗口被关闭的情况
			if(0 == key_to_window->flags)
			{
				key_to_window = layer_manage->layers_order[layer_manage->top - 1];			//当窗口被关闭的时候，自动切换成最上层的layer接收键盘输入
				keywindow_on(key_to_window);
			}
			//如果是键盘中断的话，需要处理键盘发送来的中断数据
			if((keyboard_offset <= i) && (i <= keyboard_offset + 255))			
			{
				//判断shift键是否按下，然后使用不同的keytable表
				if(i < 0x80 + keyboard_offset)
				{
					if(0 == key_shift)
					{
						strings[0] = key_table[i - keyboard_offset];
					}
					else
					{
						strings[0] = key_table_shift[i - keyboard_offset];
					}
				}
				else
				{
					strings[0] = 0;
				}
				//如果同时按下了ctrl+x，那么就关闭应用程序
				if(('x' == strings[0] || 'X' == strings[0]) && (0 != key_ctrl))
				{
					task = key_to_window->task;				//找到当前接收键盘输入的（即最前面的）图层所对应的任务
					if(0 != task && task_console[0]->tss.ss0 != 0)
					{
						console_putstring_toend(task->console, "\nTerminate program! :\n");
						io_cli();
						//让CPU直接跳转到console中执行，不再管应用程序，也就是所谓的停止应用程序继续运行
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
					}
					
				}
				//判断Caps和Shift键的情况以确定输入大写还是小写字母
				if('A' <= strings[0] && strings[0] <= 'Z')
				{
					//小写字母的情况
					if((0 == (key_leds & 4) && 0 == key_shift) || (0 != (key_leds & 4) && 0 != key_shift))
					{
						strings[0] += 0x20;
					}
				}
				//判断按下的字符是否为一般字符，0对应的字符不处理，对退格键和回车键加上了对应字符，他们对应的table表里的元素不再是0
				if(strings[0] != 0)				
				{
					fifo_put(&key_to_window->task->fifo, strings[0] + 256);
				}
				/*这些都是那些不能打印的特殊按键，它们对应的都是0*/
				//按下“TAB键”的处理情况，切换输入窗口
				if(keyboard_offset + 0x0f == i)					
				{
					//先让当前的窗口的光标OFF掉
					keywindow_off(key_to_window);
					j = key_to_window->height - 1;
					if(0 == j)
					{
						//如果切换之后的图层是最底层的操作系统的图层，那么就切换成最上层的图层
						j = layer_manage->top - 1;
					}
					key_to_window = layer_manage->layers_order[j];
					//使切换到的窗口的光标ON
					keywindow_on(key_to_window);
				}
				//左或右Ctrl ON
				if(0x1d + keyboard_offset == i)
				{
					key_ctrl = 1;
				}
				//左或右Ctrl OFF
				if(0x9d + keyboard_offset == i)
				{
					key_ctrl = 0;
				}
				//左shift ON
				if(0x2a + keyboard_offset == i)
				{
					key_shift |= 1;
				}
				//右shift ON
				if(0x36 + keyboard_offset == i)
				{
					key_shift |= 2;
				}
				//左shift OFF
				if(0xaa + keyboard_offset == i)
				{
					key_shift &= ~1;
				}
				//左shift OFF
				if(0xb6 + keyboard_offset == i)
				{
					key_shift &= ~2;
				}
				/*对各种锁定键的处理*/
				//CapsLock键
				if(i == keyboard_offset + 0x3a)
				{
					key_leds ^= 4;
					fifo_put(&keycmd_fifo, KEYCMD_LED);
					fifo_put(&keycmd_fifo, key_leds);
				}
				//NumLock键
				if(i == keyboard_offset + 0x45)
				{
					key_leds ^= 2;
					fifo_put(&keycmd_fifo, KEYCMD_LED);
					fifo_put(&keycmd_fifo, key_leds);
				}
				//ScrollLock键
				if(i == keyboard_offset + 0x46)
				{
					key_leds ^= 1;
					fifo_put(&keycmd_fifo, KEYCMD_LED);
					fifo_put(&keycmd_fifo, key_leds);
				}
				//键盘成功接收到数据
				if(i == keyboard_offset + 0xfa)
				{
					keycmd_wait = -1;
				}
				//键盘没有成功接收到数据
				if(i == keyboard_offset + 0xfe)
				{
					wait_KBC_sendready();
					io_out8(PORT_KEYDATA, keycmd_wait);
				}
			}
			//如果是鼠标中断的话，需要处理鼠标发送来的中断数据
			else if((mouse_offset <= i) && (i <= mouse_offset + 255))		
			{
				if(0 != mouse_decode(&mouse_dec, i - mouse_offset))		//只有返回值为1的时候才说明成功接收完成一次鼠标的中断
				{
					if(0 != (mouse_dec.btn & 0x01))		//按下了左键
					{
						//如果last_mcursorX小于0，说明当前不是处于窗口移动模式
						if(last_mcursorX < 0)
						{
							//layer_slide(layer_window, mCursorX - 80, mCursorY -8);		//这行代码是移动窗口的
							for(j = layer_manage->top - 1; j > 0; j--)
							{
								layer = layer_manage->layers_order[j];
								//计算一下鼠标的坐标在不在这个图层里面
								x = mCursorX - layer->x;
								y = mCursorY - layer->y;
								if(0 <= x && x < layer->length && 0 <= y && y <= layer->width)
								{
									//鼠标所在区域也不能是涂层的透明区域
									if(layer->buffer[y * layer->length + x] != layer->color_luc)
									{
										layer_switch(layer, layer_manage->top - 1);
										//如果当前的layer不是接收键盘输入的图层（即最前面的图层），那么就需要切换
										if(layer != key_to_window)
										{
											//鼠标左击窗口所做的事情跟按下TAB键的事情差不多
											keywindow_off(key_to_window);
											key_to_window = layer;
											keywindow_on(key_to_window);
										}
										//查看鼠标当前点击的区域是不是窗口的标题栏，并且不能是关闭按钮所在的区域
										if(3 <= x && x < layer->length - 21 && 3 <= y && y <= 21)
										{
											last_mcursorX = mCursorX;
											last_mcursorY = mCursorY;
											layerX = layer->x;
										}
										//这个是鼠标点击到关闭按钮的情况
										if(layer->length - 21 <= x && x <= layer->length - 5 && 5 <= y && y <= 19)
										{	
											//查看该窗口是否为应用程序的窗口，如果是应用程序产生的窗口而不是console的那个黑窗口，那么这个窗口的flags有0x10
											if(0 != (layer->flags & 0x10))
											{
												//这部分处理跟按下ctrl+x的处理基本一样
												task = layer->task;
												console_putstring_toend(task->console, "Break by mouse!\n");
												io_cli();
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int)asm_end_app;
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						//只要last_mcursorX大于0，说明当前处于窗口移动模式
						else
						{
							x = mCursorX - last_mcursorX;
							y = mCursorY - last_mcursorY;
							layer_slide(layer, (layerX + x + 2) & ~3, layer->y + y);
							//last_mcursorX = mCursorX;
							last_mcursorY = mCursorY;
						}
					}
					//如果当前没有按下左键，就把last_mcursorX设置为负数，作为当前不处于窗口移动的依据
					else
					{
						last_mcursorX = -1;
					}
					
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
					//重绘鼠标光标的位置
					layer_slide(layer_mouse, mCursorX, mCursorY);
				}
			}
		}
	}	
}




void keywindow_on(LAYER *key_to_window)
{
	//先改变这个窗口的标题栏的状态
	change_titlebar(key_to_window, 1);
	//如果是应用窗口的话，它的flags的0x20位设置成1了；应用窗口是不需要显示光标的
	if(0 != (key_to_window->flags & 0x20))
	{
		fifo_put(&key_to_window->task->fifo, 2);			//命令行窗口的光标OFF掉，因为当前是应用程序窗口在接收键盘输入
	}
	return ;
}


void keywindow_off(LAYER *key_to_window)
{
	//先改变这个窗口的标题栏的状态
	change_titlebar(key_to_window, 0);
	//如果是应用窗口的话，它的flags的0x20位设置成1了；应用窗口是不需要显示光标的
	if(0 != (key_to_window->flags & 0x20))
	{
		fifo_put(&key_to_window->task->fifo, 3);			//命令行窗口的光标OFF掉，因为当前是应用程序窗口在接收键盘输入
	}
	return ;
}




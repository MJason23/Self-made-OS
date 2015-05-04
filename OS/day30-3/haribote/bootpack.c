//这部分代码的作用：操作系统的主体程序，这才是正式的操作系统
//由于对特定装置的读写，中断的调用直接用C语言无法实现，所以有些函数需要用汇编语言来实现

//由于用到了sprintf这个函数，所以需要这个头文件，但是这个函数不需要调用任何操作系统的功能，所以可以这么写（因为我们本来就是在做操作系统，所以不能有依赖操作系统的函数）
#include <stdio.h>
#include <string.h>
#include "bootpack.h"

extern TIMERCTL timerctl;
extern TIMER *task_timer;
extern TASKCTL *taskctl;

void keywindow_on(LAYER *key_to_window);
void keywindow_off(LAYER *key_to_window);
void close_console_task(TASK *task);
void close_console(LAYER *layer);

void HariMain(void)
{
	/*
	**最初的这部分变量没有通过内存管理来分配，它们本身属于操作系统的一部分，存在于bootpack.hrb所在的那块儿内存空间中
	*/
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER_MANAGE *layer_manage;
	LAYER *layer_bg, *layer_mouse;
	FIFO fifo_mutual;									//缓冲区管理，所有中断公用的
	FIFO keycmd_fifo;									
	TASK *task_a, *task;				//后面的那个task只是作为一个临时变量来用的
	int fifobuf[128], keycmd_buf[32], *console_fifo[2];							//缓冲区
	int mCursorX, mCursorY;				//鼠标光标显示位置的横纵坐标
	int key_shift = 0, key_ctrl = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;			//标志键盘输入到哪个窗口中
	int j, x, y, last_mcursorX = -1, last_mcursorY= -1, tmp_layerX = 0;			//last_mcursorX这两个变量记录的是鼠标移动之前的坐标
	int new_mcursorX = -1, new_mcursorY = 0, new_layerX = 0x7fffffff, new_layerY = 0;
	LAYER *layer = 0, *key_to_window, *layer_to_free;							//key_to_window这个变量记录当前的键盘输入到哪个窗口中
	unsigned int memory_total, i;
	unsigned char buf_cursor[mCursorWidth * mCursorHeight];
	unsigned char *buf_bg;		//屏幕的大背景会在init_screen的时候画出来，这里只需要一个指向它的指针即可
	char strings[40];
	int *fat;
	unsigned char *hzset;
	FILEINFO *finfo;
	extern char charset[4096];
	
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//减去下方任务栏的高度
	
	
	/*初始化GDT和IDT表以及PIC板的数据*/
	init_GDTandIDT();				//初始化GDT和IDT表
	init_pic();						//初始化PIC主从板数据，除了IRQ2禁止了全部中断
	io_sti();						//开始接收中断
	
	init_fifo(&keycmd_fifo, 32, keycmd_buf, 0);
	/*初始化接收中断的缓冲区*/
	init_fifo(&fifo_mutual, 128, fifobuf, 0);				//初始化mouseFIFO缓冲区，现在还没任务，先指定为0
	
	/*初始化PIT中断控制*/
	init_PIT();
		/*若要接收鼠标中断需要两个步骤，首先必须使鼠标控制电路（就是键盘控制电路的一部分）有效，然后要使鼠标本身有效*/
	init_keyboard(&fifo_mutual, 256);						//初始化键盘控制器电路
	enable_mouse(&fifo_mutual, 512, &mouse_dec);			//激活鼠标
	
	/*开放各种中断*/
	io_out8(PIC0_IMR, 0xf8);			//PIC0开发IRQ(11111000)，开放IRQ0、IRQ1和IRQ2，定时器、键盘中断和从PIC板
	io_out8(PIC1_IMR, 0xef); 			//PIC1开放IRQ(11101111)， 开放鼠标中断
	
	/*内存检查*/
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);	//i的单位是MB
	
	/*内存管理*/
	memmanage_init(memmanage);
	memory_total = i * 1024 * 1024;
	memmanage_free_4K(memmanage, 0x00001000, 0x0009e000);
	memmanage_free_4K(memmanage, 0x00400000, memory_total - 0x00400000);
	/*初始化调色板，为图形界面做准备*/
	init_palette();						//初始化调色板
	
	/*初始化图层管理，并且初始化鼠标光标和背景的图层*/
	layer_manage = layer_man_init(memmanage, binfo->vram, binfo->scrnx, binfo->scrny);
	*((int *) 0x0fe4) = (int) layer_manage;
	
	/*初始化任务切换管理*/
	task_a = task_init(memmanage);		//这个task_a其实代表的就是这些鼠标键盘等的任务
	fifo_mutual.task = task_a;			//为鼠标键盘等的缓冲区指定唤醒任务为task_a
	task_run(task_a, 1, 2);
	task_a->lanmode = 0;					//默认的语言模式是英文模式
	
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

	/*载入汉字字符库*/
	fat = (int *)memmanage_alloc_4K(memmanage, 4 * 2880);
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));
	finfo = file_search("hzset.fnt", (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	//找到这个字符库文件的话，就把字符库载入
	if(0 != finfo)
	{
		i = finfo->size;				//为了防止finfo->size的值被修改，用了i做中间变量
		hzset = file_load_tekfile(finfo->clusterno, &i, fat);		//函数内部就已经给要解压的文件分配好了内存
	}
	//找不到字符库文件的话，就把内置英文字符库载入，后面的字符全部用方块（0xff）填充
	else
	{
		hzset = (unsigned char *)memmanage_alloc_4K(memmanage, 16 * 256 + 32 * 5170);		//共256个半角英文字符，5170个全角中文字符
		for(i = 0; i < 16 * 256; i++)
		{
			hzset[i] = charset[i];
		}
		for(i = 16 * 256; i < 16 * 256 + 32 * 5170; i++)
		{
			hzset[i] = 0xff;
		}
	}
	*((int *) 0x0fe8) = (int)hzset;
	memmanage_free_4K(memmanage,(int)fat, 4 * 2880);
	
	/*执行一个命令行窗口任务*/
	key_to_window = open_console(layer_manage, memory_total);		//最初的时候由第一个console接收键盘输入
	
	layer_slide(key_to_window, 8, 2);
	/*设置好各个图层的高度*/
	layer_switch(layer_bg, 0);						//把背景图层调为最底层，高度为0
	layer_switch(key_to_window, 1);					//命令行窗口图层调节为第三层，高度为2
	layer_switch(layer_mouse, 2);					//鼠标图层调为最高层，高度为3	
					
	keywindow_on(key_to_window);
	
	//
	fifo_put(&keycmd_fifo, KEYCMD_LED);
	fifo_put(&keycmd_fifo, key_leds);
	
	*((int *) 0x0fec) = (int) &fifo_mutual;			
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
			//利用CPU空闲的时间来执行耗时的绘图工作(这样的话会造成反应很快的表象，但其实，如果中断很多，尤其是鼠标中断很多的时候，
			//会把之前记录下的new_mcursorX覆盖掉，而只绘制最新的图层坐标)
			if(new_mcursorX >= 0)
			{
				io_sti();
				layer_slide(layer_mouse, new_mcursorX, new_mcursorY);
				new_mcursorX = -1;				//绘制完鼠标之后，就把它的坐标设置为负数，反正鼠标不会出去屏幕，为负数的时候就说明不需要重绘鼠标
			}
			//图层的横坐标为0x7fffffff的时候就不需要重绘了，这是作为一个标志的，它为其他值的时候就需要重绘了
			else if(new_layerX != 0x7fffffff)
			{
				io_sti();
				layer_slide(layer, new_layerX, new_layerY);
				new_layerX = 0x7fffffff;
			}
			//如果不需要重绘鼠标和图层的话，就没有什么工作可做了，进入休眠就可以了
			else
			{
				task_sleep(task_a);	//如果没有需要处理的数据，就自己让自己休眠
				/*上面的那个函数直接就跳转到另一个任务中去了，不过，因为每个人物的默认eflags都是开启中断的，
				**所以不用担心，这个任务还是能被唤醒的，耳环醒后的第一个动作就是为了以防万一先开启中断
				*/
				io_sti();		//万一只有自己在运行的话，则无法睡眠，那么就执行hlt指令好了，这样省电嘛	
			}
		}
		else
		{
			i = fifo_get(&fifo_mutual);
			io_sti();
			//窗口被关闭的情况
			if(0 != key_to_window && 0 == key_to_window->flags)
			{
				//如果当前画面上只剩下鼠标和背景的时候
				if(1 == layer_manage->top)
				{
					key_to_window = 0;
				}
				else
				{
					key_to_window = layer_manage->layers_order[layer_manage->top - 1];			//当窗口被关闭的时候，自动切换成最上层的layer接收键盘输入
					keywindow_on(key_to_window);
				}
			}
			/*
			**有一点需要注意，键盘输入只能由console来接收，如果只剩下task_a任务了，那么这个主任务就只接收打开窗口console的输入
			**其他输入一律不处理，否则会陷入死循环
			*/
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
				//切记key_to_window也就是当前的键盘输入接收图层不能使layer_bg，因为这会造成死循环一直向task_a的fifo中无限填充刚刚输入的字符
				if(strings[0] != 0 && key_to_window != 0 && key_to_window != layer_bg)				
				{
					fifo_put(&key_to_window->task->fifo, strings[0] + 256);
				}
				/*这些都是那些不能打印的特殊按键，它们对应的都是0*/
				//按下“TAB键”的处理情况，切换输入窗口
				if(i == keyboard_offset + 0x0f && key_to_window != 0 && key_to_window != layer_bg)					
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
				
				//如果按下了F2，那么就关闭应用程序
				if((i == keyboard_offset + 0x3C) && key_to_window != 0 && key_to_window != layer_bg)
				{
					task = key_to_window->task;				//找到当前接收键盘输入的（即最前面的）图层所对应的任务
					if(0 != task && task->tss.ss0 != 0)
					{
						console_putstring_toend(task->console, "\nTerminate program! :\n");
						io_cli();
						//让CPU直接跳转到console中执行，不再管应用程序，也就是所谓的停止应用程序继续运行
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						/*其实在console任务没有什么中断可处理的时候就会进入休眠，强制结束的时候需要先唤醒它，让他执行结束程序处理
						**但是不唤醒的话，在这个应用程序对应的console窗口有显示的时候也可以停止，只是它需要由定时器每隔0.5秒来唤醒一次
						**所以看起来反应并不及时，最大反应时间为0.5秒
						*/
						task_run(task, -1, 0);
					}
					
				}
				//如果按下了F1，就打开一个console
				if((i == keyboard_offset + 0x3B))
				{
					if(key_to_window != 0)
					{
						keywindow_off(key_to_window);
					}					
					key_to_window = open_console(layer_manage, memory_total);
					layer_slide(key_to_window, 300, 2);
					layer_switch(key_to_window, layer_manage->top);
					//自动把键盘输入切换到这个新打开的console中
					keywindow_on(key_to_window);
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
					//记录下鼠标光标的移动到的位置，等待fifo中为空的时候开始绘图
					new_mcursorX = mCursorX;
					new_mcursorY = mCursorY;
					//按下了左键
					if(0 != (mouse_dec.btn & 0x01))
					{
						//如果last_mcursorX小于0，说明当前不是处于窗口移动模式，那么就需要检查现在鼠标按下左键之后有没有落在某个图层的标题栏上
						if(last_mcursorX < 0)
						{
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
											tmp_layerX = layer->x;		//这个tmp_layerX只是个要绘制的图层横坐标的过渡部分，真正的坐标还需要处理
											new_layerY = layer->y;
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
												task_run(task, -1, 0);
											}
											//不是应用程序的话就是console命令行窗口产生的layer
											else
											{
												//不能再这里的task_a中直接关闭，因为还有些console自身管理的定时器和fat等东西无法释放了
												task = layer->task;
												/*这里只是暂时隐藏console的图层，因为对于有的应用程序的命令行窗口关闭需要消耗很多时间，
												**所以为了让用户感觉关闭的很快，就先隐藏，具体的释放等费时的工作交给后面做
												*/
												layer_switch(layer, -1);
												keywindow_off(key_to_window);
												key_to_window = layer_manage->layers_order[layer_manage->top - 1];
												keywindow_on(key_to_window);
												io_cli();
												fifo_put(&task->fifo, 4);
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						//只要last_mcursorX大于0，说明当前处于窗口移动模式，说明当前鼠标正点击着某个图层的标题栏呢
						else
						{
							x = mCursorX - last_mcursorX;
							y = mCursorY - last_mcursorY;
							new_layerX = (tmp_layerX + x + 2) & ~3;
							new_layerY = new_layerY + y;
							last_mcursorY = mCursorY;			//更新到移动后的坐标
						}
					}
					//如果当前没有按下左键，就把last_mcursorX设置为负数，作为当前不处于窗口移动的依据
					else
					{
						last_mcursorX = -1;
						if(0x7fffffff != new_layerX)
						{
							layer_slide(layer, new_layerX, new_layerY);		//一旦鼠标不点击这窗口移动了，就立即最后一次绘制窗口并且停止窗口的移动
							new_layerX = 0x7fffffff; 
						}
					}
					
				}
			}
			//收到这个之间的数据，说明是让关闭console的
			else if(768 <= i && i <= 1023)
			{
				close_console(layer_manage->layers + (i - 768));
			}
			//关闭命令行的任务
			else if(1024 <= i && i <= 2023)
			{
				close_console_task(taskctl->tasks + (i - 1024));
			}
			//关闭命令行窗口（释放它的图层）
			else if(2024 <= i && i <= 2279)			//图层最多只有256个，加上偏移量之后就是这个范围了
			{
				layer_to_free = layer_manage->layers + (i - 2024);
				memmanage_free_4K(memmanage, (int) layer_to_free->buffer, 256 * 265);
				layer_free(layer_to_free);
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

//启动一个命令行窗口任务
TASK *open_console_task(LAYER *layer, unsigned int mem_total)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	TASK *task_console = task_alloc();
	int *console_fifo = (int *)memmanage_alloc_4K(memmanage, 128 * 4);
	task_console->console_stack = memmanage_alloc_4K(memmanage, 64 * 1024);			//由于这里要传入两个参数，所以减去了12
	task_console->tss.esp = task_console->console_stack + 64 * 1024 - 12;
	task_console->tss.eip = (int) &console_task;			//eip指向的是运行console的函数首地址
	task_console->tss.es = 1 * 8;
	task_console->tss.cs = 2 * 8;
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
	*((int *) (task_console->tss.esp + 4)) = (int) layer;
	*((int *) (task_console->tss.esp + 8)) = mem_total;	
	task_run(task_console, 2, 2); 			/* level=2, priority=2 */
	init_fifo(&task_console->fifo, 128, console_fifo, task_console);
	return task_console;
}

//打开新的console的函数
LAYER *open_console(LAYER_MANAGE *layer_manage, unsigned int memory_total)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER *layer_console = layer_alloc(layer_manage);
	unsigned char *buf_console = (unsigned char *)memmanage_alloc_4K(memmanage, 256 * 165);	
	layer_set(layer_console, buf_console, 256, 165, -1);
	create_window(buf_console, 256, 165, "console", INACTIVE);
	create_textbox(layer_console, 8, 28, 240, 128, COL8_000000);
	layer_console->task = open_console_task(layer_console, memory_total);
	//layer_console->task->console->view_length = 240;
	//layer_console->task->console->view_width = 128;
	layer_console->flags |= 0x20;			//flags的0x20代表当前窗口的光标ON
	return layer_console;
}

void close_console_task(TASK *task)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	//先让当前的这个console的任务休眠之后，不再分配给它CPU时间，然后就可以安心的释放它控制的内存了
	task_sleep(task);
	//先释放栈所占用的内存
	memmanage_free_4K(memmanage, task->console_stack, 64 * 1024);
	//然后释放fifo所占用的内存
	memmanage_free_4K(memmanage, (int) task->fifo.buffer, 128 * 4);
	task->flags = UNUSED;
	return ;	
}

void close_console(LAYER *layer)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	TASK *task = layer->task;
	//先释放图层所占用的内存
	memmanage_free_4K(memmanage, (int) layer->buffer, 256 * 165);
	//然后释放图层
	layer_free(layer);
	//最后关闭这个console所占用的任务
	close_console_task(task);
	return ;
}








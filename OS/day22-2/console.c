#include "bootpack.h"
#include <string.h>

/*关于API的理解：以这些实际的代码来说明应用程序调用API。首先通过命令行输入要启动的程序的名字，然后执行console_runcmd，如果发现是
**应用程序，就执行cmd_app，然后查找这个应用程序是否存在，如果存在就为它设置好代码段和数据段，然后运行start_app从操作系统的段跳转到那个代码段开始
**执行，这个应用程序如果中间调用到了操作系统的函数，就会产生INT0x40中断，这个中断并不是真正的中断，只是根据参数相应的执行了一个
**系统函数而已，这个中断的处理函数就是asm_hrb_api，它调用的是hrb_api，这个hrb_api就是真正的API，它会根据传入的参数来决定调用哪个
**系统函数（目前就是显示单个字符和指定字符串以及指定长度字符串的函数），执行完成后再返回到应用程序，应用程序执行完之后再返回到操作
**系统。由于在向GDT中注册应用程序的段的时候，它的权限就设置成了+0x60的形式，CPU就会认为这是应用程序专用的，执行应用程序的中间如果
**产生了中断，CPU会自行切换到操作系统的栈开始执行中断处理程序。如果这个应用程序是一个企图破坏操作系统的程序，例如企图修改不属于自己的
**内存的内容或者企图修改DS等关键寄存器，就会产生INT0D中断，这个中断处理程序会结束当前的程序，返回到cmd_app当中（即操作系统）。
*/


//在命令行中显示单个指定字符的函数
void console_putchar(CONSOLE *console, int string, char move)
{
	char strings[2];
	strings[0] = string;
	strings[1] = 0;
	//如果这个字符是制表符的话
	if(0x09 == strings[0])
	{
		for(;;)
		{
			displayStrings_atLayer(console->layer, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, " ");
			console->cursor_x += 8;
			//如果到达命令行窗口的最右端则需要换行
			if(console->cursor_x >= 8 + 240)
			{
				console_newline(console);
			}
			//一个字符是八个像素的宽度，每个制表符相隔4个字符
			if(0 == ((console->cursor_x - 8) & 0x1f))
			{
				break;
			}
		}
	}
	//如果这个字符是换行符的话
	else if(0x0a == strings[0])
	{
		console_newline(console);
	}
	//如果这个字符是回车符的话
	else if(0x0d == strings[0])
	{
		/*暂时不做处理*/
	}
	//如果是正常字符的话
	else
	{
		displayStrings_atLayer(console->layer, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, strings);
		//如果move不为0，则光标不后移
		if(0 != move)
		{
			console->cursor_x += 8;
			if(console->cursor_x >= 8 + 240)
			{
				console_newline(console);
			}
		}
	}
	return ;
}

//显示指定长度字符串的函数
void console_putstring_length(CONSOLE *console, char *string, int length)
{
	int i;
	for(i = 0; i < length; i++)
	{
		console_putchar(console, string[i], 1);
	}
	return ;
}

//显示整个字符串的函数
void console_putstring_toend(CONSOLE *console, char *string)
{
	for(; 0 != *string ; string++)
	{
		console_putchar(console, *string, 1);
	}
	return ;
}

//执行指定指令的函数
void console_runcmd(char *cmdline, CONSOLE *console, int *fat, unsigned int mem_total)
{
	if(0 == strcmp(cmdline, "mem"))
	{
		cmd_mem(console, mem_total);
	}
	else if(0 == strcmp(cmdline, "clear"))
	{
		cmd_clear(console);
	}
	else if(0 == strcmp(cmdline, "ls"))
	{
		cmd_ls(console);
	}
	else if(0 == strncmp(cmdline, "type ", 5))
	{
		cmd_type(console, fat, cmdline);
	}
	//不是命令但也不是空行
	else if(0 != cmdline[0])
	{
		//运行那个应用程序的任务已经在cmd_app中执行了，返回值只是告诉这里是否正常运行了而已
		if(0 == cmd_app(console, fat, cmdline))	
		{
			//如果到这里了，说明这个命令对应的既不是命令，又不是应用程序，而且不是空行
			console_putstring_toend(console, "Bad Command!\n\n");
		}
	}
	return ;
}

//显示内存信息的函数
void cmd_mem(CONSOLE *console, unsigned int mem_total)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	char strings[40];
	/*在屏幕上显示一些内存的信息*/
	sprintf(strings, "Total memory %dMB\nFree memory %dKB\n", mem_total / 1024 / 1024, memmanage_total(memmanage) / 1024);
	console_putstring_toend(console, strings);
	return ;
}

//清屏的函数
void cmd_clear(CONSOLE *console)
{
	int x, y;
	LAYER *layer = console->layer;
	for(y = 28; y < 28 + 112; y++)
	{
		for(x = 8; x < 8 + 240; x++)
		{
			layer->buffer[x + y * layer->length] = COL8_000000;
		}
	}
	layer_part_refresh(layer, 8, 28, 8 + 240, 28 + 128);
	console->cursor_y = 28;		//光标重新回到左上方
	return ;
}

//显示文件信息的函数
void cmd_ls(CONSOLE *console)
{
	FILEINFO *finfo = (FILEINFO *)(ADR_DISKIMG + 0x002600);
	int x, y;
	char strings[30];
	for(x = 0; x < 224; x++)
	{
		//如果文件信息最初一个字节是0x00的话，说明这里没有文件
		if(0x00 == finfo[x].name[0])
		{
			break;
		}
		//如果文件信息最初一个字节是0xe5的话，说明这文件已经被删除了，所以不是的话才代表有有效文件
		if(0xe5 != finfo[x].name[0])
		{
			if(0 == (finfo[x].type & 0x18))
			{
				sprintf(strings, "filename.ext   %7d", finfo[x].size);
				for(y = 0; y < 8; y++)
				{
					strings[y] = finfo[x].name[y];
				}
				//把后缀名赋值给strings的第9到第11位
				strings[9] = finfo[x].ext[0];
				strings[10] = finfo[x].ext[1];
				strings[11] = finfo[x].ext[2];
				console_putstring_toend(console, strings);
				console_newline(console);
			}
		}
	}
	console_newline(console);
	return ;
}

//显示文件内容的函数
void cmd_type(CONSOLE *console, int *fat, char *cmdline)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	FILEINFO *finfo = file_search(cmdline + 5, (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	char *img_adr;
	int i;
	//文件找到的情况，把内容逐字打印出来
	if(0 != finfo)
	{
		//文件内容开始的地址赋值给img_adr
		img_adr = (char *)memmanage_alloc_4K(memmanage, finfo->size);
		//把可能有间断的文件内容复制到一块儿连续的内存中去，这样就可以直接到这块儿内存进行连续的读取了
		file_loadfile(finfo->clusterno, finfo->size, img_adr, fat, (char *)(0x003e00 + ADR_DISKIMG));
		console_putstring_length(console, img_adr, finfo->size);
		memmanage_free_4K(memmanage, (int)img_adr, finfo->size);
	}
	//文件没有找到的情况，打印出错误信息
	else
	{
		console_putstring_toend(console, "File not found!\n");
	}
	console_newline(console);
	return ;
}

//书上P414有详解
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	int cs_base = *((int *) 0xfe8);
	CONSOLE *console = (CONSOLE *) *((int *) 0x0fec);
	TASK *task = task_now();
	if (edx == 1) 
	{
		console_putchar(console, eax & 0xff, 1);
	} 
	else if (edx == 2)
	{
		console_putstring_toend(console, (char *) ebx + cs_base);
	} 
	else if (edx == 3)
	{
		console_putstring_length(console, (char *) ebx + cs_base, ecx);
	}
	//设置为，如果传入的edx值为4，则退出这个程序
	else if(edx == 4)
	{	
		return &(task->tss.esp0);		
	}
	return 0;
}

//专门运行应用程序的命令函数，根据命令行来判断是哪个应用程序，并且运行它，如果没有找到这个应用程序，就返回0报错
int cmd_app(CONSOLE *console, int *fat, char *cmdline)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	FILEINFO *finfo;
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *)ADR_GDT;
	TASK *task = task_now();
	char name[30], *img_adr, *data_adr;
	int i;
	//根据命令行生成文件名
	for(i = 0; cmdline[i] != 0; i++)
	{
		name[i] = cmdline[i];
	}
	/*
	//这是原版的代码，真心看不懂这是在干啥……所以写成自己的了，上面那个循环就是
	for(i = 0; i < 13; i++)
	{
		if(cmdline[i] <= ' ')
		{
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;		//字符串结尾必须是0，所以这里手动加上0
	*/
	finfo = file_search(name, (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	//如果因为用户输入没有加上后缀名而找不到，就在这里加上后缀名之后再找
	if(finfo == 0 && name[i] != '.')
	{
		name[i] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	}
	//找到文件的情况
	if(0 != finfo)
	{
		//文件内容开始的地址赋值给img_adr
		img_adr = (char *)memmanage_alloc_4K(memmanage, finfo->size);
		*((int *) 0xfe8) = (int)img_adr;		//由于在hello2.nas中没有指定段指针，导致读取数据的时候用了DS寄存器，读到了错误的数据，所以这里特别指定一下地址，并存储到指定位置
		data_adr = (char *)memmanage_alloc_4K(memmanage, 64 * 1024);
		//把可能有间断的文件内容复制到一块儿连续的内存中去，这样就可以直接到这块儿内存进行连续的读取了
		file_loadfile(finfo->clusterno, finfo->size, img_adr, fat, (char *)(0x003e00 + ADR_DISKIMG));
		//为这个任务分配一个内存段，将访问权限加上0x60之后，就可以将段设置成为应用程序专用
		set_segmdesc(gdt + 1003, finfo->size - 1, (int)img_adr, AR_CODE32_ER + 0x60);
		//为应用程序分配的数据段
		set_segmdesc(gdt + 1004, 64 * 1024 - 1, (int)data_adr, AR_DATA32_RW + 0x60);
		//如果应用程序是用C语言写的，那么就要在这里把它调整为从HariMain主函数开始运行，改写最初的6个字节即可
		if(finfo->size >= 8 && 0 == strncmp(img_adr + 4, "Hari", 4))
		{
			img_adr[0] = 0xe8;
			img_adr[1] = 0x16;
			img_adr[2] = 0x00;
			img_adr[3] = 0x00;
			img_adr[4] = 0x00;
			img_adr[5] = 0xcb;
		}
		//跳转到这个hlt任务所在的内存开始执行
		start_app(0, 1003 * 8, 64 * 1024, 1004 * 8, &(task->tss.esp0));
		//任务执行完之后就把这块儿内存释放掉
		memmanage_free_4K(memmanage, (int)img_adr, finfo->size);
		memmanage_free_4K(memmanage, (int)data_adr, 64 * 1024);
		console_newline(console);
		return 1;
	}
	//没找到文件的话直接返回0
	return 0;
}


//命令行换行函数
void console_newline(CONSOLE *console)
{
	int x, y;
	LAYER *layer = console->layer;
	//如果没有超过命令行屏幕的纵屏长度，就直接加纵坐标的值
	if(console->cursor_y < 28 + 112)
	{
		console->cursor_y += 16;
	}
	//如果回车后已经到达了最底端，那么就需要滚屏，即把屏幕的所有命令全部上移一行
	else
	{
		//先把所有行上移一行
		for(y = 28; y < 28 + 112; y++)
		{
			for(x = 8; x < 8 + 240; x++)
			{
				layer->buffer[x + y * layer->length] = layer->buffer[x + (y + 16) * layer->length];
			}
		}
		//把最后一行抹成黑色
		for(y = 28 + 112; y < 28 + 128; y++)
		{
			for(x = 8; x < 8 + 240; x++)
			{
				layer->buffer[x + y * layer->length] = COL8_000000;
			}
		}
		layer_part_refresh(layer, 8, 28, 8 + 240, 28 + 128);
	}
	console->cursor_x = 8;
	return ;
}

//处理栈异常的中断处理函数
int *inthandler0c(int *esp)
{
	CONSOLE *console = (CONSOLE *) *((int *) 0x0fec);
	TASK *task = task_now();
	console_putstring_toend(console, "\nINT0C : \nStack Exception!\n");
	return &(task->tss.esp0);		//让程序在这里强制结束
}

//当程序想要非法访问操作系统的内容的时候，会产生这个中断
int *inthandler0d(int *esp)
{
	CONSOLE *console = (CONSOLE *) *((int *) 0x0fec);
	TASK *task = task_now();
	console_putstring_toend(console, "\nINT0D : \nGeneral Protected Exception!\n");
	return &(task->tss.esp0);		//让程序在这里强制结束
}

//命令行窗口任务
void console_task(LAYER *layer, unsigned int mem_total)
{
	TIMER *timer;
	TASK *task = task_now();
	int i, fifobuf[128];
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	int *fat = (int *)memmanage_alloc_4K(memmanage, 4 * 2880);		//用来存储FAT表的内存地址
	char cmdline[30];						//cmdline用来记录在命令行中输入的命令
	CONSOLE console;
	console.layer = layer;
	console.cursor_x = 16;
	console.cursor_y = 28;
	console.cursor_color = -1;
	*((int *) 0x0fec) = (int)&console;
	
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));		//FAT表从第3个扇区开始，占用了共9个扇区
	
	init_fifo(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_set(timer, 50);
	
	//显示提示字符
	displayStrings_atLayer(layer, 8, 28, COL8_FFFFFF, COL8_000000, ">");
	
	for(;;)
	{
		io_cli();
		if(0 == fifo_status(&task->fifo))
		{
			task_sleep(task);
			io_sti();
		}
		else
		{
			//这里传出来的i直接就是键盘的对应键的字符加上一个键盘偏移量，不需要再转化了，只需要减去偏移量即可
			i = fifo_get(&task->fifo);
			io_sti();
			//计时器的中断数据处理（在这里就是光标的闪烁）
			if(1 == i || 0 == i)		//等于1或者0
			{
				if(1 == i)
				{
					if(console.cursor_color >= 0)
					{
						console.cursor_color = COL8_FFFFFF;
					}
					timer_init(timer, &task->fifo, 0);
				}
				else
				{
					if(console.cursor_color >= 0)
					{
						console.cursor_color = COL8_000000;
					}
					timer_init(timer, &task->fifo, 1);
				}
				timer_set(timer, 50);
			}
			//光标ON
			if(2 == i)		
			{
				console.cursor_color = COL8_FFFFFF;
			}
			//光标OFF
			if(3 == i)
			{
				drawRectangle(layer->buffer, layer->length, COL8_000000, console.cursor_x, console.cursor_y, 2, 15);
				console.cursor_color = -1;
			}
			//键盘数据的处理(主要是命令的处理)
			if(keyboard_offset <= i && i <= keyboard_offset + 255)
			{
				//如果是“退格键”的情况
				if(8 + keyboard_offset == i)
				{
					//不能删除最前面的提示符，而且只能在有字符的时候才能删除
					if(console.cursor_x > 16)
					{
						//首先用空格擦除光标，光标横坐标减8之后，再把要删除的那个字符用空格擦除
						console_putchar(&console, ' ', 0);
						console.cursor_x -= 8;
						console_putchar(&console, ' ', 0);
					}
				}
				//如果是“回车键”的情况
				else if(10 + keyboard_offset == i)
				{
					console_putchar(&console, ' ', 0);
					cmdline[console.cursor_x / 8 - 2] = 0;
					console_newline(&console);
					//运行命令
					console_runcmd(cmdline, &console, fat, mem_total);
					//显示提示字符
					console_putchar(&console, '>', 1);
				}
				//如果是一般字符的情况
				else
				{
					if(console.cursor_x < 240)
					{
						cmdline[console.cursor_x / 8 - 2] = i - keyboard_offset;
						console_putchar(&console, i - keyboard_offset, 1);
					}
				}
			}
			//重新显示光标
			if(console.cursor_color >= 0)
			{
				drawRectangle(layer->buffer, layer->length, console.cursor_color, console.cursor_x, console.cursor_y, 2, 15);				
			}
			layer_part_refresh(layer, console.cursor_x, console.cursor_y, console.cursor_x + 8, console.cursor_y + 16);
		}
	}
}

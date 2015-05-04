#include "bootpack.h"
#include <string.h>

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
	else if(0 == strcmp(cmdline, "hlt"))
	{
		cmd_hlt(console, fat);
	}
	//不是命令但也不是空行
	else if(0 != cmdline[0])
	{
		displayStrings_atLayer(console->layer, 8, console->cursor_y, COL8_FFFFFF, COL8_000000, "Bad command!");
		console_newline(console);
		console_newline(console);
	}
	return ;
}

//显示内存信息的函数
void cmd_mem(CONSOLE *console, unsigned int mem_total)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	char strings[30];
	/*在屏幕上显示一些内存的信息*/
	sprintf(strings, "Memory has %dMB", mem_total / 1024 / 1024);
	displayStrings_atLayer(console->layer, 8, console->cursor_y, COL8_FFFFFF, COL8_000000, strings);
	console_newline(console);
	sprintf(strings, "free memory:%dKB",memmanage_total(memmanage) / 1024);
	displayStrings_atLayer(console->layer, 8, console->cursor_y, COL8_FFFFFF, COL8_000000, strings);			//用字体显示当前内存容量
	console_newline(console);
	console_newline(console);
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
				displayStrings_atLayer(console->layer, 8, console->cursor_y, COL8_FFFFFF, COL8_000000, strings);
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
		for(i = 0; i < finfo->size; i++)
		{
			console_putchar(console, img_adr[i], 1);
		}
		memmanage_free_4K(memmanage, (int)img_adr, finfo->size);
	}
	//文件没有找到的情况，打印出错误信息
	else
	{
		displayStrings_atLayer(console->layer, 8, console->cursor_y, COL8_FFFFFF, COL8_000000, "File not found!");
		console_newline(console);
	}
	console_newline(console);
	return ;
}

//一个小应用程序
void cmd_hlt(CONSOLE *console, int *fat)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	FILEINFO *finfo = file_search("HLT.HRB", (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *)ADR_GDT;
	char *img_adr;
	/*找到这个应用程序的情况*/
	if(0 != finfo)
	{
		//文件内容开始的地址赋值给img_adr
		img_adr = (char *)memmanage_alloc_4K(memmanage, finfo->size);
		//把可能有间断的文件内容复制到一块儿连续的内存中去，这样就可以直接到这块儿内存进行连续的读取了
		file_loadfile(finfo->clusterno, finfo->size, img_adr, fat, (char *)(0x003e00 + ADR_DISKIMG));
		//为这个任务分配一个内存段
		set_segmdesc(gdt + 1003, finfo->size - 1, (int)img_adr, AR_CODE32_ER);
		//跳转到这个hlt任务所在的内存开始执行
		farcall(0, 1003 * 8);
		memmanage_free_4K(memmanage, (int)img_adr, finfo->size);
	}
	/*无法找到这个应用程序的情况*/
	else
	{
		displayStrings_atLayer(console->layer, 8, console->cursor_y, COL8_FFFFFF, COL8_000000, "File not found!");
		console_newline(console);
	}
	console_newline(console);
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

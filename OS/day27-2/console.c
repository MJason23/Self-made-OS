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

extern TASKCTL *taskctl;

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
			//只有当这个console有图层分配（即显示出来命令行窗口的console）的时候，才会在它上面显示字符
			if(0 != console->layer)
			{
				displayStrings_atLayer(console->layer, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, " ");
			}
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
		if(0 != console->layer)
		{
			displayStrings_atLayer(console->layer, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, strings);
		}
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
	if(0 == strcmp(cmdline, "mem") && 0 != console->layer)
	{
		cmd_mem(console, mem_total);
	}
	else if(0 == strcmp(cmdline, "clear") && 0 != console->layer)
	{
		cmd_clear(console);
	}
	else if(0 == strcmp(cmdline, "ls") && 0 != console->layer)
	{
		cmd_ls(console);
	}
	else if(0 == strncmp(cmdline, "type ", 5) && 0 != console->layer)
	{
		cmd_type(console, fat, cmdline);
	}
	else if(0 == strcmp(cmdline, "exit"))
	{
		cmd_exit(console, fat);
	}
	//输入命令启动一个命令行窗口和应用程序
	else if(0 == strncmp(cmdline, "start ", 6))
	{
		cmd_start(console, cmdline, mem_total);
	}
	//输入命令启动一个应用程序，但不启动命令行（实现一个命令行窗口启动多个应用程序）
	else if(0 == strncmp(cmdline, "ncst ", 5))
	{
		cmd_ncst(console, cmdline, mem_total);
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

//输入命令来启动应用程序的函数
void cmd_start(CONSOLE *console, char *cmdline, int mem_total)
{
	LAYER_MANAGE *layer_manage = (LAYER_MANAGE *) *((int *) 0x0fe4);
	LAYER *layer = open_console(layer_manage, mem_total);
	FIFO *fifo = &layer->task->fifo;
	int i;
	layer_slide(layer, 32, 4);
	layer_switch(layer, layer_manage->top);
	for(i = 6; cmdline[i] != 0; i++)
	{
		fifo_put(fifo, cmdline[i] + 256);
	}
	fifo_put(fifo, 10 + 256);
	console_newline(console);
	return ;
}

//通过一个命令行窗口来启动多个应用程序
void cmd_ncst(CONSOLE *console, char *cmdline, int mem_total)
{
	/*通过这种情况启动的应用程序，其实也要启动一个命令行窗口，但是这里不显示这个命令行窗口而已，所以不给它分配图层
	**由于没有没有键盘输入也没有定时器，所以基本上它这种情况启动的命令行窗口启动一个应用程序之后就进入休眠状态了，
	**而且还不会被唤醒，除非手动的唤醒（在task_a中只有强制关闭和点击“X”按钮的时候才会被唤醒），唤醒之后才能执行那些
	**关闭应用程序的处理
	*/
	TASK *task = open_console_task(0, mem_total);
	FIFO *fifo = &task->fifo;
	int i;
	for(i = 5; cmdline[i] != 0; i++)
	{
		fifo_put(fifo, cmdline[i] + 256);
	}
	fifo_put(fifo, 10 + 256);
	console_newline(console);
	return ;
}

//退出命令行窗口的函数
void cmd_exit(CONSOLE *console, int *fat)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	TASK *task = task_now();
	LAYER_MANAGE *layer_manage = (LAYER_MANAGE *) *((int *) 0x0fe4);
	FIFO *fifo = (FIFO *) *((int *) 0x0fec);
	if(0 != console->layer)
	{
		timer_cancel(console->timer);
	}
	memmanage_free_4K(memmanage, (int) fat, 4 * 2880);
	//由于console不能自己把自己关闭了，因为一旦执行了close_console之后就没办法执行后续程序了，需要交由操作系统的task_a来执行
	io_cli();
	if(0 != console->layer)
	{
		//向操作系统的task_a任务发送console使用的图层所在的高度，让task_a来关闭console
		fifo_put(fifo, console->layer - layer_manage->layers + 768);
	}
	else
	{
		fifo_put(fifo, task - taskctl->tasks + 1024);
	}
	io_sti();
	for(;;)
	{
		task_sleep(task);
	}
}

//供API调用的绘制直线的函数
void hrb_api_drawline(LAYER *layer, int x0, int y0, int x1, int y1, int color)
{
	/*
	**这段程序的设计思想是：目前的操作系统还不能处理小数，所以把x,y,dx,dy这四个整数先扩大1024倍，在进行处理
	**这样设计的话，横坐标每增加一个像素，x就需要增加1024，这个时候需要算出y会增加多少（这里的length是直线横坐标的跨度）
	*/
	int i, x, y, length, dx, dy;

	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0)
	{
		dx = - dx;
	}
	if (dy < 0)
	{
		dy = - dy;
	}
	//当直线的倾斜程度比较小的情况（就是横坐标的跨度比纵坐标的跨度大）
	if (dx >= dy) 
	{
		length = dx + 1;			//加1是为了把特殊情况考虑在内，例如起点终点完全相同的时候，不加1的话就没有直线可以绘制了
		if (x0 > x1) 
		{
			dx = -1024;				//起点坐标在终点坐标的右边
		} 
		else
		{
			dx =  1024;
		}
		if (y0 <= y1) 
		{
			dy = ((y1 - y0 + 1) << 10) / length;
		} 
		else 
		{
			dy = ((y1 - y0 - 1) << 10) / length;
		}
	} 
	//当直线的倾斜程度比较大的情况（就是横坐标的跨度比纵坐标的跨度小）
	else
	{
		length = dy + 1;			//加1是为了把特殊情况考虑在内，例如起点终点完全相同的时候，不加1的话就没有直线可以绘制了
		if (y0 > y1) 
		{
			dy = -1024;				//起点坐标在终点坐标的右边
		}
		else 
		{
			dy =  1024;
		}
		if (x0 <= x1) 
		{
			dx = ((x1 - x0 + 1) << 10) / length;
		}
		else
		{
			dx = ((x1 - x0 - 1) << 10) / length;
		}
	}

	for (i = 0; i < length; i++)
	{
		layer->buffer[(y >> 10) * layer->length + (x >> 10)] = color;
		x += dx;
		y += dy;
	}
	return ;
}

//书上P414有详解
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	TASK *task = task_now();
	int ds_base = task->ds_base;
	CONSOLE *console = task->console;
	LAYER_MANAGE *layer_manage = (LAYER_MANAGE *) *((int *) 0x0fe4);
	FIFO *sys_fifo = (FIFO *) *((int *) 0x0fec);
	LAYER *layer;
	int i;
	int *reg = &eax + 1;			//eax后面的地址
	/*在调用绘制窗口的函数时，最后要改写通过PUSHAD保存的寄存器的值，因为最后会通过EAX将操作窗口的句柄返回回去；
	**当初PUAHAD了两次，第二次是作为本函数的参数传值的，这个eax的值是第一次PUSHAD最后的一个压栈操作，它的地址跟
	**下一次的PUSHAD的第一个压栈数据地址挨着，也就是EDI的地址，所以&eax+1（即reg[0]）的地址就是EDI的地址
	*/
	/* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
	/* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
	//调用显示单个字符的函数
	if (edx == 1) 
	{
		console_putchar(console, eax & 0xff, 1);
	} 
	//调用显示整个字符串的函数
	else if (edx == 2)
	{
		console_putstring_toend(console, (char *) ebx + ds_base);
	} 
	//调用显示指定长度字符串的函数
	else if (edx == 3)
	{
		console_putstring_length(console, (char *) ebx + ds_base, ecx);
	}
	//设置为，如果传入的edx值为4，则退出这个程序
	else if(edx == 4)
	{	
		return &(task->tss.esp0);		
	}
	//调用显示窗口的函数
	else if(edx == 5)
	{
		layer = layer_alloc(layer_manage);
		layer->task = task;					//把这个图层分配给这个当前的任务（也就是console的task）	
		layer->flags |= 0x10;				//这个窗口是由应用程序产生的，在flags中加上这个0x10标志位
		layer_set(layer, (char *)ebx + ds_base, esi, edi, eax);
		create_window((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, INACTIVE);
		layer_slide(layer, ((layer_manage->scrnx - esi) / 2) & ~3, (layer_manage->scrny - edi) / 2);
		layer_switch(layer, layer_manage->top);
		reg[7] = (int)layer;		//作为操作窗口的句柄传出去
	}
	//调用显示字符串的函数
	else if(edx == 6)
	{
		layer = (LAYER *) (ebx & 0xfffffffe);		//把这个layer的地址恢复成偶数的地址（分配出来的layer的地址一定是偶数的）
		displayStrings_CS(layer->buffer, layer->length, esi, edi, eax, (char *)ebp + ds_base);
		if(0 == (ebx & 1))			//查看传入的layer地址是不是偶数，如果是偶数标志着需要刷新，奇数的话不需要刷新
		{
			layer_part_refresh(layer, esi, edi, esi + ecx * 8, edi + 16);
		}
	}
	//调用绘制矩形的函数
	else if(edx == 7)
	{
		layer = (LAYER *)(ebx & 0xfffffffe);
		drawRectangle(layer->buffer, layer->length, ebp, eax, ecx, esi, edi);
		if(0 == (ebx & 1))
		{
			layer_part_refresh(layer, eax, ecx, eax + esi + 1, ecx + edi + 1);
		}		
	} 
	//调用初始化内存管理结构的函数（这是对应用程序的内存管理结构初始化的，不是操作系统）
	else if(edx == 8)
	{
		memmanage_init((MEMMANAGE *) (ebx + ds_base));
		ecx &= 0xfffffff0;		//以16字节为单位
		memmanage_free((MEMMANAGE *) (ebx + ds_base), eax, ecx);
	}
	//调用分配内存的函数
	else if(edx == 9)
	{
		ecx = (ecx + 0x0f) & 0xfffffff0;		//以16字节为单位进位取整
		reg[7] = memmanage_alloc((MEMMANAGE *) (ebx + ds_base), ecx);
	}
	//调用释放内存的函数
	else if(edx == 10)
	{
		ecx = (ecx + 0x0f) & 0xfffffff0;
		memmanage_free((MEMMANAGE *) (ebx + ds_base), eax, ecx);
	}
	//调用画点的函数
	else if(edx == 11)
	{
		layer = (LAYER *)(ebx & 0xfffffffe);
		layer->buffer[layer->length * edi + esi] = eax;		//想这个图层的某一个像素上画一个点
		if(0 == (ebx & 1))
		{
			layer_part_refresh(layer, esi, edi, esi + 1, edi + 1);
		}
	}
	//调用刷新图层的函数
	else if(edx == 12)
	{
		layer = (LAYER *) ebx;
		layer_part_refresh(layer, eax, ecx, esi, edi);
	}
	//调用绘制直线的函数
	else if(edx == 13)
	{
		layer = (LAYER *)(ebx & 0xfffffffe);
		hrb_api_drawline(layer, eax, ecx, esi, edi, ebp);
		if(0 == (ebx & 1))
		{
			layer_part_refresh(layer, eax, ecx, esi + 1, edi + 1);
		}
	}
	//调用释放图层的函数
	else if(edx == 14)
	{
		layer_free((LAYER *) ebx);
	}
	//应用程序接受键盘输入的函数
	else if(edx == 15)
	{
		for(;;)
		{
			io_cli();
			if(0 == fifo_status(&task->fifo))
			{
				//传入的参数指定模式为1的时候，就让任务在空闲的时候休眠
				if(0 != eax)
				{
					task_sleep(task);
				}
				//传入的参数指定模式为0的时候，不让任务休眠
				else
				{	
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo_get(&task->fifo);
			io_sti();
			//光标用的定时器
			if(i <= 1 && console->layer != 0)
			{
				//应用程序在运行的时候不需要光标继续闪
				timer_init(console->timer, &task->fifo, 1);
				timer_set(console->timer, 50);
			}
			//光标ON
			if(2 == i)
			{
				console->cursor_color = COL8_FFFFFF;
			}
			//光标OFF
			if(3 == i)
			{
				console->cursor_color = -1;
			}
			if(4 == i)
			{
				timer_cancel(console->timer);
				io_cli();
				fifo_put(sys_fifo, console->layer - layer_manage->layers + 2024);
				console->layer = 0;				//这里只是把指向那个console的layer的指针归零了，但是真正的layer并没有被处理掉呢，留着到task_a处理
				io_sti();
			}
			/*有了定时器之后，向fifo中传入的数据范围就大了，所以可以传入比256大的所有数（因为键盘数据传入的时候都加了256，所以
			**定时器也加256，在这里向应用程序传值的时候再减去256）
			*/
			if(i >= 256)
			{
				reg[7] = i - keyboard_offset;
				return 0;
			}
		}
	}
	//获取定时器
	else if(edx == 16)
	{
		reg[7] = (int)timer_alloc(); 
		((TIMER *)reg[7])->autocancel_flags = 1;			//应用程序的定时器设置为自动取消
	}
	//对定时器进行初始化，设置定时器的超时传输shuju
	else if(edx == 17)
	{
		//由于最后向应用程序传值的时候统一减去了256，所以这里就加上了256
		timer_init((TIMER *)ebx, &task->fifo, eax + 256);
	}
	//对定时器的间隔时间进行设置
	else if(edx == 18)
	{
		timer_set((TIMER *)ebx, eax);
	}
	//释放定时器
	else if(edx == 19)
	{
		timer_free((TIMER *)ebx);
	}
	//调用蜂鸣器发生或者关闭蜂鸣器的函数
	else if(edx == 20)
	{
		//这一部分的设置在书上P516有详解
		if(0 == eax)
		{
			//这两步是关闭蜂鸣器的
			i = io_in8(0x61);
			io_out8(0x61, i & 0x0d);
		}
		else
		{
			i = 1193180000 / eax;
			io_out8(0x43, 0xb6);
			io_out8(0x42, i & 0xff);		//设置低8位
			io_out8(0x42, i >> 8);			//设置高8位
			i = io_in8(0x61);				//这两步是打开蜂鸣器的
			io_out8(0x61, (i | 0x03) & 0x0f);
			
		}
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
	LAYER_MANAGE *layer_manage;
	LAYER *layer;
	char name[30], *img_adr, *data_adr;
	int seg_size, data_size, esp, data_hrb;
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
	if(finfo == 0 && name[i - 1] != '.')
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
		//把可能有间断的文件内容复制到一块儿连续的内存中去，这样就可以直接到这块儿内存进行连续的读取了
		file_loadfile(finfo->clusterno, finfo->size, img_adr, fat, (char *)(0x003e00 + ADR_DISKIMG));
		
		//如果应用程序是用C语言写的，那么就要在这里把它调整为从HariMain主函数开始运行
		if(finfo->size >= 36 && 0 == strncmp(img_adr + 4, "Hari", 4) && 0x00 == *img_adr)
		{
			seg_size = *((int *) (img_adr + 0x0000));
			esp = *((int *) (img_adr + 0x000c));
			data_size = *((int *) (img_adr + 0x0010));
			data_hrb = *((int *) (img_adr + 0x0014));
			data_adr = (char *)memmanage_alloc_4K(memmanage, seg_size);
			task->ds_base = (int) data_adr;				//给task的ds_base赋值
			//为这个任务分配一个内存段，将访问权限加上0x60之后，就可以将段设置成为应用程序专用（把1003到2002作为代码段分配用）
			set_segmdesc(gdt + (task->selector / 8 + 1000), finfo->size - 1, (int)img_adr, AR_CODE32_ER + 0x60);
			//为应用程序分配的数据段(把2003到3002作为数据段分配用)
			set_segmdesc(gdt + (task->selector / 8 + 2000), seg_size - 1, (int)data_adr, AR_DATA32_RW + 0x60);
			for(i = 0; i < data_size; i++)
			{
				data_adr[esp + i] = img_adr[data_hrb + i];
			}
			start_app(0x1b, task->selector + 1000 * 8, esp, task->selector + 2000 * 8, &(task->tss.esp0));
			//当程序执行到这里的时候，说明已经从应用程序中跳转回来到console中了，这个时候说明应用程序已经结束了，所以需要释放图层
			//和应用程序数据段所占用的内存（代码段所占用的内存暂时不用管，因为下次启动应用程序的时候，直接就在上次的那个段中启动了）
			layer_manage = (LAYER_MANAGE *) *((int *) 0x0fe4);
			//停止任务的时候，需要从图层管理结构中查找看看当前哪个图层属于这个任务，属于这个任务的图层都要被释放掉
			for(i = 0; i < MAX_LAYER; i++)
			{
				layer = &(layer_manage->layers[i]);
				//如果这个layer处于已分配的状态，即已被程序占用的状态，并且flags的0x10位为1，则说明这个layer属于应用程序的窗口而不是console的黑窗口
				if(0x11 == (layer->flags & 0x11) && layer->task == task)
				{
					layer_free(layer);
				}
			}
			timer_cancelall(&task->fifo);
			memmanage_free_4K(memmanage, (int)data_adr, seg_size);				
		}
		else
		{	
			console_putstring_toend(console, ".hrb file format error.\n");
		}
		
		//任务执行完之后就把这块儿内存释放掉
		memmanage_free_4K(memmanage, (int)img_adr, finfo->size);
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
		if(0 != console->layer)
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
	}
	console->cursor_x = 8;
	return ;
}

//处理栈异常的中断处理函数
int *inthandler0c(int *esp)
{
	TASK *task = task_now();
	CONSOLE *console = task->console;
	console_putstring_toend(console, "\nINT0C : \nStack Exception!\n");
	return &(task->tss.esp0);		//让程序在这里强制结束
}

//当程序想要非法访问操作系统的内容的时候，会产生这个中断
int *inthandler0d(int *esp)
{
	TASK *task = task_now();
	CONSOLE *console = task->console;
	char string[30];
	console_putstring_toend(console, "\nINT0D : \nGeneral Protected Exception!\n");
	sprintf(string, "CS = %08X,EIP = %08X, DS = %08X\n",esp[12],esp[11],esp[8]);
	console_putstring_toend(console, string);
	return &(task->tss.esp0);		//让程序在这里强制结束
}

//命令行窗口任务
void console_task(LAYER *layer, unsigned int mem_total)
{
	TASK *task = task_now();
	int i;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	int *fat = (int *)memmanage_alloc_4K(memmanage, 4 * 2880);		//用来存储FAT表的内存地址
	char cmdline[30];						//cmdline用来记录在命令行中输入的命令
	CONSOLE console;
	console.layer = layer;
	console.cursor_x = 8;
	console.cursor_y = 28;
	console.cursor_color = -1;
	task->console = &console;
	
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));		//FAT表从第3个扇区开始，占用了共9个扇区
	
	//只有当这个console有窗口图层分配的时候才需要用定时器来给它提示显示光标，否则就不需要了
	if(0 != console.layer)
	{
		console.timer = timer_alloc();
		timer_init(console.timer, &task->fifo, 1);
		timer_set(console.timer, 50);
	}
	
	
	//显示提示字符
	console_putchar(&console, '>', 1);
	
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
			if(i <= 1 && 0 != console.layer)		//等于1或者0
			{
				if(1 == i)
				{
					if(console.cursor_color >= 0)
					{
						console.cursor_color = COL8_FFFFFF;
					}
					timer_init(console.timer, &task->fifo, 0);
				}
				else
				{
					if(console.cursor_color >= 0)
					{
						console.cursor_color = COL8_000000;
					}
					timer_init(console.timer, &task->fifo, 1);
				}
				timer_set(console.timer, 50);
			}
			//光标ON
			if(2 == i)		
			{
				console.cursor_color = COL8_FFFFFF;
			}
			//光标OFF
			if(3 == i)
			{
				if(0 != console.layer)
				{
					drawRectangle(console.layer->buffer, console.layer->length, COL8_000000, console.cursor_x, console.cursor_y, 2, 15);
				}				
				console.cursor_color = -1;
			}
			if(4 == i)
			{
				cmd_exit(&console, fat);
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
					if(0 == console.layer)
					{
						cmd_exit(&console, fat);
					}
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
			if(0 != console.layer)
			{
				if(console.cursor_color >= 0)
				{
					drawRectangle(console.layer->buffer, console.layer->length, console.cursor_color, console.cursor_x, console.cursor_y, 2, 15);				
				}
				layer_part_refresh(console.layer, console.cursor_x, console.cursor_y, console.cursor_x + 8, console.cursor_y + 16);
			}			
		}
	}
}

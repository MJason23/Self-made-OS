#include "bootpack.h"
#include <string.h>

TIMER *task_timer;		//任务切换时用的定时器
TASKCTL *taskctl;		//任务管理主体（这个结构相当大啊）

//返回现在的任务地址
TASK *task_now(void)
{
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	return tl->tasks_lv[tl->task_now];
}

//根据任务自己的level向相应的等级中添加任务
void task_add(TASK *task)
{
	/*找到这个任务当前运行所在的等级，然后让这个等级的下一个指针指向它，最后把它的状态设置为运行态*/
	TASKLEVEL *tl = &taskctl->task_level[task->level];
	tl->tasks_lv[tl->running_number] = task;
	tl->running_number++;
	task->flags = RUNNING; 
	return;
}

/*找到这个任务当前运行所在的等级，然后让这个等级的下一个指针指向它，最后把它的状态设置为运行态*/
void task_remove(TASK *task)
{
	int i;
	TASKLEVEL *tl = &taskctl->task_level[task->level];

	/*寻找task所在的位置，然后把task的状态设置为睡眠就行*/
	for (i = 0; i < tl->running_number; i++) 
	{
		if (tl->tasks_lv[i] == task) 
		{
			break;
		}
	}
	/*让任务所在的等级的running_number减1，并且把task后面的任务向前移动*/
	tl->running_number--;
	if (i < tl->task_now)		//如果这个任务的位置在当前运行的任务前面，则需要把task_now减1，即指针前移
	{
		tl->task_now--; 
	}
	if (tl->task_now >= tl->running_number) 				//如果记录当前等级运行着的任务数量的变量减少后比task_now还小，就需要调整了
	{
		tl->task_now = 0;
	}
	task->flags = SLEEP;

	for (; i < tl->running_number; i++) 
	{
		tl->tasks_lv[i] = tl->tasks_lv[i + 1];
	}

	return;
}

//这个函数说到底是用来找到当前最高级的level的，即找到lv_now
void level_switch(void)
{
	int i;
	/* 堦斣忋偺儗儀儖傪扵偡 */
	for (i = 0; i < MAX_TASKLEVELS; i++) 
	{
		//从最高等级开始搜索，只要找到有一个等级中有任务存在就行
		if (taskctl->task_level[i].running_number > 0) 
		{
			break;
		}
	}
	taskctl->lv_now = i;
	taskctl->lv_change = 0;
	return;
}

//这个是操作系统最低级的任务，永远存在，就是用来当所有任务都不工作的时候执行这个HLT指令而已
void task_idle(void)
{
	for (;;) {
		io_hlt();
	}
}

//任务管理主体的初始化（并为调用这个函数的那个程序分配一个任务状态段）
TASK *task_init(MEMMANAGE *memman)
{
	int i;
	TASK *task, *idle;
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) ADR_GDT;

	taskctl = (TASKCTL *) memmanage_alloc_4K(memman, sizeof (TASKCTL));
	//对taskctl进行初始化
	for (i = 0; i < MAX_TASKS; i++) 
	{
		taskctl->tasks[i].flags = UNUSED;
		taskctl->tasks[i].selector = (TASK_GDT0 + i) * 8;		//为这个任务分配任务状态段所在的GDT描述符
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks[i].tss, AR_TSS32);
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) 
	{
		taskctl->task_level[i].running_number = 0;
		taskctl->task_level[i].task_now = 0;
	}

	/*为调用这个初始化函数的程序分配一个属于它的任务*/
	/*为调用这个初始化函数的程序分配一个属于它的任务*/
	task = task_alloc();				
	task->flags = RUNNING;				//这个任务当前正在运行中
	task->priority = 2;					//任务的优先级默认的都是2，即可以占用CPU0.02秒
	task->level = 0;					//调用这个函数的任务默认设定为最高级0级
	task_add(task);						//自动根据task的level添加到相应等级任务队列中
	level_switch();						//由于有新任务添加进来，所以需要改变当前的lv_now
	load_tr(task->selector);			//改写TR寄存器，让它存储当前这个任务的段偏移值
	task_timer = timer_alloc();			//分配一个用于任务切换的计时器
	timer_set(task_timer, task->priority);			//0.02秒切换一次任务
	/*给整个操作系统一个最低级的任务，这样当没有任务且键盘鼠标等任务休眠的时候就可以转到这个任务里
	**，这个任务永远存在
	*/
	idle = task_alloc();
	idle->tss.esp = memmanage_alloc_4K(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);

	return task;
}

//为某个程序分配任务
TASK *task_alloc(void)
{
	int i;
	TASK *task;
	for (i = 0; i < MAX_TASKS; i++) 
	{
		if (taskctl->tasks[i].flags == UNUSED) 
		{
			//这里没有对esp和eip等寄存器赋值，需要在建立这个task的时候根据具体情况赋值
			task = &taskctl->tasks[i];
			task->flags = SLEEP;		//处于这种状态的任务不会被分配到CPU，但是不能被分配给新的任务使用，它随时可能被唤醒而成为运行状态
			task->tss.eflags = 0x00000202; /* IF = 1;所有的任务默认都是开放中断的 */
			task->tss.eax = 0; 			//这里先置为0
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0;		//全部都在使用当中则只能返回0
}

//让程序开始运行（包括从刚分配出的任务使其开始执行，或者已经休眠的任务恢复运行）
void task_run(TASK *task, int level, int priority)
{
	if (level < 0) 			//如果设置的level为负数，说明不想让它的level改变，那么下面的比较肯定不同了
	{
		level = task->level;
	}
	if (priority > 0)
	{
		task->priority = priority;
	}

	//如果这个任务处于运行状态，且设置了有效的level，则需要先把它从原有level中移除出去
	if (task->flags == RUNNING && task->level != level) 			//任务从睡眠状态唤醒，把任务的level赋值给它，并把自己添加入相应level中
	{
		task_remove(task);
	}
	if (task->flags != RUNNING) 
	{
		task->level = level;
		task_add(task);
	}

	taskctl->lv_change = 1;			//因为有相应任务的level改变，下次转换任务的时候就需要检查level等级了，因为有可能需要先运行更高等级的任务
	return;
}

//任务休眠
void task_sleep(TASK *task)
{
	TASK *now_task;
	if (task->flags == RUNNING) 
	{
		now_task = task_now();
		task_remove(task);		//执行这行代码之后只是把task从level中移除了，并且flags变成SLEEP状态，但是task本身并没有被释放
		if (task == now_task)
		{
			/*如果是自己让自己休眠的情况还需要任务转换*/
			level_switch();
			now_task = task_now(); 
			farjmp(0, now_task->selector);
		}
	}
	return;
}

//任务切换，主要是给timer的中断处理程序调用的
void task_switch(void)
{
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	TASK *new_task, *now_task = tl->tasks_lv[tl->task_now];
	tl->task_now++;
	//先把任务所在的当前level中的任务调整好
	if (tl->task_now == tl->running_number)		//如果running_number到最后一个了，就重新回到第一个
	{
		tl->task_now = 0;
	}
	if (taskctl->lv_change != 0)				//如果需要改变level的情况
	{
		level_switch();						//找出当前的最高级level
		tl = &taskctl->task_level[taskctl->lv_now];
	}
	//找出要转换到的那个任务，并且设置好timer，然后执行任务跳转
	new_task = tl->tasks_lv[tl->task_now];
	timer_set(task_timer, new_task->priority);
	if (new_task != now_task) 		//如果转换任务之后发现还是当前任务，就不用跳转了
	{
		farjmp(0, new_task->selector);
	}
	return;
}

void task_b_main(LAYER *layer)
{
	FIFO fifo;
	TIMER *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	init_fifo(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_set(timer_1s, 100);

	for (;;) {
		count++;
		io_cli();
		if (fifo_status(&fifo) == 0) 
		{
			io_sti();
		} 
		else 
		{
			i = fifo_get(&fifo);
			io_sti();
			if (i == 100)
			{
				sprintf(s, "%11d", count - count0);
				displayStrings_atLayer(layer, 24, 28, COL8_000000, COL8_C6C6C6, s);
				count0 = count;
				timer_set(timer_1s, 100);
			}
		}
	}
}

//命令行换行函数
void console_newline(int *cursor_y, LAYER *layer)
{
	int x, y;
	//如果没有超过命令行屏幕的纵屏长度，就直接加纵坐标的值
	if(*cursor_y < 28 + 112)
	{
		*cursor_y += 16;
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
	return ;
}

//命令行窗口任务
void console_task(LAYER *layer, unsigned int mem_total)
{
	FIFO fifo;					//记着声明的时候声明成结构体而不是结构体指针，这个不是timer或者task那种实体全都在管理结构体中的
	TIMER *timer;
	TASK *task = task_now();
	int i, count = 0,fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_color = -1;
	int x, y;
	char strings[30], cmdline[30];						//cmdline用来记录在命令行中输入的命令
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	
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
					if(cursor_color >= 0)
					{
						cursor_color = COL8_FFFFFF;
					}
					timer_init(timer, &task->fifo, 0);
				}
				else
				{
					if(cursor_color >= 0)
					{
						cursor_color = COL8_000000;
					}
					timer_init(timer, &task->fifo, 1);
				}
				timer_set(timer, 50);
			}
			//光标ON
			if(2 == i)		
			{
				cursor_color = COL8_FFFFFF;
			}
			//光标OFF
			if(3 == i)
			{
				drawRectangle(layer->buffer, layer->length, COL8_000000, cursor_x, cursor_y, 2, 15);
				cursor_color = -1;
			}
			//键盘数据的处理
			if(keyboard_offset <= i && i <= keyboard_offset + 255)
			{
				//如果是“退格键”的情况
				if(8 + keyboard_offset == i)
				{
					//不能删除最前面的提示符，而且只能在有字符的时候才能删除
					if(cursor_x > 16)
					{
						cursor_x -= 8;
						displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, "  ");
					}
				}
				//如果是“回车键”的情况
				else if(10 + keyboard_offset == i)
				{
					//用空格将光标擦除
					displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ");
					cmdline[cursor_x / 8 - 2] = 0;		//给这个命令字符串的最后加上一个“\0”
					console_newline(&cursor_y, layer);
					//查看是不是“mem”命令，如果是的话，就显示出关于内存的信息
					if(0 == strcmp(cmdline, "mem"))	
					{
						/*在屏幕上显示一些内存的信息*/
						sprintf(strings, "Memory has %dMB", mem_total / 1024 / 1024);
						displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, strings);
						console_newline(&cursor_y, layer);
						sprintf(strings, "free memory:%dKB",memmanage_total(memmanage) / 1024);
						displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, strings);			//用字体显示当前内存容量
						console_newline(&cursor_y, layer);
						console_newline(&cursor_y, layer);
					}
					//查看是不是“clear”命令，如果是的话，就清屏
					else if(0 == strcmp(cmdline, "clear"))
					{
						for(y = 28; y < 28 + 112; y++)
						{
							for(x = 8; x < 8 + 240; x++)
							{
								layer->buffer[x + y * layer->length] = COL8_000000;
							}
						}
						layer_part_refresh(layer, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;		//光标重新回到左上方
					}
					//虽然不是空命令，但是无法识别
					else if(0 != cmdline[0])
					{
						displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command!");
						console_newline(&cursor_y, layer);
						console_newline(&cursor_y, layer);
					}
					//在新的一行显示提示符
					displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">");
					cursor_x = 16;
				}
				//如果是一般字符的情况
				else
				{
					if(cursor_x < 240)
					{
						strings[0] = i - keyboard_offset;
						strings[1] = 0;
						cmdline[cursor_x / 8 -2] = i - keyboard_offset;			//需要记录用户输入的命令是什么
						displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, strings);
						cursor_x += 8;
					}
				}
			}
			//重新显示光标
			if(cursor_color >= 0)
			{
				drawRectangle(layer->buffer, layer->length, cursor_color, cursor_x, cursor_y, 2, 15);				
			}
			layer_part_refresh(layer, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}















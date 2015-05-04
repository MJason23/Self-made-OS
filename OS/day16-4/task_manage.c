#include "bootpack.h"

TIMER *task_timer;		//任务切换时用的定时器
TASKCTL *taskctl;		//任务管理主体（这个结构相当大啊）

//任务管理主体的初始化（并为调用这个函数的那个程序分配一个任务状态段）
TASK *taskctl_init(MEMMANAGE *memman)
{
	int i;
	TASK *task;
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *)ADR_GDT;
	taskctl = (TASKCTL *) memmanage_alloc_4K(memman, sizeof(TASKCTL));
	//对taskctl进行初始化
	for(i = 0;i < MAX_TASKS; i++)
	{
		taskctl->tasks[i].flags = UNUSED;
		taskctl->tasks[i].selector = (TASK_GDT + i) * 8;		//为这个任务分配任务状态段所在的GDT描述符
		set_segmdesc(gdt + TASK_GDT + i, 103, (int) &taskctl->tasks[i].tss, AR_TSS32);
	}
	for(i = 0; i < MAX_TASKLEVELS; i++)
	{
		taskctl->task_level[i].running_number = 0;
		taskctl->task_level[i].task_now = 0;
	}
	task = task_alloc();				//为调用这个初始化函数的程序分配一个属于它的任务
	task->flags = RUNNING;				//这个任务当前正在运行中
	task->priority = 2;					//任务的优先级默认的都是2，即可以占用CPU0.02秒
	task->level = 0;					//调用这个函数的任务默认设定为最高级0级
	task_add(task);						//自动根据task的level添加到相应等级任务队列中
	level_switch();						//由于有新任务添加进来，所以需要改变当前的lv_now
	load_tr(task->selector);			//改写TR寄存器，让它存储当前这个任务的段偏移值
	task_timer = timer_alloc();			//分配一个用于任务切换的计时器
	timer_set(task_timer, task->priority);			//0.02秒切换一次任务
	return task;
}

//返回现在的任务地址
TASK *task_now(void)
{
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	return tl->tasks_in_lv[tl->task_now];
}

//根据任务自己的level向相应的等级中添加任务
void task_add(TASK *task)
{
	/*找到这个任务当前运行所在的等级，然后让这个等级的下一个指针指向它，最后把它的状态设置为运行态*/
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	tl->tasks_in_lv[tl->running_number] = task;
	tl->running_number++;
	task->flags = RUNNING;
	return ;
}

//从任务所在的那个等级中删除该任务(本质上其实就是让这个任务休眠了)
void task_remove(TASK *task)
{
	int i;
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	/*寻找task所在的位置，然后把task的状态设置为睡眠就行*/
	for(i = 0; i < tl->running_number; i++)
	{
		if(task == tl->tasks_in_lv[i])
		{
			break;
		}
	}
	task->flags = SLEEP;
	/*让任务所在的等级的running_number减1，并且把task后面的任务向前移动*/
	tl->running_number--;
	if(i < tl->task_now)		//如果这个任务的位置在当前运行的任务前面，则需要把task_now减1，即指针前移
	{
		tl->task_now--;
	}
	if(tl->task_now >= tl->running_number)				//如果记录当前等级运行着的任务数量的变量减少后比task_now还小，就需要调整了
	{
		tl->task_now = 0;
	}
	for(; i < tl->running_number; i++)
	{
		tl->tasks_in_lv[i] = tl->tasks_in_lv[i + 1];
	}
	return ;
}

//这个函数说到底是用来找到当前最高级的level的，即找到lv_now
void level_switch(void)
{
	int i;
	for(i = 0; i < MAX_TASKLEVELS; i++)
	{
		//从最高等级开始搜索，只要找到有一个等级中有任务存在就行
		if(taskctl->task_level[i].running_number > 0)
		{
			break;
		}
	}
	taskctl->lv_now = i;
	taskctl->is_lv_change = 0;				//因为等级转换已经完成，所以这里就先把标志设置为0
	return ;
}

//为某个程序分配任务
TASK *task_alloc(void)
{
	int i;
	TASK *task;
	for(i = 0;i < MAX_TASKS; i++)
	{
		if(UNUSED == taskctl->tasks[i].flags)
		{
			//这里没有对esp和eip等寄存器赋值，需要在建立这个task的时候根据具体情况赋值
			task = &taskctl->tasks[i];
			task->flags = SLEEP;		//处于这种状态的任务不会被分配到CPU，但是不能被分配给新的任务使用，它随时可能被唤醒而成为运行状态
			task->tss.eflags = 0x00000202; /* IF = 1; */
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
	return 0;		//全部都在使用当中
}

//让程序开始运行（包括从刚分配出的任务使其开始执行，或者已经休眠的任务恢复运行）
void task_run(TASK *task, int level, int priority)
{
	if(level < 0)			//如果设置的level为负数，说明不想让它的level改变，那么下面的比较肯定不同了
	{
		level = task->level;
	}
	if(priority > 0)
	{
		task->priority = priority;
	}
	//如果这个任务处于运行状态，且设置了有效的level，则需要先把它从原有level中移除出去
	if(RUNNING == task->flags && level != task->level)
	{
		task_remove(task);
	}
	if(SLEEP == task->flags)			//任务从睡眠状态唤醒，把任务的level赋值给它，并把自己添加入相应level中
	{
		task->level = level;
		task_add(task);
	}
	taskctl->is_lv_change = 1;			//因为有相应任务的level改变，下次转换任务的时候就需要检查level等级了，因为有可能需要先运行更高等级的任务
	return ;
}

//任务切换，主要是给timer的中断处理程序调用的
void task_switch(void)
{
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	TASK *new_task, *now_task = task_now();
	tl->task_now++;
	//先把任务所在的当前level中的任务调整好
	if(tl->task_now == tl->running_number)		//如果running_number到最后一个了，就重新回到第一个
	{
		tl->task_now = 0;
	}
	if(0 != taskctl->is_lv_change)				//如果需要改变level的情况
	{
		level_switch();							//找出当前的最高级level
		tl = &taskctl->task_level[taskctl->lv_now];
	}
	//找出要转换到的那个任务，并且设置好timer，然后执行任务跳转
	new_task = tl->tasks_in_lv[tl->task_now];
	timer_set(task_timer, new_task->priority);
	if(new_task != now_task)		//如果转换任务之后发现还是当前任务，就不用跳转了
	{
		farjmp(0, new_task->selector);
	}
	return ;
}

//任务休眠
void task_sleep(TASK *task)
{
	TASK *now_task;
	if(RUNNING == task->flags)
	{
		now_task = task_now();
		task_remove(task);		//执行这行代码之后只是把task从level中移除了，并且flags变成SLEEP状态，但是task本身并没有被释放
		if(now_task == task)
		{
			/*如果是自己让自己休眠的情况还需要*/
			level_switch();
			now_task = task_now();
			farjmp(0, now_task->selector);
		}
	}
	return ;
}

//
/*
void task_b_main(LAYER *layer_bg)
{
	for(;;)
	{
		io_hlt();
	}
}
*/

void task_b_main(LAYER *layer)
{
	FIFO fifo;
	TIMER *timer_display;
	int i, fifobuf[128], count = 0, count0 = 0;
	char strings[20];
	
	init_fifo(&fifo, 128, fifobuf, 0);
	
	timer_display = timer_alloc();
	timer_init(timer_display, &fifo, 100);
	timer_set(timer_display, 100);
	for(;;)
	{
		count++;
		io_cli();
		if(0 == fifo_status(&fifo))
		{
			io_sti();
		}
		else
		{
			i = fifo_get(&fifo);
			io_sti();
			if(100 == i)
			{
				sprintf(strings, "%11d", count - count0);
				displayStrings_atLayer(layer, 24, 28, COL8_000000, COL8_C6C6C6, strings);
				count0 = count;
				timer_set(timer_display, 100);
			}
		}
	}
}











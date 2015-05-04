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
	task = task_alloc();				//为调用这个初始化函数的程序分配一个属于它的任务
	task->flags = RUNNING;				//这个任务当前正在运行中
	taskctl->running_number = 1;		//当前正在运行着的任务只有一个
	taskctl->task_now = 0;				//当前占用着CPU的任务是0号task
	taskctl->running_tasks[0] = task;	//让第一个数组指针指向这个正在运行的task
	load_tr(task->selector);			//改写TR寄存器，让它存储当前这个任务的段偏移值
	task_timer = timer_alloc();			//分配一个用于任务切换的计时器
	timer_set(task_timer, 2);			//0.02秒切换一次任务
	return task;
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
void task_run(TASK *task)
{
	task->flags = RUNNING;				//将当前的程序设置为运行状态
	taskctl->running_tasks[taskctl->running_number] = task;			//让记录运行任务的数组中的指针指向这个任务
	taskctl->running_number++;
	return ;
}

//任务切换
void task_switch(void)
{
	timer_set(task_timer, 2);
	if(taskctl->running_number >= 2)			//只要当前运行中的任务有两个以上，就进行任务切换，否则直接结束
	{
		taskctl->task_now++;
		if(taskctl->task_now == taskctl->running_number)
		{
			taskctl->task_now = 0;
		}
		farjmp(0, taskctl->running_tasks[taskctl->task_now]->selector);
	}
	return ;
}

//任务休眠
void task_sleep(TASK *task)
{
	int i;
	char self_sleep; 		//判断是不是自己使自己休眠的标志
	if(RUNNING == task->flags)			//这个任务只有处于运行状态的时候让它休眠才有用
	{
		/*1.先判断一下是不是自己让自己休眠的情况*/
		if(task == taskctl->running_tasks[taskctl->task_now])
		{
			self_sleep = 1;
		}
		/*2.找出这个任务在running_tasks中的位置*/
		for(i = 0; i < taskctl->running_number; i++)
		{
			if(task == taskctl->running_tasks[i])
			{
				break;
			}
		}
		/*3.减少running_number，设置这个任务为休眠状态，并且把task之后的任务向前移*/
		taskctl->running_number--;
		if(i < taskctl->task_now)		//如果需要休眠的这个任务在正在运行的任务之前，则需要把task_now减1,否则没必要
		{
			taskctl->task_now--;
		}
		for(; i < taskctl->running_number; i++)
		{
			taskctl->running_tasks[i] = taskctl->running_tasks[i + 1];
		}
		task->flags = SLEEP;
		/*4.如果是自己让自己睡眠的话，需要任务切换，并且有可能出现task_now跟running_number一样大的情况，这个时候需要调整*/
		if(1 == self_sleep)
		{
			if(taskctl->task_now >= taskctl->running_number)
			{
				taskctl->task_now = 0;
			}
			farjmp(0, taskctl->running_tasks[taskctl->task_now]->selector);			//直接就跳转到另一个任务中去了，不会返回了，但是每个任务的默认设置都是开启中断，所以不用担心
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

void task_b_main(LAYER *layer_bg)
{
	FIFO fifo;
	TIMER *timer_display;
	int i, fifobuf[128], count = 0;
	char strings[20];
	
	init_fifo(&fifo, 128, fifobuf, 0);
	
	timer_display = timer_alloc();
	timer_init(timer_display, &fifo, 1);
	timer_set(timer_display, 1);
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
			if(1 == i)
			{
				sprintf(strings, "%11d", count);
				displayStrings_atLayer(layer_bg, 0, 144, COL8_FFFFFF, COL8_008484, strings);
				timer_set(timer_display, 1);
			}
		}
	}
}











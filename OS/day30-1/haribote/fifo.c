/*FIFO循环队列型缓冲区*/

#include "bootpack.h"

//由于编译的时候不能通过，所以需要在这里特殊声明一下这个结构体的内部成员
struct TASK{
	int selector, flags;				//selector代表GDT中的描述符的位置，flags表示当前的任务状态：运行，挂起，未分配
	int priority, level;				//任务的优先级和所在的等级
	FIFO fifo;
	TSS tss;
	struct CONSOLE *console;			//每个任务由一个console启动
	int ds_base, console_stack;			//每个任务所在的数据段需要指定好，程序使用的栈所在的内存也需要记录
};

//初始化缓冲区的函数
void init_fifo(FIFO *fifo, int size, int *buf,struct TASK *task)
{
	fifo->size = size;
	fifo->buffer = buf;
	fifo->flags = FLAGS_NOTOVER;				//最初flags溢出标志肯定是未溢出的
	fifo->free = size;							//最初的剩余字节与所能容纳的字节数是一样的
	fifo->next_r = 0;							//读写位置都是0位置
	fifo->next_w = 0;
	fifo->task = task;							//有数据写入时需要唤醒的任务
}

//向FIFO缓冲区中保存数据
int fifo_put(FIFO *fifo, int data)
{
	//首先判断缓冲区是否溢出
	if(0 == fifo->free)
	{
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buffer[fifo->next_w] = data;
	fifo->next_w++;
	if(fifo->next_w == fifo->size)
	{
		fifo->next_w = 0;
	}
	fifo->free--;
	if(0 != fifo->task)				//如果有指定的需要唤醒的任务，就让它变为运行状态
	{
		if(fifo->task->flags != 2)		//只有处于休眠状态的任务才能被唤醒
		{
			task_run(fifo->task, -1, 0);	//唤醒任务，如果level设置为-1，则不需要改变任务的level，如果优先级是0，则不对其优先级进行改变
		}
	}
	return 0;
}

//从FIFO缓冲区中向外读取数据
int fifo_get(FIFO *fifo)
{
	int data;
	if(fifo->free == fifo->size)
	{
		return -1;
	}
	data = fifo->buffer[fifo->next_r];
	fifo->next_r++;
	if(fifo->next_r == fifo->size)
	{
		fifo->next_r = 0;
	}
	fifo->free++;
	return data;
}

//获取FIFO缓冲区当前的积攒的数据数量
int fifo_status(FIFO *fifo)
{
	return fifo->size - fifo->free;
}













#include "bootpack.h"

TIMERCTL timerctl;
extern TIMER *tm_timer;

//初始化PIT（可编程间隔型定时器），这个初始化与定时器的初始化不同，这个使用来开发定时器中断的
void init_PIT(void)
{
	int i;
	TIMER *timer;					//设定一个永远到达不了的定时器，作为链表的尾端，这样方便后续编程
	io_out8(PIT_CTRL, 0x34);		//设定中断间隔的步骤第一步：需要对0x0040端口传输0x34数据
	io_out8(PIT_CNT0, 0x9c);		//接下来需要对0x0043端口传输16位即4个字节的数据，线传送低8位
	io_out8(PIT_CNT0, 0x2e);		//最后传送高8位（最终用CPU主频除以这个16位的设定值就是中断频率了）
	timerctl.count = 0;
	for(i = 0; i < MAX_TIMER; i++)
	{	
		timerctl.timer[i].flags = TIMER_FLAGS_AVAILABLE;		//给所有的timer的flags位置为可用
	}
	timer = timer_alloc();			//这个定时器因为永远不会到达，所以不需要设置fifo缓冲区
	timer->timeout = 0xffffffff;
	timer->flags = TIMER_FLAGS_USING;
	timer->next = 0;
	
	timerctl.first_timer = timer;
	timerctl.next_timeout = 0xffffffff;
	return ;
}

//定时器分配
TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if(TIMER_FLAGS_AVAILABLE == timerctl.timer[i].flags)
		{
			timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0;
}

//单个定时器的初始化
void timer_init(TIMER *timer, FIFO *fifo, int data)
{
	timer->timer_fifo = fifo;
	timer->data = data;
	return ;
}

//定时器设定倒计时
void timer_set(TIMER *timer, unsigned int timeout)
{
	int eflags;
	TIMER *pre, *post;					//用来指向链表中的前一个和后一个节点，方便连接链表
	timer->timeout = timeout + timerctl.count;		//这个timeout需要加上当前时间，因为它代表的是指定时间
	timer->flags = TIMER_FLAGS_USING;
	//这期间需要对定时器进行设定，如果中间有定时器中断并且触发某个定时器的话，会写脏数据
	eflags = io_load_eflags();
	io_cli();
	
	post = timerctl.first_timer;		//先把post设置成指向链表的头结点
	//当前设置的这个定时器的预定时间是最靠前的话，直接插入到链表的最前面，即成为头结点
	if(timer->timeout <= post->timeout)		
	{
		//连接好链表，并重置下一个预订时间为当前的timer的预定时间
		timerctl.first_timer = timer;
		timer->next = post;
		timerctl.next_timeout = timer->timeout;
		io_store_eflags(eflags);
		return ;
	}
	//如果当前设置的定时器的预定时间比头结点的定时器的预定时间晚的话，需要查找合适的位置
	for(;;)
	{
		pre = post;
		post = post->next;
		//跟后面的那个节点比较就行，因为第一个节点已经在for之前的那个if语句中比较过了
		if(timer->timeout <= post->timeout)
		{
			pre->next = timer;
			timer->next = post;
			io_store_eflags(eflags);
			return ;
		}
	}
	return ;
}

//定时器释放
void timer_free(TIMER *timer)
{
	timer->flags = TIMER_FLAGS_AVAILABLE;
	return ;
}


//定时器的中断处理程序(同时兼顾了任务切换的工作)
void inthandler20(int *esp)
{
	char is_tm = 0;					//指示当前的timer是不是记录任务切换的那个定时器
	TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);		//把IRQ0信号接收完毕的信息通知PIC，使得其可以继续监视中断信号
	timerctl.count++;
	if(timerctl.next_timeout > timerctl.count)		//如果没有到下一个指定时间，说明没有定时器被触发，就不用继续了，而且大多数时候程序到这里就结束了
	{
		return ;
	}
	timer = timerctl.first_timer;
	for(;;)
	{
		//已经不需要再确认是否处于运行状态，这个链表中的定时器一定是处于运行状态才被添加到链表中的
		if(timer->timeout > timerctl.count)
		{
			break;
		}
		//只有到达了指定时间的定时器才被触发
		timer->flags = TIMER_FLAGS_ALLOC;
		if(timer != tm_timer)		//如果不是用来执行任务切换的那个定时器则正常的向缓冲区中填充数据
		{
			fifo_put(timer->timer_fifo, timer->data);
		}
		else			//如果这个定时器是用来执行任务切换的那个定时器
		{
			is_tm = 1;
		}
		timer = timer->next;			//让这个临时指针指向后面的定时器节点，以便下次循环继续检验
	}
	//让当前已经被触发的定时器后面的那个没有被触发定时器节点成为头结点
	timerctl.first_timer = timer;
	timerctl.next_timeout = timer->timeout;
	if(0 != is_tm)
	{	
		tm_taskSwitch();
	}
	return ;
}












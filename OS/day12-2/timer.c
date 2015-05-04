#include "bootpack.h"

TIMERCTL timerctl;

//初始化PIT（可编程间隔型定时器）
void init_PIT(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);		//设定中断间隔的步骤第一步：需要对0x0040端口传输0x34数据
	io_out8(PIT_CNT0, 0x9c);		//接下来需要对0x0043端口传输16位即4个字节的数据，线传送低8位
	io_out8(PIT_CNT0, 0x2e);		//最后传送高8位（最终用CPU主频除以这个16位的设定值就是中断频率了）
	timerctl.count = 0;
	for(i = 0; i < MAX_TIMER; i++)
	{	
		timerctl.timer[i].flags = TIMER_FLAGS_AVAILABLE;		//给所有的timer的flags位置为可用
	}
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

//定时器初始化
void timer_init(TIMER *timer, FIFO *fifo, unsigned char data)
{
	timer->timer_fifo = fifo;
	timer->data = data;
	return ;
}

//定时器设定倒计时
void timer_set(TIMER *timer, unsigned int timeout)
{
	timer->timeout = timeout;
	timer->flags = TIMER_FLAGS_USING;
	return ;
}

//定时器释放
void timer_free(TIMER *timer)
{
	timer->flags = TIMER_FLAGS_AVAILABLE;
	return ;
}


//定时器的中断处理程序
void inthandler20(int *esp)
{
	int i;
	io_out8(PIC0_OCW2, 0x60);		//把IRQ0信号接收完毕的信息通知PIC，使得其可以继续监视中断信号
	timerctl.count++;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if(TIMER_FLAGS_USING == timerctl.timer[i].flags)
		{
			timerctl.timer[i].timeout--;			//如果这个定时器正在使用的话，就把他的倒计时器减1
			if(0 == timerctl.timer[i].timeout)		//如果倒计时为0了，说明到时间了，可以触发这个定时器的事件了
			{
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;		//回到alloc这个已分配状态，而不是未使用状态，方便继续设定使用
				fifo_put(timerctl.timer[i].timer_fifo, timerctl.timer[i].data);
			}
		}
	}
	return ;
}












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
	timerctl.next = 0xffffffff;
	timerctl.using = 0;
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
void timer_init(TIMER *timer, FIFO *fifo, int data)
{
	timer->timer_fifo = fifo;
	timer->data = data;
	return ;
}

//定时器设定倒计时
void timer_set(TIMER *timer, unsigned int timeout)
{
	int eflags, i, j;
	timer->timeout = timeout + timerctl.count;		//这个timeout需要加上当前时间，因为它代表的是指定时间
	timer->flags = TIMER_FLAGS_USING;
	//这期间需要对定时器进行设定，如果中间有定时器中断并且触发某个定时器的话，会写脏数据
	eflags = io_load_eflags();
	io_cli();
	for(i = 0; i < timerctl.using; i++)
	{
		//寻找比当前的timer的预定时间更长的那个timer所在的位置，之后要把本timer放在它前面
		if(timerctl.timers_inorder[i]->timeout > timer->timeout)
		{
			break;
		}
	}
	//i号之后的timer全部向后移动一位
	for(j = timerctl.using; j > i; j--)
	{
		timerctl.timers_inorder[j] = timerctl.timers_inorder[j - 1];
	}
	//using的个数增加一个
	timerctl.using++;
	//把当前的timer添加当找好的位置i上
	timerctl.timers_inorder[i] = timer;
	timerctl.next = timerctl.timers_inorder[0]->timeout;
	io_store_eflags(eflags);
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
	int i, j;
	io_out8(PIC0_OCW2, 0x60);		//把IRQ0信号接收完毕的信息通知PIC，使得其可以继续监视中断信号
	timerctl.count++;
	if(timerctl.next > timerctl.count)		//如果没有到下一个指定时间，说明没有定时器被触发，就不用继续了，而且大多数时候程序到这里就结束了
	{
		return ;
	}
	for(i = 0; i < timerctl.using; i++)
	{
		//因为是从timer_inorder中选择的，这些timer必定是使用中的；这里最好写成大于号，因为如果中间有因中断禁止而跳过的定时器，也能在此触发
		if(timerctl.timers_inorder[i]->timeout > timerctl.count)		//如果这个指定时间还没有到达，则它后面的计时器肯定也没有到达
		{
			break;
		}
		timerctl.timers_inorder[i]->flags = TIMER_FLAGS_ALLOC;		//回到alloc这个已分配状态，而不是未使用状态，方便继续设定使用
		fifo_put(timerctl.timers_inorder[i]->timer_fifo, timerctl.timers_inorder[i]->data);
	}
	timerctl.using -= i;		//把已经到时间的定时器从timers_inorder当中出去，并且using也减去相应的个数i
	//把timers_inorder中还没有到时间的定时器向前移动
	for(j = 0; j < timerctl.using; j++)
	{
		timerctl.timers_inorder[j] = timerctl.timers_inorder[j + i];
	}
	//如果还有timer没到时间，则给next赋值
	if(timerctl.using > 0)
	{
		timerctl.next = timerctl.timers_inorder[0]->timeout;
	}
	//如果已经没有timer在使用了，则把next赋值成最大值
	else
	{
		timerctl.next = 0xffffffff;
	}
	return ;
}












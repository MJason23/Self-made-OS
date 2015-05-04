#include "bootpack.h"

TIMER *tm_timer;		//任务切换时用的定时器
int tm_TR;				//指示当前的TR寄存器的值

//任务管理初始化
void tm_init(void)
{
	tm_timer = timer_alloc();
	//这个计时器不需要向缓冲区中写入东西，所以就不用初始化了
	timer_set(tm_timer,2);
	tm_TR = 3 * 8;
	return ;
}

//真正的执行任务切换
void tm_taskSwitch(void)
{
	if(3 * 8 == tm_TR)
	{
		tm_TR = 4 * 8;
	}
	else if(4 * 8 == tm_TR)
	{
		tm_TR = 3 * 8;
	}
	timer_set(tm_timer, 2);
	farjmp(0, tm_TR);
	return ;
}

void task_b_main(LAYER *layer_bg)
{
	FIFO fifo;
	TIMER *timer_display;
	int i, fifobuf[128], count = 0;
	char strings[20];
	
	init_fifo(&fifo, 128, fifobuf);
	
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











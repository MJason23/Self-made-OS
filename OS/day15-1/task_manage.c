#include "bootpack.h"

void task_b_main(void)
{
	FIFO fifo;
	TIMER *timer;
	int i,fifobuf[128];
	
	init_fifo(&fifo, 128, fifobuf);
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_set(timer, 500);
	for(;;)
	{
		io_cli();
		if(0 == fifo_status(&fifo))
		{
			io_stihlt();
		}
		else
		{
			i = fifo_get(&fifo);
			io_sti();
			if(1 == i)
			{
				taskswitch3();
			}
		}
	}
}











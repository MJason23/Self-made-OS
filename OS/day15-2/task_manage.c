#include "bootpack.h"

void task_b_main(LAYER *layer_bg)
{
	FIFO fifo;
	TIMER *timer, *timer_display;
	int i, fifobuf[128], count = 0;
	char strings[20];
	
	init_fifo(&fifo, 128, fifobuf);
	
	timer = timer_alloc();
	timer_init(timer, &fifo, 2);
	timer_set(timer, 2);
	
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
			if(2 == i)
			{
				farjmp(0, 3*8);
				timer_set(timer, 2);
			}
			else if(1 == i)
			{
				sprintf(strings, "%11d", count);
				displayStrings_atLayer(layer_bg, 0, 144, COL8_FFFFFF, COL8_008484, strings);
				timer_set(timer_display, 1);
			}
		}
	}
}











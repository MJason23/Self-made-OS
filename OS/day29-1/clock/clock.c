#include <stdio.h>
#include "apilib.h"

void HariMain(void)
{
	char *buffer, string[12];
	int window, timer, second = 0, minute = 0, hour = 0;
	api_initmalloc();
	buffer = api_malloc(150 * 100);
	window = api_create_window(buffer, 150, 100, -1, "clock");
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	for(;;)
	{
		sprintf(string, "%5d:%02d:%02d", hour, minute, second);
		api_drawrectangle_onwindow(window, 28, 27, 115, 41, 7);
		api_putstring_onwindow(window, 28, 27, 0, 11, string);
		api_settimer(timer, 100);
		if(128 != api_get_fifodata(1))
		{
			break;
		}
		second++;
		if(second == 60)
		{
			second = 0;
			if(minute == 60)
			{
				minute = 0;
				hour++;
			}
		}
	}
	api_end();
}



















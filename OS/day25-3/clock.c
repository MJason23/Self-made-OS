#include <stdio.h>

void api_drawline(int window, int x0, int y0, int x1, int y1, int color);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
int api_create_window(char *buffer, int length, int width, int color_luc, char *title);
void api_end(void);
void api_putstring_onwindow(int window, int x, int y, int color, int length, char *string);
void api_drawrectangle_onwindow(int window, int x, int y, int length, int width, int color);
void api_refreshwindow(int window, int x0, int y0, int x1, int y1);
int api_get_fifodata(int mode);
void api_closewindow(int window);
int api_alloctimer(void);
void api_inittimer(int timer, int data);
void api_settimer(int timer, int time);
void api_freetimer(int timer);

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



















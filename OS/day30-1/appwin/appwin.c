#include "apilib.h"

void HariMain(void)
{
	char *buffer;
	int window, i, x, y;
	api_initmalloc();
	buffer = api_malloc(150 * 50);
	window = api_create_window(buffer, 150, 50, -1, "App_Win");
	api_drawrectangle_onwindow(window + 1, 8, 28, 134, 18, 6);
	api_putstring_onwindow(window + 1, 28, 28, 0, 12, "Hello world!");
	api_refreshwindow(window, 6, 26, 150, 50);
	api_end();
}
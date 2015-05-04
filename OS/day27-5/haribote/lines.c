#include "apilib.h"

//char buffer[160 * 100];

void HariMain(void)
{
	char *buffer;
	int window, i;
	api_initmalloc();
	buffer = api_malloc(160 * 100);
	window = api_create_window(buffer, 160, 100, -1, "Lines");
	for(i = 0;i < 8; i++)
	{
		api_drawline(window + 1, 8, 26, 77, i * 9 + 26, i);
		api_drawline(window + 1, 88, 26, i * 9 + 88, 89, i);
	}
	api_refreshwindow(window, 6, 26, 154, 90);
	for(;;)
	{
		if(0x0a == api_get_fifodata(1))
		{
			break;
		}
	}
	api_closewindow(window);
	api_end();
}
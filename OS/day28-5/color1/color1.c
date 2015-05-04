#include "apilib.h"


void HariMain(void)
{
	char *buffer;
	int window, x, y, r, g, b;
	api_initmalloc();
	buffer = api_malloc(144 * 164);
	window = api_create_window(buffer, 144, 164, -1, "color1");
	for(y = 0; y < 128; y++)
	{
		for(x = 0; x < 128; x++)
		{
			r = x * 2;
			g = y * 2;
			b = 0;
			buffer[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
		}
	}
	api_refreshwindow(window, 8, 28, 136, 156);
	api_get_fifodata(1);
	api_end();
}
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
int api_create_window(char *buffer, int length, int width, int color_luc, char *title);
void api_end(void);
void api_putstring_onwindow(int window, int x, int y, int color, int length, char *string);
void api_drawrectangle_onwindow(int window, int x, int y, int length, int width, int color);
void api_refreshwindow(int window, int x0, int y0, int x1, int y1);

int rand(void);

void HariMain(void)
{
	char *buffer;
	int window, i, x, y;
	api_initmalloc();
	buffer = api_malloc(150 * 100);
	window = api_create_window(buffer, 150, 100, -1, "Stars");
	api_drawrectangle_onwindow(window + 1, 6, 26, 138, 68, 0);
	for(i = 0;i < 50; i++)
	{
		x = (rand() % 138) + 6;
		y = (rand() % 68) + 26;
		api_drawpoint(window + 1, x, y, 3);
	}
	api_refreshwindow(window, 6, 26, 150, 100);
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


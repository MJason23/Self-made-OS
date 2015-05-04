void api_drawline(int window, int x0, int y0, int x1, int y1, int color);
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
int api_create_window(char *buffer, int length, int width, int color_luc, char *title);
void api_end(void);
void api_putstring_onwindow(int window, int x, int y, int color, int length, char *string);
void api_drawrectangle_onwindow(int window, int x, int y, int length, int width, int color);
void api_refreshwindow(int window, int x0, int y0, int x1, int y1);
int api_getkey(int mode);
void api_closewindow(int window);

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
		if(0x0a == api_getkey(1))
		{
			break;
		}
	}
	api_closewindow(window);
	api_end();
}
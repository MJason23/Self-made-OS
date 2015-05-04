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

void HariMain(void)
{
	char *buffer;
	int window, i, x, y;
	api_initmalloc();
	buffer = api_malloc(160 * 100);
	window = api_create_window(buffer, 160, 100, -1, "game");
	api_drawrectangle_onwindow(window, 4, 24, 152, 72, 0);
	x = 76;
	y = 56;
	api_putstring_onwindow(window, x, y, 3, 1, "*");
	for(;;)
	{
		i = api_getkey(1);
		api_putstring_onwindow(window, x, y, 0, 1, "*");
		if('4' == i && x > 4)
		{
			x -= 8;
		}
		if('6' == i && x < 148)
		{
			x += 8;
		}
		if('8' == i && y > 24)
		{
			y -= 8;
		}
		if('2' == i && y < 80)
		{
			y += 8;
		}
		if(0x0a == i)			//按下回车键就结束
		{
			break;
		}
		api_putstring_onwindow(window, x, y, 3, 1, "*");
	}
	api_closewindow(window);
	api_end();
}








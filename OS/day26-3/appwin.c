void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
int api_create_window(char *buffer, int length, int width, int color_luc, char *title);
void api_end(void);
void api_putstring_onwindow(int window, int x, int y, int color, int length, char *string);
void api_drawrectangle_onwindow(int window, int x, int y, int length, int width, int color);
void api_refreshwindow(int window, int x0, int y0, int x1, int y1);

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
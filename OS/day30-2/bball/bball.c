#include "apilib.h"

void HariMain(void)
{
	int window, i, j, dis;
	char buffer[216 * 237];
	struct POINT{
		int x, y;
	};
	static struct POINT table[16] = {
		{ 204, 129 }, { 195,  90 }, { 172,  58 }, { 137,  38 }, {  98,  34 },
		{  61,  46 }, {  31,  73 }, {  15, 110 }, {  15, 148 }, {  31, 185 },
		{  61, 212 }, {  98, 224 }, { 137, 220 }, { 172, 200 }, { 195, 168 },
		{ 204, 129 }
	};
	window = api_create_window(buffer, 216, 237, -1, "bball");
	api_drawrectangle_onwindow(window, 8, 29, 200, 200, 0);
	for(i = 0; i <= 14; i++)
	{
		for(j = i + 1; j <= 15;j++)
		{
			dis = j - i;			//两点间的距离
			if(dis >= 8)
			{
				dis = 15 - dis;
			}
			if(dis != 0)
			{
				api_drawline(window, table[i].x, table[i].y, table[j].x, table[j].y, 8 - dis);
			}
		}
	}
	for(;;)
	{
		if(0x0a == api_get_fifodata(1))
		{
			break;
		}
	}
	api_end();
}







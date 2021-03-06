#include "bootpack.h"

//制作窗口的函数
void create_window(unsigned char *buf, int length, int width, char *title)
{
	/*设计关闭按钮图形*/
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char color;
	
	/*向窗口缓冲区写入窗口图形内容*/
	drawRectangle(buf, length, COL8_C6C6C6, 0, 0, length, 1);
	drawRectangle(buf, length, COL8_FFFFFF, 1, 1, length - 2, 1);
	drawRectangle(buf, length, COL8_C6C6C6, 0, 0, 1, width);
	drawRectangle(buf, length, COL8_FFFFFF, 1, 1, 1, width - 2);
	drawRectangle(buf, length, COL8_848484, length - 2, 1, 1, width - 2);
	drawRectangle(buf, length, COL8_000000, length - 1, 0, 1, width);
	drawRectangle(buf, length, COL8_C6C6C6, 2, 2, length - 4, width - 4);
	drawRectangle(buf, length, COL8_000084, 3, 3, length - 6, 18);
	drawRectangle(buf, length, COL8_848484, 1, width - 2, length - 2, 1);
	drawRectangle(buf, length, COL8_000000, 0, width - 1, length, 1);
	displayStrings_CS(buf, length, 24, 4, COL8_FFFFFF, title);
	
	/*向关闭按钮缓冲区写入它的内容*/
	for(y = 0; y < 14; y++)
	{
		for(x = 0; x < 16; x++)
		{
			color = closebtn[y][x];
			if('@' == color)
			{
				color = COL8_000000;
			}
			else if ('$' == color) 
			{
				color = COL8_848484;
			} 
			else if ('Q' == color) 
			{
				color = COL8_C6C6C6;
			} 
			else 
			{
				color = COL8_FFFFFF;
			}
			buf[(5 + y) * length + (length - 21 + x)] = color;
		}
	}
	return ;
}

//制作输入框背景的函数
void create_textbox(LAYER *layer, int x, int y, int length, int width, int color)
{
	drawRectangle(layer->buffer, layer->length, COL8_848484, x - 2, y - 3, length + 4, 1);
	drawRectangle(layer->buffer, layer->length, COL8_848484, x - 3, y - 3, 1, width + 5);
	drawRectangle(layer->buffer, layer->length, COL8_FFFFFF, x - 3, y + width + 2, length + 5, 1);
	drawRectangle(layer->buffer, layer->length, COL8_FFFFFF, x + length + 2, y - 3, 1, width + 6);
	drawRectangle(layer->buffer, layer->length, COL8_000000, x - 1, y - 2, length + 2, 1);
	drawRectangle(layer->buffer, layer->length, COL8_000000, x - 2, y - 2, 1, width + 3);
	drawRectangle(layer->buffer, layer->length, COL8_C6C6C6, x - 2, y + width + 1, length + 3, 1);
	drawRectangle(layer->buffer, layer->length, COL8_C6C6C6, x + length + 1, y - 2, 1,width + 4);
	drawRectangle(layer->buffer, layer->length, color, x - 1, y - 1, length + 2, width + 2);
	return ;
}











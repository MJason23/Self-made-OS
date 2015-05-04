#include "apilib.h"
#include <stdio.h>

int strtol(char *s, char **endp, int base);
char *skipspace(char *p);
void textview(int window, int length, int width, int xskip, char *p, int tab, int lan);
char *lineview(int window, int length, int width, int xskip, unsigned char *p, int tab, int lan);
int puttab(int x, int length, int xskip, char *s, int tab);

void HariMain(void)
{
	char window_buffer[1024 * 757], text_buffer[240 * 1024];
	int length = 30, width = 10, tab = 4, speed_x = 1, speed_y = 1;
	int window, fhandle, i, j, lan = api_getlan(), xskip = 0;
	char cmdline[30], *p, *q = 0, *r = 0;
	
	api_cmdline(cmdline, 30);
	//跳过命令行中的空格
	for(p = cmdline; *p > ' '; p++){}
	/*对命令进行解析*/
	for(; *p != 0;)
	{
		//如果有空格，就跳过空格
		p = skipspace(p);
		if(*p == '-')
		{
			//分别设置阅读器的长度和宽度以及tab制表符的宽度
			if(p[1] == 'w')
			{
				length = strtol(p + 2, &p, 0);
				//如果宽度小于20个字符宽度，就设置为20个字符宽度，如果大于126个字符宽度，就设置为126个字符宽度
				if(length < 20)
				{
					length = 20;
				}
				if(length > 126)
				{
					length = 126;
				}
			}
			else if(p[1] == 'h')
			{
				width = strtol(p + 2, &p, 0);
				if(width < 1)
				{
					width = 1;
				}
				if(width > 45)
				{
					width = 45;
				}
			}
			else if(p[1] == 't')
			{
				tab = strtol(p + 2, &p, 0);
				if(tab < 1)
				{
					tab = 4;
				}
				if(tab > 8)
				{
					tab = 8;
				}
			}
			else
			{
				api_putstring_toend("Command line set format error!");
				api_end();
			}
		}
		//查找文件名
		else
		{
			//按正常的命令格式的话，这个else只会进入一次，就是第一次，之后就不会再进入；最初的设置q为0，然后q就会记录上
			//文件名地址，就不会是0了，如果在进入这个else就说明命令行格式有问题
			if(0 != q)
			{
				api_putstring_toend("Command line file name format error!");
				api_end();
			}
			q = p;		//q其实记录的是命令中的文件名起始地址
			for(; *p > ' '; p++){}
			r = p;		//r记录的是文件名的最后一个字符后面的那个字符（一般是空格）地址
		}
	}	
	//如果出了循环之后，q中还没有记录文件名的地址，就说明命令有问题
	if(0 == q)
	{
		api_putstring_toend("Command line file name format error!");
		api_end();
	}
	
	/*准备显示文字的窗口*/
	window = api_create_window(window_buffer, length * 8 + 16, width * 16 + 37, -1, "txtview");
	api_drawrectangle_onwindow(window, 6, 27, length * 8 + 4, width * 16 + 4, 7);
	
	/*向窗口中载入文本文件*/
	*r = 0;				//给文件名的字符串结尾加上'\0'
	fhandle = api_fopen(q);
	if(0 == fhandle)
	{
		api_putstring_toend("File open error!");
		api_end();
	}
	j = api_fsize(fhandle, 0);
	if(j >= 240 * 1024 - 1)
	{
		j = 240 * 1024 - 2;
	}
	text_buffer[0] = 0x0a;			//乘放文件内容的内存中第一个字节放一个标志位用来代表文件内容的起始位置
	api_fread(text_buffer + 1, j, fhandle);
	api_fclose(fhandle);
	text_buffer[j + 1] = 0;			//文件内容的最后一个字节放一个结尾符'\0'
	q = text_buffer + 1;			//q记录真正文件内容的起始地址
	//由于本系统不处理0x0d符号，所以把文件中的0x0d字符全部略去，因此需要从新排布文件内容
	for(p = text_buffer + 1; *p != 0; p++)
	{
		if(*p != 0x0d)
		{
			*q = *p;
			q++;
		}
	}
	*q = 0;				//文件内容结尾加上'\0'
	
	/*阅读器主体程序，显示文件内容，并且控制光标移动等等*/
	p = text_buffer + 1;
	for(;;)
	{
		textview(window, length, width, xskip, p, tab, lan);
		i = api_get_fifodata(1);
		//键入Q或者q的时候退出阅读器
		if('Q' == i || 'q' == i)
		{
			api_end();
		}
		if('A' <= i && i <= 'F')
		{
			speed_x = 1 << (i - 'A');		//横向移动速度等级分别为1,2,4,8,16,32个字节，共6个等级
		}
		if('a' <= i && i <= 'f')
		{
			speed_y = 1 << (i - 'a');		//纵向移动速度等级分别为1,2,4,8,16,32个字节，共6个等级
		}
		//"<"和">"控制tab制表符的大小
		if(i == '<' && tab > 1)
		{
			tab /= 2;
		}
		if(i == '>' && tab < 8)
		{
			tab *= 2;
		}
		//按下4是要左移光标
		if('4' == i)
		{
			for(;;)
			{
				xskip -= speed_x;
				if(xskip < 0)
				{
					xskip = 0;
				}
				if(api_get_fifodata(0) != '4')
				{
					break;
				}
			}
		}
		//按下6是要右移光标
		if('6' == i)
		{
			for(;;)
			{
				xskip += speed_x;
				if(api_get_fifodata(0) != '6')
				{
					break;
				}
			}
		}
		//按下8是要向上移动光标
		if('8' == i)
		{
			for(;;)
			{
				for(j = 0; j < speed_y; j++)
				{
					if(p == text_buffer + 1)
					{
						break;
					}
					for(p--; p[-1] != 0x0a;p--){}		//回溯到上一个字符为直到0x0a为止（0x0a是文件内容的起始位置）
				}
				if('8' == api_get_fifodata(0))
				{
					break;
				}
			}
		}
		//按下2是要向下移动光标
		if('2' == i)
		{
			for(;;)
			{
				for(j = 0; j < speed_y; j++)
				{
					for(q = p; *q != 0 && *q != 0x0a; q++){}
					if(0 == *q)
					{
						break;
					}
					p = q + 1;
				}
				if('2' == api_get_fifodata(0))
				{
					break;
				}
			}
		}
	}
}

//跳过空格的函数
char *skipspace(char *p)
{
	for(; *p == ' '; p++){}
	return p;
}

void textview(int window, int length, int width, int xskip, char *p, int tab, int lan)
{
	int i;
	api_drawrectangle_onwindow(window + 1, 8, 29, length * 8, width *16, 7);
	for(i = 0; i < width; i++)
	{
		p = lineview(window, length, i * 16 + 29, xskip, p, tab, lan);
	}
	api_refreshwindow(window, 8, 29, length * 8 + 8, width * 16 + 29);
	return ;
}

//显示文件的每一行内容
char *lineview(int window, int length, int width, int xskip, unsigned char *p, int tab, int lan)
{
	int x = -xskip;
	char s[130];
	for(;;)
	{
		if(0 == *p)
		{
			break;
		}
		if(0x0a == *p)
		{
			p++;
			break;
		}
		//当前语言模式为英文的情况下
		if(0 == lan)
		{
			//打印制表符
			if(0x09 == *p)
			{
				x = puttab(x, length, xskip, s, tab);
			}
			//打印正常字符
			else
			{
				if(0 <= x && x < length)
				{
					s[x] = *p;
				}
				x++;
			}
			p++;
		}
		//当前语言模式为中文模式的情况下
		if(1 == lan)
		{
			//打印制表符
			if(0x09 == *p)
			{
				x = puttab(x, length, xskip, s, tab);
				p++;
			}
			//打印全角字符
			else if(0xa1 <= *p && *p <= 0xfe)
			{
				if(-1 == x)
				{
					s[0] = ' ';
				}
				if(0 <= x && x <= length - 1)
				{
					s[x] = *p;
					s[x + 1] = p[1];
				}
				if(x == length - 1)
				{
					s[x] = ' ';
				}
				x += 2;
				p += 2;
			}
			//打印半角字符
			else
			{
				if(0 <= x && x < length)
				{
					s[x] = *p;
				}
				x++;
				p++;
			}
		}
	}
	if(x > length)
	{
		x = length;
	}
	if(x > 0)
	{
		s[x] = 0;
		api_putstring_onwindow(window + 1, 8, width, 0, x, s);
	}
	return p;
}

//画一个制表符
int puttab(int x, int length, int xskip, char *s, int tab)
{
	for(;;)
	{
		//画空格，直到到达制表符的位置为止
		if(0 <= x && x < length)
		{
			s[x] = ' ';
		}
		x++;
		if((x + xskip) % tab == 0)
		{
			break;
		}
	}
	return x;
}










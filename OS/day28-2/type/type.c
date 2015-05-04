#include "apilib.h"
#include <stdio.h>
	
void HariMain(void)
{
	int fhandle;
	char c, cmdline[30], *p;
	//1.先获取到cmdline
	api_cmdline(cmdline, 30);
	//2.从cmdline中提取出来文件名
	for(p = cmdline; *p > ' '; p++){}	
	for(; *p == ' '; p++){}		//跳过空过之前的命令，这样一来，现在的p就是文件名的首地址
	//3.查找有木有该文件
	//p = "ipl10.nas";
	fhandle = api_fopen(p);
	//4.找到该文件的话输出该文件的内容
	if(0 != fhandle)
	{
		for(; 0 != api_fread(&c, 1, fhandle);)
		{
			api_putchar(c);
		}
	}
	else
	{
		api_putstring_toend("File not found in type!");
	}
	//5.退出
	api_end();
}
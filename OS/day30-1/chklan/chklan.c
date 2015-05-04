#include "apilib.h"

void HariMain(void)
{
	int lanmode = api_getlan();
	char chimode[] = "中文语言模式！";
	if(0 == lanmode)
	{
		api_putstring_toend("English ASCII mode!\n");
	}
	else if(1 == lanmode)
	{
		api_putstring_toend(chimode);
	}
	api_end();
}
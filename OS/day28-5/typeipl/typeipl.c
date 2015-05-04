#include "apilib.h"

void HariMain(void)
{
	int fhandle;
	char c;
	fhandle = api_fopen("ipl10.nas");
	if(0 != fhandle)
	{
		for(;;)
		{
			if(0 == api_fread(&c, 1, fhandle))
			{
				break;
			}
			api_putchar(c);
		}
	}
	api_end();
}
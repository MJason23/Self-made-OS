#include "apilib.h"

void HariMain(void)
{
	static char string[7] = {0xC6, 0xFC, 0xCB, 0xDC, 0xB8, 0xEC, 0x00};
	api_putstring_toend(string);
	api_end();
}

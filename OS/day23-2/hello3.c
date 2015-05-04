void api_putchar(int c);
void api_end(void);

void HariMain(void)
{
	api_putstring_toend("Hello world!\n");
	api_end();
}
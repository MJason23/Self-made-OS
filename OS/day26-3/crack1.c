void api_end();

//企图更改操作系统的破坏函数
void HariMain(void)
{
	*((char *) 0x00102600) = 0;				//破坏操作系统的数据，把记录文件信息的第一个区域第一个字节设置为0
	api_end();
	return ;
}
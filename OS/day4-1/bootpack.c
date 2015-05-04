void io_hlt(void);
void write_mem8(int addr,int data);

void HariMain(void)
{
	int i;				//在这里遇到了麻烦，原本写的是把int定义到了for里面，但这里似乎不支持，编译信息提示只有C99标准才支持这种命名方式，以后注意了
	for(i = 0xa0000;i <= 0xaffff;i++)   //这个循环把整个显示内存的区域全部填充成为了15，这个15指定了显示的颜色为白色，所以启动的系统界面是纯白色的
	{
		write_mem8(i,15);
	}
	for(;;)
	{
		io_hlt();						//执行完成启动系统之后就让CPU停下来
	}	
}
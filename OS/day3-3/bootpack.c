/*函数声明*/

void io_hlt(void);

/*主体函数*/
void HariMain(void)
{
	fin:
		io_hlt();			//执行naskfunc.nas里的_io_hlt函数，由于C语言中没有让CPU停止工作的函数和命令
		goto fin;
}
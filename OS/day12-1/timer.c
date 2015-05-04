#include "bootpack.h"

TIMERCTL timerctl;

//初始化PIT（可编程间隔型定时器）
void init_PIT(void)
{
	io_out8(PIT_CTRL, 0x34);		//设定中断间隔的步骤第一步：需要对0x0040端口传输0x34数据
	io_out8(PIT_CNT0, 0x9c);		//接下来需要对0x0043端口传输16位即4个字节的数据，线传送低8位
	io_out8(PIT_CNT0, 0x2e);		//最后传送高8位（最终用CPU主频除以这个16位的设定值就是中断频率了）
	timerctl.count = 0;
	return ;
}

void inthandler20(int *esp)
{
	io_out8(PIC0_OCW2, 0x60);		//把IRQ0信号接收完毕的信息通知PIC，使得其可以继续监视中断信号
	/*暂时什么都还不做*/
	timerctl.count++;
	return ;
}
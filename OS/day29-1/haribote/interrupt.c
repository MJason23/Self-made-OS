/*
** 记住一点：中断处理程序切不可写的冗长，否则会影响其他程序或者中断的响应；中断处理程序其实只要做到最简单的接收到硬件传来的信息即可，剩下的就交给中断之外的需要
**这个信息的程序去更有效的处理就可以了！（这里就是交给了C语言部分处理，比如把键盘传进来的消息打印到屏幕上，打印这部分就不用中断处理去做了！！！！！）
*/


#include "bootpack.h"


//初始化PIC主/从板（监视中断的芯片）数据，这部分详细知识在书上P117
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); //禁止主PIC（PIC0）的所有中断
	io_out8(PIC1_IMR,  0xff  ); //禁止从PIC（PIC1）的所有中断

	io_out8(PIC0_ICW1, 0x11  ); //边沿触发模式（写死的，每个操作系统都是这样，这是硬件的电器属性，必须这么写，没有第二种写法）
	io_out8(PIC0_ICW2, 0x20  ); //IRQ0-7由INT20-27接收（只有这个选项可以人为设定，根据需要而不同，前32种中断已经被CPU自己占用，它们被用于系统保护，由CPU自己产生）
	io_out8(PIC0_ICW3, 1 << 2); //从PIC由IRQ2连接
	io_out8(PIC0_ICW4, 0x01  ); //无缓冲区模式（也是写死的，没有第二种写法）

	io_out8(PIC1_ICW1, 0x11  ); //
	io_out8(PIC1_ICW2, 0x28  ); //IRQ8-15由INT28-2f接收
	io_out8(PIC1_ICW3, 2     ); //从PIC由IRQ2连接
	io_out8(PIC1_ICW4, 0x01  ); //

	io_out8(PIC0_IMR,  0xfb  ); //主PIC只开发IRQ2，其他信号全部中断
	io_out8(PIC1_IMR,  0xff  ); //从PIC中断全部IRQ信号
}


void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67);
}











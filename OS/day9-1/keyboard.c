#include "bootpack.h"

FIFO keyFIFO;

//让键盘控制电路做好准备动作，等待控制指令的到来
void wait_KBC_sendready(void)
{
	for (;;) 
	{
		//这是一个用来比较的数，当CPU从键盘控制器电路读取到的数据倒数第二位是0时，说明键盘控制电路已经可以接受CPU指令了
		if (0 == (io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY)) 
		{
			break;
		}
	}
	return;
}

//激活键盘控制电路
void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SET_MODE);				//确认可以向键盘控制电路发送信息后，发送模式设定指令
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA, KBC_MOUSE_MODE);				//发送鼠标模式的指令
}

void inthandler21(int *esp)
{
	unsigned char data;
	data = io_in8(PORT_KEYDATA);		//键盘输入的信息从0x0060传入一个字节，由data接收到（键盘电路设计一次只能传入一个字节）
	io_out8(PIC0_OCW2,0x61);			//CPU通知PIC0“IRQ1已经受理完毕”
	fifo_put(&keyFIFO, data);
}
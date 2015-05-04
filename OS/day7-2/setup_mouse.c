#include "bootpack.h"

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
	io_out8(PORT_KEYDAT, KBC_MOUSE_MODE);				//发送鼠标模式的指令
}

//激活鼠标
void enable_mouse(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);			//如果向键盘发送0xd4指令，则下一个数据就会自动发送给鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);				//发送激活鼠标指令
}









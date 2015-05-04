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
	io_out8(PORT_KEYDATA, KBC_MOUSE_MODE);				//发送鼠标模式的指令
}

//激活鼠标
void enable_mouse(MOUSE_DECODE *mouse_dec)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);			//如果向键盘发送0xd4指令，则下一个数据就会自动发送给鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA, MOUSECMD_ENABLE);				//发送激活鼠标指令
	mouse_dec->phase = 0;								//进入等待0xfa的阶段
}

//解析鼠标传入的信息（鼠标每次都会产生三个中断传入3个字节的信息）
int mouse_decode(MOUSE_DECODE *mouse_dec, unsigned char data)
{
	//收到鼠标传来的0xfa信息后，说明鼠标已经激活了，就可转为开始接收鼠标传入信息的状态了，这个阶段只有一次
	if(0 == mouse_dec->phase)
	{
		if(0xfa == data)
		{
			mouse_dec->phase = 1;
		}
		return 0;
	}
	//将鼠标按键的信息存入到buffer[0]中
	if(1 == mouse_dec->phase)
	{
		if(0x08 == (data & 0xc8))
		{
			mouse_dec->buffer[0] = data;
			mouse_dec->phase = 2;
		}
		return 0;
	}
	//将左右移动的信息存入到buffer[1]中
	if(2 == mouse_dec->phase)
	{
		mouse_dec->buffer[1] = data;
		mouse_dec->phase = 3;
		return 0;
	}
	//将上下移动的信息存入到buffer[2]中
	if(3 == mouse_dec->phase)
	{
		mouse_dec->buffer[2] = data;
		mouse_dec->phase = 1;
		
		mouse_dec->btn = mouse_dec->buffer[0] & 0x07;			//鼠标键的状态放在buffer[0]的低三位，所以只取低三位的数据就行了，三位分别代表左右中
		mouse_dec->x = mouse_dec->buffer[1];
		mouse_dec->y = mouse_dec->buffer[2];
		if(0 != (mouse_dec->buffer[0] &0x10))			//自己估计这两个判断语句应该是有关是否出现横竖移动的
		{
			mouse_dec->x |= 0xffffff00;
		}
		if(0 != (mouse_dec->buffer[0] &0x20))
		{
			mouse_dec->y |= 0xffffff00;
		}
		mouse_dec->y = -mouse_dec->y;		//鼠标的坐标系统y轴与画面的坐标系统的y轴相反
		return 1;
	}
	return -1;			//不出意外应该不会走到这一步
}







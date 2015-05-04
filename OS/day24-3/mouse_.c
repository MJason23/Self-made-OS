#include "bootpack.h"

FIFO *mouseFIFO;
int mouse_offset;

//激活鼠标
void enable_mouse(FIFO *fifo, int offset, MOUSE_DECODE *mouse_dec)
{
	mouseFIFO = fifo;
	mouse_offset = offset;
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

//鼠标中断处理程序
void inthandler2c(int *esp)
{
	int data;
	data = io_in8(PORT_KEYDATA) + 512;		//读取数据方式与键盘一样，所以只能到主程序中通过各自的缓冲区判断是鼠标的数据还是键盘的数据
	io_out8(PIC1_OCW2,0x64);			//必须先通知从PIC的IRQ12已经处理完毕，然后再通知主PIC(从PIC控制IRQ8到15，IRQ12是第4个，所以是0x64)
	io_out8(PIC0_OCW2,0x62);			//因为主从PIC的协调不能靠自己完成，必须由主程序教给他们怎么做
	fifo_put(mouseFIFO, data);
}







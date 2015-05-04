#include "bootpack.h"

FIFO *keyFIFO;
int keyboard_offset;			//全局变量，作为键盘数据的偏移值

char key_table[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
};

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
void init_keyboard(FIFO *fifo, int offset)
{
	keyFIFO = fifo;
	keyboard_offset = offset;
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SET_MODE);				//确认可以向键盘控制电路发送信息后，发送模式设定指令
	wait_KBC_sendready();
	io_out8(PORT_KEYDATA, KBC_MOUSE_MODE);				//发送鼠标模式的指令
}

void inthandler21(int *esp)
{
	int data;
	data = io_in8(PORT_KEYDATA) + 256;		//键盘输入的信息从0x0060传入一个字节，由data接收到（键盘电路设计一次只能传入一个字节）
	io_out8(PIC0_OCW2,0x61);			//CPU通知PIC0“IRQ1已经受理完毕”，这样IRQ1信号线就可以继续接收键盘中断信号了，要不然它无法继续接收
	fifo_put(keyFIFO, data);
}












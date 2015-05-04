/*FIFO循环队列型缓冲区*/

#include "bootpack.h"


//初始化缓冲区的函数
void init_fifo(FIFO *fifo, int size, int *buf)
{
	fifo->size = size;
	fifo->buffer = buf;
	fifo->flags = FLAGS_NOTOVER;				//最初flags溢出标志肯定是未溢出的
	fifo->free = size;							//最初的剩余字节与所能容纳的字节数是一样的
	fifo->next_r = 0;							//读写位置都是0位置
	fifo->next_w = 0;
}

//向FIFO缓冲区中保存数据
int fifo_put(FIFO *fifo, int data)
{
	//首先判断缓冲区是否溢出
	if(0 == fifo->free)
	{
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buffer[fifo->next_w] = data;
	fifo->next_w++;
	fifo->free--;
	if(fifo->next_w == fifo->size)
	{
		fifo->next_w = 0;
	}
	return fifo->flags;
}

//从FIFO缓冲区中向外读取数据
int fifo_get(FIFO *fifo)
{
	int data;
	if(fifo->free == fifo->size)
	{
		return -1;
	}
	data = fifo->buffer[fifo->next_r];
	fifo->next_r++;
	fifo->free++;
	if(fifo->next_r >= fifo->size)
	{
		fifo->next_r = 0;
	}
	return data;
}

//获取FIFO缓冲区当前的积攒的数据数量
int fifo_status(FIFO *fifo)
{
	return fifo->size - fifo->free;
}













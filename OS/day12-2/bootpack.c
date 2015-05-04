//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

//�����õ���sprintf���������������Ҫ���ͷ�ļ������������������Ҫ�����κβ���ϵͳ�Ĺ��ܣ����Կ�����ôд����Ϊ���Ǳ���������������ϵͳ�����Բ�������������ϵͳ�ĺ�����
#include <stdio.h>
#include "bootpack.h"

extern FIFO keyFIFO;
extern FIFO mouseFIFO;
extern TIMERCTL timerctl;

void HariMain(void)
{
	/*
	**������ⲿ�ֱ���û��ͨ���ڴ���������䣬���Ǳ������ڲ���ϵͳ��һ���֣�������bootpack.hrb���ڵ��ǿ���ڴ�ռ���
	*/
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER_MANAGE *layer_manage;
	LAYER *layer_bg, *layer_mouse, *layer_window;
	TIMER *timer1, *timer2, *timer3;
	FIFO timerfifo1, timerfifo2, timerfifo3;
	char buf_cursor[mCursorWidth * mCursorHeight], timerbuf1[8], timerbuf2[8], timerbuf3[8];
	unsigned char *buf_bg, *buf_window;			//��Ļ�Ĵ󱳾�����init_screen��ʱ�򻭳���������ֻ��Ҫһ��ָ������ָ�뼴��
	unsigned char i, strings[40], keyBuffer[32], mouseBuffer[128];
	unsigned int memory_total, count = 0;
	int j;
	int mCursorX, mCursorY;				//�������ʾλ�õĺ�������
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//��ȥ�·��������ĸ߶�
	
	
	/*�ڴ���*/
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);		//i�ĵ�λ��MB
	
	/*�ڴ����*/
	memmanage_init(memmanage);
	memory_total = i * 1024 * 1024;
	memmanage_free_4K(memmanage, 0x00001000, 0x0009e000);
	memmanage_free_4K(memmanage, 0x00400000, memory_total - 0x00400000);
	
	/*��ʼ�������жϵĻ�����*/
	init_fifo(&keyFIFO, 32, keyBuffer);				//��ʼ��keyFIFO������
	init_fifo(&mouseFIFO, 128, mouseBuffer);		//��ʼ��mouseFIFO������
	/*��ʼ��GDT��IDT���Լ�PIC�������*/
	init_GDTandIDT();				//��ʼ��GDT��IDT��
	init_pic();						//��ʼ��PIC���Ӱ����ݣ�����IRQ2��ֹ��ȫ���ж�
	io_sti();						//��ʼ�����ж�
	/*��ʼ��PIT�жϿ���*/
	init_PIT();
		/*��Ҫ��������ж���Ҫ�������裬���ȱ���ʹ�����Ƶ�·�����Ǽ��̿��Ƶ�·��һ���֣���Ч��Ȼ��Ҫʹ��걾����Ч*/
	init_keyboard();						//��ʼ�����̿�������·
	enable_mouse(&mouse_dec);				//�������
	/*���Ÿ����ж�*/
	io_out8(PIC0_IMR, 0xf8);				//PIC0����IRQ(11111000)������IRQ0��IRQ1��IRQ2����ʱ���������жϺʹ�PIC��
	io_out8(PIC1_IMR, 0xef); 				//PIC1����IRQ(11101111)�� ��������ж�
	/*��ʼ����ɫ�壬Ϊͼ�ν�����׼��*/
	init_palette();					//��ʼ����ɫ��
	/*��ʼ��ͼ��������ҳ�ʼ�������ͱ�����ͼ��*/
	layer_manage = layer_man_init(memmanage, binfo->vram, binfo->scrnx, binfo->scrny);
	
	layer_bg = layer_alloc(layer_manage);			//Ϊ��������ͼ��
	layer_mouse = layer_alloc(layer_manage);		//Ϊ������ͼ��
	layer_window = layer_alloc(layer_manage);		//Ϊ���ڷ���ͼ��
	
	buf_bg = (unsigned char *)memmanage_alloc_4K(memmanage, binfo->scrnx * binfo->scrny);		//Ϊ����ͼ�ε����ݷ����ڴ�
	buf_window = (unsigned char *)memmanage_alloc_4K(memmanage, 160 * 52);						//Ϊ����ͼ�ε����ݷ����ڴ�
	/*Ϊ����ͼ�ε�ͼ�����ݽ����趨*/
	layer_set(layer_bg, buf_bg, binfo->scrnx, binfo->scrny, -1);			
	layer_set(layer_mouse, buf_cursor, 16, 16 ,99);
	layer_set(layer_window, buf_window, 160, 52, -1);
	/*��ʼ���������汳��*/
	init_screen(buf_bg, binfo->scrnx, binfo->scrny);				//���ʱ���init_screen������ֱ�ӻ���������������mBg�ڴ��ַ����д�ñ�������
	layer_slide(layer_bg, 0, 0);					//�ѱ���ͼ���(0,0)���꿪ʼ��
	/*��ʼ�����ͼ��*/
	init_mouse_cursor(buf_cursor, 99);								//��ʼ�������
	layer_slide(layer_mouse, mCursorX, mCursorY);	//������ʾͼ�β���Ҫ����displayShape�����ˣ�ֱ�������ͼ�����Ļ�ͼ��������
	/*��ʼ������*/
	create_window(buf_window, 160, 52, "counter");	//��������
	layer_slide(layer_window, 80, 72);				//��ָ��λ����ʾ������
	/*���úø���ͼ��ĸ߶�*/
	layer_switch(layer_bg, 0);						//�ѱ���ͼ���Ϊ��ײ㣬�߶�Ϊ0
	layer_switch(layer_window, 1);					//����ͼ�����Ϊ�ڶ��㣬�߶�Ϊ1
	layer_switch(layer_mouse, 2);					//���ͼ���Ϊ��߲㣬�߶�Ϊ2
	/*��ʱ���ĳ�ʼ����������*/
	init_fifo(&timerfifo1, 8, timerbuf1);
	timer1 = timer_alloc();
	timer_init(timer1, &timerfifo1, 1);
	timer_set(timer1, 1000);
	init_fifo(&timerfifo2, 8, timerbuf2);
	timer2 = timer_alloc();
	timer_init(timer2, &timerfifo2, 1);
	timer_set(timer2, 300);
	init_fifo(&timerfifo3, 8, timerbuf3);
	timer3 = timer_alloc();
	timer_init(timer3, &timerfifo3, 1);
	timer_set(timer3, 50);
	/*����Ļ����ʾһЩ�ڴ桢���ͼ��̵���Ϣ*/
	sprintf(strings, "Memory has %dMB", i);
	displayStrings_CS(buf_bg, binfo->scrnx, 0, 48, COL8_FFFFFF,strings);
	layer_refresh(layer_bg, 0, 48, binfo->scrnx, 80);
	sprintf(strings, "free memory:%dKB",memmanage_total(memmanage) / 1024);
	displayStrings_CS(buf_bg, binfo->scrnx, 120, 48, COL8_FFFFFF,strings);			//��������ʾ��ǰ�ڴ�����
	layer_refresh(layer_bg, 120, 48, binfo->scrnx, 80);
	
	
	for(;;)
	{
		sprintf(strings, "%10d", timerctl.count);
		drawRectangle(buf_window, 160, COL8_C6C6C6, 40, 28, 79,15);
		displayStrings_CS(buf_window, 160, 40, 28, COL8_000000,strings);
		layer_refresh(layer_window, 40, 28, 120, 44);
		
		/*ֻ���ڴ��жϷ��صĻ������ж�ȡ���ݵ�ʱ�����Ҫ��ֹ�жϣ���Ϊ������ʱ�������ж϶�û�н�ֹ�Ļ���
		**�п��ܶ������ݣ����ѻ�û�ж��������ݵĸ�Ĩ���򻻳ɱ������
		*/
		io_cli();		
		if(0 == fifo_status(&keyFIFO) + fifo_status(&mouseFIFO) + fifo_status(&timerfifo1) + fifo_status(&timerfifo2)
			 + fifo_status(&timerfifo3))		//��ǰû���жϲ���
		{
			io_sti();			//��CPUִ��hltָ��֮��ֻ���ⲿ�жϵ�֮����Ż��ٴλ���CPU��������
		}
		else
		{
			if(0 != fifo_status(&keyFIFO))
			{
				i = fifo_get(&keyFIFO);
				io_sti();
				sprintf(strings,"%2X",i);
				drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 0, 0, 16,16);		//��������岻����ж��vram�У�����д��mBg��������ڴ��У��뱳����Ϊһ��ͼ��
				displayStrings_CS(buf_bg, binfo->scrnx, 0, 0, COL8_FFFFFF,strings);
				layer_refresh(layer_bg, 0, 0, 16, 16);								//�����򱳾�ͼ����������¶�������Ҫ�ػ����ͼ��
			}
			else if(0 != fifo_status(&mouseFIFO))
			{
				i = fifo_get(&mouseFIFO);
				io_sti();
				if(0 != mouse_decode(&mouse_dec, i))		//ֻ�з���ֵΪ1��ʱ���˵���ɹ��������һ�������ж�
				{
					/*��ʾ������Ϣ*/
					sprintf(strings,"[lcr %3d,%3d]",mouse_dec.x, mouse_dec.y);
					if(0 != (mouse_dec.btn & 0x01))		//���������
					{
						strings[1] = 'L';
					}
					if(0 != (mouse_dec.btn & 0x02))		//�������Ҽ�
					{
						strings[3] = 'R';
					}
					if(0 != (mouse_dec.btn & 0x04))		//�������м�
					{
						strings[2] = 'C';
					}
					drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 0, 16, 15 * 8, 16);		//�ñ���ɫ�����ϴε�����
					displayStrings_CS(buf_bg, binfo->scrnx, 0, 16, COL8_FFFFFF,strings);		//��ʾ�µ�����
					layer_refresh(layer_bg,0, 16, 15 * 8, 32);
					/*�����ƶ�*/
					//����mouse_dec��洢�������Ϣ�����µ����ͼ��
					mCursorX += mouse_dec.x;
					mCursorY += mouse_dec.y;
					//����������Ƴ�����
					if(mCursorX < 0)
					{
						mCursorX = 0;
					}
					if(mCursorY < 0)
					{
						mCursorY = 0;
					}
					if(mCursorX > binfo->scrnx - 1)
					{
						mCursorX = binfo->scrnx - 1;
					}
					if(mCursorY > binfo->scrny - 1)
					{
						mCursorY = binfo->scrny - 1;
					}
					sprintf(strings, "(%3d,%3d)", mCursorX, mCursorY);
					drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 40, 32, 20 * 8,16);		//�ñ���ɫ�����ϴε�����
					displayStrings_CS(buf_bg, binfo->scrnx, 40, 32, COL8_FFFFFF,strings);		//��ʾ�µ�����
					layer_refresh(layer_bg, 40, 32, 20 * 8,48);
					layer_slide(layer_mouse, mCursorX, mCursorY);
				}
			}
			else if(0 != fifo_status(&timerfifo1))			//10�붨ʱ������ʱ����������Ļ����ʾ"10[sec]"
			{
				i = fifo_get(&timerfifo1);
				io_sti();
				displayStrings_CS(buf_bg, binfo->scrnx, 0, 64, COL8_FFFFFF,"10[sec]");
				layer_refresh(layer_bg, 0, 64, 7 * 8, 80);
			}
			else if(0 != fifo_status(&timerfifo2))			//3�붨ʱ������ʱ����������Ļ����ʾ"3[sec]"
			{
				i = fifo_get(&timerfifo2);
				io_sti();
				displayStrings_CS(buf_bg, binfo->scrnx, 0, 80, COL8_FFFFFF,"3[sec]");
				layer_refresh(layer_bg, 0, 80, 6 * 8, 96);
			}
			else if(0 != fifo_status(&timerfifo3))			//����0.5�붨ʱ������ʱ���������ԣ�����Ļ����ʾ��˸���
			{
				i = fifo_get(&timerfifo3);
				io_sti();
				if(0 != i)				//���timer������Ϊ1��ʱ����ʾ���
				{
					timer_init(timer3, &timerfifo3, 0);
					drawRectangle(buf_bg, binfo->scrnx, COL8_FFFFFF, 8, 96, 3, 16);
				}
				else					//���timer������Ϊ0��ʱ����ʾ���
				{
					timer_init(timer3, &timerfifo3, 1);
					drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 8, 96, 3, 16);
				}
				timer_set(timer3, 50);
				layer_refresh(layer_bg, 8, 96, 11, 112);
			}
		}
	}	
}






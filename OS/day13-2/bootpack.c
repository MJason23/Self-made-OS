//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

//�����õ���sprintf���������������Ҫ���ͷ�ļ������������������Ҫ�����κβ���ϵͳ�Ĺ��ܣ����Կ�����ôд����Ϊ���Ǳ���������������ϵͳ�����Բ�������������ϵͳ�ĺ�����
#include <stdio.h>
#include "bootpack.h"

extern FIFO *keyFIFO;
extern FIFO *mouseFIFO;
extern TIMERCTL timerctl;
extern int keyboard_offset;
extern int mouse_offset;

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
	FIFO fifo_mutual;
	int fifobuf[128];
	char buf_cursor[mCursorWidth * mCursorHeight];
	unsigned char *buf_bg, *buf_window;			//��Ļ�Ĵ󱳾�����init_screen��ʱ�򻭳���������ֻ��Ҫһ��ָ������ָ�뼴��
	unsigned char strings[40];
	unsigned int memory_total, count = 0, i;
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
	init_fifo(&fifo_mutual, 128, fifobuf);		//��ʼ��mouseFIFO������
	/*��ʼ��GDT��IDT���Լ�PIC�������*/
	init_GDTandIDT();				//��ʼ��GDT��IDT��
	init_pic();						//��ʼ��PIC���Ӱ����ݣ�����IRQ2��ֹ��ȫ���ж�
	io_sti();						//��ʼ�����ж�
	/*��ʼ��PIT�жϿ���*/
	init_PIT();
		/*��Ҫ��������ж���Ҫ�������裬���ȱ���ʹ�����Ƶ�·�����Ǽ��̿��Ƶ�·��һ���֣���Ч��Ȼ��Ҫʹ��걾����Ч*/
	init_keyboard(&fifo_mutual, 256);						//��ʼ�����̿�������·
	enable_mouse(&fifo_mutual, 512, &mouse_dec);				//�������
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
	timer1 = timer_alloc();
	timer_init(timer1, &fifo_mutual, 10);
	timer_set(timer1, 1000);
	
	timer2 = timer_alloc();
	timer_init(timer2, &fifo_mutual, 3);
	timer_set(timer2, 300);
	
	timer3 = timer_alloc();
	timer_init(timer3, &fifo_mutual, 1);
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
		count++;
		//sprintf(strings, "%10d", timerctl.count);
		//displayStrings_atLayer(layer_window, 40, 28, COL8_000000, COL8_C6C6C6, strings);
		
		/*ֻ���ڴ��жϷ��صĻ������ж�ȡ���ݵ�ʱ�����Ҫ��ֹ�жϣ���Ϊ������ʱ�������ж϶�û�н�ֹ�Ļ���
		**�п��ܶ������ݣ����ѻ�û�ж��������ݵĸ�Ĩ���򻻳ɱ������
		*/
		io_cli();		
		if(0 == fifo_status(&fifo_mutual))		//��ǰû���жϲ���
		{
			io_sti();			//��CPUִ��hltָ��֮��ֻ���ⲿ�жϵ�֮����Ż��ٴλ���CPU��������
		}
		else
		{
			i = fifo_get(&fifo_mutual);
			io_sti();
			if((256 <= i) && (i <= 511))		//��������
			{
				sprintf(strings, "%2X", i - 256);
				displayStrings_atLayer(layer_bg, 0, 0, COL8_FFFFFF, COL8_008484, strings);								//�����򱳾�ͼ����������¶�������Ҫ�ػ����ͼ��
			}
			else if((512 <= i) && (i <= 767))		//�������
			{
				if(0 != mouse_decode(&mouse_dec, i - 512))		//ֻ�з���ֵΪ1��ʱ���˵���ɹ��������һ�������ж�
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
					displayStrings_atLayer(layer_bg, 0, 16, COL8_FFFFFF, COL8_008484, strings);	
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
					displayStrings_atLayer(layer_bg, 40, 32, COL8_FFFFFF, COL8_008484, strings);
					layer_slide(layer_mouse, mCursorX, mCursorY);
				}
			}
			else if(10 == i)
			{	
				displayStrings_atLayer(layer_bg, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]");
				sprintf(strings, "%010d", count);
				displayStrings_atLayer(layer_window, 40, 28, COL8_000000, COL8_C6C6C6, strings);
				count = 0;
				timer_set(timer1, 700);
			}
			else if(3 == i)
			{
				count = 0;			//Ϊ�˷�ֹ���ڳ�ʼ���Ĳ���ܴ��Ӱ�죬�ڿ��������������¿�ʼ��ʱ
				displayStrings_atLayer(layer_bg, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]");
			}
			else if(1 == i || 0 == i)
			{
				if(0 != i)				//���timer������Ϊ1��ʱ����ʾ���
				{
					timer_init(timer3, &fifo_mutual, 0);
					drawRectangle(buf_bg, binfo->scrnx, COL8_FFFFFF, 8, 96, 2, 16);
				}
				else					//���timer������Ϊ0��ʱ����ʾ���
				{
					timer_init(timer3, &fifo_mutual, 1);
					drawRectangle(buf_bg, binfo->scrnx, COL8_008484, 8, 96, 2, 16);
				}
				timer_set(timer3, 50);
				layer_refresh(layer_bg, 8, 96, 10, 112);
			}
		}
	}	
}






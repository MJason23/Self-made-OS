//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

//�����õ���sprintf���������������Ҫ���ͷ�ļ������������������Ҫ�����κβ���ϵͳ�Ĺ��ܣ����Կ�����ôд����Ϊ���Ǳ���������������ϵͳ�����Բ�������������ϵͳ�ĺ�����
#include <stdio.h>
#include "bootpack.h"

extern FIFO keyFIFO;
extern FIFO mouseFIFO;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	//char string[40];
	char mCursor[mCursorWidth * mCursorHeight];
	unsigned char strings[4], keyBuffer[32], mouseBuffer[128],i;
	int j;
	int mCursorX, mCursorY;				//�������ʾλ�õĺ�������
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//��ȥ�·��������ĸ߶�
	
	
	
	init_fifo(&keyFIFO, 32, keyBuffer);				//��ʼ��FIFO������
	init_fifo(&mouseFIFO, 128, mouseBuffer);		//��ʼ��FIFO������
	
	init_GDTandIDT();				//��ʼ��GDT��IDT��
	init_pic();						//��ʼ��PIC���Ӱ�����
	io_sti();						//��ʼ�����ж�
	
	init_palette();					//��ʼ����ɫ��
	
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);				//����һ��windows����
	
	/*��Ҫ��������ж���Ҫ�������裬���ȱ���ʹ�����Ƶ�·�����Ǽ��̿��Ƶ�·��һ���֣���Ч��Ȼ��Ҫʹ��걾����Ч*/
	init_keyboard();				//��ʼ�����̿�������·
	enable_mouse();					//�������
	
	init_mouse_cursor(mCursor, COL8_008484);							//��ʼ�������
	displayShape(binfo->vram, binfo->scrnx, mCursorWidth, mCursorHeight, mCursorX, mCursorY, mCursor, mCursorWidth);		//��ʾ�����
	
	io_out8(PIC0_IMR, 0xf9); 						/* PIC0����IRQ(11111001) */
	io_out8(PIC1_IMR, 0xef); 						/* PIC1����IRQ(11101111) */
	
	for(;;)
	{
		io_cli();
		if(0 == fifo_status(&keyFIFO) + fifo_status(&mouseFIFO))		//��ǰû���жϲ���
		{
			io_stihlt();			//��CPUִ��hltָ��֮��ֻ���ⲿ�жϵ�֮����Ż��ٴλ���CPU��������
		}
		else
		{
			if(0 != fifo_status(&keyFIFO))
			{
				i = fifo_get(&keyFIFO);
				io_sti();
				sprintf(strings,"%2X",i);
				drawRectangle(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 15,31);
				displayStrings_CS(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF,strings);
			}
			else if(0 != fifo_status(&mouseFIFO))
			{
				i = fifo_get(&mouseFIFO);
				io_sti();
				sprintf(strings,"%2X",i);
				drawRectangle(binfo->vram, binfo->scrnx, COL8_008484, 0, 50, 15,31);
				displayStrings_CS(binfo->vram, binfo->scrnx, 0, 50, COL8_FFFFFF,strings);
			}
		}
	}	
}






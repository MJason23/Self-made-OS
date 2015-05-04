//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

//�����õ���sprintf���������������Ҫ���ͷ�ļ������������������Ҫ�����κβ���ϵͳ�Ĺ��ܣ����Կ�����ôд����Ϊ���Ǳ���������������ϵͳ�����Բ�������������ϵͳ�ĺ�����
#include <stdio.h>
#include "bootpack.h"

extern KEYBUF keybuf;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	//char string[40];
	char mCursor[mCursorWidth * mCursorHeight];
	unsigned char strings[4];
	unsigned char i;
	int j;
	int mCursorX, mCursorY;				//�������ʾλ�õĺ�������
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//��ȥ�·��������ĸ߶�
	
	init_GDTandIDT();
	init_pic();
	io_sti();
	
	init_palette();
	
	//����һ��windows����
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//��ʾһ���ַ���
	displayStrings_CS(binfo->vram, binfo->scrnx, 0, 8, COL8_FFFFFF, "This is the first time that I made an OS myself!!");
	//sprintf(string, "scrnx = %d", binfo->scrnx);
	//displayStrings_CS(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, string);
	init_mouse_cursor(mCursor, COL8_008484);
	displayShape(binfo->vram, binfo->scrnx, mCursorWidth, mCursorHeight, mCursorX, mCursorY, mCursor, mCursorWidth);
	
	io_out8(PIC0_IMR, 0xf9); /* PIC0����IRQ(11111001) */
	io_out8(PIC1_IMR, 0xef); /* PIC1����IRQ(11101111) */
	
	for(;;)
	{
		io_cli();
		if(0 == keybuf.length)		//��ǰû���жϲ���
		{
			io_stihlt();			//��CPUִ��hltָ��֮��ֻ���ⲿ�жϵ�֮����Ż��ٴλ���CPU��������
		}
		else
		{
			i = keybuf.data[keybuf.next_r];
			keybuf.next_r++;
			keybuf.length--;
			if(32 == keybuf.next_r)
			{
				keybuf.next_r = 0;
			}
			sprintf(strings,"%2X",i);
			drawRectangle(binfo->vram, binfo->scrnx, COL8_008484, 0, 50, 15,31);
			displayStrings_CS(binfo->vram, binfo->scrnx, 0, 50, COL8_FFFFFF,strings);
		}
	}	
}






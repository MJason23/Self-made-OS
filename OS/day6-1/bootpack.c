//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

//�����õ���sprintf���������������Ҫ���ͷ�ļ������������������Ҫ�����κβ���ϵͳ�Ĺ��ܣ����Կ�����ôд����Ϊ���Ǳ���������������ϵͳ�����Բ�������������ϵͳ�ĺ�����
#include <stdio.h>
#include "bootpack.h"


void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *)0x0ff0;
	//char string[40];
	char mCursor[mCursorWidth * mCursorHeight];
	int mCursorX, mCursorY;				//�������ʾλ�õĺ�������
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//��ȥ�·��������ĸ߶�
	
	init_GDTandIDT();
	init_palette();
	
	//����һ��windows����
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//��ʾһ���ַ���
	displayStrings_CS(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "This is the first time that I made an OS myself!!");
	//sprintf(string, "scrnx = %d", binfo->scrnx);
	//displayStrings_CS(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, string);
	init_mouse_cursor(mCursor, COL8_008484);
	displayShape(binfo->vram, binfo->scrnx, mCursorWidth, mCursorHeight, mCursorX, mCursorY, mCursor, mCursorWidth);
	
	for(;;)
	{
		io_hlt();						//ִ���������ϵͳ֮�����CPUͣ����
	}	
}






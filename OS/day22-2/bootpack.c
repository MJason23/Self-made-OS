//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

//�����õ���sprintf���������������Ҫ���ͷ�ļ������������������Ҫ�����κβ���ϵͳ�Ĺ��ܣ����Կ�����ôд����Ϊ���Ǳ���������������ϵͳ�����Բ�������������ϵͳ�ĺ�����
#include <stdio.h>
#include <string.h>
#include "bootpack.h"

extern TIMERCTL timerctl;
extern TIMER *task_timer;

void HariMain(void)
{
	/*
	**������ⲿ�ֱ���û��ͨ���ڴ���������䣬���Ǳ������ڲ���ϵͳ��һ���֣�������bootpack.hrb���ڵ��ǿ���ڴ�ռ���
	*/
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER_MANAGE *layer_manage;
	LAYER *layer_bg, *layer_mouse, *layer_window, *layer_console;
	TIMER *timer;
	FIFO fifo_mutual;									//���������������жϹ��õ�
	FIFO keycmd_fifo;									
	TASK *task_a, *task_console;
	CONSOLE *console;
	int fifobuf[128];									//������
	int keycmd_buf[32];
	int cursor_x = 8, cursor_color = COL8_000000;		//�ֱ���������ַ�����Ǹ���˸���ĺ��������ɫ
	int mCursorX, mCursorY;				//�������ʾλ�õĺ�������
	int key_to = 0, key_shift = 0, key_ctrl = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;			//��־�������뵽�ĸ�������
	unsigned int memory_total, i;
	char buf_cursor[mCursorWidth * mCursorHeight];
	unsigned char *buf_bg, *buf_window, *buf_console;			//��Ļ�Ĵ󱳾�����init_screen��ʱ�򻭳���������ֻ��Ҫһ��ָ������ָ�뼴��
	unsigned char strings[40];
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//��ȥ�·��������ĸ߶�
	
	
	/*�ڴ���*/
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);		//i�ĵ�λ��MB
	
	/*�ڴ����*/
	memmanage_init(memmanage);
	memory_total = i * 1024 * 1024;
	memmanage_free_4K(memmanage, 0x00001000, 0x0009e000);
	memmanage_free_4K(memmanage, 0x00400000, memory_total - 0x00400000);
	
	init_fifo(&keycmd_fifo, 32, keycmd_buf, 0);
	/*��ʼ�������жϵĻ�����*/
	init_fifo(&fifo_mutual, 128, fifobuf, 0);		//��ʼ��mouseFIFO�����������ڻ�û������ָ��Ϊ0
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
	/*��ʼ�������л�����*/
	task_a = task_init(memmanage);		//���task_a��ʵ����ľ�����Щ�����̵ȵ�����
	fifo_mutual.task = task_a;						//Ϊ�����̵ȵĻ�����ָ����������Ϊtask_a
	task_run(task_a, 1, 2);
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
	create_window(buf_window, 160, 52, "task_a", ACTIVE);					//��������
	layer_slide(layer_window, 80, 100);								//��ָ��λ����ʾ������
	create_textbox(layer_window, 8, 28, 144, 16, COL8_FFFFFF);		//����������д���һ�������
	/*ִ��һ�������д�������*/
	layer_console = layer_alloc(layer_manage);
	buf_console = (unsigned char *)memmanage_alloc_4K(memmanage, 256 * 165);	
	layer_set(layer_console, buf_console, 256, 165, -1);
	create_window(buf_console, 256, 165, "console", INACTIVE);
	create_textbox(layer_console, 8, 28, 240, 128, COL8_000000);
	task_console = task_alloc();
	task_console->tss.esp = memmanage_alloc_4K(memmanage, 64 * 1024) + 64 * 1024 - 12;		//��������Ҫ�����������������Լ�ȥ��12
	task_console->tss.eip = (int) &console_task;
	task_console->tss.es = 1 * 8;
	task_console->tss.cs = 2 * 8;
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
	*((int *) (task_console->tss.esp + 4)) = (int) layer_console;
	*((int *) (task_console->tss.esp + 8)) = memory_total;	
	task_run(task_console, 2, 2); /* level=2, priority=2 */
	
	
	layer_slide(layer_console, 32, 60);
	/*���úø���ͼ��ĸ߶�*/
	layer_switch(layer_bg, 0);						//�ѱ���ͼ���Ϊ��ײ㣬�߶�Ϊ0
	layer_switch(layer_console, 1);					//�����д���ͼ�����Ϊ�����㣬�߶�Ϊ2
	layer_switch(layer_window, 2);					//����ͼ�����Ϊ�ڶ��㣬�߶�Ϊ1
	layer_switch(layer_mouse, 3);					//���ͼ���Ϊ��߲㣬�߶�Ϊ3
	/*��ʱ���ĳ�ʼ����������*/
	timer = timer_alloc();
	timer_init(timer, &fifo_mutual, 1);
	timer_set(timer, 50);
	
	
	
	//
	fifo_put(&keycmd_fifo, KEYCMD_LED);
	fifo_put(&keycmd_fifo, key_leds);
	
	for(;;)
	{ 
		if(fifo_status(&keycmd_fifo) > 0 && keycmd_wait < 0)
		{
			keycmd_wait = fifo_get(&keycmd_fifo);
			wait_KBC_sendready();
			io_out8(PORT_KEYDATA, keycmd_wait);
		}
		/*ֻ���ڴ��жϷ��صĻ������ж�ȡ���ݵ�ʱ�����Ҫ��ֹ�жϣ���Ϊ������ʱ�������ж϶�û�н�ֹ�Ļ���
		**�п��ܶ������ݣ����ѻ�û�ж��������ݵĸ�Ĩ���򻻳ɱ������
		*/
		io_cli();		
		if(0 == fifo_status(&fifo_mutual))		//��ǰû���жϲ���
		{
			task_sleep(task_a);	//���û����Ҫ��������ݣ����Լ����Լ�����
			/*������Ǹ�����ֱ�Ӿ���ת����һ��������ȥ�ˣ���������Ϊÿ�������Ĭ��eflags���ǿ����жϵģ�
			**���Բ��õ��ģ�����������ܱ����ѵģ������Ѻ�ĵ�һ����������Ϊ���Է���һ�ȿ����ж�
			*/
			io_sti();		//��һֻ���Լ������еĻ������޷�˯�ߣ���ô��ִ��hltָ����ˣ�����ʡ����	
		}
		else
		{
			i = fifo_get(&fifo_mutual);
			io_sti();
			//����Ǽ����жϵĻ�����Ҫ������̷��������ж�����
			if((keyboard_offset <= i) && (i <= keyboard_offset + 255))			
			{
				//�ж�shift���Ƿ��£�Ȼ��ʹ�ò�ͬ��keytable��
				if(i < 0x80 + keyboard_offset)
				{
					if(0 == key_shift)
					{
						strings[0] = key_table[i - keyboard_offset];
					}
					else
					{
						strings[0] = key_table_shift[i - keyboard_offset];
					}
				}
				else
				{
					strings[0] = 0;
				}
				//���ͬʱ������ctrl+x����ô�͹ر�Ӧ�ó���
				if(('x' == strings[0] || 'X' == strings[0]) && (0 != key_ctrl) && task_console->tss.ss0 != 0)
				{
					console = (CONSOLE *) *((int *) 0x0fec);
					console_putstring_toend(console, "\nTerminate program! :\n");
					io_cli();
					task_console->tss.eax = (int) &(task_console->tss.esp0);
					task_console->tss.eip = (int) asm_end_app;
					io_sti();
				}
				
				//�ж�Caps��Shift���������ȷ�������д����Сд��ĸ
				if('A' <= strings[0] && strings[0] <= 'Z')
				{
					//Сд��ĸ�����
					if((0 == (key_leds & 4) && 0 == key_shift) || (0 != (key_leds & 4) && 0 != key_shift))
					{
						strings[0] += 0x20;
					}
				}
				//�жϰ��µ��ַ��Ƿ�Ϊһ���ַ���0��Ӧ���ַ�������
				if(strings[0] != 0)				
				{
					//���͸�task_a����Ĵ���
					if(0 == key_to)
					{
						//���пɴ�ӡ���ַ�����Ҫ��ӡ����
						if(cursor_x < 144)
						{
							strings[1] = 0;
							displayStrings_atLayer(layer_window, cursor_x, 28, COL8_000000, COL8_FFFFFF, strings);
							cursor_x += 8;			//����λ������ÿһ���ַ�����������ƶ�һ���ַ���ȵ�λ�ü�8����
						}
					}
					//���͸������д���
					else
					{
						fifo_put(&task_console->fifo, strings[0] + 256);
					}
				}
				/*��Щ������Щ���ܴ�ӡ�����ⰴ�������Ƕ�Ӧ�Ķ���0*/
				//�����ˡ��˸������������˸���ĺ�����0x0e
				if((keyboard_offset + 0x0e == i))				
				{
					if(0 == key_to)
					{
						//ֻ�е�ǰ������λ�������ַ��˲�����ǰɾ��
						if(cursor_x > 8)
						{
							cursor_x -= 8;
							/*�����˸��Ӧ��ֻ���һ���ո���еģ����������Ļ�û�취ˢ�¹�����ڵ��ǿ�������ˣ�
							**�����¹��ĺ�ɫ�ۼ�������ֱ����������ո�ˢ��֮��Ͳ����кۼ���
							*/
							displayStrings_atLayer(layer_window, cursor_x, 28, COL8_000000, COL8_FFFFFF, "  ");
						}
					}
					else
					{
						fifo_put(&task_console->fifo, 8 + keyboard_offset);
					}
				}
				//���ڹ����������˻�����ɾ���ַ���ǰ���ˣ�Ϊ�˼�ʱ��ʾ������Ҫ�����ػ���
				if(cursor_color > 0) 		//���������ɫֵ�������0��˵����ǰ��Ҫtask_a����ʾ���
				{
					drawRectangle(layer_window->buffer, layer_window->length, cursor_color, cursor_x, 28, 2, 15);
				}
				layer_part_refresh(layer_window, cursor_x, 28, cursor_x + 8, 44);
				//���¡�TAB�����Ĵ���������л����봰��
				if(keyboard_offset + 0x0f == i)					
				{
					if(0 == key_to)			//��ǰ��ǰ�洰��Ϊtask_a������console���ڳ�Ϊ��ǰ�˴���
					{
						key_to = 1;
						create_titlebar(buf_window, layer_window->length, "task_a", INACTIVE);
						create_titlebar(buf_console, layer_console->length, "console", ACTIVE);
						cursor_color = -1;		//task_a����ʾ���
						drawRectangle(layer_window->buffer, layer_window->length, COL8_FFFFFF, cursor_x, 28, 2, 15);
						fifo_put(&task_console->fifo, 2);		//���������д�������2ʱ��ʾ�����д��ڹ����ʾ
					}
					else					//task_a����Ĵ��ڳ�Ϊ��ǰ�˴���
					{
						key_to = 0;
						create_titlebar(buf_window, layer_window->length, "task_a", ACTIVE);
						create_titlebar(buf_console, layer_console->length, "console", INACTIVE);
						cursor_color = COL8_000000;
						fifo_put(&task_console->fifo, 3);		//���������д�������3ʱ��ʾ�����д��ڹ�겻��ʾ
					}
					layer_part_refresh(layer_window, 0, 0, layer_window->length, 21);
					layer_part_refresh(layer_console, 0, 0, layer_console->length, 21);
				}
				//���¡��س������Ĵ������
				if(keyboard_offset + 0x1c == i)
				{
					//�����������д���
					if(0 != key_to)
					{
						fifo_put(&task_console->fifo, 10 + keyboard_offset);		//����10�������˻س���
					}
				}
				//�����Ctrl ON
				if(0x1d + keyboard_offset == i)
				{
					key_ctrl = 1;
				}
				//�����Ctrl OFF
				if(0x9d + keyboard_offset == i)
				{
					key_ctrl = 0;
				}
				//��shift ON
				if(0x2a + keyboard_offset == i)
				{
					key_shift |= 1;
				}
				//��shift ON
				if(0x36 + keyboard_offset == i)
				{
					key_shift |= 2;
				}
				//��shift OFF
				if(0xaa + keyboard_offset == i)
				{
					key_shift &= ~1;
				}
				//��shift OFF
				if(0xb6 + keyboard_offset == i)
				{
					key_shift &= ~2;
				}
				/*�Ը����������Ĵ���*/
				//CapsLock��
				if(i == keyboard_offset + 0x3a)
				{
					key_leds ^= 4;
					fifo_put(&keycmd_fifo, KEYCMD_LED);
					fifo_put(&keycmd_fifo, key_leds);
				}
				//NumLock��
				if(i == keyboard_offset + 0x45)
				{
					key_leds ^= 2;
					fifo_put(&keycmd_fifo, KEYCMD_LED);
					fifo_put(&keycmd_fifo, key_leds);
				}
				//ScrollLock��
				if(i == keyboard_offset + 0x46)
				{
					key_leds ^= 1;
					fifo_put(&keycmd_fifo, KEYCMD_LED);
					fifo_put(&keycmd_fifo, key_leds);
				}
				//���̳ɹ����յ�����
				if(i == keyboard_offset + 0xfa)
				{
					keycmd_wait = -1;
				}
				//����û�гɹ����յ�����
				if(i == keyboard_offset + 0xfe)
				{
					wait_KBC_sendready();
					io_out8(PORT_KEYDATA, keycmd_wait);
				}
			}
			//���������жϵĻ�����Ҫ������귢�������ж�����
			else if((mouse_offset <= i) && (i <= mouse_offset + 255))		
			{
				if(0 != mouse_decode(&mouse_dec, i - mouse_offset))		//ֻ�з���ֵΪ1��ʱ���˵���ɹ��������һ�������ж�
				{
					if(0 != (mouse_dec.btn & 0x01))		//���������
					{
						layer_slide(layer_window, mCursorX - 80, mCursorY -8);		//���д������ƶ����ڵ�
					}
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
					layer_slide(layer_mouse, mCursorX, mCursorY);
				}
			}
			//����Ƕ�ʱ���жϵĻ�����Ҫ����ʱ����Ӧ������
			else if(1 == i || 0 == i)				
			{
				if(0 != i)				//���timer������Ϊ1��ʱ����ʾ���
				{
					timer_init(timer, &fifo_mutual, 0);
					if(cursor_color >= 0)			//��Ҫ��ǰ��ʾ����ʱ�򣬲Ŷ������ɫֵ���и�ֵ
					{
						cursor_color = COL8_000000;
					}
				}
				else					//���timer������Ϊ0��ʱ����ʾ���
				{
					timer_init(timer, &fifo_mutual, 1);
					if(cursor_color >= 0)
					{
						cursor_color = COL8_FFFFFF;
					}
				}
				timer_set(timer, 50);
				if(cursor_color >= 0)
				{
					drawRectangle(layer_window->buffer, layer_window->length, cursor_color, cursor_x, 28, 2, 15);
					layer_part_refresh(layer_window, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}	
}








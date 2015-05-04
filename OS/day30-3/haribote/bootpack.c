//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

//�����õ���sprintf���������������Ҫ���ͷ�ļ������������������Ҫ�����κβ���ϵͳ�Ĺ��ܣ����Կ�����ôд����Ϊ���Ǳ���������������ϵͳ�����Բ�������������ϵͳ�ĺ�����
#include <stdio.h>
#include <string.h>
#include "bootpack.h"

extern TIMERCTL timerctl;
extern TIMER *task_timer;
extern TASKCTL *taskctl;

void keywindow_on(LAYER *key_to_window);
void keywindow_off(LAYER *key_to_window);
void close_console_task(TASK *task);
void close_console(LAYER *layer);

void HariMain(void)
{
	/*
	**������ⲿ�ֱ���û��ͨ���ڴ���������䣬���Ǳ������ڲ���ϵͳ��һ���֣�������bootpack.hrb���ڵ��ǿ���ڴ�ռ���
	*/
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	MOUSE_DECODE mouse_dec;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER_MANAGE *layer_manage;
	LAYER *layer_bg, *layer_mouse;
	FIFO fifo_mutual;									//���������������жϹ��õ�
	FIFO keycmd_fifo;									
	TASK *task_a, *task;				//������Ǹ�taskֻ����Ϊһ����ʱ�������õ�
	int fifobuf[128], keycmd_buf[32], *console_fifo[2];							//������
	int mCursorX, mCursorY;				//�������ʾλ�õĺ�������
	int key_shift = 0, key_ctrl = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;			//��־�������뵽�ĸ�������
	int j, x, y, last_mcursorX = -1, last_mcursorY= -1, tmp_layerX = 0;			//last_mcursorX������������¼��������ƶ�֮ǰ������
	int new_mcursorX = -1, new_mcursorY = 0, new_layerX = 0x7fffffff, new_layerY = 0;
	LAYER *layer = 0, *key_to_window, *layer_to_free;							//key_to_window���������¼��ǰ�ļ������뵽�ĸ�������
	unsigned int memory_total, i;
	unsigned char buf_cursor[mCursorWidth * mCursorHeight];
	unsigned char *buf_bg;		//��Ļ�Ĵ󱳾�����init_screen��ʱ�򻭳���������ֻ��Ҫһ��ָ������ָ�뼴��
	char strings[40];
	int *fat;
	unsigned char *hzset;
	FILEINFO *finfo;
	extern char charset[4096];
	
	mCursorX = binfo->scrnx / 2;			
	mCursorY = (binfo->scrny - 28)/ 2;		//��ȥ�·��������ĸ߶�
	
	
	/*��ʼ��GDT��IDT���Լ�PIC�������*/
	init_GDTandIDT();				//��ʼ��GDT��IDT��
	init_pic();						//��ʼ��PIC���Ӱ����ݣ�����IRQ2��ֹ��ȫ���ж�
	io_sti();						//��ʼ�����ж�
	
	init_fifo(&keycmd_fifo, 32, keycmd_buf, 0);
	/*��ʼ�������жϵĻ�����*/
	init_fifo(&fifo_mutual, 128, fifobuf, 0);				//��ʼ��mouseFIFO�����������ڻ�û������ָ��Ϊ0
	
	/*��ʼ��PIT�жϿ���*/
	init_PIT();
		/*��Ҫ��������ж���Ҫ�������裬���ȱ���ʹ�����Ƶ�·�����Ǽ��̿��Ƶ�·��һ���֣���Ч��Ȼ��Ҫʹ��걾����Ч*/
	init_keyboard(&fifo_mutual, 256);						//��ʼ�����̿�������·
	enable_mouse(&fifo_mutual, 512, &mouse_dec);			//�������
	
	/*���Ÿ����ж�*/
	io_out8(PIC0_IMR, 0xf8);			//PIC0����IRQ(11111000)������IRQ0��IRQ1��IRQ2����ʱ���������жϺʹ�PIC��
	io_out8(PIC1_IMR, 0xef); 			//PIC1����IRQ(11101111)�� ��������ж�
	
	/*�ڴ���*/
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);	//i�ĵ�λ��MB
	
	/*�ڴ����*/
	memmanage_init(memmanage);
	memory_total = i * 1024 * 1024;
	memmanage_free_4K(memmanage, 0x00001000, 0x0009e000);
	memmanage_free_4K(memmanage, 0x00400000, memory_total - 0x00400000);
	/*��ʼ����ɫ�壬Ϊͼ�ν�����׼��*/
	init_palette();						//��ʼ����ɫ��
	
	/*��ʼ��ͼ��������ҳ�ʼ�������ͱ�����ͼ��*/
	layer_manage = layer_man_init(memmanage, binfo->vram, binfo->scrnx, binfo->scrny);
	*((int *) 0x0fe4) = (int) layer_manage;
	
	/*��ʼ�������л�����*/
	task_a = task_init(memmanage);		//���task_a��ʵ����ľ�����Щ�����̵ȵ�����
	fifo_mutual.task = task_a;			//Ϊ�����̵ȵĻ�����ָ����������Ϊtask_a
	task_run(task_a, 1, 2);
	task_a->lanmode = 0;					//Ĭ�ϵ�����ģʽ��Ӣ��ģʽ
	
	layer_bg = layer_alloc(layer_manage);			//Ϊ��������ͼ��
	layer_mouse = layer_alloc(layer_manage);		//Ϊ������ͼ��
	
	buf_bg = (unsigned char *)memmanage_alloc_4K(memmanage, binfo->scrnx * binfo->scrny);		//Ϊ����ͼ�ε����ݷ����ڴ�
	
	/*Ϊ����ͼ�ε�ͼ�����ݽ����趨*/
	layer_set(layer_bg, buf_bg, binfo->scrnx, binfo->scrny, -1);			
	layer_set(layer_mouse, buf_cursor, 16, 16 ,99);
	
	/*��ʼ���������汳��*/
	init_screen(buf_bg, binfo->scrnx, binfo->scrny);				//���ʱ���init_screen������ֱ�ӻ���������������mBg�ڴ��ַ����д�ñ�������
	layer_slide(layer_bg, 0, 0);									//�ѱ���ͼ���(0,0)���꿪ʼ��
	
	/*��ʼ�����ͼ��*/
	init_mouse_cursor(buf_cursor, 99);								//��ʼ�������
	layer_slide(layer_mouse, mCursorX, mCursorY);					//������ʾͼ�β���Ҫ����displayShape�����ˣ�ֱ�������ͼ�����Ļ�ͼ��������

	/*���뺺���ַ���*/
	fat = (int *)memmanage_alloc_4K(memmanage, 4 * 2880);
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));
	finfo = file_search("hzset.fnt", (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	//�ҵ�����ַ����ļ��Ļ����Ͱ��ַ�������
	if(0 != finfo)
	{
		i = finfo->size;				//Ϊ�˷�ֹfinfo->size��ֵ���޸ģ�����i���м����
		hzset = file_load_tekfile(finfo->clusterno, &i, fat);		//�����ڲ����Ѿ���Ҫ��ѹ���ļ���������ڴ�
	}
	//�Ҳ����ַ����ļ��Ļ����Ͱ�����Ӣ���ַ������룬������ַ�ȫ���÷��飨0xff�����
	else
	{
		hzset = (unsigned char *)memmanage_alloc_4K(memmanage, 16 * 256 + 32 * 5170);		//��256�����Ӣ���ַ���5170��ȫ�������ַ�
		for(i = 0; i < 16 * 256; i++)
		{
			hzset[i] = charset[i];
		}
		for(i = 16 * 256; i < 16 * 256 + 32 * 5170; i++)
		{
			hzset[i] = 0xff;
		}
	}
	*((int *) 0x0fe8) = (int)hzset;
	memmanage_free_4K(memmanage,(int)fat, 4 * 2880);
	
	/*ִ��һ�������д�������*/
	key_to_window = open_console(layer_manage, memory_total);		//�����ʱ���ɵ�һ��console���ռ�������
	
	layer_slide(key_to_window, 8, 2);
	/*���úø���ͼ��ĸ߶�*/
	layer_switch(layer_bg, 0);						//�ѱ���ͼ���Ϊ��ײ㣬�߶�Ϊ0
	layer_switch(key_to_window, 1);					//�����д���ͼ�����Ϊ�����㣬�߶�Ϊ2
	layer_switch(layer_mouse, 2);					//���ͼ���Ϊ��߲㣬�߶�Ϊ3	
					
	keywindow_on(key_to_window);
	
	//
	fifo_put(&keycmd_fifo, KEYCMD_LED);
	fifo_put(&keycmd_fifo, key_leds);
	
	*((int *) 0x0fec) = (int) &fifo_mutual;			
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
			//����CPU���е�ʱ����ִ�к�ʱ�Ļ�ͼ����(�����Ļ�����ɷ�Ӧ�ܿ�ı��󣬵���ʵ������жϺܶ࣬����������жϺܶ��ʱ��
			//���֮ǰ��¼�µ�new_mcursorX���ǵ�����ֻ�������µ�ͼ������)
			if(new_mcursorX >= 0)
			{
				io_sti();
				layer_slide(layer_mouse, new_mcursorX, new_mcursorY);
				new_mcursorX = -1;				//���������֮�󣬾Ͱ�������������Ϊ������������겻���ȥ��Ļ��Ϊ������ʱ���˵������Ҫ�ػ����
			}
			//ͼ��ĺ�����Ϊ0x7fffffff��ʱ��Ͳ���Ҫ�ػ��ˣ�������Ϊһ����־�ģ���Ϊ����ֵ��ʱ�����Ҫ�ػ���
			else if(new_layerX != 0x7fffffff)
			{
				io_sti();
				layer_slide(layer, new_layerX, new_layerY);
				new_layerX = 0x7fffffff;
			}
			//�������Ҫ�ػ�����ͼ��Ļ�����û��ʲô���������ˣ��������߾Ϳ�����
			else
			{
				task_sleep(task_a);	//���û����Ҫ��������ݣ����Լ����Լ�����
				/*������Ǹ�����ֱ�Ӿ���ת����һ��������ȥ�ˣ���������Ϊÿ�������Ĭ��eflags���ǿ����жϵģ�
				**���Բ��õ��ģ�����������ܱ����ѵģ������Ѻ�ĵ�һ����������Ϊ���Է���һ�ȿ����ж�
				*/
				io_sti();		//��һֻ���Լ������еĻ������޷�˯�ߣ���ô��ִ��hltָ����ˣ�����ʡ����	
			}
		}
		else
		{
			i = fifo_get(&fifo_mutual);
			io_sti();
			//���ڱ��رյ����
			if(0 != key_to_window && 0 == key_to_window->flags)
			{
				//�����ǰ������ֻʣ�����ͱ�����ʱ��
				if(1 == layer_manage->top)
				{
					key_to_window = 0;
				}
				else
				{
					key_to_window = layer_manage->layers_order[layer_manage->top - 1];			//�����ڱ��رյ�ʱ���Զ��л������ϲ��layer���ռ�������
					keywindow_on(key_to_window);
				}
			}
			/*
			**��һ����Ҫע�⣬��������ֻ����console�����գ����ֻʣ��task_a�����ˣ���ô����������ֻ���մ򿪴���console������
			**��������һ�ɲ����������������ѭ��
			*/
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
				//�ж�Caps��Shift���������ȷ�������д����Сд��ĸ
				if('A' <= strings[0] && strings[0] <= 'Z')
				{
					//Сд��ĸ�����
					if((0 == (key_leds & 4) && 0 == key_shift) || (0 != (key_leds & 4) && 0 != key_shift))
					{
						strings[0] += 0x20;
					}
				}
				//�жϰ��µ��ַ��Ƿ�Ϊһ���ַ���0��Ӧ���ַ����������˸���ͻس��������˶�Ӧ�ַ������Ƕ�Ӧ��table�����Ԫ�ز�����0
				//�м�key_to_windowҲ���ǵ�ǰ�ļ����������ͼ�㲻��ʹlayer_bg����Ϊ��������ѭ��һֱ��task_a��fifo���������ո�������ַ�
				if(strings[0] != 0 && key_to_window != 0 && key_to_window != layer_bg)				
				{
					fifo_put(&key_to_window->task->fifo, strings[0] + 256);
				}
				/*��Щ������Щ���ܴ�ӡ�����ⰴ�������Ƕ�Ӧ�Ķ���0*/
				//���¡�TAB�����Ĵ���������л����봰��
				if(i == keyboard_offset + 0x0f && key_to_window != 0 && key_to_window != layer_bg)					
				{
					//���õ�ǰ�Ĵ��ڵĹ��OFF��
					keywindow_off(key_to_window);
					j = key_to_window->height - 1;
					if(0 == j)
					{
						//����л�֮���ͼ������ײ�Ĳ���ϵͳ��ͼ�㣬��ô���л������ϲ��ͼ��
						j = layer_manage->top - 1;
					}
					key_to_window = layer_manage->layers_order[j];
					//ʹ�л����Ĵ��ڵĹ��ON
					keywindow_on(key_to_window);
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
				
				//���������F2����ô�͹ر�Ӧ�ó���
				if((i == keyboard_offset + 0x3C) && key_to_window != 0 && key_to_window != layer_bg)
				{
					task = key_to_window->task;				//�ҵ���ǰ���ռ�������ģ�����ǰ��ģ�ͼ������Ӧ������
					if(0 != task && task->tss.ss0 != 0)
					{
						console_putstring_toend(task->console, "\nTerminate program! :\n");
						io_cli();
						//��CPUֱ����ת��console��ִ�У����ٹ�Ӧ�ó���Ҳ������ν��ֹͣӦ�ó����������
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						/*��ʵ��console����û��ʲô�жϿɴ����ʱ��ͻ�������ߣ�ǿ�ƽ�����ʱ����Ҫ�Ȼ�����������ִ�н���������
						**���ǲ����ѵĻ��������Ӧ�ó����Ӧ��console��������ʾ��ʱ��Ҳ����ֹͣ��ֻ������Ҫ�ɶ�ʱ��ÿ��0.5��������һ��
						**���Կ�������Ӧ������ʱ�����Ӧʱ��Ϊ0.5��
						*/
						task_run(task, -1, 0);
					}
					
				}
				//���������F1���ʹ�һ��console
				if((i == keyboard_offset + 0x3B))
				{
					if(key_to_window != 0)
					{
						keywindow_off(key_to_window);
					}					
					key_to_window = open_console(layer_manage, memory_total);
					layer_slide(key_to_window, 300, 2);
					layer_switch(key_to_window, layer_manage->top);
					//�Զ��Ѽ��������л�������´򿪵�console��
					keywindow_on(key_to_window);
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
					//��¼���������ƶ�����λ�ã��ȴ�fifo��Ϊ�յ�ʱ��ʼ��ͼ
					new_mcursorX = mCursorX;
					new_mcursorY = mCursorY;
					//���������
					if(0 != (mouse_dec.btn & 0x01))
					{
						//���last_mcursorXС��0��˵����ǰ���Ǵ��ڴ����ƶ�ģʽ����ô����Ҫ���������갴�����֮����û������ĳ��ͼ��ı�������
						if(last_mcursorX < 0)
						{
							for(j = layer_manage->top - 1; j > 0; j--)
							{
								layer = layer_manage->layers_order[j];
								//����һ�����������ڲ������ͼ������
								x = mCursorX - layer->x;
								y = mCursorY - layer->y;
								if(0 <= x && x < layer->length && 0 <= y && y <= layer->width)
								{
									//�����������Ҳ������Ϳ���͸������
									if(layer->buffer[y * layer->length + x] != layer->color_luc)
									{
										layer_switch(layer, layer_manage->top - 1);
										//�����ǰ��layer���ǽ��ռ��������ͼ�㣨����ǰ���ͼ�㣩����ô����Ҫ�л�
										if(layer != key_to_window)
										{
											//�������������������������TAB����������
											keywindow_off(key_to_window);
											key_to_window = layer;
											keywindow_on(key_to_window);
										}
										//�鿴��굱ǰ����������ǲ��Ǵ��ڵı����������Ҳ����ǹرհ�ť���ڵ�����
										if(3 <= x && x < layer->length - 21 && 3 <= y && y <= 21)
										{
											last_mcursorX = mCursorX;
											last_mcursorY = mCursorY;
											tmp_layerX = layer->x;		//���tmp_layerXֻ�Ǹ�Ҫ���Ƶ�ͼ�������Ĺ��ɲ��֣����������껹��Ҫ����
											new_layerY = layer->y;
										}
										//�������������رհ�ť�����
										if(layer->length - 21 <= x && x <= layer->length - 5 && 5 <= y && y <= 19)
										{	
											//�鿴�ô����Ƿ�ΪӦ�ó���Ĵ��ڣ������Ӧ�ó�������Ĵ��ڶ�����console���Ǹ��ڴ��ڣ���ô������ڵ�flags��0x10
											if(0 != (layer->flags & 0x10))
											{
												//�ⲿ�ִ��������ctrl+x�Ĵ������һ��
												task = layer->task;
												console_putstring_toend(task->console, "Break by mouse!\n");
												io_cli();
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int)asm_end_app;
												io_sti();
												task_run(task, -1, 0);
											}
											//����Ӧ�ó���Ļ�����console�����д��ڲ�����layer
											else
											{
												//�����������task_a��ֱ�ӹرգ���Ϊ����Щconsole�������Ķ�ʱ����fat�ȶ����޷��ͷ���
												task = layer->task;
												/*����ֻ����ʱ����console��ͼ�㣬��Ϊ�����е�Ӧ�ó���������д��ڹر���Ҫ���ĺܶ�ʱ�䣬
												**����Ϊ�����û��о��رյĺܿ죬�������أ�������ͷŵȷ�ʱ�Ĺ�������������
												*/
												layer_switch(layer, -1);
												keywindow_off(key_to_window);
												key_to_window = layer_manage->layers_order[layer_manage->top - 1];
												keywindow_on(key_to_window);
												io_cli();
												fifo_put(&task->fifo, 4);
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						//ֻҪlast_mcursorX����0��˵����ǰ���ڴ����ƶ�ģʽ��˵����ǰ����������ĳ��ͼ��ı�������
						else
						{
							x = mCursorX - last_mcursorX;
							y = mCursorY - last_mcursorY;
							new_layerX = (tmp_layerX + x + 2) & ~3;
							new_layerY = new_layerY + y;
							last_mcursorY = mCursorY;			//���µ��ƶ��������
						}
					}
					//�����ǰû�а���������Ͱ�last_mcursorX����Ϊ��������Ϊ��ǰ�����ڴ����ƶ�������
					else
					{
						last_mcursorX = -1;
						if(0x7fffffff != new_layerX)
						{
							layer_slide(layer, new_layerX, new_layerY);		//һ����겻����ⴰ���ƶ��ˣ����������һ�λ��ƴ��ڲ���ֹͣ���ڵ��ƶ�
							new_layerX = 0x7fffffff; 
						}
					}
					
				}
			}
			//�յ����֮������ݣ�˵�����ùر�console��
			else if(768 <= i && i <= 1023)
			{
				close_console(layer_manage->layers + (i - 768));
			}
			//�ر������е�����
			else if(1024 <= i && i <= 2023)
			{
				close_console_task(taskctl->tasks + (i - 1024));
			}
			//�ر������д��ڣ��ͷ�����ͼ�㣩
			else if(2024 <= i && i <= 2279)			//ͼ�����ֻ��256��������ƫ����֮����������Χ��
			{
				layer_to_free = layer_manage->layers + (i - 2024);
				memmanage_free_4K(memmanage, (int) layer_to_free->buffer, 256 * 265);
				layer_free(layer_to_free);
			}
		}
	}	
}




void keywindow_on(LAYER *key_to_window)
{
	//�ȸı�������ڵı�������״̬
	change_titlebar(key_to_window, 1);
	//�����Ӧ�ô��ڵĻ�������flags��0x20λ���ó�1�ˣ�Ӧ�ô����ǲ���Ҫ��ʾ����
	if(0 != (key_to_window->flags & 0x20))
	{
		fifo_put(&key_to_window->task->fifo, 2);			//�����д��ڵĹ��OFF������Ϊ��ǰ��Ӧ�ó��򴰿��ڽ��ռ�������
	}
	return ;
}


void keywindow_off(LAYER *key_to_window)
{
	//�ȸı�������ڵı�������״̬
	change_titlebar(key_to_window, 0);
	//�����Ӧ�ô��ڵĻ�������flags��0x20λ���ó�1�ˣ�Ӧ�ô����ǲ���Ҫ��ʾ����
	if(0 != (key_to_window->flags & 0x20))
	{
		fifo_put(&key_to_window->task->fifo, 3);			//�����д��ڵĹ��OFF������Ϊ��ǰ��Ӧ�ó��򴰿��ڽ��ռ�������
	}
	return ;
}

//����һ�������д�������
TASK *open_console_task(LAYER *layer, unsigned int mem_total)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	TASK *task_console = task_alloc();
	int *console_fifo = (int *)memmanage_alloc_4K(memmanage, 128 * 4);
	task_console->console_stack = memmanage_alloc_4K(memmanage, 64 * 1024);			//��������Ҫ�����������������Լ�ȥ��12
	task_console->tss.esp = task_console->console_stack + 64 * 1024 - 12;
	task_console->tss.eip = (int) &console_task;			//eipָ���������console�ĺ����׵�ַ
	task_console->tss.es = 1 * 8;
	task_console->tss.cs = 2 * 8;
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
	*((int *) (task_console->tss.esp + 4)) = (int) layer;
	*((int *) (task_console->tss.esp + 8)) = mem_total;	
	task_run(task_console, 2, 2); 			/* level=2, priority=2 */
	init_fifo(&task_console->fifo, 128, console_fifo, task_console);
	return task_console;
}

//���µ�console�ĺ���
LAYER *open_console(LAYER_MANAGE *layer_manage, unsigned int memory_total)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	LAYER *layer_console = layer_alloc(layer_manage);
	unsigned char *buf_console = (unsigned char *)memmanage_alloc_4K(memmanage, 256 * 165);	
	layer_set(layer_console, buf_console, 256, 165, -1);
	create_window(buf_console, 256, 165, "console", INACTIVE);
	create_textbox(layer_console, 8, 28, 240, 128, COL8_000000);
	layer_console->task = open_console_task(layer_console, memory_total);
	//layer_console->task->console->view_length = 240;
	//layer_console->task->console->view_width = 128;
	layer_console->flags |= 0x20;			//flags��0x20����ǰ���ڵĹ��ON
	return layer_console;
}

void close_console_task(TASK *task)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	//���õ�ǰ�����console����������֮�󣬲��ٷ������CPUʱ�䣬Ȼ��Ϳ��԰��ĵ��ͷ������Ƶ��ڴ���
	task_sleep(task);
	//���ͷ�ջ��ռ�õ��ڴ�
	memmanage_free_4K(memmanage, task->console_stack, 64 * 1024);
	//Ȼ���ͷ�fifo��ռ�õ��ڴ�
	memmanage_free_4K(memmanage, (int) task->fifo.buffer, 128 * 4);
	task->flags = UNUSED;
	return ;	
}

void close_console(LAYER *layer)
{
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	TASK *task = layer->task;
	//���ͷ�ͼ����ռ�õ��ڴ�
	memmanage_free_4K(memmanage, (int) layer->buffer, 256 * 165);
	//Ȼ���ͷ�ͼ��
	layer_free(layer);
	//���ر����console��ռ�õ�����
	close_console_task(task);
	return ;
}








#include "bootpack.h"


//命令行换行函数
void console_newline(int *cursor_y, LAYER *layer)
{
	int x, y;
	//如果没有超过命令行屏幕的纵屏长度，就直接加纵坐标的值
	if(*cursor_y < 28 + 112)
	{
		*cursor_y += 16;
	}
	//如果回车后已经到达了最底端，那么就需要滚屏，即把屏幕的所有命令全部上移一行
	else
	{
		//先把所有行上移一行
		for(y = 28; y < 28 + 112; y++)
		{
			for(x = 8; x < 8 + 240; x++)
			{
				layer->buffer[x + y * layer->length] = layer->buffer[x + (y + 16) * layer->length];
			}
		}
		//把最后一行抹成黑色
		for(y = 28 + 112; y < 28 + 128; y++)
		{
			for(x = 8; x < 8 + 240; x++)
			{
				layer->buffer[x + y * layer->length] = COL8_000000;
			}
		}
		layer_part_refresh(layer, 8, 28, 8 + 240, 28 + 128);
	}
	return ;
}

//命令行窗口任务
void console_task(LAYER *layer, unsigned int mem_total)
{
	FIFO fifo;					//记着声明的时候声明成结构体而不是结构体指针，这个不是timer或者task那种实体全都在管理结构体中的
	TIMER *timer;
	TASK *task = task_now();
	int i, count = 0,fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_color = -1;
	int x, y;
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *)ADR_GDT;
	MEMMANAGE *memmanage = (MEMMANAGE *)MEMMANAGE_ADDR;
	int *fat = (int *)memmanage_alloc_4K(memmanage, 4 * 2880);		//用来存储FAT表的内存地址
	char strings[30], cmdline[30], *img_adr;						//cmdline用来记录在命令行中输入的命令
	FILEINFO *finfo = (FILEINFO *)(ADR_DISKIMG + 0x2600);
	
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));		//FAT表从第3个扇区开始，占用了共9个扇区
	
	init_fifo(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_set(timer, 50);
	
	//显示提示字符
	displayStrings_atLayer(layer, 8, 28, COL8_FFFFFF, COL8_000000, ">");
	
	for(;;)
	{
		io_cli();
		if(0 == fifo_status(&task->fifo))
		{
			task_sleep(task);
			io_sti();
		}
		else
		{
			//这里传出来的i直接就是键盘的对应键的字符加上一个键盘偏移量，不需要再转化了，只需要减去偏移量即可
			i = fifo_get(&task->fifo);
			io_sti();
			//计时器的中断数据处理（在这里就是光标的闪烁）
			if(1 == i || 0 == i)		//等于1或者0
			{
				if(1 == i)
				{
					if(cursor_color >= 0)
					{
						cursor_color = COL8_FFFFFF;
					}
					timer_init(timer, &task->fifo, 0);
				}
				else
				{
					if(cursor_color >= 0)
					{
						cursor_color = COL8_000000;
					}
					timer_init(timer, &task->fifo, 1);
				}
				timer_set(timer, 50);
			}
			//光标ON
			if(2 == i)		
			{
				cursor_color = COL8_FFFFFF;
			}
			//光标OFF
			if(3 == i)
			{
				drawRectangle(layer->buffer, layer->length, COL8_000000, cursor_x, cursor_y, 2, 15);
				cursor_color = -1;
			}
			//键盘数据的处理(主要是命令的处理)
			if(keyboard_offset <= i && i <= keyboard_offset + 255)
			{
				//如果是“退格键”的情况
				if(8 + keyboard_offset == i)
				{
					//不能删除最前面的提示符，而且只能在有字符的时候才能删除
					if(cursor_x > 16)
					{
						cursor_x -= 8;
						displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, "  ");
					}
				}
				//如果是“回车键”的情况
				else if(10 + keyboard_offset == i)
				{
					//用空格将光标擦除
					displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ");
					cmdline[cursor_x / 8 - 2] = 0;		//给这个命令字符串的最后加上一个“\0”
					console_newline(&cursor_y, layer);
					//查看是不是“mem”命令，如果是的话，就显示出关于内存的信息
					if(0 == strcmp(cmdline, "mem"))	
					{
						/*在屏幕上显示一些内存的信息*/
						sprintf(strings, "Memory has %dMB", mem_total / 1024 / 1024);
						displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, strings);
						console_newline(&cursor_y, layer);
						sprintf(strings, "free memory:%dKB",memmanage_total(memmanage) / 1024);
						displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, strings);			//用字体显示当前内存容量
						console_newline(&cursor_y, layer);
						console_newline(&cursor_y, layer);
					}
					//查看是不是“clear”命令，如果是的话，就清屏
					else if(0 == strcmp(cmdline, "clear"))
					{
						for(y = 28; y < 28 + 112; y++)
						{
							for(x = 8; x < 8 + 240; x++)
							{
								layer->buffer[x + y * layer->length] = COL8_000000;
							}
						}
						layer_part_refresh(layer, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;		//光标重新回到左上方
					}
					//查看是不是“ls”命令，如果是，就列出软盘中各个文件的信息
					else if(0 == strcmp(cmdline, "ls"))
					{
						for(x = 0; x < 224; x++)
						{
							//如果文件信息最初一个字节是0x00的话，说明这里没有文件
							if(0x00 == finfo[x].name[0])
							{
								break;
							}
							//如果文件信息最初一个字节是0xe5的话，说明这文件已经被删除了，所以不是的话才代表有有效文件
							if(0xe5 != finfo[x].name[0])
							{
								if(0 == (finfo[x].type & 0x18))
								{
									sprintf(strings, "filename.ext   %7d", finfo[x].size);
									for(y = 0; y < 8; y++)
									{
										strings[y] = finfo[x].name[y];
									}
									strings[9] = finfo[x].ext[0];
									strings[10] = finfo[x].ext[1];
									strings[11] = finfo[x].ext[2];
									displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, strings);
									console_newline(&cursor_y, layer);
								}
							}
						}
						console_newline(&cursor_y, layer);
					}
					//查看是不是type命令，如果是并且文件存在的话的话，就打印出该文件的内容
					else if(0 == strncmp(cmdline, "type ", 5))
					{
						/*1.根据命令里文件名，制作出对应格式的文件名*/
						//先把strings的前11个位置置成空格
						for(y = 0; y < 11; y++)
						{
							strings[y] = ' ';
						}
						y = 0;
						//把文件名和扩展名按规定形式存储到strings之中
						for(x = 5; y < 11 && cmdline[x] != 0; x++)
						{
							//遇见“.”这个符号，说明之前的字符都是文件名，之后的字符是扩展名
							if('.' == cmdline[x] && y <= 8)
							{
								y = 8;
							}
							else
							{
								strings[y] = cmdline[x];
								//如果文件名是小写字母，需要转化为大写字母，这样才能与内存中的文件名匹配
								if('a' <= strings[y] && strings[y] <= 'z')
								{
									strings[y] -= 0x20;
								}
								y++;
							}
						}
						/*2.从finfo的位置开始寻找这个文件*/
						//从0x2600到0x4200之间只能存储224个文件信息
						for(x = 0; x < 224;)
						{
							//这种情况说明再往后就没有文件信息存储了
							if(0x00 == finfo[x].name[0])
							{
								break;
							}
							//只有这个文件类型不是目录并且不是非文件的时候才能满足打印要求
							if(0 == (finfo[x].type & 0x18))
							{
								//如果有这个文件存在，就退出循环，开始打印
								if(0 == strncmp(finfo[x].name, strings, 11))
								{
									break;
								}
							}
							x++;
						}
						/*3.找到文件后制作输出*/
						//如果x < 224并且那么起始的一个字节不是0x00，则说明有着文件存在并且可以打印
						if(x < 224 && finfo[x].name[0] != 0x00)
						{
							//文件内容开始的地址赋值给img_adr
							img_adr = (char *)memmanage_alloc_4K(memmanage, finfo[x].size);
							//把可能有间断的文件内容复制到一块儿连续的内存中去，这样就可以直接到这块儿内存进行连续的读取了
							file_loadfile(finfo[x].clusterno, finfo[x].size, img_adr, fat, (char *)(0x003e00 + ADR_DISKIMG));
							cursor_x = 8;
							for(y = 0; y < finfo[x].size; y++)
							{
								//逐字输出
								strings[0] = img_adr[y];
								strings[1] = 0;
								//如果这个字符是制表符的话
								if(0x09 == strings[0])
								{
									for(;;)
									{
										displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ");
										cursor_x += 8;
										//如果到达命令行窗口的最右端则需要换行
										if(cursor_x >= 8 + 240)
										{
											cursor_x = 8;
											console_newline(&cursor_y, layer);
										}
										//一个字符是八个像素的宽度，每个制表符相隔4个字符
										if(0 == ((cursor_x - 8) & 0x1f))
										{
											break;
										}
									}
								}
								//如果这个字符是换行符的话
								else if(0x0a == strings[0])
								{
									cursor_x = 8;
									console_newline(&cursor_y, layer);
								}
								//如果这个字符是回车符的话
								else if(0x0d == strings[0])
								{
									/*暂时不做处理*/
								}
								//如果是正常字符的话
								else
								{
									displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, strings);
									cursor_x += 8;
									//如果到达命令行窗口的最右端则需要换行
									if(cursor_x >= 8 + 240)
									{
										cursor_x = 8;
										console_newline(&cursor_y, layer);
									}
								}
							}
							//把内容显示到屏幕之后就把这块儿内存释放掉
							memmanage_free_4K(memmanage, (int)img_adr, finfo[x].size);
						}
						/*4.找不到该文件的话，输出错误信息*/
						else
						{
							displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found!");
							console_newline(&cursor_y, layer);
						}
						console_newline(&cursor_y, layer);
					}
					//如果命令为hlt，则启动应用程序hlt
					else if(0 == strcmp(cmdline, "hlt"))
					{
						for (y = 0; y < 11; y++)
						{
							strings[y] = ' ';
						}
						strings[0] = 'H';
						strings[1] = 'L';
						strings[2] = 'T';
						strings[8] = 'H';
						strings[9] = 'R';
						strings[10] = 'B';
						for(x = 0; x < 224; )
						{
							if(0x00 == finfo[x].name[0])
							{
								break;
							}
							if(0 == (finfo[x].type & 0x18))
							{
								//如果有这个文件存在，就退出循环，开始打印
								if(0 == strncmp(finfo[x].name, strings, 11))
								{
									break;
								}
							}
							x++;
						}
						/*找到这个应用程序的情况*/
						if(x < 224 && finfo[x].name[0] != 0x00)
						{
							//文件内容开始的地址赋值给img_adr
							img_adr = (char *)memmanage_alloc_4K(memmanage, finfo[x].size);
							//把可能有间断的文件内容复制到一块儿连续的内存中去，这样就可以直接到这块儿内存进行连续的读取了
							file_loadfile(finfo[x].clusterno, finfo[x].size, img_adr, fat, (char *)(0x003e00 + ADR_DISKIMG));
							//为这个任务分配一个内存段
							set_segmdesc(gdt + 1003, finfo[x].size - 1, (int)img_adr, AR_CODE32_ER);
							//跳转到这个任务开始执行
							farjmp(0, 1003 * 8);
							memmanage_free_4K(memmanage, (int)img_adr, finfo[x].size);
						}
						/*无法找到这个应用程序的情况*/
						else
						{
							displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found!");
							console_newline(&cursor_y, layer);
						}
						console_newline(&cursor_y, layer);
					}
					//虽然不是空命令，但是无法识别
					else if(0 != cmdline[0])
					{
						displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command!");
						console_newline(&cursor_y, layer);
						console_newline(&cursor_y, layer);
					}
					//在新的一行显示提示符
					displayStrings_atLayer(layer, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">");
					cursor_x = 16;
				}
				//如果是一般字符的情况
				else
				{
					if(cursor_x < 240)
					{
						strings[0] = i - keyboard_offset;
						strings[1] = 0;
						cmdline[cursor_x / 8 -2] = i - keyboard_offset;			//需要记录用户输入的命令是什么
						displayStrings_atLayer(layer, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, strings);
						cursor_x += 8;
					}
				}
			}
			//重新显示光标
			if(cursor_color >= 0)
			{
				drawRectangle(layer->buffer, layer->length, cursor_color, cursor_x, cursor_y, 2, 15);				
			}
			layer_part_refresh(layer, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

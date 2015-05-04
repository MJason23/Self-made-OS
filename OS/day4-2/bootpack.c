//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

void io_hlt(void);
void io_cli(void);						//���ж���ɱ�־��Ϊ0����ֹ�жϣ���һ���ִ���Ľ���������P80
void io_out8(int port,int data);		//��ָ��װ�÷�����Ϣ
int io_load_eflags(void);					//��¼�ж���ɱ�־��ֵ
void io_store_eflags(int eflags);		//�ָ��ж���ɱ�־��ֵ
//void write_mem8(int addr,int data);
void init_palette(void);          		//��ʼ����ɫ�壬�����ɫ���������涨������������Ӧ����ɫ��
void set_palette(int start,int end,unsigned char *rgb);  			//���õ�ɫ��
//����vram��������ʾ�ڴ����ʼ��ַ������xSize�����������������������color���������ľ��ε���ɫ������x��y��������ε����ꣻ����width��length��������εĿ��
void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int length);

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

void HariMain(void)
{
	char *vram;
	int xSize,ySize;
	
	init_palette();
	
	vram = (char *)0xa0000;
	xSize = 320;
	ySize = 200;
	//����һ��windows����
	drawRectangle(vram, xSize, COL8_008484, 0, 0, xSize - 1, ySize - 29); 			//�����ı������Σ��·�������λ���������ľ���
	drawRectangle(vram, xSize, COL8_C6C6C6, 0, ySize - 28, xSize - 1, 0); 			//���һ����ϸ��ϸ������
	drawRectangle(vram, xSize, COL8_FFFFFF, 0, ySize - 27, xSize - 1, 0); 			//ͬ�ϣ����������������Ч��
	drawRectangle(vram, xSize, COL8_C6C6C6, 0, ySize - 26, xSize - 1, 25); 			//�������·��Ļ�ɫ������
	
	drawRectangle(vram, xSize, COL8_FFFFFF, 3, ySize - 24, 56, 0); 			//������ʼ��ť���Ϸ����������
	drawRectangle(vram, xSize, COL8_FFFFFF, 2, ySize - 24, 0, 20); 			//������ʼ��ť���󷽵��������
	drawRectangle(vram, xSize, COL8_848484, 3, ySize - 4, 56, 0); 			//������ʼ��ť���·����������
	drawRectangle(vram, xSize, COL8_848484, 59, ySize - 23, 0, 18); 		//������ʼ��ť���·����������
	drawRectangle(vram, xSize, COL8_000000, 2, ySize - 3, 57, 0); 			//
	drawRectangle(vram, xSize, COL8_000000, 60, ySize - 24, 0, 21);			//
	
	drawRectangle(vram, xSize, COL8_848484, xSize - 47, ySize - 24, 43, 0); 		//
	drawRectangle(vram, xSize, COL8_848484, xSize - 47, ySize - 23, 0, 19); 		//
	drawRectangle(vram, xSize, COL8_FFFFFF, xSize - 47, ySize - 3, 43, 0); 			//
	drawRectangle(vram, xSize, COL8_FFFFFF, xSize - 3, ySize - 24, 0, 21); 			//
	
	for(;;)
	{
		io_hlt();						//ִ���������ϵͳ֮�����CPUͣ����
	}	
}

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,    //��ɫ
		0xff, 0x00, 0x00,    //����ɫ
		0x00, 0xff, 0x00,    //����ɫ
		0xff, 0xff, 0x00,    //����ɫ
		0x00, 0x00, 0xff,    //����ɫ
		0xff, 0x00, 0xff,    //����ɫ
		0x00, 0xff, 0xff,    //ǳ����
		0xff, 0xff, 0xff,    //��ɫ
		0xc6, 0xc6, 0xc6,    //����ɫ
		0x84, 0x00, 0x00,    //����ɫ
		0x00, 0x84, 0x00,    //����ɫ
		0x84, 0x84, 0x00,    //����ɫ
		0x00, 0x00, 0x84,    //����ɫ
		0x84, 0x00, 0x84,    //����ɫ
		0x00, 0x84, 0x84,    //ǳ����
		0x84, 0x84, 0x84,    //����ɫ
	};
	set_palette(0, 15, table_rgb);
	return ;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int eflags,i;
	eflags = io_load_eflags();	
	io_cli();
	io_out8(0x03c8,start);
	for(i = start;i <= end;i++)
	{
		io_out8(0x03c9,rgb[0] / 4);			//������ɫRGB��R����
		io_out8(0x03c9,rgb[1] / 4);			//������ɫRGB��G����
		io_out8(0x03c9,rgb[2] / 4);			//������ɫRGB��B����
		rgb += 3;
	}
	io_store_eflags(eflags);
	return ;
}

void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int length)
{
	int iX,iY;
	for(iY = y; iY <= y + length; iY++)
	{
		for(iX = x; iX <= x + width; iX++)
		{
			vram[iX + iY * xSize] = color;
		}
	}
}






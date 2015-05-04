//�ⲿ�ִ�������ã�����ϵͳ����������������ʽ�Ĳ���ϵͳ
//���ڶ��ض�װ�õĶ�д���жϵĵ���ֱ����C�����޷�ʵ�֣�������Щ������Ҫ�û��������ʵ��

#define NULL 0x00

//�ṹ������
typedef struct{
	char cyls, leds, vmode, reserve;
	short scrnx,scrny;
	char *vram;
}BOOTINFO;

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
void init_screen(char *vram, int x, int y);			//��������Ļ�����Ĳ����ܽ��һ������
void displayfont(char *vram, int scrnx, int x, int y, char color, char *font);						//��ָ��λ����ʾָ����ɫ��ĳ������ĺ���
void displayStrings_CS(char *vram, int scrnx, int x, int y, char color, unsigned char *strings);	//������ָ��λ����ʾָ����ɫ���ַ����ĺ�����ʹ�õı��뼯��charset.txt

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
	BOOTINFO *binfo = (BOOTINFO *)0x0ff0;
	
	init_palette();
	
	//����һ��windows����
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//��ʾһ���ַ���
	displayStrings_CS(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "It's the first time that I made an OS myself!!");
	
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

void init_screen(char *vram, int x, int y)
{
	drawRectangle(vram, x, COL8_008484, 0, 0, x - 1, y - 29); 			//�����ı������Σ��·�������λ���������ľ���
	drawRectangle(vram, x, COL8_C6C6C6, 0, y - 28, x - 1, 0); 			//���һ����ϸ��ϸ������
	drawRectangle(vram, x, COL8_FFFFFF, 0, y - 27, x - 1, 0); 			//ͬ�ϣ����������������Ч��
	drawRectangle(vram, x, COL8_C6C6C6, 0, y - 26, x - 1, 25); 			//�������·��Ļ�ɫ������
	
	drawRectangle(vram, x, COL8_FFFFFF, 3, y - 24, 56, 0); 			//������ʼ��ť���Ϸ����������
	drawRectangle(vram, x, COL8_FFFFFF, 2, y - 24, 0, 20); 			//������ʼ��ť���󷽵��������
	drawRectangle(vram, x, COL8_848484, 3, y - 4, 56, 0); 			//������ʼ��ť���·����������
	drawRectangle(vram, x, COL8_848484, 59, y - 23, 0, 18); 		//������ʼ��ť���·����������
	drawRectangle(vram, x, COL8_000000, 2, y - 3, 57, 0); 			//
	drawRectangle(vram, x, COL8_000000, 60, y - 24, 0, 21);			//
	
	drawRectangle(vram, x, COL8_848484, x - 47, y - 24, 43, 0); 		//
	drawRectangle(vram, x, COL8_848484, x - 47, y - 23, 0, 19); 		//
	drawRectangle(vram, x, COL8_FFFFFF, x - 47, y - 3, 43, 0); 			//
	drawRectangle(vram, x, COL8_FFFFFF, x - 3, y - 24, 0, 21); 			//
}

void displayfont(char *vram, int scrnx, int x, int y, char color, char *font)
{
	int i;
	char *p, d;
	for(i = 0; i < 16; i++)
	{
		//������ض�λ������Ӧ����ʾ�ڴ�ĵ�ַ�ļ��㹫ʽ
		p = vram + (y + i) * scrnx + x;
		d = font[i];
		if(0 != (d & 0x80))
		{
			p[0] = color;
		}
		if(0 != (d & 0x40))
		{
			p[1] = color;
		}
		if(0 != (d & 0x20))
		{
			p[2] = color;
		}
		if(0 != (d & 0x10))
		{
			p[3] = color;
		}
		if(0 != (d & 0x08))
		{
			p[4] = color;
		}
		if(0 != (d & 0x04))
		{
			p[5] = color;
		}
		if(0 != (d & 0x02))
		{
			p[6] = color;
		}
		if(0 != (d & 0x01))
		{
			p[7] = color;
		}
	}
}

void displayStrings_CS(char *vram, int scrnx, int x, int y, char color, unsigned char *strings)
{
	extern char charset[4096];
	for(; *strings != NULL; strings++)
	{
		displayfont(vram, scrnx, x, y, color, charset + *strings * 16);			//charset����256���ַ���ÿ���ַ�ռ16���ֽڣ�����˳����ASCII��һ�����������Ҫ��ʾ��ĸ��A����ֻ��Ҫ��charset�����ַ�����ϼ���65*16���ɣ�Ҳ����д��"A"*16
		x += 8;			//һ���ַ�ռ�ð˸����صĿ��
		//����������Լ�д�ģ����������������scrnx����������ôҪ�����ƶ�16�����أ�����x��0
		if(x + 8 > scrnx)
		{
			y += 16;
			x = 0;
		}
	}
}












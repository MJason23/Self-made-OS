#ifndef BOOTPACK_H
#define BOOTPACK_H
/* asmhead.nas */
#define ADR_BOOTINFO	0x00000ff0
/* dsctbl.c */
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
/* bootpack.c */
#define mCursorWidth 16
#define mCursorHeight 16
/* graphic.c */
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

/* asmhead.nas */
//启动信息结构体声明
typedef struct{
	char cyls, leds, vmode, reserve;
	short scrnx,scrny;
	char *vram;
}BOOTINFO;

/* desctable.c */
//GDT表项（描述符）声明
typedef struct{
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
}SEGMENT_DESCRIPTOR;

//IDT表项声明
typedef struct{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
}GATE_DESCRIPTOR;


/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);						//将中断许可标志置为0，禁止中断，这一部分代码的解释在书上P80
void io_out8(int port,int data);		//向指定装置发送信息
int io_load_eflags(void);					//记录中断许可标志的值
void io_store_eflags(int eflags);		//恢复中断许可标志的值

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
//void write_mem8(int addr,int data);

/* graphic.c */
void init_palette(void);          		//初始化调色板，这个调色板是用来规定各个数字所对应的颜色的
void set_palette(int start,int end,unsigned char *rgb);  			//设置调色板
//参数vram：代表显示内存的起始地址；参数xSize：代表横屏的像素数；参数color：代表画出的矩形的颜色；参数x和y：代表矩形的坐标；参数width和length：代表矩形的宽高
void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int height);
void init_screen(char *vram, int x, int y);			//把生成屏幕背景的步骤总结成一个函数
void displayfont(char *vram, int scrnx, int x, int y, char color, char *font);						//在指定位置显示指定颜色的某个字体的函数，这个函数无编码集支持，所以以后很少用了
void displayStrings_CS(char *vram, int scrnx, int x, int y, char color, unsigned char *strings);	//用来在指定位置显示指定颜色的字符串的函数，使用的编码集是charset.txt
void init_mouse_cursor(char *mouse, char back_color);				//初始化鼠标的图像，函数中会有鼠标的图形编码，而还需要有额外的内存来存储对应的图形位置的颜色值
void displayShape(char *vram, int scrnx, int width, int height, int x, int y, char *shapeBuffer, int xPixel);		//显示指定内存中的图形
void init_GDTandIDT(void);
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int access_right);
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int access_right);

#endif 
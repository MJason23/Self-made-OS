#ifndef BOOTPACK_H
#define BOOTPACK_H
/* asmhead.nas */
#define ADR_BOOTINFO	0x00000ff0			//写入BOOTINFO的内存地址
/* desctable.c */
#define ADR_IDT			0x0026f800			//IDT所在的内存地址
#define LIMIT_IDT		0x000007ff			//IDT表中所能容纳的描述符个数的上限
#define ADR_GDT			0x00270000			//GDT所在的内存地址
#define LIMIT_GDT		0x0000ffff			//GDT表中所能容纳的描述符个数的上限
#define ADR_BOTPAK		0x00280000			//操作系统所在段的基址
#define LIMIT_BOTPAK	0x0007ffff			//操作系统所在段的上限大小
#define AR_DATA32_RW	0x4092				//代表段的访问权限为：系统专用，可读写的段，不可执行
#define AR_CODE32_ER	0x409a				//代表段的访问权限为：系统专用，可执行的段，不可读写
#define AR_INTGATE32	0x008e				//表示用于中断处理的有效设定
/* bootpack.c */
#define mCursorWidth 16						//鼠标图形的宽度
#define mCursorHeight 16					//鼠标图形的高度
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
/*int.c*/
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

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
	short limit_low, base_low;				//GDT表项有8个字节，其中基址占用了4个字节；为了向前兼容（即与80286CPU兼容），分为了低2字节base_low，中1字节base_mid，高1字节base_high
	char base_mid, access_right;			//访问权限占用了1.5个字节，即12位，其中有4个字节占用了段限的高字节（limit_high）的高4位
	char limit_high, base_high;				//段限占用了2.5个字节，即20位，其中有一个低2字节limit_low，另外4字节是limit_high的低4位
}SEGMENT_DESCRIPTOR;

//IDT表项声明
typedef struct{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
}GATE_DESCRIPTOR;

/*int.c*/
//这个结构体是用来保存键盘传入信息的，相当于缓冲区，但是只能缓存一个，一旦在处理中断的过程中又有其他中断传入，则只能丢弃，这是设计缺陷
typedef struct{
	unsigned char data[32];
	int next_w, next_r, length;
}KEYBUF;

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);						//将中断许可标志置为0，禁止中断，这一部分代码的解释在书上P80
void io_sti(void);
void io_out8(int port,int data);		//向指定装置发送信息
int io_load_eflags(void);					//记录中断许可标志的值
void io_store_eflags(int eflags);		//恢复中断许可标志的值

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
/*与下面的不带asm_的函数相比，这些函数仅仅是在调用inthandler之前保存了寄存器的值，并在执行完inthandler之后恢复了寄存器的值（这些功能需要用到汇编，但是实际中断
处理程序还是由C语言写比较好，所以有了两套函数）。曾经忘了写这三个函数导致查找错误用了很长时间，急着凡是用到汇编语言的函数都需要在这里声明一下才行，切记切记！！
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
*/
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
//void write_mem8(int addr,int data);

/* graphic.c */
void init_palette(void);          		//初始化调色板，这个调色板是用来规定各个数字所对应的颜色的
void set_palette(int start,int end,unsigned char *rgb);  			//设置调色板
//参数vram：代表显示内存的起始地址；参数xSize：代表横屏的像素数；参数color：代表画出的矩形的颜色；参数x和y：代表矩形的坐标；参数width和length：代表矩形的宽高
void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int height);

void init_screen(char *vram, int x, int y);			//把生成屏幕背景的步骤总结成一个函数
void init_mouse_cursor(char *mouse, char back_color);				//初始化鼠标的图像，函数中会有鼠标的图形编码，而还需要有额外的内存来存储对应的图形位置的颜色值

void displayfont(char *vram, int scrnx, int x, int y, char color, char *font);						//在指定位置显示指定颜色的某个字体的函数，这个函数无编码集支持，所以以后很少用了
void displayStrings_CS(char *vram, int scrnx, int x, int y, char color, unsigned char *strings);	//用来在指定位置显示指定颜色的字符串的函数，使用的编码集是charset.txt
void displayShape(char *vram, int scrnx, int width, int height, int x, int y, char *shapeBuffer, int xPixel);		//显示指定内存中的图形

/*desctable.c*/
void init_GDTandIDT(void);
//设置GDT的函数；第一个参数表示GDT表项的地址（不一定是第一个表项的地址），第二个参数表示段的上限大小，第三个参数表示段的基址，第四个参数表示段的访问权限（具体在书上P114）
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int access_right);
//设置IDT的函数：第一个参数表示IDT表项的地址（不一定是第一个表项的地址），
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int access_right);

/*int.c*/
void init_pic(void);						//初始化主从PIC的寄存器
void inthandler21(int *esp);				//键盘中断的处理程序
void inthandler27(int *esp);
void inthandler2c(int *esp);				//鼠标中断的处理程序
#endif 







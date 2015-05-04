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
/*fifo.c*/
#define FLAGS_OVERRUN 0x0001  			//表示缓冲区溢出的标志
#define FLAGS_NOTOVER 0x0000  			//表示缓冲区未溢出的标志
/*mouse_.c*/  /*keyboard.c*/
#define PORT_KEYDATA			0x0060	//键盘的设备号
#define PORT_KEYSTA				0x0064	//
#define PORT_KEYCMD				0x0064	//键盘控制电路
#define KEYSTA_SEND_NOTREADY	0x02	//这是一个用来比较的数，当CPU从键盘控制器电路读取到的数据倒数第二位是0时，说明键盘控制电路已经可以接受CPU指令了
#define KEYCMD_SET_MODE			0x60	//设定模式指令
#define KBC_MOUSE_MODE			0x47	//鼠标模式
#define KEYCMD_SENDTO_MOUSE		0xd4	//如果向键盘发送0xd4指令，则下一个数据就会自动发送给鼠标
#define MOUSECMD_ENABLE			0xf4	//鼠标激活指令
/*memory.c*/
#define EFLAGS_AC_BIT 			0x00040000
#define CR0_CACHE_DISABLE		0x60000000
#define MEMMANAGE_FREES			4906			//用于记录内存管理的数量
#define MEMMANAGE_ADDR			0x003c0000		//内存管理结构体的存放地址
/*layer_manage.c*/
#define MAX_LAYER				0x256			//可以管理的最多的图层个数
#define LAYER_USING				1				//图层已被占用中
#define LAYER_AVAILABLE			0				//图层可用
#define LAYER_HIDE				-1



/* asmhead.nas */
//启动信息结构体声明
typedef struct{
	char cyls, leds, vmode, reserve;
	short scrnx,scrny;
	unsigned char *vram;
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
	unsigned char *data;
	int next_w, next_r, size, free, flags;
}FIFO;

/*mouse_.c*/
typedef struct{
	unsigned char buffer[3], phase;
	int x, y, btn;
}MOUSE_DECODE;

/*memory.c*/
//内存地址和大小
typedef struct{
	unsigned int addr, size;		
}FREEINFO;
//内存管理
typedef struct{
	unsigned int frees, maxfrees, lostsize, losttimes;
	FREEINFO free[MEMMANAGE_FREES];
}MEMMANAGE;

/*layer_manage.c*/

typedef struct{
	unsigned char *buffer;									//指向存储图形内容的内存地址
	int length, width, x, y, height, color_luc, flags;		//height代表图层在显示状态中的高度, color_luc只是一种标志性设定，设置成什么是随便的，
	struct LAYER_MANAGE *layer_man;							//只要当初设计这个图形的时候，把想要画成透明的部分内存用color_luc填充就行, flags存放有关图层的各种设定信息
}LAYER;

typedef struct LAYER_MANAGE{
	unsigned char *vram;			//直接保存下显存的地址，这样就不用每次都传入显存地址和屏幕大小了
	int scrnx, scrny, top;			//top表示管理的图层中的最高图层的高度
	LAYER *layers_order[MAX_LAYER];	//根据图层的高度排序的数组，每个数组元素指向一个图层结构体LAYER的地址，这样便于查找管理等
	LAYER layers[MAX_LAYER];		//每个数组元素都是一个图层结构体LAYER，每个结构体都记录着自己图层的各种信息
}LAYER_MANAGE;




/* naskfunc.nas */
/*与下面的不带asm_的函数相比，这些函数仅仅是在调用inthandler之前保存了寄存器的值，并在执行完inthandler之后恢复了寄存器的值（这些功能需要用到汇编，但是实际中断
处理程序还是由C语言写比较好，所以有了两套函数）。曾经忘了写这三个函数导致查找错误用了很长时间，急着凡是用到汇编语言的函数都需要在这里声明一下才行，切记切记！！
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
*/
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

/* graphic.c */
void init_palette(void);          		//初始化调色板，这个调色板是用来规定各个数字所对应的颜色的
void set_palette(int start,int end,unsigned char *rgb);  			//设置调色板
//参数vram：代表显示内存的起始地址；参数xSize：代表横屏的像素数；参数color：代表画出的矩形的颜色；参数x和y：代表矩形的坐标；参数width和length：代表矩形的宽高
void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int width, int height);

void init_screen(unsigned char *vram, int x, int y);							//把生成屏幕背景的步骤总结成一个函数
void init_mouse_cursor(unsigned char *mouse, char back_color);				//初始化鼠标的图像，函数中会有鼠标的图形编码，而还需要有额外的内存来存储对应的图形位置的颜色值

void displayfont(unsigned char *vram, int scrnx, int x, int y, char color, char *font);										//在指定位置显示指定颜色的某个字体的函数，这个函数无编码集支持，所以以后很少用了
void displayStrings_CS(unsigned char *vram, int scrnx, int x, int y, char color, unsigned char *strings);					//用来在指定位置显示指定颜色的字符串的函数，使用的编码集是charset.txt
void displayShape(unsigned char *vram, int scrnx, int width, int height, int x, int y,unsigned char *shapeBuffer, int xPixel);		//显示指定内存中的图形

/*desctable.c*/
void init_GDTandIDT(void);
//设置GDT的函数；第一个参数表示GDT表项的地址（不一定是第一个表项的地址），第二个参数表示段的上限大小，第三个参数表示段的基址，第四个参数表示段的访问权限（具体在书上P114）
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int access_right);
//设置IDT的函数：第一个参数表示IDT表项的地址（不一定是第一个表项的地址），
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int access_right);

/*interrupt.c*/
void init_pic(void);						//初始化主从PIC的寄存器
void inthandler27(int *esp);

/*fifo.c*/
void init_fifo(FIFO *fifo, int size, unsigned char *buf);	//初始化缓冲区的函数
int fifo_put(FIFO *fifo, unsigned char data);				//向FIFO缓冲区中保存数据
unsigned char fifo_get(FIFO *fifo);							//从FIFO缓冲区中向外读取数据
int fifo_status(FIFO *fifo);								//获取FIFO缓冲区当前的积攒的数据数量

/*mouse_.c*/
void enable_mouse(MOUSE_DECODE *mouse_dec);						//激活鼠标
int mouse_decode(MOUSE_DECODE *mouse_dec, unsigned char data);	//解析鼠标传入的信息（鼠标每次都会产生三个中断传入3个字节的信息）
void inthandler2c(int *esp);									//鼠标中断处理程序

/*keyboard.c*/
void wait_KBC_sendready(void);					//让键盘控制电路处于等待CPU的命令的状态
void init_keyboard(void);						//激活键盘控制电路
void inthandler21(int *esp);					//键盘中断处理程序

/*memory.c*/
unsigned int memtest(unsigned int start, unsigned int end);							//内存检查程序
void memmanage_init(MEMMANAGE *memman);												//内存管理初始化
unsigned int memmanage_total(MEMMANAGE *memman);									//查看内存中剩余的容量
unsigned int memmanage_alloc(MEMMANAGE *memman, unsigned int size);					//内存分配
int memmanage_free(MEMMANAGE *memman, unsigned int addr, unsigned int size);		//内存释放
unsigned int memmanage_alloc_4K(MEMMANAGE *memman, unsigned int size);				//以4KB为单位的内存分配
int memmanage_free_4K(MEMMANAGE *memman, unsigned int addr, unsigned int size);		//以4KB为单位的内存释放

/*layer_manage.c*/
LAYER_MANAGE *layer_man_init(MEMMANAGE *memman, unsigned char *vram, int scrnx, int scrny);
LAYER *layer_alloc(LAYER_MANAGE *layer_man);
void layer_set(LAYER *layer, unsigned char *buf, int length0,int width0, int col_luc);
void layer_switch(LAYER *layer, int height);
void layer_slide(LAYER *layer, int x0, int y0);
int is_layer_overlap(LAYER *layer, int x0, int y0, int x1, int y1);
void layer_part_refresh(LAYER_MANAGE *layer_man, int x0, int y0, int x1, int y1);
void layer_refresh(LAYER *layer, int length0, int width0, int length1, int width1);
void layer_free(LAYER *layer);

/*window.c*/
void create_window(unsigned char *buf, int length, int width, char *title);
#endif 







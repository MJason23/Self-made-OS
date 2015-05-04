#ifndef BOOTPACK_H
#define BOOTPACK_H
/* asmhead.nas */
#define ADR_BOOTINFO	0x00000ff0			//写入BOOTINFO的内存地址
#define ADR_DISKIMG		0x00100000			//软盘内容被搬移到的地址
/* desctable.c */
#define ADR_IDT			0x0026f800			//IDT所在的内存地址
#define LIMIT_IDT		0x000007ff			//IDT表中所能容纳的描述符个数的上限
#define ADR_GDT			0x00270000			//GDT所在的内存地址
#define LIMIT_GDT		0x0000ffff			//GDT表中所能容纳的描述符个数的上限
#define ADR_BOTPAK		0x00280000			//操作系统所在段的基址
#define LIMIT_BOTPAK	0x0007ffff			//操作系统所在段的上限大小
#define AR_DATA32_RW	0x4092				//代表段的访问权限为：系统专用，可读写的段，不可执行
#define AR_CODE32_ER	0x409a				//代表段的访问权限为：系统专用，可执行的段，不可读写
#define AR_TSS32		0x0089				//代表任务状态段
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
/*interrupt.c*/
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
#define PIT_CTRL		0x0043
#define PIT_CNT0		0x0040
/*fifo.c*/
#define FLAGS_OVERRUN 0x0001  			//表示缓冲区溢出的标志
#define FLAGS_NOTOVER 0x0000  			//表示缓冲区未溢出的标志
/*timer.c*/
#define MAX_TIMER 				500		//可用的定时器个数上限
#define TIMER_FLAGS_AVAILABLE	0		//代表这个定时器未被使用
#define TIMER_FLAGS_ALLOC		1		//代表这个定时器已被分配出去，但是还没有在使用中
#define TIMER_FLAGS_USING		2		//代表这个定时器正在使用中
/*mouse_.c*/  /*keyboard.c*/
#define PORT_KEYDATA			0x0060	//键盘的设备号
#define PORT_KEYSTA				0x0064	//
#define PORT_KEYCMD				0x0064	//键盘控制电路
#define KEYSTA_SEND_NOTREADY	0x02	//这是一个用来比较的数，当CPU从键盘控制器电路读取到的数据倒数第二位是0时，说明键盘控制电路已经可以接受CPU指令了
#define KEYCMD_SET_MODE			0x60	//设定模式指令
#define KBC_MOUSE_MODE			0x47	//鼠标模式
#define KEYCMD_SENDTO_MOUSE		0xd4	//如果向键盘发送0xd4指令，则下一个数据就会自动发送给鼠标
#define MOUSECMD_ENABLE			0xf4	//鼠标激活指令
#define KEYCMD_LED				0xed
/*memory.c*/
#define EFLAGS_AC_BIT 			0x00040000
#define CR0_CACHE_DISABLE		0x60000000
#define MEMMANAGE_FREES			4090			//用于记录内存管理的数量（设计成4094的话，内存管理结构正好是占用了32KB的空间）
#define MEMMANAGE_ADDR			0x003c0000		//内存管理结构体的存放地址
/*layer_manage.c*/
#define MAX_LAYER				256				//可以管理的最多的图层个数
#define LAYER_USING				1				//图层已被占用中
#define LAYER_AVAILABLE			0				//图层可用
#define LAYER_HIDE				-1				//图层被隐藏
/*task_manage.c*/
#define MAX_TASKS				1000			//最大任务数量
#define TASK_GDT0				3				//定义从GDT的几号开始分配给TSS
#define MAX_TASKS_LV			100				//每个等级能有的最大任务数量
#define MAX_TASKLEVELS			10				//任务最多分成10个等级
#define RUNNING					2				//任务的运行状态
#define SLEEP					1				//任务的睡眠状态
#define	UNUSED					0				//任务的未分配状态
/*window.c*/
#define ACTIVE					1				//窗口处于当前活动状态
#define INACTIVE				0				//窗口不处于当前活动状态



/*文件信息结构*/
//从0x2600到0x4200之间总共可以存储224个这样的文件信息结构
typedef struct{
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clusterno;
	unsigned int size;
}FILEINFO;





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

void init_GDTandIDT(void);
//设置GDT的函数；第一个参数表示GDT表项的地址（不一定是第一个表项的地址），第二个参数表示段的上限大小，第三个参数表示段的基址，第四个参数表示段的访问权限（具体在书上P114）
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int access_right);
//设置IDT的函数：第一个参数表示IDT表项的地址（不一定是第一个表项的地址），
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int access_right);





/*fifo.c*/
//这个结构体是用来保存键盘传入信息的，相当于缓冲区，但是只能缓存一个，一旦在处理中断的过程中又有其他中断传入，则只能丢弃，这是设计缺陷
typedef struct{
	int *buffer;
	int next_w, next_r, size, free, flags;
	struct TASK *task;			//如果需要在缓冲区填充数据的时候唤醒任务，则这里需要设置一个task指针指示唤醒哪个任务
}FIFO;

void init_fifo(FIFO *fifo, int size, int *buf,struct TASK *task);	//初始化缓冲区的函数
int fifo_put(FIFO *fifo, int data);				//向FIFO缓冲区中保存数据
int fifo_get(FIFO *fifo);							//从FIFO缓冲区中向外读取数据
int fifo_status(FIFO *fifo);								//获取FIFO缓冲区当前的积攒的数据数量





/*timer.c*/
typedef struct{
	unsigned int timeout;		//代表指定时间（即现在的时间加上倒计时时间长度）
	char flags, autocancel_flags;				//代表使用标志和是否自动取消的标志
	FIFO *timer_fifo;					//时间到了之后需要向这个缓冲区输入数据
	int data;					//时间到了之后就是把这个数据填充到缓冲区中
	struct TIMER *next;			//记录紧挨着这个定时器之后的下一个定时器
}TIMER;

typedef struct{
	unsigned int count, next_timeout;	//这个count是用来计时的，每一次计时器发来中断的时候count都要++;next代表下一个指定时间
	TIMER *first_timer;	//相当于链表头
	TIMER timer[MAX_TIMER];
}TIMERCTL;

void init_PIT(void);						//初始化定时器中断
TIMER *timer_alloc(void);					//定时器分配
void timer_init(TIMER *timer, FIFO *fifo, int data);		//定时器初始化
void timer_set(TIMER *timer, unsigned int timeout);					//定时器时间设定
void timer_free(TIMER *timer);				//定时器释放
int timer_cancel(TIMER *timer);
void timer_cancelall(FIFO *fifo);
void inthandler20(int *esp);				//定时器中断处理程序






/*mouse_.c*/
typedef struct{
	unsigned char buffer[3], phase;
	int x, y, btn;
}MOUSE_DECODE;

void enable_mouse(FIFO *fifo, int offset, MOUSE_DECODE *mouse_dec);						//激活鼠标
int mouse_decode(MOUSE_DECODE *mouse_dec, unsigned char data);	//解析鼠标传入的信息（鼠标每次都会产生三个中断传入3个字节的信息）
void inthandler2c(int *esp);									//鼠标中断处理程序
/*keyboard.c*/
void wait_KBC_sendready(void);					//让键盘控制电路处于等待CPU的命令的状态
void init_keyboard(FIFO *fifo, int offset);						//激活键盘控制电路
void inthandler21(int *esp);					//键盘中断处理程序





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

unsigned int memtest(unsigned int start, unsigned int end);							//内存检查程序
void memmanage_init(MEMMANAGE *memman);												//内存管理初始化
unsigned int memmanage_total(MEMMANAGE *memman);									//查看内存中剩余的容量
unsigned int memmanage_alloc(MEMMANAGE *memman, unsigned int size);					//内存分配
int memmanage_free(MEMMANAGE *memman, unsigned int addr, unsigned int size);		//内存释放
unsigned int memmanage_alloc_4K(MEMMANAGE *memman, unsigned int size);				//以4KB为单位的内存分配
int memmanage_free_4K(MEMMANAGE *memman, unsigned int addr, unsigned int size);		//以4KB为单位的内存释放






/* task_manage.c */
/*任务状态段*/
typedef struct{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;			//这几个是与任务设置相关的信息，一般不会被CPU写入
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;	//eip指示在切换到这任务的时候，要从哪里开始运行；esp是指这个任务专门为这个任务所准备的段
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;			//这两个也是与任务设置相关的东西，不会被CPU写入
}TSS;							//task status segment（任务状态段），这是一个特殊的用于任务切换的段
/*任务*/
typedef struct{
	int selector, flags;				//selector代表GDT中的描述符的位置，flags表示当前的任务状态：运行，挂起，未分配
	int priority, level;				//任务的优先级和所在的等级
	FIFO fifo;
	TSS tss;
	struct CONSOLE *console;			//每个任务由一个console启动
	int ds_base;						//每个任务所在的数据段需要指定好
}TASK;
/*任务等级*/
typedef struct{
	int task_now;						/*当前征占用着CPU的任务*/
	int running_number;					/*正在运行的任务数量*/
	TASK *tasks_lv[MAX_TASKS_LV];		/*指向那些在这个等级中运行着的任务*/
}TASKLEVEL;
/*任务管理*/				//这个结构还是很大的，因为所有的任务和等级结构的实体都是存储在这个管理结构里的
typedef struct{
	int lv_now;									/*当前处于活动状态的level*/
	char lv_change;								/*记录level是否变化了的变量标志*/
	TASKLEVEL task_level[MAX_TASKLEVELS];		/*可以管理的10个等级*/
	TASK tasks[MAX_TASKS];						/*可以管理的1000个任务*/
}TASKCTL;

TASK *task_init(MEMMANAGE *memman);
TASK *task_alloc(void);
void task_run(TASK *task, int level, int priority);
void task_switch(void);
void task_sleep(TASK *task);






/*layer_manage.c*/
typedef struct{
	unsigned char *buffer;									//指向存储图形内容的内存地址
	int length, width, x, y, height, color_luc, flags;		//height代表图层在显示状态中的高度, color_luc只是一种标志性设定，设置成什么是随便的，
	struct LAYER_MANAGE *layer_man;							//只要当初设计这个图形的时候，把想要画成透明的部分内存用color_luc填充就行, flags存放有关图层的各种设定信息
	TASK *task;												//这个task是指当前的图层属于哪一个任务的，为了方便当关闭任务的时候释放图层
}LAYER;

typedef struct LAYER_MANAGE{
	unsigned char *vram, *map;			//直接保存下显存的地址，这样就不用每次都传入显存地址和屏幕大小了；map保存的相当于是一张高度表
	int scrnx, scrny, top;			//top表示管理的图层中的最高图层的高度
	LAYER *layers_order[MAX_LAYER];	//根据图层的高度排序的数组，每个数组元素指向一个图层结构体LAYER的地址，这样便于查找管理等
	LAYER layers[MAX_LAYER];		//每个数组元素都是一个图层结构体LAYER，每个结构体都记录着自己图层的各种信息
}LAYER_MANAGE;

LAYER_MANAGE *layer_man_init(MEMMANAGE *memman, unsigned char *vram, int scrnx, int scrny);
LAYER *layer_alloc(LAYER_MANAGE *layer_man);
void layer_set(LAYER *layer, unsigned char *buf, int length0,int width0, int col_luc);
void layer_switch(LAYER *layer, int height);
void layer_slide(LAYER *layer, int x0, int y0);
int is_layer_overlap(LAYER *layer, int x0, int y0, int x1, int y1);
void layer_refresh_map(LAYER_MANAGE *layer_man, int x0, int y0, int x1, int y1, int height);
void screen_part_refresh(LAYER_MANAGE *layer_man, int x0, int y0, int x1, int y1, int height1, int height2);
void layer_part_refresh(LAYER *layer, int length0, int width0, int length1, int width1);
void layer_free(LAYER *layer);









/*console.c*/
typedef struct{
	LAYER *layer;
	int cursor_x, cursor_y, cursor_color;
	TIMER *timer;
}CONSOLE;

void console_putchar(CONSOLE *console, int string, char move);
void console_putstring_length(CONSOLE *console, char *string, int length);
void console_putstring_toend(CONSOLE *console, char *string);
void console_newline(CONSOLE *console);
void console_runcmd(char *cmdline, CONSOLE *console, int *fat, unsigned int mem_total);
void cmd_mem(CONSOLE *console, unsigned int mem_total);
void cmd_clear(CONSOLE *console);
void cmd_ls(CONSOLE *console);
void cmd_type(CONSOLE *console, int *fat, char *cmdline);
int cmd_app(CONSOLE *console, int *fat, char *cmdline);
void cmd_hlt(CONSOLE *console, int *fat);
void hrb_api_drawline(LAYER *layer, int x0, int y0, int x1, int y1, int color);
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int *inthandler0c(int *esp);
int *inthandler0d(int *esp);
void console_task(LAYER *layer, unsigned int mem_total);





/* naskfunc.nas */
/*与下面的不带asm_的函数相比，这些函数仅仅是在调用inthandler之前保存了寄存器的值，并在执行完inthandler之后恢复了寄存器的值（这些功能需要用到汇编，但是实际中断
处理程序还是由C语言写比较好，所以有了两套函数）。曾经忘了写这三个函数导致查找错误用了很长时间，急着凡是用到汇编语言的函数都需要在这里声明一下才行，切记切记！！
！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
*/
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);

void asm_console_putchar(void);
void asm_end_app(void);
unsigned int memtest_wr(unsigned int start, unsigned int end);
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void asm_hrb_api(void);
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
void asm_inthandler0c(void);
void asm_inthandler0d(void);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

/* graphic.c */
void init_palette(void);          		//初始化调色板，这个调色板是用来规定各个数字所对应的颜色的
void set_palette(int start,int end,unsigned char *rgb);  			//设置调色板
//参数vram：代表显示内存的起始地址；参数xSize：代表横屏的像素数；参数color：代表画出的矩形的颜色；参数x和y：代表矩形的坐标；参数width和length：代表矩形的宽高
void drawRectangle(unsigned char *vram, int xSize, unsigned char color, int x, int y, int length, int width);

void init_screen(unsigned char *vram, int x, int y);							//把生成屏幕背景的步骤总结成一个函数
void init_mouse_cursor(unsigned char *mouse, char back_color);				//初始化鼠标的图像，函数中会有鼠标的图形编码，而还需要有额外的内存来存储对应的图形位置的颜色值

void displayfont(unsigned char *vram, int scrnx, int x, int y, char color, char *font);										//在指定位置显示指定颜色的某个字体的函数，这个函数无编码集支持，所以以后很少用了
void displayStrings_CS(unsigned char *vram, int scrnx, int x, int y, char color, unsigned char *strings);					//用来在指定位置显示指定颜色的字符串的函数，使用的编码集是charset.txt
void displayStrings_atLayer(LAYER *layer, int x, int y, int color, int bg_color, char *strings);	//在指定图层上的指定位置显示文字
void displayShape(unsigned char *vram, int scrnx, int width, int height, int x, int y,unsigned char *shapeBuffer, int xPixel);		//显示指定内存中的图形




/*interrupt.c*/
void init_pic(void);						//初始化主从PIC的寄存器
void inthandler27(int *esp);





/*window.c*/
void create_titlebar(unsigned char *buf, int length, char *title, char active);
void create_window(unsigned char *buf, int length, int width, char *title, char active);
void create_textbox(LAYER *layer, int x, int y, int length, int width, int color);
void change_titlebar(LAYER *layer, char active);




/*file.c*/
FILEINFO *file_search(char *name, FILEINFO *finfo, int max);
void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clusterno, int size, char *buffer, int *fat, char *img);




extern char key_table[0x80];
extern char key_table_shift[0x80];
//为了是所有中断共用一个缓冲区而不至于混淆，这里人为的为键盘和鼠标的缓冲区数据加一个不同的偏移值，这样就好分辨了
extern int keyboard_offset;
extern int mouse_offset;
#endif 







#include "bootpack.h"

void init_GDTandIDT(void)
{
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *)ADR_GDT;
	GATE_DESCRIPTOR *idt = (GATE_DESCRIPTOR *)ADR_IDT;
	int i;
	
	//GDT初始化
	for(i = 0; i <= LIMIT_GDT / 8; i++)
	{
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);		//GDT中的第一个段，段限为整个内存4G的空间，是最大的段
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);		//GDT中的第二个段，段限为512KB，用来存放真正的操作系统的段
	load_gdtr(LIMIT_GDT, ADR_GDT);		//把GDT的地址和描述符数量限制（8192个）写入到GDTR寄存器中，
	
	//IDT初始化
	for(i = 0; i <= LIMIT_IDT / 8; i++)
	{
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);
	
	/*向IDT中注册中断处理程序，前32个中断变量已经被CPU内部用了，所以CPU以外的设备只能从0x21开始了*/
	/*CPU的中断其实最多也就32种，而IDT表却有256个位置，所以32后面的位置都是空闲的，可以用于他用的*/
	//第三个参数代表中断程序属于哪个段，这三个明显都是属于第二个段（操作系统所在段），因为他们都是在操作系统中实现的嘛！
	set_gatedesc(idt + 0x0d, (int) asm_inthandler0d, 2 * 8, AR_INTGATE32);			//当应用程序试图非法访问操作系统的时候会产生这个中断
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);			//用于处理定时器中断的程序注册
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);			//用于处理键盘中断的程序注册
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);			//一种必要中断处理程序的注册
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);			//用于处理鼠标中断的程序注册
	set_gatedesc(idt + 0x40, (int) asm_hrb_api, 2 * 8, AR_INTGATE32 + 0x60);		//这个不是真正的中断，只是借用终端的形式调用的一个函数罢了,加上0x60之后代表这个是应用程序段
}

void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int access_right)
{
	if (limit > 0xfffff) {
		access_right |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = access_right & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((access_right >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int access_right)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (access_right >> 8) & 0xff;
	gd->access_right = access_right & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
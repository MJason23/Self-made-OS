#注意：ipl10.nas里的程序都是在软盘的第一扇区的，属于引导程序。
#bootpack.c是核心操作系统程序，中间需要用到使用汇编语言编写的函数文件naskfunc.nas，他们两个会编译成一个文件，然后再与asmhead.bin编译成一个文件（这个之前已经
#介绍过，是用来连接汇编语言和C语言的桥梁和向内存中记录重要信息的）haribote.sys，最后haribote.sys这个文件与引导程序ipl10.nas写成一个文件haribote.img刻到软盘上，
#haribote.sys里的内容会在haribote.img中的0x4200位置处开始出现（这是一种规定），而整个引导程序会从内存启动区0x8000开始执行，所以操作系统主程序会出现在内存的
#0xc200处！！！！！！！！！！！

OBJS_BOOTPACK = bootpack.obj naskfunc.obj charset.obj graphic.obj desctable.obj \
		interrupt.obj fifo.obj setup_mouse.obj

TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del


#普通规则
#这部分默认执行方式是由我自己改写的，这样的话就可以不用再去输入命令删除那些庞杂的中间文件了
default :
	$(MAKE) run
	$(MAKE) src_only
	
#这是一个相当于ASCII码表的文件，里面定义了各种符号对应的数字
charset.bin : charset.txt Makefile
	$(MAKEFONT) charset.txt charset.bin
	
#还需要把charset.bin转换成obj目标文件并且与bootpack.c结合起来才行（bootpack.c中需要用到这些字符），这个不能通过nas文件编译而成，需要单独写出来
charset.obj : charset.bin Makefile
	$(BIN2OBJ) charset.bin charset.obj _charset

#把bootpack.obj和naskfunc.obj以及charset.obj联合编译成bootpack.bim，bootpack.bim是一个中间文件
# 3MB+64KB=3136KB
bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)

#把中间文件bootpack.bim生成bootpack.hrb这种机器语言文件
bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
	
ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst
	
#这个文件的程序主要做了：读取键盘状态和指定程序执行地址以及由汇编语言转成C语言所需的工作，需要与bootpack.c一起编译
#这个是由以前的haribote.nas文件改名而成的asmhead.nas生成的
asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst
	
#把以前的haribote.nas改名成了asmhead.bin，联合bootpack.hrb一起编译成haribote.sys文件
haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img

		
#一般规则，凡是符合这种标准的文件都会按这种方式编译
%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c
	
%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas
	
%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst


img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

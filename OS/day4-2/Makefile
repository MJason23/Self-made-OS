#注意：ipl10.nas里的程序都是在软盘的第一扇区的，属于引导程序。
#bootpack.c是核心操作系统程序，中间需要用到使用汇编语言编写的函数文件naskfunc.nas，他们两个会编译成一个文件，然后再与asmhead.bin编译成一个文件（这个之前已经
#介绍过，是用来连接汇编语言和C语言的桥梁和向内存中记录重要信息的）haribote.sys，最后haribote.sys这个文件与引导程序ipl10.nas写成一个文件haribote.img刻到软盘上，
#haribote.sys里的内容会在haribote.img中的0x4200位置处开始出现（这是一种规定），而整个引导程序会从内存启动区0x8000开始执行，所以操作系统主程序会出现在内存的
#0xc200处！！！！！！！！！！！



TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del


#这部分默认执行方式是由我自己改写的，这样的话就可以不用再去输入命令删除那些庞杂的中间文件了
default :
	$(MAKE) run
	$(MAKE) src_only

#使用ccl.exe从 bootpack.c生成bookpack.gas
bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c 
	
#因为ccl.exe编译的C语言结果是gas汇编语言，这不是nask能翻译的语言，所以需要用gas2nask工具把gas文件翻译成nas这种nask能识别的文件
bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

#可以用nask工具把nas文件翻译成obj这种目标文件，这就是机器语言了，但是还不能单独执行，需要与其他文件链接才行
bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

#这个文件用汇编语言谢了一个让bootpack.c调用的函数，主要用来使CPU停止工作
#这是编写的让CPU停止工作的io_hlt函数所在的nas文件，需要把它编译成obj文件与bootpack.obj联合编译
naskfunc.obj : naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

#把bootpack.obj和naskfunc.obj联合编译成bootpack.bim，bootpack.bim是一个中间文件
# 3MB+64KB=3136KB
bootpack.bim : bootpack.obj naskfunc.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj

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
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

#ע�⣺ipl10.nas��ĳ����������̵ĵ�һ�����ģ�������������
#bootpack.c�Ǻ��Ĳ���ϵͳ�����м���Ҫ�õ�ʹ�û�����Ա�д�ĺ����ļ�naskfunc.nas����������������һ���ļ���Ȼ������asmhead.bin�����һ���ļ������֮ǰ�Ѿ�
#���ܹ���������ִ��ģʽת�����ѳ���ת�Ƶ�1M�Ժ�ĵ�ַ��ȥ�����ڴ��м�¼��Ҫ��Ϣ�ģ�haribote.sys�����haribote.sys����ļ�����������ipl10.nasд��һ���ļ�haribote.img�̵������ϣ�
#haribote.sys������ݻ���haribote.img�е�0x4200λ�ô���ʼ���֣�����һ�ֹ涨���������������������ڴ�������0x8000��ʼִ�У����Բ���ϵͳ�������������ڴ��
#0xc200������������������������

OBJS_BOOTPACK = bootpack.obj naskfunc.obj charset.obj graphic.obj desctable.obj \
		interrupt.obj fifo.obj mouse_.obj keyboard.obj memory.obj layer_manage.obj \
		window.obj timer.obj task_manage.obj file.obj console.obj

OBJS_API =	api1.obj api2.obj api3.obj api4.obj api5.obj api6.obj \
			api7.obj api8.obj api9.obj api10.obj api11.obj api12.obj \
			api13.obj api14.obj api15.obj api16.obj api17.obj api18.obj \
			api19.obj
		
		
TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GOLIB    = $(TOOLPATH)golib00.exe 
COPY     = copy
DEL      = del


#��ͨ����
#�ⲿ��Ĭ��ִ�з�ʽ�������Լ���д�ģ������Ļ��Ϳ��Բ�����ȥ��������ɾ����Щ���ӵ��м��ļ���
default :
	$(MAKE) run
	#$(MAKE) clean
	
#����һ���൱��ASCII������ļ������涨���˸��ַ��Ŷ�Ӧ������
charset.bin : charset.txt Makefile
	$(MAKEFONT) charset.txt charset.bin
	
#����Ҫ��charset.binת����objĿ���ļ�������bootpack.c����������У�bootpack.c����Ҫ�õ���Щ�ַ������������ͨ��nas�ļ�������ɣ���Ҫ����д����
charset.obj : charset.bin Makefile
	$(BIN2OBJ) charset.bin charset.obj _charset

#��bootpack.obj��naskfunc.obj�Լ�charset.obj���ϱ����bootpack.bim��bootpack.bim��һ���м��ļ�
# 3MB+64KB=3136KB
bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)

#���м��ļ�bootpack.bim����bootpack.hrb���ֻ��������ļ�
bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
	
ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst
	
#����ļ��ĳ�����Ҫ���ˣ���ȡ����״̬��ָ������ִ�е�ַ�Լ��ɻ������ת��C��������Ĺ�������Ҫ��bootpack.cһ�����
#���������ǰ��haribote.nas�ļ��������ɵ�asmhead.nas���ɵ�
asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst
	
#����ǰ��haribote.nas��������asmhead.bin������bootpack.hrbһ������haribote.sys�ļ�
haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

apilib.lib : Makefile $(OBJS_API)
	$(GOLIB) $(OBJS_API) out:apilib.lib
	
#һ��СӦ�ó���ֻ����CPUֹͣ��������
hello.hrb : hello.nas Makefile
	$(NASK)	hello.nas hello.hrb hello.lst
	
hello2.hrb : hello2.nas Makefile
	$(NASK) hello2.nas hello2.hrb hello2.lst

firstAPP.bim : firstAPP.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:firstAPP.bim map:firstAPP.map firstAPP.obj apilib.lib
	
firstAPP.hrb : firstAPP.bim Makefile
	$(BIM2HRB) firstAPP.bim firstAPP.hrb 0
		
hello3.bim : hello3.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello3.bim map:hello3.map hello3.obj apilib.lib

hello3.hrb : hello3.bim Makefile
	$(BIM2HRB) hello3.bim hello3.hrb 0
			
appwin.bim : appwin.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:appwin.bim stack:1k map:appwin.map \
		appwin.obj apilib.lib

appwin.hrb : appwin.bim Makefile
	$(BIM2HRB) appwin.bim appwin.hrb 40k
				
stars.bim : stars.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:stars.bim stack:1k map:stars.map \
		stars.obj apilib.lib

stars.hrb : stars.bim Makefile
	$(BIM2HRB) stars.bim stars.hrb 47k
					
lines.bim : lines.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:lines.bim stack:1k map:lines.map \
		lines.obj apilib.lib

lines.hrb : lines.bim Makefile
	$(BIM2HRB) lines.bim lines.hrb 48k
						
game.bim : game.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:game.bim stack:1k map:game.map \
		game.obj apilib.lib

game.hrb : game.bim Makefile
	$(BIM2HRB) game.bim game.hrb 48k
							
clock.bim : clock.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:clock.bim stack:1k map:clock.map \
		clock.obj apilib.lib

clock.hrb : clock.bim Makefile
	$(BIM2HRB) clock.bim clock.hrb 48k
					
beepdown.bim : beepdown.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:beepdown.bim stack:1k map:beepdown.map \
		beepdown.obj apilib.lib

beepdown.hrb : beepdown.bim Makefile
	$(BIM2HRB) beepdown.bim beepdown.hrb 48k
								
color1.bim : color1.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:color1.bim stack:1k map:color1.map \
		color1.obj apilib.lib

color1.hrb : color1.bim Makefile
	$(BIM2HRB) color1.bim color1.hrb 56k
										
color2.bim : color2.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:color2.bim stack:1k map:color2.map \
		color2.obj apilib.lib

color2.hrb : color2.bim Makefile
	$(BIM2HRB) color2.bim color2.hrb 56k
		
crack7.bim : crack7.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:crack7.bim stack:1k map:crack7.map crack7.obj

crack7.hrb : crack7.bim Makefile
	$(BIM2HRB) crack7.bim crack7.hrb 0k
	
haribote.img : ipl10.bin haribote.sys hello.hrb hello2.hrb firstAPP.hrb hello3.hrb \
	appwin.hrb stars.hrb lines.hrb game.hrb clock.hrb beepdown.hrb color1.hrb color2.hrb \
	crack7.hrb Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		copy from:ipl10.nas to:@: \
		copy from:make.bat to:@: \
		copy from:hello.hrb to:@: \
		copy from:hello2.hrb to:@: \
		copy from:firstAPP.hrb to:@: \
		copy from:hello3.hrb to:@: \
		copy from:appwin.hrb to:@: \
		copy from:stars.hrb to:@: \
		copy from:lines.hrb to:@: \
		copy from:game.hrb to:@: \
		copy from:clock.hrb to:@: \
		copy from:beepdown.hrb to:@: \
		copy from:color1.hrb to:@: \
		copy from:color2.hrb to:@: \
		copy from:crack7.hrb to:@: \
		imgout:haribote.img
		

		
#һ����򣬷��Ƿ������ֱ�׼���ļ����ᰴ���ַ�ʽ����
%.gas : %.c bootpack.h Makefile
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
	-$(DEL) *.hrb
	-$(DEL) *.bim
	-$(DEL) *.map
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
	-$(DEL) *.hrb
	

	
	
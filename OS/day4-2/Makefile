#ע�⣺ipl10.nas��ĳ����������̵ĵ�һ�����ģ�������������
#bootpack.c�Ǻ��Ĳ���ϵͳ�����м���Ҫ�õ�ʹ�û�����Ա�д�ĺ����ļ�naskfunc.nas����������������һ���ļ���Ȼ������asmhead.bin�����һ���ļ������֮ǰ�Ѿ�
#���ܹ������������ӻ�����Ժ�C���Ե����������ڴ��м�¼��Ҫ��Ϣ�ģ�haribote.sys�����haribote.sys����ļ�����������ipl10.nasд��һ���ļ�haribote.img�̵������ϣ�
#haribote.sys������ݻ���haribote.img�е�0x4200λ�ô���ʼ���֣�����һ�ֹ涨���������������������ڴ�������0x8000��ʼִ�У����Բ���ϵͳ�������������ڴ��
#0xc200������������������������



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


#�ⲿ��Ĭ��ִ�з�ʽ�������Լ���д�ģ������Ļ��Ϳ��Բ�����ȥ��������ɾ����Щ���ӵ��м��ļ���
default :
	$(MAKE) run
	$(MAKE) src_only

#ʹ��ccl.exe�� bootpack.c����bookpack.gas
bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c 
	
#��Ϊccl.exe�����C���Խ����gas������ԣ��ⲻ��nask�ܷ�������ԣ�������Ҫ��gas2nask���߰�gas�ļ������nas����nask��ʶ����ļ�
bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

#������nask���߰�nas�ļ������obj����Ŀ���ļ�������ǻ��������ˣ����ǻ����ܵ���ִ�У���Ҫ�������ļ����Ӳ���
bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

#����ļ��û������л��һ����bootpack.c���õĺ�������Ҫ����ʹCPUֹͣ����
#���Ǳ�д����CPUֹͣ������io_hlt�������ڵ�nas�ļ�����Ҫ���������obj�ļ���bootpack.obj���ϱ���
naskfunc.obj : naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

#��bootpack.obj��naskfunc.obj���ϱ����bootpack.bim��bootpack.bim��һ���м��ļ�
# 3MB+64KB=3136KB
bootpack.bim : bootpack.obj naskfunc.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj

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
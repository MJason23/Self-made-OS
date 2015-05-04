OBJS_API =	api1.obj api2.obj api3.obj api4.obj api5.obj api6.obj \
			api7.obj api8.obj api9.obj api10.obj api11.obj api12.obj \
			api13.obj api14.obj api15.obj api16.obj api17.obj api18.obj \
			api19.obj api20.obj api21.obj api22.obj api23.obj alloca.obj \
			api24.obj api25.obj
			
TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = ../haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GOLIB    = $(TOOLPATH)golib00.exe 
COPY     = copy
DEL      = del


default :
	$(MAKE) apilib.lib
	
apilib.lib : Makefile $(OBJS_API)
	$(GOLIB) $(OBJS_API) out:apilib.lib
	
%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

clean :
	-$(DEL) *.lst
	-$(DEL) *.obj

src_only :
	$(MAKE) clean
	-$(DEL) apilib.lib
	
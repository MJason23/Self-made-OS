TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del


default :
	$(MAKE) run_full
	$(MAKE) clean_full
	


haribote.img : haribote/ipl20.bin haribote/haribote.sys Makefile firstAPP/firstAPP.hrb \
		hello3/hello3.hrb appwin/appwin.hrb stars/stars.hrb lines/lines.hrb game/game.hrb \
	 clock/clock.hrb beepdown/beepdown.hrb color1/color1.hrb color2/color2.hrb primenum/primenum.hrb \
	 typeipl/typeipl.hrb type/type.hrb hanzi/hanzi.hrb xushuai/xushuai.txt
	 
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:haribote/ipl20.bin len:512 from:0 to:0 \
		copy from:haribote/haribote.sys to:@: \
		copy from:haribote/ipl20.nas to:@: \
		copy from:make.bat to:@: \
		copy from:firstAPP/firstAPP.hrb to:@: \
		copy from:hello3/hello3.hrb to:@: \
		copy from:appwin/appwin.hrb to:@: \
		copy from:stars/stars.hrb to:@: \
		copy from:lines/lines.hrb to:@: \
		copy from:game/game.hrb to:@: \
		copy from:clock/clock.hrb to:@: \
		copy from:beepdown/beepdown.hrb to:@: \
		copy from:color1/color1.hrb to:@: \
		copy from:color2/color2.hrb to:@: \
		copy from:primenum/primenum.hrb to:@: \
		copy from:typeipl/typeipl.hrb to:@: \
		copy from:type/type.hrb to:@: \
		copy from:hanzi/hanzi.hrb to:@: \
		copy from:xushuai/xushuai.txt to:@: \
		copy from:chineseset/hzset.fnt to:@: \
		imgout:haribote.img


run :
	$(MAKE) haribote.img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) haribote.img
	$(IMGTOL) w a: haribote.img

full :
	$(MAKE) -C haribote
	$(MAKE) -C apilib
	$(MAKE) -C firstAPP
	$(MAKE) -C hello3
	$(MAKE) -C appwin
	$(MAKE) -C stars
	$(MAKE) -C lines
	$(MAKE) -C game
	$(MAKE) -C clock
	$(MAKE) -C beepdown
	$(MAKE) -C color1
	$(MAKE) -C color2
	$(MAKE) -C primenum
	$(MAKE) -C typeipl
	$(MAKE) -C type
	$(MAKE) -C hanzi
	$(MAKE) haribote.img

run_full :
	$(MAKE) full
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install_full :
	$(MAKE) full
	$(IMGTOL) w a: haribote.img

run_os :
	$(MAKE) -C haribote
	$(MAKE) run

clean :

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

clean_full :
	$(MAKE) -C haribote		clean
	$(MAKE) -C apilib		clean
	$(MAKE) -C firstAPP		clean
	$(MAKE) -C hello3		clean
	$(MAKE) -C appwin		clean
	$(MAKE) -C stars		clean
	$(MAKE) -C lines		clean
	$(MAKE) -C game			clean
	$(MAKE) -C clock		clean
	$(MAKE) -C beepdown		clean
	$(MAKE) -C color1		clean
	$(MAKE) -C color2		clean
	$(MAKE) -C primenum		clean
	$(MAKE) -C typeipl		clean
	$(MAKE) -C type			clean
	$(MAKE) -C hanzi		clean
src_only_full :
	$(MAKE) -C haribote		src_only
	$(MAKE) -C apilib		src_only
	$(MAKE) -C firstAPP		src_only
	$(MAKE) -C hello3		src_only
	$(MAKE) -C appwin		src_only
	$(MAKE) -C stars		src_only
	$(MAKE) -C lines		src_only
	$(MAKE) -C game			src_only
	$(MAKE) -C clock		src_only
	$(MAKE) -C beepdown		src_only
	$(MAKE) -C color1		src_only
	$(MAKE) -C color2		src_only
	$(MAKE) -C primenum		src_only
	$(MAKE) -C typeipl		src_only
	$(MAKE) -C type			src_only
	$(MAKE) -C hanzi		src_only
	-$(DEL) haribote.img

refresh :
	$(MAKE) full
	$(MAKE) clean_full
	-$(DEL) haribote.img


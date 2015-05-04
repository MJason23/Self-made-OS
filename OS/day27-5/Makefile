TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

# デフォルト動作

default :
	$(MAKE) haribote.img

# ファイル生成規則

haribote.img : haribote/ipl10.bin haribote/haribote.sys Makefile firstAPP/firstAPP.hrb \
		hello3/hello3.hrb appwin/appwin.hrb stars/stars.hrb lines/lines.hrb game/game.hrb \
	 clock/clock.hrb beepdown/beepdown.hrb color1/color1.hrb color2/color2.hrb
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:haribote/ipl10.bin len:512 from:0 to:0 \
		copy from:haribote/haribote.sys to:@: \
		copy from:haribote/ipl10.nas to:@: \
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
		imgout:haribote.img

# コマンド

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
# 何もしない

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
	-$(DEL) haribote.img

refresh :
	$(MAKE) full
	$(MAKE) clean_full
	-$(DEL) haribote.img


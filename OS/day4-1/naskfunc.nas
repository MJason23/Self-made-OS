; naskfunc
; TAB=4

[FORMAT "WCOFF"]					;制作目标文件的模式
[INSTRSET "i486p"]					;默认不写这行代码的话，nask会直接把机器作为8086的老机器来运行，那样的话就不能用32位状态下的ECX之类的寄存器了，所以加上这行代码
[BITS 32]							;制作32位模式用的机械语言



;制作目标文件的信息
[FILE "naskfunc.nas"]				;源文件名信息

		GLOBAL		_io_hlt,_write_mem8			;程序中包含的函数名
		
	
;以下是实际的函数	

[SECTION .text]						;目标文件中写了这些之后再写程序

_io_hlt:							;void io_hlt(void);
		HLT
		RET
		
;当C语言中调用void write_mem8(int addr,int data)这个函数的时候实际会调用这个汇编写的函数，而C函数中的第一个参数会保存在[ESP + 4]这个内存地址中，第二个参数则保存在[ESP + 8]这个内存地址中，以后以此类推	
_write_mem8:			
		MOV		ECX,[ESP+4]
		MOV		AL,[ESP+8]
		MOV		[ECX],AL
		RET 
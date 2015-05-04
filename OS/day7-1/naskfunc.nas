;这部分程序的作用：由于操作系统经常与硬件直接接触，有些功能C语言是无法实现的，所以需要借助汇编语言来实现，下面这些就是bootpack.c中会调用到的函数（主要都是些对
;寄存器或者CPU的直接操作等等）

; naskfunc
; TAB=4

[FORMAT "WCOFF"]					;制作目标文件的模式
[INSTRSET "i486p"]
[BITS 32]							;制作32位模式用的机械语言



;制作目标文件的信息
[FILE "naskfunc.nas"]				;源文件名信息

		GLOBAL		_io_hlt,_io_cli,_io_sti,_io_stihlt			;程序中包含的函数名
		GLOBAL		_io_in8,_io_in16,_io_in32					;读取特定装置的输入
		GLOBAL		_io_out8,_io_out16,_io_out32				;设置特定装置的输出
		GLOBAL		_io_load_eflags,_io_store_eflags			;读取和写入EFLAGS寄存器的内容
		GLOBAL		_load_gdtr,_load_idtr						;初始化GDTR和IDTR两个寄存器
		GLOBAL		_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
		EXTERN		_inthandler21, _inthandler27, _inthandler2c	;这些是中断处理程序，由于它们是用C语言写得，所以在这里属于外部程序
		
	
;以下是实际的函数	

[SECTION .text]						;目标文件中写了这些之后再写程序

_io_hlt:			;void io_hlt(void);
		HLT
		RET
		
_io_cli:			;void io_cli(void);
		CLI
		RET 

_io_sti:			;void io_sti(void);
		STI
		RET 
		
_io_stihlt:			;void io_stihlt(void);
		STI
		HLT
		RET 
		
_io_in8:			;int io_in8(int port);C语言的默认规约，当在汇编语言中执行RET的时候，默认返回值存储在EAX中
		MOV		EDX,[ESP + 4]
		MOV		EAX,0
		IN		AL,DX
		RET 
		
_io_in16:			;int io_in16(int port);
		MOV		EDX,[ESP + 4]
		MOV		EAX,0
		IN		AX,DX
		RET 
		
_io_in32:			;int io_in32(int port);
		MOV		EDX,[ESP + 4]
		MOV		EAX,0
		IN		EAX,DX
		RET 
		
_io_out8:			;int io_out8(int port,int data);
		MOV		EDX,[ESP + 4]
		MOV		AL,[ESP + 8]
		OUT		DX,AL
		RET 
		
_io_out16:			;int io_out16(int port,int data);
		MOV		EDX,[ESP + 4]
		MOV		AX,[ESP + 8]
		OUT		DX,AX
		RET 
		
_io_out32:			;int io_out32(int port,int data);
		MOV		EDX,[ESP + 4]
		MOV		EAX,[ESP + 8]
		OUT		DX,EAX
		RET 
		
_io_load_eflags:	;int _io_load_eflags(void);
		PUSHFD
		POP EAX
		RET 
		
_io_store_eflags:	;void _io_store_eflags(int eflags);
		MOV		EAX,[ESP + 4]
		PUSH 	EAX
		POPFD
		RET 
		
;初始化GDTR寄存器，载入段的上限和GDT所在的内存地址
_load_gdtr:			;void load_gdtr(int limit, int addr);
		MOV		AX,[ESP + 4]		; limit
		MOV		[ESP + 6],AX
		LGDT	[ESP + 6]
		RET 

_load_idtr:			;void load_idtr(int limit, int addr);
		MOV		AX,[ESP + 4]		; limit
		MOV		[ESP + 6],AX
		LIDT	[ESP + 6]
		RET
		
_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

		
		
		
		
		
		
		
		
		
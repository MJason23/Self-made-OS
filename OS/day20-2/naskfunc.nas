;这部分程序的作用：由于操作系统经常与硬件直接接触，有些功能C语言是无法实现的，所以需要借助汇编语言来实现，下面这些就是bootpack.c中会调用到的函数（主要都是些对
;寄存器或者CPU的直接操作等等）这一部分代码不需要专门的通过某些代码来嫁接到C程序中，因为C程序会被编译成nas（汇编语言），然后再与这个汇编文件一起编译成一个整体文件
;所以这个文件里的函数可以直接由bootpack.c调用。（发现用这种C语言编译器编译出的汇编语言中，C的函数前面都会有下划线_，所以用汇编实现C函数的时候也要加_）

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
		GLOBAL		_load_gdtr,_load_idtr,_load_tr				;初始化GDTR和IDTR两个寄存器，以及TR寄存器
		GLOBAL		_asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
		GLOBAL		_load_cr0,_store_cr0,_memtest_wr
		GLOBAL		_taskswitch4,_taskswitch3,_farjmp
		GLOBAL		_asm_console_putchar, _farcall
		EXTERN		_inthandler20, _inthandler21, _inthandler27, _inthandler2c	;这些是中断处理程序，由于它们是用C语言写得，所以在这里属于外部程序
		EXTERN 		_console_putchar;
		
	
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
		MOV		EAX,0				;一会儿返回的时候需要EAX低位的数据作为返回值，所以先把整个EAX归零
		IN		AL,DX				;从[DX]端口读取数据到AL中
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
	
;初始化TR寄存器，它的作用是让CPU记住当前正在运行哪一个任务	
_load_tr:			;void load_tr(int tr);
		LTR		[ESP + 4]		;tr
		RET
		
_farjmp:			;void farjmp(int eip, int cs);
		JMP		FAR [ESP + 4]
		RET
		
_taskswitch4:		;void taskswitch4(void);
		JMP		4*8:0
		RET
		
_taskswitch3:		;void taskswitch3(void);
		JMP		3*8:0
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
		
_farcall:			;void farcall(int eip, int cs)
		CALL	FAR	[ESP + 4]
		RET
		
_asm_console_putchar:	
		STI								;由于这个函数不是真正的中断处理函数，所以不能让CPU禁止中断，只把它当做普通的函数就行了
		PUSHAD							;因为循环调用这个函数的话，很有可能会更改这些寄存器，所以先把它们保存下来
		PUSH 	1
		AND 	EAX,0xff				;将AH和EAX的高位置为0，使EAX成为已存入字符编码的状态
		PUSH 	EAX
		PUSH	DWORD [0x0fec]			;这里要给出console结构体的地址，这里暂且把它存放到BOOTINFO之前的内存中
		CALL	_console_putchar
		ADD		ESP,12					;把栈中存入的那三个参数忽略掉，直接把栈指针向后移动12字节即可
		POPAD
		IRETD				;这个是专门用于far-CALL的返回指令
		
_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
		
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

		
_load_cr0:			;int load_cr0(void);
		MOV		EAX,CR0
		RET
		
_store_cr0:			;void store_cr0(int cr0);
		MOV		EAX,[ESP + 4]
		MOV		CR0,EAX
		RET	
		
_memtest_wr:		;unsigned int memtest_wr(unsigned int start, unsigned int end);
		PUSH	EDI						; 由于还要使用EBX, ESI, EDI ，所以调用函数之前得先保存他们的值
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP + 12 + 4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP + 12 + 8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET
		
		
		
		
		
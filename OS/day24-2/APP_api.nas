[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_putchar, _api_end, _api_putstring_toend
		GLOBAL	_api_create_window, _api_putstring_onwindow, _api_drawrectangle_onwindow
		GLOBAL	_api_initmalloc, _api_malloc, _api_free
		GLOBAL	_api_drawpoint, _api_refreshwindow, _api_drawline
		GLOBAL	_api_closewindow, _api_getkey

[SECTION .text]

;显示单个字符的API调用函数
_api_putchar:				;void api_putchar(int c);
		MOV		EDX,1
		MOV		AL,[ESP + 4]			;参数“c”
		INT		0x40
		RET	

;结束应用程序的API调用函数
_api_end:					;void api_end(void);
		MOV		EDX,4
		INT		0x40
		
;显示整个字符串的API调用函数
_api_putstring_toend:		;void api_putstring_toend(char *string);
		PUSH	EBX
		MOV		EDX,2
		MOV		EBX,[ESP + 8]
		INT 	0x40
		POP		EBX
		RET	
		
;制作并显示窗口的函数
_api_create_window:			;int api_create_window(char *buffer, int length, int width, int color_luc, char *title);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,5
		MOV		EBX,[ESP + 16]		;buffer
		MOV		ESI,[ESP + 20]		;length
		MOV		EDI,[ESP + 24]		;width
		MOV		EAX,[ESP + 28]		;color_luc
		MOV		ECX,[ESP + 32]		;title
		INT 	0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET	
		
;在窗口上显示字符串的函数
_api_putstring_onwindow:		;void api_putstring_onwindow(int window, int x, int y, int color, int length, char *string);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,6
		MOV		EBX,[ESP+20]	; window
		MOV		ESI,[ESP+24]	; x
		MOV		EDI,[ESP+28]	; y
		MOV		EAX,[ESP+32]	; color
		MOV		ECX,[ESP+36]	; length
		MOV		EBP,[ESP+40]	; string
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET
		
;在窗口上绘制矩形的函数
_api_drawrectangle_onwindow:	;void api_drawrectangle_onwindow(int window, int x, int y, int length, int width, int color);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,7
		MOV		EBX,[ESP+20]	; window
		MOV		EAX,[ESP+24]	; x
		MOV		ECX,[ESP+28]	; y
		MOV		ESI,[ESP+32]	; length
		MOV		EDI,[ESP+36]	; width
		MOV		EBP,[ESP+40]	; color
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET	
		
;初始化应用程序的内存管理
_api_initmalloc:			; void api_initmalloc(void);			
		PUSH	EBX
		MOV		EDX,8
		MOV		EBX,[CS:0x0020]		; malloc内存空间的地址（这是编译器bim2hrb指定的malloc的地址存放在文件的0x0020处）
		MOV		EAX,EBX
		ADD		EAX,32*1024			; 应用程序的mem_manage所管理的内存空间的起始地址从[CS:0x0020]位置偏移32KB的位置开始，因为仅仅是内存管理mem_mamage这个结构就占用了32KB的空间
		MOV		ECX,[CS:0x0000]		; 指定的数据段的大小存放在文件的起始处，它是以4KB为单位的，所以每个文件的最开头字节肯定是0x00
		SUB		ECX,EAX				; 从应用程序数据段中减去mem_manage管理结构所占用的这部分内存空间，剩下的才是应用程序真正能用的内存
		INT		0x40
		POP		EBX
		RET

_api_malloc:		; char *api_malloc(int size);
		PUSH	EBX
		MOV		EDX,9
		MOV		EBX,[CS:0x0020]
		MOV		ECX,[ESP+8]			; size
		INT		0x40
		POP		EBX
		RET

_api_free:			; void api_free(char *addr, int size);
		PUSH	EBX
		MOV		EDX,10
		MOV		EBX,[CS:0x0020]
		MOV		EAX,[ESP+ 8]		; addr
		MOV		ECX,[ESP+12]		; size
		INT		0x40
		POP		EBX
		RET
		
_api_drawpoint:		; void api_drawpoint(int window, int x, int y, int color);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,11
		MOV		EBX,[ESP+16]	; window
		MOV		ESI,[ESP+20]	; x
		MOV		EDI,[ESP+24]	; y
		MOV		EAX,[ESP+28]	; color
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET
		
_api_refreshwindow:			; void api_refreshwindow(int window, int x0, int y0, int x1, int y1);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		EDX,12
		MOV		EBX,[ESP+16]	; window
		MOV		EAX,[ESP+20]	; x0
		MOV		ECX,[ESP+24]	; y0
		MOV		ESI,[ESP+28]	; x1
		MOV		EDI,[ESP+32]	; y1
		INT		0x40
		POP		EBX
		POP		ESI
		POP		EDI
		RET 
		
_api_drawline:				; void api_drawline(int window, int x0, int y0, int x1, int y1, int color);
		PUSH	EDI
		PUSH	ESI
		PUSH	EBP
		PUSH	EBX
		MOV		EDX,13
		MOV		EBX,[ESP+20]	; window
		MOV		EAX,[ESP+24]	; x0
		MOV		ECX,[ESP+28]	; y0
		MOV		ESI,[ESP+32]	; x1
		MOV		EDI,[ESP+36]	; y1
		MOV		EBP,[ESP+40]	; color
		INT		0x40
		POP		EBX
		POP		EBP
		POP		ESI
		POP		EDI
		RET		
		
_api_closewindow:				;void api_closewindow(int window);
		PUSH	EBX
		MOV		EDX,14
		MOV		EBX,[ESP+8]		;window
		INT		0x40
		POP		EBX
		RET	
		
_api_getkey:		; int api_getkey(int mode);
		MOV		EDX,15
		MOV		EAX,[ESP+4]	; mode
		INT		0x40
		RET	
		
		
		
		
		
		
[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_drawrectangle_onwindow
		
[SECTION .text]


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
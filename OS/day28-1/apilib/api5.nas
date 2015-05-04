[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_putstring_onwindow
		
[SECTION .text]

		
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
		
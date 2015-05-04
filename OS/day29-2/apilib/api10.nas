[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_drawpoint
		
[SECTION .text]

		
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
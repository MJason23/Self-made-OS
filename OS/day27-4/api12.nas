[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_drawline
		
[SECTION .text]

		
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
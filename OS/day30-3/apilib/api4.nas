[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_create_window
		
[SECTION .text]

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
[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_closewindow

[SECTION .text]
	
		
_api_closewindow:				;void api_closewindow(int window);
		PUSH	EBX
		MOV		EDX,14
		MOV		EBX,[ESP+8]		;window
		INT		0x40
		POP		EBX
		RET	
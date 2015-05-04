[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "alloca.nas"]				;原文件名信息

		GLOBAL	_api_fsize
		
[SECTION .text]

_api_fsize:				;void api_fsize(int handle, int mode);
		MOV		EDX,24
		MOV		EAX,[ESP+4]			; fhandle
		MOV		ECX,[ESP+8]			; mode
		INT		0x40
		RET

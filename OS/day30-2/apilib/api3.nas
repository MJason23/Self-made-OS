[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_putstring_toend
		
[SECTION .text]

;显示整个字符串的API调用函数
_api_putstring_toend:		;void api_putstring_toend(char *string);
		PUSH	EBX
		MOV		EDX,2
		MOV		EBX,[ESP + 8]
		INT 	0x40
		POP		EBX
		RET	
		
[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "alloca.nas"]				;原文件名信息

		GLOBAL	_api_fclose	
		
[SECTION .text]

_api_fclose:				;void api_fclose(int handle);
		MOV		EDX,22
		MOV		EAX,[ESP + 4]
		INT		0x40
		RET	
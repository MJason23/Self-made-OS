[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "alloca.nas"]				;原文件名信息

		GLOBAL	__alloca	
		
[SECTION .text]

;程序详解在书上P602
__alloca:
		ADD		EAX,-4
		SUB		ESP,EAX
		JMP		DWORD [ESP + EAX]
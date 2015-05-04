[FORMAT "WCOFF"]					;生成对象文件的模式
[INSTRSET "i486p"]					;表示使用486兼容指令集
[BITS 32]							;生成32位模式机器语言
[FILE "APP_api.nas"]				;原文件名信息

		GLOBAL	_api_initmalloc
		
[SECTION .text]

		
;初始化应用程序的内存管理
_api_initmalloc:			; void api_initmalloc(void);			
		PUSH	EBX
		MOV		EDX,8
		MOV		EBX,[CS:0x0020]		; malloc内存空间的地址（这是编译器bim2hrb指定的malloc的地址存放在文件的0x0020处）
		MOV		EAX,EBX
		ADD		EAX,32*1024			; 应用程序的mem_manage所管理的内存空间的起始地址从[CS:0x0020]位置偏移32KB的位置开始，因为仅仅是内存管理mem_mamage这个结构就占用了32KB的空间
		MOV		ECX,[CS:0x0000]		; 指定的数据段的大小存放在文件的起始处，它是以4KB为单位的，所以每个文件的最开头字节肯定是0x00
		SUB		ECX,EAX				; 从应用程序数据段中减去mem_manage管理结构所占用的这部分内存空间，剩下的才是应用程序真正能用的内存
		INT		0x40
		POP		EBX
		RET

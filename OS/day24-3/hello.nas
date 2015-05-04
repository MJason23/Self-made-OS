[INSTRSET "i486p"]
[BITS 32]
	MOV		ECX,msg
	MOV		EDX,1
putloop:
	MOV		AL,[CS:ECX]
	CMP		AL,0
	JE		fin
	INT 	0x40			;以中断调用的形式来调用那个asm_console_putchar函数，这样就不用随着操作系统的更改而改变应用程序的代码了
	ADD		ECX,1
	JMP		putloop
fin:
	MOV		EDX,4			;用来结束当前应用程序的调用
	INT		0X40
msg:
	DB		"hello",0
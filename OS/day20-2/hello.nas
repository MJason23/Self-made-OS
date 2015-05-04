[INSTRSET "i486p"]
[BITS 32]
	MOV		ECX,msg
putloop:
	MOV		AL,[CS:ECX]
	CMP		AL,0
	JE		fin
	INT 	0x40			;以中断调用的形式来调用那个asm_console_putchar函数，这样就不用随着操作系统的更改而改变应用程序的代码了
	ADD		ECX,1
	JMP		putloop
fin:
	RETF		;由操作系统转到执行应用程序是需要远跳转的，当跳转回去的时候同样需要远跳转，所以不能用普通的RET指令。
msg:
	DB		"hello",0
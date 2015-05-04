[BITS 32]

	MOV		AL,'h'
	INT 	0x40
	MOV		AL,'e'
	INT 	0x40
	MOV		AL,'l'
	INT 	0x40
	MOV		AL,'l'
	INT 	0x40
	MOV		AL,'o'
	INT 	0x40
	RETF		;由操作系统转到执行应用程序是需要远跳转的，当跳转回去的时候同样需要远跳转，所以不能用普通的RET指令。

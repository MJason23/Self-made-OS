[INSTRSET "i486p"]
[BITS 32]

;企图通过修改DS寄存器而达到可以操作系统的破坏行为
		MOV		EAX,1*8
		MOV		DS,AX
		MOV		BYTE[0X00102600], 0
		MOV		EDX,4
		INT		0X40
		
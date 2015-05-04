[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api026.nas"]

		GLOBAL	_api_getlan

[SECTION .text]

_api_getlan:		;  		int api_getlan(void);
		MOV		EDX,27
		INT		0x40
		RET	

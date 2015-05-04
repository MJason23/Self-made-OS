; hello-os
; TAB=4

		ORG		0x7c00			; 指明程序的装载地址

; 以下这段代码是标准FAT12格式软盘专用的代码

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"		; 启动区的名称可以是任意的字符串（8字节）
		DW		512				; 每个扇区（sector）的大小（必须为512字节）
		DB		1				; 簇（cluster）的大小（必须为1个扇区）
		DW		1				; FAT的起始位置（一般从第一个扇区开始）
		DB		2				; FAT的个数（必须为2）
		DW		224				; 根目录的大小（一般设成224项）
		DW		2880			; 该磁盘的大小（必须是2880扇区，因为有上下两面，每个扇区都是512字节，正好1.44M）
		DB		0xf0			; 磁盘的种类（必须是0xf0，猜想这是软盘的格式代码，需要上网查证）
		DW		9				; FAT的长度（必须是9扇区）
		DW		18				; 1个磁道（track）有几个扇区（必须是18）
		DW		2				; 磁头数（必须是2）
		DD		0				; 不用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明，固定
		DD		0xffffffff		; （可能是）卷标号码
		DB		"HELLO-OS   "	; 磁盘的名称（11字节，不够的话需要补空格）
		DB		"FAT12   "		; 磁盘格式名称（8字节）
		RESB	18				; 先空出18字节

; 程序核心，通过初始化寄存器，然后用SI寄存器不停地增加，然后把msg段的信息用putloop里设定好的格式显示到屏幕上

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		MOV		ES,AX

		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT						; 让CPU停止，等待指令
		JMP		fin				; 无限循环

		
;信息显示部分
msg:
		DB		0x0a, 0x0a		; 2个换行
		DB		"hello, world"  ;屏幕上要显示的信息
		DB		0x0a			; 换行
		DB		0

		RESB	0x7dfe-$		; 重复填充0直到0x7dfe位置的内存，-$代表当前位置的内存位置

		DB		0x55, 0xaa      ;只有在启动区的最后一个字节写入0x55aa，bios才会认为这是启动盘

;这个文件的主要工作就是：引导！！首先BIOS把0柱面0磁头1扇区的内容读到内存中，而这个扇区写了如何加载后续扇区的代码！！（把装有操作系统程序从软盘或者硬盘中读取到
;内存中的特定位置，然后把CPU设置到程序在内存中的起始位置，这样就可以正式开始运行操作系统了）记住了，这个文件整个都保存在软盘的第一扇区中！！总共代码只有512字节

; haribote-ipl
; TAB=4

;这次只是做了读取多个后续软盘多个扇区的内容到内存中，为以后做完整操作系统的引导打下基础，虽然，我们并没有在后面的软盘上写什么内容，也就是说当前读到内存中的都是些垃圾
;但是这是必要的过程！！这次的成果显示与上次是一样的。

CYLS	EQU		20				;表示读取二十个柱面的信息

		ORG		0x7c00			; 指明程序的装载地址

; 以下这段代码是标准FAT12格式软盘专用的代码

		JMP		entry
		DB		0x90
		DB		"Haribote"		; 启动区的名称可以是任意的字符串（8字节）
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
		DB		"HariboteOS "	; 磁盘的名称（11字节，不够的话需要补空格）
		DB		"FAT12   "		; 磁盘格式名称（8字节）
		RESB	18				; 先空出18字节

; 程序核心，通过初始化寄存器，然后用SI寄存器不停地增加，然后把msg段的信息用putloop里设定好的格式显示到屏幕上

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		
;day3添加的代码

		MOV		AX,0x0820		;要把程序加载到这个内存位置，仅仅是因为这块儿区域没人用而已，没什么特别含义
		MOV		ES,AX
		MOV 	CH,0			;柱面0
		MOV 	DH,0			;磁头0
		MOV 	CL,2			;扇区2
		
readloop:
		MOV  	SI,0 			;记录失败次数的寄存器（因为软盘很不可靠，容易读取失败，所以设计成自动重复启动）
		
		
;如果加载失败，则还要重新加载系统，所以加载的这一部分成为了独立的一段
retry:
		MOV 	AH,0x02			;AH=0x02：读盘
		MOV 	AL,1			;1个扇区
		MOV 	BX,0
		MOV 	DL,0x00			;A驱动器
		INT 	0x13			;调用磁盘BIOS，前面的这些设置仅仅是为了进入中断前的准备，每个寄存器在终端程序中都对应着特殊含义，具体可查看书本P46
		JNC		next			;没出错的话直接跳转到next段
		ADD		SI,1			;出错的话，先对SI加1，表示出错次数加1
		CMP		SI,5			;发生错误的次数超过5次就不在继续加载了，直接跳转到error段，进行错误显示
		JAE		error			;比较结果，如果大于5就跳转到error段
		MOV		AH,0x00			;当AH=0的时候代表复位磁盘系统
		MOV		DL,0x00			;A驱动器
		INT 	0x13			;重置驱动器（这里的中断只是重置驱动器，不是重载，看清楚寄存器的值了，不是为了读取磁盘而设置的数值）
		JMP		retry			;重置驱动器后跳回到retry段，重新开始加载
		
		
;从next这一段开始读取从第三个扇区到第10个柱面的最后一个扇区
next:
		MOV		AX,ES			;把内存地址后移0x200（其实就是512个字节，即一个扇区的大小）
		ADD		AX,0x0020
		MOV		ES,AX			;因为不能直接对ES寄存器进行ADD操作，所以需要用AX寄存器来做中介
		ADD		CL,1			;要对当前柱面的下一个扇区继续读取，所以CL需要加1
		CMP		CL,18			;因为一个柱面有18个扇区，所以需要与18进行比较
		JBE		readloop		;如果比较结果发现比18小或者等于，则说明这个柱面的扇区还没有读取完，如果读取完了，就继续向下进行，即读取另一个磁头的柱面
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop			;如果DH<2，则说明反面磁头还木有读取完，继续返回到readloop读取，如果读取完了，则需要返回到正面磁头，读取下一个柱面的信息
		MOV		DH,0
		ADD		CH,1			;读取下一个柱面的信息
		CMP		CH,CYLS			;CYLS是宏定义数值10，也就是比较当前是否读到了第10个柱面
		JB		readloop		;如果CH<10，则说明还没有读到第10个柱面，那么返回到readloop继续读取
		
		MOV		[0x0ff0],CH		;把CYLS的值存储到内存的这个位置，即把磁盘装在内容的结束地址告诉haribote.sys。提示一下：这一段时自由内存区
		JMP		0xc200			;跳转到真正的操作系统区去执行那里的程序，因为该部分程序被加载到了内存的0xc200；这一步是重要的嫁接，从启动转入到了真正的操作系统程序中

;等信息显示完毕之后（如果没有要显示的信息，则直接跳到这里使CPU等待）
fin:
		HLT						; 让CPU停止，等待指令
		JMP		fin				; 无限循环
		
error:
		MOV		SI,msg
		
		
;这一段代码是为了以一定格式显示信息，day3中如果一切运行正常的话，就不需要putloop这部分代码了其实
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop

		
;信息显示部分，在day3中不再显示hello world，而是当程序出现错误的时候显示load error，程序正确的时候不显示信息
msg:
		DB		0x0a, 0x0a		; 2个换行
		DB		"load error"    ;屏幕上要显示的信息
		DB		0x0a			; 换行
		DB		0

		RESB	0x7dfe-$		; 重复填充0直到0x7dfe位置的内存，-$代表当前位置的内存位置

		DB		0x55, 0xaa      ;只有在启动区的最后一个字节写入0x55aa，bios才会认为这是启动盘

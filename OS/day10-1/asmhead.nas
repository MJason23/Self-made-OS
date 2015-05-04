;其实，执行完ipl中的启动程序之后，跳转到了0xc200内存地址去执行，那个地址处装载的就是这部分程序了，真正的操作系统是在bootpack之后的那段程序，那段程序
;需要经过编译变成汇编语言并被添加到bootpack标签之后，然后生成的整个文件haribote.sys，最终联合ipl10.bin被编译成了haribote.img，但是haribote.sys被加载
;到内存时，是在0xc200处！！（因为此时是处于实模式下的，没有所谓的GDT和分段，所以需要直接指定好加载地址；而编译器计算地址偏移量是从0x0000开始的，所以
;偏移量就是实际地址）
;这部分代码是用来执行模式转换，把程序转移到1M以后的地址中去和向内存中记录重要信息的

; haribote-os boot asm
; TAB=4


BOTPAK	EQU		0x00280000	
DSKCAC	EQU		0x00100000	
DSKCAC0	EQU		0x00008000	


;有关BOOT_INFO的一些信息设定，保留这些信息是因为以后还有可能切换到别的模式之下，所以需要先保存这些信息到内存中以备后面用！
;下面定义的这些内存地址都是自由内存区域，可以随便使用
CYLS	EQU		0x0ff0			;这个地址记录了读取多少柱面到内存中，这个地址只占一个字节
LEDS	EQU		0x0ff1			;关于键盘的状态位都存储在这个地址中，只占一个字节，每一位都代表一个标志位
VMODE	EQU		0x0ff2			;关于颜色数目的信息，还有颜色的位数
SCRNX	EQU		0x0ff4			;屏幕横轴方向的分辨率
SCRNY	EQU		0x0ff6			;屏幕竖轴方向的分辨率
VRAM	EQU		0x0ff8			;显存内存区的开始地址

		ORG		0xc200			;这部分程序会被加载到0xc200这个内存地址中，由编译器编译好的，到时候执行时，会自动被加载到这个地址并且来这个位置执行！（这一部分属于个人猜想）
								;由于从磁盘中加载的内容（haribote.img）加载到了0x8000处，而从haribote.img的0x4200处才是内容区域，所以计算后得出内存中的内容地址为0xc200
		MOV		AL,0x13			;VGA显卡，320X200X8位彩色
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8			;记录画面模式，把这些记录存储到这三个地址中
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000			;0xa0000开始代表的是显示内存区，把这个开始地址存储到0x0ff8开始的四个字节中
		
		;用BIOS去的键盘上各种LED指示灯的状态
		MOV		AH,0x02			;AH是此中断的入口参数
		INT		0x16			;读取键盘标志的中断
		MOV		[LEDS],AL		;AL是此中断的出口参数，各种LED指示灯的状态标志会存到这个寄存器中，只需要一个字节即可

; PIC关闭一切中断
;	根据AT兼容机的规格，如果要初始化PIC，必须在CLI之前进行，否则有时会挂起。随后进行PIC的初始化。
		;这段程序就是禁止主/从PIC的全部中断
		MOV		AL,0xff
		OUT		0x21,AL			;禁止主PIC的全部中断
		NOP						; 如果连续执行OUT指令，有些机种会无法正常运行
		OUT		0xa1,AL			;禁止从PIC的全部中断

		CLI						; 禁止CPU级别的中断

; 为了让CPU能够访问1MB以上的内存空间，设定A20GATE
		;这次输出0xdf是让A20GATE信号线变成ON的状态（有点儿类似于鼠标似的，需要激活才能使用），这样就可以访问1MB以上的内存了。书上P154有详解
		CALL	waitkbdout		;这个函数类似于wait_KBC_sendready函数
		MOV		AL,0xd1
		OUT		0x64,AL			;向键盘控制电路说明要进行模式设定，下面的那个OUT实现的是模式设定。实现了将A20GATE信号电路置为ON
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20，指定键盘控制电路附属端口输出0xdf，这个附属端口连接着主板上的很多地方，通过这个端口可以实现各种各样的控制功能
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式

[INSTRSET "i486p"]				; “想要使用486指令”的叙述，这个指令是为了能够使用386以后的LGDT，EAX，CR0等关键字

		LGDT	[GDTR0]			; 设定临时GDT（因为现在还没有初始化GDT呢，所以只能先设置一个临时的先用着）
		MOV		EAX,CR0			;CR0寄存器中的值设置完成就完成了模式转换（比想象中的轻松很多啊）；这个寄存器很重要，而且只有操作系统才能用它
		AND		EAX,0x7fffffff	; 设置bit31为0（为了禁止颁）
		OR		EAX,0x00000001	; 设置bit0为1（为了切换到保护模式）
		MOV		CR0,EAX
		JMP		pipelineflush	;从实模式进入保护模式的时候，机器语言的解释要发生变化，CPU为了加快指令的执行速度使用了管道这一机制（即执行指令的同时读取下一条指令）
pipelineflush:					;由于模式的转变，下一条指令就需要重新解释一遍，所以需要对各种寄存起重新调整，从新解释命令，所以加入了JMP指令
		MOV		AX,1*8			;  可读写的段  32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack的转移

		MOV		ESI,bootpack	; 转移源地址，这里指的是本文件最后的标签的地址，因为从这里开始就是真正的OS程序了
		MOV		EDI,BOTPAK		; 转移目的地址0x00280000，这就是GDT中的第二段的基址，需要把真正的OS的程序移动到这里执行
		MOV		ECX,512*1024/4	;由于转移的时候用到的是EAX寄存器，一次可以转移4字节的数据，所以这里需要除以4
		CALL	memcpy			;这个就是实际的内存复制函数了，上面的只是参数设定罢了

; 硬盘数据最终转移到它本来的位置去

; 首先从启动区开始

		MOV		ESI,0x7c00		; 转移源地址，源地址就是最初的前1M内存中引导程序启动区的地址
		MOV		EDI,DSKCAC		; 转移目的地址，把整个磁盘的数据移动到0x00100000开始的内存中去
		MOV		ECX,512/4		;启动区只有512字节
		CALL	memcpy

; 所有剩下的数据

		MOV		ESI,DSKCAC0+512	; 转移源地址
		MOV		EDI,DSKCAC+512	; 转移目的地址
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 除了启动区磁盘中剩下的所有数据
		SUB		ECX,512/4		; 上面那一行计算的是整个从磁盘读入的数据量，还需要减去启动区的数据量512字节
		CALL	memcpy

; 必须由asmhead来完成的工作至此其实已经全部完毕了
;	以后就交由bootpack来完成

; bootpack的启动
		;这段程序把bootpack.hrb特定字节的一定数量的数据复制到0x00310000号内存地址去（具体为什么，以后才知道）
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有要转移的数据时跳转
		MOV		ESI,[EBX+20]	; 转移源地址
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 转移目的地址
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 栈初始值
		JMP		DWORD 2*8:0x0000001b		;将2*8代入到CS里，同时移动到0x1b号内存，这里的0x1b号地址是指第2个段的0x1b号地址

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		 AL,0x60		;就比wait_KBC_sendready函数多了这一步，这一步只是用来把控制器中累积的数据（如果有的话）读取出来
		JNZ		waitkbdout		; AND的结果如果不是0，就跳转到waitkbdout
		RET

memcpy:			;内存复制程序，很容易读懂
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 结果不是0，九就跳转到memcpy继续执行
		RET

		ALIGNB	16				;这个命令是一直添加DB0直到时机合适为止，这里的时机合适是指地址能被16整除的时候
GDT0:			;这段程序创建一个临时GDT表，有两个描述符可用
		RESB	8				
		DW		0xffff,0x0000,0x9200,0x00cf	; 可以读写的段32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可以执行的段32bit（bootpack用）

		DW		0
GDTR0:			;把临时GDT0的地址写入到这个GDTR0中去
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:


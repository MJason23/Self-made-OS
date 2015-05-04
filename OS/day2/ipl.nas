; hello-os
; TAB=4

		ORG		0x7c00			; ָ�������װ�ص�ַ

; ������δ����Ǳ�׼FAT12��ʽ����ר�õĴ���

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"		; �����������ƿ�����������ַ�����8�ֽڣ�
		DW		512				; ÿ��������sector���Ĵ�С������Ϊ512�ֽڣ�
		DB		1				; �أ�cluster���Ĵ�С������Ϊ1��������
		DW		1				; FAT����ʼλ�ã�һ��ӵ�һ��������ʼ��
		DB		2				; FAT�ĸ���������Ϊ2��
		DW		224				; ��Ŀ¼�Ĵ�С��һ�����224�
		DW		2880			; �ô��̵Ĵ�С��������2880��������Ϊ���������棬ÿ����������512�ֽڣ�����1.44M��
		DB		0xf0			; ���̵����ࣨ������0xf0�������������̵ĸ�ʽ���룬��Ҫ������֤��
		DW		9				; FAT�ĳ��ȣ�������9������
		DW		18				; 1���ŵ���track���м���������������18��
		DW		2				; ��ͷ����������2��
		DD		0				; ���÷�����������0
		DD		2880			; ��дһ�δ��̴�С
		DB		0,0,0x29		; ���岻�����̶�
		DD		0xffffffff		; �������ǣ��������
		DB		"HELLO-OS   "	; ���̵����ƣ�11�ֽڣ������Ļ���Ҫ���ո�
		DB		"FAT12   "		; ���̸�ʽ���ƣ�8�ֽڣ�
		RESB	18				; �ȿճ�18�ֽ�

; ������ģ�ͨ����ʼ���Ĵ�����Ȼ����SI�Ĵ�����ͣ�����ӣ�Ȼ���msg�ε���Ϣ��putloop���趨�õĸ�ʽ��ʾ����Ļ��

entry:
		MOV		AX,0			; ��ʼ���Ĵ���
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		MOV		ES,AX

		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI��1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; ��ʾһ������
		MOV		BX,15			; ָ���ַ���ɫ
		INT		0x10			; �����Կ�BIOS
		JMP		putloop
fin:
		HLT						; ��CPUֹͣ���ȴ�ָ��
		JMP		fin				; ����ѭ��

		
;��Ϣ��ʾ����
msg:
		DB		0x0a, 0x0a		; 2������
		DB		"hello, world"  ;��Ļ��Ҫ��ʾ����Ϣ
		DB		0x0a			; ����
		DB		0

		RESB	0x7dfe-$		; �ظ����0ֱ��0x7dfeλ�õ��ڴ棬-$������ǰλ�õ��ڴ�λ��

		DB		0x55, 0xaa      ;ֻ���������������һ���ֽ�д��0x55aa��bios�Ż���Ϊ����������
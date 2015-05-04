;����ļ�����Ҫ�������ǣ�������������BIOS��0����0��ͷ1���������ݶ����ڴ��У����������д����μ��غ��������Ĵ��룡������װ�в���ϵͳ��������̻���Ӳ���ж�ȡ��
;�ڴ��е��ض�λ�ã�Ȼ���CPU���õ��������ڴ��е���ʼλ�ã������Ϳ�����ʽ��ʼ���в���ϵͳ�ˣ���ס�ˣ�����ļ����������������̵ĵ�һ�����У����ܹ�����ֻ��512�ֽ�

; haribote-ipl
; TAB=4

;���ֻ�����˶�ȡ����������̶�����������ݵ��ڴ��У�Ϊ�Ժ�����������ϵͳ���������»�������Ȼ�����ǲ�û���ں����������дʲô���ݣ�Ҳ����˵��ǰ�����ڴ��еĶ���Щ����
;�������Ǳ�Ҫ�Ĺ��̣�����εĳɹ���ʾ���ϴ���һ���ġ�

CYLS	EQU		20				;��ʾ��ȡ��ʮ���������Ϣ

		ORG		0x7c00			; ָ�������װ�ص�ַ

; ������δ����Ǳ�׼FAT12��ʽ����ר�õĴ���

		JMP		entry
		DB		0x90
		DB		"Haribote"		; �����������ƿ�����������ַ�����8�ֽڣ�
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
		DB		"HariboteOS "	; ���̵����ƣ�11�ֽڣ������Ļ���Ҫ���ո�
		DB		"FAT12   "		; ���̸�ʽ���ƣ�8�ֽڣ�
		RESB	18				; �ȿճ�18�ֽ�

; ������ģ�ͨ����ʼ���Ĵ�����Ȼ����SI�Ĵ�����ͣ�����ӣ�Ȼ���msg�ε���Ϣ��putloop���趨�õĸ�ʽ��ʾ����Ļ��

entry:
		MOV		AX,0			; ��ʼ���Ĵ���
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		
;day3���ӵĴ���

		MOV		AX,0x0820		;Ҫ�ѳ�����ص�����ڴ�λ�ã���������Ϊ��������û���ö��ѣ�ûʲô�ر���
		MOV		ES,AX
		MOV 	CH,0			;����0
		MOV 	DH,0			;��ͷ0
		MOV 	CL,2			;����2
		
readloop:
		MOV  	SI,0 			;��¼ʧ�ܴ����ļĴ�������Ϊ���̺ܲ��ɿ������׶�ȡʧ�ܣ�������Ƴ��Զ��ظ�������
		
		
;�������ʧ�ܣ���Ҫ���¼���ϵͳ�����Լ��ص���һ���ֳ�Ϊ�˶�����һ��
retry:
		MOV 	AH,0x02			;AH=0x02������
		MOV 	AL,1			;1������
		MOV 	BX,0
		MOV 	DL,0x00			;A������
		INT 	0x13			;���ô���BIOS��ǰ�����Щ���ý�����Ϊ�˽����ж�ǰ��׼����ÿ���Ĵ������ն˳����ж���Ӧ�����⺬�壬����ɲ鿴�鱾P46
		JNC		next			;û�����Ļ�ֱ����ת��next��
		ADD		SI,1			;�����Ļ����ȶ�SI��1����ʾ����������1
		CMP		SI,5			;��������Ĵ�������5�ξͲ��ڼ��������ˣ�ֱ����ת��error�Σ����д�����ʾ
		JAE		error			;�ȽϽ�����������5����ת��error��
		MOV		AH,0x00			;��AH=0��ʱ�������λ����ϵͳ
		MOV		DL,0x00			;A������
		INT 	0x13			;������������������ж�ֻ���������������������أ�������Ĵ�����ֵ�ˣ�����Ϊ�˶�ȡ���̶����õ���ֵ��
		JMP		retry			;���������������ص�retry�Σ����¿�ʼ����
		
		
;��next��һ�ο�ʼ��ȡ�ӵ�������������10����������һ������
next:
		MOV		AX,ES			;���ڴ��ַ����0x200����ʵ����512���ֽڣ���һ�������Ĵ�С��
		ADD		AX,0x0020
		MOV		ES,AX			;��Ϊ����ֱ�Ӷ�ES�Ĵ�������ADD������������Ҫ��AX�Ĵ��������н�
		ADD		CL,1			;Ҫ�Ե�ǰ�������һ������������ȡ������CL��Ҫ��1
		CMP		CL,18			;��Ϊһ��������18��������������Ҫ��18���бȽ�
		JBE		readloop		;����ȽϽ�����ֱ�18С���ߵ��ڣ���˵����������������û�ж�ȡ�꣬�����ȡ���ˣ��ͼ������½��У�����ȡ��һ����ͷ������
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop			;���DH<2����˵�������ͷ��ľ�ж�ȡ�꣬�������ص�readloop��ȡ�������ȡ���ˣ�����Ҫ���ص������ͷ����ȡ��һ���������Ϣ
		MOV		DH,0
		ADD		CH,1			;��ȡ��һ���������Ϣ
		CMP		CH,CYLS			;CYLS�Ǻ궨����ֵ10��Ҳ���ǱȽϵ�ǰ�Ƿ�����˵�10������
		JB		readloop		;���CH<10����˵����û�ж�����10�����棬��ô���ص�readloop������ȡ
		
		MOV		[0x0ff0],CH		;��CYLS��ֵ�洢���ڴ�����λ�ã����Ѵ���װ�����ݵĽ�����ַ����haribote.sys����ʾһ�£���һ��ʱ�����ڴ���
		JMP		0xc200			;��ת�������Ĳ���ϵͳ��ȥִ������ĳ�����Ϊ�ò��ֳ��򱻼��ص����ڴ��0xc200����һ������Ҫ�ļ޽ӣ�������ת�뵽�������Ĳ���ϵͳ������

;����Ϣ��ʾ���֮�����û��Ҫ��ʾ����Ϣ����ֱ����������ʹCPU�ȴ���
fin:
		HLT						; ��CPUֹͣ���ȴ�ָ��
		JMP		fin				; ����ѭ��
		
error:
		MOV		SI,msg
		
		
;��һ�δ�����Ϊ����һ����ʽ��ʾ��Ϣ��day3�����һ�����������Ļ����Ͳ���Ҫputloop�ⲿ�ִ�������ʵ
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI��1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; ��ʾһ������
		MOV		BX,15			; ָ���ַ���ɫ
		INT		0x10			; �����Կ�BIOS
		JMP		putloop

		
;��Ϣ��ʾ���֣���day3�в�����ʾhello world�����ǵ�������ִ����ʱ����ʾload error��������ȷ��ʱ����ʾ��Ϣ
msg:
		DB		0x0a, 0x0a		; 2������
		DB		"load error"    ;��Ļ��Ҫ��ʾ����Ϣ
		DB		0x0a			; ����
		DB		0

		RESB	0x7dfe-$		; �ظ����0ֱ��0x7dfeλ�õ��ڴ棬-$������ǰλ�õ��ڴ�λ��

		DB		0x55, 0xaa      ;ֻ���������������һ���ֽ�д��0x55aa��bios�Ż���Ϊ����������

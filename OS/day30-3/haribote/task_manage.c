#include "bootpack.h"
#include <string.h>

TIMER *task_timer;		//�����л�ʱ�õĶ�ʱ��
TASKCTL *taskctl;		//����������壨����ṹ�൱�󰡣�

//�������ڵ������ַ
TASK *task_now(void)
{
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	return tl->tasks_lv[tl->task_now];
}

//���������Լ���level����Ӧ�ĵȼ����������
void task_add(TASK *task)
{
	/*�ҵ��������ǰ�������ڵĵȼ���Ȼ��������ȼ�����һ��ָ��ָ��������������״̬����Ϊ����̬*/
	TASKLEVEL *tl = &taskctl->task_level[task->level];
	tl->tasks_lv[tl->running_number] = task;
	tl->running_number++;
	task->flags = RUNNING; 
	return;
}

/*�ҵ��������ǰ�������ڵĵȼ���Ȼ��������ȼ�����һ��ָ��ָ��������������״̬����Ϊ����̬*/
void task_remove(TASK *task)
{
	int i;
	TASKLEVEL *tl = &taskctl->task_level[task->level];

	/*Ѱ��task���ڵ�λ�ã�Ȼ���task��״̬����Ϊ˯�߾���*/
	for (i = 0; i < tl->running_number; i++) 
	{
		if (tl->tasks_lv[i] == task) 
		{
			break;
		}
	}
	/*���������ڵĵȼ���running_number��1�����Ұ�task�����������ǰ�ƶ�*/
	tl->running_number--;
	if (i < tl->task_now)		//�����������λ���ڵ�ǰ���е�����ǰ�棬����Ҫ��task_now��1����ָ��ǰ��
	{
		tl->task_now--; 
	}
	if (tl->task_now >= tl->running_number) 				//�����¼��ǰ�ȼ������ŵ����������ı������ٺ��task_now��С������Ҫ������
	{
		tl->task_now = 0;
	}
	task->flags = SLEEP;

	for (; i < tl->running_number; i++) 
	{
		tl->tasks_lv[i] = tl->tasks_lv[i + 1];
	}

	return;
}

//�������˵�����������ҵ���ǰ��߼���level�ģ����ҵ�lv_now
void level_switch(void)
{
	int i;
	/* ��ԏ�̃��x����T�� */
	for (i = 0; i < MAX_TASKLEVELS; i++) 
	{
		//����ߵȼ���ʼ������ֻҪ�ҵ���һ���ȼ�����������ھ���
		if (taskctl->task_level[i].running_number > 0) 
		{
			break;
		}
	}
	taskctl->lv_now = i;
	taskctl->lv_change = 0;
	return;
}

//����ǲ���ϵͳ��ͼ���������Զ���ڣ������������������񶼲�������ʱ��ִ�����HLTָ�����
void task_idle(void)
{
	for (;;) {
		io_hlt();
	}
}

//�����������ĳ�ʼ������Ϊ��������������Ǹ��������һ������״̬�Σ�
TASK *task_init(MEMMANAGE *memman)
{
	int i;
	TASK *task, *idle;
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) ADR_GDT;

	taskctl = (TASKCTL *) memmanage_alloc_4K(memman, sizeof (TASKCTL));
	//��taskctl���г�ʼ��
	for (i = 0; i < MAX_TASKS; i++) 
	{
		taskctl->tasks[i].flags = UNUSED;
		taskctl->tasks[i].selector = (TASK_GDT0 + i) * 8;		//Ϊ��������������״̬�����ڵ�GDT������
		taskctl->tasks[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;						
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks[i].tss, AR_TSS32);				//TSS�δ�3��1002��������
		set_segmdesc(gdt + TASK_GDT0 + MAX_TASKS + i, 15, (int) &taskctl->tasks[i].ldt, AR_LDT);	//LDT�δ�1003��2002��������
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) 
	{
		taskctl->task_level[i].running_number = 0;
		taskctl->task_level[i].task_now = 0;
	}

	/*Ϊ���������ʼ�������ĳ������һ��������������*/
	/*Ϊ���������ʼ�������ĳ������һ��������������*/
	task = task_alloc();				
	task->flags = RUNNING;				//�������ǰ����������
	task->priority = 2;					//��������ȼ�Ĭ�ϵĶ���2��������ռ��CPU0.02��
	task->level = 0;					//�����������������Ĭ���趨Ϊ��߼�0��
	task_add(task);						//�Զ�����task��level��ӵ���Ӧ�ȼ����������
	level_switch();						//��������������ӽ�����������Ҫ�ı䵱ǰ��lv_now
	load_tr(task->selector);			//��дTR�Ĵ����������洢��ǰ�������Ķ�ƫ��ֵ
	task_timer = timer_alloc();			//����һ�����������л��ļ�ʱ��
	timer_set(task_timer, task->priority);			//0.02���л�һ������
	/*����������ϵͳһ����ͼ�������������û�������Ҽ��������������ߵ�ʱ��Ϳ���ת�����������
	**�����������Զ����
	*/
	idle = task_alloc();
	idle->tss.esp = memmanage_alloc_4K(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);

	return task;
}

//Ϊĳ�������������
TASK *task_alloc(void)
{
	int i;
	TASK *task;
	for (i = 0; i < MAX_TASKS; i++) 
	{
		if (taskctl->tasks[i].flags == UNUSED) 
		{
			//����û�ж�esp��eip�ȼĴ�����ֵ����Ҫ�ڽ������task��ʱ����ݾ��������ֵ
			task = &taskctl->tasks[i];
			task->flags = SLEEP;		//��������״̬�����񲻻ᱻ���䵽CPU�����ǲ��ܱ�������µ�����ʹ�ã�����ʱ���ܱ����Ѷ���Ϊ����״̬
			task->tss.eflags = 0x00000202; /* IF = 1;���е�����Ĭ�϶��ǿ����жϵ� */
			task->tss.eax = 0; 			//��������Ϊ0
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.iomap = 0x40000000;
			task->tss.ss0 = 0;			//����û�����е�ʱ��ss0ȫ�����ó�0
			return task;
		}
	}
	return 0;		//ȫ������ʹ�õ�����ֻ�ܷ���0
}

//�ó���ʼ���У������Ӹշ����������ʹ�俪ʼִ�У������Ѿ����ߵ�����ָ����У�
void task_run(TASK *task, int level, int priority)
{
	if (level < 0) 			//������õ�levelΪ������˵������������level�ı䣬��ô����ıȽϿ϶���ͬ��
	{
		level = task->level;
	}
	if (priority > 0)
	{
		task->priority = priority;
	}

	//����������������״̬������������Ч��level������Ҫ�Ȱ�����ԭ��level���Ƴ���ȥ
	if (task->flags == RUNNING && task->level != level) 			//�����˯��״̬���ѣ��������level��ֵ�����������Լ��������Ӧlevel��
	{
		task_remove(task);
	}
	if (task->flags != RUNNING) 
	{
		task->level = level;
		task_add(task);
	}

	taskctl->lv_change = 1;			//��Ϊ����Ӧ�����level�ı䣬�´�ת�������ʱ�����Ҫ���level�ȼ��ˣ���Ϊ�п�����Ҫ�����и��ߵȼ�������
	return;
}

//��������
void task_sleep(TASK *task)
{
	TASK *now_task;
	if (task->flags == RUNNING) 
	{
		now_task = task_now();
		task_remove(task);		//ִ�����д���֮��ֻ�ǰ�task��level���Ƴ��ˣ�����flags���SLEEP״̬������task����û�б��ͷ�
		if (task == now_task)
		{
			/*������Լ����Լ����ߵ��������Ҫ����ת��*/
			level_switch();
			now_task = task_now(); 
			farjmp(0, now_task->selector);
		}
	}
	return;
}

//�����л�����Ҫ�Ǹ�timer���жϴ��������õ�
void task_switch(void)
{
	TASKLEVEL *tl = &taskctl->task_level[taskctl->lv_now];
	TASK *new_task, *now_task = tl->tasks_lv[tl->task_now];
	tl->task_now++;
	//�Ȱ��������ڵĵ�ǰlevel�е����������
	if (tl->task_now == tl->running_number)		//���running_number�����һ���ˣ������»ص���һ��
	{
		tl->task_now = 0;
	}
	if (taskctl->lv_change != 0)				//�����Ҫ�ı�level�����
	{
		level_switch();						//�ҳ���ǰ����߼�level
		tl = &taskctl->task_level[taskctl->lv_now];
	}
	//�ҳ�Ҫת�������Ǹ����񣬲������ú�timer��Ȼ��ִ��������ת
	new_task = tl->tasks_lv[tl->task_now];
	timer_set(task_timer, new_task->priority);
	if (new_task != now_task) 		//���ת������֮���ֻ��ǵ�ǰ���񣬾Ͳ�����ת��
	{
		farjmp(0, new_task->selector);
	}
	return;
}

void task_b_main(LAYER *layer)
{
	FIFO fifo;
	TIMER *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	init_fifo(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_set(timer_1s, 100);

	for (;;) {
		count++;
		io_cli();
		if (fifo_status(&fifo) == 0) 
		{
			io_sti();
		} 
		else 
		{
			i = fifo_get(&fifo);
			io_sti();
			if (i == 100)
			{
				sprintf(s, "%11d", count - count0);
				displayStrings_atLayer(layer, 24, 28, COL8_000000, COL8_C6C6C6, s);
				count0 = count;
				timer_set(timer_1s, 100);
			}
		}
	}
}















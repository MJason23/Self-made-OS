int api_alloctimer(void);
void api_inittimer(int timer, int data);
void api_settimer(int timer, int time);
void api_end(void);
int api_get_fifodata(int mode);
void api_beep(int tone);

void HariMain(void)
{
	int i, timer;
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	for(i = 20000000; i >= 20000; i-= i / 100)
	{
		/*20KHz~20Hz是人类可以听到的范围*/
		api_beep(i);		//让蜂鸣器以iHz的频率进行发声
		if(128 != api_get_fifodata(1))		//接收用户任何键盘输入都会关闭程序
		{
			break;
		}
	}
	api_beep(0);			//关闭蜂鸣器
	api_end();
}

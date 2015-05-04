#include "bootpack.h"

//内存检测程序
unsigned int memtest(unsigned int start, unsigned int end)			//这里的变量类型必须用unsigned int型，因为要检查的内存很大，需要的是正整数
{
	char isCPU486 = 0;
	unsigned int eflag, cr0, i;
	
	//1.检查是否有高速缓存
	//设置EFLAGS寄存器的AC位为1，然后再重新读取，查看AC位是不是会变回0
	eflag = io_load_eflags();
	eflag |= EFLAGS_AC_BIT;							//根据AC位来判断CPU
	io_store_eflags(eflag);
	eflag = io_load_eflags();
	if(0 != (eflag & EFLAGS_AC_BIT))				//如果是386CPU，即使设定了AC位为1，AC的值也会自动回到0
	{
		isCPU486 = 1;
	}
	
	//2.如果有高速缓存，需要把高速缓存关闭才能继续检查内存
	//先把AC位恢复为0
	eflag &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	//为了禁止缓存，需要对cr0的某一标志位进行设置
	if(0 != isCPU486)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;		//禁止缓存
		store_cr0(cr0);
	}
	
	//3.检查内存
	i = memtest_wr(start, end);			//这个函数是用来读写内存，检查内存可用空间的；本来是可以用C语言来实现的，但是由于C编译器优化的原因，最好还是用汇编来实现了
	
	//4.如果有高速缓存，检测完毕之后还要重新开启
	if(0 != isCPU486)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;		//允许缓存
		store_cr0(cr0);
	}
	return i;
}



















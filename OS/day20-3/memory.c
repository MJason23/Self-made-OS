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

//内存管理初始化
void memmanage_init(MEMMANAGE *memman)
{
	memman->frees = 0;			//可用信息数目
	memman->maxfrees = 0;		//frees的最大值
	memman->lostsize = 0;		//释放失败的内存的大小总和
	memman->losttimes = 0;		//释放失败的次数
}

//查看内存中剩余的容量
unsigned int memmanage_total(MEMMANAGE *memman)
{
	unsigned int i,total = 0;
	for(i = 0;i < memman->frees; i++)
	{
		total += memman->free[i].size;				//把所有可以使用的内存空间相加返回
	}
	return total;
}

//内存分配
unsigned int memmanage_alloc(MEMMANAGE *memman, unsigned int size)
{
	unsigned int i, address;
	for(i = 0; i < memman->frees; i++)
	{
		if(memman->free[i].size >= size)			//找到了足够大的内存可以分配
		{
			address = memman->free[i].addr;
			memman->free[i].size -= size;
			memman->free[i].addr += size;
			if(0 == memman->free[i].size)			//如果分配出去的内存大小正好与这块儿内存的大小一样大，则需要从管理块儿中删除掉它
			{
				memman->frees--;					//内存块儿可用数量减少1
				for(; i < memman->frees;i++)
				{
					memman->free[i] = memman->free[i + 1];
				}
			}
			return address;							//把可以分配的内存起始地址返回
		}
	}
	return 0;			//没有可用空间能分配了
}

//内存释放
int memmanage_free(MEMMANAGE *memman, unsigned int addr, unsigned int size)
{
	int i, j;
	/*1.为了便于管理内存，把free按照内存地址大小的顺序排列，所以首先决定要把释放的内存放在哪里*/
	for(i = 0; i < memman->frees; i++)
	{
		if(memman->free[i].addr > addr)
		{
			break;			//得到这个i所代表的位置就可以了
		}
	}
	/*2.如果这个释放的内存地址前面还有空余内存，查看是否可以与其合并，如果可以合并的话就直接合并，同时检查合并后的内存能否再与后面的内存合并*/
	if(i > 0)				//i > 0说明这个释放的内存前面还有空余内存
	{
		if(memman->free[i - 1].addr + memman->free[i - 1].size == addr)		//说明前一块儿内存块儿正好与这个释放的地址块儿相连接
		{
			memman->free[i - 1].size += size;
			if(i < memman->frees)			//先判断一下i之后还有没有空余内存了
			{
				if(addr + size == memman->free[i].addr)				//这说明这块儿释放内存也能与后面的空域内存进行合并
				{
					memman->free[i - 1].size += memman->free[i].size;
					memman->frees--;		//合并之后就不需要free[i]这个管理块儿了，所以frees需要减1
					for(;i < memman->frees; i++)
					{
						memman->free[i] = memman->free[i + 1];
					}
				}
			}
			return 0;
		}
	}
	/*3.如果不能与前面的内存合并，则查看能否与后面的内存合并*/
	if(i < memman->frees)
	{
		if(addr + size == memman->free[i].addr)
		{
			memman->free[i].addr = addr;
			memman->free[i].size += size;
			return 0;
		}
	}
	/*4.如果前后都不能合并，则直接在这个位置分配一个free给它，并把后面的free向后移动一个位置*/
	if(memman->frees < MEMMANAGE_FREES)
	{
		for(j = memman->frees; j > i; j--)			//把i往后的free向后移动
		{
			memman->free[j] = memman->free[j - 1];
		}
		memman->free[i].addr = addr;
		memman->free[i].size = size;
		memman->frees++;
		if(memman->maxfrees < memman->frees)			//如果当前的空余内存块儿比当前的maxfrees还大，则更新maxfrees
		{
			memman->maxfrees = memman->frees;
		}
		return 0;
	}
	/*5.已经不能再向管理模块儿中添加free了*/
	memman->losttimes++;			//记录丢失内存记录的次数
	memman->lostsize += size;		//记录丢失内存的总大小
	return -1;
}

//由于以1字节为单位的分配内存方式会产生大量零碎内存，这样会占用很多的free，很容易耗尽，所以改为以4KB为分配单位的内存分配方式
unsigned int memmanage_alloc_4K(MEMMANAGE *memman, unsigned int size)
{
	unsigned int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memmanage_alloc(memman, size);
	return i;
}

//以4KB为回收单位的内存回收方式，由于分配出去的时候就是以4KB为单位的，所以这个size单位的也是
int memmanage_free_4K(MEMMANAGE *memman, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memmanage_free(memman, addr, size);
	return i;
}









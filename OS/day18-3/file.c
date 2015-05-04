#include "bootpack.h"

//将磁盘映像中的FAT表进行解压缩，使其变成易读的格式
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	//共有2880个扇区
	for(i = 0; i < 2880; i += 2)
	{
		//这一部分详解在书上P383~P386
		fat[i + 0] = (img[j + 0] | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] >> 4) & 0xfff;
		j += 3;
	}
	return ;
}

//把映像中有可能不连续的文件内容拷贝到一块儿完整的连续的内存中，这样就方便程序读取内容了
void file_loadfile(int clusterno, int size, char *buffer, int *fat, char *img)
{
	int i;
	for(;;)
	{
		//当文件内容只剩下不到512字节的时候，复制完就可以退出了
		if(size <= 512)
		{
			for(i = 0; i < size; i++)
			{
				//逐字复制
				buffer[i] = img[clusterno * 512 + i];	
			}
			break;
		}
		for(i = 0; i < 512; i++)
		{
			buffer[i] = img[clusterno * 512 + i];
		}
		//剩余文件内容的大小减少512字节
		size -= 512;
		//把buffer的首地址向后推移512个字节
		buffer += 512;
		//通过fat表找到下一个扇区
		clusterno = fat[clusterno];
	}
	return ;
}












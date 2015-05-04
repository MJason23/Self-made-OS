#include "bootpack.h"
#include <string.h>


FILEINFO *file_search(char *name, FILEINFO *finfo, int max)
{
	int i, j;
	char strings[12];
	//先把strings的前11个位置全部填充成空格
	for (j = 0; j < 11; j++) 
	{
		strings[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) 
	{
		//如果j>=11了，说明没有找到这个文件名
		if (j >= 11)
		{
			return 0;
		}
		//遇到“.”号之后就把"."后面的字符当做后缀名，直接添加到strings的第9个元素开始的位置
		if (name[i] == '.' && j <= 8) 
		{
			j = 8;
		}
		else 
		{
			strings[j] = name[i];
			//如果文件名是小写字母，则需要转化为大写字母
			if ('a' <= strings[j] && strings[j] <= 'z') 
			{
				strings[j] -= 0x20;
			} 
			j++;
		}
	}
	for (i = 0; i < max; )
	{
		//当文件名的第一个字节为0的时候，说明从这里开始往后就木有文件了
		if (finfo[i].name[0] == 0x00)
		{
			break;
		}
		//这个文件只有在不是目录而且不是非文件的情况下才能打印
		if ((finfo[i].type & 0x18) == 0) 
		{
			//找到该文件名的情况
			if(0 == strncmp(finfo[i].name, strings, 11))
			{
				return (finfo + i);
			}
		}
		i++;
	}
	return 0;
}

//将磁盘映像中的FAT表进行解压缩，使其变成易读的格式
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	//共有2880个扇区
	for(i = 0; i < 2880; i += 2)
	{
		//这一部分详解在书上P383~P386
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
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












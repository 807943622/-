#include "string.h"
#include "exfuns.h"
#include "fattester.h"	

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2011/5/13 
//�汾��V2.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************

 //�ļ������б�
const u8 *FILE_TYPE_TBL[6][13]=
{
{"BIN"},			//BIN�ļ�
{"LRC"},			//LRC�ļ�
{"NES"},			//NES�ļ�
{"TXT","C","H"},	//�ı��ļ�
{"MP1","MP2","MP3","MP4","M4A","3GP","3G2","OGG","ACC","WMA","WAV","MID","FLAC"},//�����ļ�
{"BMP","JPG","JPEG","GIF"},//ͼƬ�ļ�
};

///////////////////////////////�����ļ���////////////////////////////////////////////
FATFS fs[2];  		//�߼����̹�����.	 
FIL file;	  		//�ļ�1
FIL ftemp;	  		//�ļ�2.
UINT br,bw;			//��д����
FILINFO fileinfo;	//�ļ���Ϣ
DIR dir;  			//Ŀ¼

u8 fatbuf[512];//SD�����ݻ�����
///////////////////////////////////////////////////////////////////////////////////////

//��Сд��ĸתΪ��д��ĸ,���������,�򱣳ֲ���.
u8 char_upper(u8 c)
{
	if(c<'A')return c;//����,���ֲ���.
	if(c>='a')return c-0x20;//��Ϊ��д.
	else return c;//��д,���ֲ���
}	      
//�����ļ�������
//fname:�ļ���
//����ֵ:0XFF,��ʾ�޷�ʶ����ļ����ͱ��.
//		 ����,����λ��ʾ��������,����λ��ʾ����С��.
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 *attr='\0';//��׺��
	u8 i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')break;//ƫ�Ƶ��������.
		fname++;
	}
	if(i==250)return 0XFF;//������ַ���.
 	for(i=0;i<5;i++)//�õ���׺��
	{
		fname--;
		if(*fname=='.')
		{
			fname++;
			attr=fname;
			break;
		}
  	}
	strcpy((char *)tbuf,(const char*)attr);//copy
 	for(i=0;i<4;i++)tbuf[i]=char_upper(tbuf[i]);//ȫ����Ϊ��д 
	for(i=0;i<6;i++)
	{
		for(j=0;j<13;j++)
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//�����Ѿ�û�пɶԱȵĳ�Ա��.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//�ҵ���
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//û�ҵ�		 			   
}	 



 








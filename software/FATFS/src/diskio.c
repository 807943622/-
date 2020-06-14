#include "mmc_sd.h"
#include "diskio.h"
#include  <stdio.h>	 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//FATFS Diskio ��������		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2011/5/13 
//�汾��V2.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************

#define SD_CARD	 0  //SD��,���Ϊ0
#define EX_FLASH 1	//�ⲿflash,���Ϊ1

#define FLASH_SECTOR_SIZE 	512
//ǰ2M�ֽڸ�fatfs��,��2M�ֽ�~2M+500K���û���,2M+500K�Ժ�,���ڴ���ֿ�,�ֿ�ռ��1.5M.
#define FLASH_SECTOR_COUNT 	4096  //ȫ��һ��,�ܹ���8192������(��512�ֽ���) W25Q16Ϊ2048,Q32Ϊ4096,Q64Ϊ16384 ����ǿ����512�ֽ�
#define FLASH_BLOCK_SIZE  	8     //ÿ��BLOCK��8������

//��ʼ������
DSTATUS disk_initialize (	BYTE drv)	/* Physical drive nmuber (0..) */
{	
	u8 res;	    
	switch(drv)
	{
		case SD_CARD://SD��
			res = SD_Initialize();//SD_Initialize();
			break;
//		case EX_FLASH://�ⲿflash
//			SPI_Flash_Init();
//			if(SPI_Flash_ReadID()!=SPI_FLASH_TYPE)res=1;//W25Q16 ID=0XEF15
//			else res=0;
//			break;
		default:
			res=1; 
	}		 
	if(res)return  STA_NOINIT;
	else return 0; //��ʼ���ɹ�
}   
//��ô���״̬
DSTATUS disk_status (BYTE drv	)/* Physical drive nmuber (0..) */	
{		   
    return 0;
}
 //������
 //drv:���̱��0~9
 //*buff:���ݽ��ջ����׵�ַ
 //sector:������ַ
 //count:��Ҫ��ȡ��������
DRESULT disk_read (
										BYTE drv,		/* Physical drive nmuber (0..) */
										BYTE *buff,		/* Data buffer to store read data */
										DWORD sector,	/* Sector address (LBA) */
										BYTE count		/* Number of sectors to read (1..255) */
                  )
{
	u8 res=0; 
    if (!count)return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
	switch(drv)
	{
		case SD_CARD://SD��
			res=SD_ReadDisk(buff,sector,count);
			break;
//		case EX_FLASH://�ⲿflash
//			for(;count>0;count--)
//			{
//				SPI_Flash_Read(buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
//				sector++;
//				buff+=FLASH_SECTOR_SIZE;
//			}
//			res=0;
//			break;
		default:
			res=1; 
	}
    //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res == 0x00)return RES_OK;	 
    else return RES_ERROR;	   
}  
 //д����
 //drv:���̱��0~9
 //*buff:���������׵�ַ
 //sector:������ַ
 //count:��Ҫд���������	    
#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	        /* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	u8 res=0;  
    if (!count)return RES_PARERR;//count���ܵ���0�����򷵻ز�������		 	 
	switch(drv)
	{
		case SD_CARD://SD��
			res=SD_WriteDisk((u8*)buff,sector,count);
			break;
//		case EX_FLASH://�ⲿflash
//			for(;count>0;count--)
//			{
//				//SPI_Flash_Erase_Sector(sector);
//				SPI_Flash_Write((u8*)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
//				sector++;
//				buff+=FLASH_SECTOR_SIZE;
//			}
//			res=0;
//			break;
		default:
			res=1; 
	}
     //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res == 0x00)return RES_OK;	 
    else return RES_ERROR;		 
}
#endif /* _READONLY */

//����������Ļ��
 //drv:���̱��0~9
 //ctrl:���ƴ���
 //*buff:����/���ջ�����ָ��
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{	
	DRESULT res;						  
	if(drv==SD_CARD)//SD��
	{
	    switch(ctrl)
	    {
		    case CTRL_SYNC:
				SD_CS=0;
		        if(SD_WaitReady()==0)res = RES_OK; 
		        else res = RES_ERROR;	  
				SD_CS=1;
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = 512;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = 8;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = SD_GetSectorCount();
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}else if(drv==EX_FLASH)	//�ⲿFLASH  
	{
	    switch(ctrl)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = FLASH_SECTOR_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = FLASH_BLOCK_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = FLASH_SECTOR_COUNT;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}else res=RES_ERROR;//�����Ĳ�֧��
    return res;
}   
//���ʱ��
//User defined function to give a current time to fatfs module          */
//31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */                                                                                                                                                                                                                                          
//15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */                                                                                                                                                                                                                                                
DWORD get_fattime (void)
{
	return 0;
}





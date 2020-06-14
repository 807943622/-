#include "sys.h"
#include "mmc_sd.h"			   
#include "spi.h"
#include "usart.h"	
					   
u8  SD_Type=0;//SD�������� 
////////////////////////////////////��ֲ�޸���///////////////////////////////////
//��ֲʱ��Ľӿ�
//data:Ҫд�������
//����ֵ:����������
u8 SD_SPI_ReadWriteByte(u8 data)
{
	return SPI2_ReadWriteByte(data);
}
//SD����ʼ����ʱ��,��Ҫ����
void SD_SPI_SpeedLow(void)
{
 	SPI2_SetSpeed(SPI_BaudRatePrescaler_256);//���õ�����ģʽ	
}
//SD������������ʱ��,���Ը�����
void SD_SPI_SpeedHigh(void)
{
 	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);//���õ�����ģʽ	
}
//SPIӲ�����ʼ��
void SD_SPI_Init(void)
{
//  GPIO_InitTypeDef GPIO_InitStructure;
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	 

////  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;

////	
////	GPIO_Init(GPIOA, &GPIO_InitStructure);
////	GPIO_SetBits(GPIOA,GPIO_Pin_4);
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOB,GPIO_Pin_12);
//	SPIx_Init();
//	SD_CS=1;
}
///////////////////////////////////////////////////////////////////////////////////
//ȡ��ѡ��,�ͷ�SPI����
void SD_DisSelect(void)
{
	SD_CS=1;
	SD_SPI_ReadWriteByte(0xff);//�ṩ�����8��ʱ��
}
//ѡ��sd��,���ҵȴ���׼��OK
//����ֵ:0,�ɹ�;1,ʧ��;
u8 SD_Select(void)
{
	SD_CS=0;
	if(SD_WaitReady()==0)return 0;//�ȴ��ɹ�
	SD_DisSelect();
	return 1;//�ȴ�ʧ��
}
//�ȴ���׼����
//����ֵ:0,׼������;����,�������
u8 SD_WaitReady(void)
{
	u32 t=0;
	do
	{
		if(SD_SPI_ReadWriteByte(0XFF)==0XFF)return 0;//OK
		t++;		  	
	}while(t<0XFFFFFF);//�ȴ� 
	return 1;
}
//�ȴ�SD����Ӧ
//Response:Ҫ�õ��Ļ�Ӧֵ
//����ֵ:0,�ɹ��õ��˸û�Ӧֵ
//    ����,�õ���Ӧֵʧ��
u8 SD_GetResponse(u8 Response)
{
	u16 Count=0xFFF;//�ȴ�����	   						  
	while ((SD_SPI_ReadWriteByte(0XFF)!=Response)&&Count)Count--;//�ȴ��õ�׼ȷ�Ļ�Ӧ  	  
	if (Count==0)return MSD_RESPONSE_FAILURE;//�õ���Ӧʧ��   
	else return MSD_RESPONSE_NO_ERROR;//��ȷ��Ӧ
}
//��sd����ȡһ�����ݰ�������
//buf:���ݻ�����
//len:Ҫ��ȡ�����ݳ���.
//����ֵ:0,�ɹ�;����,ʧ��;	
u8 SD_RecvData(u8*buf,u16 len)
{			  	  
	if(SD_GetResponse(0xFE))return 1;//�ȴ�SD������������ʼ����0xFE
    while(len--)//��ʼ��������
    {
        *buf=SPI2_ReadWriteByte(0xFF);
        buf++;
    }
    //������2��αCRC��dummy CRC��
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);									  					    
    return 0;//��ȡ�ɹ�
}
//��sd��д��һ�����ݰ������� 512�ֽ�
//buf:���ݻ�����
//cmd:ָ��
//����ֵ:0,�ɹ�;����,ʧ��;	
u8 SD_SendBlock(u8*buf,u8 cmd)
{	
	u16 t;		  	  
	if(SD_WaitReady())return 1;//�ȴ�׼��ʧЧ
	SD_SPI_ReadWriteByte(cmd);
	if(cmd!=0XFD)//���ǽ���ָ��
	{
		for(t=0;t<512;t++)SPI2_ReadWriteByte(buf[t]);//����ٶ�,���ٺ�������ʱ��
	    SD_SPI_ReadWriteByte(0xFF);//����crc
	    SD_SPI_ReadWriteByte(0xFF);
		t=SD_SPI_ReadWriteByte(0xFF);//������Ӧ
		if((t&0x1F)!=0x05)return 2;//��Ӧ����									  					    
	}						 									  					    
    return 0;//д��ɹ�
}
//��SD������һ������(�����ǲ�ʧ��Ƭѡ�����к������ݴ�����
//����:u8 cmd   ���� 
//     u32 arg  �������
//     u8 crc   crcУ��ֵ	 
//����ֵ:SD�����ص���Ӧ															  
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc)
	{
	u8 Retry=0;	         
	u8 r1;			   
	SPI2_ReadWriteByte(0xff);//����д������ʱ
	SPI2_ReadWriteByte(0xff);  	 	 
	Clr_SD_CS;//Ƭѡ���õͣ�ѡ��SD��	   
	//����
	SPI2_ReadWriteByte(cmd | 0x40); //�ֱ�д������
	SPI2_ReadWriteByte(arg >> 24);
	SPI2_ReadWriteByte(arg >> 16);
	SPI2_ReadWriteByte(arg >> 8);
	SPI2_ReadWriteByte(arg);
	SPI2_ReadWriteByte(crc);   
	//�ȴ���Ӧ����ʱ�˳�
	while((r1=SPI2_ReadWriteByte(0xFF))==0xFF)
		{
		Retry++;	    
		if(Retry>200)break; 
		}  	  
	//������Ӧֵ
	return r1;
	}
//��SD������һ������
//����: u8 cmd   ���� 
//      u32 arg  �������
//      u8 crc   crcУ��ֵ	   
//����ֵ:SD�����ص���Ӧ															  
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;	
	u8 Retry=0; 
	SD_DisSelect();//ȡ���ϴ�Ƭѡ
	if(SD_Select())return 0XFF;//ƬѡʧЧ 
	//����
    SD_SPI_ReadWriteByte(cmd | 0x40);//�ֱ�д������
    SD_SPI_ReadWriteByte(arg >> 24);
    SD_SPI_ReadWriteByte(arg >> 16);
    SD_SPI_ReadWriteByte(arg >> 8);
    SD_SPI_ReadWriteByte(arg);	  
    SD_SPI_ReadWriteByte(crc); 
	if(cmd==CMD12)SD_SPI_ReadWriteByte(0xff);//Skip a stuff byte when stop reading
    //�ȴ���Ӧ����ʱ�˳�
	Retry=0X1F;
	do
	{
		r1=SD_SPI_ReadWriteByte(0xFF);
	}while((r1&0X80) && Retry--);	 
	//����״ֵ̬
    return r1;
}		    																			  
//��ȡSD����CID��Ϣ��������������Ϣ
//����: u8 *cid_data(���CID���ڴ棬����16Byte��	  
//����ֵ:0��NO_ERR
//		 1������														   
u8 SD_GetCID(u8 *cid_data)
{
    u8 r1;	   
    //��CMD10�����CID
    r1=SD_SendCmd(CMD10,0,0x01);
    if(r1==0x00)
	{
		r1=SD_RecvData(cid_data,16);//����16���ֽڵ�����	 
    }
	SD_DisSelect();//ȡ��Ƭѡ
	if(r1)return 1;
	else return 0;
}																				  
//��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
//����:u8 *cid_data(���CID���ڴ棬����16Byte��	    
//����ֵ:0��NO_ERR
//		 1������														   
u8 SD_GetCSD(u8 *csd_data)
{
    u8 r1;	 
    r1=SD_SendCmd(CMD9,0,0x01);//��CMD9�����CSD
    if(r1==0)
	{
    	r1=SD_RecvData(csd_data, 16);//����16���ֽڵ����� 
    }
	SD_DisSelect();//ȡ��Ƭѡ
	if(r1)return 1;
	else return 0;
}  
//��ȡSD����������������������   
//����ֵ:0�� ȡ�������� 
//       ����:SD��������(������/512�ֽ�)
//ÿ�������ֽ�����Ϊ512����Ϊ�������512�����ʼ������ͨ��.														  
u32 SD_GetSectorCount(void)
{
    u8 csd[16];
    u32 Capacity;  
    u8 n;
	  u16 csize;  					    
	//ȡCSD��Ϣ������ڼ��������0
    if(SD_GetCSD(csd)!=0) return 0;	    
    //���ΪSDHC�����������淽ʽ����
    if((csd[0]&0xC0)==0x40)	 //V2.00�Ŀ�
    {	
		csize = csd[9] + ((u16)csd[8] << 8) + 1;
		Capacity = (u32)csize << 10;//�õ�������	 		   
    }else//V1.XX�Ŀ�
    {	
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
		Capacity= (u32)csize << (n - 9);//�õ�������   
    }
    return Capacity;
}
//��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
//����: u8 *data(��Ŷ������ݵ��ڴ�>len)
//      u16 len(���ݳ��ȣ�
//      u8 release(������ɺ��Ƿ��ͷ�����CS�ø� 0�����ͷ� 1���ͷţ�	 
//����ֵ:0��NO_ERR
//  	 other��������Ϣ														  
u8 SD_ReceiveData(u8 *data, u16 len, u8 release)
	{
	// ����һ�δ���
	Clr_SD_CS;				  	  
	if(SD_GetResponse(0xFE))//�ȴ�SD������������ʼ����0xFE
		{	  
		Set_SD_CS;
		return 1;
		}
	while(len--)//��ʼ��������
		{
		*data=SPI2_ReadWriteByte(0xFF);
		data++;
		}
	//������2��αCRC��dummy CRC��
	SPI2_ReadWriteByte(0xFF);
	SPI2_ReadWriteByte(0xFF);
	if(release==RELEASE)//�����ͷ����ߣ���CS�ø�
		{
		Set_SD_CS;//�������
		SPI2_ReadWriteByte(0xFF);
		}											  					    
	return 0;
	}		
		
u8 SD_Idle_Sta(void)
{
	u16 i;
	u8 retry;	   	  
    for(i=0;i<0xf00;i++);//����ʱ���ȴ�SD���ϵ����	 
    //�Ȳ���>74�����壬��SD���Լ���ʼ�����
    for(i=0;i<10;i++)SPI2_ReadWriteByte(0xFF); 
    //-----------------SD����λ��idle��ʼ-----------------
    //ѭ����������CMD0��ֱ��SD������0x01,����IDLE״̬
    //��ʱ��ֱ���˳�
    retry = 0;
    do
    {	   
        //����CMD0����SD������IDLE״̬
        i = SD_SendCmd(CMD0, 0, 0x95);
        retry++;
    }while((i!=0x01)&&(retry<200));
    //����ѭ���󣬼��ԭ�򣺳�ʼ���ɹ���or ���Գ�ʱ��
    if(retry==200)return 1; //ʧ��
	return 0;//�ɹ�	 						  
}
//��ʼ��SD��
u8 SD_Initialize(void)
{
    u8 r1;      // ���SD���ķ���ֵ
    u16 retry;  // �������г�ʱ����
    u8 buf[4];  
	u16 i;

	SD_SPI_Init();		//��ʼ��IO
 	SD_SPI_SpeedLow();	//���õ�����ģʽ 
    //for(i=0;i<0xf00;i++);//����ʱ���ȴ�SD���ϵ����	  
	for(i=0;i<10;i++)SD_SPI_ReadWriteByte(0XFF);//��������74������
	retry=20;
	do
	{
		r1=SD_SendCmd(CMD0,0,0x95);//����IDLE״̬
	}while((r1!=0X01) && retry--);
 	SD_Type=0;//Ĭ���޿�
	if(r1==0X01)
	{
		if(SD_SendCmd(CMD8,0x1AA,0x87)==1)//SD V2.0
		{
			for(i=0;i<4;i++)buf[i]=SD_SPI_ReadWriteByte(0XFF);	//Get trailing return value of R7 resp
			if(buf[2]==0X01&&buf[3]==0XAA)//���Ƿ�֧��2.7~3.6V
			{
				retry=0XFFFE;
				do
				{
					SD_SendCmd(CMD55,0,0X01);	//����CMD55
					r1=SD_SendCmd(CMD41,0x40000000,0X01);//����CMD41
				}while(r1&&retry--);
				if(retry&&SD_SendCmd(CMD58,0,0X01)==0)//����SD2.0���汾��ʼ
				{
					for(i=0;i<4;i++)buf[i]=SD_SPI_ReadWriteByte(0XFF);//�õ�OCRֵ
					if(buf[0]&0x40)SD_Type=SD_TYPE_V2HC;    //���CCS
					else SD_Type=SD_TYPE_V2;   
				}
			}
		}else//SD V1.x/ MMC	V3
		{
			SD_SendCmd(CMD55,0,0X01);		//����CMD55
			r1=SD_SendCmd(CMD41,0,0X01);	//����CMD41
			if(r1<=1)
			{		
				SD_Type=SD_TYPE_V1;
				retry=0XFFFE;
				do //�ȴ��˳�IDLEģʽ
				{
					SD_SendCmd(CMD55,0,0X01);	//����CMD55
					r1=SD_SendCmd(CMD41,0,0X01);//����CMD41
				}while(r1&&retry--);
			}else
			{
				SD_Type=SD_TYPE_MMC;//MMC V3
				retry=0XFFFE;
				do //�ȴ��˳�IDLEģʽ
				{											    
					r1=SD_SendCmd(CMD1,0,0X01);//����CMD1
				}while(r1&&retry--);  
			}
			if(retry==0||SD_SendCmd(CMD16,512,0X01)!=0)SD_Type=SD_TYPE_ERR;//����Ŀ�
		}
	}
	SD_DisSelect();//ȡ��Ƭѡ
	SD_SPI_SpeedHigh();//����
	if(SD_Type)return 0;
	else if(r1)return r1; 	   
	return 0xaa;//��������
}
 
//��SD��
//buf:���ݻ�����
//sector:����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
u8 SD_ReadDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 r1;
	if(SD_Type!=SD_TYPE_V2HC)sector <<= 9;//ת��Ϊ�ֽڵ�ַ
	if(cnt==1)
	{
		r1=SD_SendCmd(CMD17,sector,0X01);//������
		if(r1==0)//ָ��ͳɹ�
		{
			r1=SD_RecvData(buf,512);//����512���ֽ�	   
		}
	}else
	{
		r1=SD_SendCmd(CMD18,sector,0X01);//����������
		do
		{
			r1=SD_RecvData(buf,512);//����512���ֽ�	 
			buf+=512;  
		}while(--cnt && r1==0); 	
		SD_SendCmd(CMD12,0,0X01);	//����ֹͣ����
	}   
	SD_DisSelect();//ȡ��Ƭѡ
	return r1;//
}
//дSD��
//buf:���ݻ�����
//sector:��ʼ����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
u8 SD_WriteDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 r1;
	if(SD_Type!=SD_TYPE_V2HC)sector *= 512;//ת��Ϊ�ֽڵ�ַ
	if(cnt==1)
	{
		r1=SD_SendCmd(CMD24,sector,0X01);//������
		if(r1==0)//ָ��ͳɹ�
		{
			r1=SD_SendBlock(buf,0xFE);//д512���ֽ�	   
		}
	}else
	{
		if(SD_Type!=SD_TYPE_MMC)
		{
			SD_SendCmd(CMD55,0,0X01);	
			SD_SendCmd(CMD23,cnt,0X01);//����ָ��	
		}
 		r1=SD_SendCmd(CMD25,sector,0X01);//����������
		if(r1==0)
		{
			do
			{
				r1=SD_SendBlock(buf,0xFC);//����512���ֽ�	 
				buf+=512;  
			}while(--cnt && r1==0);
			r1=SD_SendBlock(0,0xFD);//����512���ֽ� 
		}
	}   
	SD_DisSelect();//ȡ��Ƭѡ
	return r1;//
}	   
////////////////////////////����2������ΪUSB��д����Ҫ��/////////////////////////
//����SD���Ŀ��С	 				   
#define BLOCK_SIZE 512 
//д��MSD/SD���� 
//pBuffer:���ݴ����
//ReadAddr:д����׵�ַ
//NumByteToRead:Ҫд����ֽ���
//����ֵ:0,д�����
//    ����,д��ʧ��
u8 MSD_WriteBuffer(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite)
	{
	u32 i,NbrOfBlock = 0, Offset = 0;
	u32 sector;
	u8 r1;
	NbrOfBlock = NumByteToWrite / BLOCK_SIZE;//�õ�Ҫд��Ŀ����Ŀ	    
	Clr_SD_CS;	  		   
	while (NbrOfBlock--)//д��һ������
		{
		sector=WriteAddr+Offset;
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//ִ������ͨ�����෴�Ĳ���					  			 
		r1=SD_SendCommand_NoDeassert(CMD24,sector,0xff);//д����   
		if(r1)
			{
			Set_SD_CS;
			return 1;//Ӧ����ȷ��ֱ�ӷ��� 	   
			}
		SPI2_ReadWriteByte(0xFE);//����ʼ����0xFE   
		//��һ��sector������
		for(i=0;i<512;i++)SPI2_ReadWriteByte(*pBuffer++);  
		//��2��Byte��dummy CRC
		SPI2_ReadWriteByte(0xff);
		SPI2_ReadWriteByte(0xff); 
		if(SD_WaitReady())//�ȴ�SD������д�����
			{
			Set_SD_CS;
			return 2;    
			}
		Offset += 512;	   
		}	    
	//д����ɣ�Ƭѡ��1
	Set_SD_CS;
	SPI2_ReadWriteByte(0xff);	 
	return 0;
	}
	
//��ȡMSD/SD���� 
//pBuffer:���ݴ����
//ReadAddr:��ȡ���׵�ַ
//NumByteToRead:Ҫ�������ֽ���
//����ֵ:0,�������
//    ����,����ʧ��
u8 MSD_ReadBuffer(u8* pBuffer, u32 ReadAddr, u32 NumByteToRead)
	{
	u32 NbrOfBlock=0,Offset=0;
	u32 sector=0;
	u8 r1=0;   	 
	NbrOfBlock=NumByteToRead/BLOCK_SIZE;	  
	Clr_SD_CS;
	while (NbrOfBlock --)
		{	
		sector=ReadAddr+Offset;
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//ִ������ͨ�����෴�Ĳ���					  			 
		r1=SD_SendCommand_NoDeassert(CMD17,sector,0xff);//������	 		    
		if(r1)//����ʹ���
			{
			Set_SD_CS;
			return r1;
			}	   							  
		r1=SD_ReceiveData(pBuffer,512,RELEASE);		 
		if(r1)//��������
			{
			Set_SD_CS;
			return r1;
			}
		pBuffer+=512;	 					    
		Offset+=512;				 	 
		}	 	 
	Set_SD_CS;
	SPI2_ReadWriteByte(0xff);	 
	return 0;
	}
	
//////////////////////////////////////////////////////////////////////////
//д��SD����һ��block(δʵ�ʲ��Թ�)										    
//����:u32 sector ������ַ��sectorֵ���������ַ�� 
//     u8 *buffer ���ݴ洢��ַ����С����512byte�� 		   
//����ֵ:0�� �ɹ�
//       other��ʧ��															  
u8 SD_WriteSingleBlock(u32 sector, const u8 *data)
	{
	u8 r1;
	u16 i;
	u16 retry;
	
	//����Ϊ����ģʽ
	//SPIx_SetSpeed(SPI_SPEED_HIGH);	   
	//�������SDHC����������sector��ַ������ת����byte��ַ
	if(SD_Type!=SD_TYPE_V2HC)
		{
		sector = sector<<9;
		}   
	r1 = SD_SendCmd(CMD24, sector, 0x00);
	if(r1 != 0x00)
		{
		return r1;  //Ӧ����ȷ��ֱ�ӷ���
		}
	
	//��ʼ׼�����ݴ���
	Clr_SD_CS;
	//�ȷ�3�������ݣ��ȴ�SD��׼����
	SPI2_ReadWriteByte(0xff);
	SPI2_ReadWriteByte(0xff);
	SPI2_ReadWriteByte(0xff);
	//����ʼ����0xFE
	SPI2_ReadWriteByte(0xFE);
	
	//��һ��sector������
	for(i=0;i<512;i++)
		{
		SPI2_ReadWriteByte(*data++);
		}
	//��2��Byte��dummy CRC
	SPI2_ReadWriteByte(0xff);
	SPI2_ReadWriteByte(0xff);
	
	//�ȴ�SD��Ӧ��
	r1 = SPI2_ReadWriteByte(0xff);
	if((r1&0x1F)!=0x05)
		{
		Set_SD_CS;
		return r1;
		}
	
	//�ȴ��������
	retry = 0;
	while(!SPI2_ReadWriteByte(0xff))
		{
		retry++;
		if(retry>0xfffe)        //�����ʱ��д��û����ɣ������˳�
			{
			Set_SD_CS;
			return 1;           //д�볬ʱ����1
			}
		}	    
	//д����ɣ�Ƭѡ��1
	Set_SD_CS;
	SPI2_ReadWriteByte(0xff);
	
	return 0;
	}	
				           
//��SD���Ķ��block(ʵ�ʲ��Թ�)										    
//����:u32 sector ������ַ��sectorֵ���������ַ�� 
//     u8 *buffer ���ݴ洢��ַ����С����512byte��
//     u8 count ������count��block 		   
//����ֵ:0�� �ɹ�
//       other��ʧ��															  
u8 SD_ReadMultiBlock(u32 sector, u8 *buffer, u8 count)
	{
	u8 r1;	 			 
	//SPIx_SetSpeed(SPI_SPEED_HIGH);//����Ϊ����ģʽ  
	//�������SDHC����sector��ַת��byte��ַ
	if(SD_Type!=SD_TYPE_V2HC)sector = sector<<9;  
	//SD_WaitDataReady();
	//�����������
	r1 = SD_SendCmd(CMD18, sector, 0);//������
	if(r1 != 0x00)return r1;	 
	do//��ʼ��������
		{
		if(SD_ReceiveData(buffer, 512, NO_RELEASE) != 0x00)break; 
		buffer += 512;
		} while(--count);		 
	//ȫ��������ϣ�����ֹͣ����
	SD_SendCmd(CMD12, 0, 0);
	//�ͷ�����
	Set_SD_CS;
	SPI2_ReadWriteByte(0xFF);    
	if(count != 0)return count;   //���û�д��꣬����ʣ�����	 
	else return 0;	 
	}		
										  
//д��SD����N��block(δʵ�ʲ��Թ�)									    
//����:u32 sector ������ַ��sectorֵ���������ַ�� 
//     u8 *buffer ���ݴ洢��ַ����С����512byte��
//     u8 count д���block��Ŀ		   
//����ֵ:0�� �ɹ�
//       other��ʧ��															   
u8 SD_WriteMultiBlock(u32 sector, const u8 *data, u8 count)
	{
	u8 r1;
	u16 i;	 		 
	//SPIx_SetSpeed(SPI_SPEED_HIGH);//����Ϊ����ģʽ	 
	if(SD_Type != SD_TYPE_V2HC)sector = sector<<9;//�������SDHC����������sector��ַ������ת����byte��ַ  
	if(SD_Type != SD_TYPE_MMC) r1 = SD_SendCmd(CMD23, count, 0x00);//���Ŀ�꿨����MMC��������ACMD23ָ��ʹ��Ԥ����   
	r1 = SD_SendCmd(CMD25, sector, 0x00);//�����д��ָ��
	if(r1 != 0x00)return r1;  //Ӧ����ȷ��ֱ�ӷ���	 
	Clr_SD_CS;//��ʼ׼�����ݴ���   
	SPI2_ReadWriteByte(0xff);//�ȷ�3�������ݣ��ȴ�SD��׼����
	SPI2_ReadWriteByte(0xff);   
	//--------������N��sectorд���ѭ������
	do
		{
		//����ʼ����0xFC �����Ƕ��д��
		SPI2_ReadWriteByte(0xFC);	  
		//��һ��sector������
		for(i=0;i<512;i++)
			{
			SPI2_ReadWriteByte(*data++);
			}
		//��2��Byte��dummy CRC
		SPI2_ReadWriteByte(0xff);
		SPI2_ReadWriteByte(0xff);
		
		//�ȴ�SD��Ӧ��
		r1 = SPI2_ReadWriteByte(0xff);
		if((r1&0x1F)!=0x05)
			{
			Set_SD_CS;    //���Ӧ��Ϊ��������������ֱ���˳�
			return r1;
			}		   
		//�ȴ�SD��д�����
		if(SD_WaitReady()==1)
			{
			Set_SD_CS;    //�ȴ�SD��д����ɳ�ʱ��ֱ���˳�����
			return 1;
			}	   
		}while(--count);//��sector���ݴ������  
	//��������������0xFD
	r1 = SPI2_ReadWriteByte(0xFD);
	if(r1==0x00)
		{
		count =  0xfe;
		}		   
	if(SD_WaitReady()) //�ȴ�׼����
		{
		Set_SD_CS;
		return 1;  
		}
	//д����ɣ�Ƭѡ��1
	Set_SD_CS;
	SPI2_ReadWriteByte(0xff);  
	return count;   //����countֵ�����д����count=0������count=1
	}
						  					  
//��ָ������,��offset��ʼ����bytes���ֽ�								    
//����:u32 sector ������ַ��sectorֵ���������ַ�� 
//     u8 *buf     ���ݴ洢��ַ����С<=512byte��
//     u16 offset  �����������ƫ����
//     u16 bytes   Ҫ�������ֽ���	   
//����ֵ:0�� �ɹ�
//       other��ʧ��															   
u8 SD_Read_Bytes(unsigned long address,unsigned char *buf,unsigned int offset,unsigned int bytes)
	{
	u8 r1;u16 i=0;  
	r1=SD_SendCmd(CMD17,address<<9,0);//���Ͷ���������      
	if(r1)return r1;  //Ӧ����ȷ��ֱ�ӷ���
	Clr_SD_CS;//ѡ��SD��
	if(SD_GetResponse(0xFE))//�ȴ�SD������������ʼ����0xFE
		{
		Set_SD_CS; //�ر�SD��
		return 1;//��ȡʧ��
		}	 
	for(i=0;i<offset;i++)SPI2_ReadWriteByte(0xff);//����offsetλ 
	for(;i<offset+bytes;i++)*buf++=SPI2_ReadWriteByte(0xff);//��ȡ��������	
	for(;i<512;i++) SPI2_ReadWriteByte(0xff); 	 //����ʣ���ֽ�
	SPI2_ReadWriteByte(0xff);//����αCRC��
	SPI2_ReadWriteByte(0xff);  
	Set_SD_CS;//�ر�SD��
	return 0;
	}


#include "ld3320.h"
#include "misc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_exti.h"
#include "mmc_sd.h"	
#include "ff.h"
#include "AsrItem.h"
#include "spi.h"
#include "led.h"
#include "usart.h"
#include "delay.h"
//SD_CardInfo SDCardInfo;

extern SYS_CHECK sys_check;
 
u8  nLD_Mode = LD_MODE_IDLE;		//������¼��ǰ���ڽ���ASRʶ�����ڲ���MP3
u8 ucRegVal; 						        //����ʶ����			
extern u8 nAsrStatus;					  //��ʾһ��ʶ�����̽��������޽��
u8 ucHighInt;
u8 ucLowInt;
u8 bMp3Play=0;							    //������¼����MP3��״̬
u32 nMp3Size=0;
u32 nMp3Pos=0;
u8 ucSPVol=15; // MAX=15 MIN=0	//	Speaker�������������
unsigned long fpPoint=0;
/***********************************************************
* ��    �ƣ�LD3320_GPIO_Cfg(void)
* ��    �ܣ���ʼ����Ҫ�õ���IO��
* ��ڲ�����  
* ���ڲ�����
* ˵    ����
* ���÷����� 
**********************************************************/ 
void LD3320_GPIO_Cfg(void)
{	
		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC ,ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//RSET
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ���ģʽ
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//���� = GPIO_PuPd_UP;//�������
		GPIO_Init(GPIOA,&GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2;//RSET
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//���� = GPIO_PuPd_UP;//�������
		GPIO_Init(GPIOA,&GPIO_InitStructure);

		GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_2);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOC,&GPIO_InitStructure);

}

/***********************************************************
* ��    �ƣ� LD3320_EXTI_Cfg(void) 
* ��    �ܣ� �ⲿ�жϹ������ú���ض˿�����
* ��ڲ�����  
* ���ڲ�����
* ˵    ����
* ���÷����� 
**********************************************************/ 
void LD3320_EXTI_Cfg(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  //�ж���������
  RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//ʹ��SYSCFGʱ��
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);//PC0 ���ӵ��ж���1
	
	//�ⲿ�ж�������
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//�ж���ʹ��
  EXTI_Init(&EXTI_InitStructure);//����
		
	GPIO_SetBits(GPIOC,GPIO_Pin_0);	 //Ĭ�������ж�����

	EXTI_ClearFlag(EXTI_Line0);
//	EXTI_ClearITPendingBit(EXTI_Line1);

	//�ж�Ƕ������
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//�ⲿ�ж�1
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ�3
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
}


/***********************************************************
* ��    �ƣ�  EXTI1_IRQHandler(void)
* ��    �ܣ� �ⲿ�жϺ���
* ��ڲ�����  
* ���ڲ�����
* ˵    ����
* ���÷����� 
**********************************************************/ 
//�ⲿ�ж�0�������
void EXTI0_IRQHandler(void)
{
	delay(10);	//����
	if(EXTI_GetITStatus(EXTI_Line0) != RESET ) 
	{
		ProcessInt0();
		printf("\r\nInter the handle!\r\n");
		EXTI_ClearFlag(EXTI_Line0);
		EXTI_ClearITPendingBit(EXTI_Line0);//���LINE1�ϵ��жϱ�־λ 
	}
	 printf("\r\nEXIT from the exit!\r\n");
	EXTI_ClearFlag(EXTI_Line0);
	 //EXTI_ClearITPendingBit(EXTI_Line1);
}

//=======================================
//us����ʱ
void delay(unsigned long uldata)
{
	delay_us(uldata*10);
	return;
//	unsigned int i;
//	unsigned char j;
//	for(i=uldata*100;i>0;i--)
//		for(j=5;j>0;j--);   //j=110
}
//nop_�ӳ�
void delay_nop(unsigned long uldata)
{
	unsigned int i;
	for(i=uldata;i>0;i--);
}
/***********************************************************
* ��    �ƣ�void LD_reset(void)
* ��    �ܣ�LDоƬӲ����ʼ��
* ��ڲ�����  
* ���ڲ�����
* ˵    ����
* ���÷����� 
**********************************************************/ 
void LD_Reset(void)
{
	LD_RST_H();
	delay(100);
	LD_RST_L();
	delay(100);
	LD_RST_H();
	delay(100);
	LD_CS_L();
	delay(100);
	LD_CS_H();		
	delay(100);
}

/***********************************************************
* ��    �ƣ�void LD_WriteReg(uint8 data1,uint8 data2)
* ��    �ܣ� дld3320�Ĵ���
* ��ڲ�����  
* ���ڲ�����
* ˵    ����
* ���÷����� 
**********************************************************/ 
void LD_Write( unsigned char address, unsigned char dataout )
{
			unsigned char i = 0;
			unsigned char command=0x04;
//			SDCK = 1;
//			DELAY_NOP;
			SPIS_L;
			SCS_L;
			DELAY_NOP;
		
			//write command
			for (i=0;i < 8; i++)
			{
				if ((command & 0x80) == 0x80) 
					SDI_H;
				else
					SDI_L;
				
				DELAY_NOP;
				SDCK_L;
				command = (command << 1);  
				DELAY_NOP;
				SDCK_H;  
			}
			//write address
			for (i=0;i < 8; i++)
			{
				if ((address & 0x80) == 0x80) 
					SDI_H;
				else
					SDI_L;
				DELAY_NOP;
				SDCK_L;
				address = (address << 1); 
				DELAY_NOP;
				SDCK_H;  
			}
			//write data
			for (i=0;i < 8; i++)
			{
				if ((dataout & 0x80) == 0x80) 
					SDI_H;
				else
					SDI_L;
				DELAY_NOP;
				SDCK_L;
				dataout = (dataout << 1); 
				DELAY_NOP;
				SDCK_H;  
			}
			DELAY_NOP;
			SCS_H;
}
/***********************************************************
* ��    �ƣ�uint8 LD_ReadReg(uint8 reg_add)
* ��    �ܣ���ld3320�Ĵ���
* ��ڲ�����  
* ���ڲ�����
* ˵    ����
* ���÷����� 
**********************************************************/ 
unsigned char LD_Read( unsigned char address )
{
			unsigned char i = 0; 
			unsigned char datain =0 ;
			unsigned char temp = 0; 
			unsigned char command=0x05;
			SPIS_L;
			SCS_L;
			DELAY_NOP;
		
			//write command
			for (i=0;i < 8; i++)
			{
				if ((command & 0x80) == 0x80) 
					SDI_H;
				else
					SDI_L;
				DELAY_NOP;
				SDCK_L;
				command = (command << 1);  
				DELAY_NOP;
				SDCK_H;  
			}

			//write address
			for (i=0;i < 8; i++)
			{
				if ((address & 0x80) == 0x80) 
					SDI_H;
				else
					SDI_L;
				DELAY_NOP;
				SDCK_L;
				address = (address << 1); 
				DELAY_NOP;
				SDCK_H;  
			}
			DELAY_NOP;

			//Read data
			for (i=0;i < 8; i++)
			{
				datain = (datain << 1);
				temp = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);
				DELAY_NOP;
				SDCK_L;  
				if (temp == 1)  
					datain |= 0x01; 
				DELAY_NOP;
				SDCK_H;  
			}
		
			DELAY_NOP;
			SCS_H;
			return datain;
}


//==============================================================================================
static FIL fsrc;
//==============================================================================================
//���ź�����ʶ���ж�
void ProcessInt0(void)
{
	u8 nAsrResCount=0;

//	EX0=0;	  //�ر��ⲿ�ж�0�ж�
	ucRegVal = LD_Read(0x2B);
	if(nLD_Mode == LD_MODE_ASR_RUN)		//��ǰ��������ʶ��
	{
		// ����ʶ��������ж�
		// �����������룬����ʶ��ɹ���ʧ�ܶ����жϣ�
		LD_Write(0x29,0) ;					//�ж����� FIFO �ж����� 0��ʾ������ 
		LD_Write(0x02,0) ;					// FIFO�ж�����	 FIFO_DATA FIFO_EXT�ж�   ������
		delay(100);
		if((ucRegVal & 0x10) &&		  //2b����λΪ1 оƬ�ڲ�FIFO�жϷ��� MP3����ʱ������жϱ�־�����ⲿMCU��FIFO_DATA��Reload��
			LD_Read(0xb2)==0x21 && 	  //��b2�õ�0x21��ʾ�п��Խ�����һ��ASR����
			LD_Read(0xbf)==0x35)		  //������ֵΪ0x35������ȷ����һ������ʶ��������������
		{
			printf("\r\n�����ж�");
			nAsrResCount = LD_Read(0xba);	    //ASR���ж�ʱ���ж�����ʶ���м���ʶ���ѡ
			printf("\r\n���ܽ���� %d ��\r\n",nAsrResCount);
			if(nAsrResCount==1||(nAsrResCount>0 && nAsrResCount<=4)) 
			{
				nAsrStatus=LD_ASR_FOUNDOK;
				return;
//				LD_Read(0xc5);
			}

			if(nAsrResCount>0 && nAsrResCount<=4) 	  //1 �C 4: N��ʶ���ѡ 0���ߴ���4��û��ʶ���ѡ
			{
				nAsrStatus=LD_ASR_FOUNDOK;		 //��ʾһ��ʶ�����̽�������һ��ʶ����
			}
			else
		    {
				nAsrStatus=LD_ASR_FOUNDZERO;	   //��ʾһ��ʶ�����̽�����û��ʶ����
			}	
		}
		else
		{
			nAsrStatus=LD_ASR_FOUNDZERO;	   //��ʾһ��ʶ�����̽�����û��ʶ����
		}

		LD_Write(0x2b, 0);
    LD_Write(0x1C,0);	  //ADC���ؿ��� д00H ADC������
		
		return;
	}
//=============================================================================================
	// �������Ų������жϣ������֣�
	// A. ����������ȫ�������ꡣ
	// B. ���������ѷ�����ϡ�
	// C. ����������ʱ��Ҫ���꣬��Ҫ�����µ����ݡ�	
		ucHighInt = LD_Read(0x29); 
		ucLowInt=LD_Read(0x02); 
		LD_Write(0x29,0);
		LD_Write(0x02,0);
    if(LD_Read(0xBA)&CAUSE_MP3_SONG_END)
    {
	// A. ����������ȫ�������ꡣ

			LD_Write(0x2B,  0);
			LD_Write(0xBA, 0);	
			LD_Write(0xBC,0x0);	
			bMp3Play=0;					// ��������ȫ����������޸�bMp3Play�ı���
			LD_Write(0x08,1);

			LD_Write(0x08,0);
			LD_Write(0x33, 0);
			f_close(&fsrc);
			nLD_Mode = LD_MODE_ASR_RUN;
			nAsrStatus = LD_ASR_NONE;
			return ;
    }

	if(nMp3Pos>=nMp3Size)
	{
	// B. ���������ѷ�����ϡ�

		LD_Write(0xBC, 0x01);
		LD_Write(0x29, 0x10);
		f_close(&fsrc);
		nLD_Mode = LD_MODE_ASR_RUN;
		nAsrStatus = LD_ASR_NONE;
		return;	
	}

	// C. ����������ʱ��Ҫ���꣬��Ҫ�����µ����ݡ�	

	LD_ReloadMp3Data_Again();
			
	LD_Write(0x29,ucHighInt); 
	LD_Write(0x02,ucLowInt) ;
}

/*  ������ȡmp3�ļ����ݵ�fifo,ֱ��fifo��
 *	��дmp3�ļ����ݵ�fifoʱ,LD3320�����벥��
 *	��Ȼдmp3�ļ����ݵ�fifo��ʱ���̹�������ʱ��
 *	�������첥����ϵ�ʱ������ProcessInt0����
 *	ProcessInt0�����ֻ���ô˺���,����������������
 */
void LD_ReloadMp3Data_Again(void)
{
	u8 val;
	u8 ucStatus;
	UINT br;
	
	ucStatus = LD_Read(0x06);
	//fifo�Ƿ�����
	while (!(ucStatus&MASK_FIFO_STATUS_AFULL) && nMp3Pos<=nMp3Size)
	{

		nMp3Pos++;
		f_read(&fsrc,&val,1,&br);
		LD_Write(0x01,val);

		ucStatus = LD_Read(0x06);
	}

	if(nMp3Pos>=nMp3Size)
	{
	    LD_Write(0xBC, 0x01);
		LD_Write(0x29, 0x10);

		//�ȴ�MP3�������
		while(!(LD_Read(0xBA)&CAUSE_MP3_SONG_END));

		LD_Write(0x2B,  0);
      	LD_Write(0xBA, 0);	
		LD_Write(0xBC,0x0);	
		bMp3Play=0;					// ��������ȫ����������޸�bMp3Play�ı���
		LD_Write(0x08,1);

    LD_Write(0x08,0);
		LD_Write(0x33, 0);

		f_close(&fsrc);
		nLD_Mode = LD_MODE_ASR_RUN;
		nAsrStatus = LD_ASR_NONE;
	}
		
}

//=========================================================
//ͨ�ó�ʼ��
void LD_Init_Common(void)	
{
	bMp3Play = 0;

	LD_Read(0x06);  
	LD_Write(0x17, 0x35); //��λLD3320
	delay(100);
	LD_Read(0x06);  

	LD_Write(0x89, 0x03);  //ģ���·���� ��ʼ��ʱд03H
	delay(100);
	LD_Write(0xCF, 0x43);  //�ڲ�ʡ��ģʽ���� ��ʼ��ʱд��43H 
	delay(100);
	LD_Write(0xCB, 0x02);
	
	/*PLL setting*/
	LD_Write(0x11, LD_PLL_11);   		//ʱ��Ƶ������1  
	if (nLD_Mode == LD_MODE_MP3)    //MP3����
	{
		LD_Write(0x1E, 0x00); 					 	 //ADCר�ÿ��ƣ�Ӧ��ʼ��Ϊ00H
		LD_Write(0x19, LD_PLL_MP3_19);  	 //ʱ��Ƶ������2
		LD_Write(0x1B, LD_PLL_MP3_1B); 	   //ʱ��Ƶ������3
		LD_Write(0x1D, LD_PLL_MP3_1D);		 //ʱ��Ƶ������4
	}
	else						   									 //����ʶ��
	{
		LD_Write(0x1E,0x00);	 					   //ADCר�ÿ��ƣ�Ӧ��ʼ��Ϊ00H
		LD_Write(0x19, LD_PLL_ASR_19); 		 //ʱ��Ƶ������2
		LD_Write(0x1B, LD_PLL_ASR_1B);		 //ʱ��Ƶ������3	
	    LD_Write(0x1D, LD_PLL_ASR_1D);	 //ʱ��Ƶ������4
	}
	delay(100);
	
	LD_Write(0xCD, 0x04);    //DSP�������� ��ʼ��ʱд��04H ����DSP����
	LD_Write(0x17, 0x4C); 	 //д4CH����ʹDSP���ߣ��Ƚ�ʡ��
	delay(100);
	LD_Write(0xB9, 0x00);		 //ASR����ǰ���ʶ�����ַ������ȣ�ƴ���ַ�������ʼ��ʱд��00H	 ÿ���һ��ʶ����Ҫ�趨һ��
	LD_Write(0xCF, 0x4F); 	 //�ڲ�ʡ��ģ�``````````````````````````````����� MP3��ʼ����ASR��ʼ��ʱд�� 4FH
	LD_Write(0x6F, 0xFF); 	 //��оƬ���г�ʼ��ʱ����Ϊ0xFF
}
//==================================================
//���ų�ʼ��
void LD_Init_MP3(void)	
{
	nLD_Mode = LD_MODE_MP3;	   //��ǰ����MP3����
	LD_Init_Common();		  		 //ͨ�ó�ʼ��

	LD_Write(0xBD,0x02);	 		 	//�ڲ�������� ��ʼ��ʱд��FFH
	printf("0x02 %x\r\n",LD_Read(0xBD));
	LD_Write(0x17, 0x48);				//д48H���Լ���DSP
		delay(100);
	printf("0x48 %x\r\n",LD_Read(0x17));
	LD_Write(0x85, 0x52); 	//�ڲ��������� ��ʼ��ʱд��52H
	printf("0x52 %x\r\n",LD_Read(0x85));
	LD_Write(0x8F, 0x00);  	//LineOut(��·���)ѡ�� ��ʼ��ʱд��00H
	LD_Write(0x81, 0x00);		//���������� ����Ϊ00HΪ�������
	LD_Write(0x83, 0x00);		//���������� ����Ϊ00HΪ�������
	LD_Write(0x8E, 0xff);		//�����������  ���Ĵ�������Ϊ00HΪ�������	�˴������ر�
	LD_Write(0x8D, 0xff);		//�ڲ�������� ��ʼ��ʱд��FFH
    delay(100);
	LD_Write(0x87, 0xff);	   //ģ���·���� MP3���ų�ʼ��ʱд FFH
	LD_Write(0x89, 0xff);    //ģ���·���� MP3����ʱд FFH
		delay(100);
	LD_Write(0x22, 0x00);    //FIFO_DATA���޵�8λ
	LD_Write(0x23, 0x00);		 //FIFO_DATA���޸�8λ
	LD_Write(0x20, 0xef);    //FIFO_DATA���޵�8λ
	LD_Write(0x21, 0x07);		 //FIFO_DATA���޸�8λ
	LD_Write(0x24, 0x77);        
	LD_Write(0x25, 0x03);	
	LD_Write(0x26, 0xbb);    
	LD_Write(0x27, 0x01); 	
}
//======================================================
//��������
void LD_AdjustMIX2SPVolume(u8 val)	  
{
	ucSPVol = val;
	val = ((15-val)&0x0f) << 2;	
	LD_Write(0x8E, val | 0xc3); 
	LD_Write(0x87, 0x78); 
}
//======================================================
//����fifo�Ӷ�����fifo�ж�,Ϊ����mp3��׼��
void fill_the_fifo(void)
{
  u8 ucStatus;
	int i = 0;
	ucStatus = LD_Read(0x06);
	//fifo�Ƿ�����
	while ( !(ucStatus&MASK_FIFO_STATUS_AFULL))
	{

		LD_Write(0x01,0xff);
		i++;
		ucStatus = LD_Read(0x06);
	}
}
//==================================================
//���ź���
void LD_play()	  
{
	nMp3Pos=0;
	bMp3Play=1;

	if (nMp3Pos >=  nMp3Size)	  //���������ѷ������
		return ; 

	fill_the_fifo();
	LD_Write(0xBA, 0x00);	  //�жϸ�����Ϣ����������Ϊ00
	LD_Write(0x17, 0x48);	  //д48H���Լ���DSP
	LD_Write(0x33, 0x01);	  //MP3���������� ��ʼ����ʱд��01H, ������д��00H
	LD_Write(0x29, 0x04);	  //�ж����� ��2λ��FIFO �ж����� ��4λ��ͬ���ж����� 1����0������
	
	LD_Write(0x02, 0x01); 	  //FIFO�ж����� ��0λ������FIFO_DATA�жϣ� ��2λ������FIFO_EXT�жϣ�
	LD_Write(0x85, 0x5A);	  //�ڲ��������� ��ʼ��ʱд��52H ����MP3ʱд��5AH (�ı��ڲ�����)

	//EX0=1;		//���ⲿ�ж�0�ж�
	EXTI_ClearITPendingBit(EXTI_Line0);
}

//=============================================================
//ȡASRʶ����
u8 LD_GetResult(void)
{
	u8 res;
	res = LD_Read(0xc5);
	return res;
}
//=============================================================
// Return 1: success.
//	���ʶ��ؼ���������߿���ѧϰ"����ʶ��оƬLD3320�߽��ؼ�.pdf"�й��������������մ�����÷�
u8 LD_AsrAddFixed(void)	  //��ӹؼ����ﵽLD3320оƬ��
{
	u8 k, flag;
	flag = 1;
	for (k=0; k<ITEM_COUNT; k++)
	{	
		if(LD_Check_ASRBusyFlag_b2() == 0)	  //оƬ�ڲ�����
		{
			flag = 0;
			break;
		}
		LD_AsrAddFixed_ByIndex(k);
		delay(100);	
	}
	return flag;
}
//==========================================================
//����ʶ���ʼ��
void LD_Init_ASR(void)	   //����ʶ���ʼ��
{
	nLD_Mode=LD_MODE_ASR_RUN;	  //��ǰ��������ʶ��
	LD_Init_Common();	   //ͨ�ó�ʼ��

	LD_Write(0xBD, 0x00);	 //
	LD_Write(0x17, 0x48);	 //д48H���Լ���DSP	
	delay(100);
	LD_Write(0x3C, 0x80); 
	LD_Write(0x3E, 0x07);
	LD_Write(0x38, 0xff);
	LD_Write(0x3A, 0x07);
	LD_Write(0x40, 0); 
	LD_Write(0x42, 8);
	LD_Write(0x44, 0);    
	LD_Write(0x46, 8); 
	delay(100);
}
//==========================================================
//����ʶ���ʼ��
void LD_AsrStart(void)
{
	LD_Init_ASR();
}
//==========================================================
//��ʼʶ��
//Return 1: success.
u8 LD_AsrRun(void)		
{
	LD_Write(0x35, MIC_VOL);	   //��˷�����
//	LD_Write(0xB3, 0x0D);	// �û��Ķ� �����ֲ� ���B3�Ĵ����ĵ������������Ⱥ�ʶ������Ӱ��

	LD_Write(0x1C, 0x09);	//ADC���ؿ��� д09H Reserve����������
	LD_Write(0xBD, 0x20);	//��ʼ�����ƼĴ��� д��20H��Reserve����������
	LD_Write(0x08, 0x01);	//���FIFO���ݵ�0λ��д��1�����FIFO_DATA ��2λ��д��1�����FIFO_EXT
	delay(100);
	LD_Write(0x08, 0x00);	//���FIFO���ݵ�0λ�����ָ��FIFO����д��һ��00H�� 
	delay(100);

	printf("the BA is %x",LD_Read(0xBA));  //�жϸ�����־λ(������Ϊ00)

	if(LD_Check_ASRBusyFlag_b2() == 0)	   //оƬ�ڲ�����
	{
		printf("\r\nоƬ�ڲ�����\r\n");
		return 0;
	}

	LD_Write(0xB2, 0xff);	  //ASR��DSPæ��״̬ 0x21 ��ʾ�У���ѯ��Ϊ��״̬���Խ�����һ�� ??? Ϊʲô����read??
	LD_Write(0x37, 0x06);	  //ʶ����������·��Ĵ��� д06H ֪ͨDSP��ʼʶ������ �·�ǰ����Ҫ���B2�Ĵ���
	delay(100);
	printf("\r\nAddress(0xBF) value is %x\r\n",LD_Read(0xbf));
	LD_TEST();
	LD_Write(0x1C, 0x0b);	  // ADC���ؿ���  д0BH ��˷�����ADCͨ������  
	LD_Write(0x29, 0x10);	  //�ж����� ��2λ��FIFO �ж����� ��4λ��ͬ���ж����� 1����0������
	
	LD_Write(0xBD, 0x00);	  //��ʼ�����ƼĴ��� д��00 Ȼ������ ΪASRģ�� 
	//EX0=1;		  //���ⲿ�ж�0
	EXTI_ClearITPendingBit(EXTI_Line1);   //2020.1.5 ע��
	return 1;
}

int LD_TEST(void)
{
	 LD_Read(0xBF);
	 LD_Read(0x06);
	 LD_Read(0x35);
	 LD_Read(0xb3);
}

//================================================================
//���оƬ�ڲ����޳���
//Return 1: success.
u8 LD_Check_ASRBusyFlag_b2(void)	  
{
	u8 j;
	u8 flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_Read(0xb2) == 0x21)
		{
			flag = 1;
			break;
		}
//		printf("LD_Check_ASRBusyFlag_b2   \%x\r\n",LD_Read(0xb2) );
		delay(100);		
	}
	return flag;
}
//================================================================
//����
void LD_AsrAddFixed_ByIndex(u8 nIndex)	 
{
	switch(nIndex)
	{
		case  0: LD_AsrAddFixed_ByString(STR_00,nIndex); break;
		case  1: LD_AsrAddFixed_ByString(STR_01,nIndex); break;
		case  2: LD_AsrAddFixed_ByString(STR_02,nIndex); break;
		case  3: LD_AsrAddFixed_ByString(STR_03,nIndex); break;
		case  4: LD_AsrAddFixed_ByString(STR_04,nIndex); break;
		case  5: LD_AsrAddFixed_ByString(STR_05,nIndex); break;
		case  6: LD_AsrAddFixed_ByString(STR_06,nIndex); break;
		case  7: LD_AsrAddFixed_ByString(STR_07,nIndex); break;
		case  8: LD_AsrAddFixed_ByString(STR_08,nIndex); break;
		case  9: LD_AsrAddFixed_ByString(STR_09,nIndex); break;
		case 10: LD_AsrAddFixed_ByString(STR_10,nIndex); break;
		case 11: LD_AsrAddFixed_ByString(STR_11,nIndex); break;
		case 12: LD_AsrAddFixed_ByString(STR_12,nIndex); break;
		case 13: LD_AsrAddFixed_ByString(STR_13,nIndex); break;
		case 14: LD_AsrAddFixed_ByString(STR_14,nIndex); break;
		case 15: LD_AsrAddFixed_ByString(STR_15,nIndex); break;
		case 16: LD_AsrAddFixed_ByString(STR_16,nIndex); break;
		case 17: LD_AsrAddFixed_ByString(STR_17,nIndex); break;
		case 18: LD_AsrAddFixed_ByString(STR_18,nIndex); break;
		case 19: LD_AsrAddFixed_ByString(STR_19,nIndex); break;
		case 20: LD_AsrAddFixed_ByString(STR_20,nIndex); break;
		case 21: LD_AsrAddFixed_ByString(STR_21,nIndex); break;
		case 22: LD_AsrAddFixed_ByString(STR_22,nIndex); break;
		case 23: LD_AsrAddFixed_ByString(STR_23,nIndex); break;
		case 24: LD_AsrAddFixed_ByString(STR_24,nIndex); break;
		case 25: LD_AsrAddFixed_ByString(STR_25,nIndex); break;
		case 26: LD_AsrAddFixed_ByString(STR_26,nIndex); break;
		case 27: LD_AsrAddFixed_ByString(STR_27,nIndex); break;
		case 28: LD_AsrAddFixed_ByString(STR_28,nIndex); break;
		case 29: LD_AsrAddFixed_ByString(STR_29,nIndex); break;
		case 30: LD_AsrAddFixed_ByString(STR_30,nIndex); break;
		case 31: LD_AsrAddFixed_ByString(STR_31,nIndex); break;
		case 32: LD_AsrAddFixed_ByString(STR_32,nIndex); break;
		case 33: LD_AsrAddFixed_ByString(STR_33,nIndex); break;
		case 34: LD_AsrAddFixed_ByString(STR_34,nIndex); break;
		case 35: LD_AsrAddFixed_ByString(STR_35,nIndex); break;
		case 36: LD_AsrAddFixed_ByString(STR_36,nIndex); break;
		case 37: LD_AsrAddFixed_ByString(STR_37,nIndex); break;
		case 38: LD_AsrAddFixed_ByString(STR_38,nIndex); break;
		case 39: LD_AsrAddFixed_ByString(STR_39,nIndex); break;
		case 40: LD_AsrAddFixed_ByString(STR_40,nIndex); break;
		case 41: LD_AsrAddFixed_ByString(STR_41,nIndex); break;
		case 42: LD_AsrAddFixed_ByString(STR_42,nIndex); break;
		case 43: LD_AsrAddFixed_ByString(STR_43,nIndex); break;
		case 44: LD_AsrAddFixed_ByString(STR_44,nIndex); break;
		case 45: LD_AsrAddFixed_ByString(STR_45,nIndex); break;
		case 46: LD_AsrAddFixed_ByString(STR_46,nIndex); break;
		case 47: LD_AsrAddFixed_ByString(STR_47,nIndex); break;
		case 48: LD_AsrAddFixed_ByString(STR_48,nIndex); break;
		case 49: LD_AsrAddFixed_ByString(STR_49,nIndex); break;
	}
}
void LD_AsrAddFixed_ByString(char * pRecogString, u8 k)
{
	u8 nAsrAddLength;
	if (*pRecogString==0)
		return;

	LD_Write(0xc1, k );	   //ASR��ʶ����Index��00H��FFH��
	LD_Write(0xc3, 0 );	   //ASR��ʶ�������ʱд��00
	LD_Write(0x08, 0x04);	 //���FIFO���ݵ�0λ��д��1�����FIFO_DATA ��2λ��д��1�����FIFO_EXT
	delay(100);
	LD_Write(0x08, 0x00);	 //���FIFO���ݵ�0λ�����ָ��FIFO����д��һ��00H��
	delay(100);
	for (nAsrAddLength=0; nAsrAddLength<50; nAsrAddLength++)
	{
		if (pRecogString[nAsrAddLength] == 0)
			break;
		LD_Write(0x5, pRecogString[nAsrAddLength]);	  //дFIFO_EXT���ݿ�
	}
	
	LD_Write(0xb9, nAsrAddLength);	  //��ǰ���ʶ�����ַ������ȳ�ʼ��ʱд��00H ÿ���һ��ʶ����Ҫ�趨һ��
	LD_Write(0xb2, 0xff);	  //DSPæ��״̬ 0x21 ��ʾ�� ���Խ�����һ��	   ??
	LD_Write(0x37, 0x04);	  //����ʶ����������·��Ĵ��� д04H��֪ͨDSPҪ���һ��ʶ���
}
//==================================================
//����һ��ʶ������
u8 RunASR(void)
{
	u8 i=0;
	u8 j=0;
	u8 asrflag=0;
	for (i=0; i<5; i++)	//	��ֹ����Ӳ��ԭ����LD3320оƬ����������������һ������5������ASRʶ������
	{
		LD_AsrStart();
		delay(100);
		if (LD_AsrAddFixed()==0)		//���ʶ�����
		{
			printf("LD3320оƬ�ڲ����ֲ���������������LD3320оƬ\%d\r\n",00);
			LD_Reset();			//	LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			delay(100);			//	���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}
		printf("LD_AsrAddFixed is %x\r\n",LD_Read(0xbf));
		LD_TEST();
		delay(100);
		j= LD_AsrRun();
		if (j == 0)
		{
			LD_Reset();			//	LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			delay(100);			//	���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}
		asrflag=1;
		break;						//	ASR���������ɹ����˳���ǰforѭ������ʼ�ȴ�LD3320�ͳ����ж��ź�
	}	
	return asrflag;
}
//================================================================\
//LD3320��ʼ��
void LD3320_Init(void)
{
	LD3320_GPIO_Cfg();	
	LD3320_EXTI_Cfg();
	LD_Reset();
}

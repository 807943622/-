#include "sys.h"
#include "delay.h"
#include "timer.h"
#include "oled.h"
#include "usart.h"
#include "DataScope_DP.h"
#include "FFT.h"
#include "stm32_dsp.h"
//#include "table_fft.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/4
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

int Fileter_Num=50;

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//��ʼ��TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM2 ��ʼ������
void TIM2_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);//��ʼ��TIM2
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //����ʱ��2�����ж�
	TIM_Cmd(TIM2,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM4 ��ʼ������
void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///ʹ��TIM4ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//��ʼ��TIM4
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //����ʱ��2�����ж�
	TIM_Cmd(TIM4,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM5 ��ʼ������
void TIM5_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///ʹ��TIM3ʱ��
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);//��ʼ��TIM5
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //����ʱ��2�����ж�
	TIM_Cmd(TIM5,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM6 ��ʼ������
void TIM6_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);  ///ʹ��TIM6ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Period = arr; 	//�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE); //����ʱ��6�����ж�
	TIM_Cmd(TIM6,ENABLE); //ʹ�ܶ�ʱ��6
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM6_DAC_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
}

/*-------------------------------------------------*/
/*����������ʱ��3ʹ��30s��ʱ                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM5_ENABLE_30S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //����һ�����ö�ʱ���ı���
	NVIC_InitTypeDef NVIC_InitStructure;                          //����һ�������жϵı���
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //�����ж��������飺��2�� �������ȼ���0 1 2 3 �����ȼ���0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);           //ʹ��TIM5ʱ��	
	TIM_DeInit(TIM5);                                             //��ʱ��3�Ĵ����ָ�Ĭ��ֵ	
	TIM_TimeBaseInitStructure.TIM_Period = 60000-1; 	          //�����Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=36000-1;              //���ö�ʱ��Ԥ��Ƶ��
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1��Ƶ
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);            //����TIM5
	
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);                    //�������жϱ�־λ
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);                      //ʹ��TIM5����ж�    
	TIM_Cmd(TIM5,ENABLE);                                         //��TIM5                          
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn;                 //����TIM5�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x03;       //��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;              //�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);                               //�����ж�
}
/*-------------------------------------------------*/
/*����������ʱ��3ʹ��2s��ʱ                        */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM5_ENABLE_2S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //����һ�����ö�ʱ���ı���
	NVIC_InitTypeDef NVIC_InitStructure;                          //����һ�������жϵı���
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //�����ж��������飺��2�� �������ȼ���0 1 2 3 �����ȼ���0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);           //ʹ��TIM5ʱ��
	TIM_DeInit(TIM5);                                             //��ʱ��3�Ĵ����ָ�Ĭ��ֵ	
	TIM_TimeBaseInitStructure.TIM_Period = 20000-1; 	          //�����Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=7200-1;               //���ö�ʱ��Ԥ��Ƶ��
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1��Ƶ
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);            //����TIM5
	
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);                    //�������жϱ�־λ
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);                      //ʹ��TIM5����ж�    
	TIM_Cmd(TIM5,ENABLE);                                         //��TIM5                          
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn;                 //����TIM5�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02;       //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;              //�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);                               //�����ж�
}

/*-------------------------------------------------*/
/*����������ʱ��4ʹ��30s��ʱ                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM4_ENABLE_30S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //����һ�����ö�ʱ���ı���
	NVIC_InitTypeDef NVIC_InitStructure;                          //����һ�������жϵı���
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //�����ж��������飺��2�� �������ȼ���0 1 2 3 �����ȼ���0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);           //ʹ��TIM4ʱ��	
	TIM_DeInit(TIM4);                                             //��ʱ��2�Ĵ����ָ�Ĭ��ֵ	
	TIM_TimeBaseInitStructure.TIM_Period = 6000-1; 	          //�����Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=36000-1;              //���ö�ʱ��Ԥ��Ƶ��
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1��Ƶ
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);            //����TIM4
	
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);                    //�������жϱ�־λ
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);                      //ʹ��TIM4����ж�    
	TIM_Cmd(TIM4,ENABLE);                                         //��TIM4                          
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;                 //����TIM4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x03;       //��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;              //�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);                               //�����ж�
}



unsigned long int voice_calue[200];
unsigned long long int L_result;
unsigned long int R_voice_calue;
double  PC_dis;
#define WS_H GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define WS_L GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define SCK_H GPIO_SetBits(GPIOB, GPIO_Pin_13)
#define SCK_L GPIO_ResetBits(GPIOB, GPIO_Pin_13)

int count=0;
int tmp=0;


extern long lBufInArray[NPT];
extern long lBufOutArray[NPT/2];
extern long lBufMagArray[NPT/2];

int FREE=16777215;
int value_;
//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
	int FFT_count=0;
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		for(FFT_count=0;FFT_count<=255;FFT_count++)
{
	
	value_=0;
		delay_us(1);
		WS_H;
		delay_us(1);
		WS_L;
 
		delay_us(1);
		WS_H;
		SCK_H;
		delay_us(1);
		WS_L;
		SCK_L;
		delay_us(1);
		
		SCK_H;		//��һ��λ��
		delay_us(1);
//		SCK_L;
//		delay_us(1);
//		SCK_H;		//�ڶ���λ��
//		delay_us(1);
//		SCK_L;
		voice_calue[0]=0;
		R_voice_calue = 0;
		delay_us(1);
		for(count=23;count>=0;count--)
		{	
			value_ = (value_ << 1); 
			tmp = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15);
			SCK_L;
			if(tmp) value_|=0x01;//SD
			delay_us(1);
			SCK_H;
			delay_us(1);
		}
		for(count=6;count>=0;count--)
		{
			SCK_L;
			delay_us(1);
			SCK_H;
			delay_us(1);
		}
		WS_H;
		delay_us(1);
		for(count=31;count>=0;count--)
		{
//			delay_us(1);
//			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)
//			R_voice_calue|=0x01;//SD
//			R_voice_calue = (R_voice_calue << 1); 
			SCK_L;
			delay_us(1);
			SCK_H;
			delay_us(1);
		}
		
//		for(count=Fileter_Num-1;count>=1;count--)
//		{
//			voice_calue[count] = voice_calue[count-1];
//		}
//		for(count=Fileter_Num-1;count>=0;count--)
//		{
//			L_result += voice_calue[count];
//		}
//		 L_result = L_result/Fileter_Num;
		//voice_calue=SPI_I2S_ReceiveData(I2S2ext);
		//OLED_ShowNum(0,25,(int)L_result,12,12);
		//OLED_ShowNum(0,48,(int)R_voice_calue,12,12);
//		OLED_Refresh_Gram();        //������ʾ��OLED
		//printf("\r\nThe voice calue is %d \r\n",voice_calue);
		//printf("\r\nThe result is %d\r\n",(int)L_result);
		PC_dis = value_;
		
			lBufInArray[FFT_count] = ((signed short)(((float)PC_dis/(FREE))*4096-2048)) << 16;
			
//			if(FFT_count>=256)
//			{
//				FFT_count=0;
//			}
//			Creat_Single();
			

	}
}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}

//TIM2 ������
void TIM2_IRQHandler(void)
{
		int i=0,Send_Count=0;
	
		if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //����ж�
		{
			
			cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
			GetPowerMag();
			display3();
			

//			//��λ����ʾ
//				DataScope_Get_Channel_Data((float)PC_dis/16777215, 1 );
//////			DataScope_Get_Channel_Data(500* tan(a), 2 );
//////			DataScope_Get_Channel_Data( 500*cos(a), 3 ); 
//////			DataScope_Get_Channel_Data( 100*a , 4 );   
//////			DataScope_Get_Channel_Data(12, 5 );
//////			DataScope_Get_Channel_Data(13 , 6 );
//////			DataScope_Get_Channel_Data(14, 7 );
//////			DataScope_Get_Channel_Data( 15, 8 ); 
//////			DataScope_Get_Channel_Data(16, 9 );  
//////			DataScope_Get_Channel_Data(17 , 10);
//			Send_Count = DataScope_Data_Generate(2);
//			for( i = 0 ; i < Send_Count; i++) 
//			{
//			while((USART1->SR&0X40)==0);  
//			USART1->DR = DataScope_OutPut_Buffer[i]; 
//			}
		}
	
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //����жϱ�־λ
}

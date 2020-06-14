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
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/4
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

int Fileter_Num=50;

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM2 初始化函数
void TIM2_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);//初始化TIM2
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //允许定时器2更新中断
	TIM_Cmd(TIM2,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM4 初始化函数
void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///使能TIM4时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//初始化TIM4
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //允许定时器2更新中断
	TIM_Cmd(TIM4,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM5 初始化函数
void TIM5_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);//初始化TIM5
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //允许定时器2更新中断
	TIM_Cmd(TIM5,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//TIM6 初始化函数
void TIM6_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);  ///使能TIM6时钟
	
	TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE); //允许定时器6更新中断
	TIM_Cmd(TIM6,ENABLE); //使能定时器6
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM6_DAC_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
}

/*-------------------------------------------------*/
/*函数名：定时器3使能30s定时                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM5_ENABLE_30S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //定义一个设置定时器的变量
	NVIC_InitTypeDef NVIC_InitStructure;                          //定义一个设置中断的变量
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //设置中断向量分组：第2组 抢先优先级：0 1 2 3 子优先级：0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);           //使能TIM5时钟	
	TIM_DeInit(TIM5);                                             //定时器3寄存器恢复默认值	
	TIM_TimeBaseInitStructure.TIM_Period = 60000-1; 	          //设置自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=36000-1;              //设置定时器预分频数
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1分频
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);            //设置TIM5
	
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);                    //清除溢出中断标志位
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);                      //使能TIM5溢出中断    
	TIM_Cmd(TIM5,ENABLE);                                         //开TIM5                          
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn;                 //设置TIM5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x03;       //抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;              //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //中断通道使能
	NVIC_Init(&NVIC_InitStructure);                               //设置中断
}
/*-------------------------------------------------*/
/*函数名：定时器3使能2s定时                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM5_ENABLE_2S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //定义一个设置定时器的变量
	NVIC_InitTypeDef NVIC_InitStructure;                          //定义一个设置中断的变量
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //设置中断向量分组：第2组 抢先优先级：0 1 2 3 子优先级：0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);           //使能TIM5时钟
	TIM_DeInit(TIM5);                                             //定时器3寄存器恢复默认值	
	TIM_TimeBaseInitStructure.TIM_Period = 20000-1; 	          //设置自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=7200-1;               //设置定时器预分频数
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1分频
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);            //设置TIM5
	
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);                    //清除溢出中断标志位
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);                      //使能TIM5溢出中断    
	TIM_Cmd(TIM5,ENABLE);                                         //开TIM5                          
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn;                 //设置TIM5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02;       //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;              //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //中断通道使能
	NVIC_Init(&NVIC_InitStructure);                               //设置中断
}

/*-------------------------------------------------*/
/*函数名：定时器4使能30s定时                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM4_ENABLE_30S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //定义一个设置定时器的变量
	NVIC_InitTypeDef NVIC_InitStructure;                          //定义一个设置中断的变量
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //设置中断向量分组：第2组 抢先优先级：0 1 2 3 子优先级：0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);           //使能TIM4时钟	
	TIM_DeInit(TIM4);                                             //定时器2寄存器恢复默认值	
	TIM_TimeBaseInitStructure.TIM_Period = 6000-1; 	          //设置自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=36000-1;              //设置定时器预分频数
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1分频
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);            //设置TIM4
	
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);                    //清除溢出中断标志位
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);                      //使能TIM4溢出中断    
	TIM_Cmd(TIM4,ENABLE);                                         //开TIM4                          
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;                 //设置TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x03;       //抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;              //子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //中断通道使能
	NVIC_Init(&NVIC_InitStructure);                               //设置中断
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
//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
	int FFT_count=0;
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
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
		
		SCK_H;		//第一个位过
		delay_us(1);
//		SCK_L;
//		delay_us(1);
//		SCK_H;		//第二个位过
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
//		OLED_Refresh_Gram();        //更新显示到OLED
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
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}

//TIM2 处理函数
void TIM2_IRQHandler(void)
{
		int i=0,Send_Count=0;
	
		if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //溢出中断
		{
			
			cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
			GetPowerMag();
			display3();
			

//			//上位机显示
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
	
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //清除中断标志位
}

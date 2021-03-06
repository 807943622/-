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
 
u8  nLD_Mode = LD_MODE_IDLE;		//用来记录当前是在进行ASR识别还是在播放MP3
u8 ucRegVal; 						        //有无识别结果			
extern u8 nAsrStatus;					  //表示一次识别流程结束后有无结果
u8 ucHighInt;
u8 ucLowInt;
u8 bMp3Play=0;							    //用来记录播放MP3的状态
u32 nMp3Size=0;
u32 nMp3Pos=0;
u8 ucSPVol=15; // MAX=15 MIN=0	//	Speaker喇叭输出的音量
unsigned long fpPoint=0;
/***********************************************************
* 名    称：LD3320_GPIO_Cfg(void)
* 功    能：初始化需要用到的IO口
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
**********************************************************/ 
void LD3320_GPIO_Cfg(void)
{	
		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC ,ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//RSET
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输出模式
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉 = GPIO_PuPd_UP;//推挽输出
		GPIO_Init(GPIOA,&GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2;//RSET
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉 = GPIO_PuPd_UP;//推挽输出
		GPIO_Init(GPIOA,&GPIO_InitStructure);

		GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_2);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOC,&GPIO_InitStructure);

}

/***********************************************************
* 名    称： LD3320_EXTI_Cfg(void) 
* 功    能： 外部中断功能配置和相关端口配置
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
**********************************************************/ 
void LD3320_EXTI_Cfg(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  //中断引脚配置
  RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);//PC0 连接到中断线1
	
	//外部中断线配置
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//中断线使能
  EXTI_Init(&EXTI_InitStructure);//配置
		
	GPIO_SetBits(GPIOC,GPIO_Pin_0);	 //默认拉高中断引脚

	EXTI_ClearFlag(EXTI_Line0);
//	EXTI_ClearITPendingBit(EXTI_Line1);

	//中断嵌套配置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//外部中断1
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级3
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
}


/***********************************************************
* 名    称：  EXTI1_IRQHandler(void)
* 功    能： 外部中断函数
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
**********************************************************/ 
//外部中断0服务程序
void EXTI0_IRQHandler(void)
{
	delay(10);	//消抖
	if(EXTI_GetITStatus(EXTI_Line0) != RESET ) 
	{
		ProcessInt0();
		printf("\r\nInter the handle!\r\n");
		EXTI_ClearFlag(EXTI_Line0);
		EXTI_ClearITPendingBit(EXTI_Line0);//清除LINE1上的中断标志位 
	}
	 printf("\r\nEXIT from the exit!\r\n");
	EXTI_ClearFlag(EXTI_Line0);
	 //EXTI_ClearITPendingBit(EXTI_Line1);
}

//=======================================
//us级延时
void delay(unsigned long uldata)
{
	delay_us(uldata*10);
	return;
//	unsigned int i;
//	unsigned char j;
//	for(i=uldata*100;i>0;i--)
//		for(j=5;j>0;j--);   //j=110
}
//nop_延迟
void delay_nop(unsigned long uldata)
{
	unsigned int i;
	for(i=uldata;i>0;i--);
}
/***********************************************************
* 名    称：void LD_reset(void)
* 功    能：LD芯片硬件初始化
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
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
* 名    称：void LD_WriteReg(uint8 data1,uint8 data2)
* 功    能： 写ld3320寄存器
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
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
* 名    称：uint8 LD_ReadReg(uint8 reg_add)
* 功    能：读ld3320寄存器
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
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
//播放和语音识别中断
void ProcessInt0(void)
{
	u8 nAsrResCount=0;

//	EX0=0;	  //关闭外部中断0中断
	ucRegVal = LD_Read(0x2B);
	if(nLD_Mode == LD_MODE_ASR_RUN)		//当前进行语音识别
	{
		// 语音识别产生的中断
		// （有声音输入，不论识别成功或失败都有中断）
		LD_Write(0x29,0) ;					//中断允许 FIFO 中断允许 0表示不允许 
		LD_Write(0x02,0) ;					// FIFO中断允许	 FIFO_DATA FIFO_EXT中断   不允许
		delay(100);
		if((ucRegVal & 0x10) &&		  //2b第四位为1 芯片内部FIFO中断发生 MP3播放时会产生中断标志请求外部MCU向FIFO_DATA中Reload数
			LD_Read(0xb2)==0x21 && 	  //读b2得到0x21表示闲可以进行下一步ASR动作
			LD_Read(0xbf)==0x35)		  //读到数值为0x35，可以确定是一次语音识别流程正常结束
		{
			printf("\r\n进入判断");
			nAsrResCount = LD_Read(0xba);	    //ASR：中断时，判断语音识别有几个识别候选
			printf("\r\n可能结果有 %d 个\r\n",nAsrResCount);
			if(nAsrResCount==1||(nAsrResCount>0 && nAsrResCount<=4)) 
			{
				nAsrStatus=LD_ASR_FOUNDOK;
				return;
//				LD_Read(0xc5);
			}

			if(nAsrResCount>0 && nAsrResCount<=4) 	  //1 – 4: N个识别候选 0或者大于4：没有识别候选
			{
				nAsrStatus=LD_ASR_FOUNDOK;		 //表示一次识别流程结束后，有一个识别结果
			}
			else
		    {
				nAsrStatus=LD_ASR_FOUNDZERO;	   //表示一次识别流程结束后，没有识别结果
			}	
		}
		else
		{
			nAsrStatus=LD_ASR_FOUNDZERO;	   //表示一次识别流程结束后，没有识别结果
		}

		LD_Write(0x2b, 0);
    LD_Write(0x1C,0);	  //ADC开关控制 写00H ADC不可用
		
		return;
	}
//=============================================================================================
	// 声音播放产生的中断，有三种：
	// A. 声音数据已全部播放完。
	// B. 声音数据已发送完毕。
	// C. 声音数据暂时将要用完，需要放入新的数据。	
		ucHighInt = LD_Read(0x29); 
		ucLowInt=LD_Read(0x02); 
		LD_Write(0x29,0);
		LD_Write(0x02,0);
    if(LD_Read(0xBA)&CAUSE_MP3_SONG_END)
    {
	// A. 声音数据已全部播放完。

			LD_Write(0x2B,  0);
			LD_Write(0xBA, 0);	
			LD_Write(0xBC,0x0);	
			bMp3Play=0;					// 声音数据全部播放完后，修改bMp3Play的变量
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
	// B. 声音数据已发送完毕。

		LD_Write(0xBC, 0x01);
		LD_Write(0x29, 0x10);
		f_close(&fsrc);
		nLD_Mode = LD_MODE_ASR_RUN;
		nAsrStatus = LD_ASR_NONE;
		return;	
	}

	// C. 声音数据暂时将要用完，需要放入新的数据。	

	LD_ReloadMp3Data_Again();
			
	LD_Write(0x29,ucHighInt); 
	LD_Write(0x02,ucLowInt) ;
}

/*  继续读取mp3文件数据到fifo,直到fifo满
 *	边写mp3文件数据到fifo时,LD3320会变解码播放
 *	当然写mp3文件数据到fifo的时间会短过声音的时间
 *	当声音快播放完毕的时候会进入ProcessInt0函数
 *	ProcessInt0函数又会调用此函数,所以声音得以连续
 */
void LD_ReloadMp3Data_Again(void)
{
	u8 val;
	u8 ucStatus;
	UINT br;
	
	ucStatus = LD_Read(0x06);
	//fifo是否满了
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

		//等待MP3播放完毕
		while(!(LD_Read(0xBA)&CAUSE_MP3_SONG_END));

		LD_Write(0x2B,  0);
      	LD_Write(0xBA, 0);	
		LD_Write(0xBC,0x0);	
		bMp3Play=0;					// 声音数据全部播放完后，修改bMp3Play的变量
		LD_Write(0x08,1);

    LD_Write(0x08,0);
		LD_Write(0x33, 0);

		f_close(&fsrc);
		nLD_Mode = LD_MODE_ASR_RUN;
		nAsrStatus = LD_ASR_NONE;
	}
		
}

//=========================================================
//通用初始化
void LD_Init_Common(void)	
{
	bMp3Play = 0;

	LD_Read(0x06);  
	LD_Write(0x17, 0x35); //软复位LD3320
	delay(100);
	LD_Read(0x06);  

	LD_Write(0x89, 0x03);  //模拟电路控制 初始化时写03H
	delay(100);
	LD_Write(0xCF, 0x43);  //内部省电模式设置 初始化时写入43H 
	delay(100);
	LD_Write(0xCB, 0x02);
	
	/*PLL setting*/
	LD_Write(0x11, LD_PLL_11);   		//时钟频率设置1  
	if (nLD_Mode == LD_MODE_MP3)    //MP3播放
	{
		LD_Write(0x1E, 0x00); 					 	 //ADC专用控制，应初始化为00H
		LD_Write(0x19, LD_PLL_MP3_19);  	 //时钟频率设置2
		LD_Write(0x1B, LD_PLL_MP3_1B); 	   //时钟频率设置3
		LD_Write(0x1D, LD_PLL_MP3_1D);		 //时钟频率设置4
	}
	else						   									 //语音识别
	{
		LD_Write(0x1E,0x00);	 					   //ADC专用控制，应初始化为00H
		LD_Write(0x19, LD_PLL_ASR_19); 		 //时钟频率设置2
		LD_Write(0x1B, LD_PLL_ASR_1B);		 //时钟频率设置3	
	    LD_Write(0x1D, LD_PLL_ASR_1D);	 //时钟频率设置4
	}
	delay(100);
	
	LD_Write(0xCD, 0x04);    //DSP休眠设置 初始化时写入04H 允许DSP休眠
	LD_Write(0x17, 0x4C); 	 //写4CH可以使DSP休眠，比较省电
	delay(100);
	LD_Write(0xB9, 0x00);		 //ASR：当前添加识别句的字符串长度（拼音字符串）初始化时写入00H	 每添加一条识别句后要设定一次
	LD_Write(0xCF, 0x4F); 	 //内部省电模蔪`````````````````````````````缴柚� MP3初始化和ASR初始化时写入 4FH
	LD_Write(0x6F, 0xFF); 	 //对芯片进行初始化时设置为0xFF
}
//==================================================
//播放初始化
void LD_Init_MP3(void)	
{
	nLD_Mode = LD_MODE_MP3;	   //当前进行MP3播放
	LD_Init_Common();		  		 //通用初始化

	LD_Write(0xBD,0x02);	 		 	//内部增益控制 初始化时写入FFH
	printf("0x02 %x\r\n",LD_Read(0xBD));
	LD_Write(0x17, 0x48);				//写48H可以激活DSP
		delay(100);
	printf("0x48 %x\r\n",LD_Read(0x17));
	LD_Write(0x85, 0x52); 	//内部反馈设置 初始化时写入52H
	printf("0x52 %x\r\n",LD_Read(0x85));
	LD_Write(0x8F, 0x00);  	//LineOut(线路输出)选择 初始化时写入00H
	LD_Write(0x81, 0x00);		//耳机左音量 设置为00H为最大音量
	LD_Write(0x83, 0x00);		//耳机右音量 设置为00H为最大音量
	LD_Write(0x8E, 0xff);		//喇叭输出音量  本寄存器设置为00H为最大音量	此处声音关闭
	LD_Write(0x8D, 0xff);		//内部增益控制 初始化时写入FFH
    delay(100);
	LD_Write(0x87, 0xff);	   //模拟电路控制 MP3播放初始化时写 FFH
	LD_Write(0x89, 0xff);    //模拟电路控制 MP3播放时写 FFH
		delay(100);
	LD_Write(0x22, 0x00);    //FIFO_DATA下限低8位
	LD_Write(0x23, 0x00);		 //FIFO_DATA下限高8位
	LD_Write(0x20, 0xef);    //FIFO_DATA上限低8位
	LD_Write(0x21, 0x07);		 //FIFO_DATA上限高8位
	LD_Write(0x24, 0x77);        
	LD_Write(0x25, 0x03);	
	LD_Write(0x26, 0xbb);    
	LD_Write(0x27, 0x01); 	
}
//======================================================
//音量调整
void LD_AdjustMIX2SPVolume(u8 val)	  
{
	ucSPVol = val;
	val = ((15-val)&0x0f) << 2;	
	LD_Write(0x8E, val | 0xc3); 
	LD_Write(0x87, 0x78); 
}
//======================================================
//填满fifo从而触发fifo中断,为播放mp3做准备
void fill_the_fifo(void)
{
  u8 ucStatus;
	int i = 0;
	ucStatus = LD_Read(0x06);
	//fifo是否满了
	while ( !(ucStatus&MASK_FIFO_STATUS_AFULL))
	{

		LD_Write(0x01,0xff);
		i++;
		ucStatus = LD_Read(0x06);
	}
}
//==================================================
//播放函数
void LD_play()	  
{
	nMp3Pos=0;
	bMp3Play=1;

	if (nMp3Pos >=  nMp3Size)	  //声音数据已发送完毕
		return ; 

	fill_the_fifo();
	LD_Write(0xBA, 0x00);	  //中断辅助信息，（读或设为00
	LD_Write(0x17, 0x48);	  //写48H可以激活DSP
	LD_Write(0x33, 0x01);	  //MP3播放用设置 开始播放时写入01H, 播放完写入00H
	LD_Write(0x29, 0x04);	  //中断允许 第2位：FIFO 中断允许 第4位：同步中断允许 1允许；0不允许
	
	LD_Write(0x02, 0x01); 	  //FIFO中断允许 第0位：允许FIFO_DATA中断； 第2位：允许FIFO_EXT中断；
	LD_Write(0x85, 0x5A);	  //内部反馈设置 初始化时写入52H 播放MP3时写入5AH (改变内部增益)

	//EX0=1;		//开外部中断0中断
	EXTI_ClearITPendingBit(EXTI_Line0);
}

//=============================================================
//取ASR识别结果
u8 LD_GetResult(void)
{
	u8 res;
	res = LD_Read(0xc5);
	return res;
}
//=============================================================
// Return 1: success.
//	添加识别关键词语，开发者可以学习"语音识别芯片LD3320高阶秘籍.pdf"中关于垃圾词语吸收错误的用法
u8 LD_AsrAddFixed(void)	  //添加关键词语到LD3320芯片中
{
	u8 k, flag;
	flag = 1;
	for (k=0; k<ITEM_COUNT; k++)
	{	
		if(LD_Check_ASRBusyFlag_b2() == 0)	  //芯片内部出错
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
//语音识别初始化
void LD_Init_ASR(void)	   //语音识别初始化
{
	nLD_Mode=LD_MODE_ASR_RUN;	  //当前进行语音识别
	LD_Init_Common();	   //通用初始化

	LD_Write(0xBD, 0x00);	 //
	LD_Write(0x17, 0x48);	 //写48H可以激活DSP	
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
//语音识别初始化
void LD_AsrStart(void)
{
	LD_Init_ASR();
}
//==========================================================
//开始识别
//Return 1: success.
u8 LD_AsrRun(void)		
{
	LD_Write(0x35, MIC_VOL);	   //麦克风音量
//	LD_Write(0xB3, 0x0D);	// 用户阅读 开发手册 理解B3寄存器的调整对于灵敏度和识别距离的影响

	LD_Write(0x1C, 0x09);	//ADC开关控制 写09H Reserve保留命令字
	LD_Write(0xBD, 0x20);	//初始化控制寄存器 写入20H；Reserve保留命令字
	LD_Write(0x08, 0x01);	//清除FIFO内容第0位：写入1→清除FIFO_DATA 第2位：写入1→清除FIFO_EXT
	delay(100);
	LD_Write(0x08, 0x00);	//清除FIFO内容第0位（清除指定FIFO后再写入一次00H） 
	delay(100);

	printf("the BA is %x",LD_Read(0xBA));  //中断辅助标志位(读或设为00)

	if(LD_Check_ASRBusyFlag_b2() == 0)	   //芯片内部出错
	{
		printf("\r\n芯片内部出错\r\n");
		return 0;
	}

	LD_Write(0xB2, 0xff);	  //ASR：DSP忙闲状态 0x21 表示闲，查询到为闲状态可以进行下一步 ??? 为什么不是read??
	LD_Write(0x37, 0x06);	  //识别控制命令下发寄存器 写06H 通知DSP开始识别语音 下发前，需要检查B2寄存器
	delay(100);
	printf("\r\nAddress(0xBF) value is %x\r\n",LD_Read(0xbf));
	LD_TEST();
	LD_Write(0x1C, 0x0b);	  // ADC开关控制  写0BH 麦克风输入ADC通道可用  
	LD_Write(0x29, 0x10);	  //中断允许 第2位：FIFO 中断允许 第4位：同步中断允许 1允许；0不允许
	
	LD_Write(0xBD, 0x00);	  //初始化控制寄存器 写入00 然后启动 为ASR模块 
	//EX0=1;		  //开外部中断0
	EXTI_ClearITPendingBit(EXTI_Line1);   //2020.1.5 注释
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
//检测芯片内部有无出错
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
//索引
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

	LD_Write(0xc1, k );	   //ASR：识别字Index（00H—FFH）
	LD_Write(0xc3, 0 );	   //ASR：识别字添加时写入00
	LD_Write(0x08, 0x04);	 //清除FIFO内容第0位：写入1→清除FIFO_DATA 第2位：写入1→清除FIFO_EXT
	delay(100);
	LD_Write(0x08, 0x00);	 //清除FIFO内容第0位（清除指定FIFO后再写入一次00H）
	delay(100);
	for (nAsrAddLength=0; nAsrAddLength<50; nAsrAddLength++)
	{
		if (pRecogString[nAsrAddLength] == 0)
			break;
		LD_Write(0x5, pRecogString[nAsrAddLength]);	  //写FIFO_EXT数据口
	}
	
	LD_Write(0xb9, nAsrAddLength);	  //当前添加识别句的字符串长度初始化时写入00H 每添加一条识别句后要设定一次
	LD_Write(0xb2, 0xff);	  //DSP忙闲状态 0x21 表示闲 可以进行下一步	   ??
	LD_Write(0x37, 0x04);	  //语音识别控制命令下发寄存器 写04H：通知DSP要添加一项识别句
}
//==================================================
//启动一次识别流程
u8 RunASR(void)
{
	u8 i=0;
	u8 j=0;
	u8 asrflag=0;
	for (i=0; i<5; i++)	//	防止由于硬件原因导致LD3320芯片工作不正常，所以一共尝试5次启动ASR识别流程
	{
		LD_AsrStart();
		delay(100);
		if (LD_AsrAddFixed()==0)		//添加识别口令
		{
			printf("LD3320芯片内部出现不正常，立即重启LD3320芯片\%d\r\n",00);
			LD_Reset();			//	LD3320芯片内部出现不正常，立即重启LD3320芯片
			delay(100);			//	并从初始化开始重新ASR识别流程
			continue;
		}
		printf("LD_AsrAddFixed is %x\r\n",LD_Read(0xbf));
		LD_TEST();
		delay(100);
		j= LD_AsrRun();
		if (j == 0)
		{
			LD_Reset();			//	LD3320芯片内部出现不正常，立即重启LD3320芯片
			delay(100);			//	并从初始化开始重新ASR识别流程
			continue;
		}
		asrflag=1;
		break;						//	ASR流程启动成功，退出当前for循环。开始等待LD3320送出的中断信号
	}	
	return asrflag;
}
//================================================================\
//LD3320初始化
void LD3320_Init(void)
{
	LD3320_GPIO_Cfg();	
	LD3320_EXTI_Cfg();
	LD_Reset();
}

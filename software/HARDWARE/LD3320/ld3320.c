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
 
u8  nLD_Mode = LD_MODE_IDLE;		//ÓÃÀ´¼ÇÂ¼µ±Ç°ÊÇÔÚ½øĞĞASRÊ¶±ğ»¹ÊÇÔÚ²¥·ÅMP3
u8 ucRegVal; 						        //ÓĞÎŞÊ¶±ğ½á¹û			
extern u8 nAsrStatus;					  //±íÊ¾Ò»´ÎÊ¶±ğÁ÷³Ì½áÊøºóÓĞÎŞ½á¹û
u8 ucHighInt;
u8 ucLowInt;
u8 bMp3Play=0;							    //ÓÃÀ´¼ÇÂ¼²¥·ÅMP3µÄ×´Ì¬
u32 nMp3Size=0;
u32 nMp3Pos=0;
u8 ucSPVol=15; // MAX=15 MIN=0	//	SpeakerÀ®°ÈÊä³öµÄÒôÁ¿
unsigned long fpPoint=0;
/***********************************************************
* Ãû    ³Æ£ºLD3320_GPIO_Cfg(void)
* ¹¦    ÄÜ£º³õÊ¼»¯ĞèÒªÓÃµ½µÄIO¿Ú
* Èë¿Ú²ÎÊı£º  
* ³ö¿Ú²ÎÊı£º
* Ëµ    Ã÷£º
* µ÷ÓÃ·½·¨£º 
**********************************************************/ 
void LD3320_GPIO_Cfg(void)
{	
		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC ,ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//RSET
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//ÆÕÍ¨Êä³öÄ£Ê½
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//ÉÏÀ­ = GPIO_PuPd_UP;//ÍÆÍìÊä³ö
		GPIO_Init(GPIOA,&GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2;//RSET
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//ÆÕÍ¨Êä³öÄ£Ê½
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//ÍÆÍìÊä³ö
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//ÉÏÀ­ = GPIO_PuPd_UP;//ÍÆÍìÊä³ö
		GPIO_Init(GPIOA,&GPIO_InitStructure);

		GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_2);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//ÆÕÍ¨Êä³öÄ£Ê½
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//ÍÆÍìÊä³ö
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOC,&GPIO_InitStructure);

}

/***********************************************************
* Ãû    ³Æ£º LD3320_EXTI_Cfg(void) 
* ¹¦    ÄÜ£º Íâ²¿ÖĞ¶Ï¹¦ÄÜÅäÖÃºÍÏà¹Ø¶Ë¿ÚÅäÖÃ
* Èë¿Ú²ÎÊı£º  
* ³ö¿Ú²ÎÊı£º
* Ëµ    Ã÷£º
* µ÷ÓÃ·½·¨£º 
**********************************************************/ 
void LD3320_EXTI_Cfg(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  //ÖĞ¶ÏÒı½ÅÅäÖÃ
  RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//ÉÏÀ­
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//Ê¹ÄÜSYSCFGÊ±ÖÓ
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);//PC0 Á¬½Óµ½ÖĞ¶ÏÏß1
	
	//Íâ²¿ÖĞ¶ÏÏßÅäÖÃ
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//ÖĞ¶ÏÊÂ¼ş
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //ÏÂ½µÑØ´¥·¢
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ÖĞ¶ÏÏßÊ¹ÄÜ
  EXTI_Init(&EXTI_InitStructure);//ÅäÖÃ
		
	GPIO_SetBits(GPIOC,GPIO_Pin_0);	 //Ä¬ÈÏÀ­¸ßÖĞ¶ÏÒı½Å

	EXTI_ClearFlag(EXTI_Line0);
//	EXTI_ClearITPendingBit(EXTI_Line1);

	//ÖĞ¶ÏÇ¶Ì×ÅäÖÃ
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//Íâ²¿ÖĞ¶Ï1
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//ÇÀÕ¼ÓÅÏÈ¼¶3
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//×ÓÓÅÏÈ¼¶2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//Ê¹ÄÜÍâ²¿ÖĞ¶ÏÍ¨µÀ
  NVIC_Init(&NVIC_InitStructure);//ÅäÖÃ
}


/***********************************************************
* Ãû    ³Æ£º  EXTI1_IRQHandler(void)
* ¹¦    ÄÜ£º Íâ²¿ÖĞ¶Ïº¯Êı
* Èë¿Ú²ÎÊı£º  
* ³ö¿Ú²ÎÊı£º
* Ëµ    Ã÷£º
* µ÷ÓÃ·½·¨£º 
**********************************************************/ 
//Íâ²¿ÖĞ¶Ï0·şÎñ³ÌĞò
void EXTI0_IRQHandler(void)
{
	delay(10);	//Ïû¶¶
	if(EXTI_GetITStatus(EXTI_Line0) != RESET ) 
	{
		ProcessInt0();
		printf("\r\nInter the handle!\r\n");
		EXTI_ClearFlag(EXTI_Line0);
		EXTI_ClearITPendingBit(EXTI_Line0);//Çå³ıLINE1ÉÏµÄÖĞ¶Ï±êÖ¾Î» 
	}
	 printf("\r\nEXIT from the exit!\r\n");
	EXTI_ClearFlag(EXTI_Line0);
	 //EXTI_ClearITPendingBit(EXTI_Line1);
}

//=======================================
//us¼¶ÑÓÊ±
void delay(unsigned long uldata)
{
	delay_us(uldata*10);
	return;
//	unsigned int i;
//	unsigned char j;
//	for(i=uldata*100;i>0;i--)
//		for(j=5;j>0;j--);   //j=110
}
//nop_ÑÓ³Ù
void delay_nop(unsigned long uldata)
{
	unsigned int i;
	for(i=uldata;i>0;i--);
}
/***********************************************************
* Ãû    ³Æ£ºvoid LD_reset(void)
* ¹¦    ÄÜ£ºLDĞ¾Æ¬Ó²¼ş³õÊ¼»¯
* Èë¿Ú²ÎÊı£º  
* ³ö¿Ú²ÎÊı£º
* Ëµ    Ã÷£º
* µ÷ÓÃ·½·¨£º 
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
* Ãû    ³Æ£ºvoid LD_WriteReg(uint8 data1,uint8 data2)
* ¹¦    ÄÜ£º Ğ´ld3320¼Ä´æÆ÷
* Èë¿Ú²ÎÊı£º  
* ³ö¿Ú²ÎÊı£º
* Ëµ    Ã÷£º
* µ÷ÓÃ·½·¨£º 
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
* Ãû    ³Æ£ºuint8 LD_ReadReg(uint8 reg_add)
* ¹¦    ÄÜ£º¶Áld3320¼Ä´æÆ÷
* Èë¿Ú²ÎÊı£º  
* ³ö¿Ú²ÎÊı£º
* Ëµ    Ã÷£º
* µ÷ÓÃ·½·¨£º 
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
//²¥·ÅºÍÓïÒôÊ¶±ğÖĞ¶Ï
void ProcessInt0(void)
{
	u8 nAsrResCount=0;

//	EX0=0;	  //¹Ø±ÕÍâ²¿ÖĞ¶Ï0ÖĞ¶Ï
	ucRegVal = LD_Read(0x2B);
	if(nLD_Mode == LD_MODE_ASR_RUN)		//µ±Ç°½øĞĞÓïÒôÊ¶±ğ
	{
		// ÓïÒôÊ¶±ğ²úÉúµÄÖĞ¶Ï
		// £¨ÓĞÉùÒôÊäÈë£¬²»ÂÛÊ¶±ğ³É¹¦»òÊ§°Ü¶¼ÓĞÖĞ¶Ï£©
		LD_Write(0x29,0) ;					//ÖĞ¶ÏÔÊĞí FIFO ÖĞ¶ÏÔÊĞí 0±íÊ¾²»ÔÊĞí 
		LD_Write(0x02,0) ;					// FIFOÖĞ¶ÏÔÊĞí	 FIFO_DATA FIFO_EXTÖĞ¶Ï   ²»ÔÊĞí
		delay(100);
		if((ucRegVal & 0x10) &&		  //2bµÚËÄÎ»Îª1 Ğ¾Æ¬ÄÚ²¿FIFOÖĞ¶Ï·¢Éú MP3²¥·ÅÊ±»á²úÉúÖĞ¶Ï±êÖ¾ÇëÇóÍâ²¿MCUÏòFIFO_DATAÖĞReloadÊı
			LD_Read(0xb2)==0x21 && 	  //¶Áb2µÃµ½0x21±íÊ¾ÏĞ¿ÉÒÔ½øĞĞÏÂÒ»²½ASR¶¯×÷
			LD_Read(0xbf)==0x35)		  //¶Áµ½ÊıÖµÎª0x35£¬¿ÉÒÔÈ·¶¨ÊÇÒ»´ÎÓïÒôÊ¶±ğÁ÷³ÌÕı³£½áÊø
		{
			printf("\r\n½øÈëÅĞ¶Ï");
			nAsrResCount = LD_Read(0xba);	    //ASR£ºÖĞ¶ÏÊ±£¬ÅĞ¶ÏÓïÒôÊ¶±ğÓĞ¼¸¸öÊ¶±ğºòÑ¡
			printf("\r\n¿ÉÄÜ½á¹ûÓĞ %d ¸ö\r\n",nAsrResCount);
			if(nAsrResCount==1||(nAsrResCount>0 && nAsrResCount<=4)) 
			{
				nAsrStatus=LD_ASR_FOUNDOK;
				return;
//				LD_Read(0xc5);
			}

			if(nAsrResCount>0 && nAsrResCount<=4) 	  //1 ¨C 4: N¸öÊ¶±ğºòÑ¡ 0»òÕß´óÓÚ4£ºÃ»ÓĞÊ¶±ğºòÑ¡
			{
				nAsrStatus=LD_ASR_FOUNDOK;		 //±íÊ¾Ò»´ÎÊ¶±ğÁ÷³Ì½áÊøºó£¬ÓĞÒ»¸öÊ¶±ğ½á¹û
			}
			else
		    {
				nAsrStatus=LD_ASR_FOUNDZERO;	   //±íÊ¾Ò»´ÎÊ¶±ğÁ÷³Ì½áÊøºó£¬Ã»ÓĞÊ¶±ğ½á¹û
			}	
		}
		else
		{
			nAsrStatus=LD_ASR_FOUNDZERO;	   //±íÊ¾Ò»´ÎÊ¶±ğÁ÷³Ì½áÊøºó£¬Ã»ÓĞÊ¶±ğ½á¹û
		}

		LD_Write(0x2b, 0);
    LD_Write(0x1C,0);	  //ADC¿ª¹Ø¿ØÖÆ Ğ´00H ADC²»¿ÉÓÃ
		
		return;
	}
//=============================================================================================
	// ÉùÒô²¥·Å²úÉúµÄÖĞ¶Ï£¬ÓĞÈıÖÖ£º
	// A. ÉùÒôÊı¾İÒÑÈ«²¿²¥·ÅÍê¡£
	// B. ÉùÒôÊı¾İÒÑ·¢ËÍÍê±Ï¡£
	// C. ÉùÒôÊı¾İÔİÊ±½«ÒªÓÃÍê£¬ĞèÒª·ÅÈëĞÂµÄÊı¾İ¡£	
		ucHighInt = LD_Read(0x29); 
		ucLowInt=LD_Read(0x02); 
		LD_Write(0x29,0);
		LD_Write(0x02,0);
    if(LD_Read(0xBA)&CAUSE_MP3_SONG_END)
    {
	// A. ÉùÒôÊı¾İÒÑÈ«²¿²¥·ÅÍê¡£

			LD_Write(0x2B,  0);
			LD_Write(0xBA, 0);	
			LD_Write(0xBC,0x0);	
			bMp3Play=0;					// ÉùÒôÊı¾İÈ«²¿²¥·ÅÍêºó£¬ĞŞ¸ÄbMp3PlayµÄ±äÁ¿
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
	// B. ÉùÒôÊı¾İÒÑ·¢ËÍÍê±Ï¡£

		LD_Write(0xBC, 0x01);
		LD_Write(0x29, 0x10);
		f_close(&fsrc);
		nLD_Mode = LD_MODE_ASR_RUN;
		nAsrStatus = LD_ASR_NONE;
		return;	
	}

	// C. ÉùÒôÊı¾İÔİÊ±½«ÒªÓÃÍê£¬ĞèÒª·ÅÈëĞÂµÄÊı¾İ¡£	

	LD_ReloadMp3Data_Again();
			
	LD_Write(0x29,ucHighInt); 
	LD_Write(0x02,ucLowInt) ;
}

/*  ¼ÌĞø¶ÁÈ¡mp3ÎÄ¼şÊı¾İµ½fifo,Ö±µ½fifoÂú
 *	±ßĞ´mp3ÎÄ¼şÊı¾İµ½fifoÊ±,LD3320»á±ä½âÂë²¥·Å
 *	µ±È»Ğ´mp3ÎÄ¼şÊı¾İµ½fifoµÄÊ±¼ä»á¶Ì¹ıÉùÒôµÄÊ±¼ä
 *	µ±ÉùÒô¿ì²¥·ÅÍê±ÏµÄÊ±ºò»á½øÈëProcessInt0º¯Êı
 *	ProcessInt0º¯ÊıÓÖ»áµ÷ÓÃ´Ëº¯Êı,ËùÒÔÉùÒôµÃÒÔÁ¬Ğø
 */
void LD_ReloadMp3Data_Again(void)
{
	u8 val;
	u8 ucStatus;
	UINT br;
	
	ucStatus = LD_Read(0x06);
	//fifoÊÇ·ñÂúÁË
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

		//µÈ´ıMP3²¥·ÅÍê±Ï
		while(!(LD_Read(0xBA)&CAUSE_MP3_SONG_END));

		LD_Write(0x2B,  0);
      	LD_Write(0xBA, 0);	
		LD_Write(0xBC,0x0);	
		bMp3Play=0;					// ÉùÒôÊı¾İÈ«²¿²¥·ÅÍêºó£¬ĞŞ¸ÄbMp3PlayµÄ±äÁ¿
		LD_Write(0x08,1);

    LD_Write(0x08,0);
		LD_Write(0x33, 0);

		f_close(&fsrc);
		nLD_Mode = LD_MODE_ASR_RUN;
		nAsrStatus = LD_ASR_NONE;
	}
		
}

//=========================================================
//Í¨ÓÃ³õÊ¼»¯
void LD_Init_Common(void)	
{
	bMp3Play = 0;

	LD_Read(0x06);  
	LD_Write(0x17, 0x35); //Èí¸´Î»LD3320
	delay(100);
	LD_Read(0x06);  

	LD_Write(0x89, 0x03);  //Ä£ÄâµçÂ·¿ØÖÆ ³õÊ¼»¯Ê±Ğ´03H
	delay(100);
	LD_Write(0xCF, 0x43);  //ÄÚ²¿Ê¡µçÄ£Ê½ÉèÖÃ ³õÊ¼»¯Ê±Ğ´Èë43H 
	delay(100);
	LD_Write(0xCB, 0x02);
	
	/*PLL setting*/
	LD_Write(0x11, LD_PLL_11);   		//Ê±ÖÓÆµÂÊÉèÖÃ1  
	if (nLD_Mode == LD_MODE_MP3)    //MP3²¥·Å
	{
		LD_Write(0x1E, 0x00); 					 	 //ADC×¨ÓÃ¿ØÖÆ£¬Ó¦³õÊ¼»¯Îª00H
		LD_Write(0x19, LD_PLL_MP3_19);  	 //Ê±ÖÓÆµÂÊÉèÖÃ2
		LD_Write(0x1B, LD_PLL_MP3_1B); 	   //Ê±ÖÓÆµÂÊÉèÖÃ3
		LD_Write(0x1D, LD_PLL_MP3_1D);		 //Ê±ÖÓÆµÂÊÉèÖÃ4
	}
	else						   									 //ÓïÒôÊ¶±ğ
	{
		LD_Write(0x1E,0x00);	 					   //ADC×¨ÓÃ¿ØÖÆ£¬Ó¦³õÊ¼»¯Îª00H
		LD_Write(0x19, LD_PLL_ASR_19); 		 //Ê±ÖÓÆµÂÊÉèÖÃ2
		LD_Write(0x1B, LD_PLL_ASR_1B);		 //Ê±ÖÓÆµÂÊÉèÖÃ3	
	    LD_Write(0x1D, LD_PLL_ASR_1D);	 //Ê±ÖÓÆµÂÊÉèÖÃ4
	}
	delay(100);
	
	LD_Write(0xCD, 0x04);    //DSPĞİÃßÉèÖÃ ³õÊ¼»¯Ê±Ğ´Èë04H ÔÊĞíDSPĞİÃß
	LD_Write(0x17, 0x4C); 	 //Ğ´4CH¿ÉÒÔÊ¹DSPĞİÃß£¬±È½ÏÊ¡µç
	delay(100);
	LD_Write(0xB9, 0x00);		 //ASR£ºµ±Ç°Ìí¼ÓÊ¶±ğ¾äµÄ×Ö·û´®³¤¶È£¨Æ´Òô×Ö·û´®£©³õÊ¼»¯Ê±Ğ´Èë00H	 Ã¿Ìí¼ÓÒ»ÌõÊ¶±ğ¾äºóÒªÉè¶¨Ò»´Î
	LD_Write(0xCF, 0x4F); 	 //ÄÚ²¿Ê¡µçÄ£Ê``````````````````````````````½ÉèÖÃ MP3³õÊ¼»¯ºÍASR³õÊ¼»¯Ê±Ğ´Èë 4FH
	LD_Write(0x6F, 0xFF); 	 //¶ÔĞ¾Æ¬½øĞĞ³õÊ¼»¯Ê±ÉèÖÃÎª0xFF
}
//==================================================
//²¥·Å³õÊ¼»¯
void LD_Init_MP3(void)	
{
	nLD_Mode = LD_MODE_MP3;	   //µ±Ç°½øĞĞMP3²¥·Å
	LD_Init_Common();		  		 //Í¨ÓÃ³õÊ¼»¯

	LD_Write(0xBD,0x02);	 		 	//ÄÚ²¿ÔöÒæ¿ØÖÆ ³õÊ¼»¯Ê±Ğ´ÈëFFH
	printf("0x02 %x\r\n",LD_Read(0xBD));
	LD_Write(0x17, 0x48);				//Ğ´48H¿ÉÒÔ¼¤»îDSP
		delay(100);
	printf("0x48 %x\r\n",LD_Read(0x17));
	LD_Write(0x85, 0x52); 	//ÄÚ²¿·´À¡ÉèÖÃ ³õÊ¼»¯Ê±Ğ´Èë52H
	printf("0x52 %x\r\n",LD_Read(0x85));
	LD_Write(0x8F, 0x00);  	//LineOut(ÏßÂ·Êä³ö)Ñ¡Ôñ ³õÊ¼»¯Ê±Ğ´Èë00H
	LD_Write(0x81, 0x00);		//¶ú»ú×óÒôÁ¿ ÉèÖÃÎª00HÎª×î´óÒôÁ¿
	LD_Write(0x83, 0x00);		//¶ú»úÓÒÒôÁ¿ ÉèÖÃÎª00HÎª×î´óÒôÁ¿
	LD_Write(0x8E, 0xff);		//À®°ÈÊä³öÒôÁ¿  ±¾¼Ä´æÆ÷ÉèÖÃÎª00HÎª×î´óÒôÁ¿	´Ë´¦ÉùÒô¹Ø±Õ
	LD_Write(0x8D, 0xff);		//ÄÚ²¿ÔöÒæ¿ØÖÆ ³õÊ¼»¯Ê±Ğ´ÈëFFH
    delay(100);
	LD_Write(0x87, 0xff);	   //Ä£ÄâµçÂ·¿ØÖÆ MP3²¥·Å³õÊ¼»¯Ê±Ğ´ FFH
	LD_Write(0x89, 0xff);    //Ä£ÄâµçÂ·¿ØÖÆ MP3²¥·ÅÊ±Ğ´ FFH
		delay(100);
	LD_Write(0x22, 0x00);    //FIFO_DATAÏÂÏŞµÍ8Î»
	LD_Write(0x23, 0x00);		 //FIFO_DATAÏÂÏŞ¸ß8Î»
	LD_Write(0x20, 0xef);    //FIFO_DATAÉÏÏŞµÍ8Î»
	LD_Write(0x21, 0x07);		 //FIFO_DATAÉÏÏŞ¸ß8Î»
	LD_Write(0x24, 0x77);        
	LD_Write(0x25, 0x03);	
	LD_Write(0x26, 0xbb);    
	LD_Write(0x27, 0x01); 	
}
//======================================================
//ÒôÁ¿µ÷Õû
void LD_AdjustMIX2SPVolume(u8 val)	  
{
	ucSPVol = val;
	val = ((15-val)&0x0f) << 2;	
	LD_Write(0x8E, val | 0xc3); 
	LD_Write(0x87, 0x78); 
}
//======================================================
//ÌîÂúfifo´Ó¶ø´¥·¢fifoÖĞ¶Ï,Îª²¥·Åmp3×ö×¼±¸
void fill_the_fifo(void)
{
  u8 ucStatus;
	int i = 0;
	ucStatus = LD_Read(0x06);
	//fifoÊÇ·ñÂúÁË
	while ( !(ucStatus&MASK_FIFO_STATUS_AFULL))
	{

		LD_Write(0x01,0xff);
		i++;
		ucStatus = LD_Read(0x06);
	}
}
//==================================================
//²¥·Åº¯Êı
void LD_play()	  
{
	nMp3Pos=0;
	bMp3Play=1;

	if (nMp3Pos >=  nMp3Size)	  //ÉùÒôÊı¾İÒÑ·¢ËÍÍê±Ï
		return ; 

	fill_the_fifo();
	LD_Write(0xBA, 0x00);	  //ÖĞ¶Ï¸¨ÖúĞÅÏ¢£¬£¨¶Á»òÉèÎª00
	LD_Write(0x17, 0x48);	  //Ğ´48H¿ÉÒÔ¼¤»îDSP
	LD_Write(0x33, 0x01);	  //MP3²¥·ÅÓÃÉèÖÃ ¿ªÊ¼²¥·ÅÊ±Ğ´Èë01H, ²¥·ÅÍêĞ´Èë00H
	LD_Write(0x29, 0x04);	  //ÖĞ¶ÏÔÊĞí µÚ2Î»£ºFIFO ÖĞ¶ÏÔÊĞí µÚ4Î»£ºÍ¬²½ÖĞ¶ÏÔÊĞí 1ÔÊĞí£»0²»ÔÊĞí
	
	LD_Write(0x02, 0x01); 	  //FIFOÖĞ¶ÏÔÊĞí µÚ0Î»£ºÔÊĞíFIFO_DATAÖĞ¶Ï£» µÚ2Î»£ºÔÊĞíFIFO_EXTÖĞ¶Ï£»
	LD_Write(0x85, 0x5A);	  //ÄÚ²¿·´À¡ÉèÖÃ ³õÊ¼»¯Ê±Ğ´Èë52H ²¥·ÅMP3Ê±Ğ´Èë5AH (¸Ä±äÄÚ²¿ÔöÒæ)

	//EX0=1;		//¿ªÍâ²¿ÖĞ¶Ï0ÖĞ¶Ï
	EXTI_ClearITPendingBit(EXTI_Line0);
}

//=============================================================
//È¡ASRÊ¶±ğ½á¹û
u8 LD_GetResult(void)
{
	u8 res;
	res = LD_Read(0xc5);
	return res;
}
//=============================================================
// Return 1: success.
//	Ìí¼ÓÊ¶±ğ¹Ø¼ü´ÊÓï£¬¿ª·¢Õß¿ÉÒÔÑ§Ï°"ÓïÒôÊ¶±ğĞ¾Æ¬LD3320¸ß½×ÃØ¼®.pdf"ÖĞ¹ØÓÚÀ¬»ø´ÊÓïÎüÊÕ´íÎóµÄÓÃ·¨
u8 LD_AsrAddFixed(void)	  //Ìí¼Ó¹Ø¼ü´ÊÓïµ½LD3320Ğ¾Æ¬ÖĞ
{
	u8 k, flag;
	flag = 1;
	for (k=0; k<ITEM_COUNT; k++)
	{	
		if(LD_Check_ASRBusyFlag_b2() == 0)	  //Ğ¾Æ¬ÄÚ²¿³ö´í
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
//ÓïÒôÊ¶±ğ³õÊ¼»¯
void LD_Init_ASR(void)	   //ÓïÒôÊ¶±ğ³õÊ¼»¯
{
	nLD_Mode=LD_MODE_ASR_RUN;	  //µ±Ç°½øĞĞÓïÒôÊ¶±ğ
	LD_Init_Common();	   //Í¨ÓÃ³õÊ¼»¯

	LD_Write(0xBD, 0x00);	 //
	LD_Write(0x17, 0x48);	 //Ğ´48H¿ÉÒÔ¼¤»îDSP	
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
//ÓïÒôÊ¶±ğ³õÊ¼»¯
void LD_AsrStart(void)
{
	LD_Init_ASR();
}
//==========================================================
//¿ªÊ¼Ê¶±ğ
//Return 1: success.
u8 LD_AsrRun(void)		
{
	LD_Write(0x35, MIC_VOL);	   //Âó¿Ë·çÒôÁ¿
//	LD_Write(0xB3, 0x0D);	// ÓÃ»§ÔÄ¶Á ¿ª·¢ÊÖ²á Àí½âB3¼Ä´æÆ÷µÄµ÷Õû¶ÔÓÚÁéÃô¶ÈºÍÊ¶±ğ¾àÀëµÄÓ°Ïì

	LD_Write(0x1C, 0x09);	//ADC¿ª¹Ø¿ØÖÆ Ğ´09H Reserve±£ÁôÃüÁî×Ö
	LD_Write(0xBD, 0x20);	//³õÊ¼»¯¿ØÖÆ¼Ä´æÆ÷ Ğ´Èë20H£»Reserve±£ÁôÃüÁî×Ö
	LD_Write(0x08, 0x01);	//Çå³ıFIFOÄÚÈİµÚ0Î»£ºĞ´Èë1¡úÇå³ıFIFO_DATA µÚ2Î»£ºĞ´Èë1¡úÇå³ıFIFO_EXT
	delay(100);
	LD_Write(0x08, 0x00);	//Çå³ıFIFOÄÚÈİµÚ0Î»£¨Çå³ıÖ¸¶¨FIFOºóÔÙĞ´ÈëÒ»´Î00H£© 
	delay(100);

	printf("the BA is %x",LD_Read(0xBA));  //ÖĞ¶Ï¸¨Öú±êÖ¾Î»(¶Á»òÉèÎª00)

	if(LD_Check_ASRBusyFlag_b2() == 0)	   //Ğ¾Æ¬ÄÚ²¿³ö´í
	{
		printf("\r\nĞ¾Æ¬ÄÚ²¿³ö´í\r\n");
		return 0;
	}

	LD_Write(0xB2, 0xff);	  //ASR£ºDSPÃ¦ÏĞ×´Ì¬ 0x21 ±íÊ¾ÏĞ£¬²éÑ¯µ½ÎªÏĞ×´Ì¬¿ÉÒÔ½øĞĞÏÂÒ»²½ ??? ÎªÊ²Ã´²»ÊÇread??
	LD_Write(0x37, 0x06);	  //Ê¶±ğ¿ØÖÆÃüÁîÏÂ·¢¼Ä´æÆ÷ Ğ´06H Í¨ÖªDSP¿ªÊ¼Ê¶±ğÓïÒô ÏÂ·¢Ç°£¬ĞèÒª¼ì²éB2¼Ä´æÆ÷
	delay(100);
	printf("\r\nAddress(0xBF) value is %x\r\n",LD_Read(0xbf));
	LD_TEST();
	LD_Write(0x1C, 0x0b);	  // ADC¿ª¹Ø¿ØÖÆ  Ğ´0BH Âó¿Ë·çÊäÈëADCÍ¨µÀ¿ÉÓÃ  
	LD_Write(0x29, 0x10);	  //ÖĞ¶ÏÔÊĞí µÚ2Î»£ºFIFO ÖĞ¶ÏÔÊĞí µÚ4Î»£ºÍ¬²½ÖĞ¶ÏÔÊĞí 1ÔÊĞí£»0²»ÔÊĞí
	
	LD_Write(0xBD, 0x00);	  //³õÊ¼»¯¿ØÖÆ¼Ä´æÆ÷ Ğ´Èë00 È»ºóÆô¶¯ ÎªASRÄ£¿é 
	//EX0=1;		  //¿ªÍâ²¿ÖĞ¶Ï0
	EXTI_ClearITPendingBit(EXTI_Line1);   //2020.1.5 ×¢ÊÍ
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
//¼ì²âĞ¾Æ¬ÄÚ²¿ÓĞÎŞ³ö´í
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
//Ë÷Òı
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

	LD_Write(0xc1, k );	   //ASR£ºÊ¶±ğ×ÖIndex£¨00H¡ªFFH£©
	LD_Write(0xc3, 0 );	   //ASR£ºÊ¶±ğ×ÖÌí¼ÓÊ±Ğ´Èë00
	LD_Write(0x08, 0x04);	 //Çå³ıFIFOÄÚÈİµÚ0Î»£ºĞ´Èë1¡úÇå³ıFIFO_DATA µÚ2Î»£ºĞ´Èë1¡úÇå³ıFIFO_EXT
	delay(100);
	LD_Write(0x08, 0x00);	 //Çå³ıFIFOÄÚÈİµÚ0Î»£¨Çå³ıÖ¸¶¨FIFOºóÔÙĞ´ÈëÒ»´Î00H£©
	delay(100);
	for (nAsrAddLength=0; nAsrAddLength<50; nAsrAddLength++)
	{
		if (pRecogString[nAsrAddLength] == 0)
			break;
		LD_Write(0x5, pRecogString[nAsrAddLength]);	  //Ğ´FIFO_EXTÊı¾İ¿Ú
	}
	
	LD_Write(0xb9, nAsrAddLength);	  //µ±Ç°Ìí¼ÓÊ¶±ğ¾äµÄ×Ö·û´®³¤¶È³õÊ¼»¯Ê±Ğ´Èë00H Ã¿Ìí¼ÓÒ»ÌõÊ¶±ğ¾äºóÒªÉè¶¨Ò»´Î
	LD_Write(0xb2, 0xff);	  //DSPÃ¦ÏĞ×´Ì¬ 0x21 ±íÊ¾ÏĞ ¿ÉÒÔ½øĞĞÏÂÒ»²½	   ??
	LD_Write(0x37, 0x04);	  //ÓïÒôÊ¶±ğ¿ØÖÆÃüÁîÏÂ·¢¼Ä´æÆ÷ Ğ´04H£ºÍ¨ÖªDSPÒªÌí¼ÓÒ»ÏîÊ¶±ğ¾ä
}
//==================================================
//Æô¶¯Ò»´ÎÊ¶±ğÁ÷³Ì
u8 RunASR(void)
{
	u8 i=0;
	u8 j=0;
	u8 asrflag=0;
	for (i=0; i<5; i++)	//	·ÀÖ¹ÓÉÓÚÓ²¼şÔ­Òòµ¼ÖÂLD3320Ğ¾Æ¬¹¤×÷²»Õı³££¬ËùÒÔÒ»¹²³¢ÊÔ5´ÎÆô¶¯ASRÊ¶±ğÁ÷³Ì
	{
		LD_AsrStart();
		delay(100);
		if (LD_AsrAddFixed()==0)		//Ìí¼ÓÊ¶±ğ¿ÚÁî
		{
			printf("LD3320Ğ¾Æ¬ÄÚ²¿³öÏÖ²»Õı³££¬Á¢¼´ÖØÆôLD3320Ğ¾Æ¬\%d\r\n",00);
			LD_Reset();			//	LD3320Ğ¾Æ¬ÄÚ²¿³öÏÖ²»Õı³££¬Á¢¼´ÖØÆôLD3320Ğ¾Æ¬
			delay(100);			//	²¢´Ó³õÊ¼»¯¿ªÊ¼ÖØĞÂASRÊ¶±ğÁ÷³Ì
			continue;
		}
		printf("LD_AsrAddFixed is %x\r\n",LD_Read(0xbf));
		LD_TEST();
		delay(100);
		j= LD_AsrRun();
		if (j == 0)
		{
			LD_Reset();			//	LD3320Ğ¾Æ¬ÄÚ²¿³öÏÖ²»Õı³££¬Á¢¼´ÖØÆôLD3320Ğ¾Æ¬
			delay(100);			//	²¢´Ó³õÊ¼»¯¿ªÊ¼ÖØĞÂASRÊ¶±ğÁ÷³Ì
			continue;
		}
		asrflag=1;
		break;						//	ASRÁ÷³ÌÆô¶¯³É¹¦£¬ÍË³öµ±Ç°forÑ­»·¡£¿ªÊ¼µÈ´ıLD3320ËÍ³öµÄÖĞ¶ÏĞÅºÅ
	}	
	return asrflag;
}
//================================================================\
//LD3320³õÊ¼»¯
void LD3320_Init(void)
{
	LD3320_GPIO_Cfg();	
	LD3320_EXTI_Cfg();
	LD_Reset();
}

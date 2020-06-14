#ifndef __LD3320_H
#define __LD3320_H

#include "stm32f4xx.h" 

//	以下三个状态定义用来记录程序是在运行ASR识别还是在运行MP3播放
#define LD_MODE_IDLE		0x00
#define LD_MODE_ASR_RUN		0x08
#define LD_MODE_MP3		 	0x40

//		sbit SCS=P2^6;    //片选
//		sbit SDCK=P0^2;   //SPI 时钟
//		sbit SDI=P0^0;    //SPI 数据输入
//		sbit SDO=P0^1;    //SPI 数据输出
//		sbit SPIS=P3^6;   //SPI 模式设置
//SPI串行模式引脚
#define SCS_H GPIO_SetBits(GPIOC, GPIO_Pin_2)
#define SCS_L GPIO_ResetBits(GPIOC, GPIO_Pin_2)
#define SDCK_H GPIO_SetBits(GPIOA, GPIO_Pin_2)
#define SDCK_L GPIO_ResetBits(GPIOA, GPIO_Pin_2)
#define SDI_H GPIO_SetBits(GPIOA, GPIO_Pin_0)
#define SDI_L GPIO_ResetBits(GPIOA, GPIO_Pin_0)
//#define SDO_H GPIO_SetBits(GPIOA, GPIO_Pin_1)
//#define SDO_L GPIO_ResetBits(GPIOA, GPIO_Pin_1)
#define SPIS_H GPIO_SetBits(GPIOC, GPIO_Pin_3)
#define SPIS_L GPIO_ResetBits(GPIOC, GPIO_Pin_3)

#define DELAY_NOP delay_nop(50);

#define LD_RST_H() GPIO_SetBits(GPIOC, GPIO_Pin_1)
#define LD_RST_L() GPIO_ResetBits(GPIOC, GPIO_Pin_1)

#define LD_CS_H()	GPIO_SetBits(GPIOC, GPIO_Pin_2)
#define LD_CS_L()	GPIO_ResetBits(GPIOC, GPIO_Pin_2)

#define LD_SPIS_H()  GPIO_SetBits(GPIOC, GPIO_Pin_3)
#define LD_SPIS_L()  GPIO_ResetBits(GPIOC, GPIO_Pin_3)

//以下五个状态定义用来记录程序是在运行ASR识别过程中的哪个状态
#define LD_ASR_NONE				0x00	//	表示没有在作ASR识别
#define LD_ASR_RUNING			0x01	//	表示LD3320正在作ASR识别中
#define LD_ASR_FOUNDOK			0x10	//	表示一次识别流程结束后，有一个识别结果
#define LD_ASR_FOUNDZERO 		0x11	//	表示一次识别流程结束后，没有识别结果
#define LD_ASR_ERROR	 		0x31	//	表示一次识别流程中LD3320芯片内部出现不正确的状态

#define CLK_IN   		 16 //* user need modify this value according to clock in */
#define LD_PLL_11			(u8)((CLK_IN/2.0)-1)
#define LD_PLL_MP3_19		0x0f
#define LD_PLL_MP3_1B		0x18
#define LD_PLL_MP3_1D   	(u8)(((90.0*((LD_PLL_11)+1))/(CLK_IN))-1)

#define LD_PLL_ASR_19 		(u8)(CLK_IN*32.0/(LD_PLL_11+1) - 0.51)
#define LD_PLL_ASR_1B 		0x48
#define LD_PLL_ASR_1D 		0x1f

// LD chip fixed values.
#define        RESUM_OF_MUSIC               0x01
#define        CAUSE_MP3_SONG_END           0x20

#define        MASK_INT_SYNC				0x10
#define        MASK_INT_FIFO				0x04
#define    	   MASK_AFIFO_INT				0x01
#define        MASK_FIFO_STATUS_AFULL		0x08

extern u8 bMp3Play;


void LD3320_Init(void);
void ProcessInt0(void);	  //播放 语音识别中断
void LD_Reset(void);	  //芯片复位
int LD_TEST(void);

//static unsigned char spi_send_byte(unsigned char TxData);

unsigned char LD_Read(unsigned char a1);
void LD_Write(unsigned char data1,unsigned char data2);

void LD_ReloadMp3Data_Again(void);

void SD_INIT(void);
 

void delay_nop(unsigned long uldata);
void delay(unsigned long uldata);
//================
u8 LD_GetResult(void);//取ASR识别结果
void LD_AsrStart(void);//语音识别初始化
u8 LD_AsrAddFixed(void);//添加关键词语到LD3320芯片中
u8 LD_AsrRun(void);//开始识别
u8 LD_Check_ASRBusyFlag_b2(void);//芯片有无出错
void LD_AsrAddFixed_ByIndex(u8 nIndex);//索引
void LD_AsrAddFixed_ByString(char * pRecogString, u8 k);//↑
u8 RunASR(void);//启动一次识别流程
char PlayDemoSound_mp3(char *path,u8 Volume);

void LD_Init_Common(void);

#endif

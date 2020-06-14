#ifndef __LD3320_H
#define __LD3320_H

#include "stm32f4xx.h" 

//	��������״̬����������¼������������ASRʶ����������MP3����
#define LD_MODE_IDLE		0x00
#define LD_MODE_ASR_RUN		0x08
#define LD_MODE_MP3		 	0x40

//		sbit SCS=P2^6;    //Ƭѡ
//		sbit SDCK=P0^2;   //SPI ʱ��
//		sbit SDI=P0^0;    //SPI ��������
//		sbit SDO=P0^1;    //SPI �������
//		sbit SPIS=P3^6;   //SPI ģʽ����
//SPI����ģʽ����
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

//�������״̬����������¼������������ASRʶ������е��ĸ�״̬
#define LD_ASR_NONE				0x00	//	��ʾû������ASRʶ��
#define LD_ASR_RUNING			0x01	//	��ʾLD3320������ASRʶ����
#define LD_ASR_FOUNDOK			0x10	//	��ʾһ��ʶ�����̽�������һ��ʶ����
#define LD_ASR_FOUNDZERO 		0x11	//	��ʾһ��ʶ�����̽�����û��ʶ����
#define LD_ASR_ERROR	 		0x31	//	��ʾһ��ʶ��������LD3320оƬ�ڲ����ֲ���ȷ��״̬

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
void ProcessInt0(void);	  //���� ����ʶ���ж�
void LD_Reset(void);	  //оƬ��λ
int LD_TEST(void);

//static unsigned char spi_send_byte(unsigned char TxData);

unsigned char LD_Read(unsigned char a1);
void LD_Write(unsigned char data1,unsigned char data2);

void LD_ReloadMp3Data_Again(void);

void SD_INIT(void);
 

void delay_nop(unsigned long uldata);
void delay(unsigned long uldata);
//================
u8 LD_GetResult(void);//ȡASRʶ����
void LD_AsrStart(void);//����ʶ���ʼ��
u8 LD_AsrAddFixed(void);//��ӹؼ����ﵽLD3320оƬ��
u8 LD_AsrRun(void);//��ʼʶ��
u8 LD_Check_ASRBusyFlag_b2(void);//оƬ���޳���
void LD_AsrAddFixed_ByIndex(u8 nIndex);//����
void LD_AsrAddFixed_ByString(char * pRecogString, u8 k);//��
u8 RunASR(void);//����һ��ʶ������
char PlayDemoSound_mp3(char *path,u8 Volume);

void LD_Init_Common(void);

#endif

#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2011/6/14
//�汾��V1.4
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��

#include "stdio.h"      //������Ҫ��ͷ�ļ�
#include "stdarg.h"		//������Ҫ��ͷ�ļ� 
#include "string.h"     //������Ҫ��ͷ�ļ�

#define USART2_RX_ENABLE     1      //�Ƿ������չ���  1������  0���ر�
#define USART2_TXBUFF_SIZE   1024   //���崮��2 ���ͻ�������С 1024�ֽ�

                         //���ʹ�ܽ��չ���
#define USART2_RXBUFF_SIZE   1024              //���崮��2 ���ջ�������С 1024�ֽ�
extern char Usart2_RxCompleted ;               //�ⲿ�����������ļ����Ե��øñ���
extern unsigned int Usart2_RxCounter;          //�ⲿ�����������ļ����Ե��øñ���
extern char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //�ⲿ�����������ļ����Ե��øñ���

void Usart2_Init(u32 bound);   
void u2_printf(char*,...) ;          
void u2_TxData(unsigned char *data);

void USART2_IRQHandler(void);
void uart_init(u32 bound);
void TIM6_IRQHandler(void);
void TIM5_IRQHandler(void);
void TIM4_IRQHandler(void);


#endif



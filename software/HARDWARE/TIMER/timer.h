#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/16
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM2_Int_Init(u16 arr,u16 psc);
void TIM6_Int_Init(u16 arr,u16 psc);
void TIM5_ENABLE_2S(void);
void TIM5_ENABLE_30S(void);
void TIM4_ENABLE_30S(void);
//void TIM2_IRQHandler(void);
#endif

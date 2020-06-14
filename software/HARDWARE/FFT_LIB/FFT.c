#include "sys.h"
#include "FFT.h"

/* USER CODE BEGIN Includes */
#include "stm32_dsp.h"
#include "table_fft.h"
#include "math.h"
#include "oled.h"
#include "config.h"

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/


//�����ʼ���
//�ֱ��ʣ�Fs/NPT
//#define Fs	10000
#define Fs	9984
//ȡ9984�ܳ��������ķֱ��� 9984/256 = 39Hz

long lBufInArray[NPT];
long lBufOutArray[NPT/2];
long lBufMagArray[NPT/2];
uint32_t adc_buf[NPT]={0};
uint8_t prt = 10;	//������ʾ�ı���
#define SHOW_NUM 4				//��ʾ�����ĸ���
uint8_t display_num = 1;	//������ʾ��ʽ��
uint8_t auto_display_flag = 0;	//�Զ��л���ʾ��־ 1���Զ��л� 0:�ֶ�
uint8_t fall_pot[128];	//��¼����������


/************FFT���*****************/
//������ ����һ���ź�
float FRE=100;
void Creat_Single(void)
{
	u16 i = 0;
	float fx=0.0;
	
	for(i=0; i<NPT; i++)
	{
		fx = 2048+2048*sin(PI2 * i / FRE);
		lBufInArray[i] = ((signed short)fx) << 16;		
	}
}

//��ȡFFT���ֱ������
void GetPowerMag(void)
{
    signed short lX,lY;
    float X,Y,Mag;
    unsigned short i;
    for(i=0; i<NPT/2; i++)
    {
        lX  = (lBufOutArray[i] << 16) >> 16;
        lY  = (lBufOutArray[i] >> 16);
			
				//����32768�ٳ�65536��Ϊ�˷��ϸ������������
        X = NPT * ((float)lX) / 32768;
        Y = NPT * ((float)lY) / 32768;
        Mag = sqrt(X * X + Y * Y)*1.0/ NPT;
        if(i == 0)	
            lBufMagArray[i] = (unsigned long)(Mag * 32768);
        else
            lBufMagArray[i] = (unsigned long)(Mag * 65536);
    }
}
/*��״��ʾ*/
void display1(void)
{
	uint16_t i = 0;
	uint8_t x = 0;
	uint8_t y = 0;
	
	/*******************��ʾ*******************/
	GUI_ClearSCR();
	for(i = 0; i < 32; i++)	//�����ȡ32��Ƶ�ʳ�����ʾ
	{
		x = (i<<2);	//i*4
		y = 63-(lBufMagArray[x+1]/prt)-2;	//��1��Ϊ�˶�����һ��ֱ������
		if(y>63) y = 63;
		
		GUI_LineWith(x,y,x,63,3,1);
		
		//������ĵ�
		if(fall_pot[i]>y) fall_pot[i] = y;
		else
		{
				if(fall_pot[i]>63) fall_pot[i]=63;
				GUI_LineWith(x,fall_pot[i],x,fall_pot[i]+3,3,1);
				fall_pot[i] += 2 ;
		}
	}
	GUI_Exec();
}
/*����״��ʾ*/
void display2(void)
{
	uint16_t i = 0;
	uint8_t y = 0;
	
	/*******************��ʾ*******************/
	GUI_ClearSCR();
	for(i = 1; i < 128; i++)	
	{
		y = 63-(lBufMagArray[i]/prt)-2;
		if(y>63) y = 63;
		
		GUI_RLine(i,y,63,1);		
		//������ĵ�
		if(fall_pot[i]>y) fall_pot[i] = y;
		else
		{
				if(fall_pot[i]>63) fall_pot[i]=63;
				GUI_RLine(i,fall_pot[i],fall_pot[i]+1,1);
				fall_pot[i] += 2 ;
		}
	}
	GUI_Exec();
}
/*��״��ʾ �м�Գ�*/
void display3(void)
{
	uint16_t i = 0;
	uint8_t y = 0;
	
	/*******************��ʾ*******************/
	GUI_ClearSCR();
	for(i = 0; i < 127; i++)	
	{
		y = 31-(lBufMagArray[i+1]/prt)-2;	//��1��Ϊ�˶�����һ��ֱ������
		if(y>31) y = 31;
		
		GUI_RLine(i,32,y,1);
		GUI_RLine(i,32,63-y,1);
		
		//������ĵ�
		if(fall_pot[i]>y) fall_pot[i] = y;
		else
		{
				if(fall_pot[i]>30) fall_pot[i]=30;
				GUI_RLine(i,fall_pot[i],fall_pot[i]+1,1);
				GUI_RLine(i,63-fall_pot[i],63-(fall_pot[i]+1),1);
				fall_pot[i] += 2 ;
		}
	}
	GUI_Exec();
}
/*����״��ʾ �м�Գ�*/
void display4(void)
{
	uint16_t i = 0;
	uint8_t x = 0;
	uint8_t y = 0;
	
	/*******************��ʾ*******************/
	GUI_ClearSCR();
	for(i = 0; i < 32; i++)	//�����ȡ32��Ƶ�ʳ�����ʾ
	{
		x = (i<<2);	//i*4
		y = 31-(lBufMagArray[x+1]/prt)-2;	//��1��Ϊ�˶�����һ��ֱ������
		if(y>31) y = 31;
		
		GUI_LineWith(x,y,x,32,3,1);
		GUI_LineWith(x,63-y,x,32,3,1);
		
		//������ĵ�
		if(fall_pot[i]>y) fall_pot[i] = y;
		else
		{
				if(fall_pot[i]>31) fall_pot[i]=31;
				GUI_LineWith(x,fall_pot[i],x,fall_pot[i]+3,3,1);
				GUI_LineWith(x,63 - fall_pot[i],x,63 - fall_pot[i]-3,3,1);
				fall_pot[i] += 2 ;
		}
	}
	GUI_Exec();
}


void FFT_Action(void)
{
	uint16_t i = 0;
	Creat_Single();
	cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
	GetPowerMag();
	display1();
}

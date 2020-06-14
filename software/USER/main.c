#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "ld3320.h"
#include "i2s.h"
#include "timer.h"
#include "FFT.h"
#include "wifi.h"
#include "config.h"
#include "mqtt.h"
#include "device.h"

u8 nAsrStatus=0;
SYS_CHECK sys_check={0};
unsigned char nAsrRes;
extern uint8_t fall_pot[128];	//��¼����������

void SYS_INIT(){
		int i;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
		delay_init(168);     //��ʼ����ʱ����
		LED_Init();					//��ʼ��LED
		OLED_Init();				//��ʼ��OLED
		LD3320_Init();
		I2S_GPIO_Init();
		//��ʼ������� ������ĵ� ��ʼ��Ϊ��ײ���ʾ
		for(i=0;i<128;i++)
			fall_pot[i] = 63;
		nAsrStatus = LD_ASR_NONE;														//��ʼ״̬��û������ASR
}
/*-------------------------------------------------*/
/*���������ɼ���ʪ�ȣ���������������               */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
int value=0,value_1=0;
void TempHumi_State(void)
{
	double tempdata,humidata;	
	char temp[256];  
	printf("�¶ȣ�%.2f  ʪ�ȣ�%.2f\r\n",10.0,11.1);
	sprintf(temp,"{\"method\":\"thing.service.property.set\",\"id\":\"266351561\",\"params\":{\"CurrentTemperature\":%d,\"CurrentHumidity\":%d},\"version\":\"1.0.0\"}",value,value_1);  //�����ظ�ʪ���¶�����
	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������	
}
int main(void)
{
	int i;
	WINDOWS demow;
	SYS_INIT();
	uart_init(115200);	//��ʼ�����ڲ�����Ϊ115200
	Usart2_Init(115200);		//ESP8266��ʼ��
	TIM3_Int_Init(50-1,8400-1);	//��ʱ��ʱ��84M����Ƶϵ��8400������84M/8400=10Khz�ļ���Ƶ�ʣ�����5000��Ϊ500ms   
	TIM2_Int_Init(1000-1,8400-1);		//FFT ��ʱ����
	TIM6_Int_Init(300,7200);
	WiFi_ResetIO_Init();
	MQTT_Buff_Init();               //��ʼ������,����,�������ݵ� ������ �Լ���״̬����
	AliIoT_Parameter_Init();	    //��ʼ�����Ӱ�����IoTƽ̨MQTT�������Ĳ���		
	while(1)
	{
		//�������Է���
				switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
				break;
			case LD_ASR_ERROR:	
				break;
			case LD_ASR_NONE:
			{
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)															//����һ��ASRʶ�����̣�ASR��ʼ����ASR��ӹؼ��������ASR����
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}
			case LD_ASR_FOUNDOK:
			{
				nAsrRes = LD_GetResult();											//һ��ASRʶ�����̽�����ȥȡASRʶ����
				printf("\r\n......The result is %d......\r\n",nAsrRes);
				switch(nAsrRes){
					case 1: LED_RGB_R=1;LED_RGB_B=0;LED_RGB_G=0;break;
					case 2: LED_RGB_R=0;LED_RGB_B=1;LED_RGB_G=0;break;
					case 3: LED_RGB_R=0;LED_RGB_B=0;LED_RGB_G=1;break;
					case 4: printf("\r\n�ѿ���\r\n");break;
					default: printf("\r\nnone of then!\r\n");
				}
				nAsrStatus = LD_ASR_NONE;
				break;
			}
			case LD_ASR_FOUNDZERO:
			default:
			{
				nAsrStatus = LD_ASR_NONE;
				break;
			}
		}
		//�Ʒ���
		/*--------------------------------------------------------------------*/
		/*   Connect_flag=1ͬ����������������,���ǿ��Է������ݺͽ���������    */
		/*--------------------------------------------------------------------*/
		if(Connect_flag==1){     
			/*-------------------------------------------------------------*/
			/*                     �����ͻ���������                      */
			/*-------------------------------------------------------------*/
				if(MQTT_TxDataOutPtr != MQTT_TxDataInPtr){                //if�����Ļ���˵�����ͻ�������������
				//3������ɽ���if
				//��1�֣�0x10 ���ӱ���
				//��2�֣�0x82 ���ı��ģ���ConnectPack_flag��λ����ʾ���ӱ��ĳɹ�
				//��3�֣�SubcribePack_flag��λ��˵�����ӺͶ��ľ��ɹ����������Ŀɷ�
				if((MQTT_TxDataOutPtr[2]==0x10)||((MQTT_TxDataOutPtr[2]==0x82)&&(ConnectPack_flag==1))||(SubcribePack_flag==1)){    
					printf("��������:0x%x\r\n",MQTT_TxDataOutPtr[2]);  //������ʾ��Ϣ
					MQTT_TxData(MQTT_TxDataOutPtr);                       //��������
					MQTT_TxDataOutPtr += BUFF_UNIT;                       //ָ������
					if(MQTT_TxDataOutPtr==MQTT_TxDataEndPtr)              //���ָ�뵽������β����
						MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];            //ָ���λ����������ͷ
				} 				
			}//�����ͻ��������ݵ�else if��֧��β
			/*-------------------------------------------------------------*/
			/*                     ������ջ���������                      */
			/*-------------------------------------------------------------*/
			if(MQTT_RxDataOutPtr != MQTT_RxDataInPtr){  //if�����Ļ���˵�����ջ�������������														
				printf("���յ�����:");
				/*-----------------------------------------------------*/
				/*                    ����CONNACK����                  */
				/*-----------------------------------------------------*/				
				//if�жϣ������һ���ֽ���0x20����ʾ�յ�����CONNACK����
				//��������Ҫ�жϵ�4���ֽڣ�����CONNECT�����Ƿ�ɹ�
				if(MQTT_RxDataOutPtr[2]==0x20){             			
				    switch(MQTT_RxDataOutPtr[5]){					
						case 0x00 : printf("CONNECT���ĳɹ�\r\n");                            //���������Ϣ	
								    ConnectPack_flag = 1;                                        //CONNECT���ĳɹ������ı��Ŀɷ�
									break;                                                       //������֧case 0x00                                              
						case 0x01 : printf("�����Ѿܾ�����֧�ֵ�Э��汾��׼������\r\n");     //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������
									break;                                                       //������֧case 0x01   
						case 0x02 : printf("�����Ѿܾ������ϸ�Ŀͻ��˱�ʶ����׼������\r\n"); //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������
									break;                                                       //������֧case 0x02 
						case 0x03 : printf("�����Ѿܾ�������˲����ã�׼������\r\n");         //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������
									break;                                                       //������֧case 0x03
						case 0x04 : printf("�����Ѿܾ�����Ч���û��������룬׼������\r\n");   //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������						
									break;                                                       //������֧case 0x04
						case 0x05 : printf("�����Ѿܾ���δ��Ȩ��׼������\r\n");               //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������						
									break;                                                       //������֧case 0x05 		
						default   : printf("�����Ѿܾ���δ֪״̬��׼������\r\n");             //���������Ϣ 
									Connect_flag = 0;                                            //Connect_flag���㣬��������					
									break;                                                       //������֧case default 								
					}				
				}			
				//if�жϣ���һ���ֽ���0x90����ʾ�յ�����SUBACK����
				//��������Ҫ�ж϶��Ļظ��������ǲ��ǳɹ�
				else if(MQTT_RxDataOutPtr[2]==0x90){ 
						switch(MQTT_RxDataOutPtr[6]){					
						case 0x00 :
						case 0x01 : printf("���ĳɹ�\r\n");            //���������Ϣ
							        SubcribePack_flag = 1;                //SubcribePack_flag��1����ʾ���ı��ĳɹ����������Ŀɷ���
									Ping_flag = 0;                        //Ping_flag����
   								    TIM5_ENABLE_30S();                    //����30s��PING��ʱ��
									TIM4_ENABLE_30S();                    //����30s���ϴ����ݵĶ�ʱ��
						            TempHumi_State();                     //�ȷ�һ������
									break;                                //������֧                                             
						default   : printf("����ʧ�ܣ�׼������\r\n");  //���������Ϣ 
									Connect_flag = 0;                     //Connect_flag���㣬��������
									break;                                //������֧ 								
					}					
				}
				//if�жϣ���һ���ֽ���0xD0����ʾ�յ�����PINGRESP����
				else if(MQTT_RxDataOutPtr[2]==0xD0){ 
					printf("PING���Ļظ�\r\n"); 		  //���������Ϣ 
					if(Ping_flag==1){                     //���Ping_flag=1����ʾ��һ�η���
						 Ping_flag = 0;    				  //Ҫ���Ping_flag��־
					}else if(Ping_flag>1){ 				  //���Ping_flag>1����ʾ�Ƕ�η����ˣ�������2s����Ŀ��ٷ���
						Ping_flag = 0;     				  //Ҫ���Ping_flag��־
						TIM5_ENABLE_30S(); 				  //PING��ʱ���ػ�30s��ʱ��
					}				
				}	
				//if�жϣ������һ���ֽ���0x30����ʾ�յ����Ƿ�������������������
				//����Ҫ��ȡ��������
				else if((MQTT_RxDataOutPtr[2]==0x30)){ 
					printf("�������ȼ�0����\r\n"); 		   //���������Ϣ 
					MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr);  //����ȼ�0��������
				}				
								
				MQTT_RxDataOutPtr += BUFF_UNIT;                     //ָ������
				if(MQTT_RxDataOutPtr==MQTT_RxDataEndPtr)            //���ָ�뵽������β����
					MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];          //ָ���λ����������ͷ                        
			}//������ջ��������ݵ�else if��֧��β
			/*-------------------------------------------------------------*/
			/*                     ���������������                      */
			/*-------------------------------------------------------------*/
			if(MQTT_CMDOutPtr != MQTT_CMDInPtr){                             //if�����Ļ���˵�����������������
				char *position=0;
				printf("����:%s\r\n",&MQTT_CMDOutPtr[2]);                 //���������Ϣ
				if(position = strstr(&MQTT_CMDOutPtr[2],"PowerSwitch")) 
				{if(*(position+13)=='1') {printf("\r\n�õ���������%c\r\n",*(position+3));LED_RGB_R=1;}
				else if(*(position+13)=='0'){printf("\r\n�õ���������%c\r\n",*(position+3));LED_RGB_R=0;}
				}
				if(position = strstr(&MQTT_CMDOutPtr[2],"PowerSwitch_2")) 
				{if(*(position+15)=='1') {printf("\r\n�õ���������%c\r\n",*(position+3));LED_RGB_G=1;}
				else {printf("\r\n�õ���������%c\r\n",*(position+3));LED_RGB_G=0;}
				}
				if(position = strstr(&MQTT_CMDOutPtr[2],"PowerSwitch_3")) 
				{if(*(position+15)=='1') {printf("\r\n�õ���������%c\r\n",*(position+3));LED_RGB_B=1;}
				else {printf("\r\n�õ���������%c\r\n",*(position+3));LED_RGB_B=0;}
				}
				MQTT_CMDOutPtr += BUFF_UNIT;                             	 //ָ������
				if(MQTT_CMDOutPtr==MQTT_CMDEndPtr)           	             //���ָ�뵽������β����
					MQTT_CMDOutPtr = MQTT_CMDBuf[0];          	             //ָ���λ����������ͷ				
			}//��������������ݵ�else if��֧��β	
		}//Connect_flag=1��if��֧�Ľ�β
		/*--------------------------------------------------------------------*/
		/*      Connect_flag=0ͬ�������Ͽ�������,����Ҫ�������ӷ�����         */
		/*--------------------------------------------------------------------*/
		else{ 
			printf("��Ҫ���ӷ�����\r\n");                 //���������Ϣ
			TIM_Cmd(TIM5,DISABLE);                           //�ر�TIM4 
			TIM_Cmd(TIM6,DISABLE);                           //�ر�TIM3  
			WiFi_RxCounter=0;                                //WiFi������������������                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //���WiFi���ջ����� 
			if(WiFi_Connect_IoTServer()==0){   			     //���WiFi�����Ʒ�������������0����ʾ��ȷ������if
				printf("����TCP���ӳɹ�\r\n");            //���������Ϣ
				Connect_flag = 1;                            //Connect_flag��1����ʾ���ӳɹ�	
				WiFi_RxCounter=0;                            //WiFi������������������                        
				memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);      //���WiFi���ջ����� 
				MQTT_Buff_ReInit();                          //���³�ʼ�����ͻ�����                    
			}				
		}
	}
}
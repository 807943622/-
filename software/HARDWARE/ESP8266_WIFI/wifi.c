/*-------------------------------------------------*/
/*            ��γ����STM32ϵ�п�����              */
/*-------------------------------------------------*/
/*                                                 */
/*            ����602Wifi���ܵ�Դ�ļ�              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f4xx.h"   //������Ҫ��ͷ�ļ�
#include "wifi.h"	    //������Ҫ��ͷ�ļ�
#include "delay.h"	    //������Ҫ��ͷ�ļ�
#include "usart.h"	    //������Ҫ��ͷ�ļ�
#include "led.h"        //������Ҫ��ͷ�ļ�
#include "mqtt.h"       //������Ҫ��ͷ�ļ�

char wifi_mode = 0;     //����ģʽ 0��SSID������д�ڳ�����   1��Smartconfig��ʽ��APP����
	
/*-------------------------------------------------*/
/*����������ʼ��WiFi�ĸ�λIO                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void WiFi_ResetIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                      //����һ������IO�˿ڲ����Ľṹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);   //ʹ��PA�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                 //׼������PA4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);            		  //����PA4
//	RESET_IO(1);                                              //��λIO���ߵ�ƽ
	
	GPIO_SetBits(GPIOC, GPIO_Pin_8);
}
/*-------------------------------------------------*/
/*��������WiFi��������ָ��                         */
/*��  ����cmd��ָ��                                */
/*��  ����timeout����ʱʱ�䣨100ms�ı�����         */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_SendCmd(char *cmd, int timeout)
{
	WiFi_RxCounter=0;                           //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //���WiFi���ջ����� 
	WiFi_printf("%s\r\n",cmd);                  //����ָ��
	while(timeout--){                           //�ȴ���ʱʱ�䵽0
		delay_ms(100);                          //��ʱ100ms
		if(strstr(WiFi_RX_BUF,"OK"))            //������յ�OK��ʾָ��ɹ�
			break;       						//��������whileѭ��
		printf("%d ",timeout);               //����������ڵĳ�ʱʱ��
	}
	printf("\r\n");                          //���������Ϣ
	if(timeout<=0)return 1;                     //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�OK������1
	else return 0;		         				//��֮����ʾ��ȷ��˵���յ�OK��ͨ��break��������while
}
/*-------------------------------------------------*/
/*��������WiFi��λ                                 */
/*��  ����timeout����ʱʱ�䣨100ms�ı�����         */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_Reset(int timeout)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_8);  //��λIO���͵�ƽ                           
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //���WiFi���ջ����� 
	delay_ms(100);                                  //��ʱ500ms
	GPIO_SetBits(GPIOC, GPIO_Pin_8);                                    //��λIO���ߵ�ƽ	
	while(timeout--){                               //�ȴ���ʱʱ�䵽0
		delay_ms(100);                              //��ʱ100ms
		if(strstr(WiFi_RX_BUF,"ready"))             //������յ�ready��ʾ��λ�ɹ�
			break;       						    //��������whileѭ��
		printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��
	}
	printf("\r\n");                              //���������Ϣ
	if(timeout<=0)return 1;                         //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�ready������1
	else return 0;		         				    //��֮����ʾ��ȷ��˵���յ�ready��ͨ��break��������while
}
/*-------------------------------------------------*/
/*��������WiFi����·����ָ��                       */
/*��  ����timeout����ʱʱ�䣨1s�ı�����            */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_JoinAP(int timeout)
{		
	WiFi_RxCounter=0;                               //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ����� 
	WiFi_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,PASS); //����ָ��	
	while(timeout--){                               //�ȴ���ʱʱ�䵽0
		delay_ms(1000);                             //��ʱ1s
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP\r\n\r\nOK")) //������յ�WIFI GOT IP��ʾ�ɹ�
			break;       						    //��������whileѭ��
		printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��
	}
	printf("\r\n");                              //���������Ϣ
	if(timeout<=0)return 1;                         //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�WIFI GOT IP������1
	return 0;                                       //��ȷ������0
}
/*-------------------------------------------------*/
/*������������TCP��������������͸��ģʽ            */
/*��  ����timeout�� ��ʱʱ�䣨100ms�ı�����        */
/*����ֵ��0����ȷ  ����������                      */
/*-------------------------------------------------*/
char WiFi_Connect_Server(int timeout)
{	
	WiFi_RxCounter=0;                               //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ�����   
	WiFi_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ServerIP,ServerPort);//�������ӷ�����ָ��
	while(timeout--){                               //�ȴ���ʱ���
		delay_ms(100);                              //��ʱ100ms	
		if(strstr(WiFi_RX_BUF ,"CONNECT"))          //������ܵ�CONNECT��ʾ���ӳɹ�
			break;                                  //����whileѭ��
		if(strstr(WiFi_RX_BUF ,"CLOSED"))           //������ܵ�CLOSED��ʾ������δ����
			return 1;                               //������δ��������1
		if(strstr(WiFi_RX_BUF ,"ALREADY CONNECTED"))//������ܵ�ALREADY CONNECTED�Ѿ���������
			return 2;                               //�Ѿ��������ӷ���2
		printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��  
	}
	printf("\r\n");                        //���������Ϣ
	if(timeout<=0)return 3;                   //��ʱ���󣬷���3
	else                                      //���ӳɹ���׼������͸��
	{
		printf("���ӷ������ɹ���׼������͸��\r\n");  //������ʾ��Ϣ
		WiFi_RxCounter=0;                               //WiFi������������������                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //���WiFi���ջ�����     
		WiFi_printf("AT+CIPSEND\r\n");                  //���ͽ���͸��ָ��
		while(timeout--){                               //�ȴ���ʱ���
			delay_ms(100);                              //��ʱ100ms	
			if(strstr(WiFi_RX_BUF,"\r\nOK\r\n\r\n>"))   //���������ʾ����͸���ɹ�
				break;                          //����whileѭ��
			printf("%d ",timeout);           //����������ڵĳ�ʱʱ��  
		}
		if(timeout<=0)return 4;                 //͸����ʱ���󣬷���4	
	}
	return 0;	                                //�ɹ�����0	
}
/*-------------------------------------------------*/
/*��������WiFi_Smartconfig                         */
/*��  ����timeout����ʱʱ�䣨1s�ı�����            */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_Smartconfig(int timeout)
{
	
	WiFi_RxCounter=0;                           //WiFi������������������                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //���WiFi���ջ�����     
	while(timeout--){                           //�ȴ���ʱʱ�䵽0
		delay_ms(1000);                         //��ʱ1s
		if(strstr(WiFi_RX_BUF,"connected"))     //������ڽ��ܵ�connected��ʾ�ɹ�
			break;                              //����whileѭ��  
		printf("%d ",timeout);               //����������ڵĳ�ʱʱ��  
	}	
	printf("\r\n");                          //���������Ϣ
	if(timeout<=0)return 1;                     //��ʱ���󣬷���1
	return 0;                                   //��ȷ����0
}
/*-------------------------------------------------*/
/*���������ȴ�����·����                           */
/*��  ����timeout����ʱʱ�䣨1s�ı�����            */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_WaitAP(int timeout)
{		
	while(timeout--){                               //�ȴ���ʱʱ�䵽0
		delay_ms(1000);                             //��ʱ1s
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP"))       //������յ�WIFI GOT IP��ʾ�ɹ�
			break;       						    //��������whileѭ��
		printf("%d ",timeout);                   //����������ڵĳ�ʱʱ��
	}
	printf("\r\n");                              //���������Ϣ
	if(timeout<=0)return 1;                         //���timeout<=0��˵����ʱʱ�䵽�ˣ�Ҳû���յ�WIFI GOT IP������1
	return 0;                                       //��ȷ������0
}
/*-------------------------------------------------*/
/*��������WiFi���ӷ�����                           */
/*��  ������                                       */
/*����ֵ��0����ȷ   ����������                     */
/*-------------------------------------------------*/
char WiFi_Connect_IoTServer(void)
{	
	printf("׼����λģ��\r\n");                     //������ʾ����
	if(WiFi_Reset(50)){                                //��λ��100ms��ʱ��λ���ܼ�5s��ʱʱ��
		printf("��λʧ�ܣ�׼������\r\n");           //���ط�0ֵ������if��������ʾ����
		return 1;                                      //����1
	}else printf("��λ�ɹ�\r\n");                   //������ʾ����
	
	printf("׼������STAģʽ\r\n");                  //������ʾ����
	if(WiFi_SendCmd("AT+CWMODE=1",50)){                //����STAģʽ��100ms��ʱ��λ���ܼ�5s��ʱʱ��
		printf("����STAģʽʧ�ܣ�׼������\r\n");    //���ط�0ֵ������if��������ʾ����
		return 2;                                      //����2
	}else printf("����STAģʽ�ɹ�\r\n");            //������ʾ����
	
	if(wifi_mode==0){                                      //�������ģʽ=0��SSID������д�ڳ����� 
		printf("׼��ȡ���Զ�����\r\n");                 //������ʾ����
		if(WiFi_SendCmd("AT+CWAUTOCONN=0",50)){            //ȡ���Զ����ӣ�100ms��ʱ��λ���ܼ�5s��ʱʱ��
			printf("ȡ���Զ�����ʧ�ܣ�׼������\r\n");   //���ط�0ֵ������if��������ʾ����
			return 3;                                      //����3
		}else printf("ȡ���Զ����ӳɹ�\r\n");           //������ʾ����
				
		printf("׼������·����\r\n");                   //������ʾ����	
		if(WiFi_JoinAP(30)){                               //����·����,1s��ʱ��λ���ܼ�30s��ʱʱ��
			printf("����·����ʧ�ܣ�׼������\r\n");     //���ط�0ֵ������if��������ʾ����
			return 4;                                      //����4	
		}else printf("����·�����ɹ�\r\n");             //������ʾ����			
//	}else{                                                 //�������ģʽ=1��Smartconfig��ʽ,��APP����
//		if(KEY2_IN_STA==0){                                    //�����ʱK2�ǰ��µ�
//			printf("׼�������Զ�����\r\n");                 //������ʾ����
//			if(WiFi_SendCmd("AT+CWAUTOCONN=1",50)){            //�����Զ����ӣ�100ms��ʱ��λ���ܼ�5s��ʱʱ��
//				printf("�����Զ�����ʧ�ܣ�׼������\r\n");   //���ط�0ֵ������if��������ʾ����
//				return 3;                                      //����3
//			}else printf("�����Զ����ӳɹ�\r\n");           //������ʾ����	
//			
//			printf("׼������Smartconfig\r\n");              //������ʾ����
//			if(WiFi_SendCmd("AT+CWSTARTSMART",50)){            //����Smartconfig��100ms��ʱ��λ���ܼ�5s��ʱʱ��
//				printf("����Smartconfigʧ�ܣ�׼������\r\n");//���ط�0ֵ������if��������ʾ����
//				return 4;                                      //����4
//			}else printf("����Smartconfig�ɹ�\r\n");        //������ʾ����

//			printf("��ʹ��APP�����������\r\n");            //������ʾ����
//			if(WiFi_Smartconfig(60)){                          //APP����������룬1s��ʱ��λ���ܼ�60s��ʱʱ��
//				printf("��������ʧ�ܣ�׼������\r\n");       //���ط�0ֵ������if��������ʾ����
//				return 5;                                      //����5
//			}else printf("��������ɹ�\r\n");               //������ʾ����

//			printf("׼���ر�Smartconfig\r\n");              //������ʾ����
//			if(WiFi_SendCmd("AT+CWSTOPSMART",50)){             //�ر�Smartconfig��100ms��ʱ��λ���ܼ�5s��ʱʱ��
//				printf("�ر�Smartconfigʧ�ܣ�׼������\r\n");//���ط�0ֵ������if��������ʾ����
//				return 6;                                      //����6
//			}else printf("�ر�Smartconfig�ɹ�\r\n");        //������ʾ����
//		}else{                                                 //��֮����ʱK2��û�а���
//			printf("�ȴ�����·����\r\n");                   //������ʾ����	
//			if(WiFi_WaitAP(30)){                               //�ȴ�����·����,1s��ʱ��λ���ܼ�30s��ʱʱ��
//				printf("����·����ʧ�ܣ�׼������\r\n");     //���ط�0ֵ������if��������ʾ����
//				return 7;                                      //����7	
//			}else printf("����·�����ɹ�\r\n");             //������ʾ����					
//		}
	}
	
	printf("׼������͸��\r\n");                     //������ʾ����
	if(WiFi_SendCmd("AT+CIPMODE=1",50)){               //����͸����100ms��ʱ��λ���ܼ�5s��ʱʱ��
		printf("����͸��ʧ�ܣ�׼������\r\n");       //���ط�0ֵ������if��������ʾ����
		return 8;                                      //����8
	}else printf("����͸���ɹ�\r\n");               //������ʾ����
	
	printf("׼���رն�·����\r\n");                 //������ʾ����
	if(WiFi_SendCmd("AT+CIPMUX=0",50)){                //�رն�·���ӣ�100ms��ʱ��λ���ܼ�5s��ʱʱ��
		printf("�رն�·����ʧ�ܣ�׼������\r\n");   //���ط�0ֵ������if��������ʾ����
		return 9;                                      //����9
	}else printf("�رն�·���ӳɹ�\r\n");           //������ʾ����
	
	printf("׼�����ӷ�����\r\n");                   //������ʾ����
	if(WiFi_Connect_Server(100)){                      //���ӷ�������100ms��ʱ��λ���ܼ�10s��ʱʱ��
		printf("���ӷ�����ʧ�ܣ�׼������\r\n");     //���ط�0ֵ������if��������ʾ����
		return 10;                                     //����10
	}else printf("���ӷ������ɹ�\r\n");             //������ʾ����	
	
	return 0;                                          //��ȷ����0
	
}

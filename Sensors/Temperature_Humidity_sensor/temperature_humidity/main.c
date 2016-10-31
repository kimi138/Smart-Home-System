#include "ioCC2530.h"

#include <string.h>
#include "sht10.h"
#include <stdio.h>

//#define uint unsigned int
//#define uchar unsigned char
//#define Uint16 unsigned int
//������ƵƵĶ˿�
#define led1 P1_0
#define led2 P1_1

//��������
void Delay1(uint);
void initUARTtest(void);
void UartTX_Send_String(char *Data,int len);
void initLED(void);

union
{ unsigned int i;
  float f;
}humi_val,temp_val; //����������ͬ�壬һ������ʪ�ȣ�һ�������¶�

char temp_result[15] = {0};
float temp_value = 0;

char humi_result[15] = {0};
float humi_value = 0;

/****************************************************************
*�������� ��������								
*��ڲ��� ����							
*�� �� ֵ ����						
*˵    �� ����							
****************************************************************/
void main(void)
{	
	unsigned char error,checksum;
	unsigned char HUMI,TEMP;
	HUMI=0X01;
	TEMP=0X02;
	
	initUARTtest();    //��ʼ������
	s_connectionreset();
	while(1)
	{  
		error=0;
		error+=s_measure((unsigned char*) &humi_val.i,&checksum,HUMI);  //ʪ�Ȳ���
		error+=s_measure((unsigned char*) &temp_val.i,&checksum,TEMP);  //�¶Ȳ���
		
		temp_value = temp_val.i * 0.01 - 39.6;
		sprintf(temp_result,"%s","temperature: ");
		UartTX_Send_String(temp_result, 13);
		sprintf(temp_result,"%9.2f",temp_value);
		temp_result[9] = ':';
		UartTX_Send_String(temp_result, 10);	
		
		humi_value = humi_val.i * 0.0367 - 2.0468;
		sprintf(humi_result,"%s","humidity: ");
		UartTX_Send_String(humi_result, 10);
		sprintf(humi_result,"%9.4f",humi_value);
		humi_result[9] = '\n';
		UartTX_Send_String(humi_result, 10);
		
		if(error!=0)
		{
			s_connectionreset() ; //�����������ϵͳ��λ
			led1 =  !led1;
			led2 =  !led2;
		}	
		else
		{ 
			humi_val.f=(float)humi_val.i;                   //ת��Ϊ������
			temp_val.f=(float)temp_val.i;                   //ת��Ϊ������
			calc_sth11(&humi_val.f,&temp_val.f);            //�������ʪ�ȼ��¶�
			//dew_point=calc_dewpoint(humi_val.f,temp_val.f); //�������ʪ��ֵ
		}	
		// UartTX_Send_String(&error,1);
		Delay1(50000);                     //��ʱ
	}
}

/****************************************************************
*�������� ����ʱ						
*��ڲ��� ��������ʱ									
*�� �� ֵ ����												
*˵    �� ��													
****************************************************************/
void Delay1(uint n)
{
	uint i;
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
}

/****************************************************************
*�������� ����ʼ������1										
*��ڲ��� ����												
*�� �� ֵ ����							
*˵    �� ��57600-8-n-1						
****************************************************************/
void initUARTtest(void)
{

    CLKCONCMD &= ~0x40;              //����
    while(!(SLEEPSTA & 0x40));      //�ȴ������ȶ�
    CLKCONCMD &= ~0x47;             //TICHSPD128��Ƶ��CLKSPD����Ƶ
    SLEEPSTA |= 0x04; 		 //�رղ��õ�RC����

    PERCFG = 0x01;				//λ��1 ����0
    P1SEL |= 0x30;				//P1��������

    U0CSR |= 0x80;				//UART��ʽ
    U0GCR |= 8;				//baud_e
    U0BAUD |= 59;				//��������Ϊ9600
    UTX0IF = 1;
	
    U0CSR |= 0X40;				//�������
    IEN0 |= 0x84;				//�����жϣ������ж�
}

/****************************************************************
*�������� �����ڷ����ַ�������					
*��ڲ��� : data:����									
*			len :���ݳ���							
*�� �� ֵ ����											
*˵    �� ��				
****************************************************************/
void UartTX_Send_String(char *Data,int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}
#include "ioCC2530.h"
#include <string.h>

#define uint16 unsigned int
#define uchar unsigned char
#define TRUE 1
#define FALSE 0
#define RECEIVE P0_7
#define WARN_LED P1_7

void UartTX_Send_String(uchar *Data,int len);
void init_time1(void);
void initUART(void);
void InitAll(void);
void delay_ms(uint16);
void Warn(void);

uint16 value = 0;
uint16 counter = 0;
uint16 timer_counter = 0;
uint16 receiver_counter = 0;
uchar sound_flag = 0;
uchar uart_test[2] = {0x55, 0x55};

/***************************
//������
***************************/
void main(void)
{   
	InitAll();
	while(1)
	{ 
		if(sound_flag == 1)
		{
		    sound_flag = 0;
			Warn();
		}
	}
}
	
void init_time3(void)
{
	T3CCTL0 |= 0x04; //���óɱȽ�ģʽ�� 
	T3CC0   = 0x7D; 
	//T3CCTL1 = 0x00; 
	//T3CC1   = 0x00; 
	EA = 1;      
	T3IE = 1;
	T3CTL = 0xDE; //ʱ��16MHz��64��Ƶ����ʱ��0.25MHz����ʱ��counter��1������4us��ʹ�ܶ�ʱ����ʹ���жϡ����counter��ģʽ��modulo mode��
}

/****************************************************************
*�������� ����ʼ������1										
*��ڲ��� ����												
*�� �� ֵ ����							
*˵    �� ��38400-8-n-1						
****************************************************************/
void initUART(void)
{
    CLKCONCMD &= ~0x40;              //����
    while(!(SLEEPSTA & 0x40));      //�ȴ������ȶ�
    CLKCONCMD &= ~0x47;             //TICHSPD128��Ƶ��CLKSPD����Ƶ
    SLEEPSTA |= 0x04; 		 //�رղ��õ�RC����

    PERCFG = 0x00;				//λ��1 P0��
    P0SEL = 0x0c;				//P0��������

    U0CSR |= 0x80;				//UART��ʽ
    U0GCR |= 10;				//baud_e
    U0BAUD |= 59;				//��������Ϊ38400
    UTX0IF = 1;

    U0CSR |= 0X40;				//�������
    IEN0 |= 0x84;				//�����жϣ������ж�
}

void InitAll(void)
{
	EA=0;
	init_time3();
	initUART();
	//ADCCFG |= 0x02;
	P0DIR &= 0xFD;  //����P0_1Ϊ����
	P1DIR |= 0x80;  //����P1_7Ϊ���
	EA=1; //re-enable interrupts  
}

/****************************************************************
*�������� �����ڷ����ַ�������					
*��ڲ��� : data:����									
*			len :���ݳ���							
*�� �� ֵ ����											
*˵    �� ��				
****************************************************************/
void UartTX_Send_String(uchar *Data, int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}

#pragma vector = T3_VECTOR
__interrupt void T3_ISR(void)
{
	IRCON &= 0xFD; 
	T3IE = 0;
	counter++;
	if(counter == 50)
	{
		counter = 0;
		timer_counter++;
		F0 = RECEIVE;
		if(F0 == 1)	              
		{
			receiver_counter++; 
		}
		else
		{
		}
		if(timer_counter == 10)
		{
			if(receiver_counter >= 8)
			{
				sound_flag = 1;
			}
			else
			{
			}
			timer_counter = 0;
			receiver_counter = 0;
		}
		else
		{
		}
	}
	T3IE = 1;
}

void delay_ms(uint16 tms)
{
  uint16 ii;
  while(tms)
  {
    tms--;
    for(ii=0;ii<400;ii++)
    {
      asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
    }
  }
}

//�澯��������3��
void Warn(void)
{
  uchar ii;
  for(ii=0;ii<5;ii++)
  {
    WARN_LED = 1;
    delay_ms(120);
    WARN_LED = 0;
    delay_ms(120);
  }
}


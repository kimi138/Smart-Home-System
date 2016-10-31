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
//主函数
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
	T3CCTL0 |= 0x04; //配置成比较模式， 
	T3CC0   = 0x7D; 
	//T3CCTL1 = 0x00; 
	//T3CC1   = 0x00; 
	EA = 1;      
	T3IE = 1;
	T3CTL = 0xDE; //时钟16MHz，64分频，定时器0.25MHz，定时器counter增1，步进4us。使能定时器。使能中断。清除counter。模式：modulo mode。
}

/****************************************************************
*函数功能 ：初始化串口1										
*入口参数 ：无												
*返 回 值 ：无							
*说    明 ：38400-8-n-1						
****************************************************************/
void initUART(void)
{
    CLKCONCMD &= ~0x40;              //晶振
    while(!(SLEEPSTA & 0x40));      //等待晶振稳定
    CLKCONCMD &= ~0x47;             //TICHSPD128分频，CLKSPD不分频
    SLEEPSTA |= 0x04; 		 //关闭不用的RC振荡器

    PERCFG = 0x00;				//位置1 P0口
    P0SEL = 0x0c;				//P0用作串口

    U0CSR |= 0x80;				//UART方式
    U0GCR |= 10;				//baud_e
    U0BAUD |= 59;				//波特率设为38400
    UTX0IF = 1;

    U0CSR |= 0X40;				//允许接收
    IEN0 |= 0x84;				//开总中断，接收中断
}

void InitAll(void)
{
	EA=0;
	init_time3();
	initUART();
	//ADCCFG |= 0x02;
	P0DIR &= 0xFD;  //配置P0_1为输入
	P1DIR |= 0x80;  //配置P1_7为输出
	EA=1; //re-enable interrupts  
}

/****************************************************************
*函数功能 ：串口发送字符串函数					
*入口参数 : data:数据									
*			len :数据长度							
*返 回 值 ：无											
*说    明 ：				
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

//告警蜂鸣器响3次
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


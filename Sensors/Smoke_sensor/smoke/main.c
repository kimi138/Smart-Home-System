#include "ioCC2530.h"
#include <string.h>
#include <stdio.h>

#define uint unsigned int
#define uchar unsigned char
#define uint8 unsigned char
#define uint16 unsigned int

//定义控制灯的端口
#define led1 P0_0
#define led2 P0_1
#define HAL_ADC_REF_3V3    0x10    /* external 3.3V Reference */
#define HAL_ADC_DEC_128     0x10    /* Decimate by 128 : 10-bit resolution */
#define HAL_ADC_CHN_5       0x05    /* Input channel: channel */
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

void Delay(uint);
void initUARTtest(void);
void InitialAD(void);
void UartTX_Send_String(char *Data,int len);
char test[20] = "this is a test!";

uint16 gas_temp = 0;
char gas_result[15] = {0};
float gas_value = 0;

/****************************************************************
*函数功能 ：获取气体					*
*入口参数 ：无							*
*返 回 值 ：无							*
*说    明 ：无							*
*Value = 0x0A5F，对应电压值 = 0x0A5F * 3.3 / 8192 = 1.0695V 
****************************************************************/
uint16 Gas_Acquire(void)
{
	uint16 value;
	/* Clear ADC interrupt flag */
	ADCIF = 0;
	//ADCCON3 = (HAL_ADC_REF_3V3 | HAL_ADC_DEC_128 | HAL_ADC_CHN_5);
	//ADCCON3 = 0xb7;
	P0DIR &= 0x7F;
	ADCCON3 = 0xb7;
	ADCCON1 |= 0x30;
	ADCCON1 |= 0x40;
	while ((ADCCON1 & 0x80) == 0);
	
	/* Get the result */
	value = ADCH;
	value = value << 8;
	value = value + ADCL;
	value = value >> 2;
	return value;
}

/****************************************************************
*函数功能 ：主函数						*
*入口参数 ：无							*
*返 回 值 ：无							*
*说    明 ：无							*
****************************************************************/
void main(void)
{	
    P0DIR = 0x03;
	APCFG |= 0x80;
    led1 = 1;
    led2 = 1;		        
    //initUARTtest();
    while(1)
    {
		gas_temp = Gas_Acquire();
    }
}
/****************************************************************
*函数功能 ：延时						*
*入口参数 ：定性延时						*
*返 回 值 ：无							*
*说    明 ：							*
****************************************************************/
void Delay(uint n)
{
    uint i;
    for(i=0;i<n;i++);
    for(i=0;i<n;i++);
    for(i=0;i<n;i++);
    for(i=0;i<n;i++);
    for(i=0;i<n;i++);
}

/****************************************************************
*函数功能 ：初始化串口1										
*入口参数 ：无												
*返 回 值 ：无							
*说    明 ：57600-8-n-1						
****************************************************************/
void initUARTtest(void)
{

    CLKCONCMD &= ~0x40;              //晶振
    while(!(SLEEPSTA & 0x40));      //等待晶振稳定
    CLKCONCMD &= ~0x47;             //TICHSPD128分频，CLKSPD不分频
    SLEEPSTA |= 0x04; 		 //关闭不用的RC振荡器

    PERCFG = 0x01;				//位置1 串口0
    P1SEL |= 0x30;				//P1用作串口

    U0CSR |= 0x80;				//UART方式
    U0GCR |= 8;				//baud_e
    U0BAUD |= 59;				//波特率设为9600
    UTX0IF = 1;
	
    U0CSR |= 0X40;				//允许接收
    IEN0 |= 0x84;				//开总中断，接收中断
}

/****************************************************************
*函数功能 ：串口发送字符串函数					*
*入口参数 : data:数据						*
*	    len :数据长度				        *
*返 回 值 ：无						        *
*说    明 ：							*
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



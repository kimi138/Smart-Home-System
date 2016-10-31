#include "ioCC2530.h"

#include <string.h>
#include "sht10.h"
#include <stdio.h>

//#define uint unsigned int
//#define uchar unsigned char
//#define Uint16 unsigned int
//定义控制灯的端口
#define led1 P1_0
#define led2 P1_1

//函数声明
void Delay1(uint);
void initUARTtest(void);
void UartTX_Send_String(char *Data,int len);
void initLED(void);

union
{ unsigned int i;
  float f;
}humi_val,temp_val; //定义两个共同体，一个用于湿度，一个用于温度

char temp_result[15] = {0};
float temp_value = 0;

char humi_result[15] = {0};
float humi_value = 0;

/****************************************************************
*函数功能 ：主函数								
*入口参数 ：无							
*返 回 值 ：无						
*说    明 ：无							
****************************************************************/
void main(void)
{	
	unsigned char error,checksum;
	unsigned char HUMI,TEMP;
	HUMI=0X01;
	TEMP=0X02;
	
	initUARTtest();    //初始化串口
	s_connectionreset();
	while(1)
	{  
		error=0;
		error+=s_measure((unsigned char*) &humi_val.i,&checksum,HUMI);  //湿度测量
		error+=s_measure((unsigned char*) &temp_val.i,&checksum,TEMP);  //温度测量
		
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
			s_connectionreset() ; //如果发生错误，系统复位
			led1 =  !led1;
			led2 =  !led2;
		}	
		else
		{ 
			humi_val.f=(float)humi_val.i;                   //转换为浮点数
			temp_val.f=(float)temp_val.i;                   //转换为浮点数
			calc_sth11(&humi_val.f,&temp_val.f);            //修正相对湿度及温度
			//dew_point=calc_dewpoint(humi_val.f,temp_val.f); //计算绝对湿度值
		}	
		// UartTX_Send_String(&error,1);
		Delay1(50000);                     //延时
	}
}

/****************************************************************
*函数功能 ：延时						
*入口参数 ：定性延时									
*返 回 值 ：无												
*说    明 ：													
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
*函数功能 ：串口发送字符串函数					
*入口参数 : data:数据									
*			len :数据长度							
*返 回 值 ：无											
*说    明 ：				
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
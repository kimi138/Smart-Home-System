#include "ioCC2530.h"
#include <string.h>
#include <stdio.h>

#define uint unsigned int
#define uchar unsigned char
#define TRUE 1
#define FALSE 0
#define SCL P0_0
#define SDA P0_1
#define I2C_WRITE	0x00	//*T
#define I2C_READ	0x01	//*R
#define MPL3115A2_I2C_ADDRESS 0xC0  //

//定义控制灯的端口
#define YLED P2_0	//定义LED1为P2_0口控制
#define RLED P0_5	//定义LED2为P0_5口控制

char data_result[6] = {0x55,0x55,0x55,0x55,0x55,0x55};

long int pressure_temp = 0;
char pressure_result[15] = {0};
float pressure_value = 0;

int temp_temp = 0;
char temp_result[15] = {0};
float temp_value = 0;

char INT_FLAG[1] = {0x00};
int j = 0;  //调试用

void Init_IO_INT(void);
void Delay_1u(uint);
void WriteSDA1(void);
void WriteSDA0(void);
void WriteSCL1(void);
void WriteSCL0(void);
void ReadSDA(void);
void I2C_Start(void);
void I2C_Stop(void);
void SEND_0(void);
void SEND_1(void);
char Check_Acknowledge(void);
void WriteI2CByte(char);
char ReadI2CByte(void);
void Read_Data(char, char *);
void Write_Data(char, char);

void initUARTtest(void);
void UartTX_Send_String(char *Data,int len);
void initLed(void);

/***************************
//主函数
***************************/
void main(void)
{
  initLed();
  initUARTtest();
  Write_Data(0x26, 0x39);
  while(1)
  { 
    Read_Data(0x01, &data_result[1]);
    Read_Data(0x02, &data_result[2]);
    Read_Data(0x03, &data_result[3]);
    Read_Data(0x04, &data_result[4]);
    Read_Data(0x05, &data_result[5]);
    
	pressure_temp = data_result[1];
	pressure_temp = pressure_temp << 8;
	pressure_temp = pressure_temp  + data_result[2];
	pressure_temp = pressure_temp << 8;
	pressure_temp = pressure_temp + data_result[3];
	pressure_temp = pressure_temp >> 4;
	pressure_value = pressure_temp * 0.25;
	sprintf(pressure_result,"%s","pressure: ");
	UartTX_Send_String(pressure_result, 10);
	sprintf(pressure_result,"%9.2f",pressure_value);
	pressure_result[9] = '\n';
	UartTX_Send_String(pressure_result, 10);
	
	temp_temp = data_result[4];
	temp_temp = temp_temp << 8;
	temp_temp = temp_temp  + data_result[5];
	temp_temp = temp_temp >> 4;
	temp_value = temp_temp * 0.0625;
        
        if (temp_value > 22)
        {
          for(uchar i=0; i < 3; i++)
          {
            YLED = !YLED;
            RLED = !RLED;
            
            Delay_1u(1000);
          }
        }
        
	sprintf(pressure_result,"%s","temperature: ");
	UartTX_Send_String(pressure_result, 13);
	sprintf(temp_result, "%8.4f", temp_value);
	temp_result[8] = '\n';
	UartTX_Send_String(temp_result, 9);
  }
}

/*****************************************
//io及中断初始化
*****************************************/
void Init_IO_INT(void)
{
    P1SEL &= 0xFD; //P11作为普通IO
    P1DIR &= 0xFD; //P11作为输入
    P1INP &= 0xFD;//P11有上拉、下拉
    PICTL |= 0X02; //下降沿
    EA = 1;
    IEN2 |= 0X10; // P1IE = 1;
    P1IEN |= 0X02; //使能P1_1中断
    P1IF = 0;  //清中断标志
    P1IFG = 0; //清中断标志
}

#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
    if((P1IFG & 0x02) != 0) //P1_1中断
    {
        P1IFG &= 0xFD;
        Read_Data(0x02, &INT_FLAG[0]);
        Write_Data(0x02, INT_FLAG[0] & 0x77);
   //     Read_Data(0x08, &data_result[1]);
    //    Read_Data(0x09, &data_result[2]);
    //    Read_Data(0x0A, &data_result[3]);
        j++; //调试用
    }
    P1IFG = 0;//清中断标志
    P1IF = 0; //清中断标志
}

void Delay_1u(unsigned int microSecs) {
  while(microSecs--)
  {
    /* 32 NOPs == 1 usecs */
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
  }
}

	void WriteSDA1(void)//SDA 输出1   
	{
		 P0DIR |= 0x02;
		 SDA = 1;
	}
	
	void WriteSDA0(void)//SDA 输出0    
	{
		 P0DIR |= 0x02;
		 SDA = 0;
	}
	
	void WriteSCL1(void)//SCL 输出1    
	{
			 P0DIR |= 0x01;
			 SCL = 1;
	}
		
	void WriteSCL0(void)//SCL 输出0    
	{
			 P0DIR |= 0x01;
			 SCL = 0;
	}

void ReadSDA(void)//这里设置SDA对应IO口DIR可以接收数据    
{
     P0DIR &= 0xFD;
}

/*启动I2C总线的函数，当SCL为高电平时使SDA产生一个负跳变*/       
void I2C_Start(void)
{
	WriteSDA1();
	WriteSCL1();
	Delay_1u(50);
	WriteSDA0();
	Delay_1u(50);
	WriteSCL0();
	Delay_1u(50);
}

/*终止I2C总线，当SCL为高电平时使SDA产生一个正跳变*/
void I2C_Stop(void)
{
	WriteSDA0();
	Delay_1u(50);
	WriteSCL1();
	Delay_1u(50);
	WriteSDA1();
	Delay_1u(50);
	WriteSCL0();
	Delay_1u(50);
}

/*发送0，在SCL为高电平时使SDA信号为低*/
void SEND_0(void)
{
    WriteSDA0();
    WriteSCL1();
    Delay_1u(50);
    WriteSCL0();
    Delay_1u(50);
}

/*发送1，在SCL为高电平时使SDA信号为高*/
void SEND_1(void)
{
    WriteSDA1();
    WriteSCL1();
    Delay_1u(50);
    WriteSCL0();
    Delay_1u(50);
}

/*发送完一个字节后检验设备的应答信号*/    
char Check_Acknowledge(void)
{
    WriteSDA1();
    WriteSCL1();
    Delay_1u(50);
    F0=SDA;
    Delay_1u(50);
    WriteSCL0();
    Delay_1u(50);
    if(F0==1)
        return FALSE;
    return TRUE;
}

/*向I2C总线写一个字节*/
void WriteI2CByte(char b)
{
    char i;
    for(i=0;i<8;i++)
    {
      if((b<<i)&0x80)
      {
         SEND_1();
      }
      else
      {
         SEND_0();
      }
    }
}

/*从I2C总线读一个字节*/
char ReadI2CByte(void)
{
    char b=0,i;
    WriteSDA1();

    for(i=0;i<8;i++)
    {   
        WriteSCL0();
        Delay_1u(50);
        WriteSCL1(); 
        Delay_1u(50);

        ReadSDA();
        F0=SDA;//寄存器中的一位,用于存储SDA中的一位数据

    if(F0==1)
        {
          b=b<<1;
          b=b|0x01;
        }
        else
          b=b<<1;
    }
    WriteSCL0();
    return b; 
}

void Read_Data(char reg, char *data)
{
	I2C_Start();
        
	WriteI2CByte(MPL3115A2_I2C_ADDRESS + I2C_WRITE);
        while(Check_Acknowledge() == FALSE);
        
        WriteI2CByte(reg);
        while(Check_Acknowledge() == FALSE);
        
        I2C_Start();
        
        WriteI2CByte(MPL3115A2_I2C_ADDRESS + I2C_READ);
        while(Check_Acknowledge() == FALSE);
        
        *data = ReadI2CByte();
        I2C_Stop();
}

void Write_Data(char reg, char data)
{
	I2C_Start();
        
	WriteI2CByte(MPL3115A2_I2C_ADDRESS + I2C_WRITE);
        while(Check_Acknowledge() == FALSE);
        
        WriteI2CByte(reg);
        while(Check_Acknowledge() == FALSE);
        
        WriteI2CByte(data);
        while(Check_Acknowledge() == FALSE);
        
        I2C_Stop();
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
    U0GCR |= 8;				        //baud_e
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

void initLed(void)
{
  P0DIR |= 0x20; //P0_5定义为输出
  P2DIR |= 0x01; //P2_0定义为输出
  RLED = 1;
  YLED = 1;	//LED灯灭
}


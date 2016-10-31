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

//������ƵƵĶ˿�
#define YLED P2_0	//����LED1ΪP2_0�ڿ���
#define RLED P0_5	//����LED2ΪP0_5�ڿ���

char data_result[6] = {0x55,0x55,0x55,0x55,0x55,0x55};

long int pressure_temp = 0;
char pressure_result[15] = {0};
float pressure_value = 0;

int temp_temp = 0;
char temp_result[15] = {0};
float temp_value = 0;

char INT_FLAG[1] = {0x00};
int j = 0;  //������

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
//������
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
//io���жϳ�ʼ��
*****************************************/
void Init_IO_INT(void)
{
    P1SEL &= 0xFD; //P11��Ϊ��ͨIO
    P1DIR &= 0xFD; //P11��Ϊ����
    P1INP &= 0xFD;//P11������������
    PICTL |= 0X02; //�½���
    EA = 1;
    IEN2 |= 0X10; // P1IE = 1;
    P1IEN |= 0X02; //ʹ��P1_1�ж�
    P1IF = 0;  //���жϱ�־
    P1IFG = 0; //���жϱ�־
}

#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
    if((P1IFG & 0x02) != 0) //P1_1�ж�
    {
        P1IFG &= 0xFD;
        Read_Data(0x02, &INT_FLAG[0]);
        Write_Data(0x02, INT_FLAG[0] & 0x77);
   //     Read_Data(0x08, &data_result[1]);
    //    Read_Data(0x09, &data_result[2]);
    //    Read_Data(0x0A, &data_result[3]);
        j++; //������
    }
    P1IFG = 0;//���жϱ�־
    P1IF = 0; //���жϱ�־
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

	void WriteSDA1(void)//SDA ���1   
	{
		 P0DIR |= 0x02;
		 SDA = 1;
	}
	
	void WriteSDA0(void)//SDA ���0    
	{
		 P0DIR |= 0x02;
		 SDA = 0;
	}
	
	void WriteSCL1(void)//SCL ���1    
	{
			 P0DIR |= 0x01;
			 SCL = 1;
	}
		
	void WriteSCL0(void)//SCL ���0    
	{
			 P0DIR |= 0x01;
			 SCL = 0;
	}

void ReadSDA(void)//��������SDA��ӦIO��DIR���Խ�������    
{
     P0DIR &= 0xFD;
}

/*����I2C���ߵĺ�������SCLΪ�ߵ�ƽʱʹSDA����һ��������*/       
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

/*��ֹI2C���ߣ���SCLΪ�ߵ�ƽʱʹSDA����һ��������*/
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

/*����0����SCLΪ�ߵ�ƽʱʹSDA�ź�Ϊ��*/
void SEND_0(void)
{
    WriteSDA0();
    WriteSCL1();
    Delay_1u(50);
    WriteSCL0();
    Delay_1u(50);
}

/*����1����SCLΪ�ߵ�ƽʱʹSDA�ź�Ϊ��*/
void SEND_1(void)
{
    WriteSDA1();
    WriteSCL1();
    Delay_1u(50);
    WriteSCL0();
    Delay_1u(50);
}

/*������һ���ֽں�����豸��Ӧ���ź�*/    
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

/*��I2C����дһ���ֽ�*/
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

/*��I2C���߶�һ���ֽ�*/
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
        F0=SDA;//�Ĵ����е�һλ,���ڴ洢SDA�е�һλ����

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
    U0GCR |= 8;				        //baud_e
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

void initLed(void)
{
  P0DIR |= 0x20; //P0_5����Ϊ���
  P2DIR |= 0x01; //P2_0����Ϊ���
  RLED = 1;
  YLED = 1;	//LED����
}


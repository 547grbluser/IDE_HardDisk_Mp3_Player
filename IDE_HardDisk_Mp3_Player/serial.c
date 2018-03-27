#include "serial.h"

sfr AUXR  = 0x8E;
void UartInit(void)		//1200bps@24.000MHz
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR &= 0xBF;		//定时器1时钟为Fosc/12,即12T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xCC;		//设定定时初值
	TH1 = 0xCC;		//设定定时器重装值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
}

void Serial_Init()	  // 11.0592Mhz
{
	TMOD=0X20; //定时器T1使用方式2
	TH1=253;   //设定初值
	TL1=253;   

//	TH1=250;   //设定初值
//	TL1=250;   
//	PCON=0X80;	//SMOD=1，波特率加倍

	TR1=1;	   //启动定时器T1
	SCON=0X50;	//串口工作方式1，波特率9600bit/s ，REN=1允许接收
	REN=1;
	ES=1;		//打开/关闭串口中断
	TI=0;
	//PS=1;
	EA=1;
}

void SendByte(u8 byt)
{
	ES=0;
	SBUF=byt;
	while(!TI);
	TI=0;
	ES=1;
}

void SendStr(u8 *str)
{

	while(*str!='\0')
	{
		SendByte(*str);
		str++;	
	}


}

#include "serial.h"

sfr AUXR  = 0x8E;
void UartInit(void)		//1200bps@24.000MHz
{
	PCON &= 0x7F;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR &= 0xBF;		//��ʱ��1ʱ��ΪFosc/12,��12T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�����ʱ��1ģʽλ
	TMOD |= 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
	TL1 = 0xCC;		//�趨��ʱ��ֵ
	TH1 = 0xCC;		//�趨��ʱ����װֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
}

void Serial_Init()	  // 11.0592Mhz
{
	TMOD=0X20; //��ʱ��T1ʹ�÷�ʽ2
	TH1=253;   //�趨��ֵ
	TL1=253;   

//	TH1=250;   //�趨��ֵ
//	TL1=250;   
//	PCON=0X80;	//SMOD=1�������ʼӱ�

	TR1=1;	   //������ʱ��T1
	SCON=0X50;	//���ڹ�����ʽ1��������9600bit/s ��REN=1�������
	REN=1;
	ES=1;		//��/�رմ����ж�
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

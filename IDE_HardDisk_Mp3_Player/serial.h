#define _SERIAL_H
#define _SERIAL_H
#include <at89x52.h>

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long


/*
������غ���
*/
void UartInit(void);		//1200bps@24.000MHz
void Serial_Init();	  // 11.0592Mhz	���ڳ�ʼ��
void SendStr(u8 *str); //���ڷ����ַ���
void SendByte(u8 byt); //���ڷ���һ���ֽ�

#endif




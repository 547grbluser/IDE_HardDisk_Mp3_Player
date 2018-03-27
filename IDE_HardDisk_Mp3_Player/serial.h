#define _SERIAL_H
#define _SERIAL_H
#include <at89x52.h>

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long


/*
串口相关函数
*/
void UartInit(void);		//1200bps@24.000MHz
void Serial_Init();	  // 11.0592Mhz	串口初始化
void SendStr(u8 *str); //串口发送字符串
void SendByte(u8 byt); //串口发送一个字节

#endif




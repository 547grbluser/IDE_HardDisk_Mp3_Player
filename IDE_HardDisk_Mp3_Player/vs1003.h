#ifndef _VS1003_H
#define _VS1003_H
#include <at89x52.h>

sbit VS_MOSI=P3^3;
sbit VS_MISO=P3^1;

sbit VS_SCLK=P3^4;
sbit VS_xCS=P3^6;
sbit VS_xDCS=P3^7;
sbit VS_RESET=P3^0;
sbit VS_DREQ=P3^5;

#define VS_WRAM      0x06    //RAM¶ÁÐ´
#define VS_WRAMADDR  0x07    //RAM¶ÁÐ´µØÖ·

#define VOL_VALUE 0x0b   //ÒôÁ¿¿ØÖÆ¼Ä´æÆ÷

#define SCI_BASS 0x02  //¸ßÖÐµÍÒô¿ØÖÆ¼Ä´æÆ÷


void VS_spi_write(unsigned char dat);
unsigned char VS_spi_read(void);
void VS_WRITE_Reg(unsigned char addr,unsigned char hdat,unsigned char ldat);
unsigned int VS_READ_Reg(unsigned char addr);
void VS_Reset(void);
void VS_Send_Dat(unsigned char dat);
void VS_Flush_Buffer(void);
void VS_Sin_Test(unsigned char dat);

#endif

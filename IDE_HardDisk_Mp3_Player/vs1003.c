#include "vs1003.h"


void DelayXus(unsigned int xus)
{
	while(--xus);
}
void VS_spi_write(unsigned char dat)
{

   VS_MOSI=(dat&0x80);
		VS_SCLK=0;
		VS_SCLK=1;
		
	VS_MOSI=(dat&0x40);
		VS_SCLK=0;
		VS_SCLK=1;
	
	VS_MOSI=(dat&0x20);
		VS_SCLK=0;
		VS_SCLK=1;

	VS_MOSI=(dat&0x10);
		VS_SCLK=0;
		VS_SCLK=1;

	VS_MOSI=(dat&0x08);
		VS_SCLK=0;
		VS_SCLK=1;

	VS_MOSI=(dat&0x04);
	VS_SCLK=0;
	VS_SCLK=1;

	VS_MOSI=(dat&0x02);
		VS_SCLK=0;
		VS_SCLK=1;

	VS_MOSI=(dat&0x01);
		VS_SCLK=0;
		VS_SCLK=1;
}

unsigned char VS_spi_read(void)
{
	char i,dat=0;
	for(i=0;i<8;i++)
	{
		VS_SCLK=1;
		VS_SCLK=0;
		if(1==VS_MISO) //
		{
			dat=dat|0x01;
		}
		dat<<=1;
	}
	return dat;
}

void VS_WRITE_Reg(unsigned char addr,unsigned char hdat,unsigned char ldat)
{

	while(!VS_DREQ); //
    VS_xCS=0;
	VS_spi_write(0x02);//
	VS_spi_write(addr);
	VS_spi_write(hdat);
	VS_spi_write(ldat);
	VS_xCS=1;	
}

unsigned int VS_READ_Reg(unsigned char addr)
{
	unsigned int temp=0;
	
	while(!VS_DREQ); // 
	VS_xCS=0;
	VS_spi_write(0x03);//
	VS_spi_write(addr);
	temp=VS_spi_read();//
	temp<<=8;
	temp|=VS_spi_read();//
	VS_xCS=1;
	return temp;
}

void VS_Reset(void)
{

//	VS_RESET=1;
	DelayXus(100);
	
//	VS_RESET=0;
	DelayXus(100);
	VS_RESET=1;
	DelayXus(100);
	
	VS_WRITE_Reg(0x00,0x08,0x04);
	VS_WRITE_Reg(0x03,0x98,0x00);
	VS_WRITE_Reg(0x0b,32,32); 
	
	
	VS_xDCS=0;
	VS_spi_write(0);
	VS_spi_write(0);
	VS_spi_write(0);
	VS_spi_write(0);
	VS_xDCS=1;
}

void VS_Send_Dat(unsigned char dat)
{
VS_xDCS=0;
	while(!VS_DREQ);
	VS_spi_write(dat);
	VS_xDCS=1;
}

void VS_Flush_Buffer(void)
{
	unsigned int i;
	VS_xDCS=0;
	for(i=0;i<2048;i++)
	{
		VS_Send_Dat(0);
	}
	VS_xDCS=1;
}

void VS_Sin_Test(unsigned char dat)
{
	VS_WRITE_Reg(0x00,0x08,0x20);

	while(!VS_DREQ);
	
	VS_xDCS=0;
	VS_spi_write(0x53);
	VS_spi_write(0xef);
	VS_spi_write(0x6e);
	VS_spi_write(dat);
	VS_spi_write(0);
	VS_spi_write(0);
	VS_spi_write(0);
	VS_spi_write(0);

	DelayXus(600000);
	DelayXus(600000);
	DelayXus(600000);
	DelayXus(600000);
	DelayXus(600000);

	VS_spi_write(0x45);
	VS_spi_write(0x78);
	VS_spi_write(0x69);
	VS_spi_write(0x74);
	VS_spi_write(0);
	VS_spi_write(0);
	VS_spi_write(0);
	VS_spi_write(0);
	VS_xDCS=1;
}
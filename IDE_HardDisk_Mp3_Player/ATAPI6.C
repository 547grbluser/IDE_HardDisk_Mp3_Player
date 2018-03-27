
/***********************************
编译环境：keil c51				   *
工程名字：IDE硬盘扇区读写驱动	   *
版本：v1.02						   *
日期：2015/09/18				   *
作者：康朝阳 					   *
QQ:547336083					   *
QQ群:284285453                     *
主控芯片：STC12C5A60S2			   *
晶振频率：24.00Mhz			       *
适用硬盘：适用ATAPI-6协议的IDE硬盘 *
***********************************/

#include "ATAPI6.h"


unsigned char ReversalData(unsigned char byReversal)
{
	unsigned char temp;
	temp=byReversal<<7;
	temp=temp+((byReversal&0x2)<<5);
	temp=temp+((byReversal&0x4)<<3);
	temp=temp+((byReversal&0x8)<<1);
	temp=temp+(byReversal>>7);
	temp=temp+((byReversal&0x40)>>5);
	temp=temp+((byReversal&0x20)>>3);
	temp=temp+((byReversal&0x10)>>1);
	return temp;
}

void Delay1ms()		//@11.0592MHz
{
	unsigned char i, j;


	i = 2;
	j = 199;
	do
	{
		while (--j);
	} while (--i);
}

void DelayXms(u16 xms)
{
	while((xms--)!=0)
	{
		Delay1ms();	
	}
}

	 //设定待操作的硬盘寄存器函数
void SetRegAddress(u8 addr)
{
   switch(addr)
   {
   	 case DATA_REG:          CS1=1; CS0=0; DA2=0; DA1=0; DA0=0; break;

	 case ERROR_REG:         CS1=1; CS0=0; DA2=0; DA1=0; DA0=1; break; 
	 case FEATURES:	         CS1=1; CS0=0; DA2=0; DA1=0; DA0=1; break; 

	 case SECTOR_COUNT_REG:	 CS1=1; CS0=0; DA2=0; DA1=1; DA0=0; break;

	 case SECTOR_NUMBER_REG: CS1=1; CS0=0; DA2=0; DA1=1; DA0=1; break;
	 case LBA_0_7:           CS1=1; CS0=0; DA2=0; DA1=1; DA0=1; break;

	 case LBA_CYLINDER_L:    CS1=1; CS0=0; DA2=1; DA1=0; DA0=0; break; 
	 case LBA_8_15:          CS1=1; CS0=0; DA2=1; DA1=0; DA0=0; break;

	 case LBA_CYLINDER_H:    CS1=1; CS0=0; DA2=1; DA1=0; DA0=1; break;
	 case LBA_16_23:         CS1=1; CS0=0; DA2=1; DA1=0; DA0=1; break;

	 case DRIVE_HEAD:        CS1=1; CS0=0; DA2=1; DA1=1; DA0=0; break;
	 case LBA_24_27:         CS1=1; CS0=0; DA2=1; DA1=1; DA0=0; break;

	 case COMMAND_STATUS:    CS1=1; CS0=0; DA2=1; DA1=1; DA0=1; break;

	 case HDD_CONTROL:       CS1=0; CS0=1; DA2=1; DA1=1; DA0=0; break;

	 default: break;
   }
  
}
 //读取一个字节  低8位	 
u8 	HDDReadByte(u8 addr)
{
	u8 value;
	while(!IORDY);
	SetRegAddress(addr);
	HDDATAL=0xFF;
	HDDATAH=0xFF;
	ReadEN=0;
	
	value=ReversalData(HDDATAL);
	
	ReadEN=1;
   	
		CS0=1;CS1=1;
	return value;
}
//写一个字节  低8位	   
void HDDWriteByte(u8 addr, u8 value)
{
	while(!IORDY);
	SetRegAddress(addr);
	WriteEN=0;

	HDDATAL=ReversalData(value);

	WriteEN=1;
		CS0=1;CS1=1;
}
  //获取硬盘状态寄存器内容
u8 HDDReadStatus(void)
{
	u8 status;
	status=HDDReadByte(COMMAND_STATUS);
	return status;
}
   // 判断硬盘是否忙 
void HDDWaitBusy(void)
{
	while(0x80&HDDReadStatus());
}
  //对硬盘发送命令函数
void HDDWriteCommand(u8 command)
{

	HDDWriteByte(COMMAND_STATUS,command);
	HDDWaitBusy();
}
   //设定要操作的硬盘的扇区地址
void HDDWriteLBA(u32 LBA)
{

	HDDWriteByte(LBA_24_27,(u8)(0xe0|(DRV0_DRV1?0x10:0x00)|(LBA>>24)));
	HDDWriteByte(LBA_16_23,(u8)(LBA>>16));
	HDDWriteByte(LBA_8_15,(u8)(LBA>>8));
	HDDWriteByte(LBA_0_7,(u8)(LBA));
}

	  //设定要操作多少个扇区 
void HDDWriteSectorCount(u8 count)
{
	HDDWriteByte(SECTOR_COUNT_REG,count);		
}
	//硬盘初始化
u8 HDD_Init(void)
{
	
	HDDWriteByte(DRIVE_HEAD,0xE0|(DRV0_DRV1?0x10:0x00));
	HDDWaitBusy();
	
	HDDWriteByte(HDD_CONTROL,(1<<HOB)|(0<<SRST)|(1<<nIEN)|(0<<BIT_0));
	HDDWaitBusy();

	return 0;
}
	//运行硬盘命令
void HDRunning(void)
{
	HDDWaitBusy();
	HDDWriteCommand(0xE1);	
}
   //停止硬盘命令
void HDStop(void)
{
	HDDWaitBusy();
	HDDWriteCommand(0xE0);	
}
	//读一个指定扇区内容到	  SectorBuffer
u8 ReadSector(u32 addr, u8 *SectorBuffer )
{
	u16 i;
   	HDDWaitBusy();
	HDDWriteSectorCount(1);
	HDDWriteLBA(addr);
	HDDWriteCommand(HDD_Read_Sectors);
	SetRegAddress(DATA_REG);
	HDDATAL=0xFF;
	HDDATAH=0xFF;

	for(i=0;i<512;i++)
	{
		while(!IORDY);
		ReadEN=0;
	
		SectorBuffer[i]=ReversalData(HDDATAL);
		i++;
		SectorBuffer[i]=HDDATAH;
		ReadEN=1;
	}

	return 0;
}
   //写	SectorBuffer内容到指定扇区
u8 WriteSector(u32 addr, u8 *SectorBuffer)
{
	u16 i;

	HDDWaitBusy();
	HDDWriteLBA(addr);
	HDDWriteSectorCount(1); 
	HDDWriteCommand(HDD_Write_Sectors);
	SetRegAddress(DATA_REG);
	for(i=0;i<512;i++)
	{
		while(!IORDY);
		WriteEN=0;
		HDDATAL=ReversalData(SectorBuffer[i]);
		i++;
		HDDATAH=SectorBuffer[i];
		WriteEN=1;
		HDDATAL=0xFF;
		HDDATAH=0xFF;
	}
	
	return 0;	
}
  //读n个指定扇区内容到	  SectorBuffer
u8 ReadnSector(u32 addr, u8 *SectorBuffer, u32 nsec )
{
   u32 i=0;
   for(i=0;i<nsec;i++)
   {
   		ReadSector(addr+i,SectorBuffer);
		SectorBuffer+=512;	
   }
   return 0;
}
   //写	SectorBuffer内容到指定的n个扇区
u8 WritenSector(u32 addr, u8 *SectorBuffer, u32 nsec)
{
   u32 i=0;
   for(i=0;i<nsec;i++)
   {
   		WriteSector(addr+i,SectorBuffer);
		SectorBuffer+=512;	
   }
   return 0;
}

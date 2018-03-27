

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
#ifndef _ATAPI6_H
#define _ATAPI6_H
#include <at89x52.h>

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long 


#define HDD_CONTROL 0x0d
#define COMMAND_STATUS 0x0c

#define LBA_24_27 0x0b
#define DRIVE_HEAD 0x0a

#define LBA_16_23 0x09
#define LBA_CYLINDER_H 0x08

#define LBA_8_15 0x07
#define LBA_CYLINDER_L 0x06

#define LBA_0_7 0x05
#define SECTOR_NUMBER_REG 0x04

#define SECTOR_COUNT_REG 0x03

#define FEATURES 0x02
#define ERROR_REG 0x01

#define DATA_REG 0x00



#define  HDD_Read_Sectors   0x20
#define  HDD_Write_Sectors  0x30

#define HOB 7	 //28bit LBA_Address & 48bit LBA_Address choose
#define SRST 2	  // 软件复位，置1，开始执行软件复位协议
#define nIEN 1	  //数据中断使能位，低电平有效
#define BIT_0 0   //bit0应该被置0
/*
  硬盘主从选择
*/
#define DRV0_DRV1 1   //1:driver1  0:driver0

/*
声明数据线端口
*/
#define HDDATAL	P0
#define HDDATAH	P2


 
sbit DA0=P1^6;
sbit DA1=P1^5;
sbit DA2=P1^3;
sbit CS1=P1^4;
sbit CS0=P1^7;
sbit IORDY=P1^2;
sbit ReadEN=P1^1;
sbit WriteEN=P1^0;

/*
硬盘底层驱动函数，不需要用户直接调用
*/
void SetRegAddress(u8 addr);  //设定寄存器地址
void Delay1ms();		//@11.0592MHz
u8 	 HDDReadByte(u8 addr); //从硬盘读取一个字节
void HDDWriteByte(u8 addr, u8 value);//写一个字节到硬盘
u8   HDDReadStatus(void);  //读取硬盘的状态寄存器
void HDDWaitBusy(void);	   //等待硬盘忙
void HDDWriteCommand(u8 command);  //写命令到硬盘
void HDDWriteLBA(u32 LBA);		   //写扇区地址到硬盘
void HDDWriteSectorCount(u8 count);	 //要操作的扇区数

/*
用户函数
*/
u8 HDD_Init(void);  //硬盘初始化
void HDRunning(void); //运行硬盘
void HDStop(void);	  //停止运行硬盘
u8 ReadSector(u32 addr, u8 *SectorBuffer );	 //读取一个硬盘扇区（512字节）
u8 WriteSector(u32 addr, u8 *SectorBuffer);	 //写一个硬盘扇区（512字节）
u8 ReadnSector(u32 addr, u8 *SectorBuffer, u32 nsec );	 //读取n个硬盘扇区
u8 WritenSector(u32 addr, u8 *SectorBuffer, u32 nsec);  //写n个硬盘扇区

#endif

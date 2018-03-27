
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
#include <at89x52.h>
#include "ATAPI6.h"
#include "serial.h"
#include "znFAT/znfat.h"
#include "vs1003.h"
#include <intrins.h>

#define PLAY_SONG 0x01
#define PAUSE_SONG 0x02
#define IDLE        0x08
#define OPEN_SONG   0x10
#define CLOSE_HARD_DISK 0x40
#define OPEN_HARD_DISK 0x80

#define DEBUG 0

#define BUF_SIZE (192)
struct znFAT_Init_Args Init_Args; //初始化硬盘参数集合
struct FileInfo current_mp3_file; //文件信息集合

sbit IrIn=P3^2;

u32 current_mp3_file_index=0;
u8 com_command=0;
u8 lr_voice_value=32;
u32 read_left_bytes=0;
u8 mp3_buf[BUF_SIZE]={0};
u32 play_offset=0;
u8 play_state_machine=OPEN_SONG;

u8 IrVal=0;
void IrInit();
	   
void main(void)
{
	int res;
 
	IrInit();
	
    VS_Reset();
    VS_Sin_Test(200);//测试VS1003好坏
    VS_WRITE_Reg(SCI_BASS,0x00,0xf6);	 //设置为超重低音
	#if DEBUG==1
	UartInit();	//1200bps@24.000MHz
	#endif
REINIT:	 
	#if DEBUG==1
    SendStr("Now init HDD...\r\n\r\n");
	#endif
	if(0==znFAT_Device_Init()) //存储设备初始化
	{
		//硬盘初始化成功
		#if DEBUG==1
	 	SendStr("HDD Init is ok\r\n\r\n");
		#endif	
	}

	znFAT_Select_Device(0,&Init_Args); //选择设备0，也就是硬盘
	#if DEBUG==1	
	SendStr(" Select HDD \r\n\r\n");
	#endif
	res=znFAT_Init(); //文件系统初始化
	#if DEBUG==1
	SendStr(" znFAT init \r\n\r\n");
	#endif
	if(0==res)
	{
		//文件系统初始化成功
		#if DEBUG==1
	 	SendStr("znFAT Init is ok\r\n\r\n");
		#endif
	}else if(1==res)
	{
		#if DEBUG==1
	 	SendStr("znFAT Init is fail\r\n\r\n");
		#endif
		goto REINIT;
	} else if(2==res)
	{
 		#if DEBUG==1
 	 	SendStr("No found FAT32 \r\n\r\n");
		#endif
		goto REINIT;
	}	

	play_state_machine=OPEN_SONG; //开机打开第一首播放
	#if DEBUG==1
	SendStr("OPEN_SONG! \r\n\r\n");
	#endif
	for(;;){
		
	  switch(play_state_machine){
	  	case  IDLE:{
		   if(0x07==IrVal){	// 空闲状态可以 上一曲
		    IrVal=0;
	 	    if(current_mp3_file_index>1){
				current_mp3_file_index-=2; 
			}else if(current_mp3_file_index==1){
				current_mp3_file_index-=1;
			}else if(current_mp3_file_index==0){
				current_mp3_file_index=0;
			}
			 
			play_offset=0;//播放偏移清零
		 	play_state_machine=	 OPEN_SONG;//进入打开歌曲状态
		   }

		   if(0x45==IrVal){	 //打开硬盘		  ,空闲状态可以打开硬盘
			  play_state_machine= OPEN_HARD_DISK;//打开硬盘
		   }

		}break;

		case OPEN_SONG:{
			if(!znFAT_Open_File(&current_mp3_file,"/music/*.mp3",current_mp3_file_index,1)) {
				current_mp3_file_index++;//打开文件成功,文件索引才自加一  ,为下一曲做准备
				play_state_machine=	 PLAY_SONG;//打开成功进入播放状态
				#if DEBUG==1
				SendStr("Open mp3 file success!\r\n");
				#endif
			}	else{
				current_mp3_file_index	 =0;//播放到最后一首了,继续从第一首开始播放
				play_state_machine=	OPEN_SONG;//  继续从第一首开始播放
				#if DEBUG==1
				SendStr("Open mp3 file fail ,replay !\r\n");
				#endif
			}
		}break;

		case  PLAY_SONG:{

			 	if(read_left_bytes=znFAT_ReadData(&current_mp3_file,play_offset,BUF_SIZE,mp3_buf)){
				 	u32 i_count=0;
					for(i_count=0;i_count<read_left_bytes;i_count++){
						VS_Send_Dat(mp3_buf[i_count]);
					}
					play_offset+=  read_left_bytes;
					#if DEBUG==1
					SendStr("playing mp3 !\r\n");
					#endif
			 	}else{
					play_offset=0;//播放偏移清零
					play_state_machine=	 OPEN_SONG;//播放完了,自动切换下一曲
				}

			   if(0x40==IrVal){   //加音量
				  	IrVal=0;  
			  		if(lr_voice_value>0){
							lr_voice_value-=2;
					}else{
							lr_voice_value=0;
					}
					VS_WRITE_Reg(VOL_VALUE,lr_voice_value,lr_voice_value);
			   }

			   if(0x19==IrVal){ 	//减音量
				 IrVal=0; 
			  	 if(lr_voice_value<0xfe){
					 lr_voice_value+=2;
				 }
				 if(lr_voice_value>=0xfe) {
					lr_voice_value=0xfe;
				 }
				 VS_WRITE_Reg(VOL_VALUE,lr_voice_value,lr_voice_value);
			   }
			   if(0x07==IrVal){	//上一曲
			    IrVal=0;
		 	    if(current_mp3_file_index>1){
					current_mp3_file_index-=2; 
				}else if(current_mp3_file_index==1){
					current_mp3_file_index-=1;
				}else if(current_mp3_file_index==0){
					current_mp3_file_index=0;
				}
				znFAT_Close_File(&current_mp3_file); //关闭刚才打开的文件
				VS_Flush_Buffer(); //清空vs1003内部缓冲区
				play_offset=0;//播放偏移清零
			 	play_state_machine=	 OPEN_SONG;//进入打开歌曲状态
			   }	  
	
				 if(0x09==IrVal){  //下一曲
					 IrVal=0;  
				     play_offset=0;//播放偏移清零
			      	 play_state_machine=	 OPEN_SONG;//进入打开歌曲状态
					 VS_Flush_Buffer(); //清空vs1003内部缓冲区
					 znFAT_Close_File(&current_mp3_file);  //关闭刚才打开的文件
				 }
			   
			    if(0x15==IrVal){ //暂停
					IrVal=0;
					play_state_machine=	 PAUSE_SONG; //进入暂停状态

				}
		}break;

		case  PAUSE_SONG:{
			    if(0x15==IrVal){	 //播放	   ,暂停的时候可以进入播放状态
				   IrVal=0;
				   play_state_machine= PLAY_SONG;//进入播放状态
				}
				if(0x45==IrVal){	 //close硬盘  ,暂停的时候可以关闭硬盘
					IrVal=0;
			 		 play_state_machine= CLOSE_HARD_DISK;//关闭硬盘
		   		}
		}break;

 
		case  CLOSE_HARD_DISK:{
			  HDStop();
			  play_state_machine= IDLE;//关闭硬盘后,进入空闲态
		}break;

		case   OPEN_HARD_DISK:{
			  HDRunning();	
			  play_state_machine = PLAY_SONG; //打开硬盘后,继续播放当前歌曲
		}break;


		default:{}break;

	  }

	}
}

void usart() interrupt 4
{
	if(RI)
	{
		RI=0;
		com_command=SBUF;
		if(com_command=='R'){
			com_command=0;
			HDRunning();	
		}else if(com_command=='S')
		{
			com_command=0;
			HDStop();
		}
	}else{
		TI=0;
	}
}



void Delay140us()		//@24.000MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	i = 4;
	j = 64;
	do
	{
		while (--j);
	} while (--i);
}

void delay(unsigned int x)
{
	while(x)
	{
		Delay140us();
		x--;	
	}
}

void IrInit()
{
	IT0=1;//下降沿触发
	EX0=1;//打开中断0允许
	EA=1;	//打开总中断
	IrIn=1;//初始化端口
}

void ReadIr() interrupt 0
{	
	unsigned char j,k;
	unsigned int err,Time=0;
	unsigned char IrValue[4]={0};
	if(IrIn==0)		//确认是否真的接收到正确的信号
	{	 
		
		err=1000;				
	//当两个条件都为真是循环，如果有一个条件为假的时候跳出循环，免得程序出错的时
	//	侯，程序死在这里 
		while((IrIn==0)&&(err>0))	//等待前面9ms的低电平过去  		
		{			
			delay(1);
			err--;
		} 
		if(IrIn==1)			//如果正确等到9ms低电平
		{
			err=500;
			while((IrIn==1)&&(err>0))		 //等待4.5ms的起始高电平过去
			{
				delay(1);
				err--;
			}
			for(k=0;k<4;k++)		//共有4组数据
			{				
				for(j=0;j<8;j++)	//接收一组数据
				{

					err=60;		
					while((IrIn==0)&&(err>0))//等待信号前面的560us低电平过去
					{
						delay(1);  //140us
						err--;
					}
					err=500;
					while((IrIn==1)&&(err>0))	 //计算高电平的时间长度。
					{
						delay(1);//140us
						Time++;
						err--;
						if(Time>30)	 
						{
							EX0=1;
							return;
						}
					}
					IrValue[k]>>=1;	 //k表示第几组数据
					if(Time>=8)			//如果高电平出现大于565us，那么是1
					{
						IrValue[k]|=0x80;
					}
					Time=0;		//用完时间要重新赋值							
				}
			}
		}
		if(IrValue[2]!=~IrValue[3])//如果数据的反码取反后不等于数据码，说明接收数据错误
		{
			return;
		}else{
			IrVal=	IrValue[2];
		}
	}
}

 


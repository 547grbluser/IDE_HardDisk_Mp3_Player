
/***********************************
���뻷����keil c51				   *
�������֣�IDEӲ��������д����	   *
�汾��v1.02						   *
���ڣ�2015/09/18				   *
���ߣ������� 					   *
QQ:547336083					   *
QQȺ:284285453                     *
����оƬ��STC12C5A60S2			   *
����Ƶ�ʣ�24.00Mhz			       *
����Ӳ�̣�����ATAPI-6Э���IDEӲ�� *
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
struct znFAT_Init_Args Init_Args; //��ʼ��Ӳ�̲�������
struct FileInfo current_mp3_file; //�ļ���Ϣ����

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
    VS_Sin_Test(200);//����VS1003�û�
    VS_WRITE_Reg(SCI_BASS,0x00,0xf6);	 //����Ϊ���ص���
	#if DEBUG==1
	UartInit();	//1200bps@24.000MHz
	#endif
REINIT:	 
	#if DEBUG==1
    SendStr("Now init HDD...\r\n\r\n");
	#endif
	if(0==znFAT_Device_Init()) //�洢�豸��ʼ��
	{
		//Ӳ�̳�ʼ���ɹ�
		#if DEBUG==1
	 	SendStr("HDD Init is ok\r\n\r\n");
		#endif	
	}

	znFAT_Select_Device(0,&Init_Args); //ѡ���豸0��Ҳ����Ӳ��
	#if DEBUG==1	
	SendStr(" Select HDD \r\n\r\n");
	#endif
	res=znFAT_Init(); //�ļ�ϵͳ��ʼ��
	#if DEBUG==1
	SendStr(" znFAT init \r\n\r\n");
	#endif
	if(0==res)
	{
		//�ļ�ϵͳ��ʼ���ɹ�
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

	play_state_machine=OPEN_SONG; //�����򿪵�һ�ײ���
	#if DEBUG==1
	SendStr("OPEN_SONG! \r\n\r\n");
	#endif
	for(;;){
		
	  switch(play_state_machine){
	  	case  IDLE:{
		   if(0x07==IrVal){	// ����״̬���� ��һ��
		    IrVal=0;
	 	    if(current_mp3_file_index>1){
				current_mp3_file_index-=2; 
			}else if(current_mp3_file_index==1){
				current_mp3_file_index-=1;
			}else if(current_mp3_file_index==0){
				current_mp3_file_index=0;
			}
			 
			play_offset=0;//����ƫ������
		 	play_state_machine=	 OPEN_SONG;//����򿪸���״̬
		   }

		   if(0x45==IrVal){	 //��Ӳ��		  ,����״̬���Դ�Ӳ��
			  play_state_machine= OPEN_HARD_DISK;//��Ӳ��
		   }

		}break;

		case OPEN_SONG:{
			if(!znFAT_Open_File(&current_mp3_file,"/music/*.mp3",current_mp3_file_index,1)) {
				current_mp3_file_index++;//���ļ��ɹ�,�ļ��������Լ�һ  ,Ϊ��һ����׼��
				play_state_machine=	 PLAY_SONG;//�򿪳ɹ����벥��״̬
				#if DEBUG==1
				SendStr("Open mp3 file success!\r\n");
				#endif
			}	else{
				current_mp3_file_index	 =0;//���ŵ����һ����,�����ӵ�һ�׿�ʼ����
				play_state_machine=	OPEN_SONG;//  �����ӵ�һ�׿�ʼ����
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
					play_offset=0;//����ƫ������
					play_state_machine=	 OPEN_SONG;//��������,�Զ��л���һ��
				}

			   if(0x40==IrVal){   //������
				  	IrVal=0;  
			  		if(lr_voice_value>0){
							lr_voice_value-=2;
					}else{
							lr_voice_value=0;
					}
					VS_WRITE_Reg(VOL_VALUE,lr_voice_value,lr_voice_value);
			   }

			   if(0x19==IrVal){ 	//������
				 IrVal=0; 
			  	 if(lr_voice_value<0xfe){
					 lr_voice_value+=2;
				 }
				 if(lr_voice_value>=0xfe) {
					lr_voice_value=0xfe;
				 }
				 VS_WRITE_Reg(VOL_VALUE,lr_voice_value,lr_voice_value);
			   }
			   if(0x07==IrVal){	//��һ��
			    IrVal=0;
		 	    if(current_mp3_file_index>1){
					current_mp3_file_index-=2; 
				}else if(current_mp3_file_index==1){
					current_mp3_file_index-=1;
				}else if(current_mp3_file_index==0){
					current_mp3_file_index=0;
				}
				znFAT_Close_File(&current_mp3_file); //�رողŴ򿪵��ļ�
				VS_Flush_Buffer(); //���vs1003�ڲ�������
				play_offset=0;//����ƫ������
			 	play_state_machine=	 OPEN_SONG;//����򿪸���״̬
			   }	  
	
				 if(0x09==IrVal){  //��һ��
					 IrVal=0;  
				     play_offset=0;//����ƫ������
			      	 play_state_machine=	 OPEN_SONG;//����򿪸���״̬
					 VS_Flush_Buffer(); //���vs1003�ڲ�������
					 znFAT_Close_File(&current_mp3_file);  //�رողŴ򿪵��ļ�
				 }
			   
			    if(0x15==IrVal){ //��ͣ
					IrVal=0;
					play_state_machine=	 PAUSE_SONG; //������ͣ״̬

				}
		}break;

		case  PAUSE_SONG:{
			    if(0x15==IrVal){	 //����	   ,��ͣ��ʱ����Խ��벥��״̬
				   IrVal=0;
				   play_state_machine= PLAY_SONG;//���벥��״̬
				}
				if(0x45==IrVal){	 //closeӲ��  ,��ͣ��ʱ����Թر�Ӳ��
					IrVal=0;
			 		 play_state_machine= CLOSE_HARD_DISK;//�ر�Ӳ��
		   		}
		}break;

 
		case  CLOSE_HARD_DISK:{
			  HDStop();
			  play_state_machine= IDLE;//�ر�Ӳ�̺�,�������̬
		}break;

		case   OPEN_HARD_DISK:{
			  HDRunning();	
			  play_state_machine = PLAY_SONG; //��Ӳ�̺�,�������ŵ�ǰ����
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
	IT0=1;//�½��ش���
	EX0=1;//���ж�0����
	EA=1;	//�����ж�
	IrIn=1;//��ʼ���˿�
}

void ReadIr() interrupt 0
{	
	unsigned char j,k;
	unsigned int err,Time=0;
	unsigned char IrValue[4]={0};
	if(IrIn==0)		//ȷ���Ƿ���Ľ��յ���ȷ���ź�
	{	 
		
		err=1000;				
	//������������Ϊ����ѭ���������һ������Ϊ�ٵ�ʱ������ѭ������ó�������ʱ
	//	������������� 
		while((IrIn==0)&&(err>0))	//�ȴ�ǰ��9ms�ĵ͵�ƽ��ȥ  		
		{			
			delay(1);
			err--;
		} 
		if(IrIn==1)			//�����ȷ�ȵ�9ms�͵�ƽ
		{
			err=500;
			while((IrIn==1)&&(err>0))		 //�ȴ�4.5ms����ʼ�ߵ�ƽ��ȥ
			{
				delay(1);
				err--;
			}
			for(k=0;k<4;k++)		//����4������
			{				
				for(j=0;j<8;j++)	//����һ������
				{

					err=60;		
					while((IrIn==0)&&(err>0))//�ȴ��ź�ǰ���560us�͵�ƽ��ȥ
					{
						delay(1);  //140us
						err--;
					}
					err=500;
					while((IrIn==1)&&(err>0))	 //����ߵ�ƽ��ʱ�䳤�ȡ�
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
					IrValue[k]>>=1;	 //k��ʾ�ڼ�������
					if(Time>=8)			//����ߵ�ƽ���ִ���565us����ô��1
					{
						IrValue[k]|=0x80;
					}
					Time=0;		//����ʱ��Ҫ���¸�ֵ							
				}
			}
		}
		if(IrValue[2]!=~IrValue[3])//������ݵķ���ȡ���󲻵��������룬˵���������ݴ���
		{
			return;
		}else{
			IrVal=	IrValue[2];
		}
	}
}

 


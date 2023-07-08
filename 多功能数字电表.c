#include <reg51.h> 
//#include <STC12C5A60S2.h> 
#define uchar unsigned char 
#define uint  unsigned int 
#define ulong unsigned long 
uchar code acLEDCS[] = {0xef, 0xdf, 0xbf, 0x7f};              /* λѡ��ַ�룬�͵�ƽ��Ч������P2�ڵĸ���λ*/
uchar code acLEDCS1[] = {0xe7, 0xd7, 0xb7, 0x77};
uchar code acLedSegCode[]={0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c}; /* ���� */
uchar acLED[4];	      /* ��ʾ������ */
char cScanIndex;      /* λѡָ�� 0~3 */
uchar cKey;	          /* ��ʼ��ֵ	*/
uchar cKeyCode; 	  /* ��ֵ */
uint nDelayKey;       /*������ʱ������Ϊ��ʱ�жϼ��ʱ���������*/ 
uchar cLongDelay;     /* ��������ʱ��*/

bit bStill;           /*�Ƿ��ɼ���־*/ 
char cMode = 2;       /*��ʾ��ʽ������0~3��Ӧ4�ַ�ʽ*/
char cSetMode = 0;        /*����ģʽ������0~10��Ӧ11�ַ�ʽ*/
uint nTimer1 = 0;         /*��ʱ����������Ϊ��ʱ�жϼ��ʱ���������*/
uint nTimer = 0;          /*��ʱ����������Ϊ��ʱ�жϼ��ʱ���������*/ 
/*********���壬�͵�ƽ��Ч**************/
uchar cTimeMin = 0;    /*���嶨ʱʱ�����:����*/
uchar cTimeHour = 0;	/*���嶨ʱʱ�������Сʱ*/
uchar cTimeflag = 0;
sbit bell = P2^3;
int flag = 0;

 /************* ��� *************/

uint nStopflag = 0;	/*��������ʼ��־��Ϊ1��Ч*/
uint nStop = 0;	/*��������Ϊ�жϵ���������100uS*/
uint nStopSec = 0; /*������*/
uint nStopTenms = 0; /* ��������*/

uint ncountSec = 30; /*����ʱ����*/
uint ncountTenms = 0; /* ����ʱ������*/
uint ncountflag = 0;	/*����ʱ ��ʼ��־��Ϊ1��Ч*/
uint ncount = 0;	/*����ʱ ������Ϊ�жϵ���������100uS*/

/*ͨ��λѰַ�Ĵ���*/

uchar bdata Reg = 0;
sbit bReg0 = Reg^0;	/* ������λ֮��*/
sbit bReg7 = Reg^7;	/* ������λ֮��*/
/************ DS 1302 �������� ************* */ 
sbit DS_CLK = P1^4; 
sbit DS_IO = P1^5; 
sbit DS_RST = P1^6;


typedef struct StrClock
{
	uchar Sec;	/* �� */
	uchar Min;	/* �� */
	uchar Hour;	/* ʱ */
	uchar Day;	/* �� */
	uchar Mon;	/* �� */
	uchar Week;	/*����*/
	uchar Year;	/* �� */
	uchar Ctr;	/*������*/
};
union UniClock	/*ʱ��������1*/
{
	struct StrClock	sClock;
	uchar Time[8];	
}idata uClock;	/*ʱ��*/



typedef struct StrTime /* ʱ�ӽṹ�� 2 */
{
uchar	Sec;	/* �� */
uchar	Min;	/* �� */
uchar	Hour;	/* ʱ */
uchar	Day;	/* �� */
uchar	Mon;	/* �� */
uchar	Year;	/* �� */
};		
union UniTime	/*ʱ��������2 */
{		
	struct StrTime sTime;
	uchar Time[6];
} idata uTime;	/* ʱ�� */ 

/* 16������ת��ΪBCD��*/

 uchar H_BCD(uchar dat)
{
	uchar datl, dat2;
	datl = dat / 10;
	dat2 = dat % 10 + (datl<<4);
	return(dat2);
}
void W_DS1302(uchar adr, uchar dat);/* дһ�ֽ�(dat)��ָ����ַ(adr)*/
uchar R_DS1302(uchar adr);
/***************************************************/
 /***** DS 1302 �������� *****/ 
/*��ʼ��*/
void InitDS1302()
{
	uchar dat;

	W_DS1302(0x8e, 0);		 	/* ��������,��ֹд���� */
	W_DS1302(0x90, 0xa5);		/* 2K����,һ�������� */
	dat=R_DS1302(0x81);			/* �����ֽ� */
	if(dat>127)
	{
		dat=0;
		W_DS1302(0x80, dat);	/* ����ʱ�� */
	}	
	W_DS1302(0x8e,0x80);	 	/* ��������,ʹ��д���� */
}

/* дһ�ֽ� */
void W_DS1302Byte(uchar dat)
{
	uchar i=8;
	Reg=dat;
	while(i--)
	{
		DS_CLK=0;
		DS_IO=bReg0;
		DS_CLK=1;
		Reg >>=1; 
	} 
}
/*��һ�ֽ�*/
uchar R_DS1302Byte()
{
		uchar i = 8;
		while(i--)
		{
			DS_CLK = 0;
			Reg >>= 1;
			bReg7 = DS_IO;
			DS_CLK = 1;
		}
		return(Reg);
}
/*дһ�ֽ�(dat)��ָ����ַ(adr) */
void W_DS1302(uchar adr, uchar dat)
{
	DS_CLK = 0;
	DS_RST = 0;
	DS_RST = 1;
	W_DS1302Byte(adr);	/* ��ַ������ */
	W_DS1302Byte(dat);	/* д IByte ����*/
	DS_RST = 0;
	DS_CLK = 0;
}
/*��һ�ֽ�ָ����ַ(adr)������*/
uchar R_DS1302(uchar adr)
{
	uchar dat;
	DS_CLK = 0;
	DS_RST = 1;
	W_DS1302Byte(adr);	/* ��ַ������ */
	dat=R_DS1302Byte();	/* �� IByte ���� */
	DS_RST = 0;
	DS_CLK = 0; 
	return(dat);
}
/***************** ��ʱ������ ******************/
	
	/*��ʽΪ�����ʱ�������������*/
void R_DS1302Timer()
	{
		uchar i;
		DS_CLK = 0;
		DS_RST = 1;
		W_DS1302Byte(0xbf); /* Oxbf:ʱ�Ӷ��ֽڶ����� */ 
		for(i = 0; i < 8; i++)
		uClock.Time[i] = R_DS1302Byte();
		DS_RST = 0;
		DS_CLK = 0;
	}
/*************** �������ʾ���� ****************** /*��(I 0x80)��Ϊ�˵���������ܵ�С����*/
void display()
{
	{
		if(cSetMode > 0 )
		{
			switch(cMode)
			{

			case 0:  //��ʾ������
				acLED[0] = acLedSegCode[2];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[uTime.sTime.Year /10];
				acLED[3] = acLedSegCode[uTime.sTime.Year %10];
				break;
			case 1: //��ʾ������/������
				acLED[0] = acLedSegCode[uTime.sTime.Mon /10];
				acLED[1] = acLedSegCode[uTime.sTime.Mon %10] | 0x80;
				acLED[2] = acLedSegCode[uTime. sTime. Day / 10];
				acLED[3] = acLedSegCode[uTime. sTime. Day % 10];
				break;
			case 2:   //��ʾ����ʱ/���÷�
				if (cSetMode == 1 || cSetMode == 2)
				{
					acLED[0] = acLedSegCode[uTime.sTime.Hour /10];
					acLED[1] = acLedSegCode[uTime.sTime.Hour %10] | 0x80;
					acLED[2] = acLedSegCode[uTime. sTime. Min / 10];
					acLED[3] = acLedSegCode[uTime. sTime. Min % 10];
					break;
				}
				else if (cSetMode == 3 || cSetMode == 4)
				{
					acLED[0] = acLedSegCode[cTimeHour /10];
					acLED[1] = acLedSegCode[cTimeHour %10] | 0x80;
					acLED[2] = acLedSegCode[cTimeMin / 10];
					acLED[3] = acLedSegCode[cTimeMin % 10];
					break;
				}
			case 3: //��ʾ������
				acLED[0] = 0;
				acLED[1] = 0 | 0x80;
				acLED[2] = acLedSegCode[uTime.sTime.Sec /10];
				acLED[3] = acLedSegCode[uTime.sTime.Sec %10];
				break;
			case 6: 
				acLED[0] = acLedSegCode[ncountSec / 10];
				acLED[1] = acLedSegCode[ncountSec % 10] | 0x80;
				acLED[2] = acLedSegCode[ncountTenms / 10];
				acLED[3] = acLedSegCode[ncountTenms % 10];
				break;
			case 7: // ��ʾ
				acLED[0] = acLedSegCode[nStopSec / 10];
				acLED[1] = acLedSegCode[nStopSec % 10] | 0x80;
				acLED[2] = acLedSegCode[nStopTenms / 10] ;
				acLED[3] = acLedSegCode[nStopTenms % 10];
				break;
			default:
				break;
			}
		}
		  else
		{
			switch(cMode)
			{

			case 0:  //��ʾ��
				acLED[0] = acLedSegCode[2];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[uClock.Time[6] >>4];
				acLED[3] = acLedSegCode[uClock.Time[6] & 0x0f];
				break;
			case 1: //��ʾ��/��
				acLED[0] = acLedSegCode[uClock.Time[4]>>4];
				acLED[1] = acLedSegCode[uClock.Time[4] & 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[3] >>4];
				acLED[3] = acLedSegCode[uClock.Time[3] & 0x0f];
				break;
			case 2:   //��ʾʱ/��
				
				acLED[0] = acLedSegCode[uClock.Time[2]>>4];
				acLED[1] = acLedSegCode[uClock.Time[2] & 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[1] >>4];
				acLED[3] = acLedSegCode[uClock.Time[1]& 0x0f];
				break;
			case 3: //��ʾ�
				acLED[0] = acLedSegCode[uClock.Time[1] >>4];
				acLED[1] = acLedSegCode[uClock.Time[1]& 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[0] >>4];
				acLED[3] = acLedSegCode[uClock.Time[0] & 0x0f];
				break;
			case 4:
				acLED[0] = acLedSegCode[0];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[0];
				acLED[3] = acLedSegCode[0];
				break;
			default:
				break;
			
			}
		}
	 }
}
/******************** ����������� ******************/
void DisposeKEY()
{
	switch(cKeyCode)
	{
	case 2:
		if(bStill == 0)
			{
				cMode++;
			if(cMode >= 5)
			{
				cMode = 0;
			}
				bStill = 1;
			}
			break;
	case 3:	
		/*******���� DS 1302 ��ʱ��*******/
	if (cMode == 0 && cSetMode == 1)/*�޸�����*/ 
	{
		if(uTime.sTime.Year < 99)
		{
			uTime.sTime.Year++;
			nDelayKey = 2000;	/*��ס��������������������ֵ���൱����������*/
		}
		W_DS1302(0x8e, 0);	/*���������ֹд����*/
		W_DS1302(0x8c, H_BCD(uTime.sTime.Year));
		W_DS1302(0x8e, 0x80);	/*�������ʹ��д����*/
		break;
	}
	else if(cMode == 1 && cSetMode == 1)/*�޸�����*/
	{
		if (uTime.sTime.Mon < 12)
		{
			uTime.sTime.Mon++; 
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x88, H_BCD(uTime.sTime.Mon));
		W_DS1302(0x8e, 0x80);
		break;
	}
	else if(cMode == 1 && cSetMode == 2) /*�޸�����*/
	{
		if(uTime.sTime.Day < 31)
		{
			uTime.sTime.Day++; 
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x86, H_BCD(uTime.sTime.Day));
		W_DS1302(0x8e, 0x80);
		break;
	}
	
	else if(cMode == 2 && cSetMode == 1) /*�޸�Сʱ��*/ 
	{
		if(uTime.sTime.Hour < 24)
		{
			uTime.sTime.Hour++;
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x84, H_BCD(uTime.sTime.Hour));
		W_DS1302(0x8e, 0x80);
		break;
	}
	else if(cMode == 2 && cSetMode == 2) /*�޸ķ�����*/
	{
		if(uTime.sTime.Min < 59)
	 	{
			uTime.sTime.Min++; 
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x82, H_BCD(uTime.sTime.Min));
		W_DS1302(0x8e, 0x80);
		break;
	}
	/********��������ʱ��*******/
	else if(cMode == 2 && cSetMode == 3)/*��������Сʱ��*/
	{
		if(cTimeHour < 24)
			{
				cTimeHour++; 
				nDelayKey = 2000;
			}
			break;
	}
	else if(cMode == 2 && cSetMode == 4)/*�������������*/
	{
		if(cTimeMin < 59)
			{
				cTimeMin++; 
				nDelayKey = 2000;
			}
			break;
	}


	else if (cMode == 3 && (cSetMode == 1)) /*�޸� ����*/
	{
		if (uTime.sTime.Sec < 59)
		{
			uTime.sTime.Sec++;
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x80, H_BCD(uTime.sTime.Sec));
		W_DS1302(0x8e, 0x80);
		break;
	}
	else if (cMode == 6 && cSetMode == 3) /*�޸� ����ʱ�� */
	{
		if( ncountSec < 91)
		{
			ncountSec++;
			nDelayKey = 2000;
		}
		ncountTenms = 0;
		if (ncountSec == 91)
			ncountSec = 0;
		break;
	}
	
	else
		{
			break;
		}
	case 6:	/*���ܣ�KeySUB */
/********���� DS1302 ��ʱ��*********/
		if (cMode == 0 && cSetMode == 1 )/*�޸�����*/ 
		{
			if(uTime.sTime.Year > 0)
			{
				uTime.sTime.Year--;
				nDelayKey = 1000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x8c, H_BCD(uTime.sTime.Year));
			W_DS1302(0x8e, 0x80);
			nDelayKey = 2000;
			break;
		}
		else if(cMode == 1 && cSetMode == 1)/*�޸�����*/
		{
			if(uTime.sTime.Mon > 0)
			{
				uTime. sTime.Mon--;
				nDelayKey = 2000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x88, H_BCD(uTime. sTime. Mon));
			W_DS1302(0x8e, 0x80);
			break;
		}
		else if (cMode == 1 && cSetMode == 2)/*�޸�����*/
		{
			if(uTime. sTime. Day > 0)
			{
				uTime.sTime.Day--;
				nDelayKey = 2000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x86, H_BCD(uTime.sTime.Day));
			W_DS1302(0x8e, 0x80);
			break;
		}
		else if(cMode == 2 && cSetMode == 1)/*�޸�Сʱ��*/ 
		{
			if(uTime.sTime.Hour > 0)
			{
				uTime.sTime.Hour--;
				nDelayKey = 1000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x84, H_BCD(uTime.sTime.Hour));
			W_DS1302(0x8e, 0x80);
			break;
		}
		else if(cMode == 2 && cSetMode == 2)/*�޸ķ�����*/
		{
			if(uTime.sTime.Min > 0)
			{
				uTime.sTime.Min--;
				nDelayKey = 2000;
			}
		W_DS1302(0x8e, 0);
		W_DS1302(0x82, H_BCD(uTime.sTime.Min));
		W_DS1302(0x8e, 0x80);
		break;
		}
		else if (cMode == 3 && (cSetMode == 1 )) /*�޸� ����*/
		{
			if(uTime.sTime.Sec > 0)
			{
				uTime.sTime.Sec--;
				nDelayKey = 2000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x80, H_BCD(uTime.sTime.Sec));
			W_DS1302(0x8e, 0x80);
			break;
		}
/********��������ʱ��********/
		else if (cMode == 2 && cSetMode == 3)/*��������Сʱ��*/
		{
			if(cTimeHour > 0)
			{
				cTimeHour--; 
				nDelayKey = 2000;
			}
			break;
		}

		else if(cMode == 2 && cSetMode == 4)/*�������������*/
		{
			if(cTimeMin > 0)
			{
				cTimeMin --; 
				nDelayKey = 2000;
			}
			break;
		}
		else if (cMode == 6 && cSetMode == 3) /*�޸� ����ʱ�� */
		{
			if(ncountSec > 0)
			{
				ncountSec--;
				nDelayKey = 2000;
			}
			ncountTenms = 0;
			if (ncountSec == 0)
			ncountSec = 90;			
			break;
		}
		else
		{
			break;
		}
	case 5:	
			
			if(bStill == 0)
			{
				cSetMode++;
				bStill = 1;
			}
			if(cMode == 0 && cSetMode == 2)
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 1 && cSetMode == 3) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 2 && cSetMode == 5) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 2 && cSetMode == 1&& cTimeflag == 1) 
		{
			cTimeHour = 24;
			cTimeMin = 60;
			cTimeflag =0;
			break;
		}
		
		
		
		else if(cMode == 3 && cSetMode == 2) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 6 && cSetMode == 6) 
		{
			cSetMode = 0;
			cMode =4;
			ncountTenms = 0;
			ncountSec = 30;
			nStopTenms = 0;
			nStopSec = 0;
			
			break;
		}
		else if(cMode == 3 && cSetMode == 2) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 7 && cSetMode == 3)
		{
			ncountflag = 1;
			break;
		}
		else if(cMode == 6 && cSetMode == 5)
		{
			ncountflag = 0;
			break;
		}
		else if(cMode == 4 && cSetMode == 1)
		{
			nStopflag = 1;
			break;
		}
		else if(cMode == 7 && cSetMode == 2)
		{
			nStopflag = 0;
			break;
		}
		else
		{
			break;
		}
	}
	
		display ();
		cKeyCode = 0;
}



void Alarm()
{
	if (cTimeHour - uClock.Time[2]  == 0&& cTimeMin - uClock.Time[1]  == 0&& cSetMode == 0 )
	{	
		cTimeflag = 1;	/*��������*/
		bell = 0;
	}
	else	/*����������*/
		bell = 1;
		cTimeflag = 0;	/*����������*/
		
}
/****************** ��� �� *******************/ 
void StopWatch()
{
	if(nStopflag == 1)
	{
		cMode = 7;
		if(nStop >= 100)
		{
			nStop = 0; 
			nStopTenms++;
			if(nStopTenms >= 100) /*100*10ms=ls*/
			{
				nStopTenms = 0;
				if(nStopSec < 99)
					nStopSec++; /*������һ*/
				else	/*����Ϊ99.99s*/
				{
					nStopSec = 99;
					nStopTenms = 99;
					nStopflag = 0;
				}
			}
		}
	}
	
	display();
}
void CountDown(void)

{
	if (ncountflag == 1)
	{
		cMode= 6;
	}
	if(cMode == 6 && cSetMode == 4)/*��ʼ����ʱ*/

	{
		if(ncount >= 100)
		{
			ncount = 0;
			if (ncountTenms != 0)
			{
			ncountTenms--;
			}
			else if(ncountTenms==0 && ncountSec != 0)
			{
				ncountTenms =99;
				ncountSec--;
			}
			
			else
			{
				ncountTenms =0;
				ncountSec = 0; 
				ncountflag = 0;
			}
		}
	}
}
/************************** ������ *****************************/
void main(void)
/**************** ��ʱ����ʼ�� *******************/
{
//	AUXR &= 0x7F;	
	TMOD &= 0xF0;		
	TMOD |= 0x01;		
	TL0 = 0x48;		
	TH0 = 0xFF;	
	TF0 = 0;		
	TR0 = 1;		
	ET0 = 1;
	EA = 1;        /*�����ж�*/
//	P0M0 = 0xff; /*����P0 ��Ϊǿ����ģʽ*/
//	P0M1 = 0x00; /*POM1 = 00000000B*/
	InitDS1302();  /* ��ʼ�� DS1302 */
	R_DS1302Timer();  /*��ʱ������()*/
	display();  /*��ʾ��ʼ����*/
	while(1)



	{
		if(cKeyCode != 0) 
		{
			DisposeKEY(); /*��Ӧ��������*/
		}
		R_DS1302Timer();      /*��ʱ������*/
		display() ;  /*��ʾ����*/
		Alarm();     /*���庯��*/
		StopWatch();  /*�����*/
		CountDown();  /*����ʱ����*/
	}
}
/**************** ������ʱ�����ʱ���ж� *****************/
void IntT0() interrupt 1
{
	TL0 = 0x48;		
	TH0 = 0xFF;	
	nTimer1++;
	
	if(ncountflag == 1)
	{		
			ncount++;	/*��������Ϊ�жϵ���������100us*/
	}
	if(nStopflag == 1)
	{		
			nStop++;	/*��������Ϊ�жϵ���������100us*/
	}
	 if (cTimeflag == 1)
	{
					P0 = 0;
					P2 = acLEDCS1[cScanIndex];
					P0 = acLED[cScanIndex++]; 
					cScanIndex &= 3;
	}
	else
		{
		if (cSetMode > 0 && cSetMode < 8)	/*�˴�������˸ʱ������ʱ�估���塣��˸��ͨ������ܼ�Ъ����ʾ��Чʵ�ֵ�*/
		{	
			if(nTimer1 <= 2500 )
			{
				if (cMode == 0 && cSetMode == 1 ) /*���� ��ʱ���������˸*/
															  /*�Ȝ[��ʾ�ٻ�λѡ*/
				{
					P0 = 0;
					P2 = acLEDCS[cScanIndex];
					cScanIndex++;
					P0 = 0;
					cScanIndex &= 3;
				}
					else if(cSetMode == 1 && cMode == 1)
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						cScanIndex++;
						if(cScanIndex >= 1 && cScanIndex <= 2)
							P0 = 0;
						else
						{
							P0 = acLED[cScanIndex - 1];
							cScanIndex &= 3;
						}
					}
					else if(cSetMode == 2 && cMode == 1) /*��ʱ������λ�������˸*/
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;	
							cScanIndex &= 4;	/*λѡָ���λ*/
						}
						else	
							P0 = acLED[cScanIndex - 1];	/*����ʾ����,λѡָ����λ*/	
					}	  	
					else if(cSetMode == 1 && cMode == 2)	/*����Сʱ��ǰ��λ�������˸*/
		
					{
						P0 = 0;	/*�Ȝ[��ʾ�ٻ�λѡ*/	
						P2 = acLEDCS[cScanIndex];	/*��λѡ����*/	
						cScanIndex++;	
						if(cScanIndex >= 1 && cScanIndex <= 2)	
							P0 = 0;	
						else
						{
							P0 = acLED[cScanIndex - 1];	/*����ʾ����,λѡָ����λ*/	
							cScanIndex &= 3;	/*λѡָ���λ*/
						}
					}

					else if(cSetMode == 2 && cMode == 2)	/*���÷���ʱ������λ�������˸*/
					{	
						P0 = 0;/*�Ȝ[��ʾ�ٻ�λѡ*/
						P2 = acLEDCS[cScanIndex] ;	/* ��λѡ����*/
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;
							cScanIndex &= 4;	/* λѡָ���λ*/
						}
						else
							P0 = acLED[cScanIndex - 1];	/* ����ʾ����,λѡָ����λ*/
					}
					else if (cMode == 3 && (cSetMode == 1 || cSetMode == 2))/*������ʱ���������˸*/

					{
						P0 = 0;	/*������ʾ�ٻ�λѡ*/	
						P2 = acLEDCS[cScanIndex];	/*��λѡ����*/		
						cScanIndex++;	
						P0 = 0;	
						cScanIndex &= 3;	/*λѡָ���λ*/		
					}
					else if(cSetMode == 3 && cMode == 2)	/*��������Сʱʱ��	ǰ��λ�������˸*/
					{
						P0 = 0;	/*�Ȝ[��ʾ�ٻ�λѡ*/	
						P2 = acLEDCS[cScanIndex];	/*��λѡ����*/

						cScanIndex++;
						if(cScanIndex >= 1 && cScanIndex <= 2)
							P0 = 0;
					
						else
						{
							P0 = acLED[cScanIndex -1];  
							cScanIndex &= 3;
						}
					}
					else if(cSetMode == 4 && cMode == 2)
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;
							cScanIndex &= 4;
						}
						else
							P0 = acLED[cScanIndex - 1];
					}
					else if(cMode == 6 && cSetMode == 3)
					{
						P0 = 0;	/*�Ȝ[��ʾ�ٻ�λѡ*/	
						P2 = acLEDCS[cScanIndex];	/*��λѡ����*/	
						cScanIndex++;	
						if(cScanIndex >= 1 && cScanIndex <= 2)	
							P0 = 0;	
						else
						{
							P0 = acLED[cScanIndex - 1];	/*����ʾ����,λѡָ����λ*/	
							cScanIndex &= 3;	/*λѡָ���λ*/
						}
					}
					else /*����˸*/
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						P0 = acLED[cScanIndex++]; 
						cScanIndex &= 3;
					}
				}

				else

				{
					if(nTimer1 >= 5000 )
						nTimer1 = 0;
					P0 = 0;
					P2 = acLEDCS[cScanIndex];
					P0 = acLED[cScanIndex++];
					cScanIndex &= 3;
				}
			}
			else   
			{
				P0 = 0;
				P2 = acLEDCS[cScanIndex];
				P0 = acLED[cScanIndex++];
				cScanIndex &= 3;
			}
	}
/*********************ɨ �谴��********************/
		if(nDelayKey == 0)
		{
			cKey = P2 & 0x07;  /* ȡ��ֵ P20��P21��P22*/
			if(cKey != 0x07) 
				nDelayKey = 100;	   /*��������ʱ������*/
			else
			{

				bStill = 0;
				cLongDelay = 0;/*�ɼ�*//*�а�������DelayKey��������*/
			}
		}
		else
		{
			nDelayKey--; 
			if(nDelayKey == 0)
			{
				cKeyCode = P2 & 0x07; 
				if(cKey != cKeyCode)
				{
					cKeyCode = 0;
				}
			}
		}
}
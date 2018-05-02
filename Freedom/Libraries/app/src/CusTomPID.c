/****************************************Copyright (c)****************************************************
** File name:           PID.c
** Last modified Date:  2012-05-05
** Last Version:        V1.0
** Descriptions:        PID�����㷨
*********************************************************************************************************/

#include "CusTomPID.h" 
#include "UartLink.h"

typedef struct PID{
    
    int   Point;                                                        /* Ŀ��ֵ                       */
    long  SumError;                                                     /* �ۼ����                     */
    
    float P;                                                            /* ����ϵ��                     */
    float I;                                                            /* ����ϵ��                     */
    float D;                                                            /* ΢��ϵ��                     */
    
    int   LastError;                                                    /* Error[-1]                    */
    int   PrevError;                                                    /* Error[-2]                    */
                                                                        
}PID;

PID __PID;
PID *GoPID = &__PID;

/*********************************************************************************************************
** Function name:       PID_Init
** Descriptions:        PID���в�������
** input parameters:    *LPID:PID�ṹ��ָ��                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
void PID_Init(void) 
{
    GoPID->Point     = 0;

    GoPID->P         = 0;
    GoPID->I         = 0;
    GoPID->D         = 0;
    
    GoPID->SumError  = 0;    
    GoPID->LastError = 0;
    GoPID->PrevError = 0;
}

/*********************************************************************************************************
** Function name:       PID_SetPoint
** Descriptions:        ����PID���ڵ�Ŀ��ֵ
** input parameters:    AimValue:Ŀ��ֵ                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
void PID_SetPointGo(int AimValue) 
{
    GoPID->Point = AimValue;
}

/*********************************************************************************************************
** Function name:       PID_SetKp
** Descriptions:        ����Pֵ
** input parameters:    Kp:                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
void PID_SetKpGo(float Kp) 
{
    GoPID->P     = Kp; 
}
/*********************************************************************************************************
** Function name:       PID_SetKi
** Descriptions:        ����Iֵ
** input parameters:    Ki:                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
void PID_SetKiGo(float Ki) 
{
    GoPID->I     = Ki;
}
/*********************************************************************************************************
** Function name:       PID_SetKd
** Descriptions:        ����Dֵ
** input parameters:    Kd:                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
void PID_SetKdGo(float Kd) 
{
    GoPID->D     = Kd;
}
/*********************************************************************************************************
** Function name:       PID_SetKpid
** Descriptions:        ����Dֵ
** input parameters:    Kd:                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
void PID_SetKpidGo(float Kp,float Ki,float Kd) 
{
    GoPID->P     = Kp;
    GoPID->I     = Ki;
    GoPID->D     = Kd;
}
/*********************************************************************************************************
** Function name:       IncPID_Calculate
** Descriptions:        ����ʽPID����
** input parameters:    BackValue:����ֵ                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
int IncPID_CalculateGo(int BackValue) 
{
    static int Error,Inc;
    Error   = GoPID->Point - BackValue;
    /********************************
    ��u(k)=u(k)-u(k-1)
          =Kp*(e(k)-e(k-1) + T/Ti*e(k) + Td/T*(e(k)  - 2*e(k-1)+e(k-2)))
          =Kp*(1+T/Ti+Td/T)*e(k) - Kp*(1+2*Td/T)*e(k-1) + Kp*Td/T*e(k-2))
					=Kp*(e(k)-e(k-1))+Kp*T/Ti*e(k)+Kp*Td/T*(e(k)-2*e(k-1)+e(k-2))
					=P * (e(k)-e(k-1)) + I * e(k) + D * (e(k)-2*e(k-1)+e(k-2));
    ***********************************/
		//�򻯼���
		//P�����Ӧʱ�䣬����̫�������𵴣�I�����𵴣�D�������ú�С��һ�����С
		///*
    Inc     = (int) ( GoPID->P * Error
              - GoPID->I * GoPID->LastError
              + GoPID->D * GoPID->PrevError);
		//*/
		//�������---��������
		/*
		int POut=0, DOut=0, IOut=0;
		POut = (int)(GoPID->P *(Error-GoPID->LastError));
		DOut = (int)(GoPID->I *Error);
		IOut = (int)(GoPID->D *(Error-2*GoPID->LastError+GoPID->PrevError));
		Inc = POut + DOut + IOut;
		*/
    GoPID->PrevError = GoPID->LastError;
    GoPID->LastError = Error;
    
    return Inc;
}
/*********************************************************************************************************
** Function name:       LocPID_Calculate
** Descriptions:        λ��ʽPID����
** input parameters:    BackValue:����ֵ                         
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/ 
int LocPID_CalculateGo(int BackValue) 
{
    static int Error,delta,Loc;
    float PValue=0;
    Error           = GoPID->Point - BackValue;                          /* ���㵱ǰ���Error[0]         */

		if(Error>300){
			PValue= 300*GoPID->P;
			Error=0;
		}
		else if(Error<-300){
			PValue= -300*GoPID->P;
			Error=0;
		}
		else{
			PValue= GoPID->P * Error;
		}
		/*���޻���*/
		if(Error>100)
		{
			Error=(300-Error)/200*Error;
		}
		else if(Error<-100)
		{
			Error=(300+Error)/200*Error;
		}
		
		GoPID->SumError+=Error;
		
		if(GoPID->SumError > 700) 
			GoPID->SumError = 700;
		if(GoPID->SumError < -700) 
			GoPID->SumError = -700;
		
    delta           = Error - GoPID->LastError;
		/********************************
    ��u(k)=u(k)-u(k-1)
          =Kp*(      e(k)-e(k-1) + T/Ti*e(k) +Td*(e(k)  - 2*e(k-1)+e(k-2))/T)
          =Kp*(1+T/Ti+Td/T)*e(k) - Kp*(1+2*Td/T)*e(k-1) + Kp*Td/T*e(k-2))
          =
    ***********************************/

    Loc     = (int) ( PValue
              + GoPID->I * GoPID->SumError
              + GoPID->D * delta);
							
    GoPID->LastError = Error;
    
    return Loc;
}

void  ClearI()
{
		GoPID->SumError=700;
}

void MoreClearI(){
	GoPID->SumError=400;
}
void  ClearI1()
{
		GoPID->SumError=-300;
}
int Shake_CalculateGo(int BackValue) 
{
    static int Error,delta,Loc;
   
    Error           = GoPID->Point - BackValue;                   
		if(Error>150)
			GoPID->SumError += 150;
		else if(Error<-150)
			GoPID->SumError -= 150;
		else
			GoPID->SumError+=Error;
		if(Error>150)
			Error=150;
		if(Error<-150)
			Error=-150;
    delta           = Error - GoPID->LastError;
    if(GoPID->SumError>700)
			GoPID->SumError =700;
		if(GoPID->SumError<-700)
			GoPID->SumError=-700;
    Loc     = (int) ( GoPID->P * Error
              + GoPID->I * GoPID->SumError
              + GoPID->D * delta);
    GoPID->LastError = Error;
		return 0;
}

#include "Control.h"

//�ٶȿ���
//===ֱ���ٶ�220������ٶ�150
//===����245-220 245-200
//===����245-200
int   SpeedSetStraight=225;
int   SpeedSetRamp=200;
float SpeedKp=6.4f;
float SpeedKi=0.6f;
float SpeedKd=0;
int   SpeedHope=0;
int32_t MotorQD=0;
int isBackStop = 1;
//�������
//===1-50 2-100 3-150 4-200 5-250 6-300
#define SteerControlPLL 6
int CenterControl=0;
int SurvoCenter=824;	//825 816
float Direction_P = 6.3f;	//5.9-6.8---almost!-5.9
//===��P---P6.4---D3.2
//===�ֶ�P---D3.2
//ģ��---P6.3---D3.4
float Direction_D = 5.2f; //3.2---3.7perfect!-3.6  5.3 5.0
//===3.2f
float Direction_K = 0.0f;	//С������-��S��ƫ���ֱ����࣬����б��������
//���ڻ�е���Գƣ������޷���һ��
#define DirectionRightMax -190	//180
#define DirectionLeftMax 190
float SteerOut = 0.0f;

#define DirectionArrayLen 8
//const float DirectionP_Array[DirectionArrayLen] = {
//5.7f, //00-05	 //ֱ��
//5.9f, //05-10  //СS��+��S��
//6.2f, //10-15	 //��С���P�Լ��ϰ�P,̫�����ײ�����
//6.3f, //15-20
//6.7f, //20-25	 //������P
//6.4f, //25-30  //������P�ͳ�����P
//6.2f, //30-35  //������P��̫�����ײ����𵴣���ԱȽ�С
//5.7f  //35-40  //�����ֱ����׼��---�㹻С���Ƚ�����
//};
//const float DirectionP_Array[DirectionArrayLen] = {
//4.9f, //00-05	 //ֱ��
//4.9f, //05-10  //СS��+��S��	//4.2
//5.8f, //10-15	 //��С���P�Լ��ϰ�P,̫�����ײ�����//5.8
//7.4f, //15-20	//7.8-7.4
//6.4f, //20-25	 //������P
//5.8f, //25-30  //������P�ͳ�����P
//5.8f, //30-35  //������P��̫�����ײ����𵴣���ԱȽ�С
//5.8f  //35-40  //�����ֱ����׼��---�㹻С���Ƚ�����
//};
//const float DirectionP_Array[DirectionArrayLen] = {
//4.9f, //00-05	 //ֱ��
//5.3f, //05-10  //СS��+��S��	//4.2
//6.2f, //10-15	 //��С���P�Լ��ϰ�P,̫�����ײ�����//5.8
//7.4f, //15-20	//7.8-7.4
//6.9f, //20-25	 //������P
//6.4f, //25-30  //������P�ͳ�����P
//5.8f, //30-35  //������P��̫�����ײ����𵴣���ԱȽ�С
//5.8f  //35-40  //�����ֱ����׼��---�㹻С���Ƚ�����
//};
//const float DirectionP_Array[DirectionArrayLen] = {
//4.9f, //00-05	 //ֱ��
//4.9f, //05-10  //СS��+��S��	//4.2
//6.2f, //10-15	 //��С���P�Լ��ϰ�P,̫�����ײ�����//5.8
//7.4f, //15-20	//7.8-7.4
//6.9f, //20-25	 //������P
//6.5f, //25-30  //������P�ͳ�����P
//5.8f, //30-35  //������P��̫�����ײ����𵴣���ԱȽ�С
//5.8f  //35-40  //�����ֱ����׼��---�㹻С���Ƚ�����
//};
//const float DirectionP_Array[DirectionArrayLen] = {
//5.1f, //00-05	 //ֱ��
//5.3f, //05-10  //СS��+��S��	//4.2->5.1
//6.5f, //10-15	 //��С���P�Լ��ϰ�P,̫�����ײ�����//5.8 //6.2
//7.0f, //15-20	//7.8-7.4
//6.7f, //20-25	 //������P
//6.3f, //25-30  //������P�ͳ�����P
//5.8f, //30-35  //������P��̫�����ײ����𵴣���ԱȽ�С
//5.8f  //35-40  //�����ֱ����׼��---�㹻С���Ƚ�����
//};
const float DirectionP_Array[DirectionArrayLen] = {
5.1f, //00-05	 //ֱ��
5.3f, //05-10  //СS��+��S��	//4.2->5.1
6.5f, //10-15	 //��С���P�Լ��ϰ�P,̫�����ײ�����//5.8 //6.2
7.2f, //15-20	//7.8-7.4
6.7f, //20-25	 //������P
6.3f, //25-30  //������P�ͳ�����P
5.8f, //30-35  //������P��̫�����ײ����𵴣���ԱȽ�С
5.8f  //35-40  //�����ֱ����׼��---�㹻С���Ƚ�����
};
/**
*	@brief ���,���,��������ʼ�� 
*	@return ���״̬
*/
Status Motor_Init(void)
{
    ///���
		//���ת�Ǽ���
		//0.5~2.5 �м�ֵ1.52
		//����м�ֵ = 1.52 / ��10000����ռ�ձȣ� / 50��Ƶ�ʣ��� * 10000
		//1.52*200=304*10000
    FTM_PWM_QuickInit(FTM2_CH0_PB18, kPWM_EdgeAligned, 50 * SteerControlPLL, SurvoCenter * SteerControlPLL);//��������,���Ҽ�С
    ///�ҵ��(PA3ǰ,PA4��)
    FTM_PWM_QuickInit(FTM0_CH4_PA07, kPWM_EdgeAligned, 12000, 0);
    FTM_PWM_QuickInit(FTM0_CH3_PA06, kPWM_EdgeAligned, 12000, 0);
		
    ///�����������
    //GPIO_QuickInit(HW_GPIOB, 11, kGPIO_Mode_IPU);
    //LPTMR_PC_QuickInit(LPTMR_ALT2_PC05);   
		//�ұ�������������
    FTM_QD_QuickInit(FTM1_QD_PHA_PA12_PHB_PA13, kFTM_QD_InvertedPolarity, kQD_PHABEncoding);
		
    return OK;
}

/**
* @breif �޸Ķ����ֵ��ĸ���
**/
Status SteerInvalidAgain(){
	FTM_PWM_QuickInit(FTM2_CH0_PB18, kPWM_EdgeAligned, 50 * SteerControlPLL, SurvoCenter * SteerControlPLL);//��������,���Ҽ�С
	return OK;
}

/**
*	@brief ����������
*   ����ָ��,ͨ��ָ���޸�ԭֵ
*	@return ���״̬
*/
Status Get_Speed(int32_t *Speed)
{
	 uint8_t SpeedDir = 0;
	 uint32_t ActualSpeed = 0;
	 FTM_QD_GetData(HW_FTM1, Speed, &SpeedDir);
   FTM_QD_ClearCount(HW_FTM1);	
	 //�ٶ����ƣ���ֹ��ת
	 if(*Speed > 30000)  *Speed = 65535 - *Speed;
	 ActualSpeed = *Speed;
	 //�����ٶȷ����ٶȷ��������и�
	 if(!SpeedDir){
			*Speed = -ActualSpeed;
	 }else{
			*Speed = ActualSpeed;
	 }
   return OK;
}

/**
* @brief �ٶȵ���
**/
int ControlValuePush(int ActualSpeed, int SpeedHope){
		int ControlValue = 0;
		int BaseUp = 0,SpeedBase=0;
		int MakeUp = 0;
		PID_SetPointGo(SpeedHope);
		//�ڴ�ͨ��y=ax^2+bx+c����ΪP�����������
		//ͨ��error����������������P�������ԭ��Գ�n 
		if(SpeedHope >= SpeedSetStraight){
			BaseUp = (ABS(SpeedHope - ActualSpeed) * (SpeedHope - ActualSpeed))/15.0f;
		}else{
			BaseUp = (ABS(SpeedHope - ActualSpeed) * (SpeedHope - ActualSpeed))/20.0f+400;
		}
		//һ���̶�ֵ+һ��Ŀ��ֵ*�����Ĺ̶���ֵ�������������������ڣ�
		SpeedBase = 7.4f * SpeedHope + MakeUp + BaseUp;
		ControlValue = (int)(SpeedBase  + LocPID_CalculateGo(ActualSpeed));
		if(ControlValue > 9900) ControlValue = 9900;
		if(ControlValue < -9900) ControlValue = -9900;
		return ControlValue;
}

/**
*	@brief �ٶȿ���
* @param[in] �ٶ��趨ֵ
*	@return ״ֵ̬
*/
Status Speed_Control(int Speed_Give)
	{
		int ActualSpeed = MotorQD;	//��ǰ�������ٶ�
		int ControlValue=0;
		static int StraightCount = 0;
		static int BeepOnceTime = 0;
		static int isLongRoad = 0, LongRoadCount = 0;
		static int VeryCount=0, IsVery = 0;
		static int VeryBackStopTime = 0;
		static int RampCount = 0;
		static int StartCount = 0, StartPushFlag = 0;
		static int ContinueICount = 0,ContinueIFlag =  0;
		//=========ģ�����Ƶ����ٶ�=====================================
		static float LastCenterK = 0.0f;
		float DeltaCenterK = CenterGuideK - LastCenterK;
		//SpeedHope = SpeedSetRamp + TargetSpeed_DoFuzzy(&TartgetSpeed_Reference, ABS(CenterGuideK), ABS(DeltaCenterK));	//ģ��ִ��
		SpeedHope = SpeedSetRamp;
		LastCenterK = CenterGuideK;
		//==========ֱ������============================================
		if(abs(CenterGuideK) < 10){
			SpeedHope = SpeedSetStraight;
		}
		//===============��ʼ����=======================================
		if(StartPushFlag == 0){
			StartCount++;
			SpeedHope = SpeedSetStraight + 50;
			if(StartCount > 15) StartPushFlag = 1; 
		}
		//===========�ٶ�PID����========================================	
		ControlValue = ControlValuePush(ActualSpeed, SpeedHope);
		//=========== �ϰ���============================================
		if(1 == ObIsObStacle){
			ObIsObStacle = 0;
			if(0 == BeepOnceTime){
				BeepOnceTime = 1;
				//BeepLoopTime = 5;
			}
		}
		//===========ʶ���µ�===========================================
		if(IsRamp){
			if(RampCount < 7){
				RampCount++;
				if(ActualSpeed > 150)
					ControlValue = -9900;
				else
					ControlValue = (0 - ActualSpeed) * 25;
				if(ControlValue < -9900)
					ControlValue = -9900;
			}
			else{
				ControlValue = ControlValuePush(ActualSpeed, SpeedSetStraight);
				//BeepLoopTime = 5;
				RampCount = 0;
				IsRamp = 0;
			}
		}
		//===========��ֱ��ɲ��=========================================
		if(abs(CenterGuideK) > 22){
			IsVery = 1;
		}
		if(IsVery == 1 && abs(CenterGuideK) < 10){
			VeryCount++;
			if(VeryCount > 100) VeryCount = 100;
		}
		if(VeryCount <= 5 && abs(CenterGuideK) > 22){
			IsVery = 0;
			VeryCount = 0;
		}
		if(VeryCount > 5 && abs(CenterGuideK) > 22 && 0 != isBackStop && 0 != StartPushFlag){	//22
			VeryBackStopTime++;
			if(VeryBackStopTime > 100) VeryBackStopTime = 100;
			isLongRoad = 0;
			LongRoadCount = 0;
			StraightCount = 0;
			//===�������ɲ���ٶ�
			//180-4 200-6 220-8
			//int VeryBackStopLimit = (int)(-0.1f * SpeedSetStraight- 14.0f);
			if(VeryBackStopTime < 7){
				//ControlValue = (0 - ActualSpeed) * 15;
				//have something I don't know???
				if(ActualSpeed>130)
					ControlValue=-7000;
				else if(ActualSpeed<130)
					ControlValue = (0 - ActualSpeed) * 25;
				if(ControlValue < -9900)
					ControlValue = -9900;
			}
			else{
				//===ɲ�����֮���ٶ�̫������Ҫ�ܿ������ٶ�
				ContinueIFlag = 1;
				ControlValue = ControlValuePush(ActualSpeed, SpeedSetStraight);
				IsVery = 0;
				VeryCount = 0;
				VeryBackStopTime = 0;
				isLongRoad = 0;
				LongRoadCount = 0;
				StraightCount = 0;
			}
			if(ActualSpeed < 80){ 
				if(0 != VeryBackStopTime && 0 != LongRoadCount){
					ContinueIFlag = 1;
				}	
				ControlValue = ControlValuePush(ActualSpeed, SpeedSetStraight);			
				isLongRoad = 0;
				LongRoadCount = 0;
				StraightCount = 0;
				IsVery = 0;
				VeryCount = 0;
				VeryBackStopTime = 0;
			}
		}
		
		//===�����ܳ�һ�γ�ֱ�����ٶ�Ҫ��ɲ�����B�����Դ�PID������Ч������
		//СSҲ��Ϊ��ֱ��������ƫ���Ƚϴ�һ��	
		if(abs(CenterGuideK) < 10){
			StraightCount ++;
			if(StraightCount > 100) StraightCount = 100;
		}
		//===��ֱ����ɲ��
		if(StraightCount > 24 && abs(CenterGuideK) > 27){
			StraightCount = 0;
			isLongRoad = 1;
		}
		if(isLongRoad && 0 != isBackStop && 0 != StartPushFlag){
			LongRoadCount++;
			if(LongRoadCount > 100) LongRoadCount = 100;
			//��ֵͨ-13
			//160 - 10 , 180 - 12, 200 - 15
			StraightCount = 0;
			IsVery = 0;
			VeryCount = 0;
			VeryBackStopTime = 0;
			//SlienceTime = (int)(0.12f * SpeedSetStraight - 10.0f);
			if(LongRoadCount < 13){
				//ControlValue = (0 - ActualSpeed) * 15;
				if(ActualSpeed>60)
					ControlValue=-9900;
				else if(ActualSpeed<60)
					ControlValue = (0 - ActualSpeed) * 55;
				if(ControlValue < -9900)
					ControlValue = -9900;
			}
			else{
				ContinueIFlag = 1;
				ControlValue = ControlValuePush(ActualSpeed, SpeedSetStraight);
				isLongRoad = 0;
				LongRoadCount = 0;
				StraightCount = 0;
				IsVery = 0;
				VeryCount = 0;
				VeryBackStopTime = 0;
			}
			if(ActualSpeed < 60){ 
				if(0 != VeryBackStopTime && 0 != LongRoadCount){
					ContinueIFlag = 1;
				}	
				ControlValue = ControlValuePush(ActualSpeed, SpeedSetStraight);			
				isLongRoad = 0;
				LongRoadCount = 0;
				StraightCount = 0;
				IsVery = 0;
				VeryCount = 0;
				VeryBackStopTime = 0;
			}
		}
		//===������ɲ�����ܿ������ٶ�,��߻�����
		if(1 == ContinueIFlag){
			ContinueICount++;
			if(ContinueICount < 8){
				ControlValue += 200;
			}
			else{
				ContinueIFlag = 0;
				ContinueICount = 0;
			}
		}
		/*=========�������==============*/
		if(ControlValue > 0){
			FTM_PWM_ChangeDuty(HW_FTM0, 3, 0);
			FTM_PWM_ChangeDuty(HW_FTM0, 4, ControlValue);
		}else{
			FTM_PWM_ChangeDuty(HW_FTM0, 3, -ControlValue);
			FTM_PWM_ChangeDuty(HW_FTM0, 4, 0);
		}
		
//#define SPEED_PROCESS_PARAM
#ifdef SPEED_PROCESS_PARAM
	OutData[0]=abs(CenterGuideK);
	OutData[1]=abs(DeltaCenterK);
	OutData[2]=SpeedHope;
	OutData[3]=ActualSpeed;
	OutData[0]=StraightCount;
	OutData[1]=LongRoadCount;
	//OutData[2]=LongLongRoadCount;
	OutData[3]=VeryCount;
	OutPut_Data();
#endif
    return OK;
}

/**
*	@brief �������
* @param[in] �����趨ֵ,��ֵ685
*	@return ״ֵ̬
*/

Status Dirction_Control(int  DirectionGive)
{
	int Up = 0,Ud = 0;
	int DirectionOut = 0;
	//===============ƫ����ֵ
	if(DirectionGive > 39) DirectionGive = 39;
	if(DirectionGive < -39) DirectionGive = -39;
	/*-----------------P������---------------------*/
#define USE_RoomP
#ifdef USE_RoomP
	int Loc = abs(DirectionGive) / 5;
	Direction_P = DirectionP_Array[Loc];
#endif
	Up = (int)(Direction_P * DirectionGive);
	/*-----------------K����б�ʿ�����---------------------*/
	int Uk = 0;
	Uk = (int)(Direction_K * (float)CenterGuideK / 10.0f);
	/*------------------D������--------------------*/
	//��ͨD����---��߶������ٶ�,΢�ִ���Сƫ���ڶ�������
	//D(n) = Kd * (E(n) - E(n-1))
	/*
	static int LastErr = 0;
	int DeltaErr = DirectionGive - LastErr;
	LastErr = DirectionGive;
	Ud = (int)(Direction_D * DeltaErr);
  */
	//========ģ��΢��D==================================
	///*
	static int LastErr = 0;
	int DeltaErr = DirectionGive - LastErr;
	LastErr = DirectionGive;
	float DynamicD = 0.0f;
	DynamicD = Direction_D + Fuzzy_Do(DirectionGive,DeltaErr,&Fuzzy_AD);
	Ud = (int)(DynamicD * DeltaErr);
	//*/

	/*-------------------���������-------------------*/
	DirectionOut = Up - Uk + Ud;
	//�ڼ�С��ƫ���£��Ͳ�Ҫ�ö�����,PID��������
	//if(DirectionGive >= -3 && DirectionGive <= 3) 
	//	DirectionOut=0;
	if(DirectionOut > DirectionLeftMax) DirectionOut = DirectionLeftMax;
	if(DirectionOut < DirectionRightMax) DirectionOut = DirectionRightMax; 
	//=====��¼�¶�����ֵ����������õ�
	SteerOut = (float)DirectionOut;
	FTM_PWM_ChangeDuty(HW_FTM2, 0, (SurvoCenter - DirectionOut) * SteerControlPLL);
//#define STEER_PROCESS_PARAM
#ifdef STEER_PROCESS_PARAM
	OutData[0]=DirectionGive * 10;
	OutData[1]=DeltaErr * 10;
	OutData[2]=Up;
	OutData[3]=Ud;
	OutPut_Data();
#endif
  return OK;
}

/**
*	@brief ���ƶ������
*	@return ״ֵ̬
*/
Status TimeToControlSteer(void)
{
		Dirction_Control(CenterControl);//CenterControl	
    return OK;
}

/**
* @breif ���Ƶ������
* @return ״ֵ̬
**/
Status TimeToControlMotor(void){
	static uint8_t OnceTime = 0;
	static int StopTimeCount = 0;
	if(0 == StStartLine){
		Speed_Control(SpeedSetStraight); 
	}else{
		//===ɲ��������
		StopTimeCount++;
		if(StopTimeCount >= 100) StopTimeCount = 100;
		if(StopTimeCount >= 25){
			TimeToStop();
		}
		//===�ڼ�⵽������֮�������ִֻ��һ��
		if(0 == OnceTime){
			OnceTime = 1;
			TimeToSleep();
			BeepLoopTime = 5;
		}
	}
	return OK;
}
/**
*	@brief  ���ߺ���
*	@return ״ֵ̬
*/
Status TimeToSleep(void)
{
	//������Ϊ0 
	FTM_PWM_ChangeDuty(HW_FTM0, 3, 0);
	FTM_PWM_ChangeDuty(HW_FTM0, 4, 0);
	
	//�������
	//FTM_PWM_ChangeDuty(HW_FTM2, 0, SurvoCenter);
	return OK;
}
/**  
*	@brief  ����,ɲ��
*/
Status TimeToStop(void)
{ 
	static int FirstStopSpeed = 0, DownSpeed = 0;
	static unsigned char StopCount=0,FirstDown=0, StopFinished=0;
	int ActualSpeed = MotorQD;
	int SpeedBase = 0;
	int ControlValue=0;
	if(0 == FirstStopSpeed){
		FirstStopSpeed = ActualSpeed;
		DownSpeed = 0 - (FirstStopSpeed*FirstStopSpeed/(30*350));
		if(ABS(DownSpeed) < 20)
			DownSpeed = 0;	
		if(DownSpeed > 30)	
			DownSpeed = 30;
		if(DownSpeed < -30)
			DownSpeed = -30;
	}
	//���ٶȻ��ǱȽϴ������£�Ҳ����Ƶ����ɲ���������˵��������StopCount<20
	//StopFinshed�����־λ��ʾ�����ҵ��ٶ���DownSpeed�����ǻ���ʱ����ѡ����ɲ
	if(ActualSpeed > DownSpeed && 0 == StopFinished && StopCount < 25){
			ControlValue = -9999;
	}
	else{
		StopFinished = 1;
		if(0 == FirstDown){
			FirstDown = 1;
			ClearI1();
		}
		PID_SetPointGo(0);
		SpeedBase = (ABS(0 - ActualSpeed) * (0 - ActualSpeed))/30.0f;
		ControlValue = (int)(SpeedBase + Shake_CalculateGo(ActualSpeed));
	}
	if(ControlValue > 0){
			FTM_PWM_ChangeDuty(HW_FTM0, 3, 0);
			FTM_PWM_ChangeDuty(HW_FTM0, 4, ControlValue);
	}else{
			FTM_PWM_ChangeDuty(HW_FTM0, 3, -ControlValue);
			FTM_PWM_ChangeDuty(HW_FTM0, 4, 0);
	}
//#define SPEED_STOP_PARAM
#ifdef SPEED_STOP_PARAM
	OutData[0]=DownSpeed;
	OutData[1]=ActualSpeed;
	OutData[2]=SpeedBase;
	OutData[3]=ControlValue;
	OutPut_Data();
#endif
  return 1;
}



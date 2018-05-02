#include "Beeper.h"

/**
*	@brief ����������
*   @param[in] ��������,��������ʱ��/33ms,ͣЪʱ��/33ms
*	@return ���״̬
*/
Status Beeper(char *Count,char Continous,char Rest)
{
	static char AllDone=1,OnceDone=0,BeepCount=0,BeepContinous=0,BeepRest=0;
	if(0==*Count)
	{
		Buzzer_Off;
		return 0;
	}
	if(AllDone == 1)
	{
		BeepCount = *Count;  
		OnceDone=1;
		AllDone=0;
  }//ֻ�ڵ�һ�ν��շ�ֵ
	if(OnceDone == 1)
	{
		BeepContinous=Continous;
		BeepRest=Rest;
		OnceDone=0;
	}//ÿһ����Ъ��������ո�ֵ
	
	if(0!=BeepContinous)
	{
		Buzzer_On;
		BeepContinous--;
	}//��������
	
	if(0==BeepContinous&&0!=BeepRest)
	{
			Buzzer_Off;
			BeepRest--;
	}//��������,��Ъʱ����
	
	if(0==BeepRest)
	{
		OnceDone=1;
		BeepCount--;
	}//������Ъ������,ˢ�¸�ֵ,���ж��Ƿ�ͷ
	if(0==BeepCount)
	{
		AllDone=1;
		*Count=0;
	}//��ͷ�Ͳ���
	return 1;
}

/**
* @breif ������ͼ����
**/
Status BeeperLoopTime(unsigned char* LoopTime){
	static unsigned char KeepLoopTime = 0,OnceTime = 1; 
	if(0 == *LoopTime){
		Buzzer_Off;
		OnceTime = 1;
		return 0;
	}
	if(1 == OnceTime){
		OnceTime = 0;
		KeepLoopTime = *LoopTime;
		Buzzer_On;
	}//��һ�ε�ʱ��ֵ
	KeepLoopTime--;
	if(0 == KeepLoopTime){
		*LoopTime = 0;
		Buzzer_Off;
	}
	return 1;
}

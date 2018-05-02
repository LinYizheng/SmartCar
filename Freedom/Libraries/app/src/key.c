/**
*     COPYRIGHT NOTICE
*     All rights reserved.
*
* @file         key.c
* @brief        ����GUI
*
*
* @version 1.0
* @author  hejun
* @date    2015��1��3��
*
*
*     �޶�˵��������汾
*/

#include "key.h"
//��
CUR_SET Cur_Set;
static char Cur_Now = Back_Cur_Num;
static char Comfirm_Times = 0;
unsigned char RecX=55;
signed char Cur_Count=Back_Cur_Num;

//һ����60���ң�����ȡ57
unsigned char Threshold = 62;

/**
* @breif ��������ȥ֮���ӳ�һ��ʱ��
**/
static void DelayKeyPress(void){
	for(int i=0;i<1600;i++)
		for(int j=0;j<2650;j++);
}

/**
*	@brief ����ɨ��
*	@return ����ֵ
*/
uint8_t Key_Scan(void)
{
    uint8_t key_num = 0;
	  if(GPIO_ReadBit(KEY_PORT,KEY_UP))
			key_num = 2;
		else if(GPIO_ReadBit(KEY_PORT,KEY_DOWN))
			key_num = 8;
		else if(GPIO_ReadBit(KEY_PORT,KEY_LEFT))
			key_num = 4;
		else if(GPIO_ReadBit(KEY_PORT,KEY_RIGHT))
			key_num = 6;
		else if(GPIO_ReadBit(KEY_PORT,KEY_PRESS))
			key_num = 5;
		DelayKeyPress();
    return key_num;
}

/**
*	@brief  ��Ӧ����ɨ��
* @parma[in] ɨ��İ���ֵ
*	@return ״ֵ̬
*/
Status Key_Ack(uint8_t Key_Num)
{
    switch(Key_Num)
    {
    case 2: if(Cur_Now != Back_Cur_Num)Last_Parma();
        break;
    case 4: if(Cur_Now == Back_Cur_Num)//ѡ���������
            {
              if(Cur_Count == Back_Cur_Num)
                Cur_Count=1;
                Cur_Count--;
              if(Cur_Count<0)Cur_Count=0;
              if(Cur_Count==Speed_Cur_Num)Speed_Cur();
              else if(Cur_Count==Dirction_Cur_Num)Dirction_Cur();
              else if(Cur_Count==Camera_Cur_Num)Camera_Cur();
            }
            else
            {
              Dec_Value();
            }
        break;
    case 5:
        Confirm_Set();
        break;
    case 6: if(Cur_Now == Back_Cur_Num)
            {
              if(Cur_Count == Back_Cur_Num)
                Cur_Count=-1;
              Cur_Count++;
              if(Cur_Count>=3)Cur_Count=3;
              if(Cur_Count==Speed_Cur_Num)Speed_Cur();
              else if(Cur_Count==Dirction_Cur_Num)Dirction_Cur();
              else if(Cur_Count==Camera_Cur_Num)Camera_Cur();
              else if(Cur_Count==Stop_Cur_Num)Stop_Cur();
            }
            else
            {
             Add_Value();
            }
        break;
    case 8: if(Cur_Now != Back_Cur_Num)Next_Parma();
        break;
    default:
        break;
    }
    return OK;
}
/**
*	@brief  ���ν����ʼ��
*	@return ״ֵ̬
*/
Status Cur_Set_Init(void)
{
		GPIO_QuickInit (KEY_PORT, KEY_GND,   kGPIO_Mode_OPP);
    GPIO_QuickInit (KEY_PORT, KEY_UP,    kGPIO_Mode_IPD);
    GPIO_QuickInit (KEY_PORT, KEY_DOWN,   kGPIO_Mode_IPD);
    GPIO_QuickInit (KEY_PORT, KEY_LEFT,    kGPIO_Mode_IPD);
		GPIO_QuickInit (KEY_PORT, KEY_RIGHT,   kGPIO_Mode_IPD);
    GPIO_QuickInit (KEY_PORT, KEY_PRESS,    kGPIO_Mode_IPD);
    GPIO_WriteBit(KEY_PORT, KEY_GND,1);
		
    ///�ٶȷ���PID�����ʼ��
    Cur_Set[Speed_Cur_Num].parma_turn = 0;
    Cur_Set[Speed_Cur_Num].parma_name[0] = "SpeedST";
    Cur_Set[Speed_Cur_Num].parma_name[1] = "GoRamp";
    Cur_Set[Speed_Cur_Num].parma_name[2] = "DiffKp";
    Cur_Set[Speed_Cur_Num].parma_name[3] = "DiffKd";

    Cur_Set[Speed_Cur_Num].parma_site[0] = 0;
    Cur_Set[Speed_Cur_Num].parma_site[1] = 2;
    Cur_Set[Speed_Cur_Num].parma_site[2] = 4;
    Cur_Set[Speed_Cur_Num].parma_site[3] = 6;

    Cur_Set[Speed_Cur_Num].parma_step[0] = 10;
    Cur_Set[Speed_Cur_Num].parma_step[1] = 10;
    Cur_Set[Speed_Cur_Num].parma_step[2] = 0.1;
    Cur_Set[Speed_Cur_Num].parma_step[3] = 0.1;

    Cur_Set[Speed_Cur_Num].parma_step_Change[0] = 10;
    Cur_Set[Speed_Cur_Num].parma_step_Change[1] = 10;
    Cur_Set[Speed_Cur_Num].parma_step_Change[2] = 0.1;
    Cur_Set[Speed_Cur_Num].parma_step_Change[3] = 0.1;

    Cur_Set[Speed_Cur_Num].parma[0] = SpeedSetStraight;
    Cur_Set[Speed_Cur_Num].parma[1] = SpeedSetRamp;
    Cur_Set[Speed_Cur_Num].parma[2] = Direction_P;
    Cur_Set[Speed_Cur_Num].parma[3] = Direction_D;
    ///��������ʼ��
    Cur_Set[Dirction_Cur_Num].parma_turn = 0;
    Cur_Set[Dirction_Cur_Num].parma_name[0] = "SurvoCen";
    Cur_Set[Dirction_Cur_Num].parma_name[1] = "SpeedKp";
    Cur_Set[Dirction_Cur_Num].parma_name[2] = "SpeedKi";
    Cur_Set[Dirction_Cur_Num].parma_name[3] = "Thres";

    Cur_Set[Dirction_Cur_Num].parma_site[0] = 0;
    Cur_Set[Dirction_Cur_Num].parma_site[1] = 2;
    Cur_Set[Dirction_Cur_Num].parma_site[2] = 4;
    Cur_Set[Dirction_Cur_Num].parma_site[3] = 6;

    Cur_Set[Dirction_Cur_Num].parma_step[0] = 1;
    Cur_Set[Dirction_Cur_Num].parma_step[1] = 0.2;
    Cur_Set[Dirction_Cur_Num].parma_step[2] = 0.1;
    Cur_Set[Dirction_Cur_Num].parma_step[3] = 2;

    Cur_Set[Dirction_Cur_Num].parma_step_Change[0] = 1;
    Cur_Set[Dirction_Cur_Num].parma_step_Change[1] = 0.1;
    Cur_Set[Dirction_Cur_Num].parma_step_Change[2] = 0.1;
    Cur_Set[Dirction_Cur_Num].parma_step_Change[3] = 1;

    Cur_Set[Dirction_Cur_Num].parma[0] = SurvoCenter;
    Cur_Set[Dirction_Cur_Num].parma[1] = SpeedKp;
    Cur_Set[Dirction_Cur_Num].parma[2] = SpeedKi;
    Cur_Set[Dirction_Cur_Num].parma[3] = Threshold;

    ///����ͷ�����ʼ��
    Cur_Set[Camera_Cur_Num].parma_turn = 0;
    Cur_Set[Camera_Cur_Num].parma_name[0] = "DiffKk";
    Cur_Set[Camera_Cur_Num].parma_name[1] = "RoadSTK";
    Cur_Set[Camera_Cur_Num].parma_name[2] = "RoadCOK";
    Cur_Set[Camera_Cur_Num].parma_name[3] = "IsBackS";

    Cur_Set[Camera_Cur_Num].parma_site[0] = 0;
    Cur_Set[Camera_Cur_Num].parma_site[1] = 2;
    Cur_Set[Camera_Cur_Num].parma_site[2] = 4;
    Cur_Set[Camera_Cur_Num].parma_site[3] = 6;

    Cur_Set[Camera_Cur_Num].parma_step[0] = 0.1;
    Cur_Set[Camera_Cur_Num].parma_step[1] = 5;
    Cur_Set[Camera_Cur_Num].parma_step[2] = 5;
    Cur_Set[Camera_Cur_Num].parma_step[3] = 1;

    Cur_Set[Camera_Cur_Num].parma_step_Change[0] = 0.1;
    Cur_Set[Camera_Cur_Num].parma_step_Change[1] = 10;
    Cur_Set[Camera_Cur_Num].parma_step_Change[2] = 10;
    Cur_Set[Camera_Cur_Num].parma_step_Change[3] = 1;

    Cur_Set[Camera_Cur_Num].parma[0] = Direction_K;
    Cur_Set[Camera_Cur_Num].parma[1] = 0;
    Cur_Set[Camera_Cur_Num].parma[2] = 0;
    Cur_Set[Camera_Cur_Num].parma[3] = isBackStop;

    return OK;
}

/**
*	@brief  �ٶȵ��ν���
*	@return ״ֵ̬
*/
Status Speed_Cur(void)
{
    LED_CLS();
    //Cur_Now = Speed_Cur_Num;
    Comfirm_Times = 0;
    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[0], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[0]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[0], Cur_Set[Speed_Cur_Num].parma[0], 2);

    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[1], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[1]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[1], Cur_Set[Speed_Cur_Num].parma[1], 2);

    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[2], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[2]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[2], Cur_Set[Speed_Cur_Num].parma[2], 2);

    LED_P6x8Str(0, Cur_Set[Speed_Cur_Num].parma_site[3], (uint8_t *)Cur_Set[Speed_Cur_Num].parma_name[3]);
    LED_PrintValueF(64, Cur_Set[Speed_Cur_Num].parma_site[3], Cur_Set[Speed_Cur_Num].parma[3], 2);
    
    //LCD_Rectangle(RecX,Cur_Set[Speed_Cur_Num].parma_site[Cur_Set[Speed_Cur_Num].parma_turn]);
    return OK;
}

/**
*	@brief  ������ν���
*	@return ״ֵ̬
*/
Status Dirction_Cur(void)
{
    LED_CLS();
    //Cur_Now = Dirction_Cur_Num;
    Comfirm_Times = 0;

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[0], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[0]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[0], Cur_Set[Dirction_Cur_Num].parma[0], 2);

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[1], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[1]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[1], Cur_Set[Dirction_Cur_Num].parma[1], 2);

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[2], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[2]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[2], Cur_Set[Dirction_Cur_Num].parma[2], 2);

    LED_P6x8Str(0, Cur_Set[Dirction_Cur_Num].parma_site[3], (uint8_t *)Cur_Set[Dirction_Cur_Num].parma_name[3]);
    LED_PrintValueF(64, Cur_Set[Dirction_Cur_Num].parma_site[3], Cur_Set[Dirction_Cur_Num].parma[3], 2);
    
    //LCD_Rectangle(RecX,Cur_Set[Dirction_Cur_Num].parma_site[Cur_Set[Dirction_Cur_Num].parma_turn]);
    return OK;
}

/**
*	@brief  ����ͷ���ν���
*	@return ״ֵ̬
*/
Status Camera_Cur(void)
{
    LED_CLS();
    //Cur_Now = Camera_Cur_Num;
    Comfirm_Times = 0;

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[0], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[0]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[0], Cur_Set[Camera_Cur_Num].parma[0], 2);

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[1], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[1]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[1], Cur_Set[Camera_Cur_Num].parma[1], 2);

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[2], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[2]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[2], Cur_Set[Camera_Cur_Num].parma[2], 2);

    LED_P6x8Str(0, Cur_Set[Camera_Cur_Num].parma_site[3], (uint8_t *)Cur_Set[Camera_Cur_Num].parma_name[3]);
    LED_PrintValueF(64, Cur_Set[Camera_Cur_Num].parma_site[3], Cur_Set[Camera_Cur_Num].parma[3], 2);
    
    //LCD_Rectangle(RecX,Cur_Set[Camera_Cur_Num].parma_site[Cur_Set[Camera_Cur_Num].parma_turn]);
    return OK;
}
/**
*	@brief  ֹͣ���ν���
*	@return ״ֵ̬
*/
Status Stop_Cur(void)
{
    LED_CLS();
    Comfirm_Times = 0;
    LED_P8x16Str(20, 4, (uint8_t *)"Skip!");
    return OK;
}
/**
*	@brief  ����ֵ����
*	@return ״ֵ̬
*/
Status Add_Value(void)
{
    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn] += Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn];
    LED_PrintValueF(64, Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn],
                    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn], 2);
    return OK;
}

/**
*	@brief  ����ֵ����
*	@return ״ֵ̬
*/
Status Dec_Value(void)
{
    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn] -= Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn];
    LED_PrintValueF(64, Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn],
                    Cur_Set[Cur_Now].parma[Cur_Set[Cur_Now].parma_turn], 2);
    return OK;
}

/**
*	@brief  ������������
*	@return ״ֵ̬
*/
Status Add_Step(void)
{
    Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] += Cur_Set[Cur_Now].parma_step_Change[Cur_Set[Cur_Now].parma_turn];
    return OK;
}

/**
*	@brief  ������������
*	@return ״ֵ̬
*/
Status Dec_Step(void)
{
    Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] -= Cur_Set[Cur_Now].parma_step_Change[Cur_Set[Cur_Now].parma_turn];
    if(Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] < 0)
        Cur_Set[Cur_Now].parma_step[Cur_Set[Cur_Now].parma_turn] = 0;
    return OK;
}

/**
*	@brief  ��һ������
*	@return ״ֵ̬
*/
Status Next_Parma(void)
{
    unsigned RecTurn=0,ClearTurn=0;
    ClearTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];
    Cur_Set[Cur_Now].parma_turn += 1;
    if(Cur_Set[Cur_Now].parma_turn > Parma_Num - 1)
        Cur_Set[Cur_Now].parma_turn = Parma_Num - 1;
    RecTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];
    if(RecTurn!=ClearTurn)
    {
      LCD_Rectangle(RecX,RecTurn);
      LCD_ClearRectangle(RecX,ClearTurn); 
    }
    return OK;
}

/**
*	@brief  ��һ������
*	@return ״ֵ̬
*/
Status Last_Parma(void)
{
    unsigned RecTurn=0,ClearTurn=0;
    ClearTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];  
    if(Cur_Set[Cur_Now].parma_turn < 1)
        Cur_Set[Cur_Now].parma_turn = 1;
    Cur_Set[Cur_Now].parma_turn -= 1;
     RecTurn=Cur_Set[Cur_Now].parma_site[Cur_Set[Cur_Now].parma_turn];
    if(RecTurn!=ClearTurn)
    {
      LCD_Rectangle(RecX,RecTurn);
      LCD_ClearRectangle(RecX,ClearTurn); 
    }
    return OK;
}

/**
*	@brief  ȷ����ť
*	@return ״ֵ̬
*/
Status Confirm_Set(void)
{
    unsigned RecTurn=0;
    if(Cur_Count>=0&&Cur_Count<3)
    RecTurn=Cur_Set[Cur_Count].parma_site[Cur_Set[Cur_Count].parma_turn];
    if(Cur_Now != Back_Cur_Num)
    {
      //�˳�����Ľ���
      Cur_Now = Back_Cur_Num;
      LCD_ClearRectangle(RecX,RecTurn);
    }
    else if(Cur_Count!=Back_Cur_Num&&Cur_Count!=Stop_Cur_Num)
    {
      ///���뻺��Ľ��棨��ȥ������ʼ������˳����棩
      Cur_Now = Cur_Count;
      LCD_Rectangle(RecX,RecTurn);
    }
    if(Cur_Count==Stop_Cur_Num)
    Comfirm_Times++;
    if(Comfirm_Times>20)
    {
      LED_CLS();
      Cur_Now = Back_Cur_Num;
      Back_Cur(Comfirm_Times);
    }
    return OK;
}

/**
*	@brief  ����
*	@return ״ֵ̬
*/
Status Back_Cur(char Comfirm)
{
    if(Comfirm_Times <20)
        LED_P8x16Str(20, 4, (uint8_t *)"System Set!");
    else if(Comfirm_Times >= 40)
        LED_P8x16Str(25, 4, (uint8_t *)"Ready go!");
    return OK;
}

/**
*	@brief  ��������
*	@return ״ֵ̬
*/
Status Key_Work(void)
{
    uint8_t key = 0;
    Cur_Set_Init();
    Back_Cur(Comfirm_Times);
    while(1)
    {
        key = Key_Scan();
        Key_Ack(key);
        if(Comfirm_Times >= 4)
        {
            LED_CLS();
            FinishSet();
            break;
        }
    }
    return OK;
}

/**
*	@brief   ���ݰ���ֵд��
*	@return ״ֵ̬
*/
Status FinishSet(void)
{
    SpeedSetStraight = Cur_Set[Speed_Cur_Num].parma[0];
    SpeedSetRamp = Cur_Set[Speed_Cur_Num].parma[1];
    Direction_P = Cur_Set[Speed_Cur_Num].parma[2];
    Direction_D = Cur_Set[Speed_Cur_Num].parma[3];

    SurvoCenter=Cur_Set[Dirction_Cur_Num].parma[0];
    SpeedKp=Cur_Set[Dirction_Cur_Num].parma[1];
    SpeedKi=Cur_Set[Dirction_Cur_Num].parma[2];
    Threshold=(unsigned char)Cur_Set[Dirction_Cur_Num].parma[3]; 
   
    Direction_K = Cur_Set[Camera_Cur_Num].parma[0];
		//RoadStraight_K = (int)Cur_Set[Camera_Cur_Num].parma[1];
    //RoadCorner_K = (int)Cur_Set[Camera_Cur_Num].parma[2];
		isBackStop = (int)Cur_Set[Camera_Cur_Num].parma[3];
    return OK;
}

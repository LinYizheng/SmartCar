#include "SCCB.h"

/**
 *  @brief      SCCB�ӳٺ���
 *  @param      time    ��ʱʱ��
*/
static void SCCB_delayus(volatile uint16_t us)
{
    DelayUs(us);
}

//////////////////////////////////////
/**
*	@brief ��ʼ��ģ��SCCB�ӿ�
* ��ʼ��SDA,SCK����
*	@return  ״̬��
*/
Status SCCB_GPIO_Config(void)
{
    GPIO_QuickInit(SCCB_PORT,  SCCB_SCK,  kGPIO_Mode_OPP);
    GPIO_QuickInit(SCCB_PORT,  SCCB_SDA,  kGPIO_Mode_OPP);
    SCCB_SCK_H();
    SCCB_SDA_H();

    PORT_PinPullConfig(SCCB_PORT, SCCB_SCK, kPullUp);
    PORT_PinPullConfig(SCCB_PORT, SCCB_SDA, kPullUp);

    return OK;
}

//////////////////////////////////////
/**
*	@brief ����SCCB,SDA�������
*	@return  ״̬��
*/
Status SCCB_SDA_OUTPUT(void)
{
    GPIO_PinConfig(SCCB_PORT,  SCCB_SDA, kOutput);
    return OK;
}

//////////////////////////////////////
/**
*	@brief ����SCCB,SDA��������
*	@return  ״̬��
*/
Status SCCB_SDA_INPUT(void)
{
    GPIO_PinConfig(SCCB_PORT,  SCCB_SDA, kInput);
    return OK;
}

//////////////////////////////////////
/**
*	@brief start����,SCCB����ʼ�ź�
* SCK----
*	SDA--__
*	@return  ״̬��
*/
Status startSCCB(void)
{
    SCCB_SDA_H();     //�����߸ߵ�ƽ
    SCCB_SCK_H();	   //��ʱ���߸ߵ�ʱ���������ɸ�����
    SCCB_delayus(15);
		
		SCCB_SDA_IN;
		if(!SCCB_SDA_STATE){	//������æ
			SCCB_SDA_OUT;
			return FALSE;
		}
		SCCB_SDA_OUT;
		
		SCCB_SDA_L();
    SCCB_delayus(15);
    SCCB_SCK_L();	 //�����߻ָ��͵�ƽ��������������Ҫ
		
    return OK;
}

//////////////////////////////////////
/**
*	@brief stop����,SCCB��ֹͣ�ź�
*	@return  ״̬��
*/
Status stopSCCB(void)
{
		SCCB_SCK_L();
    SCCB_SDA_L();
    SCCB_delayus(15);

    SCCB_SCK_H();
    SCCB_delayus(15);

    SCCB_SDA_H();
    SCCB_delayus(15);

    return OK;
}

//////////////////////////////////////
/**
*	@brief noAck,����������ȡ�е����һ����������
*	@return  ״̬��
*/
Status noAck(void)
{
    SCCB_SCK_L();
    SCCB_delayus(15);

    SCCB_SDA_H();
    SCCB_delayus(15);

    SCCB_SCK_H();
    SCCB_delayus(15);

    SCCB_SCK_L();
    SCCB_delayus(15);

    return OK;
}

//////////////////////////////////////
/**
*	@brief Ack
*	@return  ״̬��
*/
Status Ack(void)
{
    SCCB_SCK_L();
    SCCB_delayus(15);

    SCCB_SDA_L();
    SCCB_delayus(15);

    SCCB_SCK_H();
    SCCB_delayus(15);

    SCCB_SCK_L();
    SCCB_delayus(15);
    return OK;
}

//////////////////////////////////////
/**
*	@brief д��һ���ֽڵ����ݵ�SCCB
*	@parma д������
*	@return  ״̬��
*/
unsigned char SCCBwriteByte(unsigned char m_data)
{
    unsigned char j, tem;

    for(j = 0; j < 8; j++) //ѭ��8�η�������
    {
        if(m_data & 0x80)
        {
            SCCB_SDA_H();
        }
        else
        {
            SCCB_SDA_L();
        }
				m_data <<= 1;
        SCCB_delayus(15);
        SCCB_SCK_H();
        SCCB_delayus(15);
        SCCB_SCK_L();
    }			
    SCCB_delayus(5);
    SCCB_SDA_IN;
    SCCB_delayus(10);
    SCCB_SCK_H();
    SCCB_delayus(25);
    if(SCCB_SDA_STATE)  //ACK
    {
        tem = 0;
    }
    else tem = 1;
    SCCB_SCK_L();
    SCCB_delayus(10);
    SCCB_SDA_OUT;
    return(tem);
}

//////////////////////////////////////
/**
*	@brief һ���ֽ����ݶ�ȡ���ҷ���
*	@return  ��ȡ������
*/
unsigned char SCCBreadByte(void)
{
    unsigned char read, j;
    read = 0x00;

    SCCB_SDA_IN;/*����SDAΪ����*/
    SCCB_delayus(10);
    for(j = 8; j > 0; j--) //ѭ��8�ν�������
    {
        read <<= 1;
				SCCB_SCK_L();
				SCCB_delayus(10);
        SCCB_SCK_H();
        SCCB_delayus(10);
        if(SCCB_SDA_STATE)
        {
            read |= 0x01;
        }
        SCCB_delayus(2);
    }
		SCCB_SCK_L();
    SCCB_SDA_OUT;/*����SDAΪ���*/
    return(read);
}

//////////////////////////////////////
/**
*	@brief   ����д2��iic
*	@return  ״̬��
*/
Status SCCB_WriteRegister(unsigned char regID, unsigned char regDat)
{
    startSCCB();
    if(0 == SCCBwriteByte(DEV_ADDR)) //д��ַ
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);

    if(0 == SCCBwriteByte(regID)) //�Ĵ���ID
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);

    if(0 == SCCBwriteByte(regDat)) //д���ݵ�������
    {
        stopSCCB();
        return 0;
    }

    stopSCCB();//����SCCB ����ֹͣ��������

    return(1);//�ɹ�����
}

//////////////////////////////////////
/**
*	@brief   ����д2��iiC(���)
*	@return  ״̬��
*/
Status SCCB_WriteDoubleRegister(unsigned char regID, unsigned char regDat)
{
    int i = 0;
    while(0 == SCCB_WriteRegister(regID, regDat))
    {
        i++;
        if(i == 20)
        {
            return 0 ;
        }
    }
    return 1;
}

/**
*	@brief   ����д2��iic
*	@return  ״̬��
*/
Status SCCB_ReadRegister(unsigned char* pBuffer, unsigned int Length, unsigned char regID)
{
		unsigned char ReadData=0;
    startSCCB();
		ReadData = SCCBwriteByte(DEV_ADDR);
    if(0 == ReadData) //д��ַ
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);
		
		ReadData = SCCBwriteByte(regID);
    if(0 == ReadData) //�Ĵ���ID
    {
        stopSCCB();
        return 0;
    }
		stopSCCB();
    DelayUs(20);
		
		startSCCB();
		ReadData = SCCBwriteByte(DEV_ADDR + 1);
    if(0 == ReadData) //����ַ
    {
        stopSCCB();
        return 0;
    }
    DelayUs(20);
		
		while(Length){
			*pBuffer = SCCBreadByte();
			if(1 == Length){
				noAck();
			}else{
				Ack();
			}
			pBuffer++;
			Length--;
		}
    return(1);//�ɹ�����
}

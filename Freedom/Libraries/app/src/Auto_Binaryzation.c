#include "Auto_Binaryzation.h"
short Threshold_5116=300;

/**
*	@brief DAC��ʼ��
*  DAC0����
*	@return  ״̬
*/
Status DAC_Ref_Init(void)
{

    DAC_InitTypeDef DAC_InitStruct = {0};
    DAC_InitStruct.bufferMode = kDAC_Buffer_Swing; /*Bufferҡ��ģʽ �������ǲ� */
    DAC_InitStruct.instance = HW_DAC0;
    DAC_InitStruct.referenceMode = kDAC_Reference_2; /* ʹ��VDDA��Ϊ�ο�Դ */
    DAC_InitStruct.triggerMode = kDAC_TriggerSoftware; /*������� */
    DAC_Init(&DAC_InitStruct);
    return  OK;

}

/**
*	@brief   ��ֵ����
*	 ��Χ��0-1024
*	@return  ״̬
*/
Status DAC_Set_Threshold(uint32_t Threshold)
{
    uint32_t i;
    uint16_t value[16];
    if(Threshold > 1024)
        return ERROR;
    for(i = 0; i < 16; i++)
    {
        value[i] = Threshold * 64 / 16; //��ģת��������
    }
    DAC_SetBufferValue(HW_DAC0, value, 16); /*д��DAC buffer ���д��16�� uint16_t ��ת��ֵ */
    return OK;
}

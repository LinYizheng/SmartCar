#include "ov7725.h"
#include "image.h"

static OV7725_CallBackType OV7725_CallBackTable[2] = {0};
char OV7725_pro = IMAGE_PROCESS_OK;
uint8_t  OV7725_IMAGEB[HIGH_D][WIDTH_D / 8];	//DMA��������
uint16_t Current_Rol = 0;    //�����м���
uint8_t Image_Flag = FALSE;

//////////////////////////////////////
/**
* @breif ��������ͼ����
**/
Status Array_ImageDeal(uint8_t SrcImage[][(WIDTH_D / 8)],uint8_t Dst[][WIDTH_D]){
	uint8_t Color[2]={0xFF,0x00};
	uint8_t Len = HIGH_D;
	while(Len--)
	{
		for(uint8_t i=0;i<(WIDTH_D / 8);i++){
			Dst[Len][0 + i*8] = Color[ (SrcImage[Len][i] >> 7) & 0x01 ];	
			Dst[Len][1 + i*8] = Color[ (SrcImage[Len][i] >> 6) & 0x01 ];
			Dst[Len][2 + i*8] = Color[ (SrcImage[Len][i] >> 5) & 0x01 ];
			Dst[Len][3 + i*8] = Color[ (SrcImage[Len][i] >> 4) & 0x01 ];
			Dst[Len][4 + i*8] = Color[ (SrcImage[Len][i] >> 3) & 0x01 ];		
			Dst[Len][5 + i*8] = Color[ (SrcImage[Len][i] >> 2) & 0x01 ];
			Dst[Len][6 + i*8] = Color[ (SrcImage[Len][i] >> 1) & 0x01 ];
			Dst[Len][7 + i*8] = Color[ (SrcImage[Len][i] >> 0) & 0x01 ];
		}
	}
	return OK;
}

//////////////////////////////////////
/**
* @breif ms��ʱ
**/
static void OV7725_delay_ms(unsigned int ms){
	for(int i=0;i<ms;i++)
		for(int j=800;j>0;j--);
}

//////////////////////////////////////
/**
* @breif ��ʼ�ɼ�ͼ��
**/
Status StartGetImage(void){
	Clear_IT_Pending(OV7725_HREFC_PORT);
	Clear_IT_Pending(OV7725_VSYNC_PORT);
	NVIC_EnableIRQ(OV7725_VSYNC_IRQ);
	NVIC_EnableIRQ(OV7725_HREFC_IRQ);
	return OK;
}

//////////////////////////////////////
/**
* @breif ���жϻص�����ִ��
**/
char OV7725_HREF_PROCESS(char Sta){
	Current_Rol++;
	if(Current_Rol < HIGH_D)
	{	
		DMA_EnableRequest(OV7725_DMA_CHANNAL);
	}
	else
	{
		DMA_EnableRequest(OV7725_DMA_CHANNAL);	//���һ��Ҳ��Ҫ����
		Current_Rol = 0;
		Image_Flag = OK;
	}
  return OK;
}

//////////////////////////////////////
/**
* @breif ���жϻص�����ִ��
**/
char OV7725_VREF_PROCESS(char Sta){
	DMA_SetDestAddress(OV7725_DMA_CHANNAL, (uint32_t)OV7725_IMAGEB);
  return OK;
}
//////////////////////////////////////
/**
* @breif ���ж�ע��ص�
**/
void OV7725_Href_CallbackInstall(OV7725_CallBackType AppCBFun){
	if(NULL != AppCBFun){
		OV7725_CallBackTable[1] = AppCBFun;
	}
}

//////////////////////////////////////
/**
* @breif ���ж�ע��ص�
**/
void OV7725_Vref_CallbackInstall(OV7725_CallBackType AppCBFun){
	if(NULL != AppCBFun){
		OV7725_CallBackTable[0] = AppCBFun;
	}
}

//////////////////////////////////////
/**
* @breif ���ж�ִ��
**/
void OV7725_VREF_ISR(uint32_t array){
	if((array & (1 << OV7725_VSYNC))) //���ж�
  {
		OV7725_CallBackTable[0](OV7725_pro);
	}
}

//////////////////////////////////////
/**
* @breif ���ж�ִ��
**/
void OV7725_HREF_ISR(uint32_t array){
	if((array & (1 << OV7725_HREFC))) //���ж�
  { 
		OV7725_CallBackTable[1](OV7725_pro);
	}
}

//////////////////////////////////////
/**
* @breif DMA�������
**/
void DMA_ISR(void){
	DMA0->TCD[OV7725_DMA_CHANNAL].CSR |= DMA_CSR_ESG_MASK;
}

//////////////////////////////////////
/**
* @breif OV7725�Ĵ�����ʼ��
**/
Status OV7725_REG_Init(void){
	unsigned char SensorIdCode = 0;
	unsigned char read_OV7725_flag = 0;
	
	do{
		read_OV7725_flag = SCCB_WriteDoubleRegister(OV7725_COM7,0x80);
	}while(read_OV7725_flag == 0);
	
	OV7725_delay_ms(5);
	
	
	do{
		read_OV7725_flag = SCCB_ReadRegister(&SensorIdCode,1,OV7725_VER);
	}while(read_OV7725_flag == 0);
	
	/*�豸����֤*/
	if(SensorIdCode == OV7725_ID){
		for(unsigned int i=0;i<RegSize;i++){
			/*����д��ֱ��д�ɹ�*/
			do{
				read_OV7725_flag = SCCB_WriteDoubleRegister(OV7725_reg_config[i].ADDR,OV7725_reg_config[i].DATA);
			}while(read_OV7725_flag == 0);
		}
		
		OV7725_delay_ms(5);

		do{
				read_OV7725_flag = SCCB_WriteDoubleRegister(OV7725_CNST,Threshold);
		}while(read_OV7725_flag == 0);

	}
	OV7725_delay_ms(5);
	
	return OK;
}

//////////////////////////////////////
/**
* @breif DMA��������
**/
Status OV7725_DMA_Init(void){
	DMA_InitTypeDef DMA_InitStruct1 = {0};

	DMA_InitStruct1.chl =OV7725_DMA_CHANNAL;
	DMA_InitStruct1.chlTriggerSource = OV7725_PCLK_PORT_DMA;//DMAͨ��
	DMA_InitStruct1.triggerSourceMode = kDMA_TriggerSource_Normal;
	DMA_InitStruct1.minorLoopByteCnt = 1;
	DMA_InitStruct1.majorLoopCnt = (WIDTH_D / 8);	//һ�δ���һ��

  DMA_InitStruct1.sAddr = (uint32_t) & (OV7725_DATA_PORT_DMA->PDIR); //Դ��ַ
  DMA_InitStruct1.sLastAddrAdj = 0;
  DMA_InitStruct1.sAddrOffset = 0;
  DMA_InitStruct1.sDataWidth = kDMA_DataWidthBit_8;//8λ
  DMA_InitStruct1.sMod = kDMA_ModuloDisable;

  DMA_InitStruct1.dAddr = (uint32_t)OV7725_IMAGEB;//Ŀ�ĵ�ַ
  DMA_InitStruct1.dLastAddrAdj = 0;
  DMA_InitStruct1.dAddrOffset = 1;
  DMA_InitStruct1.dDataWidth = kDMA_DataWidthBit_8;//8λ
  DMA_InitStruct1.dMod = kDMA_ModuloDisable;
  DMA_Init(&DMA_InitStruct1);

  DMA_CallbackInstall(OV7725_DMA_CHANNAL, DMA_ISR);//DMA�����ж�
  DMA_ITConfig(OV7725_DMA_CHANNAL, kDMA_IT_Major);//ʹ���ж�
	NVIC_EnableIRQ(OV7725_DMA_IRQ);
  return OK;
}
//////////////////////////////////////
/**
* @breif ���жϳ�ʼ��
**/
Status OV7725_Init(void){
	GPIO_QuickInit(OV7725_VSYNC_PORT, OV7725_VSYNC,   kGPIO_Mode_IPU);
  GPIO_QuickInit(OV7725_HREFC_PORT,  OV7725_HREFC,  kGPIO_Mode_IPD);
  GPIO_QuickInit(OV7725_PCLK_PORT,  OV7725_PCLK,    kGPIO_Mode_IPD);
	
	SCCB_GPIO_Config();
  OV7725_REG_Init();
  OV7725_DMA_Init();
	
	/*��,���ж�ע��*/
  GPIO_CallbackInstall(OV7725_VSYNC_PORT,  OV7725_VREF_ISR);
  GPIO_CallbackInstall(OV7725_HREFC_PORT,  OV7725_HREF_ISR);
	OV7725_Vref_CallbackInstall(OV7725_VREF_PROCESS);
	OV7725_Href_CallbackInstall(OV7725_HREF_PROCESS);
	
	/*���ж�,���ж�,PCLK�������ش���*/
  GPIO_ITDMAConfig(OV7725_VSYNC_PORT,  OV7725_VSYNC, kGPIO_IT_FallingEdge);	//Ϊ�˻�ȡ�����ʱ���ѹ��
  GPIO_ITDMAConfig(OV7725_HREFC_PORT,  OV7725_HREFC,  kGPIO_IT_RisingEdge);
  GPIO_ITDMAConfig(OV7725_PCLK_PORT,   OV7725_PCLK,  kGPIO_DMA_FallingEdge);	//DMA�����½��ش���

  /*��ֹ�ж�*/
  NVIC_DisableIRQ(OV7725_VSYNC_IRQ);
  NVIC_DisableIRQ(OV7725_HREFC_IRQ);
  NVIC_DisableIRQ(OV7725_DMA_IRQ);

  /* ��ʼ�����ݶ˿� */
  for(int i = 0; i < 8; i++)
  {
      GPIO_QuickInit(OV7725_DATA_PORT, i, kGPIO_Mode_IFT);
  }
	return OK;
}

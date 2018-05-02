/*****************************************
�� �� ����LED.h
�� �� �ߣ�������ӿƼ�
�������ڣ�2012/11/08
��    ����LED128x64��ʾ���ײ�����
------------------------------------------
*****************************************/

#ifndef __OLED_H__
#define __OLED_H__

#include "common.h"
#include "gpio.h"
#include "Status.h"
#include "ov5116.h"
#include "Image.h"

/*****************************************
���� �� ��:��ʼ�����Ŷ���

��˵ �� ��:ֱ���������IO�Ϳ�����
*******************************************/
#define LED_PORT HW_GPIOE
#define SCL_PIN 27
#define SDA_PIN 26
#define RST_PIN 25
#define DC_PIN  24

#define LED_SCL_Init  GPIO_QuickInit(LED_PORT,SCL_PIN,kGPIO_Mode_OPP)//ʱ�ӳ�ʼ������
#define LED_SDA_Init  GPIO_QuickInit(LED_PORT,SDA_PIN,kGPIO_Mode_OPP)//���ݿ�D1
#define LED_RST_Init  GPIO_QuickInit(LED_PORT,RST_PIN,kGPIO_Mode_OPP)//��λ���ܵ�ƽ
#define LED_DC_Init   GPIO_QuickInit(LED_PORT,DC_PIN ,kGPIO_Mode_OPP)//ƫ�ó���
/**********************************************

���� �� ��:IO ���� OUTPUT

��˵ �� ��:ֱ���������IO�Ϳ�����
*******************************************/
#define LED_SCLH  GPIO_WriteBit(LED_PORT,SCL_PIN,1)// ʱ�Ӷ��� 
#define LED_SCLL  GPIO_WriteBit(LED_PORT,SCL_PIN,0)

#define LED_SDAH  GPIO_WriteBit(LED_PORT,SDA_PIN,1)//���ݿ�D0
#define LED_SDAL  GPIO_WriteBit(LED_PORT,SDA_PIN,0)

#define LED_RSTH  GPIO_WriteBit(LED_PORT,RST_PIN,1)//��λ���ܵ�ƽ
#define LED_RSTL  GPIO_WriteBit(LED_PORT,RST_PIN,0)

#define LED_DCH   GPIO_WriteBit(LED_PORT,DC_PIN,1)
#define LED_DCL   GPIO_WriteBit(LED_PORT,DC_PIN,0)//ƫ�ó���

/************************************************/
#define LED_IMAGE_WHITE       1
#define LED_IMAGE_BLACK       0

#define LED_MAX_ROW_NUM      60
#define LED_MAX_COLUMN_NUM   80

#define LED_MAX_ROW_NUMMT     60
#define LED_MAX_COLUMN_NUMMT  80

void LEDPIN_Init(void);   //LED�������ų�ʼ��
void LED_Init(void);
void LED_CLS(void);
void LED_Set_Pos(uint8_t x, uint8_t y); //�������꺯��
void LED_WrDat(uint8_t data);   //д���ݺ���
void LCD_Rectangle(uint8_t x1,uint8_t y1);//ʵ�ľ���
void LCD_ClearRectangle(uint8_t x1,uint8_t y1);
void LED_P6x8Char(uint8_t x, uint8_t y, uint8_t ch);
void LED_P6x8Str(uint8_t x, uint8_t y, uint8_t ch[]);
void LED_P8x16Str(uint8_t x, uint8_t y, uint8_t ch[]);
void LED_P14x16Str(uint8_t x, uint8_t y, uint8_t ch[]);
void LED_PXx16MixStr(uint8_t x, uint8_t y, uint8_t ch[]);
void LED_PrintBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t bmp[]);
void LED_Fill(uint8_t dat);
void LED_PrintValueC(uint8_t x, uint8_t y, signed char data);
void LED_PrintValueI(uint8_t x, uint8_t y, int data);
void LED_PrintValueF(uint8_t x, uint8_t y, float data, uint8_t num);
void LED_PrintEdge(void);
void LED_Cursor(uint8_t cursor_column, uint8_t cursor_row);
void LED_PrintLine(void);
void LED_PrintImage(uint8_t pucTable[][LED_MAX_COLUMN_NUM]);//*pucTable, uint16 usRowNum, uint16 usColumnNum
void LED_PrintImageMT(uint8_t pucTable[][LED_MAX_COLUMN_NUMMT], int threshold);
#endif

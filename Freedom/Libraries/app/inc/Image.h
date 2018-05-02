#ifndef _IMAGE_H_
#define _IMAGE_H_

#ifdef __cpluscplus
extern "C"{
#endif

/**
* @brief:ͷ�ļ�
**/
#include "stdio.h"
#include "common.h"
#include "Status.h"
#include "OLED.h"
#include "control.h"
	
/**
* @brief:�û���������UDT
**/
#define TLSMLength 20	//��С����
#define ForecastTHSM(in,k,b)  (int)((in)*(k)+(b))
#ifdef _MSC_VER
#pragma pack(push,4)
#endif
typedef struct
{
	int XYIntegral;
	int XIntegral;
	int YIntegral;
	int XXIntegral;
	float k;//б��
	float b;//�ؾ�
	int PointX[TLSMLength];
	int PointY[TLSMLength];
}TLSM;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push,4)
#endif
typedef struct{
	int LeftBeginFilter;
	int RightBeginFilter;
	int LeftValidLine;
	int RightValidLine;
	int ValidEndLine;
	int LoseLineFlag;
}BeginLine;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

//ͼ�������ö�ٳ���
typedef enum{
	LeftLose,
	RightLose,
	DoubleLose,
	NoneLose
}SearchFlag;

typedef enum{
	None,
	Left,
	Right,
	Center
}Site;

//ͼ������ر���
#define CAMERA_W 80	//�곣��---������
#define CAMERA_H 60

extern unsigned char OV7725_IMAGE[CAMERA_H][CAMERA_W];
#define img(i,j) OV7725_IMAGE[(CAMERA_H - (i) - 1)][(j)]
extern int LeftEdge[CAMERA_H];
extern int RightEdge[CAMERA_H];
extern int CenterGuide[CAMERA_H];
extern int CenterErr[CAMERA_H];
#define LeftEdgeGo(i) LeftEdge[(CAMERA_H - 1 - (i))]
#define RightEdgeGo(i) RightEdge[(CAMERA_H - 1 - (i))]
#define CenterGuideGo(i) CenterGuide[(CAMERA_H - 1 - (i))]
#define CenterErrGo(i) CenterErr[(CAMERA_H - 1 - (i))]

//extern int ValidEndShow;
//extern int LeftStartShow;
//extern int RightStartShow;
//extern int CenterControl;
extern int CenterGuideK;
extern int StStartLine;
extern int IsRamp;
extern int ObIsObStacle;

//ͼ������
Status ProcessImage(void);
int myround(float f);
float my_sqrt(float number);
Status InitTLSMVaribles(TLSM *_TLSM, int _PointX[], int _PointY[], int _Length);
Status CalculateTLSM(TLSM *_TLSM, int _PointX[], int _PointY[], int _Length);
Status LinearLine(TLSM *_TLSM, int CurrentLine, int Length, Site Flag);
Status StartLineFind(int Col, int VStart, int VEnd, Site LocSite, \
	int *FirstFindFlag, int *LastFindFlag, \
	int *Bottom, int *Top, int *MidLineValue, int *VLineValue);
Status DetectStartLine1(int VStart, int VEnd, int HCenter);
Status CalculateOneLine(int OneLinePoint[], float *OneLinek, float *OneLineb);
Status InitMainVariables(void);
Status InitBeginLine(BeginLine *_BeginLine);
Status SearchOneHorrizonEdge(int CurrentLine, int HStart, int HEnd, int HCenter, Site LocSite);
Status SearchDoubleEdge(int VStart, int VEnd, int HStart, int HEnd, BeginLine *_BeginLine);
Status SearchObStacleFromMid(int CurrentLine,
	int HStart, int HEnd, int VEnd,
	Site* SearchFlag, int* ObEdge, int* ObOtherEdge,
	int* ObFirstLine, int* ObEndLine, int* ObCount);
Status ReviseObStacleFromMid(Site LocSite, int* ObEdge, int ObFirstLine, int ObEndLine);
Status ReviseSetDot(int CurrentLine, int Len, int HStart, int HEnd, Site LocSite);
Status ReviseBaseLine(int CurrentLine, Site LocSite);
Status SupplyOneHorrizon(int CurrentLine, int HStart, int HEnd, Site LocFlag, int *SearchFlag);
Status SupplyLineDotLoc(int CurrentLine, int HStart, int HEnd, Site LocSite, int* OverFlag);
Status isJumpDot(int CurrentLine, Site LocSite);
Status SupplyMidLine(int CurrentLine, int Diff,
	int LeftEdgeFlag, int RightEdgeFlag,
	int HStart, int HEnd);
Status LoseEdgeDeal(int CurrentLine, int Diff, int HStart, int HEnd, Site LocFlag);
Status DetectStartErr(int CurrentLine, BeginLine *_BeginLine, Site LocFlag);
Status ReviseStartErr(int CurrentLine, Site LocSite, BeginLine *_BeginLine);
Status SearchEndLine(BeginLine* _BeginLine);
Status ReviseSharpPatch(int HStart,int HEnd, BeginLine *_BeginLine);
Status ReviseStartValidLine(int HStart, int HEnd, BeginLine* _BeginLine);
Status DetectRampOK(BeginLine* _BeginLine);
Status OneBlackDotFilter(void);
Status ProcessImage(void);
Status GetSteerError(BeginLine *_BeginLine);
Status DetectWayType(BeginLine *_BeginLine, TLSM *_TLSM);
Status ImageFinshed(void);
Status drawLine(int CurrentLine,int Flag);

#ifdef __cplusplus
}
#endif

#endif

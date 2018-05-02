#include "stdafx.h"
#include "Image.h"

unsigned char ov7725_image[CAMERA_H][CAMERA_W] = { 0 };
unsigned char ov7725_image_show[CAMERA_H][CAMERA_W] = { 0 };
////////////////////////////////////////////
/**
* @brief����߽�/�ұ߽�/����
*       �� �����м�ɨ��ֵ
**/
int LeftEdge[CAMERA_H] = { 0 };
int RightEdge[CAMERA_H] = { 0 };
int CenterGuide[CAMERA_H] = { 0 };
int CenterErr[CAMERA_H] = { 0 };
////////////////////////////////////////////
/**
* @brief����ֵ
*       :�жϵ�Ϊ�ڻ��
**/
#define JudgeWhite(point) ((point) >= 1)
#define JudgeBlack(point) ((point) <  1)
////////////////////////////////////////////
BeginLine _BeginLine;
TLSM LeftTLSM, RightTLSM, CenterTLSM;
TLSMXX CenterTLSMXX;
int ObIsObStacle = 0;    //Lock---�����ϰ���
int ObValidLine = 0;     //Lock---�ϰ������������һ�У�������Ч��
int StStartLine = 0; //Lock---�����߱�־λ
static int TimeCount = 0;	//Lock---������ֹ������ʱ�غϣ�ֻ��ͣʹ��
static int TimeOtherCount = 0; //Lock---������ֹ������ʱ�غϣ�ֻ��ͣʹ��
int IsRamp = 0;//Lock---�ж�Ϊ���µı�־λ
#define LEFT_EDGE 3   //��ֹ��---0 + 3
#define RIGHT_EDGE 76 //��ֹ��---CAMERA_W - 1 - 3
#define CENTER_GUIDE 40 //(CAMERA_W / 2)	//Lock---��������ֵ
#define SEARCH_TIMES 7 //Lock---����һ�ж��ַ�����---CAMEARA_W/2/2/2/2/2/2=1
#define START_ROW 1	//Lock---��ʼ������
#define END_ROW 59 //(CAMERA_H - 1 - 6)	//Lock---��ֹ������
#define START_COL 2	//Lock---��ʼ������
#define END_COL 77 //(CAMERA_W - 2 - 1)	//Lock---��ֹ������
#define FILTER_MIN 0		//Lock---����˲���Сֵ
#define FILTER_MAX 20	//Lock---����˲����ֵ
#define StVStart 1	//Lock---������������ʼ��
#define StVEnd 51   //Lock---������������ֹ��
const int LoseEdgeWidth[60] = {	//���߿��
	36,35,34,34,33,33,32,32,31,31,
	30,30,29,29,28,28,27,27,26,26,
	25,24,24,23,23,22,21,21,20,20,
	20,19,18,18,17,17,16,15,15,14,
	13,13,12,12,11,11,10,10, 9, 9,
	 8, 7, 7, 6, 6, 5, 5, 4, 4, 2
};//Lock---���߶��߲��߿��
int CenterGuideK = 0; //Lock---������б�ʼ���
int ValidEndShow = _BeginLine.ValidEndLine; //Unlock---��ֹ��Ч��
int LeftStartShow = _BeginLine.LeftValidLine;	//UnLock---����ʼ��Ч��
int RightStartShow = _BeginLine.RightValidLine; //UnLock---����ʼ��Ч��
int CenterControl = 0;	//UnLock---������
////////////////////////////////////////////
//Lock
/**
*	@brief   ͼ��ɼ��������
*	@return  ״̬
*/
Status ImageFinshed(void){
	ProcessImage();
	CenterGuideK = DetectWayType(&_BeginLine);
	CenterControl = GetSteerError(&_BeginLine);
	ValidEndShow = _BeginLine.ValidEndLine;
	LeftStartShow = _BeginLine.LeftValidLine;
	RightStartShow = _BeginLine.RightValidLine;
	return 'T';
}
////////////////////////////////////////////
//Lock
/**
* @breif ���ת��ƫ��ֵ
**/
Status GetSteerError(BeginLine *_BeginLine) {
	//1.0m-44
	//1.2m-47
	//1.5m-49
	//2.0m-50
	//2.2m-51
	//2.5m-52
	int		EveryPartNum = 0;
	int IntCenterK = 0;
	int		PartCount = 5, OtherCount = 0, i = 0, j = 0;
	float	CenterSum = 0, CenterWeight = 0, CenterData = 0;
	int CenterValidLine = _BeginLine->ValidEndLine;
	static float LastErr = 0;
	//���һ�� 195 ������
	//125 �ող���
	//75 �е��϶�����
	//int PartWeight[8] = { 1, 1, 1, 1, 1, 1,1, 1 };
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 155, 185 };	//125 195
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 155, 125 };
	 int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 155, 95 };
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 75, 65 };
	//int PartWeight[8] = { 1, 1, 5, 5, 55, 105, 75,  1 };

	/*����6���򱣳�ԭ������*/
	if (CenterValidLine < 6) {
		return (int)LastErr;
	}

	/*�ϵ㴦��Ϊ�ӳ�*/
#if 0
	IntCenterK = abs(CenterGuideK);
	//С�䴦��Ϊ�ӳ�
	if ((CenterValidLine > 45 && IntCenterK > 65 && IntCenterK < 120)
		) {	//�����г��Ȳ���
		LinearLine(&CenterTLSM, CenterValidLine - 5, 5, Center);
		_BeginLine->ValidEndLine = END_ROW - 1;
		for (int n = 1; n < (END_ROW - CenterValidLine); n++) {
			CenterGuideGo(CenterValidLine + n) = \
				ForecastTHSM(CenterValidLine + n, CenterTLSM.k, CenterTLSM.b);
			if (CenterGuideGo(CenterValidLine + n) <= START_COL) CenterGuideGo(CenterValidLine + n) = START_COL;
			if (CenterGuideGo(CenterValidLine + n) >= END_COL) CenterGuideGo(CenterValidLine + n) = END_COL;
		}
	}
#endif

	for (int n = START_ROW; n <= _BeginLine->ValidEndLine; n++)
	{
		CenterErrGo(n) = CenterGuideGo(n) - CENTER_GUIDE;
	}//���ƫ��ֵ����

	if (_BeginLine->ValidEndLine > 39)
		PartCount += (_BeginLine->ValidEndLine + 1 - 39) / 5 + 1;
	EveryPartNum = (_BeginLine->ValidEndLine + 1 - START_ROW) / PartCount;
	OtherCount = (_BeginLine->ValidEndLine + 1 - START_ROW) % EveryPartNum;
	
	if (OtherCount == 0 && PartCount < 8) PartCount++;
	if (PartCount > 8) {
		OtherCount += EveryPartNum * (PartCount - 8);
		PartCount = 8;
	}

	for (i = 0; i < PartCount; i++)
	{
		for (j = 0; j<EveryPartNum; j++)
		{
			CenterSum += (float)(CenterErrGo(i * EveryPartNum + j + START_ROW))* PartWeight[i];
			CenterWeight += PartWeight[i];
		}
	}//���岿��

	for (j = 0; j < OtherCount; j++)
	{
		CenterSum += (float)CenterErrGo(PartCount * EveryPartNum + j + START_ROW) * PartWeight[PartCount - 1];
	}//�ֲ�
	CenterWeight += OtherCount * PartWeight[PartCount - 1];
	CenterData = CenterSum / CenterWeight;
	LastErr = CenterData;
	return (int)CenterData;
}
////////////////////////////////////////////
//Lock
/**
* @brief:�ж���������---����״̬InWay
* @param[in]:��ʼ������Ч�к�---VStart
* @param[in]:��ֹ������Ч�к�---VEnd
* @param[in]:��С�����û��Զ���---CenterTLMS
* @return:״̬
**/
Status DetectWayType(BeginLine *_BeginLine) {
	//��̬�Էֳ�10�Σ�ÿ��ȡ�е�,��಻����12��
	int PartCount = 8;
	int StraightCons = 59;	//ֱ�������ʳ���
	int PointNum = 0, EveryPart = 0;
	float ThisSlope = 0.0f, LastSlope = 0.0f;
	int CenterPartX[8] = { 0 }, CenterPartY[8] = { 0 };
	int SamplePointX[3] = { 0 }, SamplePointY[3] = { 0 };
	double La = 0.0f, Lb = 0.0f, Lc = 0.0f;
	double Cabc = 0.0f, Sabc = 0.0f;
	PointNum = _BeginLine->ValidEndLine + 1 - START_ROW;
	//�����
	if (PointNum < 10) return (int)(LastSlope * 100.0f) - StraightCons;
	EveryPart = PointNum / PartCount;
	for (int i = 0; i < PartCount; i++) {
		CenterPartX[i] = START_ROW + i*EveryPart + EveryPart / 2 + 1;
		CenterPartY[i] = CenterGuideGo(CenterPartX[i]);
	}
	//�������
	CalculateTLSMXX(&CenterTLSMXX, CenterPartX, CenterPartY, PartCount);
	//��������
	SamplePointX[0] = START_ROW;
	SamplePointY[0] = ForecastTHSMXX(SamplePointX[0], CenterTLSMXX.a, CenterTLSMXX.c);
	SamplePointX[2] = _BeginLine->ValidEndLine;
	SamplePointY[2] = ForecastTHSMXX(SamplePointX[2], CenterTLSMXX.a, CenterTLSMXX.c);
	SamplePointX[1] = (SamplePointX[0] + SamplePointX[2]) / 2;
	SamplePointY[1] = ForecastTHSMXX(SamplePointX[1], CenterTLSMXX.a, CenterTLSMXX.c);
	//�����α߳�����
	La = sqrt(double((SamplePointX[0] - SamplePointX[1]) * (SamplePointX[0] - SamplePointX[1])
		+ (SamplePointY[0] - SamplePointY[1]) * (SamplePointY[0] - SamplePointY[1])));
	Lb = sqrt(double((SamplePointX[0] - SamplePointX[2]) * (SamplePointX[0] - SamplePointX[2])
		+ (SamplePointY[0] - SamplePointY[2]) * (SamplePointY[0] - SamplePointY[2])));
	Lc = sqrt(double((SamplePointX[1] - SamplePointX[2]) * (SamplePointX[1] - SamplePointX[2])
		+ (SamplePointY[1] - SamplePointY[2]) * (SamplePointY[1] - SamplePointY[2])));
	Cabc = La + Lb + Lc;
	Sabc = sqrt(Cabc*(Cabc - La)*(Cabc - Lb)*(Cabc - Lc));
	ThisSlope = (float)((4 * Sabc) / (La*Lb*Lc));
	LastSlope = ThisSlope;
	return (int)(ThisSlope * 100.0f) - StraightCons;
}
////////////////////////////////////////////
//Lock               
/**
* @brief:ͼ�������
* @return:״̬
**/
Status ProcessImage(void){
	/*---ʱ���б��������ʶ��---*/
	TimeOtherCount++;
	if (TimeOtherCount >= 50){	//1s 50֡
		TimeOtherCount = 0;
		TimeCount++;
	}
	if (TimeCount >= 80) TimeCount = 80;
	if (TimeCount >= 3 && 0 == StStartLine){
		StStartLine = DetectStartLine1(StVStart, StVEnd, CENTER_GUIDE);
	}
	/*----������ʶ��-----------*/
	ObIsObStacle = 0;
	IsRamp = 0;
	StStartLine = 0;
	//===�����Ƿ�����ϰ���
	InitMainVariables();
	SearchDoubleEdge(START_ROW, END_ROW, START_COL, END_COL, &_BeginLine);
	ReviseStartValidLine(START_COL, END_COL, &_BeginLine);
	SearchEndLine(&_BeginLine);
	//===�����Ƿ�����µ�
	IsRamp = DetectRampOK(&_BeginLine);
	//===�����Ƿ����������
	StStartLine = DetectStartLine1(StVStart, StVEnd, CENTER_GUIDE);
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:��������
* @param[in]:����С��---ÿ��һ�κ����������Զ�����
* @return:������
**/
int myround(float f)
{
	if ((int)f + 0.5f>f)
		return (int)f;
	else return (int)f + 1;
}
////////////////////////////////////////////
//Lock
/**
* @brief:��ʼ����С���˲���
* @param[in]:��������������
* @return:״̬
**/
Status InitTLSMVaribles(TLSM *_TLSM, int _PointX[], int _PointY[], int _Length){
	_TLSM->XIntegral = 0;
	_TLSM->YIntegral = 0;
	_TLSM->XXIntegral = 0;
	_TLSM->XYIntegral = 0;
	_TLSM->k = 0.0f;
	_TLSM->b = 0.0f;
	for (int i = 0; i<_Length; i++){
		_TLSM->PointX[i] = _PointX[i];
		_TLSM->PointY[i] = _PointY[i];
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:������С���ˣ�����б��k,b
* @param[in]:��������������
* @return:״̬
**/
Status CalculateTLSM(TLSM *_TLSM, int _PointX[], int _PointY[], int _Length){
	int Denominator = 0;
	InitTLSMVaribles(_TLSM, _PointX, _PointY, _Length);
	for (int i = 0; i < _Length; i++)
	{
		_TLSM->XIntegral += _TLSM->PointX[i];
		_TLSM->YIntegral += _TLSM->PointY[i];
		_TLSM->XYIntegral += _TLSM->PointX[i] * _TLSM->PointY[i];
		_TLSM->XXIntegral += _TLSM->PointX[i] * _TLSM->PointX[i];
	}
	Denominator = _Length * _TLSM->XXIntegral - _TLSM->XIntegral*_TLSM->XIntegral;
	if (abs(Denominator) < 0.00001) {
		return 1;	//��ֱ��X���ֱ��
	}
	_TLSM->k = (float)(_Length * _TLSM->XYIntegral - _TLSM->XIntegral*_TLSM->YIntegral) / (float)Denominator;	//б��K
	_TLSM->b = (float)(_TLSM->XXIntegral*_TLSM->YIntegral - _TLSM->XIntegral *_TLSM->XYIntegral) / (float)Denominator;	//�ؾ�b
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:��ʼ����С���˲���
* @param[in]:��������������
* @return:״̬
**/
Status InitTLSMXXVaribles(TLSMXX *_TLSMXX, int _PointX[], int _PointY[], int _Length) {
	_TLSMXX->XIntegral = 0;
	_TLSMXX->YIntegral = 0;
	_TLSMXX->XXIntegral = 0;
	_TLSMXX->XYIntegral = 0;
	_TLSMXX->XXXIntegral = 0;
	_TLSMXX->a = 0.0f;
	_TLSMXX->c = 0.0f;
	for (int i = 0; i<_Length; i++) {
		_TLSMXX->PointX[i] = _PointX[i];
		_TLSMXX->PointY[i] = _PointY[i];
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:������С���ˣ�����б��k,b
* @param[in]:��������������
* @return:״̬
**/
Status CalculateTLSMXX(TLSMXX *_TLSMXX, int _PointX[], int _PointY[], int _Length) {
	float D = 0.0f, Dx = 0.0f, Dy = 0.0f;
	InitTLSMXXVaribles(_TLSMXX, _PointX, _PointY, _Length);
	for (int i = 0; i < _Length; i++)
	{
		_TLSMXX->XIntegral += _TLSMXX->PointX[i];
		_TLSMXX->YIntegral += _TLSMXX->PointY[i];
		_TLSMXX->XYIntegral += _TLSMXX->PointX[i] * _TLSMXX->PointY[i];
		_TLSMXX->XXIntegral += _TLSMXX->PointX[i] * _TLSMXX->PointX[i];
		_TLSMXX->XXXIntegral += _TLSMXX->PointX[i] * _TLSMXX->PointX[i] * _TLSMXX->PointX[i];
	}
	D = (float)(_Length * _TLSMXX->XXXIntegral - _TLSMXX->XXIntegral * _TLSMXX->XIntegral);
	if ((fabs(D)-0.00001f) < 0) {
		return 1;	//��ֱ��X���ֱ��
	}
	Dx = (float)(_Length * _TLSMXX->XYIntegral - _TLSMXX->XIntegral*_TLSMXX->YIntegral);
	Dy = (float)(_TLSMXX->YIntegral * _TLSMXX->XXXIntegral - _TLSMXX->XYIntegral * _TLSMXX->XXIntegral);
	_TLSMXX->a = Dx / D;
	_TLSMXX->c = Dy / D;
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:���ݱ����ж��Ƿ������С���˲���
* @param[in]:��С�����û��Զ���
* @param[in]:��ǰ��---CurrentLine
* @param[in]:��ĸ���---Length
* @param[in]:����λ��---Flag
* @return:״̬
*/
Status LinearLine(TLSM *_TLSM, int CurrentLine, int Length, Site Flag){
	int PointX[TLSMLength] = { 0 };
	int PointY[TLSMLength] = { 0 };
	if (Left == Flag){
		for (int i = 0; i < Length; i++){
			PointX[i] = CurrentLine + i;
			PointY[i] = LeftEdgeGo(CurrentLine + i);
		}
	}
	else if (Right == Flag){
		for (int i = 0; i < Length; i++){
			PointX[i] = CurrentLine + i;
			PointY[i] = RightEdgeGo(CurrentLine + i);
		}
	}
	else{
		for (int i = 0; i < Length; i++){
			PointX[i] = CurrentLine + i;
			PointY[i] = CenterGuideGo(CurrentLine + i);
		}
	}
	CalculateTLSM(_TLSM, PointX, PointY, Length);	//������С����k,b
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif ����һ�У����жϸ����Ƿ������������
**/
Status StartLineFind(int Col, int VStart, int VEnd, Site LocSite, \
	int *FirstFindFlag, int *LastFindFlag, \
	int *Bottom, int *Top, int *MidLineValue, int *VLineValue){
	int WhiteToBlackFlag = 0, BlackToWhiteFlag = 0;
	int BottomWidth = 0, TopWidth = 0, ToToWidth = 0, MidLine = 0;
	int VLoc = 0;

	for (int Rol = VStart; Rol < VEnd; Rol++){
		if (0 == WhiteToBlackFlag && 0 == *FirstFindFlag)
			if (JudgeWhite(img(Rol, Col)))
				if (JudgeBlack(img(Rol + 1, Col))){
					WhiteToBlackFlag = 1;
					*Bottom = Rol + 1;
				}
		if (1 == WhiteToBlackFlag && 0 == BlackToWhiteFlag && 0 == *FirstFindFlag)
			if (JudgeBlack(img(Rol, Col)))
				if (JudgeWhite(img(Rol + 1, Col))){
					BlackToWhiteFlag = 1;
					*Top = Rol;
					*VLineValue = Col;
					*MidLineValue = myround((float)(*Bottom + *Top) / 2.0f);
					//===Ѱ�ҵ�һ����ʼ�е���������
					if ((*Top - *Bottom) > 6 && *MidLineValue <= 40) return 0;
					if ((*Top - *Bottom) > 2 && *MidLineValue > 40) return 0;
					//===������һ�����޺ڰ�����
					if (Left == LocSite) VLoc = Col - 1;
					else VLoc = Col + 1;
					//===��ɫ��Ĭ���������ߴ���
					if (JudgeWhite(img(*MidLineValue, VLoc))) {
						*FirstFindFlag = 1;
						return 1;
					}
					//===����Ҫ�����ɫ����һ��֮��ܿ��ܾ��а�ɫ
					if (JudgeBlack(img(*MidLineValue, VLoc))
						|| JudgeWhite(img(*MidLineValue, VLoc))){
						int ToBlack = 0, ToWhite = 0;
						int Bottom = *MidLineValue - 5;
						if (Bottom < VStart) Bottom = VStart;
						int Top = *MidLineValue + 5;
						if (Top > VEnd) Top = VEnd;

						for (int n = Bottom; n <= Top; n++){
							if (0 == ToBlack)
								if (JudgeWhite(img(n, VLoc)))
									if (JudgeBlack(img(n + 1, VLoc))){
										ToBlack = 1;
										BottomWidth = n;
									}
							if (1 == ToBlack && 0 == ToWhite){
								if (JudgeBlack(img(n, VLoc)))
									if (JudgeWhite(img(n + 1, VLoc))){
										ToWhite = 1;
										TopWidth = n;
									}
							}
						}
						if (ToBlack == 1 && ToWhite == 1){
							ToToWidth = TopWidth - BottomWidth;
							MidLine = (BottomWidth + TopWidth) / 2;
							if ((abs(*MidLineValue - MidLine) <= 3)
								|| (ToToWidth <= 6 && MidLine <= 40)
								|| (ToToWidth <= 2 && MidLine <= 40)
								){
								*FirstFindFlag = 1;
								return 0;
							}
						}
						if (ToBlack == 0 || ToWhite == 0) {
							return 0;
						}
					}
				}
		if (0 == WhiteToBlackFlag && 0 == *LastFindFlag && 1 == *FirstFindFlag)
			if (JudgeWhite(img(Rol, Col)))
				if (JudgeBlack(img(Rol + 1, Col))){
					WhiteToBlackFlag = 1;
					*Bottom = Rol + 1;
				}
		if (1 == WhiteToBlackFlag && 0 == BlackToWhiteFlag && 0 == *LastFindFlag && 1 == *FirstFindFlag)
			if (JudgeBlack(img(Rol, Col)))
				if (JudgeWhite(img(Rol + 1, Col))){
					BlackToWhiteFlag = 1;
					*Top = Rol;
					*VLineValue = Col;
					*MidLineValue = myround((float)(*Bottom + *Top) / 2.0f);
					//===������һ�����޺ڰ�����
					if (Left == LocSite) VLoc = Col - 1;
					else VLoc = Col + 1;
					if (JudgeWhite(img(*MidLineValue, VLoc))){
						*LastFindFlag = 1;
						return 1;
					}
					if (JudgeBlack(img(*MidLineValue, VLoc))){
						int ToBlack = 0, ToWhite = 0;
						int Bottom = *MidLineValue - 5;
						if (Bottom < VStart) Bottom = VStart;
						int Top = *MidLineValue + 5;
						if (Top > VEnd) Top = VEnd;

						for (int n = Bottom; n <= Top; n++){
							if (0 == ToBlack)
								if (JudgeWhite(img(n, VLoc)))
									if (JudgeBlack(img(n + 1, VLoc))){
										ToBlack = 1;
										BottomWidth = n;
									}
							if (1 == ToBlack && 0 == ToWhite){
								if (JudgeBlack(img(n, VLoc)))
									if (JudgeWhite(img(n + 1, VLoc))){
										ToWhite = 1;
										TopWidth = n;
									}
							}
						}
						if (ToBlack == 1 && ToWhite == 1){
							ToToWidth = TopWidth - BottomWidth;
							MidLine = (BottomWidth + TopWidth) / 2;
							if ((abs(*MidLineValue - MidLine) > 3)
								|| (ToToWidth > 6 && MidLine <= 40)
								|| (ToToWidth > 2 && MidLine > 40)
								){
								*LastFindFlag = 1;
								return 1;
							}
						}
						if (ToBlack == 0 || ToWhite == 0) {
							*LastFindFlag = 1;
							return 1;
						}
					}
				}
	}
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @breif ʶ��������
**/
Status DetectStartLine1(int VStart, int VEnd, int HCenter){
#define LineLimit 40	//���֮�����Ҫ����ֹ����������
	int PicMode = 0;
	int WhiteCount = 0;
	int BlackToWhiteFlag = 0, WhiteToBlackFlag = 0;
	int ToToLine[2] = { 0 }, ToToLineWidth = 0;
	int OneFirstFindFlag = 0, OneLastFindFlag = 0;
	int TwoFirstFindFlag = 0, TwoLastFindFlag = 0;
	int OneFirstOneTime = 0, OneLastOneTime = 0;
	int TwoFirstOneTime = 0, TwoLastOneTime = 0;
	int StartLineFlag = 0;
	int _Top = 0, _Bottom = 0, _MidLineValue = 0, _VLineValue = 0;
	int LeftHLineValue[4] = { 0 }, LeftVLineValue[2] = { 0 }, LeftMidHLineValue[2] = { 0 };
	int RightHLineValue[4] = { 0 }, RightVLineValue[2] = { 0 }, RightMidHLineValue[2] = { 0 };

	/*����ͼ������*/
	for (int i = VStart; i < VEnd; i++){
		if (JudgeWhite(img(i, HCenter))) WhiteCount++;
		if (0 == BlackToWhiteFlag)
			if (JudgeWhite(img(i, HCenter)))
				if (JudgeBlack(img(i + 1, HCenter))){
					BlackToWhiteFlag = 1;
					ToToLine[0] = i + 1;
				}
		if (1 == BlackToWhiteFlag && 0 == WhiteToBlackFlag)
			if (JudgeBlack(img(i, HCenter)))
				if (JudgeWhite(img(i + 1, HCenter))){
					WhiteToBlackFlag = 1;
					ToToLine[1] = i + 1;
				}
		if (i == (VEnd - VStart) && 1 == WhiteToBlackFlag && 1 == BlackToWhiteFlag)
			ToToLineWidth = ToToLine[1] - ToToLine[0];
		if (i == (VEnd - VStart)){
			if (WhiteCount == (VEnd - VStart)){
				PicMode = 1;
				break;
			}
			if (0 != ToToLineWidth && ToToLineWidth < 8) {
				PicMode = 4;
				break;
			}
			if ((0 == WhiteToBlackFlag && 1 == BlackToWhiteFlag)
				|| (0 != ToToLineWidth && ToToLineWidth > 10)){
				if (JudgeBlack(img(ToToLine[0], HCenter - 1))) {
					PicMode = 2;
					break;
				}
				if (JudgeBlack(img(ToToLine[0], HCenter + 1))) {
					PicMode = 3;
					break;
				}
			}
		}

	}
	//===�Ծ��������Ľ������ж�
	if (1 == PicMode){
		int GoLeft = 0, GoRight = 0;
		int GoCenterToRight = 0, GoCenterToLeft = 0;
		//===�������һ������ı߽�
		for (int i = CENTER_GUIDE; i < END_COL; i++){
			if (JudgeWhite(img(VEnd - 1, i)))
				if (JudgeBlack(img(VEnd - 1, i + 1))){
					GoRight = i + 1;
					break;
				}
		}
		//===�ұ�����һ������ı߽�
		for (int i = CENTER_GUIDE; i > START_COL; i--){
			if (JudgeWhite(img(VEnd - 1, i)))
				if (JudgeBlack(img(VEnd - 1, i - 1))){
					GoLeft = i - 1;
					break;
				}
		}
		if (0 != GoLeft && 0 != GoRight){
			GoCenterToRight = GoRight - CENTER_GUIDE;
			GoCenterToLeft = CENTER_GUIDE - GoLeft;
			if (GoLeft < 40 && GoRight > 40){
				if ((GoCenterToRight - GoCenterToLeft) > 12){
					PicMode = 2;
				}
				if ((GoCenterToLeft - GoCenterToRight) > 12){
					PicMode = 3;
				}
			}
		}
	}
	if (0 == PicMode) return 0;
	/*�ҳ�ÿһ��ͼ�����͵��������ĸ���*/
	//===��������������߾��ȷֲ�������������
	if (1 == PicMode){
		//===�������ĸ���
		for (int j = HCenter; j > START_COL; j--){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Left,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 0 == OneLastFindFlag && 0 == OneFirstOneTime){
				OneFirstOneTime = 1;
				LeftHLineValue[2] = _Bottom;
				LeftHLineValue[3] = _Top;
				LeftVLineValue[1] = _VLineValue;
				LeftMidHLineValue[1] = _MidLineValue;
				if (1 == StartLineFlag && 0 == OneLastOneTime && 1 == OneFirstOneTime){
					OneLastOneTime = 1;
					OneLastFindFlag = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				LeftHLineValue[0] = _Bottom;
				LeftHLineValue[1] = _Top;
				LeftVLineValue[0] = _VLineValue;
				LeftMidHLineValue[0] = _MidLineValue;
			}
			//û���ҵ������Ĭ��
			if (j == (START_COL + 1) && 1 == OneFirstFindFlag && 0 == OneLastFindFlag
				&& 0 != _Bottom && 0 != _Top && LeftMidHLineValue[1] < LineLimit) {
				OneLastFindFlag = 1;
				LeftHLineValue[0] = _Bottom;
				LeftHLineValue[1] = _Top;
				LeftVLineValue[0] = _VLineValue;
				LeftMidHLineValue[0] = _MidLineValue;
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag) break;
		}
		//===���û�����������������˳�
		if (0 == OneFirstFindFlag || 0 == OneLastFindFlag) return 0;
		//===�������ĸ���
		for (int j = HCenter; j < END_COL; j++){
			//===Ϊ�˿��浽���ţ��ڶ�����������
			int KeepVStart = 0, KeepVEnd = 0;
			int MidTemp = (LeftMidHLineValue[0] + LeftMidHLineValue[1]) / 2;
			KeepVStart = MidTemp - 15;
			if (KeepVStart < VStart) KeepVStart = VStart;
			KeepVEnd = MidTemp + 15;
			if (KeepVEnd > VEnd) KeepVEnd = VEnd;
			StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Right,
				&TwoFirstFindFlag, &TwoLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
				TwoFirstOneTime = 1;
				RightHLineValue[0] = _Bottom;
				RightHLineValue[1] = _Top;
				RightVLineValue[0] = _VLineValue;
				RightMidHLineValue[0] = _MidLineValue;
				if (1 == StartLineFlag && 0 == TwoLastOneTime && 1 == TwoFirstOneTime){
					TwoLastOneTime = 1;
					TwoLastFindFlag = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
			}
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
				TwoLastOneTime = 1;
				RightHLineValue[2] = _Bottom;
				RightHLineValue[3] = _Top;
				RightVLineValue[1] = _VLineValue;
				RightMidHLineValue[1] = _MidLineValue;
			}
			//û���ҵ������Ĭ��
			if (j == (END_COL - 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
				&& 0 != _Bottom && 0 != _Top && RightMidHLineValue[0] < LineLimit) {
				TwoLastFindFlag = 1;
				RightHLineValue[2] = _Bottom;
				RightHLineValue[3] = _Top;
				RightVLineValue[1] = _VLineValue;
				RightMidHLineValue[1] = _MidLineValue;
			}
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) break;
		}
		//===�ұ�û�����������������˳�
		if (0 == TwoFirstFindFlag || 0 == TwoLastFindFlag) return 0;
	}
	//===�����ر�ƫ����������������һ��
	//===������ƫ��
	if (2 == PicMode){
		for (int j = HCenter; j < END_COL; j++){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Right,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 0 == OneLastFindFlag && 0 == OneFirstOneTime){
				OneFirstOneTime = 1;
				LeftHLineValue[0] = _Bottom;
				LeftHLineValue[1] = _Top;
				LeftVLineValue[0] = _VLineValue;
				LeftMidHLineValue[0] = _MidLineValue;
				if (JudgeBlack(img(LeftMidHLineValue[0], LeftVLineValue[0] - 1))) return 0;
				if (1 == StartLineFlag && 0 == OneLastOneTime && 1 == OneFirstOneTime){
					OneLastOneTime = 1;
					OneLastFindFlag = 1;
					j += 5; //��5�м�����
					LeftHLineValue[2] = _Bottom;
					LeftHLineValue[3] = _Top;
					LeftVLineValue[1] = _VLineValue;
					LeftMidHLineValue[1] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				j += 5; //��5�м�����
				LeftHLineValue[2] = _Bottom;
				LeftHLineValue[3] = _Top;
				LeftVLineValue[1] = _VLineValue;
				LeftMidHLineValue[1] = _MidLineValue;
			}
			//�����������ҵ�����������ұ���
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag
				&& 1 == OneFirstOneTime && 1 == OneLastOneTime){
				//===Ϊ�˿��浽���ţ��ڶ�����������
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = (LeftMidHLineValue[0] + LeftMidHLineValue[1]) / 2;
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Right,
					&TwoFirstFindFlag, &TwoLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
					TwoFirstOneTime = 1;
					RightHLineValue[0] = _Bottom;
					RightHLineValue[1] = _Top;
					RightVLineValue[0] = _VLineValue;
					RightMidHLineValue[0] = _MidLineValue;
					if (1 == StartLineFlag && 0 == TwoLastOneTime && 1 == TwoFirstOneTime){
						TwoLastOneTime = 1;
						TwoLastFindFlag = 1;
						RightHLineValue[2] = _Bottom;
						RightHLineValue[3] = _Top;
						RightVLineValue[1] = _VLineValue;
						RightMidHLineValue[1] = _MidLineValue;
					}
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
					TwoLastOneTime = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
				//û���ҵ������Ĭ��
				if (j == (END_COL - 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
					&& 0 != _Bottom && 0 != _Top && RightMidHLineValue[0] < LineLimit) {
					TwoLastFindFlag = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag \
				&& 1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) break;
		}
		if (0 == OneFirstFindFlag || 0 == OneLastFindFlag \
			|| 0 == TwoFirstFindFlag || 0 == TwoLastFindFlag) return 0;
	}
	//===������ƫ��
	if (3 == PicMode){
		for (int j = HCenter; j > START_COL; j--){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Left,
				&TwoFirstFindFlag, &TwoLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
				TwoFirstOneTime = 1;
				RightHLineValue[2] = _Bottom;
				RightHLineValue[3] = _Top;
				RightVLineValue[1] = _VLineValue;
				RightMidHLineValue[1] = _MidLineValue;
				if (JudgeBlack(img(RightMidHLineValue[1], RightVLineValue[1] + 1))) return 0;
				if (1 == StartLineFlag && 0 == TwoLastOneTime && 1 == TwoFirstOneTime){
					TwoLastOneTime = 1;
					TwoLastFindFlag = 1;
					j -= 5; //��5�м�����
					RightHLineValue[0] = _Bottom;
					RightHLineValue[1] = _Top;
					RightVLineValue[0] = _VLineValue;
					RightMidHLineValue[0] = _MidLineValue;
				}
			}
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
				TwoLastOneTime = 1;
				j -= 5; //��5�м�����
				RightHLineValue[0] = _Bottom;
				RightHLineValue[1] = _Top;
				RightVLineValue[0] = _VLineValue;
				RightMidHLineValue[0] = _MidLineValue;
			}
			//���ұ�������ҵ���������������
			if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag
				&& 1 == TwoFirstOneTime && 1 == TwoLastOneTime){
				//===Ϊ�˿��浽���ţ��ڶ�����������
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = (RightMidHLineValue[0] + RightMidHLineValue[1]) / 2;
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Left,
					&OneFirstFindFlag, &OneLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == OneFirstFindFlag && 0 == OneLastFindFlag && 0 == OneFirstOneTime){
					OneFirstOneTime = 1;
					LeftHLineValue[2] = _Bottom;
					LeftHLineValue[3] = _Top;
					LeftVLineValue[1] = _VLineValue;
					LeftMidHLineValue[1] = _MidLineValue;
					if (1 == StartLineFlag && 0 == OneLastOneTime && 1 == OneFirstOneTime){
						OneLastOneTime = 1;
						OneLastFindFlag = 1;
						LeftHLineValue[0] = _Bottom;
						LeftHLineValue[1] = _Top;
						LeftVLineValue[0] = _VLineValue;
						LeftMidHLineValue[0] = _MidLineValue;
					}
				}
				if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
					OneLastOneTime = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
				//û���ҵ������Ĭ��
				if (j == (START_COL + 1) && 1 == OneFirstFindFlag && 0 == OneLastFindFlag
					&& 0 != _Bottom && 0 != _Top && LeftMidHLineValue[1] < LineLimit) {
					OneLastFindFlag = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
			}
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag \
				&& 1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) break;
		}
		if (0 == OneFirstFindFlag || 0 == OneLastFindFlag \
			|| 0 == TwoFirstFindFlag || 0 == TwoLastFindFlag) return 0;
	}
	//===������΢ƫһ�㣬������ѹ����������
	if (4 == PicMode){
		//===��������
		int HLineValueTemp[4] = { 0 }, VLineValueTemp[2] = { 0 }, MidHLineValueTemp[2] = { 0 };
		OneFirstFindFlag = 1;
		for (int j = HCenter; j > START_COL; j--){
			StartLineFlag = StartLineFind(j, VStart, VEnd, Left,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				HLineValueTemp[0] = _Bottom;
				HLineValueTemp[1] = _Top;
				VLineValueTemp[0] = _VLineValue;
				MidHLineValueTemp[0] = _MidLineValue;
			}
		}
		OneLastFindFlag = 0;
		OneLastOneTime = 0;
		//===�Ұ������
		for (int j = HCenter; j < END_COL; j++){
			OneFirstFindFlag = 1;
			StartLineFlag = StartLineFind(j, VStart, VEnd, Right,
				&OneFirstFindFlag, &OneLastFindFlag,
				&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
			if (1 == OneFirstFindFlag && 1 == OneLastFindFlag && 0 == OneLastOneTime){
				OneLastOneTime = 1;
				HLineValueTemp[2] = _Bottom;
				HLineValueTemp[3] = _Top;
				VLineValueTemp[1] = _VLineValue;
				MidHLineValueTemp[1] = _MidLineValue;
			}
		}
		int isLeftSearch = 0, isRightSearch = 0;
		int GoLeftSearch = 0, GoRightSearch = 0;
		//===ִ��˫���ѣ��ж���ƫ����ƫ��
		int GoRightV = END_COL - RightEdgeGo(VEnd);
		int GoLeftV = LeftEdgeGo(VEnd) - START_COL;
		if ((GoRightV - GoLeftV)>20)
			GoLeftSearch = 1;
		else if ((GoRightV - GoLeftV)<-20)
			GoRightSearch = 1;
		else{
			LinearLine(&LeftTLSM, VEnd - 10, 10, Left);
			LinearLine(&RightTLSM, VEnd - 10, 10, Right);
			if ((0.03f - (fabs(LeftTLSM.k) - fabs(RightTLSM.k))) > 0)
				GoRightSearch = 1;
			else if ((0.03f - (fabs(RightTLSM.k) - fabs(LeftTLSM.k))) > 0)
				GoLeftSearch = 1;
			else if ((fabs(LeftTLSM.k) - fabs(RightTLSM.k)) > 0)
				GoRightSearch = 1;
			else if ((fabs(RightTLSM.k) - fabs(LeftTLSM.k)) > 0)
				GoLeftSearch = 1;
			else;
		}
		//===�����������
		int LeftValue = 0, RightValue = 0;
		int GoToToMid = 0;
		GoToToMid = myround((float)(MidHLineValueTemp[0] + MidHLineValueTemp[1]) / 2.0f);
		if (GoToToMid < 30)
			LeftValue = VLineValueTemp[0] - 8;
		else
			LeftValue = VLineValueTemp[0] - 4;
		if (LeftValue < START_COL) return 0;
		if (1 == GoLeftSearch){
			for (int j = LeftValue; j > START_COL; j--){
				//===Ϊ�˿��浽���ţ��ڶ�����������
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = myround((float)(MidHLineValueTemp[0] + MidHLineValueTemp[1]) / 2.0f);
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Left,
					&TwoFirstFindFlag, &TwoLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
					TwoFirstOneTime = 1;
					LeftHLineValue[2] = _Bottom;
					LeftHLineValue[3] = _Top;
					LeftVLineValue[1] = _VLineValue;
					LeftMidHLineValue[1] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
					TwoLastOneTime = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
				//û���ҵ������Ĭ��
				if (j == (START_COL + 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
					&& 0 != _Bottom && 0 != _Top && LeftMidHLineValue[1] < LineLimit) {
					TwoLastFindFlag = 1;
					LeftHLineValue[0] = _Bottom;
					LeftHLineValue[1] = _Top;
					LeftVLineValue[0] = _VLineValue;
					LeftMidHLineValue[0] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) {
					isLeftSearch = 1;
					break;
				}
			}
		}
		if (1 == isLeftSearch && 1 == GoLeftSearch){
			for (int n = 0; n < 4; n++){
				RightHLineValue[n] = HLineValueTemp[n];
				if (n < 2){
					RightVLineValue[n] = VLineValueTemp[n];
					RightMidHLineValue[n] = MidHLineValueTemp[n];
				}
			}
		}
		TwoFirstFindFlag = 0;
		TwoLastFindFlag = 0;
		TwoFirstOneTime = 0;
		TwoLastOneTime = 0;
		//===���������ұ�
		if (GoToToMid < 30)
			RightValue = VLineValueTemp[1] + 8;
		else
			RightValue = VLineValueTemp[1] + 4;
		if (RightValue > END_COL) return 0;
		if (1 == GoRightSearch){
			for (int j = RightValue; (j < END_COL && 0 == isLeftSearch); j++){
				//===Ϊ�˿��浽���ţ��ڶ�����������
				int KeepVStart = 0, KeepVEnd = 0;
				int MidTemp = (MidHLineValueTemp[0] + MidHLineValueTemp[1]) / 2;
				KeepVStart = MidTemp - 15;
				if (KeepVStart < VStart) KeepVStart = VStart;
				KeepVEnd = MidTemp + 15;
				if (KeepVEnd > VEnd) KeepVEnd = VEnd;
				StartLineFlag = StartLineFind(j, KeepVStart, KeepVEnd, Right,
					&TwoFirstFindFlag, &TwoLastFindFlag,
					&_Bottom, &_Top, &_MidLineValue, &_VLineValue);
				if (1 == TwoFirstFindFlag && 0 == TwoLastFindFlag && 0 == TwoFirstOneTime){
					TwoFirstOneTime = 1;
					RightHLineValue[0] = _Bottom;
					RightHLineValue[1] = _Top;
					RightVLineValue[0] = _VLineValue;
					RightMidHLineValue[0] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag && 0 == TwoLastOneTime){
					TwoLastOneTime = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
				//û���ҵ������Ĭ��
				if (j == (END_COL - 1) && 1 == TwoFirstFindFlag && 0 == TwoLastFindFlag
					&& 0 != _Bottom && 0 != _Top && RightMidHLineValue[0] < LineLimit) {
					TwoLastFindFlag = 1;
					RightHLineValue[2] = _Bottom;
					RightHLineValue[3] = _Top;
					RightVLineValue[1] = _VLineValue;
					RightMidHLineValue[1] = _MidLineValue;
				}
				if (1 == TwoFirstFindFlag && 1 == TwoLastFindFlag) {
					isRightSearch = 1;
					break;
				}
			}
		}
		if (1 == isRightSearch && 1 == GoRightSearch){
			for (int n = 0; n < 4; n++){
				LeftHLineValue[n] = HLineValueTemp[n];
				if (n < 2){
					LeftVLineValue[n] = VLineValueTemp[n];
					LeftMidHLineValue[n] = MidHLineValueTemp[n];
				}
			}
		}
	}
	/*���ҳ����ĸ��ǽ����жϣ��Ӷ������Ƿ�Ϊ������*/
	//===Ѱ�����������ĵĶ���
	int KeepLeftHMid = (LeftMidHLineValue[0] + LeftMidHLineValue[1]) / 2;
	int KeepRightHMid = (RightMidHLineValue[0] + RightMidHLineValue[1]) / 2;
	int KeepMidHMid = (KeepLeftHMid + KeepRightHMid) / 2;
	//===�����������������߽߱�ľ���
	int LeftRightDiff = abs(RightVLineValue[0] - LeftVLineValue[1]);
	//===�����ߵĳ���
	int LeftDiff = LeftVLineValue[1] - LeftVLineValue[0];
	int RightDiff = RightVLineValue[1] - RightVLineValue[0];
	//===�����ߵĿ��ֵ
	int LeftDiffWidth_Left = LeftHLineValue[1] - LeftHLineValue[0];
	int LeftDiffWidth_Right = LeftHLineValue[3] - LeftHLineValue[2];
	int RightDiffWidth_Left = RightHLineValue[1] - RightHLineValue[0];
	int RightDiffWidth_Right = RightHLineValue[3] - RightHLineValue[2];
	if (abs(KeepLeftHMid - KeepRightHMid) <= 10
		&& (LeftRightDiff < 20 && LeftRightDiff > 5)
		&& ((LeftDiff >= 2 && RightDiff >= 1) || (LeftDiff >= 1 && RightDiff >= 2))
		&& (LeftDiffWidth_Left <= 6 && LeftDiffWidth_Right <= 6)
		&& (RightDiffWidth_Left <= 6 && RightDiffWidth_Right <= 6)
		&& (KeepMidHMid <= 30)
		)
		return 1;
	if (abs(KeepLeftHMid - KeepRightHMid) <= 10
		&& (LeftRightDiff < 15 && LeftRightDiff > 5)
		&& ((LeftDiff >= 2 && RightDiff >= 1) || (LeftDiff >= 1 && RightDiff >= 2))
		&& (LeftDiffWidth_Left <= 3 && LeftDiffWidth_Right <= 3)
		&& (RightDiffWidth_Left <= 3 && RightDiffWidth_Right <= 3)
		&& (KeepMidHMid > 30 && KeepMidHMid <= 40)
		)
		return 1;
	if (abs(KeepLeftHMid - KeepRightHMid) <= 10
		&& (LeftRightDiff < 12 && LeftRightDiff > 5)
		&& ((LeftDiff >= 0 && RightDiff >= 1) || (LeftDiff >= 1 && RightDiff >= 0))
		&& (LeftDiffWidth_Left <= 1 && LeftDiffWidth_Right <= 1)
		&& (RightDiffWidth_Left <= 1 && RightDiffWidth_Right <= 1)
		&& (KeepMidHMid > 40)
		)
		return 1;
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief �����������һ��ֱ��
* @param[out] ����ֱ��б�ʺ�б��
**/
Status CalculateOneLine(int OneLinePoint[], float *OneLinek, float *OneLineb){
	*OneLinek = 0.0f;
	*OneLineb = 0.0f;
	if (0 == (OneLinePoint[2] - OneLinePoint[0]))	return 1;
	*OneLinek = (float)(OneLinePoint[3] - OneLinePoint[1]) / (float)(OneLinePoint[2] - OneLinePoint[0]);
	*OneLineb = (float)OneLinePoint[1] - *OneLinek * (float)OneLinePoint[0];
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:��ʼ��ȫ�ֱ���
**/
Status InitMainVariables(void){
	for (int i = 0; i < CAMERA_H; i++){
		LeftEdgeGo(i) = START_COL;
		CenterGuideGo(i) = CENTER_GUIDE;
		RightEdgeGo(i) = END_COL;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:��ʼ����ʼ����
**/
Status InitBeginLine(BeginLine  *_BeginLine){
	_BeginLine->LeftBeginFilter = START_COL;
	_BeginLine->RightBeginFilter = END_COL;
	_BeginLine->LeftValidLine = 0;
	_BeginLine->RightValidLine = 0;
	_BeginLine->ValidEndLine = 0;
	_BeginLine->LoseLineFlag = 0;
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief �������߽磬�Զ�����������,���ַ�����
* @param[in] ��ǰ��---CurrentLine
* @param[in] �е���ʼ�㡢��ֹ�㡢���ĵ�----HStart/HEnd/HCenter
* @param[in] �����ĵ�ǰλ��---LocSite
* @return 0---δ������   1---������ 2---�������ĵ����
**/
Status SearchOneHorrizonEdge(int CurrentLine, int HStart, int HEnd, int HCenter, Site LocSite){
	int KeepLeft = HStart, KeepRight = HEnd, KeepMid = HCenter;

	/*���Ʊ߽�*/
	if (KeepLeft > KeepRight) return 0;
	if (KeepMid < KeepLeft || KeepMid > KeepRight)
		KeepMid = (KeepLeft + KeepRight) / 2;
	if (KeepMid < HStart)	KeepMid = HStart;
	if (KeepMid > HEnd) KeepMid = HEnd;

	if (Left == LocSite){	//������
		for (int i = 0; i < SEARCH_TIMES; i++){
			if (JudgeWhite(img(CurrentLine, KeepMid))){
				KeepRight = KeepMid;
			}
			else{
				KeepLeft = KeepMid;
			}
			KeepMid = (KeepLeft + KeepRight) / 2;
			if ((KeepRight - KeepLeft) <= 1){
				break;
			}
		}
		if (JudgeWhite(img(CurrentLine, KeepRight)))
			if (JudgeBlack(img(CurrentLine, KeepLeft))){
				LeftEdgeGo(CurrentLine) = KeepLeft;
				return 1;
			}
		return 0;
	}
	else{	//������
		for (int i = 0; i < SEARCH_TIMES; i++){
			if (JudgeWhite(img(CurrentLine, KeepMid))){
				KeepLeft = KeepMid;
			}
			else{
				KeepRight = KeepMid;
			}
			KeepMid = (KeepLeft + KeepRight) / 2;
			if ((KeepRight - KeepLeft) <= 1){
				break;
			}
		}
		if (JudgeBlack(img(CurrentLine, KeepRight)))
			if (JudgeWhite(img(CurrentLine, KeepLeft))){
				RightEdgeGo(CurrentLine) = KeepRight;
				return 1;
			}
		return 0;
	}
}
////////////////////////////////////////////
//Lock
/**
* @brief ˫����������
**/
Status SearchDoubleEdge(int VStart, int VEnd, int HStart, int HEnd, BeginLine *_BeginLine){
	int CountLine = 0;
	int LeftSearchFlag[5] = { 0 };
	int RightSearchFlag[5] = { 0 };	//�洢�ҵ��־λ
	int LeftSearchEdge[5] = { 0 };
	int RightSearchEdge[5] = { 0 };	//�洢��
	int LoseLeftFlag = 0, LoseRightFlag = 0;
	int LastLoseLeft = 0, LastLoseRight = 0;
	int DiffDistance = 0;
	int LeftEdgeCount = 0, RightEdgeCount = 0;	//��������ʹ��
	int LeftClimbSite = 0, RightClimbSite = 0;	//0---�������� 1---���ű߽���
	int DetectLeftFlag = 0, DetectRightFlag = 0; //��������������
	int BaseLineFlag = 0;
	int OverLeftFlag = 0, OverRightFlag = 0; //�޲����ȣ���ͼ��߽�
	int LastOverLeft = 0, LastOverRight = 0;
	int ReviseLeftOutFlag = 0, ReviseRightOutFlag = 0;
	int KeepReviseLeftLine = 0, KeepReviseRightLine = 0;
	int IsObstacle = 0, ObOneTime = 0;
	int ObStartSearchLine = 0, ObEndSearchLine = 0;  //�ϰ���������ʼ����ֹ
	int ObSearchCount = 0; //�ϰ����Ե����
	Site ObComeOn = None;	//��¼����������߻����ұ�
	int ObSaveEdge[CAMERA_H] = { 0 };	//����ϰ���߽�ֵ
	int ObSaveOtherEdge[CAMERA_H] = { 0 }; //����ϰ���߽�ֵ
	int ObSearchLeftEdge = 0, ObSearchRightEdge = 0;
	int _Center = CENTER_GUIDE;	//��ʼĬ��ֵ, ��¼��һ����ֵ����
	InitBeginLine(_BeginLine);

	for (int i = VStart; i < VEnd; i++){
		//=====ִ��������
		if (0 == LeftClimbSite){
			//=====�ҵ��ұ߽��߾͸����߽���������Ԥ����߽�>�ұ߽�
			if (0 == _BeginLine->RightValidLine)	{
				LoseLeftFlag = SearchOneHorrizonEdge(i, _BeginLine->LeftBeginFilter, \
					HEnd, _Center, Left);
			}
			else{
				LoseLeftFlag = SearchOneHorrizonEdge(i, _BeginLine->LeftBeginFilter, \
					RightEdgeGo(i - 1), _Center, Left);
			}
		}
		else{
			//=====�ҵ���������ʼ5��֮�������
			LoseLeftFlag = SupplyLineDotLoc(i, HStart, HEnd, Left, &OverLeftFlag);
		}
		//=====ִ��������
		if (0 == RightClimbSite){
			//=====�ҵ���߽��߾͸����߽���������Ԥ���ұ߽������߽�
			if (0 == _BeginLine->LeftValidLine){
				if (1 == LoseLeftFlag){
					LoseRightFlag = SearchOneHorrizonEdge(i, LeftEdgeGo(i), \
						_BeginLine->RightBeginFilter, _Center, Right);
				}
				else{
					LoseRightFlag = SearchOneHorrizonEdge(i, HStart, \
						_BeginLine->RightBeginFilter, _Center, Right);
				}
			}
			//=====�ұ߽�δ�ҵ���������ʼ5�У�������߽�ȴ�Ѿ��ҵ��ˣ�����һ�ο���˲�
			else{
				LoseRightFlag = SearchOneHorrizonEdge(i, LeftEdgeGo(i - 1), \
					_BeginLine->RightBeginFilter, _Center, Right);
			}
		}
		else{
			//=====�ҵ���������ʼ5��֮�������
			LoseRightFlag = SupplyLineDotLoc(i, HStart, HEnd, Right, &OverRightFlag);
		}

		//=====ִ������������
		if (0 == LeftClimbSite && 1 == RightClimbSite){
			//=====��¼����߽綪ʧ���ұ��߶�ʧ����ֵ
			if (1 == LastLoseRight && 0 == LoseRightFlag){
				KeepReviseRightLine = i - 1;
			}
			//=====����߶�ʧ���ұ��߶�ʧ
			if (0 == LastLoseLeft && 1 == LoseLeftFlag && 0 == LoseRightFlag && 0 != KeepReviseRightLine){
				ReviseLeftOutFlag = 1;
			}
			//=====��߶��ߵ�˫���ҵ�
			if (0 == LastLoseLeft && 1 == LoseLeftFlag && 1 == LoseRightFlag){
				int GoCenter = myround((float)(LeftEdgeGo(i) + RightEdgeGo(i)) / 2.0f);
				DiffDistance = RightEdgeGo(i) - GoCenter - LoseEdgeWidth[i];
				for (int n = _BeginLine->RightValidLine; n < i; n++){
					LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Right);
				}
			}
		}
		if (1 == LeftClimbSite && 0 == RightClimbSite){
			if (1 == LastLoseLeft && 0 == LoseLeftFlag){
				KeepReviseLeftLine = i - 1;
			}
			if (0 == LastLoseRight && 1 == LoseRightFlag && 0 == LoseLeftFlag && 0 != KeepReviseLeftLine){
				ReviseRightOutFlag = 1;
			}
			if (0 == LastLoseRight && 1 == LoseRightFlag && 1 == LoseLeftFlag){
				int GoCenter = myround((float)(LeftEdgeGo(i) + RightEdgeGo(i)) / 2.0f);
				DiffDistance = GoCenter - LeftEdgeGo(i) - LoseEdgeWidth[i];
				for (int n = _BeginLine->LeftValidLine; n < i; n++){
					LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Left);
				}
			}
		}
		//=====����߶�ʧ���ұ��߶�ʧ���������޲�
		if (1 == LeftClimbSite && 1 == RightClimbSite && 1 == ReviseLeftOutFlag){
			int LeftEdgeTemp = 0, RightEdgeTemp = 0;
			ReviseLeftOutFlag = 0;
			LinearLine(&LeftTLSM, _BeginLine->LeftValidLine, 5, Left);
			for (int n = _BeginLine->RightValidLine; n <= KeepReviseRightLine; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeTemp + RightEdgeGo(n)) / 2.0f);
			}
			LinearLine(&RightTLSM, KeepReviseRightLine - 5, 5, Right);
			for (int n = KeepReviseRightLine + 1; n < _BeginLine->LeftValidLine; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeTemp + RightEdgeTemp) / 2.0f);
			}
			for (int n = _BeginLine->LeftValidLine; n <= _BeginLine->LeftValidLine + 5; n++){
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeGo(n) + RightEdgeTemp) / 2.0f);
			}
			DiffDistance = CenterGuideGo(_BeginLine->LeftValidLine) \
				- LeftEdgeGo(_BeginLine->LeftValidLine)	\
				- LoseEdgeWidth[_BeginLine->LeftValidLine];
		}
		//=====�ұ��߶�ʧ������߶�ʧ���������޲�
		if (1 == LeftClimbSite && 1 == RightClimbSite && 1 == ReviseRightOutFlag){
			int LeftEdgeTemp = 0, RightEdgeTemp = 0;
			ReviseRightOutFlag = 0;
			LinearLine(&RightTLSM, _BeginLine->RightValidLine, 5, Right);
			for (int n = _BeginLine->LeftValidLine; n <= KeepReviseLeftLine; n++){
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(RightEdgeTemp + LeftEdgeGo(n)) / 2.0f);
			}
			LinearLine(&LeftTLSM, KeepReviseLeftLine - 5, 5, Left);
			for (int n = KeepReviseLeftLine + 1; n < _BeginLine->RightValidLine; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				RightEdgeTemp = ForecastTHSM(n, RightTLSM.k, RightTLSM.b);
				CenterGuideGo(n) = myround((float)(LeftEdgeTemp + RightEdgeTemp) / 2.0f);
			}
			for (int n = _BeginLine->RightValidLine; n <= _BeginLine->RightValidLine + 5; n++){
				LeftEdgeTemp = ForecastTHSM(n, LeftTLSM.k, LeftTLSM.b);
				CenterGuideGo(n) = myround((float)(RightEdgeGo(n) + LeftEdgeTemp) / 2.0f);
			}
			DiffDistance = RightEdgeGo(_BeginLine->RightValidLine)\
				- CenterGuideGo(_BeginLine->RightValidLine)	\
				- LoseEdgeWidth[_BeginLine->RightValidLine];
		}
		//=====����˫���ҵ��󵥱߶�ʧ���������޲�
		if (1 == LeftClimbSite && 1 == RightClimbSite){
			if (1 == OverLeftFlag && 0 == LastOverLeft){
				DiffDistance = RightEdgeGo(i - 1) - CenterGuideGo(i - 1) - LoseEdgeWidth[i - 1];
			}
			if (1 == OverRightFlag && 0 == LastOverRight){
				DiffDistance = CenterGuideGo(i - 1) - LeftEdgeGo(i - 1) - LoseEdgeWidth[i - 1];
			}
		}
		//======��ͨ�����߲���
		//======δ����ͼ��߽絫�Ǳ����Ҳ���������������Ϊ�����Ч
		if (0 == OverLeftFlag && 0 == LoseLeftFlag && 1 == LeftClimbSite) LoseLeftFlag = 1;
		if (0 == OverRightFlag && 0 == LoseRightFlag && 1 == RightClimbSite) LoseRightFlag = 1;
		//======����ͼ��߽磬�޷�������Ϊ�����Ч
		if (1 == OverLeftFlag) LoseLeftFlag = !OverLeftFlag;
		if (1 == OverRightFlag)LoseRightFlag = !OverRightFlag;
		//======���ݱ��߶�ʧ�������������
		SupplyMidLine(i, DiffDistance, LoseLeftFlag, LoseRightFlag, HStart, HEnd);
		//======��¼��һ�������Ķ���ֵ�Լ���ͼ���ⶪ��ֵ
		LastLoseLeft = LoseLeftFlag;
		LastLoseRight = LoseRightFlag;
		LastOverLeft = OverLeftFlag;
		LastOverRight = OverRightFlag;

		/*����������ֵ*/
		_Center = CenterGuideGo(i - 1);

		/*�ϰ�������,������������������*/
		//Ϊ�˿����ţ������ı߽�Ӧ���������������ı߽���
		//ObComeOn��ʹ����Ϊ�˱�֤�����ܹ�һֱ��һ������������������������ٶ�
		//�����ǵ�һ����������˫������
		if (1 == LeftClimbSite) ObSearchLeftEdge = LeftEdgeGo(i);
		else ObSearchLeftEdge = HStart;
		if (1 == RightClimbSite) ObSearchRightEdge = RightEdgeGo(i);
		else ObSearchRightEdge = HEnd;
		IsObstacle = SearchObStacleFromMid(i,
			ObSearchLeftEdge, ObSearchRightEdge, VEnd,
			&ObComeOn, ObSaveEdge, ObSaveOtherEdge,
			&ObStartSearchLine, &ObEndSearchLine, &ObSearchCount);
		if (1 == IsObstacle && 0 == ObOneTime){	//�����޲�һ��
			ObOneTime = 1;
			ObIsObStacle = 1;
			ObValidLine = ObEndSearchLine;	//��¼�ϰ����Ӧ����Ч��
			ReviseObStacleFromMid(ObComeOn, ObSaveEdge, ObStartSearchLine, ObEndSearchLine);
			break;
		}

		/*��̬�洢��ʧ��ı�־λ�͵�*/
		if (i <= (VStart + 4)){	//��һ�ν�����װ��
			LeftSearchFlag[CountLine] = LoseLeftFlag;
			RightSearchFlag[CountLine] = LoseRightFlag;
			LeftSearchEdge[CountLine] = LeftEdgeGo(i);
			RightSearchEdge[CountLine] = RightEdgeGo(i);
			CountLine++;
			if (CountLine >= 5) CountLine = 0; //���´洢
		}
		if (i > (VStart + 4) && 0 == LeftClimbSite){
			for (int n = 0; n < 4; n++){
				LeftSearchFlag[n] = LeftSearchFlag[n + 1];
				LeftSearchEdge[n] = LeftSearchEdge[n + 1];
			}
			LeftSearchFlag[4] = LoseLeftFlag;
			LeftSearchEdge[4] = LeftEdgeGo(i);
		}
		if (i >(VStart + 4) && 0 == RightClimbSite){	//��������
			for (int n = 0; n < 4; n++){	//ÿ����һ���㶯̬�仯����
				RightSearchFlag[n] = RightSearchFlag[n + 1];
				RightSearchEdge[n] = RightSearchEdge[n + 1];
			}
			RightSearchFlag[4] = LoseRightFlag;
			RightSearchEdge[4] = RightEdgeGo(i);
		}

		/*��δ�ҵ���ʼ��ǰ����̬�洢�ĵ�����������������ж�*/
		if (0 == LeftClimbSite){	/*�����������*/
			LeftEdgeCount = 0;
			for (int n = 0; n < 5; n++){
				if (1 == LeftSearchFlag[n]){
					LeftEdgeCount++;
				}
			}
		}
		if (0 == RightClimbSite){	/*�ҵ���������*/
			RightEdgeCount = 0;
			for (int n = 0; n < 5; n++){
				if (1 == RightSearchFlag[n]){
					RightEdgeCount++;
				}
			}
		}
		/********************************ͼ���޲�***********************************************/
		/*ʮ�ֲ������*/
		if (i == (VStart + 4)){
			DetectLeftFlag = DetectStartErr(i, _BeginLine, Left);
			if (1 == DetectLeftFlag){	//�мн�
				ReviseStartErr(i, Left, _BeginLine);
				LeftEdgeCount = 0;
				LeftClimbSite = 1;
			}
			if (DetectLeftFlag > 1){	//б��ʮ��
				LeftEdgeCount = 0;
				for (int n = 0; n < 5; n++){
					LeftSearchFlag[n] = 0;
				}
			}
			DetectRightFlag = DetectStartErr(i, _BeginLine, Right);
			if (1 == DetectRightFlag){
				ReviseStartErr(i, Right, _BeginLine);
				RightEdgeCount = 0;
				RightClimbSite = 1;
			}
			if (DetectRightFlag > 1){
				RightEdgeCount = 0;
				for (int n = 0; n < 5; n++){
					RightSearchFlag[n] = 0;
				}
			}
		}
		/***************************************************************************************/
		/*����5�н����ж�---�Ƿ�����*/
		if (i >= (VStart + 4) && (0 == LeftClimbSite || 0 == RightClimbSite)){	//���ٴ���5����
			if (5 == LeftEdgeCount && 0 == LeftClimbSite){
				BaseLineFlag = ReviseBaseLine(i, Left);
				if (0 == BaseLineFlag){
					_BeginLine->LeftValidLine = 0;
					LeftClimbSite = 0;
					for (int n = 0; n < 5; n++){
						LeftSearchFlag[n] = 0;
					}
				}
				else{
					_BeginLine->LeftValidLine = i - 4;
					LeftClimbSite = 1;
				}
			}
			if (5 == RightEdgeCount && 0 == RightClimbSite){
				BaseLineFlag = ReviseBaseLine(i, Right);
				if (0 == BaseLineFlag){
					_BeginLine->RightValidLine = 0;
					RightClimbSite = 0;
					for (int n = 0; n < 5; n++){
						RightSearchFlag[n] = 0;
					}
				}
				else{
					_BeginLine->RightValidLine = i - 4;
					RightClimbSite = 1;
				}
			}
		}
		/*��������5������������*/
		if (DetectLeftFlag > 1){
			if (0 == RightClimbSite && 4 == DetectLeftFlag){
				_BeginLine->LeftBeginFilter = LeftEdgeGo(i - 4) + FILTER_MAX;
			}
		}
		if (DetectRightFlag > 1){
			if (0 == LeftClimbSite && 4 == DetectRightFlag){
				_BeginLine->RightBeginFilter = RightEdgeGo(i - 4) - FILTER_MAX;
			}
		}
	}
	/**********************************************************************************************/
	/*����������ɺ���ж����ж�*/
	if ((0 == _BeginLine->LeftValidLine) && (0 == _BeginLine->RightValidLine)){
		_BeginLine->LoseLineFlag = DoubleLose;//˫���Ҳ���
	}
	else if ((0 != _BeginLine->LeftValidLine) && (0 == _BeginLine->RightValidLine)){
		_BeginLine->LoseLineFlag = RightLose;	//�ҵ�����
		int KeepJumpLine = 0, KeepJumpCow = 0;
		for (int n = CENTER_GUIDE; n < (CAMERA_W - 1); n++){
			if (JudgeWhite(img(START_ROW + 7, n)))
				if (JudgeBlack(img(START_ROW + 7, n + 1))){
					KeepJumpCow = n + 1;
					break;
				}
		}
		if (0 != KeepJumpCow){
			DiffDistance = myround((float)(LeftEdgeGo(START_ROW + 7) + KeepJumpCow) / 2.0f)	\
				- LeftEdgeGo(START_ROW + 7) - LoseEdgeWidth[START_ROW + 7];
		}
		else{
			KeepJumpCow = END_COL - 1;
			for (int n = _BeginLine->LeftValidLine; n<END_ROW; n++){
				if (JudgeWhite(img(n, KeepJumpCow)))
					if (JudgeBlack(img(n + 1, KeepJumpCow))){
						KeepJumpLine = n;
						break;
					}
			}
			int Point[4] = { 0 };
			float k = 0.0f, b = 0.0f;
			Point[0] = START_ROW;
			Point[1] = LeftEdgeGo(Point[0]);
			Point[2] = KeepJumpLine;
			Point[3] = LeftEdgeGo(Point[2]);
			CalculateOneLine(Point, &k, &b);
			k = (float)fabs(k);
			DiffDistance = (int)(8.7f * k - 3.5f);
			if (START_ROW == KeepJumpLine) DiffDistance = 0;
		}
		//===�޲�������
		if (0 == IsObstacle){
			for (int n = _BeginLine->LeftValidLine; n < END_ROW; n++){
				LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Left);
			}
		}
	}
	else if ((0 == _BeginLine->LeftValidLine) && (0 != _BeginLine->RightValidLine)){
		_BeginLine->LoseLineFlag = LeftLose; //�ҵ�����
		int KeepJumpLine = 0, KeepJumpCow = 0;
		for (int n = CENTER_GUIDE; n > 0; n--){
			if (JudgeWhite(img(START_ROW + 7, n)))
				if (JudgeBlack(img(START_ROW + 7, n - 1))){
					KeepJumpCow = n - 1;
					break;
				}
		}
		if (0 != KeepJumpCow){
			DiffDistance = RightEdgeGo(START_ROW + 7)	\
				- myround((float)(KeepJumpCow + RightEdgeGo(START_ROW + 7)) / 2.0f)
				- LoseEdgeWidth[START_ROW + 7];
		}
		else{
			KeepJumpCow = START_COL + 1;
			for (int n = _BeginLine->RightValidLine; n<END_ROW; n++){
				if (JudgeWhite(img(n, KeepJumpCow)))
					if (JudgeBlack(img(n + 1, KeepJumpCow))){
						KeepJumpLine = n;
						break;
					}
			}
			int Point[4] = { 0 };
			float k = 0.0f, b = 0.0f;
			Point[0] = START_ROW;
			Point[1] = RightEdgeGo(Point[0]);
			Point[2] = KeepJumpLine;
			Point[3] = RightEdgeGo(Point[2]);
			CalculateOneLine(Point, &k, &b);
			k = (float)fabs(k);
			DiffDistance = (int)(8.7f * k - 3.5f);
			if (START_ROW == KeepJumpLine) DiffDistance = 0;
		}
		//===�޲�������
		if (0 == IsObstacle){
			for (int n = _BeginLine->RightValidLine; n < END_ROW; n++){
				LoseEdgeDeal(n, DiffDistance, HStart, HEnd, Right);
			}
		}
	}
	else{
		_BeginLine->LoseLineFlag = NoneLose; //�ҵ�����
	}
	return OK;
}
/////////////////////////////////////////////
//Lock
/**
* @breif �޲�����������ʼ������
**/
Status ReviseBaseLine(int CurrentLine, Site LocSite){
	int Point[4] = { 0 };
	float k = 0.0f, b = 0.0f;
	int IsDo[3] = { 0 };
	if (Left == LocSite){
		int LeftStartDiff[4] = { 0 };
		LeftStartDiff[0] = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 1);
		LeftStartDiff[1] = LeftEdgeGo(CurrentLine - 1) - LeftEdgeGo(CurrentLine - 2);
		LeftStartDiff[2] = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 3);
		LeftStartDiff[3] = LeftEdgeGo(CurrentLine - 3) - LeftEdgeGo(CurrentLine - 4);
		if (
			(LeftStartDiff[0] == 0 && LeftStartDiff[1] == 0 && LeftStartDiff[2] == 0 && abs(LeftStartDiff[3]) == 1)
			|| (LeftStartDiff[0] == 0 && LeftStartDiff[1] == 0 && abs(LeftStartDiff[2]) == 1 && LeftStartDiff[3] == 0)
			|| (LeftStartDiff[0] == 0 && abs(LeftStartDiff[1]) == 1 && LeftStartDiff[2] == 0 && LeftStartDiff[3] == 0)
			|| (abs(LeftStartDiff[0]) == 1 && LeftStartDiff[1] == 0 && LeftStartDiff[2] == 0 && LeftStartDiff[3] == 0)
			|| (LeftStartDiff[0] == 0 && LeftStartDiff[1] == -1 && LeftStartDiff[2] == 1 && LeftStartDiff[3] == 1)
			){
			LeftTLSM.k = 0.0f;
			LeftTLSM.b = 0.0f;
			return 0;
		}
		/*ʮ�ֻ������*/
		if (
			((LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine)) >= 4)
			|| ((LeftEdgeGo(CurrentLine - 4) - LeftEdgeGo(CurrentLine) >= 7))
			|| LeftStartDiff[0] >= 6
			|| LeftStartDiff[1] >= 6
			|| LeftStartDiff[2] >= 6
			|| LeftStartDiff[3] >= 6
			){
			LeftTLSM.k = 0.0f;
			LeftTLSM.b = 0.0f;
			_BeginLine.LeftValidLine = LeftEdgeGo(CurrentLine - 4) + FILTER_MAX;
			return 0;
		}
		else if (LeftStartDiff[0] == 0 && LeftStartDiff[1] == 0
			&& LeftStartDiff[2] == 0 && LeftStartDiff[3] == 0){
			LeftTLSM.k = 0.0f;
			LeftTLSM.b = 0.0f;
			return 0;
		}
		/*��ʼ�յ������*/
		else{
			int LeftStartDiff1 = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 2);
			int LeftStartDiff2 = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 4);
			IsDo[0] = ((LeftStartDiff2 >= 1 && LeftStartDiff1 < -1)	\
				|| (LeftStartDiff2 > 1 && LeftStartDiff1 <= -1));
			IsDo[1] = (LeftStartDiff[0] <= 0 && LeftStartDiff[1] <= 0	\
				&& LeftStartDiff[2] >= 0 && LeftStartDiff[3] >= 0);
			IsDo[2] = (LeftStartDiff[0] <= -2 && LeftStartDiff[1] == 1	\
				&& LeftStartDiff[2] == 0 && LeftStartDiff[3] == 1);
			if ((IsDo[0]&& IsDo[1]) || IsDo[2]){
				Point[0] = CurrentLine - 4;
				Point[1] = LeftEdgeGo(Point[0]);
				Point[2] = CurrentLine - 2;
				Point[3] = LeftEdgeGo(Point[2]);
				CalculateOneLine(Point, &k, &b);
				for (int n = CurrentLine - 2; n <= CurrentLine; n++){
					LeftEdgeGo(n) = ForecastTHSM(n, k, b);
				}
				LeftTLSM.k = k;
				LeftTLSM.b = b;
			}
			else{
				LinearLine(&LeftTLSM, CurrentLine - 4, 5, Left);		//������С����
			}
			return 1;
		}
	}
	if (Right == LocSite){
		int RightStartDiff[4] = { 0 };
		RightStartDiff[0] = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 1);
		RightStartDiff[1] = RightEdgeGo(CurrentLine - 1) - RightEdgeGo(CurrentLine - 2);
		RightStartDiff[2] = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 3);
		RightStartDiff[3] = RightEdgeGo(CurrentLine - 3) - RightEdgeGo(CurrentLine - 4);
		if ((RightStartDiff[0] == 0 && RightStartDiff[1] == 0 && RightStartDiff[2] == 0 && abs(RightStartDiff[3]) == 1)
			|| (RightStartDiff[0] == 0 && RightStartDiff[1] == 0 && abs(RightStartDiff[2]) == 1 && RightStartDiff[3] == 0)
			|| (RightStartDiff[0] == 0 && abs(RightStartDiff[1]) == 1 && RightStartDiff[2] == 0 && RightStartDiff[3] == 0)
			|| (abs(RightStartDiff[0]) == 1 && RightStartDiff[1] == 0 && RightStartDiff[2] == 0 && RightStartDiff[3] == 0)
			|| (RightStartDiff[0] == 0 && RightStartDiff[1] == 1 && RightStartDiff[2] == -1 && RightStartDiff[3] == -1)
			){
			RightTLSM.k = 0.0f;
			RightTLSM.b = 0.0f;
			return 0;
		}
		/*ʮ�ֻ������*/
		if (
			((RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2)) >= 4)
			|| ((RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 4) >= 7))
			|| RightStartDiff[0] <= -6
			|| RightStartDiff[1] <= -6
			|| RightStartDiff[2] <= -6
			|| RightStartDiff[3] <= -6
			){
			RightTLSM.k = 0.0f;
			RightTLSM.b = 0.0f;
			_BeginLine.RightBeginFilter = RightEdgeGo(CurrentLine - 4) - FILTER_MAX;
			return 0;
		}
		else if (RightStartDiff[0] == 0 && RightStartDiff[1] == 0
			&& RightStartDiff[2] == 0 && RightStartDiff[3] == 0){
			RightTLSM.k = 0.0f;
			RightTLSM.b = 0.0f;
			return 0;
		}
		else{
			/*��ʼ�յ��ҽ���*/
			int RightStartDiff1 = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2);
			int RightStartDiff2 = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 4);
			IsDo[0] = ((RightStartDiff1 >= 1 && RightStartDiff2 < -1)		\
				|| (RightStartDiff1 > 1 && RightStartDiff2 <= -1));
			IsDo[1] = (RightStartDiff[0] >= 0 && RightStartDiff[1] >= 0	\
				&& RightStartDiff[2] <= 0 && RightStartDiff[3] <= 0);
			IsDo[2] = (RightStartDiff[0] >= 2 && RightStartDiff[1] == -1	\
				&& RightStartDiff[2] == 0 && RightStartDiff[3] == -1);
			if ((IsDo[0] && IsDo[1]) || IsDo[2]){
				Point[0] = CurrentLine - 4;
				Point[1] = RightEdgeGo(Point[0]);
				Point[2] = CurrentLine - 2;
				Point[3] = RightEdgeGo(Point[2]);
				CalculateOneLine(Point, &k, &b);
				for (int n = CurrentLine - 2; n <= CurrentLine; n++){
					RightEdgeGo(n) = ForecastTHSM(n, k, b);
				}
				RightTLSM.k = k;
				RightTLSM.b = b;
			}
			else{
				LinearLine(&RightTLSM, CurrentLine - 4, 5, Right);		//������С����
			}
			return 1;
		}
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif �����ϰ���
**/
Status SearchObStacleFromMid(int CurrentLine,
	int HStart, int HEnd, int VEnd,
	Site* SearchFlag, int* ObEdge, int* ObOtherEdge,
	int* ObFirstLine, int* ObEndLine, int* ObCount){
	//���������ϰ����Ե���������ҿ���������һ���ľ���
	//�������������һ�淴�����أ����Ե����ж��ǲ�׼ȷ�� 
	int ObActualCenterWidth = 0, ObRealCenterWidth = 0;
	int EdgeColTemp[2] = { 0 };
	int ObCenterDot[2] = { 0 };
	int WhiteToBlackFlag = 0, BlackToWhiteFlag = 0;
	static int FirstComeIn = 0;
	int DoubleEdge[2] = { 0 };
	int MidCow = CenterGuideGo(CurrentLine);

	//��һ��������������ִ�������������������������κ������ж�
	if (None == *SearchFlag){
		//��һ��������ʱ���������ϲ������Ǻ�ɫ�ϰ���
		if (JudgeBlack(img(CurrentLine, MidCow)))
			return 0;
		//ִ��������
		for (int n = MidCow; n > HStart; n--){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n - 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n - 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n - 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n - 1;
					}
			//����Ҫ�а׺ڣ��ڰ׵�����ſ�����Ϊ�ϰ���
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[0] - DoubleEdge[1]) <= 20
				&& (DoubleEdge[0] - DoubleEdge[1]) >= 5){
				//��һ��װ��߽�ֵ,����������
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObFirstLine = CurrentLine;
				*ObCount = 0;	//��һ������
				*ObCount += 1;
				*SearchFlag = Left;
				FirstComeIn = 1;
				return 0;
			}
		}
		//����������Ͳ��ؼ�������
		if (*SearchFlag == Left) return 0;
		WhiteToBlackFlag = 0;
		BlackToWhiteFlag = 0;
		DoubleEdge[0] = 0;
		DoubleEdge[1] = 0;
		//�����ұ߽�
		for (int n = MidCow; n < HEnd; n++){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n + 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n + 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n + 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n + 1;
					}
			//����Ҫ�а׺ڣ��ڰ׵�����ſ�����Ϊ�ϰ���
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[1] - DoubleEdge[0]) <= 20
				&& (DoubleEdge[1] - DoubleEdge[0]) >= 5){
				//��һ��װ��߽�ֵ,�����ұ߽�
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObFirstLine = CurrentLine;
				*ObCount = 0;	//��һ������
				*ObCount += 1;
				*SearchFlag = Right;
				FirstComeIn = 1;
				return 0;
			}
		}
	}
	else if (Left == *SearchFlag){
		for (int n = MidCow; n > HStart; n--){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n - 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n - 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n - 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n - 1;
					}
			//�ҵ����һ�л����Ҳ�����Ĭ��
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag && n == (HStart + 1)){
				if (JudgeBlack(img(CurrentLine, n))){
					BlackToWhiteFlag = 1;
					DoubleEdge[1] = n - 1;
				}
			}
			//����Ҫ�а׺ڣ��ڰ׵�����ſ�����Ϊ�ϰ���
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[0] - DoubleEdge[1]) <= 20){
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObCount += 1;
				break;	//����������Ϊ�ж�
			}
		}
		//��һ�ν������ܻ������ػ�����Ҫ�˳�
		if (1 == FirstComeIn
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			&& (abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (CurrentLine == (*ObFirstLine + 1))
			){
			FirstComeIn = 0;
			*ObCount = 0;	//���¼���
			*SearchFlag = None;	//����������һ��
			return 0;
		}
		//������ֹ��
		if (CurrentLine == (VEnd - 1)	//���������һ�л������������������������
			&& 0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag){
			*ObEndLine = VEnd - 1;
		}
		//��������
		if ((abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			){
			*ObEndLine = CurrentLine - 1;
		}
		//��һ���Ҳ���
		if (0 == WhiteToBlackFlag || 0 == BlackToWhiteFlag){
			*ObEndLine = CurrentLine - 1;
		}

		//����3��������
		if (0 != *ObFirstLine && 0 != *ObEndLine && *ObCount < 5){
			*ObCount = 0;	//���¼���
			*SearchFlag = None;	//����������һ��
			return 0;
		}
		//�������������ҵ��ϰ����ˣ����ǻ���Ҫ�ж�һ��
		if (0 != *ObFirstLine && 0 != *ObEndLine){
			//��ȡ�ϰ�������ĵ�
			ObCenterDot[0] = (*ObFirstLine + *ObEndLine) / 2;
			EdgeColTemp[0] = (*(ObEdge + *ObFirstLine) + *(ObEdge + *ObEndLine)) / 2;
			EdgeColTemp[1] = (*(ObOtherEdge + *ObFirstLine) + *(ObOtherEdge + *ObEndLine)) / 2;
			ObCenterDot[1] = (EdgeColTemp[0] + EdgeColTemp[1]) / 2;
			//�������ߵľ���Ҫ�̶�---�������ߵľ�����5cm
			//ͼ����Ϊ�л��䣬����ֵӦ������һ�����Թ�ϵ
			ObActualCenterWidth = 25 - (int)(0.15f * ObCenterDot[0]);
			ObRealCenterWidth = (LeftEdgeGo(ObCenterDot[0]) \
				+ RightEdgeGo(ObCenterDot[0])) / 2 - ObCenterDot[1];
			//�����һ���жϣ��Ƿ�����ϰ���������������Ϊ��Ч
			if (ObRealCenterWidth <= ObActualCenterWidth){
				return 1;
			}
			else{
				*ObCount = 0;	//���¼���
				*SearchFlag = None;	//����������һ��
				return 0;
			}
		}
	}
	else if (Right == *SearchFlag){
		for (int n = MidCow; n < HEnd; n++){
			if (0 == WhiteToBlackFlag)
				if (JudgeWhite(img(CurrentLine, n)))
					if (JudgeBlack(img(CurrentLine, n + 1))){
						WhiteToBlackFlag = 1;
						DoubleEdge[0] = n + 1;
					}
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag)
				if (JudgeBlack(img(CurrentLine, n)))
					if (JudgeWhite(img(CurrentLine, n + 1))){
						BlackToWhiteFlag = 1;
						DoubleEdge[1] = n + 1;
					}
			//�ҵ����һ�л����Ҳ�����Ĭ��
			if (0 != WhiteToBlackFlag && 0 == BlackToWhiteFlag && n == (HEnd - 1)){
				if (JudgeBlack(img(CurrentLine, n))){
					BlackToWhiteFlag = 1;
					DoubleEdge[1] = n + 1;
				}
			}
			//����Ҫ�а׺ڣ��ڰ׵�����ſ�����Ϊ�ϰ���
			if (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag
				&& (DoubleEdge[1] - DoubleEdge[0]) <= 20){
				*(ObEdge + CurrentLine) = DoubleEdge[0];
				*(ObOtherEdge + CurrentLine) = DoubleEdge[1];
				*ObCount += 1;
				break;	//����������Ϊ�ж�
			}
		}
		//��һ�ν������ܻ������ػ�����Ҫ�˳�
		if (1 == FirstComeIn
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			&& (abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (CurrentLine == (*ObFirstLine + 1))
			){
			FirstComeIn = 0;
			*ObCount = 0;	//���¼���
			*SearchFlag = None;	//����������һ��
			return 0;
		}
		//������ֹ��
		if (CurrentLine == (VEnd - 1)	//���������һ�л������������������������
			&& 0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag){
			*ObEndLine = VEnd - 1;
		}
		//��������
		if ((abs(*(ObEdge + CurrentLine) - *(ObEdge + CurrentLine - 1)) > 5
			|| abs(*(ObOtherEdge + CurrentLine) - *(ObOtherEdge + CurrentLine - 1)) > 5)
			&& (0 != WhiteToBlackFlag && 0 != BlackToWhiteFlag)
			){
			*ObEndLine = CurrentLine - 1;
		}
		if (0 == WhiteToBlackFlag || 0 == BlackToWhiteFlag){
			*ObEndLine = CurrentLine - 1;
		}
		//����5��������
		if (0 != *ObFirstLine && 0 != *ObEndLine && *ObCount < 5){
			*ObCount = 0;	//���¼���
			*SearchFlag = None;	//����������һ��
			return 0;
		}
		//�������������ҵ��ϰ����ˣ����ǻ���Ҫ�ж�һ��
		if (0 != *ObFirstLine && 0 != *ObEndLine){
			//��ȡ�ϰ�������ĵ�
			ObCenterDot[0] = (*ObFirstLine + *ObEndLine) / 2;
			EdgeColTemp[0] = (*(ObEdge + *ObFirstLine) + *(ObEdge + *ObEndLine)) / 2;
			EdgeColTemp[1] = (*(ObOtherEdge + *ObFirstLine) + *(ObOtherEdge + *ObEndLine)) / 2;
			ObCenterDot[1] = (EdgeColTemp[0] + EdgeColTemp[1]) / 2;
			//�������ߵľ���Ҫ�̶�---�������ߵľ�����5cm
			//ͼ����Ϊ�л��䣬����ֵӦ������һ�����Թ�ϵ
			//50-7 35-10
			ObActualCenterWidth = 25 - (int)(0.15f * ObCenterDot[0]); //20 0.2
			ObRealCenterWidth = ObCenterDot[1] - (LeftEdgeGo(ObCenterDot[0]) \
				+ RightEdgeGo(ObCenterDot[0])) / 2;
			//�����һ���жϣ��Ƿ�����ϰ���������������Ϊ��Ч
			if (ObRealCenterWidth <= ObActualCenterWidth){
				return 1;
			}
			else{
				*ObCount = 0;	//���¼���
				*SearchFlag = None;	//����������һ��
				return 0;
			}
		}
	}
	else;

	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @bref �޲����ϰ���������������
**/
Status ReviseObStacleFromMid(Site LocSite, int* ObEdge, int ObFirstLine, int ObEndLine){
	int Point_X[5] = { 0 };
	int Point_Y[5] = { 0 };
	//===ɸѡ������ȷ�ֵ
	Point_X[0] = ObFirstLine + 1;
	Point_X[4] = ObEndLine;
	Point_X[2] = myround((float)(Point_X[0] + Point_X[4]) / 2.0f);
	Point_X[1] = myround((float)(Point_X[0] + Point_X[2]) / 2.0f);
	Point_X[3] = myround((float)(Point_X[2] + Point_X[4]) / 2.0f);

	//��������
	if (Left == LocSite){
		for (int n = 0; n < 5; n++){
			Point_Y[n] = *(ObEdge + Point_X[n]) + 0;
		}
		//===�����ϰ���������޲���ȥ�ĵ�
		CalculateTLSM(&LeftTLSM, Point_X, Point_Y, 5);
		ReviseSetDot(START_ROW, END_ROW - START_ROW, START_COL, END_COL, Left);
	}
	else if (Right == LocSite){
		for (int n = 0; n < 5; n++){
			Point_Y[n] = *(ObEdge + Point_X[n]) - 0;
		}
		CalculateTLSM(&RightTLSM, Point_X, Point_Y, 5);
		ReviseSetDot(START_ROW, END_ROW - START_ROW, START_COL, END_COL, Right);
	}
	else;

	//===��ԭ����
	for (int n = START_ROW; n <= Point_X[4]; n++){
		CenterGuideGo(n) = (LeftEdgeGo(n) + RightEdgeGo(n)) / 2;
	}
	Point_X[0] = START_ROW;
	Point_X[4] = ObEndLine;
	Point_X[2] = myround((float)(Point_X[0] + Point_X[4]) / 2.0f);
	Point_X[1] = myround((float)(Point_X[0] + Point_X[2]) / 2.0f);
	Point_X[3] = myround((float)(Point_X[2] + Point_X[4]) / 2.0f);
	for (int n = 0; n < 5; n++){
		Point_Y[n] = CenterGuideGo(Point_X[n]);
	}
	CalculateTLSM(&CenterTLSM, Point_X, Point_Y, 5);
	ReviseSetDot(Point_X[4], END_ROW - Point_X[4], START_COL, END_COL, Center);
	//��ԭ���޲��ĵ�
	for (int i = (ObEndLine + 1); i < END_ROW; i++) {
		LeftEdgeGo(i) = START_COL;
		RightEdgeGo(i) = END_COL;
		CenterGuideGo(i) = CENTER_GUIDE;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif �����޲���
**/
Status ReviseSetDot(int CurrentLine, int Len, int HStart, int HEnd, Site LocSite){
	int DataTemp = 0;
	int isSearchFlag = 0;
	if (Len == 0) return 0;
	if (Left == LocSite){
		for (int i = 0; i < Len; i++){
			DataTemp = ForecastTHSM(CurrentLine + i, LeftTLSM.k, LeftTLSM.b);
			if (DataTemp > HStart && DataTemp < HEnd){
				LeftEdgeGo(CurrentLine + i) = DataTemp;
				isSearchFlag = 0;
			}
			else{
				LeftEdgeGo(CurrentLine + i) = LeftEdgeGo(CurrentLine + i - 1);
				isSearchFlag = 1;
			}
		}
	}
	else if (Right == LocSite){
		for (int i = 0; i < Len; i++){
			DataTemp = ForecastTHSM(CurrentLine + i, RightTLSM.k, RightTLSM.b);
			if (DataTemp > HStart && DataTemp < HEnd){
				RightEdgeGo(CurrentLine + i) = DataTemp;
				isSearchFlag = 0;
			}
			else{
				RightEdgeGo(CurrentLine + i) = RightEdgeGo(CurrentLine + i - 1);
				isSearchFlag = 1;
			}
		}
	}
	else{
		for (int i = 0; i < Len; i++){
			DataTemp = ForecastTHSM(CurrentLine + i, CenterTLSM.k, CenterTLSM.b);
			if (DataTemp > HStart && DataTemp < HEnd){
				CenterGuideGo(CurrentLine + i) = DataTemp;
				isSearchFlag = 0;
			}
			else{
				CenterGuideGo(CurrentLine + i) = CenterGuideGo(CurrentLine + i - 1);
				isSearchFlag = 1;
			}
		}
	}
	return isSearchFlag;
}
////////////////////////////////////////////
//Lock
/**
* @brief:��һ����ĸ��������߽�
* @param[in]:��ʼ��---HStart����ֹ��---HEnd���޶������߽�
* @param[in]:����λ�ñ�־---LocFlag
* @param[out]:�Ƿ��������ı�־---SearchFalg 0---�������� 1---������
* @return:״̬
**/
Status SupplyOneHorrizon(int CurrentLine, int HStart, int HEnd, Site LocFlag, int *SearchFlag){
	int NearByDis = 0;
	if (CurrentLine <= 10){
		NearByDis = 5;
	}
	else if (CurrentLine <= 40){
		NearByDis = 8;
	}
	else{
		NearByDis = 10;
	}
	int PreEdge = 0;
	int Point[2] = { 0 };
	int DesPoint = 0;
	if (Left == LocFlag){
		PreEdge = LeftEdgeGo(CurrentLine - 1);	//��߽������ı߽��޶�
		Point[0] = PreEdge - NearByDis;
		Point[1] = PreEdge + NearByDis;
		if (Point[0] < HStart) Point[0] = HStart;
		if (Point[1]  > HEnd) Point[1] = HEnd;

		if (JudgeWhite(img(CurrentLine, PreEdge))){	//��ɫִ��������
			for (int i = PreEdge; i > Point[0]; i--){
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i - 1))){
						DesPoint = i - 1;
						break;
					}
			}
		}
		if (JudgeBlack(img(CurrentLine, PreEdge))){	//��ɫִ��������
			for (int i = PreEdge; i < Point[1]; i++){
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i + 1))){
						DesPoint = i;
						break;
					}
			}
		}
		if (0 == DesPoint) *SearchFlag = 0;
		else{
			*SearchFlag = 1;
			LeftEdgeGo(CurrentLine) = DesPoint;
		}
	}
	else if (Right == LocFlag){
		PreEdge = RightEdgeGo(CurrentLine - 1);
		Point[0] = PreEdge - NearByDis;
		Point[1] = PreEdge + NearByDis;
		if (Point[0] < HStart) Point[0] = HStart;
		if (Point[1]  > HEnd) Point[1] = HEnd;

		if (JudgeWhite(img(CurrentLine, PreEdge))){	//��ɫִ��������
			for (int i = PreEdge; i < Point[1]; i++){
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i + 1))){
						DesPoint = i + 1;
						break;
					}
			}
		}
		if (JudgeBlack(img(CurrentLine, PreEdge))){	//��ɫִ��������
			for (int i = PreEdge; i>Point[0]; i--){
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i - 1))){
						DesPoint = i;
						break;
					}
			}
		}
		if (0 == DesPoint) *SearchFlag = 0;
		else{
			*SearchFlag = 1;
			RightEdgeGo(CurrentLine) = DesPoint;
		}
	}
	else if (Center == LocFlag){
		PreEdge = CenterGuideGo(CurrentLine - 1);
		Point[0] = PreEdge - NearByDis;
		Point[1] = PreEdge + NearByDis;
		if (Point[0] < HStart) Point[0] = HStart;
		if (Point[1]  > HEnd) Point[1] = HEnd;

		if (CurrentLine < 6 || CurrentLine > 45) return OK;

		if (JudgeWhite(img(CurrentLine, PreEdge))){
			for (int i = PreEdge; i > Point[0]; i--){	//�޸���߽�
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i - 1))){
						LeftEdgeGo(CurrentLine) = i + 2; //i - 1;
						break;
					}
			}
			for (int i = PreEdge; i < Point[1]; i++){   //�޸��ұ߽�
				if (JudgeWhite(img(CurrentLine, i)))
					if (JudgeBlack(img(CurrentLine, i + 1))){
						RightEdgeGo(CurrentLine) = i - 2;//i + 1;
						break;
					}
			}
		}
		if (JudgeBlack(img(CurrentLine, PreEdge))){
			for (int i = PreEdge; i < Point[1]; i++){   //�޸���߽�
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i + 1))){
						LeftEdgeGo(CurrentLine) = i + 3;//i;
						break;
					}
			}
			for (int i = PreEdge; i > Point[0]; i--){	//�޸��ұ߽�
				if (JudgeBlack(img(CurrentLine, i)))
					if (JudgeWhite(img(CurrentLine, i - 1))){
						RightEdgeGo(CurrentLine) = i - 3;//i;
						break;
					}
			}
		}
		return ERROR;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief �޲��߽�
* @return 0---���ٱ�δ�ҵ� 1---���ٱ��Ѿ��ҵ�
**/
Status SupplyLineDotLoc(int CurrentLine, int HStart, int HEnd, Site LocSite, int* OverFlag){
	int isSearch = 0;
	int isJumpFlag = 0;
	if (Left == LocSite){
		SupplyOneHorrizon(CurrentLine, HStart, HEnd, Left, &isSearch);
		/*�ҵ��������С���˲����йյ���*/
		if (1 == isSearch){
			isJumpFlag = isJumpDot(CurrentLine, Left);
			if (1 == isJumpFlag){	//��⵽�յ㲻������С���ˣ�����
				if (CurrentLine <= 8){
					LinearLine(&LeftTLSM, START_ROW, 5, Left);
					ReviseSetDot(CurrentLine, 1, HStart, HEnd, Left);
				}
				else if (CurrentLine <= 35){
					if ((CurrentLine - 8) >= _BeginLine.LeftValidLine){
						LinearLine(&LeftTLSM, CurrentLine - 8, 5, Left);
					}
					else{
						LinearLine(&LeftTLSM, CurrentLine - 7, 5, Left);
					}
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Left);
				}
				else{
					LinearLine(&LeftTLSM, CurrentLine - 8, 5, Left);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Left);
				}
			}
			/*ʮ���Ҳ�������������,����������յ�*/
			else if (2 == isJumpFlag && CurrentLine >= 6){
				int TopLine = 0;
				SupplyOneHorrizon(CurrentLine + 1, HStart, HEnd, Left, &TopLine);
				if (0 == TopLine){
					if (CurrentLine >= 6 && CurrentLine <= 10)
						LinearLine(&LeftTLSM, START_ROW, 5, Left);
					else
						LinearLine(&LeftTLSM, CurrentLine - 6, 5, Left);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Left);
				}
			}
			else{
				if (CurrentLine < 40)
					LinearLine(&LeftTLSM, CurrentLine - 5, 5, Left);
				else if (CurrentLine < 45)
					LinearLine(&LeftTLSM, CurrentLine - 3, 3, Left);
				else
					LinearLine(&LeftTLSM, CurrentLine - 2, 2, Left);
			}
			return 1;
		}
		else{
			if (0 != LeftTLSM.k){
				if ((LeftTLSM.k-0.45f) >= 0)
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart + 2, HEnd - 2, Left);
				else
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart, HEnd, Left);
			}
			else{
				LeftEdgeGo(CurrentLine) = LeftEdgeGo(CurrentLine - 1);
				*OverFlag = 1;
			}
			return 0;
		}
	}
	else if (Right == LocSite){
		SupplyOneHorrizon(CurrentLine, HStart, HEnd, Right, &isSearch);
		/*�ҵ��������С���˲����йյ���ͻع��б�*/
		if (1 == isSearch){
			isJumpFlag = isJumpDot(CurrentLine, Right);
			if (1 == isJumpFlag){	//��⵽�յ㲻������С���ˣ�����
				if (CurrentLine <= 8){
					LinearLine(&RightTLSM, START_ROW, 5, Right);
					ReviseSetDot(CurrentLine, 1, HStart, HEnd, Right);
				}
				else if (CurrentLine <= 35){
					if ((CurrentLine - 8) >= _BeginLine.RightValidLine){
						LinearLine(&RightTLSM, CurrentLine - 8, 5, Right);
					}
					else{
						LinearLine(&RightTLSM, CurrentLine - 7, 5, Right);
					}
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Right);
				}
				else{
					LinearLine(&RightTLSM, CurrentLine - 8, 5, Right);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Right);
				}
			}
			/*ʮ���Ҳ�������������*/
			if (2 == isJumpFlag && CurrentLine >= 6){
				int TopLine = 0;
				SupplyOneHorrizon(CurrentLine + 1, HStart, HEnd, Right, &TopLine);
				if (0 == TopLine){
					if (CurrentLine >= 6 && CurrentLine <= 10)
						LinearLine(&RightTLSM, START_ROW, 5, Right);
					else
						LinearLine(&RightTLSM, CurrentLine - 6, 5, Right);
					ReviseSetDot(CurrentLine - 2, 3, HStart, HEnd, Right);
				}
			}
			else{
				if (CurrentLine < 40)
					LinearLine(&RightTLSM, CurrentLine - 5, 5, Right);
				else if (CurrentLine < 45)
					LinearLine(&RightTLSM, CurrentLine - 3, 3, Right);
				else
					LinearLine(&RightTLSM, CurrentLine - 2, 2, Right);
			}
			return 1;
		}
		else{	
			if (0 != RightTLSM.k){
				if ((RightTLSM.k-0.45f) >= 0)
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart + 2, HEnd - 2, Right);
				else
					*OverFlag = ReviseSetDot(CurrentLine, 1, HStart, HEnd, Right);
			}
			else{
				RightEdgeGo(CurrentLine) = RightEdgeGo(CurrentLine - 1);
				*OverFlag = 1;
			}
			return 0;
		}
	}
	else;
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief:�жϹյ�༶�ж�
* @param[in]:��ǰ��---CurrentLine
* @param[in]:����λ��LocSite
* @return:�Ƿ��ǻعյ� 0---���� 1----��
**/
Status isJumpDot(int CurrentLine, Site LocSite){
	int Diff1 = 0, Diff2 = 0, Diff3 = 0, Diff4 = 0;
	int DiffTop = 0, DiffBottom = 0;
	int Pro = 0;
	if (Left == LocSite){
		Diff1 = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 1);
		Diff2 = LeftEdgeGo(CurrentLine - 1) - LeftEdgeGo(CurrentLine - 2);
		Diff3 = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 3);
		Diff4 = LeftEdgeGo(CurrentLine - 3) - LeftEdgeGo(CurrentLine - 4);
		DiffTop = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine);
		DiffBottom = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 4);
	}
	else if (Right == LocSite){
		Diff1 = RightEdgeGo(CurrentLine - 1) - RightEdgeGo(CurrentLine);
		Diff2 = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 1);
		Diff3 = RightEdgeGo(CurrentLine - 3) - RightEdgeGo(CurrentLine - 2);
		Diff4 = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 3);
		DiffTop = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2);
		DiffBottom = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 2);
	}
	else;

	//����յ㴦��
	if (Diff1 == 1 && Diff2 == -1 && Diff3 == 0 && Diff4 == 0) return 1;
	//С��Զ��������
	if (CurrentLine > 45){
		if (Diff1 <= -2 && Diff2 == 0 && Diff3 == 1 && Diff4 == 0)
			return 0;
		if (Diff1 <= -3 && Diff2 <= -1 && Diff3 == 0 && Diff4 == 0)
			return 0;
	}
	

	if (CurrentLine < 40 && CurrentLine > 20){
		if (Diff1 <= 0 && Diff2 <= 0 && Diff3 >= 0){
			if (DiffTop >= 3 && DiffBottom >= 0)
				return 1;
		}

		if (Diff1 <= 0 && Diff2 >= 0 && Diff3 >= 0){
			if (DiffTop >= 5 && DiffBottom >= 0)
				return 1;
		}
	}

	if (Diff1 <= 0 && Diff2 <= 0	//�����йյ����
		&& Diff3 >= 0 && Diff4 >= 0){
		if (0 == DiffBottom) Pro = DiffTop;
		else Pro = DiffTop / DiffBottom;
		if ((DiffTop >= 1 && DiffBottom >= 1 && CurrentLine < 10)
			|| (DiffTop >= 2 && DiffBottom >= 1 && CurrentLine < 40 && CurrentLine >= 10)
			|| (DiffTop >= 3 && DiffBottom >= 2 && CurrentLine >= 40)
			|| (Pro >= 3 && CurrentLine <= 35)
			|| (Pro >= 5 && CurrentLine > 35)){
			return 1;
		}

		/*�������ʮ�ֺ͹��䣬�ֱ�����ʮ����һ���Ҳ����������ҵõ�*/
		if (CurrentLine < 40){
			if (
				(DiffTop >= 1 && DiffBottom >= 1)
				|| (DiffTop >= 1 && Diff2 == 0 && Diff3 == 0 && Diff4 == 0)
				|| (DiffBottom >= 1 && Diff1 == 0 && Diff2 == 0 && Diff3 == 0)
				|| (Diff1 <= -2 && Diff2 == 1 && Diff3 == 0 && Diff4 == 1)
				){
				return 2;
			}
		}
	}
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief:��������
* @param[in]:������̬��ʼ���Զ������---_BeginLine
* @param[in]:��ֹ������---VEnd
* @param[out]:������ֹ��Ч��
* @return:״̬
**/
Status SupplyMidLine(int CurrentLine, int Diff,
	int LeftEdgeFlag, int RightEdgeFlag,
	int HStart, int HEnd){
	if (1 == LeftEdgeFlag && 1 == RightEdgeFlag){
		CenterGuideGo(CurrentLine) = \
			myround((float)(LeftEdgeGo(CurrentLine) + RightEdgeGo(CurrentLine)) / 2.0f);
	}
	else if (1 == LeftEdgeFlag && 0 == RightEdgeFlag){
		LoseEdgeDeal(CurrentLine, Diff, HStart, HEnd, Left);
	}
	else if (0 == LeftEdgeFlag && 1 == RightEdgeFlag){
		LoseEdgeDeal(CurrentLine, Diff, HStart, HEnd, Right);
	}
	else{
		CenterGuideGo(CurrentLine) = CenterGuideGo(CurrentLine - 1);
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief:���߽��ߴ���
* @param[in]:��ǰ��---CurrentLine
* @param[in]:��ʼ����/��ֹ������HStart/HEnd)
* @param[in]:����λ�� Left---��������߲����� Right---�����ұ��߲���
* @return:�쳣����µ��к�---CurrentLine
**/
Status LoseEdgeDeal(int CurrentLine, int Diff, int HStart, int HEnd, Site LocFlag){
	int MidTemp = 0;
	if (Left == LocFlag){
		MidTemp = LeftEdgeGo(CurrentLine) + LoseEdgeWidth[CurrentLine] + Diff;
		if (MidTemp > HStart && MidTemp < HEnd){
			CenterGuideGo(CurrentLine) = MidTemp;
		}
		else{
			if (MidTemp <= HStart) CenterGuideGo(CurrentLine) = HStart;
			if (MidTemp >= HEnd)	CenterGuideGo(CurrentLine) = HEnd;
		}
	}
	else if (Right == LocFlag){
		MidTemp = RightEdgeGo(CurrentLine) - LoseEdgeWidth[CurrentLine] - Diff;
		if (MidTemp > HStart && MidTemp < HEnd){
			CenterGuideGo(CurrentLine) = MidTemp;
		}
		else{
			if (MidTemp <= HStart) CenterGuideGo(CurrentLine) = HStart;
			if (MidTemp >= HEnd)	CenterGuideGo(CurrentLine) = HEnd;
		}
	}
	else{
		return ERROR;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @brief ��ʼ��ʮ�ֻ������
**/
Status DetectStartErr(int CurrentLine, BeginLine *_BeginLine, Site LocFlag){
	/*��ʼ�л������*/
	int Diff[4] = { 0 };
	int DiffTop = 0, DiffBottom = 0;
	if (Left == LocFlag){
		Diff[0] = LeftEdgeGo(CurrentLine) - LeftEdgeGo(CurrentLine - 1);
		Diff[1] = LeftEdgeGo(CurrentLine - 1) - LeftEdgeGo(CurrentLine - 2);
		Diff[2] = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 3);
		Diff[3] = LeftEdgeGo(CurrentLine - 3) - LeftEdgeGo(CurrentLine - 4);
		DiffTop = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine);
		DiffBottom = LeftEdgeGo(CurrentLine - 2) - LeftEdgeGo(CurrentLine - 4);
		//������ʼ
		if (Diff[0] <= -2 && Diff[1] == 1 && Diff[2] == 0 && abs(Diff[3]) == 1) return 1;
		if (LeftEdgeGo(CurrentLine) == START_COL ||
			LeftEdgeGo(CurrentLine - 1) == START_COL ||
			LeftEdgeGo(CurrentLine - 2) == START_COL ||
			LeftEdgeGo(CurrentLine - 3) == START_COL ||
			LeftEdgeGo(CurrentLine - 4) == START_COL) return 2;
		if (Diff[0] == 0 && Diff[1] == 1 && Diff[2] == 1 && Diff[3] == 0) return 2;
		if (abs(Diff[0]) >= 1 && Diff[1] == 0 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && abs(Diff[1]) >= 1 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && abs(Diff[2]) >= 1 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) >= 1) return 2;
		//�Ҳ����߽�
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) == 0) return 3;
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] >= 0 && Diff[3] >= 0){
			if (DiffTop >= 2 && DiffBottom >= 1) return 1;
			if (DiffTop <= 1 && DiffBottom == 1) return 2;
			if (DiffTop >= 3 && 0 == DiffBottom) return 2;
		}
		//��������
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] <= 0 && Diff[3] <= 0){
			if ((LeftEdgeGo(CurrentLine - 4) - LeftEdgeGo(CurrentLine)) >= 4)
				return 4;
		}
	}

	if (Right == LocFlag){
		Diff[0] = RightEdgeGo(CurrentLine - 1) - RightEdgeGo(CurrentLine);
		Diff[1] = RightEdgeGo(CurrentLine - 2) - RightEdgeGo(CurrentLine - 1);
		Diff[2] = RightEdgeGo(CurrentLine - 3) - RightEdgeGo(CurrentLine - 2);
		Diff[3] = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 3);
		DiffTop = RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 2);
		DiffBottom = RightEdgeGo(CurrentLine - 4) - RightEdgeGo(CurrentLine - 2);
		if (Diff[0] <= -2 && Diff[1] == 1 && Diff[2] == 0 && abs(Diff[3]) == 1) return 1;
		if (RightEdgeGo(CurrentLine) == END_COL ||
			RightEdgeGo(CurrentLine - 1) == END_COL ||
			RightEdgeGo(CurrentLine - 2) == END_COL ||
			RightEdgeGo(CurrentLine - 3) == END_COL ||
			RightEdgeGo(CurrentLine - 4) == END_COL) return 2;
		if (Diff[0] == 0 && Diff[1] == 1 && Diff[2] == 1 && Diff[3] == 0) return 2;
		if (abs(Diff[0]) >= 1 && Diff[1] == 0 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && abs(Diff[1]) >= 1 && Diff[2] == 0 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && abs(Diff[2]) >= 1 && Diff[3] == 0) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) >= 1) return 2;
		if (Diff[0] == 0 && Diff[1] == 0 && Diff[2] == 0 && abs(Diff[3]) == 0) return 3;
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] >= 0 && Diff[3] >= 0){
			if (DiffTop >= 2 && DiffBottom >= 1) return 1;
			if (DiffTop <= 1 && DiffBottom == 1) return 2;
			if (DiffTop >= 3 && 0 == DiffBottom) return 2;
		}
		if (Diff[0] <= 0 && Diff[1] <= 0
			&& Diff[2] <= 0 && Diff[3] <= 0){
			if ((RightEdgeGo(CurrentLine) - RightEdgeGo(CurrentLine - 4)) >= 4)
				return 4;
		}
	}
	return 0;
}
////////////////////////////////////////////
//Lock
/**
* @brief ������ʼ�м�ⲻ��
**/
Status ReviseStartErr(int CurrentLine, Site LocSite, BeginLine *_BeginLine){
	float k = 0.0f, b = 0.0f;
	int Temp = 0;
	int Point[4] = { 0 };
	if (Left == LocSite){
		Point[0] = CurrentLine - 4;
		Point[1] = LeftEdgeGo(Point[0]);
		Point[2] = CurrentLine - 2;
		Point[3] = LeftEdgeGo(Point[2]);
		CalculateOneLine(Point, &k, &b);
		for (int i = (CurrentLine - 2); i <= CurrentLine; i++){
			Temp = ForecastTHSM(i, k, b);
			if (Temp > START_COL && Temp < END_COL){
				LeftEdgeGo(i) = Temp;
			}
			else{
				LeftEdgeGo(i) = LeftEdgeGo(i - 1);
			}
		}
		for (int i = CurrentLine; i >= (CurrentLine - 4); i--){
			Temp = LeftEdgeGo(i) + LoseEdgeWidth[i];
			if (Temp > START_COL && Temp < END_COL){
				CenterGuideGo(i) = Temp;
			}
			else{
				CenterGuideGo(i) = CenterGuideGo(i - 1);
			}
		}
		LeftTLSM.k = k;
		LeftTLSM.b = b;
		_BeginLine->LeftValidLine = CurrentLine - 4;
	}
	if (Right == LocSite){
		Point[0] = CurrentLine - 4;
		Point[1] = RightEdgeGo(Point[0]);
		Point[2] = CurrentLine - 2;
		Point[3] = RightEdgeGo(Point[2]);
		CalculateOneLine(Point, &k, &b);
		for (int i = (CurrentLine - 2); i <= CurrentLine; i++){
			Temp = ForecastTHSM(i, k, b);
			if (Temp > START_COL && Temp < END_COL){
				RightEdgeGo(i) = Temp;
			}
			else{
				RightEdgeGo(i) = RightEdgeGo(i - 1);
			}
		}
		for (int i = CurrentLine; i >= (CurrentLine - 4); i--){
			Temp = RightEdgeGo(i) - LoseEdgeWidth[i];
			if (Temp > START_COL && Temp < END_COL){
				CenterGuideGo(i) = Temp;
			}
			else{
				CenterGuideGo(i) = CenterGuideGo(i - 1);
			}
		}
		RightTLSM.k = k;
		RightTLSM.b = b;
		_BeginLine->RightValidLine = CurrentLine - 4;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif ��Զǰհ2.5���ж�
**/
Status SearchEndLine(BeginLine* _BeginLine){
	_BeginLine->ValidEndLine = END_ROW - 1;
	for (int i = 1; i <= (CAMERA_H - 6); i++){
		if (NoneLose == _BeginLine->LoseLineFlag){
			if (abs(RightEdgeGo(i) - LeftEdgeGo(i)) <= 10){
				_BeginLine->ValidEndLine = i;
				break;
			}
			if (CenterGuideGo(i) <= LEFT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
			if (CenterGuideGo(i) >= RIGHT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
		}
		if (LeftLose == _BeginLine->LoseLineFlag){
			if (CenterGuideGo(i) <= LEFT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
		}
		if (RightLose == _BeginLine->LoseLineFlag){
			if (CenterGuideGo(i) >= RIGHT_EDGE
				&& JudgeBlack(img(i + 1, CenterGuideGo(i + 1)))
				&& JudgeBlack(img(i + 2, CenterGuideGo(i + 2)))
				){
				_BeginLine->ValidEndLine = i;
				break;
			}
		}
		if (i > 45 && abs(CenterGuideGo(i) - CenterGuideGo(i - 1)) > 13){
			_BeginLine->ValidEndLine = i - 1;
			break;
		}
	}

	if (1 == ObIsObStacle){
		_BeginLine->ValidEndLine = ObValidLine;
	}
	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif �޲���ʼ��
**/
Status ReviseStartValidLine(int HStart, int HEnd, BeginLine* _BeginLine){
	int ValidStartLine = 0;
	if (NoneLose == _BeginLine->LoseLineFlag){
		if (_BeginLine->LeftValidLine == _BeginLine->RightValidLine){
			ValidStartLine = _BeginLine->LeftValidLine;
			if (START_ROW != ValidStartLine){
				LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
				ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
			}
		}
		else if (_BeginLine->LeftValidLine > _BeginLine->RightValidLine){
			ValidStartLine = _BeginLine->RightValidLine;
			int Diff = CenterGuideGo(_BeginLine->LeftValidLine)
				- LeftEdgeGo(_BeginLine->LeftValidLine)
				- LoseEdgeWidth[_BeginLine->LeftValidLine];
			for (int i = _BeginLine->RightValidLine; i < _BeginLine->LeftValidLine; i++){
				LoseEdgeDeal(i, Diff, START_COL, END_COL, Right);
			}
			if (START_ROW != ValidStartLine){
				LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
				ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
			}
		}
		else{
			ValidStartLine = _BeginLine->LeftValidLine;
			int Diff = RightEdgeGo(_BeginLine->RightValidLine) \
				- CenterGuideGo(_BeginLine->RightValidLine)
				- LoseEdgeWidth[_BeginLine->RightValidLine];
			for (int i = _BeginLine->LeftValidLine; i < _BeginLine->RightValidLine; i++){
				LoseEdgeDeal(i, Diff, START_COL, END_COL, Left);
			}
			if (START_ROW != ValidStartLine){
				LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
				ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
			}
		}
	}
	else if (LeftLose == _BeginLine->LoseLineFlag){
		ValidStartLine = _BeginLine->RightValidLine;
		if (START_ROW != ValidStartLine){
			LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
			ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
		}
	}
	else if (RightLose == _BeginLine->LoseLineFlag){
		ValidStartLine = _BeginLine->LeftValidLine;
		if (START_ROW != ValidStartLine){
			LinearLine(&CenterTLSM, ValidStartLine, 5, Center);
			ReviseSetDot(START_ROW, ValidStartLine - START_ROW, HStart, HEnd, Center);
		}
	}
	else;


	return OK;
}
////////////////////////////////////////////
//Lock
/**
* @breif ʶ���µ�
**/
Status DetectRampOK(BeginLine* _BeginLine){
	int Width[5] = { 0 }, WidthChange[4] = { 0 };
	int Slope[2] = { 0 };
	int WidthChangeSum = 0, SlopeSum = 0;
	if (NoneLose == _BeginLine->LoseLineFlag && _BeginLine->ValidEndLine >= 50){
		Width[0] = RightEdgeGo(10) - LeftEdgeGo(10);
		Width[1] = RightEdgeGo(20) - LeftEdgeGo(20);
		Width[2] = RightEdgeGo(30) - LeftEdgeGo(30);
		Width[3] = RightEdgeGo(40) - LeftEdgeGo(40);
		Width[4] = RightEdgeGo(50) - LeftEdgeGo(50);
		WidthChange[0] = Width[0] - Width[1];
		WidthChange[1] = Width[1] - Width[2];
		WidthChange[2] = Width[2] - Width[3];
		WidthChange[3] = Width[3] - Width[4];
		Slope[0] = LeftEdgeGo(50) - LeftEdgeGo(10);
		Slope[1] = RightEdgeGo(10) - RightEdgeGo(50);
		WidthChangeSum = WidthChange[0] + WidthChange[1] + WidthChange[2] + WidthChange[3];
		SlopeSum = Slope[0] + Slope[1];
		if ((WidthChange[0] <= 12 && WidthChange[1] <= 10 && WidthChange[2] <= 10 && WidthChange[3] <= 10)
			&& (WidthChange[0] >= 3 && WidthChange[1] >= 3 && WidthChange[2] >= 3 && WidthChange[3] >= 3)
			&& (WidthChangeSum < 40 && WidthChangeSum >= 20)
			&& (SlopeSum < 40 && SlopeSum >= 20)
			&& (Width[0] >= 65 && Width[4] >= 30))
			return 1;
	}
	return 0;
}
////////////////////////////////////////////
//UnLock
/**
* @breif ��ʴ�˲�
**/
Status OneBlackDotFilter(void){
	for (int i = 0; i < CAMERA_H; i++){
		for (int j = 1; j < (CAMERA_W - 1); j++){
			if (JudgeWhite(img(i, j - 1)))
				if (JudgeBlack(img(i, j)))
					if (JudgeWhite(img(i, j + 1))){
						img(i,j) = 0xFF;
					}
		}
	}
	return OK;
}
////////////////////////////////////////////

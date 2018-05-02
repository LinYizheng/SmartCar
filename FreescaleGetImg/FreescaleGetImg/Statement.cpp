// Statement.cpp : implementation file
//

#include "stdafx.h"
#include "FreescaleGetImg.h"
#include "Statement.h"
#include "afxdialogex.h"


// Statement dialog

IMPLEMENT_DYNAMIC(Statement, CDialogEx)

Statement::Statement(CWnd* pParent /*=NULL*/)
	: CDialogEx(Statement::IDD, pParent)
{
}

Statement::~Statement()
{
}

/**
* @breif ���˵��
**/
void Statement::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, StatementPtr);

	CFont font;
	font.CreatePointFont(160, _T("����"));
	StatementPtr.SetFont(&font);

	StatementPtr.ResetContent();
	CString StateStr;
	StatementPtr.InsertString(0,  _T("author���Ͼ���Ϣ���̴�ѧ"));
	StatementPtr.InsertString(1,  _T("show:ƫ��/����б��/������/�µ�/�ϰ���"));
	StatementPtr.InsertString(2,  _T("func������ͼ��/����ͼ��/����ͼ��/����ͼ��"));
	StatementPtr.InsertString(3,  _T("��λ������˵����"));
	StatementPtr.InsertString(4,  _T("     if(Rxdata=='S') isCamera=1; //д�ڴ����жϽ�����"));
	StatementPtr.InsertString(5,  _T("     if (1 == isCamera){ //д��ͼ���������"));
	StatementPtr.InsertString(6,  _T("          isCamera = 0;"));
	StatementPtr.InsertString(7,  _T("          ImageCapture();"));
	StatementPtr.InsertString(8,  _T("     }"));
	StatementPtr.InsertString(9,  _T("     Status ImageCapture(void){"));
	StatementPtr.InsertString(10, _T("          int  i=0,j=0;"));
	StatementPtr.InsertString(11, _T("          uint8_t *PHOTO_BUFFPoint = NULL;"));
	StatementPtr.InsertString(12, _T("          for(i=0;i<60;i++)"));
	StatementPtr.InsertString(13, _T("          {"));
	StatementPtr.InsertString(14, _T("               for(j=0;j<80;j++)"));
	StatementPtr.InsertString(15, _T("               {"));
	StatementPtr.InsertString(16, _T("                    PHOTO_BUFFPoint=OV7725_IMAGE[i]+j;"));
	StatementPtr.InsertString(17, _T("                    UART_WriteByte(HW_UART3,*PHOTO_BUFFPoint);"));
	StatementPtr.InsertString(18, _T("               }"));
	StatementPtr.InsertString(19, _T("          }"));
	StatementPtr.InsertString(20, _T("          return OK;"));
	StatementPtr.InsertString(21, _T("     }"));
}


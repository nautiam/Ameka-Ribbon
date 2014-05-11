// LoaddingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Ameka.h"
#include "LoaddingDlg.h"
#include "afxdialogex.h"
#include "AmekaDoc.h"
#include "DSPModule.h"
#include "AmekaView.h"


// CLoaddingDlg dialog

IMPLEMENT_DYNAMIC(CLoaddingDlg, CDialogEx)

CLoaddingDlg::CLoaddingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLoaddingDlg::IDD, pParent)
{

}

CLoaddingDlg::~CLoaddingDlg()
{
	KillTimer(timer);
	ShowWindow(SW_HIDE);
}

void CLoaddingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, LOAD_Progress, load_progress);
}


BEGIN_MESSAGE_MAP(CLoaddingDlg, CDialogEx)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLoaddingDlg message handlers


void CLoaddingDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	CAmekaView* pView = CAmekaView::GetView();
	if (!pDoc || !pView)
		return;

	val++;
	if (val > 35)
		val = 1;
	load_progress.SetPos(val);

	CDialogEx::OnTimer(nIDEvent);
}


BOOL CLoaddingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	timer = SetTimer(1,					// timer identifier 
		50,					// 10-second interval 
		NULL);					// no timer callback 
	load_progress.SetRange(1, 25);
	val = 1;
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();

	if (!pDoc)
		return FALSE;
	AfxBeginThread(DSP::ProcessRecordDataThread, (LPVOID)pDoc);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

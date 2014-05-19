#include "stdafx.h"
#include "InfoDlg.h"

CInfoDlg::CInfoDlg() : CDialogEx(CInfoDlg::IDD)
{
}

int CInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!pDoc)
		return 0;


	info_sex.SetCurSel(pDoc->patientInfo.sex);
	return 0;
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, info_sex);
	DDX_Control(pDX, IDC_CHECK1, info_handed);
}

void CInfoDlg::OnBnClickedok()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
	CAmekaView *pView = CAmekaView::GetView();
	if (pView)
		pView->OnDraw(pView->GetDC());
}


void CInfoDlg::OnBnClickedcancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

BEGIN_MESSAGE_MAP(CInfoDlg, CDialogEx)
	ON_BN_CLICKED(info_ok, &CInfoDlg::OnBnClickedok)
	ON_BN_CLICKED(info_cancel, &CInfoDlg::OnBnClickedcancel)
END_MESSAGE_MAP()
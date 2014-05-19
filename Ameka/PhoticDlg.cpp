#include "stdafx.h"
#include "PhoticDlg.h"

//------------------------------------------------------------------//
// CPhoticDlg
//------------------------------------------------------------------//

int CPhoticDlg::OnInitDialog()
{
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!pDoc)
		return -1;
	float min = theApp.photicMin;
	float max = theApp.photicMax;
	float rate = theApp.photicWRate;
	float width = pDoc->mDSP.epocLength;
	float tick = theApp.photicTick;
	CDialog::OnInitDialog();
	CString t;

	t.Format(_T("%.1f"), min);
	this->photicMin.SetWindowTextW(t);
	t.Format(_T("%.1f"), max);
	this->photicMax.SetWindowTextW(t);
	t.Format(_T("%.1f"), tick);
	this->photicTick.SetWindowTextW(t);
	t.Format(_T("%.1f"), width);
	this->photicWidth.SetWindowTextW(t);
	t.Format(_T("%.1f"), rate);
	this->photicWRate.SetWindowTextW(t);

	this->photicName1.SetWindowTextW(L"Alpha");
	this->photicName2.SetWindowTextW(L"Beta");
	this->photicName3.SetWindowTextW(L"Theta");

	return TRUE;
}

CPhoticDlg::CPhoticDlg() : CDialogEx(CPhoticDlg::IDD)
{
}

void CPhoticDlg::DoDataExchange(CDataExchange* pDX)
{
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!pDoc)
		return;

	DDX_Control(pDX, photic_min, photicMin);
	DDX_Control(pDX, photic_max, photicMax);
	DDX_Control(pDX, photic_tick, photicTick);
	DDX_Control(pDX, IDC_EDIT4, photicName1);
	DDX_Control(pDX, IDC_EDIT11, photicName2);
	DDX_Control(pDX, IDC_EDIT5, photicName3);
	DDX_Control(pDX, IDC_EDIT12, photicName4);
	DDX_Control(pDX, IDC_EDIT13, photicName5);
	DDX_Control(pDX, photic_Width, photicWidth);
	DDX_Control(pDX, photic_WRate, photicWRate);
	CString tmp;
	DDX_Text(pDX, photic_max, tmp);
	if (tmp != "")
		theApp.photicMax = _ttof(tmp);

	DDX_Text(pDX, photic_min, tmp);
	if (tmp != "")
		theApp.photicMin = _ttof(tmp);

	DDX_Text(pDX, photic_tick, tmp);
	if (tmp != "")
		theApp.photicTick = _ttof(tmp);

	DDX_Text(pDX, photic_Width, tmp);
	if (tmp != "")
		pDoc->mDSP.epocLength = _ttof(tmp);

	DDX_Text(pDX, photic_WRate, tmp);
	if (tmp != "")
		theApp.photicWRate = _ttof(tmp);

	//CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, photic_WRate, photicWRate);
}

void CPhoticDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CPhoticDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

BEGIN_MESSAGE_MAP(CPhoticDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPhoticDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPhoticDlg::OnBnClickedCancel)
END_MESSAGE_MAP()
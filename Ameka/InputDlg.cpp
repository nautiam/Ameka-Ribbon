// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Ameka.h"
#include "InputDlg.h"
#include "afxdialogex.h"


// CInputDlg dialog

IMPLEMENT_DYNAMIC(CInputDlg, CDialogEx)

CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInputDlg::IDD, pParent)
{

}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_COMBO1, strEvKind);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInputDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CInputDlg message handlers


void CInputDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CInputDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

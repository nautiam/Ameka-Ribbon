// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Ameka.h"
#include "InfoDlg.h"
#include "afxdialogex.h"


// CInfoDlg dialog

IMPLEMENT_DYNAMIC(CInfoDlg, CDialogEx)

CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInfoDlg::IDD, pParent)
{

}

CInfoDlg::~CInfoDlg()
{
}

int CInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!pDoc)
		return 1;

	m_Date.SetTime(&pDoc->patientInfo.birthday);
	m_Hand.SetCheck(pDoc->patientInfo.lefthanded);
	return 0;
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, info_fistName, mName);
	DDX_Control(pDX, info_lname, mSurName);
	DDX_Control(pDX, info_surName, m_SecondName);
	DDX_Control(pDX, info_birthday, m_Date);
	DDX_Control(pDX, IDC_COMBO1, m_Sex);
	DDX_Control(pDX, IDC_EDIT4, m_ID);
	DDX_Control(pDX, IDC_CHECK1, m_Hand);
	DDX_Control(pDX, info_view, m_Info);
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	DDX_Text(pDX, info_fistName, pDoc->patientInfo.fname);
	DDX_Text(pDX, info_lname, pDoc->patientInfo.lname);
	DDX_Text(pDX, info_surName, pDoc->patientInfo.surname);
	DDX_Text(pDX, IDC_COMBO1, pDoc->patientInfo.sex);
	DDX_Text(pDX, IDC_EDIT4, pDoc->patientInfo.uID);
	DDX_Text(pDX, info_view, pDoc->patientInfo.note);
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialogEx)
	ON_BN_CLICKED(info_ok, &CInfoDlg::OnBnClickedok)
	ON_BN_CLICKED(info_cancel, &CInfoDlg::OnBnClickedcancel)
END_MESSAGE_MAP()


// CInfoDlg message handlers


void CInfoDlg::OnBnClickedok()
{
	// TODO: Add your control notification handler code here
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!pDoc)
		return;
	m_Date.GetTime(pDoc->patientInfo.birthday);
	pDoc->patientInfo.lefthanded = m_Hand.GetCheck();
	CDialogEx::OnOK();
}


void CInfoDlg::OnBnClickedcancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

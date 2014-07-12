#pragma once
#include "afxwin.h"
#include "afxdtctl.h"


// CInfoDlg dialog

class CInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInfoDlg)

public:
	CInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInfoDlg();

// Dialog Data
	enum { IDD = DLG_Info };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
// Implementation
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CEdit mName;
	CEdit mSurName;
	CEdit m_SecondName;
	CDateTimeCtrl m_Date;
	CComboBox m_Sex;
	CEdit m_ID;
	CButton m_Hand;
	CEdit m_Info;
	afx_msg void OnBnClickedok();
	afx_msg void OnBnClickedcancel();
};

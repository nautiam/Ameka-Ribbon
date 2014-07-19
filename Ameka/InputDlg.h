#pragma once
#include "afxwin.h"


// CInputDlg dialog

class CInputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInputDlg)

public:
	CInputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInputDlg();

	int strEvKind;

// Dialog Data
	enum { IDD = DLG_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	virtual BOOL OnInitDialog();
	CComboBox event_Change;
};

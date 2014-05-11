#pragma once
#include "afxcmn.h"


// CLoaddingDlg dialog

class CLoaddingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoaddingDlg)

public:
	CLoaddingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoaddingDlg();

// Dialog Data
	enum { IDD = DLG_Loading };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl load_progress;
	uint16_t val;
	UINT_PTR timer;
private:

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
};

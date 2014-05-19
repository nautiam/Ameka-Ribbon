#include "stdafx.h"
#include "Ameka.h"

//------------------------------------------------------------------//
// CPhoticDlg
//------------------------------------------------------------------//

class CPhoticDlg : public CDialogEx
{
public:
	CPhoticDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = DLG_Photic };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit photicMin;
	CEdit photicMax;
	CEdit photicTick;
	CEdit photicName1;
	CEdit photicName2;
	CEdit photicName3;
	CEdit photicName4;
	CEdit photicName5;
	CEdit photicWidth;
	CMFCColorButton photicColor;
	CEdit photicWRate;
};
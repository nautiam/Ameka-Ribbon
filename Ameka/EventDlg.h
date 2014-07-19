#pragma once
#include "afxcmn.h"


// CEventDlg dialog

//------------------------------------------------------------------//
// CEventDlg
//------------------------------------------------------------------//

class CEventDlg : public CDialogEx
{
public:
	CEventDlg();
	int index;

	// Dialog Data
	enum { IDD = DLG_Event};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedok();
	afx_msg void OnBnClickedcancel();
	CListCtrl event_list;
	afx_msg void OnBnClickedrename();
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
};

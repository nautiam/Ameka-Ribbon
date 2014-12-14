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

	void initItems();

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
	afx_msg void OnBnClickeddel();
};

/***********************************************************************/
class CEvPane : public CDockablePane
{
	DECLARE_DYNAMIC(CEvPane)
	DECLARE_MESSAGE_MAP()
public:
	CEvPane();
	virtual ~CEvPane();
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lp);
	afx_msg void OnSize(UINT nType, int cx, int cy);
private:
	CEventDlg m_wndDlg;
};
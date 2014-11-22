#pragma once
#include "afxwin.h"


// CEventListDlg dialog

class CEventListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEventListDlg)

public:
	CEventListDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEventListDlg();

// Dialog Data
	enum { IDD = DLG_EventList };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_ListView;
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnDblclkListev();
};

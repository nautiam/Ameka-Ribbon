#pragma once
#include "afxwin.h"


// CEventListDlg dialog

class CEventListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEventListDlg)
	//DECLARE_MESSAGE_MAP()
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

/***********************************************************************/
class CEvListPane : public CDockablePane
{
	DECLARE_DYNAMIC(CEvListPane)
	DECLARE_MESSAGE_MAP()
public:
	CEvListPane();
	virtual ~CEvListPane();
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lp);
	afx_msg void OnSize(UINT nType, int cx, int cy);
private:
	CEventListDlg m_wndDlg;
};
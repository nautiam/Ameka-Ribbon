
#include "stdafx.h"
#include "Ameka.h"
#include "afxwin.h"

//------------------------------------------------------------------//
//Tab Dialog
//------------------------------------------------------------------//

vector<string> Tokenize(CString str, string delimiters);
CString itoS ( int x );

class CTabCOMDlg : public CDialogEx
{
public:
	CTabCOMDlg();
// Dialog Data
	enum { IDD = DLG_Opt_COM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox port_name;
	CComboBox port_baud;
};

//------------------------------------------------------------------//
// Rec tab Dialog
//------------------------------------------------------------------//

class CTabRecDlg : public CDialogEx
{
public:
	CTabRecDlg();
// Dialog Data
	enum { IDD = DLG_Opt_Rec };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedeeg();
	CEdit rec_ed_eeg;
	CEdit rec_ed_video;
	afx_msg void OnBnClickedvideo();
};

//------------------------------------------------------------------//
// Event tab dialog
//------------------------------------------------------------------//

class CTabEventDlg : public CDialogEx
{
public:
	CTabEventDlg();
// Dialog Data
	enum { IDD = DLG_Opt_Event };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:

	CEdit ev_1;
	CEdit ev_2;
	CEdit mEV1;
	CEdit mEV2;
	CEdit mEV3;
	CEdit mEV4;
	CEdit mEV5;
	CEdit mEV6;
	CEdit mEV7;
	CEdit mEV8;
	CEdit mEV9;
	CEdit mEV10;
};

//------------------------------------------------------------------//
// View tab dialog
//------------------------------------------------------------------//

class CTabViewDlg : public CDialogEx
{
public:
	CTabViewDlg();
// Dialog Data
	enum { IDD = DLG_Opt_View };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_view_speed;
	CEdit m_view_sen;
	CEdit m_view_lp;
	CEdit m_view_hp;
	afx_msg void OnBnClickedbtdef();
	CEdit m_view_dotPmm;
};

//------------------------------------------------------------------//
// COptionDlg
//------------------------------------------------------------------//

class COptionDlg : public CDialogEx
{
public:
	COptionDlg();
	CString m_baudRate;
	CString m_portNo;
	CString m_LP;
	CString m_HP;
	CString m_sensitivity;
	CString m_speed;

	CDialog* mDlg[4];

	int mPrePos;

// Dialog Data
	enum { IDD = DLG_Option };
	CTabCtrl tab_ctrl;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnOK();
	afx_msg void OnTabSel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedcancel();
	afx_msg void OnBnClickedok();
};

//------------------------------------------------------------------//
// CTabPrintDlg
//------------------------------------------------------------------//

#pragma once


// CTabPrintDlg dialog

class CTabPrintDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTabPrintDlg)

public:
	CTabPrintDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTabPrintDlg();

// Dialog Data
	enum { IDD = DLG_Opt_Print };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CEdit ed_left;
	CEdit ed_right;
	CEdit ed_top;
	CEdit ed_bot;
};
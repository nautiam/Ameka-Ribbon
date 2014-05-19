#include "stdafx.h"
#include "AmekaUserConfig.h"
#include "Ameka.h"

//------------------------------------------------------------------//
// CMontageDlg
//------------------------------------------------------------------//

class CMontageDlg : public CDialogEx
{
public:
	CMontageDlg();
	//~CMontageDlg();
// Dialog Data
	enum { IDD = DLG_Montage};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedadd();
	CComboBox mon_l1;
	CComboBox mon_l2;
	CListBox mon_list;
	CComboBox mon_lName;
	Amontage crtMon;
	afx_msg void OnBnClickedMonsave();
	afx_msg void OnMonListSelChange();
	void DrawMontage(CDC *dc, Amontage mMon);
	afx_msg void OnPaint();
	afx_msg void OnBnClickeddel();
};
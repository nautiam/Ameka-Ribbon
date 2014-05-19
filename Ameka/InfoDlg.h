#include "stdafx.h"
#include "Ameka.h"

//------------------------------------------------------------------//
// CInfoDlg
//------------------------------------------------------------------//

class CInfoDlg : public CDialogEx
{
public:
	CInfoDlg();

// Dialog Data
	enum { IDD = DLG_Info };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedok();
	afx_msg void OnBnClickedcancel();
	CComboBox info_sex;
	CButton info_handed;
};

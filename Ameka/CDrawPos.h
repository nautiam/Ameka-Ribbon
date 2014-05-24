#ifndef AFX_C_CDRAWPOS_H
#define AFX_C_CDRAWPOS_H

#include "stdafx.h"
#include <stdint.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CDrawPos.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDrawPos window

class CDrawPos : public CWnd
{
// Construction
public:
	CDrawPos();

// Attributes

// Operations
public:
	/// @ create the tool tip
	BOOL Create(CSize szSize, CWnd *pWnd = NULL);

	/// @ adjust tip size
	void SetRectSize(CSize szSize);

	/// @ Show tip 
	void ShowRect(int nX, int nY);

	/// @ hide tip
	void HideRect();

	/// @ Set back ground color
	void SetBkColor(COLORREF clrBack);

	/// @ Set tip text color
	void SetTipTextColor(COLORREF clrText);

	/// @ Set Frame color
	void SetFrameColor(COLORREF clrFrame);

	/// @ Set max value and min value
	void SetValue(uint16_t* val, uint16_t drawval);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawPos)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDrawPos();
private: 
    CPoint  m_ptOrg;
	CSize   m_TipSize;
	CString m_strTips;
	COLORREF m_clrBack;
	COLORREF m_clrText;
	COLORREF m_clrFrameColor;

	uint16_t maxVal;
	uint16_t minVal;
	uint16_t drawVal;

	// Generated message map functions
protected:
	//{{AFX_MSG(CDrawPos)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDrawPos_H__CFCCAC8A_A5CF_46ED_94BA_B5F572B3AC51__INCLUDED_)

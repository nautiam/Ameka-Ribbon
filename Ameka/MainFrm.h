// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// MainFrm.h : interface of the CMainFrame class
//

#pragma once

class MyToolBar : public CMFCRibbonBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
	{ return CMFCRibbonBar::OnUpdateCmdUI(pTarget, FALSE);}
};

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	// Attributes
public:

	// Operations
public:

	// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Implementation
public:
	virtual ~CMainFrame();
	MyToolBar     m_wndRibbonBar;
	bool startEnable;
	bool stopEnable;
	bool recEnable;
	bool portEnable;
	bool baudEnable;
	bool scanPortEnable;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;
	CMFCCaptionBar    m_wndCaptionBar;

	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnViewCaptionBar();
	afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	DECLARE_MESSAGE_MAP()

	BOOL CreateCaptionBar();
public:
	afx_msg void OnScalerate();
	afx_msg void OnSpeedrate();
	afx_msg void OnMonlist();
	afx_msg void OnLp();
	afx_msg void OnHp();
	afx_msg void OnSave();
	afx_msg void OnOpen();
	afx_msg void OnUpdateStartdemo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateStopdemo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateRecording(CCmdUI *pCmdUI);
	afx_msg void OnUpdateScan(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePortname(CCmdUI *pCmdUI);
	afx_msg void OnUpdateBaud(CCmdUI *pCmdUI);
	afx_msg void OnPrint();
};



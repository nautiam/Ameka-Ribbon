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

// AmekaView.h : interface of the CAmekaView class
//
#ifndef _AMEKA_VIEW_
#define _AMEKA_VIEW_
#pragma once

#include "AmekaDoc.h"
#include "SerialCtrl.h"

#define baseLine 16383
#define amp 812

class CAmekaView : public CView
{
protected: // create from serialization only
	CAmekaView();
	DECLARE_DYNCREATE(CAmekaView)

// Attributes
public:
	CAmekaDoc* GetDocument() const;
	bool isRunning;
	CWinThread* pThread;
	CWinThread* pPhoticThread;
	GraphData graphData;
	CBitmap bmp;
	boolean onPhotic;
	float factor;
// Operations
public:
	
// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CAmekaView();
	static CAmekaView* GetView();
	static UINT graphHandle(LPVOID pParam);			//handle thread
	static UINT photicHandle(LPVOID pParam);			//photic thread
	//int amekaDrawPos(CBitmap* bitmap);			//for draw current position only	
	int amekaDrawPos(CDC* pDC);			//for draw current position only	
	void setParentDoc(CAmekaDoc* doc);
	int drawBarGraph( void );
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	uint16_t crtPos;
	float distance;
	PrimaryDataType* dataBuffer;
	PrimaryDataType prePos;
	SecondaryDataType preBar;
	uint16_t count;
	uint16_t bufLen;
	bool isCountFull;
	bool isNull;
	static const uint16_t maxRange = 8192;
	uint8_t channelNum;
	static const uint16_t m_BaseLine = baseLine;
	static const uint16_t m_Amp = amp;
	CRITICAL_SECTION csess;
	CAmekaDoc* mDoc;
protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // debug version in AmekaView.cpp
inline CAmekaDoc* CAmekaView::GetDocument() const
   { return reinterpret_cast<CAmekaDoc*>(m_pDocument); }
#endif

#endif
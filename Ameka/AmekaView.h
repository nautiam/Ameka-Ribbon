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
#include "C_ColorToolTip.h"

#define baseLine 16383
#define amp 812

#define timeSleep 25
#define dataNum 8

#define CUSTOM_SCANBAR RGB(92,64,51)
#define CUSTOM_PEN RGB(72,61,139)
#define CUSTOM_PEN1 RGB(192,192,192)
#define CUSTOM_BARCOLOR RGB(41,102,0)
#define CUSTOM_BARBACK RGB(255,255,200)
#define MONNAME_BAR 35
#define SBAR_W 4
#define FOOT_RANGE 12

#define CODE_ERR_PDC_NULL -1
#define CODE_SUCCESS 0
#define CODE_ERR_OTHER -2

#define FACTOR 0.667

#define X_TOOLTIP 200
#define Y_TOOLTIP 80
#define CHECK_RANGE 5

class CAmekaView : public CScrollView
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
	virtual void OnInitialUpdate( );
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
	void drawTime(CDC* pDC, time_t x_time, uint16_t x_pos);
	uint16_t* getDataFromPos(CPoint mousePos, float crtPos, CAmekaView* pView);
	void resetData();
	uint16_t* getMaxMin(uint16_t* inputData);
	void drawRecData(CDC* pDC);
	void drawLeadName(CDC* pDC);
	void drawEvent(CDC* pDC, uint16_t evID);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	BOOL isDrawRec;
	CRITICAL_SECTION csess;
private:
	PrimaryDataType* dataBuffer;
	uint16_t count;
	uint8_t channelNum;
	uint16_t bufLen;
	static const uint16_t m_BaseLine = baseLine;
	static const uint16_t m_Amp = amp;

	float crtPos;
	float distance;
	PrimaryDataType prePos;

	SecondaryDataType preBar;
	bool isCountFull;
	bool isNull;
	static const uint16_t maxRange = 8192;

	CAmekaDoc* mDoc;
	int preTimePos;
	bool onDrawTime;
	bool isResize;
	uint16_t lastDistance;
	bool hasEv;
	uint16_t evPos;
	
	C_ColorToolTip m_Tips;
protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in AmekaView.cpp
inline CAmekaDoc* CAmekaView::GetDocument() const
   { return reinterpret_cast<CAmekaDoc*>(m_pDocument); }
#endif

#endif
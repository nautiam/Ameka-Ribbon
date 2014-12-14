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

// AmekaView.cpp : implementation of the CAmekaView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Ameka.h"
#endif

#include "AmekaDoc.h"
#include "Ameka.h"
#include "AmekaView.h"
#include <stdint.h>
//#include "easylogging++.h"
#include "AmekaLan.h"
#include "TGraphics.h"
#include <algorithm>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Gdiplus;

// CAmekaView

IMPLEMENT_DYNCREATE(CAmekaView, CView)

BEGIN_MESSAGE_MAP(CAmekaView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CAmekaView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL()
	ON_WM_KEYUP()
	ON_WM_MOUSELEAVE()
	ON_WM_ENTERSIZEMOVE()
	ON_WM_EXITSIZEMOVE()
END_MESSAGE_MAP()

// CAmekaView construction/destruction

CAmekaView::CAmekaView()
{
	// TODO: add construction code here
	isRunning = FALSE;
	crtPos = MONNAME_BAR + 2;
	isNull = TRUE;
	isCountFull = FALSE;
	pThread = NULL;
	pPhoticThread = NULL;
	count = 0;
	bufLen = 4096;
	channelNum = 16;
	graphData.scaleRate = 30;
	graphData.dotPmm = 200/30;
	graphData.paperSpeed = 75;
	graphData.sampleRate = SAMPLE_RATE;
	InitializeCriticalSection(&csess);
	onPhotic = FALSE;
	preTimePos = 0;
	onDrawTime = FALSE;
	dataBuffer = NULL;
	isResize = FALSE;
	lastDistance = 0;
	isDrawRec = FALSE;
	m_Tips.Create(CSize(X_TOOLTIP, Y_TOOLTIP), this);
	drawEnable = TRUE;
	isSmooth = TRUE;
	//m_Pos.Create(CSize(X_TOOLTIP, Y_TOOLTIP), this);
}

CAmekaView::~CAmekaView()
{
	DWORD exit_code= NULL;
	if (pThread != NULL)
	{
		GetExitCodeThread(pThread->m_hThread, &exit_code);
		if(exit_code == STILL_ACTIVE)
		{
			::TerminateThread(pThread->m_hThread, 0);
			CloseHandle(pThread->m_hThread);
		}
		pThread->m_hThread = NULL;
		pThread = NULL;
		isRunning = false;
	}

	exit_code = NULL;
	if (this->pPhoticThread != NULL)
	{
		GetExitCodeThread(this->pPhoticThread->m_hThread, &exit_code);
		if(exit_code == STILL_ACTIVE)
		{
			::TerminateThread(this->pPhoticThread->m_hThread, 0);
			CloseHandle(this->pPhoticThread->m_hThread);
		}
		this->pPhoticThread->m_hThread = NULL;
		this->pPhoticThread = NULL;
	}
	DeleteCriticalSection(&csess);
	if (dataBuffer)
		delete [] dataBuffer;
}

BOOL CAmekaView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	SetClassLong(this->m_hWnd, GCL_STYLE, (GetClassLong(this->m_hWnd,
		GCL_STYLE) & ~(CS_HREDRAW | CS_VREDRAW)));
	return CView::PreCreateWindow(cs);
}

// CAmekaView drawing

void CAmekaView::OnDraw(CDC* pDC)
{
	CAmekaDoc* pDoc = GetDocument();
	CTGraphics graphic;

	ASSERT_VALID(pDoc);
	if (!pDoc || !drawEnable)
		return;

	//check if draw record data
	if (isDrawRec)
	{
		drawLeadName(pDC);
		drawRecData(pDC);
		if (onPhotic)
		{
			photic_processing(FRE_STEP, this->GetDocument(), this->GetScrollPos(SB_HORZ));
			drawBarGraph();
		}
		return;
	}

	CBitmap Wbmp;
	float maxWidth;
	
	//CView::OnDraw(pDC);
	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);

	CRect rect;
    GetClientRect(&rect);

	if (!isRunning)
		this->isResize = TRUE;

	if (onPhotic)
			maxWidth = rect.Width()*FACTOR;
		else
			maxWidth = rect.Width();

	Wbmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height() );

	CBitmap* pOldBmp = MemDC.SelectObject(&Wbmp);
	
	CRect mrect(0,0,rect.Width(),rect.Height());
	MemDC.FillRect(mrect,WHITE_BRUSH);
	
	//draw foot line
	CPen silverPen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
	CPen* oldPen = MemDC.SelectObject(&silverPen);
	MemDC.MoveTo(MONNAME_BAR + 1, rect.Height() - FOOT_RANGE);
	MemDC.LineTo(rect.Width(), rect.Height() - FOOT_RANGE);
	MemDC.SelectObject(oldPen);
	DeleteObject(&silverPen);

	EnterCriticalSection(&csess);
	//draw whole graph
	if (isCountFull || count != 0)
	{
		distance = (float)graphData.paperSpeed*(float)graphData.dotPmm/graphData.sampleRate;
		uint16_t numPos = crtPos/distance;
		uint16_t firstPos;

		if (!isCountFull)
			firstPos = numPos>=count?0:count-1-numPos;
		else
			firstPos = (count-1+bufLen-numPos)%bufLen;

		int j,tmp;
		
		channelNum = this->GetDocument()->mMon.mList.GetCount();
		//drwa backward
		for(int i = 0; i < channelNum; i++)
		{
			j = 0;
			while(crtPos - distance*(j+1) > MONNAME_BAR)
			{
				if (dataBuffer[(count-1+bufLen-j)%bufLen].isDraw)
				{
					CPen* tmpPen = MemDC.SelectObject(&silverPen);
					MemDC.MoveTo((crtPos - distance*(j+1)), 0);
					MemDC.LineTo((crtPos - distance*(j+1)), rect.Height() - FOOT_RANGE);
					MemDC.SelectObject(tmpPen);

					drawTime(&MemDC, dataBuffer[(count-1+bufLen-j)%bufLen].time, (crtPos - distance*j));
				}
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(count-1+bufLen-j)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > (rect.Height() - FOOT_RANGE))
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				//MemDC.MoveTo((crtPos - distance*j), tmp);
				int x1 = (crtPos - distance*j);
				int y1 = tmp;
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(count-1+bufLen-j-1)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > (rect.Height() - FOOT_RANGE))
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				//MemDC.LineTo((crtPos - distance*(j+1)), tmp);
				int x2 = (crtPos - distance*(j + 1));
				int y2 = tmp;
				if (isSmooth)
				{
					graphic.DrawLine(MemDC.m_hDC, x1, y1, x2, y2, RGB(0, 0, 0));
				}
				else
				{
					MemDC.MoveTo(x1, y1);
					MemDC.LineTo(x2, y2);
				}
				j++;
			}
		}
		//draw forward
		for(int i = 0; i < channelNum; i++)
		{
			j = 0;
			while((maxWidth - distance*j) >= crtPos)
			{
				if (firstPos == 0)
					break;
				if (dataBuffer[(firstPos+bufLen-j)%bufLen].isDraw)
				{
					CPen* tmpPen = MemDC.SelectObject(&silverPen);
					MemDC.MoveTo((maxWidth-distance*(j)), 0);
					MemDC.LineTo((maxWidth-distance*(j)), rect.Height() - FOOT_RANGE);
					MemDC.SelectObject(tmpPen);;

					drawTime(&MemDC, dataBuffer[(firstPos+bufLen-j)%bufLen].time, (maxWidth-distance*(j)));
				}
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(firstPos+bufLen-j)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height() - FOOT_RANGE)
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				//MemDC.MoveTo((maxWidth-distance*j), tmp);
				int x1 = (maxWidth - distance*j);
				int y1 = tmp;
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(firstPos+bufLen-j-1)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height() - FOOT_RANGE)
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				//MemDC.LineTo((maxWidth-distance*(j+1)), tmp);
				int x2 = (maxWidth - distance*(j + 1));
				int y2 = tmp;
				if (isSmooth)
				{
					graphic.DrawLine(MemDC.m_hDC, x1, y1, x2, y2, RGB(0, 0, 0));
				}
				else
				{
					MemDC.MoveTo(x1, y1);
					MemDC.LineTo(x2, y2);
				}
				j++;
			}
		}
		if (!isRunning)
		{
			CBrush brushS(CUSTOM_PEN);
			//CBrush* pOldBrush1 = MemDC.SelectObject(&brushS);
			MemDC.FillRect(CRect(crtPos, 0, crtPos, rect.Height() - FOOT_RANGE),&brushS);
		}

	}

	//draw photic
	if (onPhotic)
	{
		float range = theApp.photicMax - theApp.photicMin;
		int barNum = range / theApp.photicTick;

		CBrush brushBar;
		brushBar.CreateSolidBrush(CUSTOM_BARBACK);
		CRect mrectBar(maxWidth,0,rect.Width(),rect.Height());
		MemDC.FillRect(mrectBar,&brushBar);
		DeleteObject(&brushBar);
		//draw grid
		CPen pen2(PS_SOLID, 1, CUSTOM_PEN1);
		MemDC.SelectObject(&pen2);
		MemDC.SetBkMode(TRANSPARENT);
		float barW = (float)(rect.Width() - maxWidth) / barNum;
		for (int i = 0; i <= barNum; i++)
		{
			MemDC.MoveTo(maxWidth + i*barW, rect.Height() - 5);
			MemDC.LineTo(maxWidth + i*barW, 0);
			//draw text
			CFont txtFont;
			txtFont.CreatePointFont(70, _T("Arial"), &MemDC);
			CFont* tmpFont = MemDC.SelectObject(&txtFont);
			CString text;
			text.Format(_T("%d"), (int)(theApp.photicTick*i + theApp.photicMin));
			MemDC.TextOutW(maxWidth + (int)(i*barW), (rect.Height() - FOOT_RANGE), text);
			MemDC.SelectObject(tmpFont);
			DeleteObject(&txtFont);
			
		}
	}
	drawLeadName(&MemDC);
	
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);
	LeaveCriticalSection(&csess);
	//UpdateWindow();
	MemDC.SelectObject(pOldBmp); 
	
	MemDC.DeleteDC();
	DeleteObject(&Wbmp);

	//DeleteObject(pOldBmp);
	// TODO: add draw code for native data here
	
}

//draw mouse move
void CAmekaView::drawMouseMove(CDC* pDC, int xPos, int maxVal, int minVal, int drawVal)
{

	CRect rtWin;
	GetClientRect(&rtWin);

	CDC MemDC;
	CBitmap bmp;

	CAmekaView* pView = CAmekaView::GetView();
	CAmekaDoc* pDoc = pView->GetDocument();
	float mDistance = pView->distance;
	//CRect rect;

	MemDC.CreateCompatibleDC(pDC);
	
	/*BITMAP bm;
    oldBitmap.GetObject(sizeof(BITMAP),&bm);

	if (bm.bmHeight >= 0)
	{
 		int bmpW = bm.bmWidth;
		int bmpH = bm.bmHeight;

		MemDC.SelectObject(&oldBitmap);
		pDC->BitBlt(0, 0, bmpW, bmpH, &MemDC, 0, 0, SRCCOPY);
	}*/
	int maxWidth = (maxVal - minVal)*distance;
	CRect bmpRect(0, 0, maxWidth, rtWin.Height() - FOOT_RANGE);
	
	bmp.CreateCompatibleBitmap(pDC, bmpRect.Width(), bmpRect.Height());
	MemDC.SelectObject(&bmp);

	MemDC.FillRect(rtWin, WHITE_BRUSH);

	//pView->GetClientRect(&rtWin);
	uint16_t channelNum = pDoc->mMon.mList.GetCount();
	for (int i = 0; i < maxVal - minVal; i++)
	{
		if (pDoc->primaryDataArray[minVal + i].isDraw)
		{
			CPen silverPen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
			CPen* tmpPen = MemDC.SelectObject(&silverPen);
			MemDC.MoveTo(i*distance, 0);
			MemDC.LineTo(i*distance, rtWin.Height() - FOOT_RANGE);
			MemDC.SelectObject(tmpPen);
		}
		for (int j = 0; j < channelNum; j++)
		{
			if (j != drawVal)
			{
				int tmp = ((bmpRect.Height()*j)/channelNum + (bmpRect.Height()/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i].value[j]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
				if (tmp > bmpRect.Height())
					tmp = bmpRect.Height();
				if (tmp < 0)
					tmp = 0;
				MemDC.MoveTo(i*mDistance, tmp);
				tmp = ((bmpRect.Height()*j)/channelNum + (bmpRect.Height()/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i + 1].value[j]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
				if (tmp > bmpRect.Height())
					tmp = bmpRect.Height();
				if (tmp < 0)
					tmp = 0;
				MemDC.LineTo((i+1)*mDistance, tmp);
			}
		}
	}
	CPen redPen(PS_SOLID, 1, RGB(255, 15, 15));
	CPen* oldPen = MemDC.SelectObject(&redPen);
	for (int i = 0; i < maxVal - minVal; i++)
	{
		{
			int tmp = (((rtWin.Height() - FOOT_RANGE)*drawVal)/channelNum + ((rtWin.Height() - FOOT_RANGE)/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i].value[drawVal]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
			if (tmp > (rtWin.Height() - FOOT_RANGE))
				tmp = rtWin.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			MemDC.MoveTo(i*mDistance, tmp);
			tmp = (((rtWin.Height() - FOOT_RANGE)*drawVal)/channelNum + ((rtWin.Height() - FOOT_RANGE)/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i + 1].value[drawVal]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
			if (tmp > (rtWin.Height() - FOOT_RANGE))
				tmp = rtWin.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			MemDC.LineTo((i+1)*mDistance, tmp);
		}
	}
	pDC->BitBlt(xPos, 0, bmpRect.Width(), bmpRect.Height(), &MemDC, 0, 0, SRCCOPY);
	DeleteObject(&redPen);
	MemDC.DeleteDC();
}


// CAmekaView printing

CAmekaView * CAmekaView::GetView()
{
    CMDIChildWnd * pChild =
        ((CMDIFrameWnd*)(AfxGetApp()->m_pMainWnd))->MDIGetActive();

    if ( !pChild )
        return NULL;

    CView * pView = pChild->GetActiveView();

    if ( !pView )
        return NULL;

    // Fail if view is of wrong kind
    if ( ! pView->IsKindOf( RUNTIME_CLASS(CAmekaView) ) )
        return NULL;

    return (CAmekaView *) pView;
}

void CAmekaView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CAmekaView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CAmekaView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CAmekaView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CAmekaView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CAmekaView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	CMenu rMenu;
	rMenu.LoadMenuW(IDR_POPUP_EDIT);
	CMenu* subMenu = rMenu.GetSubMenu(0);
	UINT tmpFlags = TPM_LEFTALIGN|TPM_RETURNCMD|TPM_NONOTIFY;

	UINT state = subMenu->GetMenuState(ID_MN_Smooth, MF_BYCOMMAND);
	ASSERT(state != 0xFFFFFFFF);

	if (!isSmooth)
		subMenu->CheckMenuItem(ID_MN_Smooth, MF_UNCHECKED | MF_BYCOMMAND);
	else
		subMenu->CheckMenuItem(ID_MN_Smooth, MF_CHECKED | MF_BYCOMMAND);

	int rID = subMenu->TrackPopupMenu(tmpFlags, point.x, point.y, this);
	switch (rID)
	{
	case ID_MN_Spec:
		theApp.OnPhotic();
		break;
	case ID_MN_Setup:
		theApp.OnOption();
		break;
	case ID_MN_Info:
		theApp.OnInfo();
		break;
	case ID_MN_Smooth:
		isSmooth = !isSmooth;
		OnDraw(GetDC());
		break;
	default:
		break;
	}
#endif
}


// CAmekaView diagnostics

#ifdef _DEBUG
void CAmekaView::AssertValid() const
{
	CView::AssertValid();
}

void CAmekaView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAmekaDoc* CAmekaView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAmekaDoc)));
	return (CAmekaDoc*)m_pDocument;
}
#endif //_DEBUG


// CAmekaView message handlers


BOOL CAmekaView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


void CAmekaView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	//return;
	CView::OnMouseMove(nFlags, point);

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = this->m_hWnd;
		
	::_TrackMouseEvent(&tme);

	if (!isNull && !isRunning && isDrawRec)
	{
		int X = GetSystemMetrics( SM_CXSCREEN );
		int Y = GetSystemMetrics( SM_CYSCREEN );

		CPoint ptLog = point;
		ClientToScreen(&ptLog);

		CRect rect;
		GetClientRect(&rect);

		if (onPhotic)
		{
			if (point.x > (rect.Width()*FACTOR))
				return;
		}

		if (point.x < MONNAME_BAR + 2)
		{
			m_Tips.HideTips();
			return;
		}
		this->OnDraw(this->GetDC());
		
		/*if (point.x <= MONNAME_BAR + 2 || point.y > rect.Height() - FOOT_RANGE || point.x > rect.Width() || point.y < 0)
			return;*/

		if ((point.x - MONNAME_BAR - 2)/distance + this->GetScrollPos(SB_HORZ) >= GetDocument()->counter)
		{
			m_Tips.HideTips();
			return;
		}

		uint16_t* posResult = this->getDataFromPos(point, this);

		uint16_t* valResult = this->getMaxMin(posResult);
		if (valResult[0] < this->GetScrollPos(SB_HORZ))
			valResult[0] = this->GetScrollPos(SB_HORZ);;

		if (valResult[1] < this->GetScrollPos(SB_HORZ))
			valResult[1] = this->GetScrollPos(SB_HORZ);;

		if (onPhotic)
		{
			if ((valResult[0] - this->GetScrollPos(SB_HORZ))*distance > (rect.Width()*FACTOR - MONNAME_BAR - 2))
				valResult[0] = (rect.Width()*FACTOR - MONNAME_BAR - 2)/distance + this->GetScrollPos(SB_HORZ);;

			if ((valResult[1] - this->GetScrollPos(SB_HORZ))*distance > (rect.Width()*FACTOR - MONNAME_BAR - 2))
				valResult[1] = (rect.Width()*FACTOR - MONNAME_BAR - 2)/distance + this->GetScrollPos(SB_HORZ);;
		}

		if (!valResult)
			return;

		CString strTemp;

		CAmekaDoc* pDoc = GetDocument();

		int elecVal = (float(pDoc->primaryDataArray[posResult[0]].value[posResult[1]])/BASELINE)*ELEC_VAL;
		/*strTemp.Format((L"Value: %d\nMin: %d\nMax: %d\n"), pDoc->primaryDataArray[posResult[0]].value[posResult[1]],
			pDoc->primaryDataArray[valResult[0]].value[posResult[1]], pDoc->primaryDataArray[valResult[1]].value[posResult[1]]) ;*/
		strTemp.Format((L"%d mV"), elecVal);
		//strTemp.Format(L"Data value: %d\r\nMix value: %d\r\nMax value: %d", this->dataBuffer[posResult[0]].value[posResult[1]], fuck[0], fuck[1]);
		// show tool tip in mouse move
		int xPos, yPos;
		if (ptLog.x + X_TOOLTIP + 5 > X)
		{
			xPos = X - X_TOOLTIP - 5;
		}
		else
		{
			xPos = ptLog.x + 5;
		}
		if (ptLog.y + Y_TOOLTIP + 5 > Y)
		{
			yPos = Y - Y_TOOLTIP - 5;
		}
		else
		{
			yPos = ptLog.y + 5;
		}
		if (valResult[0] < valResult[1])
		{
			int xDraw = (valResult[0] - (float)this->GetScrollPos(SB_HORZ))*distance + MONNAME_BAR + 2;
			drawMouseMove(this->GetDC(), xDraw, valResult[1], valResult[0], posResult[1]);
		}
		else
		{
			int xDraw = (valResult[1] - this->GetScrollPos(SB_HORZ))*distance + MONNAME_BAR + 2;
			drawMouseMove(this->GetDC(), xDraw, valResult[0], valResult[1], posResult[1]);
		}
		//m_Pos.SetValue(valResult, posResult[1]);
		//m_Pos.SetRectSize(CSize(distance*(valResult[1] - valResult[0]), rect.Height() - FOOT_RANGE));
		//this->GetWindowRect(&rect);
		//m_Pos.ShowRect(rect.left + valResult[0]*distance - this->GetScrollPos(SB_HORZ) + MONNAME_BAR + 4, rect.top + 2);
		m_Tips.ShowTips(xPos, yPos, strTemp);
		SetFocus();
		delete [] posResult;
		delete [] valResult;

		CView::OnMouseMove(nFlags, point);
	}
	else
	{
		m_Tips.HideTips();
		//m_Pos.HideRect();
	}
}

void CAmekaView::resetData()
{

	isRunning = FALSE;
	crtPos = MONNAME_BAR;
	isNull = TRUE;
	isCountFull = FALSE;
	pThread = NULL;
	delete [] dataBuffer;
	dataBuffer = NULL;
	pPhoticThread = NULL;
	count = 0;
	preTimePos = 0;
	onDrawTime = FALSE;
}

void CAmekaView::OnInitialUpdate( )
{
	CScrollView::OnInitialUpdate();

	CRect rect;
	GetClientRect(&rect);

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = rect.Width();
	sizeTotal.cy = rect.Height();
	SetScrollSizes(MM_TEXT, sizeTotal);
}	

//graph thread
UINT CAmekaView::graphHandle(LPVOID pParam)
{
	CAmekaView* pnt = (CAmekaView *)pParam;
	CDC* pDC = pnt->GetDC();
	pnt->setParentDoc(pnt->GetDocument());
	pnt->dataBuffer = (PrimaryDataType*)malloc(pnt->bufLen*sizeof(PrimaryDataType));
	int ret;
	while(1)
	{
		//ret = pnt->drawAtPos(&pnt->bmp);
		pnt->drawAtPos(pDC);

		/*
		if(ret == -1)
			return -1;
			*/
		::Sleep(timeSleep);
	}
	
	DeleteObject(pDC);
	return 0;
}

UINT CAmekaView::photicHandle(LPVOID pParam)
{
	CAmekaView* pnt = (CAmekaView *)pParam;
	CDC* pDC = pnt->GetDC();
	pnt->setParentDoc(pnt->GetDocument());
	while(1)
	{
		//ret = pnt->drawAtPos(&pnt->bmp);
		if (pnt->onPhotic)
		{
			pnt->drawBarGraph();
		/*
		if(ret == -1)
			return -1;
			*/
		}
		::Sleep(timeSleep);
	}
	DeleteObject(pDC);
	return 0;
}

void CAmekaView::setParentDoc(CAmekaDoc* doc)
{
	this->mDoc = doc;
}

//int CAmekaView::drawAtPos(CBitmap* bitmap)
int CAmekaView::drawAtPos(CDC* pDC)
{
	//TBD
	//CBitmap bitmap;
	CDC MemDC;
	CBitmap* bitmap = new CBitmap;
	CTGraphics graphic;
	//RawDataType* data = new RawDataType[dataNum];
	uint16_t buflen = 0;
	float maxWidth;
	PrimaryDataType* data;
	uint16_t crtEVID = 10;
	
	if (pDC == NULL || !this->mDoc->PrimaryData)
	{
		delete bitmap;
		return -1;
	}
	
	this->isResize = FALSE;

	CRect rect;
    GetClientRect(&rect);

	//data = theApp.pIO->RawData->popData(dataNum);
	//CAmekaDoc *doc = CAmekaDoc::GetDoc();

	EnterCriticalSection(&csess);
	distance = (float)graphData.paperSpeed*(float)graphData.dotPmm/graphData.sampleRate;
	LeaveCriticalSection(&csess);

	if (onPhotic)
		maxWidth = rect.Width()*FACTOR;
	else
		maxWidth = rect.Width();

	if ((maxWidth - crtPos - SBAR_W)/distance > dataNum)
	{
		data = this->mDoc->PrimaryData->popData(dataNum);
	}
	else
	{
		data = this->mDoc->PrimaryData->popData((maxWidth - crtPos - SBAR_W)/distance);
	}
	buflen = this->mDoc->PrimaryData->rLen;
	if ((buflen <= 0) || (data == NULL))
	{
		delete bitmap;
		return -2;
	}	

	for (uint16_t i = 0; i < buflen; i++)
	{
		if (count == (bufLen-1))
			isCountFull = true;
		dataBuffer[count] = data[i];
		count = (count+1)%bufLen;
	}
	
	if (NULL == MemDC.CreateCompatibleDC(pDC))
	{
		MemDC.DeleteDC();
		delete [] data;
		delete bitmap;
		return -1;
	}

	int drawW = ceil(distance * buflen + SBAR_W);
	if(crtPos > maxWidth)
	{
		crtPos = MONNAME_BAR + 2;
	}
	if(bitmap != NULL)
	{
		if (crtPos + drawW > maxWidth)
		{
			if(NULL == bitmap->CreateCompatibleBitmap(pDC, (maxWidth - crtPos), rect.Height()))
			{
				DWORD tmp = GetLastError();
				//LOG(ERROR) << static_cast <int>(tmp);
				delete [] data;
				delete bitmap;
				return -1;
			}
		}
		else
		{
			if(NULL == bitmap->CreateCompatibleBitmap(pDC, drawW, rect.Height()))
			{
				DWORD tmp = GetLastError();
				//LOG(ERROR) << static_cast <int>(tmp);
				delete [] data;
				delete bitmap;
				return -1;
			}
		}
	}

	MemDC.SelectObject(bitmap);
	//Graphics graphics(MemDC.m_hDC);

	//memcpy(dataBuffer[count],data,sizeof(RawDataType));
	//count = (count+1)%bufLen;
	
	//erase current position
	CBrush brush;
	brush.CreateSolidBrush(RGB(255,255,255));
	CRect mrect(0, 0, rect.Width(), rect.Height());
	MemDC.FillRect(mrect,&brush);
	
	CPen silverPen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
	
	float tmp = 0;

	if (crtPos == MONNAME_BAR)
	{
		CPen thick_pen(PS_SOLID, 2, CUSTOM_PEN);
		CPen* oldPen = MemDC.SelectObject(&thick_pen);

		MemDC.MoveTo(1, 0);
		MemDC.LineTo(1, rect.Height());

		MemDC.SelectObject(oldPen);
		DeleteObject(&thick_pen);
	}
	
	//draw foot line
	CPen* tmpPen = MemDC.SelectObject(&silverPen);
	MemDC.MoveTo(0, rect.Height() - FOOT_RANGE);
	MemDC.LineTo(rect.Width(), rect.Height() - FOOT_RANGE);
	MemDC.SelectObject(tmpPen);

	channelNum = this->GetDocument()->mMon.mList.GetCount();
	//draw all point to current bitmap
	if (!isNull)
	{
		for(int i = 0; i < channelNum;i++)
		{
			if (data[0].eventID != 10)
			{
				crtEVID = data[0].eventID;
			}
			if (data[0].isDraw)
			{
				preTimePos = crtPos;
				onDrawTime = TRUE;

				CPen* tmpPen = MemDC.SelectObject(&silverPen);
				MemDC.MoveTo(0, 0);
				MemDC.LineTo(0, rect.Height() - FOOT_RANGE);
				//graphic.DrawLine(MemDC.m_hDC, 0, 0, 0, rect.Height() - FOOT_RANGE, RGB(0xC0, 0xC0, 0xC0));
				MemDC.SelectObject(tmpPen);

				drawTime(pDC, data[0].time, crtPos);
			}
			tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)prePos.value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
				tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
			//MemDC.MoveTo(0, (int)tmp);
			int x1 = 0;
			int y1 = (int)tmp;
			tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)data[0].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
				tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel(distance,tmp ,CUSTOM_PEN);
			//MemDC.LineTo(int(distance), (int)tmp);
			int x2 = int(distance);
			int y2 = (int)tmp;
			if (isSmooth)
			{
				graphic.DrawLine(MemDC.m_hDC, x1, y1, x2, y2, RGB(0, 0, 0));
			}
			else
			{
				MemDC.MoveTo(x1, y1);
				MemDC.LineTo(x2, y2);
			}
		}
	}
	isNull = FALSE;
	for(int j = 1; j < buflen; j++)
	{
		if (data[j].eventID != 10)
		{
			crtEVID = data[j].eventID;
		}
		for(int i = 0; i < channelNum; i++)
		{
			if (data[j].isDraw)
			{
				preTimePos = crtPos + j*distance;
				onDrawTime = TRUE;

				CPen* tmpPen = MemDC.SelectObject(&silverPen);
				MemDC.MoveTo((int)(j*distance), 0);
				MemDC.LineTo((int)(j*distance), rect.Height() - FOOT_RANGE);
				MemDC.SelectObject(tmpPen);;

				drawTime(pDC, data[j].time, crtPos + j*distance);
			}
			tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)data[j-1].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*j),tmp ,CUSTOM_PEN);
			//MemDC.MoveTo((int)(distance*j), tmp);		//draw 16 channel
			int x1 = (int)(distance*j);
			int y1 = tmp;
			tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)data[j].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*(j+1)),tmp ,CUSTOM_PEN);
			//MemDC.LineTo((int)(distance*(j+1)), tmp);
			int x2 = (int)(distance*(j + 1));
			int y2 = tmp;
			if (isSmooth)
			{
				graphic.DrawLine(MemDC.m_hDC, x1, y1, x2, y2, RGB(0, 0, 0));
			}
			else
			{
				MemDC.MoveTo(x1, y1);
				MemDC.LineTo(x2, y2);
			}
		}
	}
	
	//draw scan bar
	CBrush brushS(CUSTOM_PEN);
	//CBrush* pOldBrush1 = MemDC.SelectObject(&brushS);
	MemDC.FillRect(CRect((int)(distance * buflen), 0, (int)(distance * buflen + SBAR_W), rect.Height() - FOOT_RANGE),&brushS);
	EnterCriticalSection(&csess);
	if (onDrawTime || (crtPos  < (preTimePos + MONNAME_BAR/2 + 1) && crtPos > preTimePos))
		pDC->BitBlt((int)crtPos, 0, (int)(distance * buflen + SBAR_W), rect.Height() - FOOT_RANGE + 1, &MemDC, 0, 0, SRCCOPY);
	else
		pDC->BitBlt((int)crtPos, 0, (int)(distance * buflen + SBAR_W), rect.Height(), &MemDC, 0, 0, SRCCOPY);
	onDrawTime = FALSE;
	prePos = data[buflen-1];
	
	crtPos += distance*buflen;
	if(crtPos + distance + SBAR_W > maxWidth)
	{
		lastDistance = maxWidth - crtPos + SBAR_W;
		crtPos = MONNAME_BAR + 2;
	}
	crtPos = (int)crtPos;
	LeaveCriticalSection(&csess);
	//free all resource
	if (hasEv)
	{
		drawEvent(pDC, GetDocument()->eventID);
		hasEv = FALSE;
	}
	//DeleteObject(brushS);
	//DeleteObject(brush);
	//DeleteObject(bitmap);
	delete[] data;
	DeleteObject(bitmap);
	DeleteObject(&brushS);
	DeleteObject(&brush);
	DeleteObject(&silverPen);
	delete bitmap;
	bitmap = NULL;
	MemDC.DeleteDC();
	//delete bitmap;
	return 0;
	
}

void CAmekaView::drawTime(CDC* pDC, time_t x_time, uint16_t x_pos)
{
	if (!pDC)
		return;

	if (x_time <= 0)
		return;

	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, MONNAME_BAR + 2, FOOT_RANGE - 1);
	MemDC.SelectObject(&bmp);

	CRect rect;
	GetClientRect(&rect);

	CRect txtRect(0, 0, MONNAME_BAR + 2, FOOT_RANGE - 1);
	MemDC.FillRect(&txtRect, WHITE_BRUSH);
	CFont txtFont;
	txtFont.CreatePointFont(70, _T("Arial"), pDC);
	MemDC.SelectObject(&txtFont);

	//draw time
	time_t tim = x_time;
	/*struct tm timeinfo;
	time (&tim);
	localtime_s (&timeinfo, &tim);*/
	CTime temp(tim);
	CString s;
	s = temp.Format("%H:%M:%S");
	//s.Format(_T("%02d:%02d:%02d"), timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	MemDC.DrawTextW(s, &txtRect, 0);

	if (x_pos  > MONNAME_BAR + MONNAME_BAR/2 + 1)
		pDC->BitBlt(x_pos - MONNAME_BAR/2 - 1, rect.Height() - FOOT_RANGE + 1, x_pos + MONNAME_BAR/2 + 1, rect.Height(), &MemDC, 0, 0, SRCCOPY);
	else
		pDC->BitBlt(MONNAME_BAR, rect.Height() - FOOT_RANGE + 1, MONNAME_BAR + MONNAME_BAR + 2, rect.Height(), &MemDC, 0, 0, SRCCOPY);
	DeleteObject(&txtRect);
	DeleteObject(&txtFont);
	MemDC.DeleteDC();
}

int CAmekaView::drawBarGraph( void )
{
	CDC MemDC;

	float startPos;
	if (!this)
		return -1;
	CAmekaDoc* pDoc = this->GetDocument();
	if (pDoc == NULL)
		return -1;


	//if (data->fre > theApp.photicMax)
	//	return 1;

	//mDistance = (float)graphData.paperSpeed*(float)graphData.dotPmm/graphData.sampleRate;
	CRect rect;
    GetClientRect(&rect);

	CDC* pDC = GetDC();
	//RawDataType* data = new RawDataType[dataNum];
	
	if (pDC == NULL)
		return -1;

	if (NULL == MemDC.CreateCompatibleDC(pDC))
	{
		MemDC.DeleteDC();
		DeleteObject(pDC);
		return -1;
	}

	if (onPhotic)
		startPos = rect.Width()*FACTOR;
	else
		startPos = rect.Width();

	float range = theApp.photicMax - theApp.photicMin;
	int barNum = range/pDoc->mDSP.epocLength;

	CBitmap* bitmap = new CBitmap;
	if(bitmap != NULL)
	{
		if(NULL == bitmap->CreateCompatibleBitmap(pDC, (rect.Width()-startPos), rect.Height()))
		{
			DWORD tmp = GetLastError();
			////LOG(ERROR) << static_cast <int>(tmp);
			DeleteObject(pDC);
			delete bitmap;
			return -1;
		}
	}
	//fill background
	CBitmap* pOldBmp = MemDC.SelectObject(bitmap);

	CBrush brush;
	brush.CreateSolidBrush(CUSTOM_BARBACK);
	CRect mrect(0,0,rect.Width(),rect.Height());
	MemDC.FillRect(mrect,&brush);

	//initialize
	CBrush brushS(CUSTOM_BARCOLOR);
	MemDC.SelectObject(brushS);

	channelNum = pDoc->mMon.mList.GetCount();

	uint16_t buflen = SAMPLE_RATE/FRE_STEP;
	SecondaryDataType* data = pDoc->SecondaryData->checkPopData(buflen/2);
	int size = pDoc->SecondaryData->rLen;
	if (!data || buflen <= 0)
		return -1;
	
	//draw data
	for (int i = 0; i < buflen/2; i++)
	{
		float freVal = data[i].fre - theApp.photicMin;
		int barCount = freVal/pDoc->mDSP.epocLength;
		float barPos = (float)barCount*(rect.Width()-startPos)/barNum;
		float barW = (float)range/barCount;
		for (int j = 0; j < channelNum; j++)
		{
			if ((data[i].value[j])/theApp.photicWRate < 1)
				continue;
			CRect barRect(abs(barPos - barW/2), (j+1)*(rect.Height() - FOOT_RANGE)/channelNum - (data[i].value[j])/theApp.photicWRate, 
				abs((rect.Width() - startPos)/barNum + barPos - barW/2),(j+1)*(rect.Height() - FOOT_RANGE)/channelNum);
			MemDC.FillRect(barRect,&brushS);

		}
	}
	
	//draw grid
	MemDC.SetBkMode(TRANSPARENT);
	int gridNum = range/theApp.photicTick;
	CPen pen2(PS_SOLID, 1, CUSTOM_PEN1);
	CPen* pOldPen = MemDC.SelectObject(&pen2);
	CFont txtFont;
	txtFont.CreatePointFont(70, _T("Arial"), &MemDC);
	float barGridW = (float)(rect.Width() - startPos ) / gridNum;
	for (int i = 0; i < gridNum; i++)
	{
		MemDC.MoveTo(i*barGridW, rect.Height() - FOOT_RANGE);
		MemDC.LineTo(i*barGridW, 0);
		CString text;
		MemDC.SelectObject(&txtFont);
		text.Format(_T("%d"), (int)(theApp.photicTick*i + theApp.photicMin));
		MemDC.TextOutW(i*barGridW, (rect.Height() - FOOT_RANGE), text);
	}
	MemDC.SelectObject(pOldPen);
	DeleteObject(&pen2);

	pDC->BitBlt(startPos , 0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);
	
	MemDC.SelectObject(pOldBmp);
	DeleteObject(bitmap);
	//DeleteObject(pen2);
	DeleteObject(&brush);
	DeleteObject(&brushS);
	DeleteObject(&pen2);
	MemDC.DeleteDC();
	delete bitmap;
	delete [] data;

	return 0;
};

uint16_t* CAmekaView::getDataFromPos(CPoint mousePos, CAmekaView* pView)
{
	uint16_t* result = new uint16_t[2];
	CRect rect;
	this->GetWindowRect(&rect);
	int maxWidth = rect.Width();
	int xMousePos = mousePos.x;
	int yMousePos = mousePos.y;

	CAmekaDoc* pDoc = GetDocument();
	
	/*if (onPhotic)
		maxWidth = rect.Width()*FACTOR;
	else
		maxWidth = rect.Width();*/

	uint64_t posNum = this->GetScrollPos(SB_HORZ) + float(xMousePos - MONNAME_BAR - 2)/distance;

	//float xDistance;
	//float distance = (float)this->graphData.paperSpeed*(float)this->graphData.dotPmm/this->graphData.sampleRate;
	//xDistance = crtPoint + xMousePos - MONNAME_BAR - 2;
	//get position
	
	int pos = 0;
	
	uint32_t val = abs(mousePos.y - ((float(rect.Height() - FOOT_RANGE)/channelNum)/2
			- (((float)pDoc->primaryDataArray[posNum].value[0]-m_BaseLine)/m_Amp)*graphData.scaleRate));
	for (int i = 0; i < channelNum; i++)
	{
		PrimaryDataType crtData = pDoc->primaryDataArray[posNum];
		int tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 
			- (((float)crtData.value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
		if (tmp < 0)
			tmp = 0;
		if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
		if (abs(mousePos.y - (long)tmp) <= val)
		{
			pos = i;
			val = abs(mousePos.y - tmp);
		}
	}

	result[0] = posNum;
	result[1] = pos;
	return result;
}

uint16_t* CAmekaView::getMaxMin(uint16_t* inputData)
{
	uint16_t* result = new uint16_t[2];
	//find low and high range
	int lowVal, highVal;
	CAmekaDoc* pDoc = GetDocument();
	if (!pDoc || !inputData)
		return NULL;
	if (inputData[0] < 0 || inputData[0] > pDoc->counter)
		return NULL;

	if ((inputData[0] - CHECK_RANGE) > 0)
	{
		lowVal = inputData[0] - CHECK_RANGE;
	}
	else
	{
		lowVal = 0;
	}
	if ((inputData[0] + CHECK_RANGE ) < pDoc->counter )
	{
		highVal = inputData[0] + CHECK_RANGE;
	}
	else
	{
		highVal = pDoc->counter;
	}
	if (lowVal >= highVal)
		return NULL;
		
	//find max and min position in range
	uint16_t maxVal = lowVal;
	uint16_t minVal = lowVal;
	for (int i = lowVal; i < highVal; i++)
	{
		if (pDoc->primaryDataArray[i].value[inputData[1]] < pDoc->primaryDataArray[minVal].value[inputData[1]])
		{
			minVal = i;
		}
		if (pDoc->primaryDataArray[i].value[inputData[1]] > pDoc->primaryDataArray[maxVal].value[inputData[1]])
		{
			maxVal = i;
		}
	}
	
	result[0] = minVal;
	result[1] = maxVal;
	return result;
}

void CAmekaView::drawRecData(CDC* pDC)
{
	CDC MemDC;
	CBitmap bmp;
	CRect rect;

	CAmekaDoc* pDoc = GetDocument();
	if (!pDoc)
	{
		return;
	}

	GetClientRect(&rect);

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	EnterCriticalSection(&csess);
	distance = (float)graphData.paperSpeed*(float)graphData.dotPmm/graphData.sampleRate;
	LeaveCriticalSection(&csess);

	//sizeTotal.cx = pDoc->counter*distance + MONNAME_BAR + 2;
	//sizeTotal.cy = rect.Height();
	//SetScrollSizes(MM_TEXT, sizeTotal);
	SetScrollRange(SB_HORZ, 0, pDoc->counter);

	MemDC.CreateCompatibleDC(pDC);
	uint32_t maxWidth;
	/*if (distance*pDoc->counter > rect.Width())
		maxWidth = distance*pDoc->counter;
	else*/
	if ((rect.Width() > MONNAME_BAR + 2))
		maxWidth = rect.Width() - MONNAME_BAR - 2;
	else
		return;
	if (onPhotic)
		maxWidth = rect.Width()*FACTOR - MONNAME_BAR - 2;

	bmp.CreateCompatibleBitmap(pDC, maxWidth, rect.Height());
	MemDC.SelectObject(&bmp);

	MemDC.FillRect(CRect(0, 0, maxWidth, rect.Height()), WHITE_BRUSH);

	//float crtPercent = (float)(this->GetScrollPos(SB_HORZ) - MONNAME_BAR - 2)/(GetTotalSize().cx - MONNAME_BAR - 2);
	uint64_t minPos = this->GetScrollPos(SB_HORZ);;
	//uint16_t screenPosNum = (rect.Width() - MONNAME_BAR)/distance;

	//draw foot line
	CPen silverPen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
	CPen* oldPen = MemDC.SelectObject(&silverPen);
	MemDC.MoveTo(MONNAME_BAR + 1, rect.Height() - FOOT_RANGE);
	MemDC.LineTo(maxWidth, rect.Height() - FOOT_RANGE);
	MemDC.SelectObject(oldPen);
	//DeleteObject(&silverPen);

	//draw lead name + line
	//drawLeadName(&MemDC);
	if (pDoc->counter != 0)
	{
		CFont txtFont;
		txtFont.CreatePointFont(70, _T("Arial"), pDC);
		//uint16_t maxPos = (arrPos + screenPosNum) <= pDoc->counter?(arrPos + screenPosNum):pDoc->counter;
		channelNum = this->GetDocument()->mMon.mList.GetCount();

		//CPoint point = GetScrollPosition();
		//int minPos = int((point.x )/distance);
		uint64_t maxPos = minPos + (uint64_t)(maxWidth/distance);
		if (maxPos > pDoc->counter)
			maxPos = pDoc->counter;
		//if (maxPos > pDoc->counter)
		//	maxPos = pDoc->counter - 1;
		//draw all pos from buffer data
		MemDC.SelectStockObject(BLACK_PEN);
		for (int i = minPos; i < maxPos; i++)
		{
			//if ((MONNAME_BAR + 2 + (i - minPos)*distance) > maxWidth)
			//	break;
			PrimaryDataType crtData = pDoc->primaryDataArray[i];
			if (crtData.isDraw)
			{
				CPen* tmpPen = MemDC.SelectObject(&silverPen);
				MemDC.MoveTo((i-minPos)*distance, 0);
				MemDC.LineTo((i-minPos)*distance, rect.Height() - FOOT_RANGE);
				MemDC.SelectObject(tmpPen);

				time_t tim =crtData.time;
				if (tim > 0)
					drawTime(&MemDC, tim, (i-minPos)*distance);
			}
			if (crtData.eventID >= 0 && crtData.eventID < 10)
			{
				CFont* tmpFont = MemDC.SelectObject(&txtFont);

				MemDC.SetBkMode(TRANSPARENT);
				CString evTxt = theApp.evName[crtData.eventID];
				MemDC.TextOutW((i - minPos)*distance - 15, 0, evTxt);
				MemDC.SelectObject(tmpFont);
			}
			for (int j = 0; j < channelNum; j++)
			{
				int tmp = (((rect.Height() - FOOT_RANGE)*j)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)crtData.value[j]-m_BaseLine)/m_Amp)*graphData.scaleRate);
				if (tmp > (rect.Height() - FOOT_RANGE))
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel((distance*j),tmp ,CUSTOM_PEN);
				MemDC.MoveTo((distance*(i - minPos)), tmp);		//draw 16 channel
				tmp = (((rect.Height() - FOOT_RANGE)*j)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)pDoc->primaryDataArray[i + 1].value[j]-m_BaseLine)/m_Amp)*graphData.scaleRate);
				if (tmp > (rect.Height() - FOOT_RANGE))
				tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel((distance*(j+1)),tmp ,CUSTOM_PEN);
				MemDC.LineTo((distance*(i + 1 - minPos)), tmp);	//
			}
		}
		DeleteObject(&txtFont);
	}

	pDC->BitBlt(MONNAME_BAR + 2, 0, maxWidth, rect.Height(), &MemDC, 0, 0, SRCCOPY);

	DeleteObject(&bmp);
	DeleteObject(&silverPen);
	MemDC.DeleteDC();
	//isDrawRec = FALSE;
}

void CAmekaView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	//SetScrollPos(SB_HORZ, nPos, FALSE);
	//CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
	//ScrollToDevicePosition(CPoint(nPos, 0));
	//SetScrollPos(SB_HORZ, nPos, TRUE);
	int minpos;
	int maxpos;
	GetScrollRange(SB_HORZ, &minpos, &maxpos); 
	maxpos = GetScrollLimit(SB_HORZ);

	// Get the current position of scroll box. 
	int curpos = this->GetScrollPos(SB_HORZ);;

	// Determine the new position of scroll box. 
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll. 
		break;

	case SB_LINELEFT:      // Scroll left. 
		if (curpos > minpos)
			curpos--;
		break;

	case SB_LINERIGHT:   // Scroll right. 
		if (curpos < maxpos)
			curpos++;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO   info;
		GetScrollInfo(SB_HORZ, &info, SIF_ALL);

		if (curpos > minpos)
		curpos = max(minpos, curpos - (int) info.nPage);
	}
		break;

	case SB_PAGERIGHT:      // Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO   info;
		GetScrollInfo(SB_HORZ, &info, SIF_ALL);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int) info.nPage);
	}
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation. 
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to. 
		break;
	}

	// Set the new position of the thumb (scroll box).
	SetScrollPos(SB_HORZ, curpos);
	
	drawLeadName(this->GetDC());
	drawRecData(this->GetDC());

	if (onPhotic)
	{
		photic_processing(FRE_STEP, this->GetDocument(), this->GetScrollPos(SB_HORZ));
		drawBarGraph();
	}
}

void CAmekaView::drawLeadName(CDC* pDC)
{
	//draw lead name
	CDC MemDC;
	CBitmap bmp;

	CRect rect;
	GetClientRect(&rect);

	MemDC.CreateCompatibleDC(pDC);
	bmp.CreateCompatibleBitmap(pDC, MONNAME_BAR + 2, rect.Height()); 
	MemDC.SelectObject(&bmp);

	CAmekaDoc* pDoc = this->GetDocument();
	if (!pDoc)
		return;
	//POSITION pos = pDoc->mMon.mList.GetHeadPosition();
	int leadNum = pDoc->mMon.mList.GetCount();

	CRect clearRect(0, 0, MONNAME_BAR + 2, rect.Height());
	MemDC.FillRect(&clearRect, WHITE_BRUSH);

	CFont txtFont;
	txtFont.CreatePointFont(70, _T("Arial"), &MemDC);
	MemDC.SelectObject(&txtFont);
	for (int i = 0; i < pDoc->mMon.mList.GetSize(); i++)
	{
		Alead lead = pDoc->mMon.mList.GetAt(i);
		CString leadTxt;
		CRect txtRect(0, i*(rect.Height() - FOOT_RANGE)/leadNum + 1, MONNAME_BAR,(i + 1)*(rect.Height() - FOOT_RANGE)/leadNum - 1);
		leadTxt = getElecName(lead.lSecondID) + "-" + getElecName(lead.lFirstID);
		MemDC.DrawTextW(leadTxt, txtRect, 0);
		CPen silverPen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
		//CPen* oldPen = MemDC.SelectObject(&silverPen);
		MemDC.MoveTo(0, (i + 1)*(rect.Height() - FOOT_RANGE)/leadNum);
		MemDC.LineTo(MONNAME_BAR, (i + 1)*(rect.Height() - FOOT_RANGE)/leadNum);
		//MemDC.SelectObject(oldPen);
		//DeleteObject(&silverPen);
	}
	CPen thick_pen(PS_SOLID, 2, CUSTOM_PEN);
	CPen* oldPen = MemDC.SelectObject(&thick_pen);

	MemDC.MoveTo(MONNAME_BAR + 1, 0);
	MemDC.LineTo(MONNAME_BAR + 1, rect.Height());

	MemDC.SelectObject(oldPen);
	DeleteObject(&thick_pen);
	pDC->BitBlt(0, 0, MONNAME_BAR + 2, rect.Height(), &MemDC, 0, 0, SRCCOPY);
	DeleteObject(&txtFont);
	MemDC.DeleteDC();
}

void CAmekaView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CScrollView::OnKeyUp(nChar, nRepCnt, nFlags);


	// TODO: Add your message handler code here and/or call default
	char evChar[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
	CAmekaDoc* pDoc = GetDocument();

	if (!pDoc->isRecord && !isRunning)
		return;

	char* found = std::find (evChar, std::end (evChar), nChar);
	if (found != std::end (evChar))
	{
		pDoc->eventID = nChar - '0';
		evPos = crtPos;
		if (evPos < MONNAME_BAR + 2 + 15)
			evPos = MONNAME_BAR + 2 + 15;
		hasEv = TRUE;
	}
}

void CAmekaView::drawEvent(CDC* pDC, uint16_t evID)
{
	/*CDC MemDC;
	CBitmap bmp;

	CRect rect;
	GetClientRect(&rect);

	MemDC.CreateCompatibleDC(pDC);
	bmp.CreateCompatibleBitmap(pDC, 10, rect.Height()); 
	MemDC.SelectObject(&bmp);*/

	CFont txtFont;
	txtFont.CreatePointFont(70, _T("Arial"), pDC);
	pDC->SelectObject(&txtFont);

	pDC->SetBkMode(TRANSPARENT);
	if (evID >= 0 && evID < 10)
	{
			CString evTxt = theApp.evName[evID];
			pDC->TextOutW(evPos - 15, 0, evTxt);
	}
	//pDC->BitBlt(crtPos - 10, 0, 10, FOOT_RANGE, &MemDC, 0, 0, SRCCOPY);

	DeleteObject(&txtFont);
	//MemDC.DeleteDC();
}

void CAmekaView::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default

	CScrollView::OnMouseLeave();
	m_Tips.HideTips();

}


void CAmekaView::OnEnterSizeMove()
{
	// TODO: Add your message handler code here and/or call default
	drawEnable = FALSE;
	CScrollView::OnEnterSizeMove();
}


void CAmekaView::OnExitSizeMove()
{
	// TODO: Add your message handler code here and/or call default
	drawEnable = TRUE;
	//OnDraw(GetDC());
	CScrollView::OnExitSizeMove();
}

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
#include "easylogging++.h"
#include "AmekaLan.h"
#include <algorithm>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
	graphData.dotPmm = 10;
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
	m_Tips.Create(CSize(X_TOOLTIP, Y_TOOLTIP));
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

	return CView::PreCreateWindow(cs);
}

// CAmekaView drawing

void CAmekaView::OnDraw(CDC* pDC)
{
	CAmekaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	//check if draw record data
	if (isDrawRec)
	{
		drawLeadName(pDC);
		drawRecData(pDC);
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
	
	CBrush brush;
	brush.CreateSolidBrush(RGB(255,255,255));
	CRect mrect(0,0,rect.Width(),rect.Height());
	MemDC.FillRect(mrect,&brush);
	
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

		for(int i = 0; i < channelNum; i++)
		{
			j = 0;
			while(crtPos - distance*(j+1) - SBAR_W > MONNAME_BAR)
			{
				if (dataBuffer[j+1].isDraw)
				{
					CPen* tmpPen = MemDC.SelectObject(&silverPen);
					MemDC.MoveTo((crtPos - distance*(j+1)), 0);
					MemDC.LineTo((crtPos - distance*(j+1)), rect.Height() - FOOT_RANGE);
					MemDC.SelectObject(tmpPen);

					drawTime(pDC, dataBuffer[j+1].time, (crtPos - distance*j - SBAR_W));
				}
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(count-1+bufLen-j)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > (rect.Height() - FOOT_RANGE))
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.MoveTo((crtPos - distance*j - SBAR_W), tmp);
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(count-1+bufLen-j-1)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > (rect.Height() - FOOT_RANGE))
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.LineTo((crtPos - distance*(j+1) - SBAR_W), tmp);
				j++;
			}
		}

		for(int i = 0; i < channelNum; i++)
		{
			j = 0;
			while((maxWidth - distance*j) >= crtPos - SBAR_W )
			{
				if (firstPos == 0)
					break;
				if (dataBuffer[j+1].isDraw)
				{
					CPen* tmpPen = MemDC.SelectObject(&silverPen);
					MemDC.MoveTo((maxWidth-distance*(j+1)), 0);
					MemDC.LineTo((maxWidth-distance*(j+1)), rect.Height() - FOOT_RANGE);
					MemDC.SelectObject(tmpPen);;

					drawTime(pDC, dataBuffer[j+1].time, (maxWidth-distance*(j+1) + SBAR_W));
				}
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(firstPos+bufLen-j)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height() - FOOT_RANGE)
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.MoveTo((maxWidth-distance*j + SBAR_W), tmp);
				tmp = ((rect.Height() - FOOT_RANGE)*i/channelNum) + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)dataBuffer[(firstPos+bufLen-j-1)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height() - FOOT_RANGE)
					tmp = rect.Height() - FOOT_RANGE;
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.LineTo((maxWidth-distance*(j+1) + SBAR_W), tmp);
				j++;
			}
		}
		if (!isRunning)
		{
			CBrush brushS(CUSTOM_PEN);
			//CBrush* pOldBrush1 = MemDC.SelectObject(&brushS);
			MemDC.FillRect(CRect(crtPos - SBAR_W, 0, crtPos, rect.Height() - FOOT_RANGE),&brushS);
		}

	}

	drawLeadName(&MemDC);

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
		float barW = (float)(rect.Width() - maxWidth) / barNum;
		for (int i = 0; i <= barNum; i++)
		{
			MemDC.MoveTo(maxWidth + i*barW, rect.Height() - 5);
			MemDC.LineTo(maxWidth + i*barW, 0);
			//draw text
			CRect txtRect(maxWidth + (int)(i*barW), (rect.Height() - FOOT_RANGE),
				maxWidth + i*barW + 10, rect.Height());
			CString text;
			text.Format(_T("%d"), (int)(theApp.photicTick*i + theApp.photicMin));
			MemDC.DrawTextW(text, txtRect, 0);
			
		}
	}
	LeaveCriticalSection(&csess);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);

	//UpdateWindow();
	MemDC.SelectObject(pOldBmp); 
	
	MemDC.DeleteDC();
	DeleteObject(&brush);
	DeleteObject(&Wbmp);

	//DeleteObject(pOldBmp);
	// TODO: add draw code for native data here
	
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
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
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

	CView::OnMouseMove(nFlags, point);

	//if (!isNull && !isRunning)
	//{
	//	int X = GetSystemMetrics( SM_CXSCREEN );
	//	int Y = GetSystemMetrics( SM_CYSCREEN );

	//	CPoint ptLog = point;
	//	ClientToScreen(&ptLog);
	//	uint16_t* posResult = this->getDataFromPos(point, crtPos, this);
	//	uint16_t tmpData[2];
	//	tmpData[0] = posResult[0];
	//	tmpData[1] = posResult[1];
	//	uint16_t* valResult = getMaxMin(tmpData);
	//	uint16_t fuck[2];
	//	fuck[0] = valResult[0];
	//	fuck[1] = valResult[1];

	//	CString strTemp;
	//	strTemp.Format(L"Data value: %d\r\nMix value: %d\r\nMax value: %d", this->dataBuffer[posResult[0]].value[posResult[1]], fuck[0], fuck[1]);
	//	// show tool tip in mouse move
	//	int xPos, yPos;
	//	if (ptLog.x + X_TOOLTIP + 5 > X)
	//	{
	//		xPos = X - X_TOOLTIP - 5;
	//	}
	//	else
	//	{
	//		xPos = ptLog.x + 5;
	//	}
	//	if (ptLog.y + Y_TOOLTIP + 5 > Y)
	//	{
	//		yPos = Y - Y_TOOLTIP - 5;
	//	}
	//	else
	//	{
	//		yPos = ptLog.y + 5;
	//	}
	//	m_Tips.ShowTips(xPos, yPos, strTemp);

	//	CView::OnMouseMove(nFlags, point);
	//}
	//else
	//{
	//	m_Tips.HideTips();
	//}
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
		//ret = pnt->amekaDrawPos(&pnt->bmp);
		pnt->amekaDrawPos(pDC);

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
		//ret = pnt->amekaDrawPos(&pnt->bmp);
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

//int CAmekaView::amekaDrawPos(CBitmap* bitmap)
int CAmekaView::amekaDrawPos(CDC* pDC)
{
	//TBD
	//CBitmap bitmap;
	CDC MemDC;
	CBitmap* bitmap = new CBitmap;
	//RawDataType* data = new RawDataType[dataNum];
	uint16_t buflen = 0;
	float maxWidth;
	PrimaryDataType* data;
	
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
	
	
	if (isNull)
	{
		prePos = data[buflen - 1];
		dataBuffer[count++] = data[buflen - 1];
		isNull = false;
		delete [] data;
		delete bitmap;
		return 0;
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

	if(bitmap != NULL)
	{
		if (crtPos + drawW > maxWidth)
		{
			if(NULL == bitmap->CreateCompatibleBitmap(pDC, ceil(maxWidth - crtPos), rect.Height()))
			{
				DWORD tmp = GetLastError();
				LOG(ERROR) << static_cast <int>(tmp);
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
				LOG(ERROR) << static_cast <int>(tmp);
				delete [] data;
				delete bitmap;
				return -1;
			}
		}
	}

	MemDC.SelectObject(bitmap);

	//memcpy(dataBuffer[count],data,sizeof(RawDataType));
	//count = (count+1)%bufLen;
	
	//erase current position
	CBrush brush;
	brush.CreateSolidBrush(RGB(255,255,255));
	CRect mrect(0, 0, rect.Width(), rect.Height());
	MemDC.FillRect(mrect,&brush);
	
	CPen silverPen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
	
	int tmp = 0;

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
	for(int i = 0; i < channelNum;i++)
	{
		if (data[0].isDraw)
		{
			preTimePos = crtPos;
			onDrawTime = TRUE;

			CPen* tmpPen = MemDC.SelectObject(&silverPen);
			MemDC.MoveTo(0, 0);
			MemDC.LineTo(0, rect.Height() - FOOT_RANGE);
			MemDC.SelectObject(tmpPen);

			drawTime(pDC, data[0].time, crtPos);
		}
		tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)prePos.value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
		if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
		if (tmp < 0)
			tmp = 0;
		//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
		MemDC.MoveTo(0, tmp);
		tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)data[0].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
		if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
		if (tmp < 0)
			tmp = 0;
		//MemDC.SetPixel(distance,tmp ,CUSTOM_PEN);
		MemDC.LineTo((distance), tmp);
	}
	
	for(int i = 0; i < channelNum; i++)
		for(int j = 1; j < buflen; j++)
		{
			if (data[j].isDraw)
			{
				preTimePos = crtPos + j*distance;
				onDrawTime = TRUE;

				CPen* tmpPen = MemDC.SelectObject(&silverPen);
				MemDC.MoveTo((j*distance), 0);
				MemDC.LineTo((j*distance), rect.Height() - FOOT_RANGE);
				MemDC.SelectObject(tmpPen);;

				drawTime(pDC, data[j].time, crtPos + j*distance);
			}
			tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)data[j-1].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*j),tmp ,CUSTOM_PEN);
			MemDC.MoveTo((distance*j), tmp);		//draw 16 channel
			tmp = (((rect.Height() - FOOT_RANGE)*i)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)data[j].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*(j+1)),tmp ,CUSTOM_PEN);
			MemDC.LineTo((distance*(j+1)), tmp);	//
		}
	
	//draw scan bar
	CBrush brushS(CUSTOM_PEN);
	//CBrush* pOldBrush1 = MemDC.SelectObject(&brushS);
	MemDC.FillRect(CRect(ceil(distance * buflen), 0, distance * buflen + SBAR_W, rect.Height() - FOOT_RANGE),&brushS);
	EnterCriticalSection(&csess);
	if (onDrawTime || (crtPos  < (preTimePos + MONNAME_BAR/2 + 1) && crtPos > preTimePos))
		pDC->BitBlt(crtPos, 0, distance * buflen + SBAR_W, rect.Height() - FOOT_RANGE, &MemDC, 0, 0, SRCCOPY);
	else
		pDC->BitBlt(crtPos, 0, distance * buflen + SBAR_W, rect.Height(), &MemDC, 0, 0, SRCCOPY);
	onDrawTime = FALSE;
	prePos = data[buflen-1];
	
	crtPos += distance*buflen;
	if(crtPos + distance + SBAR_W > maxWidth)
	{
		lastDistance = maxWidth - crtPos + SBAR_W;
		crtPos = MONNAME_BAR + 2;
	}
	LeaveCriticalSection(&csess);
	//free all resource
	if (hasEv)
	{
		drawEvent(pDC);
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
	CAmekaDoc* pDoc = this->GetDocument();
	if (pDoc == NULL)
		return -1;

	uint16_t buflen = SAMPLE_RATE/FRE_STEP;
	SecondaryDataType* data = this->mDoc->SecondaryData->checkPopData(buflen);
	int size = this->mDoc->SecondaryData->rLen;
	if (!data)
		return -1;

	if ((buflen <= 0) || (data == NULL))
		return -2;

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
		delete [] data;
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
			LOG(ERROR) << static_cast <int>(tmp);
			DeleteObject(pDC);
			delete [] data;
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

	//draw bar
	CBrush brushS(CUSTOM_BARCOLOR);
	MemDC.SelectObject(brushS);

	channelNum = this->GetDocument()->mMon.mList.GetCount();

	for (int i = 0; i < buflen; i++)
	{
		float freVal = data[i].fre - theApp.photicMin;
		int barCount = freVal/pDoc->mDSP.epocLength;
		float barPos = (float)barCount*(rect.Width()-startPos)/barNum;
		float barW = (float)range/barCount;
		for (int j = 0; j < channelNum; j++)
		{
			CRect barRect(abs(barPos - barW/2), (j+1)*(rect.Height() - FOOT_RANGE)/channelNum - (data[i].value[j])/theApp.photicWRate - ((rect.Height() - FOOT_RANGE)/channelNum)/2, 
				abs((rect.Width() - startPos)/barNum + barPos - barW/2),(j+1)*(rect.Height() - FOOT_RANGE)/channelNum - ((rect.Height() - FOOT_RANGE)/channelNum)/2);
			MemDC.FillRect(barRect,&brushS);

		}
	}

	//draw grid
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
		CRect txtRect((int)(i*barGridW), (rect.Height() - FOOT_RANGE),
				i*barGridW + 10, rect.Height());
		CString text;
		MemDC.SelectObject(&txtFont);
		text.Format(_T("%d"), (int)(theApp.photicTick*i + theApp.photicMin));
		MemDC.DrawTextW(text, txtRect, 0);
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
	DeleteObject(pDC);
	delete bitmap;
	delete [] data;

	return 0;
};

uint16_t* CAmekaView::getDataFromPos(CPoint mousePos, float crtPos, CAmekaView* pView)
{
	uint16_t* result = new uint16_t[2];
	CRect rect;
	this->GetWindowRect(&rect);
	int maxWidth = rect.Width();
	int xMousePos = mousePos.x;
	int yMousePos = mousePos.y;

	if (onPhotic)
		maxWidth = rect.Width()*FACTOR;
	else
		maxWidth = rect.Width();

	float xDistance;
	float distance = (float)this->graphData.paperSpeed*(float)this->graphData.dotPmm/this->graphData.sampleRate;
	if (xMousePos > crtPos)
	{
		if (!isResize)
			xDistance = crtPos + maxWidth - xMousePos + this->lastDistance;
		else
			xDistance = crtPos + maxWidth - xMousePos - SBAR_W;
	}
	else
	{
		xDistance = crtPos - xMousePos;
	}
	//get position
	uint16_t posNum = (uint16_t)(this->count - 1 + this->bufLen - (uint16_t)xDistance/distance)%(this->bufLen);
	int pos = 0;
	uint16_t val = abs(mousePos.y - ((rect.Height() - FOOT_RANGE)/this->channelNum)/2 - (((float)this->dataBuffer[posNum].value[0]
						- this->m_BaseLine)/this->m_Amp)*this->graphData.scaleRate);
	for (int i = 0; i < LEAD_NUMBER; i++)
	{
		int tmp = ((rect.Height() - FOOT_RANGE)*i/this->channelNum) + ((rect.Height() - FOOT_RANGE)/this->channelNum)/2 - (((float)this->dataBuffer[posNum].value[i]
						- this->m_BaseLine)/this->m_Amp)*this->graphData.scaleRate;
		if (abs(mousePos.y - tmp) < val)
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

	if (inputData[0] > this->count)
	{
		if ((inputData[0] - CHECK_RANGE ) > this->count )
		{
			lowVal = inputData[0] - CHECK_RANGE;
		}
		else
		{
			lowVal = this->count;
		}

		if ((inputData[0] + CHECK_RANGE)%this->bufLen < this->count - 1)
		{
			highVal = inputData[0] + CHECK_RANGE + this->bufLen;
		}
		else
		{
			highVal = this->count - 1 + this->bufLen;
		}
	}
	else
	{
		if ((inputData[0] - CHECK_RANGE + this->bufLen ) > this->count )
		{
			lowVal = inputData[0] - CHECK_RANGE + this->bufLen;
		}
		else
		{
			lowVal = this->count;
		}

		if ((inputData[0] + CHECK_RANGE) < this->count - 1)
		{
			highVal = inputData[0] + CHECK_RANGE + this->bufLen;
		}
		else
		{
			highVal = this->count - 1 + this->bufLen;
		}
	}
	//find max and min position in range
	uint16_t maxVal = 0;
	uint16_t minVal = this->dataBuffer[lowVal%this->bufLen].value[inputData[1]];
	for (int i = lowVal; i < highVal; i++)
	{
		if (this->dataBuffer[i%this->bufLen].value[inputData[1]] < minVal)
		{
			minVal = this->dataBuffer[i%this->bufLen].value[inputData[1]];
		}
		if (this->dataBuffer[i%this->bufLen].value[inputData[1]] > maxVal)
		{
			maxVal = this->dataBuffer[i%this->bufLen].value[inputData[1]];
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

	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
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

	sizeTotal.cx = pDoc->counter*distance + MONNAME_BAR + 2;
	sizeTotal.cy = rect.Height();
	SetScrollSizes(MM_TEXT, sizeTotal);

	MemDC.CreateCompatibleDC(pDC);
	uint32_t maxWidth;
	if (distance*pDoc->counter > rect.Width())
		maxWidth = distance*pDoc->counter;
	else
		maxWidth = rect.Width();

	bmp.CreateCompatibleBitmap(pDC, maxWidth, rect.Height());
	MemDC.SelectObject(&bmp);

	MemDC.FillRect(CRect(0, 0, maxWidth, rect.Height()), WHITE_BRUSH);
	
	if (pDoc->counter == 0)
		return;

	float crtPercent = (float)GetDeviceScrollPosition().x/GetTotalSize().cx;
	uint16_t arrPos = (uint16_t)(crtPercent*pDoc->counter);
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

	//uint16_t maxPos = (arrPos + screenPosNum) <= pDoc->counter?(arrPos + screenPosNum):pDoc->counter;
	channelNum = this->GetDocument()->mMon.mList.GetCount();

	for (int i = 0; i < pDoc->counter - 1; i++)
	{
		if (pDoc->PrimaryData->get(i).isDraw)
		{
			CPen* tmpPen = MemDC.SelectObject(&silverPen);
			MemDC.MoveTo(i*distance, 0);
			MemDC.LineTo(i*distance, rect.Height() - FOOT_RANGE);
			MemDC.SelectObject(tmpPen);

			time_t tim = pDoc->PrimaryData->get(i).time;
			if (tim > 0)
				drawTime(&MemDC, tim, i*distance);
		}
		for (int j = 0; j < channelNum; j++)
		{
			int tmp = (((rect.Height() - FOOT_RANGE)*j)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)pDoc->PrimaryData->get(i).value[j]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*j),tmp ,CUSTOM_PEN);
			MemDC.MoveTo((distance*i), tmp);		//draw 16 channel
			tmp = (((rect.Height() - FOOT_RANGE)*j)/channelNum + ((rect.Height() - FOOT_RANGE)/channelNum)/2 - (((float)pDoc->PrimaryData->get(i + 1).value[j]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > (rect.Height() - FOOT_RANGE))
			tmp = rect.Height() - FOOT_RANGE;
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*(j+1)),tmp ,CUSTOM_PEN);
			MemDC.LineTo((distance*(i + 1)), tmp);	//
		}
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

	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
	//ScrollToDevicePosition(CPoint(0, 0));
	//drawLeadName(GetDC());
	//OnDraw(this->GetDC());
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
		pDoc->eventID = nChar;
		evPos = crtPos;
		if (evPos < MONNAME_BAR + 2 + 15)
			evPos = MONNAME_BAR + 2 + 15;
		hasEv = TRUE;
	}
}

void CAmekaView::drawEvent(CDC* pDC)
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
	CString evTxt = L"Event";
	pDC->TextOutW(evPos - 15, 0, evTxt);

	//pDC->BitBlt(crtPos - 10, 0, 10, FOOT_RANGE, &MemDC, 0, 0, SRCCOPY);

	DeleteObject(&txtFont);
	//MemDC.DeleteDC();
}
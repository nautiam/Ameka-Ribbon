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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define timeSleep 25
#define dataNum 8

#define CUSTOM_SCANBAR RGB(92,64,51)
#define CUSTOM_PEN RGB(72,61,139)
#define CUSTOM_PEN1 RGB(192,192,192)
#define CUSTOM_BARCOLOR RGB(41,102,0)
#define CUSTOM_BARBACK RGB(255,255,200)
#define MONNAME_BAR 20
#define SBAR_W 4
#define SAMPLE_RATE 200

#define CODE_ERR_PDC_NULL -1
#define CODE_SUCCESS 0
#define CODE_ERR_OTHER -2

#define FACTOR 0.667

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
END_MESSAGE_MAP()

// CAmekaView construction/destruction

CAmekaView::CAmekaView()
{
	// TODO: add construction code here
	isRunning = false;
	crtPos = MONNAME_BAR;
	isNull = true;
	isCountFull = false;
	pThread = NULL;
	count = 0;
	bufLen = 4096;
	channelNum = 16;
	graphData.scaleRate = 30;
	graphData.dotPmm = 10;
	graphData.paperSpeed = 75;
	graphData.sampleRate = SAMPLE_RATE;
	InitializeCriticalSection(&csess);
	onPhotic = FALSE;
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
	DeleteCriticalSection(&csess);
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
	CBitmap Wbmp;
	CAmekaDoc* pDoc = GetDocument();
	float maxWidth;

	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	//CView::OnDraw(pDC);
	
	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);

	CRect rect;
    GetClientRect(&rect);

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
	
	if (isCountFull || count != 0)
	{
		EnterCriticalSection(&csess);
		distance = (float)graphData.paperSpeed*(float)graphData.dotPmm/graphData.sampleRate;
		uint16_t numPos = crtPos/distance;
		uint16_t firstPos;

		if (!isCountFull)
			firstPos = numPos>=count?0:count-1-numPos;
		else
			firstPos = (count-1+bufLen-numPos)%bufLen;

		int j,tmp;
		
		CPen thick_pen(PS_SOLID, 2, CUSTOM_PEN);
		CPen* oldPen = MemDC.SelectObject(&thick_pen);

		MemDC.MoveTo(MONNAME_BAR + 1, 0);
		MemDC.LineTo(MONNAME_BAR + 1, rect.Height());

		MemDC.SelectObject(oldPen);
		DeleteObject(&thick_pen);
		
		for(int i = 0; i < channelNum; i++)
		{
			j = 0;
			while(crtPos - distance*(j+1) > MONNAME_BAR)
			{
				tmp = (rect.Height()*i/channelNum) + (rect.Height()/channelNum)/2 - (((float)dataBuffer[(count-1+bufLen-j)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height())
				tmp = rect.Height();
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.MoveTo(crtPos - distance*j, tmp);
				tmp = (rect.Height()*i/channelNum) + (rect.Height()/channelNum)/2 - (((float)dataBuffer[(count-1+bufLen-j-1)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height())
				tmp = rect.Height();
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.LineTo(crtPos - distance*(j+1), tmp);
				j++;
			}
		}

		for(int i = 0; i < channelNum; i++)
		{
			j = 0;
			while(maxWidth - distance*j >= crtPos )
			{
				if (firstPos == 0)
					break;
				tmp = (rect.Height()*i/channelNum) + (rect.Height()/channelNum)/2 - (((float)dataBuffer[(firstPos+bufLen-j)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height())
				tmp = rect.Height();
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.MoveTo(maxWidth-distance*j, tmp);
				tmp = (rect.Height()*i/channelNum) + (rect.Height()/channelNum)/2 - (((float)dataBuffer[(firstPos+bufLen-j-1)%bufLen].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate;
				if (tmp > rect.Height())
				tmp = rect.Height();
				if (tmp < 0)
					tmp = 0;
				//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
				MemDC.LineTo(maxWidth-distance*(j+1), tmp);
				j++;
			}
		}

		CBrush brushS(CUSTOM_PEN);
		//CBrush* pOldBrush1 = MemDC.SelectObject(&brushS);
		MemDC.FillRect(CRect(crtPos, 0, crtPos + SBAR_W, rect.Height()),&brushS);
		LeaveCriticalSection(&csess);

		DeleteObject(brushS);
	}
	if (onPhotic)
	{
		CBrush brushBar;
		brushBar.CreateSolidBrush(CUSTOM_BARBACK);
		CRect mrectBar(maxWidth,0,rect.Width(),rect.Height());
		MemDC.FillRect(mrectBar,&brushBar);
		DeleteObject(&brushBar);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);
	UpdateWindow();
	MemDC.SelectObject(pOldBmp); 
	
	MemDC.DeleteDC();
	DeleteObject(&brush);
	DeleteObject(&Wbmp);

	//DeleteObject(pOldBmp);
	// TODO: add draw code for native data here
	
}


// CAmekaView printing

//graph thread
UINT CAmekaView::graphHandle(LPVOID pParam)
{
	CAmekaView* pnt = (CAmekaView *)pParam;
	CDC* pDC = pnt->GetDC();
	pnt->setParentDoc(pnt->GetDocument());
	pnt->dataBuffer = (RawDataType*)malloc(pnt->bufLen*sizeof(RawDataType));
	int ret;
	CPen pen2(PS_SOLID, 1, CUSTOM_PEN1);
	while(1)
	{
		//ret = pnt->amekaDrawPos(&pnt->bmp);
		pnt->amekaDrawPos(pDC);

		if (pnt->onPhotic)
		{
			int startPos;
			CRect rect;
			pnt->GetClientRect(&rect);
			startPos = rect.Width()*FACTOR;
			int range = theApp.photicMax - theApp.photicMin;
			int barNum = range/theApp.photicTick;
			pDC->SelectObject(&pen2);
			for (int i = 1; i <= barNum; i++)
			{
				pDC->MoveTo((startPos + i*(rect.Width()-startPos)/barNum), rect.Height()-5);
				pDC->LineTo((startPos + i*(rect.Width()-startPos)/barNum), 0);
				//MemDC.DrawText(itoa(TICKER*i),
			}
		}

		if (pnt->onPhotic)
			pnt->drawBarGraph();
		/*
		if(ret == -1)
			return -1;
			*/
		::Sleep(timeSleep);
	}
	DeleteObject(&pen2);
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
	RawDataType* data;
	
	if (pDC == NULL)
		return -1;
	
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
	if(crtPos + distance + SBAR_W >= maxWidth)
		crtPos = MONNAME_BAR;

	if ((rect.Width() - crtPos - SBAR_W)/distance > dataNum)
	{
		data = this->mDoc->PrimaryData->popData(dataNum);
	}
	else
	{
		data = this->mDoc->PrimaryData->popData((rect.Width() - crtPos - SBAR_W)/distance);
	}
	buflen = this->mDoc->PrimaryData->rLen;
	if ((buflen <= 0) || (data == NULL))
		return -2;
	
	
	if (isNull)
	{
		prePos = data[buflen - 1];
		dataBuffer[count++] = data[buflen - 1];
		isNull = false;
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
		return -1;
	}

	int drawW = distance * buflen + SBAR_W;

	if(bitmap != NULL)
	{
		if (crtPos + drawW > maxWidth)
		{
			if(NULL == bitmap->CreateCompatibleBitmap(pDC, maxWidth - crtPos, rect.Height()))
			{
				DWORD tmp = GetLastError();
				LOG(ERROR) << static_cast <int>(tmp);

				return -1;
			}
		}
		else
		{
			if(NULL == bitmap->CreateCompatibleBitmap(pDC, drawW, rect.Height()))
			{
				DWORD tmp = GetLastError();
				LOG(ERROR) << static_cast <int>(tmp);

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
	CRect mrect(0,0,rect.Width(),rect.Height());
	MemDC.FillRect(mrect,&brush);
	
	//CPen thick_pen(PS_SOLID, 1, CUSTOM_PEN);
	//MemDC.SelectObject(&thick_pen);
	
	int tmp = 0;
	//draw all point to current bitmap

	if (crtPos == MONNAME_BAR)
	{
		CPen thick_pen(PS_SOLID, 2, CUSTOM_PEN);
		CPen* oldPen = MemDC.SelectObject(&thick_pen);

		MemDC.MoveTo(1, 0);
		MemDC.LineTo(1, rect.Height());

		MemDC.SelectObject(oldPen);
		DeleteObject(&thick_pen);
	}
	
	for(int i = 0; i < channelNum;i++)
	{
		tmp = ((rect.Height()*i)/channelNum + (rect.Height()/channelNum)/2 - (((float)prePos.value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
		if (tmp > rect.Height())
			tmp = rect.Height();
		if (tmp < 0)
			tmp = 0;
		//MemDC.SetPixel(0, tmp, CUSTOM_PEN);
		MemDC.MoveTo(0, tmp);
		if (tmp > rect.Height())
			tmp = rect.Height();
		if (tmp < 0)
			tmp = 0;
		tmp = ((rect.Height()*i)/channelNum + (rect.Height()/channelNum)/2 - (((float)data[0].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
		//MemDC.SetPixel(distance,tmp ,CUSTOM_PEN);
		MemDC.LineTo(distance, tmp);
	}
	
	for(int i = 0; i < channelNum; i++)
		for(int j = 1; j < buflen; j++)
		{
			tmp = ((rect.Height()*i)/channelNum + (rect.Height()/channelNum)/2 - (((float)data[j-1].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > rect.Height())
				tmp = rect.Height();
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*j),tmp ,CUSTOM_PEN);
			MemDC.MoveTo((distance*j), tmp);		//draw 16 channel
			tmp = ((rect.Height()*i)/channelNum + (rect.Height()/channelNum)/2 - (((float)data[j].value[i]-m_BaseLine)/m_Amp)*graphData.scaleRate);
			if (tmp > rect.Height())
				tmp = rect.Height();
			if (tmp < 0)
				tmp = 0;
			//MemDC.SetPixel((distance*(j+1)),tmp ,CUSTOM_PEN);
			MemDC.LineTo((distance*(j+1)), tmp);	//
		}
	
	//draw scan bar
	CBrush brushS(CUSTOM_PEN);
	//CBrush* pOldBrush1 = MemDC.SelectObject(&brushS);
	MemDC.FillRect(CRect(distance * buflen, 0, distance * buflen + SBAR_W, rect.Height()),&brushS);

	pDC->BitBlt(crtPos, 0, distance * buflen + SBAR_W, rect.Height(), &MemDC, 0, 0, SRCCOPY);
	
	prePos = data[buflen-1];
	
	crtPos += distance*buflen;

	//free all resource
	MemDC.DeleteDC();
	//DeleteObject(brushS);
	//DeleteObject(brush);
	//DeleteObject(bitmap);
	delete[] data;
	DeleteObject(bitmap);
	DeleteObject(&brushS);
	DeleteObject(&brush);
	delete bitmap;
	bitmap = NULL;
	//delete bitmap;
	return 0;
	
}

int CAmekaView::drawBarGraph( void )
{
	CDC MemDC;
	CBitmap* bitmap = new CBitmap;
	CDC* pDC = GetDC();
	float mDistance;
	//RawDataType* data = new RawDataType[dataNum];
	uint16_t buflen = 0;
	float startPos;
	
	if (pDC == NULL)
		return -1;

	SecondaryDataType* data = this->mDoc->SecondaryData->popData(1);
	buflen = this->mDoc->SecondaryData->rLen;
	if ((buflen <= 0) || (data == NULL))
		return -2;

	if (data->fre > theApp.photicMax)
		return 1;

	//mDistance = (float)graphData.paperSpeed*(float)graphData.dotPmm/graphData.sampleRate;

	CRect rect;
    GetClientRect(&rect);

	if (NULL == MemDC.CreateCompatibleDC(pDC))
	{
		MemDC.DeleteDC();
		return -1;
	}

	if (onPhotic)
		startPos = rect.Width()*FACTOR;
	else
		startPos = rect.Width();

	int range = theApp.photicMax - theApp.photicMin;
	int barNum = range/theApp.photicBarW;

	if(bitmap != NULL)
	{
		if(NULL == bitmap->CreateCompatibleBitmap(pDC, (rect.Width()-startPos)/barNum, rect.Height()))
		{
			DWORD tmp = GetLastError();
			LOG(ERROR) << static_cast <int>(tmp);

			return -1;
		}
	}

	CBitmap* pOldBmp = MemDC.SelectObject(bitmap);

	CBrush brush;
	brush.CreateSolidBrush(CUSTOM_BARBACK);
	CRect mrect(0,0,rect.Width(),rect.Height());
	MemDC.FillRect(mrect,&brush);

	//CPen pen2(PS_SOLID, 1, CUSTOM_PEN);
	/*MemDC.SelectObject(pen1);
	MemDC.MoveTo(0, rect.Height());
	MemDC.LineTo(0, 0);*/
	//MemDC.SelectObject(pen2);
	//for (int i = 1; i <= barNum; i++)
	//{
	//	MemDC.MoveTo((i*(rect.Width()-startPos)/barNum), rect.Height()-5);
	//	MemDC.LineTo((i*(rect.Width()-startPos)/barNum), 0);
	//	//MemDC.DrawText(itoa(TICKER*i),
	//}
	CBrush brushS(CUSTOM_BARCOLOR);
	MemDC.SelectObject(brushS);
	int barCount = data->fre/2;

	for (int i = 0; i < 16; i++)
	{
		CRect barRect(0, (i+1)*rect.Height()/16 - data->value[i]/30, 
			(rect.Width()-startPos)/barNum,(i+1)*rect.Height()/16);
		MemDC.FillRect(barRect,&brushS);

		/*CPen pen1(PS_SOLID, 1, CUSTOM_PEN);
		CPen* pen2 = MemDC.SelectObject(&pen1);
		MemDC.Rectangle(CRect(0, (i+1)*rect.Height()/16 - data->value[i]/10, 
			(rect.Width()-startPos)/barNum,(i+1)*rect.Height()/16));
		MemDC.SelectObject(pen2);
		DeleteObject(&pen1);*/
	}

	pDC->BitBlt((startPos + barCount*(rect.Width()-startPos)/barNum), 0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);
	
	MemDC.SelectObject(pOldBmp);
	MemDC.DeleteDC();
	DeleteObject(bitmap);
	//DeleteObject(pen2);
	DeleteObject(&brush);
	DeleteObject(&brushS);
	delete [] data;

	return 0;
};

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

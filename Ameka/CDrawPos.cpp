// CDrawPos.cpp : implementation file
//

#include "stdafx.h"
#include "CDrawPos.h"
#include "AmekaView.h"

#ifndef  WS_EX_LAYERED
#define  WS_EX_LAYERED           0x00080000
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDrawPos

CDrawPos::CDrawPos()
:m_ptOrg(0,0),
m_TipSize(0,0)
{
	m_clrFrameColor = RGB(255, 255, 255);
	m_clrBack = RGB(255, 255, 255);
	m_clrText = RGB(0, 0, 0);

}

CDrawPos::~CDrawPos()
{

}


BEGIN_MESSAGE_MAP(CDrawPos, CWnd)
	//{{AFX_MSG_MAP(CDrawPos)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CDrawPos::Create(CSize szSize, CWnd *pWnd)
{
	HWND hWnd = NULL;
	if (pWnd != NULL && IsWindow(pWnd->m_hWnd))
	{
		hWnd = pWnd->m_hWnd;
	}
	// register class 
    LPCTSTR lpszClassName = AfxRegisterWndClass(NULL);
	BOOL bRet = CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, lpszClassName, _T(""),
		WS_POPUP /*| WS_BORDER*/, 0, 0, 1, 1, NULL, NULL);
	if (bRet)
	{
		MoveWindow(CRect(m_ptOrg, szSize));	
		m_TipSize = szSize;
	}
	return bRet;
}

void CDrawPos::SetRectSize(CSize szSize)
{
	m_TipSize = szSize;
	/*if (IsWindow(m_hWnd))
	{
		MoveWindow(CRect(m_ptOrg, szSize));
	}*/
}

void CDrawPos::ShowRect(int nX, int nY)
{	
	  m_ptOrg = CPoint(nX, nY);
	  //m_strTips = strTipsContent;
      MoveWindow(nX, nY, m_TipSize.cx, m_TipSize.cy, TRUE);
	  if (!IsWindowVisible())
	  {
		 ShowWindow(SW_SHOW);
	  }
	  Invalidate(TRUE);  
}

void CDrawPos::SetValue(uint16_t* val, uint16_t drawval)
{
	if (!val)
		return;
	if (val[0] < val[1])
	{
		minVal = val[0];
		maxVal = val[1];
	}
	else
	{
		minVal = val[1];
		maxVal = val[0];
	}
	drawVal = drawval;
}


/////////////////////////////////////////////////////////////////////////////
// CDrawPos message handlers

void CDrawPos::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rtWin;
	GetClientRect(rtWin);
	//dc.FillSolidRect(rtWin, m_clrBack);
	//dc.SetBkMode(TRANSPARENT);
	//dc.Draw3dRect(rtWin, m_clrFrameColor, m_clrFrameColor);
	//dc.SetTextColor(m_clrText);                
	//dc.DrawText(m_strTips, rtWin,  DT_CENTER |DT_VCENTER);	
	CDC MemDC;
	CBitmap bmp;

	CAmekaView* pView = CAmekaView::GetView();
	CAmekaDoc* pDoc = pView->GetDocument();
	float mDistance = pView->distance;
	//CRect rect;

	MemDC.CreateCompatibleDC(&dc);
	bmp.CreateCompatibleBitmap(&dc, rtWin.Width(), rtWin.Height());
	MemDC.SelectObject(&bmp);

	MemDC.FillRect(rtWin, WHITE_BRUSH);

	//pView->GetClientRect(&rtWin);
	uint16_t channelNum = pDoc->mMon.mList.GetCount();
	for (int i = 0; i < maxVal - minVal; i++)
	{
		for (int j = 0; j < channelNum; j++)
		{
			if (j == drawVal)
				continue;
			int tmp = (((rtWin.Height())*j)/channelNum + ((rtWin.Height())/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i].value[j]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
			if (tmp > (rtWin.Height()))
				tmp = rtWin.Height();
			if (tmp < 0)
				tmp = 0;
			MemDC.MoveTo(i*mDistance, tmp);
			tmp = ((rtWin.Height())*j/channelNum + ((rtWin.Height())/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i + 1].value[j]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
			if (tmp > (rtWin.Height()))
				tmp = rtWin.Height();
			if (tmp < 0)
				tmp = 0;
			MemDC.LineTo((i+1)*mDistance, tmp);
		}
	}
	CPen redPen(PS_SOLID, 1, RGB(255, 15, 15));
	CPen* oldPen = MemDC.SelectObject(&redPen);
	for (int i = 0; i < maxVal - minVal; i++)
	{
		{
			int tmp = ceil(((rtWin.Height())*drawVal)/channelNum + ((rtWin.Height())/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i].value[drawVal]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
			if (tmp > (rtWin.Height()))
				tmp = rtWin.Height();
			if (tmp < 0)
				tmp = 0;
			MemDC.MoveTo(i*mDistance, tmp);
			tmp = ceil(((rtWin.Height())*drawVal)/channelNum + ((rtWin.Height())/channelNum)/2 - (((float)pDoc->primaryDataArray[minVal + i + 1].value[drawVal]- pView->m_BaseLine)/pView->m_Amp)*pView->graphData.scaleRate);
			if (tmp > (rtWin.Height()))
				tmp = rtWin.Height();
			if (tmp < 0)
				tmp = 0;
			MemDC.LineTo((i+1)*mDistance, tmp);
		}
	}
	dc.BitBlt(0, 0, rtWin.Width(), rtWin.Height(), &MemDC, 0, 0, SRCCOPY);
	DeleteObject(&redPen);
	MemDC.DeleteDC();
}


BOOL CDrawPos::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default	
    // return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}

void CDrawPos::HideRect()
{
	if (IsWindow(m_hWnd))
	{
		ShowWindow(SW_HIDE);
	}
}


void CDrawPos::SetBkColor(COLORREF clrBack)
{
	m_clrBack = clrBack;
}

void CDrawPos::SetFrameColor(COLORREF clrFrame)
{
	m_clrFrameColor = clrFrame;
}

void CDrawPos::SetTipTextColor(COLORREF clrText)
{
	m_clrText = clrText;
}



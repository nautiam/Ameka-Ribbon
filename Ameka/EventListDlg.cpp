// EventListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Ameka.h"
#include "EventListDlg.h"
#include "afxdialogex.h"


// CEventListDlg dialog

IMPLEMENT_DYNAMIC(CEventListDlg, CDialogEx)

CEventListDlg::CEventListDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEventListDlg::IDD, pParent)
{
}

CEventListDlg::~CEventListDlg()
{
}

void CEventListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTEV, m_ListView);
}


BEGIN_MESSAGE_MAP(CEventListDlg, CDialogEx)
	ON_LBN_DBLCLK(IDC_LISTEV, &CEventListDlg::OnLbnDblclkListev)
END_MESSAGE_MAP()


// CEventListDlg message handlers


BOOL CEventListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	for (int i = 0; i < 10; i++)
	{
		m_ListView.AddString(theApp.evName[i]);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEventListDlg::OnLbnDblclkListev()
{
	// TODO: Add your control notification handler code here
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	CAmekaView* pView = CAmekaView::GetView();

	if (pDoc && pView)
	{
		UINT mID = m_ListView.GetCurSel();
		pView->drawEvent(pView->GetDC(), mID);
		pDoc->eventID = mID;
		pView->evPos = pView->crtPos;
		if (pView->evPos < MONNAME_BAR + 2 + 15)
			pView->evPos = MONNAME_BAR + 2 + 15;
		pView->hasEv = TRUE;
		if (mID == 2)
		{
			CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
			if (pMainWnd)
				pMainWnd->m_wndRibbonBar.SetWindowTextW(L"ASAD");
		}
	}
}

/**********************************************************************/
IMPLEMENT_DYNAMIC(CEvListPane, CDockablePane)
BEGIN_MESSAGE_MAP(CEvListPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

CEvListPane::CEvListPane(){}
CEvListPane::~CEvListPane(){}

int CEvListPane::OnCreate(LPCREATESTRUCT lp)
{
	if (CDockablePane::OnCreate(lp) == -1)
		return -1;
	if (!m_wndDlg.Create(CEventListDlg::IDD, this))
		return -1;
	m_wndDlg.ShowWindow(SW_SHOWDEFAULT);
	return  0;
}
void CEvListPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_wndDlg.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
}
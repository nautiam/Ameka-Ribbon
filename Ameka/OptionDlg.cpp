#include "stdafx.h"
#include "OptionDlg.h"

//------------------------------------------------------------------//
//Tab Dialog

//------------------------------------------------------------------//

CTabCOMDlg::CTabCOMDlg() : CDialogEx(CTabCOMDlg::IDD)
{
	
}

int CTabCOMDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	port_name.SetCurSel(0);
	port_baud.SetCurSel(0);
	return 0;
}

void CTabCOMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, opt_com_portNo, theApp.m_portNo);
	DDX_Text(pDX, opt_com_baud, theApp.m_baudRate);
	DDX_Control(pDX, opt_com_portNo, port_name);
	DDX_Control(pDX, opt_com_baud, port_baud);
}

BEGIN_MESSAGE_MAP(CTabCOMDlg, CDialogEx)
END_MESSAGE_MAP()

//------------------------------------------------------------------//
// Rec tab Dialog
//------------------------------------------------------------------//

CTabRecDlg::CTabRecDlg() : CDialogEx(CTabRecDlg::IDD)
{
	
}

int CTabRecDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	TCHAR szDirectory[MAX_PATH] = L"";
	::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);

	rec_ed_eeg.SetWindowTextW(szDirectory);
	rec_ed_video.SetWindowTextW(szDirectory);

	return 0;
}

void CTabRecDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, rec_ed_eeg);
	DDX_Control(pDX, IDC_EDIT2, rec_ed_video);
}

void CTabRecDlg::OnBnClickedeeg()
{
	// TODO: Add your control notification handler code here
	CFolderPickerDialog dlgFolder;

	if ( dlgFolder.DoModal() == IDOK )
	{
		CString tmp = dlgFolder.GetFolderPath();
		rec_ed_eeg.SetWindowTextW(tmp);
	}
}

void CTabRecDlg::OnBnClickedvideo()
{
	// TODO: Add your control notification handler code here
	CFolderPickerDialog dlgFolder;

	if ( dlgFolder.DoModal() == IDOK )
	{
		CString tmp = dlgFolder.GetFolderPath();
		rec_ed_video.SetWindowTextW(tmp);
	}
}

BEGIN_MESSAGE_MAP(CTabRecDlg, CDialogEx)
	ON_BN_CLICKED(rec_eeg, &CTabRecDlg::OnBnClickedeeg)
	ON_BN_CLICKED(rec_video, &CTabRecDlg::OnBnClickedvideo)
END_MESSAGE_MAP()

//------------------------------------------------------------------//
// Event tab dialog
//------------------------------------------------------------------//

CTabEventDlg::CTabEventDlg() : CDialogEx(CTabEventDlg::IDD)
{
	
}

int CTabEventDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	mEV1.SetWindowTextW(theApp.evName[1]);
	mEV2.SetWindowTextW(theApp.evName[2]);
	mEV3.SetWindowTextW(theApp.evName[3]);
	mEV4.SetWindowTextW(theApp.evName[4]);
	mEV5.SetWindowTextW(theApp.evName[5]);
	mEV6.SetWindowTextW(theApp.evName[6]);
	mEV7.SetWindowTextW(theApp.evName[7]);
	mEV8.SetWindowTextW(theApp.evName[8]);
	mEV9.SetWindowTextW(theApp.evName[9]);
	mEV10.SetWindowTextW(theApp.evName[0]);
	return 0;
}

void CTabEventDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, EV_1, mEV1);
	DDX_Control(pDX, EV_2, mEV2);
	DDX_Control(pDX, EV_3, mEV3);
	DDX_Control(pDX, EV_4, mEV4);
	DDX_Control(pDX, EV_5, mEV5);
	DDX_Control(pDX, EV_6, mEV6);
	DDX_Control(pDX, EV_7, mEV7);
	DDX_Control(pDX, EV_8, mEV8);
	DDX_Control(pDX, EV_9, mEV9);
	DDX_Control(pDX, EV_10, mEV10);

	DDX_Text(pDX, EV_1, theApp.evName[1]);
	DDX_Text(pDX, EV_2, theApp.evName[2]);
	DDX_Text(pDX, EV_3, theApp.evName[3]);
	DDX_Text(pDX, EV_4, theApp.evName[4]);
	DDX_Text(pDX, EV_5, theApp.evName[5]);
	DDX_Text(pDX, EV_6, theApp.evName[6]);
	DDX_Text(pDX, EV_7, theApp.evName[7]);
	DDX_Text(pDX, EV_8, theApp.evName[8]);
	DDX_Text(pDX, EV_9, theApp.evName[9]);
	DDX_Text(pDX, EV_10, theApp.evName[0]);
}

BEGIN_MESSAGE_MAP(CTabEventDlg, CDialogEx)

END_MESSAGE_MAP()

//------------------------------------------------------------------//
// View tab dialog
//------------------------------------------------------------------//

CTabViewDlg::CTabViewDlg() : CDialogEx(CTabViewDlg::IDD)
{
	
}

int CTabViewDlg::OnInitDialog()
{
	CAmekaView* pView = CAmekaView::GetView();
	if (!pView)
		return -1;
	CDialog::OnInitDialog();
	m_view_sen.SetWindowTextW(theApp.m_sensitivity);
	m_view_speed.SetWindowTextW(theApp.m_speed);
	m_view_lp.SetWindowTextW(theApp.m_LP);
	m_view_hp.SetWindowTextW(theApp.m_HP);
	
	if (pView)
	{
		
		m_view_dotPmm.SetWindowTextW(theApp.m_dotPmm);
	}
	return 0;
}

void CTabViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, view_lp, theApp.m_LP);
	DDX_Text(pDX, view_hp, theApp.m_HP);
	DDX_Text(pDX, view_sensitivity, theApp.m_sensitivity);
	DDX_Text(pDX, view_speed, theApp.m_speed);

	CAmekaView* pView = CAmekaView::GetView();
	DDX_Text(pDX, view_dotPmm, theApp.m_dotPmm);
	
	DDX_Control(pDX, view_sensitivity, m_view_sen);
	DDX_Control(pDX, view_speed, m_view_speed);
	DDX_Control(pDX, view_lp, m_view_lp);
	DDX_Control(pDX, view_hp, m_view_hp);
	DDX_Control(pDX, view_dotPmm, m_view_dotPmm);

	pView->graphData.dotPmm = atoi((LPCSTR)(CStringA)theApp.m_dotPmm)/30;
}

void CTabViewDlg::OnBnClickedbtdef()
{
	// TODO: Add your control notification handler code here
	m_view_sen.SetWindowTextW(_T(strSen));
	m_view_speed.SetWindowTextW(_T(strSpeed));
	m_view_lp.SetWindowTextW(_T(strLP));
	m_view_hp.SetWindowTextW(_T(strHP));
	m_view_dotPmm.SetWindowTextW(L"200");
}

BEGIN_MESSAGE_MAP(CTabViewDlg, CDialogEx)
	ON_BN_CLICKED(view_btdef, &CTabViewDlg::OnBnClickedbtdef)
END_MESSAGE_MAP()

//------------------------------------------------------------------//
// COptionDlg
//------------------------------------------------------------------//

COptionDlg::COptionDlg() : CDialogEx(COptionDlg::IDD)
{
	
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	/*
	DDX_Text(pDX, opt_com_portNo, m_portNo);
	DDX_Text(pDX, opt_com_baud, m_baudRate);
	DDX_Text(pDX, view_lp, m_LP);
	DDX_Text(pDX, view_hp, m_HP);
	DDX_Text(pDX, view_sensitivity, m_sensitivity);
	DDX_Text(pDX, view_speed, m_speed);
	*/
	DDX_Control(pDX, opt_tab, tab_ctrl);
}

int COptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    tab_ctrl.InsertItem(0,L"Bảng");
	tab_ctrl.InsertItem(1,L"Sự kiện");
	tab_ctrl.InsertItem(2,L"Ghi");
	tab_ctrl.InsertItem(3,L"In");

	mDlg[0] = new CTabViewDlg;
	mDlg[1] = new CTabEventDlg;
	mDlg[2] = new CTabRecDlg;
	mDlg[3] = new CTabPrintDlg;

	mDlg[0]->Create(DLG_Opt_View, &tab_ctrl);
	mDlg[1]->Create(DLG_Opt_Event, &tab_ctrl);
	mDlg[2]->Create(DLG_Opt_Rec, &tab_ctrl);
	mDlg[3]->Create(DLG_Opt_Print, &tab_ctrl);

	CRect TabRect; 
	tab_ctrl.GetClientRect(&TabRect);
	tab_ctrl.AdjustRect(FALSE, &TabRect);
	mDlg[0]->MoveWindow(TabRect);
	mDlg[1]->MoveWindow(TabRect);
	mDlg[2]->MoveWindow(TabRect);
	mDlg[3]->MoveWindow(TabRect);

    mDlg[0]->ShowWindow(true);
	mDlg[1]->ShowWindow(false);
	mDlg[2]->ShowWindow(false);
	mDlg[3]->ShowWindow(false);
    tab_ctrl.SetCurSel(0);
	mPrePos = 0;

	return 0;
}

void COptionDlg::OnBnClickedok()
{
	// TODO: Add your control notification handler code here
	for (int i = 0; i < 4; i++)
	{
		mDlg[i]->UpdateData();
	}
	CAmekaView* view = CAmekaView::GetView();
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!view || !pDoc)
		return;
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	int count = 0;
	//Set items for LowPassFilter
	CMFCRibbonComboBox* pSen = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_ScaleRate));
	pSen->RemoveAllItems();
	vector<string> vecSen = Tokenize(theApp.m_sensitivity," ");
	for (vector<string>::iterator it = vecSen.begin(); it != vecSen.end(); it++) {
		CString cs((*it).c_str());
		pSen->AddItem(cs,count++);
	}
	pSen->SetEditText(itoS(view->graphData.scaleRate));
	//Set items for LowPassFilter
	count = 0;
	CMFCRibbonComboBox* pSpeed = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_SpeedRate));
	pSpeed->RemoveAllItems();
	vector<string> vecSpeed = Tokenize(theApp.m_speed," ");
	for (vector<string>::iterator it = vecSpeed.begin(); it != vecSpeed.end(); it++) {
		CString cs((*it).c_str());
		pSpeed->AddItem(cs,count++);
	}
	pSpeed->SetEditText(itoS(view->graphData.paperSpeed));
	//Set items for LowPassFilter
	count = 0;
	CMFCRibbonComboBox* pLP = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_LP));
	pLP->RemoveAllItems();
	vector<string> vecLP = Tokenize(theApp.m_LP," ");
	for (vector<string>::iterator it = vecLP.begin(); it != vecLP.end(); it++) {
		CString cs((*it).c_str());
		pLP->AddItem(cs,count++);
	}
	CString lpVal;
	lpVal.Format(_T("%.1f"),pDoc->mDSP.LPFFre);
	pLP->SetEditText(lpVal);
	//Set items for LowPassFilter
	count = 0;
	CMFCRibbonComboBox* pHP = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_HP));
	pHP->RemoveAllItems();
	vector<string> vecHP = Tokenize(theApp.m_HP," ");
	for (vector<string>::iterator it = vecHP.begin(); it != vecHP.end(); it++) {
		CString cs((*it).c_str());
		pHP->AddItem(cs,count++);
	}
	CString hpVal;
	hpVal.Format(_T("%.1f"),pDoc->mDSP.HPFFre);
	pHP->SetEditText(hpVal);

	writeSetting(settingName);
	EndDialog(0);
}

BEGIN_MESSAGE_MAP(COptionDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &COptionDlg::OnOK)
	ON_NOTIFY(TCN_SELCHANGE, opt_tab, &COptionDlg::OnTabSel)
	ON_BN_CLICKED(opt_cancel, &COptionDlg::OnBnClickedcancel)
	ON_BN_CLICKED(opt_ok, &COptionDlg::OnBnClickedok)
END_MESSAGE_MAP()


void COptionDlg::OnTabSel(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	mDlg[mPrePos]->ShowWindow(false);
	mPrePos = tab_ctrl.GetCurSel();
	mDlg[mPrePos]->ShowWindow(true);
	*pResult = 0;
}


void COptionDlg::OnBnClickedcancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}



//------------------------------------------------------------------//
// CTabPrintDlg
//------------------------------------------------------------------//


IMPLEMENT_DYNAMIC(CTabPrintDlg, CDialogEx)

CTabPrintDlg::CTabPrintDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTabPrintDlg::IDD, pParent)
{

}

CTabPrintDlg::~CTabPrintDlg()
{
}

void CTabPrintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, ID_LEFT, ed_left);
	DDX_Control(pDX, ID_RIGHT, ed_right);
	DDX_Control(pDX, ID_TOP, ed_top);
	DDX_Control(pDX, ID_BOT, ed_bot);
	DDX_Control(pDX, ID_FONT, ed_font);
	DDX_Control(pDX, ID_SIZE, ed_size);
	DDX_Control(pDX, ID_DISTANCE, ed_distance);

	CString tmp;
	DDX_Text(pDX, ID_LEFT, tmp);
	if (tmp != "")
		theApp.marginLeft = atoi((LPCSTR)(CStringA)tmp);

	DDX_Text(pDX, ID_RIGHT, tmp);
	if (tmp != "")
		theApp.marginRight = atoi((LPCSTR)(CStringA)tmp);

	DDX_Text(pDX, ID_TOP, tmp);
	if (tmp != "")
		theApp.marginTop = atoi((LPCSTR)(CStringA)tmp);

	DDX_Text(pDX, ID_BOT, tmp);
	if (tmp != "")
		theApp.marginBot= atoi((LPCSTR)(CStringA)tmp);

	DDX_Text(pDX, ID_FONT, theApp.printFont);

	DDX_Text(pDX, ID_SIZE, tmp);
	if (tmp != "")
		theApp.printSize = atoi((LPCSTR)(CStringA)tmp);

	DDX_Text(pDX, ID_DISTANCE, tmp);
	if (tmp != "")
		theApp.printDistance = atoi((LPCSTR)(CStringA)tmp);
}

int CTabPrintDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString str;
	str.Format(L"%d", theApp.marginLeft);	
	ed_left.SetWindowTextW(str);

	str.Format(L"%d", theApp.marginRight);	
	ed_right.SetWindowTextW(str);

	str.Format(L"%d", theApp.marginTop);	
	ed_top.SetWindowTextW(str);

	str.Format(L"%d", theApp.marginBot);	
	ed_bot.SetWindowTextW(str);

	ed_font.SetWindowTextW(theApp.printFont);

	str.Format(L"%d", theApp.printSize);	
	ed_size.SetWindowTextW(str);

	str.Format(L"%d", theApp.printDistance);	
	ed_distance.SetWindowTextW(str);

	return 0;
}

BEGIN_MESSAGE_MAP(CTabPrintDlg, CDialogEx)
END_MESSAGE_MAP()


// CTabPrintDlg message handlers

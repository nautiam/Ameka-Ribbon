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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Ameka.h"
#include "AmekaView.h"
#include "DSPModule.h"
#include "LoaddingDlg.h"
#include "QPrint.h"

#include "MainFrm.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

	BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
		ON_WM_CREATE()
		ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
		ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
		ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
		ON_COMMAND(ID_VIEW_CAPTION_BAR, &CMainFrame::OnViewCaptionBar)
		ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &CMainFrame::OnUpdateViewCaptionBar)
		ON_COMMAND(ID_TOOLS_OPTIONS, &CMainFrame::OnOptions)
		ON_COMMAND(MN_ScaleRate, &CMainFrame::OnScalerate)
		ON_COMMAND(MN_SpeedRate, &CMainFrame::OnSpeedrate)
		ON_COMMAND(MN_MonList, &CMainFrame::OnMonlist)
		ON_COMMAND(MN_LP, &CMainFrame::OnLp)
		ON_COMMAND(MN_HP, &CMainFrame::OnHp)
		ON_COMMAND(MN_Save, &CMainFrame::OnSave)
		ON_COMMAND(MN_Open, &CMainFrame::OnOpen)
		ON_UPDATE_COMMAND_UI(MN_StartDemo, &CMainFrame::OnUpdateStartdemo)
		ON_UPDATE_COMMAND_UI(MN_StopDemo, &CMainFrame::OnUpdateStopdemo)
		ON_UPDATE_COMMAND_UI(MN_Recording, &CMainFrame::OnUpdateRecording)
		ON_UPDATE_COMMAND_UI(MN_Scan, &CMainFrame::OnUpdateScan)
		ON_UPDATE_COMMAND_UI(MN_PortName, &CMainFrame::OnUpdatePortname)
		ON_UPDATE_COMMAND_UI(MN_Baud, &CMainFrame::OnUpdateBaud)
		ON_COMMAND(MN_Print, &CMainFrame::OnPrint)
		ON_COMMAND(MN_FullScr, &CMainFrame::OnFullscr)
	END_MESSAGE_MAP()

	// CMainFrame construction/destruction

	vector<string> Tokenize(CString str, string delimiters);

	CMainFrame::CMainFrame()
	{
		// TODO: add member initialization code here
		theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);

		//startEnable = FALSE;
		//stopEnable = FALSE;
		//recEnable = FALSE;
		portEnable = TRUE;
		baudEnable = TRUE;
		scanPortEnable = TRUE;
	}

	CMainFrame::~CMainFrame()
	{

	}


	int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
			return -1;

		BOOL bNameValid;

		CMDITabInfo mdiTabParams;
		mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
		mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
		mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
		mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
		mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
		EnableMDITabbedGroups(FALSE, mdiTabParams);

		m_wndRibbonBar.Create(this);
		m_wndRibbonBar.LoadFromResource(IDR_RIBBON);
		m_MainButton.SetVisible(FALSE);
		//m_MainButton.SetImage(IDB_MAIN);
		//m_MainButton.SetText(_T("\nf"));
		//m_MainButton.SetToolTipText(L"huhu");
		m_wndRibbonBar.SetApplicationButton(&m_MainButton, CSize(0,0));

		if (!m_wndStatusBar.Create(this))
		{
			TRACE0("Failed to create status bar\n");
			return -1;      // fail to create
		}

		CString strTitlePane1;
		CString strTitlePane2;
		bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
		ASSERT(bNameValid);
		bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
		ASSERT(bNameValid);
		m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);
		m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);

		// enable Visual Studio 2005 style docking window behavior
		CDockingManager::SetDockingMode(DT_SMART);
		// enable Visual Studio 2005 style docking window auto-hide behavior
		EnableAutoHidePanes(CBRS_ALIGN_ANY);

		// Create a caption bar:
		if (!CreateCaptionBar())
		{
			TRACE0("Không thể tạo thanh tiêu đề\n");
			return -1;      // fail to create
		}
		// set the visual manager and style based on persisted value
		OnApplicationLook(theApp.m_nAppLook);

		// Enable enhanced windows management dialog
		EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

		// Switch the order of document name and application name on the window title bar. This
		// improves the usability of the taskbar because the document name is visible with the thumbnail.
		ModifyStyle(0, FWS_PREFIXTITLE);

		int count = 0;
		//Set items for LowPassFilter
		CMFCRibbonComboBox* pSen = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_ScaleRate));
		if (pSen != NULL)
		{
			pSen->RemoveAllItems();
			vector<string> vecSen = Tokenize(theApp.m_sensitivity," ");
			for (vector<string>::iterator it = vecSen.begin(); it != vecSen.end(); it++) {
				CString cs((*it).c_str());
				pSen->AddItem(cs,count++);
			}
		}
		pSen->SetEditText(L"30");
		//Set items for LowPassFilter
		count = 0;
		CMFCRibbonComboBox* pSpeed = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_SpeedRate));
		if (pSpeed != NULL)
		{
			pSpeed->RemoveAllItems();
			vector<string> vecSpeed = Tokenize(theApp.m_speed," ");
			for (vector<string>::iterator it = vecSpeed.begin(); it != vecSpeed.end(); it++) {
				CString cs((*it).c_str());
				pSpeed->AddItem(cs,count++);
			}
		}
		pSpeed->SetEditText(L"75");
		//Set items for LowPassFilter
		count = 0;
		CMFCRibbonComboBox* pLP = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_LP));
		if (pLP != NULL)
		{
			pLP->RemoveAllItems();
			vector<string> vecLP = Tokenize(theApp.m_LP," ");
			for (vector<string>::iterator it = vecLP.begin(); it != vecLP.end(); it++) {
				CString cs((*it).c_str());
				pLP->AddItem(cs,count++);
			}
		}
		pLP->SetEditText(L"30");
		//Set items for HighPassFilter
		count = 0;
		CMFCRibbonComboBox* pHP = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_HP));
		if (pHP != NULL)
		{
			pHP->RemoveAllItems();
			vector<string> vecHP = Tokenize(theApp.m_HP," ");
			for (vector<string>::iterator it = vecHP.begin(); it != vecHP.end(); it++) {
				CString cs((*it).c_str());
				pHP->AddItem(cs,count++);
			}
		}
		pHP->SetEditText(L"0.5");

		//create montage list
		count = 0;
		CMFCRibbonComboBox* pMon = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_MonList));
		if (pMon)
		{
			//POSITION pos =  theApp.monList.GetHeadPosition();
			for (int i = 0; i < theApp.monList.GetSize(); i++)
			{
				Amontage tmp = theApp.monList.GetAt( i );
				pMon->AddItem(tmp.mName,count++);
			}
		}

		//show port list
		TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
		DWORD test;
		bool gotPort=0; // in case the port is not found
		CMFCRibbonComboBox* pPort = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_PortName));
		if (pPort != NULL)
		{
			for(int i=0; i<255; i++) // checking ports from COM0 to COM255
			{
				CString str;
				str.Format(_T("%d"),i);
				CString ComName=CString("COM") + CString(str); // converting to COM0, COM1, COM2

				test = QueryDosDevice(ComName, (LPWSTR)lpTargetPath, 5000);

				// Test the return value and error if any
				if(test!=0) //QueryDosDevice returns zero if it didn't find an object
				{
					pPort->AddItem((CString)ComName); // add to the ComboBox
					gotPort=1; // found port
				}

				if(::GetLastError()==ERROR_INSUFFICIENT_BUFFER) //in case buffer got filled
				{
					lpTargetPath[10000]; // in case the buffer got filled, increase size of the buffer.
					continue;
				}
			}
			if(!gotPort) // if not port
				pPort->SetEditText(L"N.A"); // to display error message incase no ports 
			else
			{
				//pPort->SetEditText(pPort->GetItem(0));
				pPort->SelectItem(0);
			}
		}

		CMFCRibbonComboBox* pBaud = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_Baud));
		if (pBaud != NULL)
		{
			pBaud->AddItem(L"9600");
			pBaud->AddItem(L"19200");
			pBaud->AddItem(L"38400");
			pBaud->AddItem(L"56000");
			pBaud->AddItem(L"115200");
			//pBaud->SetEditText("115200");
			pBaud->SelectItem(4);
		}

		EnableFullScreenMode (MN_FullScr);
		EnableFullScreenMainMenu(FALSE);

		return 0;
	}

	BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
	{
		if( !CMDIFrameWndEx::PreCreateWindow(cs) )
			return FALSE;
		// TODO: Modify the Window class or styles here by modifying
		//  the CREATESTRUCT cs

		return TRUE;
	}

	BOOL CMainFrame::CreateCaptionBar()
	{
		if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
		{
			TRACE0("Failed to create caption bar\n");
			return FALSE;
		}

		BOOL bNameValid;

		CString strTemp, strTemp2;
		bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetButton(strTemp, ID_TOOLS_OPTIONS, CMFCCaptionBar::ALIGN_LEFT, FALSE);
		bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON_TIP);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetButtonToolTip(strTemp);

		bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetText(strTemp, CMFCCaptionBar::ALIGN_LEFT);

		m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
		bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
		ASSERT(bNameValid);
		bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

		return TRUE;
	}

	// CMainFrame diagnostics

#ifdef _DEBUG
	void CMainFrame::AssertValid() const
	{
		CMDIFrameWndEx::AssertValid();
	}

	void CMainFrame::Dump(CDumpContext& dc) const
	{
		CMDIFrameWndEx::Dump(dc);
	}
#endif //_DEBUG


	// CMainFrame message handlers

	void CMainFrame::OnWindowManager()
	{
		ShowWindowsDialog();
	}

	void CMainFrame::OnApplicationLook(UINT id)
	{
		CWaitCursor wait;

		theApp.m_nAppLook = id;

		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_WIN_2000:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_OFF_XP:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_WIN_XP:
			CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_OFF_2003:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_VS_2005:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_VS_2008:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_WINDOWS_7:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(TRUE);
			break;

		default:
			switch (theApp.m_nAppLook)
			{
			case ID_VIEW_APPLOOK_OFF_2007_BLUE:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
				break;

			case ID_VIEW_APPLOOK_OFF_2007_BLACK:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
				break;

			case ID_VIEW_APPLOOK_OFF_2007_SILVER:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
				break;

			case ID_VIEW_APPLOOK_OFF_2007_AQUA:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
				break;
			}

			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
		}

		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

		theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
	}

	void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
	{
		pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
	}

	void CMainFrame::OnViewCaptionBar()
	{
		m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
		RecalcLayout(FALSE);
	}

	void CMainFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
	{
		pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
	}

	void CMainFrame::OnOptions()
	{
		CMFCRibbonCustomizeDialog *pOptionsDlg = new CMFCRibbonCustomizeDialog(this, &m_wndRibbonBar);
		ASSERT(pOptionsDlg != NULL);

		pOptionsDlg->DoModal();
		delete pOptionsDlg;
	}



	void CMainFrame::OnScalerate()
	{
		// TODO: Add your command handler code here
		CAmekaView* pView = CAmekaView::GetView();
		if (!pView)
			return;

		CMFCRibbonComboBox* pScale = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_ScaleRate));
		// Get the selected index
		int nCurSel =pScale->GetCurSel();
		if (nCurSel >= 0)
		{
			CString item=pScale->GetItem(nCurSel);
			EnterCriticalSection(&pView->csess);
			pView->graphData.scaleRate = atoi((LPCSTR)(CStringA)item);
			LeaveCriticalSection(&pView->csess);
			pView->OnDraw(pView->GetDC());
		}
	}


	void CMainFrame::OnSpeedrate()
	{
		// TODO: Add your command handler code here
		CAmekaView* pView = CAmekaView::GetView();
		if (!pView)
			return;
		CMFCRibbonComboBox* pScale = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_SpeedRate));
		// Get the selected index
		int nCurSel =pScale->GetCurSel();
		if (nCurSel >= 0)
		{
			CString item=pScale->GetItem(nCurSel);
			EnterCriticalSection(&pView->csess);
			pView->graphData.paperSpeed = atoi((LPCSTR)(CStringA)item);
			LeaveCriticalSection(&pView->csess);
			pView->OnDraw(pView->GetDC());
		}
	}

	void CMainFrame::OnMonlist()
	{
		// TODO: Add your command handler code here
		CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
		CMFCRibbonComboBox* pMon = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_MonList));

		CAmekaDoc* doc = CAmekaDoc::GetDoc();
		if (!doc)
			return;

		int nCurSel = pMon->GetCurSel();
		CString data = pMon->GetItem(nCurSel);

		//POSITION pos =  theApp.monList.GetHeadPosition();
		for (int i = 0; i < theApp.monList.GetSize(); i++)
		{
			Amontage mon =  theApp.monList.GetAt( i );
			if (mon.mName == data)
				doc->mMon = mon;
		}

		CAmekaView* pView = CAmekaView::GetView();
		if (pView)
			pView->OnDraw(pView->GetDC());
	}


	void CMainFrame::OnLp()
	{
		// TODO: Add your command handler code here
		CAmekaView* pView = CAmekaView::GetView();
		if (!pView)
			return;

		CMFCRibbonComboBox* pLP = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_LP));
		// Get the selected index
		int nCurSel =pLP->GetCurSel();
		if (nCurSel >= 0)
		{
			CString item=pLP->GetItem(nCurSel);
			EnterCriticalSection(&pView->csess);
			pView->GetDocument()->mDSP.LPFFre = atoi((LPCSTR)(CStringA)item);
			LeaveCriticalSection(&pView->csess);
			if (pView->isDrawRec)
			{
				dsp_processing(pView->GetDocument());
			}
			pView->OnDraw(pView->GetDC());
		}
	}


	void CMainFrame::OnHp()
	{
		// TODO: Add your command handler code here
		CAmekaView* pView = CAmekaView::GetView();
		if (!pView)
			return;

		CMFCRibbonComboBox* pHP = DYNAMIC_DOWNCAST(
			CMFCRibbonComboBox, m_wndRibbonBar.FindByID(MN_HP));
		// Get the selected index
		int nCurSel =pHP->GetCurSel();
		if (nCurSel >= 0)
		{
			CString item=pHP->GetItem(nCurSel);
			EnterCriticalSection(&pView->csess);
			pView->GetDocument()->mDSP.HPFFre = atoi((LPCSTR)(CStringA)item);
			LeaveCriticalSection(&pView->csess);
			if (pView->isDrawRec)
			{
				dsp_processing(pView->GetDocument());
			}
			pView->OnDraw(pView->GetDC());
		}
	}


	void CMainFrame::OnSave()
	{
		// TODO: Add your command handler code here
		CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
		CAmekaView* pView = CAmekaView::GetView();

		//check if not in recording mode
		if (!pDoc || !pView)
			return;
		if (!pView->isDrawRec)
			return;

		pDoc->object.Open(pDoc->recordFileName, CFile::modeReadWrite);
		//pDoc->object.SeekToBegin();
		uint16_t temp[8];
		temp[0] = (uint16_t)(pDoc->mDSP.HPFFre * 10);
		temp[1] = (uint16_t)(pDoc->mDSP.LPFFre * 10);
		temp[2] = (uint16_t)(pDoc->mDSP.epocLength * 10);
		temp[3] = pDoc->mDSP.SampleRate;
		temp[4] = (uint16_t)(pDoc->counter);
		temp[5] = (uint16_t)(pDoc->counter >> 16);
		temp[6] = (uint16_t)(pDoc->counter >> 32);
		temp[7] = (uint16_t)(pDoc->counter >> 48);
		pDoc->object.Write(temp, sizeof(temp));

		uint8_t nLen = _tcslen(pDoc->mMon.mName);
		uint8_t temp_mon[85];
		uint8_t monNum =  pDoc->mMon.mList.GetCount();
		temp_mon[64] = monNum;
		POSITION pos;
		//pos = pDoc->mMon.mList.GetHeadPosition();
		if (monNum > 32)
			monNum = 32;
		for (int i=0; i<monNum; i++)
		{
			Alead temp;
			temp = pDoc->mMon.mList.GetAt(i);
			uint8_t fID = temp.lFirstID;
			uint8_t sID = temp.lSecondID;
			temp_mon[i*2] = fID;
			temp_mon[i*2 + 1] = sID;
		}
		temp_mon[65] = nLen; // so ky tu cua montage name
		temp_mon[66] = _tcsclen(pDoc->patientInfo.fname); // So ky tu cua Ho
		temp_mon[67] = _tcsclen(pDoc->patientInfo.lname); // So ky tu cua Ten
		temp_mon[68] = _tcsclen(pDoc->patientInfo.note); // So ky tu cua Ghi chu
		temp_mon[69] = _tcsclen(pDoc->patientInfo.uID); // So ky tu cua uID
		temp_mon[70] = _tcsclen(pDoc->patientInfo.sex); // So ky tu cua Gioi tinh
		temp_mon[71] = pDoc->patientInfo.lefthanded; // Thuan tay trai
		temp_mon[72] = _tcsclen(pDoc->patientInfo.surname); // So ky tu cua Ten dem
		// Reserved for birthday
		time_t birthday;
		birthday = pDoc->patientInfo.birthday.GetTime();
		temp_mon[73] = birthday;
		temp_mon[74] = birthday >> 8;
		temp_mon[75] = birthday >> 16;
		temp_mon[76] = birthday >> 24;
		temp_mon[77] = birthday >> 32;
		temp_mon[78] = birthday >> 40;
		temp_mon[79] = birthday >> 48;
		temp_mon[80] = birthday >> 56;

		pDoc->object.Write(temp_mon, sizeof(temp_mon));

		// Ghi montage name vao file
		char *szTo = new char[nLen + 1];
		WideCharToMultiByte(1258, 0, pDoc->mMon.mName, nLen, szTo, nLen, NULL, NULL);			
		int size = sizeof(szTo);
		pDoc->object.Write(szTo, (nLen + 1)*sizeof(char));
		delete szTo;

		uint8_t no_c;
		// Ghi Ho vao file
		no_c = _tcsclen(pDoc->patientInfo.fname);
		szTo = new char[no_c + 1];
		WideCharToMultiByte(1258, 0, pDoc->patientInfo.fname, no_c, szTo, nLen, NULL, NULL);			
		size = sizeof(szTo);
		pDoc->object.Write(szTo, (no_c + 1)*sizeof(char));
		delete szTo;

		// Ghi Ten vao file
		no_c = _tcsclen(pDoc->patientInfo.lname);
		szTo = new char[no_c + 1];
		WideCharToMultiByte(1258, 0, pDoc->patientInfo.lname, no_c, szTo, nLen, NULL, NULL);			
		size = sizeof(szTo);
		pDoc->object.Write(szTo, (no_c + 1)*sizeof(char));
		delete szTo;

		// Ghi ghi chu vao file
		no_c = _tcsclen(pDoc->patientInfo.note);
		szTo = new char[no_c + 1];
		WideCharToMultiByte(1258, 0, pDoc->patientInfo.note, no_c, szTo, nLen, NULL, NULL);			
		size = sizeof(szTo);
		pDoc->object.Write(szTo, (no_c + 1)*sizeof(char));
		delete szTo;

		// Ghi uID vao file
		no_c = _tcsclen(pDoc->patientInfo.uID);
		szTo = new char[no_c + 1];
		WideCharToMultiByte(1258, 0, pDoc->patientInfo.uID, no_c, szTo, nLen, NULL, NULL);			
		size = sizeof(szTo);
		pDoc->object.Write(szTo, (no_c + 1)*sizeof(char));
		delete szTo;

		// Ghi Gioi tinh vao file
		no_c = _tcsclen(pDoc->patientInfo.sex);
		szTo = new char[no_c + 1];
		WideCharToMultiByte(1258, 0, pDoc->patientInfo.sex, no_c, szTo, nLen, NULL, NULL);			
		size = sizeof(szTo);
		pDoc->object.Write(szTo, (no_c + 1)*sizeof(char));
		delete szTo;

		// Ghi Ten dem vao file
		no_c = _tcsclen(pDoc->patientInfo.surname);
		szTo = new char[no_c + 1];
		WideCharToMultiByte(1258, 0, pDoc->patientInfo.surname, no_c, szTo, nLen, NULL, NULL);			
		size = sizeof(szTo);
		pDoc->object.Write(szTo, (no_c + 1)*sizeof(char));
		delete szTo;

		pDoc->object.Close();

		char strFilter[] = { "Ameka Save File (*.amek)|*.amek|" }; 
		CFileDialog FileDlg(FALSE, CString(".amek"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString(strFilter));
		if (FileDlg.DoModal() == IDOK)  
		{ 
			try
			{
				CString saveFName = FileDlg.GetPathName();
				if (pDoc->saveFileName.Find(L".dat") != -1)
					//if (pDoc->saveFileName.Find(L".dat"))

						CFile::Rename(pDoc->saveFileName, saveFName);
				else
				{
					::CopyFile(pDoc->saveFileName, saveFName, FALSE);
				}
				pDoc->saveFileName = saveFName;
				pDoc->recordFileName = saveFName;
			}
			catch(CFileException* pEx )
			{
				TRACE(_T("File %20s not found, cause = %d\n"), pDoc->saveFileName, 
					pEx->m_cause);
				pEx->Delete();
			}
		}
	}


	void CMainFrame::OnOpen()
	{
		// TODO: Add your command handler code here
		CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
		CAmekaView* pView = CAmekaView::GetView();

		//check if not in recording mode

		char strFilter[] = { "Ameka Save File (*.amek)|*.amek|" }; 
		CFileDialog FileDlg(TRUE, CString(".amek"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString(strFilter));
		if (FileDlg.DoModal() == IDOK)  
		{ 
			if (!pDoc || !pView)
			{
				POSITION pos=AfxGetApp()->GetFirstDocTemplatePosition();

				CDocTemplate* pTemplate=AfxGetApp()->GetNextDocTemplate(pos); 

				CAmekaDoc* pSim =(CAmekaDoc*) pTemplate->OpenDocumentFile(NULL);

				pDoc = CAmekaDoc::GetDoc();
				pView = CAmekaView::GetView();
				//return;
			}

			pDoc->saveFileName = FileDlg.GetPathName();
			pDoc->recordFileName = FileDlg.GetPathName();

			DWORD exit_code= NULL;
			if (pView->pThread != NULL && pView->isRunning)
			{
				GetExitCodeThread(pView->pThread->m_hThread, &exit_code);
				if(exit_code == STILL_ACTIVE)
				{
					::TerminateThread(pView->pThread->m_hThread, 0);
					CloseHandle(pView->pThread->m_hThread);
				}
				pView->pThread->m_hThread = NULL;
				pView->pThread = NULL;
				pView->isRunning = false;
			}

			if (pDoc->isRecord)
			{
				pDoc->isRecord = FALSE;

				/*if (WaitForSingleObject(pDoc->CloseFileEvent, INFINITE) == WAIT_OBJECT_0)
				{*/
				exit_code = NULL;
				GetExitCodeThread(pDoc->m_dspProcess->m_hThread, &exit_code);
				if(exit_code == STILL_ACTIVE)
				{
					::TerminateThread(pDoc->m_dspProcess->m_hThread, 0);
					CloseHandle(pDoc->m_dspProcess->m_hThread);
				}
				pDoc->m_dspProcess->m_hThread = NULL;
				pDoc->m_dspProcess = NULL;
			}

			CLoaddingDlg dlg;
			dlg.Create(DLG_Loading, NULL);
			dlg.ShowWindow(SW_NORMAL);
			if (WaitForSingleObject(pDoc->onReadSuccess, INFINITE) == WAIT_OBJECT_0)
			{

				//AfxMessageBox(L"Load file success");
				ResetEvent(pDoc->onReadSuccess);
				pView->isDrawRec = TRUE;
				pView->isNull = FALSE;;
				pView->isRunning = FALSE;
				pView->OnDraw(pView->GetDC());
			}
			else
			{

			}
			dlg.ShowWindow(SW_HIDE); 
		}
	}


	void CMainFrame::OnUpdateStartdemo(CCmdUI *pCmdUI)
	{
		// TODO: Add your command update UI handler code here
		CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
		if (!pDoc)
			pCmdUI->Enable(FALSE);
		else
			pCmdUI->Enable(pDoc->startEnable);
	}


	void CMainFrame::OnUpdateStopdemo(CCmdUI *pCmdUI)
	{
		// TODO: Add your command update UI handler code here
		CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
		if (!pDoc)
			pCmdUI->Enable(FALSE);
		else
			pCmdUI->Enable(pDoc->stopEnable);
	}


	void CMainFrame::OnUpdateRecording(CCmdUI *pCmdUI)
	{
		// TODO: Add your command update UI handler code here
		CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
		if (!pDoc)
			pCmdUI->Enable(FALSE);
		else
			pCmdUI->Enable(pDoc->recEnable);
	}


	void CMainFrame::OnUpdateScan(CCmdUI *pCmdUI)
	{
		// TODO: Add your command update UI handler code here
		pCmdUI->Enable(scanPortEnable);
	}


	void CMainFrame::OnUpdatePortname(CCmdUI *pCmdUI)
	{
		// TODO: Add your command update UI handler code here
		pCmdUI->Enable(portEnable);
	}


	void CMainFrame::OnUpdateBaud(CCmdUI *pCmdUI)
	{
		// TODO: Add your command update UI handler code here
		pCmdUI->Enable(baudEnable);
	}


	void CMainFrame::OnPrint()
	{
		CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
		if (!pDoc)
			return;

		// TODO: Add your command handler code here
		CQPrint prt;
		HPRIVATEFONT   hFont;

		// Step 1 : call the CPrintDialog
		if (prt.Dialog() == -1)
			return;
		//prt.SetPageOrientation(DMORIENT_LANDSCAPE);
		//Step 2 : Start the Print
		prt.StartPrint();      
		prt.SetMargins(theApp.marginTop, theApp.marginBot, theApp.marginLeft, theApp.marginRight);

		//Step 3 : Create a printing font
		char strBuf[100];
		sprintf(strBuf, "%S", theApp.printFont);
		hFont = prt.AddFontToEnvironment(strBuf, theApp.printSize, (theApp.printSize*3)/2); 
		prt.SetActiveFont(hFont);
		prt.SetDistance(theApp.printDistance);  
		//prt.SetPageOrientation(DMORIENT_LANDSCAPE);

		//Step 4 : Start Page
		prt.StartPage(); 

		//Step 5 : The actual printing goes here
		//print partien info
		try
		{
			prt.Print(hFont,L"Bệnh nhân: " + pDoc->patientInfo.fname + " " + pDoc->patientInfo.surname + " " + pDoc->patientInfo.lname,FORMAT_NORMAL);   
			prt.Print(hFont,L"Giới tính: " + pDoc->patientInfo.sex, FORMAT_NORMAL);   
			prt.Print(hFont,L"Ngày sinh: " + pDoc->patientInfo.birthday.Format(_T("%B %d, %Y")), FORMAT_NORMAL);   
		}
		catch (exception e)
		{
			MessageBoxW(L"Không thể in trong trạng thái hiện tại!");
		}

		//print line
		prt.Line(PS_SOLID);

		//print graph
		//prt.InsertBitmap(IDB_MAIN, FORMAT_NORMAL, NULL, 0);
		prt.InsertBitmapFromView(FORMAT_NORMAL, NULL, 0);

		//Step 6 :  now end the page
		prt.EndPage();

		//Step 7 :  close the print document
		//          and release it in the spooler
		prt.EndPrint();  
	}


	void CMainFrame::OnFullscr()
	{
		// TODO: Add your command handler code here
		CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->GetMainWnd();

		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd*)pFrame->GetActiveFrame();

		pChild->ActivateFrame(SW_MAXIMIZE);

		ShowFullScreen();
	}

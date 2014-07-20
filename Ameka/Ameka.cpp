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

// Ameka.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include <Winspool.h>
#include "afxwinappex.h"
#include "afxdialogex.h"
#include <vector>
#include "afxwin.h"
#include "afxcolorbutton.h"
#include "afxcmn.h"

#include "Ameka.h"

//#include "easylogging++.h"
#include "DSPModule.h"
#include "GraphModule.h"
#include "SerialCtrl.h"
#include "LoaddingDlg.h"
#include "MontageDlg.h"
#include "OptionDlg.h"
#include "InfoDlg.h"
#include "PhoticDlg.h"
#include "EventDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//_INITIALIZE_EASYLOGGINGPP

using namespace std;

// CAmekaApp

BEGIN_MESSAGE_MAP(CAmekaApp, CWinAppEx)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)

	ON_COMMAND(MN_About, &CAmekaApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(MN_New, &CWinAppEx::OnFileNew)
	//ON_COMMAND(MN_Open, &CWinAppEx::OnFileOpen)
	ON_COMMAND(MN_Close, &CAmekaApp::OnFileClose)
	//ON_COMMAND(MN_Print, &CWinAppEx::OnFilePrintSetup)
	ON_COMMAND(MN_Setting, &CAmekaApp::OnSetting)
	ON_COMMAND(MN_Info, &CAmekaApp::OnInfo)
	ON_COMMAND(MN_Option, &CAmekaApp::OnOption)
	ON_COMMAND(MN_Photic, &CAmekaApp::OnPhotic)
	ON_COMMAND(MN_Log, &CAmekaApp::OnLog)
	ON_COMMAND(MN_Wave, &CAmekaApp::OnWave)
	ON_COMMAND(MN_Event, &CAmekaApp::OnEvent)
	ON_COMMAND(MN_StartDemo, &CAmekaApp::OnDemo)
	ON_COMMAND(MN_StopDemo, &CAmekaApp::OnStop)
	ON_COMMAND(MN_Montage, &CAmekaApp::OnMontage)
	ON_COMMAND(MN_Analyze, &CAmekaApp::OnSetupPhotic)
	ON_COMMAND(MN_PortOpen, &CAmekaApp::OnPortOpen)
	ON_COMMAND(MN_Scan, &CAmekaApp::OnScan)
	ON_COMMAND(MN_Lan, &CAmekaApp::OnLan)
	ON_COMMAND(MN_Recording, &CAmekaApp::OnRecording)
	//	ON_COMMAND(MN_StopRec, &CAmekaApp::OnStoprec)
END_MESSAGE_MAP()

//-------------------------------------------------------//

// CAmekaApp construction

CAmekaApp::CAmekaApp()
{
	//photic
	this->photicMax = 50.0;
	this->photicMin = 0.0;
	this->photicTick = 10.0;
	this->photicBarW = 2.0;
	this->photicWRate = 50;
	this->pIO = NULL;
	//init electrode name
	const char* setFileName = settingName;
	//writeSetting(setFileName);
	for (int i = 0; i < 10; i++)
		evName[i] = "undefined";
	loadSetting(setFileName);
	//init Language
	mnLan = new amekaLan();
	//init buffer
	dataBuffer = new amekaData<RawDataType>(4096);
	//dataBuf = new AmekaData<RawDataType>(4096);

	//initialize log
	//el::Configurations defaultConf;
	//defaultConf.setToDefault();
	////defaultConf.setGlobally(el::ConfigurationType::Filename, "logs\\Log.txt");
	//defaultConf.setGlobally(el::ConfigurationType::LogFlushThreshold, "1000");
	//defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
	//el::Loggers::reconfigureLogger("default", defaultConf);
	//LOG(INFO) << "Log using default file";

	//initialize montage list

	TiXmlDocument doc;
	if(!doc.LoadFile(xmlName))
	{
		//LOG(ERROR) << doc.ErrorDesc();
		return;
	}
	TiXmlElement* root = doc.FirstChildElement();
	if(root == NULL)
	{
		//LOG(ERROR) << "Failed to load file: No root element.";
		doc.Clear();
		return;
	}
	for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		Amontage mon;
		mon.mName = elem->Value();
		for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
		{
			CString attr1, attr2;
			attr1 = e->Attribute("channel1");
			attr2 = e->Attribute("channel2");
			if (attr1 != "" && attr2 != "")
			{
				Alead lead;
				lead.lFirstID = atoi((LPCSTR)(CStringA)attr1);
				lead.lSecondID = atoi((LPCSTR)(CStringA)attr2);
				mon.mList.Add(lead);
				//delete lead;
			}
		}
		//delete mon;
		monList.Add(mon);
	}
	doc.Clear();
	//init ribbon
	m_sensitivity = strSen;
	m_speed = strSpeed;
	m_LP = strLP;
	m_HP = strHP;
	m_dotPmm = strdotPmm;

	marginLeft = LEFT_MARGIN;
	marginRight= RIGHT_MARGIN;
	marginTop = TOP_MARGIN;
	marginBot = BOT_MARGIN;
	printFont = DEF_FONT;
	printSize = DEF_SIZE;
	printDistance = DEF_DISTANCE;

	char line[100];
	ifstream setFile;

	setFile.open(settingFileName);
	if(!setFile)
	{
		std::cerr<<"Cannot open the output file."<<std::endl;
		m_portNo = "COM8";
		m_baudRate = "115200";
	}
	else
	{
		setFile >> line;
		m_portNo = line;
		setFile >> line;
		m_baudRate = line;
		setFile.close();
	}


	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using ComMon Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Ameka.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CAmekaApp object

CAmekaApp theApp;


// CAmekaApp initialization


BOOL CAmekaApp::InitInstance()
{
	SetLandscape();

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	// InitComMonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the comMon control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(7);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_AmekaFileTYPE,
		RUNTIME_CLASS(CAmekaDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CAmekaView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CAmekaApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
	AfxOleTerm(FALSE);
	if (this->pIO)
	{
		delete pIO;
	}
	delete dataBuffer;
	delete mnLan;
	delete [] mElec;
	/*POSITION pos = monList.GetHeadPosition();
	while(pos)
	{
	Amontage mon = monList.GetNext(pos);
	POSITION pos1 = mon.mList.GetHeadPosition();
	while(pos1)
	{
	delete mon.mList.GetNext(pos1);
	}
	mon.mList.RemoveAll();
	delete mon;
	}
	monList.RemoveAll();*/
	return CWinAppEx::ExitInstance();
}

// CAmekaApp message handlers

//------------------------------------------------------------------//
// CAboutDlg dialog used for App About
//------------------------------------------------------------------//

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()

// App command to run the dialog
void CAmekaApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CAmekaApp customization load/save methods

void CAmekaApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CAmekaApp::LoadCustomState()
{
}

void CAmekaApp::SaveCustomState()
{
}

// CAmekaApp message handlers

//------------------------------------------------------------------//
// CSettingDlg
//------------------------------------------------------------------//

class CSettingDlg : public CDialogEx
{
public:
	CSettingDlg();

	// Dialog Data
	enum { IDD = DLG_Setting };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSetup();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

CSettingDlg::CSettingDlg() : CDialogEx(CSettingDlg::IDD)
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSettingDlg, CDialogEx)
	ON_COMMAND(MN_Setting, &CSettingDlg::OnSetup)
	ON_BN_CLICKED(IDOK, &CSettingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSettingDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnSetting()
{
	CSettingDlg settingDlg;
	settingDlg.DoModal();
}

//Show Setting Dialog
void CAmekaApp::OnInfo()
{
	CInfoDlg infoDlg;
	infoDlg.DoModal();
}

//Show Setting Dialog
void CAmekaApp::OnPhotic()
{
	/*
	CPhoticDlg photicDlg;
	photicDlg.DoModal();
	*/
	CAmekaView* pView = CAmekaView::GetView();
	if (pView)
	{
		//Cameka
		pView->onPhotic = !pView->onPhotic;
		if (pView->onPhotic && !pView->isDrawRec)
		{
			CAmekaView *pView = CAmekaView::GetView();
			pView->pPhoticThread = AfxBeginThread(pView->photicHandle, (LPVOID)pView);
		}
		else
		{
			if (pView->isDrawRec)
			{
				CDC* pDC = pView->GetDC();
				CDC MemDC;
				CRect rect;

				pView->GetClientRect(&rect);
				uint64_t startPos = rect.Width()*FACTOR;

				if (NULL == MemDC.CreateCompatibleDC(pDC))
				{
					MemDC.DeleteDC();
					DeleteObject(pDC);
					return;

				}
				CBitmap* bitmap = new CBitmap;
				if(bitmap != NULL)
				{
					if(NULL == bitmap->CreateCompatibleBitmap(pDC, (rect.Width()-startPos), rect.Height()))
					{
						DWORD tmp = GetLastError();
						////LOG(ERROR) << static_cast <int>(tmp);
						DeleteObject(pDC);
						delete bitmap;
						return;
					}
				}
				//fill background
				CBitmap* pOldBmp = MemDC.SelectObject(bitmap);

				CBrush brush;
				brush.CreateSolidBrush(CUSTOM_BARBACK);
				CRect mrect(0,0,rect.Width(),rect.Height());
				MemDC.FillRect(mrect,&brush);

				//draw grid
				float range = theApp.photicMax - theApp.photicMin;
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
				DeleteObject(&pen2);
				MemDC.DeleteDC();
				DeleteObject(pDC);

				startPos = pView->GetScrollPos(SB_HORZ);
				photic_processing(FRE_STEP, pView->GetDocument(), startPos);
				pView->drawBarGraph();
				return;
			}
			if (theApp.docList.IsEmpty())
				return;
			DWORD exit_code= NULL;
			if (pView->pPhoticThread != NULL)
			{
				GetExitCodeThread(pView->pPhoticThread->m_hThread, &exit_code);
				if(exit_code == STILL_ACTIVE)
				{
					::TerminateThread(pView->pPhoticThread->m_hThread, 0);
					CloseHandle(pView->pPhoticThread->m_hThread);
				}
				pView->pPhoticThread->m_hThread = NULL;
				pView->pPhoticThread = NULL;
			}
		}
		pView->OnDraw(pView->GetDC());
	}
}

void CAmekaApp::OnSetupPhotic()
{
	CPhoticDlg photicDlg;
	photicDlg.DoModal();
	CAmekaView* pView = CAmekaView::GetView();
	if (pView)
	{
		pView->OnDraw(pView->GetDC());
	}
}
//------------------------------------------------------------------//
// CLogDlg
//------------------------------------------------------------------//

class CLogDlg : public CDialogEx
{
public:
	CLogDlg();

	// Dialog Data
	enum { IDD = DLG_Log};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedok();
	afx_msg void OnBnClickedcancel();
};

CLogDlg::CLogDlg() : CDialogEx(CLogDlg::IDD)
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialogEx)
	ON_BN_CLICKED(log_ok, &CLogDlg::OnBnClickedok)
	ON_BN_CLICKED(log_cancel, &CLogDlg::OnBnClickedcancel)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnLog()
{
	CLogDlg logDlg;
	logDlg.DoModal();
}

//------------------------------------------------------------------//
// CWaveDlg
//------------------------------------------------------------------//

class CWaveDlg : public CDialogEx
{
public:
	CWaveDlg();

	// Dialog Data
	enum { IDD = DLG_Wave};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

CWaveDlg::CWaveDlg() : CDialogEx(CWaveDlg::IDD)
{
}

void CWaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWaveDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CWaveDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWaveDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnWave()
{
	MessageBoxA(NULL,"Tính năng này tạm thời chưa được hỗ trợ!","Thông báo",0);
	/*
	CWaveDlg waveDlg;
	waveDlg.DoModal();
	*/
}

//------------------------------------------------------------------//
void CAmekaApp::OnFileClose()
{
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (pDoc != NULL)
	{
		POSITION pos = pDoc->GetFirstViewPosition(); 
		if (pos != NULL) 
		{ 
			CView* pView = pDoc->GetNextView(pos); 
			if (pView) 
			{ 
				CFrameWnd* pFrame = pView->GetParentFrame(); 
				if (pFrame) 
				{ 
					pFrame->SendMessage(WM_CLOSE); 
				} 
			} 
		}
	}
}


//Show Setting Dialog
void CAmekaApp::OnEvent()
{
	CEventDlg eventDlg;
	eventDlg.DoModal();
}

//------------------------------------------------------------------//

//UINT genData(LPVOID pParam)
//{
//	while(1)
//	{
//		RawDataType* data = new RawDataType[1];
//		data->time = 0;
//		for (int i = 0; i < 15; i++)
//		{
//			data->value[i] = rand()%100;
//		}
//		theApp.dataBuffer->pushData(data, 1);
//		Sleep(3);
//		delete[] data;
//	}
//	return 0;
//}

//Demo Graph
void CAmekaApp::OnDemo()
{
	//AfxBeginThread(genData, NULL);
	/*
	CAmekaView *pView = CAmekaView::GetView();
	if (!pView->isRunning)
	{
	pView->pThread = AfxBeginThread(pView->graphHandle, (LPVOID)pView);
	}	pView->isRunning = true;
	*/
	if (theApp.docList.IsEmpty())
		return;
	//if ((theApp.pIO != NULL) && (theApp.pIO->m_bState == S_CONNECTED))
	CAmekaView *pView = CAmekaView::GetView();
	CAmekaDoc *pDoc = CAmekaDoc::GetDoc();
	if (pView->isRunning || pDoc->isRecord)
		return;
	{
		pView->isDrawRec = FALSE;
		CRect rect;
		pView->GetClientRect(&rect);
		CSize sizeTotal;
		// TODO: calculate the total size of this view

		sizeTotal.cx = rect.Width();
		sizeTotal.cy = rect.Height();
		pView->SetScrollSizes(MM_TEXT, sizeTotal);

		{
			//LPVOID pParam;
			initial_dsp_data((LPVOID)pDoc);

			pDoc->m_dspProcess = AfxBeginThread(DSP::DSPThread, (LPVOID)pDoc);
			pView->resetData();
			pView->OnDraw(pView->GetDC());
			pView->pThread = AfxBeginThread(pView->graphHandle, (LPVOID)pView);
			pView->isRunning = true;
		}
	}

}

void CAmekaApp::OnStop()
{
	CAmekaView *pView = CAmekaView::GetView();
	if (theApp.docList.IsEmpty())
		return;
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();

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

	/*if (!pDoc->isRecord)
	return;*/

	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonButton* pStopRec = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_StopDemo));
	if (!pStopRec)
		return;

	if (pDoc)
	{
		exit_code = NULL;
		if (pDoc->m_dspProcess != NULL)
		{
			GetExitCodeThread(pDoc->m_dspProcess->m_hThread, &exit_code);
			if(exit_code == STILL_ACTIVE)
			{
				::TerminateThread(pDoc->m_dspProcess->m_hThread, 0);
				CloseHandle(pDoc->m_dspProcess->m_hThread);
			}
			pDoc->m_dspProcess->m_hThread = NULL;
			pDoc->m_dspProcess = NULL;
		}
		if (pDoc->isRecord)
		{
			pDoc->isRecord = FALSE;

			/*if (WaitForSingleObject(pDoc->CloseFileEvent, INFINITE) == WAIT_OBJECT_0)
			{*/


			pDoc->saveFileName = pDoc->recordFileName;
			pDoc->object.SeekToBegin();
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
			pDoc->isOpenFile = FALSE;
			pDoc->counter = 0;

			CLoaddingDlg dlg;
			dlg.Create(DLG_Loading, NULL);
			dlg.ShowWindow(SW_NORMAL);
			if (WaitForSingleObject(pDoc->onReadSuccess, INFINITE) == WAIT_OBJECT_0)
			{

				//AfxMessageBox(L"Load file success");
				ResetEvent(pDoc->onReadSuccess);
				pView->isDrawRec = TRUE;
				pView->drawLeadName(pView->GetDC());
				pView->drawRecData(pView->GetDC());
				/*if (pView->onPhotic)
				{

				photic_processing(FRE_STEP, pView->GetDocument(), pView->this->GetScrollPos(SB_HORZ));
				pView->drawBarGraph();
				}*/
			}
			else
			{

			}
			dlg.ShowWindow(SW_HIDE); 
		}
	}
}

//------------------------------------------------------------------//


//------------------------------------------------------------------//


vector<string> Tokenize(CString buf, string delimiters)
{   
	vector<string> tokens;
	CT2CA pszConvertedAnsiString (buf);
	std::string str(pszConvertedAnsiString);
	string::size_type nwpos; //position of first non white space, which means it is     first real char
	nwpos = str.find_first_not_of(delimiters, 0); //ignore the whitespace before the first word

	string::size_type pos = str.find_first_of(delimiters, nwpos);

	while (string::npos != pos || string::npos != nwpos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(nwpos, pos - nwpos));
		// Skip delimiters.  Note the "not_of"
		nwpos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, nwpos);
	}
	return tokens;
};


void CSettingDlg::OnSetup()
{
	// TODO: Add your command handler code here
}

//Show Montage Dialog
void CAmekaApp::OnMontage()
{
	CMontageDlg montageDlg;
	montageDlg.DoModal();
}

//Show Setting Dialog
void CAmekaApp::OnOption()
{
	COptionDlg optionDlg;
	optionDlg.DoModal();
}

void CEventDlg::OnBnClickedok()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CEventDlg::OnBnClickedcancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CLogDlg::OnBnClickedok()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CLogDlg::OnBnClickedcancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

void CSettingDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CSettingDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CWaveDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CWaveDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CAmekaApp::OnPortOpen()
{
	// TODO: Add your command handler code here
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	CString tmp;

	CMFCRibbonButton* pPortOpen = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_PortOpen));

	if (!pPortOpen)
		return;

	CMFCRibbonComboBox* pPort = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_PortName));
	if (pPort != NULL)
	{
		tmp = pPort->GetItem(pPort->GetCurSel());
		if (!tmp)
			return;
		else
			m_portNo = tmp;
	}

	CMFCRibbonComboBox* pBaud = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_Baud));
	if (pBaud != NULL)
	{
		tmp = pBaud->GetItem(pBaud->GetCurSel());
		if (!tmp)
			return;
		else
			m_baudRate = tmp;
	}

	//PortOpen portDlg;
	//portDlg.DoModal();
	CString portFullName = L"\\\.\\" + m_portNo;
	//MessageBox("Opening Port " + portFullName,"Info",0);
	if (!pIO)
	{
		pIO = new CSerialIO(portFullName, m_baudRate);
		Sleep(1000);
		if (pIO->m_bState == S_CONNECTED)
		{
			pMainWnd->stopEnable = TRUE;
			pMainWnd->startEnable = TRUE;
			pMainWnd->recEnable = TRUE;
			pMainWnd->portEnable = FALSE;
			pMainWnd->baudEnable = FALSE;
			pMainWnd->scanPortEnable = FALSE;
			pPortOpen->SetText(L"Đóng cổng");
			//AfxMessageBox(L"Mở cổng thành công");
		}
		else
		{
			delete pIO;
			pIO = NULL;
			//AfxMessageBox(L"Lỗi kết nối phần cứng");
		}
	}
	else
	{
		pPortOpen->SetText(L"Mở cổng");
		pMainWnd->startEnable = FALSE;
		pMainWnd->stopEnable = FALSE;
		pMainWnd->recEnable = FALSE;
		pMainWnd->portEnable = TRUE;
		pMainWnd->baudEnable = TRUE;
		pMainWnd->scanPortEnable = TRUE;
		delete pIO;
		pIO = NULL;
		AfxMessageBox(L"Đã đóng cổng");
	}

	//m_dspProcess->setOwner(this);
}


void CAmekaApp::OnScan()
{
	// TODO: Add your command handler code here
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	//show port list
	TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	DWORD test;
	bool gotPort=0; // in case the port is not found
	CMFCRibbonComboBox* pPort = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_PortName));
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
}



void CAmekaApp::OnLan()
{
	// Create an instance First
	CFileDialog fOpenDlg(TRUE, L".lan", NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, L"Ameka Language Files (*.lan)|*.lan||");

	// Initializes m_pOFN structure
	fOpenDlg.m_pOFN->lpstrTitle=L"Ameka Language File";

	// Call DoModal

	if(fOpenDlg.DoModal()==IDOK)
	{
		CString tmp = fOpenDlg.GetPathName();
		CStringA tmp1(tmp);
		const char* fileName = (LPCSTR)tmp1;
		loadLanguage(fileName);
		// Do something useful here
	}

	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();

	//Menu File
	CMFCRibbonButton* pNew = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_New));
	if (pNew != NULL)
		pNew->SetText(mnLan->mnFile.strNew);

	CMFCRibbonButton* pOpen = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Open));
	if (pOpen != NULL)
		pOpen->SetText(mnLan->mnFile.strOpen);

	CMFCRibbonButton* pClose = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Close));
	if (pClose != NULL)
		pClose->SetText(mnLan->mnFile.strClose);

	CMFCRibbonButton* pSave = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Save));
	if (pSave != NULL)
		pSave->SetText(mnLan->mnFile.strSave);

	CMFCRibbonButton* pPrint = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Print));
	if (pPrint != NULL)
		pPrint->SetText(mnLan->mnFile.strPrint);

	CMFCRibbonComboBox* pPortName = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_PortName));
	if (pPortName != NULL)
		pPortName->SetText(mnLan->mnFile.strPortName);

	CMFCRibbonComboBox* pBaud = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_Baud));
	if (pBaud != NULL)
		pBaud->SetText(mnLan->mnFile.strBaudRate);

	CMFCRibbonButton* pScan = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Scan));
	if (pScan != NULL)
		pScan->SetText(mnLan->mnFile.strScanPort);

	CMFCRibbonButton* pPortOpen = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_PortOpen));
	if (pPortOpen != NULL)
		pPortOpen->SetText(mnLan->mnFile.strOpenPort);

	pPortOpen->GetParentCategory()->SetName(mnLan->mnFile.strMenuName);

	//Menu Option
	CMFCRibbonButton* pAnl = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Analyze));
	if (pAnl != NULL)
		pAnl->SetText(mnLan->mnOpt.strAnl);

	CMFCRibbonButton* pMon = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Montage));
	if (pMon != NULL)
		pMon->SetText(mnLan->mnOpt.strMon);

	CMFCRibbonButton* pEvent = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Event));
	if (pEvent != NULL)
		pEvent->SetText(mnLan->mnOpt.strEvent);

	CMFCRibbonButton* pLog = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Log));
	if (pLog != NULL)
		pLog->SetText(mnLan->mnOpt.strLog);

	CMFCRibbonButton* pWave = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Wave));
	if (pWave != NULL)
		pWave->SetText(mnLan->mnOpt.strWave);

	CMFCRibbonButton* pInfo = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Info));
	if (pInfo != NULL)
		pInfo->SetText(mnLan->mnOpt.strInfo);

	CMFCRibbonButton* pLan = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Lan));
	if (pLan != NULL)
		pLan->SetText(mnLan->mnOpt.strLan);

	pLan->GetParentCategory()->SetName(mnLan->mnOpt.strMenuName);

	//Menu Wave
	CMFCRibbonButton* pStart = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_StartDemo));
	if (pStart != NULL)
		pStart->SetText(mnLan->mnWave.strStart);

	CMFCRibbonButton* pStop = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_StopDemo));
	if (pStop != NULL)
		pStop->SetText(mnLan->mnWave.strStop);

	CMFCRibbonButton* pRec = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Recording));
	if (pRec != NULL)
		pRec->SetText(mnLan->mnWave.strRecord);

	CMFCRibbonButton* pPhotic = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Photic));
	if (pPhotic != NULL)
		pPhotic->SetText(mnLan->mnWave.strPhotic);

	CMFCRibbonComboBox* pSen = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_ScaleRate));
	if (pSen != NULL)
		pSen->SetText(mnLan->mnWave.strSensi);

	CMFCRibbonComboBox* pSpeed = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_SpeedRate));
	if (pSpeed != NULL)
		pSpeed->SetText(mnLan->mnWave.strPaperSpeed);

	CMFCRibbonComboBox* pListMon = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_MonList));
	if (pListMon != NULL)
		pListMon->SetText(mnLan->mnWave.strListMon);

	CMFCRibbonComboBox* pLP = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_LP));
	if (pLP != NULL)
		pLP->SetText(mnLan->mnWave.strLPF);

	CMFCRibbonComboBox* pHP = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_HP));
	if (pHP != NULL)
		pHP->SetText(mnLan->mnWave.strHPF);

	pHP->GetParentCategory()->SetName(mnLan->mnWave.strMenuName);

	//Menu Help

	CMFCRibbonButton* pAbout = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_About));
	if (pAbout != NULL)
		pAbout->SetText(mnLan->mnHelp.strAbout);

	pAbout->GetParentCategory()->SetName(mnLan->mnHelp.strMenuName);

}


void CAmekaApp::OnRecording()
{
	if (theApp.docList.IsEmpty())
		return;

	// TODO: Add your command handler code here.
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonButton* pRec = DYNAMIC_DOWNCAST(
		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_Recording));
	if (!pRec)
		return;

	if (pDoc)
	{
		if (!pDoc->isRecord)
		{
			pDoc->isRecord = TRUE;
		}
	}

	//if ((theApp.pIO != NULL) && (theApp.pIO->m_bState == S_CONNECTED))
	{
		CAmekaView *pView = CAmekaView::GetView();
		pView->isDrawRec = FALSE;
		CRect rect;
		pView->GetClientRect(&rect);
		CSize sizeTotal;
		// TODO: calculate the total size of this view

		sizeTotal.cx = rect.Width();
		sizeTotal.cy = rect.Height();
		pView->SetScrollSizes(MM_TEXT, sizeTotal);
		if (!pView->isRunning)
		{
			initial_dsp_data((LPVOID)pDoc);
			pDoc->isRecord = TRUE;
			pDoc->m_dspProcess = AfxBeginThread(DSP::DSPThread, (LPVOID)pDoc);
			pView->resetData();
			pView->OnDraw(pView->GetDC());
			pView->pThread = AfxBeginThread(pView->graphHandle, (LPVOID)pView);
			pView->isRunning = true;
		}
	}
}


//void CAmekaApp::OnStoprec()
//{
//	// TODO: Add your command handler code here
//	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
//	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
//	CMFCRibbonButton* pStopRec = DYNAMIC_DOWNCAST(
//		CMFCRibbonButton, pMainWnd->m_wndRibbonBar.FindByID(MN_StopRec));
//	if (!pStopRec)
//		return;
//
//	if (pDoc)
//	{
//		if (pDoc->isRecord)
//		{
//			pDoc->isRecord = FALSE;
//		}
//	}
//}


void CAmekaApp::SetLandscape(void)
{
	// Get default printer settings.
	PRINTDLG   pd;

	pd.lStructSize = (DWORD) sizeof(PRINTDLG);
	if (GetPrinterDeviceDefaults(&pd))
	{
		// Lock memory handle.
		DEVMODE FAR* pDevMode =
			(DEVMODE FAR*)::GlobalLock(m_hDevMode);
		LPDEVNAMES lpDevNames;
		LPTSTR lpszDriverName, lpszDeviceName, lpszPortName;
		HANDLE hPrinter;


		if (pDevMode)
		{
			// Change printer settings in here.
			pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
			// Unlock memory handle.
			lpDevNames = (LPDEVNAMES)GlobalLock(pd.hDevNames);
			lpszDriverName = (LPTSTR )lpDevNames + lpDevNames->wDriverOffset;
			lpszDeviceName = (LPTSTR )lpDevNames + lpDevNames->wDeviceOffset;
			lpszPortName   = (LPTSTR )lpDevNames + lpDevNames->wOutputOffset;

			::OpenPrinter(lpszDeviceName, &hPrinter, NULL);
			::DocumentProperties(NULL,hPrinter,lpszDeviceName,pDevMode,
				pDevMode, DM_IN_BUFFER|DM_OUT_BUFFER);

			// Sync the pDevMode.
			// See SDK help for DocumentProperties for more info.
			::ClosePrinter(hPrinter);
			::GlobalUnlock(m_hDevNames);
			::GlobalUnlock(m_hDevMode);
		}
	}
}

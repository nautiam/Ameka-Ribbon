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
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Ameka.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "AmekaDoc.h"
#include "AmekaView.h"

#include "SerialCtrl.h"
#include "AmekaUserConfig.h"
#include "afxcmn.h"
#include "easylogging++.h"
#include "DSPModule.h"
#include <vector>
#include "afxwin.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "GraphModule.h"
#include "AmekaLan.h"
#include "afxcolorbutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define strSen "10 15 30 60 90 120"
#define strSpeed "15 30 75 150 200 300"
#define strLP "15 20 30 40 50 60 70"
#define strHP "1 2 3 5 8"
#define strCOM "COM1 COM2 COM3 COM4 COM5 COM6 COM7 COM8 COM9 COM10"
#define strBaud "9600 14400 19200 38400 56000 115200 "
#define xmlName "listmontage.xml"
#define settingName "config.xml"

#define monXScale 300
#define monYScale 300
#define POINT_RAD 5

_INITIALIZE_EASYLOGGINGPP

using namespace std;

// CAmekaApp

BEGIN_MESSAGE_MAP(CAmekaApp, CWinAppEx)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)

	ON_COMMAND(MN_About, &CAmekaApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(MN_New, &CWinAppEx::OnFileNew)
	ON_COMMAND(MN_Open, &CWinAppEx::OnFileOpen)
	ON_COMMAND(MN_Close, &CAmekaApp::OnFileClose)
	ON_COMMAND(MN_Print, &CWinAppEx::OnFilePrintSetup)
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
	//init electrode name
	const char* setFileName = settingName;
	//writeSetting(setFileName);
	loadSetting(setFileName);
	//init Language
	mnLan = new amekaLan();
	//init buffer
	dataBuffer = new amekaData<RawDataType>(4096);
	//dataBuf = new AmekaData<RawDataType>(4096);

	//initialize log
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	//defaultConf.setGlobally(el::ConfigurationType::Filename, "logs\\Log.txt");
	defaultConf.setGlobally(el::ConfigurationType::LogFlushThreshold, "1000");
	defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
	el::Loggers::reconfigureLogger("default", defaultConf);
	LOG(INFO) << "Log using default file";

	//initialize montage list

	TiXmlDocument doc;
	if(!doc.LoadFile(xmlName))
	{
		LOG(ERROR) << doc.ErrorDesc();
		return;
	}
	TiXmlElement* root = doc.FirstChildElement();
	if(root == NULL)
	{
		LOG(ERROR) << "Failed to load file: No root element.";
		doc.Clear();
		return;
	}
	for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		LPAmontage mon = new Amontage();
		mon->mName = elem->Value();
		for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
		{
			CString attr1, attr2;
			attr1 = e->Attribute("channel1");
			attr2 = e->Attribute("channel2");
			if (attr1 != "" && attr2 != "")
			{
				LPAlead lead = new Alead();
				lead->lFirstID = atoi((LPCSTR)(CStringA)attr1);
				lead->lSecondID = atoi((LPCSTR)(CStringA)attr2);
				mon->mList.AddTail(lead);
				//delete lead;
			}
		}
		//delete mon;
		monList.AddTail(mon);
	}
	doc.Clear();
	//init ribbon
	m_sensitivity = strSen;
	m_speed = strSpeed;
	m_LP = strLP;
	m_HP = strHP;

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
	// If the application is built using Common Language Runtime support (/clr):
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
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
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
	if (theApp.pIO)
		theApp.pIO->~CSerialIO();
	delete dataBuffer;
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

//------------------------------------------------------------------//
// CInfoDlg
//------------------------------------------------------------------//

class CInfoDlg : public CDialogEx
{
public:
	CInfoDlg();

// Dialog Data
	enum { IDD = DLG_Info };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedok();
	afx_msg void OnBnClickedcancel();
	CComboBox info_sex;
	CButton info_handed;
};

CInfoDlg::CInfoDlg() : CDialogEx(CInfoDlg::IDD)
{
}

int CInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	info_sex.SetCurSel(0);
	return 0;
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, info_sex);
	DDX_Control(pDX, IDC_CHECK1, info_handed);
}

BEGIN_MESSAGE_MAP(CInfoDlg, CDialogEx)
	ON_BN_CLICKED(info_ok, &CInfoDlg::OnBnClickedok)
	ON_BN_CLICKED(info_cancel, &CInfoDlg::OnBnClickedcancel)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnInfo()
{
	CInfoDlg infoDlg;
	infoDlg.DoModal();
}


//------------------------------------------------------------------//
// CPhoticDlg
//------------------------------------------------------------------//

class CPhoticDlg : public CDialogEx
{
public:
	CPhoticDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = DLG_Photic };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit photicMin;
	CEdit photicMax;
	CEdit photicTick;
	CEdit photicName1;
	CEdit photicName2;
	CEdit photicName3;
	CEdit photicName4;
	CEdit photicName5;
	CEdit photicWidth;
	CMFCColorButton photicColor;
};

int CPhoticDlg::OnInitDialog()
{
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!pDoc)
		return -1;
	float min = theApp.photicMin;
	float max = theApp.photicMax;
	float width = pDoc->mDSP.epocLength;
	float tick = theApp.photicTick;
	CDialog::OnInitDialog();
	CString t;

	t.Format(_T("%.1f"), min);
	this->photicMin.SetWindowTextW(t);
	t.Format(_T("%.1f"), max);
	this->photicMax.SetWindowTextW(t);
	t.Format(_T("%.1f"), tick);
	this->photicTick.SetWindowTextW(t);
	t.Format(_T("%.1f"), width);
	this->photicWidth.SetWindowTextW(t);
	this->photicName1.SetWindowTextW(L"Alpha");
	this->photicName2.SetWindowTextW(L"Beta");
	this->photicName3.SetWindowTextW(L"Theta");

	return TRUE;
}

CPhoticDlg::CPhoticDlg() : CDialogEx(CPhoticDlg::IDD)
{
}

void CPhoticDlg::DoDataExchange(CDataExchange* pDX)
{
	CAmekaDoc* pDoc = CAmekaDoc::GetDoc();
	if (!pDoc)
		return;

	DDX_Control(pDX, photic_min, photicMin);
	DDX_Control(pDX, photic_max, photicMax);
	DDX_Control(pDX, photic_tick, photicTick);
	DDX_Control(pDX, IDC_EDIT4, photicName1);
	DDX_Control(pDX, IDC_EDIT11, photicName2);
	DDX_Control(pDX, IDC_EDIT5, photicName3);
	DDX_Control(pDX, IDC_EDIT12, photicName4);
	DDX_Control(pDX, IDC_EDIT13, photicName5);
	DDX_Control(pDX, photic_Width, photicWidth);
	DDX_Control(pDX, photic_Color, photicColor);
	CString tmp;
	DDX_Text(pDX, photic_max, tmp);
	if (tmp != "")
		theApp.photicMax = _ttof(tmp);

	DDX_Text(pDX, photic_min, tmp);
	if (tmp != "")
		theApp.photicMin = _ttof(tmp);

	DDX_Text(pDX, photic_tick, tmp);
	if (tmp != "")
		theApp.photicTick = _ttof(tmp);

	DDX_Text(pDX, photic_Width, tmp);
	if (tmp != "")
		pDoc->mDSP.epocLength = _ttof(tmp);

	//CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPhoticDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPhoticDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPhoticDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

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
		pView->onPhotic = ~pView->onPhotic;
		if (pView->onPhotic)
		{			
			CAmekaView *pView = CAmekaView::GetView();
			if (pView->isRunning)
			{
				pView->pPhoticThread = AfxBeginThread(pView->photicHandle, (LPVOID)pView);
			}
		}
		else
		{
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
	}
	pView->OnDraw(pView->GetDC());
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
	MessageBoxA(NULL,"This function still is not support","Warning",0);
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
//------------------------------------------------------------------//
// CEventDlg
//------------------------------------------------------------------//

class CEventDlg : public CDialogEx
{
public:
	CEventDlg();

// Dialog Data
	enum { IDD = DLG_Event};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedok();
	afx_msg void OnBnClickedcancel();
	CComboBox event_list;
};

CEventDlg::CEventDlg() : CDialogEx(CEventDlg::IDD)
{
}

int CEventDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	event_list.SetCurSel(0);
	return 0;
}

void CEventDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, evt_list, event_list);
}

BEGIN_MESSAGE_MAP(CEventDlg, CDialogEx)
	ON_BN_CLICKED(evt_ok, &CEventDlg::OnBnClickedok)
	ON_BN_CLICKED(evt_cancel, &CEventDlg::OnBnClickedcancel)
END_MESSAGE_MAP()

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
	{
		CAmekaView *pView = CAmekaView::GetView();
		if (!pView->isRunning)
		{
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
}

//------------------------------------------------------------------//


//------------------------------------------------------------------//


vector<string> Tokenize(CString buf, string delimiters = " ")
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


//------------------------------------------------------------------//
// CMontageDlg
//------------------------------------------------------------------//

class CMontageDlg : public CDialogEx
{
public:
	CMontageDlg();

// Dialog Data
	enum { IDD = DLG_Montage};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedadd();
	CComboBox mon_l1;
	CComboBox mon_l2;
	CListBox mon_list;
	CComboBox mon_lName;
	LPAmontage crtMon;
	afx_msg void OnBnClickedMonsave();
	afx_msg void OnMonListSelChange();
	void DrawMontage(CDC *dc, LPAmontage mMon);
	afx_msg void OnPaint();
	afx_msg void OnBnClickeddel();
};

CMontageDlg::CMontageDlg() : CDialogEx(CMontageDlg::IDD)
{
}

void CMontageDlg::DrawMontage(CDC* dc, LPAmontage mMon)
{
	CDC memDC;
	CBitmap bmp;
	memDC.CreateCompatibleDC(dc);
	bmp.CreateCompatibleBitmap(dc, monXScale + 20, monYScale);
	memDC.SelectObject(bmp);
	Graphics graphics(memDC.m_hDC);
	graphics.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);

	//Draw montage area
	SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
	Pen blackPen(Gdiplus::Color::Black);
	graphics.FillRectangle(&whiteBrush, 0, 0, monXScale + 20 - 1, monYScale - 1);
	
	graphics.DrawEllipse(&blackPen, 10, 0, monXScale - 1, monYScale - 1);

	//draw elec point
	for (int i = 0; i < theApp.elecNum; i++)
	{
		SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
		graphics.FillEllipse(&blackBrush, theApp.mElec[i].ePos->x - POINT_RAD, theApp.mElec[i].ePos->y - POINT_RAD, 2 * POINT_RAD, 2 * POINT_RAD);

		// Initialize arguments.
		Gdiplus::Font myFont(L"Arial", 8);
		PointF origin(theApp.mElec[i].ePos->x + 2, theApp.mElec[i].ePos->y - 2);
		graphics.DrawString(theApp.mElec[i].eName, theApp.mElec[i].eName.GetLength(),
			&myFont,
			origin,
			&blackBrush);
	}

	POSITION pos =  crtMon->mList.GetHeadPosition();
	while(pos) 
	{
		LPAlead curr = crtMon->mList.GetNext(pos);
		CPoint* p1 = getElecPoint(curr->lFirstID);
		CPoint* p2 = getElecPoint(curr->lSecondID);
		if (p1 != NULL && p2 != NULL)
		{			
			AdjustableArrowCap arrowCap(7.0, 4.0, TRUE);
			blackPen.SetCustomEndCap(&arrowCap);

			graphics.DrawLine(&blackPen, p1->x, p1->y, p2->x, p2->y);
		}
	}
	dc->BitBlt(220, 35, monXScale + 20, monYScale, &memDC, 0, 0, SRCCOPY);
	return;
}

void CMontageDlg::OnPaint()
{
	CPaintDC dc(this);

		// TODO: Add your message handler code here
		// Do not call CDialogEx::OnPaint() for painting messages

	DrawMontage(&dc, crtMon);
}


int CMontageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	BOOL flag = FALSE;

	mon_l1.SetCurSel(0);
	mon_l2.SetCurSel(0);
	mon_lName.SetCurSel(0);
	CAmekaDoc* doc = CAmekaDoc::GetDoc();
	mon_list.ResetContent();
	mon_lName.ResetContent();
	int count = 0;
	crtMon = new Amontage();
	POSITION pos =  theApp.monList.GetHeadPosition();
	for (int i = 0; i < theApp.monList.GetCount(); i++)
	{
		LPAmontage mon =  theApp.monList.GetNext( pos );
		
		POSITION pos1 =  mon->mList.GetHeadPosition();
		mon_lName.AddString(mon->mName);
		if (flag != TRUE)
			memcpy(crtMon,mon,sizeof(Amontage));
		if (doc != NULL && mon->mName == doc->mMon->mName)
			flag = TRUE;
	}
	mon_lName.SetWindowTextW(crtMon->mName);
	pos =  crtMon->mList.GetHeadPosition();
	for (int i = 0; i < crtMon->mList.GetCount(); i++)
	{
		LPAlead lead = crtMon->mList.GetNext( pos );
		CString tmp;
		tmp = getElecName(lead->lFirstID) + "   ->   " + getElecName(lead->lSecondID);
		mon_list.AddString(tmp);
	}

	for(int i = 0; i < theApp.elecNum; i++)
	{
		mon_l1.AddString(theApp.mElec[i].eName);
		mon_l2.AddString(theApp.mElec[i].eName);
	}
	mon_l1.SetCurSel(0);
	mon_l2.SetCurSel(0);
	return 0;
}

void CMontageDlg::OnMonListSelChange()
{
	// TODO: Add your control notification handler code here
	mon_list.ResetContent();
	CString tmp;
	mon_lName.GetLBText(mon_lName.GetCurSel(), tmp);
	POSITION pos =  theApp.monList.GetHeadPosition();
	for (int i = 0; i < theApp.monList.GetCount(); i++)
	{
		LPAmontage mon =  theApp.monList.GetNext( pos );
		if (tmp == mon->mName)
		{
			POSITION pos1 =  mon->mList.GetHeadPosition();
			for (int j = 0; j < mon->mList.GetCount(); j++)
			{
				LPAlead lead = mon->mList.GetNext( pos1 );
				CString tmp;
				tmp = getElecName(lead->lFirstID) + "   ->   " + getElecName(lead->lSecondID);
				mon_list.AddString(tmp);
			}
			crtMon = mon;
			this->OnPaint();
			this->Invalidate();
			this->UpdateWindow();
			return;
		}
	}
}

void CMontageDlg::OnBnClickedadd()
{
	// TODO: Add your control notification handler code here
	int pos1 = mon_l1.GetCurSel();
	int pos2 = mon_l2.GetCurSel();
	mon_list.AddString(getElecName(pos1+1) + "   ->   " + getElecName(pos2+1));
	LPAlead node = new Alead;
	node->lFirstID = pos1 + 1;
	node->lSecondID = pos2 + 1;

	//crtMon->mName = tmp;
	crtMon->mList.AddTail(node);
	//theApp.monList.AddTail(crtMon);
	//mon_lName.AddString(tmp);
	this->OnPaint();
	this->Invalidate();
	this->UpdateWindow();
}

void CMontageDlg::OnBnClickeddel()
{
	// TODO: Add your control notification handler code here
	int crtPos = mon_list.GetCurSel();
	CString data;
	mon_list.GetText(crtPos, data);
	vector<string> vecStr = Tokenize(data,"   ->   ");
	vector<string>::iterator it = vecStr.begin();
	CString cs1((*it).c_str());
	it++;
	CString cs2((*it).c_str());

	POSITION pos =  crtMon->mList.GetHeadPosition();
	POSITION savePos;
	while(pos) 
	{ 
		savePos = pos; 
		LPAlead curr = crtMon->mList.GetNext(pos); 
		if (curr->lFirstID == getElecID(cs1) && curr->lSecondID == getElecID(cs2))
		{
			crtMon->mList.RemoveAt(savePos);
			mon_list.DeleteString(crtPos);
			this->OnPaint();
			this->Invalidate();
			this->UpdateWindow();
			return;
		}
	}
}

void CMontageDlg::OnBnClickedMonsave()
{
	// TODO: Add your control notification handler code here
	CString tmp;
	mon_lName.GetWindowText(tmp);

	crtMon->mName = tmp;

	POSITION pos =  theApp.monList.GetHeadPosition();
	POSITION savePos;
	while(pos) 
	{ 
		savePos = pos; 
		LPAmontage curr = theApp.monList.GetNext(pos); 
		if (curr->mName == tmp)
		{
			theApp.monList.SetAt(savePos, crtMon); 
			return;
		}
	}
	theApp.monList.AddTail(crtMon);
	mon_lName.AddString(crtMon->mName);
}

void CMontageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, mon_1, mon_l1);
	DDX_Control(pDX, mon_2, mon_l2);
	DDX_Control(pDX, IDC_LIST3, mon_list);
	DDX_Control(pDX, mon_name, mon_lName);
}

BEGIN_MESSAGE_MAP(CMontageDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMontageDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMontageDlg::OnBnClickedCancel)
	ON_BN_CLICKED(mon_add, &CMontageDlg::OnBnClickedadd)
	ON_BN_CLICKED(mon_save, &CMontageDlg::OnBnClickedMonsave)
	ON_CBN_SELCHANGE(mon_name, &CMontageDlg::OnMonListSelChange)
	ON_WM_PAINT()
	ON_BN_CLICKED(mon_del, &CMontageDlg::OnBnClickeddel)
END_MESSAGE_MAP()
//Show Montage Dialog
void CAmekaApp::OnMontage()
{
	CMontageDlg montageDlg;
	montageDlg.DoModal();
}


//------------------------------------------------------------------//
//Tab Dialog

//------------------------------------------------------------------//
// COM Tab Dialog

vector<string> Tokenize(CString str, string delimiters);
CString itoS ( int x );

class CTabCOMDlg : public CDialogEx
{
public:
	CTabCOMDlg();
// Dialog Data
	enum { IDD = DLG_Opt_COM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox port_name;
	CComboBox port_baud;
};

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

class CTabRecDlg : public CDialogEx
{
public:
	CTabRecDlg();
// Dialog Data
	enum { IDD = DLG_Opt_Rec };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedeeg();
	CEdit rec_ed_eeg;
	CEdit rec_ed_video;
	afx_msg void OnBnClickedvideo();
};

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

BEGIN_MESSAGE_MAP(CTabRecDlg, CDialogEx)
	ON_BN_CLICKED(rec_eeg, &CTabRecDlg::OnBnClickedeeg)
	ON_BN_CLICKED(rec_video, &CTabRecDlg::OnBnClickedvideo)
END_MESSAGE_MAP()

//------------------------------------------------------------------//
// Event tab dialog
//------------------------------------------------------------------//

class CTabEventDlg : public CDialogEx
{
public:
	CTabEventDlg();
// Dialog Data
	enum { IDD = DLG_Opt_Event };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:

};

CTabEventDlg::CTabEventDlg() : CDialogEx(CTabEventDlg::IDD)
{
	
}

int CTabEventDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return 0;
}

void CTabEventDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabEventDlg, CDialogEx)

END_MESSAGE_MAP()

//------------------------------------------------------------------//
// View tab dialog
//------------------------------------------------------------------//

class CTabViewDlg : public CDialogEx
{
public:
	CTabViewDlg();
// Dialog Data
	enum { IDD = DLG_Opt_View };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_view_speed;
	CEdit m_view_sen;
	CEdit m_view_lp;
	CEdit m_view_hp;
	afx_msg void OnBnClickedbtdef();
};

CTabViewDlg::CTabViewDlg() : CDialogEx(CTabViewDlg::IDD)
{
	
}

int CTabViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_view_sen.SetWindowTextW(theApp.m_sensitivity);
	m_view_speed.SetWindowTextW(theApp.m_speed);
	m_view_lp.SetWindowTextW(theApp.m_LP);
	m_view_hp.SetWindowTextW(theApp.m_HP);
	return 0;
}

void CTabViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, view_lp, theApp.m_LP);
	DDX_Text(pDX, view_hp, theApp.m_HP);
	DDX_Text(pDX, view_sensitivity, theApp.m_sensitivity);
	DDX_Text(pDX, view_speed, theApp.m_speed);
	DDX_Control(pDX, view_sensitivity, m_view_sen);
	DDX_Control(pDX, view_speed, m_view_speed);
	DDX_Control(pDX, view_lp, m_view_lp);
	DDX_Control(pDX, view_hp, m_view_hp);
}

BEGIN_MESSAGE_MAP(CTabViewDlg, CDialogEx)
	ON_BN_CLICKED(view_btdef, &CTabViewDlg::OnBnClickedbtdef)
END_MESSAGE_MAP()

//------------------------------------------------------------------//
// COptionDlg
//------------------------------------------------------------------//

class COptionDlg : public CDialogEx
{
public:
	COptionDlg();
	CString m_baudRate;
	CString m_portNo;
	CString m_LP;
	CString m_HP;
	CString m_sensitivity;
	CString m_speed;

	CDialog* mDlg[4];

	int mPrePos;

// Dialog Data
	enum { IDD = DLG_Option };
	CTabCtrl tab_ctrl;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnOK();
	afx_msg void OnTabSel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedcancel();
	afx_msg void OnBnClickedok();
};

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
    tab_ctrl.InsertItem(0,L"Tables");
	tab_ctrl.InsertItem(1,L"Events");
	tab_ctrl.InsertItem(2,L"Recording");

	mDlg[0] = new CTabViewDlg;
	mDlg[1] = new CTabEventDlg;
	mDlg[2] = new CTabRecDlg;

	mDlg[0]->Create(DLG_Opt_View, &tab_ctrl);
	mDlg[1]->Create(DLG_Opt_Event, &tab_ctrl);
	mDlg[2]->Create(DLG_Opt_Rec, &tab_ctrl);

	CRect TabRect; 
	tab_ctrl.GetClientRect(&TabRect);
	tab_ctrl.AdjustRect(FALSE, &TabRect);
	mDlg[0]->MoveWindow(TabRect);
	mDlg[1]->MoveWindow(TabRect);
	mDlg[2]->MoveWindow(TabRect);

    mDlg[0]->ShowWindow(true);
	mDlg[1]->ShowWindow(false);
	mDlg[2]->ShowWindow(false);
    tab_ctrl.SetCurSel(0);
	mPrePos = 0;

	return 0;
}

BEGIN_MESSAGE_MAP(COptionDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &COptionDlg::OnOK)
	ON_NOTIFY(TCN_SELCHANGE, opt_tab, &COptionDlg::OnTabSel)
	ON_BN_CLICKED(opt_cancel, &COptionDlg::OnBnClickedcancel)
	ON_BN_CLICKED(opt_ok, &COptionDlg::OnBnClickedok)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnOption()
{
	COptionDlg optionDlg;
	optionDlg.DoModal();
}


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


void CMontageDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CAmekaDoc* crtdoc = CAmekaDoc::GetDoc();
	crtdoc->mMon = crtMon;
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	CMFCRibbonComboBox* pMon = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_MonList));
	if (pMon != NULL)
		pMon->SetEditText(crtdoc->mMon->mName);
		
	TiXmlDocument doc;
	TiXmlElement* root = new TiXmlElement("root");
	doc.LinkEndChild(root);

	POSITION pos =  theApp.monList.GetHeadPosition();
	while (pos)
	{
		LPAmontage mon =  theApp.monList.GetNext( pos );
		TiXmlElement* element = new TiXmlElement((LPCSTR)(CStringA)mon->mName);
		root->LinkEndChild(element);
		POSITION pos1 =  mon->mList.GetHeadPosition();
		while (pos1)
		{
			LPAlead lead = mon->mList.GetNext( pos1 );
			TiXmlElement* element1 = new TiXmlElement((LPCSTR)(CStringA)mon->mName);
			element->LinkEndChild(element1);
			element1->SetAttribute("channel1", lead->lFirstID);
			element1->SetAttribute("channel2", lead->lSecondID);
		}
	}

	bool success = doc.SaveFile(xmlName);
	doc.Clear();
	CDialogEx::OnOK();
}


void CMontageDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
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


void CInfoDlg::OnBnClickedok()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
	CAmekaView *pView = CAmekaView::GetView();
	if (pView)
		pView->OnDraw(pView->GetDC());
}


void CInfoDlg::OnBnClickedcancel()
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


void CPhoticDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CPhoticDlg::OnBnClickedCancel()
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



void COptionDlg::OnBnClickedok()
{
	// TODO: Add your control notification handler code here
	for (int i = 0; i < 3; i++)
	{
		mDlg[i]->UpdateData();
	}
	CAmekaView* view = CAmekaView::GetView();
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
	//pLP->SetEditText(itoS(view->graphData.));
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

	EndDialog(0);
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


void CTabViewDlg::OnBnClickedbtdef()
{
	// TODO: Add your control notification handler code here
	m_view_sen.SetWindowTextW((LPCTSTR)strSen);
	m_view_speed.SetWindowTextW((LPCTSTR)strSpeed);
	m_view_lp.SetWindowTextW((LPCTSTR)strLP);
	m_view_hp.SetWindowTextW((LPCTSTR)strHP);
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
		if (pIO->m_bState == S_CONNECTED || pIO->m_bState == S_NOCONNECTED)
		{
			pPortOpen->SetText(L"Đóng cổng");
		}
		else
		{
			delete pIO;
			pIO = NULL;
		}
	}
	else
	{
		pPortOpen->SetText(L"Mở cổng");
		delete pIO;
		pIO = NULL;
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
	CFileDialog fOpenDlg(TRUE, L".txt", NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, L"Ameka Language Files (*.xml)|*.xml||");

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


 
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


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAmekaApp

BEGIN_MESSAGE_MAP(CAmekaApp, CWinAppEx)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)

	ON_COMMAND(MN_About, &CAmekaApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(MN_New, &CWinAppEx::OnFileNew)
	ON_COMMAND(MN_Open, &CWinAppEx::OnFileOpen)
	ON_COMMAND(MN_Setting, &CAmekaApp::OnSetting)
	ON_COMMAND(MN_Info, &CAmekaApp::OnInfo)
	ON_COMMAND(MN_Option, &CAmekaApp::OnOption)
	ON_COMMAND(MN_Photic, &CAmekaApp::OnPhotic)
	ON_COMMAND(MN_Log, &CAmekaApp::OnLog)
	ON_COMMAND(MN_Wave, &CAmekaApp::OnWave)
	ON_COMMAND(MN_Event, &CAmekaApp::OnEvent)
	ON_COMMAND(MN_StartDemo, &CAmekaApp::OnDemo)
	ON_COMMAND(MN_StopDemo, &CAmekaApp::OnStop)
	ON_COMMAND(MN_OpenPort, &CAmekaApp::OnPortOpen)
END_MESSAGE_MAP()


// CAmekaApp construction

CAmekaApp::CAmekaApp()
{
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
	AfxOleTerm(FALSE);
	theApp.pIO->~CSerialIO();
	return CWinAppEx::ExitInstance();
}

// CAmekaApp message handlers


// CAboutDlg dialog used for App About

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
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnSetting()
{
	CSettingDlg settingDlg;
	settingDlg.DoModal();
}

//------------------------------------------------------------------//
// CInfoDlg

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
};

CInfoDlg::CInfoDlg() : CDialogEx(CInfoDlg::IDD)
{
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInfoDlg, CDialogEx)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnInfo()
{
	CInfoDlg infoDlg;
	infoDlg.DoModal();
}

//------------------------------------------------------------------//
// COptionDlg

class COptionDlg : public CDialogEx
{
public:
	COptionDlg();
	CString m_baudRate;
	CString m_portNo;

// Dialog Data
	enum { IDD = DLG_Option };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOK();
};

COptionDlg::COptionDlg() : CDialogEx(COptionDlg::IDD)
{
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, opt_com_portNo, m_portNo);
	DDX_Text(pDX, opt_com_baud, m_baudRate);

}

BEGIN_MESSAGE_MAP(COptionDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &COptionDlg::OnOK)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnOption()
{
	COptionDlg optionDlg;
	optionDlg.DoModal();
}

//------------------------------------------------------------------//
// CPhoticDlg
class CPhoticDlg : public CDialogEx
{
public:
	CPhoticDlg();

// Dialog Data
	enum { IDD = DLG_Photic };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CPhoticDlg::CPhoticDlg() : CDialogEx(CPhoticDlg::IDD)
{
}

void CPhoticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPhoticDlg, CDialogEx)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnPhotic()
{
	CPhoticDlg photicDlg;
	photicDlg.DoModal();
}

//------------------------------------------------------------------//
// CLogDlg
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
};

CLogDlg::CLogDlg() : CDialogEx(CLogDlg::IDD)
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialogEx)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnLog()
{
	CLogDlg logDlg;
	logDlg.DoModal();
}

//------------------------------------------------------------------//
// CWaveDlg
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
};

CWaveDlg::CWaveDlg() : CDialogEx(CWaveDlg::IDD)
{
}

void CWaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWaveDlg, CDialogEx)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnWave()
{
	CWaveDlg waveDlg;
	waveDlg.DoModal();
}

//------------------------------------------------------------------//
// CEventDlg
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
};

CEventDlg::CEventDlg() : CDialogEx(CEventDlg::IDD)
{
}

void CEventDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEventDlg, CDialogEx)
END_MESSAGE_MAP()

//Show Setting Dialog
void CAmekaApp::OnEvent()
{
	CEventDlg eventDlg;
	eventDlg.DoModal();
}

//------------------------------------------------------------------//

//Demo Graph
void CAmekaApp::OnDemo()
{
	CAmekaView *pView = CAmekaView::GetView();
	if (!pView->isRunning)
	{
		pView->pThread = AfxBeginThread(pView->graphHandle, (LPVOID)pView);
		pView->isRunning = true;
	}
}

void CAmekaApp::OnStop()
{
	CAmekaView *pView = CAmekaView::GetView();

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


//Show Setting Dialog
void CAmekaApp::OnPortOpen()
{
	//PortOpen portDlg;
	//portDlg.DoModal();
	CString portFullName = "\\\.\\" + m_portNo;
	//MessageBox("Opening Port " + portFullName,"Info",0);
	pIO = new CSerialIO(portFullName, m_baudRate);
}

//------------------------------------------------------------------//


void COptionDlg::OnOK()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();

	theApp.m_portNo = m_portNo;
	theApp.m_baudRate = m_baudRate;
	remove(settingFileName);
	ofstream file;
	file.open(settingFileName);
	file << theApp.m_portNo;
	file << std::endl;
	file << theApp.m_baudRate;
	file << std::endl;
	file.close();
}





void CSettingDlg::OnSetup()
{
	// TODO: Add your command handler code here
}


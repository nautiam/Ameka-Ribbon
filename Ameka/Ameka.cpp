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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define strSen "10 15 30 60 90 120"
#define strSpeed "15 30 75 150 200 300"
#define strLP "15 20 30 40 50 60 70"
#define strHP "1 2 3 5 8"
#define strCOM "COM1 COM2 COM3 COM4 COM5 COM6 COM7 COM8 COM9 COM10"
#define strBaud "9600 14400 19200 38400 56000 115200 "
#define xmlName "abc.xml"

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
	ON_COMMAND(MN_PortOpen, &CAmekaApp::OnPortOpen)
	ON_COMMAND(MN_Scan, &CAmekaApp::OnScan)
END_MESSAGE_MAP()

//-------------------------------------------------------//

// CAmekaApp construction

CAmekaApp::CAmekaApp()
{
	//init buffer
	dataBuffer = new amekaData<RawDataType>(4096);

	//initialize log
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	//defaultConf.setGlobally(el::ConfigurationType::Filename, "logs\\Log.txt");
	defaultConf.setGlobally(el::ConfigurationType::LogFlushThreshold, "1000");
	defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");
	el::Loggers::reconfigureLogger("default", defaultConf);
	LOG(INFO) << "Log using default file";

	//initialize montage list
	
	
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
};

CPhoticDlg::CPhoticDlg() : CDialogEx(CPhoticDlg::IDD)
{
}

void CPhoticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
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
	CWaveDlg waveDlg;
	waveDlg.DoModal();
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

//Demo Graph
void CAmekaApp::OnDemo()
{
	if (theApp.pIO != NULL && (theApp.pIO->m_bState == S_CONNECTED || theApp.pIO->m_bState == S_NOCONNECTED))
	{
		CAmekaView *pView = CAmekaView::GetView();
		if (!pView->isRunning)
		{
			pView->pThread = AfxBeginThread(pView->graphHandle, (LPVOID)pView);
		}	pView->isRunning = true;
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


//------------------------------------------------------------------//


vector<string> Tokenize(CString buf, string delimiters = " ")
{   
    vector<string> tokens;
	std::string str = buf.GetBuffer(buf.GetLength());
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
CString itoS ( int x)
{
	CString sout;
	sout.Format("%i", x);

	return sout;
}
//------------------------------------------------------------------//

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
	virtual int OnPaint();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedadd();
	CComboBox mon_l1;
	CComboBox mon_l2;
	CListBox mon_list;
	CComboBox mon_lName;
	afx_msg void OnBnClickedMonsave();
	afx_msg void OnBnClickedload();
};

CMontageDlg::CMontageDlg() : CDialogEx(CMontageDlg::IDD)
{
}

int CMontageDlg::OnPaint()
{
	CPaintDC dc(this);

	CWnd* pImage = GetDlgItem(mon_pic);
    CRect rc;
    pImage->GetWindowRect(rc);
    HRGN hRgn = CreateRoundRectRgn(0, 0, rc.Width(), rc.Height(), 40, 40);
    HINSTANCE hIns = AfxGetInstanceHandle();
	HBITMAP hBmp = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
    HBRUSH hBr = CreatePatternBrush(hBmp); 
	dc.Rectangle(0, 0, 4, 4);
    DeleteObject(hIns);
    DeleteObject(hBmp);
    FillRgn(pImage->GetDC()->GetSafeHdc(), hRgn, hBr);
	CDialog::OnPaint();

	return 0;
}

int CMontageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	mon_l1.SetCurSel(0);
	mon_l2.SetCurSel(0);
	mon_lName.SetCurSel(0);
	CAmekaDoc* doc = CAmekaDoc::GetDoc();
	mon_list.ResetContent();
	POSITION pos = doc->mMontage.mList.GetHeadPosition();
	for (int i = 0; i < doc->mMontage.mList.GetCount(); i++)
	{
	    LPAlead lead = doc->mMontage.mList.GetNext( pos );
		CString tmp;
		tmp = itoS(lead->lFirstID) + " -> " + itoS(lead->lSecondID);
		mon_list.AddString(tmp);
	}
	return 0;
}

void CMontageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, mon_1, mon_l1);
	DDX_Control(pDX, mon_2, mon_l2);
	DDX_Control(pDX, IDC_LIST3, mon_list);
	DDX_Control(pDX, IDC_COMBO1, mon_lName);
}

BEGIN_MESSAGE_MAP(CMontageDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMontageDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMontageDlg::OnBnClickedCancel)
	ON_BN_CLICKED(mon_add, &CMontageDlg::OnBnClickedadd)
	ON_BN_CLICKED(mon_save, &CMontageDlg::OnBnClickedMonsave)
	ON_BN_CLICKED(mon_load, &CMontageDlg::OnBnClickedload)
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
	TCHAR szDirectory[MAX_PATH] = "";
	::GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);

	rec_ed_eeg.SetWindowTextA(szDirectory);
	rec_ed_video.SetWindowTextA(szDirectory);

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
	m_view_sen.SetWindowTextA(theApp.m_sensitivity);
	m_view_speed.SetWindowTextA(theApp.m_speed);
	m_view_lp.SetWindowTextA(theApp.m_LP);
	m_view_hp.SetWindowTextA(theApp.m_HP);
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
    tab_ctrl.InsertItem(0,"View");
	tab_ctrl.InsertItem(1,"Event");
	tab_ctrl.InsertItem(2,"Recording");

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
		pSen->AddItem((*it).c_str(),count++);
	}
	pSen->SetEditText(itoS(view->graphData.scaleRate));
	//Set items for LowPassFilter
	count = 0;
	CMFCRibbonComboBox* pSpeed = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_SpeedRate));
	pSpeed->RemoveAllItems();
	vector<string> vecSpeed = Tokenize(theApp.m_speed," ");
	for (vector<string>::iterator it = vecSpeed.begin(); it != vecSpeed.end(); it++) {
		pSpeed->AddItem((*it).c_str(),count++);
	}
	pSpeed->SetEditText(itoS(view->graphData.paperSpeed));
	//Set items for LowPassFilter
	count = 0;
	CMFCRibbonComboBox* pLP = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_LP));
	pLP->RemoveAllItems();
	vector<string> vecLP = Tokenize(theApp.m_LP," ");
	for (vector<string>::iterator it = vecLP.begin(); it != vecLP.end(); it++) {
		pLP->AddItem((*it).c_str(),count++);
	}
	//pLP->SetEditText(itoS(view->graphData.));
	//Set items for LowPassFilter
	count = 0;
	CMFCRibbonComboBox* pHP = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_HP));
	pHP->RemoveAllItems();
	vector<string> vecHP = Tokenize(theApp.m_HP," ");
	for (vector<string>::iterator it = vecHP.begin(); it != vecHP.end(); it++) {
		pHP->AddItem((*it).c_str(),count++);
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
		rec_ed_eeg.SetWindowTextA(tmp);
	}
}


void CTabRecDlg::OnBnClickedvideo()
{
	// TODO: Add your control notification handler code here
	CFolderPickerDialog dlgFolder;

	if ( dlgFolder.DoModal() == IDOK )
	{
		CString tmp = dlgFolder.GetFolderPath();
		rec_ed_video.SetWindowTextA(tmp);
	}
}


void CTabViewDlg::OnBnClickedbtdef()
{
	// TODO: Add your control notification handler code here
	m_view_sen.SetWindowTextA(strSen);
	m_view_speed.SetWindowTextA(strSpeed);
	m_view_lp.SetWindowTextA(strLP);
	m_view_hp.SetWindowTextA(strHP);
}

void CMontageDlg::OnBnClickedadd()
{
	// TODO: Add your control notification handler code here
	int pos1 = mon_l1.GetCurSel();
	int pos2 = mon_l2.GetCurSel();
	mon_list.AddString(itoS(pos1+1) + " -> " + itoS(pos2+1));
	LPAlead node = new Alead;
	node->lFirstID = pos1 + 1;
	node->lSecondID = pos2 + 1;
	CAmekaDoc* doc = CAmekaDoc::GetDoc();
	doc->mMontage.mList.AddTail(node);
}

void CMontageDlg::OnBnClickedMonsave()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlgFolder(FALSE, CString(".xml"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, CString("XML Files (*.xml)|*.xml|"));
	CAmekaDoc* crtdoc = CAmekaDoc::GetDoc();
	if ( dlgFolder.DoModal() == IDOK )
	{
		TiXmlDocument doc;
		TiXmlElement* root = new TiXmlElement("root");
		doc.LinkEndChild(root);
		POSITION pos = crtdoc->mMontage.mList.GetHeadPosition();
		for (int i = 0; i < crtdoc->mMontage.mList.GetCount(); i++)
		{
			LPAlead lead = crtdoc->mMontage.mList.GetNext( pos );
			TiXmlElement* element = new TiXmlElement("Montage");
			root->LinkEndChild(element);
			element->SetAttribute("channel1", lead->lFirstID);
			element->SetAttribute("channel2", lead->lSecondID);

		}
		bool success = doc.SaveFile(dlgFolder.GetPathName());
		doc.Clear();
	}
}


void CMontageDlg::OnBnClickedload()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlgFolder(TRUE, CString(".xml"), NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, CString("XML Files (*.xml)|*.xml|"));
	CAmekaDoc* crtdoc = CAmekaDoc::GetDoc();
	if ( dlgFolder.DoModal() == IDOK )
	{
		TiXmlDocument doc(dlgFolder.GetFileName());
		uint16_t count = 0;

		if(!doc.LoadFile())
		{
	//		LOG(ERROR) << doc.ErrorDesc();
			return;
		}

		TiXmlElement* root = doc.FirstChildElement();
		crtdoc->mMontage.mList.RemoveAll();
		if(root == NULL)
		{
	//		LOG(ERROR) << "Failed to load file: No root element.";
			doc.Clear();
		}

		for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
		{
			CString elemName = elem->Value();
			const char* attr1;
			const char* attr2;
		
			if(elemName == "Montage")
			{
				attr1 = elem->Attribute("channel1");
				attr2 = elem->Attribute("channel2");
				if(attr1 != NULL && attr2 != NULL)
				{
					count++;
					LPAlead node = new Alead;
					node->lFirstID = atoi(attr1);
					node->lSecondID = atoi(attr2);
					crtdoc->mMontage.mList.AddTail(node);
					crtdoc->mMontage.leadNum++;
				}
			}
		}
	}
	mon_list.ResetContent();
	POSITION pos = crtdoc->mMontage.mList.GetHeadPosition();
	for (int i = 0; i < crtdoc->mMontage.mList.GetCount(); i++)
	{
	    LPAlead lead = crtdoc->mMontage.mList.GetNext( pos );
		CString tmp;
		tmp = itoS(lead->lFirstID) + " -> " + itoS(lead->lSecondID);
		mon_list.AddString(tmp);
	}  
}


void CAmekaApp::OnPortOpen()
{
	// TODO: Add your command handler code here
	CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
	CString tmp;
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
	CString portFullName = "\\\.\\" + m_portNo;
	//MessageBox("Opening Port " + portFullName,"Info",0);
	pIO = new CSerialIO(portFullName, m_baudRate);

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
        
			test = QueryDosDevice(ComName, (LPSTR)lpTargetPath, 5000);

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
			pPort->SetEditText("N.A"); // to display error message incase no ports 
		else
		{
			//pPort->SetEditText(pPort->GetItem(0));
			pPort->SelectItem(0);
		}
    }
}


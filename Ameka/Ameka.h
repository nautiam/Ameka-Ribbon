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

// Ameka.h : main header file for the Ameka application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "SerialCtrl.h"
#include <iostream>
#include <fstream>

#define portNo "COM2"
#define baudRate "115200"
#define settingFileName "AmekaSetting.amek"

using namespace std;


// CAmekaApp:
// See Ameka.cpp for the implementation of this class
//

class CAmekaApp : public CWinAppEx
{
public:
	CAmekaApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	CSerialIO* pIO;
	CString m_baudRate;
	CString m_portNo;

// Implementation
	UINT  m_nAppLook;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnSetting();		//Setting 
	afx_msg void OnInfo();			//Patient Summary
	afx_msg void OnOption();		//Option
	afx_msg void OnEvent();			//Event & Interval
	afx_msg void OnPhotic();		//Photic
	afx_msg void OnLog();			//Log View
	afx_msg void OnWave();			//Wave Generator
	afx_msg void OnDemo();			//Graph demo
	afx_msg void OnStop();			//Graph demo
	afx_msg void OnPortOpen();		//Open Serial COM Port
	afx_msg void OnMontage();		//Montage
	DECLARE_MESSAGE_MAP()
	afx_msg void OnScalerate();
};

extern CAmekaApp theApp;

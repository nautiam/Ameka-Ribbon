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
#ifndef _AMEKA_
#define _AMEKA_
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "SerialCtrl.h"
#include <iostream>
#include <fstream>

#include "AmekaView.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "AmekaLan.h"
#include "AmekaUserConfig.h"

#define portNo "COM2"
#define BAUD_RATE "115200"
#define settingFileName "AmekaSetting.conf"

#define strSen "10 15 30 60 90 120"
#define strSpeed "15 30 75 150 200 300"
#define strLP "10 15 20 30 40 50 60"
#define strHP "0.1 0.2 0.5 1 1.5"
#define strdotPmm "200"
#define strCOM "COM1 COM2 COM3 COM4 COM5 COM6 COM7 COM8 COM9 COM10"
#define strBaud "9600 14400 19200 38400 56000 115200 "
#define xmlName "listmontage.conf"
#define settingName "config.conf"

#define LEFT_MARGIN 20
#define RIGHT_MARGIN 20
#define TOP_MARGIN 20
#define BOT_MARGIN 20
#define DEF_FONT "Arial Greek"
#define DEF_SIZE 8
#define DEF_DISTANCE 8

using namespace std;


// CAmekaApp:
// See Ameka.cpp for the implementation of this class
//

vector<string> Tokenize(CString buf, string delimiters = " ");

class CAmekaApp : public CWinAppEx
{
public:
	CAmekaApp();
	volatile CSerialIO* pIO;
	CString m_baudRate;
	CString m_portNo;
	CString m_speed;
	CString m_sensitivity;
	CString m_LP;
	CString m_HP;
	CString m_dotPmm;
	CMyArray<Amontage, Amontage&> monList;
	CList<CAmekaDoc*, CAmekaDoc*> docList;
	amekaData<RawDataType>* dataBuffer;
	//AmekaData<RawDataType>* dataBuffer;
	amekaLan* mnLan;
	Aelectrode* mElec;
	uint16_t elecNum;
	CString evName[10];

	//draw setup
	float photicMin;
	float photicWRate;
	float photicMax;
	float photicTick;
	float photicBarW;

	//Print setup
	uint16_t marginLeft;
	uint16_t marginRight;
	uint16_t marginTop;
	uint16_t marginBot;
	CString printFont;
	uint16_t printSize;
	uint16_t printDistance;
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	ULONG_PTR m_gdiplusToken;
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
	afx_msg void OnMontage();		//Montage
	afx_msg void OnSetupPhotic();	//Setup photic
	DECLARE_MESSAGE_MAP()
	afx_msg void OnScalerate();
	afx_msg void OnPortOpen();
	afx_msg void OnScan();
	afx_msg void OnFileClose();
	afx_msg void OnLan();
	afx_msg void OnRecording();
//	afx_msg void OnStoprec();
	void SetLandscape(void);
};

extern CAmekaApp theApp;
#endif
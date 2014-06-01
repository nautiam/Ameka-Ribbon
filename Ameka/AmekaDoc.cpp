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

// AmekaDoc.cpp : implementation of the CAmekaDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Ameka.h"
#endif

#include "AmekaDoc.h"
#include "xmlParser.h"
#include "DSPModule.h"
#include "MainFrm.h"
#include "AmekaView.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAmekaDoc

IMPLEMENT_DYNCREATE(CAmekaDoc, CDocument)

BEGIN_MESSAGE_MAP(CAmekaDoc, CDocument)
	ON_COMMAND(ID_FILE_SEND_MAIL, &CAmekaDoc::OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, &CAmekaDoc::OnUpdateFileSendMail)
END_MESSAGE_MAP()


// CAmekaDoc construction/destruction
//UINT genData(LPVOID pParam)
//{
//	CAmekaDoc* pnt = (CAmekaDoc *)pParam;
//	while(1)
//	{
//		RawDataType data;
//		data.time = 0;
//		for (int i = 0; i < 15; i++)
//		{
//			data.value[i] = (13000+rand()%3000);
//		}
//		pnt->PrimaryData->pushData(data);
//		Sleep(3);
//	}
//	return 0;
//}
CAmekaDoc::CAmekaDoc()
{
	// TODO: add one-time construction code here
	dataBuffer = new amekaData<RawDataType>(BUFFER_LEN);
	PrimaryData = NULL;
	primaryDataArray = NULL;
	TemporaryData = new amekaData<PrimaryDataType>(BUFFER_LEN);
	SecondaryData = new amekaData<SecondaryDataType>(BUFFER_LEN);
	mDSP.HPFFre = 0.5;
	mDSP.LPFFre = 30;
	mDSP.SampleRate = SAMPLE_RATE;
	mDSP.epocLength = 1.6;
	isOpenFile = FALSE;
	isRecord = FALSE;
	isSave = FALSE;
	m_dspProcess = NULL;
	eventID = 0;

	onReadSuccess = CreateEvent( 
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("WriteEvent")  // object name
		); 
	if (onReadSuccess == NULL) 
    { 
        printf("CreateEvent failed (%d)\n", GetLastError());
        return;
    }
	//saveFileName = "temp.dat";
	//thrd = AfxBeginThread(genData, (LPVOID)this);	
}

CAmekaDoc::~CAmekaDoc()
{
	if (m_dspProcess)
	{
		DWORD exit_code = NULL;
		GetExitCodeThread(this->m_dspProcess->m_hThread, &exit_code);

		if(exit_code == STILL_ACTIVE)
		{
			::TerminateThread(this->m_dspProcess->m_hThread, 0);
			CloseHandle(this->m_dspProcess->m_hThread);
		}
		m_dspProcess = NULL;
	}

	POSITION pos = theApp.docList.Find(this);
	theApp.docList.RemoveAt(pos);
	if (isOpenFile == TRUE)
	{
		/*uint16_t buffer[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
		object.Write(buffer, sizeof(buffer));*/
		object.SeekToBegin();
		uint16_t temp[8];
		temp[0] = (uint16_t)(mDSP.HPFFre * 10);
		temp[1] = (uint16_t)(mDSP.LPFFre * 10);
		temp[2] = (uint16_t)(mDSP.epocLength * 10);
		temp[3] = mDSP.SampleRate;
		temp[4] = (uint16_t)(counter);
		temp[5] = (uint16_t)(counter >> 16);
		temp[6] = (uint16_t)(counter >> 32);
		temp[7] = (uint16_t)(counter >> 48);
		object.Write(temp, sizeof(temp));

		uint8_t nLen = _tcslen(mMon.mName);
		uint8_t temp_mon[66];
		uint8_t monNum =  mMon.mList.GetCount();
		temp_mon[64] = monNum;
		POSITION pos;
		if (monNum > 32)
			monNum = 32;
		for (int i=0; i<monNum; i++)
		{
			Alead temp;
			temp = mMon.mList.GetAt(i);
			uint8_t fID = temp.lFirstID;
			uint8_t sID = temp.lSecondID;
			temp_mon[i*2] = fID;
			temp_mon[i*2 + 1] = sID;
		}
		temp_mon[65] = nLen;
		object.Write(temp_mon, sizeof(temp_mon));
			
		char *szTo = new char[nLen + 1];
		WideCharToMultiByte(1258, 0, mMon.mName, nLen, szTo, nLen, NULL, NULL);			
		int size = sizeof(szTo);
		object.Write(szTo, (nLen + 1)*sizeof(char));
		delete szTo;
		object.Close();
		isOpenFile = FALSE;
	}
	delete dataBuffer;
	delete PrimaryData;
	delete SecondaryData;
	delete TemporaryData;
	// Try to remove old record file whatever it is created or not
	try
	{
		CFile::Remove(recordFileName);
	}
	catch (CFileException* pEx)
	{
		//AfxMessageBox(L"File cannot be removed");
		pEx->Delete();
	}
}

BOOL CAmekaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	dataBuffer = new amekaData<RawDataType>(BUFFER_LEN);
	theApp.docList.AddTail(this);

	//POSITION pos =  theApp.monList.GetHeadPosition();
	if (theApp.monList.GetSize() != 0)
	{
		Amontage mon =  theApp.monList.GetAt( 0 );
		mMon = mon;
		CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
		CMFCRibbonComboBox* pMon = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_MonList));
		if (pMon)
			pMon->SetEditText(mMon.mName);
	}

	return TRUE;
}

CAmekaDoc * CAmekaDoc::GetDoc()
{
    CMDIChildWnd * pChild =
        ((CMDIFrameWnd*)(AfxGetApp()->m_pMainWnd))->MDIGetActive();

    if ( !pChild )
        return NULL;

    CDocument * pDoc = pChild->GetActiveDocument();

    if ( !pDoc )
        return NULL;

    // Fail if doc is of wrong kind
    if ( ! pDoc->IsKindOf( RUNTIME_CLASS(CAmekaDoc) ) )
        return NULL;

    return (CAmekaDoc *) pDoc;
}


// CAmekaDoc serialization

void CAmekaDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CAmekaDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CAmekaDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CAmekaDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CAmekaDoc diagnostics

#ifdef _DEBUG
void CAmekaDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAmekaDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CAmekaDoc commands

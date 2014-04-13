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

#define BUFFER_LEN 4096

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
	PrimaryData = new amekaData<RawDataType>(BUFFER_LEN);
	TemporaryData = new amekaData<RawDataType>(BUFFER_LEN);
	SecondaryData = new amekaData<SecondaryDataType>(BUFFER_LEN);
	mDSP.HPFFre = 0.5;
	mDSP.LPFFre = 30;
	mDSP.SampleRate = 200;
	this->m_dspProcess = AfxBeginThread(DSP::DSPThread, (LPVOID)this);
	//thrd = AfxBeginThread(genData, (LPVOID)this);
	mDSP.epocLength = 0.1;
}

CAmekaDoc::~CAmekaDoc()
{
	delete dataBuffer;
	delete PrimaryData;
	delete SecondaryData;
	DWORD exit_code = NULL;
	GetExitCodeThread(this->m_dspProcess->m_hThread, &exit_code);

	if(exit_code == STILL_ACTIVE)
	{
		::TerminateThread(this->m_dspProcess->m_hThread, 0);
		CloseHandle(this->m_dspProcess->m_hThread);
	}
	m_dspProcess = NULL;

	POSITION pos = theApp.docList.Find(this);
	theApp.docList.RemoveAt(pos);
}

BOOL CAmekaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	dataBuffer = new amekaData<RawDataType>(arrLen);
	theApp.docList.AddTail(this);

	POSITION pos =  theApp.monList.GetHeadPosition();
	if (pos != NULL)
	{
		LPAmontage mon =  theApp.monList.GetNext( pos );
		mMon = mon;
		CMainFrame *pMainWnd = (CMainFrame *)AfxGetMainWnd();
		CMFCRibbonComboBox* pMon = DYNAMIC_DOWNCAST(
		CMFCRibbonComboBox, pMainWnd->m_wndRibbonBar.FindByID(MN_MonList));
		if (pMon && mMon)
			pMon->SetEditText(mMon->mName);
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

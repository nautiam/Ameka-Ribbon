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

#include <propkey.h>

#define xmlName "abc.xml"

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

CAmekaDoc::CAmekaDoc()
{

	// TODO: add one-time construction code here
	dataBuffer = new amekaData<RawDataType>(arrLen);
	theApp.docList.AddTail(this);

	TiXmlDocument doc(xmlName);
	uint16_t count = 0;

	if(!doc.LoadFile())
	{
//		LOG(ERROR) << doc.ErrorDesc();
	}

	TiXmlElement* root = doc.FirstChildElement();
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
		const char* attr3;
		
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
				this->mMontage.mList.AddTail(node);
				this->mMontage.leadNum++;
			}
		}
		
		if(elemName == "Electrode")
		{
			attr1 = elem->Attribute("channel1");
			attr2 = elem->Attribute("name");
			if(attr1 != NULL && attr2 != NULL)
			{
				LPAelectrode elec = new Aelectrode;
				elec->eID = atoi(attr1);
				elec->eName = attr2;
				this->mElec.AddTail(elec);
			}
		}

		if(elemName == "Filter")
		{
			attr1 = elem->Attribute("LP");
			attr2 = elem->Attribute("HP");
			attr3 = elem->Attribute("SampleRate");
			if(attr1 != NULL && attr2 != NULL && attr3 != NULL)
			{
				DSPData dsp;
				dsp.LPFFre = atoi(attr1);
				dsp.HPFFre = atoi(attr2);
				dsp.SampleRate = atoi(attr3);
				this->mDSP = dsp;
			}
		}
	}
}

CAmekaDoc::~CAmekaDoc()
{
	delete dataBuffer;
	dataBuffer = NULL;
	POSITION pos = theApp.docList.Find(this);
	theApp.docList.RemoveAt(pos);
}

BOOL CAmekaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

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

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

// AmekaDoc.h : interface of the CAmekaDoc class
//
#ifndef _AMEKADOC_H_
#define _AMEKADOC_H_
#include "AmekaUserConfig.h"
#include "DSPModule.h"

#define arrLen 4096

#pragma once


class CAmekaDoc : public CDocument
{
protected: // create from serialization only
	CAmekaDoc();
	DECLARE_DYNCREATE(CAmekaDoc)

// Attributes
public:
	LPAmontage mMon;
	DSPData mDSP;
	CList<LPAelectrode, LPAelectrode> mElec;
	CWinThread*  m_dspProcess;
	amekaData<RawDataType>* dataBuffer;
	amekaData<PrimaryDataType>* PrimaryData;
	amekaData<PrimaryDataType>* TemporaryData; // Temporary Data is used to calculate photic
	amekaData<SecondaryDataType>* SecondaryData;
	CWinThread* thrd;
	//UINT genData(LPVOID pParam);
	//AmekaData<RawDataType>* dataBuffer;
// Operations

public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CAmekaDoc();
	static CAmekaDoc* GetDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};

#endif
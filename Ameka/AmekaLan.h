#ifndef _AMEKA_LAN_
#define _AMEKA_LAN_
#include "stdafx.h"
#include "Ameka.h"

CString itoS ( int x);

void loadLanguage(const char* fileName);

void loadSetting(const char* fileName);

CString getElecName(uint16_t num);

CPoint* getElecPoint(uint16_t num);

int getElecID(CString name);

void writeLan(const char* fileName);

void writeSetting(const char* fileName);

CString getEventName(uint16_t evID);

#endif
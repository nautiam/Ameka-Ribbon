#ifndef _AMEKA_LAN_
#define _AMEKA_LAN_
#include "stdafx.h"
#include "Ameka.h"

CString itoS ( int x);

void loadLanguage(CString fileName);

void loadSetting(CString fileName);

CString getElecName(uint16_t num);

void writeLan(CString fileName);

void writeSetting(CString fileName);
#endif
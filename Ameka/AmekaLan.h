#ifndef _AMEKA_LAN_
#define _AMEKA_LAN_
#include "stdafx.h"
#include "Ameka.h"

CString itoS ( int x);

void loadLanguage(const char* fileName);

void loadSetting(const char* fileName);

CString getElecName(uint16_t num);

void writeLan(const char* fileName);

void writeSetting(const char* fileName);
#endif
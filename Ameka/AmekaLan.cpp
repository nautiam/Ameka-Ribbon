
#include "stdafx.h"
#include "AmekaLan.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "easylogging++.h"

#ifndef settingName
#define settingName "config.conf"
#endif

CString itoS ( int x)
{
	CString sout;
	sout.Format(L"%i", x);

	return sout;
}

void loadLanguage(const char* fileName)
{
	TiXmlDocument doc;
	if(!doc.LoadFile(fileName))
	{
		LOG(ERROR) << doc.ErrorDesc();
		return;
	}
	TiXmlElement* root = doc.FirstChildElement();
	if(root == NULL)
	{
		LOG(ERROR) << "Failed to load file: No root element.";
		doc.Clear();
		return;
	}
	for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		if (elem->Value() == "File")
		{
			for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
			{
				if (e->Value() == "File")
					theApp.mnLan->mnFile.strMenuName = e->Attribute("name");
				if (e->Value() == "New")
					theApp.mnLan->mnFile.strNew = e->Attribute("name");
				if (e->Value() == "Open")
					theApp.mnLan->mnFile.strOpen = e->Attribute("name");
				if (e->Value() == "Close")
					theApp.mnLan->mnFile.strClose = e->Attribute("name");
				if (e->Value() == "Save")
					theApp.mnLan->mnFile.strSave = e->Attribute("name");
				if (e->Value() == "Print")
					theApp.mnLan->mnFile.strPrint = e->Attribute("name");
				if (e->Value() == "Option")
					theApp.mnLan->mnFile.strOption = e->Attribute("name");
				if (e->Value() == "PortName")
					theApp.mnLan->mnFile.strPortName = e->Attribute("name");
				if (e->Value() == "BaudRate")
					theApp.mnLan->mnFile.strBaudRate = e->Attribute("name");
				if (e->Value() == "PortScan")
					theApp.mnLan->mnFile.strScanPort = e->Attribute("name");
				if (e->Value() == "PortOpen")
					theApp.mnLan->mnFile.strOpenPort = e->Attribute("name");
			}
		}
		if (elem->Value() == "Option")
		{
			for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
			{
				if (e->Value() == "Info")
					theApp.mnLan->mnOpt.strMenuName = e->Attribute("name");
				if (e->Value() == "Info")
					theApp.mnLan->mnOpt.strInfo = e->Attribute("name");
				if (e->Value() == "Analysis")
					theApp.mnLan->mnOpt.strAnl = e->Attribute("name");
				if (e->Value() == "Montage")
					theApp.mnLan->mnOpt.strMon = e->Attribute("name");
				if (e->Value() == "Event")
					theApp.mnLan->mnOpt.strEvent = e->Attribute("name");
				if (e->Value() == "Log")
					theApp.mnLan->mnOpt.strLog = e->Attribute("name");
				if (e->Value() == "WaveGen")
					theApp.mnLan->mnOpt.strWave = e->Attribute("name");
				if (e->Value() == "Language")
					theApp.mnLan->mnOpt.strLan = e->Attribute("name");
			}
		}
		if (elem->Value() == "Wave")
		{
			for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
			{
				if (e->Value() == "Wave")
					theApp.mnLan->mnWave.strMenuName = e->Attribute("name");
				if (e->Value() == "Start")
					theApp.mnLan->mnWave.strStart = e->Attribute("name");
				if (e->Value() == "Stop")
					theApp.mnLan->mnWave.strStop = e->Attribute("name");
				if (e->Value() == "Recording")
					theApp.mnLan->mnWave.strRecord = e->Attribute("name");
				if (e->Value() == "Photic")
					theApp.mnLan->mnWave.strPhotic = e->Attribute("name");
				if (e->Value() == "Sensitivity")
					theApp.mnLan->mnWave.strSensi = e->Attribute("name");
				if (e->Value() == "PaperSpeed")
					theApp.mnLan->mnWave.strPaperSpeed = e->Attribute("name");
				if (e->Value() == "LP")
					theApp.mnLan->mnWave.strLPF = e->Attribute("name");
				if (e->Value() == "HP")
					theApp.mnLan->mnWave.strHPF = e->Attribute("name");
			}
		}
		if (elem->Value() == "Help")
		{
			for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
			{
				if (e->Value() == "Help")
					theApp.mnLan->mnHelp.strMenuName = e->Attribute("name");
				if (e->Value() == "About")
					theApp.mnLan->mnHelp.strAbout = e->Attribute("name");
			}
		}
	}
	doc.Clear();
};

void loadSetting(const char* fileName)
{
	TiXmlDocument doc;
	if(!doc.LoadFile(fileName))
	{
		LOG(ERROR) << doc.ErrorDesc();
		return;
	}
	TiXmlElement* root = doc.FirstChildElement();
	if(root == NULL)
	{
		LOG(ERROR) << "Failed to load file: No root element.";
		doc.Clear();
		return;
	}

	uint16_t iCount = 0;

	for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		const char* tmp = elem->Attribute("Name");
		if (strcmp(tmp,"lead") == 0)
		{
			for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
			{
				iCount++;
			}
			break;
		}
	}
	if (iCount == 0)
		return;
	theApp.mElec = new Aelectrode[iCount];
	theApp.elecNum = iCount;
	iCount = 0;
	for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{		
		const char* tmp = elem->Attribute("Name");
		if (strcmp(tmp, "lead") == 0)
		{
			for(TiXmlElement* e = elem->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
			{
				if (e->Attribute("ID") != NULL)
					theApp.mElec[iCount].eID = atoi(e->Attribute("ID"));
				if (e->Attribute("Name") != NULL)
					theApp.mElec[iCount].eName = e->Attribute("Name");
				if (e->Attribute("point1") != NULL && e->Attribute("point2") != NULL)
				{
					theApp.mElec[iCount].ePos.x = atoi(e->Attribute("point1"));
					theApp.mElec[iCount].ePos.y = atoi(e->Attribute("point2"));
				}
				iCount++;
			}
			break;
		}
	}
	doc.Clear();
}

CString getElecName(uint16_t num)
{
	for (int i = 0; i < theApp.elecNum; i++)
		if (theApp.mElec[i].eID == num)
			return theApp.mElec[i].eName;
	return L"";
}

int getElecID(CString name)
{
	for (int i = 0; i < theApp.elecNum; i++)
		if (theApp.mElec[i].eName == name)
			return theApp.mElec[i].eID;
	return -1;
}

CPoint* getElecPoint(uint16_t num)
{
	for (int i = 0; i < theApp.elecNum; i++)
		if (theApp.mElec[i].eID == num)
			return &theApp.mElec[i].ePos;
	return NULL;
};

void writeSetting(const char* fileName)
{		
	CString tempElecName[16] = {L"ml", L"cd1", L"cd2", L"tf1", L"f2", L"tf2", L"tf3", L"ld", L"ss", L"ss2", L"se3", L"af", L"af1", L"af2", L"af3", L"af4"};
	TiXmlDocument doc;
	TiXmlElement* root = new TiXmlElement("root");
	doc.LinkEndChild(root);

	TiXmlElement* element = new TiXmlElement("Lead");
	element->SetAttribute("Name", "lead");
	root->LinkEndChild(element);
	for (int i = 0; i < 16; i++)
	{
		TiXmlElement* element1 = new TiXmlElement((LPCSTR)(CStringA)itoS(i+1));
		element->LinkEndChild(element1);
		element1->SetAttribute("Name", (LPCSTR)(CStringA)tempElecName[i]);
	}

	bool success = doc.SaveFile(fileName);
	doc.Clear();
}


#include "stdafx.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "AmekaUserConfig.h"
#include "AmekaDoc.h"

#define FAILURE -1
#define SUCCESS 0

int loadXML(CString fileName)
{
	return 0;
}

int saveXML(CString filename)
{
	TiXmlDocument doc;
	TiXmlElement* root = new TiXmlElement("root");
	doc.LinkEndChild(root);

	TiXmlElement* element1 = new TiXmlElement("Montage");
	root->LinkEndChild(element1);
	element1->SetAttribute("channel1", "1");
	element1->SetAttribute("channel2", "2");

	TiXmlElement* element2 = new TiXmlElement("Montage");
	root->LinkEndChild(element2);
	element2->SetAttribute("channel1", "1");
	element2->SetAttribute("channel2", "3");

	bool success = doc.SaveFile(filename);
	doc.Clear();
	if(success)
		return SUCCESS;
	else
		return FAILURE;
}
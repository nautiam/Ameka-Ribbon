
#include "stdafx.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "easylogging++.h"
#include "AmekaUserConfig.h"
#include "AmekaDoc.h"

_INITIALIZE_EASYLOGGINGPP

#define FAILURE -1
#define SUCCESS 0

int loadXML(CString fileName)
{
	TiXmlDocument doc;
	uint16_t count = 0;

	if(!doc.LoadFile(fileName))
	{
		LOG(ERROR) << doc.ErrorDesc();
		return FAILURE;
	}

	TiXmlElement* root = doc.FirstChildElement();
	if(root == NULL)
	{
		LOG(ERROR) << "Failed to load file: No root element.";
		doc.Clear();
		return FAILURE;
	}

	CAmekaDoc* crtDoc = CAmekaDoc::GetDoc();

	for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		CString elemName = elem->Value();
		const char* attr1;
		const char* attr2;
		if(elemName == "Montage")
		{
			attr1 = elem->Attribute("Channel 1");
			attr2 = elem->Attribute("Channel 2");
			if(attr1 != NULL && attr2 != NULL)
			{
				count++;
				Alead node;
				node.lFirstID = atoi(attr1);
				node.lSecondID = atoi(attr2);
				crtDoc->mMontage.mList->addTail(node);
				crtDoc->mMontage.leadNum++;
		}
		if(elemName == "Electrode")
		{
			attr1 = elem->Attribute("Channel 1");
			attr2 = elem->Attribute("Name");
			if(attr1 != NULL && attr2 != NULL)
			{
				Aelectrode elec;
				elec.eID = atoi(attr1);
				elec.eName = attr2;
				crtDoc->mElec->addTail(elec);
		}
	}

}
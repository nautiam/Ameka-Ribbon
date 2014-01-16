
#include "stdafx.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "AmekaUserConfig.h"
#include "AmekaDoc.h"


#define FAILURE -1
#define SUCCESS 0

int loadXML(CString fileName)
{
	TiXmlDocument doc(fileName);
	uint16_t count = 0;

	if(!doc.LoadFile())
	{
//		LOG(ERROR) << doc.ErrorDesc();
		return FAILURE;
	}

	TiXmlElement* root = doc.FirstChildElement();
	if(root == NULL)
	{
//		LOG(ERROR) << "Failed to load file: No root element.";
		doc.Clear();
		return FAILURE;
	}

	CAmekaDoc* crtDoc = CAmekaDoc::GetDoc();

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
				crtDoc->mMontage.mList.AddTail(node);
				crtDoc->mMontage.leadNum++;
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
				crtDoc->mElec.AddTail(elec);
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
				crtDoc->mDSP = dsp;
			}
		}
	}
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
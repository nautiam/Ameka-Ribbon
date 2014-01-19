#include "stdafx.h"
#include "DSPModule.h"
#include "easylogging++.h"
#include "Ameka.h"

IMPLEMENT_DYNCREATE(DSPModule,CWinThread)
DSPModule::DSPModule()
{
	/*for (int i=0; i<LEAD_NUMBER; i++)
	{
		audioData[i] = new float[numSamples];
	}*/
	numSamples = 2000;
	sampleRate = SAMPLE_RATE;
	HighFre = 0.5;
	LowFre = 35;
	CenterFre = sqrt(HighFre*LowFre);
	BandWidth = LowFre - HighFre;
}
DSPModule::~DSPModule()
{
	
}

BOOL DSPModule::InitInstance()
{
	return TRUE;
}

int DSPModule::Run()
{
	//dataBuffer = new RawDataType[ARRAY_LENGTH]
	//RawDataType output[ARRAY_LENGTH];
	RawDataType* dataBuffer = theApp.pIO->RawData->popAll();
	int size = theApp.pIO->RawData->rLen;
	RawDataType* output = new RawDataType[size];
	
	for (int i=0; i<LEAD_NUMBER; i++)
	{
		audioData[i] = new float[size];
	}

	for (int i=0; i<LEAD_NUMBER; i++)
		for (int j=0; j<10; j++)
	{
		audioData[i][j] = dataBuffer[j].value[i];
	}
	switch (Type_design)
	{
	case 0:	//RBJ BandPass1
		Dsp::SmoothedFilterDesign <Dsp::RBJ::Design::BandPass1, LEAD_NUMBER> f (1024);
		Dsp::Params params;
		params[0] = sampleRate; // sample rate
		params[1] = CenterFre; // Center frequency
		params[2] = BandWidth; // Band Width
		f.setParams (params);
		f.process (numSamples, audioData);
		break;
	case 1: //RBJ BandPass2
		Dsp::SmoothedFilterDesign <Dsp::RBJ::Design::BandPass1, LEAD_NUMBER> f (1024);
		Dsp::Params params;
		params[0] = sampleRate; // sample rate
		params[1] = CenterFre; // Center frequency
		params[2] = BandWidth; // Band Width
		f.setParams (params);
		f.process (numSamples, audioData);
		break;
	case 2: //Butterworth BandPass
		//Dsp::Filter* f = new Dsp::SmoothedFilterDesign
		//<Dsp::Butterworth::Design::BandPass <4>, LEAD_NUMBER, Dsp::DirectFormII> (1024);
		Dsp::SmoothedFilterDesign <Dsp::Butterworth::Design::BandPass <4>, LEAD_NUMBER, Dsp::DirectFormII> f (1024);
		Dsp::Params params;
		params[0] = sampleRate; // sample rate
		params[1] = 4; // order
		params[2] = CenterFre; // center frequency
		params[3] = BandWidth; // band width
		f.setParams (params);
		f.process (numSamples, audioData);
		break;
	case 3: //Chebyshevl1 BandPassBase
		break;
	case 4: //Chebyshevl2 BandPassBase
		break;
	case 5: //Eliliptic BandPassBase
		break;
	case 6: //Legendre BandPassBase
		break;
	default:
		break;
	}

	for (int i=0; i<LEAD_NUMBER; i++)
		for (int j=0; j<10; j++)
	{
		output[j].value[i] = audioData[i][j];
	}
}

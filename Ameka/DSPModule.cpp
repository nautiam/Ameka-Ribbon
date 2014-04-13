#include "stdafx.h"
#include "easylogging++.h"
#include "AmekaDoc.h"
#include "dsp_filters.h"
#include "DSPModule.h"
#include "kiss_fft.h"

static void convert_to_freq(kiss_fft_cpx *cout, int n) {
	const float NC = n/2.0 + 1;
	while (n-- > 0) {
		cout->r /= NC;
		cout->i /= NC;
		cout++;
	}
}

static void complex_abs(kiss_fft_cpx *cout, int n) {
	while (n-- > 0) {
		cout->r = sqrt(cout->r * cout->r + cout->i * cout->i);
		cout->i = 0;
		cout++;
	}
}

static int get_peak_pos(const kiss_fft_cpx *cout, int nfft, int start_pos) {
	int pos = 0;
	float maxdata = 0;
	int i;
	for (i = start_pos; i < nfft/2; i++) {
		if ((cout[i].r - maxdata) > 0.0001) {
			maxdata = cout[i].r;
			pos = i;
		}
	}
	return pos;
}

static float get_peak_frequence(const kiss_fft_cpx *cout, int nfft, float start_hz, float sample_hz) {
	int start_pos = start_hz * nfft / sample_hz;
	return get_peak_pos(cout, nfft, start_pos) * sample_hz / nfft;
}

UINT DSP::DSPThread(LPVOID pParam)
{
	CAmekaDoc* mDoc = (CAmekaDoc*)(pParam);
	uint16_t numSamples = 2000;
	Dsp::SmoothedFilterDesign <Dsp::Butterworth::Design::BandPass <4>, LEAD_NUMBER, Dsp::DirectFormII> f (4096);
	Dsp::Params params;
	while (1)
	{
		Sleep(50);
		float HighFre = mDoc->mDSP.HPFFre;
		float LowFre = mDoc->mDSP.LPFFre;
		float sampleRate = mDoc->mDSP.SampleRate;
		float CenterFre = sqrt(HighFre*LowFre);
		float BandWidth = LowFre - HighFre;
		params[0] = sampleRate; // sample rate
		params[1] = 4; // order
		params[2] = CenterFre; // center frequency
		params[3] = BandWidth; // band width
		f.setParams (params);
		static int count = 0;
		count++;
		//AfxMessageBox(mDoc->mMon->mName);
		//LOG(DEBUG) << mDoc->mMon->mList.GetCount();
		//POSITION pos;
		
		RawDataType* data = mDoc->dataBuffer->checkPopData(100);
		int size =  mDoc->dataBuffer->rLen;
		
		if (size > 0 && data != NULL)
		{
			float* audioData[LEAD_NUMBER];
			RawDataType* output = new RawDataType[size];
			if (!output)
				AfxMessageBox(L"Error!");
			for (int i=0; i<LEAD_NUMBER; i++)
			{
				audioData[i] = new float[size];
			}

			for (int i=0; i<LEAD_NUMBER; i++)
				for (int j=0; j<size; j++)
			{
				audioData[i][j] = (float)data[j].value[i];
			}

			f.process (size, audioData);

			for (int j=0; j<size; j++)
			{
				for (int i=0; i<LEAD_NUMBER; i++)				
				{
					output[j].value[i] = (uint16_t)audioData[i][j];
					output[j].time = data[j].time;				
				}
			}
			
			for (int i=0; i<size; i++)
			{
				if (mDoc->PrimaryData->pushData(output[i]) != 0)
				{
					LOG(DEBUG) << "Primary Data ring buffer is full";	
				}
				if (mDoc->TemporaryData->pushData(output[i]) != 0)
				{
					LOG(DEBUG) << "Temporary Data ring buffer is full";	
				}
			}
			
			for (int i=0; i<LEAD_NUMBER; i++)
			{
				delete [] audioData[i];
			}
			delete [] data;			
			delete [] output;
		}		

		float epocLength = mDoc->mDSP.epocLength;
		int nfft;
		nfft = (float)SAMPLE_RATE/epocLength;
		float NC = (float)nfft/2.0 + 1.0;

		RawDataType* output = mDoc->TemporaryData->checkPopData(nfft);
		size =  mDoc->TemporaryData->rLen;
		
		if (size > 0 && output != NULL)
		{
			int dataLen = mDoc->TemporaryData->dataLen;
			mDoc->TemporaryData->LRPos = (mDoc->TemporaryData->LRPos + dataLen - size + 100) % dataLen;
			int isinverse = 0;
			kiss_fft_cfg st;
			kiss_fft_cpx * buf[LEAD_NUMBER];
			kiss_fft_cpx * bufout[LEAD_NUMBER];

			for (int i=0; i<LEAD_NUMBER; i++)
			{
				buf[i] = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * nfft );
				bufout[i] = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * nfft );
			}
			st = kiss_fft_alloc( nfft ,isinverse ,0,0);
			
			// Add value for input buf, then convert to frequency
			for (int j=0; j<LEAD_NUMBER; j++)
			{
				for (int i=0; i<nfft; i++)
				{
					buf[j][i].r = output[i].value[j];
					buf[j][i].i = 0;
				}
				kiss_fft( st , buf[j] ,bufout[j]);
				//convert_to_freq(bufout[j], nfft);
				complex_abs(bufout[j], NC);
			}

			// Convert output to frequency
			/*float NC = nfft/2.0 + 1;
			for (int i=0; i<LEAD_NUMBER; i++)
			{
				convert_to_freq(bufout[i], nfft);
				complex_abs(bufout[i], nfft);
			}*/

			// Print output to file
			for (int i=0; i<(int)NC; i++)
			{
				SecondaryDataType temp;
				float fre = i * epocLength;
				temp.fre = fre;
				for (int j=0; j<LEAD_NUMBER; j++)	
				{
					if (fre < HighFre)
					{
						temp.value[j] = 0;
					}
					else
					{
						temp.value[j] = bufout[j][i].r;
					}
					//float fre = i * (float)(SAMPLE_RATE / nfft);
					/*LOG(INFO) << "------------";
					LOG(INFO) << j;
					LOG(INFO) << fre;
					LOG(INFO) << bufout[j][i].r;*/
				}				
				if (mDoc->SecondaryData->pushData(temp) != 0)
				{
					LOG(DEBUG) << "Secondary Data Ring buffer is full";
				};
			}
			free(st);
			for (int i=0; i<LEAD_NUMBER; i++)
			{
				free(buf[i]);
				free(bufout[i]);
			}
			delete [] output;			
		}
	}
	return 0;
}

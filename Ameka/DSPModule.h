#ifndef _DSPMODULE_H_
#define _DSPMODULE_H_

#define SAMPLE_RATE 200
#define ARRAY_LENGTH 10
#define BACKUP_ARRAY 10

void photic_processing(float fre_step, LPVOID pParam, uint64_t pos);
void dsp_processing(LPVOID pParam);
void initial_dsp_data(LPVOID pParam);
class DSP {
public:
	static UINT DSPThread(LPVOID pParam);
	static UINT ProcessRecordDataThread(LPVOID pParam);
};

#endif
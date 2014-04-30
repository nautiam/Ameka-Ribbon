#ifndef _DSPMODULE_H_
#define _DSPMODULE_H_

#define SAMPLE_RATE 256
#define ARRAY_LENGTH 10
#define BACKUP_ARRAY 10

class DSP {
public:
	static UINT DSPThread(LPVOID pParam);
	static UINT ProcessRecordDataThread(LPVOID pParam);
};

#endif
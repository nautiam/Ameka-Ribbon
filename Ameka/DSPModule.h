#ifndef _DSPMODULE_H_
#define _DSPMODULE_H_

#define SAMPLE_RATE 200
#define LEAD_NUMBER 16
#define ARRAY_LENGTH 10
#define BACKUP_ARRAY 10

class DSP {
public:
	static UINT DSPThread(LPVOID pParam);
};

#endif
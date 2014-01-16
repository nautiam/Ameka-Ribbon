//Ameka Varibale Definition
#include <stdint.h>
#include <winnt.h>
#include <time.h>
#include <mmc.h>
#include <math.h>
#include <Windows.h>
#include <vector>

#ifndef _USERCONFIG_H_
#define _USERCONFIG_H_

// Electrode struct
typedef struct _Aelectrode {
	uint16_t eID;
	CString eName;
} *LPAelectrode, Aelectrode;

typedef struct _Color{
	int R;
	int G;
	int B;
	_Color(int cl1, int cl2, int cl3){R=cl1;G=cl2;B=cl3;};
} *LPColor, Color;
// Lead struct
typedef struct _Alead {
	uint16_t lID;
	uint16_t lFirstID;
	uint16_t lSecondID;
	//Color color;
} *LPAlead, Alead;
// Montage struct
struct Amontage {
public:
	uint16_t leadNum;
	CList<LPAlead, LPAlead> mList;
	uint16_t mID;
	CString mName;
};

// DSP Engine
struct DSPData
{
	float LPFFre;
	float HPFFre;
	uint16_t SampleRate;
};

// Graph data
struct GraphData
{
	float scaleRate;
	uint16_t sampleRate;
	uint16_t paperSpeed;
	uint16_t dotPmm;
	uint8_t scanBarW;
	uint16_t drawInterval;
	uint16_t gridInterval;
};

//template<class T> CList<int, int> *rawData[16];
struct RawDataType {
	uint16_t value[16];
	time_t time;
};

struct PrimaryDataType {
	uint16_t* value;
	time_t time;
};

class RingBuffer
{
public:
	RawDataType* arrData;
	uint16_t dataLen;
	int crtWPos;
	int LRPos;

	RingBuffer(uint16_t len);
	~RingBuffer(void);
	bool isFull();
	bool isEmpty();
	int pushData(RawDataType data);	
	RawDataType * popData();
	RawDataType* popAll();
	RawDataType get(uint16_t index);
	//RawDataType* getData(uint16_t pos, uint16_t len);
	
private:
	// to be used later
	bool onLock;
};

template<class T> class amekaData {
public:
	T* arrData;
	uint16_t dataLen;
	int crtWPos;
	int LRPos;
	int rLen;
	//LPCRITICAL_SECTION csess;

	amekaData(uint16_t len);
	~amekaData(void);
	bool isFull();
	bool isEmpty();
	int pushData(T data);	
	T* popData();
	T get(uint16_t index);
	T* popAll();
	T* popData(uint16_t num);
	//T* popData(uint16_t num);
	
private:
	// to be used later
	bool onLock;
};

template<class T>
amekaData<T>::amekaData(uint16_t len)
{
	arrData = (T*)malloc(len*sizeof(T));
	dataLen = len;
	crtWPos = 0;
	LRPos = 0;
	rLen = 0;
	//InitializeCriticalSection(csess);
};

//amekaData<RawDataType> RawData(3);

template<class T>
bool amekaData<T>::isFull()
{
	if ((crtWPos + 1)%dataLen == LRPos)
		return true;
	return false;
};

template<class T>
bool amekaData<T>::isEmpty()
{
	if (crtWPos == LRPos)
		return true;
	return false;
};

template<class T>
int amekaData<T>::pushData(T data)
{
	uint16_t pos = crtWPos%dataLen;
	//EnterCriticalSection(csess);
	if (isFull())
	{
		arrData[pos] = data;
		crtWPos = (crtWPos+1)%dataLen;
		LRPos =  (LRPos+1)%dataLen;
		return -1;
	}
	arrData[pos] = data;
	crtWPos = (crtWPos+1)%dataLen;
	//LeaveCriticalSection(csess);
	return 0;
};

template<class T>
T* amekaData<T>::popData()
{
	//
	if (isEmpty())
		return NULL;
	//EnterCriticalSection(csess);
	T* tmp = &arrData[LRPos%dataLen];
	LRPos = (LRPos+1)%dataLen;
	//LeaveCriticalSection(csess);
	return tmp;
};

template<class T>
T* amekaData<T>::popAll()
{
	//
	if (isEmpty())
		return 0;
	T* data;
	//EnterCriticalSection(csess);
	uint16_t len = (crtWPos+dataLen-LRPos)%dataLen;
	data = (T*)malloc(len*sizeof(T));
	for (int i = 0; i < len; i++)
		data[i] = arrData[(LRPos+i)%dataLen];
	LRPos = (LRPos+len)%dataLen;
	//LeaveCriticalSection(csess);
	rLen = len;
	return data;
};

template<class T>
T* amekaData<T>::popData(uint16_t num)
{
	//
	//EnterCriticalSection(csess);
	T* data = (T*)malloc(num*sizeof(T));
	for (int i = 0; i < num; i++)
	{
		if (isEmpty())
		{
			rLen = i;
			return data;
		}
		data[i] = arrData[LRPos%dataLen];
		LRPos = (LRPos+1)%dataLen;
	}
	//LRPos = (LRPos+num)%dataLen;
	rLen = num;
	//LeaveCriticalSection(csess);
	return data;
};

template<class T>
T amekaData<T>::get(uint16_t index)
{
	//
	/*if (isEmpty())
		return NULL;*/

	return arrData[(index + crtWPos)%dataLen];
};


template<class T>
amekaData<T>::~amekaData(void)
{
	//
	delete[] arrData;
	arrData = NULL;
	//DeleteCriticalSection(&csess);
};


#endif
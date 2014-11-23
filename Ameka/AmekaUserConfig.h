//Ameka Varibale Definition
#include <stdint.h>
#include "stdafx.h"
#include <winnt.h>
#include <time.h>
#include <mmc.h>
#include <math.h>
#include <Windows.h>
#include <vector>
//#include "TypedFifo.h"

#ifndef _USERCONFIG_H_
#define _USERCONFIG_H_

#define FRE_STEP 0.1
#define LEAD_NUMBER 16
#define MONTAGE_NUM 64
#define BASELINE 16383
#define AMP 812

template <typename T , typename T2>
class CMyArray : public CArray<T , T2> 
{
public:

	// constructors

	CMyArray(){}

	CMyArray(int n) { SetSize(n);}

	CMyArray(int n, const T& t) 
	{ 
		for (int i=0; i<n; ++i) Add(t); 
	}

	// copy constructor

	CMyArray (const CMyArray& rhs) { Append(rhs); }

	// copy assignment operator

	CMyArray& operator = (const CMyArray& rhs) 
	{
		if (this != &rhs)
		{
			RemoveAll();
			Append(rhs);
		}

		return *this;
	}

	// destructor

	~CMyArray(){}
};

// Electrode struct
struct Aelectrode {
	uint16_t eID;
	CString eName;
	CPoint ePos;
	/*Aelectrode& operator=( const Aelectrode& rhs ) { return *this; }
	BOOL operator==( const Aelectrode& rhs ) const { return TRUE; }*/
};

// Lead struct
struct Alead {
	uint16_t lID;
	uint16_t lFirstID;
	uint16_t lSecondID;
	/*Alead& operator=( const Alead& rhs ) { return *this; }
	BOOL operator==( const Alead& rhs ) const { return TRUE; }*/
	//Color color;
};
// Montage struct
struct Amontage {
public:
	uint16_t leadNum;
	CMyArray<Alead, Alead&> mList;
	uint16_t mID;
	CString mName;
	/*Amontage& operator=( const Amontage& rhs ) { return *this; }
	BOOL operator==( const Amontage& rhs ) const { return TRUE; }*/
};

// DSP Engine
struct DSPData
{
	float LPFFre;
	float HPFFre;
	uint16_t SampleRate;

	// Spectral Data
	float epocLength;
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

//patient information structure
struct PatientInfo {
	CString fname;
	CString lname;
	CString surname;
	CString note;
	CTime birthday;
	CString uID;
	CString sex;
	bool lefthanded;
	PatientInfo() {
		fname = "N.A";
		lname = "N.A";
		surname = "N.A";
		note = "N.A";
		uID = "N.A";
		sex = "Unknown";
	};
};


/************************************************************
Data struct for Ameka
************************************************************/

//template<class T> CList<int, int> *rawData[16];
struct RawDataType {
	uint16_t value[LEAD_NUMBER];
	time_t time;
	uint16_t eventID;
};

struct PrimaryDataType {
	uint16_t value[MONTAGE_NUM];
	time_t time;
	bool isDraw;
	uint16_t eventID;
};

struct SecondaryDataType {
	float value[MONTAGE_NUM];
	float fre;
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
	//CRITICAL_SECTION csess;

	amekaData(uint16_t len);
	~amekaData(void);
	bool isFull();
	bool isEmpty();
	int pushData(T data);	
	T* popData();
	T get(uint16_t index);
	T* getMultiData(uint16_t num);
	T* popAll();
	T* popData(uint16_t num);
	T* checkPopData(uint16_t num);

private:
	// to be used later
	bool onLock;
};

template<class T>
amekaData<T>::amekaData(uint16_t len)
{
	arrData = new T[len];
	dataLen = len;
	crtWPos = 0;
	LRPos = 0;
	rLen = 0;
	//InitializeCriticalSection(&csess);
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
	//EnterCriticalSection(&csess);
	if (isFull())
	{
		arrData[pos] = data;
		crtWPos = (crtWPos+1)%dataLen;
		LRPos =  (LRPos+1)%dataLen;
		return -1;
	}
	arrData[pos] = data;
	crtWPos = (crtWPos+1)%dataLen;
	//LeaveCriticalSection(&csess);
	return 0;
};

template<class T>
T* amekaData<T>::popData()
{
	//
	if (isEmpty())
		return NULL;
	//EnterCriticalSection(&csess);
	T* tmp = &arrData[LRPos%dataLen];
	LRPos = (LRPos+1)%dataLen;
	//LeaveCriticalSection(&csess);
	return tmp;
};

template<class T>
T* amekaData<T>::popAll()
{
	//
	if (isEmpty())
	{
		rLen = 0;
		return NULL;
	}
	T* data;
	//EnterCriticalSection(&csess);
	uint16_t len = (crtWPos+dataLen-LRPos)%dataLen;
	if (len <= 0)
	{
		rLen = 0;
		return NULL;
	}
	data = new T[len];
	for (int i = 0; i < len; i++)
	{
		data[i] = arrData[LRPos%dataLen];
		LRPos = (LRPos+1)%dataLen;
	}
	//LeaveCriticalSection(&csess);
	rLen = len;
	return data;
};

template<class T>
T* amekaData<T>::popData(uint16_t num)
{
	//
	//EnterCriticalSection(&csess);
	T* data = new T[num];
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
	//LeaveCriticalSection(&csess);
	return data;
};

template<class T>
T* amekaData<T>::checkPopData(uint16_t num)
{
	//
	//EnterCriticalSection(&csess);
	if (((crtWPos+dataLen-LRPos)%dataLen) < num)
		return NULL;

	T* data = new T[num];
	for (int i = 0; i < num; i++)
	{
		if (isEmpty())
		{
			rLen = i;
			delete [] data;
			LRPos = (LRPos+dataLen-i)%dataLen;
			return NULL;
		}
		data[i] = arrData[LRPos%dataLen];
		LRPos = (LRPos+1)%dataLen;
	}
	rLen = num;
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
T* amekaData<T>::getMultiData(uint16_t num)
{
	//EnterCriticalSection(&csess);
	int old_LRPos = LRPos;
	T* data = new T[num];
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
	LRPos = old_LRPos;
	//LeaveCriticalSection(&csess);
	return data;
};

template<class T>
amekaData<T>::~amekaData(void)
{
	//
	delete[] arrData;
	arrData = NULL;
	//DeleteCriticalSection(&csess);
};

struct lan_File {
	CString strMenuName;
	CString strNew;
	CString strOpen;
	CString strClose;
	CString strSave;
	CString strPrint;
	CString strOption;
	CString strPortName;
	CString strBaudRate;
	CString strScanPort;
	CString strOpenPort;
};

struct lan_Option {
	CString strMenuName;
	CString strAnl;
	CString strMon;
	CString strEvent;
	CString strLog;
	CString strWave;
	CString strInfo;
	CString strLan;
};

struct lan_Wave {
	CString strMenuName;
	CString strStart;
	CString strStop;
	CString strRecord;
	CString strPhotic;
	CString strSensi;
	CString strPaperSpeed;
	CString strListMon;
	CString strLPF;
	CString strHPF;
};

struct lan_Help {
	CString strMenuName;
	CString strAbout;
};

struct amekaLan {
	struct lan_File mnFile;
	struct lan_Option mnOpt;
	struct lan_Wave mnWave;
	struct lan_Help mnHelp;
	amekaLan()
	{
		mnFile.strMenuName = "File";
		mnFile.strNew = "New";
		mnFile.strOpen = "Open";
		mnFile.strClose = "Close";
		mnFile.strSave = "Save";
		mnFile.strPrint = "Print";
		mnFile.strOption = "Option";
		mnFile.strPortName = "Port Name";
		mnFile.strBaudRate = "Baud Rate";
		mnFile.strScanPort = "Scan Port";
		mnFile.strOpenPort = "Open Port";

		mnOpt.strMenuName = "Option";
		mnOpt.strAnl = "Analysis";
		mnOpt.strMon = "Montage";
		mnOpt.strEvent = "Event";
		mnOpt.strLog = "Log";
		mnOpt.strWave = "Wave Generator";
		mnOpt.strInfo = "Patient Information";
		mnOpt.strLan = "Language";

		mnWave.strMenuName = "Wave";
		mnWave.strStart = "Start";
		mnWave.strStop = "Stop";
		mnWave.strRecord = "Recording";
		mnWave.strPhotic = "Photic";
		mnWave.strSensi = "Sensitivity";
		mnWave.strPaperSpeed = "Paper Speed";
		mnWave.strLPF = "LowPass Filter";
		mnWave.strHPF = "HighPass Filter";

		mnHelp.strAbout = "About";
		mnHelp.strMenuName = "Help";
	};
};


#endif
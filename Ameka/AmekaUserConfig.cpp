#include "StdAfx.h"
#include "AmekaUserConfig.h"

RingBuffer::RingBuffer(uint16_t len)
{
	arrData = (RawDataType*)malloc(len);
	dataLen = len;
	crtWPos = 0;
	LRPos = 0;
};

bool RingBuffer::isFull()
{
	if ((crtWPos + 1)%dataLen == LRPos)
		return true;
	return false;
};

bool RingBuffer::isEmpty()
{
	if (crtWPos == LRPos)
		return true;
	return false;
};

int RingBuffer::pushData(RawDataType data)
{
	if (isFull())
	{
		arrData[(crtWPos++)%dataLen] = data;
		crtWPos = crtWPos%dataLen;
		LRPos =  (LRPos+1)&dataLen;
		return -1;
	}
	arrData[(crtWPos++)%dataLen] = data;
	crtWPos = crtWPos%dataLen;
	return 0;
};

RawDataType * RingBuffer::popData()
{
	//
	if (isEmpty())
		return NULL;
	RawDataType* tmp = &arrData[(LRPos++)%dataLen];
	return tmp;
};

RawDataType* RingBuffer::popAll()
{
	if (isEmpty())
		return 0;
	RawDataType* data;
	uint16_t len = (crtWPos+dataLen-LRPos)%dataLen;
	data = (RawDataType*)malloc(len*sizeof(RawDataType));
	for (int i = 0; i < len; i++)
		*(data+i) = arrData[(LRPos+1+i)%dataLen];
	LRPos = (LRPos+len)%dataLen;
	return data;
}

RawDataType RingBuffer::get(uint16_t index)
{
	//
	/*if (isEmpty())
		return NULL;*/

	return arrData[(index + 1 + crtWPos)%dataLen];
};

//RawDataType* RingBuffer::getData(uint16_t pos, uint16_t len)
//{
//	RawDataType* tmpBuf = new RawDataType[num];
//	if ( isEmpty() || (num+LRPos+1)%dataLen>crtWPos )
//		return NULL;
//	for (int i = 0; i < num; i++)
//		tmpBuf[i] = arrData[((LRPos+1+num)%dataLen)];
//	return tmpBuf;
//};

RingBuffer::~RingBuffer(void)
{
	//
	delete[] arrData;
	arrData = NULL;
};

#include "stdafx.h"
#include <stdint.h>

struct bufferData{
public:
	bufferData() {arrLen=0;};
	uint32_t arr[1024];
	int getLen() { return arrLen; };
	int setData(uint32_t ldata) { arr[arrLen-1] = ldata;arrLen++;}
	void eraseData() { for (int i=0;i<arrLen;i++) arr[i]=0;arrLen=0;}
private:
	int arrLen;
	bool onlock;
};


class GraphModule : public CWnd
{
public:
	GraphModule()
	{
		crtPos = 0;
		distance = 0;
	};
	//declare variables here
	//CBitmap bmp;
	//CBitmap bmpBack;
	CBitmap bmp;
	CBitmap wholebmp;
	bool isData;
	uint16_t crtPos;
	uint16_t distance;
	//uint8_t distance;
	//uint32_t tmpArr[5];

	//declare functions here	
	void amekaDistanceCal(uint8_t a);			//calculate distance between 2 point on graph
	void amekaDrawWholeGraph(CBitmap* bitmap);	//for draw whole bitmap (when resize)
	
private:
	//declare variables here
	
	//declare variables here
};


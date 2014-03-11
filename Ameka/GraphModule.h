#include "stdafx.h"
#include <stdint.h>

typedef struct tARROWSTRUCT {
	int nWidth;		// width (in pixels) of the full base of the arrowhead
	float fTheta;	// angle (in radians) at the arrow tip between the two
					//  sides of the arrowhead
	bool bFill;		// flag indicating whether or not the arrowhead should be
					//  filled
} ARROWSTRUCT;

// ArrowTo()
//
// Draws an arrow, using the current pen and brush, from the current position
//  to the passed point using the attributes defined in the ARROWSTRUCT.
void ArrowTo(HDC hDC, int x, int y, ARROWSTRUCT *pArrow);
void ArrowTo(HDC hDC, const POINT *lpTo, ARROWSTRUCT *pArrow);

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


#include "stdafx.h"
#include <stdint.h>
#include "AmekaView.h"

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

class GraphModule
{
	
private:
	//declare variables here
	
	//declare variables here
};


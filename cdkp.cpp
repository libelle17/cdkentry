#include "cdkp.h"
#include <vector>
using namespace std;

vector<CDKOP*> all_objects;



/*
 * Create a new object beginning with a CDKOBJS struct.  The whole object is
 * initialized to zeroes except for special cases which have known values.
 */
CDKOP::CDKOP() // void *_newCDKObject (unsigned size, const CDKFUNCS * funcs)
{
	 hasFocus = TRUE;
	 isVisible = TRUE;

	 all_objects.push_back(this);

	 /* set default line-drawing characters */
	 ULChar = ACS_ULCORNER;
	 URChar = ACS_URCORNER;
	 LLChar = ACS_LLCORNER;
	 LRChar = ACS_LRCORNER;
	 HZChar = ACS_HLINE;
	 VTChar = ACS_VLINE;
	 BXAttr = A_NORMAL;

	 /* set default exit-types */
	 exitType = vNEVER_ACTIVATED;
	 earlyExit = vNEVER_ACTIVATED;
}

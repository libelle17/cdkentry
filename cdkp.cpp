#include "cdkp.h"
#include <vector>
using namespace std;

vector<CDKOBJS*> all_objects;

/*
 * Return an integer like 'floor()', which returns a double.
 */
int floorCDK (double value)
{
   int result = (int)value;
   if (result > value)		/* e.g., value < 0.0 and value is not an integer */
      result--;
   return result;
}

/*
 * Return an integer like 'ceil()', which returns a double.
 */
int ceilCDK (double value)
{
   return -floorCDK (-value);
}

/*
 * This beeps then flushes the stdout stream.
 */
void Beep (void)
{
   beep ();
   fflush (stdout);
}


/*
 * Create a new object beginning with a CDKOBJS struct.  The whole object is
 * initialized to zeroes except for special cases which have known values.
 */
CDKOBJS::CDKOBJS() // void *_newCDKObject(unsigned size, const CDKFUNCS * funcs)
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

/*
 * This creates a pointer to an entry widget.
 */
SEntry::SEntry (CDKSCREEN *cdkscreen,
		int xplace,
		int yplace,
		const char *title,
		const char *label,
		chtype fieldAttr,
		chtype filler,
		EDisplayType dispType,
		int fWidth,
		int min,
		int max,
		boolean Box,
		boolean shadow,
		// GSchade Anfang
		int highnr/*=0*/
		// GSchade Ende
		)
{
	/* *INDENT-EQLS* */
	CDKENTRY *entry      = 0;
	int parentWidth      = getmaxx (cdkscreen->window);
	int parentHeight     = getmaxy (cdkscreen->window);
	int fieldWidth       = fWidth;
	int boxWidth         = 0;
	int boxHeight;
	int xpos             = xplace;
	int ypos             = yplace;
	int junk             = 0;
	int horizontalAdjust, oldWidth;

	if ((entry = newCDKObject (CDKENTRY, &my_funcs)) == 0)
		return (0);

	setCDKEntryBox (entry, Box);
	boxHeight = (BorderOf (entry) * 2) + 1;

	/*
	 * If the fieldWidth is a negative value, the fieldWidth will
	 * be COLS-fieldWidth, otherwise, the fieldWidth will be the
	 * given width.
	 */
	fieldWidth = setWidgetDimension (parentWidth, fieldWidth, 0);
	boxWidth = fieldWidth + 2 * BorderOf (entry);

	/* Set some basic values of the entry field. */
	entry->label = 0;
	entry->labelLen = 0;
	entry->labelWin = 0;
	entry->labelumlz=0; // GSchade

	// GSchade

	/* Translate the label char *pointer to a chtype pointer. */
	if (label != 0)
	{

		entry->label = char2Chtypeh(label, &entry->labelLen, &junk
				// GSchade Anfang
				,highnr
				// GSchade Ende
				);
		// GSchade Anfang
		for(int i=0;entry->label[i];i++) {
			if ((int)CharOf(entry->label[i])==194 || (int)CharOf(entry->label[i])==195) {
				entry->labelumlz++;
			}
		}
		// GSchade Ende
		boxWidth += entry->labelLen;
	}

	oldWidth = boxWidth;
	boxWidth = setCdkTitle (ObjOf (entry), title, boxWidth);
	horizontalAdjust = (boxWidth - oldWidth) / 2;

	boxHeight += TitleLinesOf (entry);

	/*
	 * Make sure we didn't extend beyond the dimensions of the window.
	 */
	boxWidth = MINIMUM (boxWidth, parentWidth);
	boxHeight = MINIMUM (boxHeight, parentHeight);
	fieldWidth = MINIMUM (fieldWidth,
			boxWidth - entry->labelLen +entry->labelumlz - 2 * BorderOf (entry));

	/* Rejustify the x and y positions if we need to. */
	alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

	/* Make the label window. */
	entry->win = newwin (boxHeight, boxWidth, ypos, xpos);
	if (entry->win == 0)
	{
		destroyCDKObject (entry);
		return (0);
	}
	keypad (entry->win, TRUE);

	/* Make the field window. */
	entry->fieldWin = subwin (entry->win, 1, fieldWidth,
			(ypos + TitleLinesOf (entry) + BorderOf (entry)),
			(xpos + entry->labelLen -entry->labelumlz
			 + horizontalAdjust
			 + BorderOf (entry)));
	if (entry->fieldWin == 0)
	{
		destroyCDKObject (entry);
		return (0);
	}
	keypad (entry->fieldWin, TRUE);

	/* Make the label win, if we need to. */
	if (label != 0)
	{
		entry->labelWin = subwin (entry->win, 1, entry->labelLen,
				ypos + TitleLinesOf (entry) + BorderOf (entry),
				xpos + horizontalAdjust + BorderOf (entry));
	}

	/* Make room for the info char * pointer. */
	entry->info = typeMallocN (char, max + 3);
	if (entry->info == 0)
	{
		destroyCDKObject (entry);
		return (0);
	}
	cleanChar (entry->info, max + 3, '\0');
	entry->infoWidth = max + 3;

	/* *INDENT-EQLS* Set up the rest of the structure. */
	ScreenOf (entry)             = cdkscreen;
	entry->parent                = cdkscreen->window;
	entry->shadowWin             = 0;
	entry->fieldAttr             = fieldAttr;
	entry->fieldWidth            = fieldWidth;
	entry->filler                = filler;
	entry->hidden                = filler;
	ObjOf (entry)->inputWindow   = entry->fieldWin;
	ObjOf (entry)->acceptsFocus  = TRUE;
	ReturnOf (entry)             = NULL;
	entry->shadow                = shadow;
	entry->screenCol             = 0;
  entry->sbuch=0;
	entry->leftChar              = 0;
  entry->lbuch=0;
	entry->min                   = min;
	entry->max                   = max;
	entry->boxWidth              = boxWidth;
	entry->boxHeight             = boxHeight;
	initExitType (entry);
	entry->dispType              = dispType;
	entry->callbackfn            = CDKEntryCallBack;

	/* Do we want a shadow? */
	if (shadow)
	{
		entry->shadowWin = newwin (
				boxHeight,
				boxWidth,
				ypos + 1,
				xpos + 1);
	}

	registerCDKObject (cdkscreen, vENTRY, entry);

	return (entry);
}


void SScroller::scroller_KEY_UP()
{
   if (listSize <= 0 || currentItem <= 0) {
      Beep();
      return;
   }
   currentItem--;
   if (currentHigh) {
      currentHigh--;
   }
   if (currentTop && currentItem < currentTop) {
      currentTop--;
   }
}

void SScroller::scroller_KEY_DOWN()
{
   if (listSize <= 0 || currentItem >= lastItem) {
      Beep();
      return;
   }
   currentItem++;
   if(currentHigh < viewSize - 1) {
      currentHigh++;
   }
   if(currentTop < maxTopItem && currentItem >(currentTop + viewSize - 1)) {
      currentTop++;
   }
}

void SScroller::scroller_KEY_LEFT()
{
   if (listSize <= 0 || leftChar <= 0) {
      Beep();
      return;
   }
   leftChar--;
}

void SScroller::scroller_KEY_RIGHT()
{
   if (listSize <= 0 || leftChar >= maxLeftChar) {
      Beep();
      return;
   }
   leftChar++;
}

void SScroller::scroller_KEY_PPAGE()
{
   int viewSize = viewSize - 1;
   if (listSize <= 0 || currentTop <= 0) {
      Beep();
      return;
   }
   if (currentTop < viewSize) {
      scroller_KEY_HOME();
   } else {
      currentTop -= viewSize;
      currentItem -= viewSize;
   }
}

void SScroller::scroller_KEY_NPAGE()
{
   int viewSize = viewSize - 1;
   if (listSize <= 0 || currentTop >= maxTopItem) {
      Beep();
      return;
   }
   if ((currentTop + viewSize) <= maxTopItem) {
      currentTop += viewSize;
      currentItem += viewSize;
   } else {
      scroller_KEY_END();
   }
}

void SScroller::scroller_KEY_HOME()
{
   currentTop = 0;
   currentItem = 0;
   currentHigh = 0;
}

void SScroller::scroller_KEY_END()
{
   currentTop = maxTopItem;
   currentItem = lastItem;
   currentHigh = viewSize - 1;
}

void SScroller::scroller_FixCursorPosition()
{
   int scrollbarAdj = (scrollbarPlacement == LEFT) ? 1 : 0;
   int ypos = SCREEN_YPOS(this,currentItem - currentTop);
   int xpos = SCREEN_XPOS(this,0) + scrollbarAdj;
   wmove(InputWindowOf(this), ypos, xpos);
   wrefresh(InputWindowOf(this));
}

void SScroller::scroller_SetPosition(int item)
{
   /* item out of band */
   if (item <= 0) {
      scroller_KEY_HOME();
      return;
   }
   /* item out of band */
   if (item >= lastItem) {
      scroller_KEY_END();
      return;
   }
   /* item in first view port */
   if (item < viewSize) {
      currentTop = 0;
   }
   /* item in last view port */
   else if (item >= lastItem - viewSize) {
      currentTop = maxTopItem;
   }
   /* item not in visible view port */
   else if (item < currentTop || item >= currentTop + viewSize) {
      currentTop = item;
   }
   currentItem = item;
   currentHigh = currentItem - currentTop;
}

int SScroller::scroller_MaxViewSize()
{
   return(boxHeight - (2 * BorderOf(this) + TitleLinesOf(this)));
}

void SScroller::scroller_SetViewSize(int size)
{
   int max_view_size = scroller_MaxViewSize();
   viewSize = max_view_size;
   listSize = size;
   lastItem = size - 1;
   maxTopItem = size - viewSize;
   if (size < viewSize) {
      viewSize = size;
      maxTopItem = 0;
   }
   if (listSize > 0 && max_view_size > 0) {
      step = (float)(max_view_size /(double)listSize);
      toggleSize =((listSize > max_view_size) ? 1 : ceilCDK(step));
   } else {
      step = 1;
      toggleSize = 1;
   }
}

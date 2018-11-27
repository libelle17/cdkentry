#ifdef HAVE_XCURSES
#include <xcurses.h>
#ifndef mvwhline
#define mvwhline(win,y,x,c,n)     (wmove(win,y,x) == ERR ? ERR : whline(win,c,n))
#endif
#ifndef mvwvline
#define mvwvline(win,y,x,c,n)     (wmove(win,y,x) == ERR ? ERR : wvline(win,c,n))
#endif
#elif defined(HAVE_NCURSESW_NCURSES_H)
#include <ncursesw/ncurses.h>
#elif defined(HAVE_NCURSES_NCURSES_H)
#include <ncurses/ncurses.h>
#elif defined(HAVE_NCURSES_H)
#include <ncurses.h>
#else
#include <curses.h>
#endif

/*
 * Declare miscellaneous defines.
 */
#define	LEFT		9000
#define	RIGHT		9001
#define	CENTER		9002
#define	TOP		9003
#define	BOTTOM		9004
#define	HORIZONTAL	9005
#define	VERTICAL	9006
#define	FULL		9007

#define NONE		0
#define ROW		1
#define COL		2

#define MAX_BINDINGS	300	/* unused by widgets */
#define MAX_ITEMS	2000	/* unused by widgets */
#define MAX_BUTTONS	200	/* unused by widgets */

#define	MAXIMUM(a,b)	((a) > (b) ? (a) : (b))
#define	MINIMUM(a,b)	((a) < (b) ? (a) : (b))
#define	HALF(a)		((a) >> 1)

#ifndef COLOR_PAIR
#define	COLOR_PAIR(a)	A_NORMAL
#endif

#define ObjOf(ptr)              (&(ptr)->obj)
#define MethodOf(ptr)           (ObjOf(ptr)->fn)
#define ScreenOf(ptr)           (ObjOf(ptr)->screen)
#define WindowOf(ptr)           (ScreenOf(ptr)->window)
#define BorderOf(p)             (ObjOf(p)->borderSize)
#define ResultOf(p)             (ObjOf(p)->resultData)
#define ExitTypeOf(p)           (ObjOf(p)->exitType)
#define EarlyExitOf(p)          (ObjOf(p)->earlyExit)

/* titles */
#define TitleOf(w)              ObjOf(w)->title
#define TitlePosOf(w)           ObjOf(w)->titlePos
#define TitleLenOf(w)           ObjOf(w)->titleLen
#define TitleLinesOf(w)         ObjOf(w)->titleLines

/* line-drawing characters */
#define ULCharOf(w)             ObjOf(w)->ULChar
#define URCharOf(w)             ObjOf(w)->URChar
#define LLCharOf(w)             ObjOf(w)->LLChar
#define LRCharOf(w)             ObjOf(w)->LRChar
#define VTCharOf(w)             ObjOf(w)->VTChar
#define HZCharOf(w)             ObjOf(w)->HZChar
#define BXAttrOf(w)             ObjOf(w)->BXAttr

#define setULCharOf(o,c)        MethodOf(o)->setULcharObj(ObjOf(o),c)
#define setURCharOf(o,c)        MethodOf(o)->setURcharObj(ObjOf(o),c)
#define setLLCharOf(o,c)        MethodOf(o)->setLLcharObj(ObjOf(o),c)
#define setLRCharOf(o,c)        MethodOf(o)->setLRcharObj(ObjOf(o),c)
#define setVTCharOf(o,c)        MethodOf(o)->setVTcharObj(ObjOf(o),c)
#define setHZCharOf(o,c)        MethodOf(o)->setHZcharObj(ObjOf(o),c)
#define setBXAttrOf(o,c)        MethodOf(o)->setBXattrObj(ObjOf(o),c)
#define setBKAttrOf(o,c)        MethodOf(o)->setBKattrObj(ObjOf(o),c)

   /* pre/post-processing */
#define PreProcessFuncOf(w)	(ObjOf(w)->preProcessFunction)
#define PreProcessDataOf(w)	(ObjOf(w)->preProcessData)
#define PostProcessFuncOf(w)	(ObjOf(w)->postProcessFunction)
#define PostProcessDataOf(w)	(ObjOf(w)->postProcessData)
/*
 * Position within the data area of a widget, accounting for border and title.
 */
#define SCREEN_XPOS(w,n) ((n) + BorderOf(w))
#define SCREEN_YPOS(w,n) ((n) + BorderOf(w) + TitleLinesOf(w))

/* The cast is needed because traverse.c wants to use CDKOBJS pointers */
#define ObjPtr(p)           ((CDKOBJS*)(p))

#define MethodPtr(p,m)      ((ObjPtr(p))->fn->m)

/* Use these when we're certain it is a CDKOBJS pointer */
#define ObjTypeOf(p)            MethodPtr(p,objectType)
#define DataTypeOf(p)           MethodPtr(p,returnType)
#define DrawObj(p)              MethodPtr(p,drawObj)         (p,p->box)
#define EraseObj(p)             MethodPtr(p,eraseObj)        (p)
#define DestroyObj(p)           MethodPtr(p,destroyObj)      (p)
#define InjectObj(p,k)          MethodPtr(p,injectObj)       (p,(k))
#define InputWindowObj(p)       MethodPtr(p,inputWindowObj)  (p)
#define FocusObj(p)             MethodPtr(p,focusObj)        (p)
#define UnfocusObj(p)           MethodPtr(p,unfocusObj)      (p)
#define SaveDataObj(p)          MethodPtr(p,saveDataObj)     (p)
#define RefreshDataObj(p)       MethodPtr(p,refreshDataObj)  (p)
#define SetBackAttrObj(p,c)     MethodPtr(p,setBKattrObj)    (p,c)

#define AcceptsFocusObj(p)      (ObjPtr(p)->acceptsFocus)
#define HasFocusObj(p)          (ObjPtr(p)->hasFocus)
#define IsVisibleObj(p)         (ObjPtr(p)->isVisible)
#define InputWindowOf(p)        (ObjPtr(p)->inputWindow)

void Beep();
int floorCDK (double value);
int ceilCDK (double value);


struct CDKOBJS; // CDKOBJS

typedef enum {
      CDKSCREEN_NOEXIT = 0
      , CDKSCREEN_EXITOK
      , CDKSCREEN_EXITCANCEL
} EExitStatus;

typedef union {
   char * valueString;
   int    valueInt;
   float  valueFloat;
   double valueDouble;
   unsigned valueUnsigned;
} CDKDataUnion;

/*
 * This enumerated typedef lists all of the CDK widget types.
 */
typedef enum {	vNULL = 0
		,vALPHALIST
		,vBUTTON
		,vBUTTONBOX
		,vCALENDAR
		,vDIALOG
		,vDSCALE
		,vENTRY
		,vFSCALE
		,vFSELECT
		,vFSLIDER
		,vGRAPH
		,vHISTOGRAM
		,vITEMLIST
		,vLABEL
		,vMARQUEE
		,vMATRIX
		,vMENTRY
		,vMENU
		,vRADIO
		,vSCALE
		,vSCROLL
		,vSELECTION
		,vSLIDER
		,vSWINDOW
		,vTEMPLATE
		,vTRAVERSE
		,vUSCALE
		,vUSLIDER
		,vVIEWER
		} EObjectType;

/*
 * This enumerated typedef lists all the valid display types for
 * the entry, mentry, and template widgets.
 */
typedef enum {	vINVALID = 0
		,vCHAR
		,vHCHAR
		,vINT
		,vHINT
		,vMIXED
		,vHMIXED
		,vUCHAR
		,vLCHAR
		,vUHCHAR
		,vLHCHAR
		,vUMIXED
		,vLMIXED
		,vUHMIXED
		,vLHMIXED
		,vVIEWONLY
		} EDisplayType;

/*
 * This is the prototype for the process callback functions.
 */
typedef int (*PROCESSFN) (
		EObjectType	/* cdktype */,
		void *		/* object */,
		void *		/* clientData */,
		chtype 		/* input */);

/*
 * This is the key binding prototype, typed for use with Perl.
 */
#define BINDFN_PROTO(func)  \
	int (func) ( \
		EObjectType	/* cdktype */, \
		void *		/* object */, \
		void *		/* clientData */, \
		chtype		/* input */)
typedef BINDFN_PROTO(*BINDFN);

typedef struct CDKBINDING {
   BINDFN       bindFunction;
   void *       bindData;
   PROCESSFN    callbackfn;
} CDKBINDING;


struct CDKOBJS;

/*
 * Define the CDK screen structure.
 */
struct SSCREENP { // SScreen
   WINDOW *		window;
   CDKOBJS**	object; // CDKOBJS
   int			objectCount;	/* last-used index in object[] */
   int			objectLimit;	/* sizeof(object[]) */
   EExitStatus		exitStatus;
   int			objectFocus;	/* focus index in object[] */
};
typedef struct SScreen CDKSCREEN;

/*
 * This enumerated typedef defines the type of exits the widgets
 * recognize.
 */
typedef enum {vEARLY_EXIT, vESCAPE_HIT, vNORMAL, vNEVER_ACTIVATED, vERROR} EExitType;

/*
 * Data common to all objects (widget instances).  This appears first in
 * each widget's struct to allow us to use generic functions in binding.c,
 * cdkscreen.c, position.c, etc.
 */
struct CDKOBJS { // CDKOBJS
   int          screenIndex;
   SSCREENP *  screen;
   //const CDKFUNCS * fn;
   bool      box;
   int          borderSize;
   bool      acceptsFocus;
   bool      hasFocus;
   bool      isVisible;
   WINDOW *     inputWindow;
   void *       dataPtr;
   CDKDataUnion resultData;
   unsigned     bindingCount;
   CDKBINDING * bindingList;
   /* title-drawing */
   chtype **	title;
   int *	titlePos;
   int *	titleLen;
   int		titleLines;
   /* line-drawing (see 'box') */
   chtype       ULChar;		/* lines: upper-left */
   chtype       URChar;		/* lines: upper-right */
   chtype       LLChar;		/* lines: lower-left */
   chtype       LRChar;		/* lines: lower-right */
   chtype       VTChar;		/* lines: vertical */
   chtype       HZChar;		/* lines: horizontal */
   chtype       BXAttr;
   /* events */
   EExitType	exitType;
   EExitType	earlyExit;
   PROCESSFN	preProcessFunction;
   void *	preProcessData;
   PROCESSFN	postProcessFunction;
   void *	postProcessData;
   //EObjectType  objectType;
   //CDKDataType  returnType;
	 virtual void drawObj(bool);
	 virtual void eraseObj();
	 virtual void moveObj(int,int,bool,bool);
	 virtual void injectObj(chtype);
	 virtual void focusObj();
	 virtual void unfocusObj();
	 virtual void saveDataObj();
	 virtual void refreshDataObj();
	 virtual void destroyObj();
   /* line-drawing */
	 virtual void setULcharObj(chtype);
	 virtual void setURcharObj(chtype);
	 virtual void setLLcharObj(chtype);
	 virtual void setLRcharObj(chtype);
	 virtual void setVTcharObj(chtype);
	 virtual void setHZcharObj(chtype);
	 virtual void setBXattrObj(chtype);
   /* background attribute */
	 virtual void setBKattrObj(chtype);
	 CDKOBJS();
}; // struct CDKOBJS

/*
 * Define the CDK entry widget structure.
 */
typedef struct SEntry CDKENTRY;
typedef void (*ENTRYCB) (struct SEntry *entry, chtype character);

struct SEntry {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   WINDOW *	labelWin;
   WINDOW *	fieldWin;
   chtype *	label;
   int		labelLen;
	 int		labelumlz; // GSchade
   int		titleAdj;
   chtype	fieldAttr;
   int		fieldWidth;
   char *	info;
   int		infoWidth;
   int		screenCol;
   int    sbuch; // GSchade
   int		leftChar;
   int    lbuch; // GSchade
   int		min;
   int		max;
   int		boxWidth;
   int		boxHeight;
   void settoend(); // GSchade
   void schreibl(chtype); // GSchade, callbackfn
   void zeichneFeld(); // GSchade
   EExitType	exitType;
   EDisplayType dispType;
   bool	shadow;
   chtype	filler;
   chtype	hidden;
   ENTRYCB	callbackfn;
   void		*callbackData;
	 /*
		* This creates a pointer to a new CDK entry widget.
		*/
	 SEntry(
			 CDKSCREEN *	/* cdkscreen */,
			 int		/* xpos */,
			 int		/* ypos */,
			 const char *	/* title */,
			 const char *	/* label */,
			 chtype		/* fieldAttrib */,
			 chtype		/* filler */,
			 EDisplayType	/* disptype */,
			 int		/* fieldWidth */,
			 int		/* min */,
			 int		/* max */,
			 bool   /* Box */,
			 bool		/* shadow */,
			 // GSchade 17.11.18
			 int highnr/*=0*/
			 // Ende GSchade 17.11.18
			 );

};


struct SScroller {
	/* This field must stay on top */
	CDKOBJS  obj; 
	WINDOW * parent; 
	WINDOW * win; 
	WINDOW * scrollbarWin; 
	WINDOW * shadowWin; 
	int      titleAdj;   /* unused */ 
	chtype **    item; 
	int *    itemLen; 
	int *    itemPos; 

	int      currentTop; 
	int      currentItem; 
	int      currentHigh; 

	int      maxTopItem; 
	int      maxLeftChar; 
	int      maxchoicelen; 
	int      leftChar; 
	int      lastItem; 
	int      listSize; 
	int      boxWidth; 
	int      boxHeight; 
	int      viewSize; 

	int      scrollbarPlacement; 
	bool  scrollbar; 
	int      toggleSize; /* size of scrollbar thumb/toggle */ 
	int      togglePos; /* position of scrollbar thumb/toggle */ 
	float    step; /* increment for scrollbar */ 

	EExitType    exitType; 
	bool  shadow; 
	chtype   highlight;
	void scroller_KEY_UP();
	void scroller_KEY_DOWN();
	void scroller_KEY_LEFT();
	void scroller_KEY_RIGHT();
	void scroller_KEY_PPAGE();
	void scroller_KEY_NPAGE();
	void scroller_KEY_HOME();
	void scroller_KEY_END();
	void scroller_FixCursorPosition();
	void scroller_SetPosition(int item);
	int scroller_MaxViewSize();
	void scroller_SetViewSize(int size);
};
typedef struct SScroller CDKSCROLLER;


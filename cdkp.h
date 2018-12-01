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

#include <string.h> // strlen
#include <stdlib.h> // malloc

#define CDK_CONST /*nothing*/
#define CDK_CSTRING CDK_CONST char *
#define CDK_CSTRING2 CDK_CONST char * CDK_CONST *
#define CDK_PATCHDATE 20180306
#define CDK_VERSION "5.0"
#define HAVE_DIRENT_H 1
#define HAVE_GETBEGX 1
#define HAVE_GETBEGY 1
#define HAVE_GETCWD 1
#define HAVE_GETLOGIN 1
#define HAVE_GETMAXX 1
#define HAVE_GETMAXY 1
#define HAVE_GETOPT_H 1
#define HAVE_GETOPT_HEADER 1
#define HAVE_GRP_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_LSTAT 1
#define HAVE_MEMORY_H 1
#define HAVE_MKTIME 1
#define HAVE_NCURSES_H 1
#define HAVE_PWD_H 1
#define HAVE_SETLOCALE 1
#define HAVE_SLEEP 1
#define HAVE_START_COLOR 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TERM_H 1
#define HAVE_TYPE_CHTYPE 1
#define HAVE_UNCTRL_H 1
#define HAVE_UNISTD_H 1
#define MIXEDCASE_FILENAMES 1
#define NCURSES 1
#define PACKAGE "cdk"
#define STDC_HEADERS 1
#define SYSTEM_NAME "linux-gnu"
#define TYPE_CHTYPE_IS_SCALAR 1
#define setbegyx(win,y,x) ((win)->_begy = (y), (win)->_begx = (x), OK)
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
/*
 * This header file adds some useful curses-style definitions.
 */

#undef CTRL
#define CTRL(c)		((c)&0x1f)

#undef  CDK_REFRESH
#define CDK_REFRESH	CTRL('L')
#undef  CDK_PASTE
#define CDK_PASTE	CTRL('V')
#undef  CDK_COPY
#define CDK_COPY	CTRL('Y')
#undef  CDK_ERASE
#define CDK_ERASE	CTRL('U')
#undef  CDK_CUT
#define CDK_CUT		CTRL('X')
#undef  CDK_BEGOFLINE
#define CDK_BEGOFLINE	CTRL('A')
#undef  CDK_ENDOFLINE
#define CDK_ENDOFLINE	CTRL('E')
#undef  CDK_BACKCHAR
#define CDK_BACKCHAR	CTRL('B')
#undef  CDK_FORCHAR
#define CDK_FORCHAR	CTRL('F')
#undef  CDK_TRANSPOSE
#define CDK_TRANSPOSE	CTRL('T')
#undef  CDK_NEXT
#define CDK_NEXT	CTRL('N')
#undef  CDK_PREV
#define CDK_PREV	CTRL('P')
#undef  SPACE
#define SPACE		' '
#undef  DELETE
#define DELETE		'\177'	/* Delete key				*/
#undef  TAB
#define TAB		'\t'	/* Tab key.				*/
#undef  KEY_ESC
#define KEY_ESC		'\033'	/* Escape Key.				*/
#undef  KEY_RETURN
#define KEY_RETURN	'\012'	/* Return key				*/
#undef  KEY_TAB
#define KEY_TAB		'\t'	/* Tab key				*/
#undef  KEY_F1
#define KEY_F1          KEY_F(1)
#undef  KEY_F2
#define KEY_F2          KEY_F(2)
#undef  KEY_F3
#define KEY_F3          KEY_F(3)
#undef  KEY_F4
#define KEY_F4          KEY_F(4)
#undef  KEY_F5
#define KEY_F5          KEY_F(5)
#undef  KEY_F6
#define KEY_F6          KEY_F(6)
#undef  KEY_F7
#define KEY_F7          KEY_F(7)
#undef  KEY_F8
#define KEY_F8          KEY_F(8)
#undef  KEY_F9
#define KEY_F9          KEY_F(9)
#undef  KEY_F10
#define KEY_F10		KEY_F(10)
#undef  KEY_F11
#define KEY_F11		KEY_F(11)
#undef  KEY_F12
#define KEY_F12		KEY_F(12)

#define KEY_ERROR       ((chtype)ERR)


//#define ObjOf(ptr)              (&(ptr)->obj)
#define ObjOf(ptr)              (ptr)
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


#define typeCallocN(type,n)     (type*)calloc((size_t)(n), sizeof(type))
#define typeCalloc(type)        typeCallocN(type,1)

#define typeReallocN(type,p,n)  (type*)realloc(p, (size_t)(n) * sizeof(type))

#define typeMallocN(type,n)     (type*)malloc((size_t)(n) * sizeof(type))
#define typeMalloc(type)        typeMallocN(type,1)

#define freeChecked(p)          if ((p) != 0) free (p)
#define freeAndNull(p)          if ((p) != 0) { free (p); p = 0; }

#define isChar(c)               ((int)(c) >= 0 && (int)(c) < KEY_MIN)
#define CharOf(c)               ((unsigned char)(c))

#define SIZEOF(v)               (sizeof(v)/sizeof((v)[0]))

#define MAX_COLORS		8

#define L_MARKER '<'
#define R_MARKER '>'
#define DigitOf(c) ((c)-'0')
#define ReturnOf(p)   (ObjPtr(p)->dataPtr)

/*
 * Hide details of modifying widget->exitType
 */
#define storeExitType(d)	ObjOf(d)->exitType = (d)->exitType
#define initExitType(d)		storeExitType(d) = vNEVER_ACTIVATED
#define setExitType(w,c)	setCdkExitType(ObjOf(w), &((w)->exitType), c)
#define copyExitType(d,s)	storeExitType(d) = ExitTypeOf(s)


#if	!defined(HAVE_GETMAXYX) && !defined(getmaxyx)
#define getmaxyx(win,y,x)	(y = (win)?(win)->_maxy:ERR, x = (win)?(win)->_maxx:ERR)
#endif
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

#define unknownString   (char *)0
#define unknownInt      (-1)
#define unknownFloat    (0.0)
#define unknownDouble   (0.0)
#define unknownUnsigned (0)

/*
 * This enumerated typedef lists all of the CDK widget types.
 */
enum EObjectType
{	vNULL = 0
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
		} ;

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
struct SScreen { // SScreen
   WINDOW *		window;
   CDKOBJS**	object; // CDKOBJS
   int			objectCount;	/* last-used index in object[] */
   int			objectLimit;	/* sizeof(object[]) */
   EExitStatus		exitStatus;
   int			objectFocus;	/* focus index in object[] */
};

/*
 * This enumerated typedef defines the type of exits the widgets
 * recognize.
 */
typedef enum {vEARLY_EXIT, vESCAPE_HIT, vNORMAL, vNEVER_ACTIVATED, vERROR} EExitType;

int getmaxxf(WINDOW *win);
int getmaxyf(WINDOW *win);

void Beep();
int floorCDK (double value);
int ceilCDK (double value);
int setWidgetDimension (int parentDim, int proposedDim, int adjustment);
static int encodeAttribute (const char *string, int from, chtype *mask);
chtype *char2Chtypeh(const char *string, int *to, int *align, int highinr=0);
char **CDKsplitString (const char *string, int separator);
static unsigned countChar (const char *string, int separator);
unsigned CDKcountStrings (CDK_CSTRING2 list);
chtype *char2Chtype(const char *string, int *to, int *align);
int chlen(const chtype *string);
void freeChtype (chtype *string);
int justifyString (int boxWidth, int mesgLength, int justify);
void CDKfreeStrings (char **list);
void CDKfreeChtypes (chtype **list);
void alignxy (WINDOW *window, int *xpos, int *ypos, int boxWidth, int boxHeight);
void cleanChar (char *s, int len, char character);
void writeChtype(WINDOW *window, int xpos, int ypos, chtype *string, int align, int start, int end);
void writeChtypeAttrib (WINDOW *window, int xpos, int ypos, chtype *string, chtype attr, int align, int start, int end);
void attrbox (WINDOW *win, chtype tlc, chtype trc, chtype blc, chtype brc, chtype horz, chtype vert, chtype attr);
void drawShadow (WINDOW *shadowWin);
int getcCDKBind(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData GCC_UNUSED, chtype input GCC_UNUSED);

typedef struct SScreen CDKSCREEN;
void registerCDKObject (CDKSCREEN *screen, EObjectType cdktype, void *object);


/*
 * Data common to all objects (widget instances).  This appears first in
 * each widget's struct to allow us to use generic functions in binding.c,
 * cdkscreen.c, position.c, etc.
 */
struct CDKOBJS 
{ 
   int          screenIndex;
   SScreen *  screen;
	 EObjectType cdktype; 
	 CDKOBJS* bindableObject=0; 
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
	 virtual int injectObj(chtype);
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
	 ~CDKOBJS();
	 int setCdkTitle(const char *title, int boxWidth);
	 void drawCdkTitle(WINDOW *);
	 void cleanCdkTitle();
	 bool validObjType(EObjectType type);
	 //CDKOBJS * bindableObject (EObjectType * cdktype, void *object);
	 void registerCDKObject(CDKSCREEN *screen, EObjectType cdktype);
	 void setScreenIndex(CDKSCREEN *pscreen, int number);
	 void drawObjBox(WINDOW *win);
	 int getcCDKObject();
	 int getchCDKObject(bool *functionKey);
}; // struct CDKOBJS

/*
 * Define the CDK entry widget structure.
 */

struct SEntry:CDKOBJS 
{
//   CDKOBJS	obj;
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
	 void _drawCDKEntry (bool Box);
	 //void drawObj(bool);
	 int injectObj(chtype);
	 char * activateCDKEntry(chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/, int obpfeil/*=0*/);
	 void setCDKEntryBox (bool Box);
	 void CDKEntryCallBack(chtype character);
	 void (SEntry::*callbfn)(chtype character)=NULL;
}; // struct SEntry:CDKOBJS
typedef struct SEntry CDKENTRY;

struct SScroll_basis:CDKOBJS 
{
	/* This field must stay on top */
//	CDKOBJS  obj; 
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
	virtual void nix()=0;
};

struct SScroller:SScroll_basis
{
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

/*
 * Declare scrolling list definitions.
 */
struct SScroll:SScroll_basis 
{
   bool	numbers;	/* */
   chtype	titlehighlight;	/* */
	 WINDOW	*listWin;
	 SScroll(
			 CDKSCREEN *	/* cdkscreen */,
			 int		/* xpos */,
			 int		/* ypos */,
			 int		/* spos */,
			 int		/* height */,
			 int		/* width */,
			 const char *	/* title */,
			 CDK_CSTRING2	/* itemList */,
			 int		/* items */,
			 bool		/* numbers */,
			 chtype		/* highlight */,
			 bool		/* Box */,
			 bool		/* shadow */);
};
typedef struct SScroll CDKSCROLL;


struct SFileSelector:CDKOBJS
{
	SFileSelector();
}; // struct SFileSelector:CDKOBJS
typedef struct SFileSelector CDKFSELECT;

struct SAlphalist:CDKOBJS
{
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   CDKENTRY *	entryField;
   CDKSCROLL *	scrollField;
   char **	list;
   int		listSize;
   int		xpos;
   int		ypos;
   int		height;
   int		width;
   int		boxHeight;
   int		boxWidth;
   chtype	highlight;
   chtype	fillerChar;
   bool	shadow;
   EExitType	exitType;
	 SAlphalist();
};
typedef struct SAlphalist CDKALPHALIST;


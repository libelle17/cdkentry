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

struct CDKOP; // CDKOBJS

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
   CDKOP**	object; // CDKOBJS
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
struct CDKOP { // CDKOBJS
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
   /* pre/post-processing */
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
	 CDKOP();
};

#define NCURSES_INTERNALS // fuer openSuse, nicht ubuntu

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
#include <map> // bindv
//#include <set> // plist
#include <vector> // vector<chtstr> titles

//#include "cdk_test.h"
#ifndef CDKINCLUDES
#ifndef CDK_TEST_H
#define CDK_TEST_H
/*
 * The whole point of this header is to define ExitProgram(), which is used for
 * leak-checking when ncurses's _nc_free_and_exit() function is available. 
 * Invoking that rather than 'exit()' tells ncurses to free all of the
 * "permanent" memory leaks, making analysis much simpler.
 */
#ifdef HAVE_NC_ALLOC_H

#ifndef HAVE_LIBDBMALLOC
#define HAVE_LIBDBMALLOC 0
#endif

#ifndef HAVE_LIBDMALLOC
#define HAVE_LIBDMALLOC 0
#endif

#ifndef HAVE_LIBMPATROL
#define HAVE_LIBMPATROL 0
#endif

#include <nc_alloc.h>

#else

#if defined(NCURSES_VERSION) && defined(HAVE__NC_FREE_AND_EXIT)
/* nc_alloc.h normally not installed */
extern void _nc_free_and_exit(int) GCC_NORETURN;
#define ExitProgram(code) _nc_free_and_exit(code)
#endif

#endif /* HAVE_NC_ALLOC_H */

#include <pwd.h> // getpwd
#include <grp.h> // getgrgid
#include <time.h> // ctime

#ifndef ExitProgram
#define ExitProgram(code) exit(code)
#endif

#endif /* CDK_TEST_H */
#endif /* CDKINCLUDES */
// GSchade 17.11.18
enum einbauart {
	einb_direkt,
	einb_alphalist,
	einb_sonst
};
extern einbauart akteinbart;

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

#define freeChecked(p)          if ((p) != 0) free (p)
#define freeAndNull(p)          if ((p) != 0) { free (p); p = 0; }
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

#define NUMBER_FMT      "%4d. %s"
#define NUMBER_LEN(s)   (8 + strlen (s))

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
//#define MethodOf(ptr)           (ObjOf(ptr)->fn)
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

//#define MethodPtr(p,m)      ((ObjPtr(p))->fn->m)
//#define MethodPtr(p,m)      ((ObjPtr(p))->m)

/* Use these when we're certain it is a CDKOBJS pointer */
/*
#define ObjTypeOf(p)            MethodPtr(p,objectType)
#define DataTypeOf(p)           MethodPtr(p,returnType)
#define DrawObj(p)              MethodPtr(p,drawObj)         (p,p->obbox)
#define EraseObj(p)             MethodPtr(p,eraseObj)        (p)
#define DestroyObj(p)           MethodPtr(p,destroyObj)      (p)
#define InjectObj(p,k)          MethodPtr(p,injectObj)       (p,(k))
#define InputWindowObj(p)       MethodPtr(p,inputWindowObj)  (p)
#define FocusObj(p)             MethodPtr(p,focusObj)        (p)
#define UnfocusObj(p)           MethodPtr(p,unfocusObj)      (p)
#define SaveDataObj(p)          MethodPtr(p,saveDataObj)     (p)
#define RefreshDataObj(p)       MethodPtr(p,refreshDataObj)  (p)
#define SetBackAttrObj(p,c)     MethodPtr(p,setBKattrObj)    (p,c)
*/

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
// #define CharOf(c)               ((unsigned char)(c))

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
// #define setExitType(w,c)	setCdkExitType(ObjOf(w), &((w)->exitType), c)
#define copyExitType(d,s)	storeExitType(d) = ExitTypeOf(s)
/*
 * Use this if checkCDKObjectBind() returns true, use this function to
 * decide if the exitType should be set as a side-effect.
 */
#define checkEarlyExit(w)	if (EarlyExitOf(w) != vNEVER_ACTIVATED) \
				    storeExitType(w) = EarlyExitOf(w)

/*
 * Macros to check if caller is attempting to make the widget as high (or wide)
 * as the screen.
 */
#define isFullWidth(n)		((n) == FULL || (COLS != 0 && ((n) >= COLS)))
#define isFullHeight(n)		((n) == FULL || (LINES != 0 && ((n) >= LINES)))

/*
 * These set the drawing characters of the widget.
 */
#define setCDKEntryULChar(w,c)             setULCharOf(w,c)
#define setCDKEntryURChar(w,c)             setURCharOf(w,c)
#define setCDKEntryLLChar(w,c)             setLLCharOf(w,c)
#define setCDKEntryLRChar(w,c)             setLRCharOf(w,c)
#define setCDKEntryVerticalChar(w,c)       setVTCharOf(w,c)
#define setCDKEntryHorizontalChar(w,c)     setHZCharOf(w,c)
#define setCDKEntryBoxAttribute(w,c)       setBXAttrOf(w,c)

#if	!defined(HAVE_GETMAXYX) && !defined(getmaxyx)
#define getmaxyx(win,y,x)	(y = (win)?(win)->_maxy:ERR, x = (win)?(win)->_maxx:ERR)
#endif

#define	NONUMBERS	FALSE
#define	NUMBERS		TRUE

#define SCREENPOS(w,n) (w)->itemPos[n] - (w)->leftChar	/* + scrollbarAdj + BorderOf(w) */


struct CDKOBJS; // CDKOBJS

enum EExitStatus 
{
	CDKSCREEN_NOEXIT = 0
		, CDKSCREEN_EXITOK
		, CDKSCREEN_EXITCANCEL
};

union CDKDataUnion {
   char const * valueString;
   int    valueInt;
   float  valueFloat;
   double valueDouble;
   unsigned valueUnsigned;
};

#define unknownString   (char *)0
#define unknownInt      (-1)
#define unknownFloat    (0.0)
#define unknownDouble   (0.0)
#define unknownUnsigned (0)

/*
 * This injects a single character into the menu widget.
 */
//#define injectCDKObject(o,c,type)      (MethodOf(o)->injectObj    (ObjOf(o),c) ? ResultOf(o).value ## type : unknown ## type)
#define injectCDKObject(/*o,*/c,type)      (injectObj    (/*ObjOf(o),*/c) ? ResultOf(this).value ## type : unknown ## type)
#define injectCDKMenu(/*obj,*/input) injectCDKObject(/*obj,*/input,Int)

/*
 * This enumerated typedef lists all of the CDK widget types.
 */
enum EObjectType
{
	vNULL = 0
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
};

/*
 * This enumerated typedef lists all the valid display types for
 * the entry, mentry, and template widgets.
 */
enum EDisplayType 
{	
	vINVALID = 0
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
};

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
#ifdef false
#define BINDFN_PROTO(func)  \
	int (func) ( \
		EObjectType	/* cdktype */, \
		void *		/* object */, \
		void *		/* clientData */, \
		chtype		/* input */)
typedef BINDFN_PROTO(*BINDFN);
#endif
typedef int (*BINDFN) (EObjectType,void*,void*,chtype);

struct CDKBINDING {
   BINDFN       bindFunction;
   void *       bindData;
   PROCESSFN    callbackfn;
	 CDKBINDING(BINDFN f,void* d):bindFunction(f),bindData(d){}
	 CDKBINDING(){}
};

EDisplayType char2DisplayType (const char *string);
bool isHiddenDisplayType (EDisplayType type);
int filterByDisplayType (EDisplayType type, chtype input);

// typedef struct _all_objects { struct _all_objects *link; CDKOBJS *object; } ALL_OBJECTS;
typedef bool (*CHECK_KEYCODE)(int /* keyCode */, int /* functionKey */);
/*
 * Define the CDK screen structure.
 */
struct SScreen 
{ // SScreen
   WINDOW *		window;
#ifdef pneu
	 std::vector<CDKOBJS*> object;// CDKOBJS
#else
   CDKOBJS**	object; // CDKOBJS
   int			objectLimit;	/* sizeof(object[]) */
#endif
   int			objectCount;	/* last-used index in object[] */
   EExitStatus		exitStatus;
   int			objectFocus;	/* focus index in object[] */
	 void eraseCDKScreen();
	 CDKOBJS* setCDKFocusNext();
	 CDKOBJS* setCDKFocusPrevious();
	 CDKOBJS* setCDKFocusCurrent(/*SScreen *screen, */CDKOBJS *newobj);
	 CDKOBJS* setCDKFocusFirst(/*SScreen *screen*/);
	 /*CDKOBJS **/void setCDKFocusLast(/*SScreen *screen*/);
	 int getFocusIndex();
	 void setFocusIndex(int value);
	 SScreen(WINDOW *window);
	 void swapCDKIndices(/*SScreen *screen, */int n1, int n2);
	 void destroyCDKScreenObjects();
	 void destroyCDKScreen();
	 CDKOBJS* getCDKFocusCurrent();
	 CDKOBJS* handleMenu(/*SScreen *screen, */CDKOBJS *menu, CDKOBJS *oldobj);
	 void saveDataCDKScreen(/*SScreen *screen*/);
	 void refreshDataCDKScreen(/*SScreen *screen*/);
	 void resetCDKScreen(/*SScreen *screen*/);
	 void exitOKCDKScreen(/*SScreen *screen*/);
	 void exitCancelCDKScreen(/*SScreen *screen*/);
	 void traverseCDKOnce(/*SScreen *screen,*/ CDKOBJS *curobj, int keyCode, bool functionKey, CHECK_KEYCODE funcMenuKey);
	 int traverseCDKScreen(/*SScreen *screen*/);
	 void popupLabel (/*SScreen *screen, */
#ifdef pneu
			 std::vector<std::string> mesg
#else
			 CDK_CSTRING2 mesg, int count
#endif
			 );
	 void popupLabelAttrib (/*SScreen *screen, */
#ifdef pneu
			 std::vector<std::string> mesg
#else
			 CDK_CSTRING2 mesg, int count
#endif
			 , chtype attrib);
	 virtual void refreshCDKScreen();
};

/*
 * This enumerated typedef defines the type of exits the widgets
 * recognize.
 */
enum EExitType 
{
	vEARLY_EXIT
		, vESCAPE_HIT
		, vNORMAL
		, vNEVER_ACTIVATED
		, vERROR
} ;

int getmaxxf(WINDOW *win);
int getmaxyf(WINDOW *win);

void Beep();
int floorCDK (double value);
int ceilCDK (double value);
int setWidgetDimension(int parentDim, int proposedDim, int adjustment);
static int encodeAttribute(const char *string, int from, chtype *mask);
#ifdef pneu
void aufSplit(std::vector<std::string> *tokens, const char* const text, const char sep=' ',bool auchleer=1);
void aufSplit(std::vector<std::string> *tokens, const std::string& text, const char sep=' ',bool auchleer=1);
#else
char **CDKsplitString(const char *string, int separator);
static unsigned countChar(const char *string, int separator);
unsigned CDKcountStrings(CDK_CSTRING2 list);
#endif
chtype *char2Chtype(const char *string, int *to, int *align);
int chlen(const chtype *string);
void freeChtype(chtype *string);
int justifyString(int boxWidth, int mesgLength, int justify);
void CDKfreeStrings(char **list);
void CDKfreeChtypes(chtype **list);
void alignxy(WINDOW *window, int *xpos, int *ypos, int boxWidth, int boxHeight);
void cleanChar(char *s, int len, char character);
void writeChtype(WINDOW *window, int xpos, int ypos, const chtype *const string, int align, int start, int end);
void writeChtypeAttrib(WINDOW *window, int xpos, int ypos, const chtype *const string, chtype attr, int align, int start, int end);
void attrbox(WINDOW *win, chtype tlc, chtype trc, chtype blc, chtype brc, chtype horz, chtype vert, chtype attr);
void drawShadow (WINDOW *shadowWin);
int getcCDKBind(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData GCC_UNUSED, chtype input GCC_UNUSED);
void refreshCDKWindow(WINDOW *win);
#ifdef pneu
#else
char *copyChar(const char *original);
chtype *copyChtype(const chtype *original);
#endif
void eraseCursesWindow(WINDOW *window);
void deleteCursesWindow(WINDOW *window);
void moveCursesWindow(WINDOW *window, int xdiff, int ydiff);
bool isHiddenDisplayType(EDisplayType type);
int comparSort(const void *a, const void *b);
void sortList(CDK_CSTRING *list, int length);
static int adjustAlphalistCB(EObjectType objectType GCC_UNUSED, void
			      *object GCC_UNUSED,
			      void *clientData,
			      chtype key);
static int completeWordCB(EObjectType objectType GCC_UNUSED, void *object GCC_UNUSED,
			   void *clientData,
			   chtype key GCC_UNUSED);
#ifdef pneu
#else
char *chtype2Char (const chtype *string);
#endif
int searchList(
//#define pneu
#ifdef pneu
		std::vector<std::string> *plistp,
#else
		CDK_CSTRING2 list, int listSize, 
#endif
		const char *pattern);
// unsigned CDKallocStrings (char ***list, char *item, unsigned length, unsigned used);
#ifdef pneu
void
#else
unsigned 
#endif
CDKallocStrings(
#ifdef pneu
		std::vector<std::string> *plistp,
#else
		char ***list, 
#endif
		char *item
#ifdef pneu
#else
		, unsigned length, unsigned used
#endif
		);
void writeBlanks(WINDOW *window, int xpos, int ypos, int align, int start, int end);
void writeChar(WINDOW *window, int xpos, int ypos, const char *string, int align, int start, int end);
void writeCharAttrib(WINDOW *window, int xpos, int ypos, const char *string, chtype attr, int align, int start, int end);
static bool checkMenuKey(int keyCode, int functionKey);
CDKOBJS* switchFocus(CDKOBJS *newobj, CDKOBJS *oldobj);
#ifdef pneu
#else
char **copyCharList (const char **list);
#endif
int lenCharList(const char **list);
void initCDKColor(void);
void endCDK(void);
static char *errorMessage (const char *format);
void freeCharList (char **list, unsigned size);
static int displayFileInfoCB (EObjectType objectType GCC_UNUSED, void *object, void *clientData, chtype key GCC_UNUSED);
int mode2Char (char *string, mode_t mode);

//typedef struct SScreen CDKSCREEN;

struct _all_screens
{
   struct _all_screens *link;
   SScreen *screen;
};
// ALL_SCREENS;

// chtype string
class chtstr
{
	private:
	chtype *inh;
	char *ch=0;
	size_t len;
	public:
	void gibaus() const;
//	chtstr(size_t len);
	// chtype *char2Chtypeh(const char *string, int *to, int *align, int highinr=0);
	chtstr(const char *string, int *to, int *align, const int highnr=0);
	int rauskopier(chtype **ziel);
#ifdef pneu
	char *chtype2Char();
#endif
	inline chtype *getinh() const { return inh; }
	inline size_t getlen() const { return len; }
};



void registerCDKObject(SScreen *screen, EObjectType cdktype, void *object);

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
	 //const CDKFUNCS * fn;
   bool      obbox;
   int          borderSize;
   bool      acceptsFocus;
   bool      hasFocus;
   bool      isVisible;
   WINDOW *     inputWindow;
   void *       dataPtr;
   CDKDataUnion resultData;
#define bneu
#ifdef bneu
	 std::map<chtype,CDKBINDING> bindv;
	 std::map<chtype,CDKBINDING>::const_iterator bindvit;
#else
   unsigned     bindingCount=0;
   CDKBINDING * bindingList=0;
#endif
   /* title-drawing */
//   chtype **	title=0;
	 std::vector<chtstr> titles;
   int *	titlePos=0;
   int *	titleLen=0;
   int		titleLines=0;
   /* line-drawing (see 'obbox') */
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
   PROCESSFN	preProcessFunction=0;
   void *	preProcessData=0;
   PROCESSFN	postProcessFunction=0;
   void *	postProcessData=0;
   // EObjectType  objectType;
   //CDKDataType  returnType;
	 virtual void drawObj(bool);
	 virtual void eraseObj();
	 virtual void destroyObj();
	 virtual void focusObj(){};
	 virtual void unfocusObj(){};
	 virtual void setFocus();
	 virtual int injectObj(chtype){return 0;};
	 /*
	 virtual void moveObj(int,int,bool,bool);
	 virtual void saveDataObj();
	 virtual void refreshDataObj();
	 */
   // line-drawing 
	 virtual void setULcharObj(chtype);
	 /*
	 virtual void setURcharObj(chtype);
	 virtual void setLLcharObj(chtype);
	 virtual void setLRcharObj(chtype);
	 virtual void setVTcharObj(chtype);
	 virtual void setHZcharObj(chtype);
	 virtual void setBXattrObj(chtype);
	 */
	 void setBox(bool Box);
   // background attribute
	 virtual void setBKattrObj(chtype);
	 void refreshDataCDK();
	 virtual void saveDataCDK();
	 void drawCDKScreen();
	 virtual CDKOBJS* bindableObject();
	 void bindCDKObject(chtype key, BINDFN function, void *data);
	 void unbindCDKObject(chtype key);
	 void cleanCDKObjectBindings();
	 int checkCDKObjectBind(chtype key);
	 bool isCDKObjectBind(chtype key);
	 //	 void setCdkExitType(chtype ch);
	 void setExitType(chtype ch);
	 void setCDKObjectPreProcess (/*CDKOBJS *obj, */PROCESSFN fn, void *data);
	 void setCDKObjectPostProcess (/*CDKOBJS *obj, */PROCESSFN fn, void *data);
	 CDKOBJS();
	 ~CDKOBJS();
	 void unregisterCDKObject(EObjectType cdktype/*, void *object*/);
	 void destroyCDKObject(/*CDKOBJS *obj*/);
	 int setCdkTitle(/*CDKOBJS *obj, */const char *title, int boxWidth);
	 void drawCdkTitle(WINDOW *);
	 void cleanCdkTitle();
	 bool validObjType(EObjectType type);
	 void registerCDKObject(SScreen *screen, EObjectType cdktype);
	 void reRegisterCDKObject(EObjectType cdktype/*, void *object*/);
#ifdef pneu
	 void setScreenIndex(SScreen *pscreen);
#else
	 void setScreenIndex(SScreen *pscreen, int number);
#endif
	 void drawObjBox(WINDOW *win);
	 int getcCDKObject();
	 int getchCDKObject(bool *functionKey);
	 void raiseCDKObject(EObjectType cdktype/*, void *object*/);
	 void lowerCDKObject(EObjectType cdktype/*, void *object*/);
	 void unsetFocus();
	 void exitOKCDKScreenOf(/*CDKOBJS *obj*/);
	 void exitCancelCDKScreenOf(/*CDKOBJS *obj*/);
	 void resetCDKScreenOf(/*CDKOBJS *obj*/);
	 void setCDKObjectBackgroundColor (/*CDKOBJS *obj, */const char *color);
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
//   chtype *	label;
	 chtstr *labelp=0;
   int		labelLen;
	 int		labelumlz; // GSchade
   int		titleAdj;
   chtype	fieldAttr;
   int		fieldWidth;
#ifdef pneu
#define ineu
#endif
#ifdef ineu
	 std::string efld/*info*/;
#else
   char *	efld/*info*/;
#endif
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
	 void setCDKEntry(const char *value, int min, int max, bool Box GCC_UNUSED);
	 const char* getCDKEntryValue();
	 void setBKattrEntry(chtype attrib);
	 void setBKattrObj(chtype);
	 void setCDKEntryHighlight(chtype highlight, bool cursor);
	 void focusCDKEntry();
	 void focusObj(){focusCDKEntry();}
	 void unfocusCDKEntry();
	 void unfocusObj(){unfocusCDKEntry();}
   EDisplayType dispType;
   bool	shadow;
   chtype	filler;
   chtype	hidden;
#ifdef pneu
	 std::string GPasteBuffer;
#else
	 char *GPasteBuffer = 0;
#endif
	 void		*callbackData;
	 /*
		* This creates a pointer to a new CDK entry widget.
		*/
	 SEntry(
			 SScreen *	/* cdkscreen */,
			 int		/* xpos */,
			 int		/* ypos */,
			 const char *	/* title */,
			 const char *	/* labelstr */,
			 chtype		/* fieldAttrib */,
			 chtype		/* filler */,
			 EDisplayType	/* disptype */,
			 int		/* fWidth */,
			 int		/* min */,
			 int		/* max */,
			 bool   /* Box */,
			 bool		/* shadow */,
			 // GSchade 17.11.18
			 int highnr/*=0*/
			 // Ende GSchade 17.11.18
			 );
	 ~SEntry();
	 void destroyObj(){this->~SEntry();}
	 void drawCDKEntry(bool);
	 void drawObj(bool Box);
	 void cleanCDKEntry();
	 int injectCDKEntry(chtype);
	 int injectObj(chtype ch){return injectCDKEntry(ch);}
	 void setCDKEntryValue(const char *newValue);
	 void eraseCDKEntry();
	 void eraseObj(){eraseCDKEntry();}
	 const char* activateCDKEntry(chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/, int obpfeil/*=0*/);
	 void moveCDKEntry(int,int,bool,bool);
	 void CDKEntryCallBack(chtype character);
	 void (SEntry::*callbfn)(chtype character)=NULL;
}; // struct SEntry:CDKOBJS
// typedef struct SEntry CDKENTRY;

struct SScroll_basis:public CDKOBJS 
{
	/* This field must stay on top */
//	CDKOBJS  obj; 
	WINDOW * parent; 
	WINDOW * win; 
	WINDOW * scrollbarWin; 
	WINDOW * shadowWin; 
	int      titleAdj;   /* unused */ 
#ifdef pneu
	std::vector<chtstr> pitem;
	std::vector<chtstr>::const_iterator piter;
	std::vector<int> itemLen;
	std::vector<int> itemPos;
#else
	chtype **    sitem=0; 
	int *    itemLen=0; 
	int *    itemPos=0; 
#endif

	int      currentTop; 
	int      currentItem; 
	int      currentHigh; 

	int      maxTopItem; 
	int      maxLeftChar; 
	int      maxchoicelen; 
	int      leftChar; 
	int      lastItem; 
#ifdef pneu
#else
#endif
	int      listSize=0; 
	int      boxWidth; 
	int      boxHeight; 
	int      viewSize; 

	int      scrollbarPlacement; 
	bool  scrollbar; 
	int      toggleSize; /* size of scrollbar thumb/toggle */ 
	int      togglePos; /* position of scrollbar thumb/toggle */ 
	float    step; /* increment for scrollbar */ 

	bool  shadow; 
	chtype   highlight;
	void updateViewWidth(int widest);
	int MaxViewSize();
	void SetPosition(int item);
	void scroll_KEY_HOME();
	void scroll_KEY_END();
	void scroll_FixCursorPosition();
	void scroll_KEY_UP();
	void scroll_KEY_DOWN();
	void scroll_KEY_LEFT();
	void scroll_KEY_RIGHT();
	void scroll_KEY_PPAGE();
	void scroll_KEY_NPAGE();
	void setViewSize(int listSize);
};

/*
struct SScroller:SScroll_basis
{
};
typedef struct SScroller CDKSCROLLER;
*/

/*
 * Declare scrolling list definitions.
 */
struct SScroll:SScroll_basis 
{
	bool	numbers;	/* */
	chtype	titlehighlight;	/* */
	WINDOW	*listWin;
	SScroll(
			SScreen *	/* cdkscreen */,
			int		/* xpos */,
			int		/* ypos */,
			int		/* spos */,
			int		/* height */,
			int		/* width */,
			const char *	/* title */,
#ifdef pneu
			 std::vector<std::string> *plistp,
#else
			 CDK_CSTRING2 list/* itemList */,
			 int		/* items */,
#endif
			bool		/* numbers */,
			chtype		/* highlight */,
			bool		/* Box */,
			bool		/* shadow */);
	~SScroll();
	void destroyObj(){this->~SScroll();}
	void eraseCDKScroll/*_eraseCDKScroll*/(/*CDKOBJS *object*/);
	void eraseObj(){eraseCDKScroll();}
	int createCDKScrollItemList(bool numbers, 
#ifdef pneu
						std::vector<std::string> *plistp
#else
				    CDK_CSTRING2 list,
				    int listSize
#endif
			);
#ifdef pneu
#else
	bool allocListArrays(int oldSize, int newSize);
#endif
	bool allocListItem(int which, 
#ifdef pneu
#else
                          			char **work, size_t * used, 
#endif
                                                      			int number, const char *value);
	int injectCDKScroll(/*CDKOBJS *object, */chtype input);
	int injectObj(chtype ch){return injectCDKScroll(ch);}
	void drawCDKScrollList(bool Box);
	int activateCDKScroll(chtype *actions);
	void setCDKScrollPosition(int item);
	void drawCDKScroll(bool Box,bool obmit=0);
	 void drawObj(bool Box);
	 void drawCDKScrollCurrent();
	 void moveCDKScroll(int xplace, int yplace, bool relative, bool refresh_flag);
	 void setCDKScroll(
#ifdef pneu
               			 std::vector<std::string> *plistp,
#else
              			 CDK_CSTRING2 list, int listSize, 
#endif
                                               			 bool numbers, chtype hl, bool Box);
#ifdef pneu
#else
	 int getCDKScrollItems(/*SScroll *scrollp, */char **list);
#endif
	 void setCDKScrollItems(
#ifdef pneu
													 std::vector<std::string> *plistp,
#else
													 CDK_CSTRING2 list, int listSize, 
#endif
																													 bool numbers);
	 void setCDKScrollCurrentTop(/*SScroll *widget, */int item);
	 void setCDKScrollCurrent(int item);
	 void setBKattrScroll(chtype attrib);
	 void setBKattrObj(chtype attrib);
	 //void setCDKScrollBox(/*SScroll *scrollp, */bool Box);
	 //bool getCDKScrollBox();
	 void resequence(/*SScroll *scrollp*/);
#ifdef pneu
#else
	 bool insertListItem (/*SScroll *scrollp, */int item);
#endif
	 void addCDKScrollItem(/*SScroll *scrollp,*/ const char *item);
	 void insertCDKScrollItem(/*SScroll *scrollp, */const char *item);
	 void deleteCDKScrollItem(/*SScroll *scrollp, */int position);
	 void focusCDKScroll(/*CDKOBJS *object*/);
	 void focusObj(){focusCDKScroll();}
	 void unfocusCDKScroll(/*CDKOBJS *object*/);
	 void unfocusObj(){unfocusCDKScroll();}
}; // struct SScroll:SScroll_basis
// typedef struct SScroll CDKSCROLL;


int fselectAdjustScrollCB(EObjectType objectType GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key);
static char *format1String (const char *format, const char *string);
static char *format1StrVal (const char *format, const char *string, int value);
static char *format1Number (const char *format, long value);
static char *format1Date (const char *format, time_t value);
static char *expandTilde (const char *filename);
#ifdef pneu
std::string dirName(std::string path);
std::string dirName(const char* pfad);
#else
char *dirName(const char *pathname);
#endif
static char *trim1Char(char *source);
static char *make_pathname (const char *directory, const char *filename);
char *format3String(const char *format, const char *s1, const char *s2, const char *s3);
int mode2Filetype(mode_t mode);
int CDKgetDirectoryContents(const char *directory, char ***list);
static int preProcessEntryField(EObjectType cdktype GCC_UNUSED, void
				 *object GCC_UNUSED,
				 void *clientData,
				 chtype input);

/*
 * Define the CDK file selector widget structure.
 */
struct SFSelect:CDKOBJS
{
	 int setCDKFselectdirContents(/*SFSelect *fselect*/);
	//   CDKOBJS	obj;
	WINDOW *	parent;
	WINDOW *	win;
	WINDOW *	shadowWin;
	SEntry *	entryField;
	SScroll *	scrollField;
	CDKOBJS* bindableObject();
#ifdef pneu
	std::vector<std::string> dirContents;
	std::string pwd;
#else
	char **	dirContents;
	int		fileCounter;
	char *	pwd;
#endif
#ifdef pneu
	std::string pfadname;
#else
	char *	pathname;
#endif
	int		xpos;
	int		ypos;
	int		boxHeight;
	int		boxWidth;
	chtype	fieldAttribute;
	chtype	fillerCharacter;
	chtype	highlight;
#ifdef pneu
	std::string dirAttribute;
	std::string fileAttribute;
	std::string linkAttribute;
	std::string sockAttribute;
#else
	char *	dirAttribute;
	char *	fileAttribute;
	char *	linkAttribute;
	char *	sockAttribute;
#endif
	bool	shadow;
/*
 * This creates a new CDK file selector widget.
 */
// SFSelect *newCDKFselect (
	SFSelect(
		SScreen*	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		const char *	/* label */,
		chtype		/* fieldAttribute */,
		chtype		/* fillerChar */,
		chtype		/* highlight */,
		const char *	/* dirAttributes */,
		const char *	/* fileAttributes */,
		const char *	/* linkAttribute */,
		const char *	/* sockAttribute */,
		bool		/* Box */,
		bool		/* shadow */,
		int highnr
		);
	~SFSelect();
	void moveCDKFselect(/*CDKOBJS *object, */int xplace, int yplace, bool relative, bool refresh_flag);
	const char *activateCDKFselect(/*SFSelect *fselect, */chtype *actions);
	void destroyObj(){this->~SFSelect();}
	void eraseCDKFselect();
	void eraseObj(){eraseCDKFselect();}
	 void drawCDKFselect(bool Box);
	 void drawMyScroller(/*SFSelect *widget*/);
	 void drawObj(bool Box);
	 void setPWD(/*SFSelect *fselect*/);
	 int injectCDKFselect(chtype input);
	 int injectObj(chtype ch){return injectCDKFselect(ch);}
	 void injectMyScroller(chtype key);
	 void setCDKFselect(/*SFSelect *fselect, */const char *directory, chtype fieldAttrib, chtype filler, chtype highlight, const char *dirAttribute, const char *fileAttribute, const char *linkAttribute, const char *sockAttribute, bool Box GCC_UNUSED);
	 char *contentToPath (/*SFSelect *fselect, */const char *content);
	 void focusCDKFileSelector();
	 void focusObj(){focusCDKFileSelector();}
	 void unfocusCDKFileSelector();
	 void unfocusObj(){unfocusCDKFileSelector();}
}; // struct SFSelect:CDKOBJS
//typedef struct SFSelect CDKFSELECT;

struct SAlphalist:CDKOBJS
{
   WINDOW*	parent;
   WINDOW*	win;
   WINDOW*	shadowWin;
   SEntry*	entryField;
   SScroll*	scrollField;
#ifdef pneu
	 std::vector<std::string> plist;
#else
   char **	slist=0;
   int		listSize;
#endif
   int		xpos;
   int		ypos;
   int		height;
   int		width;
   int		boxHeight;
   int		boxWidth;
   chtype	highlight;
   chtype	fillerChar;
   bool	shadow;
	 CDKOBJS* bindableObject();
#ifdef pneu
#else
	 int createList(CDK_CSTRING *list, int listSize);
#endif
	 SAlphalist(SScreen *cdkscreen,
			 int xplace,
			 int yplace,
			 int height,
			 int width,
			 const char *title,
			 const char *label,
#ifdef pneu
			 std::vector<std::string> *plistp,
#else
			 CDK_CSTRING *list,
			 int listSize,
#endif
			 chtype fillerChar,
			 chtype highlight,
			 bool Box,
			 bool shadow,
			 // GSchade Anfang
			 int highnr/*=0*/
			 // GSchade Ende
			 );
	 ~SAlphalist();
	 void destroyObj(){this->~SAlphalist();}
	 void drawMyScroller(/*SAlphalist *widget*/);
	 void drawCDKAlphalist(bool Box GCC_UNUSED);
	 void drawObj(bool Box);
	 void moveCDKAlphalist(int xplace, int yplace, bool relative, bool refresh_flag);
	 void injectMyScroller(chtype key);
	 const char* activateCDKAlphalist(chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/,int obpfeil/*=0*/);
	 int injectCDKAlphalist(chtype input);
	 int injectObj(chtype ch){return injectCDKAlphalist(ch);}
	 void eraseCDKAlphalist();
	 void eraseObj(){eraseCDKAlphalist();}
	 void destroyInfo();
	 void setCDKAlphalist(
#ifdef pneu
			 std::vector<std::string> *plistp,
#else
			 CDK_CSTRING *list, int listSize, 
#endif
			 chtype fillerChar, chtype highlight, bool Box);
	 void setCDKAlphalistContents(
#ifdef pneu
		      std::vector<std::string> *plistp
#else
			 CDK_CSTRING *list, int listSize
#endif
			 );
#ifdef pneu
std::vector<std::string> *
#else
char ** 
#endif
                   getCDKAlphalistContents(
#ifdef pneu
#else
                                        	 int *size
#endif
                                                 		);
	 int getCDKAlphalistCurrentItem();
	 void setCDKAlphalistCurrentItem(int item);
	 void setCDKAlphalistFillerChar(chtype fillerCharacter);
	 chtype getCDKAlphalistFillerChar();
	 void setCDKAlphalistHighlight(chtype hl);
	 chtype getCDKAlphalistHighlight();
	 void setBKattrObj(chtype attrib);
	 //	 void setCDKAlphalistBox(bool Box);
	 bool getCDKAlphalistBox();
	 void setMyULchar(chtype character);
	 void setMyURchar(chtype character);
	 void setMyLLchar(chtype character);
	 void setMyLRchar(chtype character);
	 void setMyVTchar(chtype character);
	 void setMyHZchar(chtype character);
	 void setMyBXattr(chtype character);
	 void setMyBKattr(chtype character);
	 void setCDKAlphalistPreProcess(PROCESSFN callback, void *data);
	 void setCDKAlphalistPostProcess(PROCESSFN callback, void *data);
	 void focusCDKAlphalist();
	 void focusObj(){focusCDKAlphalist();}
	 void unfocusCDKAlphalist();
	 void unfocusObj(){unfocusCDKAlphalist();}
};
// typedef struct SAlphalist CDKALPHALIST;

/*
 * Define menu specific values.
 */
#define MAX_MENU_ITEMS	30
#define MAX_SUB_ITEMS	98

#ifndef INT_MIN
#define INT_MIN (-INT_MAX - 1)
#endif
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif


/*
 * This draws the label.
 */
//#define drawCDKObject(/*o,*/box)           /*MethodOf(o)->*/drawObj       (/*ObjOf(o),*/box)
// #define drawCDKLabel(/*obj,*/Box) drawCDKObject(/*obj,*/Box)

/*
 * This erases the label.
 */
//#define eraseCDKObject(/*o*/)              /*MethodOf(o)->*/eraseObj      (/*ObjOf(o)*/)
//#define eraseCDKLabel(/*obj*/) eraseCDKObject(/*obj*/)


/*
 * Declare the CDK label structure.
 */
struct SLabel:CDKOBJS {
//   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
#ifdef pneu
	std::vector<chtstr> pinfo;
	std::vector<chtstr>::const_iterator pitinfo;
	std::vector<int> infoLen;
	std::vector<int> infoPos;
#else
   chtype **	sinfo;
   int *	infoLen;
   int *	infoPos;
#endif
   int		boxWidth;
   int		boxHeight;
   int		xpos;
   int		ypos;
   int		rows;
   bool	shadow;
	 SLabel(SScreen *cdkscreen, int xplace, int yplace, 
#ifdef pneu
			 std::vector<std::string> mesg
#else
			 CDK_CSTRING2 mesg, int rows
#endif
			 , bool Box, bool shadow);
	 void setCDKLabelBox(/*SLabel *label, */bool Box);
	 bool getCDKLabelBox(/*SLabel *label*/);
	 void activateCDKLabel(/*SLabel *label, */chtype *actions GCC_UNUSED);
	 void setCDKLabel(/*SLabel *label, */
#ifdef pneu
			 std::vector<std::string> mesg
#else
			 CDK_CSTRING2 mesg, int lines
#endif
			 , bool Box);
	 void setCDKLabelMessage (/*SLabel *label, */
#ifdef pneu
			 std::vector<std::string> s_info
#else
			 CDK_CSTRING2 s_info, int infoSize
#endif
			 );
#ifdef pneu
#else
	 chtype **getCDKLabelMessage(/*SLabel *label, */int *size);
#endif
	 void setBKattrLabel(chtype attrib);
	 void setBKattrObj(chtype attrib);
	 void drawCDKLabel(/*CDKOBJS *object, */bool Box GCC_UNUSED);
	 void eraseCDKLabel(/*CDKOBJS *object*/);
	 void moveCDKLabel(/*CDKOBJS *object,*/ int xplace, int yplace, bool relative, bool refresh_flag);
	 void destroyCDKLabel(/*CDKOBJS *object*/);
	 char waitCDKLabel(/*SLabel *label, */char key);
};
//typedef struct SLabel SLabel;

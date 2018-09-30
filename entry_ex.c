/* $Id: entry_ex.c,v 1.17 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>
#include <locale.h>
// GSchade 25.9.18
CDKSCREEN *allgscr;

#ifdef HAVE_XCURSES
char *XCursesProgramName = "entry_ex";
#endif

static BINDFN_PROTO (XXXCB);

static char **myUserList = 0;
char **userList              = 0;
static int userSize;

typedef struct
{
	int deleted;			/* index in current list which is deleted */
	int original;		/* index in myUserList[] of deleted item */
	int position;		/* position before delete */
	int topline;			/* top-line before delete */
} UNDO;

static UNDO *myUndoList;
static int undoSize;

/*
 * This reads the passwd file and retrieves user information.
 */
static int getUserList (char ***list)
{
#if defined (HAVE_PWD_H)
	struct passwd *ent;
#endif
	int x = 0;
	unsigned used = 0;

#if defined (HAVE_PWD_H)
	while ((ent = getpwent ()) != 0)
	{
		used = CDKallocStrings (list, ent->pw_name, (unsigned)x++, used);
	}
	endpwent ();
#endif
	return x;
}

#define CB_PARAMS EObjectType cdktype GCC_UNUSED, void* object GCC_UNUSED, void* clientdata GCC_UNUSED, chtype key GCC_UNUSED

static void fill_undo (CDKALPHALIST *widget, int deleted, char *data)
{
	int top = getCDKScrollCurrentTop (widget->scrollField);
	int item = getCDKAlphalistCurrentItem (widget);
	int n;

	myUndoList[undoSize].deleted = deleted;
	myUndoList[undoSize].topline = top;
	myUndoList[undoSize].original = -1;
	myUndoList[undoSize].position = item;
	for (n = 0; n < userSize; ++n)
	{
		if (!strcmp (myUserList[n], data))
		{
			myUndoList[undoSize].original = n;
			break;
		}
	}
	++undoSize;
}

static int do_delete (CB_PARAMS)
{
	CDKALPHALIST *widget = (CDKALPHALIST *)clientdata;
	int size;
	char **list = getCDKAlphalistContents (widget, &size);
	int result = FALSE;

	if (size)
	{
		int save = getCDKScrollCurrentTop (widget->scrollField);
		int first = getCDKAlphalistCurrentItem (widget);
		int n;

		fill_undo (widget, first, list[first]);
		for (n = first; n < size; ++n)
			list[n] = list[n + 1];
		setCDKAlphalistContents (widget, (CDK_CSTRING *)list, size - 1);
		setCDKScrollCurrentTop (widget->scrollField, save);
		setCDKAlphalistCurrentItem (widget, first);
		drawCDKAlphalist (widget, BorderOf (widget));
		result = TRUE;
	}
	return result;
}

static int do_delete1 (CB_PARAMS)
{
	CDKALPHALIST *widget = (CDKALPHALIST *)clientdata;
	int size;
	char **list = getCDKAlphalistContents (widget, &size);
	int result = FALSE;

	if (size)
	{
		int save = getCDKScrollCurrentTop (widget->scrollField);
		int first = getCDKAlphalistCurrentItem (widget);

		if (first-- > 0)
		{
			int n;

			fill_undo (widget, first, list[first]);
			for (n = first; n < size; ++n)
				list[n] = list[n + 1];
			setCDKAlphalistContents (widget, (CDK_CSTRING *)list, size - 1);
			setCDKScrollCurrentTop (widget->scrollField, save);
			setCDKAlphalistCurrentItem (widget, first);
			drawCDKAlphalist (widget, BorderOf (widget));
			result = TRUE;
		}
	}
	return result;
}

static int do_help (CB_PARAMS)
{
	static const char *message[] =
	{
		"Alpha List tests:",
		"",
		"F1 = help (this message)",
		"F2 = delete current item",
		"F3 = delete previous item",
		"F4 = reload all items",
		"F5 = undo deletion",
		0
	};
	popupLabel (allgscr,
			(CDK_CSTRING2)message,
			(int)CDKcountStrings ((CDK_CSTRING2)message));
	return TRUE;
}

static int do_reload (CB_PARAMS)
{
	int result = FALSE;

	if (userSize)
	{
		CDKALPHALIST *widget = (CDKALPHALIST *)clientdata;
		setCDKAlphalistContents (widget, (CDK_CSTRING *)myUserList, userSize);
		setCDKAlphalistCurrentItem (widget, 0);
		drawCDKAlphalist (widget, BorderOf (widget));
		result = TRUE;
	}
	return result;
}

static int do_undo (CB_PARAMS)
{
	int result = FALSE;

	if (undoSize > 0)
	{
		CDKALPHALIST *widget = (CDKALPHALIST *)clientdata;
		int size;
		int n;
		char **oldlist = getCDKAlphalistContents (widget, &size);
		char **newlist = (char **)malloc ((size_t) (++size + 1) * sizeof (char *));

		--undoSize;
		newlist[size] = 0;
		for (n = size - 1; n > myUndoList[undoSize].deleted; --n)
		{
			newlist[n] = copyChar (oldlist[n - 1]);
		}
		newlist[n--] = copyChar (myUserList[myUndoList[undoSize].original]);
		while (n >= 0)
		{
			newlist[n] = copyChar (oldlist[n]);
			--n;
		}
		setCDKAlphalistContents (widget, (CDK_CSTRING *)newlist, size);
		setCDKScrollCurrentTop (widget->scrollField, myUndoList[undoSize].topline);
		setCDKAlphalistCurrentItem (widget, myUndoList[undoSize].position);
		drawCDKAlphalist (widget, BorderOf (widget));
		free (newlist);
		result = TRUE;
	}
	return result;
}

/*
 * This demonstrates the Cdk entry field widget.
 */
int main (int argc, char **argv)
{
	 //setlocale(LC_ALL,"de_DE.UTF-8");
	 setlocale(LC_ALL,"");
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;

	/* Get the user list. */
	userSize = getUserList (&userList);
	if (userSize <= 0)
	{
		fprintf (stderr, "Cannot get user list\n");
		ExitProgram (EXIT_FAILURE);
	}
	myUserList = copyCharList ((const char **)userList);
	myUndoList = (UNDO *) malloc ((size_t) userSize * sizeof (UNDO));
	undoSize = 0;
	 /*
   CDKENTRY *directory  = 0,*file=0;
   const char *title    = "<C>Gib aößä\n<C>dürectory name.";
   const char *ftit    = "<C>Dateiname.";
   const char *label    = "</R/U/6>Dürectory:<!R!6>";
   const char *flabel    = "</R/U/6>Dätei:<!R!6>";
	 */
   char *info;
   const char *mesg[10];
   char temp0[256],temp[256];

   CDK_PARAMS params;
	 CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);
	 /*
	 CDKparamValue (&params, 'N', FALSE), CDKparamValue (&params, 'S', FALSE); CDKparamValue (&params, 'X', CENTER), CDKparamValue (&params, 'Y', CENTER),
*/
   cdkscreen = initCDKScreen (NULL);
	 // GSchade 25.9.18
	 allgscr=cdkscreen;

   /* Start CDK colors. */
   initCDKColor ();
//	 for(int i=0;i<64;i++) mvwaddch (cdkscreen->window, 20+(i/8), 20+i, 'a'|COLOR_PAIR(i));
	 /*
	 chtype wcol=COLOR_PAIR(11)|A_BLINK;
				wattron(cdkscreen->window,wcol); // wirkt nicht
		mvwprintw(cdkscreen->window,3,60,"%s","weltoffen");
				wattroff(cdkscreen->window,wcol); // wirkt nicht
				*/
	 const int max=100;
	 struct hotkst {
		 int nr;
		 char buch;
		 const char *label;
		 u_char obalph;
		 CDKENTRY *entry;
	 } hk[]={
		 {4,'r',"</R/U/6>Dürectory:<!R!6>"},
		 {4,'t',"</R/U/6>Dätei:<!R!6>"},
		 {4,'n',"</R/U/6>Ordner:<!R!6>"},
		 {4,'h',"</R/U/6>Alphalist:<!R!6>",1},
	 };
	 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) {
		 if (hk[aktent].obalph) {
			 hk[aktent].entry=(CDKENTRY*)newCDKAlphalist(cdkscreen,50,12+aktent,10,40,"",hk[aktent].label,(CDK_CSTRING*)userList,userSize,'.',A_REVERSE,0,0,hk[aktent].nr);
		 } else {
			 hk[aktent].entry=newCDKEntry(cdkscreen,50,12+aktent,"",hk[aktent].label,A_NORMAL,'.',vMIXED,30,0,max,0,0,hk[aktent].nr);
			 bindCDKObject (vENTRY, hk[aktent].entry, '?', XXXCB, 0);
		 }
		 /* Is the widget null? */
		 if (hk[aktent].entry == 0)
		 {
			 /* Clean up. */
			 destroyCDKScreen (cdkscreen);
			 endCDK ();

			 printf ("Cannot create the entry box. Is the window too small?\n");
			 ExitProgram (EXIT_FAILURE);
		 }
	 }

	 /* Create the entry field widget. */
//	 directory = newCDKEntry (cdkscreen,/*xplace*/50,/*yplace*/12, /*title*/"", label, A_UNDERLINE, '.', vMIXED, 30, 0, max,/*Box*/0,/*shadow*/0,/*highnr*/2);
//	 file = newCDKEntry (cdkscreen, 50, 13, /*ftit*/"", flabel, A_NORMAL, '.', vMIXED, 30, 0, max,/*Box*/0,/*shadow*/0,/*highnr*/1);


   /* Draw the screen. */
   refreshCDKScreen (cdkscreen);

   /*
    * Pass in whatever was given off of the command line. Notice we
    * don't check if argv[1] is null or not. The function setCDKEntry
    * already performs any needed checks.
    */
   //setCDKEntry (hk[0].entry, argv[optind], 0, max, TRUE);

   /* Activate the entry field. */
	 int Znr=0,Zweitzeichen=0;
   EExitType	exitType;
	 while (1) {
	//		mvwprintw(cdkscreen->window,30,30,"<R>werde eingeben:%i %i ",info,Zweitzeichen);
		 info = activateCDKEntry (hk[Znr].entry, 0,&Zweitzeichen);
		 mesg[0] = "<C>Letzte Eingabe:";
		 sprintf (temp, "<C>(%.*s|%s|%i)", (int)(sizeof (temp) - 10), info,hk[Znr].entry->info,Zweitzeichen);
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 exitType=hk[Znr].entry->exitType;
//		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
		 // Tab
		 if (Zweitzeichen==-9) {
			 Znr++;
			 if (Znr==sizeof hk/sizeof *hk) Znr=0;
			 // Alt+Tab
		 } else if (Zweitzeichen==-8) {
			 Znr--;
			 if (Znr<0) Znr=sizeof hk/sizeof *hk-1;
			 // Alt- +Buchstabe
		 } else if (Zweitzeichen) /*(info && *info==27)*/ {
			 Znr=-1;
			 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) {
				 if (Zweitzeichen==hk[aktent].buch) {
					 Znr=aktent;
					 break;
				 }
			 }
			 if (Znr==-1) break;
		 } else break;
	 }  // while (1)
	 destroyCDKScreen (cdkscreen); endCDK (); return EXIT_SUCCESS;

	 /* Tell them what they typed. */
	 if (exitType == vESCAPE_HIT && !Zweitzeichen) {
		 mesg[0] = "<C>Sie schlügen escape. No information passed back.";
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKObject(hk[aktent].entry);
		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
	 } else if (exitType == vNORMAL) {
		 mesg[0] = "<C>Sie gaben folgendes ein";
		 sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKObject(hk[aktent].entry);
		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
	 } else {
		 sprintf(temp0,"Exit-type: %i",exitType);
		 mesg[0]=temp0;
		 mesg[1] = "<C>Sie gaben folgendes ein: ";
		 sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
		 mesg[2] = temp;
		 mesg[3] = "<C>Eine Taste drücken zum Fortfahren.";
		 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKObject(hk[aktent].entry);
		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 4);
	 }

	 /* Clean up and exit. */
	 destroyCDKScreen (cdkscreen);
	 endCDK ();
	 return EXIT_SUCCESS;
}

static int XXXCB (EObjectType cdktype GCC_UNUSED,
		void *object GCC_UNUSED,
		void *clientData GCC_UNUSED,
		chtype key GCC_UNUSED)
{
	const char* mesg[2];
	mesg[0]="Hilfefunktion";
	popupLabel((CDKSCREEN*)allgscr,(CDK_CSTRING2)mesg,1);
//	printf("Hilfefunktion aufgerufen\n\r\n\r");
	return (TRUE);
}

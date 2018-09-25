/* $Id: entry_ex.c,v 1.17 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>
#include <locale.h>
// GSchade 25.9.18
CDKSCREEN *allgscr;

#ifdef HAVE_XCURSES
char *XCursesProgramName = "entry_ex";
#endif

static BINDFN_PROTO (XXXCB);

/*
 * This demonstrates the Cdk entry field widget.
 */
int main (int argc, char **argv)
{
	 //setlocale(LC_ALL,"de_DE.UTF-8");
	 setlocale(LC_ALL,"");
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKENTRY *directory  = 0,*file=0;
   const char *title    = "<C>Gib aößä\n<C>dürectory name.";
   const char *ftit    = "<C>Dateiname.";
   const char *label    = "</R/6>Dürectory:<!R!6>";
   char *info,*infdat;
   const char *mesg[10];
	 const char *m0[1];
   char temp0[256],temp[256];

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);

   cdkscreen = initCDKScreen (NULL);
	 // GSchade 25.9.18
	 allgscr=cdkscreen;

   /* Start CDK colors. */
   initCDKColor ();

		mvwprintw(cdkscreen->window,3,60,"%s","weltoffen");
		const int max=10;
   /* Create the entry field widget. */
   directory = newCDKEntry (cdkscreen,
			    CDKparamValue (&params, 'X', CENTER),
			    CDKparamValue (&params, 'Y', CENTER),
			    title, label, A_NORMAL, '.', vMIXED,
			    5, 0, max,
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', FALSE));
   bindCDKObject (vENTRY, directory, '?', XXXCB, 0);
   file = newCDKEntry (cdkscreen,
			    50,
			    14,
			    ftit, label, A_NORMAL, '.', vMIXED,
			    30, 0, max,
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', FALSE));
   bindCDKObject (vENTRY, file, '?', XXXCB, 0);

   /* Is the widget null? */
   if (directory == 0)
   {
      /* Clean up. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the entry box. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Draw the screen. */
   refreshCDKScreen (cdkscreen);

   /*
    * Pass in whatever was given off of the command line. Notice we
    * don't check if argv[1] is null or not. The function setCDKEntry
    * already performs any needed checks.
    */
   setCDKEntry (directory, argv[optind], 0, max, TRUE);

   /* Activate the entry field. */
	 int Zweitzeichen=0;
	 info = activateCDKEntry (directory, 0,&Zweitzeichen);
	 info= activateCDKEntry (file, 0,&Zweitzeichen);
	 do {
		 info = activateCDKEntry (directory, 0,&Zweitzeichen);
	 } while (!info || *info==27);

	 /* Tell them what they typed. */
	 if (directory->exitType == vESCAPE_HIT && !Zweitzeichen) {
		 mesg[0] = "<C>Sie schlügen escape. No information passed back.";
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 destroyCDKEntry (directory);
		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
	 } else if (directory->exitType == vNORMAL) {
		 mesg[0] = "<C>Sie gaben folgendes ein";
		 sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 destroyCDKEntry (directory);
		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
	 } else {
		 sprintf(temp0,"Exit-type: %i",(int)directory->exitType);
		 mesg[0]=temp0;
		 mesg[1] = "<C>Sie gaben folgendes ein: ";
		 sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
		 mesg[2] = temp;
		 mesg[3] = "<C>Eine Taste drücken zum Fortfahren.";
		 destroyCDKEntry (directory);
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
	return (TRUE);
}

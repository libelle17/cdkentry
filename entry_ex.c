/* $Id: entry_ex.c,v 1.17 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>
#include <locale.h>

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
   const char *title    = "<C>Gib a\n<C>directory name.";
   const char *ftit    = "<C>Da<C>teiname.";
   const char *label    = "</U/6>Directory:<!U!6>";
   char *info,*infdat;
   const char *mesg[10];
   char temp[256];

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);

   cdkscreen = initCDKScreen (NULL);

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
   info = activateCDKEntry (directory, 0);
   infdat= activateCDKEntry (file, 0);
   info = activateCDKEntry (directory, 0);

   /* Tell them what they typed. */
   if (directory->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No information passed back.";
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";

      destroyCDKEntry (directory);

      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (directory->exitType == vNORMAL)
   {
      mesg[0] = "<C>You typed in the following";
      sprintf (temp, "<C>(%.*s)", (int)(sizeof (temp) - 10), info);
      mesg[1] = temp;
      mesg[2] = "";
      mesg[3] = "<C>Press any key to continue.";

      destroyCDKEntry (directory);

      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 4);
   }
   else
   {
      destroyCDKEntry (directory);
   }

   /* Clean up and exit. */
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

static int XXXCB (EObjectType cdktype GCC_UNUSED,
		  void *object GCC_UNUSED,
		  void *clientData GCC_UNUSED,
		  chtype key GCC_UNUSED)
{
   return (TRUE);
}

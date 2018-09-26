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
		 CDKENTRY *entry;
	 } hk[]={
		 {4,'r',"</R/U/6>Dürectory:<!R!6>"},
		 {4,'t',"</R/U/6>Dätei:<!R!6>"},
		 {4,'n',"</R/U/6>Ordner:<!R!6>"}
	 };
	 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) {
		 hk[aktent].entry=newCDKEntry(cdkscreen,50,12+aktent,"",hk[aktent].label,A_NORMAL,'.',vMIXED,30,0,max,0,0,hk[aktent].nr);
		 bindCDKObject (vENTRY, hk[aktent].entry, '?', XXXCB, 0);
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
   setCDKEntry (hk[0].entry, argv[optind], 0, max, TRUE);

   /* Activate the entry field. */
	 int Znr=1,Zweitzeichen=0;
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

	 /* Tell them what they typed. */
	 if (exitType == vESCAPE_HIT && !Zweitzeichen) {
		 mesg[0] = "<C>Sie schlügen escape. No information passed back.";
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKEntry(hk[aktent].entry);
		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
	 } else if (exitType == vNORMAL) {
		 mesg[0] = "<C>Sie gaben folgendes ein";
		 sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKEntry(hk[aktent].entry);
		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
	 } else {
		 sprintf(temp0,"Exit-type: %i",exitType);
		 mesg[0]=temp0;
		 mesg[1] = "<C>Sie gaben folgendes ein: ";
		 sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
		 mesg[2] = temp;
		 mesg[3] = "<C>Eine Taste drücken zum Fortfahren.";
		 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKEntry(hk[aktent].entry);
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

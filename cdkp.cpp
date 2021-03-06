#define HAVE_NCURSESW_NCURSES_H
#include "cdkp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include <unistd.h> // getcwd
#include <sys/stat.h> // struct stat
#include <dirent.h>
#include <cctype> // isdigit
#include <errno.h> // errno
#include <iostream> // cout
#include <algorithm> // sort
using namespace std;

void chtstr::gibaus() const
{ 
	for(size_t i=0;i<len;i++)cout<<inh[i]<<" ";cout<<endl;
}
// chtstr::chtstr(size_t len):len(len) { inh=new chtype[len]; }

// GSchade 17.11.18; s. cdk.h
einbauart akteinbart;

//ALL_SCREENS *all_screens;
vector<SScreen*> all_screens;
vector<CDKOBJS*> all_objects;

/*
 * Return an integer like 'floor()', which returns a double.
 */
int floorCDK(double value)
{
   int result = (int)value;
   if (result > value)		/* e.g., value < 0.0 and value is not an integer */
      result--;
   return result;
}

/*
 * Return an integer like 'ceil()', which returns a double.
 */
int ceilCDK(double value)
{
   return -floorCDK(-value);
}

/*
 * This beeps then flushes the stdout stream.
 */
void Beep(void)
{
   beep();
   fflush(stdout);
}

int getmaxxf(WINDOW *win)
{
	int y, x;
	getmaxyx(win, y, x);
	return x;
}

int getmaxyf(WINDOW *win)
{
	int y, x;
	getmaxyx(win, y, x);
	return y;
}

/*
 * If the dimension is a negative value, the dimension will be the full
 * height/width of the parent window - the value of the dimension.  Otherwise,
 * the dimension will be the given value.
 */
int setWidgetDimension(int parentDim, int proposedDim, int adjustment)
{
	int dimension = 0;
	/* If the user passed in FULL, return the parent's size. */
	if ((proposedDim == FULL) || (!proposedDim)) {
		dimension = parentDim;
	} else {
		/* If they gave a positive value, return it. */
		if (proposedDim >= 0) {
			if (proposedDim >= parentDim)
				dimension = parentDim;
			else
				dimension = (proposedDim + adjustment);
		} else {
			/*
			 * If they gave a negative value, then return the
			 * dimension of the parent plus the value given.
			 */
			dimension = parentDim + proposedDim;
			/* Just to make sure. */
			if (dimension < 0)
				dimension = parentDim;
		}
	}
	return dimension;
}

static int encodeAttribute(const char *string, int from, chtype *mask)
{
	int pair = 0;
	*mask = 0;
	switch (string[from + 1]) {
		case 'B':
			*mask = A_BOLD;
			break;
		case 'D':
			*mask = A_DIM;
			break;
		case 'K':
			*mask = A_BLINK;
			break;
		case 'R':
			*mask = A_REVERSE;
			break;
		case 'S':
			*mask = A_STANDOUT;
			break;
		case 'U':
			*mask = A_UNDERLINE;
			break;
	}
	if (*mask) {
		from++;
	} else {
		int digits;
		pair = 0;
		for (digits = 1; digits <= 3; ++digits) {
			if (!isdigit ((unsigned char)string[1 + from]))
				break;
			pair *= 10;
			pair += DigitOf(string[++from]);
		}
#ifdef HAVE_START_COLOR
#define MAX_PAIR (int) (A_COLOR / (((~A_COLOR) << 1) & A_COLOR))
		if (pair > MAX_PAIR )
			pair = MAX_PAIR;
		*mask = (chtype)COLOR_PAIR(pair);
#else
		*mask = A_BOLD;
#endif
	}
	return from;
} // static int encodeAttribute(const char *string, int from, chtype *mask)

/*
 * This function takes a character string, full of format markers
 * and translates them into a chtype * array. This is better suited
 * to curses, because curses uses chtype almost exclusively
 */
// highinr G.Schade 26.9.18
chtstr::chtstr(const char *string, int *to, int *align, const int highinr/*=0*/)
{
	//	chtype *result = 0;
	inh=0;
	chtype attrib,lastChar,mask;
	(*to) = 0;
	*align = LEFT;
	if (string && *string) {
		/*int */len = (int)strlen(string);
		int pass;
		int used = 0;
		/*
		 * We make two passes because we may have indents and tabs to expand, and
		 * do not know in advance how large the result will be.
		 */
		for (pass = 0; pass < 2; pass++) {
			int insideMarker;
			int from;
			int adjust;
			int start;
			int x = 3;
			if (pass) {
				//				if ((result = typeMallocN(chtype, used + 2)) == 0)
				if (!(inh=new chtype[used+2])) {
					used = 0;
					break;
				}
			}
			adjust = 0;
			attrib = A_NORMAL;
			start = 0;
			used = 0;
			/* Look for an alignment marker.  */
			if (*string == L_MARKER) {
				if (string[1] == 'C' && string[2] == R_MARKER) {
					(*align) = CENTER;
					start = 3;
				} else if (string[1] == 'R' && string[2] == R_MARKER) {
					(*align) = RIGHT;
					start = 3;
				} else if (string[1] == 'L' && string[2] == R_MARKER) {
					start = 3;
				} else if (string[1] == 'B' && string[2] == '=') {
					/* Set the item index value in the string.       */
					if (inh) {
						inh[0] = ' ';
						inh[1] = ' ';
						inh[2] = ' ';
					}
					/* Pull out the bullet marker.  */
					while (string[x] != R_MARKER && string[x]) {
						if (inh)
							inh[x] = (chtype)string[x] | A_BOLD;
						x++;
					}
					adjust = 1;
					/* Set the alignment variables.  */
					start = x;
					used = x;
				} else if (string[1] == 'I' && string[2] == '=') {
					from = 2;
					x = 0;
					while (string[++from] != R_MARKER && string[from]) {
						if (isdigit ((unsigned char)string[from])) {
							adjust = (adjust * 10) + DigitOf (string[from]);
							x++;
						}
					}
					start = x + 4;
				}
			}
			while (adjust-- > 0) {
				if (inh)
					inh[used] = ' ';
				used++;
			}
			/* Set the format marker boolean to false.  */
			insideMarker = FALSE;
			// GSchade 25.9.18
			//size_t pos=0;
			/* Start parsing the character string.  */
			for (from = start; from < len; from++) {
				/* Are we inside a format marker?  */
				if (!insideMarker) {
					if (string[from] == L_MARKER
							&& (string[from + 1] == '/'
								|| string[from + 1] == '!'
								|| string[from + 1] == '#'))
					{
						insideMarker = TRUE;
					} else if (string[from] == '\\' && string[from + 1] == L_MARKER) {
						from++;
						if (inh)
							inh[used] = (unsigned char)string[from] | attrib;
						used++;
						from++;
					} else if (string[from] == '\t') {
						do {
							if (inh)
								inh[used] = ' ';
							used++;
						}
						while (used & 7);
					} // else if (inh && !strchr("ö",string[from])) KLA inh[used++]=(unsigned char)'o'|attrib;
					// GSchade 25.9.18
					/*
						 else if (strchr("äöüÄÖÜß",string[from])) {
						 printf("from: %i, string[from]: %i\n",from,(int)string[from]+256);
					//if (inh) inh[used]=(unsigned char)'z'|attrib; used++;
					if (inh) inh[used]=(unsigned char)-61|attrib; used++;
					if (!strchr("ä",string[from])) { if (inh) inh[used]=(unsigned char)164-256|attrib; used++; }
					else if (!strchr("ö",string[from])) { if (inh) inh[used]=(unsigned char)182-256|attrib; used++; }
					else if (!strchr("ü",string[from])) { if (inh) inh[used]=(unsigned char)188-256|attrib; used++; }
					else if (!strchr("Ä",string[from])) { if (inh) inh[used]=(unsigned char)132|attrib; used++; }
					else if (!strchr("Ö",string[from])) { if (inh) inh[used]=(unsigned char)150|attrib; used++; }
					else if (!strchr("Ü",string[from])) { if (inh) inh[used]=(unsigned char)156|attrib; used++; }
					else if (!strchr("ß",string[from])) { if (inh) inh[used]=(unsigned char)159|attrib; used++; }
					}
					 */
					// Ende GSchade 25.9.18
					else {
						if (inh) {
							// GSchade 26.9.18
							if (used==highinr-1) {
								inh[used] = (unsigned char)string[from] | attrib|COLOR_PAIR(1);
							} else {
								// Ende GSchade 
								inh[used] = (unsigned char)string[from] | attrib;
							}
						}
						used++;
					}
				} else {
					switch (string[from]) {
						case R_MARKER:
							insideMarker = 0;
							break;
						case '#':
							{
								lastChar = 0;
								switch (string[from + 2]) {
									case 'L':
										switch (string[from + 1]) {
											case 'L':
												lastChar = ACS_LLCORNER;
												break;
											case 'U':
												lastChar = ACS_ULCORNER;
												break;
											case 'H':
												lastChar = ACS_HLINE;
												break;
											case 'V':
												lastChar = ACS_VLINE;
												break;
											case 'P':
												lastChar = ACS_PLUS;
												break;
										}
										break;
									case 'R':
										switch (string[from + 1]) {
											case 'L':
												lastChar = ACS_LRCORNER;
												break;
											case 'U':
												lastChar = ACS_URCORNER;
												break;
										}
										break;
									case 'T':
										switch (string[from + 1]) {
											case 'T':
												lastChar = ACS_TTEE;
												break;
											case 'R':
												lastChar = ACS_RTEE;
												break;
											case 'L':
												lastChar = ACS_LTEE;
												break;
											case 'B':
												lastChar = ACS_BTEE;
												break;
										}
										break;
									case 'A':
										switch (string[from + 1]) {
											case 'L':
												lastChar = ACS_LARROW;
												break;
											case 'R':
												lastChar = ACS_RARROW;
												break;
											case 'U':
												lastChar = ACS_UARROW;
												break;
											case 'D':
												lastChar = ACS_DARROW;
												break;
										}
										break;
									default:
										if (string[from + 1] == 'D' &&
												string[from + 2] == 'I')
											lastChar = ACS_DIAMOND;
										else if (string[from + 1] == 'C' &&
												string[from + 2] == 'B')
											lastChar = ACS_CKBOARD;
										else if (string[from + 1] == 'D' &&
												string[from + 2] == 'G')
											lastChar = ACS_DEGREE;
										else if (string[from + 1] == 'P' &&
												string[from + 2] == 'M')
											lastChar = ACS_PLMINUS;
										else if (string[from + 1] == 'B' &&
												string[from + 2] == 'U')
											lastChar = ACS_BULLET;
										else if (string[from + 1] == 'S' &&
												string[from + 2] == '1')
											lastChar = ACS_S1;
										else if (string[from + 1] == 'S' &&
												string[from + 2] == '9')
											lastChar = ACS_S9;
								}
								if (lastChar) {
									adjust = 1;
									from += 2;
									if (string[from + 1] == '(')
										/* Check for a possible numeric modifier.  */
									{
										from++;
										adjust = 0;
										while (string[++from] != ')' && string[from]) {
											if (isdigit((unsigned char)string[from])) {
												adjust =(adjust * 10) + DigitOf(string[from]);
											}
										}
									}
								}
								for (x = 0; x < adjust; x++) {
									if (inh)
										inh[used] = lastChar | attrib;
									used++;
								}
								break;
							}
						case '/':
							from = encodeAttribute(string, from, &mask);
							attrib = attrib | mask;
							break;
						case '!':
							from = encodeAttribute(string, from, &mask);
							attrib = attrib & ~mask;
							break;
					}
				}
			}
			if (inh) {
				inh[used] = 0;
				inh[used + 1] = 0;
			}
			/*
			 * If there are no characters, put the attribute into the
			 * the first character of the array.
			 */
			if (!used && inh) {
				inh[0] = attrib;
			}
		}
		*to = used;
	} else {
		/*
		 * Try always to return something; otherwise lists of chtype strings
		 * would get a spurious null pointer whenever there is a blank line,
		 * and CDKfreeChtypes() would fail to free the whole list.
		 */
		//		inh = typeCallocN(chtype, 1);
		inh=new chtype[1];
	}
	return /*result*/;
} // chtstr::chtstr(const char *string, int *to, int *align, const int highinr/*=0*/)

#ifdef pneu
/*
 * This returns a pointer to char * of a chtype *
 * Formatting codes are omitted.
 */
char *chtstr::chtype2Char()
{
	if (inh) {
		int len = chlen(inh);
		if (ch) delete[] ch;
		if ((ch = new char[len+1])) {
			for (int x = 0; x < len; x++) {
				ch[x] = (char)(unsigned char)inh[x];
			}
			ch[len] = '\0';
		}
	}
	return ch;
}
#else
/*
 * This returns a pointer to char * of a chtype *
 * Formatting codes are omitted.
 */
char *chtype2Char(const chtype *string)
{
	char *newstring = 0;
	if (string) {
		int len = chlen(string);
		if ((newstring = typeMallocN(char, len + 1))) {
			for (int x = 0; x < len; x++) {
				newstring[x] =(char)(unsigned char)string[x];
			}
			newstring[len] = '\0';
		}
	}
	return (newstring);
}

/*
 * Count the number of items in a list of strings.
 */
unsigned CDKcountStrings(CDK_CSTRING2 list)
{
	unsigned result = 0;
	if (list) {
		while (*list++)
			result++;
	}
	return result;
}
#endif


/*
 * This determines the length of a chtype string
 */
int chlen(const chtype *string)
{
	int result = 0;
	if (string) {
		while (string[result])
			result++;
	}
	return (result);
}

void freeChtype(chtype *string)
{
	freeChecked(string);
}

/*
 * This takes a string, a field width and a justification type
 * and returns the adjustment to make, to fill
 * the justification requirement.
 */
int justifyString(int boxWidth, int mesgLength, int justify)
{
	/*
	 * Make sure the message isn't longer than the width.
	 * If it is, return 0.
	 */
	if (mesgLength >= boxWidth)
		return (0);
	/* Try to justify the message.  */
	if (justify == LEFT)
		return (0);
	if (justify == RIGHT)
		return boxWidth - mesgLength;
	if (justify == CENTER)
		return ((int)((boxWidth - mesgLength) / 2));
	return (justify);
}

//#ifdef pneu
//#else
/*
 * Free a list of strings, terminated by a null pointer.
 */
void CDKfreeStrings(char **list)
{
	if (list) {
		void *base =(void *)list;
		while (*list)
			free(*list++);
		free(base);
	}
}
//#endif

/*
 * Free a list of chtype-strings, terminated by a null pointer.
 */
void CDKfreeChtypes(chtype **list)
{
	if (list) {
		void *base = (void *)list;
		while (*list) {
			freeChtype(*list++);
		}
		free(base);
	}
}

/*
 * This takes an x and y position and realigns the values iff they sent in
 * values like CENTER, LEFT, RIGHT, ...
 */
void alignxy(WINDOW *window, int *xpos, int *ypos, int boxWidth, int boxHeight)
{
	int first, gap, last;
	first = getbegx(window);
	last = getmaxx(window);
	if ((gap = (last - boxWidth)) < 0)
		gap = 0;
	last = first + gap;

	switch (*xpos) {
		case LEFT:
			(*xpos) = first;
			break;
		case RIGHT:
			(*xpos) = first + gap;
			break;
		case CENTER:
			(*xpos) = first + (gap / 2);
			break;
		default:
			if ((*xpos) > last)
				(*xpos) = last;
			else if ((*xpos) < first)
				(*xpos) = first;
			break;
	}

	first = getbegy(window);
	last = getmaxy(window);
	if ((gap =(last - boxHeight)) < 0)
		gap = 0;
	last = first + gap;

	switch (*ypos) {
		case TOP:
			(*ypos) = first;
			break;
		case BOTTOM:
			(*ypos) = first + gap;
			break;
		case CENTER:
			(*ypos) = first +(gap / 2);
			break;
		default:
			if ((*ypos) > last) {
				(*ypos) = last;
			}
			else if ((*ypos) < first) {
				(*ypos) = first;
			}
			break;
	}
}

/*
 * This sets a string to the given character.
 */
void cleanChar(char *s, int len, char character)
{
	if (s) {
		int x;
		for (x = 0; x < len; x++) {
			s[x] = character;
		}
		s[--x] = '\0';
	}
}

/*
 * This writes out a chtype * string.
 */
void writeChtype(WINDOW *window,
		int xpos,
		int ypos,
		const chtype *const string,
		int align,
		int start,
		int end)
{
	writeChtypeAttrib(window, xpos, ypos, string, A_NORMAL, align, start, end);
}

/*
 * This writes out a chtype * string * with the given attributes added.
 */
void writeChtypeAttrib(WINDOW *window,
		int xpos,
		int ypos,
		const chtype *const string,
		chtype attr,
		int align,
		int start,
		int end)
{
	/* *INDENT-EQLS* */
	int diff             = end - start;
	int display          = 0;
	int x                = 0;

	if (align == HORIZONTAL) {
		/* Draw the message on a horizontal axis. */
		display = MINIMUM(diff, getmaxx(window) - xpos);
		int altumlz=0;
		for (x = 0; x < display; x++) {
			// GSchade 25.9.18
			if (1&&((int)(unsigned char)string[x+start]==194||(int)(unsigned char)string[x+start]==195)) {
				//			printf("Buchstabe: %c %i\r\n",(unsigned char)string[x+start], (int)(unsigned char)string[x+start]);
				char ausg[3];
				*ausg=string[x+start];
				ausg[1]=string[x+start+1];
				ausg[2]=0;
//				ausg[0]='o';
//				ausg[1]=0;
//				chtype testa;
//				wattr_get(window)

//				const chtype attrib=COLOR_PAIR(2)|A_REVERSE;//A_REVERSE|COLOR_GREEN;
//				printf("String: %s, Farbe: %lu\n\r",ausg,attrib/*window->_attrs*/);
				wattron(window,string[x+start]); 
				mvwprintw(window,ypos,xpos+x-altumlz,"%s",ausg);
				wattroff(window,string[x+start]); 
				x++;
				altumlz++;
			} else {
//				printf("Buchstabe: %c, Farbe: %lu\n\r",(unsigned char)string[x+start],attr);
				(void)mvwaddch(window, ypos, xpos + x-altumlz, string[x + start] |attr);
			}
		}
	} else {
		/* Draw the message on a vertical axis. */
		display = MINIMUM(diff, getmaxy(window) - ypos);
		for (x = 0; x < display; x++) {
			(void)mvwaddch(window, ypos + x, xpos, string[x + start] | attr);
		}
	}
} // void writeChtypeAttrib(

/*
 * This draws a box with attributes and lets the user define
 * each element of the box.
 */
void attrbox(WINDOW *win,
		chtype tlc,
		chtype trc,
		chtype blc,
		chtype brc,
		chtype horz,
		chtype vert,
		chtype attr)
{
	/* *INDENT-EQLS* */
	int x1       = 0;
	int y1       = 0;
	int y2       = getmaxy(win) - 1;
	int x2       = getmaxx(win) - 1;
	int count    = 0;

	/* Draw horizontal lines. */
	if (horz) {
		(void)mvwhline(win, y1, 0, horz | attr, getmaxx(win));
		(void)mvwhline(win, y2, 0, horz | attr, getmaxx(win));
		count++;
	}

	/* Draw vertical lines. */
	if (vert) {
		(void)mvwvline(win, 0, x1, vert | attr, getmaxy(win));
		(void)mvwvline(win, 0, x2, vert | attr, getmaxy(win));
		count++;
	}

	/* Draw in the corners. */
	if (tlc) {
		(void)mvwaddch(win, y1, x1, tlc | attr);
		count++;
	}
	if (trc) {
		(void)mvwaddch(win, y1, x2, trc | attr);
		count++;
	}
	if (blc) {
		(void)mvwaddch(win, y2, x1, blc | attr);
		count++;
	}
	if (brc) {
		(void)mvwaddch(win, y2, x2, brc | attr);
		count++;
	}
	if (count) {
		wrefresh(win);
	}
} // void attrbox(

/*
 * This draws a shadow around a window.
 */
void drawShadow(WINDOW *shadowWin)
{
	if (shadowWin) {
		int x_hi = getmaxx(shadowWin) - 1;
		int y_hi = getmaxy(shadowWin) - 1;

		/* Draw the line on the bottom. */
		(void)mvwhline(shadowWin, y_hi, 1, ACS_HLINE | A_DIM, x_hi);

		/* Draw the line on the right. */
		(void)mvwvline(shadowWin, 0, x_hi, ACS_VLINE | A_DIM, y_hi);

		(void)mvwaddch(shadowWin, 0, x_hi, ACS_URCORNER | A_DIM);
		(void)mvwaddch(shadowWin, y_hi, 0, ACS_LLCORNER | A_DIM);
		(void)mvwaddch(shadowWin, y_hi, x_hi, ACS_LRCORNER | A_DIM);
		wrefresh(shadowWin);
	}
}

/*
 * This is a dummy function used to ensure that the constant for mapping has
 * a distinct address.
 */
int getcCDKBind(EObjectType cdktype GCC_UNUSED,
		 void *object GCC_UNUSED,
		 void *clientData GCC_UNUSED,
		 chtype input GCC_UNUSED)
{
   return 0;
}

/*
 * Refresh one CDK window.
 * FIXME: this should be rewritten to use the panel library, so it would not
 * be necessary to touch the window to ensure that it covers other windows.
 */
void refreshCDKWindow(WINDOW *win)
{
   touchwin(win);
   wrefresh(win);
}

#ifdef pneu
#else
/*
 * This performs a safe copy of a string. This means it adds the null
 * terminator on the end of the string, like strdup().
 */
char *copyChar(const char *original)
{
	char *newstring = 0;
	if (original) {
		if ((newstring = typeMallocN(char, strlen(original) + 1)))
			strcpy(newstring, original);
	}
	return (newstring);
}

chtype *copyChtype(const chtype *original)
{
	chtype *newstring = 0;
	if (original) {
		int len = chlen(original);
		if ((newstring = typeMallocN(chtype, len + 4))) {
			for (int x = 0; x < len; x++) {
				newstring[x] = original[x];
			}
			newstring[len] = '\0';
			newstring[len + 1] = '\0';
		}
	}
	return (newstring);
}
#endif

/*
 * This safely erases a given window.
 */
void eraseCursesWindow(WINDOW *window)
{
	if (window) {
		werase(window);
		wrefresh(window);
	}
}

/*
 * This safely deletes a given window.
 */
void deleteCursesWindow(WINDOW *window)
{
	if (window) {
		eraseCursesWindow(window);
		delwin(window);
	}
}

/*
 * This moves a given window (if we're able to set the window's beginning).
 * We do not use mvwin(), because it does (usually) not move subwindows.
 */
void moveCursesWindow(WINDOW *window, int xdiff, int ydiff)
{
	if (window) {
		int xpos, ypos;
		getbegyx(window, ypos, xpos);
		(void)setbegyx(window,(short)ypos,(short)xpos);
		window->_begy=ypos;
		window->_begx=xpos;
		xpos += xdiff;
		ypos += ydiff;
		werase(window);
		(void)setbegyx(window,(short)ypos,(short)xpos);
	}
}


int comparSort(const void *a, const void *b)
{
	return strcmp(*(const char *const *)a,(*(const char *const *)b));
}

void sortList(CDK_CSTRING *list, int length)
{
	if (length > 1)
		qsort(list,(unsigned)length, sizeof(list[0]), comparSort);
}

/*
 * This looks for a subset of a word in the given list.
 */
int searchList(
#ifdef pneu
		vector<string> *plistp,
#else
		CDK_CSTRING2 list, int listSize, 
#endif
		const char *pattern)
{
	int Index = -1;
	/* Make sure the pattern isn't null. */
	if (pattern) {
		size_t len = strlen(pattern);
		/* Cycle through the list looking for the word. */
#ifdef pneu
		int x=0;
		for(vector<string>::const_iterator it0=plistp->begin();it0!=plistp->end();++x,++it0) {
			const int ret{strncmp(it0->c_str(),pattern,len)};
			if (ret<0) {
				Index=ret;
			} else {
				if (!ret) Index=x;
				break;
			}
		}
#else
	for (int x = 0; x < listSize; x++) {
		/* Do a string compare. */
		int ret = strncmp(list[x], pattern, len);
		/*
		 * If 'ret' is less than 0, then the current word is alphabetically
		 * less than the provided word.  At this point we will set the index
		 * to the current position.  If 'ret' is greater than 0, then the
		 * current word is alphabetically greater than the given word.  We
		 * should return with index, which might contain the last best match. 
			 * If they are equal, then we've found it.
			 */
			if (ret < 0) {
				Index = ret;
			} else {
				if (!ret)
					Index = x;
				break;
			}
		}
#endif
	}
	return Index;
}

/*
 * Add a new string to a list.  Keep a null pointer on the end so we can use
 * CDKfreeStrings() to deallocate the whole list.
 */
#ifdef pneu
void
#else
unsigned 
#endif
CDKallocStrings(
#ifdef pneu
		vector<string> *plistp,
#else
		char ***list, 
#endif
		char *item
#ifdef pneu
#else
		, unsigned length, unsigned used
#endif
		)
{
#ifdef pneu
	plistp->push_back(string(item));
#else
	unsigned need = 1;
	while (need < length + 2)
		need *= 2;
	if (need > used) {
		used = need;
		if (!*list) {
			*list = typeMallocN(char *, used);
		} else {
			*list = typeReallocN(char *, *list, used);
		}
	}
	(*list)[length++] = copyChar(item);
	(*list)[length] = 0;
	return used;
#endif
}

/*
 * Write a string of blanks, using writeChar().
 */
void writeBlanks(WINDOW *window, int xpos, int ypos, int align, int start, int end)
{
	if (start < end) {
#ifdef pneu
		string blanks(end-start+1000,' ');
		writeChar(window, xpos, ypos, blanks.c_str(), align, start, end);
#else
		unsigned want =(unsigned)(end - start) + 1000;
		char *blanks =(char *)malloc(want);
		if (blanks) {
			cleanChar(blanks,(int)(want - 1), ' ');
			writeChar(window, xpos, ypos, blanks, align, start, end);
			freeChecked(blanks);
		}
#endif
	}
}

/*
 * This writes out a char * string with no attributes.
 */
void writeChar(WINDOW *window,
		int xpos,
		int ypos,
		const char *string,
		int align,
		int start,
		int end)
{
	writeCharAttrib(window, xpos, ypos, string, A_NORMAL, align, start, end);
}

/*
 * This writes out a char * string with attributes.
 */
void writeCharAttrib(WINDOW *window,
		int xpos,
		int ypos,
		const char *string,
		chtype attr,
		int align,
		int start,
		int end)
{
	int display = end - start;
	int x;
	if (align == HORIZONTAL) {
		/* Draw the message on a horizontal axis. */
		display = MINIMUM(display, getmaxx(window) - 1);
		for (x = 0; x < display; x++) {
			(void)mvwaddch(window,
					ypos,
					xpos + x,
					(unsigned char)string[x + start] | attr);
		}
	} else {
		/* Draw the message on a vertical axis. */
		display = MINIMUM(display, getmaxy(window) - 1);
		for (x = 0; x < display; x++) {
			(void)mvwaddch(window,
					ypos + x,
					xpos,
					(unsigned char)string[x + start] | attr);
		}
	}
}

// die nächsten 3 aus display.c
/*
 * Given a character pointer, returns the equivalent display type.
 */
EDisplayType char2DisplayType(const char *string)
{
	/* *INDENT-OFF* */
	static const struct {
		const char *name;
		EDisplayType code;
	} table[] = {
		{ "CHAR",		vCHAR },
		{ "HCHAR",	vHCHAR },
		{ "INT",		vINT },
		{ "HINT",		vHINT },
		{ "UCHAR",	vUCHAR },
		{ "LCHAR",	vLCHAR },
		{ "UHCHAR",	vUHCHAR },
		{ "LHCHAR",	vLHCHAR },
		{ "MIXED",	vMIXED },
		{ "HMIXED",	vHMIXED },
		{ "UMIXED",	vUMIXED },
		{ "LMIXED",	vLMIXED },
		{ "UHMIXED",	vUHMIXED },
		{ "LHMIXED",	vLHMIXED },
		{ "VIEWONLY",	vVIEWONLY },
		{ 0,		vINVALID },
	};
	/* *INDENT-ON* */
	if (string) {
		for (int n = 0; table[n].name; n++) {
			if (!strcmp(string, table[n].name))
				return table[n].code;
		}
	}
	return (EDisplayType) vINVALID;
}

/*
 * Tell if a display type is "hidden"
 */
bool isHiddenDisplayType(EDisplayType type)
{
	bool result = FALSE;
	switch (type) {
		case vHCHAR:
		case vHINT:
		case vHMIXED:
		case vLHCHAR:
		case vLHMIXED:
		case vUHCHAR:
		case vUHMIXED:
			result = TRUE;
			break;
		case vCHAR:
		case vINT:
		case vINVALID:
		case vLCHAR:
		case vLMIXED:
		case vMIXED:
		case vUCHAR:
		case vUMIXED:
		case vVIEWONLY:
			result = FALSE;
			break;
	}
	return result;
}

/*
 * Given a character input, check if it is allowed by the display type,
 * and return the character to apply to the display, or ERR if not.
 */
int filterByDisplayType(EDisplayType type, chtype input)
{
	int result = (unsigned char)input;
	if (!isChar(input)) {
		result = ERR;
	} else if ((type == vINT ||
				type == vHINT) &&
			!isdigit((unsigned char)result)) {
		result = ERR;
	} else if ((type == vCHAR ||
				type == vUCHAR ||
				type == vLCHAR ||
				type == vUHCHAR ||
				type == vLHCHAR) &&
			isdigit((unsigned char)result)) {
		result = ERR;
	} else if (type == vVIEWONLY) {
		result = ERR;
	} else if ((type == vUCHAR ||
				type == vUHCHAR ||
				type == vUMIXED ||
				type == vUHMIXED) &&
			isalpha((unsigned char)result)) {
		result = toupper(result);
	} else if ((type == vLCHAR ||
				type == vLHCHAR ||
				type == vLMIXED ||
				type == vLHMIXED) &&
			isalpha((unsigned char)result)) {
		result = tolower(result);
	}
	return result;
}

/*
 * This is added to remain consistent.
 */
void endCDK(void)
{
   /* Turn echoing back on. */
   echo();
   /* Turn off cbreak. */
   nocbreak();
   /* End the curses windows. */
   endwin();
#ifdef HAVE_XCURSES
   XCursesExit();
#endif
}

static bool checkMenuKey(int keyCode, int functionKey)
{
	return (keyCode == KEY_ESC && !functionKey);
}
// aus cdk.c:

#ifdef pneu
#else
/*
 * Copy the given lists.
 */
char **copyCharList(const char **list)
{
	size_t size =(size_t) lenCharList(list) + 1;
	char **result = typeMallocN(char *, size);
	if (result) {
		unsigned n;
		for (n = 0; n < size; ++n)
			result[n] = copyChar(list[n]);
	}
	return result;
}
#endif

/*
 * Return the length of the given lists.
 */
int lenCharList(const char **list)
{
	int result = 0;
	if (list) {
		while (*list++)
			++result;
	}
	return result;
}

void initCDKColor(void)
{
#ifdef HAVE_START_COLOR
	if (has_colors()) {
		int color[] =
		{
			COLOR_WHITE, COLOR_RED, COLOR_GREEN,
			COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA,
			COLOR_CYAN, COLOR_BLACK
		};
		int pair = 1;
		int fg, bg;
		int limit;
		start_color();
		limit =(COLORS < MAX_COLORS) ? COLORS : MAX_COLORS;
		/* Create the color pairs. */
		for (fg = 0; fg < limit; fg++) {
			for (bg = 0; bg < limit; bg++) {
				init_pair((short)pair++,(short)color[fg],(short)color[bg]);
			}
		}
	}
#endif
}


/*
 * Returns the object on which the focus lies.
 */
CDKOBJS* SScreen::getCDKFocusCurrent()
{
   CDKOBJS *result = 0;
   int n = this->objectFocus;
   if (n >= 0 && n < this->objectCount)
      result = this->object[n];
   return result;
}

/*
 * Set focus to the next object, returning it.
 */
CDKOBJS *SScreen::setCDKFocusNext()
{
	CDKOBJS *result = 0;
	CDKOBJS *curobj;
	int n = getFocusIndex();
	int first = n;
	for (;;) {
		if (++n >= this->objectCount)
			n = 0;
		curobj = this->object[n];
		if (curobj && AcceptsFocusObj(curobj)) {
			result = curobj;
			break;
		} else {
			if (n == first) {
				break;
			}
		}
	}
	setFocusIndex((result) ? n : -1);
	return result;
}

/*
 * Set focus to the previous object, returning it.
 */
CDKOBJS* SScreen::setCDKFocusPrevious()
{
	CDKOBJS *result = 0;
	CDKOBJS *curobj;
	int n = getFocusIndex();
	int first = n;
	for (;;) {
		if (--n < 0)
			n = this->objectCount - 1;
		curobj = this->object[n];
		if (curobj && AcceptsFocusObj(curobj)) {
			result = curobj;
			break;
		} else if (n == first) {
			break;
		}
	}
	setFocusIndex((result) ? n : -1);
	return result;
}

/*
 * Set focus to a specific object, returning it.
 * If the object cannot be found, return null.
 */
CDKOBJS* SScreen::setCDKFocusCurrent(/*SScreen *screen, */CDKOBJS *newobj)
{
	CDKOBJS *result = 0;
	CDKOBJS *curobj;
	int n = getFocusIndex();
	int first = n;
	for (;;) {
		if (++n >= this->objectCount)
			n = 0;
		curobj = this->object[n];
		if (curobj == newobj) {
			result = curobj;
			break;
		} else if (n == first) {
			break;
		}
	}
	setFocusIndex(/*this, */(result) ? n : -1);
	return result;
}

/*
 * Set focus to the first object in the screen.
 */
CDKOBJS* SScreen::setCDKFocusFirst(/*SScreen *screen*/)
{
   setFocusIndex(/*screen, screen->*/objectCount - 1);
   return switchFocus(setCDKFocusNext(/*screen*/), 0);
}

/*
 * Set focus to the last object in the screen.
 */
/*CDKOBJS **/void SScreen::setCDKFocusLast(/*SScreen *screen*/)
{
   setFocusIndex(/*screen, */0);
   /*return */switchFocus(setCDKFocusPrevious(/*screen*/), 0);
}

/*
 * Save data in widgets on a screen
 */
void SScreen::saveDataCDKScreen(/*SScreen *screen*/)
{
   for (int i = 0; i < /*screen->*/objectCount; ++i)
      object[i]->/*saveDataObj*/saveDataCDK(/*screen->*//*object[i]*/);
}

/*
 * Refresh data in widgets on a screen
 */
void SScreen::refreshDataCDKScreen(/*SScreen *screen*/)
{
   for (int i = 0; i < /*screen->*/objectCount; ++i)
      object[i]->/*refreshDataObj*/refreshDataCDK(/*screen->*//*object[i]*/);
}

void SScreen::resetCDKScreen(/*SScreen *screen*/)
{
   refreshDataCDKScreen(/*screen*/);
}

void SScreen::exitOKCDKScreen(/*SScreen *screen*/)
{
   /*screen->*/exitStatus = CDKSCREEN_EXITOK;
}

void SScreen::exitCancelCDKScreen(/*SScreen *screen*/)
{
   /*screen->*/exitStatus = CDKSCREEN_EXITCANCEL;
}

void CDKOBJS::exitOKCDKScreenOf(/*CDKOBJS *obj*/)
{
   screen->exitOKCDKScreen(/*obj->screen*/);
}

void CDKOBJS::exitCancelCDKScreenOf(/*CDKOBJS *obj*/)
{
   screen->exitCancelCDKScreen(/*obj->screen*/);
}

void CDKOBJS::resetCDKScreenOf(/*CDKOBJS *obj*/)
{
   screen->resetCDKScreen(/*obj->screen*/);
}

void SScreen::traverseCDKOnce(/*SScreen *screen,*/
		CDKOBJS *curobj,
		int keyCode,
		bool functionKey,
		CHECK_KEYCODE funcMenuKey)
{
	switch (keyCode) {
		case KEY_BTAB:
			switchFocus(setCDKFocusPrevious(/*screen*/), curobj);
			break;
		case KEY_TAB:
			switchFocus(setCDKFocusNext(/*screen*/), curobj);
			break;
		case KEY_F(10):
			/* save data and exit */
			exitOKCDKScreen(/*screen*/);
			break;
		case CTRL('X'):
			exitCancelCDKScreen(/*screen*/);
			break;
		case CTRL('R'):
			/* reset data to defaults */
			resetCDKScreen(/*screen*/);
			curobj->setFocus();
			break;
		case CDK_REFRESH:
			/* redraw screen */
			refreshCDKScreen(/*screen*/); // oder object-> ?
			curobj->setFocus();
			break;
		default:
			/* not everyone wants menus, so we make them optional here */
			if (funcMenuKey && funcMenuKey(keyCode, functionKey)) {
				/* find and enable drop down menu */
				for (int j = 0; j < /*screen->*/objectCount; ++j)
					if (/*ObjTypeOf(*//*screen->*/object[j]->cdktype/*)*/ == vMENU) {
						handleMenu(/*screen, *//*screen->*/object[j], curobj);
						break;
					}
			} else {
				curobj->injectObj(/*curobj, */(chtype)keyCode);
			}
			break;
	}
}

/*
 * Traverse the widgets on a screen.
 */
int SScreen::traverseCDKScreen(/*SScreen *screen*/)
{
	int result = 0;
	CDKOBJS *curobj = setCDKFocusFirst(/*screen*/);
	if (curobj) {
		refreshDataCDKScreen(/*screen*/);
		/*screen->*/exitStatus = CDKSCREEN_NOEXIT;
		while (((curobj = getCDKFocusCurrent(/*screen*/))) && (/*screen->*/exitStatus == CDKSCREEN_NOEXIT)) {
			bool function;
			int key = curobj->getchCDKObject(/*curobj, */&function);
			traverseCDKOnce(/*screen, */curobj, key, function, checkMenuKey);
		}
		if (/*screen->*/exitStatus == CDKSCREEN_EXITOK) {
			saveDataCDKScreen(/*screen*/);
			result = 1;
		}
	}
	return result;
}


CDKOBJS* SScreen::handleMenu(/*SScreen *screen, */CDKOBJS *menu, CDKOBJS *oldobj)
{
	bool done = FALSE;
	CDKOBJS *newobj;
	switchFocus(menu, oldobj);
	while (!done) {
		bool functionKey;
		int key = menu->getchCDKObject(/*menu, */&functionKey);
		switch (key) {
			case KEY_TAB:
				done = TRUE;
				break;
			case KEY_ESC:
				/* cleanup the menu */
				//				((CDKMENU*)menu)->injectCDKMenu(/*(CDKMENU *)menu, */(chtype)key);
				menu->injectObj((chtype)key);
				done = TRUE;
				break;
			default:
//				done =(menu->injectCDKMenu(/*(CDKMENU *)menu, */(chtype)key) >= 0);
				done=(menu->injectObj(chtype(key))?menu->resultData.valueInt:unknownInt);
				break;
		}
	}
	if (!(newobj = this->getCDKFocusCurrent()))
		newobj = this->setCDKFocusNext();
	/*return */switchFocus(newobj, menu);
	return newobj;
}



void CDKOBJS::unsetFocus()
{
   curs_set(0);
//   if (obj) {
//      HasFocusObj(this) = FALSE;
			hasFocus=0;
      unfocusObj();
//   }
}

void CDKOBJS::setFocus()
{
//   if (obj) {
//      HasFocusObj(this) = TRUE;
			hasFocus=1;
      focusObj();
//   }
   curs_set(1);
}

CDKOBJS* switchFocus(CDKOBJS *newobj, CDKOBJS *oldobj)
{
   if (oldobj != newobj) {
      if (oldobj) oldobj->unsetFocus();
      if (newobj) newobj->setFocus();
   }
 return newobj;
}

/*
 * This registers a CDK object with a screen.
 */
void CDKOBJS::registerCDKObject(SScreen *screen, EObjectType cdktype)
{
#ifdef pneu
#else
	if (screen->objectCount + 1 >= screen->objectLimit) {
		screen->objectLimit += 2;
		screen->objectLimit *= 2;
		screen->object = typeReallocN(CDKOBJS *, screen->object, screen->objectLimit);
	}
#endif
	if (validObjType(cdktype)) {
#ifdef pneu
		setScreenIndex(screen);
		screen->objectCount++;
#else
		setScreenIndex(screen, screen->objectCount++);
#endif
	}
}

/*
 * This registers a CDK object with a screen.
 */
void CDKOBJS::reRegisterCDKObject(EObjectType cdktype/*, void *object*/)
{
//   CDKOBJS *obj =(CDKOBJS *)object;
   registerCDKObject(/*obj->*/screen, cdktype/*, object*/);
}

//#define validIndex(screen, n)((n) >= 0 &&(n) <(screen)->objectCount)
#define validIndex(n)((n) >= 0 &&(n) <(this)->objectCount)

void SScreen::swapCDKIndices(/*SScreen *screen, */int n1, int n2)
{
	if (n1 != n2 && validIndex(n1) && validIndex(n2)) {
#ifdef pneu
		iter_swap(object.begin()+n1,object.begin()+n2);
#else
		object[n2]->setScreenIndex(this, n1);
		object[n1]->setScreenIndex(this, n2);
#endif
		if (this->objectFocus == n1)
			this->objectFocus = n2;
		else if (this->objectFocus == n2)
			this->objectFocus = n1;
	}
}

/*
 * This 'brings' a CDK object to the top of the stack.
 */
void CDKOBJS::raiseCDKObject(EObjectType cdktype/*, void *object*/)
{
//   CDKOBJS *obj = (CDKOBJS *)object;
//   if (validObjType(obj, cdktype)) {
//      SScreen *screen = obj->screen;
      screen->swapCDKIndices(screenIndex, screen->objectCount - 1);
//   }
}

/*
 * This 'lowers' an object.
 */
void CDKOBJS::lowerCDKObject(EObjectType cdktype/*, void *object*/)
{
//   CDKOBJS *obj = (CDKOBJS *)object;
//   if (validObjType(obj, cdktype)) {
//      SScreen *screen = obj->screen;
      screen->swapCDKIndices(screenIndex, 0);
//   }
}



/*
 * Set the object's exit-type based on the input.
 * The .exitType field should have been part of the CDKOBJS struct, but it
 * is used too pervasively in older applications to move(yet).
 */
//void CDKOBJS::setCdkExitType(chtype ch)
void CDKOBJS::setExitType(chtype ch)
{
   switch (ch) {
   case KEY_ERROR:
      exitType = vERROR;
      break;
   case KEY_ESC:
      exitType = vESCAPE_HIT;
      break;
   case KEY_TAB:
   case KEY_ENTER:
      exitType = vNORMAL;
      break;
   case 0:
      exitType = vEARLY_EXIT;
      break;
   }
}

/*
 * Set indices so the screen and object point to each other.
 */
#ifdef pneu
// etwas andere Funktionalität
void CDKOBJS::setScreenIndex(SScreen *pscreen)
{
  screen=pscreen;
	screen->object.push_back(this);
	screenIndex=screen->object.size();
}
#else
void CDKOBJS::setScreenIndex(SScreen *pscreen, int number/*, CDKOBJS *obj*/)
{
	screenIndex = number;
	screen = pscreen;
	screen->object[number] = this;
}
#endif


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

CDKOBJS::~CDKOBJS()
{
	destroyCDKObject();
}

void CDKOBJS::destroyCDKObject()
{
	size_t pos{0};
	for(auto akt:all_objects) {
		if (akt==this) {
			all_objects.erase(all_objects.begin()+pos);
			break;
			akt->destroyObj();
			free(akt);
		}
		pos++;
	}
}

void CDKOBJS::destroyObj()
{
	destroyCDKObject();
}

/*
 * This removes an object from the CDK screen.
 */
void CDKOBJS::unregisterCDKObject(EObjectType cdktype/*, void *object*/)
{
	//   CDKOBJS *obj = (CDKOBJS *)object;
	if (validObjType(cdktype) && this->screenIndex >= 0) {
//		SScreen *screen = (this)->screen;
		if (screen) {
			int Index = (this)->screenIndex;
			this->screenIndex = -1;
#ifdef pneu
			screen->object.erase(screen->object.begin()+screenIndex);
			if (--screen->objectCount>0) {
#else
			/*
			 * Resequence the objects.
			 */
			for (int x = Index; x < screen->objectCount - 1; x++) {
				screen->object[x+1]->setScreenIndex(screen, x/*, screen->object[x + 1]*/);
			}
			if (screen->objectCount <= 1) {
				/* if no more objects, remove the array */
				freeAndNull(screen->object);
				screen->objectCount = 0;
				screen->objectLimit = 0;
			} else {
				/* Reduce the list by one object. */
				screen->object[screen->objectCount--] = 0;
				/*
				 * Update the object-focus
				 */
#endif
				if (screen->objectFocus == Index) {
					screen->objectFocus--;
					screen->setCDKFocusNext();
				} else if (screen->objectFocus > Index) {
					screen->objectFocus--;
				}
			}
		}
	}
}

/*
 * Resequence the numbers after a insertion/deletion.
 */
void SScroll::resequence(/*SScroll *scrollp*/)
{
	if (/*scrollp->*/numbers) {
		int j{0};
#ifdef pneu
		for(piter=pitem.begin();piter!=pitem.end();++j,piter++) {
#else
		for (; j < /*scrollp->*/listSize; ++j) {
#endif
			char source[80];
			chtype *target =  // eigentlich const chtype *target
#ifdef pneu
				piter->getinh();
#else
				/*scrollp->*/sitem[j];
#endif
			sprintf(source, NUMBER_FMT, j + 1, "");
			for (int k = 0; source[k]; ++k) {
				/* handle deletions that change the length of number */
				if (source[k] == '.' && (unsigned char)target[k] != '.') {
					int k2 = k;
					while ((target[k2] = target[k2 + 1]))
						++k2;
					/*scrollp->*/itemLen[j] -= 1;
				}
				target[k] &= A_ATTRIBUTES;
				target[k] |= (chtype)(unsigned char)source[k];
			}
		}
	}
}

#ifdef pneu
#else
bool SScroll::insertListItem(/*SScroll *scrollp, */int inr)
{
	for (int x = /*scrollp->*/listSize; x > inr; --x) {
		/*scrollp->*/sitem[x] = /*scrollp->*/sitem[x - 1];
		/*scrollp->*/itemLen[x] = /*scrollp->*/itemLen[x - 1];
		/*scrollp->*/itemPos[x] = /*scrollp->*/itemPos[x - 1];
	}
	return TRUE;
}
#endif

/*
 * This adds a single item to a scrolling list, at the end of the list.
 */
//#define AvailableWidth(w)  ((w)->boxWidth - 2 * BorderOf(w))
#define AvailableWidth() (boxWidth - 2 * borderSize)
// #define WidestItem(w)      ((w)->maxLeftChar + AvailableWidth(w))
#define WidestItem()      (maxLeftChar + AvailableWidth())
void SScroll::addCDKScrollItem(/*SScroll *scrollp,*/ const char *item)
{
   int itemNumber = /*scrollp->*/ listSize ;
   int widestItem = WidestItem(/*scrollp*//*this*/);
#ifdef pneu
#else
   char *temp = 0;
   size_t have = 0;
#endif
   if (
#ifdef pneu
#else
			 allocListArrays(/*scrollp, *//*scrollp->*/listSize, /*scrollp->*/listSize + 1) &&
#endif
       allocListItem(/*scrollp,*/
		      itemNumber,
#ifdef pneu
#else
		      &temp,
		      &have,
#endif
		      /*scrollp->*/numbers ?(itemNumber + 1) : 0,
		      item)) {
      /* Determine the size of the widest item. */
      widestItem = MAXIMUM(/*scrollp->*/itemLen[itemNumber], widestItem);
      updateViewWidth(/*scrollp, */widestItem);
      setViewSize(/*scrollp, *//*scrollp->*/ listSize 
#ifdef pneu
#else
					+ 1
#endif
					);
   }
#ifdef pneu
#else
   freeChecked(temp);
#endif
}

/*
 * This adds a single item to a scrolling list, before the current item.
 */
void SScroll::insertCDKScrollItem(/*SScroll *scrollp, */const char *item)
{
	int widestItem = WidestItem(/*scrollp*//*this*/);
#ifdef pneu
#else
	char *temp = 0;
	size_t have = 0;
#endif
	if (
#ifdef pneu
#else
			allocListArrays(/*scrollp, *//*scrollp->*/listSize, /*scrollp->*/listSize + 1) &&
			insertListItem(/*scrollp, *//*scrollp->*/currentItem) &&
#endif
			allocListItem(/*scrollp,*/
				/*scrollp->*/currentItem,
#ifdef pneu
#else
				&temp,
				&have,
#endif
				/*scrollp->*/numbers ?(/*scrollp->*/currentItem + 1) : 0,
				item))
	{
		/* Determine the size of the widest item. */
		widestItem = MAXIMUM(/*scrollp->*/itemLen[/*scrollp->*/currentItem], widestItem);
		updateViewWidth(/*scrollp, */widestItem);
		setViewSize(/*scrollp, *//*scrollp->*/ listSize 
#ifdef pneu
#else
					+ 1
#endif
				);
		resequence(/*scrollp*/);
	}
#ifdef pneu
#else
	freeChecked(temp);
#endif
}

/*
 * This removes a single item from a scrolling list.
 */
void SScroll::deleteCDKScrollItem(/*SScroll *scrollp, */int position)
{
#ifdef pneu
	if (position>=0 && position<listSize) {
		pitem.erase(pitem.begin()+position);
		listSize--;
    itemLen.erase(itemLen.begin()+position);
    itemPos.erase(itemPos.begin()+position);
#else
	if (position >= 0 && position < /*scrollp->*/listSize) {
		freeChtype(/*scrollp->*/sitem[position]);
		/* Adjust the list. */
		for (int x = position; x < /*scrollp->*/listSize; x++) {
			/*scrollp->*/sitem[x] = /*scrollp->*/sitem[x + 1];
			/*scrollp->*/itemLen[x] = /*scrollp->*/itemLen[x + 1];
			/*scrollp->*/itemPos[x] = /*scrollp->*/itemPos[x + 1];
		}
#endif
		setViewSize(/*scrollp, *//*scrollp->*/ listSize 
#ifdef pneu
#else
				- 1
#endif
				);
		if (/*scrollp->*/ listSize > 0)
			resequence(/*scrollp*/);
		if (/*scrollp->*/ listSize < /*m*/MaxViewSize(/*scrollp*/))
			werase(/*scrollp->*/win);	/* force the next redraw to be complete */
		/* do this to update the view size, etc. */
		setCDKScrollPosition(/*scrollp, *//*scrollp->*/currentItem);
	}
}

void SScroll::focusCDKScroll(/*CDKOBJS *object*/)
{
//   SScroll *scrollp =(SScroll *)object;
   drawCDKScrollCurrent(/*scrollp*/);
   wrefresh(/*scrollp->*/listWin);
}

void SScroll::unfocusCDKScroll(/*CDKOBJS *object*/)
{
//   SScroll *scrollp =(SScroll *)object;
   drawCDKScrollCurrent(/*scrollp*/);
   wrefresh(/*scrollp->*/listWin);
}


SEntry::~SEntry()
{
//		SEntry *entry =(SEntry *)object;
		cleanCdkTitle();
		//freeChtype(this->label);
		delete labelp;
#ifdef ineu
#else
		freeChecked(this->efld);
#endif
		/* Delete the windows. */
		deleteCursesWindow(this->fieldWin);
		deleteCursesWindow(this->labelWin);
		deleteCursesWindow(this->shadowWin);
		deleteCursesWindow(this->win);
		/* Clean the key bindings. */
		cleanCDKObjectBindings();
		/* Unregister this object. */
		unregisterCDKObject(vENTRY);
}

const int abstand{
	// wer rausfindet warum, bekommt einen Preis
#ifdef pneu
	2
#else
	1
#endif
}; // Abstand des Scrollfeldes vom Entryfeld
/*
 * This moves the entry field to the given location.
 */
void SEntry::moveCDKEntry(/*CDKOBJS *object,*/
		int xplace,
		int yplace,
		bool relative,
		bool refresh_flag)
{
	/* *INDENT-EQLS* */
//	SEntry *entry =(SEntry *)object;
	int currentX    = getbegx(this->win);
	int currentY    = getbegy(this->win);
	int xpos        = xplace;
	int ypos        = yplace;
	int xdiff       = 0;
	int ydiff       = 0;
	/*
	 * If this is a relative move, then we will adjust where we want
	 * to move to.
	 */
	if (relative) {
		xpos = getbegx(this->win) + xplace;
		ypos = getbegy(this->win) + yplace;
	}
	/* Adjust the window if we need to. */
	alignxy(WindowOf(this), &xpos, &ypos, this->boxWidth, this->boxHeight);
	/* Get the difference. */
	xdiff = currentX - xpos;
	ydiff = currentY - ypos;
	/* Move the window to the new location. */
	moveCursesWindow(this->win, -xdiff, -ydiff);
	moveCursesWindow(this->fieldWin, -xdiff, -ydiff);
	moveCursesWindow(this->labelWin, -xdiff, -ydiff);
	moveCursesWindow(this->shadowWin, -xdiff, -ydiff);
	/* Touch the windows so they 'move'. */
	refreshCDKWindow(WindowOf(this));
	/* Redraw the window, if they asked for it. */
	if (refresh_flag) {
		drawCDKEntry(obbox);
	}
}

/*
 * This moves the alphalist field to the given location.
 */
void SAlphalist::moveCDKAlphalist(
			       int xplace,
			       int yplace,
			       bool relative,
			       bool refresh_flag)
{
//	SAlphalist *alphalist = (SAlphalist *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx(this->win);
   int currentY = getbegy(this->win);
   int xpos     = xplace;
   int ypos     = yplace;
   int xdiff    = 0;
   int ydiff    = 0;
   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative) {
      xpos = getbegx(this->win) + xplace;
      ypos = getbegy(this->win) + yplace;
   }
   /* Adjust the window if we need to. */
   alignxy(WindowOf(this), &xpos, &ypos, this->boxWidth, this->boxHeight);
   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;
   /* Move the window to the new location. */
   moveCursesWindow(this->win, -xdiff, -ydiff);
   moveCursesWindow(this->shadowWin, -xdiff, -ydiff);
   /* Move the sub-widgets. */
   entryField->moveCDKEntry(xplace, yplace, relative, FALSE);
   scrollField->moveCDKScroll(xplace, yplace+abstand+obbox, relative, FALSE);
   /* Touch the windows so they 'move'. */
   refreshCDKWindow(WindowOf(this));
   /* Redraw the window, if they asked for it. */
   if (refresh_flag) {
      drawCDKAlphalist(obbox);
   }
} // void SAlphalist::moveCDKAlphalist(

/*
 * This moves the fselect field to the given location.
 */
void SFSelect::moveCDKFselect(/*CDKOBJS *object,*/
			     int xplace,
			     int yplace,
			     bool relative,
			     bool refresh_flag)
{
	//   SFSelect *fselect =(SFSelect *)object;
	/* *INDENT-EQLS* */
	int currentX = getbegx(this->win);
	int currentY = getbegy(this->win);
	int xpos     = xplace;
	int ypos     = yplace;
	int xdiff    = 0;
	int ydiff    = 0;
	/*
	 * If this is a relative move, then we will adjust where we want
	 * to move to.
	 */
	if (relative) {
		xpos = getbegx(this->win) + xplace;
		ypos = getbegy(this->win) + yplace;
	}
	/* Adjust the window if we need to. */
	alignxy(WindowOf(this), &xpos, &ypos, this->boxWidth, this->boxHeight);
	/* Get the difference. */
	xdiff = currentX - xpos;
	ydiff = currentY - ypos;
	/* Move the window to the new location. */
	moveCursesWindow(this->win, -xdiff, -ydiff);
	moveCursesWindow(this->shadowWin, -xdiff, -ydiff);
	/* Move the sub-widgets. */
	entryField->moveCDKEntry(xplace, yplace, relative, FALSE);
	scrollField->moveCDKScroll(xplace, yplace+abstand+obbox, relative, FALSE);
	/* Redraw the window, if they asked for it. */
	if (refresh_flag) {
		drawCDKFselect(/*this, ObjOf(this)->*/obbox);
	}
}

/*
 * This means you want to use the given file selector. It takes input
 * from the keyboard, and when it's done, it fills the entry info
 * element of the structure with what was typed.
 */
const char *SFSelect::activateCDKFselect(/*SFSelect *fselect, */chtype *actions)
{
	chtype input = 0;
	bool functionKey;
	const char *ret = 0;
	/* Draw the widget. */
	drawCDKFselect(/*fselect, ObjOf(fselect)->*/obbox);
	if (!actions) {
		for (;;) {
			input =(chtype)getchCDKObject(/*ObjOf(fselect->entryField), */&functionKey);
			/* Inject the character into the widget. */
			ret=injectCDKFselect(input)?resultData.valueString:0/*unknownString*/;
			if (this->exitType != vEARLY_EXIT) {
				return ret;
			}
		}
	} else {
		int length = chlen(actions);
		/* Inject each character one at a time. */
		for (int x = 0; x < length; x++) {
			ret=injectCDKFselect(actions[x])?resultData.valueString:0/*unknownString*/;
			if (this->exitType != vEARLY_EXIT) {
				return ret;
			}
		}
	}
	/* Set the exit type and exit. */
	setExitType(/*fselect, */0);
	return 0;
}

/*
 * This moves the scroll field to the given location.
 */
void SScroll::moveCDKScroll(
			    int xplace,
			    int yplace,
			    bool relative,
			    bool refresh_flag)
{
   /* *INDENT-EQLS* */
//   SScroll *scrollp = (SScroll *)object;
   int currentX       = getbegx(this->win);
   int currentY       = getbegy(this->win);
   int xpos           = xplace;
   int ypos           = yplace;
   int xdiff          = 0;
   int ydiff          = 0;
   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative) {
      xpos = getbegx(this->win) + xplace;
      ypos = getbegy(this->win) + yplace;
   }
   /* Adjust the window if we need to. */
   alignxy(WindowOf(this), &xpos, &ypos, this->boxWidth, this->boxHeight);
   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;
   /* Move the window to the new location. */
   moveCursesWindow(this->win, -xdiff, -ydiff);
   moveCursesWindow(this->listWin, -xdiff, -ydiff);
   moveCursesWindow(this->shadowWin, -xdiff, -ydiff);
   moveCursesWindow(this->scrollbarWin, -xdiff, -ydiff);
   /* Touch the windows so they 'move'. */
   refreshCDKWindow(WindowOf(this));
   /* Redraw the window, if they asked for it. */
   if (refresh_flag) {
		 // hier entstehen keine Fehler
      drawCDKScroll(obbox,1);
   }
}


void SAlphalist::destroyInfo()
{
#ifdef pneu
	plist.clear();
#else
   CDKfreeStrings(slist);
   slist = 0;
   listSize = 0;
#endif
}

SAlphalist::~SAlphalist()
{
//      SAlphalist *alphalist =(SAlphalist *)object;

      destroyInfo();
      /* Clean the key bindings. */
      cleanCDKObjectBindings();
//      destroyCDKEntry(this->entryField);
			entryField->~SEntry();
//      destroyCDKScroll(this->scrollField);
			scrollField->~SScroll();
      /* Free up the window pointers. */
      deleteCursesWindow(this->shadowWin);
      deleteCursesWindow(this->win);
      /* Unregister the object. */
      unregisterCDKObject(vALPHALIST);
}


SFSelect::~SFSelect()
{
//      SAlphalist *alphalist = (SAlphalist *)object;

	/*
      destroyInfo();
      cleanCDKObjectBindings();
//      destroyCDKEntry(this->entryField);
			entryField->~SEntry();
//      destroyCDKScroll(this->scrollField);
			scrollField->~SScroll();
      deleteCursesWindow(this->shadowWin);
      deleteCursesWindow(this->win);
      unregisterCDKObject(vALPHALIST);
			*/

//   if (object) {
//      SFSelect *fselect =(SFSelect *)object;

      cleanCDKObjectBindings(/*vFSELECT, fselect*/);

      /* Free up the character pointers. */
#ifdef pneu
#else
      freeChecked(this->pwd);
      freeChecked(this->pathname);
#endif
#ifdef pneu
#else
      freeChecked(this->dirAttribute);
      freeChecked(this->fileAttribute);
      freeChecked(this->linkAttribute);
      freeChecked(this->sockAttribute);
#endif
#ifdef pneu
#else
      CDKfreeStrings(this->dirContents);
#endif

      /* Destroy the other Cdk objects. */
			//      destroyCDKScroll(this->scrollField);
			scrollField->destroyObj();
			//      destroyCDKEntry(this->entryField);
			entryField->~SEntry();

      /* Free up the window pointers. */
      deleteCursesWindow(this->shadowWin);
      deleteCursesWindow(this->win);

      /* Clean the key bindings. */
      /* Unregister the object. */
      unregisterCDKObject(vFSELECT/*, this*/);
//   }
}

/*
 * This function sets the pre-process function.
 */
void SAlphalist::setCDKAlphalistPreProcess(
				PROCESSFN callback,
				void *data)
{
   entryField->setCDKObjectPreProcess(callback, data);
}

/*
 * This function sets the post-process function.
 */
void SAlphalist::setCDKAlphalistPostProcess(
				 PROCESSFN callback,
				 void *data)
{
   entryField->setCDKObjectPostProcess(callback, data);
}



/*
 * This function draws the scrolling list widget.
 */
void SScroll::drawCDKScroll(bool Box,bool obmit)
{
//   SScroll *scrollp =(SScroll *)object;
   /* Draw in the shadow if we need to. */
   if (this->shadowWin)
      drawShadow(this->shadowWin);
   drawCdkTitle(this->win);
   /* Draw in the scolling list items. */
	 // Kommentar GSchade 0 11.11.18
	 // GSchade: auskommentieren und dann noch vor dem Wechsel zu anderem alle übrigen zeichnen
	 if (akteinbart==einb_alphalist &&obmit) {
		 drawCDKScrollList(Box); wrefresh(parent); // gleichbedeutend: wrefresh(obj.screen->window);
	 }
}


/*
 * This function destroys
 */
SScroll::~SScroll(/*CDKOBJS *object*/)
{
		//SScroll *scrollp =(SScroll *)object;
		cleanCdkTitle();
#ifdef pneu
#else
		CDKfreeChtypes(this->sitem);
		freeChecked(this->itemPos);
		freeChecked(this->itemLen);
#endif
		/* Clean up the windows. */
		deleteCursesWindow(this->scrollbarWin);
		deleteCursesWindow(this->shadowWin);
		deleteCursesWindow(this->listWin);
		deleteCursesWindow(this->win);
		/* Clean the key bindings. */
		cleanCDKObjectBindings();
		/* Unregister this object. */
		unregisterCDKObject(vSCROLL);
}

#ifdef pneu
void aufSplit(vector<string> *tokens, const char* const text, const char sep/*=' '*/,bool auchleer/*=1*/)
{
  aufSplit(tokens,string(text),sep,auchleer);
} // void aufSplit(vector<string> *tokens, const char *text, const char sep/*=' '*/, bool auchler/*=1*/)

void aufSplit(vector<string> *tokens, const string& text, const char sep/*=' '*/,bool auchleer/*=1*/)
{
	size_t start = 0, end = 0;
	tokens->clear();
	while ((end = text.find(sep, start)) != string::npos) {
		if (end!=start || auchleer) {
			tokens->push_back(text.substr(start,end-start));
		} // 		if (!akttok.empty() || auchleer)
		start = end + 1;
	} //   while ((end = text.find(sep, start)) !=(int)string::npos)
	if (text.length() !=start || auchleer)
		tokens->push_back(text.substr(start));
} // void aufSplit(vector<string> *tokens, const string& text, const char sep,bool auchleer/*=1*/)
#else

/*
 * Split a string into a list of strings.
 */
char **CDKsplitString(const char *string, int separator)
{
	char **result = 0;
	char *temp;

	if (string && *string) {
		unsigned need = countChar(string, separator) + 2;
		if ((result = typeMallocN(char *, need))) {
			unsigned nr = 0;
			const char *first = string;
			for (;;) {
				while (*string && *string != separator)
					string++;

				need =(unsigned)(string - first);
				if (!(temp = typeMallocN(char, need + 1)))
					break;

				memcpy(temp, first, need);
				temp[need] = 0;
				result[nr++] = temp;

				if (!*string++)
					break;
				first = string;
			}
			result[nr] = 0;
		}
	}
	return result;
}

static unsigned countChar(const char *string, int separator)
{
	unsigned result = 0;
	int ch;
	while ((ch = *string++)) {
		if (ch == separator)
			result++;
	}
	return result;
}
#endif

/*
 * Set the widget's title.
 */
int CDKOBJS::setCdkTitle(const char *titlec, int boxWidth)
{
	cleanCdkTitle();
	if (titlec) {
#if pneu
		vector<string> temp;
#else
		char **temp = 0;
#endif
		int titleWidth;
		int x;
		int len;
		int align;

		/* We need to split the title on \n. */
#ifdef pneu
		aufSplit(&temp,titlec,'\n');
		titleLines=temp.size();
		titlePos = new int[titleLines];
		titleLen=new int[titleLines];
#else
		temp = CDKsplitString(titlec, '\n');
		titleLines = (int)CDKcountStrings((CDK_CSTRING2)temp);
		//title = typeCallocN(chtype *, titleLines + 1);
		titlePos = typeCallocN(int, titleLines + 1);
		titleLen = typeCallocN(int, titleLines + 1);
#endif
		if (boxWidth >= 0) {
			int maxWidth = 0;
			/* We need to determine the widest title line. */
			for (x = 0; x < titleLines; x++) {
//				chtype *holder = char2Chtypeh(temp[x], &len, &align);
				chtstr holder(temp[x]
#ifdef pneu
	 					.c_str()
#else
#endif
						,&len,&align);
				maxWidth = MAXIMUM(maxWidth, len);
//				freeChtype(holder);
			}
			boxWidth = MAXIMUM(boxWidth, maxWidth + 2 * borderSize);
		} else {
			boxWidth = -(boxWidth - 1);
		}

		/* For each line in the title, convert from char * to chtype * */
		titleWidth = boxWidth - (2 * borderSize);
		for (x = 0; x < titleLines; x++) {
//			title[x] = char2Chtypeh(temp[x], &titleLen[x], &titlePos[x]);
			titles.push_back(chtstr(temp[x]
#ifdef pneu
	 					.c_str()
#else
#endif
						,&titleLen[x],&titlePos[x]));
			titlePos[x] = justifyString(titleWidth, titleLen[x], titlePos[x]);
		}
#ifdef pneu
#else
		CDKfreeStrings(temp);
#endif
	}
	return boxWidth;
} // int CDKOBJS::setCdkTitle(const char *title, int boxWidth)

/*
 * Draw the widget's title.
 */
void CDKOBJS::drawCdkTitle(WINDOW *win)
{
	for (int x = 0; x < titleLines; x++) {
		writeChtype(win,
				titlePos[x] + borderSize,
				x + borderSize,
				titles[x].getinh(),
				HORIZONTAL, 0,
				titleLen[x]);
	}
}


/*
 * Remove storage for the widget's title.
 */
void CDKOBJS::cleanCdkTitle()
{
//	CDKfreeChtypes(title);
//	title = 0;
	freeAndNull(titlePos);
	freeAndNull(titleLen);
	titleLines = 0;
}

bool CDKOBJS::validObjType(EObjectType type)
{
	bool valid = FALSE;
	//   if (obj && ObjTypeOf(obj) == type) {
	switch (type) {
		case vALPHALIST:
		case vBUTTON:
		case vBUTTONBOX:
		case vCALENDAR:
		case vDIALOG:
		case vDSCALE:
		case vENTRY:
		case vFSCALE:
		case vFSELECT:
		case vFSLIDER:
		case vGRAPH:
		case vHISTOGRAM:
		case vITEMLIST:
		case vLABEL:
		case vMARQUEE:
		case vMATRIX:
		case vMENTRY:
		case vMENU:
		case vRADIO:
		case vSCALE:
		case vSCROLL:
		case vSELECTION:
		case vSLIDER:
		case vSWINDOW:
		case vTEMPLATE:
		case vUSCALE:
		case vUSLIDER:
		case vVIEWER:
			valid = TRUE;
			break;
		case vTRAVERSE:		/* not really an object */
		case vNULL:
			break;
	}
	//	 }
	return valid;
}

/*
 * The cdktype parameter passed to bindCDKObject, etc., is redundant since
 * the object parameter also has the same information.  For compatibility
 * just use it for a sanity check.
 */

#ifndef KEY_MAX
#define KEY_MAX 512
#endif

CDKOBJS* CDKOBJS::bindableObject()
{
	return this;
}

CDKOBJS* SFSelect::bindableObject()
{
	return entryField;
}

CDKOBJS* SAlphalist::bindableObject()
{
	return entryField;
}

/*
 * This inserts a binding.
 */
void CDKOBJS::bindCDKObject(
		    chtype key,
		    BINDFN function,
		    void *data)
{
	CDKOBJS *obj = bindableObject();
#ifdef bneu
	if (obj) {
		if (key==9) {
			int test{0};
		}
		obj->bindv[key]=CDKBINDING(function,data);
	}
#else
	if ((key < KEY_MAX) && obj) {
		if (key && (unsigned)key >= obj->bindingCount) {
			unsigned next = (unsigned) (key + 1);
			if (obj->bindingList)
				obj->bindingList = typeReallocN(CDKBINDING, obj->bindingList, next);
			else
				obj->bindingList = typeMallocN(CDKBINDING, next);
			memset(&(obj->bindingList[obj->bindingCount]), 0,(next - obj->bindingCount) * sizeof(CDKBINDING));
			obj->bindingCount = next;
		}
		if (obj->bindingList) {
			obj->bindingList[key].bindFunction = function;
			obj->bindingList[key].bindData = data;
		}
	}
#endif
}

/*
 * This removes a binding on an object.
 */
void CDKOBJS::unbindCDKObject(chtype key)
{
	CDKOBJS *obj = bindableObject();
#ifdef bneu
	if (obj) {
		obj->bindv.erase(key);
	}
#else
	if (obj &&((unsigned)key < obj->bindingCount)) {
		obj->bindingList[key].bindFunction = 0;
		obj->bindingList[key].bindData = 0;
	}
#endif
}

/*
 * This removes all the bindings for the given objects.
 */
void CDKOBJS::cleanCDKObjectBindings()
{
	CDKOBJS *obj = bindableObject();
#ifdef bneu
	if (obj) {
		obj->bindv.clear();
	}
#else
	if (obj && obj->bindingList) {
		for (unsigned x = 0; x < obj->bindingCount; x++) {
			(obj)->bindingList[x].bindFunction = 0;
			(obj)->bindingList[x].bindData = 0;
		}
		freeAndNull((obj)->bindingList);
	}
#endif
}


/*
 * This checks to see if the binding for the key exists:
 * If it does then it runs the command and returns its value, normally TRUE.
 * If it doesn't it returns a FALSE.  This way we can 'overwrite' coded
 * bindings.
 */
int CDKOBJS::checkCDKObjectBind(chtype key)
{
	CDKOBJS *obj = bindableObject();
#ifdef bneu
	if (obj) {
		if (obj->bindv.find(key)!=obj->bindv.end()) {
			BINDFN function=obj->bindv[key].bindFunction;
			void *data=obj->bindv[key].bindData;
			return function(cdktype, this, data, key);
		}
	}
#else
	if (obj && ((unsigned)key < obj->bindingCount)) {
		if ((obj)->bindingList[key].bindFunction) {
			BINDFN function = obj->bindingList[key].bindFunction;
			void *data = obj->bindingList[key].bindData;
			return function(cdktype, this, data, key);
		}
	}
#endif
	return(FALSE);
}

/*
 * This checks to see if the binding for the key exists.
 */
bool CDKOBJS::isCDKObjectBind(chtype key)
{
	bool result = FALSE;
	CDKOBJS *obj = bindableObject();
#ifdef bneu
	if (obj) {
		if (obj->bindv.find(key)!=obj->bindv.end()) {
			result=TRUE;
		}
	}
#else
	if (obj && ((unsigned)key < obj->bindingCount)) {
		if ((obj)->bindingList[key].bindFunction)
			result = TRUE;
	}
#endif
	return (result);
}


/*
 * Draw a box around the given window using the object's defined line-drawing
 * characters.
 */
void CDKOBJS::drawObjBox(WINDOW *win)
{
	attrbox(win,
			ULChar,
			URChar,
			LLChar,
			LRChar,
			HZChar,
			VTChar,
			BXAttr);
}

/*
 * Read from the input window, filtering keycodes as needed.
 */
int CDKOBJS::getcCDKObject()
{
	// EObjectType cdktype = ObjTypeOf(this);
	CDKOBJS *test = bindableObject();
	int result = wgetch(InputWindowOf(this));
	// printf("%c %ul\n",result,result); //G.Schade
#ifdef bneu
	BINDFN fn{0};
	CDKBINDING *bnd{0}; 
	if (result>=0 && test) {
		bindvit=test->bindv.find(result);
	}
	if (result>=0 && test && bindvit!=test->bindv.end() && bindvit->second.bindFunction == getcCDKBind) {
		result=(int)(long)test->bindv[result].bindData;
	} else if (!test || bindvit==test->bindv.end() || !bindvit->second.bindFunction) {
		switch (result) {
			case '\r':
			case '\n':
				result = KEY_ENTER;
				break;
			case '\t':
				result = KEY_TAB;
				break;
			case DELETE:
				result = KEY_DC;
				break;
			case '\b':		/* same as CTRL('H'), for ASCII */
				result = KEY_BACKSPACE;
				break;
			case CDK_BEGOFLINE:
				result = KEY_HOME;
				break;
			case CDK_ENDOFLINE:
				result = KEY_END;
				break;
			case CDK_FORCHAR:
				result = KEY_RIGHT;
				break;
			case CDK_BACKCHAR:
				result = KEY_LEFT;
				break;
			case CDK_NEXT:
				result = KEY_TAB;
				break;
			case CDK_PREV:
				result = KEY_BTAB;
				break;
		}
	}
#else
	if (result >= 0
			&& test
			&& (unsigned)result < test->bindingCount
			&& test->bindingList[result].bindFunction == getcCDKBind) {
		result =(int)(long)test->bindingList[result].bindData;
	} else if (!test 
			|| (unsigned)result >= test->bindingCount
			|| !test->bindingList[result].bindFunction) {
		switch (result) {
			case '\r':
			case '\n':
				result = KEY_ENTER;
				break;
			case '\t':
				result = KEY_TAB;
				break;
			case DELETE:
				result = KEY_DC;
				break;
			case '\b':		// same as CTRL('H'), for ASCII 
				result = KEY_BACKSPACE;
				break;
			case CDK_BEGOFLINE:
				result = KEY_HOME;
				break;
			case CDK_ENDOFLINE:
				result = KEY_END;
				break;
			case CDK_FORCHAR:
				result = KEY_RIGHT;
				break;
			case CDK_BACKCHAR:
				result = KEY_LEFT;
				break;
			case CDK_NEXT:
				result = KEY_TAB;
				break;
			case CDK_PREV:
				result = KEY_BTAB;
				break;
		}
	}
#endif
	return result;
} // int CDKOBJS::getcCDKObject()

/*
 * Use this function rather than getcCDKObject(), since we can extend it to
 * handle wide-characters.
 */
int CDKOBJS::getchCDKObject(bool *functionKey)
{
   int key = getcCDKObject();
   *functionKey = (key >= KEY_MIN && key <= KEY_MAX);
   return key;
}


void SEntry::CDKEntryCallBack(chtype character)
{
	SEntry::schreibl(character);
}

/*
 * This is a generic character parser for the entry field. It is used as a
 * callback function, so any personal modifications can be made by creating
 * a new function and calling the activation with its name.
 */
void SEntry::schreibl(chtype character)
{
  static bool altobuml=0;
  const bool obuml=(character==(chtype)-61||character==(chtype)-62);
  int plainchar;
  if (altobuml||obuml) plainchar=character; else plainchar=filterByDisplayType(dispType, character);
	// wenn Ende erreicht wuerde, dann von 2-Buchstabenlaengen langen Buchstaben keinen schreiben
#ifdef ineu
//	const int slen=strlen(efld.c_str());
	const int slen=efld.length();
#else
	const int slen=strlen(efld);
#endif
  if (plainchar == ERR ||(obuml&&slen>max-2)||(altobuml&&slen>max-2&&efld[slen-1]!=-61&&efld[slen-1]!=-62)||(slen >= max)) {
    Beep();
  } else {
    /* Update the screen and pointer. */
    if (sbuch != fieldWidth - 1) {
#ifdef ineu
			efld.insert(efld.begin()+screenCol+leftChar,(char)plainchar);
#else
      for (int x = slen; x >(screenCol + leftChar); x--) {
        efld[x] = efld[x - 1];
      }
      efld[screenCol + leftChar] =(char)plainchar;
#endif
      screenCol++;
      if (!obuml) sbuch++;
    } else {
      /* Update the character pointer. */
      size_t temp = slen;
#ifdef ineu
			efld.resize(temp+1);
#else
      efld[temp + 1] = '\0';
#endif
      efld[temp] =(char)plainchar;
			if (obuml) {
				screenCol++;
			} else {
        /* Do not update the pointer if it's the last character */
        if ((int)(temp + 1) < max) {
          lbuch++;
          if (efld[leftChar]==-61||efld[leftChar]==-62) {
						leftChar++;
						screenCol--;
					}
          leftChar++;
        }
      }
    }
    /* Update the entry field. */
    if (!obuml) {
      zeichneFeld();
    }
  }
  altobuml=obuml;
}

/*
 * This redraws the entry field.
 */
void SEntry::zeichneFeld()
{
	// setlocale(LC_ALL,"");
	int x = 0;
	/* Set background color and attributes of the entry field */
	wbkgd(fieldWin, fieldAttr);
	/* Draw in the filler characters. */
	(void)mvwhline(fieldWin, 0, x, filler | fieldAttr, fieldWidth);
	/* If there is information in the field. Then draw it in. */
#ifdef ineu
//	if (!efld.empty()) {
	if (1) {
//		const int infoLength =(int)strlen(efld.c_str());
		const int infoLength = efld.length();
#else
	if (efld) {
		const int infoLength =(int)strlen(efld);
#endif
		/* Redraw the field. */
		if (isHiddenDisplayType(dispType)) {
			for (x = leftChar; x < infoLength; x++) {
				(void)mvwaddch(fieldWin, 0, x - leftChar, hidden | fieldAttr);
			}
		} else {
			if (0) {
				char ausgabe[infoLength-leftChar+1];
#ifdef ineu
				memcpy(ausgabe,&efld[leftChar],infoLength-leftChar);
#else
				memcpy(ausgabe,efld+leftChar,infoLength-leftChar);
#endif
				ausgabe[infoLength-leftChar]=0;
			} else if (0) {
				/*
				mvwprintw(parent,1,1,"x:%i,len:%i,fwidth:%i,max:%i,lChar:%i,lbuch:%i,sCol:%i,sbuch:%i,info:%s   ",x,infoLength,fieldWidth,max,leftChar,lbuch,screenCol,sbuch,info);
				for (x = leftChar; x < infoLength; x++) {
					mvwprintw(parent,2+x,2,"x:%i, info[x]:%i  ",x,info[x]);
				}
				mvwprintw(parent,2+infoLength,2,"                            ");
				mvwprintw(parent,2+infoLength+1,2,"                            ");
				*/
				wrefresh(parent); // gleichbedeutend: wrefresh(obj.screen->window);
			}
			size_t aktumlz=0;
			for (x = leftChar; x < infoLength; x++) {
				if (efld[x]==-61 || efld[x]==-62) {
					char ausgb[3]={0};
					ausgb[0]=efld[x];
					ausgb[1]=efld[x+1];
					//GSchade: Hier Umlautausgabe
					mvwprintw(fieldWin,0,x-leftChar-aktumlz,ausgb);
					x++;
					aktumlz++;
				} else {
					(void)mvwaddch(fieldWin, 0, x - leftChar-aktumlz,(unsigned char)efld[x] | fieldAttr);
				}
			}
		}
		wmove(fieldWin, 0, sbuch);
	}
	wrefresh(fieldWin);
} // void SEntry::zeichneFeld


/*
 * This creates a pointer to an entry widget.
 */
SEntry::SEntry(SScreen *cdkscreen,
		int xplace,
		int yplace,
		const char *title,
		const char *labelstr,
		chtype fieldAttrp,
		chtype fillerp,
		EDisplayType dispTypep,
		int fWidth,
		int minp,
		int maxp,
		bool Box,
		bool shadowp,
		// GSchade Anfang
		int highnr/*=0*/
		// GSchade Ende
		):fieldWidth(fWidth),boxWidth(0)
{
	// GSchade Anfang
	cdktype=vENTRY;
	// GSchade Ende
	/* *INDENT-EQLS* */
	int parentWidth      = getmaxxf(cdkscreen->window);
	int parentHeight     = getmaxyf(cdkscreen->window);
	int xpos             = xplace;
	int ypos             = yplace;
	int junk             = 0;
	int horizontalAdjust, oldWidth;

	//	if ((entry = newCDKObject(SEntry, &my_funcs)) == 0) return (0);
	::CDKOBJS();
	setBox(Box);
	boxHeight =(borderSize * 2) + 1;

	/*
	 * If the fieldWidth is a negative value, the fieldWidth will
	 * be COLS-fieldWidth, otherwise, the fieldWidth will be the
	 * given width.
	 */
	fieldWidth = setWidgetDimension(parentWidth, fieldWidth, 0);
	boxWidth = fieldWidth + 2 * borderSize;

	/* Set some basic values of the entry field. */
	//label = 0;
	labelLen = 0;
	labelWin = 0;
	labelumlz=0; // GSchade

	// GSchade

	/* Translate the label char *pointer to a chtype pointer. */
	if (labelstr) {
//		label = char2Chtypeh(labelstr, &labelLen, &junk /* GSchade Anfang*/ ,highnr /* GSchade Ende*/);
		labelp=new chtstr(labelstr,&labelLen,&junk,highnr);
		// GSchade Anfang
		for(int i=0;labelp->getinh()[i];i++) {
			if ((int)((unsigned char)labelp->getinh()[i])==194 ||(int)((unsigned char)labelp->getinh()[i])==195) {
				labelumlz++;
			}
		}
		// GSchade Ende
		boxWidth += labelLen;
	}

	oldWidth = boxWidth;
	boxWidth = setCdkTitle(title, boxWidth);
	horizontalAdjust =(boxWidth - oldWidth) / 2;

	boxHeight += TitleLinesOf(this);

	/*
	 * Make sure we didn't extend beyond the dimensions of the window.
	 */
	boxWidth = MINIMUM(boxWidth, parentWidth);
	boxHeight = MINIMUM(boxHeight, parentHeight);
	fieldWidth = MINIMUM(fieldWidth,
			boxWidth - labelLen +labelumlz - 2 * borderSize);

	/* Rejustify the x and y positions if we need to. */
	alignxy(cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

	/* Make the label window. */
	win = newwin(boxHeight, boxWidth, ypos, xpos);
	if (!win) {
		destroyCDKObject();
		return;
	} else {
		keypad(win, TRUE);

		/* Make the field window. */
		fieldWin = subwin(win, 1, fieldWidth,
				(ypos + TitleLinesOf(this) + borderSize),
				(xpos + labelLen -labelumlz
				 + horizontalAdjust
				 + borderSize));
		if (!fieldWin) {
			destroyCDKObject();
		} else {
			keypad(fieldWin, TRUE);

			/* Make the label win, if we need to. */
			if (labelstr) {
				labelWin = subwin(win, 1, labelLen,
						ypos + TitleLinesOf(this) + borderSize,
						xpos + horizontalAdjust + borderSize);
			}

			/* Make room for the info char * pointer. */
#ifdef ineu
			// info ist vorher noch leer
			//efld.resize(maxp+3);
			efld.reserve(maxp+3);
			{
#else
			efld = typeMallocN(char, maxp + 3);
			if (!efld) {
				destroyCDKObject();
			} else {
				cleanChar(efld, maxp + 3, '\0');
#endif
				infoWidth = maxp + 3;

				/* *INDENT-EQLS* Set up the rest of the structure. */
				ScreenOf(this)        = cdkscreen;
				parent                = cdkscreen->window;
				shadowWin             = 0;
				fieldAttr             = fieldAttrp;
//				fieldWidth            = fieldWidth;
				filler                = fillerp;
				hidden                = fillerp;
				ObjOf(this)->inputWindow   = fieldWin;
				ObjOf(this)->acceptsFocus  = TRUE;
				ReturnOf(this)             = NULL;
				shadow                = shadowp;
				screenCol             = 0;
				sbuch=0;
				leftChar              = 0;
				lbuch=0;
				min                   = minp;
				max                   = maxp;
//				boxWidth              = boxWidth;
//				boxHeight             = boxHeight;
				initExitType(this);
				dispType              = dispTypep;
				callbfn            = &SEntry::CDKEntryCallBack;

				/* Do we want a shadow? */
				if (shadowp) {
					shadowWin = newwin(
							boxHeight,
							boxWidth,
							ypos + 1,
							xpos + 1);
				}
				registerCDKObject(cdkscreen, vENTRY);
			}
		}
	}
//	return (entry);
} // SEntry::SEntry


/*
 * This means you want to use the given entry field. It takes input
 * from the keyboard, and when its done, it fills the entry info
 * element of the structure with what was typed.
 */
const char* SEntry::activateCDKEntry(chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/, int obpfeil/*=0*/)
{
	chtype input = 0;
	bool functionKey;
	const char *ret = 0;
	int zweit;
	if (!Zweitzeichen) Zweitzeichen=&zweit;
	/* Draw the widget. */
	drawCDKEntry(/*entry, ObjOf(entry)->*/obbox);
	if (!actions) {
		for (;;) {
			//static int y=2;
			*Zweitzeichen=0;
			input = (chtype)getchCDKObject(&functionKey);
			// GSchade Anfang
			if (input==27) {
				*Zweitzeichen =(chtype)getchCDKObject(&functionKey);
				if (*Zweitzeichen==194||*Zweitzeichen==195) {
					*Drittzeichen =(chtype)getchCDKObject(&functionKey);
				}
			} else if (input==9||(obpfeil && input==KEY_DOWN)) {
				*Zweitzeichen=-9;
			} else if (input==KEY_BTAB||(obpfeil && input==KEY_UP)) {
				*Zweitzeichen=-8;
			} else if (input==KEY_NPAGE) {
				*Zweitzeichen=-10;
			} else if (input==KEY_PPAGE) {
				*Zweitzeichen=-11;
			}
//		if (0) {
//				static bool afk{0}; static chtype ai{0}; static int aZz{0}; static EExitType	aex{vEARLY_EXIT};
				/*
				if (afk!=functionKey||ai!=input||aZz!=*Zweitzeichen||aex!=entry->exitType)
					mvwprintw(entry->parent,y++,30,"eingeb:%i %i %i %i",functionKey,input,*Zweitzeichen,entry->exitType);
				 */
//			afk=functionKey; ai=input; aZz=*Zweitzeichen; aex=entry->exitType;
//			}
			

			//mvwprintw(entry->parent,1,60,"info:%s -> ",entry->info);
			// GSchade Ende
			/* Inject the character into the widget. */
//			ret = injectCDKEntry(entry, input);
			ret=injectCDKEntry(input)?resultData.valueString:0/*unknownString*/;
			// GSchade Anfang
      /*
			mvwprintw(entry->parent,1,80,"info:%s ",entry->info);
			for(int i=0;i<strlen(entry->info);i++) {
				mvwprintw(entry->parent,2+i,60,"i: %i: %i",i,entry->info[i]);
			}
			wrefresh(entry->parent); // gleichbedeutend: wrefresh(entry->obj.screen->window);
      */
      drawCDKEntry(/*entry, ObjOf(entry)->*/obbox);
      // GSchade Ende

			if (this->exitType != vEARLY_EXIT||*Zweitzeichen==-8||*Zweitzeichen==-9||*Zweitzeichen==-10||*Zweitzeichen==-11) {
//					mvwprintw(entry->parent,3,2,"Zweitzeichen: %i         , Drittzeichen: %i     ",*Zweitzeichen,*Drittzeichen);
				return ret;
			}
//			mvwprintw(entry->parent,3,2,"kein Zweitzeichen");
		}
	} else {
		int length = chlen(actions);
		/* Inject each character one at a time. */
		for (int x = 0; x < length; x++) {
//					mvwprintw(entry->parent,4,2,"vor inject 2");
//			ret = injectCDKEntry(entry, actions[x]);
			ret = injectCDKEntry(actions[x])?resultData.valueString:0/*unknownString*/;
			if (this->exitType != vEARLY_EXIT) {
				return ret;
			}
		}
	}
	/* Make sure we return the correct info. */
	if (this->exitType == vNORMAL) {
#ifdef ineu
		return this->efld.c_str();
#else
		return this->efld;
#endif
	} else {
		return 0;
	}
} // char * SEntry::activateCDKEntry(chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/, int obpfeil/*=0*/)


/*
 * This activates the file selector.
 */
const char* SAlphalist::activateCDKAlphalist(chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/,int obpfeil/*=0*/)
{
   const char *ret = 0;
   /* Draw the widget. */
   drawCDKAlphalist(obbox);
   /* Activate the widget. */
   ret = entryField->activateCDKEntry(actions,Zweitzeichen,Drittzeichen,obpfeil);
   /* Copy the exit type from the entry field. */
   copyExitType(this, this->entryField);
   /* Determine the exit status. */
   if (this->exitType != vEARLY_EXIT) {
      return ret;
   }
   return 0;
}



/*
 * This draws the entry field.
 */
//void SEntry::drawCDKEntry(bool Box)
void SEntry::drawCDKEntry(bool Box)
{
//	SEntry *entry =(SEntry *)object;
	/* Did we ask for a shadow? */
	if (this->shadowWin) {
		drawShadow(this->shadowWin);
	}
	/* Box the widget if asked. */
	if (Box) {
		drawObjBox(this->win/*, ObjOf(this)*/);
	}
	drawCdkTitle(this->win/*, object*/);
	wrefresh(this->win);

	/* Draw in the label to the widget. */
	if (this->labelWin) {
		//int f1,f2;
		writeChtype(this->labelWin, 0, 0, this->labelp->getinh(), HORIZONTAL, 0, this->labelLen);
		wrefresh(this->labelWin);
	}
	this->zeichneFeld();
}

/*
 * This injects a single character into the widget.
 */
int SEntry::injectCDKEntry(chtype input)
{
//	SEntry *widget =(SEntry *)object;
	int ppReturn = 1;
	const char *ret = 0/*unknownString*/;
	bool complete = FALSE;
	static char umlaut[3]={0};
	const int inpint=input;
	static int zahl{0};
	mvwprintw(this->screen->window,2,2,"%i injectCDKEntry %c %i          ",zahl++,input,input);
	// screen->refreshCDKScreen(); // 21.12.18: Übeltäter, schreibt Listeneinträge an falsche Stellen
	if (inpint==194 || inpint==195) {
//		printf("Eintrag: %i\n",inpint);
		*umlaut=inpint;
		umlaut[1]=0;
	} else if ((unsigned char)*umlaut==194 ||(unsigned char)*umlaut==195) {
//		printf("Folgezeichen: %i\n",inpint);
		//printf("%c (%i)\n",inpint,inpint);
		umlaut[1]=inpint;
	} else {
//		printf("sonstiges Zeichen: %i\n",inpint);
		umlaut[1]=*umlaut=0;
	}
	/* Set the exit type. */
	setExitType(0);
	/* Refresh the widget field. */
	this->zeichneFeld();
	/* Check if there is a pre-process function to be called. */
	if (PreProcessFuncOf(this)) {
		ppReturn = PreProcessFuncOf(this)(vENTRY,
				this,
				PreProcessDataOf(this),
				input);
	}
	/* Should we continue? */
	if (ppReturn) {
		/* Check a predefined binding... */
		if (checkCDKObjectBind(input)) {
			checkEarlyExit(this);
			complete = TRUE;
		} else {
#ifdef ineu
//			const int infoLength = (int)strlen(this->efld.c_str());
			const size_t infoLength=efld.length();
#else
			const int infoLength =(int)strlen(this->efld);
#endif
			int currPos = this->screenCol + this->leftChar;
			switch (input) {
				case KEY_UP:
				case KEY_DOWN:
					Beep();
					break;
				case KEY_HOME:
					this->leftChar = 0;
					this->lbuch=0;
					this->screenCol = 0;
					this->sbuch=0;
					this->zeichneFeld();
					mvwprintw(this->parent,3,3,"Key_home");
					wrefresh(this->win);
					//refreshCDKScreen(allgscr);
					break;
				case CDK_TRANSPOSE:
					if (currPos >= infoLength - 1) {
						Beep();
					} else {
						const char holder = this->efld[currPos];
						this->efld[currPos] = this->efld[currPos + 1];
						this->efld[currPos + 1] = holder;
						this->zeichneFeld();
					}
					break;
				case KEY_END:
					this->settoend();
					this->zeichneFeld();
					break;
				case KEY_LEFT:
					if (currPos <= 0) {
						Beep();
					} else if (!this->screenCol) {
						/* Scroll left.  */
						if (currPos>1) if (this->efld[currPos-2]==-61 || this->efld[currPos-2]==-62) this->leftChar--;
						this->leftChar--;
						this->lbuch--;
						this->zeichneFeld();
					} else {
						/* Move left. */
						wmove(this->fieldWin, 0, --this->sbuch);
						this->screenCol--;
						if (currPos>1) if (this->efld[currPos-2]==-61 || this->efld[currPos-2]==-62) this->screenCol--;
					}
					break;
				case KEY_RIGHT:
					if (currPos >= infoLength || currPos>this->max) {
						Beep();
					} else if (this->sbuch == this->fieldWidth - 1) {
						/* Scroll to the right. */
						if (this->efld[this->leftChar]==-61 || this->efld[this->leftChar]==-62) {
							this->screenCol--;
							this->leftChar++;
						}
						this->leftChar++;
						this->lbuch++;
						if (this->efld[currPos]==-61 || this->efld[currPos]==-62) this->screenCol++;
						this->zeichneFeld();
					} else {
						/* Move right. */
						wmove(this->fieldWin, 0, ++this->sbuch);
						this->screenCol++;
						if (this->efld[currPos]==-61 || this->efld[currPos]==-62) this->screenCol++;
					}
					break;
				case KEY_BACKSPACE:
				case KEY_DC:
					if (this->dispType == vVIEWONLY) {
						Beep();
					} else {
						// mvwprintw(this->parent,1,100,"!!!!!!!!!, currPos: %i  ",currPos);
						bool success = FALSE;
						if (input == KEY_BACKSPACE) {
							--currPos;
							if (this->efld[currPos-1]==-61||this->efld[currPos-1]==-62) --currPos;
						}
						// .. und jetzt fuer den zu loeschenden
						const int obuml=(this->efld[currPos]==-61||this->efld[currPos]==-62);
						if (currPos >= 0 && infoLength > 0) {
							if (currPos < infoLength) {
						// mvwprintw(this->parent,2,100,"!!!!!!!!!, currPos: %i, obuml: %i",currPos,obuml);
								// wrefresh(this->parent);
#ifdef ineu
								efld.erase(currPos,1+obuml);
#else
								for (int x = currPos; x < infoLength; x++) {
									if (x+1+obuml>this->max-1) 
										this->efld[x]=0;
									else 
										this->efld[x]=this->efld[x+1+obuml];
								}
								if (obuml) if (infoLength>1) 
									this->efld[infoLength-2]=0;
#endif
								success = TRUE;
							} else if (input == KEY_BACKSPACE) {
#ifdef ineu
								efld.resize(infoLength-1);
#else
								this->efld[infoLength - 1] = '\0';
#endif
								success = TRUE;
                if (infoLength>1) if (obuml) 
#ifdef ineu
									efld.resize(infoLength-2);
#else
									this->efld[infoLength-2]=0;
#endif
              }
						}
						if (success) {
							if (input == KEY_BACKSPACE) {
								if (this->screenCol > 0 && !this->lbuch) {
									this->screenCol--;
                  if (obuml) this->screenCol--;
                  this->sbuch--;
                } else {
									this->leftChar--;
                  if (this->efld[this->leftChar-1]==-61||this->efld[this->leftChar-1]==-62) {
										this->leftChar--;
										this->screenCol++;
									}
                  this->lbuch--;
									if (obuml) this->screenCol--;
                }
							}
							this->zeichneFeld();
						} else {
							Beep();
						}
					}
					break;
				case KEY_ESC:
					setExitType(input);
					complete = TRUE;
					mvwprintw(this->parent,2,2,"Key_esc");
					break;
				case CDK_ERASE:
					if (infoLength) {
						cleanCDKEntry();
						this->zeichneFeld();
					}
					break;
				case CDK_CUT:
					if (infoLength) {
#ifdef pneu
						GPasteBuffer=efld;
#else
						freeChecked(GPasteBuffer);
						GPasteBuffer = copyChar(this->efld);
#endif
						cleanCDKEntry();
						this->zeichneFeld();
					} else {
						Beep();
					}
					break;
				case CDK_COPY:
					if (infoLength) {
#ifdef pneu
						GPasteBuffer=efld;
#else
						freeChecked(GPasteBuffer);
						GPasteBuffer = copyChar(this->efld);
#endif
					} else {
						Beep();
					}
					break;
				case CDK_PASTE:
#ifdef pneu
					if (!GPasteBuffer.empty()) {
						setCDKEntryValue(GPasteBuffer.c_str());
#else
					if (GPasteBuffer) {
						setCDKEntryValue(GPasteBuffer);
#endif
						this->zeichneFeld();
					} else {
						Beep();
					}
					break;
				case KEY_TAB:
				case KEY_ENTER:
					if (infoLength >= this->min)
					{
						setExitType(input);
#ifdef ineu
						ret = efld.c_str();
#else
						ret = this->efld;
#endif
						complete = TRUE;
					} else {
						Beep();
					}
					break;
				case KEY_ERROR:
					setExitType(input);
					complete = TRUE;
					break;
				case CDK_REFRESH:
					screen->eraseCDKScreen();
					screen->refreshCDKScreen();
					break;
				default:
					// printf("%i %i %i\n",umlaut[0],umlaut[1],umlaut[2]);
					if (umlaut[1]) {
//						printf("Sonderdruck Anfang");
//						setlocale(LC_ALL,"");
			//			wprintw(this->fieldWin,"%s",umlaut);
//			wprintw(this->fieldWin,"Achtung!");
						this->schreibl(umlaut[0]);
						this->schreibl(umlaut[1]);
						umlaut[1]=*umlaut=0;
//						printf("\n%i %i %i\n",umlaut[0],umlaut[1],umlaut[2]);
//						printf("Sonderdruck Ende");
					} else if (!*umlaut) {
						(this->*callbfn)(input);
					}
					break;
			}
		}
		/* Should we do a post-process? */
		if (!complete && (PostProcessFuncOf(this))) {
			PostProcessFuncOf(this)(vENTRY,
					this,
					PostProcessDataOf(this),
					input);
		}
	}
	if (!complete) setExitType(0);
	ResultOf(this).valueString = ret;
	return (ret != 0/*unknownString*/);
} // int SEntry::injectCDKEntry(chtype input)

/*
 * This removes the old information in the entry field and keeps the
 * new information given.
 */
void SEntry::setCDKEntryValue(const char *newValue)
{
	/* If the pointer sent in is the same pointer as before, do nothing. */
	if (!newValue || this->efld != newValue) {
		/* Just to be sure, if lets make sure the new value isn't null. */
		if (!newValue) {
			/* Then we want to just erase the old value. */
#ifdef ineu
			efld.clear();
//			efld.resize(infoWidth);
#else
			cleanChar(this->efld, this->infoWidth, '\0');
#endif

			/* Set the pointers back to zero. */
			this->leftChar = 0;
      this->lbuch=0;
			this->screenCol = 0;
      this->sbuch=0;
		} else {
			/* Determine how many characters we need to copy. */
			int copychars = MINIMUM((int)strlen(newValue), this->max);
			/* OK, erase the old value, and copy in the new value. */
#ifdef ineu
			efld=newValue;
//			if (max>efld.length()) efld.resize(max);
#else
			cleanChar(this->efld, this->max, '\0');
			strncpy(this->efld, newValue,(unsigned)copychars);
#endif
      this->settoend();
		}
	}
}

const char* SEntry::getCDKEntryValue()
{
#ifdef ineu
	return efld.c_str();
#else
	return efld;
#endif
}

/*
 * This sets specific attributes of the entry field.
 */
void SEntry::setCDKEntry(
		const char *value,
		int pmin,
		int pmax,
		bool Box GCC_UNUSED)
{
	setCDKEntryValue(value);
	min=pmin;
	max=pmax;
}


void SEntry::settoend()
{
  screenCol=sbuch=leftChar=lbuch=0;
#ifdef ineu
  for(int i=efld.length();i;) {
#else
  for(int i=strlen(efld);i;) {
#endif
    --i;
    if (sbuch<fieldWidth) {
      screenCol++;
      if ((unsigned char)efld[i]!=194 && (unsigned char)efld[i]!=195) sbuch++;
    } else {
      leftChar++;
      if ((unsigned char)efld[i]!=194 && (unsigned char)efld[i]!=195) lbuch++;
    }
  }
  if (sbuch>=fieldWidth && (sbuch+lbuch<max)) {
    leftChar++;
		lbuch++;
    screenCol--;
		sbuch--;
  }
}

/*
 * This erases the information in the entry field
 * and redraws a clean and empty entry field.
 */
void SEntry::cleanCDKEntry()
{
	/* Erase the information in the character pointer. */
#ifdef ineu
	efld.clear();
//	efld=string(infoWidth,'\0');
#else
	cleanChar(efld,infoWidth,'\0');
#endif
	/* Clean the entry screen field. */
	(void)mvwhline(fieldWin, 0, 0, this->filler, fieldWidth);
	/* Reset some variables. */
	this->screenCol = 0;
	this->leftChar = 0;
	/* Refresh the entry field. */
	wrefresh(fieldWin);
}  

/*
 * This erases an entry widget from the screen.
 */
void SEntry::eraseCDKEntry()
{
//	if (validCDKObject(object))
	{
		eraseCursesWindow(fieldWin);
		eraseCursesWindow(labelWin);
		eraseCursesWindow(win);
		eraseCursesWindow(shadowWin);
	}
}

/*
 * This erases the file selector from the screen.
 */
void SAlphalist::eraseCDKAlphalist/*_eraseCDKAlphalist*/()
{
	//   if (validCDKObject(object))
	{
		//      SAlphalist *alphalist =(SAlphalist *)object;
		scrollField->eraseCDKScroll();
		//      eraseCDKEntry(entryField);
		entryField->eraseCDKEntry();
		eraseCursesWindow(shadowWin);
		eraseCursesWindow(win);
	}
}

/*
 * This function erases the scrolling list from the screen.
 */
void SScroll::eraseCDKScroll/*_eraseCDKScroll*/(/*CDKOBJS *object*/)
{
//   if (validCDKObject(object))
   {
//      SScroll *scrollp =(SScroll *)object;
      eraseCursesWindow(win);
      eraseCursesWindow(shadowWin);
   }
}

/*
 * This calls refreshCDKScreen. (made consistent with widgets)
 */
void CDKOBJS::drawCDKScreen()
{
   screen->refreshCDKScreen();
}

/*
 * This refreshes all the objects in the screen.
 */
void SScreen::refreshCDKScreen(/*SScreen *cdkscreen*/)
{
//	int objectCount = /*screen->*/objectCount;
	int x;
	int focused = -1;
	int visible = -1;
#define richtig
#ifdef richtig
	refreshCDKWindow(/*screen->*/window);
#endif
	/* We erase all the invisible objects, then only
	 * draw it all back, so that the objects
	 * can overlap, and the visible ones will always
	 * be drawn after all the invisible ones are erased */
	for (x = 0; x < objectCount; x++) {
		CDKOBJS *obj = /*screen->*/object[x];
		if (obj) {
			//		if (validObjType(obj, ObjTypeOf(obj))) KLA
			if (obj->validObjType(obj->cdktype)) {
				if (obj->isVisible) {
					if (visible < 0)
						visible = x;
					if (obj->hasFocus && focused < 0)
						focused = x;
				} else {
					obj->eraseObj();
				}
			}
		}
	}
	for (x = 0; x < objectCount; x++) {
		CDKOBJS *obj = /*screen->*/object[x];
		if (obj) {
			//		if (validObjType (obj, ObjTypeOf (obj))) KLA
			if (obj->validObjType(obj->cdktype)) {
				obj->hasFocus =(x == focused);
				if (obj->isVisible) {
					// GSchade 13.11.18 hier gehts vorbei
					obj->drawObj(obj->obbox);
				}
			}
		}
	}
} // void SScreen::refreshCDKScreen()


/*
 * This sets the widgets box attribute.
 */
void CDKOBJS::setBox(bool Box)
{
	obbox = Box;
	borderSize = Box ? 1 : 0;
}

/*
 * This sets the background attribute of the widget.
 */
void SEntry::setBKattrEntry(chtype attrib)
{
	wbkgd(win, attrib);
	wbkgd(fieldWin, attrib);
	if (labelWin) {
		wbkgd(labelWin, attrib);
	}
}

/*
 * This sets the attribute of the entry field.
 */
void SEntry::setCDKEntryHighlight(chtype highlight, bool cursor)
{
	wbkgd(fieldWin, highlight);
	fieldAttr = highlight;
	curs_set(cursor);
	/*
	 *  FIXME -  if (cursor) { move the cursor to this widget }
	 */
}

void SEntry::focusCDKEntry()
{
	wmove(fieldWin, 0, sbuch);
	wrefresh(fieldWin);
}

void SEntry::unfocusCDKEntry()
{
	drawObj(obbox);
	wrefresh(fieldWin);
}

void CDKOBJS::refreshDataCDK()
{
}

void CDKOBJS::saveDataCDK()
{
}

#ifdef pneu
#else
int SAlphalist::createList(CDK_CSTRING *list, int listSize)
{
	int status = 0;
	if (listSize >= 0) {
		char **newlist = typeCallocN(char *, listSize + 1);
		if (newlist) {
			/*
			 * We'll sort the list before we use it.  It would have been better to
			 * declare list[] const and only modify the copy, but there may be
			 * clients that rely on the old behavior.
			 */
			sortList(list, listSize);
			/* Copy in the new information. */
			status = 1;
			for (int x = 0; x < listSize; x++) {
				if (!(newlist[x] = copyChar(list[x]))) {
					status = 0;
					break;
				}
			}
			if (status) {
				destroyInfo();
				this->listSize = listSize;
				this->slist = newlist;
			} else {
				CDKfreeStrings(newlist);
			}
		}
	} else {
		destroyInfo();
		status = TRUE;
	}
	return status;
}
#endif

/*
 * The alphalist's focus resides in the entry widget.  But the scroll widget
 * will not draw items highlighted unless it has focus.  Temporarily adjust the
 * focus of the scroll widget when drawing on it to get the right highlighting.
 */
#define SaveFocus(widget) \
   bool save = HasFocusObj(ObjOf(widget->scrollField)); \
   HasFocusObj(ObjOf(widget->scrollField)) = \
   HasFocusObj(ObjOf(widget->entryField))

#define RestoreFocus(widget) \
   HasFocusObj(ObjOf(widget->scrollField)) = save

void SAlphalist::injectMyScroller(chtype key)
{
	SaveFocus(this);
	scrollField->injectCDKScroll(key);
	RestoreFocus(this);
}

void SFSelect::injectMyScroller(chtype key)
{
	SaveFocus(this);
	scrollField->injectCDKScroll(key);
	RestoreFocus(this);
}

/*
 * This injects a single character into the alphalist.
 */
int SAlphalist::injectCDKAlphalist(chtype input)
{
//   SAlphalist *alphalist =(SAlphalist *)object;
   const char *ret;
   /* Draw the widget. */
   drawCDKAlphalist(obbox);
   /* Inject a character into the widget. */
	 ret=entryField->injectCDKEntry(input)?entryField->resultData.valueString:0/*unknownString*/;
	 /* Copy the exit type from the entry field. */
   copyExitType(this, this->entryField);
   /* Determine the exit status. */
   if (this->exitType == vEARLY_EXIT)
      ret = 0/*unknownString*/;
   ResultOf(this).valueString = ret;
   return (ret != 0/*unknownString*/);
}

/*
 * This injects a single character into the file selector.
 */
int SFSelect::injectCDKFselect(/*CDKOBJS *object, */chtype input)
{
	//   SFSelect *fselect =(SFSelect *)object;
	const char *filename{""};
	bool file;
#ifdef pneu
#else
	char *ret = 0/*unknownString*/;
#endif
	bool complete = FALSE;
	/* Let the user play. */
	if (entryField) {
#ifdef pneu
	 if (entryField->injectCDKEntry(/*this->entryField, */input)) 
		 filename=entryField->resultData.valueString;
#else
	 filename = entryField->injectCDKEntry(/*this->entryField, */input)?entryField->resultData.valueString:0/*unknownString*/;
#endif
  }
	/* Copy the entry field exitType to the fileselector. */
	copyExitType(this, this->entryField);
	/* If we exited early, make sure we don't interpret it as a file. */
	if (this->exitType == vEARLY_EXIT) {
		return 0;
	}
	/* Can we change into the directory? */
	file = chdir(filename);
	if (chdir(this->pwd
#ifdef pneu
				.c_str()
#else
#endif
				)) {
		return 0;
	}
	/* If it's not a directory, return the filename. */
	if (file) {
		/* It's a regular file, create the full path. */
#ifdef pneu
		this->pfadname=filename;
		/* Return the complete pathname. */
#else
		this->pathname = copyChar(filename);
		/* Return the complete pathname. */
		ret = (this->pathname);
#endif
		complete = TRUE;
	} else {
		/* Set the file selector information. */
		setCDKFselect(/*this, */filename,
				this->fieldAttribute, this->fillerCharacter,
				this->highlight,
				this->dirAttribute
#ifdef pneu
				.c_str()
#else
#endif
				, this->fileAttribute
#ifdef pneu
				.c_str()
#else
#endif
				,
				this->linkAttribute
#ifdef pneu
				.c_str()
#else
#endif
				, this->sockAttribute
#ifdef pneu
				.c_str()
#else
#endif
				,
				/*ObjOf(this)->*/obbox);

		/* Redraw the scrolling list. */
		drawMyScroller(/*this*/);
	}
	if (!complete)
		setExitType(/*this, */0);
#ifdef pneu
  resultData.valueString=pfadname.c_str();
#else
	ResultOf(this).valueString = ret;
#endif
#ifdef pneu
	return !pfadname.empty();
#else
	return (ret != 0/*unknownString*/);
#endif
}


/*
 * This sets multiple attributes of the widget.
 */
void SAlphalist::setCDKAlphalist(
#ifdef pneu
		      vector<string> *plistp,
#else
		      CDK_CSTRING *list,
		      int listSize,
#endif
		      chtype fillerChar,
		      chtype highlight,
		      bool Box)
{
   setCDKAlphalistContents(
#ifdef pneu
			 plistp
#else
			 list, listSize
#endif
			 );
   setCDKAlphalistFillerChar(fillerChar);
   setCDKAlphalistHighlight(highlight);
   setBox/*setCDKAlphalistBox*/(Box);
}

/*
 * This sets certain attributes of the scrolling list.
 */
void SScroll::setCDKScroll(
#ifdef pneu
		vector<string> *plistp,
#else
		 CDK_CSTRING2 list,
		 int listSize,
#endif
		   bool numbers,
		   chtype hl,
		   bool Box)
{
   setCDKScrollItems(
#ifdef pneu
			 								plistp,
#else
			 								list, listSize, 
#endif
																			numbers);
	 highlight=hl;
	 obbox=Box;
}
#ifdef pneu
#else
// wird bisher nicht gebraucht
int SScroll::getCDKScrollItems(/*SScroll *scrollp, */char **list)
{
	if (list) {
		for (int x = 0; x < /*scrollp->*/listSize; x++) {
			list[x] = chtype2Char(/*scrollp->*/sitem[x]);
		}
	}
	return /*scrollp->*/listSize;
}
#endif

/*
 * This sets the box attribute of the scrolling list.
 */
/*
	 // statt dessen: setBox
void SScroll::setCDKScrollBox(//SScroll *scrollp, 
							bool Box)
{
   //ObjOf(scrollp)->
	obbox = Box;
   //ObjOf(scrollp)->
	borderSize = Box ? 1 : 0;
}
bool SScroll::getCDKScrollBox()
{
	return //ObjOf(scrollp)->
		obbox;
}
*/

/*
 * This sets the scrolling list items.
 */
void SScroll::setCDKScrollItems(
#ifdef pneu
			 													vector<string> *plistp,
#else
																CDK_CSTRING2 list, int listSize, 
#endif
																																bool numbers)
{
   if (createCDKScrollItemList(numbers, 
#ifdef pneu
				 plistp
#else
				 list, listSize
#endif
				 ) <= 0)
      return;
   /* Clean up the display. */
   for (int x = 0; x < this->viewSize; x++) {
      writeBlanks(this->win, 1, SCREEN_YPOS(this, x),
		   HORIZONTAL, 0, this->boxWidth - 2);
   }
   setViewSize(listSize);
   setCDKScrollPosition(0);
   this->leftChar = 0;
}

void SScroll::setCDKScrollCurrentTop(/*SScroll *widget, */int item)
{
   if (item < 0)
      item = 0;
   else if (item > /*widget->*/maxTopItem)
      item = /*widget->*/maxTopItem;
   /*widget->*/currentTop = item;
   SetPosition(/*(CDKSCROLLER *)widget,*/ item);
}


/*
 * This allows the user to accelerate to a position in the scrolling list.
 */
void SScroll::setCDKScrollPosition(int item)
{
 SetPosition(item);
}

/*
 * This function sets the information inside the file selector.
 */
void SAlphalist::setCDKAlphalistContents(
#ifdef pneu
		      vector<string> *plistp
#else
		CDK_CSTRING *list, int listSize
#endif
		)
{
#ifdef pneu
	 plist=*plistp;
#else
   if (!createList(list, listSize))
      return;
#endif
   /* Set the information in the scrolling list. */
   scrollField->setCDKScroll(
#ifdef pneu
		 plistp,
#else
		 (CDK_CSTRING2)this->slist,
		 this->listSize,
#endif
		 NONUMBERS,
		 scrollField->highlight,
		 ObjOf(scrollField)->obbox);
   /* Clean out the entry field. */
   setCDKAlphalistCurrentItem(0);
   entryField->cleanCDKEntry();
   /* Redraw the this. */
   this->eraseCDKAlphalist();
   this->drawCDKAlphalist(obbox);
}

/*
 * This returns the contents of the widget.
 */
#ifdef pneu
vector<string> *
#else
char ** 
#endif
SAlphalist::getCDKAlphalistContents(
#ifdef pneu
#else
		int *size
#endif
		)
{
#ifdef pneu
	return &plist;
#else
   (*size) = listSize;
   return slist;
#endif
}

/*
 * Get/set the current position in the scroll-widget.
 */
int SAlphalist::getCDKAlphalistCurrentItem()
{
   return scrollField->currentItem;
}

void SAlphalist::setCDKAlphalistCurrentItem(int item)
{
#ifdef pneu
	 if (plist.size()) {
#else
   if (this->listSize) {
#endif
      scrollField->setCDKScrollCurrent(item);
#ifdef pneu
			entryField->setCDKEntryValue(next(plist.begin(),item)->c_str());
#else
      entryField->setCDKEntryValue(this->slist[scrollField->currentItem]);
#endif
   }
}

void SScroll::setCDKScrollCurrent(int item)
{
	SetPosition(item);
}

/*
 * This sets the filler character of the entry field of the alphalist.
 */
void SAlphalist::setCDKAlphalistFillerChar(chtype fillerCharacter)
{
	fillerChar = fillerCharacter;
	entryField->filler=fillerCharacter;
}

chtype SAlphalist::getCDKAlphalistFillerChar()
{
   return fillerChar;
}

/*
 * This sets the highlight bar attributes.
 */
void SAlphalist::setCDKAlphalistHighlight(chtype hl)
{
   highlight = hl;
}

chtype SAlphalist::getCDKAlphalistHighlight()
{
   return highlight;
}

/*
 * This sets whether or not the widget will be drawn with a box.
 */
/*
void SAlphalist::setCDKAlphalistBox(bool Box)
{
   obbox = Box;
   borderSize = Box ? 1 : 0;
}
*/

bool SAlphalist::getCDKAlphalistBox()
{
   return obbox;
}

/*
 * These functions set the drawing characters of the widget.
 */
void SAlphalist::setMyULchar(chtype character)
{
	 entryField->ULChar=character;
}
void SAlphalist::setMyURchar(chtype character)
{
	 entryField->URChar=character;
}
void SAlphalist::setMyLLchar(chtype character)
{
	 scrollField->LLChar=character;
}
void SAlphalist::setMyLRchar(chtype character)
{
	 scrollField->LRChar=character;
}
void SAlphalist::setMyVTchar(chtype character)
{
   entryField->VTChar=character;// setCDKEntryVerticalChar(character);
   scrollField->VTChar=character;//setCDKScrollVerticalChar(character);
}
void SAlphalist::setMyHZchar(chtype character)
{
   entryField->HZChar=character;//setCDKEntryHorizontalChar(character);
   scrollField->HZChar=character;//setCDKScrollHorizontalChar(character);
}
void SAlphalist::setMyBXattr(chtype character)
{
   entryField->BXAttr=character; //setCDKEntryBoxAttribute(character);
   scrollField->BXAttr=character; // setCDKScrollBoxAttribute(character);
}
/*
 * This sets the background attribute of the widget.
 */
void SAlphalist::setMyBKattr(chtype character)
{
	 entryField->setBKattrEntry(character);//setCDKEntryBoxAttribute(character);
	 scrollField->setBKattrScroll(character);// setCDKScrollBoxAttribute(character);
}

/*
 * This sets the background attribute of the widget.
 */
void SScroll::setBKattrScroll(chtype attrib)
{
	//      SScroll *widget =(SScroll *)object;
	wbkgd(this->win, attrib);
	wbkgd(this->listWin, attrib);
	if (this->scrollbarWin) {
		wbkgd(this->scrollbarWin, attrib);
	}
}

/*
 * Start of callback functions.
 */
static int adjustAlphalistCB(EObjectType objectType GCC_UNUSED, void
		*object GCC_UNUSED,
		void *clientData,
		chtype key)
{
	/* *INDENT-EQLS* */
   SAlphalist *alphalist = (SAlphalist *)clientData;
   SScroll *scrollp      = alphalist->scrollField;
   SEntry *entry         = alphalist->entryField;
   if (scrollp->listSize > 0) {
      char *current;
      /* Adjust the scrolling list. */
      alphalist->injectMyScroller(key);
      /* Set the value in the entry field. */
#ifdef pneu
			current=scrollp->pitem[scrollp->currentItem].chtype2Char();
#else
      current = chtype2Char(scrollp->sitem[scrollp->currentItem]);
#endif
      entry->setCDKEntryValue(current);
      entry->drawObj(alphalist->obbox);
#ifdef pneu
#else
      freeChecked(current);
#endif
      return TRUE;
   }
   Beep();
   return FALSE;
}

/*
 * This tries to complete the word in the entry field.
 */
static int completeWordCB(EObjectType objectType GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   SAlphalist *alphalist = (SAlphalist *)clientData;
   SEntry *entry         = (SEntry *)alphalist->entryField;
   SScroll *scrollp      = 0;
   int wordLength          = 0;
   int Index               = 0;
   int ret                 = 0;
#ifdef pneu
	 vector<string> altWords;
#else
   char **altWords         = 0;
#endif
#ifdef ineu
	 if (0) {
#else
   if (!entry->efld) {
#endif
      Beep();
      return TRUE;
   }
#ifdef ineu
//   wordLength = (int)strlen(entry->efld.c_str());
	 wordLength=entry->efld.length();
#else
   wordLength = (int)strlen(entry->efld);
#endif

   /* If the word length is equal to zero, just leave. */
   if (!wordLength) {
      Beep();
      return TRUE;
   }

   /* Look for a unique word match. */
   Index = searchList(
#ifdef pneu
		&alphalist->plist,
#else
			 (CDK_CSTRING2)alphalist->slist, alphalist->listSize, 
#endif
#ifdef ineu
			 entry->efld.c_str()
#else
			 entry->efld
#endif
			 );

   /* If the index is less than zero, return we didn't find a match. */
   if (Index < 0) {
      Beep();
      return TRUE;
   }

   /* Did we find the last word in the list? */
   if (Index == alphalist->
#ifdef pneu
			 plist.size()
#else
			 listSize 
#endif
			 - 1) {
      entry->setCDKEntryValue(alphalist->
#ifdef pneu
					plist[Index].c_str()
#else
					slist[Index]
#endif
											);
      entry->drawObj(entry->obbox);
      return TRUE;
   }

   /* Ok, we found a match, is the next item similar? */
#ifdef pneu
   ret = strncmp(alphalist->plist[Index + 1].c_str(), 
#else
   ret = strncmp(alphalist->slist[Index + 1], 
#endif
#ifdef ineu
			 entry->efld.c_str(),(size_t) wordLength);
#else
			 entry->efld,(size_t) wordLength);
#endif
	 if (!ret) {
		 int currentIndex = Index;
		 int altCount = 0;
#ifdef pneu
#else
		 unsigned used = 0;
#endif
		 int selected;
		 int height;
		 int match;
		 int x;

#ifdef pneu
		 while ((currentIndex<alphalist->plist.size()) && (!strncmp(alphalist->plist[currentIndex].c_str(),
#else
		 /* Start looking for alternate words. */
		 /* FIXME: bsearch would be more suitable */
		 while ((currentIndex < alphalist->listSize)
				 && (!strncmp(alphalist->slist[currentIndex],
#endif
#ifdef ineu
						 entry->efld.c_str(),(size_t)wordLength))) {
#else
						 entry->efld, (size_t)wordLength))) {
#endif
#ifdef pneu
			 altWords.push_back(alphalist->plist[currentIndex++]);
#else
			 used = CDKallocStrings(&altWords,
					 alphalist->slist[currentIndex++],
					 (unsigned)altCount++,
					 used);
#endif
		 }

		 /* Determine the height of the scrolling list. */
		 height = (altCount < 8 ? altCount + 3 : 11);

		 /* Create a scrolling list of close matches. */
		 scrollp = new SScroll(entry->/*obj.*/screen,
				 CENTER, CENTER, RIGHT, height, -30,
				 "<C></B/5>Possible Matches.",
#ifdef pneu
				 &altWords,
#else
				 (CDK_CSTRING2)altWords, altCount,
#endif
				 NUMBERS, A_REVERSE, TRUE, FALSE);

		 /* Allow them to select a close match. */
		 match = scrollp->activateCDKScroll(0);
		 selected = scrollp->currentItem;

		 /* Check how they exited the list. */
		 if (scrollp->exitType == vESCAPE_HIT) {
			 /* Destroy the scrolling list. */
			// scrollp->destroyCDKScroll();
			 scrollp->destroyObj();


			 /* Clean up. */
#ifdef pneu
#else
			 CDKfreeStrings(altWords);
#endif

			 /* Beep at the user. */
			 Beep();

			 /* Redraw the alphalist and return. */
			 alphalist->drawCDKAlphalist(alphalist->obbox);
			 return (TRUE);
		 }

		 /* Destroy the scrolling list. */
//		 destroyCDKScroll(scrollp);
		 scrollp->destroyObj();

		 /* Set the entry field to the selected value. */
		 entry->setCDKEntry(
#ifdef pneu
				 altWords[match].c_str(),
#else
				 altWords[match],
#endif
				 entry->min,
				 entry->max,
				 alphalist->obbox);

		 /* Move the highlight bar down to the selected value. */
		 for (x = 0; x < selected; x++) {
			 alphalist->injectMyScroller(KEY_DOWN);
		 }

		 /* Clean up. */
#ifdef pneu
#else
		 CDKfreeStrings(altWords);
#endif

		 /* Redraw the alphalist. */
		 alphalist->drawCDKAlphalist(alphalist->obbox);
	 } else {
		 /* Set the entry field with the found item. */
		 entry->setCDKEntry(
#ifdef pneu
				 alphalist->plist[Index].c_str(),
#else
				 alphalist->slist[Index],
#endif
				 entry->min,
				 entry->max,
				 ObjOf(entry)->obbox);
		 entry->drawObj(alphalist->obbox);
	 }
	 return (TRUE);
}

void SAlphalist::focusCDKAlphalist()
{
//   SAlphalist *widget = (SAlphalist *)object;
	/*
	FocusObj(entryField);
	MethodPtr(entryField,focusObj)(entryField);
	((ObjPtr(entryField))->focusObj)(entryField);
	(((CDKOBJS*)(entryField))->focusObj)(entryField);
	entryField->focusObj(entryField);
	*/
	entryField->focusCDKEntry();
}

void SAlphalist::unfocusCDKAlphalist()
{
//   SAlphalist *widget = (SAlphalist *)object;
//   UnfocusObj(ObjOf(entryField));
	entryField->unfocusCDKEntry();
}

void SFSelect::focusCDKFileSelector()
{
	entryField->focusCDKEntry();
}

void SFSelect::unfocusCDKFileSelector()
{
	entryField->unfocusCDKEntry();
}

/*
 * Set data for preprocessing.
 */
void CDKOBJS::setCDKObjectPreProcess(/*CDKOBJS *obj, */PROCESSFN fn, void *data)
{
   preProcessFunction = fn;
   preProcessData = data;
}

/*
 * Set data for postprocessing.
 */
void CDKOBJS::setCDKObjectPostProcess(/*CDKOBJS *obj, */PROCESSFN fn, void *data)
{
   postProcessFunction = fn;
   postProcessData = data;
}

/*
 * This is the heart-beat of the widget.
 */
static int preProcessEntryField(EObjectType cdktype GCC_UNUSED, void
				 *object GCC_UNUSED,
				 void *clientData,
				 chtype input)
{
	/* *INDENT-EQLS* */
	SAlphalist *alphalist = (SAlphalist *)clientData;
	SScroll *scrollp      = alphalist->scrollField;
	SEntry *entry         = alphalist->entryField;
#ifdef ineu
//	int infoLen             = strlen(entry->efld.c_str());
	int infoLen=entry->efld.length();
#else
	int infoLen             = (entry->efld ?(int)strlen(entry->efld) : 0);
#endif
	int result              = 1;
	bool empty              = FALSE;

	/* Make sure the entry field isn't empty. */
#ifdef ineu
	if (0) {
#else
	if (!entry->efld) {
#endif
		empty = TRUE;
	} else if (alphalist->isCDKObjectBind(input)) {
		result = 1;		/* don't try to use this key in editing */
	} else if ((isChar(input) &&
				(isalnum((unsigned char)input) ||
				 ispunct(input))) ||
			input == KEY_BACKSPACE ||
			input == KEY_DC) {
		int Index, difference, absoluteDifference, x;
		int currPos = (entry->screenCol + entry->leftChar);
#ifdef qneu
		string pattern;
#else
		char *pattern = (char *)malloc((size_t) infoLen + 2);
#endif

#ifdef ineu
#ifdef qneu
			pattern=entry->efld;
			{
#else
			strcpy(pattern,&entry->efld[0]);
			if (pattern) {
#endif
#else
#ifdef qneu
			pattern=entry->efld;
			{
#else
			strcpy(pattern, entry->efld);
			if (pattern) {
#endif
#endif

			if (input == KEY_BACKSPACE || input == KEY_DC) {
				if (input == KEY_BACKSPACE)
					--currPos;
				if (currPos >= 0)
#ifdef qneu
					pattern.erase(currPos,1);
#else
#ifdef ineu
					strcpy(pattern + currPos, entry->efld.substr(currPos+1).c_str());
#else
					strcpy(pattern + currPos, entry->efld + currPos + 1);
#endif
#endif
			} else {
#ifdef qneu
				/*
				if (currPos==pattern.length()-1) {
					pattern.append(1,(char)input);
				} else {
				*/
	//mvwprintw(entry->screen->window,3,2,"preProcessEntryField,input: %c,crrPos: %i, pattern.length(): %i,screenCol: %i, leftChar: %i          ",input,currPos,pattern.length(),entry->screenCol, entry->leftChar);
					pattern.insert(pattern.begin()+currPos,(char)input);
					/*
				}
				*/
#else
				pattern[currPos] =(char)input;
#ifdef ineu
				strcpy(pattern + currPos+1, entry->efld.substr(currPos).c_str());
#else
				strcpy(pattern + currPos+1, entry->efld+currPos);
#endif
#endif
	// wrefresh(entry->screen->window);
			}
		}
#ifdef qneu
			if (!strlen(pattern.c_str())) empty=TRUE;
#else
		if (!pattern) {
			Beep();
		} else if (!strlen(pattern)) {
			empty = TRUE;
		}
#endif
		else if ((Index = searchList(
#ifdef pneu
		&alphalist->plist,
#else
						(CDK_CSTRING2)alphalist->slist,
						alphalist->listSize,
#endif
#ifdef qneu
						pattern.c_str()
#else
						pattern
#endif
							))>=0) {
			/* *INDENT-EQLS* */
			difference           = Index - scrollp->currentItem;
			absoluteDifference   = abs(difference);

			/*
			 * If the difference is less than zero, then move up.
			 * Otherwise move down.
			 *
			 * If the difference is greater than 10 jump to the new
			 * index position.  Otherwise provide the nice scroll.
			 */
			if (absoluteDifference <= 10) {
				for (x = 0; x < absoluteDifference; x++) {
					alphalist->injectMyScroller(
							(chtype)((difference <= 0)
								? KEY_UP
								: KEY_DOWN));
				}
			} else {
				scrollp->SetPosition(Index);
			}
			alphalist->drawMyScroller();
		} else {
			/* Kommentar G.Schade 17.11.18, erlaubt nicht in der Liste vertretene Eingaben
			Beep();
			result = 0;
			*/
		}
#ifdef pneu
#else
		if (pattern)
			free(pattern);
#endif
	}
	if (empty) {
		scrollp->SetPosition(0);
		alphalist->drawMyScroller();
	}
	return result;
} // static int preProcessEntryField(


/*
 * This creates the alphalist widget.
 */
SAlphalist::SAlphalist(SScreen *cdkscreen,
			       int xplace,
			       int yplace,
			       int height,
			       int width,
			       const char *title,
			       const char *label,
#ifdef pneu
						 vector<string> *plistp,
#else
			       CDK_CSTRING *slist,
			       int listSize,
#endif
			       chtype fillerChar,
			       chtype phighlight,
			       bool Box,
						 bool shadow,
						 // GSchade Anfang
						 int highnr/*=0*/
						 // GSchade Ende
		):xpos(xplace),ypos(yplace),highlight(phighlight),fillerChar(fillerChar),shadow(shadow)
#ifdef pneu
			,plist(*plistp)
#endif
{
	cdktype = vALPHALIST;
	/* *INDENT-EQLS* */
//	SAlphalist *alphalist      = 0;
	int parentWidth              = getmaxx(cdkscreen->window);
	int parentHeight             = getmaxy(cdkscreen->window);
	int tempWidth                = 0;
	int tempHeight               = 0;
	int labelLen                 = 0;
	int x, junk2;
	/* *INDENT-OFF* */
	static const struct { int from; int to; } bindings[] = {
		{ CDK_BACKCHAR,	KEY_PPAGE },
		{ CDK_FORCHAR,	KEY_NPAGE },
	};
	/* *INDENT-ON* */

	::CDKOBJS();
#ifdef pneu
#else
	if (/*(alphalist = newCDKObject(SAlphalist, &my_funcs)) == 0 || */ !createList(slist, listSize)) {
		destroyCDKObject();
		return;
	}
#endif
	setBox(Box);
	/*
	 * If the height is a negative value, the height will
	 * be ROWS-height, otherwise, the height will be the
	 * given height.
	 */
	boxHeight = setWidgetDimension(parentHeight, height, 0);
	/*
	 * If the width is a negative value, the width will
	 * be COLS-width, otherwise, the width will be the
	 * given width.
	 */
	boxWidth = setWidgetDimension(parentWidth, width, 0);
	/* Translate the label char *pointer to a chtype pointer. */
	if (label) {
//		chtype *chtypeLabel = char2Chtypeh(label, &labelLen, &junk2 /* GSchade Anfang */ ,highnr /* GSchade Ende */);
//		freeChtype(chtypeLabel);
		chtstr chtypeLabel(label,&labelLen,&junk2,highnr);
	}
	/* Rejustify the x and y positions if we need to. */
	alignxy(cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);
	/* Make the file selector window. */
	this->win = newwin(boxHeight, boxWidth, ypos, xpos);
	if (!this->win) {
		destroyCDKObject();
		return;
	}
	keypad(this->win, TRUE);
	/* *INDENT-EQLS* Set some variables. */
//	ScreenOf(this)         = cdkscreen;
	screen									= cdkscreen;
	this->parent            = cdkscreen->window;
//	this->highlight         = highlight;
//	this->fillerChar        = fillerChar;
//	this->boxHeight         = boxHeight;
//	this->boxWidth          = boxWidth;
	initExitType(this);
//	this->shadow            = shadow;
	this->shadowWin         = 0;
	/* Do we want a shadow? */
	if (shadow) {
		this->shadowWin = newwin(boxHeight, boxWidth, ypos + 1, xpos + 1);
	}

	/* Create the entry field. */
	tempWidth = (isFullWidth(width)
			? FULL
			: boxWidth - 2 - labelLen);
	this->entryField = new SEntry(cdkscreen,
			getbegx(this->win),
			getbegy(this->win),
			title, label,
			A_NORMAL, fillerChar,
			vMIXED, tempWidth, 0, 512,
			Box, FALSE
			// GSchade Anfang
			,highnr
			// GSchade Ende
			);
	if (!this->entryField) {
//		destroyCDKObject(this);
		return;
	}
	//setCDKEntryLLChar(this->entryField, ACS_LTEE);
   LLChar=ACS_LTEE;		/* lines: lower-left */
	//setCDKEntryLRChar(this->entryField, ACS_RTEE);
   LRChar=ACS_RTEE;		/* lines: lower-right */

	/* Set the key bindings for the entry field. */
	entryField->bindCDKObject(
			KEY_UP,
			adjustAlphalistCB,
			this);
	entryField->bindCDKObject(
			KEY_DOWN,
			adjustAlphalistCB,
			this);
	entryField->bindCDKObject(
			KEY_NPAGE,
			adjustAlphalistCB,
			this);
	entryField->bindCDKObject(
			KEY_PPAGE,
			adjustAlphalistCB,
			this);
	entryField->bindCDKObject(
			KEY_TAB,
			completeWordCB,
			this);

	/* Set up the post-process function for the entry field. */
	entryField->setCDKObjectPreProcess(preProcessEntryField, this);

	/*
	 * Create the scrolling list.  It overlaps the entry field by one line if
	 * we are using box-borders.
	 */
	tempHeight = getmaxy(this->entryField->win) - borderSize;
	tempWidth =(isFullWidth(width) ? FULL : boxWidth - 1);
	this->scrollField = new SScroll(cdkscreen,
			getbegx(this->win),
			getbegy(this->entryField->win) + tempHeight,
			RIGHT,
			boxHeight - tempHeight,
			tempWidth, 0, 
#ifdef pneu
			plistp,
#else
			(CDK_CSTRING2)slist, listSize,
#endif
			NONUMBERS, A_REVERSE,
			Box, FALSE);
//	setCDKScrollULChar(this->scrollField, ACS_LTEE);
   ULChar=ACS_LTEE;
//	setCDKScrollURChar(this->scrollField, ACS_RTEE);
   URChar=ACS_LTEE;	

	/* Setup the key bindings. */
	for (x = 0; x <(int)SIZEOF(bindings); ++x)
		bindCDKObject(
				(chtype)bindings[x].from,
				getcCDKBind,
				(void *)(long)bindings[x].to);
	registerCDKObject(cdkscreen, vALPHALIST);
//	return (this);
}

void SAlphalist::drawMyScroller(/*SAlphalist *widget*/)
{
   SaveFocus(this);
	// mit 1 entstehen hier Fehler nicht unten, nur oben
   scrollField->drawCDKScroll(obbox,1);
   RestoreFocus(this);
}

/*
 * This draws the file selector widget.
 */
void SAlphalist::drawCDKAlphalist(bool Box GCC_UNUSED)
{
//   SAlphalist *alphalist =(SAlphalist *)obj;
   /* Does this widget have a shadow? */
   if (shadowWin) {
      drawShadow(shadowWin);
   }
   /* Draw in the entry field. */
   entryField->drawObj(entryField->obbox);
   /* Draw in the scroll field. */
	 // Kommentar GSchade 11.11.18: bewirkt, dass der Scroller erst gezeichnet wird, wenn in ihm ein Tastendruck erfolgt, z.B. Pfeil nach unten
   this->drawMyScroller();
}


void SScroll_basis::scroll_KEY_UP()
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

void SScroll_basis::scroll_KEY_DOWN()
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

void SScroll_basis::scroll_KEY_LEFT()
{
   if (listSize <= 0 || leftChar <= 0) {
      Beep();
      return;
   }
   leftChar--;
}

void SScroll_basis::scroll_KEY_RIGHT()
{
   if (listSize <= 0 || leftChar >= maxLeftChar) {
      Beep();
      return;
   }
   leftChar++;
}

void SScroll_basis::scroll_KEY_PPAGE()
{
   int vS = viewSize - 1;
   if (listSize <= 0 || currentTop <= 0) {
      Beep();
      return;
   }
   if (currentTop < vS) {
      scroll_KEY_HOME();
   } else {
      currentTop -= vS;
      currentItem -= vS;
   }
}

void SScroll_basis::scroll_KEY_NPAGE()
{
   int vS = viewSize - 1;
   if (listSize <= 0 || currentTop >= maxTopItem) {
      Beep();
      return;
   }
   if ((currentTop + vS) <= maxTopItem) {
      currentTop += vS;
      currentItem += vS;
   } else {
      scroll_KEY_END();
   }
}

void SScroll_basis::scroll_KEY_HOME()
{
   currentTop = 0;
   currentItem = 0;
   currentHigh = 0;
}

void SScroll_basis::scroll_KEY_END()
{
   currentTop = maxTopItem;
   currentItem = lastItem;
   currentHigh = viewSize - 1;
}

void SScroll_basis::scroll_FixCursorPosition()
{
   int scrollbarAdj =(scrollbarPlacement == LEFT) ? 1 : 0;
   int ypos = SCREEN_YPOS(this,currentItem - currentTop);
   int xpos = SCREEN_XPOS(this,0) + scrollbarAdj;
   wmove(InputWindowOf(this), ypos, xpos);
   wrefresh(InputWindowOf(this));
}

void SScroll_basis::SetPosition(int item)
{
   /* item out of band */
   if (item <= 0) {
      scroll_KEY_HOME();
      return;
   }
   /* item out of band */
   if (item >= lastItem) {
      scroll_KEY_END();
      return;
   }
   /* item in first view port */
   if (item < viewSize) {
      currentTop = 0;
   } /* item in last view port */ else if (item >= lastItem - viewSize) {
      currentTop = maxTopItem;
   } /* item not in visible view port */ else if (item < currentTop || item >= currentTop + viewSize) {
      currentTop = item;
   }
   currentItem = item;
   currentHigh = currentItem - currentTop;
}

int SScroll_basis::MaxViewSize()
{
   return(boxHeight -(2 * borderSize + titleLines));
}

void SScroll_basis::setViewSize(int size)
{
   int max_view_size = MaxViewSize();
   viewSize = max_view_size;
#ifdef pneu
#else
	 listSize = size;
#endif
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

/*
 * This function creates a new scrolling list widget.
 */
SScroll::SScroll(SScreen *cdkscreen,
			 int xplace,
			 int yplace,
			 int splace,
			 int height,
			 int width,
			 const char *title,
#ifdef pneu
			 vector<string> *plistp,
#else
			 CDK_CSTRING2 list,
			 int listSize,
#endif
			 bool numbers,
			 chtype phighlight,
			 bool Box,
			 bool pshadow)
{
#ifdef pneu
	size_t listSize{plistp->size()};
#else
#endif
	cdktype=vSCROLL;
   /* *INDENT-EQLS* */
   //SScroll *scrollp           = 0;
   int parentWidth              = getmaxx(cdkscreen->window);
   int parentHeight             = getmaxy(cdkscreen->window);
   int xpos                     = xplace;
   int ypos                     = yplace;
   int scrollAdjust             = 0;
   int x;
   /* *INDENT-OFF* */
   static const struct { int from; int to; } bindings[] = {
		{ CDK_BACKCHAR,	KEY_PPAGE },
		{ CDK_FORCHAR,	KEY_NPAGE },
		{ 'g',		KEY_HOME },
		{ '1',		KEY_HOME },
		{ 'G',		KEY_END },
		{ '<',		KEY_HOME },
		{ '>',		KEY_END },
   };
   /* *INDENT-ON* */

//   if ((scrollp = newCDKObject(SScroll, &my_funcs)) == 0) { destroyCDKObject(scrollp); return(0); }
	::CDKOBJS();
   setBox(Box);

   /*
    * If the height is a negative value, the height will
    * be ROWS-height, otherwise, the height will be the
    * given height.
    */
   boxHeight = setWidgetDimension(parentHeight, height, 0);

   /*
    * If the width is a negative value, the width will
    * be COLS-width, otherwise, the width will be the
    * given width.
    */
   boxWidth = setWidgetDimension(parentWidth, width, 0);

   boxWidth = setCdkTitle(title, boxWidth);

   /* Set the box height. */
   if (titleLines > boxHeight) {
      boxHeight =(titleLines + MINIMUM(listSize , 8) + 2 * borderSize);
   }

   /* Adjust the box width if there is a scrollp bar. */
   if ((splace == LEFT) ||(splace == RIGHT)) {
      scrollbar = TRUE;
      boxWidth += 1;
   } else {
      scrollbar = FALSE;
   }

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   boxWidth =(boxWidth > parentWidth
			?(parentWidth - scrollAdjust)
			: boxWidth);
   boxHeight =(boxHeight > parentHeight
			 ? parentHeight
			 : boxHeight);

   setViewSize(listSize);

   /* Rejustify the x and y positions if we need to. */
   alignxy(cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the scrolling window */
   win = newwin(boxHeight, boxWidth, ypos, xpos);

   /* Is the scrolling window null?? */
   if (!win) {
      destroyCDKObject();
			return;
   }

   /* Turn the keypad on for the window. */
   keypad(win, TRUE);

   /* Create the scrollbar window. */
   if (splace == RIGHT) {
      scrollbarWin = subwin(win,
				      MaxViewSize(), 1,
				      SCREEN_YPOS(this, ypos),
				      xpos + boxWidth
				      - borderSize - 1);
   } else if (splace == LEFT) {
      scrollbarWin = subwin(win,
				      MaxViewSize(), 1,
				      SCREEN_YPOS(this, ypos),
				      SCREEN_XPOS(this, xpos));
   } else {
      scrollbarWin = 0;
   }

   /* create the list window */

   listWin = subwin(win,
			      MaxViewSize(),
			      boxWidth
			      - 2 * borderSize - scrollAdjust,
			      SCREEN_YPOS(this, ypos),
			      SCREEN_XPOS(this, xpos)
			      +(splace == LEFT ? 1 : 0));

   /* *INDENT-EQLS* Set the rest of the variables */
   //ScreenOf(this)           = cdkscreen;
	 screen							 = cdkscreen;
   parent              = cdkscreen->window;
   shadowWin           = 0;
   scrollbarPlacement  = splace;
   maxLeftChar         = 0;
   leftChar            = 0;
   initExitType(this);
   ObjOf(this)->acceptsFocus = TRUE;
   ObjOf(this)->inputWindow = win;
   shadow              = pshadow;
	 highlight						 = phighlight;
   SetPosition(0);
   /* Create the scrolling list item list and needed variables. */
   if (createCDKScrollItemList(numbers, 
#ifdef pneu
				 plistp
#else
				 list, listSize
#endif
				 ) <= 0) {
      destroyCDKObject();
      return;
   }
   /* Do we need to create a shadow? */
   if (shadow) {
      shadowWin = newwin(boxHeight,
				   boxWidth,
				   ypos + 1,
				   xpos + 1);
   }
   /* Setup the key bindings. */
   for (x = 0; x <(int)SIZEOF(bindings); ++x)
      bindCDKObject(/*vSCROLL,
		     this,*/
		    (chtype)bindings[x].from,
		     getcCDKBind,
		    (void *)(long)bindings[x].to);
   registerCDKObject(cdkscreen, vSCROLL);
   /* Return the scrolling list */
//   return this;
}

/*
 * This actually does all the 'real' work of managing the scrolling list.
 */
int SScroll::activateCDKScroll(chtype *actions)
{
	/* Draw the scrolling list */
	// hier entstehen keine Fehler
	this->drawCDKScroll(obbox,1);
	if (!actions) {
		chtype input;
		bool functionKey;
		for (;;) {
			int ret;
			scroll_FixCursorPosition();
			input = (chtype)this->getchCDKObject(&functionKey);
			/* Inject the character into the widget. */
			ret = this->injectCDKScroll(input);
			if (this->exitType != vEARLY_EXIT) {
				return ret;
			}
		}
	} else {
		int length = chlen(actions);
		/* Inject each character one at a time. */
		for (int i = 0; i < length; i++) {
			int ret = injectCDKScroll(actions[i]);
			if (this->exitType != vEARLY_EXIT)
				return ret;
		}
	}
	/* Set the exit type for the widget and return. */
	setExitType(0);
	return -1;
}

const int einrueck{1};
void SScroll::drawCDKScrollCurrent()
{
   /* Rehighlight the current menu item. */
   int screenPos = this->itemPos[this->currentItem] - this->leftChar;
   chtype highlight = /*HasFocusObj(this)*/hasFocus ? this->highlight : 
		 // Anfang G.Schade 2.10.18
		 this->highlight
		 /*A_NORMAL*/
		 // Ende G.Schade 2.10.18
		 ;
   writeChtypeAttrib(this->listWin,
		      ((screenPos >= 0) ? screenPos : 0)+einrueck,
		      this->currentHigh,
#ifdef pneu
					this->pitem[this->currentItem].getinh(),
#else
		      this->sitem[this->currentItem],
#endif
		      highlight,
		      HORIZONTAL,
		      (screenPos >= 0) ? 0 :(1 - screenPos),
		      this->itemLen[this->currentItem]);
}

#undef  SCREEN_YPOS		/* because listWin is separate */
#define SCREEN_YPOS(w,n)(n)

/*
 * This redraws the scrolling list.
 */
void SScroll::drawCDKScrollList(bool Box)
{
	static int reihe{0};
	int anzy{0};
	/* If the list is empty, don't draw anything. */
	if (this->listSize > 0) {
		/* Redraw the list */
		reihe++;
		if (reihe>=22 && reihe<42) {
			reihe=reihe;
		}
		for (int j = 0; j < this->viewSize; j++) {
			int xpos = SCREEN_YPOS(this, 0);
			int ypos = SCREEN_YPOS(this, j);
			writeBlanks(this->listWin, xpos, ypos, HORIZONTAL, 0, this->boxWidth - 2 * borderSize);
			int k = j + this->currentTop;
			/* Draw the elements in the scroll list. */
			if (k < this->listSize) {
				int screenPos = SCREENPOS(this, k);
				/* Write in the correct line. */
				// zeichnet alle, ohne das Aktuelle zu markieren
#ifdef pneu
				mvwprintw(parent,anzy++,90,"%i: cury: %i %s",reihe,listWin->_cury,pitem[k].chtype2Char());
#else
				mvwprintw(parent,anzy++,90,"%i: cury: %i %s",reihe,listWin->_cury,chtype2Char(sitem[k]));
#endif
				writeChtype(this->listWin,
						((screenPos >= 0) ? screenPos : 1)+einrueck,
						ypos, 
#ifdef pneu
						this->pitem[k].getinh(),
#else
						this->sitem[k], 
#endif
						HORIZONTAL,
						(screenPos >= 0) ? 0 :(1 - screenPos),
						this->itemLen[k]);
			}
		}
		// zeichnet nur das Markierte
		this->drawCDKScrollCurrent(); wrefresh(this->win);
		/* Determine where the toggle is supposed to be. */
		if (this->scrollbarWin) {
			this->togglePos = floorCDK(this->currentItem *(double)this->step);
			/* Make sure the toggle button doesn't go out of bounds. */
			if (this->togglePos >= getmaxy(this->scrollbarWin))
				this->togglePos = getmaxy(this->scrollbarWin) - 1;
			/* Draw the scrollbar. */
			(void)mvwvline(this->scrollbarWin,
					0, 0,
					ACS_CKBOARD,
					getmaxy(this->scrollbarWin));
			(void)mvwvline(this->scrollbarWin,
					this->togglePos, 0,
					' ' | A_REVERSE,
					this->toggleSize);
		}
	}
	/* Box it if needed. */
	if(Box) {
		drawObjBox(win);
	} else {
		touchwin(this->win);
	}
	wrefresh(this->win);
} // static void drawCDKScrollList


/*
 * This injects a single character into the widget.
 */
int SScroll::injectCDKScroll(/*CDKOBJS *object, */chtype input)
{
	//   SScroll *myself =(SScroll *)object;
//	CDKSCROLLER *widget =(CDKSCROLLER *)this;
	int ppReturn = 1;
	int ret = unknownInt;
	bool complete = FALSE;

	/* Set the exit type for the widget. */
	setExitType(0);

	/* Draw the scrolling list */
	drawCDKScrollList(obbox);

	/* Check if there is a pre-process function to be called. */
	if (PreProcessFuncOf(this)) {
		/* Call the pre-process function. */
		ppReturn = PreProcessFuncOf(this)(vSCROLL,
				this,
				PreProcessDataOf(this),
				input);
	}

	/* Should we continue? */
	if (ppReturn) {
		/* Check for a predefined key binding. */
		if (checkCDKObjectBind(input)) {
			checkEarlyExit(this);
			complete = TRUE;
		} else {
			switch (input) {
				case KEY_UP:
					scroll_KEY_UP();
					break;

				case KEY_DOWN:
					scroll_KEY_DOWN();
					break;

				case KEY_RIGHT:
					scroll_KEY_RIGHT();
					break;

				case KEY_LEFT:
					scroll_KEY_LEFT();
					break;

				case KEY_PPAGE:
					scroll_KEY_PPAGE();
					break;

				case KEY_NPAGE:
					scroll_KEY_NPAGE();
					break;

				case KEY_HOME:
					scroll_KEY_HOME();
					break;

				case KEY_END:
					scroll_KEY_END();
					break;

				case '$':
					/*widget->*/leftChar = /*widget->*/maxLeftChar;
					break;

				case '|':
					/*widget->*/leftChar = 0;
					break;

				case KEY_ESC:
					setExitType(input);
					complete = TRUE;
					break;

				case KEY_ERROR:
					setExitType(input);
					complete = TRUE;
					break;

				case CDK_REFRESH:
					screen->eraseCDKScreen();
					screen->refreshCDKScreen();
					break;

				case KEY_TAB:
				case KEY_ENTER:
					setExitType(input);
					ret = /*widget->*/currentItem;
					complete = TRUE;
					break;

				default:
					break;
			}
		}
		/* Should we call a post-process? */
		if (!complete && (PostProcessFuncOf(this/*widget*/)))
		{
			PostProcessFuncOf (this/*widget*/)(vSCROLL,
					this/*widget*/,
					PostProcessDataOf(this/*widget*/),
					input);
		}
	}
	if (!complete) {
		drawCDKScrollList(obbox);
		setExitType(0);
	}
	scroll_FixCursorPosition();
	ResultOf(this/*widget*/).valueInt = ret;
	return (ret != unknownInt);
} // static int _injectCDKScroll

/*
 * This sets the background attribute of the widget.
 */
void CDKOBJS::setBKattrObj(/*CDKOBJS *object, */chtype attrib)
{
//   if (object) {
//      SLabel *widget =(SLabel *)object;
//      wbkgd(this->win, attrib);
//   }
}

/*
 * This function creates the scrolling list information and sets up the needed
 * variables for the scrolling list to work correctly.
 */
int SScroll::createCDKScrollItemList(
				    bool nummern,
#ifdef pneu
						vector<string> *plistp
#else
				    CDK_CSTRING2 list,
				    int listSize
#endif
						)
{
	int status = 0;
	if (
#ifdef pneu
			plistp->size()
#else
			listSize
#endif
			> 0) {
		/* *INDENT-EQLS* */
#ifdef pneu
#else
		size_t have               = 0;
		char *temp                = 0;
#endif
		if (
#ifdef pneu
				1
#else
				allocListArrays(0, listSize)
#endif
				) {
			int widestItem = 0;
			int x = 0;
			/* Create the items in the scrolling list. */
			status = 1;
			for (x = 0; x < 
#ifdef pneu
					plistp->size()
#else
					listSize
#endif
					; x++) {
				if (!allocListItem(
							x,
#ifdef pneu
#else
							&temp,
							&have,
#endif
							nummern ?(x + 1) : 0,
#ifdef pneu
							plistp->at(x).c_str()
#else
							list[x]
#endif
							)) {
					status = 0;
					break;
				}
				status=1;
				widestItem = MAXIMUM(this->itemLen[x], widestItem);
			}
#ifdef pneu
#else
			freeChecked(temp);
#endif
			if (status) {
				updateViewWidth(widestItem);
				/* Keep the boolean flag 'numbers' */
				this->numbers = nummern;
			}
		}
	} else {
		status = 1;		/* null list is ok - for a while */
	}
	return status;
}

void SScroll_basis::updateViewWidth(int widest)
{
/* Determine how many characters we can shift to the right */
/* before all the items have been scrolled off the screen. */
	maxLeftChar=boxWidth>widest?0:widest-(boxWidth-2*borderSize);
}
#ifdef pneu
#else
bool SScroll::allocListArrays(int oldSize, int newSize)
{
	/* *INDENT-EQLS* */
	bool result;
	int nchunk           = ((newSize + 1) | 31) + 1;
	chtype **newList     = typeCallocN(chtype *, nchunk);
	int *newLen          = typeCallocN(int, nchunk);
	int *newPos          = typeCallocN(int, nchunk);
	if (newList && newLen && newPos ) {
		for (int n = 0; n < oldSize; ++n) {
			newList[n] = this->sitem[n];
			newLen[n] = this->itemLen[n];
			newPos[n] = this->itemPos[n];
		}
		freeChecked(this->sitem);
		freeChecked(this->itemPos);
		freeChecked(this->itemLen);
		this->sitem = newList;
		this->itemLen = newLen;
		this->itemPos = newPos;
		result = TRUE;
	} else {
		freeChecked(newList);
		freeChecked(newLen);
		freeChecked(newPos);
		result = FALSE;
	}
	return result;
}
#endif

bool SScroll::allocListItem(
			      int which,
#ifdef pneu
#else
			      char **work,
			      size_t * used,
#endif
			      int number,
			      const char *value)
{
	if (number > 0) {
		size_t need = NUMBER_LEN(value);
#ifdef pneu
	  string valuestr;
		valuestr.resize(need);
		sprintf(&valuestr[0],NUMBER_FMT,number,value);
		value=valuestr.c_str();
#else
		if (need > *used) {
			*used =((need + 2) * 2);
			if (!*work) {
				if (!(*work =(char*)malloc(*used)))
					return FALSE;
			} else {
				if (!(*work =(char*)realloc(*work, *used)))
					return FALSE;
			}
		}
		sprintf(*work, NUMBER_FMT, number, value);
		value = *work;
#endif
	}
//	if (!(this->sitem[which] = char2Chtypeh(value, &(this->itemLen[which]), &(this->itemPos[which])))) return FALSE;
#ifdef pneu
	int len,pos;
	chtstr sitemneu(value,&len,&pos);
  pos=justifyString(boxWidth,len,pos);
	pitem.insert(pitem.begin()+which,sitemneu);
	listSize++;
	itemLen.insert(itemLen.begin()+which,len);
	itemPos.insert(itemPos.begin()+which,pos);
#else
	chtstr sitemneu(value,&(this->itemLen[which]), &(this->itemPos[which]));
	sitemneu.rauskopier(&sitem[which]);
	this->itemPos[which] = justifyString(this->boxWidth, this->itemLen[which], this->itemPos[which]);
#endif
	return TRUE;
}

int chtstr::rauskopier(chtype **ziel)
{
	if ((*ziel=new chtype[len])) {
    memcpy(ziel,&inh,len);	
		return TRUE;
	}
	return FALSE;
}

/*
 * Destroy all of the objects on a screen
 */

void SScreen::destroyCDKScreenObjects()
{
	for (int x = 0; x < this->objectCount; x++) {
		CDKOBJS *obj = this->object[x];
		if (obj) {
			int before = this->objectCount;
			//		if (validObjType(obj, ObjTypeOf(obj))) KLA
			//			MethodPtr(obj, eraseObj)(obj);
			obj->eraseObj();
			obj->destroyCDKObject();
			x -= (this->objectCount - before);
			//		KLZ
		}
	}
}

/*
 * This destroys a CDK screen.
 */
void SScreen::destroyCDKScreen()
{
	size_t pos{0};
	for(auto akt:all_screens) {
		if (akt==this) {
			all_screens.erase(all_screens.begin()+pos);
			break;
//			akt->destroyObj();
			free(akt);
		}
		pos++;
	}
	/*
	ALL_SCREENS *p, *q;
	for (p = all_screens, q = 0; p; q = p, p = p->link) {
		if (screen == p->screen) {
			if (q)
				q->link = p->link;
			else
				all_screens = p->link;
			free(p);
			free(screen);
			break;
		}
	}
	*/
}

/*
 * This clears all the objects in the screen.
 */
void SScreen::eraseCDKScreen()
{
//	int objectCount = this->objectCount;
	/* We just call the drawObject function. */
	for (int x = 0; x < objectCount; x++) {
		CDKOBJS *obj = this->object[x];
		if (obj) {
			//		if (validObjType(obj, ObjTypeOf(obj))) KLA
			if (obj->validObjType(obj->cdktype)) {
				obj->eraseObj();
			}
		}
	}
	/* Refresh the screen. */
	wrefresh (this->window);
}

void CDKOBJS::eraseObj()
{
}

#define limitFocusIndex(screen, value) \
 	(((value) >=(screen)->objectCount ||(value) < 0) \
	 ? 0 \
	 :(value))

int SScreen::getFocusIndex()
{
   int result = limitFocusIndex(this, objectFocus);
   return result;
}


void SScreen::setFocusIndex(int value)
{
   objectFocus = limitFocusIndex(this, value);
}


/*
 * This creates a new CDK screen.
 */
//SScreen *initCDKScreen(WINDOW *window)
SScreen::SScreen(WINDOW *window)
{
	//	ALL_SCREENS *item;
	//	SScreen *screen = 0;
	/* initialization, for the first time */
	if (/*all_screens == 0 || */!stdscr || !window) {
		/* Set up basic curses settings. */
#ifdef HAVE_SETLOCALE
		setlocale(LC_ALL, "");
#endif
		/* Initialize curses after setting the locale, since curses depends
		 * on having a correct locale to reflect the terminal's encoding.
		 */
		if (!stdscr || !window) {
			window = initscr();
		}
		noecho();
		cbreak();
	}

	//	if ((item = typeMalloc(ALL_SCREENS))) {
	//		if ((screen = typeCalloc(SScreen))) {
	/*
		 item->link = all_screens;
		 item->screen = this;
		 all_screens = item;
	 */
	all_screens.push_back(this);

	/* Initialize the SScreen pointer. */
	this->objectCount = 0;
#ifdef pneu
#else
	this->objectLimit = 2;
	this->object = typeMallocN(CDKOBJS *, this->objectLimit);
#endif
	this->window = window;

	/* OK, we are done. */
	//		} else { free(item); }
	//	}
	//	return(screen);
}

/*
 * This creates a label widget.
 */
//SLabel *newCDKLabel(SScreen *cdkscreen,
SLabel::SLabel(SScreen *cdkscreen,
		       int xplace,
		       int yplace,
#ifdef pneu
					 vector<string> mesg,
#else
		       CDK_CSTRING2 mesg,
		       int prows,
#endif
		       bool Box,
		       bool shadow): xpos(xplace),ypos(yplace),boxWidth(INT_MIN),
#ifdef pneu
	rows(mesg.size()),
#else
	rows(prows),
#endif
	shadow(shadow)
{
   /* *INDENT-EQLS* */
//   SLabel *label      = 0;
   int parentWidth      = getmaxx(cdkscreen->window);
   int parentHeight     = getmaxy(cdkscreen->window);
//   int boxWidth         = INT_MIN;
//   int boxHeight;
   //int xpos             = xplace;
   //int ypos             = yplace;
   int x                = 0;

	::CDKOBJS();
   if (rows <= 0
#ifdef pneu
#else
       /*|| (label = newCDKObject(SLabel, &my_funcs)) == 0*/
       || (/*label->*/sinfo = typeCallocN(chtype *, rows + 1)) == 0
       || (/*label->*/infoLen = typeCallocN(int, rows + 1)) == 0
       || (/*label->*/infoPos = typeCallocN(int, rows + 1)) == 0
#endif
			 )
   {
      destroyCDKObject(/*label*/);
      return /*(0)*/;
   }

   setCDKLabelBox(/*label, */Box);
   boxHeight = rows + 2 * /*BorderOf(label)*/ borderSize;

   /* Determine the box width. */
   for (x = 0; x < 
#ifdef pneu
			 mesg.size()
#else
			 rows
#endif
			 ; x++) {
      /* Translate the char * to a chtype. */
//      /*label->*/info[x] = char2Chtypeh(mesg[x], &/*label->*/infoLen[x], &/*label->*/infoPos[x]);
#ifdef pneu
		 int len,pos;
		 chtstr infoneu(mesg[x].c_str(),&len,&pos);
		 //	pos=justifyString(boxWidth,len,pos);
		 pinfo.insert(pinfo.begin()+x,infoneu);
		 // listSize++
		 infoLen.insert(infoLen.begin()+x,len);
		 infoPos.insert(infoPos.begin()+x,pos);
#else
		 chtstr infoneu(mesg[x]
#ifdef pneu
	 					.c_str()
#else
#endif
				 ,&infoLen[x],&infoPos[x]);
		 infoneu.rauskopier(&sinfo[x]);
#endif
		 boxWidth = MAXIMUM(boxWidth, /*label->*/infoLen[x]);
	 }
   boxWidth += 2 * /*BorderOf(label)*/ borderSize;

   /* Create the string alignments. */
   for (x = 0; x < rows; x++) {
      /*label->*/infoPos[x] = justifyString(boxWidth - 2 * /*BorderOf(label)*/ borderSize,
					 /*label->*/infoLen[x],
					 /*label->*/infoPos[x]);
   }

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   boxWidth =(boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight =(boxHeight > parentHeight ? parentHeight : boxHeight);

   /* Rejustify the x and y positions if we need to. */
   alignxy(cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* *INDENT-EQLS* Create the label. */
   /*ScreenOf(label)*/screen     = cdkscreen;
   /*label->*/parent        = cdkscreen->window;
   /*label->*/win           = newwin(boxHeight, boxWidth, ypos, xpos);
   /*label->*/shadowWin     = 0;
//   /*label->*/xpos          = xpos;
//   /*label->*/ypos          = ypos;
//   /*label->*/rows          = rows;
//   /*label->*/boxWidth      = boxWidth;
//   /*label->*/boxHeight     = boxHeight;
   /*ObjOf(label)->*/inputWindow = /*label->*/win;
   /*ObjOf(label)->*/hasFocus = FALSE;
//   label->shadow        = shadow;

   /* Is the window null? */
   if (/*label->*/win == 0)
   {
      destroyCDKObject(/*label*/);
      return /*(0)*/;
   }
   keypad(/*label->*/win, TRUE);

   /* If a shadow was requested, then create the shadow window. */
   if (shadow)
   {
      /*label->*/shadowWin = newwin(boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Register this baby. */
   registerCDKObject(cdkscreen, vLABEL/*, label*/);

   /* Return the label pointer. */
//   return (label);
}

/*
 * This sets the box flag for the label widget.
 */
void SLabel::setCDKLabelBox(/*SLabel *label, */bool Box)
{
   /*ObjOf(label)->*/obbox = Box;
   /*ObjOf(label)->*/borderSize = Box ? 1 : 0;
}

bool SLabel::getCDKLabelBox(/*SLabel *label*/)
{
   return /*ObjOf(label)->*/obbox;
}

/*
 * This was added for the builder.
 */
void SLabel::activateCDKLabel(/*SLabel *label, */chtype *actions GCC_UNUSED)
{
   drawCDKLabel(/*label, ObjOf(label)->*/obbox);
}

/*
 * This sets multiple attributes of the widget.
 */
void SLabel::setCDKLabel(/*SLabel *label, */
#ifdef pneu
			 std::vector<std::string> mesg
#else
			 CDK_CSTRING2 mesg, int lines
#endif
		, bool Box)
{
   setCDKLabelMessage(/*label, */mesg
#ifdef pneu
#else
			 , lines
#endif
	 );
   setCDKLabelBox(/*label, */Box);
}

/*
 * This sets the information within the label.
 */
void SLabel::setCDKLabelMessage(/*SLabel *label, */
#ifdef pneu
			 std::vector<std::string> s_info
#else
			 CDK_CSTRING2 s_info, int infoSize
#endif
		)
{
   int x;
   int limit;

   /* Clean out the old message. */
#ifdef pneu
	 pinfo.clear();
	 infoPos.clear();
	 infoLen.clear();
#else
   for (x = 0; x < /*label->*/rows; x++) {
      freeChtype(/*label->*/sinfo[x]);
      /*label->*/sinfo[x] = 0;
      /*label->*/infoPos[x] = 0;
      /*label->*/infoLen[x] = 0;
   }
#endif

   /* update the label's length - but taking into account its window size */
   limit = /*label->*/boxHeight -(2 * /*BorderOf(label)*/ borderSize);
#ifdef pneu
	 rows=s_info.size();
	 if (rows>limit) rows=limit;
#else
   if (infoSize > limit)
      infoSize = limit;
   /*label->*/rows = infoSize;
#endif

   /* Copy in the new message. */
   for (x = 0; x < /*label->*/rows; x++) {
//      /*label->*/info[x] = char2Chtypeh(s_info[x], &/*label->*/infoLen[x], &/*label->*/infoPos[x]);
#ifdef pneu
		 int len,pos;
		 chtstr infoneu(s_info[x].c_str(),&len,&pos);
		 pos=justifyString(boxWidth-2*borderSize,len,pos);
		 pinfo.insert(pinfo.begin()+x,infoneu);
		 // listSize++
		 infoLen.insert(infoLen.begin()+x,len);
		 infoPos.insert(infoPos.begin()+x,pos);
#else
      chtstr infoneu(s_info[x],&infoLen[x],&infoPos[x]);
			infoneu.rauskopier(&sinfo[x]);
      /*label->*/infoPos[x] = justifyString(/*label->*/boxWidth - 2 * /*BorderOf(label)*/ borderSize,
					 /*label->*/infoLen[x],
					 /*label->*/infoPos[x]);
#endif
   }

   /* Redraw the label widget. */
   eraseCDKLabel(/*label*/);
   drawCDKLabel(/*label, ObjOf(label)->*/obbox);
}

#ifdef pneu
#else
chtype **SLabel::getCDKLabelMessage(/*SLabel *label, */int *size)
{
   (*size) = /*label->*/rows;
   return /*label->*/sinfo;
}
#endif

/*
 * This sets the background attribute of the widget.
 */
void SLabel::setBKattrLabel(/*CDKOBJS *object, */chtype attrib)
{
//   if (object) {
//      SLabel *widget =(SLabel *)object;
      wbkgd(/*widget->*/win, attrib);
//   }
}

/*
 * This draws the label widget.
 */
void SLabel::drawCDKLabel(/*CDKOBJS *object, */bool Box GCC_UNUSED)
{
//   SLabel *label =(SLabel *)object;
   /* Is there a shadow? */
   if (/*label->*/shadowWin) {
      drawShadow(/*label->*/shadowWin);
   }
   /* Box the widget if asked. */
   if (/*ObjOf(label)->*/obbox) {
      drawObjBox(/*label->*/win/*, ObjOf(label)*/);
   }
   /* Draw in the message. */
   for (int x = 0; x < /*label->*/rows; x++) {
      writeChtype(/*label->*/win,
		   /*label->*/infoPos[x] + /*BorderOf(label)*/ borderSize,
		   x + /*BorderOf(label)*/ borderSize,
#ifdef pneu
						this->pinfo[x].getinh(),
#else
		   /*label->*/sinfo[x],
#endif
		   HORIZONTAL,
		   0,
		   /*label->*/infoLen[x]);
   }
   /* Refresh the window. */
   wrefresh(/*label->*/win);
}

/*
 * This erases the label widget.
 */
void SLabel::eraseCDKLabel(/*CDKOBJS *object*/)
{
//   if (validCDKObject(object)) {
//      SLabel *label =(SLabel *)object;
      eraseCursesWindow(/*label->*/win);
      eraseCursesWindow(/*label->*/shadowWin);
//   }
}

/*
 * This moves the label field to the given location.
 */
void SLabel::moveCDKLabel(/*CDKOBJS *object,*/
			   int xplace,
			   int yplace,
			   bool relative,
			   bool refresh_flag)
{
//   SLabel *label =(SLabel *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx(/*label->*/win);
   int currentY = getbegy(/*label->*/win);
   int xpos     = xplace;
   int ypos     = yplace;
   int xdiff    = 0;
   int ydiff    = 0;

   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative) {
      xpos = getbegx(/*label->*/win) + xplace;
      ypos = getbegy(/*label->*/win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy(WindowOf(this), &xpos, &ypos, /*label->*/boxWidth, /*label->*/boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow(/*label->*/win, -xdiff, -ydiff);
   moveCursesWindow(/*label->*/shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow(WindowOf(this));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag) {
      drawCDKLabel(/*label, ObjOf(label)->*/obbox);
   }
}

/*
 * This destroys the label object pointer.
 */
void SLabel::destroyCDKLabel(/*CDKOBJS *object*/)
{
//   if (object) {
//      SLabel *label =(SLabel *)object;

#ifdef pneu
#else
      CDKfreeChtypes(/*label->*/sinfo);
      freeChecked(/*label->*/infoLen);
      freeChecked(/*label->*/infoPos);
#endif

      /* Free up the window pointers. */
      deleteCursesWindow(/*label->*/shadowWin);
      deleteCursesWindow(/*label->*/win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings(/*vLABEL, this*/);

      /* Unregister the object. */
      unregisterCDKObject(vLABEL/*, this*/);
//   }
}

/*
 * This pauses until a user hits a key...
 */
char SLabel::waitCDKLabel(/*SLabel *label, */char key)
{
	int code;
	bool functionKey;
	/* If the key is null, we'll accept anything. */
	if (!key) {
		code = getchCDKObject(/*ObjOf(label), */&functionKey);
	} else {
		/* Only exit when a specific key is hit. */
		for (;;) {
			code = getchCDKObject(/*ObjOf(label), */&functionKey);
			if (code == key) {
				break;
			}
		}
	}
	return (char)(code);
}

/*
 * This pops up a message.
 */
void SScreen::popupLabel(/*SScreen *screen, */
#ifdef pneu
			 std::vector<std::string> mesg
#else
			 CDK_CSTRING2 mesg, int count
#endif
		)
{
//   SLabel *popup = 0;
   int oldCursState;
   bool functionKey;
   /* Create the label. */
   SLabel popup(this,CENTER, CENTER, mesg, 
#ifdef pneu
#else
			 count, 
#endif
			 TRUE, FALSE);
   oldCursState = curs_set(0);
   /* Draw it on the screen. */
   popup.drawCDKLabel(/*popup, */TRUE);
   /* Wait for some input. */
   keypad(popup.win, TRUE);
   popup.getchCDKObject(/*ObjOf(popup), */&functionKey);
   /* Kill it. */
   popup.destroyCDKLabel();
   /* Clean the screen. */
   curs_set(oldCursState);
   eraseCDKScreen();
   refreshCDKScreen();
}

/*
 * This pops up a message.
 */
void SScreen::popupLabelAttrib(/*SScreen *screen, */
#ifdef pneu
			 std::vector<std::string> mesg
#else
			 CDK_CSTRING2 mesg, int count
#endif
		, chtype attrib)
{
//   SLabel *popup = 0;
   int oldCursState;
   bool functionKey;
   /* Create the label. */
   SLabel popup(this,CENTER, CENTER, mesg, 
#ifdef pneu
#else
			 count, 
#endif
			 TRUE, FALSE);
//   popup.setCDKLabelBackgroundAttrib(attrib);
   popup.setBKattrObj(attrib);
   oldCursState = curs_set(0);
   /* Draw it on the screen. */
   popup.drawCDKLabel(TRUE);
   /* Wait for some input. */
   keypad(popup.win, TRUE);
   popup.getchCDKObject(/*ObjOf(popup), */&functionKey);
   /* Kill it. */
   popup.destroyCDKLabel();
   /* Clean the screen. */
   curs_set(oldCursState);
   eraseCDKScreen();
   refreshCDKScreen();
} 

/*
 * This sets the background color of the widget.
 */
void CDKOBJS::setCDKObjectBackgroundColor(/*CDKOBJS *obj, */const char *color)
{
   int junk1, junk2;
   /* Make sure the color isn't null. */
   if (!color) {
      return;
   }
   /* Convert the value of the environment variable to a chtype. */
//   chtype *holder = char2Chtypeh(color, &junk1, &junk2);
	 chtstr holder(color,&junk1,&junk2);
   /* Set the widget's background color. */
   setBKattrObj(/*obj, */holder.getinh()[0]);
   /* Clean up. */
//   freeChtype(holder);
}


void CDKOBJS::setULcharObj(chtype ch)
{
	ULChar=ch;
}

void CDKOBJS::drawObj(bool Box)
{
}
void SEntry::drawObj(bool Box)
{
	drawCDKEntry(Box);
}
void SScroll::drawObj(bool Box)
{
	// mit 1 entstehen hier Fehler nur unten, nicht oben
	drawCDKScroll(Box,1);
}
void SFSelect::drawObj(bool Box)
{
	drawCDKFselect(Box);
}
void SAlphalist::drawObj(bool Box)
{
	drawCDKAlphalist(Box);
}

//void CDKOBJS::setBKattrObj(chtype attrib) { wbkgd(win, attrib); }

void SLabel::setBKattrObj(chtype attrib)
{
	setBKattrLabel(attrib);
}
/*
 * This sets the background attribute of the widget.
 */
void SEntry::setBKattrObj(chtype attrib)
{
	setBKattrEntry(attrib);
}

/*
 * This sets the background attribute of the widget.
 */
void SAlphalist::setBKattrObj(chtype attrib)
{
	entryField->setBKattrObj(attrib);
	scrollField->setBKattrObj(attrib);
}

/*
 * This sets the background attribute of the widget.
 */
void SScroll::setBKattrObj(chtype attrib)
{
	setBKattrScroll(attrib);
}

/*
 * This draws the file selector widget.
 */
void SFSelect::drawCDKFselect(/*CDKOBJS *object, */bool Box GCC_UNUSED)
{
//   SFSelect *fselect =(SFSelect *)object;

   /* Draw in the shadow if we need to. */
   if (/*fselect->*/shadowWin) {
      drawShadow(/*fselect->*/shadowWin);
   }

   /* Draw in the entry field. */
   entryField->drawCDKEntry(/*fselect->entryField, ObjOf(fselect->entryField)->*/obbox);

   /* Draw in the scroll field. */
   drawMyScroller(/*fselect*/);
}

/*
 * The fselect's focus resides in the entry widget.  But the scroll widget
 * will not draw items highlighted unless it has focus.  Temporarily adjust the
 * focus of the scroll widget when drawing on it to get the right highlighting.
 */
/*
#define SaveFocus(widget) \
   boolean save = HasFocusObj(ObjOf(widget->scrollField)); \
   HasFocusObj(ObjOf(widget->scrollField)) = \
   HasFocusObj(ObjOf(widget->entryField))

#define RestoreFocus(widget) \
   HasFocusObj(ObjOf(widget->scrollField)) = save
	 */

void SFSelect::drawMyScroller(/*SFSelect *widget*/)
{
   SaveFocus(this);
   scrollField->drawCDKScroll(/*widget->scrollField, ObjOf(widget->scrollField)->*/scrollField->obbox);
   RestoreFocus(this);
}

/*
 * Store the name of the current working directory.
 */
void SFSelect::setPWD(/*SFSelect *fselect*/)
{
   char buffer[512];
#ifdef pneu
#else
   freeChecked(this->pwd);
#endif
   if (!getcwd(buffer, sizeof(buffer)))
      strcpy(buffer, ".");
#ifdef pneu
	 this->pwd=buffer;
#else
   this->pwd = copyChar(buffer);
#endif
}

/*
 * This opens the current directory and reads the contents.
 */
int CDKgetDirectoryContents(const char *directory, 
#ifdef pneu
		vector<string> *plist
#else
		char ***list
#endif
		)
{
	/* Declare local variables.  */
	struct dirent *dirStruct;
#ifdef pneu
#else
	int counter = 0;
#endif
	DIR *dp;
	unsigned used = 0;
	/* Open the directory.  */
	dp = opendir(directory);
	/* Could we open the directory?  */
	if (!dp) {
		return -1;
	}
	/* Read the directory.  */
	while ((dirStruct = readdir(dp))) {
		if (strcmp(dirStruct->d_name, "."))
#ifdef pneu
      plist->push_back(dirStruct->d_name);
#else
			used = CDKallocStrings(list, dirStruct->d_name,(unsigned)counter++, used);
#endif
	}
	/* Close the directory.  */
	closedir(dp);
	/* Sort the info.  */
#ifdef pneu
	sort(plist->begin(),plist->end());
#else
	sortList((CDK_CSTRING *)*list, counter);
#endif
	/* Return the number of files in the directory.  */
	return 
#ifdef pneu
		plist->size();
#else
		counter
#endif
		;
}

int mode2Filetype(mode_t mode)
{
	/* *INDENT-OFF* */
	static const struct {
		mode_t	mode;
		char	code;
	} table[] = {
#ifdef S_IFBLK
		{ S_IFBLK,  'b' },  /* Block device */
#endif
		{ S_IFCHR,  'c' },  /* Character device */
		{ S_IFDIR,  'd' },  /* Directory */
		{ S_IFREG,  '-' },  /* Regular file */
#ifdef S_IFLNK
		{ S_IFLNK,  'l' },  /* Socket */
#endif
#ifdef S_IFSOCK
		{ S_IFSOCK, '@' },  /* Socket */
#endif
		{ S_IFIFO,  '&' },  /* Pipe */
	};
	/* *INDENT-ON* */

	int filetype = '?';
	for (unsigned n = 0; n < sizeof(table) / sizeof(table[0]); n++) {
		if ((mode & S_IFMT) == table[n].mode) {
			filetype = table[n].code;
			break;
		}
	}
	return filetype;
}

#ifdef pneu
#else
char *format3String(const char *format, const char *s1, const char *s2, const char *s3)
{
	char *result;
	if ((result =(char *)malloc(strlen(format) +
					strlen(s1) +
					strlen(s2) +
					strlen(s3))))
		sprintf(result, format, s1, s2, s3);
	return result;
}
#endif

/*
 * This creates a list of the files in the current directory.
 */
int SFSelect::setCDKFselectdirContents(/*CDKFSELECT *fselect*/)
{
	struct stat fileStat;
#ifdef pneu
	vector<string> dirList;
#else
	char **dirList = 0;
#endif
	int fileCount;
	/* Get the directory contents. */
	fileCount = CDKgetDirectoryContents(this->pwd
#ifdef pneu
			.c_str()
#else
#endif
			, &dirList);
	if (fileCount <= 0) {
#ifdef pneu
#else
		/* We couldn't read the directory. Return. */
		CDKfreeStrings(dirList);
#endif
		return 0;
	}
	/* Clean out the old directory list. */
#ifdef pneu
#else
	CDKfreeStrings(this->dirContents);
	this->fileCounter = fileCount;
#endif
	this->dirContents = dirList;
	/* Set the properties of the files. */
	for (int x = 0; x < 
#ifdef pneu
			dirList.size()
#else
			this->fileCounter
#endif
			; x++) {
		const char *attr = "";
		const char *mode = "?";

		/* FIXME: access() would give a more correct answer */
		if (!lstat(dirList[x]
#ifdef pneu
					.c_str()
#else
#endif
					, &fileStat)) {
			mode = " ";
			if ((fileStat.st_mode & S_IXUSR)) {
				mode = "*";
			}
#if defined(S_IXGRP) && defined(S_IXOTH)
			else if (((fileStat.st_mode & S_IXGRP)) ||
					((fileStat.st_mode & S_IXOTH))) {
				mode = "*";
			}
#endif
		}
		switch (mode2Filetype(fileStat.st_mode)) {
			case 'l':
				attr = this->linkAttribute
#ifdef pneu
					.c_str()
#else
#endif
					;
				mode = "@";
				break;
			case '@':
				attr = this->sockAttribute
#ifdef pneu
					.c_str()
#else
#endif
					;
				mode = "&";
				break;
			case '-':
				attr = this->fileAttribute
#ifdef pneu
					.c_str()
#else
#endif
					;
				break;
			case 'd':
				attr = this->dirAttribute
#ifdef pneu
					.c_str()
#else
#endif
					;
				mode = "/";
				break;
			default:
				break;
		}
#ifdef pneu
		this->dirContents[x]=attr+dirList[x]+mode;
#else
		char *oldItem = dirList[x];
		this->dirContents[x] = format3String("%s%s%s", attr, dirList[x], mode);
		free(oldItem);
#endif
	}
	return 1;
}
#ifdef pneu
string make_pathname(const string& dir,const char* file)
{
	if (dir=="/") return dir+file;
	else return dir+"/"+file;
}
string make_pathname(const char *dir,const string& file)
{
	if (!strcmp(dir, "/")) return dir+file;
	else return dir+("/"+file);
}
#else
static char *make_pathname(const char *directory, const char *filename)
{
	size_t need = strlen(filename) + 2;
	bool root =(!strcmp(directory, "/"));
	char *result;

	if (!root)
		need += strlen(directory);
	if ((result =(char *)malloc(need))) {
		if (root)
			sprintf(result, "/%s", filename);
		else
			sprintf(result, "%s/%s", directory, filename);
	}
	return result;
}
#endif

/*
 * trim the 'mode' from a copy of a dirContents[] entry.
 */
static char *trim1Char(char *source)
{
   size_t len;
   if ((len = strlen(source)))
      source[--len] = '\0';
   return source;
}

/*
 * Start of callback functions.
 */
int fselectAdjustScrollCB(EObjectType objectType GCC_UNUSED,
				  void *object GCC_UNUSED,
				  void *clientData,
				  chtype key)
{
	/* *INDENT-EQLS* */
	SFSelect *fselect  =(SFSelect *)clientData;
	SScroll *scrollp   =(SScroll *)fselect->scrollField;
	SEntry *entry      =(SEntry *)fselect->entryField;
	if (scrollp->listSize > 0) {
		char *current;
		/* Move the scrolling list. */
		fselect->injectMyScroller(key);
		/* Get the currently highlighted filename. */
#ifdef pneu
		current = scrollp->pitem[scrollp->currentItem].chtype2Char();
#else
		current = chtype2Char(scrollp->sitem[scrollp->currentItem]);
#endif
		trim1Char(current);
#ifdef pneu
		/* Set the value in the entry field. */
		entry->setCDKEntryValue(make_pathname(fselect->pwd, current).c_str());
#else
		char *temp = make_pathname(fselect->pwd, current);
		/* Set the value in the entry field. */
		entry->setCDKEntryValue(temp);
#endif
		entry->drawCDKEntry(/*entry, ObjOf(entry)->*/entry->obbox);
#ifdef pneu
#else
		freeChecked(current);
		freeChecked(temp);
#endif
		return (TRUE);
	}
	Beep();
	return (FALSE);
}

#ifdef pneu
// Pfadname einer Datei
//std::string dirName(const std::string& path)
std::string dirName(string path)
{
  size_t letzt=path.find_last_of("/\\");
	if (letzt==string::npos) return {};
  return path.substr(0,letzt);
} // std::string dir_name(std::string const & path)
std::string dirName(const char* pfad)
{
	string path{pfad};
	return dirName(path);
} // std::string dir_name(std::string const & path)
#else
/*
 * Returns the directory for the given pathname, i.e., the part before the
 * last slash.
 */
char *dirName(const char *pathname)
{
	char *dir = 0;
	size_t pathLen;

	/* Check if the string is null.  */
	if (pathname
			&&(dir = copyChar(pathname))
			&&(pathLen = strlen(pathname)))
	{
		size_t x = pathLen;
		while ((dir[x] != '/') &&(x > 0))
		{
			dir[x--] = '\0';
		}
	}
	return dir;
}
#endif

/*
 * This takes a ~ type account name and returns the full pathname.
 */
static const char *expandTilde(const char *filename)
{
	const char *result = 0;
#ifdef pneu
	std::string account,pathname;
#else
	char *account;
	char *pathname;
#endif
	int len;

	/* Make sure the filename is not null/empty, and begins with a tilde */
	if ((filename) &&
			(len =(int)strlen(filename)) &&
			filename[0] == '~' &&
#ifdef pneu
			(account=filename,!account.empty())
			&& (pathname=filename,!pathname.empty())
#else
			(account = copyChar(filename))
				&& (pathname = copyChar(filename))
#endif
		)
	{
		bool slash = FALSE;
		const char *home;
		int x;
		int len_a = 0;
		int len_p = 0;
		struct passwd *accountInfo;

		/* Find the account name in the filename. */
		for (x = 1; x < len; x++)
		{
			if (filename[x] == '/' && !slash) {
				slash = TRUE;
			} else if (slash) {
				pathname[len_p++] = filename[x];
			} else {
				account[len_a++] = filename[x];
			}
		}
#ifdef pneu
		account.resize(len_a);
		pathname.resize(len_p);
#else
		account[len_a] = '\0';
		pathname[len_p] = '\0';
#endif

		home = 0;
#ifdef HAVE_PWD_H
#ifdef pneu
		if (!account.empty() && (accountInfo=getpwnam(account.c_str())))
#else
		if (strlen(account) &&
				(accountInfo = getpwnam(account)))
#endif
		{
			home = accountInfo->pw_dir;
		}
#endif
		if (home == 0 || *home == '\0')
			home = getenv("HOME");
		if (home == 0 || *home == '\0')
			home = "/";

		/*
		 * Construct the full pathname. We do this because someone
		 * may have a pathname at the end of the account name
		 * and we want to keep it.
		 */
#ifdef pneu
		result = make_pathname(home, pathname).c_str();
#else
		result = make_pathname(home, pathname);
#endif

#ifdef pneu
#else
		freeChecked(account);
		freeChecked(pathname);
#endif
	}
	return result;
}

#ifdef pneu
string format1String(const char* format, const char *stri)
{
	string ziel;
	ziel.resize(strlen(format)+strlen(stri));
	sprintf(&ziel[0],format,stri);
	ziel.resize(ziel.find((char)0));
	return ziel;
}
#else
static char *format1String(const char *format, const char *stri)
{
   char *result;
   if ((result = (char *)malloc(strlen(format) + strlen(stri))))
      sprintf(result, format, stri);
   return result;
}
#endif

#ifdef pneu
string errorMessage(const char *format)
#else
static char *errorMessage(const char *format)
#endif
{
	char *message;
#ifdef HAVE_STRERROR
	message = strerror(errno);
#else
	message = "Unknown reason.";
#endif
	return format1String(format, message);
}

#ifdef pneu
string format1StrVal(const char* format, const char *stri, int value)
{
	string ziel;
	ziel.resize(strlen(format)+strlen(stri)+20);
	sprintf(&ziel[0],format,stri,value);
	ziel.resize(ziel.find((char)0));
	return ziel;
}
#else
static char *format1StrVal(const char *format, const char *string, int value)
{
   char *result;
   if ((result =(char *)malloc(strlen(format) + strlen(string) + 20)))
      sprintf(result, format, string, value);
   return result;
}
#endif

#ifdef pneu
string format1Number(const char* format, long value)
{
	string ziel;
	ziel.resize(strlen(format)+20);
	sprintf(&ziel[0],format,value);
	ziel.resize(ziel.find((char)0));
	return ziel;
}
#else
static char *format1Number(const char *format, long value)
{
   char *result;
   if ((result =(char *)malloc(strlen(format) + 20)))
      sprintf(result, format, value);
   return result;
}
#endif

#ifdef pneu
string format1Date(const char* format, time_t value)
{
	string ziel;
	char *temp=ctime(&value);
	ziel.resize(strlen(format)+strlen(temp)+1);
	sprintf(&ziel[0],format,trim1Char(temp));
	ziel.resize(ziel.find((char)0));
	return ziel;
}
#else
static char *format1Date(const char *format, time_t value)
{
   char *result;
   char *temp = ctime(&value);
   if ((result =(char *)malloc (strlen(format) + strlen(temp) + 1))) {
      sprintf(result, format, trim1Char(temp));
   }
   return result;
}
#endif



#ifdef pneu
#else
/*
 * Corresponding list freeing (does not free the list pointer).
 */
void freeCharList(char **list, unsigned size)
{
	if (list) {
		while (size--) {
			freeChecked(list[size]);
			list[size] = 0;
		}
	}
}
#endif

/*
 * This erases the file selector from the screen.
 */
void SFSelect::eraseCDKFselect(/*CDKOBJS *object*/)
{
	//   if (validCDKObject(object)) {
	//      SFSelect *fselect =(SFSelect *)object;
	scrollField->eraseCDKScroll(/*fselect->scrollField*/);
	entryField->eraseCDKEntry(/*fselect->entryField*/);
	eraseCursesWindow(/*fselect->*/win);
	//   }
}

/*
 * This function sets the information inside the file selector.
 */
void SFSelect::setCDKFselect(/*SFSelect *fselect,*/
		    const char *directory,
		    chtype fieldAttrib,
		    chtype pfiller,
		    chtype phighlight,
		    const char *dirAttribute,
		    const char *fileAttribute,
		    const char *linkAttribute,
		    const char *sockAttribute,
		    bool Box GCC_UNUSED)
{
	/* *INDENT-EQLS* */
	SScroll *fscroll   = this->scrollField;
	SEntry *fentry     = this->entryField;
	const char *tempDir        = 0;
	/* Keep the info sent to us. */
	this->fieldAttribute = fieldAttrib;
	this->fillerCharacter = pfiller;
	this->highlight = phighlight;
	/* Set the attributes of the entry field/scrolling list. */
	//   setCDKEntryFillerChar(fentry, filler);
	fentry->filler=pfiller;
	//   setCDKScrollHighlight(fscroll, phighlight);
	fscroll->highlight=phighlight;

	/* Only do the directory stuff if the directory is not null. */
	if (directory) {
#ifdef pneu
		string newDirectory;
#else
		const char *newDirectory;
#endif

		/* Try to expand the directory if it starts with a ~ */
		if ((tempDir = expandTilde(directory))) {
			newDirectory = tempDir;
		} else {
#ifdef pneu
			newDirectory=directory;
#else
			newDirectory = copyChar(directory);
#endif
		}

		/* Change directories. */
#ifdef pneu
		if (chdir(newDirectory.c_str())) {
			vector<string> mesg(4);
#else
		if (chdir(newDirectory)) {
			char *mesg[4];
#endif
			Beep();

			/* Could not get into the directory, pop up a little message. */
			mesg[0] = format1String("<C>Could not change into %s", newDirectory
#ifdef pneu
					.c_str()
#else
#endif
					);
			mesg[1] = errorMessage("<C></U>%s");
#ifdef pneu
			mesg[2]=" ";
			mesg[3]="<C>Press Any Key To Continue.";
#else
			mesg[2] = copyChar(" ");
			mesg[3] = copyChar("<C>Press Any Key To Continue.");
#endif

			/* Pop Up a message. */
			screen->popupLabel(/*ScreenOf(this), */
#ifdef pneu
					mesg
#else
					(CDK_CSTRING2)mesg, 4
#endif
					);

			/* Clean up some memory. */
#ifdef pneu
#else
			freeCharList(mesg, 4);
#endif

			/* Get out of here. */
			eraseCDKFselect(/*this*/);
			drawCDKFselect(/*this, ObjOf(this)->*/obbox);
#ifdef pneu
#else
			freeChecked((void*)newDirectory);
#endif
			return;
		}
#ifdef pneu
#else
		freeChecked((void*)newDirectory);
#endif
	}

	/*
	 * If the information coming in is the same as the information
	 * that is already there, there is no need to destroy it.
	 */
	if (this->pwd != directory) {
		setPWD(/*this*/);
	}
#ifdef pneu
	this->fileAttribute=fileAttribute;
	this->dirAttribute=dirAttribute;
	this->linkAttribute=linkAttribute;
	this->sockAttribute=sockAttribute;
#else
	if (this->fileAttribute != fileAttribute) {
		/* Remove the old pointer and set the new value. */
		freeChecked(this->fileAttribute);
		this->fileAttribute = copyChar(fileAttribute);
	}
	if (this->dirAttribute != dirAttribute) {
		/* Remove the old pointer and set the new value. */
		freeChecked(this->dirAttribute);
		this->dirAttribute = copyChar(dirAttribute);
	}
	if (this->linkAttribute != linkAttribute) {
		/* Remove the old pointer and set the new value. */
		freeChecked(this->linkAttribute);
		this->linkAttribute = copyChar(linkAttribute);
	}
	if (this->sockAttribute != sockAttribute) {
		/* Remove the old pointer and set the new value. */
		freeChecked(this->sockAttribute);
		this->sockAttribute = copyChar(sockAttribute);
	}
#endif

	/* Set the contents of the entry field. */
	fentry->setCDKEntryValue(/*fentry, */this->pwd
#ifdef pneu
			.c_str()
#else
#endif
			);
	fentry->drawCDKEntry(/*fentry, ObjOf(fentry)->*/obbox);

	/* Get the directory contents. */
	if (setCDKFselectdirContents(/*this*/) == 0) {
		Beep();
		return;
	}

	/* Set the values in the scrolling list. */
	fscroll->setCDKScrollItems(/*fscroll,*/
#ifdef pneu
			 								&dirContents,
#else
			(CDK_CSTRING2)this->dirContents,
			this->fileCounter,
#endif
			FALSE);
}

/*
 * Return the plain string that corresponds to an item in dirContents[].
 */
const char *SFSelect::contentToPath(/*SFSelect *fselect, */const char *content)
{
   char *tempChar;
   int j, j2;

//   chtype *tempChtype = char2Chtypeh(content, &j, &j2);
	 chtstr tempChtype(content,&j,&j2);
#ifdef pneu
   tempChar = tempChtype.chtype2Char();
#else
   tempChar = chtype2Char(tempChtype.getinh());
#endif
   trim1Char(tempChar);	/* trim the 'mode' stored on the end */

   /* Create the pathname. */
   const char *result = make_pathname(this->pwd,tempChar)
#ifdef pneu
			 .c_str()
#else
#endif
			 ;

   /* Clean up. */
//   freeChtype(tempChtype);
#ifdef pneu
#else
   freeChecked(tempChar);
#endif
   return result;
}


/*
 * This tries to complete the filename.
 */
static int completeFilenameCB(EObjectType objectType GCC_UNUSED,
			       void *object GCC_UNUSED,
			       void *clientData,
			       chtype key GCC_UNUSED)
{
	/* *INDENT-EQLS* */
	SFSelect *fselect  = (SFSelect *)clientData;
	SScroll *scrollp   = fselect->scrollField;
	SEntry *entry      = fselect->entryField;
#ifdef pneu
	string filename(entry->efld);
	string mydirname      = dirName(filename);
#else
	const char *filename       = copyChar(entry->efld);
	char *mydirname      = dirName(filename);
	size_t filenameLen   = 0;
#endif
	const char *newFilename    = 0;
	int isDirectory;
#ifdef pneu
	vector<string> plist;
#else
	char **list;
#endif

	/* Make sure the filename is not null/empty. */
#ifdef pneu
	size_t filenameLen{filename.length()};
	if (filename.empty()) {
#else
	if (filename == 0 || !(filenameLen = strlen(filename))) {
#endif
		Beep();
#ifdef pneu
#else
		freeChecked((void*)filename);
		freeChecked(mydirname);
#endif
		return (TRUE);
	}

	/* Try to expand the filename if it starts with a ~ */
#ifdef pneu
	if ((newFilename = expandTilde(filename.c_str()))) {
#else
	if ((newFilename = expandTilde(filename))) {
		freeChecked((void*)filename);
#endif
		filename = newFilename;
#ifdef pneu
		entry->setCDKEntryValue(filename.c_str());
#else
		entry->setCDKEntryValue(filename);
#endif
		entry->drawCDKEntry(entry->obbox);
	}

	/* Make sure we can change into the directory. */
#ifdef pneu
	isDirectory = chdir(filename.c_str());
	if (chdir(fselect->pwd.c_str())) {
#else
	isDirectory = chdir(filename);
	if (chdir(fselect->pwd)) {
		freeChecked((void*)filename);
		freeChecked(mydirname);
#endif
		return FALSE;
	}
	fselect->setCDKFselect(/*fselect,*/
			(isDirectory ? mydirname : filename)
#ifdef pneu
			.c_str()
#else
#endif
			,
			fselect->fieldAttribute,
			fselect->fillerCharacter,
			fselect->highlight,
			fselect->dirAttribute
#ifdef pneu
			.c_str()
#else
#endif
			,
			fselect->fileAttribute
#ifdef pneu
			.c_str()
#else
#endif
			,
			fselect->linkAttribute
#ifdef pneu
			.c_str()
#else
#endif
			,
			fselect->sockAttribute
#ifdef pneu
			.c_str()
#else
#endif
			,
			ObjOf(fselect)->obbox);
#ifdef pneu
#else
	freeChecked(mydirname);
#endif

	/* If we can, change into the directory. */
	if (isDirectory) {
		/*
		 * Set the entry field with the filename so the current
		 * filename selection shows up.
		 */
#ifdef pneu
		entry->setCDKEntryValue(/*entry, */filename.c_str());
#else
		entry->setCDKEntryValue(/*entry, */filename);
#endif
		entry->drawCDKEntry(/*entry, ObjOf(entry)->*/entry->obbox);
	}

	/* Create the file list. */
	int x;
#ifdef pneu
		for (x = 0; x < fselect->dirContents.size(); x++) {
			plist.push_back(fselect->contentToPath(/*fselect,*/fselect->dirContents[x].c_str()));
#else
	if ((list = typeMallocN(char *, fselect->fileCounter))) {
		for (x = 0; x < fselect->fileCounter; x++) {
			list[x] = (char*)fselect->contentToPath(/*fselect, */fselect->dirContents[x]);
#endif
		}

		int Index;
		/* Look for a unique filename match. */
		Index = searchList(
#ifdef pneu
		&plist,
				filename.c_str());
#else
				(CDK_CSTRING2)list, fselect->fileCounter, 
				filename);
#endif
		/* If the index is less than zero, return we didn't find a match. */
		if (Index < 0) {
			Beep();
		} else {
			/* Move to the current item in the scrolling list. */
			int difference = Index - scrollp->currentItem;
			int absoluteDifference = abs(difference);
			if (difference < 0) {
				for (x = 0; x < absoluteDifference; x++) {
					fselect->injectMyScroller(/*fselect, */KEY_UP);
				}
			} else if (difference > 0) {
				for (x = 0; x < absoluteDifference; x++) {
					fselect->injectMyScroller(/*fselect, */KEY_DOWN);
				}
			}
			fselect->drawMyScroller(/*fselect*/);

			/* Ok, we found a match, is the next item similar? */
			if (Index + 1 < fselect->
#ifdef pneu
					dirContents.size()
#else
					fileCounter 
#endif
#ifdef pneu
					&& !plist[Index + 1].empty()
					&& plist[Index + 1]==filename
#else
					&& list[Index + 1]
					&& !strncmp(list[Index + 1], filename, filenameLen)
#endif
						) {
				int currentIndex = Index;
				int baseChars =(int)filenameLen;
				int matches = 0;

				/* Determine the number of files which match. */
				while (currentIndex < fselect->
#ifdef pneu
					dirContents.size()
#else
						fileCounter
#endif
						) {
#ifdef pneu
					if (!plist[currentIndex].empty()) {
					  if (filename==plist[currentIndex]) {
#else
					if (list[currentIndex]) {
						if (!strncmp(list[currentIndex], filename, filenameLen)) {
#endif
							matches++;
						}
					}
					currentIndex++;
				}
				/* Start looking for the common base characters. */
				for (;;) {
					int secondaryMatches = 0;
					for (x = Index; x < Index + matches; x++) {
#ifdef pneu
						if (plist[Index][baseChars] == plist[x][baseChars]) {
#else
						if (list[Index][baseChars] == list[x][baseChars]) {
#endif
							secondaryMatches++;
						}
					}
					if (secondaryMatches != matches) {
						Beep();
						break;
					}
					/* Inject the character into the entry field. */
#ifdef pneu
					fselect->entryField->injectCDKEntry(/*fselect->entryField,*/(chtype)plist[Index][baseChars]);
#else
					fselect->entryField->injectCDKEntry(/*fselect->entryField,*/(chtype)list[Index][baseChars]);
#endif
					baseChars++;
				}
			} else {
				/* Set the entry field with the found item. */
#ifdef pneu
				entry->setCDKEntryValue(/*entry, */plist[Index].c_str());
#else
				entry->setCDKEntryValue(/*entry, */list[Index]);
#endif
				entry->drawCDKEntry(/*entry, ObjOf(entry)->*/entry->obbox);
			}
		}
#ifdef pneu
#else
		freeCharList(list,(unsigned)fselect->fileCounter);
		free(list);
	}
	freeChecked((void*)filename);
#endif
	return(TRUE);
}

/*
 * This function takes a mode_t type and creates a string represntation
 * of the permission mode.
 */
int mode2Char(char *string, mode_t mode)
{
	/* *INDENT-OFF* */
	static struct {
		mode_t	mask;
		unsigned	col;
		char	flag;
	} table[] = {
		{ S_IRUSR,	1,	'r' },
		{ S_IWUSR,	2,	'w' },
		{ S_IXUSR,	3,	'x' },
#if defined(S_IRGRP) && defined(S_IWGRP) && defined(S_IXGRP)
		{ S_IRGRP,	4,	'r' },
		{ S_IWGRP,	5,	'w' },
		{ S_IXGRP,	6,	'x' },
#endif
#if defined(S_IROTH) && defined(S_IWOTH) && defined(S_IXOTH)
		{ S_IROTH,	7,	'r' },
		{ S_IWOTH,	8,	'w' },
		{ S_IXOTH,	9,	'x' },
#endif
#ifdef S_ISUID
		{ S_ISUID,	3,	's' },
#endif
#ifdef S_ISGID
		{ S_ISGID,	6,	's' },
#endif
#ifdef S_ISVTX
		{ S_ISVTX,	9,	't' },
#endif
	};
	/* *INDENT-ON* */

	/* Declare local variables.  */
	int permissions = 0;
	int filetype = mode2Filetype(mode);
	unsigned n;

	/* Clean the string.  */
	cleanChar(string, 11, '-');
	string[11] = '\0';

	if (filetype == '?')
		return -1;

	for (n = 0; n < sizeof(table) / sizeof(table[0]); n++) {
		if ((mode & table[n].mask)) {
			string[table[n].col] = table[n].flag;
			permissions |= (int)table[n].mask;
		}
	}

	/* Check for unusual permissions.  */
#ifdef S_ISUID
	if (((mode & S_IXUSR) == 0) &&
			((mode & S_IXGRP) == 0) &&
			((mode & S_IXOTH) == 0) &&
			(mode & S_ISUID))
	{
		string[3] = 'S';
	}
#endif

	return permissions;
}

/*
 * This is a callback to the scrolling list which displays information
 * about the current file. (and the whole directory as well)
 */
static int displayFileInfoCB(EObjectType objectType GCC_UNUSED,
			      void *object,
			      void *clientData,
			      chtype key GCC_UNUSED)
{
	SEntry *entry =(SEntry *)object;
	SFSelect *fselect =(SFSelect *)clientData;
	SLabel *infoLabel;
	struct stat fileStat;
#ifdef HAVE_PWD_H
	struct passwd *pwEnt;
	struct group *grEnt;
#endif
	const char *filetype;
#ifdef pneu
	string filename;
	vector<string> mesg(9);
#else
	const char *filename;
	char *mesg[9];
#endif
	char stringMode[15];
	int intMode;
	bool functionKey;
#ifdef ineu
#ifdef pneu
	filename = fselect->entryField->efld;
#else
	filename = fselect->entryField->efld.c_str();
#endif
#else
	filename = fselect->entryField->efld;
#endif
#ifdef pneu
	if (!lstat(filename.c_str(), &fileStat)) {
#else
	if (!lstat(filename, &fileStat)) {
#endif
		switch (mode2Filetype(fileStat.st_mode)) {
			case 'l':
				filetype = "Symbolic Link";
				break;
			case '@':
				filetype = "Socket";
				break;
			case '-':
				filetype = "Regular File";
				break;
			case 'd':
				filetype = "Directory";
				break;
			case 'c':
				filetype = "Character Device";
				break;
			case 'b':
				filetype = "Block Device";
				break;
			case '&':
				filetype = "FIFO Device";
				break;
			default:
				filetype = "Unknown";
				break;
		}
	} else {
		filetype = "Unknown";
	}
	/* Get the user name and group name. */
#ifdef HAVE_PWD_H
	pwEnt = getpwuid(fileStat.st_uid);
	grEnt = getgrgid(fileStat.st_gid);
#endif
	/* Convert the mode_t type to both string and int. */
	intMode = mode2Char(stringMode, fileStat.st_mode);
	/* Create the message. */
	mesg[0] = format1String("Directory  : </U>%s", fselect->pwd
#ifdef pneu
			.c_str()
#else
#endif
			);
	mesg[1] = format1String("Filename   : </U>%s", filename
#ifdef pneu
	.c_str()
#else
#endif
	 );
#ifdef HAVE_PWD_H
	mesg[2] = format1StrVal("Owner      : </U>%s<!U> (%d)",
			pwEnt->pw_name,
			(int)fileStat.st_uid);
	mesg[3] = format1StrVal("Group      : </U>%s<!U> (%d)",
			grEnt->gr_name,
			(int)fileStat.st_gid);
#else
	mesg[2] = format1Number("Owner      : (%ld)", (long)fileStat.st_uid);
	mesg[3] = format1Number("Group      : (%ld)", (long)fileStat.st_gid);
#endif
	mesg[4] = format1StrVal("Permissions: </U>%s<!U> (%o)", stringMode, intMode);
	mesg[5] = format1Number("Size       : </U>%ld<!U> bytes",(long)fileStat.st_size);
	mesg[6] = format1Date("Last Access: </U>%s", fileStat.st_atime);
	mesg[7] = format1Date("Last Change: </U>%s", fileStat.st_ctime);
	mesg[8] = format1String("File Type  : </U>%s", filetype);
	/* Create the pop up label. */
	infoLabel = new SLabel(entry->/*obj.*/screen,
			CENTER, CENTER,
#ifdef pneu
			mesg,
#else
			 (CDK_CSTRING2)mesg,9, 
#endif
			TRUE, FALSE);
	infoLabel->drawCDKLabel(/*infoLabel, */TRUE);
	infoLabel->getchCDKObject(/*ObjOf(infoLabel), */&functionKey);
	/* Clean up some memory. */
	infoLabel->destroyCDKLabel(/*infoLabel*/);
#ifdef pneu
#else
	freeCharList(mesg, 9);
#endif
	/* Redraw the file selector. */
	fselect->drawCDKFselect(/*fselect, ObjOf(fselect)->*/fselect->obbox);
	return (TRUE);
}


/*
 * This creates a file selection widget.
 */
//SFSelect *newCDKFselect(
SFSelect::SFSelect(
		SScreen *cdkscreen,
		int xplace,
		int yplace,
		int height,
		int width,
		const char *title,
		const char *label,
		chtype fieldAttribute,
		chtype fillerChar,
		chtype phighlight,
		const char *dAttribute,
		const char *fAttribute,
		const char *lAttribute,
		const char *sAttribute,
		bool Box,
		bool shadow,
		int highnr)
{
	cdktype = vFSELECT;
	/* *INDENT-EQLS* */
//	SFSelect *fselect  = 0;
	int parentWidth      = getmaxx(cdkscreen->window);
	int parentHeight     = getmaxy(cdkscreen->window);
	int boxWidth;
	int boxHeight;
	int xpos             = xplace;
	int ypos             = yplace;
	int tempWidth        = 0;
	int tempHeight       = 0;
	int labelLen, junk;
	int x;
	/* *INDENT-OFF* */
	static const struct
	{
		int from;
		int to;
	} bindings[] =
	{
		{ CDK_BACKCHAR,	KEY_PPAGE },
		{ CDK_FORCHAR,	KEY_NPAGE },
	};
	/* *INDENT-ON* */

//	if ((fselect = newCDKObject(SFSelect, &my_funcs)) == 0) return (0);
	::CDKOBJS();
	setBox(Box);

	/*
	 * If the height is a negative value, the height will
	 * be ROWS-height, otherwise, the height will be the
	 * given height.
	 */
	boxHeight = setWidgetDimension(parentHeight, height, 0);

	/*
	 * If the width is a negative value, the width will
	 * be COLS-width, otherwise, the width will be the
	 * given width.
	 */
	boxWidth = setWidgetDimension(parentWidth, width, 0);

	/* Rejustify the x and y positions if we need to. */
	alignxy(cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

	/* Make sure the box isn't too small. */
	boxWidth =(boxWidth < 15 ? 15 : boxWidth);
	boxHeight =(boxHeight < 6 ? 6 : boxHeight);

	/* Make the file selector window. */
	this->win = newwin(boxHeight, boxWidth, ypos, xpos);

	/* Is the window null? */
	if (!this->win) {
		destroyCDKObject();
		return;
	}
	keypad(this->win, TRUE);

	/* *INDENT-EQLS* Set some variables. */
	ScreenOf(this)           = cdkscreen;
	this->parent              = cdkscreen->window;
#ifdef pneu
	this->dirAttribute				=	dAttribute;
	this->fileAttribute       = fAttribute;
	this->linkAttribute				= lAttribute;
	this->sockAttribute				= sAttribute;
#else
	this->dirAttribute        = copyChar(dAttribute);
	this->fileAttribute       = copyChar(fAttribute);
	this->linkAttribute       = copyChar(lAttribute);
	this->sockAttribute       = copyChar(sAttribute);
#endif
	this->highlight           = phighlight;
	this->fillerCharacter     = fillerChar;
	this->fieldAttribute      = fieldAttribute;
	this->boxHeight           = boxHeight;
	this->boxWidth            = boxWidth;
#ifdef pneu
#else
	this->fileCounter         = 0;
	this->pwd                 = 0;
#endif
	initExitType(this);
	ObjOf(this)->inputWindow = this->win;
	this->shadow              = shadow;
	this->shadowWin           = 0;

   /* Get the present working directory. */
   setPWD();

   /* Get the contents of the current directory. */
   setCDKFselectdirContents();

   /* Create the entry field in the selector. */
//	 chtype *chtypeString= char2Chtypeh(label, &labelLen, &junk);
	 chtstr chtypeString(label,&labelLen,&junk);
//   freeChtype(chtypeString);
   tempWidth = (isFullWidth(width)
		? FULL
		: boxWidth - 2 - labelLen);
   this->entryField = new SEntry(cdkscreen,
				      getbegx(this->win),
				      getbegy(this->win),
				      title, label,
				      fieldAttribute, fillerChar,
				      vMIXED, tempWidth, 0, 512,
				      Box, FALSE,highnr);

   /* Make sure the widget was created. */
   if (!this->entryField) {
      destroyCDKObject();
      return ;
   }

   /* Set the lower left/right characters of the entry field. */
	 entryField->LLChar=ACS_LTEE;
   //setCDKEntryLLChar(this->entryField, ACS_LTEE);
	 entryField->LRChar=ACS_RTEE;
   //setCDKEntryLRChar(this->entryField, ACS_RTEE);

   /* Define the callbacks for the entry field. */
   entryField->bindCDKObject(/*vENTRY,
		  this->entryField,*/
		  KEY_UP,
		  fselectAdjustScrollCB,
		  this);
   entryField->bindCDKObject(/*vENTRY,
		  this->entryField,*/
		  KEY_PPAGE,
		  fselectAdjustScrollCB,
		  this);
   entryField->bindCDKObject(/*vENTRY,
		  this->entryField,*/
		  KEY_DOWN,
		  fselectAdjustScrollCB,
		  this);
   entryField->bindCDKObject(/*vENTRY,
		  this->entryField, */
		  KEY_NPAGE,
		  fselectAdjustScrollCB,
		  this);
   entryField->bindCDKObject(/*vENTRY,
		  this->entryField,*/
		  KEY_TAB,
		  completeFilenameCB,
		  this);
   entryField->bindCDKObject(/*vENTRY,
		  this->entryField,*/
		  CTRL('^'),
		  displayFileInfoCB,
		  this);

   /* Put the current working directory in the entry field. */
   entryField->setCDKEntryValue(/*this->entryField, */this->pwd
#ifdef pneu
			 .c_str()
#else
#endif
			 );

   /* Create the scrolling list in the selector. */
   tempHeight = getmaxy(this->entryField->win) - borderSize;
   tempWidth = (isFullWidth(width)
		? FULL
		: boxWidth - 1);
   this->scrollField = new SScroll(cdkscreen,
					getbegx(this->win),
					getbegy(this->win) + tempHeight,
					RIGHT,
					boxHeight - tempHeight,
					tempWidth,
					0,
#ifdef pneu
					&this->dirContents,
#else
					(CDK_CSTRING2)this->dirContents,
					this->fileCounter,
#endif
					NONUMBERS, this->highlight,
					Box, FALSE);

   /* Set the lower left/right characters of the entry field. */
//   scrollField->setCDKScrollULChar(/*this->scrollField, */ACS_LTEE);
   ULChar=ACS_LTEE;
//   scrollField->setCDKScrollURChar(/*this->scrollField, */ACS_RTEE);
	 URChar=ACS_RTEE;

   /* Do we want a shadow? */
   if (shadow) {
      this->shadowWin = newwin(boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Setup the key bindings. */
   for (x = 0; x <(int)SIZEOF(bindings); ++x)
      bindCDKObject(/*vFSELECT,
		     this, */
		    (chtype)bindings[x].from,
		     getcCDKBind,
		    (void *)(long)bindings[x].to);

   registerCDKObject(cdkscreen, vFSELECT/*, this*/);

//   return (this);
}


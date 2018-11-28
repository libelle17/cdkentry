#include "cdkp.h"
#include <vector>
#include <cctype> // isdigit
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

int getmaxxf(WINDOW *win)
{
	int y, x;
	getmaxyx (win, y, x);
	return x;
}

int getmaxyf(WINDOW *win)
{
	int y, x;
	getmaxyx (win, y, x);
	return y;
}

/*
 * If the dimension is a negative value, the dimension will be the full
 * height/width of the parent window - the value of the dimension.  Otherwise,
 * the dimension will be the given value.
 */
int setWidgetDimension (int parentDim, int proposedDim, int adjustment)
{
	int dimension = 0;
	/* If the user passed in FULL, return the parent's size. */
	if ((proposedDim == FULL) || (proposedDim == 0)) {
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

static int encodeAttribute (const char *string, int from, chtype *mask)
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
	if (*mask != 0) {
		from++;
	} else {
		int digits;
		pair = 0;
		for (digits = 1; digits <= 3; ++digits) {
			if (!isdigit (CharOf (string[1 + from])))
				break;
			pair *= 10;
			pair += DigitOf (string[++from]);
		}
#ifdef HAVE_START_COLOR
#define MAX_PAIR (int) (A_COLOR / (((~A_COLOR) << 1) & A_COLOR))
		if (pair > MAX_PAIR)
			pair = MAX_PAIR;
		*mask = (chtype)COLOR_PAIR (pair);
#else
		*mask = A_BOLD;
#endif
	}
	return from;
} // static int encodeAttribute (const char *string, int from, chtype *mask)

/*
 * This function takes a character string, full of format markers
 * and translates them into a chtype * array. This is better suited
 * to curses, because curses uses chtype almost exclusively
 */
// highinr G.Schade 26.9.18
chtype *char2Chtypeh(const char *string, int *to, int *align, int highinr/*=0*/)
{
	chtype *result = 0;
	chtype attrib;
	chtype lastChar;
	chtype mask;

	(*to) = 0;
	*align = LEFT;

	if (string != 0 && *string != 0)
	{
		int len = (int)strlen(string);
		int pass;
		int used = 0;
		/*
		 * We make two passes because we may have indents and tabs to expand, and
		 * do not know in advance how large the result will be.
		 */
		for (pass = 0; pass < 2; pass++)
		{
			int insideMarker;
			int from;
			int adjust;
			int start;
			int x = 3;

			if (pass != 0)
			{
				if ((result = typeMallocN (chtype, used + 2)) == 0)
				{
					used = 0;
					break;
				}
			}
			adjust = 0;
			attrib = A_NORMAL;
			start = 0;
			used = 0;

			/* Look for an alignment marker.  */
			if (*string == L_MARKER)
			{
				if (string[1] == 'C' && string[2] == R_MARKER)
				{
					(*align) = CENTER;
					start = 3;
				}
				else if (string[1] == 'R' && string[2] == R_MARKER)
				{
					(*align) = RIGHT;
					start = 3;
				}
				else if (string[1] == 'L' && string[2] == R_MARKER)
				{
					start = 3;
				}
				else if (string[1] == 'B' && string[2] == '=')
				{
					/* Set the item index value in the string.       */
					if (result != 0)
					{
						result[0] = ' ';
						result[1] = ' ';
						result[2] = ' ';
					}

					/* Pull out the bullet marker.  */
					while (string[x] != R_MARKER && string[x] != 0)
					{
						if (result != 0)
							result[x] = (chtype)string[x] | A_BOLD;
						x++;
					}
					adjust = 1;

					/* Set the alignment variables.  */
					start = x;
					used = x;
				}
				else if (string[1] == 'I' && string[2] == '=')
				{
					from = 2;
					x = 0;

					while (string[++from] != R_MARKER && string[from] != 0)
					{
						if (isdigit (CharOf (string[from])))
						{
							adjust = (adjust * 10) + DigitOf (string[from]);
							x++;
						}
					}

					start = x + 4;
				}
			}

			while (adjust-- > 0)
			{
				if (result != 0)
					result[used] = ' ';
				used++;
			}

			/* Set the format marker boolean to false.  */
			insideMarker = FALSE;
// GSchade 25.9.18
			//size_t pos=0;
			/* Start parsing the character string.  */
			for (from = start; from < len; from++)
			{
				/* Are we inside a format marker?  */
				if (!insideMarker)
				{
					if (string[from] == L_MARKER
							&& (string[from + 1] == '/'
								|| string[from + 1] == '!'
								|| string[from + 1] == '#'))
					{
						insideMarker = TRUE;
					}
					else if (string[from] == '\\' && string[from + 1] == L_MARKER)
					{
						from++;
						if (result != 0)
							result[used] = CharOf (string[from]) | attrib;
						used++;
						from++;
					}
					else if (string[from] == '\t')
					{
						do
						{
							if (result != 0)
								result[used] = ' ';
							used++;
						}
						while (used & 7);
					} // else if (result && !strchr("ö",string[from])) { result[used++]=CharOf('o')|attrib;
					// GSchade 25.9.18
					/*
					else if (strchr("äöüÄÖÜß",string[from])) {
						printf("from: %i, string[from]: %i\n",from,(int)string[from]+256);
						//if (result) result[used]=CharOf('z')|attrib; used++;
						if (result) result[used]=CharOf(-61)|attrib; used++;
						if (!strchr("ä",string[from])) { if (result) result[used]=CharOf(164-256)|attrib; used++; }
						else if (!strchr("ö",string[from])) { if (result) result[used]=CharOf(182-256)|attrib; used++; }
						else if (!strchr("ü",string[from])) { if (result) result[used]=CharOf(188-256)|attrib; used++; }
						else if (!strchr("Ä",string[from])) { if (result) result[used]=CharOf(132)|attrib; used++; }
						else if (!strchr("Ö",string[from])) { if (result) result[used]=CharOf(150)|attrib; used++; }
						else if (!strchr("Ü",string[from])) { if (result) result[used]=CharOf(156)|attrib; used++; }
						else if (!strchr("ß",string[from])) { if (result) result[used]=CharOf(159)|attrib; used++; }
					}
				*/
					// Ende GSchade 25.9.18
					else
					{
						if (result != 0) {
							// GSchade 26.9.18
							if (used==highinr-1) {
								result[used] = CharOf (string[from]) | attrib|COLOR_PAIR(1);
							} else {
								// Ende GSchade 
							result[used] = CharOf (string[from]) | attrib;
							}
						}
						used++;
					}
				}
				else
				{
					switch (string[from])
					{
						case R_MARKER:
							insideMarker = 0;
							break;
						case '#':
							{
								lastChar = 0;
								switch (string[from + 2])
								{
									case 'L':
										switch (string[from + 1])
										{
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
										switch (string[from + 1])
										{
											case 'L':
												lastChar = ACS_LRCORNER;
												break;
											case 'U':
												lastChar = ACS_URCORNER;
												break;
										}
										break;
									case 'T':
										switch (string[from + 1])
										{
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
										switch (string[from + 1])
										{
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

								if (lastChar != 0)
								{
									adjust = 1;
									from += 2;

									if (string[from + 1] == '(')
										/* Check for a possible numeric modifier.  */
									{
										from++;
										adjust = 0;

										while (string[++from] != ')' && string[from] != 0)
										{
											if (isdigit (CharOf (string[from])))
											{
												adjust = (adjust * 10) + DigitOf (string[from]);
											}
										}
									}
								}
								for (x = 0; x < adjust; x++)
								{
									if (result != 0)
										result[used] = lastChar | attrib;
									used++;
								}
								break;
							}
						case '/':
							from = encodeAttribute (string, from, &mask);
							attrib = attrib | mask;
							break;
						case '!':
							from = encodeAttribute (string, from, &mask);
							attrib = attrib & ~mask;
							break;
					}
				}
			}

			if (result != 0)
			{
				result[used] = 0;
				result[used + 1] = 0;
			}

			/*
			 * If there are no characters, put the attribute into the
			 * the first character of the array.
			 */
			if (used == 0
					&& result != 0)
			{
				result[0] = attrib;
			}
		}
		*to = used;
	}
	else
	{
		/*
		 * Try always to return something; otherwise lists of chtype strings
		 * would get a spurious null pointer whenever there is a blank line,
		 * and CDKfreeChtypes() would fail to free the whole list.
		 */
		result = typeCallocN (chtype, 1);
	}
	return result;
}

/*
 * Split a string into a list of strings.
 */
char **CDKsplitString (const char *string, int separator)
{
	char **result = 0;
	char *temp;

	if (string != 0 && *string != 0) {
		unsigned need = countChar (string, separator) + 2;
		if ((result = typeMallocN (char *, need)) != 0) {
			unsigned item = 0;
			const char *first = string;
			for (;;) {
				while (*string != 0 && *string != separator)
					string++;

				need = (unsigned)(string - first);
				if ((temp = typeMallocN (char, need + 1)) == 0)
					break;

				memcpy (temp, first, need);
				temp[need] = 0;
				result[item++] = temp;

				if (*string++ == 0)
					break;
				first = string;
			}
			result[item] = 0;
		}
	}
	return result;
}

static unsigned countChar (const char *string, int separator)
{
	unsigned result = 0;
	int ch;
	while ((ch = *string++) != 0) {
		if (ch == separator)
			result++;
	}
	return result;
}

/*
 * Count the number of items in a list of strings.
 */
unsigned CDKcountStrings (CDK_CSTRING2 list)
{
	unsigned result = 0;
	if (list != 0)
	{
		while (*list++ != 0)
			result++;
	}
	return result;
}

/*
 * This function takes a character string, full of format markers
 * and translates them into a chtype * array. This is better suited
 * to curses, because curses uses chtype almost exclusively
 */
chtype *char2Chtype (const char *string, int *to, int *align)
{
	chtype *result = 0;
	chtype attrib;
	chtype lastChar;
	chtype mask;

	(*to) = 0;
	*align = LEFT;

	if (string != 0 && *string != 0)
	{
		int len = (int)strlen (string);
		int pass;
		int used = 0;
		/*
		 * We make two passes because we may have indents and tabs to expand, and
		 * do not know in advance how large the result will be.
		 */
		for (pass = 0; pass < 2; pass++)
		{
			int insideMarker;
			int from;
			int adjust;
			int start;
			int x = 3;

			if (pass != 0)
			{
				if ((result = typeMallocN (chtype, used + 2)) == 0)
				{
					used = 0;
					break;
				}
			}
			adjust = 0;
			attrib = A_NORMAL;
			start = 0;
			used = 0;

			/* Look for an alignment marker.  */
			if (*string == L_MARKER)
			{
				if (string[1] == 'C' && string[2] == R_MARKER)
				{
					(*align) = CENTER;
					start = 3;
				}
				else if (string[1] == 'R' && string[2] == R_MARKER)
				{
					(*align) = RIGHT;
					start = 3;
				}
				else if (string[1] == 'L' && string[2] == R_MARKER)
				{
					start = 3;
				}
				else if (string[1] == 'B' && string[2] == '=')
				{
					/* Set the item index value in the string.       */
					if (result != 0)
					{
						result[0] = ' ';
						result[1] = ' ';
						result[2] = ' ';
					}

					/* Pull out the bullet marker.  */
					while (string[x] != R_MARKER && string[x] != 0)
					{
						if (result != 0)
							result[x] = (chtype)string[x] | A_BOLD;
						x++;
					}
					adjust = 1;

					/* Set the alignment variables.  */
					start = x;
					used = x;
				}
				else if (string[1] == 'I' && string[2] == '=')
				{
					from = 2;
					x = 0;

					while (string[++from] != R_MARKER && string[from] != 0)
					{
						if (isdigit (CharOf (string[from])))
						{
							adjust = (adjust * 10) + DigitOf (string[from]);
							x++;
						}
					}

					start = x + 4;
				}
			}

			while (adjust-- > 0)
			{
				if (result != 0)
					result[used] = ' ';
				used++;
			}

			/* Set the format marker boolean to false.  */
			insideMarker = FALSE;

			/* Start parsing the character string.  */
			for (from = start; from < len; from++)
			{
				/* Are we inside a format marker?  */
				if (!insideMarker)
				{
					if (string[from] == L_MARKER
							&& (string[from + 1] == '/'
								|| string[from + 1] == '!'
								|| string[from + 1] == '#'))
					{
						insideMarker = TRUE;
					}
					else if (string[from] == '\\' && string[from + 1] == L_MARKER)
					{
						from++;
						if (result != 0)
							result[used] = CharOf (string[from]) | attrib;
						used++;
						from++;
					}
					else if (string[from] == '\t')
					{
						do
						{
							if (result != 0)
								result[used] = ' ';
							used++;
						}
						while (used & 7);
					}
					else
					{
						if (result != 0)
							result[used] = CharOf (string[from]) | attrib;
						used++;
					}
				}
				else
				{
					switch (string[from])
					{
						case R_MARKER:
							insideMarker = 0;
							break;
						case '#':
							{
								lastChar = 0;
								switch (string[from + 2])
								{
									case 'L':
										switch (string[from + 1])
										{
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
										switch (string[from + 1])
										{
											case 'L':
												lastChar = ACS_LRCORNER;
												break;
											case 'U':
												lastChar = ACS_URCORNER;
												break;
										}
										break;
									case 'T':
										switch (string[from + 1])
										{
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
										switch (string[from + 1])
										{
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

								if (lastChar != 0)
								{
									adjust = 1;
									from += 2;

									if (string[from + 1] == '(')
										/* Check for a possible numeric modifier.  */
									{
										from++;
										adjust = 0;

										while (string[++from] != ')' && string[from] != 0)
										{
											if (isdigit (CharOf (string[from])))
											{
												adjust = (adjust * 10) + DigitOf (string[from]);
											}
										}
									}
								}
								for (x = 0; x < adjust; x++)
								{
									if (result != 0)
										result[used] = lastChar | attrib;
									used++;
								}
								break;
							}
						case '/':
							from = encodeAttribute (string, from, &mask);
							attrib = attrib | mask;
							break;
						case '!':
							from = encodeAttribute (string, from, &mask);
							attrib = attrib & ~mask;
							break;
					}
				}
			}

			if (result != 0)
			{
				result[used] = 0;
				result[used + 1] = 0;
			}

			/*
			 * If there are no characters, put the attribute into the
			 * the first character of the array.
			 */
			if (used == 0
					&& result != 0)
			{
				result[0] = attrib;
			}
		}
		*to = used;
	}
	else
	{
		/*
		 * Try always to return something; otherwise lists of chtype strings
		 * would get a spurious null pointer whenever there is a blank line,
		 * and CDKfreeChtypes() would fail to free the whole list.
		 */
		result = typeCallocN (chtype, 1);
	}
	return result;
}

/*
 * This determines the length of a chtype string
 */
int chlen(const chtype *string)
{
	int result = 0;
	if (string != 0) {
		while (string[result] != 0)
			result++;
	}
	return (result);
}

void freeChtype (chtype *string)
{
	freeChecked (string);
}

/*
 * This takes a string, a field width and a justification type
 * and returns the adjustment to make, to fill
 * the justification requirement.
 */
int justifyString (int boxWidth, int mesgLength, int justify)
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

/*
 * Free a list of strings, terminated by a null pointer.
 */
void CDKfreeStrings (char **list)
{
	if (list != 0) {
		void *base = (void *)list;
		while (*list != 0)
			free (*list++);
		free (base);
	}
}

/*
 * Free a list of chtype-strings, terminated by a null pointer.
 */
void CDKfreeChtypes (chtype **list)
{
	if (list != 0)
	{
		void *base = (void *)list;
		while (*list != 0)
		{
			freeChtype (*list++);
		}
		free (base);
	}
}

/*
 * This takes an x and y position and realigns the values iff they sent in
 * values like CENTER, LEFT, RIGHT, ...
 */
void alignxy (WINDOW *window, int *xpos, int *ypos, int boxWidth, int boxHeight)
{
	int first, gap, last;

	first = getbegx (window);
	last = getmaxx (window);
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

	first = getbegy (window);
	last = getmaxy (window);
	if ((gap = (last - boxHeight)) < 0)
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
			(*ypos) = first + (gap / 2);
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
void cleanChar (char *s, int len, char character)
{
	if (s != 0) {
		int x;
		for (x = 0; x < len; x++) {
			s[x] = character;
		}
		s[--x] = '\0';
	}
}

/*
 * This registers a CDK object with a screen.
 */
void CDKOBJS::registerCDKObject(CDKSCREEN *screen, EObjectType cdktype)
{
	if (screen->objectCount + 1 >= screen->objectLimit) {
		screen->objectLimit += 2;
		screen->objectLimit *= 2;
		screen->object = typeReallocN (CDKOBJS *, screen->object, screen->objectLimit);
	}
	if (validObjType(cdktype)) {
		setScreenIndex(screen, screen->objectCount++);
	}
}

/*
 * Set indices so the screen and object point to each other.
 */
void CDKOBJS::setScreenIndex(CDKSCREEN *pscreen, int number)
{
	screenIndex = number;
	screen = pscreen;
	screen->object[number] = this;
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

CDKOBJS::~CDKOBJS()
{
	size_t pos{0};
	for(auto akt:all_objects) {
		if (akt==this) {
			all_objects.erase(all_objects.begin()+pos);
			break;
		}
		pos++;
	}
}

/*
 * Set the widget's title.
 */
int CDKOBJS::setCdkTitle (const char *titlec, int boxWidth)
{
	cleanCdkTitle ();
	if (titlec != 0) {
		char **temp = 0;
		int titleWidth;
		int x;
		int len;
		int align;

		/* We need to split the title on \n. */
		temp = CDKsplitString(titlec, '\n');
		titleLines = (int)CDKcountStrings((CDK_CSTRING2)temp);
		title = typeCallocN (chtype *, titleLines + 1);
		titlePos = typeCallocN (int, titleLines + 1);
		titleLen = typeCallocN (int, titleLines + 1);
		if (boxWidth >= 0) {
			int maxWidth = 0;
			/* We need to determine the widest title line. */
			for (x = 0; x < titleLines; x++) {
				chtype *holder = char2Chtype(temp[x], &len, &align);
				maxWidth = MAXIMUM (maxWidth, len);
				freeChtype(holder);
			}
			boxWidth = MAXIMUM (boxWidth, maxWidth + 2 * borderSize);
		} else {
			boxWidth = -(boxWidth - 1);
		}

		/* For each line in the title, convert from char * to chtype * */
		titleWidth = boxWidth - (2 * borderSize);
		for (x = 0; x < titleLines; x++) {
			title[x] = char2Chtype(temp[x], &titleLen[x], &titlePos[x]);
			titlePos[x] = justifyString(titleWidth, titleLen[x], titlePos[x]);
		}
		CDKfreeStrings (temp);
	}
	return boxWidth;
} // int CDKOBJS::setCdkTitle(const char *title, int boxWidth)

/*
 * Remove storage for the widget's title.
 */
void CDKOBJS::cleanCdkTitle()
{
	CDKfreeChtypes(title);
	title = 0;
	freeAndNull(titlePos);
	freeAndNull(titleLen);
	titleLines = 0;
}

bool CDKOBJS::validObjType(EObjectType type)
{
	bool valid = FALSE;
	//   if (obj != 0 && ObjTypeOf (obj) == type) {
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

void SEntry::CDKEntryCallBack(chtype character)
{
	SEntry::schreibl(character);
}


/*
 * This creates a pointer to an entry widget.
 */
SEntry::SEntry(CDKSCREEN *cdkscreen,
		int xplace,
		int yplace,
		const char *title,
		const char *labelp,
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
		)
{
	/* *INDENT-EQLS* */
	int parentWidth      = getmaxxf(cdkscreen->window);
	int parentHeight     = getmaxyf(cdkscreen->window);
	int fieldWidth       = fWidth;
	int boxWidth         = 0;
	int boxHeight;
	int xpos             = xplace;
	int ypos             = yplace;
	int junk             = 0;
	int horizontalAdjust, oldWidth;

	//	if ((entry = newCDKObject (CDKENTRY, &my_funcs)) == 0) return (0);
	::CDKOBJS();

	setCDKEntryBox(Box);
	boxHeight = (BorderOf (this) * 2) + 1;

	/*
	 * If the fieldWidth is a negative value, the fieldWidth will
	 * be COLS-fieldWidth, otherwise, the fieldWidth will be the
	 * given width.
	 */
	fieldWidth = setWidgetDimension(parentWidth, fieldWidth, 0);
	boxWidth = fieldWidth + 2 * BorderOf (this);

	/* Set some basic values of the entry field. */
	label = 0;
	labelLen = 0;
	labelWin = 0;
	labelumlz=0; // GSchade

	// GSchade

	/* Translate the label char *pointer to a chtype pointer. */
	if (labelp != 0) {
		label = char2Chtypeh(labelp, &labelLen, &junk
				// GSchade Anfang
				,highnr
				// GSchade Ende
				);
		// GSchade Anfang
		for(int i=0;label[i];i++) {
			if ((int)CharOf(label[i])==194 || (int)CharOf(label[i])==195) {
				labelumlz++;
			}
		}
		// GSchade Ende
		boxWidth += labelLen;
	}

	oldWidth = boxWidth;
	boxWidth = setCdkTitle (title, boxWidth);
	horizontalAdjust = (boxWidth - oldWidth) / 2;

	boxHeight += TitleLinesOf (this);

	/*
	 * Make sure we didn't extend beyond the dimensions of the window.
	 */
	boxWidth = MINIMUM (boxWidth, parentWidth);
	boxHeight = MINIMUM (boxHeight, parentHeight);
	fieldWidth = MINIMUM (fieldWidth,
			boxWidth - labelLen +labelumlz - 2 * BorderOf (this));

	/* Rejustify the x and y positions if we need to. */
	alignxy(cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

	/* Make the label window. */
	win = newwin (boxHeight, boxWidth, ypos, xpos);
	if (win == 0) {
		//		destroyCDKObject(this); return (0); 
	} else {
		keypad (win, TRUE);

		/* Make the field window. */
		fieldWin = subwin (win, 1, fieldWidth,
				(ypos + TitleLinesOf (this) + BorderOf (this)),
				(xpos + labelLen -labelumlz
				 + horizontalAdjust
				 + BorderOf (this)));
		if (fieldWin == 0) {
			//		destroyCDKObject (this); return (0);
		} else {
			keypad (fieldWin, TRUE);

			/* Make the label win, if we need to. */
			if (labelp != 0) {
				labelWin = subwin (win, 1, labelLen,
						ypos + TitleLinesOf (this) + BorderOf (this),
						xpos + horizontalAdjust + BorderOf (this));
			}

			/* Make room for the info char * pointer. */
			info = typeMallocN (char, maxp + 3);
			if (info == 0) {
				//		destroyCDKObject (this); return (0);
			} else {
				cleanChar (info, maxp + 3, '\0');
				infoWidth = maxp + 3;

				/* *INDENT-EQLS* Set up the rest of the structure. */
				ScreenOf (this)             = cdkscreen;
				parent                = cdkscreen->window;
				shadowWin             = 0;
				fieldAttr             = fieldAttrp;
				fieldWidth            = fieldWidth;
				filler                = fillerp;
				hidden                = fillerp;
				ObjOf (this)->inputWindow   = fieldWin;
				ObjOf (this)->acceptsFocus  = TRUE;
				ReturnOf (this)             = NULL;
				shadow                = shadowp;
				screenCol             = 0;
				sbuch=0;
				leftChar              = 0;
				lbuch=0;
				min                   = minp;
				max                   = maxp;
				boxWidth              = boxWidth;
				boxHeight             = boxHeight;
				initExitType (this);
				dispType              = dispTypep;
				callbfn            = &SEntry::CDKEntryCallBack;

				/* Do we want a shadow? */
				if (shadowp) {
					shadowWin = newwin (
							boxHeight,
							boxWidth,
							ypos + 1,
							xpos + 1);
				}
				registerCDKObject (cdkscreen, vENTRY);
			}
		}
	}
//	return (entry);
} // SEntry::SEntry

/*
 * This sets the widgets box attribute.
 */
void SEntry::setCDKEntryBox (bool Box)
{
	box = Box;
	borderSize = Box ? 1 : 0;
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

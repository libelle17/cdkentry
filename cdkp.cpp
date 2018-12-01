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

	if (string != 0 && *string != 0) {
		int len = (int)strlen(string);
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

			if (pass != 0) {
				if ((result = typeMallocN (chtype, used + 2)) == 0) {
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
					if (result != 0) {
						result[0] = ' ';
						result[1] = ' ';
						result[2] = ' ';
					}

					/* Pull out the bullet marker.  */
					while (string[x] != R_MARKER && string[x] != 0) {
						if (result != 0)
							result[x] = (chtype)string[x] | A_BOLD;
						x++;
					}
					adjust = 1;

					/* Set the alignment variables.  */
					start = x;
					used = x;
				} else if (string[1] == 'I' && string[2] == '=') {
					from = 2;
					x = 0;

					while (string[++from] != R_MARKER && string[from] != 0) {
						if (isdigit (CharOf (string[from]))) {
							adjust = (adjust * 10) + DigitOf (string[from]);
							x++;
						}
					}

					start = x + 4;
				} //  else if (string[1] == 'B' && string[2] == '=')
			} // 			if (*string == L_MARKER)

			while (adjust-- > 0) {
				if (result != 0)
					result[used] = ' ';
				used++;
			}

			/* Set the format marker bool to false.  */
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
								|| string[from + 1] == '#')) {
						insideMarker = TRUE;
					} else if (string[from] == '\\' && string[from + 1] == L_MARKER) {
						from++;
						if (result != 0)
							result[used] = CharOf (string[from]) | attrib;
						used++;
						from++;
					} else if (string[from] == '\t') {
						do {
							if (result != 0)
								result[used] = ' ';
							used++;
						} while (used & 7);
					// KLZ else if (result && !strchr("ö",string[from])) KLA result[used++]=CharOf('o')|attrib;
					// GSchade 25.9.18
					/*
						 else if (strchr("äöüÄÖÜß",string[from])) KLA
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
					KLZ
					 */
					// Ende GSchade 25.9.18
				} else {
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

								if (lastChar != 0) {
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
								} for (x = 0; x < adjust; x++) {
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
					} // 					switch (string[from])
				} // 				if (!insideMarker) else
			} // 			for (from = start; from < len; from++)

			if (result != 0) {
				result[used] = 0;
				result[used + 1] = 0;
			}

			/*
			 * If there are no characters, put the attribute into the
			 * the first character of the array.
			 */
			if (!used && result) {
				result[0] = attrib;
			}
		}
		*to = used;
	} else {
		/*
		 * Try always to return something; otherwise lists of chtype strings
		 * would get a spurious null pointer whenever there is a blank line,
		 * and CDKfreeChtypes() would fail to free the whole list.
		 */
		result = typeCallocN (chtype, 1);
	}
	return result;
} // chtype *char2Chtypeh(const char *string, int *to, int *align, int highinr/*=0*/)

/*
 * Split a string into a list of strings.
 */
char **CDKsplitString(const char *string, int separator)
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

#ifdef doppelt
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

	if (string != 0 && *string != 0) {
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

			/* Set the format marker bool to false.  */
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
#endif

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
void cleanChar(char *s, int len, char character)
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
 * This writes out a chtype * string.
 */
void writeChtype(WINDOW *window,
		int xpos,
		int ypos,
		chtype *string,
		int align,
		int start,
		int end)
{
	writeChtypeAttrib(window, xpos, ypos, string, A_NORMAL, align, start, end);
}

/*
 * This writes out a chtype * string * with the given attributes added.
 */
void writeChtypeAttrib (WINDOW *window,
		int xpos,
		int ypos,
		chtype *string,
		chtype attr,
		int align,
		int start,
		int end)
{
	/* *INDENT-EQLS* */
	int diff             = end - start;
	int display          = 0;
	int x                = 0;

	if (align == HORIZONTAL)
	{
		/* Draw the message on a horizontal axis. */
		display = MINIMUM (diff, getmaxx (window) - xpos);
		int altumlz=0;
		for (x = 0; x < display; x++)
		{
			// GSchade 25.9.18
			if (1&&((int)CharOf(string[x+start])==194||(int)CharOf(string[x+start])==195)) {
				//			printf("Buchstabe: %c %i\r\n",CharOf(string[x+start]), (int)CharOf(string[x+start]));
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
//				printf("Buchstabe: %c, Farbe: %lu\n\r",CharOf(string[x+start]),attr);
				(void)mvwaddch (window, ypos, xpos + x-altumlz, string[x + start] |attr);
			}
		}
	}
	else
	{
		/* Draw the message on a vertical axis. */
		display = MINIMUM (diff, getmaxy (window) - ypos);
		for (x = 0; x < display; x++)
		{
			(void)mvwaddch (window, ypos + x, xpos, string[x + start] | attr);
		}
	}
} // void writeChtypeAttrib(

/*
 * This draws a box with attributes and lets the user define
 * each element of the box.
 */
void attrbox (WINDOW *win,
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
	int y2       = getmaxy (win) - 1;
	int x2       = getmaxx (win) - 1;
	int count    = 0;

	/* Draw horizontal lines. */
	if (horz != 0)
	{
		(void)mvwhline (win, y1, 0, horz | attr, getmaxx (win));
		(void)mvwhline (win, y2, 0, horz | attr, getmaxx (win));
		count++;
	}

	/* Draw vertical lines. */
	if (vert != 0)
	{
		(void)mvwvline (win, 0, x1, vert | attr, getmaxy (win));
		(void)mvwvline (win, 0, x2, vert | attr, getmaxy (win));
		count++;
	}

	/* Draw in the corners. */
	if (tlc != 0)
	{
		(void)mvwaddch (win, y1, x1, tlc | attr);
		count++;
	}
	if (trc != 0)
	{
		(void)mvwaddch (win, y1, x2, trc | attr);
		count++;
	}
	if (blc != 0)
	{
		(void)mvwaddch (win, y2, x1, blc | attr);
		count++;
	}
	if (brc != 0)
	{
		(void)mvwaddch (win, y2, x2, brc | attr);
		count++;
	}
	if (count != 0)
	{
		wrefresh (win);
	}
} // void attrbox(

/*
 * This draws a shadow around a window.
 */
void drawShadow (WINDOW *shadowWin)
{
	if (shadowWin != 0) {
		int x_hi = getmaxx (shadowWin) - 1;
		int y_hi = getmaxy (shadowWin) - 1;

		/* Draw the line on the bottom. */
		(void)mvwhline (shadowWin, y_hi, 1, ACS_HLINE | A_DIM, x_hi);

		/* Draw the line on the right. */
		(void)mvwvline (shadowWin, 0, x_hi, ACS_VLINE | A_DIM, y_hi);

		(void)mvwaddch (shadowWin, 0, x_hi, ACS_URCORNER | A_DIM);
		(void)mvwaddch (shadowWin, y_hi, 0, ACS_LLCORNER | A_DIM);
		(void)mvwaddch (shadowWin, y_hi, x_hi, ACS_LRCORNER | A_DIM);
		wrefresh (shadowWin);
	}
}

/*
 * This is a dummy function used to ensure that the constant for mapping has
 * a distinct address.
 */
int getcCDKBind (EObjectType cdktype GCC_UNUSED,
		 void *object GCC_UNUSED,
		 void *clientData GCC_UNUSED,
		 chtype input GCC_UNUSED)
{
   return 0;
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
 * Draw the widget's title.
 */
void CDKOBJS::drawCdkTitle(WINDOW *win)
{
	int x;
	for (x = 0; x < titleLines; x++) {
		writeChtype(win,
				titlePos[x] + borderSize,
				x + borderSize,
				title[x],
				HORIZONTAL, 0,
				titleLen[x]);
	}
}


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

/*
 * The cdktype parameter passed to bindCDKObject, etc., is redundant since
 * the object parameter also has the same information.  For compatibility
 * just use it for a sanity check.
 */

#ifndef KEY_MAX
#define KEY_MAX 512
#endif

/*
CDKOBJS * CDKOBJS::bindableObject (EObjectType * cdktype, void *object)
{
	CDKOBJS *obj = (CDKOBJS *)object;
	if (obj != 0 && *cdktype == this->cdktype) {
		if (*cdktype == vFSELECT) {
			*cdktype = vENTRY;
			object = ((CDKFSELECT *)object)->entryField;
		} else if (*cdktype == vALPHALIST) {
			*cdktype = vENTRY;
			object = ((CDKALPHALIST *)object)->entryField;
		}
	} else {
		object = 0;
	}
	return (CDKOBJS *)object;
}
*/


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
	// EObjectType cdktype = ObjTypeOf (this);
//	CDKOBJS *test = bindableObject (&cdktype, this);
	int result = wgetch (InputWindowOf (this));
	// printf("%c %ul\n",result,result); //G.Schade
	if (result >= 0
			&& bindableObject != 0
			&& (unsigned)result < bindableObject->bindingCount
			&& bindableObject->bindingList[result].bindFunction == getcCDKBind)
	{
		result = (int)(long)bindableObject->bindingList[result].bindData;
	}
	else if (bindableObject == 0
			|| (unsigned)result >= bindableObject->bindingCount
			|| bindableObject->bindingList[result].bindFunction == 0)
	{
		switch (result)
		{
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
	// GSchade Anfang
	cdktype=vENTRY;
	// GSchade Ende
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
 * This draws the entry field.
 */
void SEntry::_drawCDKEntry (bool Box)
{
//	CDKENTRY *entry = (CDKENTRY *)object;
	/* Did we ask for a shadow? */
	if (this->shadowWin != 0) {
		drawShadow(this->shadowWin);
	}
	/* Box the widget if asked. */
	if (Box) {
		drawObjBox(this->win/*, ObjOf (this)*/);
	}
	drawCdkTitle(this->win/*, object*/);
	wrefresh (this->win);

	/* Draw in the label to the widget. */
	if (this->labelWin) {
		//int f1,f2;
		writeChtype (this->labelWin, 0, 0, this->label, HORIZONTAL, 0, this->labelLen);
		wrefresh (this->labelWin);
	}
	this->zeichneFeld();

}


/*
 * This means you want to use the given entry field. It takes input
 * from the keyboard, and when its done, it fills the entry info
 * element of the structure with what was typed.
 */
char * SEntry::activateCDKEntry (chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/, int obpfeil/*=0*/)
{
	chtype input = 0;
	bool functionKey;
	char *ret = 0;
	int zweit;
	if (!Zweitzeichen) Zweitzeichen=&zweit;
	/* Draw the widget. */
	_drawCDKEntry(/*entry, ObjOf (entry)->*/box);
	if (!actions) {
		for (;;) {
			//static int y=2;
			*Zweitzeichen=0;
			input = (chtype)getchCDKObject (&functionKey);
			// GSchade Anfang
			if (input==27) {
				*Zweitzeichen = (chtype)getchCDKObject (&functionKey);
				if (*Zweitzeichen==194||*Zweitzeichen==195) {
					*Drittzeichen = (chtype)getchCDKObject (&functionKey);
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
			ret=injectObj(input)?resultData.valueString:unknownString;
			// GSchade Anfang
      /*
			mvwprintw(entry->parent,1,80,"info:%s ",entry->info);
			for(int i=0;i<strlen(entry->info);i++) {
				mvwprintw(entry->parent,2+i,60,"i: %i: %i",i,entry->info[i]);
			}
			wrefresh(entry->parent); // gleichbedeutend: wrefresh(entry->obj.screen->window);
      */
      drawObj(/*entry, ObjOf (entry)->*/box);
      // GSchade Ende

			if (this->exitType != vEARLY_EXIT||*Zweitzeichen==-8||*Zweitzeichen==-9||*Zweitzeichen==-10||*Zweitzeichen==-11) {
//					mvwprintw(entry->parent,3,2,"Zweitzeichen: %i         , Drittzeichen: %i     ",*Zweitzeichen,*Drittzeichen);
				return ret;
			}
//			mvwprintw(entry->parent,3,2,"kein Zweitzeichen");
		}
	} else {
		int length = chlen (actions);
		int x;
		/* Inject each character one at a time. */
		for (x = 0; x < length; x++) {
//					mvwprintw(entry->parent,4,2,"vor inject 2");
//			ret = injectCDKEntry(entry, actions[x]);
			ret = injectObj(actions[x])?resultData.valueString:unknownString;
			if (this->exitType != vEARLY_EXIT) {
				return ret;
			}
		}
	}
	/* Make sure we return the correct info. */
	if (this->exitType == vNORMAL) {
		return this->info;
	} else {
		return 0;
	}
} // char * SEntry::activateCDKEntry (chtype *actions,int *Zweitzeichen/*=0*/,int *Drittzeichen/*=0*/, int obpfeil/*=0*/)

void SEntry::drawObj(bool Box)
{
//	CDKENTRY *entry = (CDKENTRY *)object;

	/* Did we ask for a shadow? */
	if (this->shadowWin != 0)
	{
		drawShadow (this->shadowWin);
	}

	/* Box the widget if asked. */
	if (Box)
	{
		drawObjBox (this->win/*, ObjOf (this)*/);
	}

	drawCdkTitle (this->win/*, this*/);

	wrefresh (this->win);

	/* Draw in the label to the widget. */
	if (this->labelWin != 0)
	{
		//int f1,f2;
		writeChtype (this->labelWin, 0, 0, this->label, HORIZONTAL, 0, this->labelLen);
		wrefresh (this->labelWin);
	}
	this->zeichneFeld();

}


/*
 * This injects a single character into the widget.
 */
int SEntry::injectObj(chtype input)
{
//	CDKENTRY *widget = (CDKENTRY *)object;
	int ppReturn = 1;
	char *ret = unknownString;
	bool complete = FALSE;
	static char umlaut[3]={0};
	const int inpint=input;
	mvwprintw(this->screen->window,2,2,"injectCDKEntry %c %i          ",input,input);
	 refreshCDKScreen(this->screen);
	if (inpint==194 || inpint==195) {
//		printf("Eintrag: %i\n",inpint);
		*umlaut=inpint;
		umlaut[1]=0;
	} else if ((unsigned char)*umlaut==194 || (unsigned char)*umlaut==195) {
//		printf("Folgezeichen: %i\n",inpint);
		//printf("%c (%i)\n",inpint,inpint);
		umlaut[1]=inpint;
	} else {
//		printf("sonstiges Zeichen: %i\n",inpint);
		umlaut[1]=*umlaut=0;
	}
	/* Set the exit type. */
	setExitType (this, 0);
	/* Refresh the widget field. */
	this->zeichneFeld();
	/* Check if there is a pre-process function to be called. */
	if (PreProcessFuncOf (this) != 0) {
		ppReturn = PreProcessFuncOf (this) (vENTRY,
				this,
				PreProcessDataOf (this),
				input);
	}
	/* Should we continue? */
	if (ppReturn != 0) {
		/* Check a predefined binding... */
		if (checkCDKObjectBind (vENTRY, this, input) != 0) {
			checkEarlyExit (this);
			complete = TRUE;
		} else {
			int infoLength = (int)strlen (this->info);
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
					refreshCDKScreen(allgscr);
					break;
				case CDK_TRANSPOSE:
					if (currPos >= infoLength - 1) {
						Beep ();
					} else {
						char holder = this->info[currPos];
						this->info[currPos] = this->info[currPos + 1];
						this->info[currPos + 1] = holder;
						this->zeichneFeld();
					}
					break;
				case KEY_END:
					this->settoend();
					this->zeichneFeld();
					break;
				case KEY_LEFT:
					if (currPos <= 0) {
						Beep ();
					} else if (this->screenCol == 0) {
						/* Scroll left.  */
						if (currPos>1) if (this->info[currPos-2]==-61 || this->info[currPos-2]==-62) this->leftChar--;
						this->leftChar--;
						this->lbuch--;
						this->zeichneFeld();
					} else {
						/* Move left. */
						wmove (this->fieldWin, 0, --this->sbuch);
						this->screenCol--;
						if (currPos>1) if (this->info[currPos-2]==-61 || this->info[currPos-2]==-62) this->screenCol--;
					}
					break;
				case KEY_RIGHT:
					if (currPos >= infoLength || currPos>this->max) {
						Beep ();
					} else if (this->sbuch == this->fieldWidth - 1) {
						/* Scroll to the right. */
						if (this->info[this->leftChar]==-61 || this->info[this->leftChar]==-62) {
							this->screenCol--;
							this->leftChar++;
						}
						this->leftChar++;
						this->lbuch++;
						if (this->info[currPos]==-61 || this->info[currPos]==-62) this->screenCol++;
						this->zeichneFeld();
					} else {
						/* Move right. */
						wmove (this->fieldWin, 0, ++this->sbuch);
						this->screenCol++;
						if (this->info[currPos]==-61 || this->info[currPos]==-62) this->screenCol++;
					}
					break;
				case KEY_BACKSPACE:
				case KEY_DC:
					if (this->dispType == vVIEWONLY) {
						Beep ();
					} else {
						// mvwprintw(this->parent,1,100,"!!!!!!!!!, currPos: %i  ",currPos);
						bool success = FALSE;
						if (input == KEY_BACKSPACE) {
							--currPos;
							if (this->info[currPos-1]==-61||this->info[currPos-1]==-62) --currPos;
						}
						// .. und jetzt fuer den zu loeschenden
						const int obuml=(this->info[currPos]==-61||this->info[currPos]==-62);
						if (currPos >= 0 && infoLength > 0) {
							if (currPos < infoLength) {
						// mvwprintw(this->parent,2,100,"!!!!!!!!!, currPos: %i, obuml: %i",currPos,obuml);
						wrefresh(this->parent);
								int x;
								for (x = currPos; x < infoLength; x++) {
									if (x+1+obuml>this->max-1) this->info[x]=0;
									else 												 this->info[x]=this->info[x+1+obuml];
								}
								if (obuml) if (infoLength>1) this->info[infoLength-2]=0;
								success = TRUE;
							} else if (input == KEY_BACKSPACE) {
								this->info[infoLength - 1] = '\0';
								success = TRUE;
                if (infoLength>1) if (obuml) this->info[infoLength-2]=0;
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
                  if (this->info[this->leftChar-1]==-61||this->info[this->leftChar-1]==-62) {
										this->leftChar--;
										this->screenCol++;
									}
                  this->lbuch--;
									if (obuml) this->screenCol--;
                }
							}
							this->zeichneFeld();
						} else {
							Beep ();
						}
					}
					break;
				case KEY_ESC:
					setExitType (this, input);
					complete = TRUE;

					mvwprintw(this->parent,2,2,"Key_esc");
					break;
				case CDK_ERASE:
					if (infoLength != 0) {
						cleanCDKEntry (this);
						this->zeichneFeld();
					}
					break;
				case CDK_CUT:
					if (infoLength != 0) {
						freeChar (GPasteBuffer);
						GPasteBuffer = copyChar (this->info);
						cleanCDKEntry (this);
						this->zeichneFeld();
					} else {
						Beep ();
					}
					break;
				case CDK_COPY:
					if (infoLength != 0) {
						freeChar (GPasteBuffer);
						GPasteBuffer = copyChar (this->info);
					} else {
						Beep ();
					}
					break;
				case CDK_PASTE:
					if (GPasteBuffer != 0) {
						setCDKEntryValue (this, GPasteBuffer);
						this->zeichneFeld();
					} else {
						Beep ();
					}
					break;
				case KEY_TAB:
				case KEY_ENTER:
					if (infoLength >= this->min)
					{
						setExitType (this, input);
						ret = (this->info);
						complete = TRUE;
					} else {
						Beep ();
					}
					break;
				case KEY_ERROR:
					setExitType (this, input);
					complete = TRUE;
					break;
				case CDK_REFRESH:
					eraseCDKScreen (ScreenOf (this));
					refreshCDKScreen (ScreenOf (this));
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
						(this->callbackfn) (this, input);
					}
					break;
			}
		}
		/* Should we do a post-process? */
		if (!complete && (PostProcessFuncOf (this) != 0))
		{
			PostProcessFuncOf (this) (vENTRY,
					this,
					PostProcessDataOf (this),
					input);
		}
	}
	if (!complete) setExitType (this, 0);
	ResultOf (this).valueString = ret;
	return (ret != unknownString);
} // int SEntry::injectObj(chtype input)



/*
 * This sets the widgets box attribute.
 */
void SEntry::setCDKEntryBox (bool Box)
{
	box = Box;
	borderSize = Box ? 1 : 0;
}

SFileSelector::SFileSelector()
{
	bindableObject=entryField;
	cdktype = vFSELECT;
}

SAlphalist::SAlphalist()
{
	bindableObject=entryField;
	cdktype = vALPHALIST;
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

SScroll::SScroll
{
	cdktype=vSCROLL;
}

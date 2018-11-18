/* $Id: entry_ex.c,v 1.17 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>
#include <locale.h>
// GSchade 25.9.18
CDKSCREEN *allgscr;

#ifdef HAVE_XCURSES
char *XCursesProgramName = "entry_ex";
#endif
#include <vector>
#include <string>
using namespace std;
vector<string> erg;

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

struct hotkst {
	int nr;
	char buch;
	const char *label;
	u_char obalph;
	CDKENTRY *eingabef;
} hk[]={
	//		 /*
	{4,'r',"</R/U/6>Dürectory:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	//		 */
	{4,'h',"</R/U/6>Alphalist:<!R!6>",1},
	{3,'t',"</R/U/6>Betalist:<!R!6>",1},
	{4,'r',"</R/U/6>Dürectory:<!R!6>"},
	{4,'r',"</R/U/4>Dürectory:<!R!4>"},
	{4,'r',"</R/U/6>Dürectory 5:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 6:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 7:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 8:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 9:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 10:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 11:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 12:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 13:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 14:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 15:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 16:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 17:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 18:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 19:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 20:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 21:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 22:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 23:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 24:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 25:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 26:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 27:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 28:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 29:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 30:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 31:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 32:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 33:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 34:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 35:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 36:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 37:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 38:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 39:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 40:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 41:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 42:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 43:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 44:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 45:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 46:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 47:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 48:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 49:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 50:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 51:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 52:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 53:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 54:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 55:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 56:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 57:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 58:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 59:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 60:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 61:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 62:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 63:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 64:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 65:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 66:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 67:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 68:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 69:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 70:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 71:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 72:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 73:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 74:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 75:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 76:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 77:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 78:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 79:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 80:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 81:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 82:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 83:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 84:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 85:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 86:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 87:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 88:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 89:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 90:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 91:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 92:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 93:<!R!6>"},
	{4,'r',"</R/U/6>Dürectory 94:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 95:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 96:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 97:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 98:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 99:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 100:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 101:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 102:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 103:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 104:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 105:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 106:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 107:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 108:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 109:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 110:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 111:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 150:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 160:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 170:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 180:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 190:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 200:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 210:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 220:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 230:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 240:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 250:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei 260:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 270:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner 280:<!R!6>"},
	{4,'t',"</R/U/6>Dätei:<!R!6>"},
	{4,'n',"</R/U/6>Ordner:<!R!6>"},
};
const size_t maxhk=sizeof hk/sizeof *hk;
const size_t yabst=7;
const int xpos=11;

void zeichne(CDKSCREEN *cdkscreen,int Znr)
{
	static size_t maxy=getmaxy(cdkscreen->window)-yabst;
	static size_t maxh=maxy>maxhk?maxhk:maxy;
	static size_t ymin=0;
	static size_t ymax=maxh;
	static bool erstmals=1;
	bool obverschiebe=erstmals;
	 if (Znr<ymin) {
		 ymin-=(ymin-Znr);
		 ymax-=(ymin-Znr);
		 obverschiebe=1;
	 } else if (Znr>=ymax) {
		 ymin+=(Znr-ymax+1);
		 ymax+=(Znr-ymax+1);
		 obverschiebe=1;
	 } 
	 if (obverschiebe) {
		 mvwprintw(cdkscreen->window,1,xpos,"mit Neuzeichnen: %i-%i, Znr: %i  ",ymin,ymax,Znr);
		 for(int aktent=0;aktent<maxhk;aktent++) {
			 if (aktent>=ymin && aktent<ymax) {
				 hk[aktent].eingabef->obj.isVisible=1;
				 if (hk[aktent].obalph)
					 moveCDKAlphalist(((CDKALPHALIST*)hk[aktent].eingabef),xpos,yabst+aktent-ymin,0,1);
				 else
					 moveCDKEntry(hk[aktent].eingabef,xpos,yabst+aktent-ymin,0,1);
			 } else {
				 hk[aktent].eingabef->obj.isVisible=0;
			 }
		 }
	 }else {
		 mvwprintw(cdkscreen->window,1,xpos,"ohne Neuzeichnen: %i-%i, Znr: %i  ",ymin,ymax,Znr);
	 }
	 refreshCDKScreen(cdkscreen);
	 erstmals=0;
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
//	 cdkscreen->window->_maxx=300;
//	 cdkscreen->window->_scroll=1;
//	 scrollok(cdkscreen->window,TRUE);
//	 scroll(cdkscreen->window);
//	 for(int i=0;i<64;i++) mvwaddch (cdkscreen->window, 20+(i/8), 20+i, 'a'|COLOR_PAIR(i));
	 /*
	 chtype wcol=COLOR_PAIR(11)|A_BLINK;
				wattron(cdkscreen->window,wcol); // wirkt nicht
		mvwprintw(cdkscreen->window,3,60,"%s","weltoffen");
				wattroff(cdkscreen->window,wcol); // wirkt nicht
				*/

	 /* Create the entry field widget. */
//	 directory = newCDKEntry (cdkscreen,/*xplace*/xpos,/*yplace*/12, /*title*/"", label, A_UNDERLINE, '.', vMIXED, 30, 0, max,/*Box*/0,/*shadow*/0,/*highnr*/2);
//	 file = newCDKEntry (cdkscreen, xpos, 13, /*ftit*/"", flabel, A_NORMAL, '.', vMIXED, 30, 0, max,/*Box*/0,/*shadow*/0,/*highnr*/1);

		 allgscr=cdkscreen=initCDKScreen(0);
		 static size_t maxy=getmaxy(cdkscreen->window)-yabst;
		 static size_t maxh=maxy>maxhk?maxhk:maxy;
		 /* Start CDK colors. */
		 initCDKColor ();
		 const int maxlen=100;
		 for(size_t aktent=0;aktent<maxhk;aktent++) {
			 if (hk[aktent].obalph) {
				 hk[aktent].eingabef=(CDKENTRY*)newCDKAlphalist(cdkscreen,xpos,yabst+aktent,10,40,"",hk[aktent].label,(CDK_CSTRING*)userList,userSize,'.',A_REVERSE,0,0,hk[aktent].nr);
			 } else {
				 hk[aktent].eingabef=newCDKEntry(cdkscreen,xpos,yabst+aktent,"",hk[aktent].label,A_NORMAL,'.',vMIXED,30,0,maxlen,0,0,hk[aktent].nr);
				 bindCDKObject (vENTRY, hk[aktent].eingabef, '?', XXXCB, 0);
			 }
			 /* Is the widget null? */
			 if (!hk[aktent].eingabef) {
				 /* Clean up. */
				 destroyCDKScreen(cdkscreen);
				 endCDK ();

				 printf ("Cannot create the entry box. Is the window too small, aktent: %lu   \n",aktent);
				 ExitProgram (EXIT_FAILURE);
			 }
		 }

		 /*
    * Pass in whatever was given off of the command line. Notice we
    * don't check if argv[1] is null or not. The function setCDKEntry
    * already performs any needed checks.
    */
   //setCDKEntry (hk[0].eingabef, argv[optind], 0, max, TRUE);

   /* Activate the entry field. */
	 int Znr=0,Zweitzeichen=0;
   EExitType	exitType;
	 while (1) {
		 akteinbart=einb_direkt;
		 /* Draw the screen. */
		 // refreshCDKScreen (cdkscreen);
		 zeichne(cdkscreen,Znr);
		 //		mvwprintw(cdkscreen->window,30,30,"<R>werde eingeben:%i %i ",info,Zweitzeichen);
		 if (hk[Znr].obalph) {
			 akteinbart=einb_alphalist;
			 info = activateCDKAlphalist((CDKALPHALIST*)hk[Znr].eingabef, 0,&Zweitzeichen, /*obpfeil*/0);
			 eraseCDKScroll (((CDKALPHALIST*)hk[Znr].eingabef)->scrollField);
		 } else {
			 info = activateCDKEntry(hk[Znr].eingabef, 0,&Zweitzeichen, /*obpfeil*/1);
		 }
		 //#ifdef mdebug
		 /*
		 mesg[0] = "<C>Letzte Eingabe:";
			 snprintf (temp, sizeof temp-1,"<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info?info:"",Zweitzeichen);
//		 if (info) if (hk[Znr].eingabef) if (hk[Znr].eingabef->info)
			 //snprintf(temp,sizeof temp-1,"<C>(%.*s|%s|%i)",(int)(sizeof(temp)-10),info?info:"",info&&hk[Znr].eingabef&&hk[Znr].eingabef->info?hk[Znr].eingabef->info:"",Zweitzeichen);
		 mesg[1] = temp;
		 mesg[2] = "<C>Press any key to continue.";
		 */
//#endif
		 exitType=hk[Znr].eingabef->exitType;
//		 popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
		 // Tab
//#ifdef richtig		 
		mvwprintw(cdkscreen->window,3,60,"Zweitzeichen: %i",Zweitzeichen);
		 if (Zweitzeichen==-9) {
			 Znr++;
			 // Alt+Tab
			 // Seite nach unten
		 } else if (Zweitzeichen==-10) {
			 Znr+=maxh-1;
		 } else if (Zweitzeichen==-8) {
			 Znr--;
			 // Seite nach oben
		 } else if (Zweitzeichen==-11) {
			 Znr-=maxh-1;
			 // Alt- +Buchstabe
		 } else {
			 if (Zweitzeichen) /*(info && *info==27)*/ {
				 Znr=-1;
				 for(int aktent=0;aktent<maxhk;aktent++) {
					 if (Zweitzeichen==hk[aktent].buch) {
						 Znr=aktent;
						 break;
					 }
				 }
				 if (Znr==-1) break;
			 } else {
				 break;
			 }
		 }
		 if (Znr<0) {
			 Znr=0; // Znr=sizeof hk/sizeof *hk-1;
		 } else if (Znr>=maxhk) {
			 Znr=maxhk-1; // sizeof hk/sizeof *hk) Znr=0;
		 }
		 //#endif 
	 }  // while (1)
	 for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) {
		 //		 erg.push_back(hk[aktent].eingabef&&hk[aktent].eingabef->info?hk[aktent].eingabef->info:"");
		 const char *ueb=
			 hk[aktent].obalph?
			 ueb=((CDKALPHALIST*)hk[aktent].eingabef)->entryField->info:
			 ueb=hk[aktent].eingabef->info;
		 erg.push_back(ueb);
	 }
	 /*
			if (0) {
// Tell them what they typed.
if (exitType == vESCAPE_HIT && !Zweitzeichen) {
mesg[0] = "<C>Sie schlügen escape. No information passed back.";
mesg[1] = temp;
mesg[2] = "<C>Press any key to continue.";
for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKObject(hk[aktent].eingabef);
popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
} else if (exitType == vNORMAL) {
mesg[0] = "<C>Sie gaben folgendes ein";
sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
mesg[1] = temp;
mesg[2] = "<C>Press any key to continue.";
for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKObject(hk[aktent].eingabef);
popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
} else {
sprintf(temp0,"Exit-type: %i",exitType);
mesg[0]=temp0;
mesg[1] = "<C>Sie gaben folgendes ein: ";
sprintf (temp, "<C>(%.*s|%i)", (int)(sizeof (temp) - 10), info,Zweitzeichen);
mesg[2] = temp;
mesg[3] = "<C>Eine Taste drücken zum Fortfahren.";
for(int aktent=0;aktent<sizeof hk/sizeof *hk;aktent++) destroyCDKObject(hk[aktent].eingabef);
popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 4);
}
}
		*/

/* Clean up and exit. */
	 destroyCDKScreen(cdkscreen);
	 endCDK ();
	 for(int aktent=0;aktent<erg.size();aktent++) {
		 printf("%s\n",erg[aktent].c_str());
	 }
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

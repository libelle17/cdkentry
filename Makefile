CC=$(shell which g++-8 2>/dev/null||which g++-7 2>/dev/null)
FLIST=-I. -I$$HOME/cdk/include -I/usr/include/ncursesw -o entry entry_ex.c cdkscreen.c traverse.c binding.c popup_label.c label.c cdk_objs.c draw.c cdk.c alphalist.c fselect.c dialog.c entry.c cdk_display.c cdk_params.c scroll.c scroller.c -lncursesw
.PHONY: all
all: entry

entry: *.c *.h
	[ -n "$(CC)" ]&&{ $(CC) -g $(FLIST); }||:

opt:
	[ -n "$(CC)" ]&&{ $(CC) -O -I. -I$$HOME/cdk/include -I/usr/include/ncursesw -o entry entry_ex.c cdkscreen.c traverse.c binding.c popup_label.c label.c cdk_objs.c -lncursesw draw.c cdk.c alphalist.c fselect.c entry.c cdk_display.c cdk_params.c scroll.c scroller.c; }||:

.PHONY: p
p: cdkp.o

cdkp.o: cdkp.cpp cdkp.h
	[ -n "$(CC)" ]&&{ F=fehler.txt; $(CC) -c cdkp.cpp 2>$$F; test -s $$F && vi $$F||:; }||:
git:
	git add -u
	git commit -m"aus Makefile"
	git push

.PHONY: neu
neu: eingabep
eingabep: *.cpp cdkp.h
	[ -n "$(CC)" ]&&{ F=fehler.txt; $(CC) -o $@ -I. -I$$HOME/cdk/include -I/usr/include/ncursesw eingabe.cpp cdkp.cpp -lncursesw >$$F 2>&1; test -s $$F && vi $$F||:; }||:

.PHONY: comp
comp:
	@echo CC=\'$(CC)\'

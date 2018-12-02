.PHONY: all
all: entry

entry: *.c *.h
	g++-7 -g -I. -I$$HOME/cdk/include -I/usr/include/ncursesw -o entry entry_ex.c cdkscreen.c traverse.c binding.c popup_label.c label.c cdk_objs.c draw.c cdk.c alphalist.c entry.c cdk_display.c cdk_params.c scroll.c scroller.c -lncursesw 

opt:
	g++-7 -O -I. -I$$HOME/cdk/include -I/usr/include/ncursesw -o entry entry_ex.c cdkscreen.c traverse.c binding.c popup_label.c label.c cdk_objs.c -lncursesw draw.c cdk.c alphalist.c entry.c cdk_display.c cdk_params.c scroll.c scroller.c

.PHONY: p
p: cdkp.o

cdkp.o: cdkp.cpp cdkp.h
	F=fehler.txt; g++-7 -c cdkp.cpp 2>$$F; test -s $$F && vi $$F||:
git:
	git add -u
	git commit -m"aus Makefile"
	git push

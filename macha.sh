#!/bin/bash
g++ -I. -I$HOME/cdk/include -I/usr/include/ncursesw -o alph alphalist_ex.c alphalist.c cdkscreen.c traverse.c binding.c popup_label.c label.c cdk_objs.c -lncursesw draw.c cdk.c entry.c cdk_display.c cdk_params.c scroll.c scroller.c

#!/bin/bash

for fl in `find include/ src/ tools/ -type f`; do
  sed -e 's/\r//' -e 's/[ \t]*$//' -i $fl
done;

astyle --style=kr --mode=c --indent=spaces=2 --indent-switches --indent-col1-comments --break-blocks=all --pad-oper --pad-header --delete-empty-lines --align-pointer=type --convert-tabs  src/*/*.c include/*.h  

# @ astyle --style=kr --mode=c --indent=spaces=2 --attach-extern-c --indent-switches --indent-preproc-define --indent-col1-comments --break-blocks=all --pad-oper --pad-header --delete-empty-lines --align-pointer=type --align-reference=type --remove-brackets --convert-tabs --close-templates --remove-comment-prefix --max-code-length=80 --break-after-logical src/dbg/inodes.c  


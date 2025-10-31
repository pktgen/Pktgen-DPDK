#! /bin/bash
#

for filename in $(find . -path ./Builddir -prune -o -name "*.[ch]"); do
    if [ -f $filename ]; then
        echo "Process File: $filename"
        clang-format -style=file -i $filename; # git add $filename;
    fi
done

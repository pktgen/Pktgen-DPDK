#! /bin/bash
#

for filename in $(find . -path ./Builddir -prune -o -name "*.[ch]"); do
    echo "Process File: $filename"
    clang-format -style=file -i $filename; # git add $filename;
done


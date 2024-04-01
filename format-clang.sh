#! /bin/bash
#

for filename in $(find . -path ./builddir -prune -o -name "*.[ch]"); do
    echo "Process File: $filename"
    clang-format -style=file -i $filename; # git add $filename;
done


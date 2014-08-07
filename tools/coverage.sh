#!/bin/bash

gcov -bcupl `find -name *.gcda | tr '\n' ' '`

lcov -c --directory . -b . > Fs.lcov

genhtml -o lcov Fs.lcov

# pmccabe -tv src/*/*.c src/*/*/*.c include/*.h > complexity.scb
# pmccabe -tnv src/*/*.c src/*/*/*.c include/*.h > comment.ncb

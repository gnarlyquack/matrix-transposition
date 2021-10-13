#!/usr/bin/env bash

EXENAME=transpose

CC=clang
#CC=gcc

# C options
CFLAGS="--std=c17"
#CFLAGS+=" -g"
CFLAGS+=" -O3"
CFLAGS+=" -Wall -Wextra -Wpedantic"
CFLAGS+=" -Wcast-qual"
CFLAGS+=" -Wconversion"
CFLAGS+=" -Wshadow"
CFLAGS+=" -Wunused"
CFLAGS+=" -Werror=vla"

CPPFLAGS=
LDFLAGS=
LDLIBS=


START=$(date +"%s.%N")

echo "Building ${EXENAME} executable..."
$CC $CPPFLAGS $CFLAGS -o $EXENAME ${EXENAME}.c $LDFLAGS $LDLIBS
RESULT=$?

if [ $RESULT -eq 0 ]
then
    END=$(date +"%s.%N")
    ELAPSED=$(echo "$END - $START" | bc)
    printf "Finished in %.2f secs\n" $ELAPSED
fi

exit $RESULT

#!/bin/sh

# Replace named constants in linker script using C preprocessor
# $1    - original linker script file
# $2    - file name for generated linker script
# $3... - CFLAGS

in=$1
out=$2
shift
shift
cflags=$*

echo "/*"						>$out
echo " * This file was generated automatically."	>>$out
echo " * Please do not modify it manually!"		>>$out
echo " */"						>>$out

gcc -E -x c $cflags $in | grep -v '^#' | sed '/^$/N;/^\n$/D' >>$out
res=$?
if [ $res -ne 0 ]; then
	echo "Error: Unable to generate linker script" >&2
	rm -f $out
	exit $res
fi

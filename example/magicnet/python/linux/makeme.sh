#! /bin/sh
UNAME=`uname | awk -F\_ '{print $1}'`
PYTHON=`python -c "from sys import version_info as v;print 'python%d.%d'%(v[0],v[1])"`
VERSION=`python -c "from sys import version_info as v;print '%d.%d'%(v[0],v[1])"`
INC="/usr/include/$PYTHON/"
LIB="/usr/lib"
CC=gcc
FILE="*.h *.c"
FLAG="-Wall -DNDEBUG -fPIC -O3"
LIBS=""
OUTPUT="magicnet.so"

$CC -shared $FLAG -I"$INC" -L"$LIB" $FILE -o $OUTPUT $LIBS

rm -r -f *.plg *.o > /dev/null



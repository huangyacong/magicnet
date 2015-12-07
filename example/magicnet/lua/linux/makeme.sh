#! /bin/sh
PYTHON=`python -c "from sys import version_info as v;print 'python%d.%d'%(v[0],v[1])"`
INC="/usr/local/include"
LIB="/usr/local/lib"
CC=gcc
FLAG="-Wall -g -DNDEBUG "
LIBS=""
OUTPUT="magicnet.so"

COMMON_SRC="./../../../../common/*.c"
COMMON_H="./../../../../common/"
MAGICNET_SRC_A="./../../../../magicnet/SeMagicNet.c"
MAGICNET_SRC="./../../../../magicnet/ccmoduleLua.c"
MAGICNET_H="./../../../../magicnet/"
NETBASE_SRC="./../../../../netbase/*.c"
NETBASE_H="./../../../../netbase/"

$CC  $FLAG -fPIC -shared -ldl -llua -lm -I"$INC" -L"$LIB" $COMMON_SRC -I"$COMMON_H" $MAGICNET_SRC $MAGICNET_SRC_A -I"$MAGICNET_H" $NETBASE_SRC -I"$NETBASE_H" -o $OUTPUT $LIBS -lpthread -lrt -Wl,-E

rm -r -f *.plg *.o > /dev/null
mv -f $OUTPUT ../



#! /bin/sh
CC=gcc
FLAG="-Wall -DNDEBUG -g "
LIBS=""
OUTPUT="magicnet.dll"

COMMON_SRC="./../../../../common/*.c"
COMMON_H="./../../../../common/"
MAGICNET_SRC_A="./../../../../magicnet/SeMagicNet.c"
MAGICNET_SRC="./../../../../magicnet/ccmoduleCSharp.c"
MAGICNET_H="./../../../../magicnet/"
NETBASE_SRC="./../../../../netbase/*.c"
NETBASE_H="./../../../../netbase/"

$CC -shared -fPIC $FLAG $COMMON_SRC -I"$COMMON_H" $MAGICNET_SRC $MAGICNET_SRC_A -I"$MAGICNET_H" $NETBASE_SRC -I"$NETBASE_H" -o $OUTPUT $LIBS -lrt -lpthread

rm -r -f *.plg *.o > /dev/null
mv -f $OUTPUT ../



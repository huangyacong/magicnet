#! /bin/sh
gcc -Wall -shared -fPIC -o magicnet.so -lpthread -I/home/huangyc/lua-5.3.2/src/ -llua   *.c
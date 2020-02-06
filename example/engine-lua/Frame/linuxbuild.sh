#!/bin/sh
cd CoreLog
make clean
make
cd ..
cd CoreMySql
rm -fr *.o
make
cd ..
cd CoreNet
rm -fr *.o
make
cd ..
cd CoreTool
rm -rf *.o
make
cd ..
cd protobuf
rm -rf *.o
make
cd ..

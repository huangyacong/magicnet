#!/bin/sh

cp -rf ../../../common/* ./src/CoreSrc/common/
cp -rf ../../../netbase/* ./src/CoreSrc/netbase/
cp -rf ../../../mysql/* ./src/CoreSrc/mysql/
cp -rf ../../../cpp/* ./src/NetEngine/

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
cd CoreNetAgent
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

rm -rf ./src/CoreSrc/mysql/dev
rm -r ./src/CoreSrc/common/*.h
rm -r ./src/CoreSrc/common/*.c
rm -r ./src/CoreSrc/netbase/*.h
rm -r ./src/CoreSrc/netbase/*.c
rm -r ./src/CoreSrc/mysql/*.h
rm -r ./src/CoreSrc/mysql/*.c
rm -r ./src/NetEngine/*.h
rm -r ./src/NetEngine/*.cpp

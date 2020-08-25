#!/bin/sh
cd jemalloc-5.2.1
chmod +x configure
find . -name "*.sh" -exec chmod +x {} \;
./configure
make
sudo make install
exit


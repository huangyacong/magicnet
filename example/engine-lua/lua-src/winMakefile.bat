@echo off

echo 建立日期 %date%

SET VSCMD=%VS120COMNTOOLS%../IDE/devenv

echo 开始编译 "%VSCMD%"

"%VSCMD%"  "./lua-win/buildall.sln" /Rebuild "Release|x64"

echo 编译所有工程完毕

PAUSE

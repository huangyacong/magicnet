@echo off

echo �������� %date%

SET VSCMD=%VS120COMNTOOLS%../IDE/devenv

echo ��ʼ���� "%VSCMD%"

"%VSCMD%"  "./lua-win/buildall.sln" /Rebuild "Release|x64"

echo �������й������

PAUSE

@echo off

echo �������� %date%

SET VSCMD=%VS120COMNTOOLS%../IDE/devenv

echo ��ʼ���� "%VSCMD%"

"%VSCMD%"  "./CoreNet/CoreNet.sln" /Rebuild "Release|x64"

echo �������й������

PAUSE

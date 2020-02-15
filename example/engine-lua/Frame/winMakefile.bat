@echo off

xcopy /y /e /i /q ..\..\..\common\* .\src\CoreSrc\common\
xcopy /y /e /i /q ..\..\..\netbase\* .\src\CoreSrc\netbase\
xcopy /y /e /i /q ..\..\..\mysql\* .\src\CoreSrc\mysql\
xcopy /y /e /i /q ..\..\..\cpp\* .\src\NetEngine\

echo �������� %date%

SET VSCMD=%VS120COMNTOOLS%../IDE/devenv

echo ��ʼ���� "%VSCMD%"

"%VSCMD%"  "./CoreNet/CoreNet.sln" /Rebuild "Release|x64"

echo �������й������

rd /S /Q .\src\CoreSrc\mysql\dev
del /Q /S .\src\CoreSrc\common\*.h
del /Q /S .\src\CoreSrc\common\*.c
del /Q /S .\src\CoreSrc\netbase\*.h
del /Q /S .\src\CoreSrc\netbase\*.c
del /Q /S .\src\CoreSrc\mysql\*.h
del /Q /S .\src\CoreSrc\mysql\*.c
del /Q /S .\src\NetEngine\*.h
del /Q /S .\src\NetEngine\*.cpp

PAUSE

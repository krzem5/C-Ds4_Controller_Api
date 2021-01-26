@echo off
cls
set _INCLUDE=%INCLUDE%
set INCLUDE=../src/include;%INCLUDE%
if exist build rmdir /s /q build
mkdir build
cd build
cl /c /permissive- /GS /W3 /Zc:wchar_t /Gm- /sdl /Zc:inline /fp:precise /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /errorReport:none /WX /Zc:forScope /Gd /Oi /FC /EHsc /nologo /diagnostics:column /ZI /Od /RTC1 /MDd ..\src\main.c ..\src\ds4\*.c&&link *.obj /OUT:index.exe /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /MACHINE:X64 /SUBSYSTEM:CONSOLE /ERRORREPORT:none /NOLOGO /TLBID:1 /WX /DEBUG /INCREMENTAL&&del *.obj&&del *.ilk&&del *.idb&&del *.pdb&&cls&&index.exe
cd ../
set INCLUDE=%_INCLUDE%

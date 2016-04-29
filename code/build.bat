@echo off
rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
cd ..\build\
cl -FC -Zi ..\code\handmade.cpp user32.lib gdi32.lib
cd ..\code\
rem ..\build\handmade.exe
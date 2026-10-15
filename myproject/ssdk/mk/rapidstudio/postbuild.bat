@echo off
::set BOARD=%1
::set PROJECT=%2

set MSYSPATH=%~dp0\..\..\prebuilts\windows\msys\usr\bin
set PYTHON3=%~dp0\..\..\prebuilts\windows\python-3.7.0\python.exe
set path=%MSYSPATH%;%path%
set ROOTDIR=%~dp0\..\..\
cd /d %ROOTDIR%

echo "board:"%1
echo "project:"%2
echo "core:"%3
echo "build":%4

cp %~dp0\..\..\boards\%1\build\%2\rapidstudio\%3\%4\%3.bin %~dp0\..\..\boards\%1\build\%2\rapidstudio\%3\%3.bin

%PYTHON3% tools\genpac.py  -b %1 -p %2 -v rapidstudio -q
echo %errorlevel%
if %errorlevel% neq 0 (
	echo "Some images don't be built!"
	exit 0
)

sh build.sh -b %1 -p %2 -rapidstudio -pac

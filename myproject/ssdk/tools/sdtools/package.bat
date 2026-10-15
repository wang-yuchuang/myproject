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

%PYTHON3% tools\genpac.py  -b %1 -p %2 -v iar -q
echo %errorlevel%
if %errorlevel% neq 0 (
	echo "Some images don't be built!"
	exit 0
)

sh build.sh -b %1 -p %2 -iar -pac

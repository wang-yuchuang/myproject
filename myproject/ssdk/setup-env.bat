@echo off

setlocal EnableDelayedExpansion
set MSYSPATH=%~dp0prebuilts\windows\msys\usr\bin
echo msys path:%MSYSPATH%

set has_set=false
set USER_PATH_V=

for /f "skip=2 tokens=*" %%A in ('reg query "HKCU\Environment" /v PATH') do (
   set regstr=%%A

   for /f "tokens=3 delims=|" %%X in ("!regstr:    =^|!") do (
	 set USER_PATH_V=%%X
   )
)

::remove space at the end of USER_PATH_V
if "%USER_PATH_V:~-1%"==" " set "USER_PATH_V=%USER_PATH_V:~0,-1%"
echo "%USER_PATH_V%"

for  %%A in ("%USER_PATH_V:;=" "%") do (
	::echo val:%%A
	if /i %%A equ "%MSYSPATH%" (
		set has_set=true
		goto End
	)
)

echo msys has not set,set it

::set current cmd envionment variables
set PATH=%MSYSPATH%;%PATH%

::set current user envionment variables
setx PATH "%MSYSPATH%;%USER_PATH_V%"

:End
echo msys has set successfully

::reopen cmd
cmd.exe

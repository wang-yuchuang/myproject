set TOP_PATH=%4
cd /d %TOP_PATH%
%TOP_PATH%\prebuilts\windows\msys\usr\bin\sh.exe %TOP_PATH%\build.sh -b %1 -p %2 -c %3 -iar -make_configs
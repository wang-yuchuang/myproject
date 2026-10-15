set TOP_PATH=%4
cd /d %TOP_PATH%

rm -f %TOP_PATH%/boards/%1/build/%2/rapidstudio/%3/%3.bin
sh.exe %TOP_PATH%/build.sh -b %1 -p %2 -c %3 -rapidstudio -make_configs
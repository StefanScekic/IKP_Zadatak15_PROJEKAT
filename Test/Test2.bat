@echo off
set start_time=%time%

FOR /L %%i IN (1, 1, 20) DO (
    start /b .\Process.exe 10 1800 2
    echo %%i
)

set end_time=%time%

set /a start_time_seconds = 1%start_time:~0,2%-100 * 3600 + 1%start_time:~3,2%-100 * 60 + 1%start_time:~6,2%-100
set /a end_time_seconds = 1%end_time:~0,2%-100 * 3600 + 1%end_time:~3,2%-100 * 60 + 1%end_time:~6,2%-100
set /a execution_time = end_time_seconds - start_time_seconds

echo TOTAL Execution time: %execution_time% secondss
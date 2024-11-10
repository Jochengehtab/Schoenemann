@echo off
for /f "tokens=2" %%a in ('tasklist /FI "IMAGENAME eq cmd.exe" /NH') do (
    for /f "tokens=1,2*" %%b in ('tasklist /FI "PID eq %%a" /fo csv /v ^| findstr /i "null.exe"') do (
        taskkill /PID %%a /F
    )
)
for /f "tokens=2" %%a in ('tasklist /FI "IMAGENAME eq null.exe" /NH') do (
    taskkill /PID %%a /F
)
for /f "tokens=2" %%a in ('tasklist /FI "IMAGENAME eq cmd.exe" /NH') do (
    taskkill /PID %%a /F
)
echo All cmd.exe processes have been killed.

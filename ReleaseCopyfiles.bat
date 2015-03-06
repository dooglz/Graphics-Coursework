REM DLLS FILES
xcopy /s /y "redist\Win32\*.dll" "bin\Win32_Release\"
REM Resources
xcopy /s /y "res\*" "bin\Win32_Release\"
pause>nul
REM DLLS FILES
xcopy /s /y "redist\Win32\*.dll" "bin\Win32_Debug\"
xcopy /s /y "redist\Win32_Debug\*.dll" "bin\Win32_Debug\"
REM Resources
xcopy /s /y "res\*" "bin\Win32_Debug\"
pause>nul
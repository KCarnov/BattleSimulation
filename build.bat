@echo off

echo ====================  Compilation begin 
set  MODE=%1
echo Build mode is %MODE%
echo.
REM set CommonCompilerFlags=-MT -O2 -nologo -Gm- -GR- -FC -Z7 
set CommonCompilerFlags=-MT -Od -Oi -nologo -Gm- -GR- -DDEBUG=1 -FC -Z7 
set CommonLinkerFlags=user32.lib gdi32.lib winmm.lib

IF NOT EXIST bin mkdir bin
pushd bin
cl %CommonCompilerFlags% ../src/platform/platform_win32.cpp /link %CommonLinkerFlags%
popd 



echo.
echo ==================== Compilation end


if "%MODE%" NEQ "DEBUG" (
pushd bin
.\platform_win32.exe
popd)


REM -Zi : Produces DEBUG information (.pdb)
REM -Z7 : Only one .pdb (not the vc140.pdb)

REM Compiler intrinsics
REM -Oi : use things from processor that knows something instead of the CRT variant ?? TODO: comfirm this
REM -Od ?
REM -GR- : Disable Runtime type information (c++)
REM -Gm- : Turns off incremental build ?

REM -EHa- : Exception handling

REM -MD : use the dll for the CRT
REM -MT : use static library for CRT

REM /RTC1 : Enable runtime checks ! 
REM -nologo : Removes the message : Microsoft (R) C/C++ Optimizing Compiler Version 19.50.35724 for x86 and stuff

REM -Fm map file path : tells linker a location to stick a map file

REM Linker : TODO: -opt:ref ?

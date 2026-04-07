@echo off

REM bat file to start running vs code wih developper command from MSVC
echo -------------------------------
echo ----- C_Project starting ------
echo -------------------------------

call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat"

REM Go to project directory 
cd /d C:\Users\PC\Dev\C_Project 

REM Launch VSCode (2 options) 
REM code . 

REM or this to close the cmd dev 
start "" code . exit
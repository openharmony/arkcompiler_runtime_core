@echo off
echo Building all ArkTS docs ...
call build-cookbook.bat 
call build-stdlib.bat 
call build-tutorial.bat 
call build-spec.bat 
echo all pdf files are in appropriate 'build' sub-folder for every document

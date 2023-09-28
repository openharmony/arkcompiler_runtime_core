@echo off
md __build
cd tutorial
sphinx-build -n -b latex . ..\__build
cd ..\__build
latexmk -f -silent -pdf -dvi- -ps- *.tex
md ..\tutorial\build
move *.pdf ..\tutorial\build
cd ..
rmdir /S /Q __build



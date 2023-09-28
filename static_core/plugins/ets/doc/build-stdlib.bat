@echo off
md __build
cd stdlib
sphinx-build -n -b latex . ..\__build
cd ..\__build
latexmk -f -silent -pdf -dvi- -ps- *.tex
md ..\stdlib\build
move *.pdf ..\stdlib\build
cd ..
rmdir /S /Q __build

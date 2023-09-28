@echo off
md __build
cd spec
sphinx-build -n -b latex . ..\__build
cd ..\__build
latexmk -f -silent -pdf -dvi- -ps- *.tex
md ..\spec\build
move *.pdf ..\spec\build
cd ..
rmdir /S /Q __build

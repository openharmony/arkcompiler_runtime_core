@echo off
md __build
cd cookbook
sphinx-build -n -b latex . ..\__build
cd ..\__build
latexmk -f -silent -pdf -dvi- -ps- *.tex
md ..\cookbook\build
move *.pdf ..\cookbook\build
cd ..
rmdir /S /Q __build




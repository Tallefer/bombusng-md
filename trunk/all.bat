@echo off
del /s /q dist
mkdir dist
mkdir dist/asis
mkdir disk/cab
mkdir disk/arh
echo *****************************
echo ******** ������ ����� *******
echo *****************************
call make.bat
echo *****************************
echo **** ����������� ���� *******
echo **** ������ � dist\asis *****
echo *****************************
call build_all.bat
echo *****************************
echo ******** ������ cab`� *******
echo *****************************
call cabbing.bat
echo *****************************
echo ******* ������������� *******
echo *****************************
call zip.bat
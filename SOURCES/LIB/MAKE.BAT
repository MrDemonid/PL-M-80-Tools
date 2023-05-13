:: free environment space
set PROCESSOR_ARCHITECTURE=
set PROCESSOR_IDENTIFIER=
set PROCESSOR_LEVEL=
set PROCESSOR_REVISION=
set PROGRAMFILES=
set USERPROFILE=
set ALLUSERSPROFILE=
set DXSDKROOT=
set APPDATA=
set COMMONPROGRAMFILES=
set COMPUTERNAME=

:: set WATCOM path's
if exist %WATCOM% goto docomp
set DRIVE=D
set WATCOM=%DRIVE%:\WATCOM
set PATH=%DRIVE%:\WATCOM\BINNT;%DRIVE%:\WATCOM\BINW;%PATH%
set INCLUDE=%DRIVE%:\WATCOM\H;%DRIVE%:\WATCOM\H\NT;%INCLUDE%
set LIB=%DRIVE%:\WATCOM\lib386;%DRIVE%:\WATCOM\lib386\NT

:docomp
del ..\..\Bin\lib.exe
del *.obj

cls
wcc386 io.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=io.obj -mf
wcc386 lib.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=lib.obj -mf
wcc386 lib1.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=lib1.obj -mf
wcc386 lib3.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=lib3.obj -mf
wcc386 lib4.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=lib4.obj -mf
wcc386 version.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=version.obj -mf
wrc version.rc -bt=nt -dWIN32 -d_WIN32 -d__NT__ -q -ad -r -fo=version.res

wlink name ..\..\Bin\lib sys nt op m op maxe=25 op q op symf @make.txt

del *.obj
del *.map
del version.res

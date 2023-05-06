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
del ..\..\Bin\asm80.exe
del *.obj

cls
wcc386 asm1m.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 asm2m.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 asm3m.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 asm4m.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 asm5m.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 asm6m.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 asm8m.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 asxref.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 cntrlm.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 debug.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 emitm.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 globlm.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 initm.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 io.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 keym.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 listm.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 pcktok.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 rdsrc.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 startm.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wcc386 version.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=.obj -mf
wrc version.rc -bt=nt -dWIN32 -d_WIN32 -d__NT__ -q -ad -r -fo=version.res

wlink name ..\..\Bin\asm80 sys nt op m op maxe=25 op q op symf @make.txt

del *.obj
del *.map
del *.res

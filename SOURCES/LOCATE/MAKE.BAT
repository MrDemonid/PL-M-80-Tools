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
del ..\..\Bin\locate.exe
del *.obj

cls
wcc386 io.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=io.obj -mf
wcc386 loc1.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc1.obj -mf
wcc386 loc2a.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc2a.obj -mf
wcc386 loc2b.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc2b.obj -mf
wcc386 loc3.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc3.obj -mf
wcc386 loc4.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc4.obj -mf
wcc386 loc4a.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc4a.obj -mf
wcc386 loc5a.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc5a.obj -mf
wcc386 loc5b.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc5b.obj -mf
wcc386 loc6.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc6.obj -mf
wcc386 loc7.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc7.obj -mf
wcc386 loc8.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc8.obj -mf
wcc386 loc8a.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc8a.obj -mf
wcc386 loc9.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=loc9.obj -mf
wcc386 version.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=version.obj -mf
wrc version.rc -bt=nt -dWIN32 -d_WIN32 -d__NT__ -q -ad -r -fo=version.res

wlink name ..\..\Bin\locate sys nt op m op maxe=25 op q op symf @make.txt

del *.obj
del *.map
del *.res

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
del ..\..\Bin\link.exe
del *.obj

cls
wcc386 io.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=io.obj -mf
wcc386 binasc.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=binasc.obj -mf
wcc386 errrpt.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=errrpt.obj -mf
wcc386 lineuc.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=lineuc.obj -mf
wcc386 link.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=link.obj -mf
wcc386 link1a.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=link1a.obj -mf
wcc386 link3.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=link3.obj -mf
wcc386 link3a.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=link3a.obj -mf
wcc386 linkov.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=linkov.obj -mf
wcc386 mkfnam.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=mkfnam.obj -mf
wcc386 pstafn.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=pstafn.obj -mf
wcc386 pstfn.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=pstfn.obj -mf
wcc386 skpspc.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=skpspc.obj -mf
wcc386 strequ.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=strequ.obj -mf
wcc386 wrerr.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=wrerr.obj -mf
wcc386 version.c -w4 -e25 -zq -os -of+ -s -6s -bt=nt -fo=version.obj -mf
wrc version.rc -bt=nt -dWIN32 -d_WIN32 -d__NT__ -q -ad -r -fo=version.res

wlink name ..\..\Bin\link sys nt op m op maxe=25 op q op symf @make.txt

del *.obj
del *.map
del *.res

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
del ..\..\Bin\plm80.exe
del *.obj

cls
wcc386 accessor.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 adninf.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 alloc.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 allocs.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 backup.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 cpytil.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 creati.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 data.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 debug.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 endcom.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 fatal.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 fatlio.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 fi.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 findmi.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 fiscop.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 io.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 itoa.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 lookup.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 lstinf.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 lstlin.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 lstsup.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 main.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 main0.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 main1.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 main2.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 main3.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf -d9
wcc386 main4.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 main5.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 main6.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 page.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 pdata4.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 pdata6.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm0a.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm0b.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm0e.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm0f.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm0g.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm1a.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm1b.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm1c.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm1d.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm1e.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm1f.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2a.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2b.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2c.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2d.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2e.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2f.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2g.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm2h.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm3a.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm3b.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm4a.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm4b.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm4c.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plm6a.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plma.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plmb.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 plmfile.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 prints.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 putlst.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 strcmp.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 util.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 version.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wcc386 wrclst.c -w4 -e25 -zq -os -of+ -6s -bt=nt -fo=.obj -mf
wrc version.rc -bt=nt -dWIN32 -d_WIN32 -d__NT__ -q -ad -r -fo=version.res

wlink name ..\..\Bin\plm80 sys nt op m op maxe=25 op q op symf @make.txt

del *.obj
del *.map
del *.res

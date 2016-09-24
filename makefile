# MAKEFILE f. FleetStreet


# directory containing the Squish MsgAPI

MSGAPIDIR=\msgapi32

# Compiler and Linker Options for main EXE file

COPT=/Ti /Gn /Gm /Ss /Si /Q /Wdcleffgotiniobsparprorearettruusecnd- /C
LOPT="/NOD /M:full /L /NOE /pmtype:pm /packd"

.SUFFIXES:

RCOMP=RC -r -n
RBIND=RC -x -n

!IFDEF DEBUG
CC=icc $(COPT)
LOPT="/DE /NOD /M:full /L /NOE /pmtype:pm /packd"
!ELSE
CC=icc $(COPT) /Oc
!ENDIF

# Compiler and Linker Options for DLL files

COPTDLL=/Ti /Q /Wdcleffgotiniobsparprorearettruusecnd- /Gi /Gm /Ge- /G5 /Gn /C
LOPTDLL="/L /M:full /NOD /PACKD /NOLOGO"

!IFDEF DEBUG
CCDLL=icc $(COPTDLL)
LOPTDLL="/DE /L /M:full /NOD /PACKD /NOLOGO"
!ELSE
CCDLL=icc $(COPTDLL) /Oc
!ENDIF

ALL: obj\fltrun.lib \
     obj\fltdump.lib \
     obj\fltutil.lib \
     obj\flthmsg.lib \
     obj\fltjam.lib \
     obj\fltctls.lib \
     obj\fltprint.lib \
     obj\fltv7.lib \
     bin\fltrun.dll \
     bin\fltdump.dll \
     bin\fltutil.dll \
     bin\fltaman.dll \
     bin\fltjam.dll \
     bin\flthmsg.dll \
     obj\clrsel.res \
     bin\fltctls.dll \
     bin\fltprint.dll \
     bin\fltv7.dll \
     obj\dialog.res  \
     bin\fltcf_sq.dll \
     bin\fltcf_fe.dll \
     bin\fltcf_hp.dll \
     bin\fltcf_im.dll \
#     bin\fltcf_ge.dll \
     bin\fltcf_lo.dll \
     bin\fltcf_fm.dll \
     bin\fltcf_wm.dll \
     bin\fltcf_tm.dll \
     bin\fltcf_np.dll \
     bin\fltcf_bb.dll \
     bin\fltcf_sg.dll \
     bin\testcfg.exe \
     obj\main.res    \
     bin\FLTSTRT.EXE \
     obj\fleetlng.dll \
     english \
     italian \
     swedish \
     german \
     bin\fleetcom.exe \
     obj\install.res \
     bin\install.exe \
     bin\buildinstall.exe

CLEAN:
 -@del obj\*.obj
 -@del obj\*.res
 -@del obj\*.lng
 -@del obj\*.inf
 -@del obj\*.hlp
 -@del obj\*.lib
 -@del bin\*.exe
 -@del bin\*.dll
 -@del bin\*.inf
 -@del bin\*.hlp
 -@del bin\*.map

BINARY: fleet.zip

fleet.zip: install.fil \
           bin\install.exe \
           docs\readme.eng \
           docs\whatsnew.eng \
           docs\file_id.diz
 @eautil bin\install.exe nul /s
 @eautil docs\file_id.diz nul /s
 @eautil docs\readme.eng  nul /s
 @eautil docs\whatsnew.eng nul /s
 @zip -j -9 $@ $**
 @del install.fil

install.fil : files.lst          \
              bin\*.exe          \
              bin\*.dll          \
              obj\*.hlp          \
              obj\*.lng          \
              obj\*.inf          \
              scripts\*.frx      \
              docs\readme.txt    \
              docs\readme.eng    \
              docs\whatsnew.txt  \
              docs\whatsnew.eng  \
              docs\lizenz.txt    \
              docs\license.eng
 @bin\buildinstall %s

SOURCE: fsrc.zip

fsrc.zip:
 @echo *.c *.h *.def *.rc makefile files.lst addons\* areaman\* bbtoss\* >pack.lst
 @echo controls\* devkit\* docs\* dump\* fastecho\* fleetcom\* >>pack.lst
 @echo FltV7\* FMail\* handlemsg\* hpt\* imail\* install\* jamapi\* >>pack.lst
 @echo Languages\* Languages\german\* Languages\english\* Languages\italian\* Languages\swedish\* >>pack.lst
 @echo Lora\* NWPM\* printmsg\* Resources\* >>pack.lst
 @echo runtime\* scripts\* shotgun\* SOS\* squish_cfg\* TerMail\* >>pack.lst
 @echo testcfg\* tosstpl\* util\* wmail\* >>pack.lst
 @echo obj bin >>pack.lst
 @zip -9o -@ $@ <pack.lst
 @del pack.lst

# ----------------------------------------------------------------------------

bin\fltrun.dll: \
    obj\fltrun.obj \
    runtime\fltrun.def
 @echo Linking $@ ...
 @icc /Q /B"/L /M /packd /PACKC" /Fm$*.map /Fe$@ $**

obj\fltrun.lib: runtime\fltrun.def
 @implib /NOI /nologo $@ %s
 @ILIB /NOLOGO $@ +\vacpp\lib\cppom30o.lib , ,

obj\fltrun.obj: runtime\fltrun.c
 @echo Compiling %s ...
 @icc /Ti /Q /Gm /Ge- /C /Fo$@ %s

# ----------------------------------------------------------------------------

obj\fltdump.lib: dump\fltdump.def
 @implib /nologo $@ %s

bin\fltdump.dll: obj\expt.obj \
                 obj\fltrun.lib \
                 dump\fltdump.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\expt.obj: dump\expt.c \
              version.h \
              main.h \
              dump\expt.h \
              dump\pmassert.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

obj\FLTUTIL.LIB: util\fltutil.def
 @implib /nologo $@ %s

bin\FLTUTIL.DLL:  obj\FLTUTIL.OBJ \
                  obj\cnrutil.obj \
                  obj\crc32.obj \
                  obj\approx.obj \
                  obj\addrcnv.obj \
                  obj\fltrun.lib \
                  util\fltutil.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**

obj\CRC32.obj: util\CRC32.C
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\MATCH.obj: util\MATCH.C \
               util\match.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\addrcnv.obj: util\addrcnv.c \
                 main.h \
                 msgheader.h \
                 util\addrcnv.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\approx.obj: util\approx.c
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\cnrutil.obj: util\cnrutil.c
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fltutil.obj: util\fltutil.c
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

obj\fltaman.lib: areaman\fltaman.def
 @implib /nologo $@ %s

bin\fltaman.dll: obj\areaman.obj \
                 obj\folderman.obj \
                 areaman\fltaman.def \
                 obj\fltrun.lib
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\areaman.obj: areaman\areaman.c \
                 main.h \
                 structs.h \
                 areaman\areaman.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\folderman.obj: areaman\folderman.c \
                   areaman\folderman.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

obj\fltjam.lib: jamapi\jamapi.def
 @implib /nologo $@ %s

bin\fltjam.dll: obj\jamfetch.obj obj\jamhinfo.obj obj\jamlock.obj \
                obj\jamlread.obj obj\jammbini.obj obj\jamstore.obj\
                obj\jamsys.obj   obj\jamindex.obj obj\jamapi.obj \
                jamapi\jamapi.def \
                obj\fltrun.lib \
                obj\fltutil.lib \
                obj\flthmsg.lib
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**

obj\JAMFETCH.obj: jamapi\JAMFETCH.C \
                  jamapi\jammb.h \
                  jamapi\jam.h \
                  jamapi\jamsys.h \
                  jamapi\jamprot.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\JAMHINFO.obj: jamapi\JAMHINFO.C \
                  jamapi\jammb.h \
                  jamapi\jam.h \
                  jamapi\jamsys.h \
                  jamapi\jamprot.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\JAMLOCK.obj: jamapi\JAMLOCK.C \
                 jamapi\jammb.h \
                 jamapi\jam.h \
                 jamapi\jamsys.h \
                 jamapi\jamprot.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\JAMLREAD.obj: jamapi\JAMLREAD.C \
                  jamapi\jammb.h \
                  jamapi\jam.h \
                  jamapi\jamsys.h \
                  jamapi\jamprot.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\JAMMBINI.obj: jamapi\JAMMBINI.C \
                  jamapi\jammb.h \
                  jamapi\jam.h \
                  jamapi\jamsys.h \
                  jamapi\jamprot.h \
                  util\crc32.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\JAMSTORE.obj: jamapi\JAMSTORE.C \
                  jamapi\jammb.h \
                  jamapi\jam.h \
                  jamapi\jamsys.h \
                  jamapi\jamprot.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\JAMSYS.obj: jamapi\JAMSYS.C \
                jamapi\jammb.h \
                jamapi\jam.h \
                jamapi\jamsys.h \
                jamapi\jamprot.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\jamapi.obj: jamapi\jamapi.c \
                main.h \
                structs.h \
                msgheader.h \
                areaman\areaman.h \
                handlemsg\handlemsg.h \
                handlemsg\kludgeapi.h \
                util\crc32.h \
                util\addrcnv.h \
                jamapi\jammb.h \
                jamapi\jam.h \
                jamapi\jamsys.h \
                jamapi\jamprot.h \
                jamapi\jamindex.h \
                jamapi\jamapi.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\jamindex.obj: jamapi\jamindex.c \
                  main.h \
                  structs.h \
                  msgheader.h \
                  jamapi\jammb.h \
                  jamapi\jam.h \
                  jamapi\jamsys.h \
                  jamapi\jamprot.h \
                  jamapi\jamindex.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

obj\flthmsg.lib: handlemsg\handlemsg.def
 @implib /nologo $@ %s

bin\FLTHMSG.DLL:  obj\handlemsg.OBJ \
                  obj\handletemplate.OBJ \
                  obj\charset.obj \
                  obj\kludgeapi.obj \
                  obj\squishapi.obj \
                  obj\ftsapi.obj \
                  obj\common.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltjam.lib \
                  obj\fltutil.lib \
                  handlemsg\handlemsg.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $(MSGAPIDIR)\lib\dll32.lib $**

obj\charset.obj: handlemsg\charset.c \
                 handlemsg\charset.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\common.obj: handlemsg\common.c \
                main.h \
                structs.h \
                msgheader.h \
                util\addrcnv.h \
                areaman\areaman.h \
                handlemsg\handlemsg.h \
                handlemsg\kludgeapi.h \
                handlemsg\ftsapi.h \
                handlemsg\common.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\ftsapi.obj: handlemsg\ftsapi.c \
                main.h \
                messages.h \
                structs.h \
                msgheader.h \
                areaman\areaman.h \
                handlemsg\handlemsg.h \
                handlemsg\kludgeapi.h \
                handlemsg\common.h \
                handlemsg\ftsapi.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\handlemsg.obj: handlemsg\handlemsg.c \
                   main.h \
                   version.h \
                   structs.h \
                   areaman\areaman.h \
                   msgheader.h \
                   util\fltutil.h \
                   util\addrcnv.h \
                   handlemsg\handlemsg.h \
                   handlemsg\handletemplate.h \
                   handlemsg\charset.h \
                   handlemsg\kludgeapi.h \
                   handlemsg\squishapi.h \
                   handlemsg\ftsapi.h \
                   jamapi\jamapi.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\handletemplate.obj: handlemsg\handletemplate.c \
                        main.h \
                        structs.h \
                        msgheader.h \
                        areaman\areaman.h \
                        util\addrcnv.h \
                        handlemsg\handlemsg.h \
                        handlemsg\handletemplate.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\kludgeapi.obj: handlemsg\kludgeapi.c \
                   main.h \
                   msgheader.h \
                   handlemsg\kludgeapi.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\squishapi.obj: handlemsg\squishapi.c \
                   main.h \
                   structs.h \
                   msgheader.h \
                   util\addrcnv.h \
                   areaman\areaman.h \
                   handlemsg\handlemsg.h \
                   handlemsg\kludgeapi.h \
                   handlemsg\common.h \
                   handlemsg\squishapi.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

obj\fltctls.lib: controls\fltctls.def
 @implib /nologo $@ %s

bin\fltctls.dll: \
    obj\colorwheel.obj obj\colorselect.obj obj\editwin.OBJ \
    obj\fontdisp.obj obj\listbox.obj obj\attrselect.obj obj\msgviewer.OBJ \
    obj\reflow.OBJ obj\util.OBJ obj\mlist.obj obj\toolbar.obj obj\statline.OBJ \
    obj\fltrun.lib \
    obj\fltutil.lib \
    obj\flthmsg.lib \
    obj\fltdump.lib \
    controls\fltctls.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /B"/NOE" /Fe$@ /Fm$*.map os2386.lib $**
 @rc -x obj\clrsel.res $@

obj\clrsel.res: controls\clrsel.rc \
                controls\wheel.bmp
 @$(RCOMP) %s $@

obj\attrselect.obj: controls\attrselect.c \
                    controls\attrselect.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\test.obj: controls\clrwtest.c \
              controls\clrwhl.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\colorselect.obj: controls\colorselect.c \
                     controls\clrwhl.h \
                     controls\clrsel.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\colorwheel.obj: controls\colorwheel.c \
                    controls\colorwheel.h \
                    controls\clrwhl.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\editwin.obj: controls\editwin.c \
                 main.h \
                 resids.h \
                 nodedrag.h \
                 controls\editwin.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fontdisp.obj: controls\fontdisp.c \
                  controls\fontdisp.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\listbox.obj: controls\listbox.c \
                 main.h \
                 util\fltutil.h \
                 dump\pmassert.h \
                 controls\listboxprv.h \
                 controls\listbox.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\mlist.obj: controls\mlist.c \
               main.h \
               msgheader.h \
               structs.h \
               areaman\areaman.h \
               handlemsg\handlemsg.h \
               dump\pmassert.h \
               controls\mlist.h \
               controls\mlistprv.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\msgviewer.obj: controls\msgviewer.c \
                   main.h \
                   resids.h \
                   controls\editwin.h \
                   util\fltutil.h \
                   dump\pmassert.h \
                   controls\viewer_int.h \
                   controls\util.h \
                   controls\reflow.h \
                   controls\msgviewer.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\reflow.obj: controls\reflow.c \
                controls\viewer_int.h \
                controls\util.h \
                controls\reflow.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\statline.obj: controls\statline.c \
                  dump\pmassert.h \
                  controls\statline.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\toolbar.obj: controls\toolbar.c \
                 dump\pmassert.h \
                 controls\toolbar.h \
                 controls\toolbar_int.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\util.obj: controls\util.c \
              controls\viewer_int.h \
              controls\util.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

obj\fltprint.lib: printmsg\fltprint.def
 @implib /nologo $@ %s

bin\fltprint.dll: \
     obj\printmsg.obj \
     obj\setup.obj \
     obj\fltrun.lib \
     obj\fltaman.lib \
     obj\flthmsg.lib \
     obj\fltutil.lib \
     printmsg\fltprint.def
 @echo Linking $@ ...
 @icc /q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**

obj\SETUP.obj: printmsg\SETUP.C \
               main.h \
               structs.h \
               printsetup.h \
               printmsg\setup.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ /Ss %s

obj\printmsg.obj: printmsg\printmsg.c \
                  main.h \
                  resids.h \
                  structs.h \
                  msgheader.h \
                  areaman\areaman.h \
                  handlemsg\handlemsg.h \
                  printsetup.h \
                  util\addrcnv.h \
                  printmsg\printmsg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ /Ss %s

# ----------------------------------------------------------------------------

obj\fltv7.lib: fltv7\fltv7.def
 @implib /nologo $@ %s

bin\FLTV7.DLL: obj\fltv7.OBJ \
               obj\fltrun.lib \
               obj\fltutil.lib \
               fltv7\fltv7.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**

obj\fltv7.obj: fltv7\fltv7.c \
               fltv7\vers7.h \
               main.h \
               structs.h \
               msgheader.h \
               util\addrcnv.h \
               fltv7\fltv7.h \
               fltv7\fltv7structs.h \
               fltv7\v7browse.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_sq.dll: obj\read_sq_cfg.obj \
                  obj\parsecfg.obj \
                  obj\echoman_sq.obj \
                  obj\fltrun.lib \
                  obj\flthmsg.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  squish_cfg\fltcf_sq.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**
 @$(RBIND) obj\dialog.res $@

obj\dialog.res: squish_cfg\dialog.rc \
                squish_cfg\dialog.dlg \
                squish_cfg\echoman_sq.h
 @$(RCOMP) %s $@

obj\echoman_sq.obj: squish_cfg\echoman_sq.c \
                    devkit\echoman.h \
                    util\fltutil.h \
                    util\crc32.h \
                    squish_cfg\echoman_sq.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\parsecfg.obj: squish_cfg\parsecfg.c \
                  main.h \
                  msgheader.h \
                  structs.h \
                  areaman\areaman.h \
                  cfgfile_interface.h \
                  util\fltutil.h \
                  util\addrcnv.h \
                  handlemsg\handlemsg.h \
                  squish_cfg\parsecfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\read_sq_cfg.obj: squish_cfg\read_sq_cfg.c \
                     main.h \
                     structs.h \
                     msgheader.h \
                     areaman\areaman.h \
                     handlemsg\handlemsg.h \
                     cfgfile_interface.h \
                     squish_cfg\read_sq_cfg.h \
                     squish_cfg\parsecfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\FLTCF_FE.dll: obj\read_fe_cfg.obj \
                  obj\fe_common.obj \
                  obj\fe_revision4.obj   \
                  obj\fe_revision5.obj   \
                  obj\fe_revision6.obj   \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  fastecho\FLTCF_FE.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**

obj\READ_FE_CFG.obj: fastecho\READ_FE_CFG.C \
                     main.h \
                     structs.h \
                     areaman\areaman.h \
                     cfgfile_interface.h \
                     fastecho\read_fe_cfg.h \
                     fastecho\revision4.h \
                     fastecho\revision5.h \
                     fastecho\revision6.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fe_REVISION4.obj: fastecho\REVISION4.C \
                      main.h \
                      structs.h \
                      fastecho\fecfg130.h \
                      areaman\areaman.h \
                      cfgfile_interface.h \
                      msgheader.h \
                      util\addrcnv.h \
                      fastecho\common.h \
                      fastecho\revision4.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fe_REVISION5.obj: fastecho\REVISION5.C \
                      main.h \
                      structs.h \
                      fastecho\fecfg141.h \
                      areaman\areaman.h \
                      cfgfile_interface.h \
                      msgheader.h \
                      util\addrcnv.h \
                      fastecho\common.h \
                      fastecho\revision5.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fe_revision6.obj: fastecho\revision6.c \
                      main.h \
                      structs.h \
                      fastecho\fecfg142.h \
                      areaman\areaman.h \
                      cfgfile_interface.h \
                      msgheader.h \
                      util\addrcnv.h \
                      fastecho\common.h \
                      fastecho\revision6.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fe_common.obj: fastecho\common.c \
                   main.h \
                   structs.h \
                   msgheader.h \
                   areaman\areaman.h \
                   util\addrcnv.h \
                   fastecho\common.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\FLTCF_HP.dll: obj\hpt_cfg.obj \
                  obj\hptparse.obj \
                  obj\flthmsg.lib \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  hpt\FLTCF_hp.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**

obj\hpt_cfg.obj:  hpt\hpt_cfg.c \
                  hpt\hpt_cfg.h \
                  hpt\hptparse.h \
                  main.h \
                  structs.h \
                  areaman\areaman.h \
                  cfgfile_interface.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\hptparse.obj: hpt\hptparse.C \
                  main.h \
                  msgheader.h \
                  structs.h \
                  areaman\areaman.h \
                  cfgfile_interface.h \
                  util\fltutil.h \
                  util\addrcnv.h \
                  handlemsg\handlemsg.h \
                  hpt\hpt_cfg.h \
                  hpt\hptparse.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\FLTCF_IM.dll: obj\im_cfg.obj \
                  obj\im_revision4.obj \
                  obj\im_revision5.obj \
#                  obj\im_revision6.obj \
                  obj\im_common.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  imail\FLTCF_IM.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPTDLL) /Fe$@ /Fm$*.map os2386.lib $**

obj\im_common.obj: imail\common.c \
                   main.h \
                   structs.h \
                   msgheader.h \
                   areaman\areaman.h \
                   util\fltutil.h \
                   util\addrcnv.h \
                   imail\common.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\im_cfg.obj: imail\im_cfg.c \
                main.h \
                structs.h \
                msgheader.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                imail\common.h \
                imail\revision4.h \
                imail\revision5.h \
#                imail\revision6.h \
                imail\im_cfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\im_revision4.obj: imail\revision4.c \
                      main.h \
                      structs.h \
                      msgheader.h \
                      areaman\areaman.h \
                      cfgfile_interface.h \
                      util\fltutil.h \
                      util\addrcnv.h \
                      imail\im_struc.h \
                      imail\revision4.h \
                      imail\common.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\im_revision5.obj: imail\revision5.c \
                      main.h \
                      structs.h \
                      msgheader.h \
                      areaman\areaman.h \
                      cfgfile_interface.h \
                      util\fltutil.h \
                      util\addrcnv.h \
                      imail\imail170.h \
                      imail\revision5.h \
                      imail\common.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# IMail >= 1.86: source not included
#
#obj\im_revision6.obj: imail\revision6.c \
#                      main.h \
#                      structs.h \
#                      msgheader.h \
#                      areaman\areaman.h \
#                      cfgfile_interface.h \
#                      util\fltutil.h \
#                      util\addrcnv.h \
#                      imail\imail185.h \
#                      imail\revision6.h \
#                      imail\common.h
# @echo Compiling %s ...
# @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

# GEcho: source not included
#
#bin\fltcf_ge.dll: obj\ge_cfg.obj \
#                  obj\ge_revision2.obj \
#                  obj\fltrun.lib \
#                  obj\fltaman.lib \
#                  obj\fltutil.lib \
#                  gecho\fltcf_ge.def
# @echo Linking $@ ...
# @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**
#
#obj\ge_cfg.obj: gecho\ge_cfg.c \
#                main.h \
#                structs.h \
#                areaman\areaman.h \
#                cfgfile_interface.h \
#                gecho\revision2.h \
#                gecho\ge_cfg.h
# @echo Compiling %s ...
# @$(CCDLL) /Fo$@ %s
#
#obj\ge_revision2.obj: gecho\revision2.c \
#                      main.h \
#                      structs.h \
#                      msgheader.h \
#                      areaman\areaman.h \
#                      cfgfile_interface.h \
#                      util\fltutil.h \
#                      util\addrcnv.h \
#                      gecho\gestruct.h
# @echo Compiling %s ...
# @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_lo.dll: obj\lo_cfg.obj \
                  obj\lo_revision3.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  lora\fltcf_lo.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\lo_cfg.obj: lora\lo_cfg.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                lora\revision3.h \
                lora\lo_cfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\lo_revision3.obj: lora\revision3.c \
                      main.h \
                      structs.h \
                      msgheader.h \
                      areaman\areaman.h \
                      cfgfile_interface.h \
                      util\fltutil.h \
                      util\addrcnv.h \
                      lora\lora.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_fm.dll: obj\fm_cfg.obj \
                  obj\fm_revision1.obj \
                  obj\fm_revision11.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  fmail\fltcf_fm.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\fm_cfg.obj: fmail\fm_cfg.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                fmail\revision1.h \
                fmail\revision11.h \
                fmail\fm_cfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fm_revision1.obj: fmail\revision1.c \
                      fmail\revision1.h \
                      main.h \
                      structs.h \
                      msgheader.h \
                      areaman\areaman.h \
                      cfgfile_interface.h \
                      util\fltutil.h \
                      util\addrcnv.h \
                      fmail\fmstruct.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\fm_revision11.obj: fmail\revision11.c \
                       fmail\revision11.h \
                       main.h \
                       structs.h \
                       msgheader.h \
                       areaman\areaman.h \
                       cfgfile_interface.h \
                       util\fltutil.h \
                       util\addrcnv.h \
                       fmail\fmstr110.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_wm.dll: obj\wm_cfg.obj \
                  obj\wm_revision12.obj \
                  obj\pas2c.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  wmail\fltcf_wm.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\pas2c.obj: wmail\pas2c.c
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\wm_revision12.obj: wmail\revision12.c \
                       main.h \
                       structs.h \
                       msgheader.h \
                       areaman\areaman.h \
                       cfgfile_interface.h \
                       util\fltutil.h \
                       util\addrcnv.h \
                       wmail\pas2c.h \
                       wmail\wmstruct.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\wm_cfg.obj: wmail\wm_cfg.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                wmail\revision12.h \
                wmail\wm_cfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_tm.dll: obj\tm_cfg.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  obj\flthmsg.lib \
                  termail\fltcf_tm.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\tm_cfg.obj: termail\tm_cfg.c \
                main.h \
                structs.h \
                msgheader.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                util\fltutil.h \
                util\addrcnv.h \
                handlemsg\handlemsg.h \
                termail\tm_cfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_np.dll: obj\np_cfg.obj \
                  obj\rev099.obj \
                  obj\rev100.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  nwpm\fltcf_np.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\np_cfg.obj: nwpm\np_cfg.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                util\fltutil.h \
                msgheader.h \
                nwpm\np_cfg.h \
                nwpm\rev099.h \
                nwpm\rev100.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\rev099.obj: nwpm\rev099.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                util\fltutil.h \
                msgheader.h \
                util\addrcnv.h \
                nwpm\rev099.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

obj\rev100.obj: nwpm\rev100.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                util\fltutil.h \
                msgheader.h \
                util\addrcnv.h \
                nwpm\rev100.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_bb.dll: obj\bb_cfg.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  bbtoss\fltcf_bb.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\bb_cfg.obj: bbtoss\bb_cfg.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                util\fltutil.h \
                msgheader.h \
                util\addrcnv.h \
                bbtoss\bb_cfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fltcf_sg.dll: obj\sg_cfg.obj \
                  obj\pas2c.obj \
                  obj\fltrun.lib \
                  obj\fltaman.lib \
                  obj\fltutil.lib \
                  shotgun\fltcf_sg.def
 @echo Linking $@ ...
 @icc /Q /Fe$@ /B$(LOPTDLL) /Fm$*.map os2386.lib $**

obj\sg_cfg.obj: shotgun\sg_cfg.c \
                main.h \
                structs.h \
                areaman\areaman.h \
                cfgfile_interface.h \
                util\fltutil.h \
                msgheader.h \
                util\addrcnv.h \
                shotgun\shotgun.h \
                shotgun\pas2c.h \
                shotgun\sg_cfg.h
 @echo Compiling %s ...
 @$(CCDLL) /Fo$@ %s

# ----------------------------------------------------------------------------

bin\testcfg.exe: obj\tcfgmain.obj \
                 testcfg\main.def
 @echo Linking $@ ...
 @icc /Q /B"/DE /NOLOGO" /Fe$@ /Fm$*.map $**

obj\tcfgmain.obj: testcfg\main.c \
                  main.h \
                  structs.h \
                  areaman\areaman.h \
                  cfgfile_interface.h
 @echo Compiling %s ...
 @icc /Ti /Gm /Tdc /Ss /Si /Q /Wdcleffgotiniobsparprorearettruuse /C /Fo$@ %s

# ----------------------------------------------------------------------------

obj\fleetlng.dll: \
    obj\fleetlng.obj \
    languages\fleetlng.def \
    obj\fltrun.lib
 @echo Linking $@ ...
 @icc /Q /B"/NOE /m /packd /nologo" /Fe$@ /Fm$*.map $**

obj\fleetlng.obj: languages\fleetlng.c
 @echo Compiling %s ...
 @icc /O /Q /Rn /Ge- /Gs /C /Fo$@ %s

# ----------------------------------------------------------------------------

bin\fleetcom.exe: obj\fleetcom.obj \
                  obj\fltrun.lib \
                  fleetcom\fleetcom.def
 @echo Linking $@ ...
 @icc /Q /B"/L /packd /nologo /m /NOD" /Fe$@ /Fm$*.map os2386.lib $**

obj\fleetcom.obj: fleetcom\fleetcom.c \
                  asciitable.h \
                  version.h
 @echo Compiling %s ...
 @icc /Gm /Gn /O /Oi- /Ti /Q /Wdcleffgotobsordparprorearetuse /Si /C /Fo$@ %s

# ----------------------------------------------------------------------------

bin\INSTALL.EXE: obj\INSTALL.OBJ \
                 install\INSTALL.DEF
 @echo Linking $@ ...
 @icc /Q /B"/M /l /NOE /pmtype:pm /packd /PACKC /nologo" /Fe$@ /Fm$*.map $**
 @$(RBIND) obj\INSTALL.RES bin\INSTALL.EXE

obj\INSTALL.OBJ: install\INSTALL.C \
                 install\INSTALL.H
 @echo Compiling %s ...
 @icc /O /Ti /Gm /Tdc /Ss /Si /Q /Wdcleffgotiniobsparprorearettruusecnd- /C /Fo$@ %s

obj\INSTALL.RES: install\INSTALL.RC \
                 install\INSTALL.DLG \
                 install\INSTALL.H \
                 resources\logo2.bmp \
                 resources\install.ico
 @$(RCOMP) %s $@

# ----------------------------------------------------------------------------

bin\buildinstall.exe: obj\buildinstall.obj \
                      install\buildinstall.def
 @echo Linking $@ ...
 @ICC /Q /B"/l /exepack /packd /packc /m /nologo" /Fe$@ /Fm$*.map $**

obj\buildinstall.obj: install\buildinstall.c
 @echo Compiling %s ...
 @ICC /Ti /Oc /C /Q /Wdcleffgotiniobsparprorearettruusecnd- /Fo$@ %s

# ----------------------------------------------------------------------------

english: obj\english.LNG \
         obj\english.INF \
         obj\english.HLP

obj\english.LNG: obj\english.res \
                 obj\fleetlng.dll
 @copy obj\fleetlng.dll $@
 @$(RBIND) %s $@
 @copy $@ bin\fleetlng.dll

obj\english.res: languages\english\main.rc \
                 languages\english\main.dlg \
                 dialogids.h \
                 resids.h \
                 setupdlg.h \
                 areasettings.h
 @$(RCOMP) %s $@

obj\english.hlp: languages\english\*.ips \
                 languages\help.names
 @set IPFCARTWORK=resources
 @IPFC languages\english\main.ips $@ /s
 @copy $@ bin\fltstrt.hlp

obj\english.INF: languages\english\USERDOC.IPF
 @set IPFCARTWORK=resources
 @IPFC %s $@ /s /inf
 @copy $@ bin\fltstrt.inf

# ----------------------------------------------------------------------------

german: obj\german.LNG \
        obj\german.INF \
        obj\german.HLP

obj\german.LNG: obj\german.res \
                 obj\fleetlng.dll
 @copy obj\fleetlng.dll $@
 @$(RBIND) %s $@
 @copy $@ bin\fleetlng.dll

obj\german.res: languages\german\main.rc \
                 languages\german\main.dlg \
                 dialogids.h \
                 resids.h \
                 setupdlg.h \
                 areasettings.h
 @$(RCOMP) %s $@

obj\german.hlp: languages\german\*.ips \
                languages\help.names
 @set IPFCARTWORK=resources
 @IPFC languages\german\main.ips $@ /s
 @copy $@ bin\fltstrt.hlp

obj\german.INF: languages\german\USERDOC.IPF
 @set IPFCARTWORK=resources
 @IPFC %s $@ /s /inf
 @copy $@ bin\fltstrt.inf

# ----------------------------------------------------------------------------

italian: obj\italian.LNG \
         obj\italian.INF \
         obj\italian.HLP

obj\italian.LNG: obj\italian.res \
                 obj\fleetlng.dll
 @copy obj\fleetlng.dll $@
 @$(RBIND) %s $@
 @copy $@ bin\fleetlng.dll

obj\italian.res: languages\italian\main.rc \
                 languages\italian\main.dlg \
                 dialogids.h \
                 resids.h \
                 setupdlg.h \
                 areasettings.h
 @$(RCOMP) %s $@

obj\italian.hlp: languages\italian\*.ips \
                 languages\help.names
 @set IPFCARTWORK=resources
 @IPFC languages\italian\main.ips $@ /s
 @copy $@ bin\fltstrt.hlp

obj\italian.INF: languages\italian\USERDOC.IPF
 @set IPFCARTWORK=resources
 @IPFC %s $@ /s /inf
 @copy $@ bin\fltstrt.inf

# ----------------------------------------------------------------------------

swedish: obj\swedish.LNG \
         obj\swedish.INF \
         obj\swedish.HLP

obj\swedish.LNG: obj\swedish.res \
                 obj\fleetlng.dll
 @copy obj\fleetlng.dll $@
 @$(RBIND) %s $@
 @copy $@ bin\fleetlng.dll

obj\swedish.res: languages\swedish\main.rc \
                 languages\swedish\main.dlg \
                 dialogids.h \
                 resids.h \
                 setupdlg.h \
                 areasettings.h
 @$(RCOMP) %s $@

obj\swedish.hlp: languages\swedish\*.ips \
                 languages\help.names
 @set IPFCARTWORK=resources
 @IPFC languages\swedish\main.ips $@ /s
 @copy $@ bin\fltstrt.hlp

languages\help.names: help.h
 @languages\def2name %s $@

obj\swedish.INF: languages\swedish\USERDOC.IPF
 @set IPFCARTWORK=resources
 @IPFC %s $@ /s /inf
 @copy $@ bin\fltstrt.inf

# ----------------------------------------------------------------------------
# @@

bin\FLTSTRT.EXE:  \
  obj\attachcheck.obj \
  obj\areadlg.OBJ \
  obj\areascan.obj \
  obj\areasettings.OBJ \
  obj\arealistsettings.OBJ \
  obj\cclist.OBJ \
  obj\ccmanage.obj \
  obj\cfgfile_interface.obj \
  obj\dialogs.OBJ \
  obj\echomanager.obj \
  obj\finddlg.OBJ \
  obj\findexec.obj \
  obj\init.OBJ \
  obj\lookups.obj \
  obj\main.OBJ \
  obj\mainwindow.OBJ \
  obj\markmanage.obj \
  obj\msglist.OBJ \
  obj\nickmanage.obj \
  obj\nicknames.obj \
  obj\nlbrowser.obj \
  obj\pipeserv.OBJ \
  obj\printsetup.obj \
  obj\request.OBJ \
  obj\request_manage.OBJ \
  obj\rexxexec.obj \
  obj\rxfolder.obj \
  obj\savemsg.OBJ \
  obj\secwin.obj \
  obj\setupdlg.OBJ \
  obj\templatedlg.OBJ \
  obj\threadlist.OBJ \
  obj\threadlistsettings.OBJ \
  obj\toolbarconfig.obj \
  obj\utility.OBJ \
  obj\fltaman.lib \
  obj\fltrun.lib \
  obj\fltctls.lib \
  obj\fltv7.lib \
  obj\flthmsg.lib \
  obj\fltprint.lib \
  obj\fltutil.lib \
  obj\fltdump.lib \
  main.def
 @echo Linking $@ ...
 @icc /Q /B$(LOPT) /Fe$@ /Fm$*.map os2386.lib rexx.lib $**
 @$(RBIND) obj\main.RES $@

obj\main.RES: main.rc     \
              resids.h    \
              dialogids.h \
              setupdlg.h  \
              areadlg.h   \
              help.rc     \
              resources\*
 @$(RCOMP) %s $@

obj\areadlg.obj: areadlg.c \
                 main.h \
                 resids.h \
                 messages.h \
                 structs.h \
                 areaman\areaman.h \
                 areaman\folderman.h \
                 msgheader.h \
                 dialogids.h \
                 setupdlg.h \
                 dialogs.h \
                 handlemsg\handlemsg.h \
                 utility.h \
                 msglist.h \
                 util\fltutil.h \
                 arealistsettings.h \
                 areasettings.h \
                 areascan.h \
                 areadlg.h \
                 areadrag.h \
                 folderdrag.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\arealistsettings.obj: arealistsettings.c \
                          main.h \
                          resids.h \
                          structs.h \
                          msgheader.h \
                          areaman\areaman.h \
                          dialogids.h \
                          areadlg.h \
                          utility.h \
                          controls\clrsel.h \
                          arealistsettings.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\areascan.obj: areascan.c \
                  main.h \
                  messages.h \
                  structs.h \
                  msgheader.h \
                  areaman\areaman.h \
                  handlemsg\handlemsg.h \
                  dump\expt.h \
                  areascan.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\areasettings.obj: areasettings.c \
                      main.h \
                      resids.h \
                      messages.h \
                      structs.h \
                      msgheader.h \
                      areaman\areaman.h \
                      dialogids.h \
                      setupdlg.h \
                      areadlg.h \
                      utility.h \
                      controls\editwin.h \
                      controls\attrselect.h \
                      handlemsg\handlemsg.h \
                      areasettings.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\attachcheck.obj: attachcheck.c \
                     main.h \
                     resids.h \
                     structs.h \
                     msgheader.h \
                     areaman\areaman.h \
                     dialogids.h \
                     utility.h \
                     attachcheck.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\cclist.obj: cclist.c \
                main.h \
                resids.h \
                messages.h \
                structs.h \
                msgheader.h \
                areaman\areaman.h \
                dialogids.h \
                controls\editwin.h \
                utility.h \
                cclist.h \
                fltv7\fltv7.h \
                nodedrag.h \
                lookups.h \
                util\fltutil.h \
                util\addrcnv.h \
                ccmanage.h \
                nickmanage.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\ccmanage.obj: ccmanage.c \
                  main.h \
                  structs.h \
                  ccmanage.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\cfgfile_interface.obj: cfgfile_interface.c \
                           main.h \
                           structs.h \
                           areaman\areaman.h \
                           cfgfile_interface.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\dialogs.obj: dialogs.c \
                 main.h \
                 resids.h \
                 messages.h \
                 structs.h \
                 msgheader.h \
                 areaman\areaman.h \
                 dialogs.h \
                 dialogids.h \
                 init.h \
                 handlemsg\handlemsg.h \
                 handlemsg\kludgeapi.h \
                 controls\editwin.h \
                 utility.h \
                 fltv7\fltv7.h \
                 lookups.h \
                 nickmanage.h \
                 dump\expt.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\echomanager.obj: echomanager.c \
                     main.h \
                     resids.h \
                     messages.h \
                     structs.h \
                     msgheader.h \
                     areaman\areaman.h \
                     utility.h \
                     dialogids.h \
                     handlemsg\handlemsg.h \
                     handlemsg\kludgeapi.h \
                     areadlg.h \
                     setupdlg.h \
                     savemsg.h \
                     devkit\echoman.h \
                     util\fltutil.h \
                     util\addrcnv.h \
                     dump\expt.h \
                     echomanager.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\finddlg.obj: finddlg.c \
                 main.h \
                 resids.h \
                 messages.h \
                 structs.h \
                 msgheader.h \
                 areaman\areaman.h \
                 dialogids.h \
                 handlemsg\handlemsg.h \
                 util\fltutil.h \
                 finddlg.h \
                 utility.h \
                 areadlg.h \
                 findexec.h \
                 markmanage.h \
                 msglist.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\findexec.obj: findexec.c \
                  main.h \
                  messages.h \
                  structs.h \
                  msgheader.h \
                  areaman\areaman.h \
                  handlemsg\handlemsg.h \
                  handlemsg\kludgeapi.h \
                  finddlg.h \
                  util\approx.h \
                  util\match.h \
                  markmanage.h \
                  util\fltutil.h \
                  utility.h \
                  dump\expt.h \
                  findexec.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\init.obj: init.c \
              main.h \
              resids.h \
              dialogids.h \
              structs.h \
              msgheader.h \
              areaman\areaman.h \
              areaman\folderman.h \
              init.h \
              utility.h \
              msglist.h \
              handlemsg\handlemsg.h \
              ccmanage.h \
              finddlg.h \
              markmanage.h \
              nlbrowser.h \
              cfgfile_interface.h \
              nickmanage.h \
              echomanager.h \
              templatedlg.h \
              printsetup.h \
              toolbarconfig.h \
              request_manage.h \
              dump\expt.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\lookups.obj: lookups.c \
                 main.h \
                 resids.h \
                 structs.h \
                 msgheader.h \
                 fltv7\fltv7.h \
                 dialogids.h \
                 areaman\areaman.h \
                 utility.h \
                 util\addrcnv.h \
                 lookups.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\main.obj: main.c \
              main.h \
              version.h \
              structs.h \
              msgheader.h \
              areaman\areaman.h \
              areaman\folderman.h \
              dialogs.h \
              dialogids.h \
              resids.h \
              messages.h \
              controls\editwin.h \
              controls\statline.h \
              controls\msgviewer.h \
              controls\toolbar.h \
              handlemsg\handlemsg.h \
              handlemsg\kludgeapi.h \
              init.h \
              help.h \
              areadlg.h \
              areascan.h \
              mainwindow.h \
              msglist.h \
              setupdlg.h \
              finddlg.h \
              pipeserv.h \
              utility.h \
              cclist.h \
              templatedlg.h \
              savemsg.h \
              fltv7\fltv7.h \
              lookups.h \
              attachcheck.h \
              threadlist.h \
              secwin.h \
              ccmanage.h \
              rxfolder.h \
              rexxexec.h \
              markmanage.h \
              nlbrowser.h \
              nickmanage.h \
              nicknames.h \
              echomanager.h \
              printsetup.h \
              toolbarconfig.h \
              request.h \
              request_manage.h \
              dump\expt.h \
              dump\pmassert.h \
              util\addrcnv.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\mainwindow.obj: mainwindow.c \
                    main.h \
                    structs.h \
                    msgheader.h \
                    areaman\areaman.h \
                    dialogids.h \
                    resids.h \
                    messages.h \
                    handlemsg\handlemsg.h \
                    handlemsg\kludgeapi.h \
                    utility.h \
                    mainwindow.h \
                    controls\editwin.h \
                    controls\msgviewer.h \
                    controls\statline.h \
                    finddlg.h \
                    markmanage.h \
                    controls\toolbar.h \
                    toolbarconfig.h \
                    dump\pmassert.h \
                    util\addrcnv.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\markmanage.obj: markmanage.c \
                    main.h \
                    structs.h \
                    msgheader.h \
                    markmanage.h \
                    areaman\areaman.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\msglist.obj: msglist.c \
                 main.h \
                 resids.h \
                 messages.h \
                 structs.h \
                 msgheader.h \
                 areaman\areaman.h \
                 dialogids.h \
                 handlemsg\handlemsg.h \
                 handlemsg\kludgeapi.h \
                 msglist.h \
                 utility.h \
                 areadlg.h \
                 savemsg.h \
                 controls\mlist.h \
                 controls\clrsel.h \
                 printsetup.h \
                 printmsg\printmsg.h \
                 dump\expt.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\nickmanage.obj: nickmanage.c \
                    main.h \
                    structs.h \
                    nickmanage.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\nicknames.obj: nicknames.c \
                   main.h \
                   resids.h \
                   messages.h \
                   structs.h \
                   msgheader.h \
                   areaman\areaman.h \
                   mainwindow.h \
                   fltv7\fltv7.h \
                   lookups.h \
                   dialogids.h \
                   utility.h \
                   handlemsg\handlemsg.h \
                   util\addrcnv.h \
                   controls\attrselect.h \
                   nickmanage.h \
                   nicknames.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\nlbrowser.obj: nlbrowser.c \
                   main.h \
                   resids.h \
                   messages.h \
                   structs.h \
                   msgheader.h \
                   areaman\areaman.h \
                   dialogids.h \
                   fltv7\fltv7.h \
                   fltv7\fltv7structs.h \
                   fltv7\v7browse.h \
                   utility.h \
                   lookups.h \
                   nodedrag.h \
                   nlbrowser.h \
                   dump\expt.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\pipeserv.obj: pipeserv.c \
                  main.h \
                  version.h \
                  structs.h \
                  msgheader.h \
                  areaman\areaman.h \
                  pipeserv.h \
                  handlemsg\handlemsg.h \
                  handlemsg\kludgeapi.h \
                  utility.h \
                  areadlg.h \
                  areascan.h \
                  asciitable.h \
                  dump\expt.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\printsetup.obj: printsetup.c \
                    main.h \
                    structs.h \
                    msgheader.h \
                    areaman\areaman.h \
                    dialogids.h \
                    resids.h \
                    utility.h \
                    controls\fontdisp.h \
                    printsetup.h \
                    printmsg\setup.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\request.obj: request.c \
                 main.h \
                 structs.h \
                 dialogids.h \
                 messages.h \
                 resids.h \
                 msgheader.h \
                 areaman\areaman.h \
                 utility.h \
                 dialogs.h \
                 setupdlg.h \
                 finddlg.h \
                 controls\editwin.h \
                 controls\listbox.h \
                 fltv7\fltv7.h \
                 lookups.h \
                 request_manage.h \
                 request.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\request_manage.obj: request_manage.c \
                        main.h \
                        structs.h \
                        dump\expt.h \
                        request_manage.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\rexxexec.obj: rexxexec.c \
                  main.h \
                  resids.h \
                  messages.h \
                  structs.h \
                  msgheader.h \
                  areaman\areaman.h \
                  dialogids.h \
                  rexxexec.h \
                  mainwindow.h \
                  savemsg.h \
                  utility.h \
                  fltv7\fltv7.h \
                  lookups.h \
                  ccmanage.h \
                  controls\editwin.h \
                  handlemsg\handlemsg.h \
                  handlemsg\kludgeapi.h \
                  util\addrcnv.h \
                  dump\expt.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\rxfolder.obj: rxfolder.c \
                  main.h \
                  resids.h \
                  messages.h \
                  structs.h \
                  msgheader.h \
                  dialogids.h \
                  areaman\areaman.h \
                  utility.h \
                  setupdlg.h \
                  rxfolder.h \
                  controls\editwin.h \
                  util\fltutil.h \
                  rexxexec.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\savemsg.obj: savemsg.c \
                 main.h \
                 resids.h \
                 messages.h \
                 structs.h \
                 msgheader.h \
                 areaman\areaman.h \
                 savemsg.h \
                 handlemsg\handlemsg.h \
                 handlemsg\kludgeapi.h \
                 utility.h \
                 dialogids.h \
                 util\addrcnv.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\secwin.obj: secwin.c \
                main.h \
                msgheader.h \
                controls\msgviewer.h \
                controls\statline.h \
                controls\clrsel.h \
                controls\mlist.h \
                controls\toolbar.h \
                controls\fontdisp.h \
                controls\editwin.h \
                controls\listbox.h \
                controls\attrselect.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\setupdlg.obj: setupdlg.c \
                  main.h \
                  resids.h \
                  messages.h \
                  structs.h \
                  msgheader.h \
                  areaman\areaman.h \
                  dialogs.h \
                  dialogids.h \
                  init.h \
                  setupdlg.h \
                  utility.h \
                  controls\editwin.h \
                  controls\msgviewer.h \
                  controls\clrsel.h \
                  util\fltutil.h \
                  util\addrcnv.h \
                  cfgfile_interface.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\templatedlg.obj: templatedlg.c \
                     main.h \
                     resids.h \
                     messages.h \
                     structs.h \
                     msgheader.h \
                     areaman\areaman.h \
                     dialogids.h \
                     templatedlg.h \
                     setupdlg.h \
                     utility.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\threadlist.obj: threadlist.c \
                    main.h \
                    resids.h \
                    messages.h \
                    structs.h \
                    msgheader.h \
                    dialogids.h \
                    areaman\areaman.h \
                    handlemsg\handlemsg.h \
                    threadlist.h \
                    msglist.h \
                    utility.h \
                    threadlistsettings.h \
                    areadlg.h \
                    dialogs.h \
                    util\fltutil.h \
                    dump\expt.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\threadlistsettings.obj: threadlistsettings.c \
                            main.h \
                            structs.h \
                            msgheader.h \
                            areaman\areaman.h \
                            areadlg.h \
                            controls\clrsel.h \
                            dialogids.h \
                            resids.h \
                            utility.h \
                            threadlistsettings.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\toolbarconfig.obj: toolbarconfig.c \
                       main.h \
                       structs.h \
                       msgheader.h \
                       areaman\areaman.h \
                       utility.h \
                       resids.h \
                       dialogids.h \
                       controls\toolbar.h \
                       toolbarconfig.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s

obj\utility.obj: utility.c \
                 main.h \
                 resids.h \
                 messages.h \
                 structs.h \
                 msgheader.h \
                 areaman\areaman.h \
                 dialogs.h \
                 dialogids.h \
                 utility.h \
                 setupdlg.h \
                 handlemsg\handlemsg.h \
                 util\fltutil.h \
                 util\addrcnv.h \
                 mainwindow.h
 @echo Compiling %s ...
 @$(CC) /Fo$@ %s


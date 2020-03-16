dmake -f makefile.emx
del *.o
del msgapie.lib
ren msgapi.lib msgapie.lib
del msgapi.a
nmake -nologo -f makefile.ibm
del *.obj
del msgapic.lib
ren msgapi.lib msgapic.lib
del msgapi.bak
make -f makefile.bcc
del *.obj
del msgapib.lib
ren msgapi.lib msgapib.lib
del msgapi.bak
wmake -f makefile.wat
del *.obj
del msgapiw.lib
ren msgapi.lib msgapiw.lib
del msgapi.bak

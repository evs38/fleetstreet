make -f makefile.dos
del *.obj
del msgapid.lib
ren msgapi.lib msgapid.lib
del msgapi.bak

wmake -u -f makefile.w16
del *.obj
del msgapiw6.lib
ren msgapi.lib msgapiw6.lib
del msgapi.bak

make -f makefile.tcc
del *.obj
del msgapit.lib
ren msgapi.lib msgapit.lib
del msgapi.bak



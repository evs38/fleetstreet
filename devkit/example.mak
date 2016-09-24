# Makefile for EXAMPLE.C and CSet++ 2.1

# Compiler options:

# /Q          Be quiet
# /C          Compile only
# /Ge-        Build DLL
# /G4         Optimize for 486
# /Gm         Multi-Thread library
# /O          Optimize

COPT = /Q /C /Ge- /G4 /Gm /O

# Linker options:

# /Q          Be quiet

LINKOPT = /Q /Fe"example.dll"

ALL: example.dll \
     devkit.inf

example.dll: example.obj \
             example.def \
             example.mak
  icc @<<
  $(LINKOPT)
  example.obj
  example.def
<<

example.obj: example.c \
             echoman.h \
             example.mak
  icc $(COPT) example.c


devkit.inf: devkit.ipf \
            example.mak
  ipfc devkit.ipf /inf /s


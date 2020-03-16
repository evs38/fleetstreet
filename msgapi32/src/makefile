#############################################################################
##                  MsgAPI Makefile for OS/2 2.11                          ##
#############################################################################

# This makefile only creates a library for static linking, not a 32-bit DLL.
# The whole 32-bit port of MsgAPI to OS/2 compiles, and has undergone minimal
# testing, but do *not* assume it is free of bugs.

# The assumed shell is \OS2\CMD.EXE.  This makefile remains untested.

# Please uncomment the appropriate section(s) below before invoking your MAKE
# utility.  Refer to the accompanying documentation (../readme) for further
# information.


#####[ Compiler-specific options ]###########################################


# Borland C++ 1.01

# CC=	bcc
# CFLAGS=	-K -w -w-sig
# CCDEFS=	
# COPT=	-O2 \
# 	# -G -d -k- -r # -4
# O=	obj
# L=	lib
# LIBA=	tlib
# LIBP=	+
# LIBZ=	


# EMX 0.8h (GCC 2.5.8) - OMF

# CC=	gcc -Zomf -Zsys -s
# CFLAGS=	-Wall
# CCDEFS=	-D__FLAT__
# COPT=	-O4 \
# 	# -fomit-frame-pointer -fstrength-reduce -funroll-loops # -m486
# O=	obj
# L=	lib
# LIBA=	emxomfar crv
# LIBP=	
# LIBZ=	


# EMX 0.8h (GCC 2.5.8) - Unix

CC=	gcc -s
CFLAGS=	-Wall
CCDEFS=	-D__FLAT__
COPT=	-O4 \
	# -fomit-frame-pointer -fstrength-reduce -funroll-loops # -m486
O=	o
L=	a
LIBA=	ar ruv
LIBP=	
LIBZ=	


# IBM C Set++ 2.0

# CC=	icc
# CFLAGS=	-Sp1 -W2
# CCDEFS=	-D__FLAT__
# COPT=	-O \
# 	# (optimisations here) # (cpu-specific stuff here)
# O=	obj
# L=	lib
# LIBA=	lib
# LIBP=	+
# LIBZ=	;


#####[ General options ]#####################################################

CDEFS=	-DOS_2 -D__386__ $(CCDEFS)
I=	../include
TARGET=	msgapi.$(L)
RM=	rm -f

# CDEFS=	-DOS_2 -D__386__ $(CCDEFS)
# I=	.
# TARGET=	bc_mapif.$(L)	# Borland C++
# TARGET=	cs_mapif.$(L)	# C Set++
# TARGET=	em_mapif.$(L)	# EMX
# RM=	erase


#####[ Recipes ]#############################################################

OBJS=	1stchar.$(O) msgapi.$(O) api_sdm.$(O) api_sq.$(O) cvtdate.$(O) \
	date2bin.$(O) dosdate.$(O) fexist.$(O) ffind.$(O) flush.$(O) \
	months.$(O) parsenn.$(O) qksort.$(O) strftim.$(O) stristr.$(O) \
	strocpy.$(O) trail.$(O) weekday.$(O)

.c.$(O):
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -I$(I) -c $<

$(TARGET):	$(OBJS)
	@if exist $@ $(RM) $@
	@for %z in ( $(OBJS) ) do @$(LIBA) $@ $(LIBP)%z $(LIBZ)

clean:
	$(RM) $(TARGET) $(OBJS) *.bak 2>/dev/nul


#####[ Dependencies ]########################################################

1stchar.$(O):	1stchar.c
msgapi.$(O):	msgapi.c
api_sdm.$(O):	api_sdm.c
api_sq.$(O):	api_sq.c
cvtdate.$(O):	cvtdate.c
date2bin.$(O):	date2bin.c
dosdate.$(O):	dosdate.c
fexist.$(O):	fexist.c
ffind.$(O):	ffind.c
flush.$(O):	flush.c
months.$(O):	months.c
parsenn.$(O):	parsenn.c
qksort.$(O):	qksort.c
strftim.$(O):	strftim.c
stristr.$(O):	stristr.c
strocpy.$(O):	strocpy.c
trail.$(O):	trail.c
weekday.$(O):	weekday.c

CC=	tcc
CFLAGS=	-K -w -w-sig -ml
CDEFS=	-DMSDOS -DUSEDIRFORTC
COPT= -O2
	# -G -d -k- -r # -4

I=	../include
TARGET=	msgapi.lib
RM=	erase

OBJS=	1stchar.obj msgapi.obj api_sdm.obj api_sq.obj cvtdate.obj \
	date2bin.obj dosdate.obj fexist.obj ffind.obj flush.obj \
	months.obj parsenn.obj qksort.obj strftim.obj stristr.obj \
	strocpy.obj trail.obj weekday.obj flusha.obj sqasm.obj


# Thunderbirds are go!


.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -I$(I) -c $<
    
.asm.obj:
    tasm /ml $<    

$(TARGET):	$(OBJS)
	@if exist $@ $(RM) $@
    tlib $@ +1stchar.obj
    tlib $@ +msgapi.obj
    tlib $@ +api_sdm.obj
    tlib $@ +api_sq.obj
    tlib $@ +cvtdate.obj
    tlib $@ +date2bin.obj
    tlib $@ +dosdate.obj
    tlib $@ +fexist.obj
    tlib $@ +ffind.obj
    tlib $@ +flush.obj
    tlib $@ +months.obj
    tlib $@ +parsenn.obj
    tlib $@ +qksort.obj
    tlib $@ +strftim.obj
    tlib $@ +stristr.obj
    tlib $@ +strocpy.obj
    tlib $@ +trail.obj
    tlib $@ +weekday.obj
    tlib $@ +flusha.obj
    tlib $@ +sqasm.obj

clean:
	$(RM) msgapi.lib $(TARGET) $(OBJS) 2>/dev/nul


# Dependencies taken from SJD's generic makefile.

1stchar.obj:	1stchar.c
msgapi.obj:	msgapi.c
api_sdm.obj:	api_sdm.c
api_sq.obj:	api_sq.c
cvtdate.obj:	cvtdate.c
date2bin.obj:	date2bin.c
dosdate.obj:	dosdate.c
fexist.obj:	fexist.c
ffind.obj:	ffind.c
flush.obj:	flush.c
months.obj:	months.c
parsenn.obj:	parsenn.c
qksort.obj:	qksort.c
strftim.obj:	strftim.c
stristr.obj:	stristr.c
strocpy.obj:	strocpy.c
trail.obj:	trail.c
weekday.obj:	weekday.c

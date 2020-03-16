#############################################################################
##       MsgAPI Makefile for Watcom C 10.0 under OS/2 2.10                 ##
#############################################################################

CC=	wcl386
CFLAGS= -zq
CDEFS= -DOS_2 -D__386__ -D__FLAT__
COPT=	

I=..\include
TARGET=	msgapi.lib
RM=	erase

OBJS=	1stchar.obj msgapi.obj api_sdm.obj api_sq.obj cvtdate.obj &
	date2bin.obj dosdate.obj fexist.obj ffind.obj flush.obj &
	months.obj parsenn.obj qksort.obj strftim.obj stristr.obj &
	strocpy.obj trail.obj weekday.obj


# Thunderbirds are go!


.c.obj:
	$(CC) $(CFLAGS) $(CDEFS) $(COPT) -I$(I) -c $<

$(TARGET):	$(OBJS)
	@if exist $@ $(RM) $@
	@for %z in ( $(OBJS) ) do wlib -q $@ +%z

clean:
	$(RM) msgapi.lib $(TARGET) $(OBJS) 2>NUL


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

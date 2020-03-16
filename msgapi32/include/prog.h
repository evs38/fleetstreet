/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  General include file.  Lots of machine-dependant stuff here.           *
 *                                                                         *
 *  For complete details of the licensing restrictions, please refer to    *
 *  the licence agreement, which is published in its entirety in           *
 *  README.1ST.                                                            *
 *                                                                         *
 *  USE OF THIS FILE IS SUBJECT TO THE RESTRICTIONS CONTAINED IN THE       *
 *  MSGAPI LICENSING AGREEMENT.  IF YOU DO NOT FIND THE TEXT OF THIS       *
 *  AGREEMENT IN ANY OF THE AFOREMENTIONED FILES, OR IF YOU DO NOT HAVE    *
 *  THESE FILES, YOU SHOULD IMMEDIATELY CONTACT THE AUTHOR AT ONE OF THE   *
 *  ADDRESSES LISTED BELOW.  IN NO EVENT SHOULD YOU PROCEED TO USE THIS    *
 *  FILE WITHOUT HAVING ACCEPTED THE TERMS OF THE MSGAPI LICENSING         *
 *  AGREEMENT, OR SUCH OTHER AGREEMENT AS YOU ARE ABLE TO REACH WITH THE   *
 *  AUTHOR.                                                                *
 *                                                                         *
 *  You can contact the author at one of the address listed below:         *
 *                                                                         *
 *  Scott Dudley           FidoNet  1:249/106                              *
 *  777 Downing St.        Internet f106.n249.z1.fidonet.org               *
 *  Kingston, Ont.         BBS      (613) 389-8315   HST/14.4k, 24hrs      *
 *  Canada - K7M 5N3                                                       *
 *                                                                         *
 ***************************************************************************/

/* $Id: prog.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

#ifndef __PROG_H_DEFINED
#define __PROG_H_DEFINED

#include <stdio.h>
#include <time.h>
#include "compiler.h"
#include "typedefs.h"
#include "stamp.h"


#ifdef __FARDATA__
  #include "alc.h"

  #if !defined(ZREE)
    #define malloc(n)     farmalloc(n)
    #define calloc(n,u)   farcalloc(n,u)
    #define free(p)       farfree(p)
    #define realloc(p,n)  farrealloc(p,n)
  #endif

  #ifdef __TURBOC__
    #define coreleft()    farcoreleft()
  #endif
#else
  unsigned cdecl coreleft(void);
  unsigned long cdecl farcoreleft(void);
#endif


#if defined(__MSC__)

  #ifndef ZREE
    #define farmalloc(n)    _fmalloc(n)
    #define farfree(p)      _ffree(p)
    #define farrealloc(p,n) _frealloc(p,n)
    void far *farcalloc(int n,int m);

    #ifdef _MSC_VER
      #if _MSC_VER >= 600
        #define farcalloc(a,b) _fcalloc(a,b)
      #endif /* _MSC_VER >= 600 */
    #endif /* _MSC_VER */
  #endif /* ZREE */

  #define da_year year
  #define da_day day
  #define da_mon month

  #define ti_min minute
  #define ti_hour hour
  #define ti_hund hsecond
  #define ti_sec second

  #define getdate _dos_getdate
  #define gettime _dos_gettime

  #define NO_STRFTIME

  #define NO_MKTIME

#elif defined(__TURBOC__)

  #define dosdate_t date
  #define dostime_t time

  #if (__TURBOC__==0x0295)    /* TC++ includes a strftime() function */
    #define NO_STRFTIME
    #define NO_MKTIME
  #endif

#endif
 
#if (1) /* defined(ZREE) || defined(__FLAT__) */
    #undef farcalloc
    #undef farmalloc
    #undef farrealloc
    #undef farfree
    #undef _fmalloc

    #define farcalloc  calloc
    #define farmalloc  malloc
    #define farrealloc realloc
    #define farfree    free
    #define _fmalloc   malloc
#endif

#if !defined(__TURBOC__) && !defined(__BCOS2__)

  /* For ERRNO definitions */
  #define ENOTSAM EXDEV

  int _stdc fnsplit(const char *path,char *drive,char *dir,char *name,char *ext);
  int _stdc getcurdir(int drive, char *directory);

  int fossil_wherex(void);
  int fossil_wherey(void);
  void fossil_getxy(char *row, char *col);

  #define textattr(attr)
  #define getdisk()                  get_disk()
  #define setdisk(drive)             set_disk(drive)

  #define getvect(int)            _dos_getvect(int)
  #define setvect(int, func)      _dos_setvect(int, func)
  #define inportb(port)           inp(port)
  #define inport(port)            inpw(port)
  #define outportb(port, byte)    outp(port, byte)
  #define outport(port, byte)     outpw(port, byte)

#if 0
  #ifndef MK_FP
    #define MK_FP(seg, off)  (void far *)((unsigned long)(seg)<<16L | (off))
  #endif
#endif  
#endif

#ifdef __MSC__
  int _fast lock(int fh, long offset, long len);
  int _fast unlock(int fh, long offset, long len);
  #undef toupper
  extern unsigned char _MyUprTab[256];      /* see _ctype.c */
  #define toupper(c)  ((int)_MyUprTab[(c)])
#endif
 
#ifdef OS_2
  void _fast vbuf_flush(void);
  void SnSetPipeName(char *pipename);
  void SnWrite(char *str);

  #define  Start_Shadow()
  #define  End_Shadow()
#else
  void pascal Start_Shadow(void);
  void pascal End_Shadow(void);
#endif


#if !defined(offsetof) && !defined(_MSC_VER) && !defined(__TURBOC__)
#define offsetof(typename,var) (size_t)(&(((typename *)0)->var))
#endif

#ifdef __TURBOC__
#if __TURBOC__ <= 0x0200
#define offsetof(typename,var) (size_t)(&(((typename *)0)->var))
#endif
#endif


typedef long timer_t;

#define REGISTER

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#ifdef PATHLEN
#undef PATHLEN
#endif

#define LITTLE_ENDIAN           /* If compiling on a "back-words" (Intel)  *
                                 * Otherwise, #define BIG_ENDIAN           *//

#define PATH_DELIM       '\\'   /* Default separator for path specification */
#define _PRIVS_NUM         12   /* Maximum priv levels for Maximus         */
#define CHAR_BITS           8   /* Number of bits in a `char' variable     */
#define PATHLEN           120   /* Max. length of a path                   */
#define MAX_DRIVES         26   /* Maximum number of drives on system;     *
                                 * for MS-DOS, A through Z.  Used by       *
                                 * Save_Dir()...                           */

#define INTBIT_C        0x0001  /* Carry */
#define INTBIT_P        0x0004  /* Parity */
#define INTBIT_AUX      0x0010  /* Aux carry */
#define INTBIT_Z        0x0040  /* Zero flag */
#define INTBIT_SIG      0x0080  /* Sign flag */
#define INTBIT_TRC      0x0100  /* Trace flag */
#define INTBIT_INT      0x0200  /* Interrupt flag */
#define INTBIT_D        0x0400  /* Direction flag */
#define INTBIT_OVF      0x0800  /* Overflow flag */

#define ZONE_ALL  56685u
#define NET_ALL   56685u
#define NODE_ALL  56685u
#define POINT_ALL 56685u


#define THIS_YEAR "1991"
#define Hello(prog,desc,version,year) printf("\n" prog "  " desc ", Version %s.\nCopyright " year " by Scott J. Dudley of 1:249/106.  All rights reserved.\n\n",version)
#define shopen(path,access)   sopen(path,access,SH_DENYNONE,S_IREAD | S_IWRITE)
#define GTdate(s1, s2) (GEdate(s1, s2) && (s1)->ldate != (s2)->ldate)
#define carrier_flag          (prm.carrier_mask)
#define BitOff(a,x)           ((void)((a)[(x)/CHAR_BITS] &= ~(1 << ((x) % CHAR_BITS))))
#define BitOn(a,x)            ((void)((a)[(x)/CHAR_BITS] |= (1 << ((x) % CHAR_BITS))))
#define IsBit(a,x)            ((a)[(x)/CHAR_BITS] & (1 << ((x) % CHAR_BITS)))

#define lputs(handle,string)  write(handle,string,strlen(string))

#define dim(a)                (sizeof(a)/sizeof(a[0]))
#define eqstr(str1,str2)      (strcmp(str1,str2)==0)
#define eqstri(str1,str2)     (stricmp(str1,str2)==0)
#define eqstrn(str1,str2,n)   (strncmp(str1,str2,n)==0)
#define eqstrni(str1,str2,n)  (strnicmp(str1,str2,n)==0)
#define eqstrin(str1,str2,n)  eqstrni(str1,str2,n)
#define divby(num,div)        ((num % div)==0)
#define f_tolwr(c)            (_to_lwr[c])
#define f_toupr(c)            (_to_upr[c])

/* Macro to propercase MS-DOS filenames.  If your OS is case-dependent,     *
 * use "#define fancy_fn(s) (s)" instead.  Ditto for upper_fn().            */

#define fancy_fn(s)           fancy_str(s)
#define upper_fn(s)           strupr(s)

#ifndef updcrc
#define updcrc(cp, crc)       (crctab[((crc >> 8) & 255) ^ cp] ^ (crc << 8))
#endif


#ifndef max
#define max(a,b)              (((a) > (b)) ? (a) : (b))
#define min(a,b)              (((a) < (b)) ? (a) : (b))
#endif


/* Don't change this struct!  The code in win_pick.c and max_locl.c relies  *
 * on it as being the same as PLIST...                                      */

struct __priv
{
  char *name;
  int priv;
};



extern char _stdc months[][10];
extern char _stdc weekday[][10];

extern char _stdc months_ab[][4];
extern char _stdc weekday_ab[][4];

extern struct __priv _stdc _privs[];


#include "progprot.h"


#ifndef NO_STRFTIME
  /* If compiler doesn't include a strftime(), use our own */

  #include <time.h>
  #include <sys/types.h>

  size_t _stdc strftime(char *,size_t,const char *,const struct tm *);
#endif


#ifndef NO_MKTIME
  /* If compiler doesn't include a mktime(), use our own */

  #include <time.h>
  #include <sys/types.h>

  time_t _stdc mktime(struct tm * tm_ptr);
#endif

/* MS docs use both SH_DENYNONE and SH_DENYNO */

#if !defined(SH_DENYNONE) && defined(SH_DENYNO)
  #define SH_DENYNONE SH_DENYNO
#endif

#endif /* __PROG_H_DEFINED */


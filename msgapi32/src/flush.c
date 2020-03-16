/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Flush a file handle to disk                                            *
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

/* $Id: flush.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#include <io.h>
#include "prog.h"

#if (!defined(__IBMC__) && !defined(MSDOS))
#include <dos.h>
#endif

#if defined(OS_2)
#define INCL_NOPM
#include <os2.h>

#if defined(__386__) || defined(__FLAT__)
#undef DosBufReset
#define DosBufReset DosResetBuffer
#endif

void pascal far flush_handle2( int fh )
{
    DosBufReset((HFILE)fh);
}

#endif

/* This makes sure a file gets flushed to disk.  Thanks to Ray Duncan      *
 * for this tip in his _Advanced MS-DOS_ book.                             */

void _fast flush_handle(FILE *fp)
{

  fflush(fp);

#if defined(OS_2)
  DosBufReset(fileno(fp));
#elif defined(MSDOS) || defined(__MSDOS__) || defined(__TURBOC__)
  flush_handle2(fileno(fp));
#else
  {
    int nfd;

    if ((nfd=dup(fileno(fp))) != -1)
      close(nfd);
  }
#endif
}



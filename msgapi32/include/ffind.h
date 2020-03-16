/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  FFIND.C include file.                                                  *
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
 *  Thanks to Peter Fitzsimmons for this module.                           *
 *                                                                         *
 ***************************************************************************/

/* $Id: ffind.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

#ifndef __FFIND_H_
#define __FFIND_H_

#include "compiler.h"

#define FFIND struct ffind

struct ffind
{
#ifndef OS_2
    char reserved[21];
#endif

    char            ff_attrib;
    unsigned short  ff_ftime;
    unsigned short  ff_fdate;
    long            ff_fsize;
    char            ff_name[13];

#ifdef OS_2
#if defined(__386__) || defined(__FLAT__)
    unsigned long   hdir;       /* directory handle from DosFindFirst */
#else
    unsigned short  hdir;       /* directory handle from DosFindFirst */
#endif
#endif
};

FFIND * _fast FindOpen(char *filespec,unsigned short attribute);
FFIND * _fast FindInfo(char *filespec); /*PLF Thu  10-17-1991  18:03:09 */
int _fast FindNext(FFIND *ff);
void _fast FindClose(FFIND *ff);

/* The MS-DOS type attributes that can be stored in zipfile.file_attr_ext,
   only when ver_made_by==0.  There are a few more types, but none
   that we use for anything.                                       */

#define MSDOS_READONLY  0x01
#define MSDOS_HIDDEN    0x02
#define MSDOS_SYSTEM    0x04
#define MSDOS_VOLUME    0x08
#define MSDOS_SUBDIR    0x10
#define MSDOS_ARCHIVE   0x20
#define MSDOS_RSVD1     0x40
#define MSDOS_RSVD2     0x80

#endif


/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Include file to pick between MALLOC.H and ALLOC.H                      *
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

/* $Id: alc.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

#ifndef __ALC_H_DEFINED
#define __ALC_H_DEFINED

#include "compiler.h"

#if defined(__MSC__)
  #include <malloc.h>

  #ifdef __FARDATA__

  /* for some insane reason the turbo-c coreleft() function changes
   * it's return value based on the memory model.
   */

    unsigned long cdecl coreleft   (void);
  #else
    unsigned cdecl coreleft        (void);
  #endif

#elif defined(__TURBOC__) || defined(__BCOS2__)
  #include <alloc.h>
#elif defined(__EMX__) || defined(__IBMC__) || defined(__WATCOMC__)
  #include <malloc.h>
#else
  #error Which compiler are you using?
#endif

#ifdef __TURBOC__
#define halloc(x,y) ((char far *)farmalloc(x*y))
#define hfree(p)    farfree(p)
#endif

#endif /* __ALC_H_DEFINED */


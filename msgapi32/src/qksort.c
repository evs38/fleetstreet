/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Integer quicksort routine.                                             *
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
 *  Thanks to Thomas Plum and _Reliable_Data_Structure_In_C_               *
 *  for this routine.                                                      *
 *                                                                         *
 ***************************************************************************/

/* $Id: qksort.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#include <stdio.h>
#include "prog.h"

#define NUM sizeof(array)/sizeof(array[0])
#define SWAP(a,b,s) s=a; a=b; b=s;

static void _fast iqksort(int *p_lo,int *p_hi);

void _fast qksort(int a[],size_t n)
{
  if (n > 1)
    iqksort(a,&a[n-1]);
}



static void _fast iqksort(int *p_lo,int *p_hi)
{
  int  *p_mid=p_lo+(((int)(p_hi-p_lo))/2),
       *p_i,
       *p_lastlo,
       tmp;

  SWAP(*p_lo,*p_mid,tmp);

  p_lastlo=p_lo;

  for (p_i=p_lo+1;p_i <= p_hi;++p_i)
  {
    if (*p_lo > *p_i)
    {
      ++p_lastlo;
      SWAP(*p_lastlo,*p_i,tmp);
    }
  }

  SWAP(*p_lo,*p_lastlo,tmp);

  if (p_lo < p_lastlo && p_lo < p_lastlo-1)
    iqksort(p_lo,p_lastlo-1);

  if (p_lastlo+1 < p_hi)
    iqksort(p_lastlo+1,p_hi);
}


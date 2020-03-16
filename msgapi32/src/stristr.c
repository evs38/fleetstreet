/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Case-insensitive search-for-str-inside-another routine                 *
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

/* $Id: stristr.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "prog.h"

/* Code to handle chinese characters */

#define ISLEFT(c) ((c) > (byte)0x80 && (c) < (byte)0xff)
#define ISRIGHT(c) (((c) >= (byte)0x40 && (c) <= (byte)0x7e) || \
                    ((c) >= (byte)0xa1 && (c) <= (byte)0xfe))

static word near ischin(byte *buf)
{
  return (ISLEFT(buf[0]) && ISRIGHT(buf[1]));
}

char * _fast stristr(char *string,char *search)
{
  /* "register" keyword used to fix the brain-dead MSC (opti)mizer */

  word last_found=0;
  word strlen_search=strlen(search);
  byte l1, l2;
  word i;

  if (string)
  {
    while (*string)
    {
      /**** start chinese modifications *****/
      l1=(byte)(ischin(string) ? 2 : 1);
      l2=(byte)(ischin(search+last_found) ? 2 : 1);

      if (l1==l2)
        i=(l1==1) ? memicmp(string, search+last_found, l1) : 1;
      else i=1;
      
      if (!i)
        last_found += l1;
      /**** end chinese modifications *****/
      /* old code: if ((tolower(*string))==(tolower(search[last_found])))
                     last_found++;
      */
      else
      {
        if (last_found != 0)
        {
          last_found=0;
          continue;
        }
      }

      string += l1;

      if (last_found==strlen_search) return(string-last_found);
    }
  }

  return(NULL);
}


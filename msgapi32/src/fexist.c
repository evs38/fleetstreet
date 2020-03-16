/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  File-exist and directory-searching routines                            *
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

/* $Id: fexist.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "ffind.h"
#include "prog.h"

#if (!defined(__IBMC__) && !defined(MSDOS))
#include <dos.h>
#endif

/*
main()
{
  printf("asdfe=%d\n",direxist("c:\\asdfe"));
  printf("blank=%d\n",direxist("c:\\blank"));
  printf("tc=%d\n",direxist("c:\\tc"));
  printf("c:\\=%d\n",direxist("c:\\"));
  printf("d:\\=%d\n",direxist("d:\\"));
  printf("e:\\=%d\n",direxist("e:\\"));
  printf("f:\\=%d\n",direxist("f:\\"));
}
*/


int _fast fexist(char *filename)
{
  FFIND *ff;

  ff=FindOpen(filename,0);

  if (ff)
  {
    FindClose(ff);
    return TRUE;
  }
  else return FALSE;
}

long _fast fsize(char *filename)
{
  FFIND *ff;
  long ret=-1L;

  ff=FindOpen(filename,0);

  if (ff)
  {
    ret=ff->ff_fsize;
    FindClose(ff);
  }

  return ret;
}

#if defined(__MSDOS__)
int _fast direxist(char *directory)
{
  FFIND *ff;
  char *tempstr;
  int ret;

  if ((tempstr=(char *)malloc(strlen(directory)+5))==NULL)
    return FALSE;

  strcpy(tempstr, directory);

  Add_Trailing(tempstr,'\\');

  /* Root directory of any drive always exists! */

  if ((isalpha(tempstr[0]) && tempstr[1]==':' &&
      ((tempstr[2]=='\0') ||
       (tempstr[2]=='\\' || tempstr[2]=='/') && tempstr[3]=='\0')) ||
      eqstri(tempstr, "\\"))
  {
    ret=TRUE;
  }
  else
  {
    Strip_Trailing(tempstr, '\\');

    ff=FindOpen(tempstr, MSDOS_SUBDIR | MSDOS_HIDDEN | MSDOS_READONLY);

    ret=(ff != NULL && (ff->ff_attrib & MSDOS_SUBDIR));

    if (ff)
      FindClose(ff);
  }

  free(tempstr);
  return ret;

}

#elif defined(OS_2)
#include <io.h>
int _fast direxist(char *directory)
{
  int ret;
  char *tempstr;
  size_t l;

  if (NULL == (tempstr=(char *)strdup(directory)))
    return FALSE;

  /* Root directory of any drive always exists! */

  if ((isalpha(tempstr[0]) && tempstr[1]==':' &&
      (tempstr[2]=='\\' || tempstr[2]=='/') && !tempstr[3]) ||
      eqstr(tempstr, "\\"))
  {
    free(tempstr);
    return TRUE;
  }

  l = strlen(tempstr);
  if( tempstr[l-1] == '\\' || tempstr[l-1] == '/')
    tempstr[l-1] = 0;		/* remove trailing backslash */

  ret = !access(tempstr, 06);

  free(tempstr);
  return ret;
}

#else
  #error Define this f() for your OS.
#endif

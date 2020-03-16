/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Portable file-searching hooks                                          *
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
 *  Thanks go to Peter Fitzsimmons for these routines.                     *
 *                                                                         *
 ***************************************************************************/

/* $Id: ffind.c_v 1.0 1991/11/16 16:16:40 sjd Rel sjd $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#ifdef __TURBOC__
#include <dir.h>
#endif

#ifndef __IBMC__
#include <dos.h>
#endif

#include "ffind.h"

#ifdef OS_2
#define INCL_NOPM
#define INCL_DOS
#include <os2.h>
#if defined(__386__) || defined(__FLAT__)
#undef DosQPathInfo
#define DosQPathInfo(a,b,c,d,e)  DosQueryPathInfo(a,b,c,d)
#endif
#endif

/* FindOpen;  Use like MSDOS "find first" function,  except be sure to
 * release allocated system resources by caling FindClose() with the
 * handle returned by this function.
 *
 * returns:  NULL  -- file not found.
 *
 */

FFIND * _fast FindOpen(char *filespec, unsigned short attribute)
{
  FFIND *ff;

  ff=malloc(sizeof(FFIND));

  if (ff)
  {

#if defined(__TURBOC__)

    if (findfirst(filespec,(struct ffblk *)ff,attribute))
    {
      free(ff);
      ff=NULL;
    }

#elif defined(OS_2)

  #if defined(__386__) || defined(__FLAT__)
    ULONG  SearchCount = 1;
    FILEFINDBUF3 findbuf;
  #else
    USHORT SearchCount = 1;
    FILEFINDBUF findbuf;
  #endif

    ff->hdir=HDIR_CREATE;

    if (!DosFindFirst(filespec,&ff->hdir,attribute,&findbuf,
                      sizeof(findbuf),&SearchCount,1L))
    {
      ff->ff_attrib=(char)findbuf.attrFile;
      ff->ff_fsize=findbuf.cbFile;

      ff->ff_ftime=*((USHORT *)&findbuf.ftimeLastWrite);
      ff->ff_fdate=*((USHORT *)&findbuf.fdateLastWrite);

      strncpy(ff->ff_name,findbuf.achName,sizeof(ff->ff_name));
    }
    else
    {
      free(ff);
      ff = NULL;
    }

#elif defined(__MSC__) || defined(__WATCOMC__)

    if (_dos_findfirst(filespec,attribute,(struct find_t *)ff))
    {
      free(ff);
      ff=NULL;
    }
#else
#error "I don't know which compiler we're using!"
#endif
  }

  return ff;
}

/* FindNext:   returns 0 if next file was found, non-zero if it was not.
 *
 */

int _fast FindNext(FFIND *ff)
{
  int rc=-1;

  if (ff)
  {

#if defined(__TURBOC__)

    rc=findnext((struct ffblk *)ff);

#elif defined(OS_2)

  #if defined(__386__) || defined(__FLAT__)
    ULONG  SearchCount = 1;
    FILEFINDBUF3 findbuf;
  #else
    USHORT SearchCount = 1;
    FILEFINDBUF findbuf;
  #endif

    if(ff->hdir && !DosFindNext(ff->hdir,&findbuf,sizeof(findbuf),&SearchCount))
    {
      ff->ff_attrib=(char)findbuf.attrFile;
      ff->ff_ftime=*((USHORT *)&findbuf.ftimeLastWrite);
      ff->ff_fdate=*((USHORT *)&findbuf.fdateLastWrite);
      ff->ff_fsize=findbuf.cbFile;
      strncpy(ff->ff_name, findbuf.achName, sizeof(ff->ff_name));
      rc=0;
    }
#elif defined(__MSC__) || defined(__WATCOMC__)
    rc=_dos_findnext((struct find_t *)ff);
#else
#error "I don't know which compiler we're using!"
#endif
  }

  return rc;
}

/* FindClose: End a directory search.  Failure to call this function will
 * result in unclosed file handles (os/2),  and un-free()'d memory (dos/os2).
 *
 */
void _fast FindClose(FFIND *ff)
{
  if (ff)
  {
#ifdef OS_2
    if(ff->hdir)
        DosFindClose(ff->hdir);
#endif
    free(ff);
  }
}

/* This function was added because it is SIGNIFICANTLY faster under OS/2 to
 * call DosQPathInfo() rather than DosFindFirst(),   if all you are
 * intested in is getting a specific file's date/time/size.
 *
 *PLF Thu  10-17-1991  18:12:37
 */

FFIND * _fast FindInfo(char *filespec)
{
#ifndef OS_2
    return FindOpen(filespec, 0);
#else
    FFIND *ff;
    FILESTATUS fs;
    char *f;

    ff = malloc(sizeof(*ff));
    if(!ff)
        return NULL;

    memset(ff, 0, sizeof(*ff));
    if (!DosQPathInfo(filespec, FIL_STANDARD, (PBYTE)&fs, sizeof(fs), 0L))
    {
      ff->ff_attrib=(char)fs.attrFile;
      ff->ff_ftime=*((USHORT *)&fs.ftimeLastWrite);
      ff->ff_fdate=*((USHORT *)&fs.fdateLastWrite);
      ff->ff_fsize=fs.cbFile;


      f = strrchr(filespec, '\\');    /* isolate file name */
      if(!f)
          f = filespec;
      else
          f++;
      strncpy(ff->ff_name, f, sizeof(ff->ff_name));
    }
    else{
        free(ff);
        return NULL;
    }
    return ff;
#endif
}

#ifdef TEST_SHELL

#define TRUE 1
#define FALSE 0

int walk(char *path);

void main(int argc, char **argv)
{
    walk("\\");     /* start at root*/
}

/* this simple function assumes the path ALWAYS has an ending backslash */
walk(char *path)
{
    FFIND *ff;
    int done = FALSE;
    char full[66];

    strcpy(full, path);
    strcat(full, "*.*");
    if( ff = FindOpen(full, MSDOS_subdir) ){
        for(done = FALSE; !done; done = FindNext(ff)){
            if( (ff->ff_attrib & MSDOS_subdir) && (ff->ff_name[0] != '.') ){
                strcpy(full, path);
                strcat(full, ff->ff_name);
                puts(full);
                strcat(full, "\\");
                if( !walk(full) )
                    return(FALSE);
            }
        }
        FindClose(ff);
        return(TRUE);
    }
    else{
        puts("FindOpen() failed");
    }
    return(FALSE);
}

#endif


/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Main *.MSG include file                                                *
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

/* $Id: api_sdm.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

#ifndef __API_SDM_H_DEFINED
#define __API_SFM_H_DEFINED


#define MAX_SDM_CLEN  512   /* Maximum number of bytes which can be used    *
                             * for kludge lines at top of *.MSG-type        *
                             * messages.                                    */


struct _msgh
{
  MSG *sq;
  dword id;      /* Must always equal MSGH_ID */

  dword bytes_written;
  dword cur_pos;

  /* For *.MSG only! */

  sdword clen;
  byte *ctrl;
  sdword msg_len;
  sdword msgtxt_start;
  word zplen;
  int fd;
};


/*************************************************************************/
/* This junk is unique to *.MSG!       NO APPLICATIONS SHOULD USE THESE! */
/*************************************************************************/

struct _sdmdata
{
  byte base[80];
  
  unsigned *msgnum;   /* has to be of type 'int' for qksort() fn */
  word msgnum_len;
    
  dword hwm;
  word hwm_chgd;
  
  word msgs_open;
};




#endif /* __API_SDM_H_DEFINED */

int EXPENTRY WriteZPInfo(XMSG *msg, void (_stdc OS2LOADDS *wfunc)(byte *str), byte *kludges);


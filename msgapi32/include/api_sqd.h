/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Structure definitions of the SQD header                                *
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

/* $Id: api_sqd.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

struct _sqdata
{

  int sfd;                /* SquishFile handle */
  int ifd;                /* SquishIndex handle */

  byte base[80];          /* Base name for SquishFile */

  FOFS begin_frame;       /* Offset of first frame in file */
  FOFS last_frame;        /* Offset to last frame in file */
  FOFS free_frame;        /* Offset of first FREE frame in file */
  FOFS last_free_frame;   /* Offset of LAST free frame in file */
  FOFS end_frame;         /* Pointer to end of file */

  FOFS next_frame;
  FOFS prev_frame;
  FOFS cur_frame;

  dword uid;
  dword max_msg;
  dword skip_msg;
/*dword zero_ofs;*/
  word keep_days;
  
  byte flag;
  byte rsvd1;
  
  word sz_sqhdr;
  byte rsvd2;

  word len;              /* Old length of sqb structure                     */

  dword idxbuf_size;     /* Size of the allocated buffer                    */
  dword idxbuf_used;     /* # of bytes being used to hold messages          */
  dword idxbuf_write;    /* # of bytes we should write to index file        */
  dword idxbuf_delta;    /* Starting position from which the index has chhg */
  
  struct _sqbase delta; /* Copy of last-read sqbase, to determine changes   */
  word msgs_open;
  
  SQIDX far *idxbuf;
};




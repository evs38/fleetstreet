/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Main Squish-format header file                                         *
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

/* $Id: api_sq.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

#ifndef __API_SQ_H_DEFINED
#define __API_SQ_H_DEFINED

struct _sqhdr;
struct _sqidx;

typedef struct _sqidx SQIDX;
typedef struct _sqhdr SQHDR;
typedef long FOFS;


#define NULL_FRAME      ((FOFS)0L)
#define FRAME_normal    0x00
#define FRAME_free      0x01
#define FRAME_rle       0x02  /* not implemented */
#define FRAME_lzw       0x03  /* not implemented */

#define EVERYTHING      0xffffu

#define SFB_LEN         2048
#define IFB_LEN         1024

#define EXTRA_BUF       16

struct _sqhdr
{

  #define SQHDRID       0xafae4453UL

  dword id;             /* sqhdr.id must always equal SQHDRID */

  FOFS next_frame;
  FOFS prev_frame;

  dword frame_length;
  dword msg_length;
  dword clen;

  word frame_type;
  word rsvd;
};



struct _msgh
{
  MSG *sq;
  dword id;  /* Must always equal MSGH_ID */

  dword bytes_written;
  dword cur_pos;

  /* For SQUISH only! */

  dword cur_len;
  dword clen;
  dword msgnum;
  dword totlen;

  SQHDR *hdr;

  FOFS seek_frame;

  word mode;
};





struct _sqidx
{
  FOFS ofs;
/*  dword attr;*/
  UMSGID umsgid;
  dword hash;
/*  byte to[20];*/
};


/* Used for buffering index writes within API_SQ.C */
struct _bufidx
{
  struct _sqidx ix;
  dword idx_ofs;
};


struct _sqbase
{
  word len;               /* LENGTH OF THIS STRUCTURE! */           /*   0 */
  word rsvd1;             /* reserved */                            /*   2 */

  dword num_msg;          /* Number of messages in area */          /*   4 */
  dword high_msg;         /* Highest msg in area. Same as num_msg*/ /*   8 */
  dword skip_msg;         /* Skip killing first x msgs in area */   /*  12 */
  dword high_water;       /* Msg# (not umsgid) of HWM */            /*  16 */

  dword uid;              /* Number of the next UMSGID to use */    /*  20 */

  byte base[80];          /* Base name of SquishFile */             /*  24 */

  FOFS begin_frame;       /* Offset of first frame in file */       /* 104 */
  FOFS last_frame;        /* Offset to last frame in file */        /* 108 */
  FOFS free_frame;        /* Offset of first FREE frame in file */  /* 112 */
  FOFS last_free_frame;   /* Offset of last free frame in file */   /* 116 */
  FOFS end_frame;         /* Pointer to end of file */              /* 120 */

  dword max_msg;          /* Max # of msgs to keep in area */       /* 124 */
  word keep_days;         /* Max age of msgs in area (SQPack) */    /* 128 */
  word sz_sqhdr;          /* sizeof(SQHDR) */                       /* 130 */
  byte rsvd2[124];        /* Reserved by Squish for future use*/    /* 132 */

                                                             /* total: 256 */
};

#define SF_STATIC 0x0001  /* Perform static (not dynamic) renumbering */

#include "api_sqd.h"

#endif /* __API_SQ_H_DEFINED */


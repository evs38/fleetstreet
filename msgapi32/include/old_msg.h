/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Old *.MSG header                                                       *
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

/* $Id: old_msg.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

#ifndef __OLD_MSG_H_DEFINED
#define __OLD_MSG_H_DEFINED

struct _omsg
   {
      byte from[36];
      byte to[36];
      byte subj[72];
      byte date[20];       /* Obsolete/unused ASCII date information        */
/**/  word times;          /* FIDO<tm>: Number of times read                */
      sword dest;          /* Destination node                              */
      sword orig;          /* Origination node number                       */
/**/  word cost;           /* Unit cost charged to send the message         */

      sword orig_net;      /* Origination network number                    */
      sword dest_net;      /* Destination network number                    */

                           /* A TIMESTAMP is a 32-bit integer in the Unix   */
                           /* flavor (ie. the number of seconds since       */
                           /* January 1, 1970).  Timestamps in messages are */
                           /* always Greenwich Mean Time, never local time. */

      struct _stamp date_written;   /* When user wrote the msg              */
      struct _stamp date_arrived;   /* When msg arrived on-line             */

      word reply;          /* Current msg is a reply to this msg number     */
      word attr;           /* Attribute (behavior) of the message           */
      word up;             /* Next message in the thread                    */
   };

#endif


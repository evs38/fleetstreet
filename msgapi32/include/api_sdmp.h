/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Priivate *.MSG include file                                            *
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

/* $Id: api_sdmp.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

static sword EXPENTRY SdmCloseArea(MSG *mh);
static MSGH * EXPENTRY SdmOpenMsg(MSG *mh,word mode,dword msgnum);
static sword EXPENTRY SdmCloseMsg(MSGH *msgh);
static dword EXPENTRY SdmReadMsg(MSGH *msgh,XMSG *msg,dword offset,dword bytes,byte *text,dword clen,byte *ctxt);
static sword EXPENTRY SdmWriteMsg(MSGH *msgh,word append,XMSG *msg,byte *text,dword textlen,dword totlen,dword clen,byte *ctxt);
static sword EXPENTRY SdmKillMsg(MSG *mh,dword msgnum);
static sword EXPENTRY SdmLock(MSG *mh);
static sword EXPENTRY SdmUnlock(MSG *mh);
static sword EXPENTRY SdmSetCurPos(MSGH *msgh,dword pos);
static dword EXPENTRY SdmGetCurPos(MSGH *msgh);
static UMSGID EXPENTRY SdmMsgnToUid(MSG *mh,dword msgnum);
static dword EXPENTRY SdmUidToMsgn(MSG *mh,UMSGID umsgid,word type);
static dword EXPENTRY SdmGetHighWater(MSG *mh);
static sword EXPENTRY SdmSetHighWater(MSG *sq,dword hwm);
static dword EXPENTRY SdmGetTextLen(MSGH *msgh);
static dword EXPENTRY SdmGetCtrlLen(MSGH *msgh);

static void Convert_Fmsg_To_Xmsg(struct _omsg *fmsg,XMSG *msg,word def_zone);
static void Convert_Xmsg_To_Fmsg(XMSG *msg,struct _omsg *fmsg);
static void Init_Xmsg(XMSG *msg);
static sword near _SdmRescanArea(MSG *mh);
static sword near _Grab_Clen(MSGH *msgh);
static void _stdc OS2LOADDS WriteToFd(byte *str);
static void near Get_Binary_Date(struct _stamp *todate,struct _stamp *fromdate,byte *asciidate);


static int statfd; /* file handle for WriteToFd */
static byte *sd_msg="%s%u.msg";

/* Pointer to 'struct _sdmdata' so we can get Turbo Debugger to use         *
 * the _sdmdata structure...                                                */

static struct _sdmdata *_junksqd;

static struct _apifuncs sdm_funcs=
{
  SdmCloseArea,
  SdmOpenMsg,
  SdmCloseMsg,
  SdmReadMsg,
  SdmWriteMsg,
  SdmKillMsg,
  SdmLock,
  SdmUnlock,
  SdmSetCurPos,
  SdmGetCurPos,
  SdmMsgnToUid,
  SdmUidToMsgn,
  SdmGetHighWater,
  SdmSetHighWater,
  SdmGetTextLen,
  SdmGetCtrlLen,
};

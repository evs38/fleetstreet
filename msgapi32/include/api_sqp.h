/***************************************************************************
 *                                                                         *
 *  MSGAPI Source Code, Version 2.00                                       *
 *  Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.          *
 *                                                                         *
 *  Private include file for API_SQ.C                                      *
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

/* $Id: api_sqp.h_v 1.0 1991/11/16 16:16:51 sjd Rel sjd $ */

static sword EXPENTRY SquishCloseArea(MSG *sq);
static MSGH * EXPENTRY SquishOpenMsg(MSG *sq,word mode,dword msgnum);
static sword EXPENTRY SquishCloseMsg(MSGH *msgh);
static dword EXPENTRY SquishReadMsg(MSGH *msgh,XMSG *msg,dword offset,dword bytes,byte *text,dword clen,byte *ctxt);
static sword EXPENTRY SquishWriteMsg(MSGH *msgh,word append,XMSG *msg,byte *text,dword textlen,dword totlen,dword clen,byte *ctxt);
static sword EXPENTRY SquishKillMsg(MSG *sq,dword msgnum);
static sword EXPENTRY SquishLock(MSG *sq);
static sword EXPENTRY SquishUnlock(MSG *sq);
static sword EXPENTRY SquishSetCurPos(MSGH *msgh,dword pos);
static dword EXPENTRY SquishGetCurPos(MSGH *msgh);
static UMSGID EXPENTRY SquishMsgnToUid(MSG *sq,dword msgnum);
static dword EXPENTRY SquishUidToMsgn(MSG *sq,UMSGID umsgid,word type);
static dword EXPENTRY SquishGetHighWater(MSG *mh);
static sword EXPENTRY SquishSetHighWater(MSG *sq,dword hwm);
static dword EXPENTRY SquishGetTextLen(MSGH *msgh);
static dword EXPENTRY SquishGetCtrlLen(MSGH *msgh);
static sword MSGAPI _OpenSquish(MSG *sq,word *mode);
static SQHDR * MSGAPI _SquishGotoMsg(MSG *sq,dword msgnum,FOFS *seek_frame,SQIDX *idx,word updptrs);
static MSGH * _SquishOpenMsgRead(MSG *sq,word mode,dword msgnum);
static sword MSGAPI _SquishReadHeader(MSG *sq,dword ofs,SQHDR *hdr);
static sword MSGAPI _SquishWriteHeader(MSG *sq,dword ofs,SQHDR *hdr);
static sword MSGAPI _SquishUpdateHeaderNext(MSG *sq,dword ofs,SQHDR *hdr,dword newval);
static sword MSGAPI _SquishUpdateHeaderPrev(MSG *sq,dword ofs,SQHDR *hdr,dword newval);
static sword MSGAPI _SquishWriteSq(MSG *sq);
static sword MSGAPI _SquishUpdateSq(MSG *sq, word force);
static void MSGAPI Init_Hdr(SQHDR *sh);
static void SqbaseToSq(struct _sqbase *sqbase,MSG *sq);
static void SqToSqbase(MSG *sq,struct _sqbase *sqbase);
static sword near AddIndex(MSG *sq,SQIDX *ix,dword msgnum);
static sword near Add_To_Free_Chain(MSG *sq,FOFS killofs,SQHDR *killhdr);
static sword near _SquishReadIndex(MSG *sq);
static sword near _SquishWriteIndex(MSG *sq);
static sword near _SquishGetIdxFrame(MSG *sq,dword num,SQIDX *idx);
static void far * near farmemmove(void far *destin,const void far *source,unsigned n);
static void far * near farmemset(void far *s,int c,size_t length);
static int near _SquishLock(MSG *sq);
static void near _SquishUnlock(MSG *sq);
static sword near _SquishFindFree(MSG *sq, FOFS *this_frame, dword totlen,
                                  dword clen, SQHDR *freehdr,
                                  FOFS *last_frame, SQHDR *lhdr, MSGH *msgh);


#define fop_wpb (O_CREAT | O_TRUNC | O_RDWR | O_BINARY)
#define fop_rpb (O_RDWR | O_BINARY)

#define Sqd ((struct _sqdata *)(sq->apidata))
#define MsghSqd ((struct _sqdata *)(((struct _msgh far *)msgh)->sq->apidata))


static struct _apifuncs sq_funcs=
{
  SquishCloseArea,
  SquishOpenMsg,
  SquishCloseMsg,
  SquishReadMsg,
  SquishWriteMsg,
  SquishKillMsg,
  SquishLock,
  SquishUnlock,
  SquishSetCurPos,
  SquishGetCurPos,
  SquishMsgnToUid,
  SquishUidToMsgn,
  SquishGetHighWater,
  SquishSetHighWater,
  SquishGetTextLen,
  SquishGetCtrlLen
};


static byte *ss_sqd="%s.sqd";
static byte *ss_sqi="%s.sqi";

static struct _sqdata * _junksq;


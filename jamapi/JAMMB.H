/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**  jammb.h (JAMmb)
**
**  Prototypes and definitions for the JAM message base API
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    JoHo
**  Initial coding.
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __JAMMB_H__
#define __JAMMB_H__

#pragma pack(1)

#ifndef __JAM_H__
#include "jam.h"
#endif

#define JAMAPIMSG_NOTHING       0
#define JAMAPIMSG_HDRPRINT      1  /* Header fingerprint missing or invalid */
#define JAMAPIMSG_HDRPASSWORD   2  /* Invalid password for header file */
#define JAMAPIMSG_MSGPASSWORD   3  /* Invalid password for message */
#define JAMAPIMSG_ISOPEN        4  /* Message base is open */
#define JAMAPIMSG_ISNOTOPEN     5  /* Message base is not open */
#define JAMAPIMSG_ISNOTLOCKED   6  /* Message base is not locked */
#define JAMAPIMSG_SEEKERROR     7  /* Unable to seek in file */
#define JAMAPIMSG_CANTMKFILE    8  /* Unable to create file */
#define JAMAPIMSG_CANTRDFILE    9  /* Unable to read file */
#define JAMAPIMSG_CANTWRFILE    10 /* Unable to write file */
#define JAMAPIMSG_CANTRMFILE    11 /* Unable to remove file */
#define JAMAPIMSG_FIRSTMSG      12 /* Already on first message */
#define JAMAPIMSG_NEWMODCOUNTER 13 /* ModCounter have been updated since read */
#define JAMAPIMSG_NOMORETEXT    14 /* Read to end of message text */
#define JAMAPIMSG_CANTLKFILE    15 /* Unable to lock message base */
#define JAMAPIMSG_CANTFINDUSER  16 /* Unable to locate LastRead record */
#define JAMAPIMSG_CANTFINDMSG   17 /* Unable to locate message to user */
#define JAMAPIMSG_NOMOREMSGS    18 /* At end or beginning of file */
#define JAMAPIMSG_BADHEADERSIG  19 /* Invalid message header signature */
#define JAMAPIMSG_BADHEADERREV  20 /* Invalid message header revision */
#define JAMAPIMSG_INVMSGNUM     21 /* Invalid message number specified */

/*
**  Values for "fromwhere" parameter to SeekFunc, identical to those normally
**  defined in IO.H as SEEK_SET, SEEK_CUR, and SEEK_END.
*/
#define JAMSEEK_SET             0  /* To specified position */
#define JAMSEEK_CUR             1  /* From current position */
#define JAMSEEK_END             2  /* From end of file */

/*
**  Return values for user comparison functions
*/
#define ScanMsgHdrStop          0
#define ScanMsgHdrDiscard       1
#define ScanMsgHdrNextHdr       2

#define ScanMsgIdxStop          0
#define ScanMsgIdxNextMsg       1

/*
**  Basic structure definition. Most functions in the API require that
**  JAMAPIRECptr is passed to them.
*/

typedef struct _JAMAPIREC
    {
    CHAR8           BaseName[260];  /* Path for message base */
    INT16           isOpen;         /* Is open */
    INT16           HaveLock;       /* .JHR file is locked */
    int             Errno;          /* Last value of errno in case of error */
    int             APImsg;         /* API informative message */
    FHANDLE         HdrHandle;      /* File handle for .JHR file */
    FHANDLE         TxtHandle;      /* File handle for .JDT file */
    FHANDLE         IdxHandle;      /* File handle for .JDX file */
    FHANDLE         LrdHandle;      /* File handle for .JLR file */
    JAMHDRINFO      HdrInfo;        /* Message header info */
    ULONG           ulIdxAlloc;     /* Anzahl der allokierten ULONGs im Index */

    /* Functions */

    FHANDLE (_JAMPROC * CreateFunc)(struct _JAMAPIREC _JAMDATA *, CHAR8ptr FileName);
    FHANDLE (_JAMPROC * OpenFunc)(struct _JAMAPIREC _JAMDATA *, CHAR8ptr FileName);
    int (_JAMPROC * CloseFunc)(struct _JAMAPIREC _JAMDATA *, FHANDLE Handle);
    INT32 (_JAMPROC * ReadFunc)(struct _JAMAPIREC _JAMDATA *, FHANDLE Handle, VOIDptr Buffer, INT32 Length);
    INT32 (_JAMPROC * WriteFunc)(struct _JAMAPIREC _JAMDATA *, FHANDLE Handle, VOIDptr Buffer, INT32 Length);
    INT32 (_JAMPROC * SeekFunc)(struct _JAMAPIREC _JAMDATA *, FHANDLE Handle, int FromWhere, INT32 Offset);
    int (_JAMPROC * LockFunc)(struct _JAMAPIREC _JAMDATA *, int DoLock);
    int (_JAMPROC * UnlinkFunc)(struct _JAMAPIREC _JAMDATA *, CHAR8ptr FileName);
    }
    JAMAPIREC, _JAMDATA * JAMAPIRECptr;

/*
**  User comparison functions
*/
typedef int (_JAMPROC * ScanMsgHdrComp)(JAMAPIRECptr);
typedef int (_JAMPROC * ScanMsgIdxComp)(JAMAPIRECptr);

/*
**  API function prototypes
*/
int _JAMPROC JAMmbOpen(JAMAPIRECptr apirec);
int _JAMPROC JAMmbCreate(JAMAPIRECptr apirec);
int _JAMPROC JAMmbClose(JAMAPIRECptr apirec);
int _JAMPROC JAMmbUnlink(JAMAPIRECptr apirec);
int _JAMPROC JAMmbReIndex(JAMAPIRECptr apirec);

int _JAMPROC JAMmbUpdateHeaderInfo(JAMAPIRECptr apirec, int);

int _JAMPROC JAMmbFindField(JAMAPIRECptr apirec, UINT32, UINT32ptr);
int _JAMPROC JAMmbAddField(JAMAPIRECptr apirec, UINT32, int,
                                unsigned int, UINT32ptr, CHAR8ptr);

int _JAMPROC JAMmbFetchMsgHdr(JAMAPIRECptr apirec, JAMHDRptr Hdr, UINT32 WhatMsg);
int _JAMPROC JAMmbFetchHdrDirect(JAMAPIRECptr apirec, JAMHDRptr Hdr, UINT32 Offs);
int _JAMPROC JAMmbFetchSubfields(JAMAPIRECptr apirec, PVOID pBuffer, ULONG ulBufLen, UINT32 WhatMsg);
int _JAMPROC JAMmbFetchMsgIdx(JAMAPIRECptr apirec, UINT32 WhatMsg, JAMIDXRECptr Idx);
int _JAMPROC JAMmbFetchMsgTxt(JAMAPIRECptr apirec, INT32 TxtOffset, INT32 ReadCount, PCHAR pBuffer);
unsigned long _JAMPROC JAMmbReadIdxNr(JAMAPIRECptr apirec, UINT32 msgnr);
unsigned long _JAMPROC JAMmbReadIdxID(JAMAPIRECptr apirec, UINT32 MsgID, BOOL exact);

int _JAMPROC JAMmbSetMsgDeleted(JAMAPIRECptr apirec, UINT32 WhatMsg);
int _JAMPROC JAMmbSetMsgRead(JAMAPIRECptr apirec, UINT32 WhatMsg, int bPersonal);


int _JAMPROC JAMmbStoreMsgHdr(JAMAPIRECptr apirec, JAMHDRptr Hdr, UINT32 WhatMsg);
int _JAMPROC JAMmbAddMsgHdr(JAMAPIRECptr apirec, JAMHDRptr Hdr, PULONG pulNewOffset);
int _JAMPROC JAMmbAddSubfield(JAMAPIRECptr apirec, UINT16 LoID, UINT32 DataLen, PVOID pData);
int _JAMPROC JAMmbStoreMsgIdx(JAMAPIRECptr apirec, JAMIDXRECptr Idx, UINT32 WhatMsg);
int _JAMPROC JAMmbAddMsgIdx(JAMAPIRECptr apirec, JAMIDXRECptr Idx);
int _JAMPROC JAMmbAddMsgTxt(JAMAPIRECptr apirec, CHAR8ptr Buffer, ULONG BufLen, PULONG pulNewOffset);

int _JAMPROC JAMmbLockMsgBase(JAMAPIRECptr apirec, int);
int _JAMPROC JAMmbUnLockMsgBase(JAMAPIRECptr apirec, int);

int _JAMPROC JAMmbFetchLastRead(JAMAPIRECptr apirec, UINT32 UserID, JAMLREADptr LastRead);
int _JAMPROC JAMmbStoreLastRead(JAMAPIRECptr apirec, UINT32 UserID, UINT32 LastReadMsg);

#ifndef __JAMPROT_H__
#include "jamprot.h"
#endif

#endif /* __JAMMB_H__ */

#pragma pack()

#ifdef __cplusplus
}
#endif

/* end of file "jammb.h" */

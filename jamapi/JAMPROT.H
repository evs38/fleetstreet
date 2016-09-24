/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Mats Wallin
**
**  ----------------------------------------------------------------------
**
**  jamprot.h (JAMmb)
**
**  Compiler and platform dependant function prototypes
**
**  See JAMSYS.H for information about assumptions made about compilers
**  and platforms.
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    MW
**  Initial coding.
*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __JAMPROT_H__
#define __JAMPROT_H__

/*
**  Initialization of the JAMAPIREC structure
*/
int _JAMPROC JAMsysInitApiRec(JAMAPIRECptr apirec, CHAR8ptr pFile);
int _JAMPROC JAMsysDeinitApiRec(JAMAPIRECptr apirec);

/*
**  I/O functions
*/
int _JAMPROC JAMsysClose(JAMAPIRECptr apirec, FHANDLE fh);
FHANDLE _JAMPROC JAMsysCreate(JAMAPIRECptr apirec, CHAR8ptr pFileName);
int _JAMPROC JAMsysLock(JAMAPIRECptr apirec, int DoLock);
FHANDLE _JAMPROC JAMsysOpen(JAMAPIRECptr apirec, CHAR8ptr pFileName);
INT32 _JAMPROC JAMsysRead(JAMAPIRECptr apirec, FHANDLE fh, VOIDptr pBuf, INT32 Len);
INT32 _JAMPROC JAMsysSeek(JAMAPIRECptr apirec, FHANDLE fh, int FromWhere, INT32 Offset);
FHANDLE _JAMPROC JAMsysSopen(JAMAPIRECptr apirec, CHAR8ptr pFileName, int AccessMode, int ShareMode);
int _JAMPROC JAMsysUnlink(JAMAPIRECptr apirec, CHAR8ptr pFileName);
INT32 _JAMPROC JAMsysWrite(JAMAPIRECptr apirec, FHANDLE fh, VOIDptr pBuf, INT32 Len);

#if defined(_WINDOWS)
INT32 _JAMPROC JAMsysReadHuge(JAMAPIRECptr apirec, FHANDLE fh, VOIDptr pBuf, INT32 Len);
INT32 _JAMPROC JAMsysWriteHuge(JAMAPIRECptr apirec, FHANDLE fh, VOIDptr pBuf, INT32 Len);
#endif

/*
** CRC-32
*/
UINT32 _JAMPROC JAMsysCrc32(void _JAMFAR *pBuf, unsigned int len, UINT32 crc);

/*
**  Time functions
*/
UINT32 _JAMPROC JAMsysTime(UINT32ptr pTime);
UINT32 _JAMPROC JAMsysMkTime(JAMTMptr pTm);
JAMTMptr _JAMPROC JAMsysLocalTime(UINT32ptr pt);

/*
**  Pointer functions / macros
*/
#if defined(__MSDOS__)
    #if defined(__SMALL__) || defined(__MEDIUM__)
        #define JAMsysAddPtr(Ptr,Off) ((void *)(((CHAR8 *)(Ptr))+(UINT16)(Off)))
        void _JAMFAR * _JAMPROC JAMsysAddFarPtr(void _JAMFAR *Ptr, INT32 Offset);
    #else
        void _JAMFAR * _JAMPROC JAMsysAddPtr(void _JAMFAR *Ptr, INT32 Offset);
        #define JAMsysAddFarPtr   JAMsysAddPtr
    #endif
#elif defined(_WINDOWS)
    #if defined(__SMALL__) || defined(__MEDIUM__)
        #define JAMsysAddPtr(Ptr,Off) ((void *)(((CHAR8 *)(Ptr))+(UINT16)(Off)))
    #else
        #define JAMsysAddPtr JAMsysAddFarPtr
    #endif
    #define JAMsysAddFarPtr(Ptr,Offset) ((void _JAMFAR *)(((CHAR8 _HUGE *)(Ptr))+(Offset)))
#else
    #define JAMsysAddPtr(Ptr,Offset) ((void *)(((CHAR8 *)(Ptr))+(Offset)))
    #define JAMsysAddFarPtr   JAMsysAddPtr
#endif

/*
**  Aligment macro
**  JAMsysAlign should be defined for platforms that requires INT16s
**  and/or INT32s to be aligned on any boundary.
*/
#if defined(__MSDOS__) || defined(__OS2__) || defined(_WINDOWS)
    #define JAMsysAlign(v)    (v)
#elif defined(__sparc__)
    #define JAMsysAlign(v)    ((v)+(((v)%4)?(4-((v)%4)):0))
#elif defined(__50SERIES)
    #define JAMsysAlign(v)    ((v)+(((v)%4)?(4-((v)%4)):0))
#else
    #error Undefined platform
#endif

#endif /* __JAMPROT_H__ */

#ifdef __cplusplus
}
#endif

/* end of file "jamprot.h" */

/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**  jamfetch.c (JAMmb)
**
**  Fetch messages and texts
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    JoHo
**  Initial coding. No decryption or unescaping supported yet.
*/
#define JAMCAPI 1

#include <string.h>
#include <time.h>
#include <io.h>
#include "jammb.h"

/*
**  Fetch specified message number. Returns 1 upon success and 0 upon
**  failure.
*/
int _JAMPROC JAMmbFetchMsgHdr(JAMAPIRECptr apirec, JAMHDRptr Hdr, UINT32 WhatMsg)
{
    JAMIDXREC Idx;

    /* Fetch index record, checks for IsOpen */
    if (!JAMmbFetchMsgIdx(apirec, WhatMsg, &Idx))
        return (0);

    return JAMmbFetchHdrDirect(apirec, Hdr, Idx.HdrOffset);
}

int _JAMPROC JAMmbFetchHdrDirect(JAMAPIRECptr apirec, JAMHDRptr Hdr, UINT32 Offs)
{
    /* Fetch header */
    if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, (INT32)Offs)!=(INT32)Offs)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->ReadFunc(apirec, apirec->HdrHandle, Hdr, (INT32)sizeof(JAMHDR))== -1)
        {
        apirec->APImsg=JAMAPIMSG_CANTRDFILE;
        return (0);
        }

    /* Check header */
    if (strcmp(Hdr->Signature, HEADERSIGNATURE)!=0)
        {
        apirec->APImsg=JAMAPIMSG_BADHEADERSIG;
        return (0);
        }
    if (Hdr->Revision!=CURRENTREVLEV)
        {
        apirec->APImsg=JAMAPIMSG_BADHEADERREV;
        return (0);
        }


    /* Got it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/* Rckgabe:    0  Fehler
                1  gel”scht
                2  war schon gel”scht
*/

int _JAMPROC JAMmbSetMsgDeleted(JAMAPIRECptr apirec, UINT32 WhatMsg)
{
    JAMHDR Hdr;
    JAMIDXREC Idx;
    int rc=1;

    /* Fetch index record, checks for IsOpen */
    if (!JAMmbFetchMsgIdx(apirec, WhatMsg, &Idx))
        return (0);

    /* Fetch header */
    if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, (INT32)Idx.HdrOffset)!=(INT32)Idx.HdrOffset)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->ReadFunc(apirec, apirec->HdrHandle, &Hdr, (INT32)sizeof(JAMHDR))!=(INT32)sizeof(JAMHDR))
        {
        apirec->APImsg=JAMAPIMSG_CANTRDFILE;
        return (0);
        }

    /* Check header */
    if (strcmp(Hdr.Signature, HEADERSIGNATURE)!=0)
        {
        apirec->APImsg=JAMAPIMSG_BADHEADERSIG;
        return (0);
        }
    if (Hdr.Revision!=CURRENTREVLEV)
        {
        apirec->APImsg=JAMAPIMSG_BADHEADERREV;
        return (0);
        }

    /* Delete-Flag setzen */
    if (!(Hdr.Attribute & MSG_DELETED))
    {
       Hdr.Attribute |= MSG_DELETED;

       /* zurckschreiben */
       if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, (INT32)Idx.HdrOffset)!=(INT32)Idx.HdrOffset)
       {
          apirec->APImsg=JAMAPIMSG_SEEKERROR;
          return (0);
       }
       if (apirec->WriteFunc(apirec, apirec->HdrHandle, &Hdr, (INT32)sizeof(JAMHDR))!=(INT32)sizeof(JAMHDR))
       {
          apirec->APImsg=JAMAPIMSG_CANTWRFILE;
          return (0);
       }
    }
    else
       rc=2;

    /* User-CRC loeschen = Loesch-Kennzeichen */
    Idx.HdrOffset = Idx.UserCRC = 0xffffffffUL;
    JAMmbStoreMsgIdx(apirec, &Idx, WhatMsg);

    /* Got it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;

    return (rc);
}

int _JAMPROC JAMmbSetMsgRead(JAMAPIRECptr apirec, UINT32 WhatMsg, int bPersonal)
{
    JAMHDR Hdr;
    JAMIDXREC Idx;
    int bWrite=FALSE;

    /* Fetch index record, checks for IsOpen */
    if (!JAMmbFetchMsgIdx(apirec, WhatMsg, &Idx))
        return (0);

    /* Fetch header */
    if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, (INT32)Idx.HdrOffset)!=(INT32)Idx.HdrOffset)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->ReadFunc(apirec, apirec->HdrHandle, &Hdr, (INT32)sizeof(JAMHDR))!=(INT32)sizeof(JAMHDR))
        {
        apirec->APImsg=JAMAPIMSG_CANTRDFILE;
        return (0);
        }

    /* Check header */
    if (strcmp(Hdr.Signature, HEADERSIGNATURE)!=0)
        {
        apirec->APImsg=JAMAPIMSG_BADHEADERSIG;
        return (0);
        }
    if (Hdr.Revision!=CURRENTREVLEV)
        {
        apirec->APImsg=JAMAPIMSG_BADHEADERREV;
        return (0);
        }

    /* Read-Flag setzen */
    if (!Hdr.TimesRead)
    {
       Hdr.TimesRead++;
       bWrite=TRUE;
    }
    if (bPersonal)
    {
       if (!(Hdr.Attribute & MSG_READ))
       {
          Hdr.Attribute |= MSG_READ;
          bWrite = TRUE;
       }
       if (!Hdr.DateReceived)
       {
          Hdr.DateReceived = time(NULL);
          bWrite = TRUE;
       }
    }

    if (bWrite)
    {
       /* zurckschreiben */
       if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, (INT32)Idx.HdrOffset)!=(INT32)Idx.HdrOffset)
           {
           apirec->APImsg=JAMAPIMSG_SEEKERROR;
           return (0);
           }
       if (apirec->WriteFunc(apirec, apirec->HdrHandle, &Hdr, (INT32)sizeof(JAMHDR))!=(INT32)sizeof(JAMHDR))
           {
           apirec->APImsg=JAMAPIMSG_CANTWRFILE;
           return (0);
           }
    }

    /* Got it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

int _JAMPROC JAMmbFetchSubfields(JAMAPIRECptr apirec, PVOID pBuffer, ULONG ulBufLen, UINT32 WhatMsg)
{
    JAMIDXREC Idx;

    /* Fetch index record, checks for IsOpen */
    if (!JAMmbFetchMsgIdx(apirec, WhatMsg, &Idx))
        return (0);

    /* Fetch header */
    if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, (INT32)Idx.HdrOffset)!=(INT32)Idx.HdrOffset)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_CUR, (INT32)sizeof(JAMHDR))== -1)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

     if (apirec->ReadFunc(apirec, apirec->HdrHandle, pBuffer, ulBufLen)!=ulBufLen)
         {
         apirec->APImsg=JAMAPIMSG_CANTRDFILE;
         return (0);
         }

    return 1;
}

/*
**  Fetch index record with specified message number. Returns 1 upon success
**  and 0 upon failure.
*/
int _JAMPROC JAMmbFetchMsgIdx(JAMAPIRECptr apirec, UINT32 WhatMsg, JAMIDXRECptr Idx)
{
    INT32 WhatOffset;

    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Make sure the message number is valid */
    if (WhatMsg<apirec->HdrInfo.BaseMsgNum)
        {
        apirec->APImsg=JAMAPIMSG_INVMSGNUM;
        return (0);
        }

    /* Fetch index record */
    WhatOffset=(INT32)((WhatMsg-apirec->HdrInfo.BaseMsgNum) * (INT32)sizeof(JAMIDXREC));
    if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_SET, WhatOffset)!=WhatOffset)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->ReadFunc(apirec, apirec->IdxHandle, Idx, (INT32)sizeof(JAMIDXREC))!=(INT32)sizeof(JAMIDXREC))
        {
        apirec->APImsg=JAMAPIMSG_CANTRDFILE;
        return (0);
        }

    /* Got it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

#if 0

unsigned long _JAMPROC JAMmbReadIdxNr(JAMAPIRECptr apirec, UINT32 msgnr)
{
    ULONG ulID=0;
    int iRead=0;
    JAMIDXREC Idx;

    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Fetch index record */
    if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_SET, 0)!=0)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

    while (!__eof(apirec->IdxHandle))
    {
       if (apirec->ReadFunc(apirec, apirec->IdxHandle, &Idx, (INT32)sizeof(JAMIDXREC))==(INT32)sizeof(JAMIDXREC))
       {
          if (Idx.UserCRC != 0xffffffffUL)
          {
             iRead++;
             if (iRead == msgnr)
             {
                apirec->APImsg=JAMAPIMSG_NOTHING;
                return apirec->HdrInfo.BaseMsgNum + ulID;
             }
          }
       }
       ulID++;
    }

    /* Got it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (0);
}

unsigned long _JAMPROC JAMmbReadIdxID(JAMAPIRECptr apirec, UINT32 MsgID, BOOL exact)
{
    int iRead=0;
    JAMIDXREC Idx;

    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    if (MsgID < apirec->HdrInfo.BaseMsgNum)  /* vor allen IDs */
       return 0;

    MsgID -= apirec->HdrInfo.BaseMsgNum;

    /* Fetch index record */
    if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_SET, 0)!=0)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

    while (!__eof(apirec->IdxHandle) )
    {
       if (apirec->ReadFunc(apirec, apirec->IdxHandle, &Idx, (INT32)sizeof(JAMIDXREC))==(INT32)sizeof(JAMIDXREC))
       {
          if (Idx.UserCRC != 0xffffffffUL)
             iRead++;

          if (!MsgID)
             if (exact && Idx.UserCRC == 0xffffffffUL)
             {
                apirec->APImsg=JAMAPIMSG_NOTHING;
                return 0;
             }
             else
             {
                apirec->APImsg=JAMAPIMSG_NOTHING;
                return iRead;
             }

          MsgID--;
       }
    }

    /* Got it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    if (exact)
       return 0;
    else
       return iRead;
}

#endif

/*
**  Fetch text for specified message number. Returns 1 upon success and
**  0 upon failure. FirstFetch determines if the function seeks to the
**  actual text position or simply keeps reading.
*/
int _JAMPROC JAMmbFetchMsgTxt(JAMAPIRECptr apirec, INT32 TxtOffset, INT32 ReadCount, PCHAR pBuffer)
{
    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Seek to appropriate text position if this is first fetch */
    if (apirec->SeekFunc(apirec, apirec->TxtHandle, JAMSEEK_SET, TxtOffset)!=TxtOffset)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

    /* Get it from disk */
    if (apirec->ReadFunc(apirec, apirec->TxtHandle, pBuffer, ReadCount)!=ReadCount)
        {
        apirec->APImsg=JAMAPIMSG_CANTRDFILE;
        return (0);
        }

    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/* end of file "jamfetch.c" */

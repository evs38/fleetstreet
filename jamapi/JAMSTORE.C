/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**  jamstore.c (JAMmb)
**
**  Store messages and texts
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    JoHo
**  Initial coding. No encryption or escaping supported yet.
*/
#define JAMCAPI 1

#include <string.h>
#include "jammb.h"

/*
**  Store message header with specified number. Returns 1 upon success
**  and 0 upon failure. The HdrHandle's file offset will point to the
**  end of the fixed-length header when the function returns (if
**  successful) so the application can write any subfields directly to
**  the file.
*/
int _JAMPROC JAMmbStoreMsgHdr(JAMAPIRECptr apirec, JAMHDRptr Hdr, UINT32 WhatMsg)
{
    JAMIDXREC Idx;

    /* Fetch index record, checks for IsOpen and valid msg number */
    if (!JAMmbFetchMsgIdx(apirec, WhatMsg, &Idx))
        return (0);

    /* Make sure it's locked */
    if (!apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Update structure, even if below fails */
    Hdr->MsgNum=WhatMsg;

    /* Make sure header signature and revision is OK */
    strcpy(Hdr->Signature, HEADERSIGNATURE);
    Hdr->Revision=CURRENTREVLEV;

    /* Write header */
    if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, (INT32)Idx.HdrOffset)!=(INT32)Idx.HdrOffset)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->WriteFunc(apirec, apirec->HdrHandle, Hdr, (INT32)sizeof(JAMHDR))!=(INT32)sizeof(JAMHDR))
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    /* Wrote it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

int _JAMPROC JAMmbAddMsgHdr(JAMAPIRECptr apirec, JAMHDRptr Hdr, PULONG pulNewOffset)
{
    /* Make sure it's locked */
    if (!apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Make sure header signature and revision is OK */
    strcpy(Hdr->Signature, HEADERSIGNATURE);
    Hdr->Revision=CURRENTREVLEV;

    /* Write header */
    if ((*pulNewOffset = apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_END, 0))== -1)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->WriteFunc(apirec, apirec->HdrHandle, Hdr, (INT32)sizeof(JAMHDR))!=(INT32)sizeof(JAMHDR))
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    /* Wrote it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

int _JAMPROC JAMmbAddSubfield(JAMAPIRECptr apirec, UINT16 LoID, UINT32 DataLen, PVOID pData)
{
    JAMBINSUBFIELD SubField;

    SubField.LoID = LoID;
    SubField.HiID = 0;
    SubField.DatLen = DataLen;

    /* Make sure it's locked */
    if (!apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Subfield-Header schreiben */
    if (apirec->WriteFunc(apirec, apirec->HdrHandle, &SubField, (INT32)sizeof(JAMBINSUBFIELD))!=(INT32)sizeof(JAMBINSUBFIELD))
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    /* Subfield-Daten schreiben */
    if (apirec->WriteFunc(apirec, apirec->HdrHandle, pData, (INT32)DataLen)!=(INT32)DataLen)
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    /* Wrote it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/*
**  Store message index record with specified number. Returns 1 upon
**  success and 0 upon failure. The IdxHandle's file offset will point
**  to the end of the fixed-length index record when the function
**  returns (if successful).
*/
int _JAMPROC JAMmbStoreMsgIdx(JAMAPIRECptr apirec, JAMIDXRECptr Idx, UINT32 WhatMsg)
{
    INT32 WhatOffset;

    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Make sure it's locked */
    if (!apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Make sure the message number is valid */
    if (WhatMsg<apirec->HdrInfo.BaseMsgNum)
        {
        apirec->APImsg=JAMAPIMSG_INVMSGNUM;
        return (0);
        }

    /* Store index record */
    WhatOffset=(INT32)((WhatMsg-apirec->HdrInfo.BaseMsgNum) * (INT32)sizeof(JAMIDXREC));
    if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_SET, WhatOffset)!=WhatOffset)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->WriteFunc(apirec, apirec->IdxHandle, Idx, (INT32)sizeof(JAMIDXREC))!=(INT32)sizeof(JAMIDXREC))
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    /* Wrote it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

int _JAMPROC JAMmbAddMsgIdx(JAMAPIRECptr apirec, JAMIDXRECptr Idx)
{
    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Make sure it's locked */
    if (!apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Store index record */
    if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_END, 0)== -1)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }
    if (apirec->WriteFunc(apirec, apirec->IdxHandle, Idx, (INT32)sizeof(JAMIDXREC))!=(INT32)sizeof(JAMIDXREC))
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    /* Wrote it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}


int _JAMPROC JAMmbAddMsgTxt(JAMAPIRECptr apirec, CHAR8ptr Buffer, ULONG BufLen, PULONG pulNewOffset)
{
    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Make sure it's locked */
    if (!apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Seek if we're told to */
    if ((*pulNewOffset = apirec->SeekFunc(apirec, apirec->TxtHandle, JAMSEEK_END, 0))==-1)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

    /* Write the text */
    if (apirec->WriteFunc(apirec, apirec->TxtHandle, Buffer, BufLen)!=BufLen)
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    /* Wrote it OK */
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/* end of file "jamstore.c" */

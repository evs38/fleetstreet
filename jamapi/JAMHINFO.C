/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**  jamhinfo.c (JAMmb)
**
**  Updating of header info block at beginning of header file
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    JoHo
**  Initial coding.
*/
#define JAMCAPI 1

#include "jammb.h"

/*
**  Update or retrieve header info block at beginning of header file. Returns
**  1 upon success and 0 upon failure.
*/
int _JAMPROC JAMmbUpdateHeaderInfo(JAMAPIRECptr apirec, int WriteIt)
{
    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Make sure we have lock if we need to */
    if (WriteIt && !apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Seek to beginning of file */
    if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, 0L)!=0L)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

    /* Update ModCounter if told to*/
    if (WriteIt)
        {
        apirec->HdrInfo.ModCounter++;

        if (apirec->HdrInfo.BaseMsgNum==0L)
            apirec->HdrInfo.BaseMsgNum=1L;

        /* Update header info record */
        if (apirec->WriteFunc(apirec, apirec->HdrHandle, &apirec->HdrInfo, (INT32)sizeof(JAMHDRINFO))!=(INT32)sizeof(JAMHDRINFO))
            {
            apirec->HdrInfo.ModCounter--;
            apirec->APImsg=JAMAPIMSG_CANTWRFILE;
            return (0);
            }
        }
    else
        /* Fetch header info record */
        {
        if (apirec->ReadFunc(apirec, apirec->HdrHandle, &apirec->HdrInfo, (INT32)sizeof(JAMHDRINFO))!=(INT32)sizeof(JAMHDRINFO))
            {
            apirec->APImsg=JAMAPIMSG_CANTRDFILE;
            return (0);
            }

        if (apirec->HdrInfo.BaseMsgNum==0L)
            apirec->HdrInfo.BaseMsgNum=1L;
        }

    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/* end of file "jamhinfo.c" */

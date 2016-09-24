/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**  jamlock.c (JAMmb)
**
**  Locking and unlocking of message base
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
**  Locks message base and if successful, optionally reads the header info
**  block at the beginning of the header file. Returns 1 upon success and
**  0 upon failure.
*/
int _JAMPROC JAMmbLockMsgBase(JAMAPIRECptr apirec, int FetchHdrInfo)
{
    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Attempt to lock it if we don't already have a lock */
    if (!apirec->HaveLock)
        {
        if (apirec->LockFunc(apirec, 1)<0)
            {
            apirec->APImsg=JAMAPIMSG_CANTLKFILE;
            return (0);
            }
        /* Make sure we know about the lock */
        apirec->HaveLock=1;
        }

    /* Read header info block if told to */
    if (FetchHdrInfo)
        {
        if (!JAMmbUpdateHeaderInfo(apirec, 0))
            return (0);
        }

    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/*
**  Unlocks message base and if successful, optionally updates the header
**  info block (and its ModCounter) at the beginning of the header file.
**  Returns 1 upon success and 0 upon failure.
*/
int _JAMPROC JAMmbUnLockMsgBase(JAMAPIRECptr apirec, int UpdateHdrInfo)
{
    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Make sure we have lock */
    if (!apirec->HaveLock)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTLOCKED;
        return (0);
        }

    /* Update header info if told to before unlocking */
    if (UpdateHdrInfo)
        {
        if (!JAMmbUpdateHeaderInfo(apirec, 1))
            return (0);
        }

    /* Unlock the file */
    apirec->LockFunc(apirec, 0);
    apirec->HaveLock=0;
    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/* end of file "jamlock.c" */

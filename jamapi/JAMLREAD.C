/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**  jamlread.c (JAMmb)
**
**  LastRead handling
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
**  Fetch LastRead for passed UserID. Returns 1 upon success and 0 upon
**  failure.
*/
int _JAMPROC JAMmbFetchLastRead(JAMAPIRECptr apirec, UINT32 UserID, JAMLREADptr LastRead)
{
    INT32 ReadCount;
    UINT32 LastReadRec;

    /* Make sure it's open */
    if (!apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;
        return (0);
        }

    /* Seek to beginning of file */
    if (apirec->SeekFunc(apirec, apirec->LrdHandle, JAMSEEK_SET, 0L)!=0L)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

    /* Read file from top to bottom */
    LastReadRec=0L;
    while (1)
        {
        ReadCount=apirec->ReadFunc(apirec, apirec->LrdHandle, LastRead, (INT32)sizeof(JAMLREAD));
        if (ReadCount!=(INT32)sizeof(JAMLREAD))
            {
            if (!ReadCount)
                /* End of file */
                apirec->APImsg=JAMAPIMSG_CANTFINDUSER;
            else
                /* Read error */
                apirec->APImsg=JAMAPIMSG_CANTRDFILE;
            return (0);
            }

        /* See if it matches what we want */
        if (LastRead->UserCRC==UserID)
            {
            apirec->APImsg=JAMAPIMSG_NOTHING;
            return (1);
            }

        /* Next record number */
        LastReadRec++;
        }/*while*/

    /* Dummy return to avoid warnings from some compilers */
    /*return (0);*/
}

/*
**  Store LastRead record and if successful, optionally updates the header
**  info block (and its ModCounter) at the beginning of the header file.
**  Returns 1 upon success and 0 upon failure.
*/
int _JAMPROC JAMmbStoreLastRead(JAMAPIRECptr apirec, UINT32 UserID, UINT32 LastReadMsg)
{
    INT32 ReadCount;
    JAMLREAD LastRead;
    UINT32 LastReadRec=0;

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

    /* Seek to the appropriate position */
    if (apirec->SeekFunc(apirec, apirec->LrdHandle, JAMSEEK_SET, 0)!=0)
        {
        apirec->APImsg=JAMAPIMSG_SEEKERROR;
        return (0);
        }

    while (1)
        {
        ReadCount=apirec->ReadFunc(apirec, apirec->LrdHandle, &LastRead, (INT32)sizeof(JAMLREAD));
        if (ReadCount!=(INT32)sizeof(JAMLREAD))
            {
            if (!ReadCount)
            {
                /* End of file */
                LastRead.UserCRC = UserID;
                LastRead.UserID = UserID;
                LastRead.LastReadMsg=0;
                LastRead.HighReadMsg=0;
                break;
            }
            else
                /* Read error */
                apirec->APImsg=JAMAPIMSG_CANTRDFILE;
            return (0);
            }

        /* See if it matches what we want */
        if (LastRead.UserCRC==UserID)
        {
           /* zuruecksetzen */
           if (apirec->SeekFunc(apirec, apirec->LrdHandle, JAMSEEK_SET, LastReadRec * sizeof(JAMLREAD))!=LastReadRec * sizeof(JAMLREAD))
               {
               apirec->APImsg=JAMAPIMSG_SEEKERROR;
               return (0);
               }
           break;
        }

        /* Next record number */
        LastReadRec++;
        }/*while*/

    /* Daten updaten */
    LastRead.LastReadMsg = LastReadMsg;
    if (LastReadMsg > LastRead.HighReadMsg)
       LastRead.HighReadMsg = LastReadMsg;

    /* Write record */
    if (apirec->WriteFunc(apirec, apirec->LrdHandle, &LastRead, (INT32)sizeof(JAMLREAD))!=(INT32)sizeof(JAMLREAD))
        {
        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
        return (0);
        }

    apirec->APImsg=JAMAPIMSG_NOTHING;
    return (1);
}

/* end of file "jamlread.c" */

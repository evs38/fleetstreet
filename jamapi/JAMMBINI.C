/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Joaquim Homrighausen.
**
**  ----------------------------------------------------------------------
**
**  jammbini.c (JAMmb)
**
**  Initialization, open, and close functions for message base
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    JoHo
**  Initial coding
*/
#define JAMCAPI 1

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __sparc__
#include <memory.h>
#endif

#include "jammb.h"
#include "..\util\crc32.h"

/*
**  Open message base pointed to by API structure. Returns 1 on success,
**  0 on failure.
*/
int _JAMPROC JAMmbOpen(JAMAPIRECptr apirec)
{
    CHAR8   FileName[260];

    /* Make sure it's not already open */
    if (apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISOPEN;
        return (1);
        }
    else
        apirec->APImsg=JAMAPIMSG_NOTHING;

    /* .JHR file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_HDRFILE);
    apirec->HdrHandle=apirec->OpenFunc(apirec, FileName);
    if (apirec->HdrHandle<0)
        return (0);

    if (apirec->ReadFunc(apirec, apirec->HdrHandle, &apirec->HdrInfo, (INT32)sizeof(JAMHDRINFO))!=(INT32)sizeof(JAMHDRINFO))
    {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        return (0);
    }
    if (apirec->HdrInfo.BaseMsgNum==0L)
        apirec->HdrInfo.BaseMsgNum=1L;

    /* .JDT file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_TXTFILE);
    apirec->TxtHandle=apirec->OpenFunc(apirec, FileName);
    if (apirec->TxtHandle<0)
        {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        return (0);
        }

    /* .JDX file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_IDXFILE);
    apirec->IdxHandle=apirec->OpenFunc(apirec, FileName);
    if (apirec->IdxHandle<0)
        {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        apirec->CloseFunc(apirec, apirec->TxtHandle);
        return (0);
        }

    /* .JLR file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_LRDFILE);
    apirec->LrdHandle=apirec->OpenFunc(apirec, FileName);
    if (apirec->LrdHandle<0)
        {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        apirec->CloseFunc(apirec, apirec->TxtHandle);
        apirec->CloseFunc(apirec, apirec->IdxHandle);
        return (0);
        }

    apirec->isOpen=1;
    return (1);
}


/*
**  Close message base pointed to by API structure. Returns 1 on success,
**  0 on failure.
*/
int _JAMPROC JAMmbClose(JAMAPIRECptr apirec)
{
    /* Close files */
    if (apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_NOTHING;

        /* Make sure header file is unlocked */
        if (apirec->HaveLock)
            apirec->LockFunc(apirec, 0);

        /* Close all handles */
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        apirec->CloseFunc(apirec, apirec->TxtHandle);
        apirec->CloseFunc(apirec, apirec->IdxHandle);
        apirec->CloseFunc(apirec, apirec->LrdHandle);

        apirec->HdrHandle=
            apirec->TxtHandle=
                apirec->IdxHandle=
                    apirec->LrdHandle=-1;

        /* Set flag */
        apirec->isOpen=0;
        }
    else
        apirec->APImsg=JAMAPIMSG_ISNOTOPEN;

    return (1);
}

#if 0
/*
**  Remove message base pointed to by API structure. Returns 1 on success,
**  0 on failure.
*/
int _JAMPROC JAMmbUnlink(JAMAPIRECptr apirec)
{
    CHAR8 FileName[260];
    int J1, J2, J3, J4;

    apirec->APImsg=JAMAPIMSG_NOTHING;
    if (apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISOPEN;
        return (0);
        }

    /* .JHR file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_HDRFILE);
    J1=apirec->UnlinkFunc(apirec, FileName);

    /* .JDT file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_TXTFILE);
    J2=apirec->UnlinkFunc(apirec, FileName);

    /* .JDX file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_IDXFILE);
    J3=apirec->UnlinkFunc(apirec, FileName);

    /* .JLR file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_LRDFILE);
    J4=apirec->UnlinkFunc(apirec, FileName);

    return (!J1 && !J2 && !J3 && !J4);
}

#endif

/*
**  Create new message base pointed to by API structure. Returns 1 on
**  success, 0 on failure.
*/
int _JAMPROC JAMmbCreate(JAMAPIRECptr apirec)
{
    CHAR8 FileName[260];

    /* Make sure it's not already open */
    if (apirec->isOpen)
        {
        apirec->APImsg=JAMAPIMSG_ISOPEN;
        return (1);
        }
    else
        apirec->APImsg=JAMAPIMSG_NOTHING;

    /* .JHR file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_HDRFILE);
    apirec->HdrHandle=apirec->CreateFunc(apirec, FileName);
    if (apirec->HdrHandle<0)
        return (0);

    memset(&apirec->HdrInfo, 0, sizeof(JAMHDRINFO));
    strcpy(apirec->HdrInfo.Signature, HEADERSIGNATURE);
    apirec->HdrInfo.DateCreated=time(NULL);
    apirec->HdrInfo.PasswordCRC=0xffffffffL;
    apirec->HdrInfo.BaseMsgNum=1L;

    if (apirec->WriteFunc(apirec, apirec->HdrHandle, &apirec->HdrInfo, (INT32)sizeof(JAMHDRINFO))!=(INT32)sizeof(JAMHDRINFO))
        {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        return (0);
        }

    /* .JDT file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_TXTFILE);
    apirec->TxtHandle=apirec->CreateFunc(apirec, FileName);
    if (apirec->TxtHandle<0)
        {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        return (0);
        }

    /* .JDX file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_IDXFILE);
    apirec->IdxHandle=apirec->CreateFunc(apirec, FileName);
    if (apirec->IdxHandle<0)
        {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        apirec->CloseFunc(apirec, apirec->TxtHandle);
        return (0);
        }

    /* .JLR file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_LRDFILE);
    apirec->LrdHandle=apirec->CreateFunc(apirec, FileName);
    if (apirec->LrdHandle<0)
        {
        apirec->CloseFunc(apirec, apirec->HdrHandle);
        apirec->CloseFunc(apirec, apirec->TxtHandle);
        apirec->CloseFunc(apirec, apirec->IdxHandle);
        return (0);
        }

    apirec->isOpen=1;
    return (1);
}

#if 0

/*
**  Reindex messages in an open message base pointed to by API structure.
**  The message base must be locked by the application prior to calling this
**  function. Returns 1 on success, 0 on failure.
*/
int _JAMPROC JAMmbReIndex(JAMAPIRECptr apirec)
{
    CHAR8 FileName[260], JunkBuf[110];
    JAMIDXREC IdxRec;
    JAMSUBFIELD *SubPtr;
    INT32 Junk, NextOffset, CurIdxOffset, NextIdxOffset, IdxSize;
    UINT32 JunkLong, SubDatLen;
    INT16 Done, Error, FoundField;

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

    /* Close index file */
    if (apirec->IdxHandle>=0)
        apirec->CloseFunc(apirec, apirec->IdxHandle);

    /* Create new index file */
    sprintf(FileName, "%s%s", apirec->BaseName, EXT_IDXFILE);
    apirec->IdxHandle=apirec->CreateFunc(apirec, FileName);
    if (apirec->IdxHandle<0)
        {
        apirec->APImsg=JAMAPIMSG_CANTMKFILE;
        return (0);
        }

    /* Process header file */
    NextOffset=(INT32)sizeof(JAMHDRINFO);
    CurIdxOffset=0L;
    IdxSize=0L;
    Done=Error=0L;
    SubPtr=(JAMSUBFIELD *)JunkBuf;

    while (!Done && !Error)
        {
        /* Seek to correct position in header file */
        if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_SET, NextOffset)!=NextOffset)
            {
            apirec->APImsg=JAMAPIMSG_SEEKERROR;
            Error=1;
            }
        else
            {
            /* Read header */
            Junk=apirec->ReadFunc(apirec, apirec->HdrHandle, &apirec->Hdr, (INT32)sizeof(JAMHDR));
            if (Junk!=(INT32)sizeof(JAMHDR))
                {
                if (!Junk)
                    /* Nothing left to read */
                    Done=1;
                else
                    {
                    /* Read error */
                    apirec->APImsg=JAMAPIMSG_CANTRDFILE;
                    Error=1;
                    }
                }
            else
                {
                /* Set values */
                IdxRec.HdrOffset=(UINT32)NextOffset;

                /* Search subfields if we have any and this hasn't been deleted */
                if (apirec->Hdr.SubfieldLen && !(apirec->Hdr.Attribute & MSG_DELETED))
                    {
                    FoundField=0;
                    JunkLong=0L;
                    do
                        {
                        Junk=apirec->ReadFunc(apirec, apirec->HdrHandle, &JunkBuf, sizeof(JAMBINSUBFIELD));
                        if (Junk!=sizeof(JAMBINSUBFIELD))
                            {
                            if (!Junk)
                                Done=1;
                            else
                                {
                                apirec->APImsg=JAMAPIMSG_CANTRDFILE;
                                Error=1;
                                }
                            }
                        else
                            {
                            JunkLong+=sizeof(JAMBINSUBFIELD);
                            SubDatLen=SubPtr->DatLen;

                            /* Is this what we're looking for? */
                            if (SubPtr->LoID==JAMSFLD_RECVRNAME)
                                {
                                /* Yes, read username */
                                Junk=apirec->ReadFunc(apirec, apirec->HdrHandle, &JunkBuf, (INT32)SubDatLen);
                                if (Junk==(INT32)SubDatLen)
                                    FoundField=1;
                                else
                                    {
                                    apirec->APImsg=JAMAPIMSG_CANTRDFILE;
                                    Error=1;
                                    }
                                }
                            else
                                /* No, skip to next header */
                                {
                                SubDatLen=JAMsysAlign(SubDatLen);
                                JunkLong+=SubDatLen;
                                if (apirec->SeekFunc(apirec, apirec->HdrHandle, JAMSEEK_CUR, SubDatLen)<0L)
                                    {
                                    apirec->APImsg=JAMAPIMSG_SEEKERROR;
                                    Error=1;
                                    }
                                }/*Skip to next*/
                            }/*ReadOK*/
                        }
                    while (!Error && !FoundField && !Done && (INT32)JunkLong<(INT32)apirec->Hdr.SubfieldLen);

                    if (FoundField)
                        {
                        *((UCHAR8 _JAMFAR *)(JAMsysAddFarPtr (JunkBuf, SubDatLen))) = '\0';
                        strlwr (JunkBuf);
                        IdxRec.UserCRC=Crc32((void _JAMFAR *)&JunkBuf, (UINT16)SubDatLen, 0xffffffffL);
                        }
                    else
                        IdxRec.UserCRC=0xffffffffL;
                    }
                else
                    /* No subfields */
                    IdxRec.UserCRC=0xffffffffL;

                /* Skip message if invalid message number */
                if (apirec->Hdr.MsgNum>=apirec->HdrInfo.BaseMsgNum)
                    {
                    if (!Error)
                        {
                        NextIdxOffset=(apirec->Hdr.MsgNum-apirec->HdrInfo.BaseMsgNum)*sizeof(JAMIDXREC);
                        if (CurIdxOffset!=NextIdxOffset)
                            {
                            if (NextIdxOffset>IdxSize)
                                {
                                if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_SET, IdxSize)!=IdxSize)
                                    {
                                    apirec->APImsg=JAMAPIMSG_SEEKERROR;
                                    Error=1;
                                    }
                                while (!Error && IdxSize!=NextIdxOffset)
                                    {
                                    JAMIDXREC   TmpIdxRec = {0xffffffffL,0xffffffffL};

                                    if (apirec->WriteFunc(apirec, apirec->IdxHandle, &TmpIdxRec, (INT32)sizeof(JAMIDXREC))!=(INT32)sizeof(JAMIDXREC))
                                        {
                                        apirec->APImsg=JAMAPIMSG_CANTWRFILE;
                                        Error=1;
                                        }
                                    else
                                        IdxSize+=sizeof(JAMIDXREC);
                                    }
                                }
                            else
                                {
                                if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_SET, NextIdxOffset)!=NextIdxOffset)
                                    {
                                    apirec->APImsg=JAMAPIMSG_SEEKERROR;
                                    Error=1;
                                    }
                                }
                            }
                        }

                    if (!Error)
                        {
                        /* Write index record */
                        if (apirec->WriteFunc(apirec, apirec->IdxHandle, &IdxRec, (INT32)sizeof(JAMIDXREC))!=(INT32)sizeof(JAMIDXREC))
                            {
                            /* Write error */
                            apirec->APImsg=JAMAPIMSG_CANTWRFILE;
                            Error=1;
                            }
                        else
                            {
                            CurIdxOffset=NextIdxOffset+sizeof(JAMIDXREC);
                            if (CurIdxOffset>IdxSize)
                                IdxSize=CurIdxOffset;

                            /* Calculate next header offset */
                            NextOffset+=(INT32)sizeof(JAMHDR);
                            if (apirec->Hdr.SubfieldLen)
                                NextOffset+=(INT32)apirec->Hdr.SubfieldLen;
                            }
                        }
                    }
                }
            }
        }/*while*/

    return (!Error);
}
#endif

/* end of file "jammbini.c" */

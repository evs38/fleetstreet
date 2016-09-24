/*
**  JAM(mbp) - The Joaquim-Andrew-Mats Message Base Proposal
**
**  C API
**
**  Written by Mats Wallin
**
**  ----------------------------------------------------------------------
**
**  jamsys.c (JAMmb)
**
**  Platform/compiler dependant functions
**
**  See jamsys.h for information about assumptions made about compilers
**  and platforms
**
**  Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, and
**  Mats Wallin. ALL RIGHTS RESERVED.
**
**  93-06-28    MW
**  Initial coding
*/

#define JAMCAPI 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    #include <fcntl.h>
    #include <io.h>
    #include <share.h>
    #include <sys\stat.h>
#if defined(__MULTI__)
    #define INCL_BASE
    #define INCL_WIN
    #include <os2.h>
#endif

#include "jammb.h"


/*
**  JAMsysInitApiRec
**
**  Initializes the JAMAPIREC structure with all needed information
**
**      Parameters:     JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                      CHAR8ptr pFile      - Name of JAM messagebase
**                      UINT32 Size         - Size of workbuffer to allocate
**
**         Returns:     0                   - Failure to allocate workbuffer
**                      1                   - Success
*/
int _JAMPROC JAMsysInitApiRec(JAMAPIRECptr apirec, CHAR8ptr pFile)
{
    memset(apirec, 0, sizeof(JAMAPIREC));

    strcpy(apirec->BaseName, pFile);

    apirec->HdrHandle=
        apirec->TxtHandle=
            apirec->IdxHandle=
                apirec->LrdHandle=-1;

    apirec->CreateFunc = JAMsysCreate;
    apirec->OpenFunc   = JAMsysOpen;
    apirec->CloseFunc  = JAMsysClose;
    apirec->ReadFunc   = JAMsysRead;
    apirec->WriteFunc  = JAMsysWrite;
    apirec->SeekFunc   = JAMsysSeek;
    apirec->LockFunc   = JAMsysLock;
    apirec->UnlinkFunc = JAMsysUnlink;

    return (1);
}

/*
**  JAMsysDeinitApiRec
**
**  Deinitializes the JAMAPIREC structure. If the message base still is
**  opened, it is closed
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**
**         Returns:   0                   - Failure to allocate workbuffer
**                    1                   - Success
*/
int _JAMPROC JAMsysDeinitApiRec(JAMAPIRECptr apirec)
{
    /*This will release any locks we might has as well*/
    if (apirec->isOpen)
        JAMmbClose(apirec);

    apirec->BaseName[0]='\0';

    return (1);
}

/*
**  JAMsysClose
**
**  Closes the specified filehandle
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    FHANDLE fh          - Filehandle
**
**         Returns:   0 on success, or -1 in the case of an error
*/
int _JAMPROC JAMsysClose(JAMAPIRECptr apirec, FHANDLE fh)
{
    if (close(fh)<0)
    {
        if (apirec)
            apirec->Errno=errno;
        return (-1);
    }

    return (0);
}

/*
**  JAMsysCreate
**
**  Creates the specified file, closes it, and then reopens it in the
**  correct mode.
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    CHAR8ptr pFileName  - Name of file to create
**
**         Returns:   The filehandle when successful, -1 if it fails
*/
FHANDLE _JAMPROC JAMsysCreate(JAMAPIRECptr apirec, CHAR8ptr pFileName)
{
    FHANDLE fh;

    fh=creat(pFileName, S_IREAD|S_IWRITE);

    if (fh<0)
        {
        if (apirec)
            apirec->Errno = errno;
        return (-1);
        }

    if (JAMsysClose(apirec, fh)<0)
        return (-1);

    return (JAMsysOpen(apirec, pFileName));
}

/*
**  JAMsysLock
**
**  Sets or resets the lock on the JAM messagebase. This is done with
**  a lock on the first byte in the header file
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    int DoLock          - 1 if messagebase should be locked
**                                        - 0 to unlock messagebase
**
**         Returns:   0 on success, and -1 on failure
*/
int _JAMPROC JAMsysLock(JAMAPIRECptr apirec, int DoLock)
{
    FILELOCK FileLock= {0, 1};

    if (DosSetFileLocks(apirec->HdrHandle,
                        DoLock?NULL:&FileLock,
                        DoLock?&FileLock:NULL,
                        2000, 0))
       return -1;
    else
       return 0;
}

/*
**  JAMsysOpen
**
**  Opens the specified file in ReadWrite/DenyNone mode
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    CHAR8ptr pFileName  - Name of file to open
**
**         Returns:   The filehandle when successful, -1 if it fails
*/
FHANDLE _JAMPROC JAMsysOpen(JAMAPIRECptr apirec, CHAR8ptr pFileName)
{
    FHANDLE fh;

    fh=JAMsysSopen (apirec, pFileName, JAMO_RDWR, JAMSH_DENYNO);
    return (fh);
}

/*
**  JAMsysRead
**
**  Reads the specified number of bytes from the specified filehandle
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    FHANDLE fh          - Filehandle of file to read from
**                    VOIDptr pBuf        - Buffer where read data are stored
**                    INT32               - Number of bytes to read
**
**         Returns: The number of bytes read, or -1 in the case of error
**
**  NOTE! In small and medium memory model, the maximum size that can
**        be read is 0xFF00 bytes
*/
INT32 _JAMPROC JAMsysRead(JAMAPIRECptr apirec, FHANDLE fh, VOIDptr pBuf, INT32 Len)
{
    INT32       TotalRead = 0;


    if((TotalRead = read(fh, pBuf, Len )) == -1)
        {
        if (apirec)
            apirec->Errno = errno;
        }

    return(TotalRead);
}

/*
**  JAMsysSeek
**
**  Move the file pointer for the filehandle to the specified location.
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    FHANDLE fh          - Filehandle
**                    int FromWhere       - From where should the seek start
**                    INT32 Offset        - Which position to seek to
**
**         Returns:   The new offset of the file pointer, from the beginning
**                    of the file, or -1L in the case of error
*/
INT32 _JAMPROC JAMsysSeek(JAMAPIRECptr apirec, FHANDLE fh, int FromWhere, INT32 Offset)
{
    INT32   NewOffset;

    if((NewOffset = lseek(fh, Offset, FromWhere)) == -1L)
        {
        if (apirec)
            apirec->Errno = errno;
        }

    return(NewOffset);
}

/*
**  JAMsysSopen
**
**  Opens the specified file in the specified access and sharing mode
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    CHAR8ptr pFileName  - Name of file to open
**                    int AccessMode      - AccessMode
**                    int ShareMode       - ShareMode
**
**         Returns:   The filehandle when successful, -1 if it fails
*/
FHANDLE _JAMPROC JAMsysSopen(JAMAPIRECptr apirec, CHAR8ptr pFileName, int AccessMode, int ShareMode)
{
    FHANDLE fh;

    fh=sopen(pFileName, AccessMode | O_BINARY, ShareMode, S_IREAD|S_IWRITE);
    if (fh<0)
        {
        if (apirec)
            apirec->Errno=errno;
        }

    return (fh);
}

/*
**  JAMsysUnlink
**
**  Deletes the specified file
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    CHAR8PTR pFileName   - Name of file to delete
**
**         Returns:   0 if successful, -1 in the case of error
*/
int _JAMPROC JAMsysUnlink(JAMAPIRECptr apirec, CHAR8ptr pFileName)
{
    if(unlink(pFileName) == -1)
        {
        if (apirec)
            apirec->Errno = errno;
        return(-1);
        }

    return(0);
}

/*
**  JAMsysWrite
**
**  Writes the specified number of bytes from the specified filehandle
**
**      Parameters:   JAMAPIRECptr apirec - Ptr to JAMAPIREC to initiate
**                    FHANDLE fh          - Filehandle of file to write to
**                    VOIDptr pBuf        - Buffer with data to write
**                    INT32 Len           - Number of bytes to write
**
**         Returns:   The number of bytes written, or -1 in the case of error
**
**  NOTE! In small and medium memory model, the maximum size that can
**        be written is 0xFF00 bytes
*/
INT32 _JAMPROC JAMsysWrite(JAMAPIRECptr apirec, FHANDLE fh, VOIDptr pBuf, INT32 Len)
{
    INT32       TotalWrit = 0L;

    if((TotalWrit = write(fh, pBuf, Len)) == -1)
        {
        if (apirec)
            apirec->Errno = errno;
        }

    return(TotalWrit);

}

/* end of file "jamsys.c" */

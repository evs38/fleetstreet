/*---------------------------------------------------------------------------+
 | Titel: FTSAPI.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 21.03.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Interface HMSG <-> FTS MSGAPI32                                        |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define OS_2
#define INCL_WIN
#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <share.h>
#include <fcntl.h>
#include <sys\stat.h>
#define NO_MSGH_DEF
#include <msgapi.h>
#include <api_sdm.h>
#include "..\main.h"
#include "..\messages.h"
#include "..\structs.h"
#include "..\msgheader.h"
#include "..\areaman\areaman.h"
#include "handlemsg.h"
#include "kludgeapi.h"
#include "common.h"
#include "ftsapi.h"

/*--------------------------------- Defines ---------------------------------*/

#define LRFILE      "\\LASTREAD"          /* Name der SDM-Lastread - Files   */

#define ATTRIB_MASK (ATTRIB_PRIVATE | ATTRIB_CRASH | ATTRIB_RCVD | ATTRIB_SENT |\
                     ATTRIB_FILEATTACHED | ATTRIB_INTRANSIT | ATTRIB_ORPHAN |\
                     ATTRIB_KILLSENT | ATTRIB_LOCAL | ATTRIB_HOLD | ATTRIB_READ |\
                     ATTRIB_FREQUEST | ATTRIB_RRQ | ATTRIB_RECEIPT | ATTRIB_AUDIT |\
                     ATTRIB_UPDATEREQ |\
                     ATTRIB_ARCHIVESENT | ATTRIB_DIRECT | ATTRIB_HUBROUTE |\
                     ATTRIB_IMMEDIATE | ATTRIB_KILLFILE | ATTRIB_TRUNCFILE|\
                     ATTRIB_NPD )

#define SIZE_FINDBUFFER  4096

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/
static int M_ReadNumList(PAREADEFLIST pAreaDef, PCHAR pchPathFile);
static int M_QCompare(const void *el1, const void *el2);
static int WriteReadFlagDirect(int fh, ULONG ulAttrib);
static void RemoveBackslash(char *pString);

/*---------------------------------------------------------------------------*/
/* Funktionsname: FTS_QueryAttribMask                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liefert eine Maske der Attribute zurueck, die von           */
/*               FTS-Areas unterstuetzt werden                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: -                                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Attribut-Maske                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG  FTS_QueryAttribMask(void)
{
   return ATTRIB_MASK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FTS_OpenArea                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet eine FTS-Area                                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAreaDef: Area-Definition                                      */
/*            pchPathName: Pfadname der Area, bereits umgemappt              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf Area-Handle                                     */
/*                NULL: Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PVOID FTS_OpenArea(PAREADEFLIST pAreaDef, PCHAR pchPathName)
{
   PVOID pRet=NULL;
   USHORT usAreaType = MSGTYPE_SDM;

   if (pAreaDef->areadata.areatype != AREATYPE_NET)
      usAreaType |= MSGTYPE_ECHO;

   pRet = MsgOpenArea(pchPathName, MSGAREA_CRIFNEC, usAreaType);

   if (pRet)
   {
      pAreaDef->maxmessages = MsgGetNumMsg((HAREA) pRet);
      M_ReadNumList(pAreaDef, pchPathName);
   }

   return pRet;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FTS_CloseArea                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Schlie疸 eine FTS-Area                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pAreaDef: Area-Definition                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0   OK                                                     */
/*                sonst  Fehler                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Area-Handle wird von der rufenden Funktion geloescht.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_CloseArea(PAREADEFLIST pAreaDef)
{
   USHORT ret;

   ret = MsgCloseArea((HAREA)pAreaDef->areahandle);
   if (!ret)
   {
      if (pAreaDef->msgnumlist)
      {
         free(pAreaDef->msgnumlist);
         pAreaDef->msgnumlist=NULL;
      }
   }

   return ret;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG FTS_ReadLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset)
{
   unsigned long lastread;                        /* lastread record    */
   char work[LEN_PATHNAME+1];                     /* path to sql file   */
   int  num=0;                                    /* number of message  */
   int  sql;                                      /* file handle        */

   strcpy(work, pchPathName);
   RemoveBackslash(work);
   strcat(work, LRFILE);

   sql = sopen(work, O_BINARY|O_RDWR, SH_DENYNO, S_IWRITE|S_IREAD);
   if (sql != -1)
   {
      lseek(sql, lOffset * sizeof(long), SEEK_SET);
      if (read(sql, &lastread, sizeof(long)) == sizeof(long))
      {
         lastread = lastread & 0xffff;
         num = FTS_UidToMsgn(pAreaDef, lastread, FALSE);
      }
      close(sql);
   }

   if (pAreaDef->oldlastread==0)
      pAreaDef->oldlastread=lastread;

   return num;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_WriteLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset)
{
   unsigned long lastread;
   char work[LEN_PATHNAME+1];
   int ret = 0;
   int last;
   int sql;

   strcpy(work, pchPathName);
   RemoveBackslash(work);
   strcat(work, LRFILE);

   sql = sopen(work, O_BINARY | O_RDWR, SH_DENYNO, S_IWRITE | S_IREAD);

   if (sql == -1)
      if (errno != EACCES && errno != EMFILE)
      {
         sql = sopen(work, O_BINARY|O_WRONLY|O_CREAT, SH_DENYNO, S_IWRITE|S_IREAD);
         if (sql == -1)
            ret = 1;
      }

   if (ret == 0)
   {
      lseek(sql, lOffset * sizeof(long), SEEK_SET);
      if (pAreaDef->maxmessages == 0)
         last=0;
      else
         last = (int) pAreaDef->currentmessage;

      lastread = FTS_MsgnToUid(pAreaDef, last);
      lastread |= (pAreaDef->msgnumlist[pAreaDef->maxmessages-1] << 16);

      write(sql, (PCHAR) &lastread, sizeof(long));
      close(sql);
   }
   return ret;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG  FTS_UidToMsgn(PAREADEFLIST pAreaDef, ULONG msgID, BOOL exact)
{
   int i=0;

   if (!pAreaDef->areahandle)
      return 0;

   if (pAreaDef->maxmessages==0)
      return 0;

   while (i<pAreaDef->maxmessages && pAreaDef->msgnumlist[i]< msgID)
      i++;

   if (i<pAreaDef->maxmessages)
   {
     if (pAreaDef->msgnumlist[i] == msgID)
        return i+1;
     else
     {
        if (exact)
           return 0;
        else
           return i;
     }
   }
   else
     if (exact)
        return 0;
     else
        return i;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG  FTS_MsgnToUid(PAREADEFLIST pAreaDef, int msgnum)
{
   if (pAreaDef->areahandle==NULL)
      return 0;

   if (pAreaDef->maxmessages==0)
      return 0;

   if (!pAreaDef->msgnumlist)
      return 0;
   else
      return pAreaDef->msgnumlist[msgnum-1];
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   USHORT usRet;

   usRet = ReadMessage(pAreaDef, pHeader, pMessage, msgnum?(pAreaDef->msgnumlist[msgnum-1]):0);

   if (!usRet)
      pHeader->ulMsgID = pAreaDef->msgnumlist[msgnum-1];

   return usRet;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_ReadHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum)
{
   HMSG msghandle;
   XMSG header;
   USHORT ret;
   BOOL bRead=FALSE;

   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_READ,
                              pAreaDef->msgnumlist[msgnum-1])))
   {
      return MSG_READ_ERROR;
   }
   ret = MsgReadMsg(msghandle, &header, 0, 0, NULL, 0, NULL);
   bRead = QueryReadFlag(msghandle);

   MsgCloseMsg(msghandle);         /* Am besten sofort wieder schliessen */

   if (!ret)
   {
      Xmsg2Msgheader(&header, pHeader);
      if (bRead)
         pHeader->ulAttrib |= ATTRIB_READ;
      pHeader->ulMsgID = pAreaDef->msgnumlist[msgnum-1];
   }

   return ret;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_WriteHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum, PDRIVEREMAP pdriveremap, BOOL bReplyAndAttr)
{
   HMSG msghandle;
   ULONG       ulMsgID;
   char        pchMsgName[LEN_PATHNAME+1];
   char        pchTemp[20];
   int         pfMsg;
   XMSG        header;

   Msgheader2Xmsg(&header, pHeader);

   if (bReplyAndAttr)
   {
      ulMsgID = pAreaDef->msgnumlist[msgnum-1];
      if (ulMsgID)
      {
         MSG_RemapArea(pchMsgName, pAreaDef, pdriveremap);
         RemoveBackslash(pchMsgName);
         sprintf(pchTemp, "\\%d.MSG", ulMsgID);
         strcat(pchMsgName, pchTemp);

         if (pfMsg = _sopen(pchMsgName, O_WRONLY | O_BINARY, SH_DENYRW))
         {
            if (_lseek(pfMsg, 184, SEEK_SET)>=0) /* Offset v. ReplyTo (16 Bit) */
            {
               /* Attribute neu schreiben */
               _write(pfMsg, &header.replyto, 2);  /* 2 Bytes */
               _write(pfMsg, &header.attr, 2);   /* 2 Bytes */
               _write(pfMsg, &header.replies, 2);  /* 2 Bytes */
               WriteReadFlagDirect(pfMsg, pHeader->ulAttrib);
               _close(pfMsg);
            }
            else
            {
               _close(pfMsg);
               return MSG_WRITE_ERROR;
            }
         }
         else
         {
            return MSG_WRITE_ERROR;
         }
      }
   }
   else
   {
      if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_WRITE,
                                 pAreaDef->msgnumlist[msgnum-1])))
      {
         return MSG_OPEN_ERROR;
      }
      if (MsgWriteMsg(msghandle, 0, &header, NULL, 0, 0, 0, NULL))
      {
         return MSG_WRITE_ERROR;
      }
      WriteReadFlag(msghandle, pHeader->ulAttrib);
      MsgCloseMsg(msghandle);         /* Am besten sofort wieder schliessen */
   }

   return OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_LinkMessages(PAREADEFLIST pAreaDef, ULONG ulReplyID, ULONG ulOrigID, PDRIVEREMAP pdriveremap)
{
   char        pchMsgName[LEN_PATHNAME+1];
   char        pchTemp[20];
   int         pfMsg;

   if (!FTS_UidToMsgn(pAreaDef, ulReplyID, TRUE))
      return 1;

   if (!FTS_UidToMsgn(pAreaDef, ulOrigID, TRUE))
      return 1;


   /* Original updaten */
   if (ulOrigID)
   {
      MSG_RemapArea(pchMsgName, pAreaDef, pdriveremap);
      RemoveBackslash(pchMsgName);
      sprintf(pchTemp, "\\%d.MSG", ulOrigID);
      strcat(pchMsgName, pchTemp);
      if (pfMsg = _sopen(pchMsgName, O_WRONLY | O_BINARY, SH_DENYRW))
      {
         if (_lseek(pfMsg, 188, SEEK_SET)>=0) /* Offset v. Replies */
         {
            /* Attribute neu schreiben */
            _write(pfMsg, &ulReplyID, 2);  /* 2 Bytes */
            _close(pfMsg);
         }
         else
         {
            _close(pfMsg);
            return 1;
         }
      }
      else
      {
         return 1;
      }
   }

   /* Reply updaten */
   if (ulReplyID)
   {
      MSG_RemapArea(pchMsgName, pAreaDef, pdriveremap);
      RemoveBackslash(pchMsgName);
      sprintf(pchTemp, "\\%d.MSG", ulReplyID);
      strcat(pchMsgName, pchTemp);
      if (pfMsg = _sopen(pchMsgName, O_WRONLY | O_BINARY, SH_DENYRW))
      {
         if (_lseek(pfMsg, 184, SEEK_SET)>=0) /* Offset v. Replies */
         {
            /* Attribute neu schreiben */
            _write(pfMsg, &ulOrigID, 2);  /* 2 Bytes */
            _close(pfMsg);
         }
         else
         {
            _close(pfMsg);
            return 1;
         }
      }
      else
      {
         return 1;
      }
   }


   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int FTS_UnlinkMessage(PAREADEFLIST pAreaDef, int msgnum, PDRIVEREMAP pdriveremap)
{
   MSGHEADER Header, Header2;
   int number;

   if (!FTS_ReadHeader(pAreaDef, &Header, msgnum))
   {
      /* Abwaerts-Referenz */
      number = FTS_UidToMsgn(pAreaDef, Header.ulReplies[0], TRUE);
      if (number)
      {
         /* Link loeschen */
         if (!FTS_ReadHeader(pAreaDef, &Header2, number))
         {
            Header2.ulReplyTo = 0;
            FTS_WriteHeader(pAreaDef, &Header2, number, pdriveremap, TRUE);
         }
      }

      /* Aufwaerts-Referenz */
      number = FTS_UidToMsgn(pAreaDef, Header.ulReplyTo, TRUE);
      if (number)
      {
         /* Link loeschen */
         if (!FTS_ReadHeader(pAreaDef, &Header2, number))
         {
            Header2.ulReplies[0] = 0;
            FTS_WriteHeader(pAreaDef, &Header2, number, pdriveremap, TRUE);
         }
      }

      /* Message selbst */
      Header.ulReplyTo = 0;
      Header.ulReplies[0]=0;
      FTS_WriteHeader(pAreaDef, &Header, msgnum, pdriveremap, TRUE);
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_MarkRead(PAREADEFLIST pAreaDef, int msgnum, BOOL bPersonal, PDRIVEREMAP pdriveremap)
{
   char        pchMsgName[LEN_PATHNAME+1];
   char        pchTemp[20];
   int         pfMsg;
   ULONG       ulNum;
   USHORT      usAttr;

   if (!(ulNum = FTS_MsgnToUid(pAreaDef, msgnum)))
      return 1;

   MSG_RemapArea(pchMsgName, pAreaDef, pdriveremap);
   RemoveBackslash(pchMsgName);
   sprintf(pchTemp, "\\%d.MSG", ulNum);
   strcat(pchMsgName, pchTemp);
   if (pfMsg = _sopen(pchMsgName, O_RDWR | O_BINARY, SH_DENYRW))
   {
      if (_lseek(pfMsg, 186, SEEK_SET)>=0) /* Offset v. Attrib */
      {
         /* Attribute lesen schreiben */
         _read(pfMsg, &usAttr, 2);
         if (bPersonal)
            usAttr |= MSGREAD;
         _lseek(pfMsg, 186, SEEK_SET);
         _write(pfMsg, &usAttr, 2);  /* 2 Bytes */

         WriteReadFlagDirect(pfMsg, ATTRIB_READ);
         _close(pfMsg);
      }
      else
      {
         _close(pfMsg);
         return 1;
      }
   }
   else
   {
      return 1;
   }


   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_AddMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, PDRIVEREMAP pDriveRemap)
{
   char pchPath[LEN_PATHNAME+1];
   USHORT usRet;

   usRet = WriteMessage(pAreaDef, pHeader, pMessage, 0);

   MSG_RemapArea(pchPath, pAreaDef, pDriveRemap);
   M_ReadNumList(pAreaDef, pchPath);

   if (!usRet)
      pHeader->ulMsgID = FTS_MsgnToUid(pAreaDef, pAreaDef->maxmessages);

   return usRet;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_ChangeMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   return WriteMessage(pAreaDef, pHeader, pMessage, pAreaDef->msgnumlist[msgnum-1]);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FTS_KillMessage(PAREADEFLIST pAreaDef, int msgnum)
{
   int i;

   if (!MsgKillMsg((HAREA)pAreaDef->areahandle, pAreaDef->msgnumlist[msgnum-1]))
   {
      pAreaDef->maxmessages--;
      if (pAreaDef->currentmessage > pAreaDef->maxmessages)
         pAreaDef->currentmessage = pAreaDef->maxmessages;
      if (pAreaDef->maxmessages==0)
         pAreaDef->msgnumlist[0]=0;
      else
         for (i=msgnum; i <= pAreaDef->maxmessages; i++)
            pAreaDef->msgnumlist[i-1]=pAreaDef->msgnumlist[i];
      return OK;
   }
   else
   {
      return MSG_DELETE_ERROR;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int FTS_RenumberArea(PAREADEFLIST pAreaDef, HWND hwndProgress, PDRIVEREMAP pDriveRemap)
{
   ULONG newnum=0;
   ULONG index=0;
   MSGHEADER Header, Header2;
   int refmsg=0;
   char pchOldName[LEN_PATHNAME+1];
   char pchNewName[LEN_PATHNAME+1];
   char pchTemp[15];

   if (pAreaDef->maxmessages > 0)
   {
      WinPostMsg(hwndProgress, RENM_STAGE, MPFROMLONG(0), NULL);

      /* Alle Messages durchgehen */
      for (index=0; index < pAreaDef->maxmessages; index++)
      {
         WinPostMsg(hwndProgress, RENM_PROGRESS,
                        MPFROMLONG(index+1), MPFROMLONG(pAreaDef->maxmessages));

         newnum= index+1;  /* neue Nummer der Message */

         if (!FTS_ReadHeader(pAreaDef, &Header, index+1))
         {
            /* Verweise auf Replies anpassen */
            refmsg=FTS_UidToMsgn(pAreaDef, Header.ulReplies[0], TRUE);
            if (refmsg)
            {
               if (!FTS_ReadHeader(pAreaDef, &Header2, refmsg))
               {
                  if (Header2.ulReplyTo != newnum)
                  {
                     Header2.ulReplyTo=newnum;
                     FTS_WriteHeader(pAreaDef, &Header2, refmsg, pDriveRemap, TRUE);
                  }
               }
            }

            /* Aufwaerts-Referenz anpassen */
            refmsg=FTS_UidToMsgn(pAreaDef, Header.ulReplyTo, TRUE);
            if (refmsg)
            {
               if (!FTS_ReadHeader(pAreaDef, &Header2, refmsg))
               {
                  if (Header2.ulReplies[0] == pAreaDef->msgnumlist[index] &&
                      Header2.ulReplies[0] != newnum)
                  {
                     Header2.ulReplies[0]= newnum;

                     FTS_WriteHeader(pAreaDef, &Header2, refmsg, pDriveRemap, TRUE);
                  }
               }
            }
         }
      }

      /* Messages umbenennen */
      WinPostMsg(hwndProgress, RENM_STAGE, MPFROMLONG(1), NULL);
      for (index=0; index < pAreaDef->maxmessages; index++)
      {
         WinPostMsg(hwndProgress, RENM_PROGRESS,
                    MPFROMLONG(index+1), MPFROMLONG(pAreaDef->maxmessages));
         strcpy(pchOldName, pAreaDef->areadata.pathfile);
         RemoveBackslash(pchOldName);
         sprintf(pchTemp, "\\%d.MSG", pAreaDef->msgnumlist[index]);
         strcat(pchOldName, pchTemp);

         strcpy(pchNewName, pAreaDef->areadata.pathfile);
         RemoveBackslash(pchNewName);
         sprintf(pchTemp, "\\%d.MSG", index+1);
         strcat(pchNewName, pchTemp);

         rename(pchOldName, pchNewName);
         pAreaDef->msgnumlist[index]=index+1;
      }
   }

   return OK;
}

/*ﾕﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍ M_ReadNumList  ﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍｸ
  ｳ M_ReadNumList liesst fuer FTS-Areas die Messagenummernliste ein.        ｳ
  ｳ Das entehende Array hat so viele Elemente wie Messages in der Area vor- ｳ
  ｳ handen sind. In jedem Arrayelement steht die Nummer der Message.        ｳ
  ｳ Bei einem Fehler wird 1 zurueckgegeben, sonst 0.                        ｳ
  ｳ                                                                         ｳ
  ﾔﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍｾ*/

static int M_ReadNumList(PAREADEFLIST pAreaDef, PCHAR pchPathFile)
{
   PFILEFINDBUF3 pfindbuf, pResult;
   ULONG ulNumMsgs;
   ULONG ulFindCount=SIZE_FINDBUFFER/sizeof(FILEFINDBUF3);
   ULONG ulMsgNum;
   ULONG ulCount=0;
   HDIR hdir=HDIR_CREATE;
   char pchPath[LEN_PATHNAME+1];

   if (pAreaDef->msgnumlist)
   {
      /* alte Liste freigeben */
      free(pAreaDef->msgnumlist);
      pAreaDef->msgnumlist=NULL;
   }

   /* Anzahl der Messages holen */
   ulNumMsgs=pAreaDef->maxmessages;

   if (ulNumMsgs==0)
      return 0;

   pAreaDef->msgnumlist=malloc(ulNumMsgs * sizeof(ULONG));
   strcpy(pchPath, pchPathFile);
   RemoveBackslash(pchPath);
   strcat(pchPath, "\\*.MSG");

   pfindbuf=malloc(SIZE_FINDBUFFER);

   if (DosFindFirst(pchPath, &hdir, FILE_ARCHIVED | FILE_READONLY, pfindbuf,
                    SIZE_FINDBUFFER, &ulFindCount, FIL_STANDARD))
   {
      free(pAreaDef->msgnumlist);
      pAreaDef->msgnumlist=NULL;
      return 1;
   }
   do
   {
      pResult = pfindbuf;
      do
      {
         ulMsgNum=strtoul(pResult->achName, NULL, 10);
         if (ulMsgNum>0 && ulCount < ulNumMsgs)   /* 0.MSG und abc.MSG ignorieren */
            pAreaDef->msgnumlist[ulCount++]=ulMsgNum;
         if (pResult->oNextEntryOffset)
            pResult = (PFILEFINDBUF3)(((PCHAR) pResult) + pResult->oNextEntryOffset);
         else
            pResult = NULL;

      } while (pResult);

      ulFindCount=SIZE_FINDBUFFER/sizeof(FILEFINDBUF3);
   } while (ulCount<ulNumMsgs &&
            DosFindNext(hdir, pfindbuf, SIZE_FINDBUFFER, &ulFindCount)<ERROR_NO_MORE_FILES);
   DosFindClose(hdir);
   free(pfindbuf);

   /* Array noch sortieren */
   qsort(pAreaDef->msgnumlist, ulNumMsgs, sizeof(ULONG), M_QCompare);

   return 0;
}

/*ﾕﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍ M_QCompare     ﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍｸ
  ｳ Simple Vergleichsfunktion fuer Quicksort.                               ｳ
  ｳ                                                                         ｳ
  ﾔﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍｾ*/

static int M_QCompare(const void *el1, const void *el2)
{
   if (*(PULONG)el1 < *(PULONG)el2)
      return -1;
   if (*(PULONG)el1 > *(PULONG)el2)
      return 1;
   return 0;
}

int WriteReadFlag(HMSG hmsg, ULONG ulAttrib)
{
   /* File-Handle der Message besorgen */
   int fh = hmsg->fd;

   return WriteReadFlagDirect(fh, ulAttrib);
}

static int WriteReadFlagDirect(int fh, ULONG ulAttrib)
{
   USHORT usAttr;

   if (ulAttrib & ATTRIB_READ)
      usAttr = 1;
   else
      usAttr = 0;

   if (_lseek(fh, 164, SEEK_SET) >=0)  /* Offset v. "times read" */
   {
      _write(fh, &usAttr, sizeof(usAttr));
      return 0;
   }
   else
      return 1;
}

BOOL QueryReadFlag(HMSG hmsg)
{
   int fh = hmsg->fd;
   USHORT usAttr=0;

   if (_lseek(fh, 164, SEEK_SET) >=0)  /* Offset v. "times read" */
   {
      _read(fh, &usAttr, sizeof(usAttr));
      return usAttr;
   }
   else
      return 0;
}

static void RemoveBackslash(char *pString)
{
   size_t len;

   if (!pString)
      return;

   len = strlen(pString);
   if (len && pString[len-1] == '\\')
      pString[len-1] = 0;

   return;
}

/*-------------------------------- Modulende --------------------------------*/


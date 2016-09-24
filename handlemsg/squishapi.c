/*---------------------------------------------------------------------------+
 | Titel: SQUISHAPI.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 21.03.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Interface HMSG <-> Squish MSGAPI32                                      |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define OS_2
#define INCL_BASE
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <share.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <msgapi.h>
#include "..\main.h"
#include "..\structs.h"
#include "..\msgheader.h"
#include "..\util\addrcnv.h"
#include "..\areaman\areaman.h"
#include "handlemsg.h"
#include "kludgeapi.h"
#include "common.h"
#include "squishapi.h"

/*--------------------------------- Defines ---------------------------------*/

#define LASTREADEXT ".SQL"                /* Extension der Squish-Lastread - */
                                          /* Files                           */

#define ATTRIB_MASK (ATTRIB_PRIVATE | ATTRIB_CRASH | ATTRIB_RCVD | ATTRIB_SENT |\
                     ATTRIB_FILEATTACHED | ATTRIB_INTRANSIT | ATTRIB_ORPHAN |\
                     ATTRIB_KILLSENT | ATTRIB_LOCAL | ATTRIB_HOLD | ATTRIB_READ |\
                     ATTRIB_FREQUEST | ATTRIB_RRQ | ATTRIB_RECEIPT | ATTRIB_AUDIT |\
                     ATTRIB_UPDATEREQ | ATTRIB_SCANNED |\
                     ATTRIB_ARCHIVESENT | ATTRIB_DIRECT | ATTRIB_HUBROUTE |\
                     ATTRIB_IMMEDIATE | ATTRIB_KILLFILE | ATTRIB_TRUNCFILE | ATTRIB_KEEP|\
                     ATTRIB_NPD )

/*#define MY_ALLOC*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

#ifdef MY_ALLOC
void * _System sysmalloc(size_t size);
void _System sysfree(void *ptr);
void * _System sysrealloc(void *ptr, size_t size);
#endif

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

ULONG SQ_QueryAttribMask(void)
{
   return ATTRIB_MASK;
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

USHORT SQ_OpenApi(PCHAR mainaddress)
{
   struct _minf apiinf={0, 0, 0, 0, 0, 0, 0, 0, 0};

#if 0
   FTNADDRESS address;

   StringToNetAddr(mainaddress, &address, NULL);
   apiinf.def_zone = address.usZone;
#endif

#ifdef MY_ALLOC
   apiinf.req_version =  1;
   apiinf.palloc = apiinf.farpalloc = sysmalloc;
   apiinf.repalloc = apiinf.farrepalloc = sysrealloc;
   apiinf.pfree = apiinf.farpfree = sysfree;
#endif

   return MsgOpenApi(&apiinf);
}

#ifdef MY_ALLOC
void * _System sysmalloc(size_t size)
{
   return malloc(size);
}

void _System sysfree(void *ptr)
{
   free(ptr);
   return;
}

void * _System sysrealloc(void *ptr, size_t size)
{
   return realloc(ptr, size);
}

#endif

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

USHORT SQ_CloseApi(void)
{
   return MsgCloseApi();
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

PVOID  SQ_OpenArea(PAREADEFLIST pAreaDef, PCHAR pchPathName)
{
   PVOID pRet=NULL;

   pRet = MsgOpenArea(pchPathName, MSGAREA_CRIFNEC, MSGTYPE_SQUISH);

   if (pRet)
      pAreaDef->maxmessages = MsgGetNumMsg((HAREA)pRet);

   return pRet;
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

USHORT SQ_CloseArea(PAREADEFLIST pAreaDef)
{
   return MsgCloseArea((HAREA)pAreaDef->areahandle);
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

ULONG  SQ_ReadLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset)
{
   unsigned long lastread;                        /* lastread record    */
   char work[LEN_PATHNAME+1];                     /* path to sql file   */
   int  num=0;                                    /* number of message  */
   int  sql;                                      /* file handle        */

   strcpy(work, pchPathName);
   strcat(work, LASTREADEXT);

   sql = sopen(work, O_BINARY|O_RDWR, SH_DENYNO, S_IWRITE|S_IREAD);
   if (sql != -1)
   {
      lseek(sql, lOffset * sizeof(long), SEEK_SET);
      if (read(sql, &lastread, sizeof(long)) == sizeof(long))
      {
         num = SQ_UidToMsgn(pAreaDef, lastread, FALSE);
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

USHORT SQ_WriteLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset)
{
   unsigned long lastread;
   char work[LEN_PATHNAME+1];
   int ret = OK;
   int last;
   int sql;

   strcpy(work, pchPathName);
   strcat(work, LASTREADEXT);

   sql = sopen(work, O_BINARY | O_RDWR, SH_DENYNO, S_IWRITE | S_IREAD);

   if (sql == -1)
      if (errno != EACCES && errno != EMFILE)
      {
         sql = sopen(work, O_BINARY|O_WRONLY|O_CREAT, SH_DENYNO, S_IWRITE|S_IREAD);
         if (sql == -1)
            ret = 1;
      }

   if (ret == OK)
   {
      lseek(sql, lOffset * sizeof(long), SEEK_SET);
      if (pAreaDef->maxmessages == 0)
         last=0;
      else
         last = (int) pAreaDef->currentmessage;

      lastread = SQ_MsgnToUid(pAreaDef, last);

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

ULONG  SQ_UidToMsgn(PAREADEFLIST pAreaDef, ULONG msgID, BOOL exact)
{
   int msgnum;

   if (pAreaDef->areahandle==NULL)
      return 0;

   if (pAreaDef->maxmessages==0)
      return 0;

   if (exact)
      msgnum=MsgUidToMsgn((HAREA)pAreaDef->areahandle, msgID, UID_EXACT);
   else
      msgnum=MsgUidToMsgn((HAREA)pAreaDef->areahandle, msgID, UID_PREV);

   return msgnum;
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

ULONG  SQ_MsgnToUid(PAREADEFLIST pAreaDef, int msgnum)
{
   ULONG uid;

   if (pAreaDef->areahandle==NULL)
      return 0;

   if (pAreaDef->maxmessages==0)
      return 0;

   uid=MsgMsgnToUid((HAREA)pAreaDef->areahandle, msgnum);

   return uid;
}

/*ﾕﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍ M_ReadMessage  ﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍｸ
  ｳ M_ReadMessage liest die durch currentmessage angegebene Message ein und ｳ
  ｳ stellt sie in der MESSAGEINFO-Struktur zur Verf“ung.                   ｳ
  ｳ Die Message wird kurz mit MsgOpenMsg ge杷fnet und eingelesen, danach    ｳ
  ｳ wieder mit MsgCloseMsg geschlossen, um anderen Programmen einen Zugriff ｳ
  ｳ zu gew�hren(???).                                                       ｳ
  ｳ                                                                         ｳ
  ｳ Beim Lesen aus Echo-Areas wird die Absenderadresse in der Origin-Line   ｳ
  ｳ und dann evtl. in der Msgid gesucht und in den Header eingetragen, damitｳ
  ｳ man es gleich beim Lesen ｜erpr’en kann.                               ｳ
  ｳ                                                                         ｳ
  ﾔﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍﾍｾ*/

USHORT SQ_ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   USHORT usRet;

   usRet = ReadMessage(pAreaDef, pHeader, pMessage, msgnum);
   if (!usRet && !pHeader->ulMsgID)
      pHeader->ulMsgID = SQ_MsgnToUid(pAreaDef, msgnum);

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

USHORT SQ_ReadHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum)
{
   HMSG msghandle;
   XMSG header;
   USHORT ret;

   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_READ, msgnum)))
   {
      return MSG_READ_ERROR;
   }
   ret = MsgReadMsg(msghandle, &header, 0, 0, NULL, 0, NULL);
   MsgCloseMsg(msghandle);         /* Am besten sofort wieder schliessen */

   if (!ret)
   {
      Xmsg2Msgheader(&header, pHeader);
      if (header.attr & FLEET_READ)
         pHeader->ulAttrib |= ATTRIB_READ;
      if (!pHeader->ulMsgID)
         pHeader->ulMsgID = SQ_MsgnToUid(pAreaDef, msgnum);
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

USHORT SQ_WriteHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum)
{
   HMSG msghandle;
   XMSG header;

   Msgheader2Xmsg(&header, pHeader);
   if (pHeader->ulAttrib & ATTRIB_READ)
      header.attr |= FLEET_READ;

   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_WRITE, msgnum)))
   {
      return MSG_OPEN_ERROR;
   }
   if (MsgWriteMsg(msghandle, 0, &header, NULL, 0, 0, 0, NULL))
   {
      return MSG_WRITE_ERROR;
   }
   MsgCloseMsg(msghandle);         /* Am besten sofort wieder schliessen */

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

USHORT SQ_LinkMessages(PAREADEFLIST pAreaDef, ULONG ulReplyID, ULONG ulOrigID)
{
   HMSG msghandle;
   XMSG header;
   int ReplyNr, OrigNr;
   int i=0;

   if (!(ReplyNr = SQ_UidToMsgn(pAreaDef, ulReplyID, TRUE)))
      return 1;

   if (!(OrigNr = SQ_UidToMsgn(pAreaDef, ulOrigID, TRUE)))
      return 1;


   /* Original-Message */
   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_RW, OrigNr)))
   {
      return MSG_READ_ERROR;
   }
   MsgReadMsg(msghandle, &header, 0, 0, NULL, 0, NULL);

   while (i<MAX_REPLY && header.replies[i])
      i++;
   if (i==MAX_REPLY)
      i--;    /* Letzten Reply ggf. ueberschreiben */
   header.replies[i]=ulReplyID;
   MsgWriteMsg(msghandle, 0, &header, NULL, 0, 0, 0, NULL);
   MsgCloseMsg(msghandle);

   /* Reply-Message */
   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_RW, ReplyNr)))
   {
      return MSG_READ_ERROR;
   }
   MsgReadMsg(msghandle, &header, 0, 0, NULL, 0, NULL);

   header.replyto=ulOrigID;
   MsgWriteMsg(msghandle, 0, &header, NULL, 0, 0, 0, NULL);
   MsgCloseMsg(msghandle);

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

int SQ_UnlinkMessage(PAREADEFLIST pAreaDef, int msgnum)
{
   MSGHEADER Header, Header2;
   ULONG msgID;
   int i;
   int number;
   int Ret;

   Ret = SQ_ReadHeader(pAreaDef, &Header, msgnum);

   if (!Ret)
   {
      /* Verweise in den Replies loeschen
      ﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄ */
      for (i=0; i<NUM_REPLIES && Header.ulReplies[i]; i++)
      {
         number = SQ_UidToMsgn(pAreaDef, Header.ulReplies[i], TRUE);

         if (number)
         {
            if (!SQ_ReadHeader(pAreaDef, &Header2, number))
            {
               Header2.ulReplyTo=0;
               SQ_WriteHeader(pAreaDef, &Header2, number);
            }
         }
      }

      /* UMSGID fuer diese Message berechnen */
      msgID=SQ_MsgnToUid(pAreaDef, msgnum);


      /* Verweis in ReplyTo loeschen
      ﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄ */
      if (Header.ulReplyTo)
      {
         number=SQ_UidToMsgn(pAreaDef, Header.ulReplyTo, TRUE);

         if (number)
         {
            if (!SQ_ReadHeader(pAreaDef, &Header2, number))
            {
               i=0;
               while (i<NUM_REPLIES && Header2.ulReplies[i]!=msgID)
                  i++;
               if (i<NUM_REPLIES)
               {
                  for(;i<NUM_REPLIES-1; i++)
                     Header2.ulReplies[i]=Header2.ulReplies[i+1];
                  Header2.ulReplies[NUM_REPLIES-1]=0;
               }
               SQ_WriteHeader(pAreaDef, &Header2, number);
            }
         }
      }

      /* Links loeschen */
      Header.ulReplyTo = 0;
      memset(Header.ulReplies, 0, sizeof(Header.ulReplies));

      SQ_WriteHeader(pAreaDef, &Header, msgnum);
   }

   return Ret;
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

USHORT SQ_MarkRead(PAREADEFLIST pAreaDef, int msgnum, BOOL bPersonal)
{
   HMSG msghandle;
   XMSG header;
   BOOL bWrite=FALSE;


   /* Original-Message */
   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_RW, msgnum)))
   {
      return MSG_READ_ERROR;
   }
   MsgReadMsg(msghandle, &header, 0, 0, NULL, 0, NULL);

#if 0
   if (!(header.attr & MSGXX2))
   {
      header.attr |= MSGXX2;
#else
   if (!(header.attr & FLEET_READ))
   {
      header.attr |= FLEET_READ;
#endif
      bWrite = TRUE;
   }
   if (bPersonal && !(header.attr & MSGREAD))
   {
      header.attr |= MSGREAD;
      bWrite = TRUE;
   }

   if (bWrite)
      MsgWriteMsg(msghandle, 0, &header, NULL, 0, 0, 0, NULL);
   MsgCloseMsg(msghandle);

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

USHORT SQ_AddMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage)
{
   USHORT rc=0;

   rc = WriteMessage(pAreaDef, pHeader, pMessage, 0);

   if (!rc)
      pHeader->ulMsgID = SQ_MsgnToUid(pAreaDef, pAreaDef->maxmessages);

   return rc;
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

USHORT SQ_ChangeMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   return WriteMessage(pAreaDef, pHeader, pMessage, msgnum);
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

USHORT SQ_KillMessage(PAREADEFLIST pAreaDef, int msgnum)
{
   if (!MsgKillMsg((HAREA)pAreaDef->areahandle, msgnum))
   {
      pAreaDef->maxmessages--;
      if (pAreaDef->currentmessage > pAreaDef->maxmessages)
         pAreaDef->currentmessage = pAreaDef->maxmessages;
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

int SQ_ReadSquishParams(PAREADEFLIST pAreaDef, PSQUISHPARAMS pSquishParams, PDRIVEREMAP pDriveRemap)
{
   char pchFileName[LEN_PATHNAME+1];
   FILE *AreaFile;
   int rc=OK;

   MSG_RemapArea(pchFileName, pAreaDef, pDriveRemap);
   strcat(pchFileName, ".SQD");

   if (AreaFile=fopen(pchFileName, "rb"))
   {
      if (!fseek(AreaFile, 12, SEEK_SET))
      {
         fread(&(pSquishParams->ulSkipMsgs), sizeof(ULONG), 1, AreaFile);

         if (!fseek(AreaFile, 124, SEEK_SET))
         {
            fread(&(pSquishParams->ulMaxMsgs), sizeof(ULONG), 1, AreaFile);

            if (!fseek(AreaFile, 128, SEEK_SET))
               fread(&(pSquishParams->usDaysToKeep), sizeof(USHORT), 1, AreaFile);
            else
               rc=ERROR;
         }
         else
            rc=ERROR;
      }
      else
         rc=ERROR;
   }
   else
      return AREA_OPEN_ERROR;

   fclose(AreaFile);
   return rc;
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

int SQ_WriteSquishParams(PAREADEFLIST pAreaDef, PSQUISHPARAMS pSquishParams, PDRIVEREMAP pDriveRemap)
{
   char pchFileName[LEN_PATHNAME+1];
   FILE *AreaFile;
   int rc=OK;

   MSG_RemapArea(pchFileName, pAreaDef, pDriveRemap);
   strcat(pchFileName, ".SQD");

   if (AreaFile=fopen(pchFileName, "r+b"))
   {
      if (!fseek(AreaFile, 12, SEEK_SET))
      {
         fwrite(&(pSquishParams->ulSkipMsgs), sizeof(ULONG), 1, AreaFile);

         if (!fseek(AreaFile, 124, SEEK_SET))
         {
            fwrite(&(pSquishParams->ulMaxMsgs), sizeof(ULONG), 1, AreaFile);

            if (!fseek(AreaFile, 128, SEEK_SET))
               fwrite(&(pSquishParams->usDaysToKeep), sizeof(USHORT), 1, AreaFile);
            else
               rc=ERROR;
         }
         else
            rc=ERROR;
      }
      else
         rc=ERROR;
   }
   else
      return AREA_OPEN_ERROR;

   fclose(AreaFile);
   return rc;
}
/*-------------------------------- Modulende --------------------------------*/


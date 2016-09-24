/*---------------------------------------------------------------------------+
 | Titel: COMMON.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 22.03.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Gemeinsame Funktionen von SQUISHAPI.C und FTSAPI.C                      |
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
#include <msgapi.h>
#include "..\main.h"
#include "..\structs.h"
#include "..\msgheader.h"
#include "..\util\addrcnv.h"
#include "..\areaman\areaman.h"
#include "handlemsg.h"
#include "kludgeapi.h"
#include "ftsapi.h"
#include "common.h"

/*--------------------------------- Defines ---------------------------------*/

#define MSGID_KLUDGE     "MSGID:"
#define MAX_KLUDGEBUFFER 3000

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static ATTRIBMAP AttribMap[]=
{
   {"A/S", ATTRIB_ARCHIVESENT},
   {"DIR", ATTRIB_DIRECT},
   {"HUB", ATTRIB_HUBROUTE},
   {"IMM", ATTRIB_IMMEDIATE},
   {"KFS", ATTRIB_KILLFILE},
   {"TFS", ATTRIB_TRUNCFILE},
   {"NPD", ATTRIB_NPD},
   {"", 0},
};

/*----------------------- interne Funktionsprototypen -----------------------*/

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

void Xmsg2Msgheader(PXMSG pXmsg, PMSGHEADER pHeader)
{
   pHeader->ulAttrib = pXmsg->attr & 0x0001fbffUL;
   if (pXmsg->attr & FLEET_KEEP)
      pHeader->ulAttrib |= ATTRIB_KEEP;
   memcpy(pHeader->pchFromName, pXmsg->from, LEN_USERNAME+1);
   memcpy(pHeader->pchToName,   pXmsg->to,   LEN_USERNAME+1);
   memcpy(pHeader->pchSubject,  pXmsg->subj, LEN_SUBJECT+1);

   memcpy(&pHeader->FromAddress, &pXmsg->orig, sizeof(FTNADDRESS));
   memcpy(&pHeader->ToAddress,   &pXmsg->dest, sizeof(FTNADDRESS));

   memcpy(&pHeader->StampWritten, &pXmsg->date_written, sizeof(TIMESTAMP));

   /* special workaround for old MSGAPI32.DLLs: ignore year in timestamp,
      recreate year from FTSC string if present */

   if (pXmsg->__ftsc_date[0])
   {
      int year;

      year = (pXmsg->__ftsc_date[7] - '0') * 10 + (pXmsg->__ftsc_date[8] - '0');
      if (year >= 80)
         pHeader->StampWritten.year = year - 80;
      else
         pHeader->StampWritten.year = year + 20;
   }

   memcpy(&pHeader->StampArrived, &pXmsg->date_arrived, sizeof(TIMESTAMP));

   pHeader->ulReplyTo = pXmsg->replyto;
   memset(pHeader->ulReplies, 0, sizeof(pHeader->ulReplies));
   memcpy(pHeader->ulReplies, pXmsg->replies, sizeof(pXmsg->replies));

   if (pXmsg->attr & MSGUID)
      pHeader->ulMsgID = pXmsg->umsgid;
   else
      pHeader->ulMsgID = 0;

   return;
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

void Msgheader2Xmsg(PXMSG pXmsg, PMSGHEADER pHeader)
{
   pXmsg->attr = pHeader->ulAttrib & 0x0001fbffUL;
   if (pHeader->ulAttrib & ATTRIB_KEEP)
      pXmsg->attr |= FLEET_KEEP;

   memcpy(pXmsg->from, pHeader->pchFromName, LEN_USERNAME+1);
   memcpy(pXmsg->to,   pHeader->pchToName,   LEN_USERNAME+1);
   memcpy(pXmsg->subj, pHeader->pchSubject,  LEN_SUBJECT+1);

   memcpy(&pXmsg->orig, &pHeader->FromAddress, sizeof(FTNADDRESS));
   memcpy(&pXmsg->dest, &pHeader->ToAddress,   sizeof(FTNADDRESS));

   memcpy(&pXmsg->date_written, &pHeader->StampWritten, sizeof(TIMESTAMP));
   memcpy(&pXmsg->date_arrived, &pHeader->StampArrived, sizeof(TIMESTAMP));

   pXmsg->replyto = pHeader->ulReplyTo;
   memcpy(pXmsg->replies, pHeader->ulReplies, sizeof(pXmsg->replies));

   pXmsg->utc_ofs =0;

   sprintf(pXmsg->__ftsc_date, "%02d %s %02d  %02d:%02d:%02d",
           pXmsg->date_written.date.da,
           (pXmsg->date_written.date.mo >= 1 && pXmsg->date_written.date.mo <=12)?
              months[pXmsg->date_written.date.mo-1]:"???",
           (pXmsg->date_written.date.yr>=20)?(pXmsg->date_written.date.yr-20):(pXmsg->date_written.date.yr+80),
           pXmsg->date_written.time.hh,
           pXmsg->date_written.time.mm,
           pXmsg->date_written.time.ss*2);

   return;
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

void ExtractKludges(PCHAR pchMessageText, PCHAR *pchCtlText, PULONG ctllen)
{
   PCHAR src, dst;
   PCHAR pchKludgeBuffer, pchKludgeHelp, pchKludgeBak;

   src=dst=pchMessageText;

   while(*src)
      switch(*src)
      {
         /* Kludge-Line */
         case '\x01':
            if (src == pchMessageText ||
                (src > pchMessageText &&
                 (*(src-1) == '\n' || *(src-1) == '\r')))
            {
               pchKludgeBuffer=malloc(MAX_KLUDGEBUFFER+1);
               /* Kludge kopieren */
               pchKludgeHelp=pchKludgeBuffer;
               while(*src && (*src != '\n' && *src != '\r') &&
                     (pchKludgeHelp-pchKludgeBuffer) < MAX_KLUDGEBUFFER)
                  *pchKludgeHelp++=*src++;

               if ((pchKludgeHelp-pchKludgeBuffer) >= MAX_KLUDGEBUFFER)
                  /* zu lang, Rest ueberlesen */
                  while(*src && *src != '\n' && *src != '\r')
                     src++;

               if (*src)
                  src++;
               *pchKludgeHelp='\0';
               /* Kludge an andere Kludges anhaengen */
               pchKludgeBak = *pchCtlText;
               *pchCtlText=realloc(*pchCtlText, *ctllen+strlen(pchKludgeBuffer)+5);
               if (!*pchCtlText)
               {
                  *pchCtlText = malloc(*ctllen+strlen(pchKludgeBuffer)+5);
                  memcpy(*pchCtlText, pchKludgeBak, *ctllen);
                  free(pchKludgeBak);
               }
               strcat(*pchCtlText, pchKludgeBuffer);
               *ctllen=strlen(*pchCtlText)+1;
               free(pchKludgeBuffer);
            }
            else
            {
               *dst++='@';
               src++;
            }
            break;

         default:
            *dst=*src;
            src++;
            dst++;
            break;
      }
   *dst='\0';

   return;
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

void QueryOrigin(PCHAR pchMessageText, ULONG textlen, PCHAR ctltext, PXMSG pXmsg)
{
   PCHAR Orig_Offset;
   FTNADDRESS fromaddress={0, 0, 0, 0};

   if (ctltext)
   {
      Orig_Offset = strstr(ctltext, MSGID_KLUDGE);
      if (Orig_Offset)
      {
         Orig_Offset += strlen(MSGID_KLUDGE)+1;  /* Kludge selbst ｜erspringen */
         StringToNetAddr(Orig_Offset, &fromaddress, NULL);
      }
   }
   if (fromaddress.usZone == 0 && pchMessageText)
   {
      Orig_Offset=pchMessageText + textlen;
      while (Orig_Offset > pchMessageText && *Orig_Offset!='(')
         Orig_Offset--;

      if (*Orig_Offset=='(')
      {
         while ((Orig_Offset[0] <= '0' || Orig_Offset[0] > '9') && Orig_Offset[0]!='\0')
            Orig_Offset++;
         if (Orig_Offset[0])
            StringToNetAddr(Orig_Offset, &fromaddress, NULL);
      }
   }
   if (fromaddress.usZone != 0)
   {
      pXmsg->orig.zone  = fromaddress.usZone;
      pXmsg->orig.net   = fromaddress.usNet;
      pXmsg->orig.node  = fromaddress.usNode;
      pXmsg->orig.point = fromaddress.usPoint;
   }
   return;
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

USHORT WriteMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   HMSG msghandle;
   XMSG header;
   int itextlen = strlen(pMessage->pchMessageText)+1;
   int ictllen;
   char *ctltext=NULL;

   Msgheader2Xmsg(&header, pHeader);
   /* Read-Flag bei FTS-Areas setzen */
   if (pAreaDef->areadata.areaformat == AREAFORMAT_SQUISH &&
       (pHeader->ulAttrib & ATTRIB_READ) )
      header.attr |= FLEET_READ;

   AttribToFlags(pMessage, pHeader, AttribMap);
   ictllen = MSG_CalcKludgeBufferSize(pMessage);

   if (ictllen)
   {
      ctltext = malloc(ictllen);
      MSG_KludgesToBuffer(pMessage, ctltext);
   }

   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_CREATE, msgnum)))
   {
      free(ctltext);
      return MSG_OPEN_ERROR;
   }
   if (MsgWriteMsg(msghandle, 0, &header, pMessage->pchMessageText, itextlen, itextlen, ictllen, ctltext))
   {
      MsgCloseMsg(msghandle);
      free(ctltext);
      return MSG_WRITE_ERROR;
   }

   /* Read-Flag bei FTS-Areas setzen */
   if (pAreaDef->areadata.areaformat == AREAFORMAT_FTS)
      WriteReadFlag(msghandle, pHeader->ulAttrib);

   MsgCloseMsg(msghandle);
   pAreaDef->maxmessages = MsgGetNumMsg((HAREA)pAreaDef->areahandle);

   if (msgnum)
   {
      /* Aendern, High-Water zur…ksetzen */
      int oldhigh;

      oldhigh = MsgGetHighWater((HAREA)pAreaDef->areahandle);
      if (oldhigh && oldhigh >= msgnum)
         MsgSetHighWater((HAREA)pAreaDef->areahandle, msgnum-1);
   }

   free(ctltext);

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

USHORT ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   MSGH    *msghandle;
   ULONG   textlen, ctllen;
   PCHAR   ctltext;
   XMSG    header;
   BOOL    bRead=FALSE;

   /* Abfangen, ob ueberhaupt Messages da sind ! Sonst kann das mit
      currentmessage (min=1) nicht klappen.
   ﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄ */
   if (pAreaDef->maxmessages == 0)
      return NO_MESSAGE;

   /* Message ｜er die Squish-API lesen.
   ﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄ */
   if (!(msghandle=MsgOpenMsg((HAREA)pAreaDef->areahandle, MOPEN_READ, msgnum)))
   {
      return MSG_READ_ERROR;
   }

   textlen = MsgGetTextLen(msghandle);
   ctllen  = MsgGetCtrlLen(msghandle);

   if (ctllen == 0xffffffffUL)   /* Fehler */
      ctllen=0;

   pMessage->pchMessageText =  malloc(textlen+1);
   ctltext =  malloc(ctllen +1);

   pMessage->pchMessageText[textlen]='\0';
   pMessage->pchMessageText[0]='\0';
   ctltext[ctllen]='\0';
   ctltext[0]='\0';

   /* Message mit allen Control-Informationen und dem Text von Anfang
      bis textlen einlesen. Der Text und die Controlvariablen landen
      in den angeforderten Puffern.
   ﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄ */
   MsgReadMsg(msghandle, &header, 0, textlen, pMessage->pchMessageText,
                                     ctllen,  ctltext);


   if (pAreaDef->areadata.areaformat == AREAFORMAT_FTS)
      bRead = QueryReadFlag(msghandle);
   else
      if (pAreaDef->areadata.areaformat == AREAFORMAT_SQUISH)
         bRead = header.attr & FLEET_READ;

   MsgCloseMsg(msghandle);         /* Am besten sofort wieder schliessen */

   ExtractKludges(pMessage->pchMessageText, &ctltext, &ctllen);

   /* Origin-Line und die Msgid nach gueltigen Adressen durchsuchen.
      Das Ganze nur in Echo-Areas.
   ﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄﾄ */
   if (pAreaDef->areadata.areatype != AREATYPE_NET)
      QueryOrigin(pMessage->pchMessageText, strlen(pMessage->pchMessageText), ctltext, &header);

   MSG_BufferToKludges(pMessage, ctltext);
   Xmsg2Msgheader(&header, pHeader);

   /* 5.1.98: Wenn Default-Zone 0 gelesen wurde, dann Zone der Area eintragen */
   if (pHeader->FromAddress.usZone == 0 ||
       pHeader->ToAddress.usZone == 0)
   {
      FTNADDRESS TempAddr;
      StringToNetAddr(pAreaDef->areadata.address, &TempAddr, NULL);

      if (pHeader->FromAddress.usZone == 0)
         pHeader->FromAddress.usZone = TempAddr.usZone;
      if (pHeader->ToAddress.usZone == 0)
         pHeader->ToAddress.usZone = TempAddr.usZone;
   }


   if (bRead)
      pHeader->ulAttrib |= ATTRIB_READ;
   FlagsToAttrib(pMessage, pHeader, AttribMap);
   free(ctltext);

   return OK;
}

/*-------------------------------- Modulende --------------------------------*/


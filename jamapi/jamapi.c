/*---------------------------------------------------------------------------+
 | Titel: JAMAPI.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 26.03.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    API f. FleetStreet u. JAM                                              |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*
    To-Do: - Subfields f. Attaches und Requests besonders behandeln !
*/

#pragma strings(readonly)

#define RAMIDX

/*----------------------------- Header-Dateien ------------------------------*/

#define INCL_BASE
#include <os2.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <time.h>
#include "..\main.h"
#include "..\structs.h"
#include "..\msgheader.h"
#include "..\areaman\areaman.h"
#include "..\handlemsg\handlemsg.h"
#include "..\handlemsg\kludgeapi.h"
#include "..\util\crc32.h"
#include "..\util\addrcnv.h"
#include "jammb.h"
#include "jamindex.h"

#include "jamapi.h"

/*--------------------------------- Defines ---------------------------------*/

#define ATTRIB_MASK (ATTRIB_PRIVATE | ATTRIB_CRASH | ATTRIB_RCVD | ATTRIB_SENT |\
                     ATTRIB_FILEATTACHED | ATTRIB_INTRANSIT | ATTRIB_ORPHAN | ATTRIB_KILLSENT |\
                     ATTRIB_LOCAL | ATTRIB_HOLD | ATTRIB_READ | ATTRIB_FREQUEST |\
                     ATTRIB_RRQ | ATTRIB_RECEIPT |\
                     ATTRIB_ARCHIVESENT | ATTRIB_DIRECT | ATTRIB_TRUNCFILE | ATTRIB_KILLFILE |\
                     ATTRIB_IMMEDIATE | ATTRIB_GATE | ATTRIB_FORCEPICKUP | ATTRIB_DELETED | ATTRIB_KEEP |\
                     ATTRIB_NPD)

#define LEN_KLUDGE 100

/*---------------------------------- Typen ----------------------------------*/

typedef struct stringlist
{
   struct stringlist *next;
   char *pchString;
   long length;
} STRINGLIST, *PSTRINGLIST;

/*---------------------------- Globale Variablen ----------------------------*/

static ATTRIBMAP AttribMap[]=
{
   {"NPD", ATTRIB_NPD},
   {"", 0},
};

/*----------------------- interne Funktionsprototypen -----------------------*/

static PTIMESTAMP ConvertFromJAMTime(PTIMESTAMP pDestStamp, ULONG ulJAMTime);
static ULONG ConvertToJAMTime(PTIMESTAMP pStamp);
static ULONG ConvertFromJAMAttrib(ULONG ulJAMAttrib);
static ULONG ConvertToJAMAttrib(ULONG ulAttrib);
static ULONG CalcSubfieldsSize(PMSGHEADER pHeader, PFTNMESSAGE pMessage);
static ULONG CalcNewNumber(JAMAPIRECptr apirec);
static PKLUDGE ReadSubfields(PAREADEFLIST pAreaDef, ULONG msgnum, ULONG SubfieldLen, PMSGHEADER pHeader, PFTNMESSAGE pMessage);
static void TraceReplyChain(PAREADEFLIST pAreaDef, JAMHDRptr RawHeader, PMSGHEADER pHeader);
static char *MergeText(PFTNMESSAGE pMessage);
static void AddAttachSubfields(PAREADEFLIST pAreaDef, PMSGHEADER pHeader);
static USHORT AddMessageContents(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, PULONG pulHdrOffs);

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

ULONG JAM_QueryAttribMask(void)
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

USHORT JAM_OpenApi(void)
{
   /* UTC-Offset 0 erzwingen */
   _putenv("TZ=CET");
   _tzset();

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

USHORT JAM_CloseApi(void)
{
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

PVOID JAM_OpenArea(PAREADEFLIST pAreaDef, PCHAR pchPathName)
{
   JAMAPIRECptr pArea;

   if (pArea = malloc(sizeof(JAMAPIREC)))
   {
      JAMsysInitApiRec(pArea, pchPathName);

      if (JAMmbOpen(pArea))
      {
         JAMmbUpdateHeaderInfo(pArea, FALSE);
         pAreaDef->msgnumlist = JAMLoadIndex(pArea, &pAreaDef->maxmessages);
         return pArea;
      }
      else
      {
         /* Neu erstellen probieren */
         if (JAMmbCreate(pArea))
         {
            JAMmbUpdateHeaderInfo(pArea, FALSE);
            pAreaDef->msgnumlist = JAMLoadIndex(pArea, &pAreaDef->maxmessages);
            return pArea;
         }
         else
         {
            free(pArea);
            return NULL;
         }
      }
   }
   else
      return NULL;
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

USHORT JAM_CloseArea(PAREADEFLIST pAreaDef)
{
   if (JAMmbClose((JAMAPIRECptr)pAreaDef->areahandle))
   {
      JAMFreeIndex(pAreaDef);
      JAMsysDeinitApiRec((JAMAPIRECptr)pAreaDef->areahandle);
      free(pAreaDef->areahandle);
      pAreaDef->areahandle=NULL;
      return 0;
   }
   else
      return 1;
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

ULONG JAM_ReadLastread(PAREADEFLIST pAreaDef, LONG lOffset)
{
   JAMLREAD LastRead;
   UCHAR chLowerName[LEN_USERNAME+1];
   ULONG ulNameCRC;

   memcpy(chLowerName, pAreaDef->areadata.username, LEN_USERNAME+1);
   strlwr(chLowerName);

   ulNameCRC = Crc32(chLowerName, strlen(chLowerName), 0xffffffff);

   if (!JAMmbFetchLastRead((JAMAPIRECptr)pAreaDef->areahandle, ulNameCRC, &LastRead))
      LastRead.LastReadMsg=0;

   if (pAreaDef->oldlastread==0)
      pAreaDef->oldlastread=LastRead.LastReadMsg;

   return JAM_UidToMsgn(pAreaDef, LastRead.LastReadMsg, TRUE);
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

USHORT JAM_WriteLastread(PAREADEFLIST pAreaDef, LONG lOffset)
{
   ULONG ulMsgID;
   UCHAR chLowerName[LEN_USERNAME+1];
   ULONG ulNameCRC;

   memcpy(chLowerName, pAreaDef->areadata.username, LEN_USERNAME+1);
   strlwr(chLowerName);

   ulNameCRC = Crc32(chLowerName, strlen(chLowerName), 0xffffffff);

   /* Messagebase locken */
   while (!JAMmbLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE))
      DosSleep(500); /* halbe Sekunde warten, dann nochmal */

   ulMsgID = JAM_MsgnToUid(pAreaDef, pAreaDef->currentmessage);

   if (!JAMmbStoreLastRead((JAMAPIRECptr)pAreaDef->areahandle, ulNameCRC, ulMsgID))
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);
      return 1;
   }

   JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);

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

USHORT JAM_ReadHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum)
{
   JAMHDR RawHeader;

   msgnum = JAM_MsgnToUid(pAreaDef, msgnum);

   if (!msgnum)
      return 1;

   if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &RawHeader, msgnum))
   {
      /* Header konvertieren */
      pHeader->ulMsgID = RawHeader.MsgNum;
      pHeader->ulReplyTo = RawHeader.ReplyTo;
      pHeader->ulReplies[0] = RawHeader.Reply1st;
      ConvertFromJAMTime(&pHeader->StampWritten, RawHeader.DateWritten);
      if (RawHeader.DateProcessed)
         ConvertFromJAMTime(&pHeader->StampArrived, RawHeader.DateProcessed);
      else
         pHeader->StampArrived = pHeader->StampWritten;
      pHeader->ulAttrib = ConvertFromJAMAttrib(RawHeader.Attribute);
      if (RawHeader.TimesRead)
         pHeader->ulAttrib |= ATTRIB_READ;

      ReadSubfields(pAreaDef, msgnum, RawHeader.SubfieldLen, pHeader, NULL);

      TraceReplyChain(pAreaDef, &RawHeader, pHeader);

      return 0;
   }
   else
      return 1;
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

USHORT JAM_LinkMessages(PAREADEFLIST pAreaDef, ULONG ulReplyID, ULONG ulOrigID)
{
   JAMHDR ReplyHeader;
   JAMHDR OrigHeader;
   ULONG ulTempID;

   /* Messagebase locken */
   while (!JAMmbLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE))
      DosSleep(500); /* halbe Sekunde warten, dann nochmal */

   /* Reply-Header lesen */
   if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &ReplyHeader, ulReplyID))
   {
      /* Original-Header lesen */
      if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulOrigID))
      {
         if (!OrigHeader.Reply1st) /* noch kein Reply */
         {
            OrigHeader.Reply1st = ulReplyID;
            JAMmbStoreMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulOrigID);
         }
         else
         {
            /* mehrere Replies, suchen */
            if (OrigHeader.Reply1st != ulReplyID)
            {
               ulTempID = OrigHeader.Reply1st;

               do
               {
                  if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulTempID))
                  {
                     if (OrigHeader.ReplyNext != ulReplyID)
                     {
                        if (!OrigHeader.ReplyNext)
                        {
                           OrigHeader.ReplyNext = ulReplyID;
                           JAMmbStoreMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulTempID);
                           break;
                        }
                        else
                           ulTempID = OrigHeader.ReplyNext;
                     }
                     else
                        break; /* schon eingetragen */
                  }
                  else
                     break;
               } while (ulTempID);
            }
         }
      }
      else
      {
         JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
         return 1;
      }
   }
   else
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
      return 1;
   }

   /* Aufwaertsreferenz eintragen */
   if (ReplyHeader.ReplyTo != ulOrigID)
   {
      ReplyHeader.ReplyTo = ulOrigID;
      JAMmbStoreMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &ReplyHeader, ulReplyID);
   }

   JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);

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

USHORT JAM_UnlinkMessage(PAREADEFLIST pAreaDef, int msgnum)
{
   JAMHDR ReplyHeader;
   JAMHDR OrigHeader;
   ULONG ulTempID;
   ULONG ulOrigID;
   ULONG ulReplyID;

   ulReplyID = JAM_MsgnToUid(pAreaDef, msgnum);

   if (!ulReplyID)
      return 1;

   /* Messagebase locken */
   while (!JAMmbLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE))
      DosSleep(500); /* halbe Sekunde warten, dann nochmal */

   /* Reply-Header lesen */
   if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &ReplyHeader, ulReplyID))
   {
      if (ReplyHeader.ReplyTo)
      {
         ulOrigID = ReplyHeader.ReplyTo;

         /* Original-Header lesen */
         if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulOrigID))
         {
            if (OrigHeader.Reply1st == ulReplyID) /* ist erster Reply */
            {
               OrigHeader.Reply1st = ReplyHeader.ReplyNext;
               JAMmbStoreMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulOrigID);
            }
            else
            {
               /* mehrere Replies, suchen */
               if (OrigHeader.Reply1st)
               {
                  ulTempID = OrigHeader.Reply1st;

                  do
                  {
                     if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulTempID))
                     {
                        if (OrigHeader.ReplyNext == ulReplyID)
                        {
                           OrigHeader.ReplyNext = ReplyHeader.ReplyNext;
                           JAMmbStoreMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulTempID);
                           break;
                        }
                        else
                           ulTempID = OrigHeader.ReplyNext;
                     }
                     else
                        break;
                  } while (ulTempID);
               }
            }
         }
         else
         {
            JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
            return 1;
         }
      }
   }
   else
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
      return 1;
   }

   /* Abwaerts-Referenzen loeschen */
   ulTempID = ReplyHeader.Reply1st;
   while (ulTempID)
   {
      if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulTempID))
      {
         OrigHeader.ReplyTo = 0;
         JAMmbStoreMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &OrigHeader, ulTempID);

         ulTempID = OrigHeader.ReplyNext;
      }
      else
         break;
   }

   /* Links loeschen */
   ReplyHeader.ReplyTo = 0;
   ReplyHeader.ReplyNext = 0;
   ReplyHeader.Reply1st = 0;
   JAMmbStoreMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &ReplyHeader, ulReplyID);

   JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);

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

USHORT JAM_MarkRead(PAREADEFLIST pAreaDef, int msgnum, BOOL bPersonal)
{
   msgnum = JAM_MsgnToUid(pAreaDef, msgnum);

   if (!msgnum)
      return 1;

   /* Messagebase locken */
   while (!JAMmbLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE))
      DosSleep(500); /* halbe Sekunde warten, dann nochmal */

   if (JAMmbSetMsgRead((JAMAPIRECptr)pAreaDef->areahandle, msgnum, bPersonal))
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);
      return 0;
   }
   else
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
      return 1;
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

USHORT JAM_ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   JAMHDR RawHeader;
   PCHAR Orig_Offset;
   PKLUDGE pKludge=NULL;

   msgnum = JAM_MsgnToUid(pAreaDef, msgnum);

   if (!msgnum)
      return 1;

   if (JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &RawHeader, msgnum))
   {
      /* Header konvertieren */
      pHeader->ulMsgID = RawHeader.MsgNum;
      pHeader->ulReplyTo = RawHeader.ReplyTo;
      pHeader->ulReplies[0] = RawHeader.Reply1st;
      ConvertFromJAMTime(&pHeader->StampWritten, RawHeader.DateWritten);
      if (RawHeader.DateProcessed)
         ConvertFromJAMTime(&pHeader->StampArrived, RawHeader.DateProcessed);
      else
         pHeader->StampArrived = pHeader->StampWritten;
      pHeader->ulAttrib = ConvertFromJAMAttrib(RawHeader.Attribute);
      if (RawHeader.TimesRead)
         pHeader->ulAttrib |= ATTRIB_READ;

      pKludge = ReadSubfields(pAreaDef, msgnum, RawHeader.SubfieldLen, pHeader, pMessage);

      if (!pHeader->FromAddress.usZone && pKludge)
         StringToNetAddr(pKludge->pchKludgeText, &pHeader->FromAddress, NULL);

      /* Message-Text lesen */
      pMessage->pchMessageText = calloc(1, RawHeader.TxtLen+1);

      if (RawHeader.TxtLen && pMessage->pchMessageText)
      {
         if (!JAMmbFetchMsgTxt((JAMAPIRECptr)pAreaDef->areahandle,
                               RawHeader.TxtOffset, RawHeader.TxtLen, pMessage->pchMessageText))
         {
            free(pMessage->pchMessageText);
            pMessage->pchMessageText=NULL;
            return 1;
         }
         pMessage->pchMessageText[RawHeader.TxtLen]=0;
      }

      /* Adresse v. Echomail feststellen */
      if (!pHeader->FromAddress.usZone && pMessage->pchMessageText)
      {
         Orig_Offset=strstr(pMessage->pchMessageText, " * Origin:");
         if (Orig_Offset)
         {
            while (*Orig_Offset && *Orig_Offset != '\r')
               Orig_Offset++;

            while (Orig_Offset > pMessage->pchMessageText && *Orig_Offset!='(')
               Orig_Offset--;

            if (*Orig_Offset=='(')
            {
               while ((Orig_Offset[0] <= '0' || Orig_Offset[0] > '9') && Orig_Offset[0]!='\0')
                  Orig_Offset++;
               if (Orig_Offset[0])
                  StringToNetAddr(Orig_Offset, &pHeader->FromAddress, NULL);
            }
         }
      }

      MergeText(pMessage);

      TraceReplyChain(pAreaDef, &RawHeader, pHeader);

      return 0;
   }
   else
      return 1;
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

USHORT JAM_ChangeMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum)
{
   JAMIDXREC Idx;
   char pchKludgeBuffer[LEN_KLUDGE+1];

   msgnum = JAM_MsgnToUid(pAreaDef, msgnum);

   if (!msgnum)
      return 1;

   /* Messagebase locken */
   while (!JAMmbLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE))
      DosSleep(500); /* halbe Sekunde warten, dann nochmal */

   if (AddMessageContents(pAreaDef, pHeader, pMessage, &Idx.HdrOffset))
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
      return 1;
   }

   /* alten Header als gel敗cht markieren */
   if (!JAMmbSetMsgDeleted((JAMAPIRECptr)pAreaDef->areahandle, msgnum))
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
      return 1;
   }

   /* alten Index updaten */
   strlwr(strcpy(pchKludgeBuffer, pHeader->pchToName));
   Idx.UserCRC = Crc32(pchKludgeBuffer, strlen(pchKludgeBuffer), 0xffffffffUL);
   JAMmbStoreMsgIdx((JAMAPIRECptr)pAreaDef->areahandle, &Idx, msgnum);

   /* Base freigeben */
   JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);

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

USHORT JAM_AddMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage)
{
   ULONG ulMsgNum=0;
   JAMIDXREC Idx;
   char pchKludgeBuffer[LEN_KLUDGE+1];

   /* Messagebase locken */
   while (!JAMmbLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE))
      DosSleep(500); /* halbe Sekunde warten, dann nochmal */

   /* Neue Messagenummer berechnen */
   ulMsgNum = CalcNewNumber((JAMAPIRECptr)pAreaDef->areahandle);

   pHeader->ulMsgID = ulMsgNum;

   if (AddMessageContents(pAreaDef, pHeader, pMessage, &Idx.HdrOffset))
   {
      JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
      return 1;
   }

   /* Index-Record anlegen */
   strlwr(strcpy(pchKludgeBuffer, pHeader->pchToName));
   Idx.UserCRC = Crc32(pchKludgeBuffer, strlen(pchKludgeBuffer), 0xffffffffUL);
   JAMmbAddMsgIdx((JAMAPIRECptr)pAreaDef->areahandle, &Idx);

   /* Base-Header updaten */
   ((JAMAPIRECptr)pAreaDef->areahandle)->HdrInfo.ActiveMsgs++;
   pAreaDef->maxmessages++;

   JAMAddToIdx(pAreaDef, ulMsgNum);

   /* Base freigeben */
   JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);

   return 0;
}

static void AddAttachSubfields(PAREADEFLIST pAreaDef, PMSGHEADER pHeader)
{
  if (pHeader->ulAttrib & (ATTRIB_FILEATTACHED | ATTRIB_FREQUEST))
  {
      char *pchTemp = pHeader->pchSubject;
      char *pchPrev = pchTemp;
      USHORT usSubID;

      while (*pchTemp)
      {
         /* Leerzeichen uebergehen */
         while (*pchTemp == ' ')
            pchTemp++;

         if (*pchTemp) /* noch ein File */
         {
            /* subfield voreinstellen */
            if (pHeader->ulAttrib & ATTRIB_FILEATTACHED)
               usSubID = JAMSFLD_ENCLFILE;
            else
               usSubID = JAMSFLD_ENCLFREQ;

            /* Name ueberspringen */
            pchPrev = pchTemp;
            while (*pchTemp && *pchTemp != ' ')
            {
               /* Attach mit Wildcard pr’en */
               if ((*pchTemp == '*' ||
                    *pchTemp == '?') &&
                   (pHeader->ulAttrib & ATTRIB_FILEATTACHED))
                  usSubID = JAMSFLD_ENCLFILEWC;

               pchTemp++;
            }
            JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, usSubID, pchTemp-pchPrev, pchPrev);
         }
      }
  }
  return;
}

static USHORT AddMessageContents(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, PULONG pulHdrOffs)
{
   ULONG ulSubfieldLen;
   JAMHDR NewHeader;
   PKLUDGE pKludge;
   char pchAddress[LEN_5DADDRESS+1];
   char pchKludgeBuffer[LEN_KLUDGE+1];

   /* Platz f. Subfields ermitteln */
   AttribToFlags(pMessage, pHeader, AttribMap);
   ulSubfieldLen = CalcSubfieldsSize(pHeader, pMessage);

   /* neuen Header vorbereiten */
   memset(&NewHeader, 0, sizeof(NewHeader));
   strcpy(NewHeader.Signature, HEADERSIGNATURE);
   NewHeader.Revision = CURRENTREVLEV;
   NewHeader.SubfieldLen = ulSubfieldLen;
   NewHeader.TimesRead = 1;
   if (pKludge = MSG_FindKludge(pMessage, KLUDGE_MSGID, NULL))
      NewHeader.MsgIdCRC = Crc32(pKludge->pchKludgeText, strlen(pKludge->pchKludgeText), 0xffffffffUL);
   if (pKludge = MSG_FindKludge(pMessage, KLUDGE_REPLY, NULL))
      NewHeader.ReplyCRC = Crc32(pKludge->pchKludgeText, strlen(pKludge->pchKludgeText), 0xffffffffUL);
   NewHeader.ReplyTo = pHeader->ulReplyTo;
   NewHeader.DateWritten = ConvertToJAMTime(&pHeader->StampWritten);
   NewHeader.DateProcessed = 0; /*ConvertToJAMTime(&pHeader->StampArrived);*/
   NewHeader.DateReceived = NewHeader.DateWritten;
   NewHeader.MsgNum = pHeader->ulMsgID;
   NewHeader.Attribute = ConvertToJAMAttrib(pHeader->ulAttrib);
   switch(pAreaDef->areadata.areatype)
   {
      case AREATYPE_ECHO:
         NewHeader.Attribute |= MSG_TYPEECHO;
         break;

      case AREATYPE_NET:
         NewHeader.Attribute |= MSG_TYPENET;
         break;

      case AREATYPE_LOCAL:
         NewHeader.Attribute |= MSG_TYPELOCAL;
         break;
   }

   if (pMessage->pchMessageText)
      NewHeader.TxtLen = strlen(pMessage->pchMessageText);

   /* neuen Text schreiben */
   if (NewHeader.TxtLen)
      if (!JAMmbAddMsgTxt((JAMAPIRECptr)pAreaDef->areahandle, pMessage->pchMessageText, NewHeader.TxtLen,
                          &NewHeader.TxtOffset))
      {
         return 1;
      }

   /* Neuen Header schreiben */
   if (!JAMmbAddMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, &NewHeader, pulHdrOffs))
   {
      return 1;
   }

   /* Subfields schreiben */
   NetAddrToString(pchAddress, &pHeader->FromAddress);
   JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, JAMSFLD_OADDRESS, strlen(pchAddress), pchAddress);
   NetAddrToString(pchAddress, &pHeader->ToAddress);
   JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, JAMSFLD_DADDRESS, strlen(pchAddress), pchAddress);
   JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, JAMSFLD_SENDERNAME, strlen(pHeader->pchFromName), pHeader->pchFromName);
   JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, JAMSFLD_RECVRNAME, strlen(pHeader->pchToName), pHeader->pchToName);
   JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, JAMSFLD_SUBJECT, strlen(pHeader->pchSubject), pHeader->pchSubject);

   pKludge=NULL;
   while (pKludge = MSG_FindKludge(pMessage, KLUDGE_ANY, pKludge))
   {
      USHORT usSubID;

      switch(pKludge->ulKludgeType)
      {
         case KLUDGE_TOPT:  /* werden nicht geschrieben */
         case KLUDGE_FMPT:
         case KLUDGE_INTL:
            continue;

         case KLUDGE_MSGID:
            usSubID = JAMSFLD_MSGID;
            break;

         case KLUDGE_REPLY:
            usSubID = JAMSFLD_REPLYID;
            break;

         case KLUDGE_FLAGS:
            usSubID = JAMSFLD_FLAGS;
            break;

         case KLUDGE_PID:
            usSubID = JAMSFLD_PID;
            break;

         case KLUDGE_OTHER:
            usSubID = JAMSFLD_FTSKLUDGE;
            break;

         default: /* Alle anderen Kludges ausser KLUDGE_OTHER */
            strcpy(pchKludgeBuffer, MSG_QueryKludgeName(pKludge->ulKludgeType));
            strcat(pchKludgeBuffer, " ");
            strcat(pchKludgeBuffer, pKludge->pchKludgeText);
            JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, JAMSFLD_FTSKLUDGE, strlen(pchKludgeBuffer), pchKludgeBuffer);
            continue;
      }
      JAMmbAddSubfield((JAMAPIRECptr)pAreaDef->areahandle, usSubID, strlen(pKludge->pchKludgeText), pKludge->pchKludgeText);
   }

   AddAttachSubfields(pAreaDef, pHeader);

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

USHORT JAM_KillMessage(PAREADEFLIST pAreaDef, int msgnum)
{
   int oldnum = msgnum;

   msgnum = JAM_MsgnToUid(pAreaDef, msgnum);

   if (!msgnum)
      return 1;

   while (!JAMmbLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE))
      DosSleep(500); /* halbe Sekunde warten, dann nochmal */

   switch (JAMmbSetMsgDeleted((JAMAPIRECptr)pAreaDef->areahandle, msgnum))
   {
      case 0:
         JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, FALSE);
         return 1;

      case 1:
         ((JAMAPIRECptr)pAreaDef->areahandle)->HdrInfo.ActiveMsgs--;

      case 2:
         pAreaDef->maxmessages--;
         RemoveFromIdx(pAreaDef, oldnum);

         if (pAreaDef->currentmessage > pAreaDef->maxmessages)
            pAreaDef->currentmessage = pAreaDef->maxmessages;
         JAMmbUnLockMsgBase((JAMAPIRECptr)pAreaDef->areahandle, TRUE);
         return 0;

      default:
         return 1;
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

ULONG JAM_UidToMsgn(PAREADEFLIST pAreaDef, ULONG msgID, BOOL exact)
{
   if (msgID == 0)
      return 0;
   else
      return JAMFindUid(pAreaDef, msgID, exact);
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

ULONG JAM_MsgnToUid(PAREADEFLIST pAreaDef, int msgnum)
{
   if (msgnum == 0)
      return 0;
   else
   {
      if (msgnum > pAreaDef->maxmessages)
         return 0;
      else
      {   /* Index lesen */
         return JAMFindNum(pAreaDef, msgnum);
      }
   }
}

static PTIMESTAMP ConvertFromJAMTime(PTIMESTAMP pDestStamp, ULONG ulJAMTime)
{
   struct tm* tmstruct;

   tmstruct = localtime((PLONG)&ulJAMTime);

   if (tmstruct)
   {
      pDestStamp->day     = tmstruct->tm_mday;
      pDestStamp->month   = tmstruct->tm_mon  + 1;
      pDestStamp->year    = tmstruct->tm_year - 80;
      pDestStamp->hours   = tmstruct->tm_hour;
      pDestStamp->minutes = tmstruct->tm_min;
      if (tmstruct->tm_sec > 59)
         pDestStamp->seconds = 59/2;
      else
         pDestStamp->seconds = tmstruct->tm_sec/2;
   }
   else
      memset(pDestStamp, 0, sizeof(TIMESTAMP));

   return pDestStamp;
}

static ULONG ConvertToJAMTime(PTIMESTAMP pStamp)
{
   struct tm tmstruct;

   tmstruct.tm_mday = pStamp->day;
   tmstruct.tm_mon  = pStamp->month-1;
   tmstruct.tm_year = pStamp->year+80;
   tmstruct.tm_hour = pStamp->hours;
   tmstruct.tm_min  = pStamp->minutes;
   tmstruct.tm_sec  = pStamp->seconds*2;
   tmstruct.tm_isdst = -1;

   return mktime(&tmstruct);
}

static ULONG ConvertFromJAMAttrib(ULONG ulJAMAttrib)
{
   ULONG ulTemp=0;

   if (ulJAMAttrib & MSG_LOCAL)
      ulTemp |= ATTRIB_LOCAL;
   if (ulJAMAttrib & MSG_INTRANSIT)
      ulTemp |= ATTRIB_INTRANSIT;
   if (ulJAMAttrib & MSG_PRIVATE)
      ulTemp |= ATTRIB_PRIVATE;
   if (ulJAMAttrib & MSG_READ)
      ulTemp |= ATTRIB_RCVD;
   if (ulJAMAttrib & MSG_SENT)
      ulTemp |= ATTRIB_SENT;
   if (ulJAMAttrib & MSG_KILLSENT)
      ulTemp |= ATTRIB_KILLSENT;
   if (ulJAMAttrib & MSG_ARCHIVESENT)
      ulTemp |= ATTRIB_ARCHIVESENT;
   if (ulJAMAttrib & MSG_HOLD)
      ulTemp |= ATTRIB_HOLD;
   if (ulJAMAttrib & MSG_CRASH)
      ulTemp |= ATTRIB_CRASH;
   if (ulJAMAttrib & MSG_IMMEDIATE)
      ulTemp |= ATTRIB_IMMEDIATE;
   if (ulJAMAttrib & MSG_DIRECT)
      ulTemp |= ATTRIB_DIRECT;
   if (ulJAMAttrib & MSG_GATE)
      ulTemp |= ATTRIB_GATE;
   if (ulJAMAttrib & MSG_FILEREQUEST)
      ulTemp |= ATTRIB_FREQUEST;
   if (ulJAMAttrib & MSG_FILEATTACH)
      ulTemp |= ATTRIB_FILEATTACHED;
   if (ulJAMAttrib & MSG_TRUNCFILE)
      ulTemp |= ATTRIB_TRUNCFILE;
   if (ulJAMAttrib & MSG_KILLFILE)
      ulTemp |= ATTRIB_KILLFILE;
   if (ulJAMAttrib & MSG_RECEIPTREQ)
      ulTemp |= ATTRIB_RRQ;
   if (ulJAMAttrib & MSG_CONFIRMREQ)
      ulTemp |= ATTRIB_RECEIPT;
   if (ulJAMAttrib & MSG_ORPHAN)
      ulTemp |= ATTRIB_ORPHAN;
   if (ulJAMAttrib & MSG_FPU)
      ulTemp |= ATTRIB_FORCEPICKUP;
   if (ulJAMAttrib & MSG_DELETED)
      ulTemp |= ATTRIB_DELETED;
   if (ulJAMAttrib & MSG_LOCKED)
      ulTemp |= ATTRIB_KEEP;

   return ulTemp;
}

static ULONG ConvertToJAMAttrib(ULONG ulAttrib)
{
   ULONG ulTemp=0;

   if (ulAttrib & ATTRIB_PRIVATE)
      ulTemp |= MSG_PRIVATE;
   if (ulAttrib & ATTRIB_CRASH)
      ulTemp |= MSG_CRASH;
   if (ulAttrib & ATTRIB_RCVD)
      ulTemp |= MSG_READ;
   if (ulAttrib & ATTRIB_SENT)
      ulTemp |= MSG_SENT;
   if (ulAttrib & ATTRIB_FILEATTACHED)
      ulTemp |= MSG_FILEATTACH;
   if (ulAttrib & ATTRIB_INTRANSIT)
      ulTemp |= MSG_INTRANSIT;
   if (ulAttrib & ATTRIB_ORPHAN)
      ulTemp |= MSG_ORPHAN;
   if (ulAttrib & ATTRIB_KILLSENT)
      ulTemp |= MSG_KILLSENT;
   if (ulAttrib & ATTRIB_LOCAL)
      ulTemp |= MSG_LOCAL;
   if (ulAttrib & ATTRIB_HOLD)
      ulTemp |= MSG_HOLD;
   if (ulAttrib & ATTRIB_FREQUEST)
      ulTemp |= MSG_FILEREQUEST;
   if (ulAttrib & ATTRIB_RRQ)
      ulTemp |= MSG_RECEIPTREQ;
   if (ulAttrib & ATTRIB_RECEIPT)
      ulTemp |= MSG_CONFIRMREQ;
   if (ulAttrib & ATTRIB_ARCHIVESENT)
      ulTemp |= MSG_ARCHIVESENT;
   if (ulAttrib & ATTRIB_DIRECT)
      ulTemp |= MSG_DIRECT;
   if (ulAttrib & ATTRIB_TRUNCFILE)
      ulTemp |= MSG_TRUNCFILE;
   if (ulAttrib & ATTRIB_KILLFILE)
      ulTemp |= MSG_KILLFILE;
   if (ulAttrib & ATTRIB_IMMEDIATE)
      ulTemp |= MSG_IMMEDIATE;
   if (ulAttrib & ATTRIB_GATE)
      ulTemp |= MSG_GATE;
   if (ulAttrib & ATTRIB_FORCEPICKUP)
      ulTemp |= MSG_FPU;
   if (ulAttrib & ATTRIB_DELETED)
      ulTemp |= MSG_DELETED;
   if (ulAttrib & ATTRIB_KEEP)
      ulTemp |= MSG_LOCKED;

   return ulTemp;
}

static ULONG CalcSubfieldsSize(PMSGHEADER pHeader, PFTNMESSAGE pMessage)
{
   ULONG ulSum=0;
   char pchTemp[LEN_5DADDRESS+1];
   PKLUDGE pKludge=NULL;

   /* Header-Elemente */
   ulSum += strlen(pHeader->pchFromName)+sizeof(JAMBINSUBFIELD);
   ulSum += strlen(pHeader->pchToName)+sizeof(JAMBINSUBFIELD);
   ulSum += strlen(pHeader->pchSubject)+sizeof(JAMBINSUBFIELD);

   NetAddrToString(pchTemp, &pHeader->FromAddress);
   ulSum += strlen(pchTemp)+sizeof(JAMBINSUBFIELD);
   NetAddrToString(pchTemp, &pHeader->ToAddress);
   ulSum += strlen(pchTemp)+sizeof(JAMBINSUBFIELD);

   /* Kludges */
   while (pKludge = MSG_FindKludge(pMessage, KLUDGE_ANY, pKludge))
   {
      switch(pKludge->ulKludgeType)
      {
         case KLUDGE_TOPT:  /* werden nicht geschrieben */
         case KLUDGE_FMPT:
         case KLUDGE_INTL:
            break;

         case KLUDGE_MSGID: /* spezielle Subfields */
         case KLUDGE_REPLY:
         case KLUDGE_FLAGS:
         case KLUDGE_PID:
         case KLUDGE_OTHER:
            ulSum += strlen(pKludge->pchKludgeText) + sizeof(JAMBINSUBFIELD);
            break;

         default: /* Alle anderen Kludges ausser KLUDGE_OTHER */
            ulSum += strlen(MSG_QueryKludgeName(pKludge->ulKludgeType)) +
                     strlen(pKludge->pchKludgeText) + 1 +
                     sizeof(JAMBINSUBFIELD);
            break;
      }
   }

   /* Attaches und Requests */
   if (pHeader->ulAttrib & (ATTRIB_FILEATTACHED | ATTRIB_FREQUEST))
   {
      char *pchTemp = pHeader->pchSubject;
      char *pchPrev = pchTemp;
      int NumFiles=0;

      while (*pchTemp)
      {
         /* Leerzeichen uebergehen */
         while (*pchTemp == ' ')
            pchTemp++;

         if (*pchTemp) /* noch ein File */
         {
            /* Name ueberspringen */
            pchPrev = pchTemp;

            while (*pchTemp && *pchTemp != ' ')
               pchTemp++;

            ulSum += pchTemp - pchPrev; /* Anzahl der Zeichen dazwischen */
            NumFiles++;
         }
      }
      /* Platz fuer Header dazuzaehlen */
      ulSum += sizeof(JAMBINSUBFIELD)*NumFiles;
   }

   return ulSum;
}

static ULONG CalcNewNumber(JAMAPIRECptr apirec)
{
   ULONG ulNewNum;

   /* Anzahl der Messages im Index berechnen */
   ulNewNum = _filelength(apirec->IdxHandle)/ sizeof(JAMIDXREC);

   /* Base-Nummer dazu -> neue Nummer */
   ulNewNum += apirec->HdrInfo.BaseMsgNum;

   return ulNewNum;
}


/*-----------------------------------------------------------------------------
 | Funktionsname: ReadSubfields
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Liesst die Subfields einer Message und konvertiert sie
 |               in Header-Felder und Kludges
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pAreaDef: Area-Definition
 |            msgnum: Nummer der Message
 |            SubfieldLen: Laenge der Subfields
 |            pHeader: Ziel-Header
 |            pMessage: Ziel-Message
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: Kludge-Pointer der MSGID-Kludge; NULL, falls keine MSGID
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: Wenn f〉 pMessage NULL uebergeben wird, werden keine Subfields
 |            gelesen, die nicht zum Header gehoeren
 +---------------------------------------------------------------------------*/

static PKLUDGE ReadSubfields(PAREADEFLIST pAreaDef, ULONG msgnum, ULONG SubfieldLen, PMSGHEADER pHeader, PFTNMESSAGE pMessage)
{
   JAMSUBFIELDptr pSubFields;
   JAMSUBFIELDptr pTemp;
   char pchAddress[LEN_5DADDRESS+1];
   PKLUDGE pMsgIDKludge=NULL;
   PSTRINGLIST pSeenList=NULL, pCurrentSeen=NULL;
   size_t len;

   /* jetzt alle Subfields */
   if (SubfieldLen)
   {
      pSubFields = malloc(SubfieldLen);
      if (JAMmbFetchSubfields((JAMAPIRECptr)pAreaDef->areahandle, pSubFields, SubfieldLen, msgnum))
      {
         /* abklappern */
         pTemp = pSubFields;

         while ((((PCHAR)pTemp)-((PCHAR)pSubFields)) < SubfieldLen)
         {
            switch(pTemp->LoID)
            {
               case JAMSFLD_OADDRESS:
                  if (pTemp->DatLen <= LEN_5DADDRESS)
                     len = pTemp->DatLen;
                  else
                     len = LEN_5DADDRESS;
                  memcpy(pchAddress, pTemp->Buffer, len);
                  pchAddress[len]=0;
                  StringToNetAddr(pchAddress, &pHeader->FromAddress, NULL);
                  break;

               case JAMSFLD_DADDRESS:
                  if (pTemp->DatLen <= LEN_5DADDRESS)
                     len = pTemp->DatLen;
                  else
                     len = LEN_5DADDRESS;
                  memcpy(pchAddress, pTemp->Buffer, len);
                  pchAddress[len]=0;
                  StringToNetAddr(pchAddress, &pHeader->ToAddress, NULL);
                  break;

               case JAMSFLD_SENDERNAME:
                  if (pTemp->DatLen <= LEN_USERNAME)
                     memcpy(pHeader->pchFromName, pTemp->Buffer, pTemp->DatLen);
                  else
                     memcpy(pHeader->pchFromName, pTemp->Buffer, LEN_USERNAME);
                  break;

               case JAMSFLD_RECVRNAME:
                  if (pTemp->DatLen <= LEN_USERNAME)
                     memcpy(pHeader->pchToName, pTemp->Buffer, pTemp->DatLen);
                  else
                     memcpy(pHeader->pchToName, pTemp->Buffer, LEN_USERNAME);
                  break;

               case JAMSFLD_SUBJECT:
                  if (pTemp->DatLen <= LEN_SUBJECT)
                     memcpy(pHeader->pchSubject, pTemp->Buffer, pTemp->DatLen);
                  else
                     memcpy(pHeader->pchSubject, pTemp->Buffer, LEN_SUBJECT);
                  break;

               case JAMSFLD_MSGID:
                  if (pMessage)
                  {
                     char pchKludge[LEN_KLUDGE+1];

                     if (pTemp->DatLen <= LEN_KLUDGE)
                        len = pTemp->DatLen;
                     else
                        len = LEN_KLUDGE;
                     memcpy(pchKludge, pTemp->Buffer, len);
                     pchKludge[len]=0;
                     pMsgIDKludge = MSG_SetKludge(pMessage, KLUDGE_MSGID, pchKludge, SETKLUDGE_UNIQUE);
                  }
                  break;

               case JAMSFLD_REPLYID:
                  if (pMessage)
                  {
                     char pchKludge[LEN_KLUDGE+1];

                     if (pTemp->DatLen <= LEN_KLUDGE)
                        len = pTemp->DatLen;
                     else
                        len = LEN_KLUDGE;
                     memcpy(pchKludge, pTemp->Buffer, len);
                     pchKludge[len]=0;
                     MSG_SetKludge(pMessage, KLUDGE_REPLY, pchKludge, SETKLUDGE_UNIQUE);
                  }
                  break;

               case JAMSFLD_PID:
                  if (pMessage)
                  {
                     char pchKludge[LEN_KLUDGE+1];

                     if (pTemp->DatLen <= LEN_KLUDGE)
                        len = pTemp->DatLen;
                     else
                        len = LEN_KLUDGE;
                     memcpy(pchKludge, pTemp->Buffer, len);
                     pchKludge[len]=0;
                     MSG_SetKludge(pMessage, KLUDGE_PID, pchKludge, SETKLUDGE_UNIQUE);
                  }
                  break;

               case JAMSFLD_TRACE:
                  if (pMessage)
                  {
                     char *pchKludge2;
                     char pchKludge[LEN_KLUDGE+20];

                     memset(pchKludge, 0, sizeof(pchKludge));
                     strcpy(pchKludge, "Via ");
                     pchKludge2=strchr(pchKludge, 0);
                     if (pTemp->DatLen <= LEN_KLUDGE)
                        memcpy(pchKludge2, pTemp->Buffer, pTemp->DatLen);
                     else
                        memcpy(pchKludge2, pTemp->Buffer, LEN_KLUDGE);
                     MSG_SetKludge(pMessage, KLUDGE_OTHER, pchKludge, SETKLUDGE_MULTIPLE);
                  }
                  break;

               case JAMSFLD_FTSKLUDGE:
                  if (pMessage)
                  {
                     char pchKludge[LEN_KLUDGE+1];

                     if (pTemp->DatLen <= LEN_KLUDGE)
                        len = pTemp->DatLen;
                     else
                        len = LEN_KLUDGE;
                     memcpy(pchKludge, pTemp->Buffer, len);
                     pchKludge[len]=0;
                     MSG_SetKludgeFromBuffer(pMessage, pchKludge, SETKLUDGE_MULTIPLE);
                  }
                  break;

               case JAMSFLD_SEENBY2D:
                  if (pMessage)
                  {
                     char pchKludge[LEN_KLUDGE+20];

                     if (pTemp->DatLen <= LEN_KLUDGE)
                        len = pTemp->DatLen;
                     else
                        len = LEN_KLUDGE;
                     memcpy(pchKludge, pTemp->Buffer, len);
                     pchKludge[len]=0;

                     if (!pSeenList)
                        pSeenList = pCurrentSeen = calloc(1, sizeof(STRINGLIST));
                     else
                     {
                        pCurrentSeen->next = calloc(1, sizeof(STRINGLIST));
                        pCurrentSeen = pCurrentSeen->next;
                     }

                     pCurrentSeen->length = len;
                     pCurrentSeen->pchString = malloc(pCurrentSeen->length+1);
                     strcpy(pCurrentSeen->pchString, pchKludge);
                  }
                  break;

               case JAMSFLD_PATH2D:
                  if (pMessage)
                  {
                     char *pchKludge2;
                     char pchKludge[LEN_KLUDGE+20];

                     memset(pchKludge, 0, sizeof(pchKludge));
                     strcpy(pchKludge, "PATH ");
                     pchKludge2=strchr(pchKludge, 0);
                     if (pTemp->DatLen <= LEN_KLUDGE)
                        memcpy(pchKludge2, pTemp->Buffer, pTemp->DatLen);
                     else
                        memcpy(pchKludge2, pTemp->Buffer, LEN_KLUDGE);
                     MSG_SetKludge(pMessage, KLUDGE_OTHER, pchKludge, SETKLUDGE_MULTIPLE);
                  }
                  break;

               case JAMSFLD_FLAGS:
                  if (pMessage)
                  {
                     char pchKludge[LEN_KLUDGE+1];

                     if (pTemp->DatLen <= LEN_KLUDGE)
                        len = pTemp->DatLen;
                     else
                        len = LEN_KLUDGE;
                     memcpy(pchKludge, pTemp->Buffer, len);
                     pchKludge[len]=0;
                     MSG_SetKludge(pMessage, KLUDGE_FLAGS, pchKludge, SETKLUDGE_UNIQUE);
                     FlagsToAttrib(pMessage, pHeader, AttribMap);
                  }
                  break;

               default:
                  break;

            }
            pTemp = (JAMSUBFIELDptr)(((PCHAR)pTemp)+sizeof(JAMBINSUBFIELD)+pTemp->DatLen);
         }
      }
      free(pSubFields);
   }

   if (pMessage)
      pMessage->pchSeenPath = (PCHAR) pSeenList;

   return pMsgIDKludge;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: TraceReplyChain
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Verfolgt die Reply-Kette und fuellt das Reply-Array
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pAreaDef: Area-Definition
 |            RawHeader: Puffer f. JAM-Header
 |            pHeader: Ziel-Header
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void TraceReplyChain(PAREADEFLIST pAreaDef, JAMHDRptr RawHeader, PMSGHEADER pHeader)
{
   int iReplyNr=0;
   ULONG ulNextMsg;

   /* Reply-Kette verfolgen */
   ulNextMsg =RawHeader->Reply1st;

   while (ulNextMsg && iReplyNr < NUM_REPLIES)
   {
      if (!JAMmbFetchMsgHdr((JAMAPIRECptr)pAreaDef->areahandle, RawHeader, ulNextMsg) ||
          (RawHeader->Attribute & MSG_DELETED))
         break;

      pHeader->ulReplies[iReplyNr] = ulNextMsg;
      iReplyNr++;
      ulNextMsg = RawHeader->ReplyNext;
   }

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: MergeText
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fuegt den normalen Messagetext und die SEEN-BYs zu einem
 |               Text zusammen
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pMessage: Message. Die Kette der SEEN-BYs steht in pchSeenPath
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | R…kgabewerte: Zeiger auf den neuen Message-Text
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static char *MergeText(PFTNMESSAGE pMessage)
{
   ULONG ulNewSize=0;
   PCHAR pchNewText, pchTemp;
   PSTRINGLIST pStringList;

   if (!pMessage->pchSeenPath)
      return pMessage->pchMessageText;

   /* Laenge der SEEN-BY-Zeilen ermitteln */
   pStringList = (PSTRINGLIST) pMessage->pchSeenPath;
   while (pStringList)
   {
      ulNewSize += pStringList->length+
                   1+    /* Zeilentrenner */
                   9;    /* "SEEN-BY: " */
      pStringList = pStringList->next;
   }

   ulNewSize+= strlen(pMessage->pchMessageText)+2;

   pchNewText = malloc(ulNewSize);
   strcpy(pchNewText, pMessage->pchMessageText);
   pchTemp = strchr(pchNewText, 0);
   if (pchTemp > pchNewText)
      pchTemp--;
   if (*pchTemp)
      if (*pchTemp != '\r')
      {
         pchTemp++;
         *pchTemp++ = '\r';
      }
      else
         pchTemp++;

   pStringList = (PSTRINGLIST) pMessage->pchSeenPath;
   pMessage->pchSeenPath = NULL;
   while (pStringList)
   {
      PSTRINGLIST pDel;

      /* Text anhaengen */
      strcpy(pchTemp, "SEEN-BY: ");
      pchTemp = strchr(pchTemp, 0);

      strcpy(pchTemp, pStringList->pchString);
      pchTemp = strchr(pchTemp, 0);
      *pchTemp++ = '\r';
      *pchTemp = 0;

      /* Knoten entfernen */
      pDel = pStringList;
      pStringList = pStringList->next;
      free(pDel->pchString);
      free(pDel);
   }
   free(pMessage->pchMessageText);
   pMessage->pchMessageText = pchNewText;

   return pchNewText;
}

/*-------------------------------- Modulende --------------------------------*/


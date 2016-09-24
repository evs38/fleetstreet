/*+---------------------------------------------------------------------------+
  | Titel: HANDLEMSG.C                                                        |
  +-----------------------------------------+---------------------------------+
  | Original von: Michael Hohner            | Am: 15.05.93                    |
  | Erstellt von: Harry Herrmannsdoerfer    | Am: 20.07.93                    |
  +-----------------------------------------+---------------------------------+
  | System: OS/2 2.x PM                                                       |
  +---------------------------------------------------------------------------+
  | Beschreibung:                                                             |
  |                                                                           |
  |    Schnittstelle Fleet Street - Squish-API                                |
  |                                                                           |
  +---------------------------------------------------------------------------+*/

/*---------------------------- Header-Dateien -------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#define INCL_DOS
#define INCL_WIN

#include <os2.h>

#include <stdlib.h>

#include <sys\stat.h>
#include <direct.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "..\main.h"
#include "..\version.h"
#include "..\structs.h"
#include "..\areaman\areaman.h"
#include "..\msgheader.h"
#include "..\util\fltutil.h"
#include "..\util\addrcnv.h"
#include "handlemsg.h"
#include "handletemplate.h"
#include "charset.h"
#include "kludgeapi.h"
#include "squishapi.h"
#include "ftsapi.h"
#include "..\jamapi\jamapi.h"

/*---------------------------- Defines --------------------------------------*/
#define LEN_MAXKLUDGE (LEN_5DADDRESS+ 15) /* Puffer f. MSGID                 */
#define MAX_TEXTBUFFER 50000              /* Zwischenpuffer beim Message-    */
                                          /* speichern.                      */
#define MAX_INITIALS     4                /* Maximale Anzahl der Initialen   */
                                          /* zum Quoten. 4 sollten reichen.  */
#define PID_KLUDGE   "FleetStreet " FLEETVER
#define TEARLINE_S   "---\n"
#define TEARLINE     "--- FleetStreet " FLEETVER "\n"
#define ORIGINLINE   " * Origin: "

#define LEN_MSGOVERHEAD  300

#define UMLAUTE "”แ …ข•ก"

/*---------------------------- Lokale Prototypen ----------------------------*/
static int  M_ReadMessage(PAREADEFLIST pAreaDef, PFTNMESSAGE pMessage, PMSGHEADER pHeader, ULONG nummsg);
static void M_CopyStdHeader(MSGHEADER *pHeader, AREADEFLIST *pactarea, char *pchCurrentName, char *pchCurrentAddress);

static char *M_CreateMsgID(char *pchMsgIDBuff, FTNADDRESS *address);
static char *M_GetInitials(PCHAR pchName, char *pchInitials, BOOL with_initials, CHAR chQuote);
static char *M_ConstructPrefix(char *pchStart, char *pchPrefixBuff, char chQuote);
static BOOL M_AddTearline(PCHAR msgtext, BOOL isecho, BOOL addtear, BOOL addorigin,
                          BOOL usepid, PCHAR pchOrigin, FTNADDRESS *pAddr);
static ULONG SplitMessage(char *pchOrigMsg, char **parts, char *pchToLine, ULONG ulMaxLen);
static char *StampToString2(PCHAR buffer, TIMESTAMP *timestamp);
static char *BuildRequestName(OUTBOUND *pOutbound, FTNADDRESS *pAddr, FTNADDRESS *pDefaultAddr, char *pchPath,
                              char *pchExtension);
static int CreatePath(char *pchPath);
static char *M_ReadRandomOrigin(PMSGTEMPLATE pTemplate, PCHAR pchOriginBuffer, USERDATAOPT *pUserData);
static void EnterSerial(void);
static void ExitSerial(void);
static void FillCurrentTime(PTIMESTAMP pTimeStamp);
static int AddToJamFile(AREADEFLIST *pAreaDef, char *pchJamPath, ULONG ulMsgID);

/*ออออออออออออออออออออออออออออ Globale Variablen ออออออออออออออออออออออออออออ*/

static HMTX hMsgApiSem=NULLHANDLE;     /* Mutex-Semaphore fuer Message-API */
static LONG lOpenApi = 0;              /* API-Open-Zaehler                 */
static ULONG ulCodePage;

PCHAR months[]={"Jan","Feb","Mar","Apr","May","Jun",
                "Jul","Aug","Sep","Oct","Nov","Dec"};

/*ีออออออออออออออออออออออออออออ MSG_OpenApi ออออออออออออออออออออออออออออออออธ
  ณMSG_OpenApi, initialisiert die Squish-API                                ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_OpenApi(PCHAR mainaddress)
{
   /* Semaphore anlegen */
   if (DosCreateMutexSem(NULL, &hMsgApiSem, 0, FALSE))
      return ERROR;

   if (SQ_OpenApi(mainaddress))
      return ERROR;
   else
   {
      JAM_OpenApi();
      lOpenApi ++;
      return OK;
   }
}

/*ีออออออออออออออออออออออออออออ MSG_IsApiOpen ออออออออออออออออออออออออออออออธ
  ณMSG_IsApiOpen, liefert TRUE, wenn die API geoeffnet ist                  ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

BOOL MSG_IsApiOpen(void)
{
   if (lOpenApi > 0)
      return TRUE;
   else
      return FALSE;
}

/*ีออออออออออออออออออออออออออออ MSG_CloseApi อออออออออออออออออออออออออออออออธ
  ณMSG_CloseApi, beendet die Arbeit mit der Squish-API                      ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

int MSG_CloseApi(PAREALIST arealist, PDRIVEREMAP pDriveRemap)
{
   AREADEFLIST *pactarea;

   pactarea = arealist->pFirstArea;  /* Evtl. offene Areas erstmal schliessen */
   while (pactarea)
   {
      if (pactarea->areahandle)
         MSG_CloseArea(arealist, pactarea->areadata.areatag, TRUE, 0, pDriveRemap);
      pactarea = pactarea->next;
   } /* endwhile */

   if (SQ_CloseApi())
      return ERROR;
   else
   {
      JAM_CloseApi();
      if (lOpenApi >0)
         lOpenApi--;
      DosCloseMutexSem(hMsgApiSem);
      return OK;
   }
}

/*ีออออออออออออออออออออออออออออ MSG_OpenArea อออออออออออออออออออออออออออออออธ
  ณOpenArea, oeffnet eine Message-Area und setzt einige AREA-Struktur-Vars  ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_OpenArea(PAREALIST arealist, PCHAR tag, LONG lOffset,
                          PDRIVEREMAP pDriveRemap)
{
   AREADEFLIST *pactarea;
   char pchPathName[LEN_PATHNAME+1];

   pactarea = AM_FindArea(arealist, tag);            /* Area in der Liste suchen */

   if (pactarea)
   {
      MSG_RemapArea(pchPathName, pactarea, pDriveRemap);

      EnterSerial();
      if (pactarea->usage==0)
      {
         switch(pactarea->areadata.areaformat)
         {
            case AREAFORMAT_FTS:
               pactarea->areahandle = FTS_OpenArea(pactarea, pchPathName);
               break;

            case AREAFORMAT_SQUISH:
               pactarea->areahandle = SQ_OpenArea(pactarea, pchPathName);
               break;

            case AREAFORMAT_JAM:
               pactarea->areahandle = JAM_OpenArea(pactarea, pchPathName);
               break;

            default:
               pactarea->areahandle = NULL;
               break;
         }
         if (!pactarea->areahandle)
         {
            ExitSerial();
            return AREA_OPEN_ERROR;
         }
      }
      pactarea->usage++;

      /* Anzahl der Messages in der Area setzen */
      if (pactarea->maxmessages == 0)
         pactarea->currentmessage=0;
      else
         if (pactarea->usage == 1)  /* gerade geoeffnet */
         {
            ULONG ulNum;

            switch(pactarea->areadata.areaformat)
            {
               case AREAFORMAT_FTS:
                  ulNum = FTS_ReadLastread(pactarea, pchPathName, lOffset);
                  break;

               case AREAFORMAT_SQUISH:
                  ulNum = SQ_ReadLastread(pactarea, pchPathName, lOffset);
                  break;

               case AREAFORMAT_JAM:
                  ulNum = JAM_ReadLastread(pactarea, lOffset);
                  break;

               default:
                  break;
            }

            if (ulNum > pactarea->maxmessages || ulNum==0)
               pactarea->currentmessage=0;
            else
               pactarea->currentmessage=ulNum;
         }

      pactarea->scanned = TRUE;
      ExitSerial();

      return OK;
   }
   else
      return NO_AREA_OPEN;
}


/*ีออออออออออออออออออออออออออออ MSG_CloseArea ออออออออออออออออออออออออออออออธ
  ณCloseArea, schliesst eine Message-Area und setzt den LastRead-Pointer.   ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_CloseArea(PAREALIST arealist, PCHAR tag, BOOL Write_LR, LONG lOffset, PDRIVEREMAP pDriveRemap)
{
   AREADEFLIST *pactarea;
   char pchPathName[LEN_PATHNAME+1];

   pactarea = AM_FindArea(arealist, tag);            /* Area in der Liste suchen */
   if (pactarea)
   {
      MSG_RemapArea(pchPathName, pactarea, pDriveRemap);

      if (Write_LR && pactarea->maxmessages>0)
         switch(pactarea->areadata.areaformat)
         {
            case AREAFORMAT_FTS:
               FTS_WriteLastread(pactarea, pchPathName, lOffset);
               break;

            case AREAFORMAT_SQUISH:
               SQ_WriteLastread(pactarea, pchPathName, lOffset);
               break;

            case AREAFORMAT_JAM:
               JAM_WriteLastread(pactarea, lOffset);
               break;

            default:
               break;
         }

      EnterSerial();
      if (pactarea->usage == 1)
      {
         int ret=1;
         switch(pactarea->areadata.areaformat)
         {
            case AREAFORMAT_FTS:
               ret = FTS_CloseArea(pactarea);
               break;

            case AREAFORMAT_SQUISH:
               ret = SQ_CloseArea(pactarea);
               break;

            case AREAFORMAT_JAM:
               ret = JAM_CloseArea(pactarea);
               break;

            default:
               break;
         }
         pactarea->usage = 0;
         pactarea->areahandle = NULL;
         if (ret)
         {
            ExitSerial();
            return AREA_CLOSE_ERROR;
         }
      }
      else
         if (pactarea->usage > 0)
            pactarea->usage--;

      ExitSerial();
   }
   return OK;
}

int MSG_LockArea(PCHAR tag, PAREALIST arealist)
{
   AREADEFLIST *pactarea;

   EnterSerial();

   pactarea = AM_FindArea(arealist, tag);   /* Area in der Liste suchen */
   if (!pactarea)
   {
      ExitSerial();
      return AREA_NOT_FOUND;
   }
   else
   {
      if (pactarea->bLocked || pactarea->usage > 0)
      {
         ExitSerial();
         return ERROR;   /* bereits gelockt oder verwendet */
      }
      else
      {
         pactarea->bLocked = TRUE;
         ExitSerial();
         return OK;
      }
   }
}

int MSG_UnlockArea(PCHAR tag, PAREALIST arealist)
{
   AREADEFLIST *pactarea;

   EnterSerial();

   pactarea = AM_FindArea(arealist, tag);   /* Area in der Liste suchen */
   if (!pactarea)
   {
      ExitSerial();
      return AREA_NOT_FOUND;
   }
   else
   {
      if (!pactarea->bLocked)
      {
         ExitSerial();
         return ERROR;   /* nicht gelockt */
      }
      else
      {
         pactarea->bLocked = FALSE;
         ExitSerial();
         return OK;
      }
   }
}

/*ีอออออออออออออออออออออออออออ MSG_ReadAct  ออออออออออออออออออออออออออออออออออธ
  ณMSG_ReadAct  liest die aktuelle Message (=pactarea->currentmessage) ein.   ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_ReadAct(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag)
{
   AREADEFLIST *pactarea;

   pactarea = AM_FindArea(arealist, tag);
   if (!pactarea)
      return AREA_NOT_FOUND;
   else
      return M_ReadMessage(pactarea, pMessage, pHeader, pactarea->currentmessage);
}

/*ีอออออออออออออออออออออออออออ MSG_ReadNum  ออออออออออออออออออออออออออออออออออธ
  ณMSG_ReadAct  liest die Message mit der Nummer 'number' ein.                ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_ReadNum(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag, int number)
{
   AREADEFLIST *pactarea;

   pactarea = AM_FindArea(arealist, tag);
   if (!pactarea)
      return AREA_NOT_FOUND;
   else
      return M_ReadMessage(pactarea, pMessage, pHeader, number);
}


/*ีอออออออออออออออออออออออออออ MSG_ReadPrev ออออออออออออออออออออออออออออออออออธ
  ณMSG_ReadPrev liest - soweit m”glich - die vorherige Message aus einer Area ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_ReadPrev(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag)
{
   AREADEFLIST *pactarea;

   pactarea = AM_FindArea(arealist, tag);
   if (pactarea)
   {
      if (pactarea->currentmessage > 1)
      {                                      /* Sind wir schon bei der      */
         pactarea->currentmessage--;         /* ersten Message angekommen ? */

         return M_ReadMessage(pactarea, pMessage, pHeader, pactarea->currentmessage);
      }
      else
         return FIRST_MESSAGE;
   }
   else
      return ERROR;
}


/*ีอออออออออออออออออออออออออออ MSG_ReadNext ออออออออออออออออออออออออออออออออออธ
  ณMSG_ReadNext liest - soweit m”glich - die nchste Message aus einer Area   ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_ReadNext(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag)
{
   AREADEFLIST *pactarea;

   pactarea = AM_FindArea(arealist, tag);
   if (pactarea)
   {
      if (pactarea->currentmessage < pactarea->maxmessages)
      {
         pactarea->currentmessage++;

         return M_ReadMessage(pactarea, pMessage, pHeader, pactarea->currentmessage);
      }
      else
         return LAST_MESSAGE;
   }
   else
      return ERROR;
}


/*ีอออออออออออออออออออออออออออ MSG_ReadHeader ออออออออออออออออออออออออออออออออธ
  ณMSG_ReadHeader liest den Message-Header der Nachricht number ein.          ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

int MSG_ReadHeader(PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag, int number)
{
   AREADEFLIST *pactarea;
   int rc;

   pactarea = AM_FindArea(arealist, tag);
   if (pactarea)
   {
      if (number == 0)
         number = pactarea->currentmessage;

      MSG_ClearMessage(pHeader, NULL);

      /* Abfangen, ob ueberhaupt Messages da sind ! Sonst kann das mit
         currentmessage (min=1) nicht klappen.
      ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
      if (pactarea->maxmessages == 0)
         return NO_MESSAGE;

      /* Message ber die jeweilige API lesen.
      ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
      EnterSerial();
      switch (pactarea->areadata.areaformat)
      {
         case AREAFORMAT_FTS:
            rc = FTS_ReadHeader(pactarea, pHeader, number);
            break;

         case AREAFORMAT_SQUISH:
            rc = SQ_ReadHeader(pactarea, pHeader, number);
            break;

         case AREAFORMAT_JAM:
            rc = JAM_ReadHeader(pactarea, pHeader, number);
            break;

         default:
            rc=ERROR;
            break;
      }
      ExitSerial();
      return rc;
   }
   else
      return AREA_NOT_FOUND;
}

int MSG_LinkMessages(PAREALIST arealist, PCHAR tag, ULONG ulReplyID, ULONG ulOrigID, PDRIVEREMAP pdriveremap)
{
   AREADEFLIST *pactarea;
   int rc;

   if (!ulReplyID || !ulOrigID)
      return ERROR;

   pactarea = AM_FindArea(arealist, tag);
   if (pactarea)
   {
      EnterSerial();
      switch (pactarea->areadata.areaformat)
      {
         case AREAFORMAT_FTS:
            rc = FTS_LinkMessages(pactarea, ulReplyID, ulOrigID, pdriveremap);
            break;

         case AREAFORMAT_SQUISH:
            rc = SQ_LinkMessages(pactarea, ulReplyID, ulOrigID);
            break;

         case AREAFORMAT_JAM:
            rc = JAM_LinkMessages(pactarea, ulReplyID, ulOrigID);
            break;

         default:
            rc = ERROR;
            break;
      }
      ExitSerial();
      return rc;
   }
   else
      return AREA_NOT_FOUND;
}


/*+--------------------------- MSG_AddMessage --------------------------------+
  | Speichert eine neue Message und fgt die notwendigen Kludges ein.         |
  | Es wird die Tearline und die Origin-Line angehngt, falls es eine Echo-   |
  | area ist. Ansonsten wird anhand des Setups entschieden.                   |
  +---------------------------------------------------------------------------+*/

int MSG_AddMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                   PAREALIST arealist, PCHAR tag, USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                   PDRIVEREMAP pDriveRemap, LONG lSplitLen,
                   TEMPLATELIST *ptemplatelist, ULONG ulOptions, MsgCallback AddCallback)
{
   int          itextlen, j, len;
   AREADEFLIST  *pactarea;
   FTNADDRESS   NewAddress;
   ULONG        prevmsgid;
   char         *msgparts[99];
   int          iPart=0;
   char         pchToLine[100]="";
   char         pchKludgeBuffer[80];
   ULONG        ulCountParts=0;
   ULONG        msgnum=0;
   char         pchOriginLine[LEN_ORIGIN+1];
   PMSGTEMPLATE pTemplate;
   int          rc;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, tag);
   pactarea = AM_FindArea(arealist, tag);

   memset(msgparts, 0, sizeof(msgparts));
   NewAddress = pHeader->FromAddress;

   if (ulOptions & ADDOPT_MATCHADDRESS)
   {
      /* Address-Matching */
      j = MSG_MatchAddress(&pHeader->ToAddress, userdaten, &pHeader->FromAddress);

      if (j>=0)
         StringToNetAddr(userdaten->address[j], &pHeader->FromAddress, NULL);
   }


#if 0 /* Mit Default-Zone = 0 erzeugt die MSGAPI jetzt immer die INTL-Kludge selbst */
   /* INTL-Kludge in Netmail-Areas erzeugen und anfuegen
   ------------------------------------------------------------------ */
   if (pactarea->areadata.areatype == AREATYPE_NET &&
       pHeader->ToAddress.usZone == pHeader->FromAddress.usZone)
      MSG_SetKludgeVar(pMessage, KLUDGE_INTL, SETKLUDGE_UNIQUE, "%u:%u/%u %u:%u/%u",
                                                                   pHeader->ToAddress.usZone,
                                                                   pHeader->ToAddress.usNet,
                                                                   pHeader->ToAddress.usNode,
                                                                   pHeader->FromAddress.usZone,
                                                                   pHeader->FromAddress.usNet,
                                                                   pHeader->FromAddress.usNode);
   else
#endif
      MSG_RemoveKludge(pMessage, KLUDGE_INTL);

   /* Evtl. den PID-Kludge anhaengen
   ------------------------------------------------------------------ */
   if (generaloptions->usepid ||
       (pactarea->areadata.areatype == AREATYPE_NET && !generaloptions->tearinnet))
   {
      MSG_SetKludge(pMessage, KLUDGE_PID, PID_KLUDGE, SETKLUDGE_UNIQUE);
   }
   else
      MSG_RemoveKludge(pMessage, KLUDGE_PID);

   /* To:-Zeile sichern
   ------------------------------------------------------------------ */

   if (!strnicmp(pMessage->pchMessageText, "TO:", 3))
   {
      char *pchHelp=pMessage->pchMessageText;
      char *pchDest=pchToLine;

      while (*pchHelp && (pchHelp - pMessage->pchMessageText) <80 && *pchHelp!='\n')
         *pchDest++=*pchHelp++;
      *pchDest='\0';
   }

   /* Restliche Kludges zuruecksetzen */
   MSG_RemoveKludge(pMessage, KLUDGE_REPLYTO);
   MSG_RemoveKludge(pMessage, KLUDGE_REPLYADDR);
   MSG_RemoveKludge(pMessage, KLUDGE_AREA);
   MSG_RemoveKludge(pMessage, KLUDGE_FLAGS);
   MSG_RemoveKludge(pMessage, KLUDGE_SPLIT);
   MSG_RemoveKludge(pMessage, KLUDGE_OTHER);
   MSG_RemoveKludge(pMessage, KLUDGE_FMPT);
   MSG_RemoveKludge(pMessage, KLUDGE_TOPT);

   /* Message aufteilen
   ------------------------------------------------------------------ */
   ulCountParts=SplitMessage(pMessage->pchMessageText, msgparts, pchToLine, lSplitLen?(lSplitLen - LEN_MSGOVERHEAD):0);
   msgnum=pactarea->maxmessages;
   pMessage->pchMessageText=NULL;

   while (iPart <= 99 && msgparts[iPart])
   {
      char pchOrigSubj[LEN_SUBJECT+1];

      if (ulCountParts >1)
      {
         char pchNode[LEN_5DADDRESS+1];
         char pchDate[20];

         /* Split-Kludge erzeugen */
         sprintf(pchNode, "%d/%d", NewAddress.usNet, NewAddress.usNode);
         StampToString2(pchDate, &pHeader->StampWritten);
         MSG_SetKludgeVar(pMessage, KLUDGE_SPLIT, SETKLUDGE_UNIQUE,
                          "%-18s @%-11s %-5d %02d/%02d ++++++++++++",
                          pchDate, pchNode, msgnum,
                          iPart+1, ulCountParts);

         if (iPart==0)
            strcpy(pchOrigSubj, pHeader->pchSubject);
      }

      if (iPart>0)
      {
         ULONG ulLen=0;

         /* Flags zuruecksetzen */
         pHeader->ulAttrib &= ~(ATTRIB_FILEATTACHED | ATTRIB_FREQUEST);

         /* Zahl im Subject einsetzen */
         strcpy(pHeader->pchSubject, pchOrigSubj);
         ulLen=strlen(pchOrigSubj);

         if (ulLen > (LEN_SUBJECT-5))
            sprintf(pHeader->pchSubject+ (LEN_SUBJECT-5), " (%2d)", iPart+1);
         else
            sprintf(pHeader->pchSubject+ ulLen, " (%2d)", iPart+1);
      }

      /* MSGID erzeugen
      ------------------------------------------------------------------ */
      M_CreateMsgID(pchKludgeBuffer, &NewAddress);
      MSG_SetKludge(pMessage, KLUDGE_MSGID, pchKludgeBuffer, SETKLUDGE_UNIQUE);
      if (ulCodePage == 437)
         MSG_SetKludge(pMessage, KLUDGE_CHRS, "IBMPC 2", SETKLUDGE_UNIQUE);
      else
         MSG_RemoveKludge(pMessage, KLUDGE_CHRS);

      /* Leerzeile zwischen Text und Tearline erzwingen */
      len=strlen(msgparts[iPart])-1;
      while(len >=0 && (msgparts[iPart][len]=='\n' || msgparts[iPart][len]==' '))
         len--;
      len++;
      if (len>0)
         strcpy(&msgparts[iPart][len], "\n\n");
      else
         msgparts[iPart][0]='\0';

      /* Tearline und ORIGIN-Line anfuegen, falls noch keine Origin-Line
         da ist. Dabei die richtige (???) Adresse verwenden.
      ------------------------------------------------------------------ */
      if (pTemplate->randomorigin)
         M_ReadRandomOrigin(pTemplate, pchOriginLine, userdaten);
      else
         if (pTemplate->TOrigin[0])
            strcpy(pchOriginLine, pTemplate->TOrigin);
         else
            strcpy(pchOriginLine, userdaten->defaultorigin);

      if (M_AddTearline(msgparts[iPart],
                    (pactarea->areadata.areatype == AREATYPE_ECHO),
                    generaloptions->tearinnet,
                    generaloptions->origininnet,
                    generaloptions->usepid,
                    pchOriginLine,
                    &NewAddress))
      {
         /* PID trotzdem erzeugen */
         MSG_SetKludge(pMessage, KLUDGE_PID, PID_KLUDGE, SETKLUDGE_UNIQUE);
      }

      /* Evtl. auftretende Steuerzeichen ersetzen.
      ------------------------------------------------------------------ */
      itextlen = strlen(msgparts[iPart]);
      for (j=0; j<itextlen; j++)
         switch(msgparts[iPart][j])
         {
            case '\x09':
            case '\x8D':
               msgparts[iPart][j]=' ';
               break;

            case '\n':
               msgparts[iPart][j]='\r';        /* wegen MLFIE_NOTRANS im MLE */
               break;

            default:
               break;
         }

      /* alte Message-ID merken */
      prevmsgid=MSG_MsgnToUid(arealist, tag, pactarea->currentmessage);

      /* Message ber die passende API schreiben.
      ------------------------------------------------------------------ */
      pMessage->pchMessageText = msgparts[iPart];
      EnterSerial();
      switch (pactarea->areadata.areaformat)
      {
         case AREAFORMAT_FTS:
            rc = FTS_AddMessage(pactarea, pHeader, pMessage, pDriveRemap);
            break;

         case AREAFORMAT_SQUISH:
            rc = SQ_AddMessage(pactarea, pHeader, pMessage);
            break;

         case AREAFORMAT_JAM:
            rc = JAM_AddMessage(pactarea, pHeader, pMessage);
            break;

         default:
           break;
      } /* endswitch */
      ExitSerial();

      if (rc) /* Fehlerpruefung */
      {
         /* Speicher f. Teile wieder freigeben */
         for (iPart=0; iPart < ulCountParts; iPart++)
            free(msgparts[iPart]);

         pMessage->pchMessageText=NULL;
         pMessage->pchSeenPath=NULL;

         return MSG_WRITE_ERROR;
      }
      else
      {
         /* ECHOMAIL.JAM, NETMAIL.JAM schreiben */
         if (pactarea->areadata.areaformat == AREAFORMAT_JAM &&
             generaloptions->jampath[0])
            AddToJamFile(pactarea, generaloptions->jampath, pHeader->ulMsgID);

         /* Callback rufen */
         if (AddCallback)
            AddCallback(pactarea, pHeader->ulMsgID, pHeader);
      }

      /* Anzahl der Messages erh”hen. Wenn die aktuelle auch die letzte
         ist oder die Area vorher leer war, dann kann die Neue die
         Aktuelle werden.
      ------------------------------------------------------------------ */
      j=MSG_UidToMsgn(arealist, tag, prevmsgid, TRUE);
      if (j==0)
         pactarea->currentmessage=1;
      else
      {
         pactarea->currentmessage=j;
#if 0
         if (j == (pactarea->maxmessages-1))
            pactarea->currentmessage++;
#endif
      }

      iPart++;
   }

   /* Speicher f. Teile wieder freigeben */
   for (iPart=0; iPart < ulCountParts; iPart++)
      free(msgparts[iPart]);

   pMessage->pchMessageText=NULL;
   pMessage->pchSeenPath=NULL;

   return OK;
}

static ULONG SplitMessage(char *pchOrigMsg, char **parts, char *pchToLine, ULONG ulMaxLen)
{
   int iPart=0;
   char *pchText=NULL;
   char *pchOText=NULL;
   char *pchHelp=NULL;

   if (!pchOrigMsg || strlen(pchOrigMsg)==0)
   {
      parts[0]=malloc(LEN_MSGOVERHEAD);
      parts[0][0]='\0';
      return 1;
   }

   if (ulMaxLen == 0)   /* kein Split */
   {
      parts[0] = malloc(strlen(pchOrigMsg)+LEN_MSGOVERHEAD);
      strcpy(parts[0], pchOrigMsg);
      return 1;
   }

   pchText=strdup(pchOrigMsg);
   pchOText=pchText;

   while (strlen(pchText) > ulMaxLen)
   {
      pchHelp=pchText+ulMaxLen;

      /* Absatz suchen */
      while(pchHelp > pchText && *pchHelp!='\n')
         pchHelp--;

      if (pchHelp == pchText)
      {
         /* kein Absatz da, Leerzeichen suchen */
         pchHelp=pchText+ulMaxLen;
         while(pchHelp > pchText && *pchHelp!=' ')
            pchHelp--;
      }
      if (pchHelp == pchText)
      {
         /* kein Leerzeichen, brutal drueberbuegeln */
         pchHelp=pchText+ulMaxLen;
      }
      *pchHelp='\0';
      parts[iPart]=malloc(ulMaxLen+LEN_MSGOVERHEAD);
      if (pchToLine && iPart >0)
      {
         strcpy(parts[iPart], pchToLine);
         strcat(parts[iPart], "\n\n");
      }
      else
         parts[iPart][0]='\0';
      strcat(parts[iPart], pchText);
      pchText=++pchHelp;
      iPart++;
   }
   /* letzter Teil */
   if (pchText[0])
   {
      parts[iPart]=malloc(ulMaxLen+LEN_MSGOVERHEAD);
      if (pchToLine && iPart >0)
      {
         strcpy(parts[iPart], pchToLine);
         strcat(parts[iPart], "\n\n");
      }
      else
         parts[iPart][0]='\0';
      strcat(parts[iPart], pchText);
   }
   free(pchOText);

   return iPart+1;
}

static int AddToJamFile(AREADEFLIST *pAreaDef, char *pchJamPath, ULONG ulMsgID)
{
   char pchFileName[LEN_PATHNAME+1];
   FILE *pfJamFile;

   /* Filename erzeugen */
   strcpy(pchFileName, pchJamPath);
   switch(pAreaDef->areadata.areatype)
   {
      case AREATYPE_ECHO:
         strcat(pchFileName, "\\ECHOMAIL.JAM");
         break;

      case AREATYPE_NET:
         strcat(pchFileName, "\\NETMAIL.JAM");
         break;

      default:
         return 1; /* sollte nie vorkommen */
   }

   if (pfJamFile = fopen(pchFileName, "a"))
   {
      fprintf(pfJamFile, "%s %d\n", pAreaDef->areadata.pathfile, ulMsgID);

      fclose(pfJamFile);
      return 0;
   }
   else
      return 2;
}

int MSG_MatchAddress(FTNADDRESS *pDestAddress, USERDATAOPT *userdaten, FTNADDRESS *pCurrentAddr)
{
   int matchcode, bestmatch=0, matchaddr=-1;
   FTNADDRESS MyAddr;
   int j;

   /* initialisieren mit aktueller Adresse */
   if (pDestAddress->usZone == pCurrentAddr->usZone)
   {
      bestmatch=1;
      if (pDestAddress->usNet == pCurrentAddr->usNet)
      {
         if (pDestAddress->usNode == pCurrentAddr->usNode)
         {
            bestmatch=3;
         }
      }
   }

   /* besseren Match ermitteln */
   j=0;
   while (j<MAX_ADDRESSES && userdaten->address[j][0])
   {
      StringToNetAddr(userdaten->address[j], &MyAddr, NULL);
      matchcode=0;

      if (pDestAddress->usZone == MyAddr.usZone)
      {
         matchcode=1;
         if (pDestAddress->usNet == MyAddr.usNet)
         {
            if (pDestAddress->usNode == MyAddr.usNode)
            {
               matchcode=3;
            }
         }
      }
      if (matchcode > bestmatch)
      {
         bestmatch = matchcode;
         matchaddr = j;
      }
      j++;
   }

   return matchaddr;
}


/*ีอออออออออออออออออออออออออออ MSG_ChangeMessage อออออออออออออออออออออออออออออธ
  ณ Die aktuelle Message verndern. Dabei drfen keine Control-Infos ver-     ณ
  ณ ndert werden, da die Squish-API das nicht haben will.                    ณ
  ณ                                                                           ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_ChangeMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                      PAREALIST arealist, PCHAR tag,
                      USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                      BOOL bChangeKludges, TEMPLATELIST *ptemplatelist, MsgCallback ChangeCallback)
{
   int          itextlen, j;
   char         *pchTextBuff=NULL,
                pchLineBuff[LEN_MAXKLUDGE+1]    = "",
                pchOriginLine[LEN_ORIGIN+1];
   AREADEFLIST  *pactarea;
   FTNADDRESS   NewAddress;
   int len;
   PMSGTEMPLATE pTemplate;
   int ret;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, tag);
   pactarea = AM_FindArea(arealist, tag);

   NewAddress = pHeader->FromAddress;

   /* neuen Messagetext in den Puffer kopieren
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   pchTextBuff = pMessage->pchMessageText;
   pMessage->pchMessageText=malloc(strlen(pchTextBuff)+LEN_MSGOVERHEAD);
   strcpy(pMessage->pchMessageText, pchTextBuff);

   /* Leerzeile zwischen Text und Tearline erzwingen */
   len=strlen(pMessage->pchMessageText)-1;
   while(len >=0 && (pMessage->pchMessageText[len]=='\n' || pMessage->pchMessageText[len]==' '))
      len--;
   len++;
   if (len>0)
      strcpy(&pMessage->pchMessageText[len], "\n\n");
   else
      pMessage->pchMessageText[0]='\0';

   if (bChangeKludges)
   {
      /* Kludges updaten */

#if 0 /* Mit Default-Zone = 0 erzeugt die MSGAPI jetzt immer die INTL-Kludge selbst */
      /* INTL-Kludge in Netmail-Areas erzeugen und anfuegen
      ------------------------------------------------------------------ */
      if (pactarea->areadata.areatype == AREATYPE_NET &&
          pHeader->ToAddress.usZone == pHeader->FromAddress.usZone)
         MSG_SetKludgeVar(pMessage, KLUDGE_INTL, SETKLUDGE_UNIQUE, "%u:%u/%u %u:%u/%u", pHeader->ToAddress.usZone,
                                                                      pHeader->ToAddress.usNet,
                                                                      pHeader->ToAddress.usNode,
                                                                      pHeader->FromAddress.usZone,
                                                                      pHeader->FromAddress.usNet,
                                                                      pHeader->FromAddress.usNode);
      else
#endif
         MSG_RemoveKludge(pMessage, KLUDGE_INTL);

      /* Evtl. den PID-Kludge anhaengen
      ------------------------------------------------------------------ */
      if (generaloptions->usepid)
      {
         MSG_SetKludge(pMessage, KLUDGE_PID, PID_KLUDGE, SETKLUDGE_UNIQUE);
      }
      else
         MSG_RemoveKludge(pMessage, KLUDGE_PID);

      /* MSGID erzeugen
      ------------------------------------------------------------------ */
      M_CreateMsgID(pchLineBuff, &NewAddress);
      MSG_SetKludge(pMessage, KLUDGE_MSGID, pchLineBuff, SETKLUDGE_UNIQUE);

      if (ulCodePage == 437)
         MSG_SetKludge(pMessage, KLUDGE_CHRS, "IBMPC 2", SETKLUDGE_UNIQUE);
      else
         MSG_RemoveKludge(pMessage, KLUDGE_CHRS);

      /* Restliche Kludges zuruecksetzen */
      MSG_RemoveKludge(pMessage, KLUDGE_TOPT);
      MSG_RemoveKludge(pMessage, KLUDGE_FMPT);
      MSG_RemoveKludge(pMessage, KLUDGE_REPLYTO);
      MSG_RemoveKludge(pMessage, KLUDGE_REPLYADDR);
      MSG_RemoveKludge(pMessage, KLUDGE_AREA);
      MSG_RemoveKludge(pMessage, KLUDGE_FLAGS);
      MSG_RemoveKludge(pMessage, KLUDGE_OTHER);
   }

   /* Tearline und ORIGIN-Line anfuegen, falls noch keine Origin-Line
      da ist. Dabei die richtige (???) Adresse verwenden.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   if (pTemplate->randomorigin)
      M_ReadRandomOrigin(pTemplate, pchOriginLine, userdaten);
   else
      if (pTemplate->TOrigin[0])
         strcpy(pchOriginLine, pTemplate->TOrigin);
      else
         strcpy(pchOriginLine, userdaten->defaultorigin);

   if (M_AddTearline(pMessage->pchMessageText,
                     (pactarea->areadata.areatype == AREATYPE_ECHO),
                     generaloptions->tearinnet,
                     generaloptions->origininnet,
                     generaloptions->usepid,
                     pchOriginLine,
                     &NewAddress))
   {
      if (bChangeKludges)
      {
         /* PID trotzdem erzeugen */
         MSG_SetKludge(pMessage, KLUDGE_PID, PID_KLUDGE, SETKLUDGE_UNIQUE);
      }
   }

   itextlen = strlen(pMessage->pchMessageText)   +1;

   /* Evtl. auftretende Steuerzeichen ersetzen.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   for (j=0;j<itextlen;j++)
      switch(pMessage->pchMessageText[j])
      {
         case '\x09':
         case '\x8D':
            pMessage->pchMessageText[j]=' ';
            break;

         case '\n':
            pMessage->pchMessageText[j]='\r';        /* wegen MLFIE_NOTRANS im MLE */
            break;

         default:
            break;
      }

   pHeader->ulAttrib |= ATTRIB_LOCAL;

   /* Datum anpassen */
   FillCurrentTime(&pHeader->StampWritten);
   pHeader->StampArrived = pHeader->StampWritten;

   /* Message ber die passende API schreiben.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   EnterSerial();
   switch (pactarea->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         ret = FTS_ChangeMessage(pactarea, pHeader, pMessage, pactarea->currentmessage);
         break;

      case AREAFORMAT_SQUISH:
         ret = SQ_ChangeMessage(pactarea, pHeader, pMessage, pactarea->currentmessage);
         break;

      case AREAFORMAT_JAM:
         ret = JAM_ChangeMessage(pactarea, pHeader, pMessage, pactarea->currentmessage);
         if (bChangeKludges && generaloptions->jampath[0])
            AddToJamFile(pactarea, generaloptions->jampath, pHeader->ulMsgID);
         break;

      default:
         ret=ERROR;
         break;
   }
   ExitSerial();

   if (!ret)
   {
      if (ChangeCallback)
         ChangeCallback(pactarea, pHeader->ulMsgID, pHeader);
   }

   free(pMessage->pchMessageText);
   pMessage->pchMessageText = pchTextBuff;

   return ret;
}


/*ีอออออออออออออออออออออออออออ MSG_NewMessage ออออออออออออออออออออออออออออออออธ
  ณ Es wird evtl. vorhandener Puffer freigegeben und alle msginfo-Werte auf   ณ
  ณ 0 gesetzt. Die aufrufende Funktion muss sich um den Pufferspeicher fr    ณ
  ณ die Message selbst kmmern. Die Standard-Header-Informationen werden ge-  ณ
  ณ setzt. Die MSGID wird erst beim echten Speichern erzeugt.                 ณ
  ณ                                                                           ณ
  ณ Irgendwann wird hier mal ein Message-Template eingesetzt.                 ณ
  ณ                                                                           ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_NewMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                   PAREALIST arealist, PCHAR tag,
                   char *pchCurrentName, char *pchCurrentAddress, LONG *iptInitialPos)
{
   AREADEFLIST  *pactarea;

   pactarea = AM_FindArea(arealist, tag);

   /* Evtl. noch aktive Puffer freigeben
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   MSG_ClearMessage(pHeader, pMessage);

   /* Neue Puffer anfordern, l”schen und der MESSAGEINFO zuweisen
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   if (!(pMessage->pchMessageText = malloc(MAX_TEXTBUFFER+1)))
      return OUT_OF_MEMORY;

   pMessage->pchSeenPath=NULL;

   /* Default-Header-Informationen passend zur Area einfgen.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   M_CopyStdHeader(pHeader, pactarea, pchCurrentName, pchCurrentAddress);
   pHeader->pchToName[0] = '\0';
   pHeader->pchSubject[0]='\0';
   memset(&(pHeader->ToAddress), 0, sizeof(FTNADDRESS));

   pMessage->pchMessageText[0]='\0';
   *iptInitialPos=0;

   return OK;
}

int MSG_NewMessageStep2(TEMPLATELIST *ptemplatelist, PAREALIST arealist, PCHAR tag,
                        PFTNMESSAGE pMessage, PMSGHEADER pHeader, LONG *iptInitialPos)
{
   char *pchNext;
   PMSGTEMPLATE pTemplate;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, tag);

   /* Message-Template einsetzen
   ------------------------------------------------------------------ */
   pchNext=TplHeader(pTemplate, pMessage->pchMessageText, pHeader->pchToName);
   *iptInitialPos=strlen(pMessage->pchMessageText);
   TplFooter(pTemplate, pchNext, pHeader->pchFromName);

   return OK;
}


/*ีอออออออออออออออออออออออออออ MSG_CopyMessage อออออออออออออออออออออออออออออออธ
  ณ Kopiert die aktuelle Message in eine andere Area                          ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_CopyMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist,
                    PCHAR dest_tag, PDRIVEREMAP pDriveRemap, USERDATAOPT *userdaten,
                    GENERALOPT *generaloptions, MsgCallback CopyCallback, ULONG ulOptions)
{
   FTNMESSAGE NewMessage = {NULL, NULL, NULL, NULL};
   FTNADDRESS NetReplyAddress = {0, 0, 0, 0};
   MSGHEADER NewHeader;
   AREADEFLIST *pactarea=NULL;
   int rc2=OK;
   PKLUDGE pKludge=NULL;
   PCHAR pchTemp;
   int j;

   if (!(pactarea=AM_FindArea(arealist, dest_tag)))
      return AREA_NOT_FOUND;

   if (!pactarea->areahandle)
      return AREA_OPEN_ERROR;

   /* MESSAGEINFO-Struktur erzeugen und Informationen kopieren
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   NewMessage.pchMessageText=strdup(pMessage->pchMessageText);
   while(pKludge = MSG_FindKludge(pMessage, KLUDGE_ANY, pKludge))
   {
      if ((pKludge->ulKludgeType == KLUDGE_REPLY) &&
          (pactarea->areadata.areatype == AREATYPE_NET))
         StringToNetAddr(pKludge->pchKludgeText, &NetReplyAddress, NULL);
      MSG_SetKludge(&NewMessage, pKludge->ulKludgeType, pKludge->pchKludgeText, SETKLUDGE_MULTIPLE);
   }

   /* auf CHRS: IBMPC 2 einstellen (wurde beim Lesen konvertiert) */
   if (ulCodePage == 437)
      MSG_SetKludge(pMessage, KLUDGE_CHRS, "IBMPC 2", SETKLUDGE_UNIQUE);
   else
      MSG_RemoveKludge(pMessage, KLUDGE_CHRS);

   /* Header-Informationen kopieren.
   ------------------------------------------------------------------ */
   NewHeader = *pHeader;
   NewHeader.ulReplyTo=0;
   NewHeader.ulMsgID=0;
   memset(NewHeader.ulReplies, 0, sizeof(NewHeader.ulReplies));

   if (ulOptions & COPYMOVE_RESEND)
   {
      NewHeader.ulAttrib &= ~(ATTRIB_SENT | ATTRIB_SCANNED);
      NewHeader.ulAttrib |= ATTRIB_LOCAL;
   }
   else
      NewHeader.ulAttrib |= ATTRIB_SENT | ATTRIB_SCANNED;

   if (pactarea->areadata.areatype != AREATYPE_NET)
   {
      /* Address-Matching */
      j = MSG_MatchAddress(&NewHeader.ToAddress, userdaten, &NewHeader.FromAddress);

      if (j>=0)
         StringToNetAddr(userdaten->address[j], &NewHeader.FromAddress, NULL);
   }
   else
      NewHeader.ToAddress = NetReplyAddress;

   /* Neu erzeugte Message abspeichern.
   ------------------------------------------------------------------ */

   /* Messagetext konvertieren */
   pchTemp = NewMessage.pchMessageText;
   while (*pchTemp)
      if (*pchTemp == '\n')
         *pchTemp++ = '\r';
      else
         pchTemp++;

   EnterSerial();
   switch (pactarea->areadata.areaformat)
   {
      case AREAFORMAT_SQUISH:
         rc2 = SQ_AddMessage(pactarea, &NewHeader, &NewMessage);
         break;

      case AREAFORMAT_FTS:
         rc2 = FTS_AddMessage(pactarea, &NewHeader, &NewMessage, pDriveRemap);
         break;

      case AREAFORMAT_JAM:
         rc2 = JAM_AddMessage(pactarea, &NewHeader, &NewMessage);
         if (!rc2)
         {
            /* ECHOMAIL.JAM, NETMAIL.JAM schreiben */
            if (generaloptions->jampath[0])
               AddToJamFile(pactarea, generaloptions->jampath, NewHeader.ulMsgID);
         }
         break;

      default:
         rc2 = ERROR;
         break;
   }
   ExitSerial();

   if (!rc2 && CopyCallback)
      CopyCallback(pactarea, NewHeader.ulMsgID, &NewHeader);

   /* Puffer freigeben.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   MSG_ClearMessage(&NewHeader, &NewMessage);

   return rc2;
}


/*ีอออออออออออออออออออออออออออ MSG_MoveMessage อออออออออออออออออออออออออออออออธ
  ณ Verschiebt die Message in eine andere Area                                ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_MoveMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist,
                    PCHAR tag, PCHAR dest_tag, ULONG msgnum, PDRIVEREMAP pDriveRemap,
                    USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                    MsgCallback CopyCallback, MsgCallback KillCallback, ULONG ulOptions)
{
   FTNMESSAGE NewMessage={NULL, NULL, NULL, NULL};
   FTNADDRESS NetReplyAddress = {0, 0, 0, 0};
   MSGHEADER NewHeader;
   AREADEFLIST *pactarea=NULL;
   int rc=OK, rc2=OK;
   PCHAR pchTemp;
   PKLUDGE pKludge=NULL;
   int j;

   if (!(pactarea=AM_FindArea(arealist, dest_tag)))
      return AREA_NOT_FOUND;

   if (!pactarea->areahandle)
      return AREA_OPEN_ERROR;

   if (!stricmp(tag, dest_tag))
      return OK;

   /* MESSAGEINFO-Struktur erzeugen und Informationen kopieren
   ------------------------------------------------------------------ */
   NewMessage.pchMessageText=strdup(pMessage->pchMessageText);
   while(pKludge = MSG_FindKludge(pMessage, KLUDGE_ANY, pKludge))
   {
      if ((pKludge->ulKludgeType == KLUDGE_REPLY) &&
          (pactarea->areadata.areatype == AREATYPE_NET))
         StringToNetAddr(pKludge->pchKludgeText, &NetReplyAddress, NULL);

      MSG_SetKludge(&NewMessage, pKludge->ulKludgeType, pKludge->pchKludgeText, SETKLUDGE_MULTIPLE);
   }

   /* auf CHRS: IBMPC 2 einstellen (wurde beim Lesen konvertiert) */
   if (ulCodePage == 437)
      MSG_SetKludge(pMessage, KLUDGE_CHRS, "IBMPC 2", SETKLUDGE_UNIQUE);
   else
      MSG_RemoveKludge(pMessage, KLUDGE_CHRS);

   /* Header-Informationen kopieren.
   ------------------------------------------------------------------ */
   NewHeader = *pHeader;
   NewHeader.ulReplyTo=0;
   NewHeader.ulMsgID=0;
   memset(NewHeader.ulReplies, 0, sizeof(NewHeader.ulReplies));

   if (ulOptions & COPYMOVE_RESEND)
   {
      NewHeader.ulAttrib &= ~(ATTRIB_SENT | ATTRIB_SCANNED);
      NewHeader.ulAttrib |= ATTRIB_LOCAL;
   }
   else
      NewHeader.ulAttrib |= ATTRIB_SENT | ATTRIB_SCANNED;

   if (pactarea->areadata.areatype != AREATYPE_NET)
   {
      /* Address-Matching */
      j = MSG_MatchAddress(&NewHeader.ToAddress, userdaten, &NewHeader.FromAddress);

      if (j>=0)
         StringToNetAddr(userdaten->address[j], &NewHeader.FromAddress, NULL);
   }
   else
      NewHeader.ToAddress = NetReplyAddress;

   /* Neu erzeugte Message abspeichern.
   ------------------------------------------------------------------ */

   /* Messagetext konvertieren */
   pchTemp = NewMessage.pchMessageText;
   while (*pchTemp)
      if (*pchTemp == '\n')
         *pchTemp++ = '\r';
      else
         pchTemp++;

   EnterSerial();
   switch (pactarea->areadata.areaformat)
   {
      case AREAFORMAT_SQUISH:
         rc2 = SQ_AddMessage(pactarea, &NewHeader, &NewMessage);
         break;

      case AREAFORMAT_FTS:
         rc2 = FTS_AddMessage(pactarea, &NewHeader, &NewMessage, pDriveRemap);
         break;

      case AREAFORMAT_JAM:
         rc2 = JAM_AddMessage(pactarea, &NewHeader, &NewMessage);
         if (!rc2)
         {
            /* ECHOMAIL.JAM, NETMAIL.JAM schreiben */
            if (generaloptions->jampath[0])
               AddToJamFile(pactarea, generaloptions->jampath, NewHeader.ulMsgID);
         }
         break;

      default:
         rc2 = ERROR;
         break;
   }
   ExitSerial();

   /* Alte Message loeschen
   ------------------------------------------------------------------ */
   if (!rc2)
   {
      if (CopyCallback)
         CopyCallback(pactarea, NewHeader.ulMsgID, &NewHeader);

      if ((rc=MSG_KillMessage(arealist, tag, msgnum, pDriveRemap, KillCallback)) &&
          rc!=NO_MESSAGE)
      {
         MSG_ClearMessage(&NewHeader, &NewMessage);
         return rc;
      }
   }

   /* Puffer freigeben.
   ------------------------------------------------------------------ */
   MSG_ClearMessage(&NewHeader, &NewMessage);

   if (rc2)
      return rc2;
   else
      return OK;
}

/*ีอออออออออออออออออออออออออออ MSG_ForwardMessage ออออออออออออออออออออออออออออธ
  ณ Weiterleiten einer Message in eine andere Area. Dabei einen Kopf anhngen.ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_ForwardMessage(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                       PAREALIST arealist, PCHAR tag, PCHAR dest_tag, char *pchCurrentName,
                       BOOL bGenFwdSubj)
{
   AREADEFLIST *pactarea, *porigarea;
   int i;
   char *kludgepos;
   char *pchTemp;
   char *pchOldText;
   FTNADDRESS MyAddr;
   PMSGTEMPLATE pTemplate;
   char pchTempAd[LEN_5DADDRESS+1];
   PKLUDGE pKludge=NULL;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, dest_tag);

   pactarea=AM_FindArea(arealist, dest_tag);
   porigarea=AM_FindArea(arealist, tag);

   if (!pactarea)
      return AREA_NOT_FOUND;

   StringToNetAddr(pactarea->areadata.address, &MyAddr, NULL);

   pchOldText=pMessage->pchMessageText;

   pMessage->pchMessageText=malloc(strlen(pMessage->pchMessageText)+4000);

   if (porigarea->areadata.areatype == AREATYPE_ECHO)
      pchTemp=TplForward(pTemplate, pMessage->pchMessageText, tag, pchCurrentName,
                         pHeader->pchFromName, pHeader->pchToName,
                         &pHeader->StampWritten, &pHeader->FromAddress, NULL,
                         pHeader->pchSubject, porigarea->areadata.areadesc, &MyAddr);
   else
      pchTemp=TplForward(pTemplate, pMessage->pchMessageText, tag, pchCurrentName,
                         pHeader->pchFromName, pHeader->pchToName,
                         &pHeader->StampWritten, &pHeader->FromAddress, &pHeader->ToAddress,
                         pHeader->pchSubject, porigarea->areadata.areadesc, &MyAddr);
   strcat(pMessage->pchMessageText, pchOldText);
   free(pchOldText);
   while(*pchTemp)
      pchTemp++;
   pchTemp=TplForwardFooter(pTemplate, pchTemp, tag, pchCurrentName, pHeader->pchFromName, pHeader->pchToName);

   pMessage->pchSeenPath=NULL;

   pKludge = MSG_FindKludge(pMessage, KLUDGE_FWDORIG, NULL);

   if (!pKludge)
   {
      /* neuer Forward */
      char *pchOldMsgid=NULL;

      pKludge = MSG_FindKludge(pMessage, KLUDGE_MSGID, NULL);
      if (pKludge)
         pchOldMsgid = strdup(pKludge->pchKludgeText);

      MSG_RemoveKludge(pMessage, KLUDGE_ALL);

      /* alten Header sichern */
      if (pHeader->pchFromName[0])
         MSG_SetKludge(pMessage, KLUDGE_FWDFROM, pHeader->pchFromName, SETKLUDGE_UNIQUE);
      if (pHeader->pchToName[0])
         MSG_SetKludge(pMessage, KLUDGE_FWDTO, pHeader->pchToName, SETKLUDGE_UNIQUE);
      if (bGenFwdSubj && pHeader->pchSubject[0])
         MSG_SetKludge(pMessage, KLUDGE_FWDSUBJ, pHeader->pchSubject, SETKLUDGE_UNIQUE);
      if (pchOldMsgid)
      {
         MSG_SetKludge(pMessage, KLUDGE_FWDMSGID, pchOldMsgid, SETKLUDGE_UNIQUE);
         free(pchOldMsgid);
      }
      NetAddrToString(pchTempAd, &pHeader->FromAddress);
      MSG_SetKludge(pMessage, KLUDGE_FWDORIG, pchTempAd, SETKLUDGE_UNIQUE);
      if (porigarea->areadata.areatype == AREATYPE_ECHO)
         MSG_SetKludge(pMessage, KLUDGE_FWDAREA, porigarea->areadata.areatag, SETKLUDGE_UNIQUE);
      else
         if (porigarea->areadata.areatype == AREATYPE_NET)
         {
            NetAddrToString(pchTempAd, &pHeader->ToAddress);
            MSG_SetKludge(pMessage, KLUDGE_FWDDEST, pchTempAd, SETKLUDGE_UNIQUE);
         }
   }
   else
   {
      /* Forward von Forward */
      int i;

      for (i = KLUDGE_FMPT; i < KLUDGE_FWDFROM; i++)
         MSG_RemoveKludge(pMessage, i);
   }

   /* Header-Informationen kopieren.
   ------------------------------------------------------------------ */
   if (pactarea->areadata.username[0])
      strcpy(pHeader->pchFromName, pactarea->areadata.username);
   else
      strcpy(pHeader->pchFromName, pchCurrentName);
   pHeader->pchToName[0]=0;

   StringToNetAddr(pactarea->areadata.address, &pHeader->FromAddress, NULL);
   memset(&pHeader->ToAddress, 0, sizeof(pHeader->ToAddress));

   pHeader->ulReplyTo = 0;
   pHeader->ulMsgID = 0;
   for (i = 0; i < NUM_REPLIES ; i++)
      pHeader->ulReplies[i] = 0;

   /* TimeStamp fllen.
   ------------------------------------------------------------------ */
   FillCurrentTime(&pHeader->StampWritten);
   pHeader->StampArrived = pHeader->StampWritten;

   /* Message-Attribute aus der Einstellung der Area nehmen.
      Zumindest muss das LOCAL-Bit gesetzt sein.
   ------------------------------------------------------------------ */
   pHeader->ulAttrib =  ATTRIB_LOCAL | ATTRIB_READ | pactarea->areadata.ulDefAttrib;

   /* Tearline und Origin ungueltig machen
   ------------------------------------------------------------------ */
   kludgepos = strstr(pMessage->pchMessageText, "\n--- ");
   if (!kludgepos)
      kludgepos = strstr(pMessage->pchMessageText, "\n---\n");
   if (kludgepos)
      kludgepos[2] = '!';
   kludgepos = strstr(pMessage->pchMessageText, "\n * Origin:");
   if (kludgepos)
      kludgepos[2] = '!';


   return 0;
}

int MSG_ForwardMessageStep2(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                            PAREALIST arealist, PCHAR dest_tag,
                            char *pchCurrentName, LONG *iptInitialPos)
{
   AREADEFLIST *pactarea;
   char *pchTemp;
   char *pchOldText;
   FTNADDRESS MyAddr;
   PMSGTEMPLATE pTemplate;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, dest_tag);

   pactarea=AM_FindArea(arealist, dest_tag);

   if (!pactarea)
      return AREA_NOT_FOUND;

   StringToNetAddr(pactarea->areadata.address, &MyAddr, NULL);

   pchOldText=pMessage->pchMessageText;

   pMessage->pchMessageText=malloc(strlen(pMessage->pchMessageText)+4000);
   pMessage->pchMessageText[0]='\0';

   if (pTemplate->forwardfirst)
   {
      strcpy(pMessage->pchMessageText, pchOldText);
      free(pchOldText);
      pchTemp=pMessage->pchMessageText;
      while(*pchTemp)
         pchTemp++;
      pchTemp=TplHeader(pTemplate, pchTemp, pHeader->pchToName);
      *iptInitialPos=strlen(pMessage->pchMessageText);
      pchTemp=TplFooter(pTemplate, pchTemp, pchCurrentName);
   }
   else
   {
      pchTemp=TplHeader(pTemplate, pMessage->pchMessageText, pHeader->pchToName);
      *iptInitialPos=strlen(pMessage->pchMessageText);
      *pchTemp++='\n';
      *pchTemp++='\n';
      *pchTemp='\0';
      strcat(pMessage->pchMessageText, pchOldText);
      free(pchOldText);
      while(*pchTemp)
         pchTemp++;
      pchTemp=TplFooter(pTemplate, pchTemp, pchCurrentName);
   }

   return OK;
}


/*ีอออออออออออออออออออออออออออ MSG_KillMessage อออออออออออออออออออออออออออออออธ
  ณ L”scht die Message, deren Nummer bergeben wurde. Wird eine '0' bergeben,ณ
  ณ wird die aktuelle Message aus den Areainformationen gel”scht. In den Area-ณ
  ณ informationen wird gleichzeitig die Anzahl der Messages korrigiert.       ณ
  ณ Es wird dann gleich zur nchsten Message gesprungen, wenn die aktuelle    ณ
  ณ Message gel”scht wurde, um nicht extra nochmal blttern zu mssen.        ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_KillMessage(PAREALIST arealist, PCHAR tag, int msgnum, PDRIVEREMAP pdriveremap,
                    MsgCallback KillCallback)
{
   AREADEFLIST *pactarea;
   int rc;
   ULONG ulMsgID;

   pactarea = AM_FindArea(arealist, tag);

   if (msgnum == 0)
      msgnum = pactarea->currentmessage;

   if (!msgnum)
      return NO_MESSAGE;

   ulMsgID = MSG_MsgnToUid(arealist, tag, msgnum);

   /* Links der Message aufbrechen */
   EnterSerial();
   switch (pactarea->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         FTS_UnlinkMessage(pactarea, msgnum, pdriveremap);
         break;

      case AREAFORMAT_SQUISH:
         SQ_UnlinkMessage(pactarea, msgnum);
         break;

      case AREAFORMAT_JAM:
         JAM_UnlinkMessage(pactarea, msgnum);
         break;

   } /* endswitch */

   /* Message loeschen */

   switch (pactarea->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         rc = FTS_KillMessage(pactarea, msgnum);
         break;

      case AREAFORMAT_SQUISH:
         rc = SQ_KillMessage(pactarea, msgnum);
         break;

      case AREAFORMAT_JAM:
         rc = JAM_KillMessage(pactarea, msgnum);
         break;

      default:
         rc = ERROR;
         break;
   }
   ExitSerial();

   if (!rc)
      if (KillCallback)
         KillCallback(pactarea, ulMsgID, NULL);

   return rc;
}


/*ีอออออออออออออออออออออออออออ MSG_QuoteMessage ออออออออออออออออออออออออออออออธ
  ณ Die Message im der bergebenen Messageinfostruktur wird anhand des Para-  ณ
  ณ meters 'left' formatiert und gequotet. Die Control-Infos werden geloescht ณ
  ณ bis auf den MSGID-Kludge, um daraus spaeter einen REPLY zu machen.        ณ
  ณ                                                                           ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_QuoteMessage(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist,
                     PCHAR tag, PCHAR desttag, ULONG ulFlags, ULONG ulDest,
                     char *pchCurrentName, char *pchCurrentAddress, LONG *iptInitialPos)
{
   char        *kludgepos, *pchTextBuff, *pchTemplBuff, *pchHelp;
   char        pchLetters[MAX_INITIALS+3+1];   /* Fr den '> ' */
   AREADEFLIST *porigarea, *pdestarea;

   char *pchNewLine=NULL, *pchLastSpace=NULL, *pchDest=NULL, *pchSrc=NULL,
        *pchLastSpaceSrc=NULL, *pchNewLineSrc=NULL, *pchLineEndSrc=NULL;
   char pchPrevPrefix[80]="";
   char pchNewPrefix[80]="";
   ULONG origattr;
   PMSGTEMPLATE pTemplate;
   PKLUDGE pKludge;
   PKLUDGE pOrigKludge=NULL, pOrigAdKludge=NULL, pOrigToKludge=NULL;
   PKLUDGE pOrigSubj=NULL, pOrigArea=NULL, pOrigDest=NULL;
   ULONG ulAllocLen;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, desttag);


   porigarea = AM_FindArea(arealist, tag);
   pdestarea = AM_FindArea(arealist, desttag);

   /* Wie bei neu erzeugten Messages erstmal Puffer anfordern.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   if (pMessage->pchMessageText &&
       pMessage->pchMessageText[0])
   {
      ulAllocLen = 2 * strlen(pMessage->pchMessageText);

      /* zustzlich f. kurze Zeilen: */
      pchSrc = pMessage->pchMessageText;
      while (*pchSrc)
      {
         if (*pchSrc == '\n')
            ulAllocLen += 5 + MAX_INITIALS;
         pchSrc++;
      }
   }
   else
      ulAllocLen = 2;

   if (!(pchTextBuff   = malloc(ulAllocLen)))
      return OUT_OF_MEMORY;
   pchTextBuff[0]      = '\0';

   if (ulDest == QUOTE_ORIG)
      pKludge = MSG_FindKludge(pMessage, KLUDGE_FWDMSGID, NULL);
   else
      pKludge = MSG_FindKludge(pMessage, KLUDGE_MSGID, NULL);

   if (pKludge)
   {
      MSG_SetKludge(pMessage, KLUDGE_REPLY, pKludge->pchKludgeText, SETKLUDGE_UNIQUE);
      MSG_RemoveKludge(pMessage, KLUDGE_MSGID);
   }
   if (pKludge = MSG_FindKludge(pMessage, KLUDGE_CISMSGID, NULL))
   {
      MSG_SetKludge(pMessage, KLUDGE_CISREPLY, pKludge->pchKludgeText, SETKLUDGE_UNIQUE);
      MSG_RemoveKludge(pMessage, KLUDGE_CISMSGID);
   }
   if (pKludge = MSG_FindKludge(pMessage, KLUDGE_CISFROM, NULL))
   {
      MSG_SetKludge(pMessage, KLUDGE_CISTO, pKludge->pchKludgeText, SETKLUDGE_UNIQUE);
      MSG_RemoveKludge(pMessage, KLUDGE_CISFROM);
   }

   /* Kein "To:" bei Echo-Reply */
   if (pdestarea->areadata.areatype == AREATYPE_ECHO)
      MSG_RemoveKludge(pMessage, KLUDGE_REPLYADDR);

   if (ulDest == QUOTE_ORIG)
   {
      pOrigKludge = MSG_FindKludge(pMessage, KLUDGE_FWDFROM, NULL);
      pOrigAdKludge = MSG_FindKludge(pMessage, KLUDGE_FWDORIG, NULL);
      pOrigToKludge = MSG_FindKludge(pMessage, KLUDGE_FWDTO, NULL);
      pOrigSubj = MSG_FindKludge(pMessage, KLUDGE_FWDSUBJ, NULL);
      pOrigArea = MSG_FindKludge(pMessage, KLUDGE_FWDAREA, NULL);
      pOrigDest = MSG_FindKludge(pMessage, KLUDGE_FWDDEST, NULL);
   }

   if (ulFlags & QUOTE_TEXT)
   {
      /* Initialen zum Quoten suchen und festhalten
      ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
      M_GetInitials(pOrigKludge?pOrigKludge->pchKludgeText:pHeader->pchFromName,
                    pchLetters, pTemplate->useinitials, pTemplate->chQuoteChar);

      /* Im zu quotenden Text die Tearline und die Origin-Line
         unkenntlich machen.
      ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
      kludgepos = strstr(pMessage->pchMessageText, "\n--- ");
      if (!kludgepos)
         kludgepos= strstr(pMessage->pchMessageText, "\n---\n");
      if (kludgepos)
         kludgepos[2] = '!';
      kludgepos = strstr(pMessage->pchMessageText, "\n * Origin");
      if (kludgepos)
         kludgepos[2] = '!';

      /* Text in Zeilen mit maximaler Lnge 'left' aufteilen.
         Dabei mssen Hard-CR's gesetzt werden. Dazu erstmal einen
         Textpuffer anfordern..kopieren..den alten Text freigeben
         und neuen Textbereich anfordern und zurckkopieren.
      ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */

      /* erstes Prefix vorbereiten */
      pchSrc=M_ConstructPrefix(pMessage->pchMessageText, pchNewPrefix, pTemplate->chQuoteChar);

      strcpy(pchPrevPrefix, pchNewPrefix);

      pchDest=pchTextBuff;
      pchHelp=pchNewPrefix[0]? pchNewPrefix : pchLetters;
      while(*pchHelp)
         *pchDest++=*pchHelp++;
      *pchDest++=' ';
      pchNewLine=pchTextBuff;
      pchNewLineSrc=pchSrc;
      pchLineEndSrc=pchSrc;

      /* Messagetext durchgehen */
      while(*pchSrc && (pchDest-pchTextBuff) < (ulAllocLen-10))
         switch(*pchSrc)
         {
            case '\n':
               pchSrc++;

               if (*pchSrc == '\n')
               {
                  /* Absatz */
                  /* Leerzeilen nicht quoten */
                  while(*pchSrc && (*pchSrc=='\n'))
                  {
                     *pchDest++=*pchSrc++;
                     pchNewLineSrc=pchSrc;
                  }

                  /* alten Quote suchen */
                  pchLineEndSrc=pchSrc;
                  pchSrc=M_ConstructPrefix(pchSrc, pchNewPrefix, pTemplate->chQuoteChar);

                  strcpy(pchPrevPrefix, pchNewPrefix);
                  *pchDest++='\n';
                  pchNewLine=pchDest;

                  /* Prefix kopieren */
                  pchHelp=pchNewPrefix[0]?pchNewPrefix:pchLetters;
                  while (*pchHelp)
                     *pchDest++ = *pchHelp++;

                  *pchDest++=' ';
                  pchNewLineSrc=pchSrc;
               }
               else
               {
                  pchLineEndSrc=pchSrc;
                  pchSrc=M_ConstructPrefix(pchSrc, pchNewPrefix, pTemplate->chQuoteChar);
                  pchLastSpaceSrc=pchSrc;

                  if (pchPrevPrefix[0] && pchNewPrefix[0] &&   /* beide Zeilen Quotes */
                      !strcmp(pchNewPrefix, pchPrevPrefix) &&  /* vom gleichen Abs. */
                      (pchLineEndSrc-pchNewLineSrc) > pTemplate->joinlen &&
                      !(ulFlags & QUOTE_NOJOIN))
                  {
                     pchLastSpace=pchDest;
                     *pchDest++=' ';
                  }
                  else
                  {
                     if (!pchPrevPrefix[0] && !pchNewPrefix[0] &&   /* beide Zeilen kein Quote */
                         (pchLineEndSrc-pchNewLineSrc) > pTemplate->joinlen &&
                         !(ulFlags & QUOTE_NOJOIN))
                     {
                        pchLastSpace=pchDest;
                        *pchDest++=' ';
                     }
                     else
                     {
                        /* alle anderen Faelle */
                        strcpy(pchPrevPrefix, pchNewPrefix);
                        *pchDest++='\n';
                        pchNewLine=pchDest;

                        /* Prefix kopieren */
                        pchHelp=pchNewPrefix[0] ? pchNewPrefix : pchLetters;
                        while (*pchHelp)
                           *pchDest++ = *pchHelp++;

                        pchLastSpace=pchDest;
                        *pchDest++=' ';
                        pchNewLineSrc=pchSrc;
                     }
                  }
               }
               break;

            case ' ':
               /* Zeiger auf letztes Leerzeichen setzen */
               pchLastSpaceSrc=pchSrc;
               pchLastSpace=pchDest;
               *pchDest++=*pchSrc++;
               break;

            default:
               if ((pchDest-pchNewLine+1)> pTemplate->quotelinelen)
               {
                  /* Zeile zu lang */
                  if (pchLastSpace && (pchLastSpace > pchNewLine))
                  {
                     /* Beim letzten Leerzeichen neu aufsetzen */
                     pchNewLine=pchLastSpace;
                     pchDest=pchNewLine;
                     pchSrc=pchLastSpaceSrc;
                  }
                  else
                     pchNewLine=pchDest+1;
                  *pchDest++='\n';

                  /* Prefix kopieren */
                  pchHelp=(pchNewPrefix[0]) ? pchNewPrefix : pchLetters;
                  while (*pchHelp)
                     *pchDest++=*pchHelp++;
                  *pchDest++=' ';

                  /* Leerzeichen nach dem Zeilenwechsel ueberlesen */
                  if (*pchSrc && *pchSrc==' ')
                     pchSrc++;
               }
               *pchDest++=*pchSrc++;
               break;
         }
      *pchDest='\0';
   }

   /* Message-Template einsetzen
   ------------------------------------------------------------------ */
   pchTemplBuff=malloc(strlen(pchTextBuff)+5000);
   pchTemplBuff[0]='\0';
   pchHelp=pchTemplBuff;

   if (ulFlags & QUOTE_TEXT)
   {
      FTNADDRESS FwdOrig, FwdDest;

      if (pOrigAdKludge)
         StringToNetAddr(pOrigAdKludge->pchKludgeText, &FwdOrig, NULL);
      if (pOrigDest)
         StringToNetAddr(pOrigDest->pchKludgeText, &FwdDest, NULL);

      pchHelp=TplReply(pTemplate, pchHelp,
                       pOrigKludge?pOrigKludge->pchKludgeText:pHeader->pchFromName,
                       pOrigToKludge?pOrigToKludge->pchKludgeText:pHeader->pchToName,
                       (ulDest == QUOTE_TO)?pHeader->pchToName:pHeader->pchFromName,
                       pOrigArea?pOrigArea->pchKludgeText:tag,
                       &pHeader->StampWritten,
                       pOrigAdKludge?&FwdOrig:&pHeader->FromAddress,
                       pOrigDest?&FwdDest:&pHeader->ToAddress,
                       pOrigSubj?pOrigSubj->pchKludgeText:pHeader->pchSubject,
                       porigarea->areadata.areadesc);
   }

   *iptInitialPos=strlen(pchTemplBuff);
   strcpy(pchHelp, pchTextBuff);
   free(pchTextBuff);
   pchTextBuff=pchTemplBuff;

   free(pMessage->pchMessageText);          /* der SEENPATH wird mit freigegeben */
   pMessage->pchMessageText = pchTextBuff;
   pMessage->pchSeenPath=NULL;

   /* Message-Header neu erzeugen: Adressen austauschen und neue
      From-Adresse einsetzen.
   ------------------------------------------------------------------ */
   if (ulDest != QUOTE_TO)
   {
      if (pOrigKludge)
         strcpy(pHeader->pchToName, pOrigKludge->pchKludgeText);
      else
         strcpy(pHeader->pchToName, pHeader->pchFromName);

      if (pOrigAdKludge)
         StringToNetAddr(pOrigAdKludge->pchKludgeText, &pHeader->ToAddress, NULL);
      else
         pHeader->ToAddress = pHeader->FromAddress;
   }
   if (pdestarea->areadata.areatype != AREATYPE_NET)
      memset(&pHeader->ToAddress, 0, sizeof(FTNADDRESS));

   if ((pKludge = MSG_FindKludge(pMessage, KLUDGE_REPLYTO, NULL)) && pdestarea->areadata.areatype != AREATYPE_ECHO)
   {
      char *pchRep=pKludge->pchKludgeText;

      while (*pchRep && *pchRep != ' ')
         pchRep++;
      *pchRep='\0';
      StringToNetAddr(pKludge->pchKludgeText, &pHeader->ToAddress, NULL);
      if (pchRep != pKludge->pchKludgeText)
      {
         pchRep++;
         while (*pchRep && *pchRep==' ')
            pchRep++;
      }
      strcpy(pHeader->pchToName, pchRep);

      /* Kludge loeschen */
      MSG_RemoveKludge(pMessage, KLUDGE_REPLYTO);
   }

   if (ulDest == QUOTE_ORIG && pOrigSubj)
      strncpy(pHeader->pchSubject, pOrigSubj->pchKludgeText, LEN_SUBJECT);

   MSG_RemoveKludge(pMessage, KLUDGE_FWDFROM);
   MSG_RemoveKludge(pMessage, KLUDGE_FWDTO);
   MSG_RemoveKludge(pMessage, KLUDGE_FWDORIG);
   MSG_RemoveKludge(pMessage, KLUDGE_FWDDEST);
   MSG_RemoveKludge(pMessage, KLUDGE_FWDSUBJ);
   MSG_RemoveKludge(pMessage, KLUDGE_FWDAREA);
   MSG_RemoveKludge(pMessage, KLUDGE_FWDMSGID);

   if (ulFlags & QUOTE_STRIPRE)
      /* "Re:" im Subject loeschen */
      StripRe(pHeader->pchSubject);

   /* Default-Header-Informationen passend zur Area einfgen.
   ------------------------------------------------------------------ */
   origattr = pHeader->ulAttrib;
   if (pdestarea != porigarea &&
       pdestarea->areadata.areatype != AREATYPE_NET)
      M_CopyStdHeader(pHeader, pdestarea, pchCurrentName, pdestarea->areadata.address);
   else
      M_CopyStdHeader(pHeader, pdestarea, pchCurrentName, pchCurrentAddress);

   if (origattr & ATTRIB_RRQ)
      pHeader->ulAttrib |= ATTRIB_RECEIPT;
   if (origattr & ATTRIB_PRIVATE)
      pHeader->ulAttrib |= ATTRIB_PRIVATE;

   return OK;
}

int MSG_QuoteMessageStep2(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                          PAREALIST arealist, PCHAR tag, PCHAR desttag,
                          char *pchCurrentName, LONG *iptInitialPos)
{
   char *pchHelp;
   char *pchTemplBuff;
   AREADEFLIST *porigarea;
   PMSGTEMPLATE pTemplate;
   PKLUDGE pKludge;
   BOOL diffarea = stricmp(tag, desttag);

   pTemplate = M_FindTemplate(ptemplatelist, arealist, desttag);

   porigarea = AM_FindArea(arealist, tag);

   /* Message-Template einsetzen
   ------------------------------------------------------------------ */
   pchTemplBuff=malloc(strlen(pMessage->pchMessageText)+5000);
   pchHelp=pchTemplBuff;

   if (pKludge = MSG_FindKludge(pMessage, KLUDGE_REPLYADDR, NULL))
   {
      sprintf(pchHelp, "To: %s\n\n", pKludge->pchKludgeText);
      while(*pchHelp)
         pchHelp++;
      MSG_RemoveKludge(pMessage, KLUDGE_REPLYADDR);
   }

   if (diffarea)
      pchHelp=TplReplyOther(pTemplate, pchHelp, tag, porigarea->areadata.areadesc);
   pchHelp=TplHeader(pTemplate, pchHelp, pHeader->pchToName);

   *iptInitialPos+=strlen(pchTemplBuff);
   strcpy(pchHelp, pMessage->pchMessageText);
   while (*pchHelp)
      pchHelp++;
   *pchHelp++='\n';
   TplFooter(pTemplate, pchHelp, pchCurrentName);
   free(pMessage->pchMessageText);
   pMessage->pchMessageText=pchTemplBuff;

   return OK;
}

/*ีอออออออออออออออออออออออออออ M_ConstructPrefix อออออออออออออออออออออออออออออธ
  ณ Holt sich aus dem Messagetext ein Prefix und Quotet es                    ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

static char *M_ConstructPrefix(char *pchStart, char *pchPrefixBuff, char chQuote)
{
   char *pchHelp=pchStart;
   char *pchPrefDest=pchPrefixBuff;
   char *pchLastQuote=NULL;
   int i=0, count_quotes=0;
   int realchars=0;

   /* Quote-Zeichen suchen */
   while(*pchHelp && (pchHelp-pchStart)<10 &&
         *pchHelp != chQuote &&
         (isalpha(*pchHelp) || *pchHelp == ' ' || strchr(UMLAUTE, *pchHelp)))
   {
      if (*pchHelp != ' ')
         realchars++;
      pchHelp++;
   }

   if (*pchHelp != chQuote ||
       realchars > 4 ||
       (pchHelp > pchStart &&
        (*(pchHelp-1) == '-' ||
         *(pchHelp-1) == '=')))
   {
      /* nix gefunden, default */
      pchPrefixBuff[0]='\0';
      return pchStart;
   }
   else
   {
      /* gefunden */
      pchLastQuote=pchHelp;
      count_quotes=1;
      pchHelp++;

      /* alle alten Quotes abklappern */
      while(*pchHelp && (pchHelp-pchStart)<80 && i<5 &&
            *pchHelp != '\n')
      {
         if (*pchHelp == chQuote)
         {
            count_quotes++;
            pchLastQuote=pchHelp;
            i=0;
         }
         if (*pchHelp != ' ' && *pchHelp != chQuote)
            i++;
         pchHelp++;
      }
      pchHelp=pchLastQuote;   /* zum letzten gefundenen */
      while (pchHelp > pchStart &&
             *pchHelp==chQuote)
          pchHelp--;
      /* Initialen kopieren */
      i=0;
      while(pchHelp>pchStart &&
             *pchHelp !=' ' && *pchHelp !='\n' && i<3)
      {
         pchHelp--;
         i++;
      }
      if (*pchHelp == ' ' || *pchHelp == '\n')
         pchHelp++;

      *pchPrefDest++=' ';

      while(*pchHelp != chQuote)
         *pchPrefDest++ = *pchHelp++;

      /* alle Quotes rekonstruieren */
      while(count_quotes > 0)
      {
         *pchPrefDest++=chQuote;
         count_quotes--;
      }
      /* und nochmal quoten */
      *pchPrefDest++=chQuote;
      /* terminieren */
      *pchPrefDest='\0';

      while(*pchHelp && *pchHelp == chQuote)
         pchHelp++;
      if(*pchHelp && *pchHelp == ' ')
         pchHelp++;

      return pchHelp;
   }
}


/*ีอออออออออออออออออออออออออออ MSG_RequestFiles ออออออออออออออออออออออออออออออธ
  ณ Erzeugt File-Requests fr 'files' bei 'address'.                          ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_RequestFiles(PAREALIST arealist, PCHAR tag,
                     PCHAR address, PCHAR name, PREQUESTLIST pFiles, ULONG ulAttrib,
                     char *pchCurrentName, char *pchCurrentAddress,
                     USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                     LONG lOffset, PDRIVEREMAP pDriveRemap, TEMPLATELIST *ptemplatelist,
                     MsgCallback ReqCallback)
{
   FTNMESSAGE  req_msg;
   MSGHEADER   req_hdr;
   FTNADDRESS  req_addr;
   char        filebuffer[LEN_REQFILE+1+LEN_PASSWORD+1];
   AREADEFLIST *pactarea;

   pactarea = AM_FindArea(arealist, tag);

   /* Netmail-Message(s) erzeugen und die Files in die Subject-Line(s)
      einsetzen. Als Messagetext nur eine leere Tearline
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   if (pactarea && pFiles)
   {
      StringToNetAddr(address, &req_addr, NULL);
      memset(&req_hdr, 0, sizeof(req_hdr));
      memset(&req_msg, 0, sizeof(req_msg));

      MSG_OpenArea(arealist, tag, lOffset, pDriveRemap);

      while (pFiles)   /* Zu lange Listen auf mehrere Msgs verteilen */
      {
         while (pFiles)  /* Subject zusammenstellen */
         {
            if (pFiles->pchPassword[0])
               sprintf(filebuffer, "%s !%s", pFiles->pchFileName, pFiles->pchPassword);
            else
               strcpy(filebuffer, pFiles->pchFileName);

            if (strlen(req_hdr.pchSubject)+strlen(filebuffer) < LEN_SUBJECT)
            {
               if (req_hdr.pchSubject[0])
                  strcat(req_hdr.pchSubject, " ");
               strcat(req_hdr.pchSubject, filebuffer);

               pFiles = pFiles->next;
            }
            else
               break;
         }

         M_CopyStdHeader(&req_hdr, pactarea, pchCurrentName, pchCurrentAddress);
         req_hdr.ToAddress = req_addr;
         strcpy(req_hdr.pchToName, name);
         req_hdr.ulAttrib = ulAttrib | ATTRIB_LOCAL;
         req_msg.pchMessageText = malloc(4);
         req_msg.pchMessageText[0]='\0';

         MSG_AddMessage(&req_msg, &req_hdr, arealist, tag, userdaten,
                        generaloptions, pDriveRemap, 0, ptemplatelist, 0,
                        ReqCallback);
         MSG_ClearMessage(&req_hdr, &req_msg);
      }
      MSG_CloseArea(arealist, tag, TRUE, lOffset, pDriveRemap);
   }
   else
      return AREA_NOT_FOUND;
   return OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MSG_BroadcastDelete                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt eine Message mit Broadcast-Delete-Kludge            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: AreaList: Liste der Areas                                      */
/*            AreaTag:  Area-Tag                                             */
/*            pHeader:  alter Header, wird upgedated                         */
/*            pMessage: alte Message, wird upgedated                         */
/*            userdaten: User-Optionen                                       */
/*            generaloptions: allgemeine Optionen                            */
/*            pchCurrentName: momentaner Name                                */
/*            pchCurrentAddress: momentane Adresse                           */
/*            pulNewID: Puffer f. UMSGID der neuen Message                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int MSG_BroadcastDelete(PAREALIST arealist, PCHAR AreaTag,
                        MSGHEADER *pHeader, FTNMESSAGE *pMessage,
                        USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                        char *pchCurrentName, char *pchCurrentAddress,
                        PDRIVEREMAP pDriveRemap, TEMPLATELIST *ptemplatelist,
                        MsgCallback BCDCallback)
{
   AREADEFLIST *pactarea;
   char pchKludgeBuffer[100]="DELETE ";
   PKLUDGE pKludge;
   char *pchSave;

   pactarea = AM_FindArea(arealist, AreaTag);

   /* ACUPDATE-Kludge vorbereiten */
   pKludge = MSG_FindKludge(pMessage, KLUDGE_MSGID, NULL);
   if (!pKludge)
      return ERROR;

   pchSave = strdup(pKludge->pchKludgeText);
   MSG_RemoveKludge(pMessage, KLUDGE_ALL);
   strcat(pchKludgeBuffer, pchSave);
   free(pchSave);
   MSG_SetKludge(pMessage, KLUDGE_ACUPDATE, pchKludgeBuffer, SETKLUDGE_UNIQUE);

   /* restlichen Header */
   M_CopyStdHeader(pHeader, pactarea, pchCurrentName, pchCurrentAddress);
   strcpy(pHeader->pchToName, "All");
   strcpy(pHeader->pchSubject, "Broadcast delete");
   memset(&(pHeader->ToAddress), 0, sizeof(FTNADDRESS));

   if (pMessage->pchMessageText)
      pMessage->pchMessageText[0] = '\0';

   pMessage->pchSeenPath=NULL;

   /* Kill/Sent setzen */
   pHeader->ulAttrib |= ATTRIB_KILLSENT;

   return MSG_AddMessage(pMessage, pHeader, arealist, AreaTag, userdaten,
                         generaloptions, pDriveRemap, 0, ptemplatelist, 0, BCDCallback);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MSG_BroadcastModify                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt eine Message mit Broadcast-Modify-Kludge            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: AreaList: Liste der Areas                                      */
/*            AreaTag:  Area-Tag                                             */
/*            pHeader:  alter Header, wird upgedated                         */
/*            pMessage: alte Message, wird upgedated                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int MSG_BroadcastModify(PMSGHEADER pHeader, PFTNMESSAGE pMessage)
{
   PKLUDGE pKludge = MSG_FindKludge(pMessage, KLUDGE_MSGID, NULL);

   /* ACUPDATE-Kludge vorbereiten */
   MSG_SetKludgeVar(pMessage, KLUDGE_ACUPDATE, SETKLUDGE_UNIQUE, "MODIFY %s", pKludge->pchKludgeText);

   /* sent/scanned zuruecksetzen, local setzen */
   pHeader->ulAttrib |= ATTRIB_LOCAL;
   pHeader->ulAttrib &= ~(ATTRIB_SENT | ATTRIB_SCANNED);

   return OK;
}

/*ีอออออออออออออออออออออออออออ MSG_UidToMsgn อออออออออออออออออออออออออออออออออธ
  ณ Wandelt UMSGID um in Message-Nummer.                                      ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

int MSG_UidToMsgn(PAREALIST arealist, PCHAR AreaTag, ULONG msgID, BOOL exact)
{
   AREADEFLIST *pactarea;
   int rc;

   if ((pactarea=AM_FindArea(arealist, AreaTag))==NULL)
      return 0;

   EnterSerial();
   switch(pactarea->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         rc = FTS_UidToMsgn(pactarea, msgID, exact);
         break;

      case AREAFORMAT_SQUISH:
         rc = SQ_UidToMsgn(pactarea, msgID, exact);
         break;

      case AREAFORMAT_JAM:
         rc = JAM_UidToMsgn(pactarea, msgID, exact);
         break;

      default:
         rc = 0;
         break;
   }
   ExitSerial();
   return rc;
}

/*ีอออออออออออออออออออออออออออ MSG_MsgnToUid อออออออออออออออออออออออออออออออออธ
  ณ Wandelt Message-Nummer in UMSGID um.                                      ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

ULONG MSG_MsgnToUid(PAREALIST arealist, PCHAR AreaTag, int msgn)
{
   AREADEFLIST *pactarea;
   ULONG rc;

   if ((pactarea=AM_FindArea(arealist, AreaTag))==NULL)
      return 0;

   EnterSerial();
   switch(pactarea->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         rc = FTS_MsgnToUid(pactarea, msgn);
         break;

      case AREAFORMAT_SQUISH:
         rc = SQ_MsgnToUid(pactarea, msgn);
         break;

      case AREAFORMAT_JAM:
         rc = JAM_MsgnToUid(pactarea, msgn);
         break;

      default:
         rc = 0;
         break;
   }
   ExitSerial();
   return rc;
}

/*ีอออออออออออออออออออออออออออ MSG_MarkRead ออออออออออออออออออออออออออออออออออธ
  ณ Markiert eine Message als gelesen. msgnum ist die Nummer der Message oder ณ
  ณ 0 fr die aktuelle Message. DIE MARKIERUNG ERFOLGT IM XX2-FELD (reserved) ณ
  ณ DES MESSAGEHEADERS !!!                                                    ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
int MSG_MarkRead(PAREALIST AreaList, PCHAR tag, int msgnum, char *pchName, PDRIVEREMAP pdriveremap)
{
   AREADEFLIST *pactarea;
   MSGHEADER Header;
   BOOL bPersonal;

   pactarea=AM_FindArea(AreaList, tag);
   if (!pactarea)
      return AREA_NOT_FOUND;

   if (pactarea->maxmessages==0)
      return NO_MESSAGE;

   if (msgnum==0)
      msgnum=pactarea->currentmessage;

   if (MSG_ReadHeader(&Header, AreaList, tag, msgnum))
      return MSG_READ_ERROR;

   bPersonal = !stricmp(Header.pchToName, pchName);

   EnterSerial();
   switch(pactarea->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         FTS_MarkRead(pactarea, msgnum, bPersonal, pdriveremap);
         break;

      case AREAFORMAT_SQUISH:
         SQ_MarkRead(pactarea, msgnum, bPersonal);
         break;

      case AREAFORMAT_JAM:
         JAM_MarkRead(pactarea, msgnum, bPersonal);
         break;

      default:
         break;
   }
   ExitSerial();

   return OK;
}


/*ีอออออออออออออออออออออออออออ StampToString อออออออออออออออออออออออออออออออออธ
  ณ Wandelt eine TimeStamp-Struktur in einen String um.                       ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
char * StampToString(PCHAR buffer, TIMESTAMP *timestamp)
{
   if (timestamp->month   < 1  ||   /* Validitaetspruefung */
       timestamp->month   > 12 ||
       timestamp->day     < 1  ||
       timestamp->hours   > 23 ||
       timestamp->minutes > 59 ||
       timestamp->seconds > 29)
      sprintf(buffer, "???");
   else
      sprintf(buffer,"%02d %s %04d  %02d:%02d:%02d",
              timestamp->day, months[timestamp->month-1],
              timestamp->year + 1980,
              timestamp->hours,
              timestamp->minutes, timestamp->seconds*2);
   return buffer;
}

static char * StampToString2(PCHAR buffer, TIMESTAMP *timestamp)
{
   if (timestamp->month < 1  ||   /* Validitaetspruefung */
       timestamp->month > 12 ||
       timestamp->day < 1  ||
       timestamp->hours > 23 ||
       timestamp->minutes > 59 ||
       timestamp->seconds > 29)
      sprintf(buffer, "???");
   else
      sprintf(buffer,"%02d %s %02d %02d:%02d:%02d",
              timestamp->day, months[timestamp->month-1],
              (timestamp->year>=20)?(timestamp->year-20):(timestamp->year+80),
              timestamp->hours,
              timestamp->minutes, timestamp->seconds*2);
   return buffer;
}

/*ีออออออออออออออออออออออออออออ M_CreateMsgID ออออออออออออออออออออออออออออออธ
  ณ                                                                         ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
static char *M_CreateMsgID(char *pchMsgIDBuff, FTNADDRESS *address)
{
   static ULONG ulHighID=0;
   ULONG        ulIDBuffer;
   char         pchAddressBuff[LEN_5DADDRESS]     = "";

   DosQuerySysInfo(QSV_TIME_LOW, QSV_TIME_LOW, &ulIDBuffer, sizeof(ULONG));
   if (ulIDBuffer <= ulHighID)
      ulIDBuffer = ++ulHighID;
   ulHighID=ulIDBuffer;

   NetAddrToString(pchAddressBuff, address);
   sprintf(pchMsgIDBuff, "%s %08x", pchAddressBuff, ulIDBuffer);

   return pchMsgIDBuff;
}


/*ีออออออออออออออออออออออออออออ M_GetInitials ออออออออออออออออออออออออออออออธ
  ณ Holt die Initialen zum Quoten aus dem Absendernamen und fgt einen '>'  ณ
  ณ an. Gibt den Pointer auf die Initialen zurck.                          ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
static char  *M_GetInitials(PCHAR pchName, char *pchInitials, BOOL with_initials, CHAR chQuote)
{
   int i = 1;
   char *actstring;
   char buffer[LEN_USERNAME+1];

   if (with_initials)
   {
      strcpy(buffer, pchName);
      actstring      = strtok(buffer, " ");
      if (actstring)
      {
         pchInitials[0] = ' ';
         pchInitials[1] = *actstring;               /* Erster Buchstabe */
         while (i <= MAX_INITIALS &&
                (actstring = strtok(NULL, " ")) &&
                isalpha(*actstring))
         {
            pchInitials[i+1] = *actstring;
            i++;
         } /* endwhile */
         pchInitials[i+1] = '\0';
      }
      else
         strcpy(pchInitials, " ?");

      actstring = strchr(pchInitials, 0);
      *actstring++ = chQuote;
      *actstring = 0;
   }
   else
   {
      pchInitials[0]=' ';
      pchInitials[1]=chQuote;
      pchInitials[2]=0;
   }

   return pchInitials;
}


/*ีออออออออออออออออออออออออออออ M_CopyStdHeader ออออออออออออออออออออออออออออธ
  ณ M_CopyStdHeader kopiert die Standard-Definitionen aus den Area-Infos in ณ
  ณ den bergebenen Header. Der Name wird aus der globalen Struktur         ณ
  ณ USERDATAOPT bernommen. Die Adresse in 'orig' aus den Area-Infos...     ณ
  ณ                                                                         ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
static void  M_CopyStdHeader(MSGHEADER *pHeader, AREADEFLIST *pactarea,
                             char *pchCurrentName, char *pchCurrentAddress)
{
   int          i;

   /* Usernamen aus der momentanen Einstellung nehmen und Empfnger
      und Subject leeren.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   strcpy(pHeader->pchFromName, pchCurrentName);

   /* Replyto und replies auf 0 setzen
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   pHeader->ulReplyTo = 0;
   pHeader->ulMsgID = 0;
   for (i = 0; i < NUM_REPLIES ; i++)
      pHeader->ulReplies[i] = 0;

   /* Standard-Adresse entweder aus den Area-Definitionen nehmen oder
      aus der momentanen Adress-Einstellung
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   StringToNetAddr(pchCurrentAddress, &pHeader->FromAddress, NULL);

   /* TimeStamp fllen.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   FillCurrentTime(&pHeader->StampWritten);
   pHeader->StampArrived = pHeader->StampWritten;

   /* Message-Attribute aus der Einstellung der Area nehmen.
      Zumindest muss das LOCAL-Bit gesetzt sein.
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   pHeader->ulAttrib =  ATTRIB_LOCAL | ATTRIB_READ | pactarea->areadata.ulDefAttrib;

   return;
}


/*ีออออออออออออออออออออออออออออ M_ReadMessage  อออออออออออออออออออออออออออออธ
  ณ M_ReadMessage liest die durch currentmessage angegebene Message ein und ณ
  ณ stellt sie in der MESSAGEINFO-Struktur zur Verfgung.                   ณ
  ณ Die Message wird kurz mit MsgOpenMsg ge”ffnet und eingelesen, danach    ณ
  ณ wieder mit MsgCloseMsg geschlossen, um anderen Programmen einen Zugriff ณ
  ณ zu gewhren(???).                                                       ณ
  ณ                                                                         ณ
  ณ Beim Lesen aus Echo-Areas wird die Absenderadresse in der Origin-Line   ณ
  ณ und dann evtl. in der Msgid gesucht und in den Header eingetragen, damitณ
  ณ man es gleich beim Lesen berprfen kann.                               ณ
  ณ                                                                         ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
static int M_ReadMessage(PAREADEFLIST pAreaDef, PFTNMESSAGE pMessage, PMSGHEADER pHeader, ULONG nummsg)
{
   unsigned char *src, *dst, *pchLineEnd;
   BOOL    bRealChars;
   USHORT  ret;
   PKLUDGE pCHRSKludge;
   PCHAR ctloffset, ctloffset2;

   MSG_ClearMessage(pHeader, pMessage);

   EnterSerial();
   switch(pAreaDef->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         ret = FTS_ReadMessage(pAreaDef, pHeader, pMessage, nummsg);
         break;

      case AREAFORMAT_SQUISH:
         ret = SQ_ReadMessage(pAreaDef, pHeader, pMessage, nummsg);
         break;

      case AREAFORMAT_JAM:
         ret = JAM_ReadMessage(pAreaDef, pHeader, pMessage, nummsg);
         break;

      default:
         ExitSerial();
         return ERROR;
   }
   ExitSerial();

   if (ret)
      return ret;

   /* Im Text alle LF/Tabs alle ctrl-A durch linefeeds ersetzen
   ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
   pchLineEnd=src=dst=pMessage->pchMessageText;
   bRealChars=FALSE;

   while(*src)
      switch(*src)
      {
         /* Tab und Soft-CR */
         case '\t':
         case '\n':
         case '\x8D':
         case ' ':
            *dst++=' ';
            src++;
            break;

         /* New Line */
         case '\r':
            if (!bRealChars) /* in diesem Absatz nur Whitespace */
               dst = pchLineEnd; /* zuruecksetzen */
            *dst++='\n';
            pchLineEnd = dst;
            bRealChars=FALSE;
            if (*(++src)=='\n')
               src++;
            break;

         default:
            *dst++=*src++;
            bRealChars=TRUE;
            break;
      }
   if (!bRealChars) /* in diesem Absatz nur Whitespace */
      dst = pchLineEnd; /* zuruecksetzen */
   *dst='\0';

   if (pAreaDef->areadata.areatype != AREATYPE_NET)
   {
      if (!pMessage->pchSeenPath)
      {
         /* Die SEEN-BY und PATH-Zeilen suchen, dann in den ctltext
            verschieben und die Groessen anpassen. Aufpassen, ob berhaupt
            welche da sind.
         ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ */
         ctloffset = strstr(pMessage->pchMessageText, "\n * Origin: ");
         if (ctloffset)                        /* Erst ab dem Origin suchen */
            ctloffset2 = strstr(ctloffset, "\nSEEN-BY: ");
         else
            ctloffset2 = strstr(pMessage->pchMessageText, "\nSEEN-BY: ");

         if (ctloffset2)
         {
            *ctloffset2++      = '\0';
            pMessage->pchSeenPath = ctloffset2;        /* Ist ja schon alloziert */
         }
         else
            pMessage->pchSeenPath = NULL;
      }
   }
   else
      pMessage->pchSeenPath = NULL;

   if (pCHRSKludge = MSG_FindKludge(pMessage, KLUDGE_CHRS, NULL))
      ConvertFromCharset(pMessage->pchMessageText, pCHRSKludge->pchKludgeText);

   return OK;
}

/*ีออออออออออออออออออออออออออออ MSG_ReadSquishParams อออออออออออออออออออออออธ
  ณ MSG_ReadSquishParams liesst die drei Squish-Parameter aus einer         ณ
  ณ Squish-Messagebase.                                                     ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

int MSG_ReadSquishParams(PSQUISHPARAMS pSquishParams, PAREALIST arealist, PCHAR tag, PDRIVEREMAP pDriveRemap)
{
   AREADEFLIST *pactarea;

   if (!(pactarea=AM_FindArea(arealist, tag)))
      return AREA_NOT_FOUND;

   if (pactarea->areadata.areaformat != AREAFORMAT_SQUISH)
      return ERROR;

   return SQ_ReadSquishParams(pactarea, pSquishParams, pDriveRemap);
}

/*ีออออออออออออออออออออออออออออ MSG_WriteSquishParams ออออออออออออออออออออออธ
  ณ MSG_WriteSquishParams schreibt die drei Squish-Parameter in eine        ณ
  ณ Squish-Messagebase.                                                     ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

int MSG_WriteSquishParams(PSQUISHPARAMS pSquishParams, PAREALIST arealist, PCHAR tag, PDRIVEREMAP pDriveRemap)
{
   AREADEFLIST *pactarea;

   if (!(pactarea=AM_FindArea(arealist, tag)))
      return AREA_NOT_FOUND;

   if (pactarea->areadata.areaformat != AREAFORMAT_SQUISH)
      return ERROR;

   return SQ_WriteSquishParams(pactarea, pSquishParams, pDriveRemap);
}

/*ีออออออออออออออออออออออออออออ MSG_RenumberArea อออออออออออออออออออออออออออธ
  ณ MSG_RenumberArea nummeriert eine *.MSG-Area neu, so dass die Messages   ณ
  ณ wieder mit 1 anfangen und kontinuierlich nummeriert sind.               ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

int MSG_RenumberArea(PAREALIST arealist, PCHAR tag, HWND hwndProgress, LONG lOffset, PDRIVEREMAP pDriveRemap)
{
   AREADEFLIST *pactarea=NULL;

   pactarea=AM_FindArea(arealist, tag);

   if (!pactarea)
      return AREA_NOT_FOUND;

   switch(pactarea->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         if (!MSG_OpenArea(arealist, tag, lOffset, pDriveRemap))
         {
            FTS_RenumberArea(pactarea, hwndProgress, pDriveRemap);
            MSG_CloseArea(arealist, tag, TRUE, lOffset, pDriveRemap);
         }
         break;

      default:
         return ERROR;
   }

   return OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: M_AddTearline                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt Tearline und Origin zum Messagetext hinzu, falls      */
/*               dies noetig ist                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: msgtext: Zeiger auf den Message-Text                           */
/*            isecho: Message ist Echomail-Message                           */
/*            addtear: Tearline soll hinzugefuegt werden                     */
/*            addorigin: Origin-Line soll hinzugefuegt werden                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE:  PID trotzdem erzeugen.                              */
/*                FALSE: Alles OK                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL M_AddTearline(PCHAR msgtext, BOOL isecho, BOOL addtear, BOOL addorigin,
                          BOOL usepid, PCHAR pchOrigin, FTNADDRESS *pAddr)
{
   char pchLineBuff[80];
   BOOL bReturn=FALSE;
   char pchTearBuff[80]="";
   char *pchFound;

   if (isecho || ((addtear || addorigin) && strlen(msgtext)>2))
   {
      /* Tearline vorbereiten */
      strcpy(pchTearBuff, TEARLINE);

      if (!(pchFound=strstr(msgtext, "\n---\n")) && !(pchFound=strstr(msgtext, "\n--- ")))
      {
         if (isecho || addtear)
            if (usepid)
               strcat(msgtext, TEARLINE_S);
            else
               strcat(msgtext, pchTearBuff);
      }
      else
      {
         PCHAR pchTemp=msgtext;

         if (strncmp(pchFound+1, pchTearBuff, strlen(pchTearBuff)))
            bReturn=TRUE;

         while (*pchTemp)
            pchTemp++;
         pchTemp--;
         while (pchTemp > msgtext && *pchTemp == '\n')
            *pchTemp--='\0';
         if (pchTemp >= msgtext)
            strcpy(++pchTemp, "\n");
      }
      if (!strstr(msgtext, "\n * Origin: "))
      {
         if (isecho || addorigin)
         {
            ULONG ulMaxLen;

            NetAddrToString(pchLineBuff, pAddr);
            ulMaxLen = 65-strlen(pchLineBuff);

            strcat(msgtext, ORIGINLINE);
            strncat(msgtext, pchOrigin, ulMaxLen);
            strcat(msgtext, " (");
            strcat(msgtext, pchLineBuff);
            strcat(msgtext, ")\n");
         }
      }
   }
   return bReturn;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MSG_QueryHomeMsg                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ermittelt die Messagenummer der Home-Message (Message des   */
/*               letzten Squish-Laufs)                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: arealist: Arealiste                                            */
/*            tag: Areatag                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: 0  Fehler                                                  */
/*                sonst Message-Nummer                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG MSG_QueryHomeMsg(PAREALIST arealist, PCHAR tag)
{
   AREADEFLIST *zeiger;

   if (!arealist || !tag || !tag[0])
      return 0;

   if (zeiger=AM_FindArea(arealist, tag))
   {
      if (zeiger->scanned)
         return MSG_UidToMsgn(arealist, tag, zeiger->oldlastread, FALSE);
      else
         return 0;
   }
   else
      return 0;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: MSG_RequestDirect                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erstellt das Requestfile direkt                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchReqAddr: Adresse, von der requestet wird                    */
/*            pchCurrentAdress: die eigene Adresse                           */
/*            pFiles: Die Files                                              */
/*            pOutbound: Zeiger auf die Outbounds                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: Fehlercode                                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Verzeichnisse werden nicht erstellt                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int MSG_RequestDirect(PCHAR pchReqAddr, PCHAR pchCurrentAddress,
                      PREQUESTLIST pFiles, OUTBOUND *pOutbound,
                      PDRIVEREMAP pDriveRemap, ULONG ulAttrib)
{
   PREQUESTLIST pOneFile=pFiles;
   PCHAR pchReqFile=NULL;
   PCHAR pchReqDup=NULL;
   PCHAR pchLastSlash=NULL;
   FILE  *fReqFile=NULL;
   FTNADDRESS FromAddr;
   FTNADDRESS MyAddr;
   struct stat filestat;
   PCHAR pchFloExt= "FLO";


   StringToNetAddr(pchReqAddr, &FromAddr, pchCurrentAddress);
   StringToNetAddr(pchCurrentAddress, &MyAddr, NULL);

   if (FromAddr.usZone == 0 ||
       MyAddr.usZone == 0)
      return WRONG_HEADER;

   pchReqFile = malloc(LEN_PATHNAME+1);

   BuildRequestName(pOutbound, &FromAddr, &MyAddr, pchReqFile, "REQ");
   MSG_RemapDrive(pchReqFile, pDriveRemap);

   pchReqDup=strdup(pchReqFile);

   /* Pfad zum File erzeugen */
   pchLastSlash=strrchr(pchReqDup, '\\');

   if (pchLastSlash)
   {
      *pchLastSlash='\0';

      if (_stat(pchReqDup, &filestat))
         CreatePath(pchReqDup);
   }
   free(pchReqDup);

   /* Pfad ist nun erzeugt */

   if (fReqFile = fopen(pchReqFile, "a"))
   {
      while(pOneFile)
      {
         if (pOneFile->pchPassword[0])
            fprintf(fReqFile, "%s !%s\n", pOneFile->pchFileName, pOneFile->pchPassword);
         else
            fprintf(fReqFile, "%s\n", pOneFile->pchFileName);
         pOneFile = pOneFile->next;
      }
      fclose(fReqFile);

      /* FLO-File erzeugen */
      if (ulAttrib & ATTRIB_IMMEDIATE)
         pchFloExt = "ILO";
      else
         if (ulAttrib & ATTRIB_CRASH)
            pchFloExt = "CLO";
         else
            if (ulAttrib & ATTRIB_DIRECT)
               pchFloExt = "DLO";
            else
               if (ulAttrib & ATTRIB_HOLD)
                  pchFloExt = "HLO";
               /* else FLO */

      BuildRequestName(pOutbound, &FromAddr, &MyAddr, pchReqFile, pchFloExt);
      MSG_RemapDrive(pchReqFile, pDriveRemap);
      if (fReqFile = fopen(pchReqFile, "a+b"))
         fclose(fReqFile);
   }
   else
   {
      free(pchReqFile);
      return ERROR;
   }

   free(pchReqFile);

   return OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: BuildRequestName                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erstellt den Pfadnamen eines Requestfiles                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pOutbound: Zeiger auf Outbound-Array                           */
/*            pAddr: Adresse, von der Requestet werden soll                  */
/*            pDefaultAddr: Default-Adresse                                  */
/*            pchPath: Puffer f. Pfadname                                    */
/*            ulFlags: Flags                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: Zeiger auf Puffer                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static char *BuildRequestName(OUTBOUND *pOutbound, FTNADDRESS *pAddr, FTNADDRESS *pDefaultAddr, char *pchPath,
                              char *pchExtension)
{
   char pchTemp[20];

   /* Zone pruefen */
   if (pAddr->usZone == pDefaultAddr->usZone)
      sprintf(pchPath, "%s\\", pOutbound[0].outbound);
   else
   {
      /* Outbound suchen */
      int i=0;
      while (i < MAX_ADDRESSES && pOutbound[i].outbound[0])
         if (pAddr->usZone == pOutbound[i].zonenum)
            break;
         else
            i++;

      if (i == MAX_ADDRESSES || !pOutbound[i].outbound[0])
         i=0;  /* nicht gefunden, Default-Outbound nehmen */

      sprintf(pchPath, "%s.%03x\\", pOutbound[i].outbound, pAddr->usZone);
   }

   /* Point untersuchen */
   if (pAddr->usPoint)
   {
      sprintf(pchTemp, "%04x%04x.PNT\\", pAddr->usNet, pAddr->usNode);
      strcat(pchPath, pchTemp);
      sprintf(pchTemp, "0000%04x.%s", pAddr->usPoint, pchExtension);
      strcat(pchPath, pchTemp);
   }
   else
   {
      /* Filename */
      sprintf(pchTemp, "%04x%04x.%s", pAddr->usNet, pAddr->usNode, pchExtension);
      strcat(pchPath, pchTemp);
   }

   return pchPath;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreatePath                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erstellt einen kompletten Pfad                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchPath: Pfad, der erzeugt werden soll.                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: Fehlercode von _mkdir                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Der Pfad darf am Ende kein \ haben. Die Funktion ist rekursiv. */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int CreatePath(char *pchPath)
{
   char *pchPartPath=strdup(pchPath);
   char *pchLastSlash;
   struct stat filestat;

   pchLastSlash = strrchr(pchPartPath, '\\');

   if (pchLastSlash)
   {
      *pchLastSlash = '\0';

      /* nun wurde hinten ein Teil abgeschnitten */
      /* existiert der Pfad bis dorthin? */

      if (_stat(pchPartPath, &filestat))
      {
         /* existiert nicht, weiter oben erzeugen */
         CreatePath(pchPartPath);
      }
   }
   free(pchPartPath);

   return _mkdir(pchPath);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RemapDrive                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt Drive-Remapping durch                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchPathName: Pfadname, der umgesetzt werden soll               */
/*            pRemapOptions: Remap-Optionen                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: -1  Fehler                                                 */
/*                0   OK                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int MSG_RemapDrive(char *pchPathName, PDRIVEREMAP pRemapOptions)
{
   if (!pchPathName || !pRemapOptions)
      return -1;

   if (pchPathName[0] &&
       pchPathName[1] == ':')
   {
      /* Laufwerk angegeben, remappen */
      *pchPathName = pRemapOptions->pchRemapString[toupper(*pchPathName) - 'C'];

      return 0;
   }
   else
      return -1;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MSG_RemapArea                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt Drive-Remapping fuer eine Area durch                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchPathName: Puffer f. Pfadname                                */
/*            pAreaDef: Area-Definition                                      */
/*            pRemapOptions: Remap-Optionen                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: -1  Fehler                                                 */
/*                0   OK                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Der Area-Pfad wird in den Puffer kopiert und das Remapping     */
/*            (falls noetig) durchgefhrt.                                   */
/*---------------------------------------------------------------------------*/

int MSG_RemapArea(char *pchBuffer, AREADEFLIST *pAreaDef, PDRIVEREMAP pDriveRemap)
{
   strcpy(pchBuffer, pAreaDef->areadata.pathfile);

   if ((pAreaDef->areadata.ulAreaOpt & AREAOPT_FROMCFG) &&
       !(pAreaDef->areadata.ulTempFlags & AREAFLAG_NOREMAP))
      return MSG_RemapDrive(pchBuffer, pDriveRemap);
   else
      return 0;
}

PMSGTEMPLATE M_FindTemplate(TEMPLATELIST *ptemplatelist, PAREALIST arealist, PCHAR tag)
{
   PMSGTEMPLATE pTemplate = ptemplatelist->pTemplates;
   PMSGTEMPLATE pDefTemplate=NULL;
   AREADEFLIST *zeiger;

   zeiger = AM_FindArea(arealist, tag);

   while(pTemplate)
   {
      if (pTemplate->ulID == 0)
         pDefTemplate = pTemplate;

      if (zeiger && pTemplate->ulID == zeiger->areadata.ulTemplateID)
         return pTemplate;

      pTemplate = pTemplate->next;
   }
   return pDefTemplate;
}

static char *M_ReadRandomOrigin(PMSGTEMPLATE pTemplate, PCHAR pchOriginBuffer, USERDATAOPT *pUserData)
{
   FILE *pfOrigins;
   ULONG i;
   ULONG ulFileOffset;
   struct stat filestat;
   char chTemp;

   pchOriginBuffer[0]=0;

   if (_stat(pTemplate->TOriginFile, &filestat) == 0)
   {
      srand(time(NULL));
      ulFileOffset = ((ULONG)rand())% filestat.st_size;

      /* File oeffnen */
      if (pfOrigins = fopen(pTemplate->TOriginFile, "rb"))
      {
         /* positionieren */
         fseek(pfOrigins, ulFileOffset, SEEK_SET);

         /* Anfang der Zeile suchen */
         while(ftell(pfOrigins) >= 0 &&
               (chTemp = fgetc(pfOrigins)) &&
               chTemp != '\r' &&
               chTemp != '\n' &&
               chTemp != '\f')
            fseek(pfOrigins, -2, SEEK_CUR);

         /* File-Pointer am Anfang der Zeile, Zeile einlesen */
         fgets(pchOriginBuffer, LEN_ORIGIN+1, pfOrigins);

         /* EOL ueberschreiben */
         i=0;
         while(i<LEN_ORIGIN && pchOriginBuffer[i])
            if (pchOriginBuffer[i] == '\r' ||
                pchOriginBuffer[i] == '\n' ||
                pchOriginBuffer[i] == EOF)
            {
               pchOriginBuffer[i]=0;
               break;
            }
            else
               i++;

         fclose(pfOrigins);
      }
   }
   if (pchOriginBuffer[0] == 0)
      strcpy(pchOriginBuffer, pTemplate->TOrigin);

   if (pchOriginBuffer[0] == 0)
      strcpy(pchOriginBuffer, pUserData->defaultorigin);

   return pchOriginBuffer;
}

void MSG_ClearMessage(PMSGHEADER pHeader, PFTNMESSAGE pMessage)
{
   if (pHeader)
      memset(pHeader, 0, sizeof(MSGHEADER));

   if (pMessage)
   {
      if (pMessage->pchMessageText)
         free(pMessage->pchMessageText);
      pMessage->pchMessageText = NULL;
      pMessage->pchSeenPath=NULL;
      MSG_RemoveKludge(pMessage, KLUDGE_ALL);
   }

   return;
}

char *MSG_AttribToText(ULONG ulAttrib, char *pchBuffer)
{
   static char *flags[]={"priv ", "crash ", "rcvd ", "sent ", "f/a ", "trans ",
                         "orph ", "kill/sent ", "local ", "hold ", "read ",
                         "freq ", "rrq ", "recpt ", "aud ", "upd ", "scanned ",
                         "arc/sent ", "direct ", "truncf ", "killf ", "imm ",
                         "gate ", "fpu ", "hubroute ", "keep", "npd ", "deleted ",
                         "r2 ", "r3 ", "r4 ", "r5", NULL};
   int i=0;

   pchBuffer[0]='\0';

   while(flags[i])
   {
      if (ulAttrib & 1UL)
         strcat(pchBuffer, flags[i]);
      ulAttrib >>= 1;

      i++;
   }

   return pchBuffer;
}


int MSG_QueryAttribCaps(PAREALIST arealist, PCHAR tag, PULONG pulAttribMask)
{
   PAREADEFLIST pAreaDef = AM_FindArea(arealist, tag);

   if (!pAreaDef)
      return AREA_NOT_FOUND;

   switch(pAreaDef->areadata.areaformat)
   {
      case AREAFORMAT_FTS:
         *pulAttribMask = FTS_QueryAttribMask();
         break;

      case AREAFORMAT_SQUISH:
         *pulAttribMask = SQ_QueryAttribMask();
         break;

      case AREAFORMAT_JAM:
         *pulAttribMask = JAM_QueryAttribMask();
         break;

      default:
         return ERROR;
   }
   return OK;
}

static void EnterSerial(void)
{
   WinRequestMutexSem(hMsgApiSem, SEM_INDEFINITE_WAIT);

   return;
}

static void ExitSerial(void)
{
   DosReleaseMutexSem(hMsgApiSem);

   return;
}

static void FillCurrentTime(PTIMESTAMP pTimeStamp)
{
   struct tm *tmstruct;
   time_t loctime;

   loctime  = time(NULL);
   tmstruct = localtime(&loctime);
   pTimeStamp->day = tmstruct->tm_mday;
   pTimeStamp->month = tmstruct->tm_mon  + 1;
   pTimeStamp->year = tmstruct->tm_year - 80;
   pTimeStamp->hours = tmstruct->tm_hour;
   pTimeStamp->minutes = tmstruct->tm_min;
   if (tmstruct->tm_sec > 59)
      pTimeStamp->seconds = 59/2;
   else
      pTimeStamp->seconds = tmstruct->tm_sec/2;

   return;
}

void MSG_SetCPInfo(ULONG ulCP)
{
   ulCodePage = ulCP;

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

void AttribToFlags(PFTNMESSAGE pMessage, PMSGHEADER pHeader, ATTRIBMAP *pMap)
{
   int i=0;
   char pchKludge[80]="";

   while (pMap[i].ulAttrib)
   {
      if (pHeader->ulAttrib & pMap[i].ulAttrib)
      {
         if (pchKludge[0])
            strcat(pchKludge, " ");
         strcat(pchKludge, pMap[i].chFlag);
      }
      i++;
   }

   if (pchKludge[0])
      MSG_SetKludge(pMessage, KLUDGE_FLAGS, pchKludge, SETKLUDGE_UNIQUE);
   else
      MSG_RemoveKludge(pMessage, KLUDGE_FLAGS);
}

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

void FlagsToAttrib(PFTNMESSAGE pMessage, PMSGHEADER pHeader, ATTRIBMAP *pMap)
{
   int i=0;
   char *pchKludge;
   char *pchTemp;
   PKLUDGE pKludge;

   if (pKludge = MSG_FindKludge(pMessage, KLUDGE_FLAGS, NULL))
   {
      pchKludge = strdup(pKludge->pchKludgeText);
      pchTemp = pchKludge;

      while (*pchTemp)
      {
         /* Leerzeichen uebergehen */
         while (*pchTemp == ' ')
            pchTemp++;

         /* Schauen, ob konvertierbar */
         i=0;
         while (pMap[i].ulAttrib)
         {
            if (!strnicmp(pMap[i].chFlag, pchTemp, 3))
            {
               char *pchDest = pchTemp;
               char *pchNext;

               /* Attribut setzen */
               pHeader->ulAttrib |= pMap[i].ulAttrib;

               /* Flag aus Kludge loeschen */
               pchNext=pchTemp;
               while (*pchNext && *pchNext != ' ')
                  pchNext++;
               while (*pchNext == ' ')
                  pchNext++;

               while (*pchNext)
                  *pchDest++ = *pchNext++;
               *pchDest = 0;

               break;
            }
            i++;
         }

         if (!pMap[i].ulAttrib) /* nicht konvertiert */
            /* bis zum naechsten Zwischenraum oder Ende */
            while (*pchTemp && *pchTemp != ' ')
               pchTemp++;
      }

      if (pchKludge[0])
         /* restliche FLAGS behalten */
         MSG_SetKludge(pMessage, KLUDGE_FLAGS, pchKludge, SETKLUDGE_UNIQUE);
      else
         /* Alle FLAGS konvertiert, Kludge loeschen */
         MSG_RemoveKludge(pMessage, KLUDGE_FLAGS);

      free(pchKludge);
   }
   return;
}

/*-------------------------------- Modulende --------------------------------*/

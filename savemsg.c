/*---------------------------------------------------------------------------+
 | Titel: SAVEMSG.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 04.10.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Message-Speicher-Funktionen von FleetStreet                             |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "savemsg.h"
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "utility.h"
#include "dialogids.h"
#include "util\addrcnv.h"


/*--------------------------------- Defines ---------------------------------*/

#define MIN_MSGLEN   200
#define EXPORT_BUFFER 64001

/*---------------------------- Globale Variablen ----------------------------*/

extern char CurrentAddress[LEN_5DADDRESS+1];
extern char CurrentArea[LEN_AREATAG+1];
extern AREALIST arealiste;
extern char *pchXPostList;
extern BOOL MailEntered[3];
extern int QuotedMsgNum;
extern HAB anchor;
extern HMODULE hmodLang;
extern DRIVEREMAP driveremap;

/*--------------------------- Funktionsprototypen ---------------------------*/

static void CopyKludges(PFTNMESSAGE pDest, PFTNMESSAGE pSrc);
static int CrosspostMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader,
                            char *pchAreaTag, char *pchAreaList);
static int CarbonCopyMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader,
                             char *pchAreaTag, PCCLIST pCCList, PCCENTRY pEntry, BOOL bUseTemplate);
static int AddNewMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader, char *pchAreaTag);

/*------------------------------ GetMsgContents -----------------------------*/
/* Holt den Message-Text und die Adressen aus den jeweiligen Feldern         */
/* Rueckgabewerte: 0 OK                                                      */
/*                 1 Fehlende Daten                                          */
/*                 2 Fehlerhafte Adressen                                    */
/*---------------------------------------------------------------------------*/

int GetMsgContents(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *MsgHeader,
                   BOOL isEcho)
{
   char chBuffer[LEN_5DADDRESS+1];
   ULONG ulTextLen=0;
   HWND hwndMLE = WinWindowFromID(hwndClient, IDML_MAINEDIT);

   /* Messagedaten holen */
   WinQueryDlgItemText(hwndClient, IDE_FROMNAME, LEN_USERNAME+1, MsgHeader->pchFromName);
   WinQueryDlgItemText(hwndClient, IDE_TONAME, LEN_USERNAME+1, MsgHeader->pchToName);
   WinQueryDlgItemText(hwndClient, IDE_SUBJTEXT, LEN_SUBJECT+1, MsgHeader->pchSubject);

   /* Adressen holen und umwandeln */
   /* Absender-Adresse ermitteln */
   WinQueryDlgItemText(hwndClient, IDE_FROMADDRESS, LEN_5DADDRESS+1, chBuffer);

   if (!chBuffer[0])
      return 1;

   if (StringToNetAddr(chBuffer, &MsgHeader->FromAddress, CurrentAddress)==NULL)
      return 2;

   /* Empfaenger-Adresse ermitteln */
   if (isEcho)
      memset(&MsgHeader->ToAddress, 0, sizeof(FTNADDRESS));
   else
   {
      WinQueryDlgItemText(hwndClient, IDE_TOADDRESS, LEN_5DADDRESS+1, chBuffer);

      if (StringToNetAddr(chBuffer, &MsgHeader->ToAddress, CurrentAddress)==NULL)
         return 2;
   }

   /* MLE fuer Export vorbereiten */
   SendMsg(hwndMLE, MLM_FORMAT, MPFROMSHORT(MLFIE_NOTRANS), NULL);
   ulTextLen=(ULONG)SendMsg(hwndMLE, MLM_QUERYTEXTLENGTH, NULL, NULL);
   ulTextLen=(ULONG)SendMsg(hwndMLE, MLM_QUERYFORMATTEXTLENGTH, NULL, MPFROMLONG(ulTextLen));

   if (ulTextLen) /* Text da, eportieren */
   {
      char *pchDest=NULL;
      ULONG ulExport;
      ULONG ipt=0;
      ULONG ulTextExported=0;

      MsgInfo->pchMessageText=calloc(1, ulTextLen+1+MIN_MSGLEN);
      DosAllocMem((PPVOID)&pchDest, EXPORT_BUFFER, PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);

      do
      {
         ulExport=EXPORT_BUFFER-1;
         memset(pchDest, 0, EXPORT_BUFFER);

         SendMsg(hwndMLE, MLM_SETIMPORTEXPORT, MPFROMP(pchDest), MPFROMLONG(ulExport));
         ulTextExported=(ULONG)SendMsg(hwndMLE, MLM_EXPORT,
                                       MPFROMP(&ipt), MPFROMLONG(&ulExport));
         if (ulTextExported)
            strcat(MsgInfo->pchMessageText, pchDest);
      } while (ulTextExported);
      DosFreeMem(pchDest);
   }
   else
   {
      MsgInfo->pchMessageText=malloc(MIN_MSGLEN);
      MsgInfo->pchMessageText[0]=0;
   }

   return 0;
}



/*----------------------------- AddNewMessage   -----------------------------*/
/* Holt die Messagedaten aus den Feldern, prueft die Daten, holt den         */
/* MessageText, speichert in der Messagebase.                                */
/* Rueckgabewerte: 0  alles OK                                               */
/*                 1  Fehlende Headerdaten                                   */
/*                 2  Falsche Headerdaten                                    */
/*                 3  Message-Text zu lang                                   */
/*                 4  Fehler beim Speichern                                  */
/*---------------------------------------------------------------------------*/

static int AddNewMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader, char *pchAreaTag)
{
   FTNMESSAGE Message={NULL, NULL, NULL, NULL};
   extern MISCOPTIONS miscoptions;
   extern USERDATAOPT userdaten;
   extern GENERALOPT generaloptions;
   extern DRIVEREMAP driveremap;
   extern TEMPLATELIST templatelist;
   int rc;
   PAREADEFLIST pAreaDef;

   /* Kludges kopieren */
   CopyKludges(&Message, OldMessage);

   pAreaDef = AM_FindArea(&arealiste, pchAreaTag);

   if (rc=GetMsgContents(hwndClient, &Message, OldHeader,
                         pAreaDef->areadata.areatype != AREATYPE_NET))
      return rc;

   /* Ueberpruefen, ob alle Headerdaten vorhanden */
   if (!OldHeader->pchToName[0] ||
       !OldHeader->pchFromName[0])
      return 1;

   if (stricmp(pchAreaTag, CurrentArea))
      MSG_OpenArea(&arealiste, pchAreaTag, miscoptions.lastreadoffset, &driveremap);

   /* Message speichern */
   switch(MSG_AddMessage(&Message, OldHeader, &arealiste, pchAreaTag,
                         &userdaten, &generaloptions, &driveremap,
                         generaloptions.lMaxMsgLen * 1024, &templatelist, 0, SendAddMessage))
   {
      case OK:
         pAreaDef->mailentered=TRUE;
         if (stricmp(pchAreaTag, CurrentArea))
            MSG_CloseArea(&arealiste, pchAreaTag, TRUE, miscoptions.lastreadoffset, &driveremap);
         else
            MSG_LinkMessages(&arealiste, CurrentArea, OldHeader->ulMsgID, OldHeader->ulReplyTo, &driveremap);
         MSG_ClearMessage(NULL, &Message);
         MailEntered[pAreaDef->areadata.areatype]=TRUE;
         return 0;

      default:
         MSG_ClearMessage(NULL, &Message);
         if (stricmp(pchAreaTag, CurrentArea))
            MSG_CloseArea(&arealiste, pchAreaTag, TRUE, miscoptions.lastreadoffset, &driveremap);
         return 4;
   }
}

/*----------------------------- ChangeMessage   -----------------------------*/
/* Holt die Messagedaten aus den Feldern, prueft die Daten, holt den         */
/* MessageText, speichert in der Messagebase.                                */
/* Rueckgabewerte: 0  alles OK                                               */
/*                 1  Fehlende Headerdaten                                   */
/*                 2  Falsche Headerdaten                                    */
/*                 3  Message-Text zu lang                                   */
/*                 4  Fehler beim Speichern                                  */
/*---------------------------------------------------------------------------*/

int ChangeMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader, char *pchAreaTag, BOOL bChangeKludges)
{
   FTNMESSAGE Message = {NULL, NULL, NULL, NULL};
   int rc;
   extern USERDATAOPT userdaten;
   extern GENERALOPT generaloptions;
   extern TEMPLATELIST templatelist;
   PAREADEFLIST pAreaDef;

   /* Kludges kopieren */
   CopyKludges(&Message, OldMessage);

   pAreaDef = AM_FindArea(&arealiste, pchAreaTag);

   if (rc=GetMsgContents(hwndClient, &Message, OldHeader,
                         pAreaDef->areadata.areatype != AREATYPE_NET))
      return rc;

   /* Ueberpruefen, ob alle Headerdaten vorhanden */
   if (!OldHeader->pchToName[0] ||
       !OldHeader->pchFromName[0])
      return 1;

   /* Messagetext zu lang? */
   if (strlen(Message.pchMessageText) > generaloptions.lMaxMsgLen * 1024)
      return 3;

   /* Message speichern */
   switch(MSG_ChangeMessage(&Message, OldHeader, &arealiste, pchAreaTag,
                            &userdaten, &generaloptions, bChangeKludges, &templatelist, SendChangeMessage))
   {
      case OK:
         pAreaDef->mailentered=TRUE;
         MSG_ClearMessage(NULL, &Message);
         MailEntered[pAreaDef->areadata.areatype]=TRUE;
         return 0;

      default:
         MSG_ClearMessage(NULL, &Message);
         return 4;
   }
}

/*----------------------------- CrosspostMessage ----------------------------*/
/* Holt die Messagedaten aus den Feldern, prueft die Daten, holt den         */
/* MessageText, speichert in der Messagebase.                                */
/* Rueckgabewerte: 0  alles OK                                               */
/*                 1  Fehlende Headerdaten                                   */
/*                 2  Falsche Headerdaten                                    */
/*                 3  Message-Text zu lang                                   */
/*                 4  Fehler beim Speichern                                  */
/*---------------------------------------------------------------------------*/

static int CrosspostMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader,
                            char *pchAreaTag, char *pchAreaList)
{
   FTNMESSAGE Message={NULL, NULL, NULL, NULL};
   int rc;
   char *pchTemp, *pchXList, *pchXArea, *pchHelp, *pchHelp2;
   BOOL bEnd=FALSE;
   extern USERDATAOPT userdaten;
   extern GENERALOPT generaloptions;
   extern MISCOPTIONS miscoptions;
   extern DRIVEREMAP driveremap;
   extern TEMPLATELIST templatelist;
   AREADEFLIST *pAreaDef;

   pAreaDef = AM_FindArea(&arealiste, pchAreaTag);

   if (!pAreaDef)
      return -1;

   /* Kludges kopieren */
   CopyKludges(&Message, OldMessage);

   if (rc=GetMsgContents(hwndClient, &Message, OldHeader,
                         pAreaDef->areadata.areatype != AREATYPE_NET))
      return rc;

   StringToNetAddr(pAreaDef->areadata.address, &OldHeader->FromAddress, NULL);

   /* Ueberpruefen, ob alle Headerdaten vorhanden */
   if (!OldHeader->pchToName[0] ||
       !OldHeader->pchFromName[0])
      return 1;

   /* Crosspost-Liste einfuegen */
   pchTemp=malloc(strlen(Message.pchMessageText)+5000);
   pchXList=malloc(strlen(pchAreaList)+1);
   strcpy(pchXList, pchAreaList);
   pchHelp=pchTemp;

   pchXArea=pchXList;
   pchHelp2=pchXArea;
   while (*pchHelp2 && *pchHelp2!=' ')
      pchHelp2++;
   if (*pchHelp2)
      *pchHelp2='\0';
   else
      pchXArea=NULL;
   while(pchXArea)
   {
      if (stricmp(pchXArea, pchAreaTag))
         pchHelp=TplXPost(&templatelist, pchHelp, pchXArea, &arealiste, pchAreaTag);

      if (bEnd)
         pchXArea=NULL;
      else
      {
         pchHelp2++;
         pchXArea=pchHelp2;
         while (*pchHelp2 && *pchHelp2!=' ')
            pchHelp2++;
         if (*pchHelp2)
            *pchHelp2='\0';
         else
            bEnd=TRUE;
      }
   }
   if (pchHelp != pchTemp) /* nur wenn nicht leer */
      *pchHelp++='\n';
   strcpy(pchHelp, Message.pchMessageText);
   free(Message.pchMessageText);
   Message.pchMessageText=pchTemp;

   free(pchXList);

   if (stricmp(pchAreaTag, CurrentArea))
      MSG_OpenArea(&arealiste, pchAreaTag, miscoptions.lastreadoffset, &driveremap);

   /* Message speichern */
   switch(MSG_AddMessage(&Message, OldHeader, &arealiste, pchAreaTag,
                         &userdaten, &generaloptions, &driveremap,
                         generaloptions.lMaxMsgLen * 1024, &templatelist, ADDOPT_MATCHADDRESS,
                         SendAddMessage))
   {
      case OK:
         pAreaDef->mailentered=TRUE;
         MSG_ClearMessage(NULL, &Message);
         if (stricmp(pchAreaTag, CurrentArea))
            MSG_CloseArea(&arealiste, pchAreaTag, TRUE, miscoptions.lastreadoffset, &driveremap);
         MailEntered[pAreaDef->areadata.areatype]=TRUE;
         return 0;

      default:
         MSG_ClearMessage(NULL, &Message);
         if (stricmp(pchAreaTag, CurrentArea))
            MSG_CloseArea(&arealiste, pchAreaTag, TRUE, miscoptions.lastreadoffset, &driveremap);
         return 4;
   }
}

/*---------------------------- CarbonCopyMessage ----------------------------*/
/* Holt die Messagedaten aus den Feldern, prueft die Daten, holt den         */
/* MessageText, speichert in der Messagebase.                                */
/* Rueckgabewerte: 0  alles OK                                               */
/*                 1  Fehlende Headerdaten                                   */
/*                 2  Falsche Headerdaten                                    */
/*                 3  Message-Text zu lang                                   */
/*                 4  Fehler beim Speichern                                  */
/*---------------------------------------------------------------------------*/

static int CarbonCopyMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader,
                             char *pchAreaTag, PCCLIST pCCList, PCCENTRY pEntry, BOOL bUseTemplate)
{
   FTNMESSAGE Message={NULL, NULL, NULL, NULL};
   int rc;
   char *pchTemp, *pchHelp;
   BOOL bOpened=FALSE;
   AREADEFLIST *zeiger=NULL;
   extern USERDATAOPT userdaten;
   extern GENERALOPT generaloptions;
   extern MISCOPTIONS miscoptions;
   extern DRIVEREMAP driveremap;
   extern TEMPLATELIST templatelist;

   zeiger=AM_FindArea(&arealiste, pchAreaTag);

   if (!zeiger)
      return 4;

   /* Kludges kopieren */
   CopyKludges(&Message, OldMessage);

   if (rc=GetMsgContents(hwndClient, &Message, OldHeader,
                         zeiger->areadata.areatype != AREATYPE_NET))
      return rc;

   /* Ueberpruefen, ob alle Headerdaten vorhanden */
   if (!OldHeader->pchFromName[0])
      return 1;

   /* Empfaenger eintragen */
   strcpy(OldHeader->pchToName, pEntry->pchName);
   StringToNetAddr(pEntry->pchAddress, &OldHeader->ToAddress, NULL);

   /* CC-Liste einfuegen */
   pchTemp=malloc(strlen(Message.pchMessageText)+5000);
   if (pEntry->pchFirstLine[0])
   {
      strcpy(pchTemp, pEntry->pchFirstLine);
      strcat(pchTemp, "\n\n");
      pchHelp = strchr(pchTemp, 0);
   }
   else
      pchHelp=pchTemp;
   if (bUseTemplate)
      pchHelp=TplCCopy(&templatelist, pchHelp, pCCList, pEntry, &arealiste, pchAreaTag);
   strcpy(pchHelp, Message.pchMessageText);
   free(Message.pchMessageText);
   Message.pchMessageText=pchTemp;

   if (zeiger->areahandle == NULL)
   {
      MSG_OpenArea(&arealiste, pchAreaTag, miscoptions.lastreadoffset, &driveremap);
      bOpened=TRUE;
   }

   /* Message speichern */
   switch(MSG_AddMessage(&Message, OldHeader, &arealiste, pchAreaTag,
                          &userdaten, &generaloptions, &driveremap,
                          generaloptions.lMaxMsgLen * 1024, &templatelist, ADDOPT_MATCHADDRESS,
                          SendAddMessage))
   {
      case OK:
         zeiger->mailentered=TRUE;
         MSG_ClearMessage(NULL, &Message);
         MailEntered[zeiger->areadata.areatype]=TRUE;
         if (bOpened)
            MSG_CloseArea(&arealiste, pchAreaTag, TRUE, miscoptions.lastreadoffset, &driveremap);
         return 0;

      default:
         MSG_ClearMessage(NULL, &Message);
         if (bOpened)
            MSG_CloseArea(&arealiste, pchAreaTag, TRUE, miscoptions.lastreadoffset, &driveremap);
         return 4;
   }
}

/*-------------------------- SaveMessage ------------------------------------*/
/* Speichert eine Message im Normal-Modus                                    */
/* Rueckgabewerte: FALSE  OK, Status zurueck auf Lesen                       */
/*                 TRUE   Fehler, Programmstatus belassen                    */
/*---------------------------------------------------------------------------*/

BOOL SaveMessage(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *Header, PCHAR pAreaTag)
{
   int rc;

   switch(rc = AddNewMessage(hwndClient, MsgInfo, Header, pAreaTag))
   {
      /* Alles OK */
      case 0:
         return FALSE;

      default:
         SaveErrorMessage(hwndClient, rc);
         return TRUE;
   }
}

/*------------------------- SaveCrosspostMessage ----------------------------*/
/* Speichert eine Message im Crosspost-Modus                                 */
/* Rueckgabewerte: FALSE  OK, Status zurueck auf Lesen                       */
/*                 TRUE   Fehler, Programmstatus belassen                    */
/*---------------------------------------------------------------------------*/

BOOL SaveCrosspostMessage(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *Header,
                          PCHAR pAreaTag, char **pchAreaList)
{
   char *AreaTag=NULL, *pchXTemp;
   int rc;

   pchXTemp=malloc(strlen(*pchAreaList)+2+strlen(pAreaTag));

   /* Uberpruefen, ob pAreaTag schon vorhanden */
   strcpy(pchXTemp, *pchAreaList);
   AreaTag=strtok(pchXTemp, " ");
   while(AreaTag)
   {
      if (!stricmp(AreaTag, pAreaTag))
         break;
      else
         AreaTag=strtok(NULL, " ");
   }
   if (AreaTag)
      strcpy(pchXTemp, *pchAreaList);
   else
   {
      strcpy(pchXTemp, pAreaTag);
      strcat(pchXTemp, " ");
      strcat(pchXTemp, *pchAreaList);
   }

   free(*pchAreaList);
   *pchAreaList=strdup(pchXTemp);

   AreaTag=strtok(pchXTemp, " ");
   while(AreaTag)
   {
      switch(rc = CrosspostMessage(hwndClient, MsgInfo, Header, AreaTag, *pchAreaList))
      {
         case 0:
            break;

         default:
            SaveErrorMessage(hwndClient, rc);
            free(pchXTemp);
            return TRUE;
      }
      AreaTag=strtok(NULL, " ");
   }
   free(pchXTemp);
   return FALSE;
}

/*----------------------------- SaveCCMessage -------------------------------*/
/* Speichert eine Message im Carbon-Copy-Modus                               */
/* Rueckgabewerte: FALSE  OK, Status zurueck auf Lesen                       */
/*                 TRUE   Fehler, Programmstatus belassen                    */
/*---------------------------------------------------------------------------*/

BOOL SaveCCMessage(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *Header, PCHAR pAreaTag,
                   PCCLIST pCCList)
{
   PCCENTRY pEntry;
   BOOL bUseTemplate=FALSE;
   int rc;

   pEntry=pCCList->pEntries;
   while (pEntry)
   {
      if (pEntry->ulFlags & CCENTRY_MENTION)
      {
         bUseTemplate = TRUE;
         break;
      }
      pEntry = pEntry->next;
   }

   pEntry=pCCList->pEntries;

   while(pEntry)
   {
      switch(rc = CarbonCopyMessage(hwndClient, MsgInfo, Header, pAreaTag,
                                    pCCList, pEntry, bUseTemplate))
      {
         /* Alles OK */
         case 0:
            break;

         default:
            SaveErrorMessage(hwndClient, rc);
            return TRUE;
      }
      if (pCCList->ulFlags & CCLIST_KILLSENT)
         Header->ulAttrib |= ATTRIB_KILLSENT;
      pEntry=pEntry->next;
   }
   return FALSE;
}


void SaveErrorMessage(HWND hwndClient, ULONG ulMsg)
{
   switch(ulMsg)
   {
      case 0: /* kein Fehler */
         return;

      /* Fehlende Headerdaten */
      case 1:
         MessageBox(hwndClient, IDST_MSG_INFOMISSING, 0,
                    IDD_INFOMISSING, MB_OK | MB_ERROR);
         return;

      /* Falsche Headerdaten */
      case 2:
         QuickMessage(hwndClient, "Falsche Headerdaten");
         return;

      /* Text zu lang */
      case 3:
         MessageBox(hwndClient, IDST_MSG_TEXTTOOLONG, 0,
                    IDD_TOOLONG, MB_OK | MB_ERROR);
         return;

      /* Fehler in der MSGAPI */
      case 4:
         MessageBox(hwndClient, IDST_MSG_ERRORMSGSAVE, 0,
                    IDD_ERRORMSGSAVE, MB_OK | MB_ERROR);
         return;

      default:
         WinAlarm(HWND_DESKTOP, WA_ERROR);
         return;
   }
}

static void CopyKludges(PFTNMESSAGE pDest, PFTNMESSAGE pSrc)
{
   PKLUDGE pKludge=NULL;

   while (pKludge = MSG_FindKludge(pSrc, KLUDGE_ANY, pKludge))
      MSG_SetKludge(pDest, pKludge->ulKludgeType, pKludge->pchKludgeText, SETKLUDGE_MULTIPLE);

   return;
}

MRESULT SendAddMessage(AREADEFLIST *pAreaDef, ULONG ulMsgID, PMSGHEADER pHeader)
{
   MESSAGEID MessageID;
   extern HWND client;

   strcpy(MessageID.pchAreaTag, pAreaDef->areadata.areatag);
   MessageID.ulMsgID = ulMsgID;

   return SendMsg(client, WORKM_ADDED, &MessageID, pHeader);
}

MRESULT SendChangeMessage(AREADEFLIST *pAreaDef, ULONG ulMsgID, PMSGHEADER pHeader)
{
   MESSAGEID MessageID;
   extern HWND client;

   strcpy(MessageID.pchAreaTag, pAreaDef->areadata.areatag);
   MessageID.ulMsgID = ulMsgID;

   return SendMsg(client, WORKM_CHANGED, &MessageID, pHeader);
}

MRESULT SendKillMessage(AREADEFLIST *pAreaDef, ULONG ulMsgID, PMSGHEADER pHeader)
{
   MESSAGEID MessageID;
   extern HWND client;

   strcpy(MessageID.pchAreaTag, pAreaDef->areadata.areatag);
   MessageID.ulMsgID = ulMsgID;

   return SendMsg(client, WORKM_DELETED, &MessageID, pHeader);
}
/*-------------------------------- Modulende --------------------------------*/


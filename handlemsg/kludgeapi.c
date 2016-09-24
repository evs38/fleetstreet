/*---------------------------------------------------------------------------+
 | Titel: KLUDGEAPI.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 20.03.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   API zur Behandlung von  Kludges                                         |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "..\main.h"
#include "..\msgheader.h"
#include "kludgeapi.h"

/*--------------------------------- Defines ---------------------------------*/

#define KLUDGE_MAX   KLUDGE_FWDMSGID

/*---------------------------------- Typen ----------------------------------*/

typedef struct
{
   const char *pchKludgeName;
   size_t      iKludgeLen;
   ULONG       ulEquivType;
} KLUDGETABLE;

/*---------------------------- Globale Variablen ----------------------------*/

static const KLUDGETABLE KludgeTable[]=
{
  {"",              0, KLUDGE_OTHER},
  {"FMPT",          4, KLUDGE_FMPT},
  {"TOPT",          4, KLUDGE_TOPT},
  {"INTL",          4, KLUDGE_INTL},
  {"MSGID:",        6, KLUDGE_MSGID},
  {"REPLY:",        6, KLUDGE_REPLY},
  {"REPLYTO",       7, KLUDGE_REPLYTO},
  {"REPLYADDR",     9, KLUDGE_REPLYADDR},
  {"FLAGS",         5, KLUDGE_FLAGS},
  {"SPLIT:",        6, KLUDGE_SPLIT},
  {"PID:",          4, KLUDGE_PID},
  {"AREA:",         5, KLUDGE_AREA},
  {"APPEND:",       7, KLUDGE_APPEND},
  {"REALADDRESS:", 12, KLUDGE_REALADDRESS},
  {"ACUPDATE:",    10, KLUDGE_ACUPDATE},
  {"CHRS:",         5, KLUDGE_CHRS},
  {"CHARSET:",      8, KLUDGE_CHRS},
  {"CISTO",         5, KLUDGE_CISTO},
  {"CISFROM",       7, KLUDGE_CISFROM},
  {"CISMSGID",      8, KLUDGE_CISMSGID},
  {"CISREPLY",      8, KLUDGE_CISREPLY},
  {"FWDFROM",       7, KLUDGE_FWDFROM},
  {"FWDTO",         5, KLUDGE_FWDTO},
  {"FWDORIG",       7, KLUDGE_FWDORIG},
  {"FWDDEST",       7, KLUDGE_FWDDEST},
  {"FWDSUBJ",       7, KLUDGE_FWDSUBJ},
  {"FWDAREA",       7, KLUDGE_FWDAREA},
  {"FWDMSGID",      8, KLUDGE_FWDMSGID}
};

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int FirstWordEqual(char *pchLine, const char *pchWord);

/*---------------------------------------------------------------------------*/
/* Funktionsname: MSG_SetKludge                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt eine Kludge in einer Message                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pMessage: Ziel-Message                                         */
/*            ulKludgeType: KLUDGE_*                                         */
/*            pchKludgeText: Kludge-Inhalt                                   */
/*            ulFlags:  SETKLUDGE_*                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf neuen Kludge-Eintrag                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

PKLUDGE MSG_SetKludge(PFTNMESSAGE pMessage, ULONG ulKludgeType,
                      char *pchKludgeText, ULONG ulFlags)
{
   PKLUDGE pTemp=NULL;

   if (ulKludgeType && !(ulFlags & SETKLUDGE_MULTIPLE))
      pTemp = MSG_FindKludge(pMessage, ulKludgeType, NULL);

   if (pTemp)
   {
      /* bestehende Kludge aendern */
      if (pTemp->pchKludgeText)
         free(pTemp->pchKludgeText);
      pTemp->pchKludgeText = strdup(pchKludgeText);
   }
   else
   {
      /* hinten anhaengen */
      if (pTemp = calloc(1, sizeof(KLUDGE)))
      {
         pTemp->ulKludgeType = ulKludgeType;
         if (pTemp->pchKludgeText = strdup(pchKludgeText))
         {
            pTemp->prev = pMessage->pLastKludge;
            if (pMessage->pLastKludge)
               pMessage->pLastKludge->next = pTemp;
            pMessage->pLastKludge = pTemp;
            if (!pMessage->pFirstKludge)
               pMessage->pFirstKludge = pTemp;
         }
         else
         {
            free(pTemp);
            pTemp = NULL;
         }
      }
   }

   return pTemp;
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

PKLUDGE MSG_SetKludgeVar(PFTNMESSAGE pMessage, ULONG ulKludgeType,
                         ULONG ulFlags, char *pchFormat, ...)
{
   char pchText[100];
   va_list arg_ptr;

   va_start(arg_ptr, pchFormat);
   vsprintf(pchText, pchFormat, arg_ptr);
   va_end(arg_ptr);

   return MSG_SetKludge(pMessage, ulKludgeType, pchText, ulFlags);
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

PKLUDGE MSG_FindKludge(PFTNMESSAGE pMessage, ULONG ulKludgeType, PKLUDGE pFirst)
{
   if (!pMessage)
      return NULL;

   if (!pFirst)
      pFirst = pMessage->pFirstKludge;
   else
      pFirst = pFirst->next;

   while (pFirst && ulKludgeType && (ulKludgeType != pFirst->ulKludgeType))
      pFirst = pFirst->next;

   return pFirst;
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

int MSG_RemoveKludge(PFTNMESSAGE pMessage, ULONG ulKludgeType)
{
   PKLUDGE pTemp = pMessage->pFirstKludge;
   PKLUDGE pTemp2;

   while (pTemp)
   {
      if (ulKludgeType == KLUDGE_ALL ||  /* alle loeschen */
          ulKludgeType == pTemp->ulKludgeType)
      {
         /* loeschen */
         if (pTemp->pchKludgeText)
            free(pTemp->pchKludgeText);
         if (pTemp->prev)
            pTemp->prev->next = pTemp->next;
         if (pTemp->next)
            pTemp->next->prev = pTemp->prev;
         if (pMessage->pFirstKludge == pTemp)
            pMessage->pFirstKludge = pTemp->next;
         if (pMessage->pLastKludge == pTemp)
            pMessage->pLastKludge = pTemp->prev;
         pTemp2 = pTemp;
         pTemp = pTemp->next;
         free(pTemp2);
      }
      else
         pTemp = pTemp->next;
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

const char *MSG_QueryKludgeName(ULONG ulKludgeType)
{
   if (ulKludgeType <= KLUDGE_MAX)
      return KludgeTable[ulKludgeType].pchKludgeName;
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

ULONG MSG_CalcKludgeBufferSize(PFTNMESSAGE pMessage)
{
   ULONG ulTotlSize=1;  /* abschliessende 0 einberechnen */
   PKLUDGE pTemp = pMessage->pFirstKludge;

   while (pTemp)
   {
      ulTotlSize += strlen(pTemp->pchKludgeText) +
                    KludgeTable[pTemp->ulKludgeType].iKludgeLen +
                    1; /* ^a */
      if (pTemp->ulKludgeType != KLUDGE_OTHER)
         ulTotlSize++; /* zusaetzlich Leerzeichen zw. Kludge und Inhalt */
      pTemp = pTemp->next;
   }

   if (ulTotlSize == 1) /* keine Kludges: Platz f. ^a und \0 */
      ulTotlSize = 2;

   return ulTotlSize;
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

char *MSG_KludgesToBuffer(PFTNMESSAGE pMessage, char *pchBuffer)
{
   PKLUDGE pTemp = pMessage->pFirstKludge;
   char *pchDest = pchBuffer;

   if (!pchBuffer)
      return NULL;

   if (!pMessage->pFirstKludge)
   {
      pchBuffer[0]=1;  /* mindestens ^a\0 */
      pchBuffer[1]=0;
      return pchBuffer;
   }

   while (pTemp)
   {
      if (pTemp->ulKludgeType)
         sprintf(pchDest, "%c%s %s", 0x01, MSG_QueryKludgeName(pTemp->ulKludgeType),
                                           pTemp->pchKludgeText);
      else
         sprintf(pchDest, "%c%s", 0x01, pTemp->pchKludgeText);

      pchDest = strchr(pchDest, '\0');

      pTemp = pTemp->next;
   }
   return pchBuffer;
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

int MSG_BufferToKludges(PFTNMESSAGE pMessage, char *pchBuffer)
{
   char *pchDup = strdup(pchBuffer);
   char *pchTemp;

   pchTemp = strtok(pchDup, "\x01");

   while (pchTemp)
   {
      MSG_SetKludgeFromBuffer(pMessage, pchTemp, SETKLUDGE_MULTIPLE);

      pchTemp = strtok(NULL, "\x01");
   }

   free(pchDup);

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

PKLUDGE MSG_SetKludgeFromBuffer(PFTNMESSAGE pMessage, char *pchBuffer, ULONG ulFlags)
{
   ULONG ulType;
   PCHAR pchSpace;

   /* schauen, ob in Kludges vorhanden */
   ulType = KLUDGE_FMPT;
   while (ulType <= KLUDGE_MAX)
   {
      if (FirstWordEqual(pchBuffer, KludgeTable[ulType].pchKludgeName))
         break;
      ulType++;
   }

   if (ulType <= KLUDGE_MAX)
   {
      /* erstes Leerzeichen suchen */
      pchSpace = pchBuffer;
      while (*pchSpace && *pchSpace != ' ' && *pchSpace != ':')
         pchSpace++;
      while (*pchSpace && (*pchSpace == ' ' || *pchSpace ==':'))
         pchSpace++;

      return MSG_SetKludge(pMessage, KludgeTable[ulType].ulEquivType, pchSpace, ulFlags);
   }
   else
      return MSG_SetKludge(pMessage, KLUDGE_OTHER, pchBuffer, ulFlags);
}

static int FirstWordEqual(char *pchLine, const char *pchWord)
{
   BOOL bHadColon=FALSE;

   while (*pchWord &&
          *pchWord == *pchLine)
   {
      if (*pchWord == ':')
         bHadColon=TRUE;
      pchWord++;
      pchLine++;
   }

   if (!(*pchWord) &&
       (!(*pchLine) || (*pchLine == ' ') || bHadColon))
      return TRUE;
   else
      return FALSE;
}
/*-------------------------------- Modulende --------------------------------*/


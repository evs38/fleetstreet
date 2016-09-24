/*---------------------------------------------------------------------------+
 | Titel: HANDLETEMPLATE.C                                                   |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 01.10.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |  Erzeugung von Messageteilen aus Templates                                |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\main.h"
#include "..\structs.h"
#include "..\msgheader.h"
#include "..\areaman\areaman.h"
#include "..\util\addrcnv.h"
#include "handlemsg.h"
#include "handletemplate.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static INTLSETTING intlsetting;

/*--------------------------- Funktionsprototypen ---------------------------*/

static char *InsertDate(char *pchDest, TIMESTAMP *DateTime);
static char *InsertTime(char *pchDest, TIMESTAMP *DateTime);

/*------------------------------ TplSetIntl    ------------------------------*/
/* Ubergibt die Einstellungen f. Datum und Zeit an das Modul                 */
/*---------------------------------------------------------------------------*/

void TplSetIntl(INTLSETTING *pIntl)
{
   memcpy(&intlsetting,  pIntl, sizeof(INTLSETTING));
   return;
}

/*------------------------------ TplHeader     ------------------------------*/
/* Template-Ersetzung fuer Header                                            */
/*---------------------------------------------------------------------------*/

char *TplHeader(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchToName)
{
   char *pchSource, *pchDest, *pchHelp;

   pchSource=msgtemplate->THeader;
   pchDest=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'T':
               case 't':
                  if (stricmp(pchToName, msgtemplate->TAllSyn))
                  {
                     strcpy(pchDest, pchToName);
                     while (*pchDest)
                        pchDest++;
                  }
                  pchSource++;
                  break;

               case 'Z':
               case 'z':
                  if (stricmp(pchToName, msgtemplate->TAllSyn))
                  {
                     pchHelp=pchToName;
                     while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                        *pchDest++=*pchHelp++;
                  }
                  pchSource++;
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         default:            /* Sonst alles kopieren */
            *pchDest++=*pchSource++;
            break;
      }
   if (msgtemplate->THeader[0])
      *pchDest++='\n';
   *pchDest='\0';

   return pchDest;
}

/*------------------------------ TplFooter     ------------------------------*/
/* Template-Ersetzung fuer Footer                                            */
/*---------------------------------------------------------------------------*/

char *TplFooter(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchUser)
{
   char *pchSource, *pchDest, *pchHelp;

   pchSource=msgtemplate->TFooter;
   pchDest=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'U':
               case 'u':
                  strcpy(pchDest, pchUser);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'I':
               case 'i':
                  pchHelp=pchUser;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         default:
            *pchDest++=*pchSource++;
            break;
      }
   if (msgtemplate->TFooter[0])
      *pchDest++='\n';
   *pchDest='\0';

   return pchDest;
}

/*------------------------------ TplReply      ------------------------------*/
/* Template-Ersetzung fuer Reply                                             */
/*---------------------------------------------------------------------------*/

char *TplReply(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchFromName, char *pchToName, char *pchNewTo,
               char *pchArea, TIMESTAMP *DateWritten, FTNADDRESS *pFromAd, FTNADDRESS *pToAd,
               char *pchSubject, char *pchAreaDes)
{
   char *pchSource, *pchDest, *pchHelp;

   pchSource=msgtemplate->TReply;
   pchDest=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'T':
               case 't':
                  strcpy(pchDest, pchToName);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'Z':
               case 'z':
                  pchHelp=pchToName;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'O':
               case 'o':
                  strcpy(pchDest, pchNewTo);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'P':
               case 'p':
                  pchHelp=pchNewTo;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'F':
               case 'f':
                  strcpy(pchDest, pchFromName);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'G':
               case 'g':
                  pchHelp=pchFromName;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'A':
               case 'a':
                  strcpy(pchDest, pchArea);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'D':
               case 'd':
                  pchDest = InsertDate(pchDest, DateWritten);
                  pchSource++;
                  break;

               case 'M':
               case 'm':
                  pchDest = InsertTime(pchDest, DateWritten);
                  pchSource++;
                  break;

               case 'J':
               case 'j':
                  NetAddrToString(pchDest, pFromAd);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'R':
               case 'r':
                  NetAddrToString(pchDest, pToAd);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'S':
               case 's':
                  strcpy(pchDest, pchSubject);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'E':
               case 'e':
                  strcpy(pchDest, pchAreaDes);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         default:
            *pchDest++=*pchSource++;
            break;
      }
   if (msgtemplate->TReply[0])
      *pchDest++='\n';
   *pchDest='\0';

   return pchDest;
}

/*------------------------------ TplReplyOther ------------------------------*/
/* Template-Ersetzung fuer Reply in anderer Area                             */
/*---------------------------------------------------------------------------*/

char *TplReplyOther(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchOrigArea, char *pchAreaDes)
{
   char *pchSource, *pchDest;

   pchSource=msgtemplate->TDArea;
   pchDest=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'A':
               case 'a':
                  strcpy(pchDest, pchOrigArea);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'E':
               case 'e':
                  strcpy(pchDest, pchAreaDes);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         default:
            *pchDest++=*pchSource++;
            break;
      }

   if (msgtemplate->TDArea[0])
   {
      /* Zeile abschlieáen und Leerzeile (nur wenn Template-Text vorhanden) */
      *pchDest++='\n';
      *pchDest++='\n';
   }
   *pchDest='\0';

   return pchDest;
}

/*------------------------------ TplForward    ------------------------------*/
/* Template-Ersetzung fuer Forward                                           */
/*  Falls pToAd == NULL ist, wird das Token nicht ersetzt (Forward von       */
/*  Echomail)                                                                */
/*---------------------------------------------------------------------------*/

char *TplForward(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchOrigArea, char *pchUser, char *pchFromName,
                 char *pchToName, TIMESTAMP *DateWritten, FTNADDRESS *pFromAd, FTNADDRESS *pToAd,
                 char *pchSubject, char *pchAreaDes, FTNADDRESS *pMyAddr)
{
   char *pchSource, *pchDest, *pchHelp;

   pchSource=msgtemplate->TForward;
   pchDest=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'T':
               case 't':
                  strcpy(pchDest, pchToName);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'Z':
               case 'z':
                  pchHelp=pchToName;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'F':
               case 'f':
                  strcpy(pchDest, pchFromName);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'G':
               case 'g':
                  pchHelp=pchFromName;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'A':
               case 'a':
                  strcpy(pchDest, pchOrigArea);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'U':
               case 'u':
                  strcpy(pchDest, pchUser);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'I':
               case 'i':
                  pchHelp=pchUser;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'J':
               case 'j':
                  NetAddrToString(pchDest, pFromAd);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'R':
               case 'r':
                  if (pToAd)
                  {
                     NetAddrToString(pchDest, pToAd);
                     while (*pchDest)
                        pchDest++;
                  }
                  pchSource++;
                  break;

               case 'W':
               case 'w':
                  NetAddrToString(pchDest, pMyAddr);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'S':
               case 's':
                  strcpy(pchDest, pchSubject);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'E':
               case 'e':
                  strcpy(pchDest, pchAreaDes);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'D':
               case 'd':
                  pchDest = InsertDate(pchDest, DateWritten);
                  pchSource++;
                  break;

               case 'M':
               case 'm':
                  pchDest = InsertTime(pchDest, DateWritten);
                  pchSource++;
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         default:
            *pchDest++=*pchSource++;
            break;
      }
   if (msgtemplate->TForward[0])
      *pchDest++='\n';
   *pchDest='\0';

   return pchDest;
}

/*------------------------------ TplForwardFooter ---------------------------*/
/* Template-Ersetzung fuer Forward-Footer                                    */
/*---------------------------------------------------------------------------*/

char *TplForwardFooter(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchOrigArea, char *pchUser, char *pchFromName,
                       char *pchToName)
{
   char *pchSource, *pchDest, *pchHelp;

   pchSource=msgtemplate->TForwardFooter;
   pchDest=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'T':
               case 't':
                  strcpy(pchDest, pchToName);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'Z':
               case 'z':
                  pchHelp=pchToName;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'F':
               case 'f':
                  strcpy(pchDest, pchFromName);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'G':
               case 'g':
                  pchHelp=pchFromName;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               case 'A':
               case 'a':
                  strcpy(pchDest, pchOrigArea);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'U':
               case 'u':
                  strcpy(pchDest, pchUser);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'I':
               case 'i':
                  pchHelp=pchUser;
                  while (*pchHelp && *pchHelp!=' ' && *pchHelp!='\t')
                     *pchDest++=*pchHelp++;
                  pchSource++;
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         default:
            *pchDest++=*pchSource++;
            break;
      }
   if (msgtemplate->TForwardFooter[0])
      *pchDest++='\n';
   *pchDest='\0';

   return pchDest;
}

/*------------------------------ TplXPost      ------------------------------*/
/* Template-Ersetzung fuer Crosspost                                         */
/*---------------------------------------------------------------------------*/

char * TplXPost(TEMPLATELIST *ptemplatelist, char *pchBuffer, char *pchAreaList, PAREALIST arealist, PCHAR tag)
{
   char *pchSource, *pchDest;
   PMSGTEMPLATE pTemplate;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, tag);

   /* leeres Template nicht ersetzen */
   if (!pTemplate->TXPost)
      return pchBuffer;

   pchSource=pTemplate->TXPost;
   pchDest=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'A':
               case 'a':
                  strcpy(pchDest, pchAreaList);
                  while (*pchDest)
                     pchDest++;
                  pchSource++;
                  break;

               case 'E':
               case 'e':
                  {
                     AREADEFLIST *pAreaDef;

                     pAreaDef = AM_FindArea(arealist, pchAreaList);
                     if (pAreaDef)
                     {
                        strcpy(pchDest, pAreaDef->areadata.areadesc);
                        while (*pchDest)
                           pchDest++;
                     }
                     pchSource++;
                  }
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         default:
            *pchDest++=*pchSource++;
            break;
      }
   if (pTemplate->TXPost[0])
      *pchDest++='\n';
   *pchDest='\0';

   return pchDest;
}

/*------------------------------ TplCCopy      ------------------------------*/
/* Template-Ersetzung fuer Carbon Copy                                       */
/*---------------------------------------------------------------------------*/

char * TplCCopy(TEMPLATELIST *ptemplatelist, char *pchBuffer, CCLIST *pCCList, PCCENTRY pDestEntry, PAREALIST arealist, PCHAR tag)
{
   char *pchSource, *pchDest, *pchLine;
   PCCENTRY pEntry;
   BOOL bFirst=TRUE;
   PMSGTEMPLATE pTemplate;

   pTemplate = M_FindTemplate(ptemplatelist, arealist, tag);

   pchSource=pTemplate->TCCopy;
   pchDest=pchBuffer;
   pchLine=pchBuffer;

   while (*pchSource)
      switch(*pchSource)
      {
         case '%':
            pchSource++;
            switch (*pchSource)
            {
               case 'C':
               case 'c':
                  *pchDest='\0';
                  pEntry=pCCList->pEntries;
                  while(pEntry)
                  {
                     if ((pEntry->ulFlags & CCENTRY_MENTION) &&
                         pEntry != pDestEntry)
                     {
                        if (!bFirst)
                        {
                           strcat(pchLine, ", ");
                           while (*pchDest)
                              pchDest++;
                        }
                        else
                           bFirst=FALSE;
                        if (strlen(pchLine)+strlen(pEntry->pchName)+2 > 80)
                        {
                           *pchDest++='\n';
                           *pchDest='\0';
                           pchLine=pchDest;
                        }
                        strcat(pchLine, pEntry->pchName);
                        while (*pchDest)
                           pchDest++;
                     }
                     pEntry=pEntry->next;
                  }
                  pchSource++;
                  break;

               default:
                  *pchDest++='%';
                  *pchDest++=*pchSource++;
                  break;
            }
            break;

         case '\r':          /* CR bei CRLF uebergehen */
            pchSource++;
            break;

         case '\n':
            *pchDest++=*pchSource++;
            pchLine=pchDest;
            break;

         default:
            *pchDest++=*pchSource++;
            break;
      }
   if (pTemplate->TCCopy[0])
   {
      *pchDest++='\n';
      *pchDest++='\n';
   }
   *pchDest='\0';

   return pchDest;
}

static char *InsertDate(char *pchDest, TIMESTAMP *DateTime)
{
    switch(intlsetting.DMY)
    {
       case 0:
          sprintf(pchDest, "%02d%c%02d%c%4d", DateTime->month,
                                              intlsetting.datesep,
                                              DateTime->day,
                                              intlsetting.datesep,
                                              DateTime->year+1980);
          break;

       case 1:
          sprintf(pchDest, "%02d%c%02d%c%4d", DateTime->day,
                                              intlsetting.datesep,
                                              DateTime->month,
                                              intlsetting.datesep,
                                              DateTime->year+1980);
          break;

       case 2:
          sprintf(pchDest, "%4d%c%02d%c%02d", DateTime->year+1980,
                                              intlsetting.datesep,
                                              DateTime->month,
                                              intlsetting.datesep,
                                              DateTime->day);
          break;

       default:
          break;
    }
    return strchr(pchDest, 0);
}

static char *InsertTime(char *pchDest, TIMESTAMP *DateTime)
{
    if (intlsetting.h24)
    {
       sprintf(pchDest, "%02d%c%02d%c%02d", DateTime->hours,
                                            intlsetting.timesep,
                                            DateTime->minutes,
                                            intlsetting.timesep,
                                            DateTime->seconds);
    }
    else
    {
       switch(DateTime->hours)
       {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
          case 10:
          case 11:
             sprintf(pchDest, "%02d%c%02d%c%02d%s", DateTime->hours,
                                                    intlsetting.timesep,
                                                    DateTime->minutes,
                                                    intlsetting.timesep,
                                                    DateTime->seconds,
                                                    intlsetting.amtext);
             break;

          case 12:
             sprintf(pchDest, "%02d%c%02d%c%02d%s", DateTime->hours,
                                                    intlsetting.timesep,
                                                    DateTime->minutes,
                                                    intlsetting.timesep,
                                                    DateTime->seconds,
                                                    intlsetting.pmtext);
             break;

          case 13:
          case 14:
          case 15:
          case 16:
          case 17:
          case 18:
          case 19:
          case 20:
          case 21:
          case 22:
          case 23:
             sprintf(pchDest, "%02d%c%02d%c%02d%s", DateTime->hours-12,
                                                    intlsetting.timesep,
                                                    DateTime->minutes,
                                                    intlsetting.timesep,
                                                    DateTime->seconds,
                                                    intlsetting.pmtext);
             break;
       }
    }
    return strchr(pchDest, 0);
}
/*-------------------------------- Modulende --------------------------------*/


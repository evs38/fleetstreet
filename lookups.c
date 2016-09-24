/*---------------------------------------------------------------------------+
 | Titel: LOOKUPS.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 31.12.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |  Nodelist-Lookups und Dialoge                                             |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Moegl. Verbesserungen:                                                    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Geaendert: 04.01. Modem-Typen;                                            |
 |            19.07. Lookup nach Adresse;                                    |
 | 1995       28.05. Lookups mit Messages zentralisiert                      |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_WIN
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "resids.h"
#include "structs.h"
#include "msgheader.h"
#include "fltv7\fltv7.h"
#include "dialogids.h"
#include "areaman\areaman.h"
#include "utility.h"
#include "util\addrcnv.h"
#include "lookups.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

typedef struct {
        MINIRECORDCORE RecordCore;
        PCHAR pchSysop;
        PCHAR pchAddress;
        PCHAR pchSystemName;
        PCHAR pchLocation;
        PCHAR pchPhone;
        PCHAR pchModem;
        ULONG ulBaud;
        ULONG ulCallCost;
        ULONG ulUserCost;
        PCHAR pchFlags;
        ULONG ulCounter;
     } LOOKUPRECORD, *PLOOKUPRECORD;

typedef struct _nodedatalist {
         struct _nodedatalist *next;
         NODEDATA NodeData;
      } NODEDATALIST, *PNODEDATALIST;

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HMODULE hmodLang;
static PFNWP OldLookupContainerProc;

/*--------------------------- Funktionsprototypen ---------------------------*/

static MRESULT EXPENTRY NewLookupContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static void CleanupLookup(HWND hwndCnr);
static void LookupErrorMessage(HWND hwndOwner, char *pchErrDomain, int iErrNr);

/*----------------------- interne Funktionsprototypen -----------------------*/


/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: LookupProc                                                 º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Auswahl-Dialog beim Nodelist-Lookup                         º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: (Window-Procedure)                                             º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: MRESULT                                                    º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

MRESULT EXPENTRY LookupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern WINDOWFONTS windowfonts;
   extern WINDOWCOLORS windowcolors;
   extern DIRTYFLAGS dirtyflags;
   extern LOOKUPOPTIONS lookupoptions;
   extern HWND hwndhelp;
   static LOOKUPPAR *pLookupPar;
   int i;
   PFIELDINFO pFieldInfo, pFirstFieldInfo, pSeparator=NULL;
   FIELDINFOINSERT FieldInfoInsert;
   CNRINFO CnrInfo;
   PLOOKUPRECORD pRecord, pFirstRecord;
   RECORDINSERT RecordInsert;
   HWND hwndCnr;
   static char pchTitleSysop[50];
   static char pchTitleAddress[50];
   static char pchTitleSystem[50];
   static char pchTitleLocation[50];
   static char pchTitlePhone[50];
   static char pchTitleModem[50];
   static char pchTitleBaud[50];
   static char pchTitleCallcost[50];
   static char pchTitleUsercost[50];
   static char pchTitleFlags[50];
   static char pchTitleContainer[80];
   BOOL bTemp;
   MRESULT resultbuf;

   switch(message)
   {
      case WM_INITDLG:
         pLookupPar=(LOOKUPPAR*) mp2;
         WinAssociateHelpInstance(hwndhelp, parent);

         pchTitleContainer[0] = 0;
         LoadString(IDST_LU_TITLE, 45, pchTitleContainer);
         strcat(pchTitleContainer, pLookupPar->pchName);

         WinCheckButton(parent, IDD_LOOKUP+5, lookupoptions.bBrief);

         hwndCnr=WinWindowFromID(parent, IDD_LOOKUP+3);
         OldLookupContainerProc=WinSubclassWindow(hwndCnr, NewLookupContainerProc);

         SetFont(hwndCnr, windowfonts.lookupfont);
         SetForeground(hwndCnr, &windowcolors.lookupfore);
         SetBackground(hwndCnr, &windowcolors.lookupback);

         /* Felder des Containers vorbereiten */
         pFirstFieldInfo=(PFIELDINFO)SendMsg(hwndCnr, CM_ALLOCDETAILFIELDINFO,
                                                MPFROMLONG(10), NULL);

         pFieldInfo=pFirstFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_SYSOP, 50, pchTitleSysop);
         pFieldInfo->pTitleData= pchTitleSysop;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, pchSysop);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_ADDRESS, 50, pchTitleAddress);
         pFieldInfo->pTitleData= pchTitleAddress;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, pchAddress);
         pSeparator=pFieldInfo;
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_SYSTEM, 50, pchTitleSystem);
         pFieldInfo->pTitleData= pchTitleSystem;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, pchSystemName);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR |
                            ((lookupoptions.bBrief)?(CFA_INVISIBLE):0);
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_LOCATION, 50, pchTitleLocation);
         pFieldInfo->pTitleData= pchTitleLocation;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, pchLocation);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR |
                            ((lookupoptions.bBrief)?(CFA_INVISIBLE):0);
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_PHONE, 50, pchTitlePhone);
         pFieldInfo->pTitleData= pchTitlePhone;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, pchPhone);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_MODEM, 50, pchTitleModem);
         pFieldInfo->pTitleData= pchTitleModem;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, pchModem);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_ULONG | CFA_HORZSEPARATOR | CFA_SEPARATOR |
                            CFA_RIGHT | ((lookupoptions.bBrief)?(CFA_INVISIBLE):0);
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_BAUD, 50, pchTitleBaud);
         pFieldInfo->pTitleData= pchTitleBaud;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, ulBaud);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_ULONG | CFA_HORZSEPARATOR | CFA_SEPARATOR | CFA_RIGHT |
                            ((lookupoptions.bBrief)?(CFA_INVISIBLE):0);
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_CALLCOST, 50, pchTitleCallcost);
         pFieldInfo->pTitleData= pchTitleCallcost;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, ulCallCost);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_ULONG | CFA_HORZSEPARATOR | CFA_SEPARATOR | CFA_RIGHT |
                            ((lookupoptions.bBrief)?(CFA_INVISIBLE):0);
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_USERCOST, 50, pchTitleUsercost);
         pFieldInfo->pTitleData= pchTitleUsercost;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, ulUserCost);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_FLAGS, 50, pchTitleFlags);
         pFieldInfo->pTitleData= pchTitleFlags;
         pFieldInfo->offStruct= FIELDOFFSET(LOOKUPRECORD, pchFlags);

         /* Felder des Containers einfuegen */
         FieldInfoInsert.cb=sizeof(FIELDINFOINSERT);
         FieldInfoInsert.pFieldInfoOrder=(PFIELDINFO) CMA_FIRST;
         FieldInfoInsert.fInvalidateFieldInfo=TRUE;
         FieldInfoInsert.cFieldInfoInsert=10;

         SendMsg(hwndCnr, CM_INSERTDETAILFIELDINFO,
                    pFirstFieldInfo, &FieldInfoInsert);

         /* Container-Attribute setzen */
         CnrInfo.cb=sizeof(CNRINFO);
         CnrInfo.pFieldInfoLast=pSeparator;
         CnrInfo.pszCnrTitle = pchTitleContainer;
         CnrInfo.flWindowAttr=CV_DETAIL | CA_DETAILSVIEWTITLES | CA_CONTAINERTITLE | CA_TITLEREADONLY |
                              CA_TITLESEPARATOR;
         CnrInfo.xVertSplitbar=lookupoptions.lSplitBar;

         SendMsg(hwndCnr, CM_SETCNRINFO, &CnrInfo,
                    MPFROMLONG(CMA_PFIELDINFOLAST | CMA_FLWINDOWATTR | CMA_XVERTSPLITBAR | CMA_CNRTITLE));

         /* Elemente einfuegen */

         pFirstRecord=(PLOOKUPRECORD)SendMsg(hwndCnr, CM_ALLOCRECORD,
                                                MPFROMLONG(sizeof(LOOKUPRECORD)-sizeof(MINIRECORDCORE)),
                                                MPFROMLONG(pLookupPar->iCountNodes));
         pRecord=pFirstRecord;

         for (i=0; i<pLookupPar->iCountNodes; i++)
         {
            pRecord->pchSysop= pLookupPar->pNodes[i].SysopName;
            pRecord->pchAddress=malloc(LEN_5DADDRESS+1);
            if (pLookupPar->pNodes[i].Address.usPoint)
               sprintf(pRecord->pchAddress, "%d:%d/%d.%d", pLookupPar->pNodes[i].Address.usZone,
                                               pLookupPar->pNodes[i].Address.usNet,
                                               pLookupPar->pNodes[i].Address.usNode,
                                               pLookupPar->pNodes[i].Address.usPoint);
            else
               sprintf(pRecord->pchAddress, "%d:%d/%d", pLookupPar->pNodes[i].Address.usZone,
                                            pLookupPar->pNodes[i].Address.usNet,
                                            pLookupPar->pNodes[i].Address.usNode);
            pRecord->pchSystemName= pLookupPar->pNodes[i].SystemName;
            pRecord->pchPhone= pLookupPar->pNodes[i].PhoneNr;
            pRecord->pchLocation= pLookupPar->pNodes[i].Location;
            pRecord->ulBaud= pLookupPar->pNodes[i].BaudRate;
            pRecord->ulUserCost= pLookupPar->pNodes[i].UserCost;
            pRecord->ulCallCost= pLookupPar->pNodes[i].CallCost;

            pRecord->pchFlags=malloc(30);
            NLFlagsToString(&pLookupPar->pNodes[i], pRecord->pchFlags);

            pRecord->pchModem= malloc(MAX_MODEMTYPES*(LEN_MODEMTYPE+2)+1);
            NLModemToString(pLookupPar->pNodes[i].ModemType, pRecord->pchModem);

            pRecord->ulCounter=i;

            pRecord=(PLOOKUPRECORD)pRecord->RecordCore.preccNextRecord;
         }

         RecordInsert.cb=sizeof(RECORDINSERT);
         RecordInsert.pRecordOrder=(PRECORDCORE) CMA_FIRST;
         RecordInsert.pRecordParent=NULL;
         RecordInsert.fInvalidateRecord=TRUE;
         RecordInsert.zOrder=CMA_TOP;
         RecordInsert.cRecordsInsert=pLookupPar->iCountNodes;

         SendMsg(hwndCnr, CM_INSERTRECORD, pFirstRecord, &RecordInsert);

         RestoreWinPos(parent, &windowpositions.lookuppos, TRUE, TRUE);
         break;

      case WM_QUERYTRACKINFO:
         {
            SWP swp;

            /* Default-Werte aus Original-Prozedur holen */
            resultbuf=WinDefDlgProc(parent,message,mp1,mp2);

            WinQueryWindowPos(WinWindowFromID(parent, IDD_LOOKUP+5),
                              &swp);

            /* Minimale Fenstergroesse einstellen */
            ((PTRACKINFO)mp2)->ptlMinTrackSize.x=swp.x+swp.cx+5;
            ((PTRACKINFO)mp2)->ptlMinTrackSize.y=200;
         }
         return resultbuf;

      case WM_ADJUSTFRAMEPOS:
         if (((PSWP)mp1)->fl & (SWP_SIZE|SWP_MAXIMIZE|SWP_MINIMIZE|SWP_RESTORE))
         {
            SWP swp;
            RECTL rectl;

            rectl.xLeft=0;
            rectl.xRight=((PSWP)mp1)->cx;
            rectl.yBottom=0;
            rectl.yTop=((PSWP)mp1)->cy;

            CalcClientRect(anchor, parent, &rectl);
            WinQueryWindowPos(WinWindowFromID(parent, DID_OK), &swp);
            rectl.yBottom += swp.y + swp.cy;
            WinSetWindowPos(WinWindowFromID(parent, IDD_LOOKUP+3),
                            NULLHANDLE,
                            rectl.xLeft, rectl.yBottom,
                            rectl.xRight-rectl.xLeft, rectl.yTop-rectl.yBottom,
                            SWP_MOVE | SWP_SIZE);
         }
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)==DID_OK)
         {
            /* Auswahl abfragen */
            pRecord=(PLOOKUPRECORD)WinSendDlgItemMsg(parent, IDD_LOOKUP+3,
                                     CM_QUERYRECORDEMPHASIS,
                                     MPFROMP(CMA_FIRST),
                                     MPFROMSHORT(CRA_CURSORED));
            pLookupPar->ulSelected=pRecord->ulCounter;
         }
         break;

      case WM_CONTROL:
         switch (SHORT1FROMMP(mp1))
         {
            case IDD_LOOKUP+3:
               if (SHORT2FROMMP(mp1)==CN_ENTER)
               {
                  if (((PNOTIFYRECORDENTER)mp2)->pRecord)
                  {
                     pLookupPar->ulSelected=((PLOOKUPRECORD)((PNOTIFYRECORDENTER)mp2)->pRecord)->ulCounter;
                     WinDismissDlg(parent, DID_OK);
                  }
               }
               break;

            case IDD_LOOKUP+5:
               if (SHORT2FROMMP(mp1)==BN_CLICKED ||
                   SHORT2FROMMP(mp1)==BN_DBLCLICKED)
               {
                  BOOL bBrief;

                  bBrief=WinQueryButtonCheckstate(parent, IDD_LOOKUP+5);

                  pFieldInfo=(PFIELDINFO)WinSendDlgItemMsg(parent, IDD_LOOKUP+3,
                                          CM_QUERYDETAILFIELDINFO,
                                          NULL, MPFROMSHORT(CMA_FIRST));

                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  /* Location, Phone */
                  if (bBrief)
                     pFieldInfo->flData |= CFA_INVISIBLE;
                  else
                     pFieldInfo->flData &= ~CFA_INVISIBLE;
                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  if (bBrief)
                     pFieldInfo->flData |= CFA_INVISIBLE;
                  else
                     pFieldInfo->flData &= ~CFA_INVISIBLE;
                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  /* Baud */
                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  if (bBrief)
                     pFieldInfo->flData |= CFA_INVISIBLE;
                  else
                     pFieldInfo->flData &= ~CFA_INVISIBLE;
                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  /* Callcost, Usercost */
                  if (bBrief)
                     pFieldInfo->flData |= CFA_INVISIBLE;
                  else
                     pFieldInfo->flData &= ~CFA_INVISIBLE;
                  pFieldInfo=pFieldInfo->pNextFieldInfo;
                  if (bBrief)
                     pFieldInfo->flData |= CFA_INVISIBLE;
                  else
                     pFieldInfo->flData &= ~CFA_INVISIBLE;
                  WinSendDlgItemMsg(parent, IDD_LOOKUP+3,
                                    CM_INVALIDATEDETAILFIELDINFO,
                                    NULL, NULL);
               }
               break;

            default:
               break;
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         CleanupLookup(WinWindowFromID(parent, IDD_LOOKUP+3));
         QueryWinPos(parent, &windowpositions.lookuppos);
         QueryFont(WinWindowFromID(parent, IDD_LOOKUP+3), windowfonts.lookupfont);
         QueryForeground(WinWindowFromID(parent, IDD_LOOKUP+3), &windowcolors.lookupfore);
         QueryBackground(WinWindowFromID(parent, IDD_LOOKUP+3), &windowcolors.lookupback);
         bTemp=WinQueryButtonCheckstate(parent, IDD_LOOKUP+5);
         if (lookupoptions.bBrief != bTemp)
         {
            lookupoptions.bBrief = bTemp;
            dirtyflags.lookupdirty=TRUE;
         }
         WinSendDlgItemMsg(parent, IDD_LOOKUP+3, CM_QUERYCNRINFO,
                           &CnrInfo, MPFROMLONG(sizeof(CNRINFO)));

         if (lookupoptions.lSplitBar != CnrInfo.xVertSplitbar)
         {
            lookupoptions.lSplitBar = CnrInfo.xVertSplitbar;
            dirtyflags.lookupdirty=TRUE;
         }
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: CleanupLookup                                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Raeumt den Container fuer den Lookup auf                    º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: hwndCnr: Window-Handle des Containers                          º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: keine                                                      º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static void CleanupLookup(HWND hwndCnr)
{
   PLOOKUPRECORD pRecord;

   while(pRecord=(PLOOKUPRECORD)SendMsg(hwndCnr, CM_QUERYRECORD,
                                  NULL, MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER)))
   {
      free(pRecord->pchAddress);
      free(pRecord->pchFlags);
      SendMsg(hwndCnr, CM_REMOVERECORD, &pRecord,
                 MPFROM2SHORT(1, CMA_FREE));
   }

   return;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: LookupNodelists                                            º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Durchsucht die Nodelisten nach dem gesuchten Namen, liefert º
 º               die Eintraege zurueck                                       º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pchName: Sysop-Name                                            º
 º            pDomains: Domain-Liste                                         º
 º            ppResults: Erzeugtes Ergebnis-Array                            º
 º            errdomain: Domain-Name bei Fehler                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: 0  OK, nichts gefunden                                     º
 º                >0 Anzahl der gefundenen Eintr„ge                          º
 º                -1 Fehler beim Oeffenen des Index-Files                    º
 º                -2 Fehler beim Oeffenen des Daten-Files                    º
 º                -3 Fehler beim Lesen des Index-Files                       º
 º                -4 Fehler beim Lesen des Daten-Files                       º
 º                -5 Kein Speicher                                           º
 º                -6 Ungltiges Handle                                       º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

int LookupNodelists(char *pchName, PDOMAINS pDomain, PNODEDATA *ppResults, char *errdomain)
{
   int foundentries=0;
   HV7LOOKUP hLookup;
   NODEDATA NodeData;
   ULONG rc;
   char *p1, *p2;
   char pchNLName[LEN_USERNAME+5];
   PDOMAINS pTemp;
   PNODEDATALIST pNodeDataList=NULL, pThisNode=NULL;

   if (strlen(pchName)<3)
      return 0;

   p1=pchName;
   while(*p1)   /* Ende des Strings suchen */
      p1++;

   while(p1 >= pchName && *p1 != ' ')  /* letztes Leerzeichen suchen */
      p1--;

   if (p1 >= pchName) /* Leerzeichen gefunden */
   {
      *p1='\0';

      p1++;

      strcpy(pchNLName, p1);
      strcat(pchNLName, ", ");

      p2=pchName;
      while(*p2 && *p2==' ')  /* Anfang des Vornamens suchen */
         p2++;

      strcat(pchNLName, p2);
   }
   else
      strcpy(pchNLName, pchName);


   pTemp=pDomain;

   while(pTemp)
   {
      if (pTemp->indexfile[0] && pTemp->nodelistfile[0])
      {
         switch(FLTV7OpenSearch(&hLookup, pchNLName, pTemp->indexfile,
                                pTemp->nodelistfile, V7SEARCHTYPE_NAME))
         {
            case V7ERR_IDXOPENERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -1;

            case V7ERR_DATOPENERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -2;

            case V7ERR_IDXREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -3;

            case V7ERR_DATREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -4;

            case V7ERR_NOMEM:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -5;

            case V7ERR_INVHANDLE:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -6;

            default:
               break;
         }
         while (!(rc=FLTV7SearchNext(hLookup, &NodeData)))
         {
            /* Eintrag gefunden */
            if (pNodeDataList==NULL)
            {
               pNodeDataList=malloc(sizeof(NODEDATALIST));
               memcpy(&pNodeDataList->NodeData, &NodeData, sizeof(NODEDATA));
               pNodeDataList->next=NULL;
               pThisNode=pNodeDataList;
            }
            else
            {
               pThisNode->next=malloc(sizeof(NODEDATALIST));
               pThisNode=pThisNode->next;
               memcpy(&pThisNode->NodeData, &NodeData, sizeof(NODEDATA));
               pThisNode->next=NULL;
            }
            foundentries++;
         }
         switch(rc)
         {
            case V7ERR_IDXREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               foundentries=-3;
               break;

            case V7ERR_DATREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               foundentries=-4;
               break;

            case V7ERR_INVHANDLE:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               foundentries=-6;
               break;

            default:
               break;
         }
         FLTV7CloseSearch(hLookup);
         if (foundentries <0)
         {
            /* Ergebnisse verwerfen */
            while(pNodeDataList)
            {
               pThisNode=pNodeDataList->next;
               free(pNodeDataList);
               pNodeDataList=pThisNode;
            }
            break;
         }
      }
      pTemp=pTemp->next;
   }

   if (foundentries > 0)
   {
      ULONG i=0;

      *ppResults=malloc(sizeof(NODEDATA) * foundentries);
      while(pNodeDataList)
      {
         memcpy(&(*ppResults)[i], &pNodeDataList->NodeData, sizeof(NODEDATA));
         pThisNode=pNodeDataList->next;
         free(pNodeDataList);
         pNodeDataList=pThisNode;
         i++;
      }
   }
   else
      *ppResults=NULL;

   return foundentries;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: LookupAddress                                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Durchsucht die Nodelisten nach der gesuchten Adresse,       º
 º               liefert die Eintraege zurueck                               º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pchAddress: gesuchte Adresse                                   º
 º            pDomains: Domain-Liste                                         º
 º            ppResults: Erzeugtes Ergebnis-Array                            º
 º            errdomain: Domain-Name bei Fehler                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: 0  OK, nichts gefunden                                     º
 º                >0 Anzahl der gefundenen Eintr„ge                          º
 º                -1 Fehler beim Oeffenen des Index-Files                    º
 º                -2 Fehler beim Oeffenen des Daten-Files                    º
 º                -3 Fehler beim Lesen des Index-Files                       º
 º                -4 Fehler beim Lesen des Daten-Files                       º
 º                -5 Kein Speicher                                           º
 º                -6 Ungltiges Handle                                       º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

int LookupAddress(char *pchAddress, PDOMAINS pDomain, PNODEDATA *ppResults, char *errdomain)
{
   int foundentries=0;
   HV7LOOKUP hLookup;
   NODEDATA NodeData;
   ULONG rc;
   PDOMAINS pTemp;
   PNODEDATALIST pNodeDataList=NULL, pThisNode=NULL;

   if (!*pchAddress)
      return 0;

   pTemp=pDomain;

   while(pTemp)
   {
      if (pTemp->nodelistfile[0])
      {
         /* Index-Dateiname erzeugen */
         char drive[_MAX_DRIVE];
         char   dir[_MAX_DIR];
         char fname[_MAX_FNAME];
         char   ext[_MAX_EXT];
         char indexfile[LEN_PATHNAME+1];

         _splitpath(pTemp->nodelistfile, drive, dir, fname, ext);
         strcpy(ext, ".NDX");
         _makepath(indexfile, drive, dir, fname, ext);

         switch(FLTV7OpenSearch(&hLookup, pchAddress, indexfile,
                                pTemp->nodelistfile, V7SEARCHTYPE_ADDRESS))
         {
            case V7ERR_IDXOPENERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -1;

            case V7ERR_DATOPENERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -2;

            case V7ERR_IDXREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -3;

            case V7ERR_DATREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -4;

            case V7ERR_NOMEM:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -5;

            case V7ERR_INVHANDLE:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               return -6;

            default:
               break;
         }
         if (!(rc=FLTV7SearchNext(hLookup, &NodeData)))
         {
            /* Eintrag gefunden */
            if (pNodeDataList==NULL)
            {
               pNodeDataList=malloc(sizeof(NODEDATALIST));
               memcpy(&pNodeDataList->NodeData, &NodeData, sizeof(NODEDATA));
               pNodeDataList->next=NULL;
               pThisNode=pNodeDataList;
            }
            else
            {
               pThisNode->next=malloc(sizeof(NODEDATALIST));
               pThisNode=pThisNode->next;
               memcpy(&pThisNode->NodeData, &NodeData, sizeof(NODEDATA));
               pThisNode->next=NULL;
            }

            foundentries++;
         }
         switch(rc)
         {
            case V7ERR_IDXREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               foundentries=-3;
               break;

            case V7ERR_DATREADERR:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               foundentries=-4;
               break;

            case V7ERR_INVHANDLE:
               strcpy(errdomain, pTemp->domainname);
               FLTV7CloseSearch(hLookup);
               foundentries=-6;
               break;

            default:
               break;
         }
         FLTV7CloseSearch(hLookup);
         if (foundentries <0)
         {
            /* Ergebnisse verwerfen */
            while(pNodeDataList)
            {
               pThisNode=pNodeDataList->next;
               free(pNodeDataList);
               pNodeDataList=pThisNode;
            }
            break;
         }
      }
      pTemp=pTemp->next;
   }

   if (foundentries > 0)
   {
      ULONG i=0;

      *ppResults=malloc(sizeof(NODEDATA) * foundentries);
      while(pNodeDataList)
      {
         memcpy(&(*ppResults)[i], &pNodeDataList->NodeData, sizeof(NODEDATA));
         pThisNode=pNodeDataList->next;
         free(pNodeDataList);
         pNodeDataList=pThisNode;
         i++;
      }
   }
   else
      *ppResults=NULL;

   return foundentries;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: NewLookupContainerProc                                     º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Neue Window-Procedure f. Container (wg. OS/2-Bug)           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: (Window-Procedure)                                             º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: MRESULT                                                    º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static MRESULT EXPENTRY NewLookupContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
      case DM_DRAGOVER:
         DrgAccessDraginfo(mp1);
         break;

      default:
         break;
   }
   return OldLookupContainerProc(parent, message, mp1, mp2);
}

char *NLFlagsToString(PNODEDATA pNodeData, PCHAR pchFlags)
{
   pchFlags[0]='\0';
   if (pNodeData->isZC)
      strcat(pchFlags, "ZC");
   if (pNodeData->isRC)
   {
      if (pchFlags[0])
         strcat(pchFlags, ", ");
      strcat(pchFlags, "RC");
   }
   if (pNodeData->isCM)
   {
      if (pchFlags[0])
         strcat(pchFlags, ", ");
      strcat(pchFlags, "CM");
   }
   if (pNodeData->isMO)
   {
      if (pchFlags[0])
         strcat(pchFlags, ", ");
      strcat(pchFlags, "MO");
   }
   if (pNodeData->isHost)
   {
      if (pchFlags[0])
         strcat(pchFlags, ", ");
      strcat(pchFlags, "Host");
   }
   if (pNodeData->isHub)
   {
      if (pchFlags[0])
         strcat(pchFlags, ", ");
      strcat(pchFlags, "Hub");
   }
   return pchFlags;
}

char *NLModemToString(ULONG ulModemType, PCHAR pchModem)
{
   int iType;
   ULONG ulMask;
   extern NODELISTOPT nodelist;

   pchModem[0]='\0';

   if (nodelist.ulOptions & MODEMFLAGS_BYTETYPE)
   {
      if (ulModemType >0 && ulModemType <= MAX_MODEMTYPES_BYTE)
         strcpy(pchModem, nodelist.bytetypes[ulModemType-1]);
   }
   else
      for (iType=0, ulMask=1; iType<MAX_MODEMTYPES; iType++, ulMask= ulMask<<1)
      {
         if (ulModemType & ulMask)
         {
            if (pchModem[0])
               strcat(pchModem, ", ");
            strcat(pchModem, nodelist.modemtype[iType]);
         }
      }

   return pchModem;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PerformNameLookup                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt einen kompletten Namens-Lookup durch                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchSearchName: Gesuchter Name (oder Teilname)                  */
/*            hwndDlg: Owner-Window f. Messages und Dialoge                  */
/*            ulFlags: Flags, s. LOOKUPS.H                                   */
/*            pchFoundName: Gefundener bzw. ausgewaehlter Name               */
/*            pchFoundAddress: Gefundene bzw. ausgewaehlte Adresse           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE:  Gefunden und/oder ausgew„hlt                        */
/*                FALSE: nicht gefunden bzw. Auswahl abgebrochen             */
/*                       (pchFoundName und pchFoundAddress werden nicht      */
/*                        veraendert).                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL PerformNameLookup(char *pchSearchName, HWND hwndDlg, ULONG ulFlags,
                       char *pchFoundName, char *pchFoundAddress)
{
   extern PDOMAINS domains;
   PNODEDATA pFoundNodes=NULL;
   char pchErrDomain[LEN_DOMAIN+1]="";
   int countNodes=0;
   char *pchName;
   BOOL bSuccess=FALSE;

   if (domains && pchSearchName[0])
   {
      pchName=strdup(pchSearchName);

      switch(countNodes=LookupNodelists(pchName, domains, &pFoundNodes, pchErrDomain))
      {
         LOOKUPPAR LookupPar;

         /* Nix gefunden */
         case 0:
            break;

         case -1:
         case -2:
         case -3:
         case -4:
         case -5:
         case -6:
            LookupErrorMessage(hwndDlg, pchErrDomain, countNodes);
            break;

         /* eine oder mehrere Adressen */
         default:
            if (countNodes == 1 && !(ulFlags & LOOKUP_FORCESELECT))
            {
               /* sofort einfgen */
               NetAddrToString(pchFoundAddress, &pFoundNodes[0].Address);
               strcpy(pchFoundName, pFoundNodes[0].SysopName);
               bSuccess = TRUE;
            }
            else
            {
               /* Auswahldialog */
               LookupPar.cb=sizeof(LOOKUPPAR);
               strcpy(LookupPar.pchName, pchSearchName);
               LookupPar.pNodes=pFoundNodes;
               LookupPar.iCountNodes=countNodes;

               if (WinDlgBox(HWND_DESKTOP, hwndDlg, LookupProc,
                             hmodLang, IDD_LOOKUP, &LookupPar)==DID_OK)
               {
                  NetAddrToString(pchFoundAddress, &pFoundNodes[LookupPar.ulSelected].Address);
                  strcpy(pchFoundName, pFoundNodes[LookupPar.ulSelected].SysopName);
                  bSuccess = TRUE;
               }
            }
            break;
      }
      if (pFoundNodes)
         free(pFoundNodes);
      free(pchName);
   }
   return bSuccess;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PerformNodeLookup                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt einen kompletten Nodenummern-Lookup durch            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchSearchAddress: Gesuchte Adresse                             */
/*            hwndDlg: Owner-Window f. Messages und Dialoge                  */
/*            pchFoundName: Gefundener Name                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: TRUE:  Gefunden                                            */
/*                FALSE: nicht gefunden (pchFoundName wird nicht ver„ndert)  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL PerformNodeLookup(char *pchSearchAddress, HWND hwndDlg, char *pchFoundName)
{
   extern PDOMAINS domains;
   PNODEDATA pFoundNodes=NULL;
   char pchErrDomain[LEN_DOMAIN+1]="";
   int countNodes=0;
   BOOL bSuccess=FALSE;

   if (domains && pchSearchAddress[0])
   {
      switch (countNodes = LookupAddress(pchSearchAddress, domains, &pFoundNodes, pchErrDomain))
      {
         /* Nix gefunden */
         case 0:
            break;

         case -1:
         case -2:
         case -3:
         case -4:
         case -5:
         case -6:
            LookupErrorMessage(hwndDlg, pchErrDomain, countNodes);
            break;

         default:
            strcpy(pchFoundName, pFoundNodes[0].SysopName);
            bSuccess = TRUE;
            break;
      }
      if (pFoundNodes)
         free(pFoundNodes);
   }
   return bSuccess;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: LookupErrorMessage                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Zeigt eine Lookup-Fehlermeldung an                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndOwner: Owner-Window fuer die Message-Box                   */
/*            pchErrDomain: Domain, in dem der Fehler aufgetreten ist        */
/*            iErrNr: Fehler-Nummer (-1 .. -6)                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*---------------------------------------------------------------------------*/

static void LookupErrorMessage(HWND hwndOwner, char *pchErrDomain, int iErrNr)
{
   char rawmsg[200];
   char message[300];

   switch (iErrNr)
   {
      case -1:
         LoadString(IDST_MSG_IDXOPENERR, 200, rawmsg);
         sprintf(message, rawmsg, pchErrDomain);
         WinMessageBox(HWND_DESKTOP, hwndOwner, message, NULL, IDD_IDXOPENERR,
                       MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
         break;

      case -2:
         LoadString(IDST_MSG_DATOPENERR, 200, rawmsg);
         sprintf(message, rawmsg, pchErrDomain);
         WinMessageBox(HWND_DESKTOP, hwndOwner, message, NULL, IDD_DATOPENERR,
                       MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
         break;

      case -3:
         LoadString(IDST_MSG_IDXREADERR, 200, rawmsg);
         sprintf(message, rawmsg, pchErrDomain);
         WinMessageBox(HWND_DESKTOP, hwndOwner, message, NULL, IDD_IDXREADERR,
                       MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
         break;

      case -4:
         LoadString(IDST_MSG_DATREADERR, 200, rawmsg);
         sprintf(message, rawmsg, pchErrDomain);
         WinMessageBox(HWND_DESKTOP, hwndOwner, message, NULL, IDD_DATREADERR,
                       MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
         break;

      case -5:
         LoadString(IDST_MSG_LOOKUPMEM, 200, rawmsg);
         sprintf(message, rawmsg, pchErrDomain);
         WinMessageBox(HWND_DESKTOP, hwndOwner, message, NULL, IDD_LOOKUPMEM,
                       MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
         break;

      case -6:
         MessageBox(hwndOwner, IDST_MSG_LOOKUPHANDLE, 0, IDD_LOOKUPHANDLE,
                    MB_OK | MB_HELP);
         break;

      default:
         break;
   }
   return;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Modulende ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

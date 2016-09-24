/*---------------------------------------------------------------------------+
 | Titel: NLBROWSER.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 20.10.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Nodelist-Browser f. FleetStreet                                       |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "fltv7\fltv7.h"
#include "fltv7\fltv7structs.h"
#include "fltv7\v7browse.h"
#include "utility.h"
#include "lookups.h"
#include "nodedrag.h"
#include "nlbrowser.h"
#include "dump\expt.h"

/*--------------------------------- Defines ---------------------------------*/

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L
#endif


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
         } NODERECORD, *PNODERECORD;

typedef struct {
            MINIRECORDCORE RecordCore;
            PNODEINDEX     pStartIndex;
            PNAMEINDEX     pNStartIndex;
            char           pchIndexText[100];
         } INDEXRECORD, *PINDEXRECORD;

typedef struct {
            BOOL         bNotify;
            BOOL         bNoPoints;
            ULONG        ulCurrentMode;
            char         pchCurrentDomain[LEN_DOMAIN+1];
            BOOL         bIcons;
            HPOINTER     hptrFolder;
            HPOINTER     hptrPlus;
            HPOINTER     hptrMinus;
            HPOINTER     hptrOneNode;
            HSWITCH      hSwitch;
            PINDEXRECORD pCurrentIndex;
            char         pchTitleSysop[50];
            char         pchTitleAddress[50];
            char         pchTitleSystem[50];
            char         pchTitleLocation[50];
            char         pchTitlePhone[50];
            char         pchTitleModem[50];
            char         pchTitleBaud[50];
            char         pchTitleCallcost[50];
            char         pchTitleUsercost[50];
            char         pchTitleFlags[50];
            PNODEBROWSE  pNodeBrowse;
            PNAMEBROWSE  pNameBrowse;
            LONG         lMinX;
            LONG         lMinY;
            char         pchICnrTitle[100];
            char         pchNCnrTitle[100];
            ULONG        ulNumNodes;
            PNODEDATA    pNodeData;
         } NLBROWSERDATA, *PNLBROWSERDATA;

typedef struct {
             HWND           hwnd;
             PNLBROWSERDATA pBrowserData;
             PDOMAINS       pDomain;
         } THREADDATA, *PTHREADDATA;

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HWND client;
extern HMODULE hmodLang;
extern HWND hwndhelp;
extern BROWSEROPTIONS BrowserOptions;
extern PDOMAINS domains;
extern BOOL bDoingBrowse;

#if 0
static PFNWP OldContainerProc;
#endif

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int SwitchBrowser(HWND hwnd, PNLBROWSERDATA pBrowserData);
static int CloseOldBrowsemode(HWND hwnd, PNLBROWSERDATA pBrowserData);
static void _Optlink OpenNodeBrowseThread(PVOID pParam);
static void _Optlink OpenNameBrowseThread(PVOID pParam);
static void NodeIndexReady(HWND hwnd, PNLBROWSERDATA pBrowserData);
static void NameIndexReady(HWND hwnd, PNLBROWSERDATA pBrowserData);
static void TitleWork(HWND hwnd, PNLBROWSERDATA pBrowserData);
static void _Optlink ReadNetDataThread(PVOID pParam);
static void _Optlink ReadNameDataThread(PVOID pParam);
static void CleanupNodeContainer(HWND hwnd, PNLBROWSERDATA pBrowserData);
static void OpenIndex(HWND hwnd, PNLBROWSERDATA pBrowserData, PINDEXRECORD pIndexRecord, BOOL bForce);
static void NodeDataReady(HWND hwnd, PNLBROWSERDATA pBrowserData);
static void InitNodeDrag(HWND hwndDlg, PCNRDRAGINIT pInit, PNLBROWSERDATA pBrowserData);
#if 0
static MRESULT EXPENTRY NewContainerProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


static MRESULT EXPENTRY NewContainerProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      case WM_BUTTON2DOWN:
         return (MRESULT) TRUE;

      default:
         break;
   }
   return OldContainerProc(hwnd, msg, mp1, mp2);
}
#endif

/*---------------------------------------------------------------------------*/
/* Funktionsname: NLBrowserProc                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fensterprozedur f. Nodelist-Browser                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY NLBrowserProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PNLBROWSERDATA pBrowserData = (PNLBROWSERDATA) WinQueryWindowULong(hwnd, QWL_USER);
   CNRINFO cnrinfo;
   PFIELDINFO pFieldInfo, pFirstFieldInfo;
   FIELDINFOINSERT FieldInfoInsert;
   PDOMAINS pDomain;
   MRESULT mrbuffer;
   SWP swp;
   static ULONG MinorVersion=0;

   switch(msg)
   {
      case WM_INITDLG:
         pBrowserData = calloc(1, sizeof(NLBROWSERDATA));
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pBrowserData);

         strcpy(pBrowserData->pchCurrentDomain, BrowserOptions.pchLastDomain);
         pBrowserData->ulCurrentMode = BrowserOptions.ulLastMode;
         pBrowserData->bIcons =        BrowserOptions.bIcons;
         pBrowserData->bNoPoints =     BrowserOptions.bNoPoints;
         WinCheckButton(hwnd, IDD_NLBROWSER+10, pBrowserData->bNoPoints);

         /* Switch-Listen-Eintrag */
         pBrowserData->hSwitch=AddToWindowList(hwnd);

         /* Icons */
         pBrowserData->hptrFolder= LoadIcon(IDIC_NETFOLDER);
         pBrowserData->hptrOneNode= LoadIcon(IDIC_ONENODE);

         /* OS/2 3.0 and below: replace tree icons */
         DosQuerySysInfo(QSV_VERSION_MINOR, QSV_VERSION_MINOR, &MinorVersion,
                         sizeof(MinorVersion));

         if (MinorVersion < 40)
         {
            pBrowserData->hptrPlus=LoadIcon(IDIC_PLUS);
            pBrowserData->hptrMinus=LoadIcon(IDIC_MINUS);
         }

         SendMsg(hwnd, WM_SETICON, (MPARAM) pBrowserData->hptrFolder, NULL);

         /* Index-Container initialisieren */
         cnrinfo.cb = sizeof(CNRINFO);
         cnrinfo.flWindowAttr = CV_TREE | CA_DRAWICON | CA_TREELINE |
                                CA_CONTAINERTITLE | CA_TITLEREADONLY | CA_TITLESEPARATOR;
         if (pBrowserData->bIcons)
         {
            WinCheckButton(hwnd, IDD_NLBROWSER+9, TRUE);
            cnrinfo.flWindowAttr |= CV_ICON;
         }
         else
         {
            WinCheckButton(hwnd, IDD_NLBROWSER+9, FALSE);
            cnrinfo.flWindowAttr |= CV_TEXT;
         }
         if (MinorVersion < 40)
         {
            cnrinfo.hptrCollapsed = pBrowserData->hptrPlus;
            cnrinfo.hptrExpanded = pBrowserData->hptrMinus;
         }
         cnrinfo.pszCnrTitle = pBrowserData->pchICnrTitle;

         WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_FLWINDOWATTR |
                                      ((MinorVersion<40)?CMA_TREEICON:0) |
                                      CMA_CNRTITLE));

         /* Node-Container initialisieren */
#if 0
         OldContainerProc=WinSubclassWindow(WinWindowFromID(WinWindowFromID(hwnd, IDD_NLBROWSER+2), CID_LEFTDVWND), NewContainerProc);
#endif
         pFirstFieldInfo = WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_ALLOCDETAILFIELDINFO,
                                             MPFROMSHORT(10), NULL);
         pFieldInfo = pFirstFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_ADDRESS, 50, pBrowserData->pchTitleAddress);
         pFieldInfo->pTitleData= pBrowserData->pchTitleAddress;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, pchAddress);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_SYSOP, 50, pBrowserData->pchTitleSysop);
         pFieldInfo->pTitleData= pBrowserData->pchTitleSysop;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, pchSysop);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_SYSTEM, 50, pBrowserData->pchTitleSystem);
         pFieldInfo->pTitleData= pBrowserData->pchTitleSystem;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, pchSystemName);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_LOCATION, 50, pBrowserData->pchTitleLocation);
         pFieldInfo->pTitleData= pBrowserData->pchTitleLocation;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, pchLocation);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_PHONE, 50, pBrowserData->pchTitlePhone);
         pFieldInfo->pTitleData= pBrowserData->pchTitlePhone;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, pchPhone);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_MODEM, 50, pBrowserData->pchTitleModem);
         pFieldInfo->pTitleData= pBrowserData->pchTitleModem;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, pchModem);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_ULONG | CFA_HORZSEPARATOR | CFA_SEPARATOR |
                            CFA_RIGHT;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_BAUD, 50, pBrowserData->pchTitleBaud);
         pFieldInfo->pTitleData= pBrowserData->pchTitleBaud;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, ulBaud);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_ULONG | CFA_HORZSEPARATOR | CFA_SEPARATOR | CFA_RIGHT;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_CALLCOST, 50, pBrowserData->pchTitleCallcost);
         pFieldInfo->pTitleData= pBrowserData->pchTitleCallcost;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, ulCallCost);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_ULONG | CFA_HORZSEPARATOR | CFA_SEPARATOR | CFA_RIGHT;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_USERCOST, 50, pBrowserData->pchTitleUsercost);
         pFieldInfo->pTitleData= pBrowserData->pchTitleUsercost;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, ulUserCost);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         LoadString(IDST_LU_FLAGS, 50, pBrowserData->pchTitleFlags);
         pFieldInfo->pTitleData= pBrowserData->pchTitleFlags;
         pFieldInfo->offStruct= FIELDOFFSET(NODERECORD, pchFlags);

         /* Felder des Containers einfuegen */
         FieldInfoInsert.cb=sizeof(FIELDINFOINSERT);
         FieldInfoInsert.pFieldInfoOrder=(PFIELDINFO) CMA_FIRST;
         FieldInfoInsert.fInvalidateFieldInfo=TRUE;
         FieldInfoInsert.cFieldInfoInsert=10;

         WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_INSERTDETAILFIELDINFO,
                           pFirstFieldInfo, &FieldInfoInsert);

         cnrinfo.cb = sizeof(CNRINFO);
         cnrinfo.flWindowAttr = CV_DETAIL| CA_DETAILSVIEWTITLES |
                                CA_CONTAINERTITLE | CA_TITLEREADONLY | CA_TITLESEPARATOR;
         cnrinfo.pFieldInfoLast = pFirstFieldInfo->pNextFieldInfo;
         cnrinfo.xVertSplitbar  = BrowserOptions.lSplitbar;
         cnrinfo.pszCnrTitle = pBrowserData->pchNCnrTitle;

         WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_FLWINDOWATTR | CMA_PFIELDINFOLAST |
                                      CMA_CNRTITLE | CMA_XVERTSPLITBAR));

         /* Listbox mit Domains initialisieren */
         pDomain = domains;
         while (pDomain)
         {
            SHORT sItem;

            sItem = (SHORT) WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+6, LM_INSERTITEM,
                                              MPFROMSHORT(LIT_SORTASCENDING), pDomain->domainname);
            /* leeren Default durch ersten Domain ersetzen */
            if (!pBrowserData->pchCurrentDomain[0])
               strcpy(pBrowserData->pchCurrentDomain, pDomain->domainname);

            /* letzten Domain selektieren */
            if (!stricmp(pDomain->domainname, pBrowserData->pchCurrentDomain))
               WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+6, LM_SELECTITEM,
                                 MPFROMSHORT(sItem), MPFROMLONG(TRUE));

            pDomain = pDomain->next;
         }

         /* Radio-Buttons */
         if (pBrowserData->ulCurrentMode == BROWSEMODE_NODE)
            WinCheckButton(hwnd, IDD_NLBROWSER+7, TRUE);
         else
            WinCheckButton(hwnd, IDD_NLBROWSER+8, TRUE);

         /* minimale Fenstergroessen */
         WinQueryWindowPos(WinWindowFromID(hwnd, IDD_NLBROWSER+2),
                           &swp);
         pBrowserData->lMinX = swp.x + swp.cx +2;
         pBrowserData->lMinY = swp.y + 2 * WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);

         RestoreWinPos(hwnd, &BrowserOptions.BrowserPos, TRUE, TRUE);
         pBrowserData->bNotify = TRUE;

         SwitchBrowser(hwnd, pBrowserData);
         SetInitialAccel(hwnd);
         break;

      case WM_DESTROY:
         CloseOldBrowsemode(hwnd, pBrowserData);

         WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_QUERYCNRINFO, &cnrinfo,
                           MPFROMLONG(sizeof(cnrinfo)));
         if (BrowserOptions.lSplitbar != cnrinfo.xVertSplitbar)
         {
            extern DIRTYFLAGS dirtyflags;

            BrowserOptions.lSplitbar = cnrinfo.xVertSplitbar;
            dirtyflags.browserdirty = TRUE;
         }
         if (BrowserOptions.bIcons != pBrowserData->bIcons)
         {
            extern DIRTYFLAGS dirtyflags;

            BrowserOptions.bIcons = pBrowserData->bIcons;
            dirtyflags.browserdirty = TRUE;
         }
         if (BrowserOptions.bNoPoints != pBrowserData->bNoPoints)
         {
            extern DIRTYFLAGS dirtyflags;

            BrowserOptions.bNoPoints = pBrowserData->bNoPoints;
            dirtyflags.browserdirty = TRUE;
         }
         if (BrowserOptions.ulLastMode != pBrowserData->ulCurrentMode)
         {
            extern DIRTYFLAGS dirtyflags;

            BrowserOptions.ulLastMode = pBrowserData->ulCurrentMode;
            dirtyflags.browserdirty = TRUE;
         }
         if (!BrowserOptions.pchLastDomain[0] ||
             strcmp(BrowserOptions.pchLastDomain, pBrowserData->pchCurrentDomain))
         {
            extern DIRTYFLAGS dirtyflags;

            strcpy(BrowserOptions.pchLastDomain, pBrowserData->pchCurrentDomain);
            dirtyflags.browserdirty = TRUE;
         }

         if (pBrowserData->hptrFolder)
            WinDestroyPointer(pBrowserData->hptrFolder);
         if (pBrowserData->hptrOneNode)
            WinDestroyPointer(pBrowserData->hptrOneNode);

         if (MinorVersion < 40)
         {
            if (pBrowserData->hptrPlus)
               WinDestroyPointer(pBrowserData->hptrPlus);
            if (pBrowserData->hptrMinus)
               WinDestroyPointer(pBrowserData->hptrMinus);
         }
         RemoveFromWindowList(pBrowserData->hSwitch);
         free(pBrowserData);
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2) == CMDSRC_ACCELERATOR)
            return RedirectCommand(mp1, mp2);

         if (!WinIsWindowEnabled(WinWindowFromID(hwnd, IDD_NLBROWSER+3)))
            return (MRESULT) FALSE;

         WinPostMsg(client, BRSM_CLOSE, NULL, NULL);
         break;

      case WM_CONTROL:
         switch (SHORT1FROMMP(mp1))
         {
            case IDD_NLBROWSER+1:
               if (SHORT2FROMMP(mp1) == CN_ENTER)
               {
                  PNOTIFYRECORDENTER pEnter = (PNOTIFYRECORDENTER) mp2;

                  if (!bDoingBrowse && pEnter && pEnter->pRecord)
                     OpenIndex(hwnd, pBrowserData, (PINDEXRECORD) pEnter->pRecord, FALSE);
               }
               break;

            case IDD_NLBROWSER+2:
               if (SHORT2FROMMP(mp1) == CN_INITDRAG)
               {
                  InitNodeDrag(hwnd, (PCNRDRAGINIT) mp2, pBrowserData);
               }
               break;

            case IDD_NLBROWSER+9:
               if (SHORT2FROMMP(mp1) == BN_CLICKED ||
                   SHORT2FROMMP(mp1) == BN_DBLCLICKED )
               {
                  pBrowserData->bIcons = WinQueryButtonCheckstate(hwnd, IDD_NLBROWSER+9);

                  WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_QUERYCNRINFO, &cnrinfo,
                                    MPFROMLONG(sizeof(cnrinfo)));
                  if (pBrowserData->bIcons)
                  {
                     cnrinfo.flWindowAttr &= ~CV_TEXT;
                     cnrinfo.flWindowAttr |= CV_ICON;
                  }
                  else
                  {
                     cnrinfo.flWindowAttr &= ~CV_ICON;
                     cnrinfo.flWindowAttr |= CV_TEXT;
                  }
                  WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_SETCNRINFO, &cnrinfo,
                                    MPFROMLONG(CMA_FLWINDOWATTR));
               }
               break;

            case IDD_NLBROWSER+10:
               if (SHORT2FROMMP(mp1) == BN_CLICKED ||
                   SHORT2FROMMP(mp1) == BN_DBLCLICKED )
               {
                  if (WinQueryButtonCheckstate(hwnd, IDD_NLBROWSER+10))
                     pBrowserData->bNoPoints = TRUE;
                  else
                     pBrowserData->bNoPoints = FALSE;
                  if (pBrowserData->pCurrentIndex)
                     OpenIndex(hwnd, pBrowserData, pBrowserData->pCurrentIndex, TRUE);
               }
               break;

            case IDD_NLBROWSER+6:
               if (SHORT2FROMMP(mp1) == CBN_ENTER)
               {
                  char pchTemp[LEN_DOMAIN+1];
                  SHORT sItem;

                  sItem = (SHORT) WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+6, LM_QUERYSELECTION,
                                                    MPFROMSHORT(LIT_FIRST), NULL);
                  if (sItem >= 0)
                  {
                     WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+6, LM_QUERYITEMTEXT,
                                       MPFROM2SHORT(sItem, LEN_DOMAIN+1),
                                       pchTemp);

                     if (stricmp(pBrowserData->pchCurrentDomain, pchTemp))
                     {
                        strcpy(pBrowserData->pchCurrentDomain, pchTemp);
                        SwitchBrowser(hwnd, pBrowserData);
                     }
                  }
               }
               break;

            case IDD_NLBROWSER+7:
            case IDD_NLBROWSER+8:
               if (WinQueryButtonCheckstate(hwnd, IDD_NLBROWSER+7))
               {
                  if (pBrowserData->ulCurrentMode == BROWSEMODE_NAME)
                  {
                     pBrowserData->ulCurrentMode = BROWSEMODE_NODE;
                     SwitchBrowser(hwnd, pBrowserData);
                  }
               }
               else
               {
                  if (pBrowserData->ulCurrentMode == BROWSEMODE_NODE)
                  {
                     pBrowserData->ulCurrentMode = BROWSEMODE_NAME;
                     SwitchBrowser(hwnd, pBrowserData);
                  }
               }
               break;

            default:
               break;
         }
         break;

      case WM_CLOSE:
         if (!WinIsWindowEnabled(WinWindowFromID(hwnd, IDD_NLBROWSER+3)))
            return (MRESULT) FALSE;

         WinPostMsg(client, BRSM_CLOSE, NULL, NULL);
         break;

      case WM_QUERYTRACKINFO:
         WinQueryWindowPos(hwnd, &swp);
         if (swp.fl & SWP_MINIMIZE)
            break;

         mrbuffer = WinDefDlgProc(hwnd, msg, mp1, mp2);
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x = pBrowserData->lMinX;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y = pBrowserData->lMinY;
         return mrbuffer;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
            WinShowWindow(WinWindowFromID(hwnd, IDD_NLBROWSER+3), FALSE);
         if (((PSWP)mp1)->fl & (SWP_MAXIMIZE|SWP_RESTORE))
            WinShowWindow(WinWindowFromID(hwnd, IDD_NLBROWSER+3), TRUE);
         break;

      case WM_ADJUSTFRAMEPOS:
         if (((PSWP)mp1)->fl & (SWP_SIZE|SWP_MINIMIZE|SWP_MAXIMIZE|SWP_RESTORE))
         {
            SWP swp, swp2;

            LONG lHeight = ((PSWP)mp1)->cy - WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR) -
                           2 * WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);

            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_NLBROWSER+1), &swp);

            /* Index-Container */
            WinSetWindowPos(WinWindowFromID(hwnd, IDD_NLBROWSER+1),
                            NULLHANDLE,
                            0, 0,
                            swp.cx, lHeight - swp.y,
                            SWP_SIZE);

            /* Node-Container */
            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_NLBROWSER+2), &swp2);

            WinSetWindowPos(WinWindowFromID(hwnd, IDD_NLBROWSER+2),
                            NULLHANDLE,
                            0, 0,
                            ((PSWP)mp1)->cx - swp.x - swp2.x-1, lHeight - swp.y,
                            SWP_SIZE);
         }
         break;

      case WM_WINDOWPOSCHANGED:
         if (pBrowserData && pBrowserData->bNotify)
         {
            extern DIRTYFLAGS dirtyflags;

            SaveWinPos(hwnd, (PSWP) mp1, &BrowserOptions.BrowserPos, &dirtyflags.browserdirty);
         }
         break;

      case BRSM_INDEX_READY:
         if (pBrowserData->pNodeBrowse)
            NodeIndexReady(hwnd, pBrowserData);
         else
            NameIndexReady(hwnd, pBrowserData);

         /* Controls einschalten */
         WinEnableControl(hwnd, IDD_NLBROWSER+3, TRUE);
         WinEnableControl(hwnd, IDD_NLBROWSER+6, TRUE);
         WinEnableControl(hwnd, IDD_NLBROWSER+7, TRUE);
         WinEnableControl(hwnd, IDD_NLBROWSER+8, TRUE);
         if (pBrowserData->ulCurrentMode == BROWSEMODE_NODE)
            WinEnableControl(hwnd, IDD_NLBROWSER+10, TRUE);
         bDoingBrowse = FALSE;
         break;

      case BRSM_DATA_READY:
         NodeDataReady(hwnd, pBrowserData);
         /* Controls einschalten */
         WinEnableControl(hwnd, IDD_NLBROWSER+3, TRUE);
         WinEnableControl(hwnd, IDD_NLBROWSER+6, TRUE);
         WinEnableControl(hwnd, IDD_NLBROWSER+7, TRUE);
         WinEnableControl(hwnd, IDD_NLBROWSER+8, TRUE);
         if (pBrowserData->ulCurrentMode == BROWSEMODE_NODE)
            WinEnableControl(hwnd, IDD_NLBROWSER+10, TRUE);
         bDoingBrowse = FALSE;
         break;

      case BRSM_INDEX_ERROR:
      case BRSM_DATA_ERROR:
         {
            char pchTemplate[100]="";
            char pchMessage[150]="";

            switch((ULONG) mp1)
            {
               case V7ERR_IDXOPENERR:
                  LoadString(IDST_MSG_IDXOPENERR, sizeof(pchTemplate), pchTemplate);
                  sprintf(pchMessage, pchTemplate, pBrowserData->pchCurrentDomain);
                  WinMessageBox(HWND_DESKTOP, hwnd, pchMessage, NULL, IDD_IDXOPENERR,
                                MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
                  break;

               case V7ERR_DATOPENERR:
                  LoadString(IDST_MSG_DATOPENERR, sizeof(pchTemplate), pchTemplate);
                  sprintf(pchMessage, pchTemplate, pBrowserData->pchCurrentDomain);
                  WinMessageBox(HWND_DESKTOP, hwnd, pchMessage, NULL, IDD_DATOPENERR,
                                MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
                  break;

               case V7ERR_IDXREADERR:
                  LoadString(IDST_MSG_IDXREADERR, sizeof(pchTemplate), pchTemplate);
                  sprintf(pchMessage, pchTemplate, pBrowserData->pchCurrentDomain);
                  WinMessageBox(HWND_DESKTOP, hwnd, pchMessage, NULL, IDD_IDXREADERR,
                                MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
                  break;

               case V7ERR_DATREADERR:
                  LoadString(IDST_MSG_DATREADERR, sizeof(pchTemplate), pchTemplate);
                  sprintf(pchMessage, pchTemplate, pBrowserData->pchCurrentDomain);
                  WinMessageBox(HWND_DESKTOP, hwnd, pchMessage, NULL, IDD_DATREADERR,
                                MB_OK | MB_HELP | MB_MOVEABLE | MB_ERROR);
                  break;

               default:
                  break;
            }

            /* Controls einschalten */
            WinEnableControl(hwnd, IDD_NLBROWSER+3, TRUE);
            WinEnableControl(hwnd, IDD_NLBROWSER+6, TRUE);
            WinEnableControl(hwnd, IDD_NLBROWSER+7, TRUE);
            WinEnableControl(hwnd, IDD_NLBROWSER+8, TRUE);
            WinEnableControl(hwnd, IDD_NLBROWSER+10, TRUE);
            bDoingBrowse = FALSE;
         }
         break;

      case WORKM_SWITCHACCELS:
         SwitchAccels(hwnd, (ULONG) mp1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SwitchBrowser                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Schaltet auf ein anderes Domain oder in einen anderen       */
/*               Browse-Modus                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                1  unbekannter Modus                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int SwitchBrowser(HWND hwnd, PNLBROWSERDATA pBrowserData)
{
   PDOMAINS pDomain = domains;
   static THREADDATA ThreadData;


   /* Domain suchen */
   while (pDomain && stricmp(pDomain->domainname, pBrowserData->pchCurrentDomain))
      pDomain = pDomain->next;

   if (!pDomain)
   {
      return 1;  /* Domain nicht gefunden */
   }

   if (pBrowserData->ulCurrentMode == BROWSEMODE_NODE)
   {
      CloseOldBrowsemode(hwnd, pBrowserData);
      TitleWork(hwnd, pBrowserData);
      pBrowserData->pNodeBrowse = calloc(1, sizeof(NODEBROWSE));

      /* Controls abschalten */
      WinEnableControl(hwnd, IDD_NLBROWSER+3, FALSE);
      WinEnableControl(hwnd, IDD_NLBROWSER+6, FALSE);
      WinEnableControl(hwnd, IDD_NLBROWSER+7, FALSE);
      WinEnableControl(hwnd, IDD_NLBROWSER+8, FALSE);

      ThreadData.hwnd = hwnd;
      ThreadData.pDomain = pDomain;
      ThreadData.pBrowserData = pBrowserData;

      _beginthread(OpenNodeBrowseThread, NULL, 16384, &ThreadData);

      return 0;
   }
   else
      if (pBrowserData->ulCurrentMode == BROWSEMODE_NAME)
      {
         CloseOldBrowsemode(hwnd, pBrowserData);
         TitleWork(hwnd, pBrowserData);
         pBrowserData->pNameBrowse = calloc(1, sizeof(NAMEBROWSE));

         /* Controls abschalten */
         WinEnableControl(hwnd, IDD_NLBROWSER+3, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+6, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+7, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+8, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+10, FALSE);

         ThreadData.hwnd = hwnd;
         ThreadData.pDomain = pDomain;
         ThreadData.pBrowserData = pBrowserData;

         _beginthread(OpenNameBrowseThread, NULL, 16384, &ThreadData);

         return 0;
      }
      else
      {
         return 1; /* unbekannter Modus */
      }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CloseOldBrowsemode                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Beendet das Browsen, leert die Container, gibt Speicher frei*/
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int CloseOldBrowsemode(HWND hwnd, PNLBROWSERDATA pBrowserData)
{
   /* Index-Container leeren */
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_REMOVERECORD, NULL,
                     MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));
   pBrowserData->pCurrentIndex = NULL;

   /* Node-Container leeren */
   CleanupNodeContainer(hwnd, pBrowserData);

   if (pBrowserData->pNodeBrowse)
   {
      FLTV7CloseNodeBrowse(pBrowserData->pNodeBrowse);
      free(pBrowserData->pNodeBrowse);
      pBrowserData->pNodeBrowse = NULL;
   }

   if (pBrowserData->pNameBrowse)
   {
      FLTV7CloseNameBrowse(pBrowserData->pNameBrowse);
      free(pBrowserData->pNameBrowse);
      pBrowserData->pNameBrowse = NULL;
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenNodeBrowseThread                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Initialisiert fuer das Browsen nach Nodenummern             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Zeiger auf THREADDATA-Struktur                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void _Optlink OpenNodeBrowseThread(PVOID pParam)
{
   PTHREADDATA pThreadData = pParam;
   char pchIndexFile[LEN_PATHNAME+1];
   char drive[_MAX_DRIVE];
   char   dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char   ext[_MAX_EXT];
   int rc;

   INSTALLEXPT("OpenNodeBrowse");

   bDoingBrowse = TRUE;

   _splitpath(pThreadData->pDomain->nodelistfile, drive, dir, fname, ext);
   strcpy(ext, ".NDX");
   _makepath(pchIndexFile, drive, dir, fname, ext);


   switch(rc=FLTV7OpenNodeBrowse(pchIndexFile,
                              pThreadData->pDomain->nodelistfile,
                              pThreadData->pBrowserData->pNodeBrowse))
   {
      case 0:
         WinPostMsg(pThreadData->hwnd, BRSM_INDEX_READY, NULL, NULL);
         break;

      default:
         WinPostMsg(pThreadData->hwnd, BRSM_INDEX_ERROR, MPFROMLONG(rc), NULL);
         break;
   }

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenNameBrowseThread                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Initialisiert fuer das Browsen nach Namen                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Zeiger auf THREADDATA-Struktur                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void _Optlink OpenNameBrowseThread(PVOID pParam)
{
   PTHREADDATA pThreadData = pParam;
   int rc;

   INSTALLEXPT("OpenNameBrowse");

   bDoingBrowse = TRUE;

   switch(rc=FLTV7OpenNameBrowse(pThreadData->pDomain->indexfile,
                                 pThreadData->pDomain->nodelistfile,
                                 pThreadData->pBrowserData->pNameBrowse))
   {
      case 0:
         WinPostMsg(pThreadData->hwnd, BRSM_INDEX_READY, NULL, NULL);
         break;

      default:
         WinPostMsg(pThreadData->hwnd, BRSM_INDEX_ERROR, MPFROMLONG(rc), NULL);
         break;
   }

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: NodeIndexReady                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt den eingelesenen Node-Index in den Index-Container    */
/*               ein                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void NodeIndexReady(HWND hwnd, PNLBROWSERDATA pBrowserData)
{
   CNRINFO cnrinfo;
   HWND hwndCnr = WinWindowFromID(hwnd, IDD_NLBROWSER+1);
   HWND hwndCnr2 = WinWindowFromID(hwnd, IDD_NLBROWSER+2);
   char pchTemplate[100];
   PZONEINDEX pZone;
   PNETINDEX pNet;
   ULONG ulNumNets;
   PINDEXRECORD pZoneIndexRecord, pNetIndexRecord, pFirstNetIndexRecord;
   RECORDINSERT RecordInsert;

   /* Titel f. Index-Container neu setzen */
   SendMsg(hwndCnr, CM_QUERYCNRINFO, &cnrinfo, MPFROMLONG(sizeof(cnrinfo)));
   LoadString(IDST_BRS_NODESTPL, sizeof(pchTemplate), pchTemplate);
   sprintf(cnrinfo.pszCnrTitle, pchTemplate, pBrowserData->pNodeBrowse->ulNumNodes);
   SendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_CNRTITLE));

   /* Titel f. Node-Container neu setzen */
   SendMsg(hwndCnr2, CM_QUERYCNRINFO, &cnrinfo, MPFROMLONG(sizeof(cnrinfo)));
   cnrinfo.pszCnrTitle[0]=' ';
   cnrinfo.pszCnrTitle[1]=0;
   SendMsg(hwndCnr2, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_CNRTITLE));


   /* Zonen einfuegen */
   LoadString(IDST_BRS_ZONETPL, sizeof(pchTemplate), pchTemplate);
   pZone = pBrowserData->pNodeBrowse->pZoneIndex;
   while(pZone)
   {
      /* Folder f. Zone */
      pZoneIndexRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                                    MPFROMLONG(sizeof(INDEXRECORD) - sizeof(MINIRECORDCORE)),
                                    MPFROMLONG(1));
      pZoneIndexRecord->RecordCore.flRecordAttr = CRA_COLLAPSED;
      pZoneIndexRecord->RecordCore.pszIcon      = pZoneIndexRecord->pchIndexText;
      pZoneIndexRecord->RecordCore.hptrIcon     = pBrowserData->hptrFolder;
      pZoneIndexRecord->pStartIndex = NULL; /* nicht anwaehlbar */
      LoadString(IDST_BRS_ZONETPL, sizeof(pchTemplate), pchTemplate);
      sprintf(pZoneIndexRecord->pchIndexText, pchTemplate, pZone->usZone);

      RecordInsert.cb = sizeof(RecordInsert);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent = NULL;
      RecordInsert.fInvalidateRecord = FALSE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = 1;
      SendMsg(hwndCnr, CM_INSERTRECORD, pZoneIndexRecord, &RecordInsert);

      /* Netze der Zone */
      LoadString(IDST_BRS_NETTPL, sizeof(pchTemplate), pchTemplate);
      pNet = pZone->pNets;
      /* zaehlen */
      ulNumNets=0;
      while (pNet)
      {
         ulNumNets++;
         pNet = pNet->next;
      }
      if (ulNumNets)
      {
         pFirstNetIndexRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                                           MPFROMLONG(sizeof(INDEXRECORD) - sizeof(MINIRECORDCORE)),
                                           MPFROMLONG(ulNumNets));
         pNetIndexRecord = pFirstNetIndexRecord;
         pNet = pZone->pNets;
         while (pNet)
         {
            pNetIndexRecord->RecordCore.flRecordAttr = CRA_COLLAPSED;
            pNetIndexRecord->RecordCore.pszIcon      = pNetIndexRecord->pchIndexText;
            pNetIndexRecord->RecordCore.hptrIcon     = pBrowserData->hptrFolder;
            pNetIndexRecord->pStartIndex = pNet->pStart; /* anwaehlbar */
            sprintf(pNetIndexRecord->pchIndexText, pchTemplate, pNet->usNet);

            pNetIndexRecord = (PINDEXRECORD) pNetIndexRecord->RecordCore.preccNextRecord;
            pNet = pNet->next;
         }
         RecordInsert.cb = sizeof(RecordInsert);
         RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
         RecordInsert.pRecordParent = (PRECORDCORE) pZoneIndexRecord;
         RecordInsert.fInvalidateRecord = FALSE;
         RecordInsert.zOrder = CMA_TOP;
         RecordInsert.cRecordsInsert = ulNumNets;
         SendMsg(hwndCnr, CM_INSERTRECORD, pFirstNetIndexRecord, &RecordInsert);
      }

      pZone = pZone->next;
   }
   SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: NameIndexReady                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt den eingelesenen Namens-Index in den Index-Container  */
/*               ein                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void NameIndexReady(HWND hwnd, PNLBROWSERDATA pBrowserData)
{
   CNRINFO cnrinfo;
   HWND hwndCnr = WinWindowFromID(hwnd, IDD_NLBROWSER+1);
   ULONG ulNumNames=0;
   char pchTemplate[100];
   PINDEXRECORD pNameIndexRecord, pFirstNameIndexRecord;
   RECORDINSERT RecordInsert;
   int i;

   /* Titel f. Index-Container neu setzen */
   SendMsg(hwndCnr, CM_QUERYCNRINFO, &cnrinfo, MPFROMLONG(sizeof(cnrinfo)));
   LoadString(IDST_BRS_NAMESTPL, sizeof(pchTemplate), pchTemplate);
   sprintf(cnrinfo.pszCnrTitle, pchTemplate, pBrowserData->pNameBrowse->ulNumNames);
   SendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_CNRTITLE));

   /* Titel f. Node-Container neu setzen */
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_QUERYCNRINFO, &cnrinfo, MPFROMLONG(sizeof(cnrinfo)));
   cnrinfo.pszCnrTitle[0]=' ';
   cnrinfo.pszCnrTitle[1]=0;
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_CNRTITLE));

   /* Namen zaehlen */
   for (i=0; i< 28; i++)
   {
      if (pBrowserData->pNameBrowse->Alpha[i])
         ulNumNames++;
   }

   if (ulNumNames)
   {
      pFirstNameIndexRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                                        MPFROMLONG(sizeof(INDEXRECORD) - sizeof(MINIRECORDCORE)),
                                        MPFROMLONG(ulNumNames));
      pNameIndexRecord = pFirstNameIndexRecord;
      for (i=0; i< 28; i++)
      {
         if (pBrowserData->pNameBrowse->Alpha[i])
         {
            pNameIndexRecord->RecordCore.flRecordAttr = CRA_COLLAPSED;
            pNameIndexRecord->RecordCore.pszIcon      = pNameIndexRecord->pchIndexText;
            pNameIndexRecord->RecordCore.hptrIcon     = pBrowserData->hptrFolder;
            pNameIndexRecord->pNStartIndex = pBrowserData->pNameBrowse->Alpha[i]; /* anwaehlbar */
            if (i > 0)
            {
               pNameIndexRecord->pchIndexText[0] = i + '@';
               pNameIndexRecord->pchIndexText[1] = 0;
            }
            else
               LoadString(IDST_BRS_OTHERNAME, sizeof(pNameIndexRecord->pchIndexText), pNameIndexRecord->pchIndexText);

            pNameIndexRecord = (PINDEXRECORD) pNameIndexRecord->RecordCore.preccNextRecord;
         }
      }
      RecordInsert.cb = sizeof(RecordInsert);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent = (PRECORDCORE) NULL;
      RecordInsert.fInvalidateRecord = FALSE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = ulNumNames;
      SendMsg(hwndCnr, CM_INSERTRECORD, pFirstNameIndexRecord, &RecordInsert);
   }
   SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TitleWork                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt "Bitte Warten" in beide Containertitel                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TitleWork(HWND hwnd, PNLBROWSERDATA pBrowserData)
{
   CNRINFO cnrinfo;

   /* Index-Container */
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_QUERYCNRINFO, &cnrinfo,
                     MPFROMLONG(sizeof(cnrinfo)));
   LoadString(IDST_BRS_WORKING, sizeof(pBrowserData->pchICnrTitle), pBrowserData->pchICnrTitle);
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_SETCNRINFO, &cnrinfo,
                     MPFROMLONG(CMA_CNRTITLE));

   /* Node-Container */
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_QUERYCNRINFO, &cnrinfo,
                     MPFROMLONG(sizeof(cnrinfo)));
   LoadString(IDST_BRS_WORKING, sizeof(pBrowserData->pchNCnrTitle), pBrowserData->pchNCnrTitle);
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_SETCNRINFO, &cnrinfo,
                     MPFROMLONG(CMA_CNRTITLE));
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CleanupNodeContainer                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Entfernt alle Record aus dem Node-Container, gibt Speicher  */
/*               frei                                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CleanupNodeContainer(HWND hwnd, PNLBROWSERDATA pBrowserData)
{
   PNODERECORD pRecord=NULL;

   while(pRecord=WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_QUERYRECORD,
                                   pRecord, MPFROM2SHORT(pRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
   {
      if (pRecord->pchAddress)
         free(pRecord->pchAddress);
      if (pRecord->pchModem)
         free(pRecord->pchModem);
      if (pRecord->pchFlags)
         free(pRecord->pchFlags);
   }
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_REMOVERECORD, NULL,
                     MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));

   pBrowserData->ulNumNodes = 0;
   if (pBrowserData->pNodeData)
   {
      free(pBrowserData->pNodeData);
      pBrowserData->pNodeData=NULL;
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenIndex                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt Doppelklick auf Index-Icon aus                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*            pIndexRecord: Zeiger auf den Index-Record                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void OpenIndex(HWND hwnd, PNLBROWSERDATA pBrowserData, PINDEXRECORD pIndexRecord, BOOL bForce)
{
   CNRINFO cnrinfo;
   static THREADDATA ThreadData;

   if (pBrowserData->ulCurrentMode == BROWSEMODE_NAME ||
       (pBrowserData->ulCurrentMode == BROWSEMODE_NODE && pIndexRecord->pStartIndex))
   {
      /* Klick auf Index */
      if (bForce || pBrowserData->pCurrentIndex != pIndexRecord)
      {
         /* Neuer Index */
         if (pBrowserData->pCurrentIndex)
            /* In-Use vom vorherigen wegnehmen */
            WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_SETRECORDEMPHASIS,
                              pBrowserData->pCurrentIndex,
                              MPFROM2SHORT(FALSE, CRA_INUSE));

         pBrowserData->pCurrentIndex = pIndexRecord;
         /* In-Use setzen */
         WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+1, CM_SETRECORDEMPHASIS,
                           pBrowserData->pCurrentIndex,
                           MPFROM2SHORT(TRUE, CRA_INUSE));

         /* Node-Container leeren */
         CleanupNodeContainer(hwnd, pBrowserData);

         /* Titel setzen */
         WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_QUERYCNRINFO, &cnrinfo,
                           MPFROMLONG(sizeof(cnrinfo)));
         LoadString(IDST_BRS_WORKING, sizeof(pBrowserData->pchNCnrTitle), pBrowserData->pchNCnrTitle);
         WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_CNRTITLE));

         /* Controls abschalten */
         WinEnableControl(hwnd, IDD_NLBROWSER+3, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+6, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+7, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+8, FALSE);
         WinEnableControl(hwnd, IDD_NLBROWSER+10, FALSE);

         ThreadData.hwnd = hwnd;
         ThreadData.pBrowserData = pBrowserData;

         if (pBrowserData->ulCurrentMode == BROWSEMODE_NODE)
            _beginthread(ReadNetDataThread, NULL, 16384, &ThreadData);
         else
            _beginthread(ReadNameDataThread, NULL, 16384, &ThreadData);
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadNameDataThread                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Thread-Funktion zum Einlesen der Node-Daten fuer einen      */
/*               Anfangsbuchstaben.                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Zeiger auf THREADDATA-Struktur                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void _Optlink ReadNameDataThread(PVOID pParam)
{
   PTHREADDATA pThreadData = pParam;
   ULONG ulNumNodes=0, i, j;
   UCHAR uchEndChar;
   PNAMEINDEX pNameIndex;

   INSTALLEXPT("ReadNameData");

   bDoingBrowse = TRUE;

   /* Nodes zaehlen */
   pNameIndex = pThreadData->pBrowserData->pCurrentIndex->pNStartIndex;
   if (pNameIndex->pchSysopName[0] < 'A')
      uchEndChar = '@';
   else
      uchEndChar = toupper(pNameIndex->pchSysopName[0]);

   while(pNameIndex < &(pThreadData->pBrowserData->pNameBrowse->pNameIndex[pThreadData->pBrowserData->pNameBrowse->ulNumNames]) &&
         toupper(pNameIndex->pchSysopName[0]) <= uchEndChar )
   {
      ulNumNodes++;
      pNameIndex++;
   }

   if (ulNumNodes)
   {
      /* Speicher fuer Node-Daten */
      pThreadData->pBrowserData->ulNumNodes = ulNumNodes;
      pThreadData->pBrowserData->pNodeData = calloc( ulNumNodes, sizeof(NODEDATA));

      /* Node-Daten einlesen */
      for (j=0; j<ulNumNodes; j++)
      {
         int sel=-1;

         /* K〉zesten Offset suchen */

         pNameIndex = pThreadData->pBrowserData->pCurrentIndex->pNStartIndex;
         for (i=0; i<ulNumNodes; i++)
         {
            if (!pThreadData->pBrowserData->pNodeData[i].SysopName[0] &&
                (sel < 0 ||
                 pNameIndex->lDataOffs <
                  pThreadData->pBrowserData->pCurrentIndex->pNStartIndex[sel].lDataOffs))
               sel = i;
            pNameIndex++;
         }

         if (sel >= 0)
            FLTV7ReadNameData(pThreadData->pBrowserData->pNameBrowse,
                              &pThreadData->pBrowserData->pCurrentIndex->pNStartIndex[sel],
                              &pThreadData->pBrowserData->pNodeData[sel]);
      }

      WinPostMsg(pThreadData->hwnd, BRSM_DATA_READY, NULL, NULL);
   }
   else
      WinPostMsg(pThreadData->hwnd, BRSM_DATA_ERROR, NULL, NULL);


   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadNetDataThread                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Thread-Funktion zum Einlesen der Node-Daten fuer ein Netz   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Zeiger auf THREADDATA-Struktur                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void _Optlink ReadNetDataThread(PVOID pParam)
{
   PTHREADDATA pThreadData = pParam;
   ULONG ulNumNodes=0, ulNumInsert=0, i, j;
   USHORT usZone, usNet;
   PNODEINDEX pNodeIndex;

   INSTALLEXPT("ReadNetData");

   bDoingBrowse = TRUE;

   /* Nodes zaehlen */
   pNodeIndex = pThreadData->pBrowserData->pCurrentIndex->pStartIndex;
   usZone = pNodeIndex->NodeAddr.usZone;
   usNet = pNodeIndex->NodeAddr.usNet;

   while(pNodeIndex < &(pThreadData->pBrowserData->pNodeBrowse->pNodeIndex[pThreadData->pBrowserData->pNodeBrowse->ulNumNodes]) &&
         pNodeIndex->NodeAddr.usZone == usZone &&
         pNodeIndex->NodeAddr.usNet  == usNet )
   {
      if (!pThreadData->pBrowserData->bNoPoints || !pNodeIndex->NodeAddr.usPoint)
         ulNumInsert++;

      ulNumNodes++;
      pNodeIndex++;
   }

   if (ulNumInsert)
   {
      /* Speicher fuer Node-Daten */
      pThreadData->pBrowserData->ulNumNodes = ulNumInsert;
      pThreadData->pBrowserData->pNodeData = calloc( ulNumInsert, sizeof(NODEDATA));

      /* Node-Daten einlesen */
      pNodeIndex = pThreadData->pBrowserData->pCurrentIndex->pStartIndex;
      for (i=0, j=0; i<ulNumNodes; i++)
      {
         if (!pThreadData->pBrowserData->bNoPoints || !pNodeIndex->NodeAddr.usPoint)
            FLTV7ReadNodeData(pThreadData->pBrowserData->pNodeBrowse, pNodeIndex, &pThreadData->pBrowserData->pNodeData[j++]);
         pNodeIndex++;
      }

      WinPostMsg(pThreadData->hwnd, BRSM_DATA_READY, NULL, NULL);
   }
   else
      WinPostMsg(pThreadData->hwnd, BRSM_DATA_ERROR, NULL, NULL);

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: NodeDataReady                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt die eingelesenen Node-Daten in den Node-Container     */
/*               ein                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pBrowserData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void NodeDataReady(HWND hwnd, PNLBROWSERDATA pBrowserData)
{
   PNODERECORD pRecord, pFirstRecord;
   RECORDINSERT RecordInsert;
   PNODEDATA pNodeData;
   CNRINFO cnrinfo;

   if (pBrowserData->ulNumNodes)
   {
      pFirstRecord = WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_ALLOCRECORD,
                                       MPFROMLONG(sizeof(NODERECORD) - sizeof(MINIRECORDCORE)),
                                       MPFROMLONG(pBrowserData->ulNumNodes));
      pRecord = pFirstRecord;
      pNodeData = pBrowserData->pNodeData;
      while (pRecord)
      {
         pRecord->RecordCore.flRecordAttr=0;

         pRecord->pchSysop = pNodeData->SysopName;
         pRecord->pchSystemName = pNodeData->SystemName;
         pRecord->pchLocation = pNodeData->Location;
         pRecord->pchPhone = pNodeData->PhoneNr;
         pRecord->ulBaud = pNodeData->BaudRate;
         pRecord->ulCallCost = pNodeData->CallCost;
         pRecord->ulUserCost = pNodeData->UserCost;

         pRecord->pchAddress = malloc(LEN_5DADDRESS+1);
         if (pNodeData->Address.usPoint)
            sprintf(pRecord->pchAddress, "%d:%d/%d.%d", pNodeData->Address.usZone,
                                                        pNodeData->Address.usNet,
                                                        pNodeData->Address.usNode,
                                                        pNodeData->Address.usPoint);
         else
            sprintf(pRecord->pchAddress, "%d:%d/%d", pNodeData->Address.usZone,
                                                     pNodeData->Address.usNet,
                                                     pNodeData->Address.usNode);
         pRecord->pchFlags=malloc(30);
         NLFlagsToString(pNodeData, pRecord->pchFlags);

         pRecord->pchModem= malloc(MAX_MODEMTYPES*(LEN_MODEMTYPE+2)+1);
         NLModemToString(pNodeData->ModemType, pRecord->pchModem);

         pRecord = (PNODERECORD) pRecord->RecordCore.preccNextRecord;
         pNodeData++;
      }
      RecordInsert.cb = sizeof(RECORDINSERT);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent = (PRECORDCORE) NULL;
      RecordInsert.fInvalidateRecord = TRUE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = pBrowserData->ulNumNodes;
      WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_INSERTRECORD, pFirstRecord, &RecordInsert);
   }

   /* Titel anpassen */
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_QUERYCNRINFO, &cnrinfo,
                     MPFROMLONG(sizeof(cnrinfo)));

   if (pBrowserData->ulCurrentMode == BROWSEMODE_NODE)
   {
      sprintf(pBrowserData->pchNCnrTitle, "%d:%d", pBrowserData->pNodeData->Address.usZone,
                                                   pBrowserData->pNodeData->Address.usNet);
   }
   else
   {
      if (pBrowserData->pCurrentIndex->pNStartIndex->pchSysopName[0] < 'A')
         LoadString(IDST_BRS_OTHERNAME, sizeof(pBrowserData->pchNCnrTitle), pBrowserData->pchNCnrTitle);
      else
      {
         pBrowserData->pchNCnrTitle[0] = pBrowserData->pCurrentIndex->pNStartIndex->pchSysopName[0];
         pBrowserData->pchNCnrTitle[1] = 0;
      }
   }
   WinSendDlgItemMsg(hwnd, IDD_NLBROWSER+2, CM_SETCNRINFO, &cnrinfo,
                     MPFROMLONG(CMA_CNRTITLE));

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InitNodeDrag                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Startet das Drag-Drop eines Nodes aus dem Container         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Fenster                                           */
/*            pInit: Init-Struktur des Containers                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InitNodeDrag(HWND hwndDlg, PCNRDRAGINIT pInit, PNLBROWSERDATA pBrowserData)
{
   PNODERECORD pNodeRecord = (PNODERECORD) pInit->pRecord;
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_NLBROWSER+2);
   PDRAGINFO pDraginfo;
   DRAGITEM dItem;
   DRAGIMAGE dImage[3];
   char pchTemp[LEN_5DADDRESS+LEN_USERNAME+2];

   if (!pNodeRecord)
      return;  /* Drag ｜er whitespace */

   if (pNodeRecord->RecordCore.flRecordAttr & CRA_SELECTED)
   {
      /* selektiert -> alle anderen selektierten auch */
      ULONG ulNum=0, i=0;
      PNODERECORD pTemp=NULL;

      while (pTemp = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pTemp?pTemp:MPFROMLONG(CMA_FIRST),
                                MPFROMLONG(CRA_SELECTED)))
      {
         ulNum++;
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pTemp, MPFROM2SHORT(TRUE, CRA_SOURCE));
      }
      pDraginfo = DrgAllocDraginfo(ulNum);
      pDraginfo->usOperation=DO_DEFAULT;
      pDraginfo->hwndSource=hwndDlg;

      pTemp=NULL;
      while (pTemp = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pTemp?pTemp:MPFROMLONG(CMA_FIRST),
                                MPFROMLONG(CRA_SELECTED)))
      {
         /* Drag-Item vorbereiten*/
         dItem.hwndItem=hwndDlg;
         dItem.ulItemID= 0;
         dItem.hstrType=DrgAddStrHandle(NODEDRAGTYPE);
         dItem.hstrRMF=DrgAddStrHandle(NODERMF);
         dItem.hstrContainerName=DrgAddStrHandle(pBrowserData->pchCurrentDomain);

         strcpy(pchTemp, pTemp->pchAddress);
         strcat(pchTemp, " ");
         strcat(pchTemp, pTemp->pchSysop);
         dItem.hstrSourceName=DrgAddStrHandle(pchTemp);
         dItem.hstrTargetName=NULLHANDLE;

         dItem.fsControl= 0;
         dItem.fsSupportedOps=DO_COPYABLE;
         DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), i);

         i++;
      }

      for (i=0; i < ulNum && i <3; i++)
      {
         /* Drag-Image vorbereiten */
         dImage[i].cb=sizeof(DRAGIMAGE);
         if (ulNum == 2)
            dImage[i].hImage=pBrowserData->hptrOneNode;
         else
            dImage[i].hImage=pBrowserData->hptrOneNode;
         dImage[i].fl=DRG_ICON;
         dImage[i].cxOffset=i*10;
         dImage[i].cyOffset=i*10;
      }

      /* Und los gehts */
#if 0
      if (!DrgDrag(hwndDlg, pDraginfo, dImage, (ulNum <3)?ulNum:3, VK_ENDDRAG, NULL))
         DrgDeleteDraginfoStrHandles(pDraginfo);
#else
      DrgDrag(hwndDlg, pDraginfo, dImage, (ulNum <3)?ulNum:3, VK_ENDDRAG, NULL);
#endif
      DrgFreeDraginfo(pDraginfo);

      pTemp=NULL;
      while (pTemp = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pTemp?pTemp:MPFROMLONG(CMA_FIRST),
                                MPFROMLONG(CRA_SOURCE)))
      {
         /* Source-Emphasis ausschalten */
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pTemp,
                    MPFROM2SHORT(FALSE, CRA_SOURCE));
      }
   }
   else
   {
      /* nur ein Node */
      /* Source-Emphasis einschalten */
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pNodeRecord,
                 MPFROM2SHORT(TRUE, CRA_SOURCE));

      pDraginfo = DrgAllocDraginfo(1);
      pDraginfo->usOperation=DO_DEFAULT;
      pDraginfo->hwndSource=hwndDlg;

      /* Drag-Item vorbereiten*/
      dItem.hwndItem=hwndDlg;
      dItem.ulItemID= 0;
      dItem.hstrType=DrgAddStrHandle(NODEDRAGTYPE);
      dItem.hstrRMF=DrgAddStrHandle(NODERMF);
      dItem.hstrContainerName=DrgAddStrHandle(pBrowserData->pchCurrentDomain);

      strcpy(pchTemp, pNodeRecord->pchAddress);
      strcat(pchTemp, " ");
      strcat(pchTemp, pNodeRecord->pchSysop);
      dItem.hstrSourceName=DrgAddStrHandle(pchTemp);
      dItem.hstrTargetName=NULLHANDLE;

      dItem.fsControl= 0;
      dItem.fsSupportedOps=DO_COPYABLE;
      DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);

      /* Drag-Image vorbereiten */
      dImage[0].cb=sizeof(DRAGIMAGE);
      dImage[0].hImage=pBrowserData->hptrOneNode;
      dImage[0].fl=DRG_ICON;
      dImage[0].cxOffset=0;
      dImage[0].cyOffset=0;

      /* Und los gehts */
#if 0
      if (!DrgDrag(hwndDlg, pDraginfo, dImage, 1, VK_ENDDRAG, NULL))
         DrgDeleteDraginfoStrHandles(pDraginfo);
#else
      DrgDrag(hwndDlg, pDraginfo, dImage, 1, VK_ENDDRAG, NULL);
#endif
      DrgFreeDraginfo(pDraginfo);

      /* Source-Emphasis ausschalten */
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pNodeRecord,
                 MPFROM2SHORT(FALSE, CRA_SOURCE));
   }
   return;
}
/*-------------------------------- Modulende --------------------------------*/


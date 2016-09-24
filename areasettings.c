/*---------------------------------------------------------------------------+
 | Titel: AREASETTINGS.C                                                     |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 22.02.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Settings-Notebook einer Area                                          |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

#pragma strings(readonly)
/*----------------------------- Header-Dateien ------------------------------*/
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "setupdlg.h"
#include "areadlg.h"
#include "utility.h"
#include "controls\editwin.h"
#include "controls\attrselect.h"
#include "handlemsg\handlemsg.h"

#include "areasettings.h"

/*--------------------------------- Defines ---------------------------------*/

#define APM_AREAFORMAT   (WM_USER+1)

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

extern HMODULE hmodLang;
extern HAB anchor;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int SendToAllPages(HWND hwndNotebook, ULONG msg, MPARAM mp1, MPARAM mp2);
static void InsertSettingsPages(HWND notebook, PVOID dlgpar);
static void AddCancelItem(HWND hwnd);
static MRESULT EXPENTRY AreaGenSettProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY AreaMsgBaseProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY AreaAttribProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaSettingsProc                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Dialog-Prozedur der Area-Settings                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY AreaSettingsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp;
   char pchTemp[200];
   char pchTitle[250];
   HWND notebook=NULLHANDLE;
   MRESULT resultbuf=0;
   int rc;

   switch(message)
   {
      case WM_INITDLG:
         notebook=WinWindowFromID(parent,IDD_AREASETTINGS+1);
         AddCancelItem(parent);
         InsertSettingsPages(notebook, (PVOID) mp2);
         WinSetWindowULong(parent, QWL_USER, ((PAREAPAR)mp2)->bMultiple);
         RestoreWinPos(parent, &windowpositions.areasetuppos, TRUE, TRUE);
         WinAssociateHelpInstance(hwndhelp, parent);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, parent, IDD_AREASETTINGS+1);
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(parent,message,mp1,mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=255;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=190;

         return resultbuf;

      case WM_CLOSE:
         if (!WinQueryWindowULong(parent, QWL_USER))
         {
            /* Message an alle Windows schicken */
            rc = SendToAllPages(WinWindowFromID(parent, IDD_AREASETTINGS+1), APM_REQCLOSE, NULL, NULL);
            if (rc == 1)
               return (MRESULT) FALSE;
            if (rc == 2)
            {
               SendToAllPages(WinWindowFromID(parent, IDD_AREASETTINGS+1), APM_CANCEL, NULL, NULL);
               WinDismissDlg(parent, DID_CANCEL);
            }
            else
               WinDismissDlg(parent, DID_OK);
         }
         else
            WinDismissDlg(parent, DID_OK);
         QueryWinPos(parent, &(windowpositions.areasetuppos));
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         return (MRESULT) FALSE;

      case APM_SETTITLE:
         if (!((PCHAR) mp1)[0])
            LoadString(IDST_AP_EMPTYTITLE, 250, pchTitle);
         else
         {
            LoadString(IDST_AP_TITLE, 200, pchTemp);
            sprintf(pchTitle, pchTemp, (PCHAR) mp1);
         }
         WinSetWindowText(parent, pchTitle);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == IDM_AS_CANCEL)
         {
            SendToAllPages(WinWindowFromID(parent, IDD_AREASETTINGS+1), APM_CANCEL, NULL, NULL);
            WinDismissDlg(parent, DID_CANCEL);
         }
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SendToAllPages                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Schickt eine Message an alle Seiten eines Notebooks.        */
/*               Wenn eine Seite eine int != 0 zurueckliefert, wird zu der   */
/*               Seite umgeschaltet und der int zurueckgeliefert. Sonst      */
/*               wird 0 zurueckgeliefert.                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndNotebook: Notebook                                         */
/*            msg: Zu sendende Message                                       */
/*            mp1: Parameter 1                                               */
/*            mp2: Parameter 2                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Rueckgabewert der Seite                                    */
/*                0  sonst                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int SendToAllPages(HWND hwndNotebook, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   ULONG ulPageID = 0;
   int rc=0;

   while(ulPageID = (ULONG) SendMsg(hwndNotebook, BKM_QUERYPAGEID,
                                    MPFROMLONG(ulPageID),
                                    MPFROM2SHORT(ulPageID?BKA_NEXT:BKA_FIRST, 0)))
   {
      HWND hwndPage;

      hwndPage = (HWND) SendMsg(hwndNotebook, BKM_QUERYPAGEWINDOWHWND,
                                   MPFROMLONG(ulPageID), NULL);
      if (rc=(int)SendMsg(hwndPage, msg, mp1, mp2))
      {
         /* zur Seite blaettern */
         SendMsg(hwndNotebook, BKM_TURNTOPAGE, MPFROMLONG(ulPageID), NULL);
         return rc;
      }
   }
   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddCancelItem                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt den Abbruch-Menuepunkt zum Systemmenue hinzu          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Dialog-Window                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void AddCancelItem(HWND hwnd)
{
   HWND        hSysMenu,
               hSysSubMenu;
   MENUITEM    SysMenu;
   SHORT       idSysMenu;

   MENUITEM Item[2] = {{MIT_END, MIS_SEPARATOR, 0, 0,             NULLHANDLE, 0},
                       {MIT_END, MIS_TEXT,      0, IDM_AS_CANCEL, NULLHANDLE, 0}};
   char pchTemp[100]="";

   hSysMenu = WinWindowFromID(hwnd, FID_SYSMENU);
   idSysMenu = SHORT1FROMMR(SendMsg(hSysMenu, MM_ITEMIDFROMPOSITION, NULL, NULL));

   SendMsg(hSysMenu, MM_QUERYITEM, MPFROM2SHORT(idSysMenu, FALSE),
              &SysMenu);
   hSysSubMenu = SysMenu.hwndSubMenu;

   LoadString(IDST_AP_CANCEL, sizeof(pchTemp), pchTemp);

   SendMsg(hSysSubMenu, MM_INSERTITEM, &Item[0], NULL);
   SendMsg(hSysSubMenu, MM_INSERTITEM, &Item[1], pchTemp);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertSettingsPages                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt alle Seiten in das Notebook ein, initialisiert        */
/*               das Notebook                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: notebook: Window-Handle des Notebooks                          */
/*            dlgpar: Parameter, wird an die Seiten weitergegeben            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InsertSettingsPages(HWND notebook, PVOID dlgpar)
{
   PAREAPAR pAreaPar = (PAREAPAR) dlgpar;

   SetNotebookParams(notebook, 120);

   InsertOnePage(notebook, IDD_AS_GENERAL, IDST_TAB_AS_GENERAL, AreaGenSettProc, dlgpar);
   if (!pAreaPar->bMultiple)
      InsertOnePage(notebook, IDD_AS_MSGBASE, IDST_TAB_AS_MSGBASE, AreaMsgBaseProc, dlgpar);
   InsertOnePage(notebook, IDD_AS_ATTRIB,  IDST_TAB_AS_ATTRIB,  AreaAttribProc, dlgpar);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaGenSettProc                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Allgemeine Einstellungen einer Area                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY AreaGenSettProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   HWND hwndList;
   extern USERDATAOPT userdaten;
   extern AREALIST arealiste;
   extern TEMPLATELIST templatelist;
   SHORT sItem=0, sSelect=0;
   static AREADEFLIST *pArea=NULL;
   char pchTemp[LEN_PATHNAME+1];
   static HWND hwndSettingsFrame;
   PMSGTEMPLATE pTemplate;
   AREADEFLIST *pAreaTemp;

   switch(message)
   {
      case WM_INITDLG:
         pArea=((PAREAPAR)mp2)->pAreaDef;

         if (!((PAREAPAR)mp2)->bMultiple)
         {
            SendMsg(hwndSettingsFrame=WinQueryWindow(parent, QW_PARENT),
                       APM_SETTITLE, pArea->areadata.areatag, NULL);
            WinSendDlgItemMsg(parent,IDD_AS_GENERAL+3,
                              EM_SETTEXTLIMIT,
                              MPFROMSHORT(LEN_AREATAG),
                              (MPARAM) NULL);

            WinSendDlgItemMsg(parent,IDD_AS_GENERAL+5,
                              EM_SETTEXTLIMIT,
                              MPFROMSHORT(LEN_AREADESC),
                              (MPARAM) NULL);
         }
         else
         {
            WinShowWindow(WinWindowFromID(parent, IDD_AS_GENERAL+2), FALSE);
            WinShowWindow(WinWindowFromID(parent, IDD_AS_GENERAL+3), FALSE);
            WinShowWindow(WinWindowFromID(parent, IDD_AS_GENERAL+4), FALSE);
            WinShowWindow(WinWindowFromID(parent, IDD_AS_GENERAL+5), FALSE);
         }

         /* Adressen eintragen */
         hwndList=WinWindowFromID(parent, IDD_AS_GENERAL+9);
         SendMsg(hwndList, LM_DELETEALL, (MPARAM)0, (MPARAM)0);
         sItem=0;
         sSelect = LIT_NONE;
         while(sItem<MAX_ADDRESSES && userdaten.address[sItem][0])
         {
            SendMsg(hwndList, LM_INSERTITEM,
                       MPFROMSHORT(LIT_END),
                       (MPARAM) userdaten.address[sItem]);
            if (pArea && !strcmp(pArea->areadata.address, userdaten.address[sItem]))
               sSelect = sItem;
            sItem++;
         }
         SendMsg(hwndList, LM_SELECTITEM, MPFROMSHORT(sSelect), (MPARAM) TRUE);

         /* Namen eintragen */
         hwndList=WinWindowFromID(parent, IDD_AS_GENERAL+12);
         SendMsg(hwndList, LM_DELETEALL, (MPARAM)0, (MPARAM)0);
         sItem=0;
         sSelect = LIT_NONE;
         while(sItem<MAX_USERNAMES && userdaten.username[sItem][0])
         {
            SendMsg(hwndList, LM_INSERTITEM,
                       MPFROMSHORT(LIT_END),
                       (MPARAM) userdaten.username[sItem]);
            if (pArea && !strcmp(pArea->areadata.username, userdaten.username[sItem]))
               sSelect = sItem;
            sItem++;
         }
         SendMsg(hwndList, LM_SELECTITEM, MPFROMSHORT(sSelect), (MPARAM) TRUE);

         if (pArea)
         {
            WinSetDlgItemText(parent, IDD_AS_GENERAL+3,
                              pArea->areadata.areatag);
            WinSetDlgItemText(parent, IDD_AS_GENERAL+5,
                              pArea->areadata.areadesc);

            if (pArea->areadata.ulAreaOpt & AREAOPT_FROMCFG)
               WinSendDlgItemMsg(parent, IDD_AS_GENERAL+3, EM_SETREADONLY,
                                 (MPARAM)TRUE, NULL);
         }

         /* Template-Namen eintragen */
         pTemplate = templatelist.pTemplates;
         while (pTemplate)
         {
            sItem = (SHORT) WinSendDlgItemMsg(parent, IDD_AS_GENERAL+14, LM_INSERTITEM,
                                              MPFROMSHORT(LIT_SORTASCENDING),
                                              pTemplate->TName);
            WinSendDlgItemMsg(parent, IDD_AS_GENERAL+14, LM_SETITEMHANDLE,
                              MPFROMSHORT(sItem),
                              MPFROMLONG(pTemplate->ulID));
            if (pArea && pTemplate->ulID == pArea->areadata.ulTemplateID)
               WinSendDlgItemMsg(parent, IDD_AS_GENERAL+14, LM_SELECTITEM,
                                 MPFROMSHORT(sItem), MPFROMSHORT(TRUE));

            pTemplate = pTemplate->next;
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         if (!WinQueryWindowULong(parent, QWL_USER))
         {
            WinQueryDlgItemText(parent, IDD_AS_GENERAL+3, LEN_AREATAG+1,
                                pchTemp);
            if (strcmp(pArea->areadata.areatag, pchTemp))
            {
               strcpy(pArea->areadata.areatag, pchTemp);
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }
            WinQueryDlgItemText(parent, IDD_AS_GENERAL+5, LEN_AREADESC+1,
                                pchTemp);
            if (strcmp(pArea->areadata.areadesc, pchTemp))
            {
               strcpy(pArea->areadata.areadesc, pchTemp);
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }
            WinQueryDlgItemText(parent, IDD_AS_GENERAL+9, LEN_5DADDRESS+1,
                                pchTemp);
            if (strcmp(pArea->areadata.address, pchTemp))
            {
               strcpy(pArea->areadata.address, pchTemp);
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }
            WinQueryDlgItemText(parent, IDD_AS_GENERAL+12, LEN_USERNAME+1,
                                pchTemp);
            if (strcmp(pArea->areadata.username, pchTemp))
            {
               strcpy(pArea->areadata.username, pchTemp);
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }
            sItem = (SHORT)WinSendDlgItemMsg(parent, IDD_AS_GENERAL+14, LM_QUERYSELECTION,
                                               MPFROMSHORT(LIT_FIRST), NULL);
            if (sItem >= 0)
            {
               ULONG ulID;

               ulID = (ULONG) WinSendDlgItemMsg(parent, IDD_AS_GENERAL+14, LM_QUERYITEMHANDLE,
                                                MPFROMSHORT(sItem), NULL);
               if (ulID != pArea->areadata.ulTemplateID)
               {
                  pArea->areadata.ulTemplateID = ulID;
                  arealiste.bDirty = TRUE;
                  pArea->dirty=TRUE;
               }
            }
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_AS_GENERAL+3)
            if (SHORT2FROMMP(mp1)==EN_KILLFOCUS)
            {
               char pchTempTag[LEN_AREATAG+1];

               WinQueryDlgItemText(parent, IDD_AS_GENERAL+3, LEN_AREATAG+1, pchTempTag);
               SendMsg(hwndSettingsFrame,
                          APM_SETTITLE, pchTempTag, NULL);
            }
         if (SHORT1FROMMP(mp1) == IDD_AS_GENERAL+15)
            if (SHORT2FROMMP(mp1) == BN_CLICKED ||
                SHORT2FROMMP(mp1) == BN_DBLCLICKED)
            {
               if (WinQueryButtonCheckstate(parent, IDD_AS_GENERAL+15))
                  WinEnableControl(parent, IDD_AS_GENERAL+7, FALSE);
               else
                  WinEnableControl(parent, IDD_AS_GENERAL+7, TRUE);
            }
         break;

      case APM_REQCLOSE:
         WinQueryDlgItemText(parent, IDD_AS_GENERAL+3, LEN_AREATAG+1,
                             pchTemp);
         if (pchTemp[0] == 0)
         {
            /* Fehler, kein Area-Tag */
            if (MessageBox(parent, IDST_MSG_NOAREATAG, 0,
                           IDD_NOAREATAG, MB_RETRYCANCEL | MB_ERROR) == MBID_CANCEL)
                return (MRESULT) 2;
            else
                return (MRESULT) 1;
         }
         else
         {
            pAreaTemp = AM_FindArea(&arealiste, pchTemp);
            if (pAreaTemp && pAreaTemp != pArea)
            {
               /* Fehler, schon vorhanden */
               if (MessageBox(parent, IDST_MSG_ALREADYHAVEAREA, 0,
                              IDD_ALREADYHAVEAREA, MB_RETRYCANCEL | MB_ERROR) == MBID_CANCEL)
                   return (MRESULT) 2;
               else
                   return (MRESULT) 1;
            }
         }
         return (MRESULT) 0;

      case APM_CANCEL:
         WinSetWindowULong(parent, QWL_USER, 1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaMsgBaseProc                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Message-Base-Settings einer Area                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY AreaMsgBaseProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern AREALIST arealiste;
   extern HWND hwndhelp;
   static AREADEFLIST *pArea=NULL;
   char pchTemp[LEN_PATHNAME+1];
   SHORT sTemp;

   switch(message)
   {
      case WM_INITDLG:
         pArea=((AREAPAR *)mp2)->pAreaDef;
         WinSubclassWindow(WinWindowFromID(parent, IDD_AS_MSGBASE+9),
                           FileEntryProc);

         WinSendDlgItemMsg(parent, IDD_AS_MSGBASE+9,
                           EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME),
                           (MPARAM) NULL);
         if (pArea)
         {
            WinSetDlgItemText(parent, IDD_AS_MSGBASE+9,
                              pArea->areadata.pathfile);

            switch(pArea->areadata.areaformat)
            {
               case AREAFORMAT_FTS:
                  WinCheckButton(parent, IDD_AS_MSGBASE+2, TRUE);
                  break;

               case AREAFORMAT_SQUISH:
                  WinCheckButton(parent, IDD_AS_MSGBASE+3, TRUE);
                  break;

               case AREAFORMAT_JAM:
                  WinCheckButton(parent, IDD_AS_MSGBASE+4, TRUE);
                  break;

               default:
                  WinCheckButton(parent, IDD_AS_MSGBASE+2, TRUE);
                  break;
            }
            switch(pArea->areadata.areatype)
            {
               case AREATYPE_NET:
                  WinCheckButton(parent, IDD_AS_MSGBASE+6, TRUE);
                  break;

               case AREATYPE_ECHO:
                  WinCheckButton(parent, IDD_AS_MSGBASE+7, TRUE);
                  break;

               case AREATYPE_LOCAL:
                  WinCheckButton(parent, IDD_AS_MSGBASE+12, TRUE);
                  break;
            }
            if (pArea->areadata.ulAreaOpt & AREAOPT_FROMCFG)
            {
               WinEnableControl(parent, IDD_AS_MSGBASE+2, FALSE);
               WinEnableControl(parent, IDD_AS_MSGBASE+3, FALSE);
               WinEnableControl(parent, IDD_AS_MSGBASE+4, FALSE);
               WinEnableControl(parent, IDD_AS_MSGBASE+6, FALSE);
               WinEnableControl(parent, IDD_AS_MSGBASE+7, FALSE);
               WinEnableControl(parent, IDD_AS_MSGBASE+12, FALSE);
               WinSendDlgItemMsg(parent, IDD_AS_MSGBASE+9, EM_SETREADONLY,
                                 (MPARAM)TRUE, (MPARAM) NULL);
               WinEnableControl(parent, IDD_AS_MSGBASE+10, FALSE);
            }
         }
         WinDefDlgProc(parent, message, mp1, mp2);
         SetFocusControl(parent, IDD_AS_MSGBASE+9);
         return (MRESULT) TRUE;

      case WM_CLOSE:
      case WM_DESTROY:
         if (!WinQueryWindowULong(parent, QWL_USER))
         {
            WinQueryDlgItemText(parent, IDD_AS_MSGBASE+9, LEN_PATHNAME+1,
                                pchTemp);
            if (strcmp(pArea->areadata.pathfile, pchTemp))
            {
               strcpy(pArea->areadata.pathfile, pchTemp);
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }

            if (WinQueryButtonCheckstate(parent, IDD_AS_MSGBASE+2))
               sTemp=AREAFORMAT_FTS;
            else
               if (WinQueryButtonCheckstate(parent, IDD_AS_MSGBASE+3))
                  sTemp=AREAFORMAT_SQUISH;
               else
                  if (WinQueryButtonCheckstate(parent, IDD_AS_MSGBASE+4))
                     sTemp=AREAFORMAT_JAM;
                  else
                     sTemp=AREAFORMAT_FTS;

            if (sTemp!=pArea->areadata.areaformat)
            {
               pArea->areadata.areaformat=sTemp;
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }

            if (WinQueryButtonCheckstate(parent, IDD_AS_MSGBASE+6))
               sTemp=AREATYPE_NET;
            else
               if (WinQueryButtonCheckstate(parent, IDD_AS_MSGBASE+7))
                  sTemp=AREATYPE_ECHO;
               else
                  if (WinQueryButtonCheckstate(parent, IDD_AS_MSGBASE+12))
                     sTemp=AREATYPE_LOCAL;
                  else
                     sTemp=AREATYPE_NET;

            if (sTemp != pArea->areadata.areatype)
            {
               pArea->areadata.areatype=sTemp;
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }
         }
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            if(SHORT1FROMMP(mp1)==IDD_AS_MSGBASE+10)
            {
               WinQueryDlgItemText(parent, IDD_AS_MSGBASE+9,
                                   LEN_PATHNAME+1, pchTemp);
               if (GetPathname(parent, pchTemp)==DID_OK)
               {
                  char drive[_MAX_DRIVE];
                  char path[_MAX_DIR];
                  char name[_MAX_FNAME];
                  char ext[_MAX_EXT];

                  _splitpath(pchTemp, drive, path, name, ext);
                  if (WinQueryButtonCheckstate(parent, IDD_AS_MSGBASE+3))
                     _makepath(pchTemp, drive, path, name, "");
                  else
                  {
                    _makepath(pchTemp, drive, path, "", "");
                    pchTemp[strlen(pchTemp)-1]='\0';
                  }
                  WinSetDlgItemText(parent, IDD_AS_MSGBASE+9,
                                    pchTemp);
                  arealiste.bDirty = TRUE;
               }
            }
         }
         return (MRESULT) FALSE;

      case APM_REQCLOSE:
         if (WinQueryDlgItemTextLength(parent, IDD_AS_MSGBASE+9)==0)
         {
            /* Fehler, kein Pfadname */
            if (MessageBox(parent, IDST_MSG_NOPATHFILE, 0,
                           IDD_NOPATHFILE, MB_RETRYCANCEL | MB_ERROR) == MBID_CANCEL)
                return (MRESULT) 2;
            else
                return (MRESULT) 1;
         }
         return (MRESULT) 0;

      case APM_CANCEL:
         WinSetWindowULong(parent, QWL_USER, 1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AreaAttribProc                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sonstige Area-Attribute                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WinProc                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY AreaAttribProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern AREALIST arealiste;
   static AREADEFLIST *pArea=NULL;

   switch(message)
   {
      case WM_INITDLG:
         pArea=((AREAPAR *)mp2)->pAreaDef;
         if (pArea)
         {
            ULONG ulMask;

            WinCheckButton(parent, IDD_AS_ATTRIB+9, !!(pArea->areadata.ulAreaOpt & AREAOPT_HIGHASCII));
            WinCheckButton(parent, IDD_AS_ATTRIB+10, !!(pArea->areadata.ulAreaOpt & AREAOPT_SEPARATOR));
            WinCheckButton(parent, IDD_AS_ATTRIB+11, !!(pArea->areadata.ulAreaOpt & AREAOPT_NOHIGHLIGHT));
            WinCheckButton(parent, IDD_AS_ATTRIB+3, !!(pArea->areadata.ulAreaOpt & AREAOPT_MONOSPACED));

            /* Message Attributes */
            MSG_QueryAttribCaps(&arealiste, pArea->areadata.areatag, &ulMask);
            WinSendDlgItemMsg(parent, IDD_AS_ATTRIB+2, ATTSM_SETATTRIB,
                              MPFROMLONG(pArea->areadata.ulDefAttrib),
                              MPFROMLONG(ulMask));
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         if (!WinQueryWindowULong(parent, QWL_USER))
         {
            ULONG ulTemp = pArea->areadata.ulAreaOpt & AREAOPT_FROMCFG;

            if (WinQueryButtonCheckstate(parent, IDD_AS_ATTRIB+9))
               ulTemp |= AREAOPT_HIGHASCII;
            if (WinQueryButtonCheckstate(parent, IDD_AS_ATTRIB+10))
               ulTemp |= AREAOPT_SEPARATOR;
            if (WinQueryButtonCheckstate(parent, IDD_AS_ATTRIB+11))
               ulTemp |= AREAOPT_NOHIGHLIGHT;
            if (WinQueryButtonCheckstate(parent, IDD_AS_ATTRIB+3))
               ulTemp |= AREAOPT_MONOSPACED;
            if (pArea->areadata.ulAreaOpt != ulTemp)
            {
               pArea->areadata.ulAreaOpt = ulTemp;
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }

            ulTemp = (ULONG) WinSendDlgItemMsg(parent, IDD_AS_ATTRIB+2, ATTSM_QUERYATTRIB, NULL, NULL);
            if (pArea->areadata.ulDefAttrib != ulTemp)
            {
               pArea->areadata.ulDefAttrib = ulTemp;
               arealiste.bDirty = TRUE;
               pArea->dirty=TRUE;
            }
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case APM_REQCLOSE:
         return (MRESULT) 0;

      case APM_CANCEL:
         WinSetWindowULong(parent, QWL_USER, 1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*-------------------------------- Modulende --------------------------------*/


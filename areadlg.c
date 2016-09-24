/*---------------------------------------------------------------------------+
 | Titel: AREADLG.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von:  Michael Hohner           | Am:  14.07.93                   |
 +-----------------------------------------+---------------------------------+
 | System:  OS/2 2.x PM und CSet/2                                           |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Area-Liste und Area-Setup                                              |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#pragma strings(readonly)

#define INCL_GPI
#define INCL_BASE
#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "areaman\areaman.h"
#include "areaman\folderman.h"
#include "msgheader.h"
#include "dialogids.h"
#include "setupdlg.h"
#include "dialogs.h"
#include "handlemsg\handlemsg.h"
#include "utility.h"
#include "msglist.h"
#include "util\fltutil.h"
#include "arealistsettings.h"
#include "areasettings.h"
#include "areascan.h"
#include "areadlg.h"
#include "areadrag.h"
#include "folderdrag.h"

/*--------------------------------- Defines ---------------------------------*/

#ifndef CN_COLORCHANGED
#define CN_COLORCHANGED 200
#endif

#ifndef CCS_MINIICONS
#define CCS_MINIICONS 0x800
#endif

#define CONTAINERSPACE  3

typedef struct
{
   RECORDCORE RecordCore;
   PSZ areadesc;     /* zeigt in AREADEF */
   PSZ msgnotread;
   PSZ totlmsgs;
   AREADEFLIST *pAreaDef;
} AREARECORD, *PAREARECORD;

typedef struct
{
   MINIRECORDCORE RecordCore;
   PAREAFOLDER pAreaFolder;
} FOLDERRECORD, *PFOLDERRECORD;

typedef struct
{
   HPOINTER    icon;             /* Icon f. Systemmenue */
   HPOINTER    icnFolder;
   HPOINTER    icnFolderOpen;
   HPOINTER    icnFolderUnread;
   HPOINTER    hptrPlus;
   HPOINTER    hptrMinus;
   POINTL      pointl;           /* Merker f. Mindestgroesse */
   char        pchTAreaDesc[50]; /* Strings */
   char        pchTUnread[50];
   char        pchTTotal[50];
   PAREARECORD selectRecord;    /* Record f. Popup-Menue   */
   PFOLDERRECORD selectFolder;
   HWND        hwndpopup;        /* Popup f. Areas          */
   HWND        hwndpopupsmall;   /* Popup f. Gesamtliste    */
   HWND        hwndFolderPopup;
   HWND        hwndSmallFolderPopup;
   ULONG       ulSort;           /* aktuelle Sortierung     */
   BOOL        bKeyboard;        /* Flag f. Tastatur-Aktion */
   BOOL        bAcceptChange;    /* Farbaenderung annehmen  */
   BOOL        bDirty;           /* Inhalt muss upgedated werden */
   PCHAR       *pchParamString;  /* Zeiger f. Auswahl       */
   HSWITCH     hSwitch;          /* Eintrag in Fensterliste */
   LONG        lDisable;         /* Disable-Counter         */
   BOOL        bForeground;      /* Liste im Vordergrund    */
   ULONG       ulIncludeTypes;
   BOOL        bChange;
   BOOL        bExtendedSel;
   BOOL        bDirectEdit;
   PFOLDERRECORD pOpenFolder;
} AREALISTDATA, *PAREALISTDATA;

typedef struct
{
   USHORT       cb;
   SQUISHPARAMS SquishParams;
} SQUISHPARAMSPAR;


#define TAB_FONT    "8.Helv"
#define RGB_GREY    0x00cccccc
#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L    /* 2.1-spezifisch !!! */
#endif

/*---------------------------- Globale Variablen ----------------------------*/

static PFNWP OldAEditProc;
static PFNWP OldContainerProc;

extern HMODULE hmodLang;
extern HAB anchor;
extern AREALISTOPTIONS arealistoptions;
extern HWND client;
extern int CurrentStatus;
extern char CurrentArea[LEN_AREATAG+1];
extern FOLDERANCHOR FolderAnchor;
extern AREALIST arealiste;

/*--------------------------- Funktionsprototypen ---------------------------*/
static MRESULT EXPENTRY NewAContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static void InitAreaContainer(HWND hwndDlg, PAREALISTDATA pAreaListData);
static void CleanupAreaList(HWND hwndContainer);
static BOOL DrawItem(POWNERITEM Item);
static SHORT _System SortOrig(PRECORDCORE p1, PRECORDCORE p2, PVOID AreaList);
static SHORT _System SortName(PRECORDCORE p1, PRECORDCORE p2, PVOID AreaList);
static SHORT _System SortNumber(PRECORDCORE p1, PRECORDCORE p2, PVOID AreaList);
static void ResortAreaList(HWND hwndDlg, ULONG ulSortType);
static MRESULT EXPENTRY NewAEditProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static BOOL ScrollToRecord(HWND hwndCnr, PRECORDCORE pRecord);
static PAREARECORD UpdateAreaList(HWND hwndCnr, PAREALISTDATA pAreaListData, char *pchCurrArea);
static MRESULT EXPENTRY SquishParamsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static void MarkSelectedAreas(PAREALIST arealist, ULONG ulFlags,
                              PAREARECORD selectRecord, HWND hwndCnr);

static MRESULT DragOverAreaList(HWND hwndDlg, PCNRDRAGINFO pCnrDrag);
static void DropAreaList(HWND hwndDlg, PCNRDRAGINFO pCnrDrag);
static void UpdateAreaNums(PAREARECORD pRecord);
static void InitAreaRecord(PAREARECORD pRecord, PAREADEFLIST pAreaDef, PAREALISTDATA pAreaListData);


static void InitFolderContainer(HWND hwndDlg, PAREALISTDATA pAreaListData);
static void InsertChildFolders(HWND hwndCnr, PFOLDERRECORD pParentRecord, PAREALISTDATA pAreaListData, LONG FolderToOpen);
static PFOLDERRECORD InsertFolderRecord(HWND hwndCnr, PAREAFOLDER pFolder, PFOLDERRECORD pParent, PAREALISTDATA pAreaListData, LONG FolderToOpen);
static void OpenFolderContext(HWND hwndDlg, PAREALISTDATA pAreaListData, PFOLDERRECORD pRecord);
static PFOLDERRECORD CreateNewFolder(HWND hwndDlg, PAREALISTDATA pAreaListData);
static void OpenFolder(HWND hwndDlg, PAREALISTDATA pAreaListData, PFOLDERRECORD pFolder);
static void SetFolderIcons(HWND hwndDlg, PAREALISTDATA pAreaListData);
static void InitAreaDrag(HWND hwndDlg, PAREALISTDATA pAreaListData, PCNRDRAGINIT pInit);
static void FillDragItem(HWND hwndDlg, PAREARECORD pRecord, PDRAGITEM dItem);
static BOOL IsBetweenContainers(HWND hwndDlg, SHORT x, SHORT y);
static void SetPointer(void);
static void TrackSeparator(HWND hwndDlg);
static void RepositionContainers(HWND hwndDlg);
static MRESULT DragOverFolder(HWND hwndDlg, PCNRDRAGINFO pInfo);
static void DropOnFolder(PCNRDRAGINFO pInfo);
static void DeleteFolder(HWND hwndDlg, PAREALISTDATA pAreaListData, PFOLDERRECORD pFolder);
static void InitFolderDrag(HWND hwndDlg, PAREALISTDATA pAreaListData, PCNRDRAGINIT pInit);
static SHORT EXPENTRY SortFolders(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage);
static void UpdateScannedFolders(HWND hwndDlg, PAREALISTDATA pAreaListData);
static BOOL FolderHasUnreadAreas(PAREAFOLDER pFolder);

/* Sortierfunktionen */
typedef SHORT (* _System SortFuncType)(PRECORDCORE, PRECORDCORE, PVOID);

static SortFuncType SortFuncs[] =
{
   SortOrig,
   SortName,
   SortNumber
};


/*------------------------------ AreaListProc -------------------------------*/
/* Dialog-Prozedur der Area-Liste                                            */
/*---------------------------------------------------------------------------*/
/* Parameter bei Initialisierung:                                            */
/* NULL            Speicher selbst belegen je nach Platzbedarf, Zeiger zu-   */
/*                 rueckgeben;                                               */
/* !=NULL          Speicher ist schon belegt, Rueckgabewerte werden dorthin  */
/*                 geschrieben;                                              */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY AreaListProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWFONTS windowfonts;
   extern DIRTYFLAGS dirtyflags;
   extern GENERALOPT generaloptions;
   extern BOOL DoingAreaScan;
   extern BOOL DoingFind;
   extern int tidAreaScan;
   extern HWND hwndhelp;
   extern HAB anchor;
   extern BOOL bDoingWork;
   extern DRIVEREMAP driveremap;
   AREADEFLIST *zeiger;
   RECORDINSERT recordInsert;
   PAREARECORD pRecords, pfirstRecord;
   MRESULT resultbuf=0;
   ULONG ulAreaCount=0;
   char *pchTemp;
   AREAPAR AreaPar;
   RECTL rectl, rectl2;
   SWP swp;
   SQUISHPARAMSPAR SqParamsPar;
   RENUMBERPAR RenumberPar;
   PAREALISTDATA pAreaListData;
   HWND hwndCnr = WinWindowFromID(parent, IDD_AREALIST+1);

   pAreaListData=(PAREALISTDATA) WinQueryWindowULong(parent, QWL_USER);

   switch(message)
   {
      case WM_INITDLG:
         /* Instanzdaten-Block anfordern */
         pAreaListData=calloc(1, sizeof(AREALISTDATA));
         WinSetWindowULong(parent, QWL_USER, (ULONG) pAreaListData);

         /* Instanzdaten vorbelegen */
         pAreaListData->pchParamString = &((AREALISTPAR *)mp2)->pchString;
         pAreaListData->ulIncludeTypes = ((AREALISTPAR *)mp2)->ulIncludeTypes;
         pAreaListData->bExtendedSel   = ((AREALISTPAR *)mp2)->bExtendedSel;
         pAreaListData->bChange        = ((AREALISTPAR *)mp2)->bChange;

         /* Titel-Override */
         if (((AREALISTPAR *)mp2)->idTitle)
         {
            char pchTitle[100];

            LoadString(((AREALISTPAR *)mp2)->idTitle, sizeof(pchTitle), pchTitle);
            WinSetWindowText(parent, pchTitle);
         }

         if (pAreaListData->bChange)
            pAreaListData->hSwitch=AddToWindowList(parent);

         pAreaListData->hwndpopup=WinLoadMenu(hwndCnr, hmodLang, IDM_AREAPOPUP);
         pAreaListData->hwndpopupsmall=WinLoadMenu(HWND_DESKTOP, hmodLang, IDM_SAREAPOPUP);
         if (pAreaListData->hwndpopupsmall)
            ReplaceSysMenu(parent, pAreaListData->hwndpopupsmall, 1);

         pAreaListData->hwndFolderPopup=WinLoadMenu(HWND_DESKTOP, hmodLang, IDM_FOLDERPOPUP);
         pAreaListData->hwndSmallFolderPopup=WinLoadMenu(HWND_DESKTOP, hmodLang, IDM_SFOLDERPOPUP);

         SetForeground(hwndCnr, &arealistoptions.lEchoAreaColor);
         SetForeground(WinWindowFromID(parent,IDD_AREALIST+4), &arealistoptions.lFolderFore);
         SetBackground(hwndCnr, &arealistoptions.lBackColor);
         SetBackground(WinWindowFromID(parent,IDD_AREALIST+4), &arealistoptions.lFolderBack);
         SetFont(hwndCnr, windowfonts.arealistfont);
         SetFont(WinWindowFromID(parent,IDD_AREALIST+4), windowfonts.areafolderfont);

         pAreaListData->icon=LoadIcon(IDB_AREA);
         SendMsg(parent, WM_SETICON, (MPARAM) pAreaListData->icon, (MPARAM) 0);

         pAreaListData->icnFolder=LoadIcon(IDIC_AREAFOLDER);
         pAreaListData->icnFolderOpen=LoadIcon(IDIC_AREAFOLDER_OPEN);
         pAreaListData->icnFolderUnread=LoadIcon(IDIC_AREAFOLDER_UNREAD);
         pAreaListData->hptrPlus=LoadIcon(IDIC_PLUS);
         pAreaListData->hptrMinus=LoadIcon(IDIC_MINUS);

         /* Folder-Container */
         if (FolderAnchor.lSplit)
            RepositionContainers(parent);
         InitFolderContainer(parent, pAreaListData);

         if (pAreaListData->bChange)
         {
            if (arealistoptions.ulFlags & AREALISTFLAG_FOREGROUND)
            {
               pAreaListData->bForeground = TRUE;
               WinCheckMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_FGROUND, TRUE);
               WinSetOwner(parent, client);
            }
            else
            {
               pAreaListData->bForeground = FALSE;
               WinCheckMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_FGROUND, FALSE);
               WinSetOwner(parent, HWND_DESKTOP);
            }
         }

         OldContainerProc=WinSubclassWindow(hwndCnr,
                                            NewAContainerProc);
         WinSubclassWindow(WinWindowFromID(parent, IDD_AREALIST+4),
                                            NewAContainerProc);
         if (pAreaListData->bChange)
            RestoreWinPos(parent, &arealistoptions.ListPos, TRUE, FALSE);
         else
            RestoreWinPos(parent, &arealistoptions.SelectPos, TRUE, FALSE);
         pAreaListData->bAcceptChange=TRUE;
         if (!pAreaListData->bChange)
            WinEnableControl(parent, IDD_AREALIST+2, FALSE);
         else
            WinEnableControl(parent, IDD_AREALIST+2, !DoingAreaScan);

         /* minimale Fensterbreite berechnen */
         WinQueryWindowPos(WinWindowFromID(parent, IDD_AREALIST+3), &swp);
         pAreaListData->pointl.x=swp.x+swp.cx+10;
         pAreaListData->pointl.y=160;

         InitAreaContainer(parent, pAreaListData);

         pAreaListData->selectRecord=UpdateAreaList(hwndCnr,
                                                    pAreaListData,
                                                    *pAreaListData->pchParamString);
         free(*pAreaListData->pchParamString);
         *pAreaListData->pchParamString=NULL;
         if (pAreaListData->bChange)
            pAreaListData->pchParamString=NULL;

         if (pAreaListData->selectRecord)
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS,
                    pAreaListData->selectRecord, MPFROM2SHORT(TRUE, CRA_CURSORED | CRA_SELECTED));
         pfirstRecord=SendMsg(hwndCnr, CM_QUERYRECORD,
                              NULL, MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
         if (pAreaListData->selectRecord != pfirstRecord)
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS,
                    pfirstRecord, MPFROM2SHORT(FALSE, CRA_SELECTED));
         SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, MPFROM2SHORT(0, CMA_TEXTCHANGED));

         /* Ausgewaehlte Area anfahren */
         if (pAreaListData->selectRecord)
            ScrollToRecord(hwndCnr,
                           (PRECORDCORE) pAreaListData->selectRecord);

         SetInitialAccel(parent);
         WinShowWindow(parent, TRUE);
         break;

      case WM_QUERYTRACKINFO:
         WinQueryWindowPos(parent, &swp);
         if (swp.fl & SWP_MINIMIZE)
            break;

         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(parent,message,mp1,mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=pAreaListData->pointl.x;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=pAreaListData->pointl.y;
         return resultbuf;

      case WM_ACTIVATE:
         if (mp1)
         {
            WinAssociateHelpInstance(hwndhelp, parent);
            if (pAreaListData->bDirty)
            {
#if 1
               WinEnableWindowUpdate(WinWindowFromID(hwndCnr, CID_LEFTDVWND), FALSE);
#endif
               SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
               if (pAreaListData->ulSort == FOLDER_SORT_UNREAD)
                  ResortAreaList(parent, pAreaListData->ulSort);
#if 1
               WinEnableWindowUpdate(WinWindowFromID(hwndCnr, CID_LEFTDVWND), TRUE);
#endif

               pAreaListData->bDirty = FALSE;
            }
         }
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CLOSE:
         WinPostMsg(client, ALM_CLOSE, (MPARAM) parent, NULL);
         break;

      case WM_DRAWITEM:
         if (SHORT1FROMMP(mp1)==IDD_AREALIST+1)
            return (MRESULT) DrawItem((POWNERITEM)mp2);
         else
            return (MRESULT) FALSE;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
         {
            WinShowWindow(WinWindowFromID(parent, DID_OK), FALSE);
            WinShowWindow(hwndCnr, FALSE);
         }
         if (((PSWP)mp1)->fl & (SWP_MAXIMIZE|SWP_RESTORE))
         {
            WinShowWindow(WinWindowFromID(parent, DID_OK), TRUE);
            WinShowWindow(hwndCnr, TRUE);
         }
         break;

      case WM_ADJUSTFRAMEPOS:
         if (((PSWP)mp1)->fl & (SWP_SIZE|SWP_MAXIMIZE|SWP_MINIMIZE|SWP_RESTORE))
         {
            SWP swp;

            rectl.xLeft=0;
            rectl.xRight=((PSWP)mp1)->cx;
            rectl.yBottom=0;
            rectl.yTop=((PSWP)mp1)->cy;

            CalcClientRect(anchor, parent, &rectl);
            WinQueryWindowPos(WinWindowFromID(parent, DID_OK), &swp);
            rectl.yBottom += swp.y + swp.cy;
            WinQueryWindowPos(WinWindowFromID(parent, IDD_AREALIST+4), &swp);
            WinSetWindowPos(WinWindowFromID(parent, IDD_AREALIST+4),
                            NULLHANDLE,
                            rectl.xLeft, rectl.yBottom,
                            swp.cx, rectl.yTop-rectl.yBottom,
                            SWP_MOVE | SWP_SIZE);
            WinSetWindowPos(hwndCnr,
                            NULLHANDLE,
                            rectl.xLeft+swp.cx+CONTAINERSPACE, rectl.yBottom,
                            rectl.xRight-rectl.xLeft-swp.cx-CONTAINERSPACE, rectl.yTop-rectl.yBottom,
                            SWP_MOVE | SWP_SIZE);
         }
         break;

      case WM_WINDOWPOSCHANGED:
         if (pAreaListData && pAreaListData->bAcceptChange)
         {
            extern DIRTYFLAGS dirtyflags;

            if (pAreaListData->bChange)
               SaveWinPos(parent, (PSWP) mp1, &arealistoptions.ListPos, &dirtyflags.alsettingsdirty);
            else
               SaveWinPos(parent, (PSWP) mp1, &arealistoptions.SelectPos, &dirtyflags.alsettingsdirty);
         }
         break;

      case WM_DESTROY:
         RemoveFromWindowList(pAreaListData->hSwitch);
         QueryFont(hwndCnr, windowfonts.arealistfont);
         QueryFont(WinWindowFromID(parent,IDD_AREALIST+4), windowfonts.areafolderfont);
         WinDestroyPointer(pAreaListData->icon);
         WinDestroyPointer(pAreaListData->icnFolder);
         WinDestroyPointer(pAreaListData->icnFolderOpen);
         WinDestroyPointer(pAreaListData->icnFolderUnread);
         WinDestroyPointer(pAreaListData->hptrPlus);
         WinDestroyPointer(pAreaListData->hptrMinus);
         WinDestroyWindow(pAreaListData->hwndpopup);
         WinDestroyWindow(pAreaListData->hwndpopupsmall);
         WinDestroyWindow(pAreaListData->hwndFolderPopup);
         WinDestroyWindow(pAreaListData->hwndSmallFolderPopup);
         CleanupAreaList(hwndCnr);

         if (pAreaListData->bForeground)
         {
            if (!(arealistoptions.ulFlags & AREALISTFLAG_FOREGROUND))
            {
               arealistoptions.ulFlags |= AREALISTFLAG_FOREGROUND;
               dirtyflags.alsettingsdirty = TRUE;
            }
         }
         else
         {
            if (arealistoptions.ulFlags & AREALISTFLAG_FOREGROUND)
            {
               arealistoptions.ulFlags &= ~AREALISTFLAG_FOREGROUND;
               dirtyflags.alsettingsdirty = TRUE;
            }
         }
         if (pAreaListData->pOpenFolder &&
             FolderAnchor.LastFolder != pAreaListData->pOpenFolder->pAreaFolder->FolderID)
         {
            FolderAnchor.LastFolder = pAreaListData->pOpenFolder->pAreaFolder->FolderID;
            FolderAnchor.bDirty = TRUE;
         }
         free(pAreaListData);
         break;

      case WM_SYSCOMMAND:
         if (SHORT1FROMMP(mp1) == SC_MINIMIZE &&
             !pAreaListData->bChange)
            return (MRESULT) FALSE;
         else
            break;

      case WM_COMMAND:
         if (pAreaListData->lDisable > 0)
            return (MRESULT) FALSE;
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            switch(SHORT1FROMMP(mp1))
            {
               case DID_OK:
                  if (pAreaListData->bChange)
                  {
                     pRecords=SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                                      (MPARAM)CMA_FIRST, MPFROMSHORT(CRA_SELECTED));
                     if (pRecords > (PAREARECORD)NULL)
                     {
                        SendMsg(client, ALM_SWITCHAREA, pRecords->pAreaDef->areadata.areatag, NULL);
                        SetFocusControl(client, IDML_MAINEDIT);
                        return (MRESULT) FALSE;
                     }
                     else
                        return (MRESULT) FALSE;
                  }
                  else
                  {
                     pchTemp=NULL;
                     ulAreaCount=0;

                     pRecords=(PAREARECORD)CMA_FIRST;
                     while((pRecords=SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                                             pRecords, MPFROMSHORT(CRA_SELECTED)))>(PAREARECORD)NULL)
                     {
                        ulAreaCount++;
                        pchTemp=realloc(pchTemp, ulAreaCount * (LEN_AREATAG+2));
                        if (ulAreaCount==1)
                           pchTemp[0]='\0';
                        if (ulAreaCount>1)
                           strcat(pchTemp, " ");
                        strcat(pchTemp, pRecords->pAreaDef->areadata.areatag);
                     }
                     if (ulAreaCount)
                        *pAreaListData->pchParamString=pchTemp;
                     else
                        *pAreaListData->pchParamString=NULL;
                  }
                  break;

               /* Scan-Button */
               case IDD_AREALIST+2:
                  WinEnableControl(parent, IDD_AREALIST+2, FALSE);
                  MarkAllAreas(&arealiste, pAreaListData->pOpenFolder->pAreaFolder->FolderID, WORK_SCAN);
                  if ((tidAreaScan=_beginthread(&ScanAreas, NULL, 16384, &arealiste))==-1)
                  {
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                     WinEnableControl(parent, IDD_AREALIST+2, TRUE);
                  }
                  return (MRESULT) FALSE;

               case DID_CANCEL:
                  WinPostMsg(client, ALM_CLOSE, (MPARAM) parent, NULL);
                  break;

               default:
                  break;
            }
         }
         if (SHORT1FROMMP(mp2)==CMDSRC_MENU)
         {
            PWORKDATA pWorkData=NULL, pWorkData2=NULL;
            PAREARECORD *ppChangeRecords=NULL;
            ULONG ulNum;

            switch(SHORT1FROMMP(mp1))
            {
               case IDM_AP_SETTINGS:
                  ulNum = CollectRecordPointers(hwndCnr, (PRECORDCORE**)&ppChangeRecords, (PRECORDCORE) pAreaListData->selectRecord);
                  if (ulNum == 1)
                  {
                     if (MSG_LockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste))
                     {
                        MessageBox(parent, IDST_MSG_LOCKERROR, 0,
                                   IDD_LOCKERROR, MB_OK | MB_ERROR);
                        return (MRESULT) FALSE;
                     }
                     AreaPar.cb=sizeof(AREAPAR);
                     AreaPar.pAreaDef=pAreaListData->selectRecord->pAreaDef;
                     AreaPar.bMultiple=FALSE;
                     if (WinDlgBox(HWND_DESKTOP,
                                   parent,
                                   &AreaSettingsProc,
                                   hmodLang,
                                   IDD_AREASETTINGS,
                                   &AreaPar)!= DID_OK)
                     {
                        MSG_UnlockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste);
                        break;
                     }

                     MSG_UnlockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste);
                     WinEnableWindowUpdate(hwndCnr, FALSE);
                     SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, MPFROM2SHORT(0, CMA_TEXTCHANGED));
                  }
                  else
                     if (ulNum > 1)
                     {
                        int i;
                        AREADEFLIST AreaDef;

                        for (i=0; i<ulNum; i++)
                           if (MSG_LockArea(ppChangeRecords[i]->pAreaDef->areadata.areatag, &arealiste))
                           {
                              int j;

                              MessageBox(parent, IDST_MSG_LOCKERROR, 0,
                                         IDD_LOCKERROR, MB_OK | MB_ERROR);
                              /* bisherige wieder unlocken */
                              for (j=0; j<i; j++)
                                 MSG_UnlockArea(ppChangeRecords[j]->pAreaDef->areadata.areatag, &arealiste);

                              free(ppChangeRecords);
                              return (MRESULT) FALSE;
                           }
                        memcpy(&AreaDef, ppChangeRecords[0]->pAreaDef, sizeof(AreaDef));
                        AreaPar.cb=sizeof(AREAPAR);
                        AreaPar.pAreaDef=&AreaDef;
                        AreaPar.bMultiple=TRUE;
                        if (WinDlgBox(HWND_DESKTOP,
                                      parent,
                                      &AreaSettingsProc,
                                      hmodLang,
                                      IDD_AREASETTINGS,
                                      &AreaPar)!= DID_OK)
                        {
                           for (i=0; i<ulNum; i++)
                              MSG_UnlockArea(ppChangeRecords[i]->pAreaDef->areadata.areatag, &arealiste);
                           free(ppChangeRecords);
                           break;
                        }

                        for (i=0; i<ulNum; i++)
                        {
                           /* Aenderungen uebernehmen */
                           ppChangeRecords[i]->pAreaDef->areadata.ulTemplateID   = AreaDef.areadata.ulTemplateID;
                           ppChangeRecords[i]->pAreaDef->areadata.ulDefAttrib    = AreaDef.areadata.ulDefAttrib;
                           ppChangeRecords[i]->pAreaDef->areadata.ulAreaOpt      &= AREAOPT_FROMCFG;
                           ppChangeRecords[i]->pAreaDef->areadata.ulAreaOpt      |= (AreaDef.areadata.ulAreaOpt & ~AREAOPT_FROMCFG);
                           strcpy(ppChangeRecords[i]->pAreaDef->areadata.address,  AreaDef.areadata.address);
                           strcpy(ppChangeRecords[i]->pAreaDef->areadata.username, AreaDef.areadata.username);
                           ppChangeRecords[i]->pAreaDef->dirty=TRUE;

                           MSG_UnlockArea(ppChangeRecords[i]->pAreaDef->areadata.areatag, &arealiste);
                        }
                        arealiste.bDirty = TRUE;
                        WinEnableWindowUpdate(hwndCnr, FALSE);
                        SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, MPFROM2SHORT(0, CMA_TEXTCHANGED));
                     }
                  ResortAreaList(parent, pAreaListData->ulSort);
                  SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, MPFROM2SHORT(0, CMA_TEXTCHANGED));
                  WinEnableWindowUpdate(hwndCnr, TRUE);
                  ScrollToRecord(hwndCnr, NULL);
                  free(ppChangeRecords);
                  break;

               case IDM_SAP_CREATE:
               case IDM_AP_CREATE:
                  AreaPar.cb=sizeof(AREAPAR);
                  AreaPar.pAreaDef=malloc(sizeof(AREADEFLIST));
                  AreaPar.bMultiple=FALSE;
                  memset(AreaPar.pAreaDef, 0, sizeof(AREADEFLIST));
                  /* Daten auffuellen */
                  AreaPar.pAreaDef->areadata.areatype = AREATYPE_LOCAL;
                  if (pAreaListData->pOpenFolder)
                     AreaPar.pAreaDef->areadata.ulFolderID = pAreaListData->pOpenFolder->pAreaFolder->FolderID;

                  if (WinDlgBox(HWND_DESKTOP,
                                parent,
                                &AreaSettingsProc,
                                hmodLang,
                                IDD_AREASETTINGS,
                                &AreaPar) != DID_OK)
                  {
                     free(AreaPar.pAreaDef);
                     break;
                  }
                  if (!AreaPar.pAreaDef->areadata.areatag[0])
                  {
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                     free(AreaPar.pAreaDef);
                     break;
                  }
                  {
                     AREADEFLIST *pNewArea;

                     /* Area anhaengen */
                     if (pNewArea = AM_AddArea(&arealiste, &AreaPar.pAreaDef->areadata, ADDAREA_TAIL | ADDAREA_MARKDIRTY))
                     {
                        pRecords=SendMsg(hwndCnr, CM_ALLOCRECORD,
                                         MPFROMLONG(sizeof(AREARECORD)-sizeof(RECORDCORE)),
                                         MPFROMLONG(1));

                        InitAreaRecord(pRecords, pNewArea, pAreaListData);

                        recordInsert.cb = sizeof(RECORDINSERT);
                        recordInsert.pRecordParent= NULL;
                        recordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
                        recordInsert.zOrder = CMA_TOP;
                        recordInsert.cRecordsInsert = 1;
                        recordInsert.fInvalidateRecord = TRUE;

                        SendMsg(hwndCnr, CM_INSERTRECORD, pRecords, &recordInsert);
                        ResortAreaList(parent, pAreaListData->ulSort);
                        arealiste.bDirty = TRUE;
                     }
                     free(AreaPar.pAreaDef);
                  }
                  break;

               case IDM_AP_DELETE:
                  if (MSG_LockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste))
                  {
                     MessageBox(parent, IDST_MSG_LOCKERROR, 0,
                                IDD_LOCKERROR, MB_OK | MB_ERROR);
                     return (MRESULT) FALSE;
                  }
                  if (generaloptions.safety & SAFETY_CHANGESETUP)
                  {
                     if (MessageBox(parent, IDST_MSG_DELAREA, IDST_AL_DELETE,
                                    IDD_DELETEAREA, MB_YESNO | MB_ICONQUESTION |
                                    MB_DEFBUTTON2)==MBID_NO)
                     {
                        MSG_UnlockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste);
                        break;
                     }
                  }
                  /* Area loeschen */
                  AM_DeleteAreaDirect(&arealiste, pAreaListData->selectRecord->pAreaDef);

                  SendMsg(hwndCnr, CM_REMOVERECORD, &pAreaListData->selectRecord,
                          MPFROM2SHORT(1, CMA_FREE | CMA_INVALIDATE));
                  arealiste.bDirty = TRUE;
                  ScrollToRecord(hwndCnr, NULL);
                  break;

               case IDM_AP_SQPARAMS:
                  if (MSG_LockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste))
                  {
                     MessageBox(parent, IDST_MSG_LOCKERROR, 0,
                                IDD_LOCKERROR, MB_OK | MB_ERROR);
                     return (MRESULT) FALSE;
                  }
                  SqParamsPar.cb=sizeof(SQUISHPARAMSPAR);
                  if (!MSG_ReadSquishParams(&SqParamsPar.SquishParams,
                                            &arealiste, pAreaListData->selectRecord->pAreaDef->areadata.areatag, &driveremap))
                  {
                     if (WinDlgBox(HWND_DESKTOP, parent, SquishParamsProc,
                                   hmodLang, IDD_SQUISHBASEPARAM, &SqParamsPar)==DID_OK)
                     {
                        /* Parameter wieder speichern */
                        if (MSG_WriteSquishParams(&SqParamsPar.SquishParams,
                                                  &arealiste, pAreaListData->selectRecord->pAreaDef->areadata.areatag, &driveremap))
                        {
                           MessageBox(parent, IDST_MSG_ERRWRITESQPARAM, 0, IDD_ERRWRITESQPARAM,
                                      MB_OK | MB_ERROR);
                        }
                     }
                  }
                  else
                  {
                     MessageBox(parent, IDST_MSG_ERRREADSQPARAM, 0, IDD_ERRREADSQPARAM,
                                MB_OK | MB_ERROR);
                  }
                  MSG_UnlockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste);
                  break;

               case IDM_AP_RENUMBER:
                  if (MSG_LockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste))
                  {
                     MessageBox(parent, IDST_MSG_LOCKERROR, 0,
                                IDD_LOCKERROR, MB_OK | MB_ERROR);
                     return (MRESULT) FALSE;
                  }
                  RenumberPar.cb=sizeof(RENUMBERPAR);
                  strcpy(RenumberPar.pchArea, pAreaListData->selectRecord->pAreaDef->areadata.areatag);
                  RenumberPar.arealist=&arealiste;
                  WinDlgBox(HWND_DESKTOP, parent, RenumberProc, hmodLang,
                            IDD_RENUMBER, &RenumberPar);
                  MSG_UnlockArea(pAreaListData->selectRecord->pAreaDef->areadata.areatag, &arealiste);
                  break;

               case IDM_AP_SCAN:
                  MarkSelectedAreas(&arealiste, WORK_SCAN,
                                    pAreaListData->selectRecord,
                                    hwndCnr);
                  WinEnableControl(parent, IDD_AREALIST+2, FALSE);
                  if ((tidAreaScan=_beginthread(&ScanAreas, NULL, 16384, &arealiste))==-1)
                  {
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                     WinEnableControl(parent, IDD_AREALIST+2, TRUE);
                  }
                  break;

               case IDM_SAP_SETTINGS:
                  {
                     ALSETTINGSPARAM ALSettingsParam;

                     ALSettingsParam.cb=sizeof(ALSettingsParam);
                     ALSettingsParam.pAreaListOptions=&arealistoptions;
#if 0
                     ALSettingsParam.bTagChanged = FALSE;
#endif

                     WinDlgBox(HWND_DESKTOP, parent, AreaListSettingsProc,
                               hmodLang, IDD_AREALISTSETTINGS, &ALSettingsParam);
                     if (dirtyflags.alsettingsdirty)
                     {
                        WinEnableWindowUpdate(hwndCnr, FALSE);
                        pAreaListData->bAcceptChange=FALSE;
                        SetForeground(hwndCnr, &arealistoptions.lEchoAreaColor);
                        SetBackground(hwndCnr, &arealistoptions.lBackColor);
                        pAreaListData->bAcceptChange=TRUE;

                        UpdateAreaList(hwndCnr, pAreaListData, NULL);
                        ResortAreaList(parent, pAreaListData->ulSort);
                        SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);

                        WinEnableWindowUpdate(hwndCnr, TRUE);
                     }
                  }
                  break;

               case IDM_SAP_SCANALL:
                  WinEnableControl(parent, IDD_AREALIST+2, FALSE);
                  MarkAllAreas(&arealiste, FOLDERID_ALL, WORK_SCAN);
                  if ((tidAreaScan=_beginthread(&ScanAreas, NULL, 16384, &arealiste))==-1)
                  {
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                     WinEnableControl(parent, IDD_AREALIST+2, TRUE);
                  }
                  break;

               case IDM_SAP_FGROUND:
                  if (pAreaListData->bForeground)
                  {
                     pAreaListData->bForeground = FALSE;
                     WinCheckMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_FGROUND, FALSE);
                     WinSetOwner(parent, HWND_DESKTOP);
                  }
                  else
                  {
                     pAreaListData->bForeground = TRUE;
                     WinCheckMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_FGROUND, TRUE);
                     WinSetOwner(parent, client);
                  }

                  break;

               case IDM_AP_CATCHUP:
                  if (bDoingWork)
                  {
                     /* Fehlermeldung */
                     MessageBox(parent, IDST_MSG_DOINGWORK, 0, IDD_DOINGWORK,
                                MB_OK);
                     break;
                  }
                  if (generaloptions.safety & SAFETY_CATCHUP)
                     if (MessageBox(parent, IDST_MSG_CATCHUP, IDST_TITLE_CATCHUP,
                                    IDD_CATCHUP, MB_YESNO) != MBID_YES)
                        break;
                  if (pAreaListData->selectRecord->RecordCore.flRecordAttr & CRA_SELECTED)
                  {
                     /* Alle selektierten Areas absuchen */
                     pRecords=SendMsg(hwndCnr,
                                      CM_QUERYRECORDEMPHASIS, (MPARAM) CMA_FIRST,
                                      MPFROMSHORT(CRA_SELECTED));

                     while (pRecords)
                     {
                        if (pWorkData)
                        {
                           pWorkData->next=calloc(1, sizeof(WORKDATA));
                           pWorkData = pWorkData->next;
                        }
                        else
                        {
                           pWorkData=calloc(1, sizeof(WORKDATA));
                           pWorkData2=pWorkData;
                        }

                        strcpy(pWorkData->pchSrcArea, pRecords->pAreaDef->areadata.areatag);
                        pWorkData->flWorkToDo = WORK_MARKALL;

                        pRecords=SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecords,
                                         MPFROMSHORT(CRA_SELECTED));
                     }
                  }
                  else
                  {
                     /* nur gew„hlte Area bearbeiten */
                     pWorkData=calloc(1, sizeof(WORKDATA));

                     strcpy(pWorkData->pchSrcArea, pAreaListData->selectRecord->pAreaDef->areadata.areatag);
                     pWorkData->flWorkToDo = WORK_MARKALL;
                     pWorkData2=pWorkData;
                  }
                  if (pWorkData2)
                  {
                     extern int tidWorker;

                     bDoingWork=TRUE;
                     tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData2);
                  }
                  break;

               case IDM_FP_CREATE:
               case IDM_SFP_CREATE:
                  CreateNewFolder(parent, pAreaListData);
                  break;

               case IDM_FP_OPEN:
                  if (pAreaListData->selectFolder)
                     OpenFolder(parent, pAreaListData, pAreaListData->selectFolder);
                  break;

               case IDM_FP_SCAN:
                  if (pAreaListData->selectFolder)
                  {
                     pAreaListData->selectFolder->pAreaFolder->ulFlags ^= FOLDER_AUTOSCAN;
                     pAreaListData->selectFolder->pAreaFolder->bDirty = TRUE;
                     FolderAnchor.bDirty = TRUE;
                  }
                  break;

               case IDM_FP_SORT_NONE:
                  pAreaListData->selectFolder->pAreaFolder->ulFlags &= ~FOLDER_SORT_MASK;
                  pAreaListData->selectFolder->pAreaFolder->ulFlags |= FOLDER_SORT_UNSORTED;
                  pAreaListData->selectFolder->pAreaFolder->bDirty = TRUE;
                  FolderAnchor.bDirty = TRUE;
                  if (pAreaListData->pOpenFolder == pAreaListData->selectFolder)
                  {
                     pAreaListData->ulSort = FOLDER_SORT_UNSORTED;
                     ResortAreaList(parent, FOLDER_SORT_UNSORTED);
                  }
                  break;

               case IDM_FP_SORT_NAME:
                  pAreaListData->selectFolder->pAreaFolder->ulFlags &= ~FOLDER_SORT_MASK;
                  pAreaListData->selectFolder->pAreaFolder->ulFlags |= FOLDER_SORT_NAME;
                  pAreaListData->selectFolder->pAreaFolder->bDirty = TRUE;
                  FolderAnchor.bDirty = TRUE;
                  if (pAreaListData->pOpenFolder == pAreaListData->selectFolder)
                  {
                     pAreaListData->ulSort = FOLDER_SORT_NAME;
                     ResortAreaList(parent, FOLDER_SORT_NAME);
                  }
                  break;

               case IDM_FP_SORT_UNR:
                  pAreaListData->selectFolder->pAreaFolder->ulFlags &= ~FOLDER_SORT_MASK;
                  pAreaListData->selectFolder->pAreaFolder->ulFlags |= FOLDER_SORT_UNREAD;
                  pAreaListData->selectFolder->pAreaFolder->bDirty = TRUE;
                  FolderAnchor.bDirty = TRUE;
                  if (pAreaListData->pOpenFolder == pAreaListData->selectFolder)
                  {
                     pAreaListData->ulSort = FOLDER_SORT_UNREAD;
                     ResortAreaList(parent, FOLDER_SORT_UNREAD);
                  }
                  break;

               case IDM_FP_DELETE:
                  DeleteFolder(parent, pAreaListData, pAreaListData->selectFolder);
                  break;

               case IDM_SFP_ICONS_LARGE:
                  FolderAnchor.ulFlags = (FolderAnchor.ulFlags & ~AREAFOLDERS_ICONMASK) | AREAFOLDERS_LARGEICONS;
                  FolderAnchor.bDirty = TRUE;

                  SetFolderIcons(parent, pAreaListData);
                  break;

               case IDM_SFP_ICONS_SMALL:
                  FolderAnchor.ulFlags = (FolderAnchor.ulFlags & ~AREAFOLDERS_ICONMASK) | AREAFOLDERS_SMALLICONS;
                  FolderAnchor.bDirty = TRUE;

                  SetFolderIcons(parent, pAreaListData);
                  break;

               case IDM_SFP_ICONS_NONE:
                  FolderAnchor.ulFlags = (FolderAnchor.ulFlags & ~AREAFOLDERS_ICONMASK) | AREAFOLDERS_NOICONS;
                  FolderAnchor.bDirty = TRUE;

                  SetFolderIcons(parent, pAreaListData);
                  break;

               default:
                  break;
            }
            return (MRESULT) FALSE;
         }
         if (SHORT1FROMMP(mp2)==CMDSRC_ACCELERATOR)
            return RedirectCommand(mp1, mp2);
         break;

      case SM_SCANENDED:
         if (pAreaListData->bChange)
            WinEnableControl(parent, IDD_AREALIST+2, TRUE);

         /* Liste updaten */
         pRecords=NULL;
         while (pRecords=SendMsg(hwndCnr, CM_QUERYRECORD, pRecords,
                         MPFROM2SHORT(pRecords?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
            UpdateAreaNums(pRecords);

         WinEnableWindowUpdate(WinWindowFromID(hwndCnr, CID_LEFTDVWND), FALSE);
         SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
         if (pAreaListData->ulSort == FOLDER_SORT_UNREAD)
            ResortAreaList(parent, pAreaListData->ulSort);
         WinEnableWindowUpdate(WinWindowFromID(hwndCnr, CID_LEFTDVWND), TRUE);
         UpdateScannedFolders(parent, pAreaListData);
         return (MRESULT) FALSE;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_AREALIST+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               case CN_CONTEXTMENU:
                  if (DoingAreaScan || DoingFind || pAreaListData->lDisable > 0)
                  {
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                     break;
                  }
                  /* erstmal alles enablen */
                  WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SETTINGS, TRUE);
                  WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_CREATE  , TRUE);
                  WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_DELETE  , TRUE);
                  WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_RENUMBER, TRUE);
                  WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SQPARAMS, TRUE);
                  WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SCAN    , TRUE);
                  WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_CATCHUP , TRUE);

                  pAreaListData->selectRecord=(PAREARECORD)mp2;
                  /* Nicht auf einer Areazeile */
                  if (pAreaListData->selectRecord == NULL)
                     zeiger=NULL;
                  else
                     /* Area-Daten suchen */
                     zeiger=pAreaListData->selectRecord->pAreaDef;

                  if (!zeiger)
                  {
                     /* nicht gefunden oder nicht auf Areazeile */
                     WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_DELETE,   FALSE);
                     WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SETTINGS, FALSE);
                     WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_RENUMBER, FALSE);
                     WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SQPARAMS, FALSE);
                     WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SCAN,     FALSE);
                     WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_CATCHUP,  FALSE);
                     WinQueryWindowRect(hwndCnr, &rectl);
                  }
                  else
                  {
                     QUERYRECORDRECT qrecordc;

                     if (!pAreaListData->bChange)
                        break;

                     qrecordc.cb=sizeof(QUERYRECORDRECT);
                     qrecordc.pRecord=(PRECORDCORE) mp2;
                     qrecordc.fRightSplitWindow=FALSE;
                     qrecordc.fsExtent=CMA_TEXT;
                     SendMsg(hwndCnr, CM_QUERYRECORDRECT, &rectl, &qrecordc);

                     /* sind mehrere Areas markiert */
                     pRecords=SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, (MPARAM) CMA_FIRST,
                                      MPFROMSHORT(CRA_SELECTED));
                     pRecords=SendMsg(hwndCnr,
                                      CM_QUERYRECORDEMPHASIS, pRecords,
                                      MPFROMSHORT(CRA_SELECTED));
                     if (pRecords && (pAreaListData->selectRecord->RecordCore.flRecordAttr & CRA_SELECTED))
                     {
                        WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_DELETE,   FALSE);
                        WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_RENUMBER, FALSE);
                        WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SQPARAMS, FALSE);

                        /* Source-Emphasis auf die selektierten Areas */
                        ApplySourceEmphasis(hwndCnr, (PRECORDCORE) pAreaListData->selectRecord);
                     }

                     if (DoingAreaScan || DoingFind)
                        WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_RENUMBER, FALSE);
                  }
                  if (zeiger)
                  {
                     if (zeiger->areadata.ulAreaOpt & AREAOPT_FROMCFG)
                        WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_DELETE, FALSE);

                     if (zeiger->areadata.areaformat != AREAFORMAT_SQUISH)
                        WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_SQPARAMS, FALSE);

                     if (zeiger->areadata.areaformat != AREAFORMAT_FTS ||
                         CurrentStatus != PROGSTATUS_READING ||
                         !stricmp(CurrentArea, zeiger->areadata.areatag))
                        WinEnableMenuItem(pAreaListData->hwndpopup, IDM_AP_RENUMBER, FALSE);
                  }
                  else
                  {
                     if (!pAreaListData->bChange)
                     {
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_SETTINGS, FALSE);
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_CREATE,   FALSE);
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_SCANALL,  FALSE);
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_FGROUND,  FALSE);
                     }
                     else
                     {
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_SETTINGS, TRUE);
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_CREATE,   TRUE);
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_SCANALL,  TRUE);
                        WinEnableMenuItem(pAreaListData->hwndpopupsmall, IDM_SAP_FGROUND,  TRUE);
                     }
                  }
                  if (pAreaListData->selectRecord)
                     SendMsg(hwndCnr, CM_SETRECORDEMPHASIS,
                                       pAreaListData->selectRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

                  if (pAreaListData->bKeyboard)
                  {
                     WinQueryWindowRect(hwndCnr, &rectl2);
                     WinMapWindowPoints(hwndCnr, HWND_DESKTOP, (PPOINTL) &rectl2, 2);

                     if (zeiger)
                        WinPopupMenu(HWND_DESKTOP, parent, pAreaListData->hwndpopup,
                                     rectl2.xLeft+(rectl2.xRight-rectl2.xLeft)/2,
                                     rectl2.yBottom+rectl.yBottom+(rectl.yTop-rectl.yBottom)/2,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
                     else
                        WinPopupMenu(HWND_DESKTOP, parent, pAreaListData->hwndpopupsmall,
                                     rectl2.xLeft+(rectl2.xRight-rectl2.xLeft)/2,
                                     rectl2.yBottom+rectl.yBottom+(rectl.yTop-rectl.yBottom)/2,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
                  }
                  else
                  {
                     POINTL pointl;

                     WinQueryPointerPos(HWND_DESKTOP, &pointl);
                     if (zeiger)
                        WinPopupMenu(HWND_DESKTOP, parent, pAreaListData->hwndpopup,
                                     pointl.x, pointl.y,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
                     else
                        WinPopupMenu(HWND_DESKTOP, parent, pAreaListData->hwndpopupsmall,
                                     pointl.x, pointl.y,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
                  }
                  break;

               case CN_ENTER:
                  /* OK-Button simulieren */
                  WinPostMsg(parent, WM_COMMAND, MPFROMSHORT(DID_OK),
                             MPFROMSHORT(CMDSRC_PUSHBUTTON));
                  break;

               case CN_REALLOCPSZ:
                  if (((PCNREDITDATA)mp2)->cbText > LEN_AREADESC)
                  {
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                     return (MRESULT) FALSE;
                  }
                  else
                  {
                     arealiste.bDirty = TRUE;
                     ((PAREARECORD)((PCNREDITDATA)mp2)->pRecord)->pAreaDef->dirty=TRUE;
                     return (MRESULT) TRUE;
                  }

               case CN_BEGINEDIT:
                  SendMsg(WinWindowFromID(hwndCnr, CID_MLE),
                             MLM_SETTEXTLIMIT, MPFROMLONG(LEN_AREADESC), NULL);
                  OldAEditProc=WinSubclassWindow(WinWindowFromID(hwndCnr, CID_MLE),
                                                 NewAEditProc);
                  pAreaListData->bDirectEdit = TRUE;
                  break;

               case CN_ENDEDIT:
                  WinEnableWindowUpdate(hwndCnr, FALSE);
                  SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, MPFROM2SHORT(0, CMA_TEXTCHANGED));
                  if (pAreaListData->ulSort == FOLDER_SORT_NAME)
                     ResortAreaList(parent, pAreaListData->ulSort);
                  ScrollToRecord(hwndCnr, NULL);
                  WinEnableWindowUpdate(hwndCnr, TRUE);
                  pAreaListData->bDirectEdit = FALSE;
                  break;

               case CN_HELP:
                  SendMsg(parent, WM_HELP, MPFROMSHORT(IDD_AREALIST+1), NULL);
                  break;

               case CN_COLORCHANGED:
                  if (pAreaListData->bAcceptChange)
                  {
                     QueryBackground(hwndCnr, &arealistoptions.lBackColor);
                     QueryForeground(hwndCnr, &arealistoptions.lEchoAreaColor);
                     dirtyflags.alsettingsdirty=TRUE;
                  }
                  break;

               case CN_DRAGOVER:
                  return DragOverAreaList(parent, (PCNRDRAGINFO) mp2);

               case CN_DROP:
                  DropAreaList(parent, (PCNRDRAGINFO) mp2);
                  break;

               case CN_INITDRAG:
                  InitAreaDrag(parent, pAreaListData, (PCNRDRAGINIT) mp2);
                  break;

               default:
                  break;
            }
         }
         if (SHORT1FROMMP(mp1)==IDD_AREALIST+4)
         {
            PCNREDITDATA pEdit;
            PFOLDERRECORD pRecord;
            PNOTIFYRECORDENTER pEnter;

            switch(SHORT2FROMMP(mp1))
            {
               case CN_COLORCHANGED:
                  if (pAreaListData->bAcceptChange)
                  {
                     QueryBackground(WinWindowFromID(parent, IDD_AREALIST+4),
                                     &arealistoptions.lFolderBack);
                     QueryForeground(WinWindowFromID(parent, IDD_AREALIST+4),
                                     &arealistoptions.lFolderFore);
                     dirtyflags.alsettingsdirty=TRUE;
                  }
                  break;

               case CN_CONTEXTMENU:
                  OpenFolderContext(parent, pAreaListData, (PFOLDERRECORD) mp2);
                  break;

               case CN_BEGINEDIT:
                  pAreaListData->bDirectEdit = TRUE;
                  break;

               case CN_ENDEDIT:
                  pAreaListData->bDirectEdit = FALSE;
                  pEdit = (PCNREDITDATA) mp2;
                  SendMsg(pEdit->hwndCnr, CM_SORTRECORD, (MPARAM) SortFolders, NULL);
                  if (pEdit->pRecord == (PRECORDCORE) pAreaListData->pOpenFolder)
                  {
                     CNRINFO cnrinfo;

                     /* Titel neu setzen */
                     cnrinfo.cb = sizeof(cnrinfo);
                     cnrinfo.pszCnrTitle = pAreaListData->pOpenFolder->pAreaFolder->pchName;
                     WinSendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_CNRTITLE));
                  }
                  break;

               case CN_REALLOCPSZ:
                  pEdit = (PCNREDITDATA) mp2;
                  pRecord = (PFOLDERRECORD) pEdit->pRecord;
                  free (pRecord->pAreaFolder->pchName);
                  pRecord->pAreaFolder->pchName = malloc(pEdit->cbText+1);
                  pRecord->pAreaFolder->pchName[0] = '\0';
                  pRecord->RecordCore.pszIcon = pRecord->pAreaFolder->pchName;
                  pRecord->pAreaFolder->bDirty=TRUE;
                  FolderAnchor.bDirty = TRUE;
                  return (MRESULT) TRUE;

               case CN_HELP:
                  SendMsg(parent, WM_HELP, MPFROMSHORT(IDD_AREALIST+4), NULL);
                  break;

               case CN_ENTER:
                  pEnter = (PNOTIFYRECORDENTER) mp2;
                  if (pEnter->pRecord)
                     OpenFolder(parent, pAreaListData, (PFOLDERRECORD) pEnter->pRecord);
                  break;

               case CN_INITDRAG:
                  InitFolderDrag(parent, pAreaListData, (PCNRDRAGINIT) mp2);
                  break;

               case CN_DROP:
                  DropOnFolder((PCNRDRAGINFO) mp2);
                  break;

               case CN_DRAGOVER:
                  return DragOverFolder(parent, (PCNRDRAGINFO) mp2);

               case CN_EXPANDTREE:
                  if (pRecord = (PFOLDERRECORD) mp2)
                  {
                     pRecord->pAreaFolder->ulFlags |= FOLDER_EXPANDED;
                     pRecord->pAreaFolder->bDirty = TRUE;
                     FolderAnchor.bDirty = TRUE;
                  }
                  break;

               case CN_COLLAPSETREE:
                  if (pRecord = (PFOLDERRECORD) mp2)
                  {
                     pRecord->pAreaFolder->ulFlags &= ~FOLDER_EXPANDED;
                     pRecord->pAreaFolder->bDirty = TRUE;
                     FolderAnchor.bDirty = TRUE;
                  }
                  break;

               default:
                  break;
            }
         }
         break;

      case WM_MENUEND:
         if ((HWND) mp2 != pAreaListData->hwndpopup &&
             (HWND) mp2 != pAreaListData->hwndpopupsmall &&
             (HWND) mp2 != pAreaListData->hwndFolderPopup &&
             (HWND) mp2 != pAreaListData->hwndSmallFolderPopup)
            break;
         pAreaListData->bKeyboard=FALSE;

         if ((HWND) mp2 == pAreaListData->hwndpopupsmall)
            ResetMenuStyle(pAreaListData->hwndpopupsmall, parent);

         /* Source-Emphasis loeschen */
         RemoveSourceEmphasis(hwndCnr);
         RemoveSourceEmphasis(WinWindowFromID(parent, IDD_AREALIST+4));
         if ((HWND) mp2 == pAreaListData->hwndpopupsmall && !pAreaListData->selectRecord)
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS,
                              NULL, MPFROM2SHORT(FALSE, CRA_SOURCE));
         if ((HWND) mp2 == pAreaListData->hwndSmallFolderPopup && !pAreaListData->selectFolder)
            WinSendDlgItemMsg(parent, IDD_AREALIST+4, CM_SETRECORDEMPHASIS,
                              NULL, MPFROM2SHORT(FALSE, CRA_SOURCE));
         break;

      case WM_INITMENU:
         if ((HWND) mp2 == pAreaListData->hwndpopupsmall)
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS,
                              NULL, MPFROM2SHORT(TRUE, CRA_SOURCE));
         break;

      case WM_CONTEXTMENU:
      case WM_TEXTEDIT:
         if (!SHORT1FROMMP(mp1))
         {
            if (WinQueryFocus(HWND_DESKTOP)==hwndCnr)
            {
               pAreaListData->bKeyboard=TRUE;
               SendMsg(hwndCnr, message, mp1, mp2);
            }
            else
               if (WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(parent, IDD_AREALIST+4))
               {
                  pAreaListData->bKeyboard=TRUE;
                  WinSendDlgItemMsg(parent, IDD_AREALIST+4, message, mp1, mp2);
               }
         }
         break;

      case WM_TRANSLATEACCEL:
         if (pAreaListData->bDirectEdit)
            return FALSE;
         else
            break;

      case WM_MOUSEMOVE:
         if (IsBetweenContainers(parent, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1)))
         {
            SetPointer();
            return (MRESULT) TRUE;
         }
         break;

      case WM_BUTTON1DOWN:
         if (IsBetweenContainers(parent, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1)))
         {
            TrackSeparator(parent);
         }
         break;

      case WORKM_ENABLEVIEWS:
         if (pAreaListData->lDisable > 0)
            pAreaListData->lDisable--;
         if (pAreaListData->lDisable == 0)
         {
            WinEnableControl(parent, DID_OK, TRUE);
            WinEnableControl(parent, DID_CANCEL, TRUE);
            if (pAreaListData->bChange && !DoingAreaScan)
               WinEnableControl(parent, IDD_AREALIST+2, TRUE);
         }
         break;

      case WORKM_DISABLEVIEWS:
         pAreaListData->lDisable++;
         if (pAreaListData->lDisable > 0)
         {
            WinEnableControl(parent, DID_OK, FALSE);
            WinEnableControl(parent, DID_CANCEL, FALSE);
            WinEnableControl(parent, IDD_AREALIST+2, FALSE);
         }
         break;

      case WORKM_DELETED:
      case WORKM_ADDED:
      case WORKM_READ:
         {
            /* Anzahl der Messages in der Area veraendert sich evtl. */
            PMESSAGEID pMessageID = (PMESSAGEID) mp1;

            /* Liste updaten */
            pRecords=NULL;
            while (pRecords=SendMsg(hwndCnr, CM_QUERYRECORD, pRecords,
                                    MPFROM2SHORT(pRecords?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
            {
               if (!stricmp(pRecords->pAreaDef->areadata.areatag, pMessageID->pchAreaTag))
               {
                  UpdateAreaNums(pRecords);
                  pAreaListData->bDirty=TRUE;

                  break;
               }
            }
         }
         break;

      case WORKM_MARKEND:
         {
            /* Anzahl der Messages in der Area veraendert sich evtl. */
            PMESSAGEID pMessageID = (PMESSAGEID) mp1;

            /* Liste updaten */
            pRecords=NULL;
            while (pRecords=SendMsg(hwndCnr, CM_QUERYRECORD, pRecords,
                            MPFROM2SHORT(pRecords?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
            {
               if (!stricmp(pRecords->pAreaDef->areadata.areatag, pMessageID->pchAreaTag))
               {
                  UpdateAreaNums(pRecords);
                  WinEnableWindowUpdate(WinWindowFromID(hwndCnr, CID_LEFTDVWND), FALSE);
                  SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
                  if (pAreaListData->ulSort == FOLDER_SORT_UNREAD)
                     ResortAreaList(parent, pAreaListData->ulSort);
                  WinEnableWindowUpdate(WinWindowFromID(hwndCnr, CID_LEFTDVWND), TRUE);

                  pAreaListData->bDirty = FALSE;
                  break;
               }
            }
         }
         break;

      case WORKM_SWITCHACCELS:
         SwitchAccels(parent, (ULONG) mp1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

static void InitAreaContainer(HWND hwndDlg, PAREALISTDATA pAreaListData)
{
   CNRINFO cnrinfo;
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+1);
   PFIELDINFO pFieldInfo=NULL;
   PFIELDINFO firstFieldInfo=NULL;
   FIELDINFOINSERT fieldInfoInsert;

   /* Felder im Container aufbauen */
   pFieldInfo = SendMsg(hwndCnr, CM_ALLOCDETAILFIELDINFO, MPFROMLONG(3), NULL);
   firstFieldInfo = pFieldInfo;

   pFieldInfo->cb = sizeof(FIELDINFO);
   pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_LEFT | CFA_OWNER | CFA_SEPARATOR;
   pFieldInfo->flTitle = CFA_FITITLEREADONLY;
   pFieldInfo->pTitleData = pAreaListData->pchTAreaDesc;
   LoadString(IDST_AL_AREADESC, 50, pFieldInfo->pTitleData);
   pFieldInfo->offStruct = FIELDOFFSET(AREARECORD, areadesc);
   pFieldInfo = pFieldInfo->pNextFieldInfo;

   pFieldInfo->cb = sizeof(FIELDINFO);
   pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_RIGHT |
                        CFA_FIREADONLY | CFA_SEPARATOR | CFA_OWNER;
   pFieldInfo->flTitle = CFA_FITITLEREADONLY;
   pFieldInfo->pTitleData = pAreaListData->pchTUnread;
   LoadString(IDST_AL_UNREAD, 50, pFieldInfo->pTitleData);
   pFieldInfo->offStruct = FIELDOFFSET(AREARECORD, msgnotread);
   pFieldInfo = pFieldInfo->pNextFieldInfo;

   pFieldInfo->cb = sizeof(FIELDINFO);
   pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_RIGHT |
                        CFA_FIREADONLY | CFA_OWNER;
   pFieldInfo->flTitle = CFA_FITITLEREADONLY;
   pFieldInfo->pTitleData = pAreaListData->pchTTotal;
   LoadString(IDST_AL_TOTAL, 50, pFieldInfo->pTitleData);
   pFieldInfo->offStruct = FIELDOFFSET(AREARECORD, totlmsgs);

   fieldInfoInsert.cb = (ULONG)(sizeof(FIELDINFOINSERT));
   fieldInfoInsert.pFieldInfoOrder = (PFIELDINFO)CMA_FIRST;
   fieldInfoInsert.cFieldInfoInsert = 3;
   fieldInfoInsert.fInvalidateFieldInfo = TRUE;

   SendMsg(hwndCnr, CM_INSERTDETAILFIELDINFO, firstFieldInfo, &fieldInfoInsert);

   cnrinfo.cb=sizeof(CNRINFO);
   cnrinfo.flWindowAttr=CV_DETAIL | CA_DETAILSVIEWTITLES |
                        CA_CONTAINERTITLE | CA_TITLEREADONLY | CA_TITLESEPARATOR;

   cnrinfo.pSortRecord=(PVOID) SortFuncs[pAreaListData->ulSort];

   SendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_FLWINDOWATTR | CMA_PSORTRECORD));

   if (pAreaListData->bExtendedSel)
   {
      ULONG ulStyle=WinQueryWindowULong(hwndCnr, QWL_STYLE);
      WinSetWindowULong(hwndCnr, QWL_STYLE, ulStyle | CCS_EXTENDSEL);
   }

   return;
}

/*------------------------------ NewContainerProc ---------------------------*/
/* Neue Window-Prozedur f. Container, um OS/2-Bug zu umschiffen              */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewAContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
#if 0
      case DM_DRAGOVER:
         return (MRESULT) DOR_NEVERDROP;
#endif

      case WM_PRESPARAMCHANGED:
         if ((ULONG) mp1 == PP_FOREGROUNDCOLOR ||
             (ULONG) mp1 == PP_BACKGROUNDCOLOR)
         {
            /* Notification an Owner */
            SendMsg(WinQueryWindow(parent, QW_OWNER),
                       WM_CONTROL,
                       MPFROM2SHORT(WinQueryWindowUShort(parent, QWS_ID),
                                    CN_COLORCHANGED),
                       NULL);
         }
         break;

      default:
         break;
   }
   return OldContainerProc(parent, message, mp1, mp2);
}

/*------------------------------ CleanupAreaList  ---------------------------*/
/* Gibt den Speicher fuer die Textfelder wieder frei, entfernt alle Records  */
/*---------------------------------------------------------------------------*/

static void CleanupAreaList(HWND hwndContainer)
{
   PAREARECORD pRecord=NULL;

   while(pRecord=SendMsg(hwndContainer, CM_QUERYRECORD,
                                 pRecord, MPFROM2SHORT(pRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
   {
      if (pRecord->msgnotread)
         free(pRecord->msgnotread);
      if (pRecord->totlmsgs)
         free(pRecord->totlmsgs);
   }
   SendMsg(hwndContainer, CM_REMOVERECORD, NULL,
                                MPFROM2SHORT(0, CMA_FREE));

   return;
}

/*--------------------------- ScrollToRecord  -------------------------------*/
/* Scrollt den Container zu einem Record                                     */
/* pRecord: Zeiger auf den Record                                            */
/* tocursored: Scrollt zum Record mit dem Cursored-Attribut, pRecord ign.    */
/*---------------------------------------------------------------------------*/

static BOOL ScrollToRecord(HWND hwndCnr, PRECORDCORE pRecord)
{
   QUERYRECORDRECT qrecord;
   RECTL rectl, rectl2;

   if (!pRecord)
   {
      pRecord=(PRECORDCORE)SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                              (MPARAM) CMA_FIRST, MPFROMSHORT(CRA_CURSORED));
   }

   if (pRecord)
   {
      WinQueryWindowRect(hwndCnr, &rectl2);
      qrecord.cb=sizeof(QUERYRECORDRECT);
      qrecord.pRecord=pRecord;
      qrecord.fRightSplitWindow=FALSE;
      qrecord.fsExtent=CMA_TEXT;
      if (SendMsg(hwndCnr, CM_QUERYRECORDRECT, &rectl, &qrecord))
      {
         if (rectl.yBottom < (rectl2.yTop-rectl2.yBottom)/2)
         {
            SendMsg(hwndCnr, CM_SCROLLWINDOW,
                       MPFROMSHORT(CMA_VERTICAL),
                       MPFROMLONG((rectl2.yTop-rectl2.yBottom)/2-rectl.yBottom));
         }
      }
      return TRUE;
   }
   else
      return FALSE;
}

/*------------------------------   SortFunc   -------------------------------*/
/* Sort-  Funktion fuer die Area-Liste                                       */
/*---------------------------------------------------------------------------*/

static SHORT _System SortOrig(PRECORDCORE p1, PRECORDCORE p2, PVOID AreaList)
{
   AREADEFLIST *zeiger;

   AreaList=AreaList;

   zeiger=arealiste.pFirstArea;

   while (zeiger)
   {
      if (zeiger == ((PAREARECORD)p1)->pAreaDef)
         return -1;

      if (zeiger == ((PAREARECORD)p2)->pAreaDef)
         return 1;

      zeiger=zeiger->next;
   }
   return 0;
}

static SHORT _System SortName(PRECORDCORE p1, PRECORDCORE p2, PVOID AreaList)
{
   AREADEFLIST *zeiger;
   PAREADEFLIST pArea1 = ((PAREARECORD)p1)->pAreaDef;
   PAREADEFLIST pArea2 = ((PAREARECORD)p2)->pAreaDef;

   AreaList=AreaList;

   /* Reihenfolge: Net - Echo/Lokal,
                   bei Typgleichheit: nach Desc oder Tag entscheiden */

   if (pArea1->areadata.areatype == AREATYPE_NET &&  /* Net vor anderen */
       pArea2->areadata.areatype != AREATYPE_NET)
      return -1;

   if (pArea1->areadata.areatype != AREATYPE_NET &&  /* Net vor anderen */
       pArea2->areadata.areatype == AREATYPE_NET)
      return 1;

   if (pArea1->areadata.areatype == pArea2->areadata.areatype &&
       pArea1->areadata.areatype == AREATYPE_NET)
   {
      /* nach Original-Reihenfolge */
      zeiger=arealiste.pFirstArea;

      while (zeiger)
      {
         if (zeiger == pArea1)
            return -1;

         if (zeiger == pArea2)
            return 1;

         zeiger=zeiger->next;
      }
      return 0;
   }
   else
      return stricmp(((PAREARECORD)p1)->areadesc, ((PAREARECORD)p2)->areadesc);
}

static SHORT _System SortNumber(PRECORDCORE p1, PRECORDCORE p2, PVOID AreaList)
{
   PAREADEFLIST pArea1 = ((PAREARECORD)p1)->pAreaDef;
   PAREADEFLIST pArea2 = ((PAREARECORD)p2)->pAreaDef;

   AreaList=AreaList;

   /* Net-Areas zuerst */
   if (pArea1->areadata.areatype == AREATYPE_NET &&  /* Net vor anderen */
       pArea2->areadata.areatype != AREATYPE_NET)
      return -1;

   if (pArea1->areadata.areatype != AREATYPE_NET &&  /* Net vor anderen */
       pArea2->areadata.areatype == AREATYPE_NET)
      return 1;


   if (pArea1->areadata.areatype == pArea2->areadata.areatype)
   {
      AREADEFLIST *zeiger;

      /* Beides sind Net-Areas, nach Original-Reihenfolge */
      if (pArea1->areadata.areatype == AREATYPE_NET)
      {
         zeiger=arealiste.pFirstArea;

         while (zeiger)
         {
            if (zeiger == ((PAREARECORD)p1)->pAreaDef)
               return -1;

            if (zeiger == ((PAREARECORD)p2)->pAreaDef)
               return 1;

            zeiger=zeiger->next;
         }
         return 0;
      }
   }

   /* Beides sind Echos, weiter mit Anzahl ungel. Msgs */

   /* gescannte zuerst */
   if (pArea1->scanned &&
       !pArea2->scanned)
      return -1;

   if (!pArea1->scanned &&
       pArea2->scanned)
      return 1;

   if (!pArea1->scanned &&
       !pArea2->scanned)
   {
      /* Beide ungescannt, nach Description */
      return stricmp(((PAREARECORD)p1)->areadesc, ((PAREARECORD)p2)->areadesc);
   }

   /* Beide gescanned, nach Anzahl */
   if ((pArea1->maxmessages -
        pArea1->currentmessage) <
       (pArea2->maxmessages -
        pArea2->currentmessage))
      return 1;

   if ((pArea1->maxmessages -
        pArea1->currentmessage) >
       (pArea2->maxmessages -
        pArea2->currentmessage))
      return -1;

   /* Beide gleiche Anzahl, nach Description */

   return stricmp(((PAREARECORD)p1)->areadesc, ((PAREARECORD)p2)->areadesc);
}

/*------------------------------ ResortAreaList -----------------------------*/
/* Sortiert die Arealiste neu                                                */
/*---------------------------------------------------------------------------*/

static void ResortAreaList(HWND hwndDlg, ULONG ulSortType)
{

   WinSendDlgItemMsg(hwndDlg, IDD_AREALIST+1, CM_SORTRECORD,
                     MPFROMP(SortFuncs[ulSortType]), NULL);

#if 0
   /* Workaround f. Fixpak 17 und Merlin */
   ScrollToRecord(WinWindowFromID(hwndDlg, IDD_AREALIST+1), NULL);
#endif

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MarkAllAreas                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert alle (nicht excludeten) Areas mit der angegebenen  */
/*               Flag-Kombination                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:    arealist:  Area-Liste                                       */
/*               ulFlags:   Flags, die gesetzt werden sollen                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Flags WORK_* sind in structs.h definiert                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void MarkAllAreas(PAREALIST arealist, LONG lFolderID, ULONG ulFlags)
{
   AREADEFLIST *zeiger= arealist->pFirstArea;

   while (zeiger)
   {
      if (lFolderID == FOLDERID_ALL)
      {
         PAREAFOLDER pFolder = FM_FindFolder(&FolderAnchor, zeiger->areadata.ulFolderID);
         if (pFolder->ulFlags & FOLDER_AUTOSCAN)
            zeiger->flWork |= ulFlags;
      }
      else
         if (zeiger->areadata.ulFolderID == lFolderID)
            zeiger->flWork |= ulFlags;
      zeiger= zeiger->next;
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MarkSelectedAreas                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert alle selektierten Areas mit der angegebenen        */
/*               Flag-Kombination                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:    arealist:  Area-Liste                                       */
/*               ulFlags:   Flags, die gesetzt werden sollen                 */
/*               selectRecord: ausgew„hlter Record                           */
/*               hwndCnr:   Window-Handle des Containers, der die Areas      */
/*                          enthaelt                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Flags WORK_* sind in structs.h definiert                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void MarkSelectedAreas(PAREALIST arealist, ULONG ulFlags,
                              PAREARECORD selectRecord, HWND hwndCnr)
{
   PAREARECORD pRecord=NULL;
   AREADEFLIST *zeiger=NULL;

   if (selectRecord->RecordCore.flRecordAttr & CRA_SELECTED)
   {
      /* Alle selektierten Areas includen */
      pRecord=SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                                       (MPARAM) CMA_FIRST,
                                       MPFROMSHORT(CRA_SELECTED));
      do
      {
         zeiger = AM_FindArea(arealist, pRecord->pAreaDef->areadata.areatag);
         if (zeiger)
            zeiger->flWork |= ulFlags;
      } while (pRecord=SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
                                                pRecord,
                                                MPFROMSHORT(CRA_SELECTED)));
   }
   else
   {
      /* nur gew„hlte Area includen */
      zeiger = AM_FindArea(arealist, selectRecord->pAreaDef->areadata.areatag);
      if (zeiger)
         zeiger->flWork |= ulFlags;
   }

   return;
}



/*--------------------------------- DrawItem --------------------------------*/
/* Behandelt die Ownerdraw-Message in der Arealiste                          */
/*---------------------------------------------------------------------------*/

static BOOL DrawItem(POWNERITEM Item)
{
   AREADEFLIST *zeiger;
   POINTL pointl;
   BOOL bRet=FALSE;
   LONG lForeColor;
   PAREARECORD pRecord;

   pRecord = (PAREARECORD)((PCNRDRAWITEMINFO)Item->hItem)->pRecord;

   if (!pRecord)
      return FALSE;

   if (!pRecord->pAreaDef)
      return FALSE;

   zeiger = pRecord->pAreaDef;

   if (Item->fsAttribute & CRA_SELECTED)
      return FALSE;

   if (!((PCNRDRAWITEMINFO)Item->hItem)->pFieldInfo)
      return FALSE;

   GpiCreateLogColorTable(Item->hps, 0, LCOLF_RGB, 0, 0, 0);

   if (((PCNRDRAWITEMINFO)Item->hItem)->pFieldInfo->offStruct == FIELDOFFSET(AREARECORD, areadesc))
   {
      Item->rclItem.xLeft += 5;

      switch(zeiger->areadata.areatype)
      {
         case AREATYPE_ECHO:
            lForeColor = arealistoptions.lEchoAreaColor;
            break;

         case AREATYPE_NET:
            lForeColor = arealistoptions.lNetAreaColor;
            break;

         case AREATYPE_LOCAL:
            lForeColor = arealistoptions.lLocalAreaColor;
            break;

         default:
            break;
      }

      WinDrawText(Item->hps, -1, pRecord->areadesc, &Item->rclItem,
                  lForeColor,
                  arealistoptions.lBackColor, DT_LEFT);
      bRet=TRUE;
   }
   else
      bRet=FALSE;

   if (zeiger->areadata.ulAreaOpt & AREAOPT_SEPARATOR)
   {
      LONG FrameClr;

      if (!WinQueryPresParam(Item->hwnd, PP_BORDERCOLOR, PP_BORDERCOLORINDEX,
                             NULL, sizeof(LONG), &FrameClr, QPF_ID2COLORINDEX))
         FrameClr = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWFRAME, 0);

      GpiSetColor(Item->hps, FrameClr);
      pointl.x=Item->rclItem.xLeft;
      pointl.y=Item->rclItem.yBottom;
      GpiMove(Item->hps, &pointl);
      pointl.x=Item->rclItem.xRight;
      pointl.y=Item->rclItem.yBottom;
      GpiLine(Item->hps, &pointl);
   }

   return bRet;
}

/*------------------------------ NewAEditProc  ------------------------------*/
/* Window-Procedure des MLE beim Direct Edit                                 */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewAEditProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
      case WM_CHAR:
         if ((SendMsg(parent, MLM_QUERYTEXTLENGTH, NULL, NULL)==
             SendMsg(parent, MLM_QUERYTEXTLIMIT, NULL, NULL)) &&
             !(SHORT1FROMMP(mp1) & KC_VIRTUALKEY))
         {
            if (!(SHORT1FROMMP(mp1) & KC_KEYUP))
               WinAlarm(HWND_DESKTOP, WA_NOTE);
            return (MRESULT) TRUE;
         }
         else
            break;

      default:
         break;
   }
   return OldAEditProc(parent, message, mp1, mp2);
}

/*------------------------------ UpdateAreaList -----------------------------*/
/* Arealiste neu fuellen, Exclude-Flag beachten                              */
/* Rueckgabewert: Zeiger auf momentane Area                                  */
/*---------------------------------------------------------------------------*/

static PAREARECORD UpdateAreaList(HWND hwndCnr, PAREALISTDATA pAreaListData, char *pchCurrArea)
{
   AREADEFLIST *zeiger=NULL;
   ULONG ulAreaCount=0;
   PAREARECORD pfirstRecord=NULL, selectRecord=NULL, pRecords=NULL;
   RECORDINSERT recordInsert;
   CNRINFO cnrinfo;

   CleanupAreaList(hwndCnr);

   /* Titel neu setzen */
   cnrinfo.cb = sizeof(cnrinfo);
   cnrinfo.pszCnrTitle = pAreaListData->pOpenFolder->pAreaFolder->pchName;
   WinSendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_CNRTITLE));

   /* Areas einfuegen */
   zeiger=arealiste.pFirstArea;
   while (zeiger)
   {
      if (((zeiger->areadata.areatype == AREATYPE_LOCAL && (pAreaListData->ulIncludeTypes & INCLUDE_LOCAL)) ||
           (zeiger->areadata.areatype == AREATYPE_ECHO  && (pAreaListData->ulIncludeTypes & INCLUDE_ECHO)) ||
           (zeiger->areadata.areatype == AREATYPE_NET   && (pAreaListData->ulIncludeTypes & INCLUDE_NET))) &&
          zeiger->areadata.ulFolderID == pAreaListData->pOpenFolder->pAreaFolder->FolderID)
         ulAreaCount++;
      zeiger=zeiger->next;
   }
   if (ulAreaCount==0)
      return NULL;

   pfirstRecord=SendMsg(hwndCnr, CM_ALLOCRECORD,
                           MPFROMLONG(sizeof(AREARECORD)-sizeof(RECORDCORE)),
                           MPFROMLONG(ulAreaCount));
   zeiger=arealiste.pFirstArea;
   pRecords=pfirstRecord;
   selectRecord=NULL;
   while (zeiger && pRecords)
   {
      if (((zeiger->areadata.areatype == AREATYPE_LOCAL && (pAreaListData->ulIncludeTypes & INCLUDE_LOCAL)) ||
           (zeiger->areadata.areatype == AREATYPE_ECHO  && (pAreaListData->ulIncludeTypes & INCLUDE_ECHO)) ||
           (zeiger->areadata.areatype == AREATYPE_NET   && (pAreaListData->ulIncludeTypes & INCLUDE_NET))) &&
          zeiger->areadata.ulFolderID == pAreaListData->pOpenFolder->pAreaFolder->FolderID)
      {
         InitAreaRecord(pRecords, zeiger, pAreaListData);

         if (pAreaListData->ulSort != FOLDER_SORT_UNREAD)
           if (AreaInAreaSet(pchCurrArea, zeiger->areadata.areatag))
           {
              pRecords->RecordCore.flRecordAttr |= CRA_SELECTED;
              selectRecord=pRecords;
           }

         pRecords=(PAREARECORD)(pRecords->RecordCore.preccNextRecord);
      }
      zeiger=zeiger->next;
   }
   recordInsert.cb = sizeof(RECORDINSERT);
   recordInsert.pRecordParent= NULL;
   recordInsert.pRecordOrder = (PRECORDCORE)CMA_FIRST;
   recordInsert.zOrder = CMA_TOP;
   recordInsert.cRecordsInsert = ulAreaCount;
   recordInsert.fInvalidateRecord = FALSE;

   SendMsg(hwndCnr, CM_INSERTRECORD,
              (PRECORDCORE)pfirstRecord, &recordInsert);

   SetFocus(hwndCnr);

   if (!selectRecord)
      selectRecord = pfirstRecord;

   if (pAreaListData->ulSort == FOLDER_SORT_UNREAD)
      return pfirstRecord;
   else
      return selectRecord;
}

static void InitAreaRecord(PAREARECORD pRecord, PAREADEFLIST pAreaDef, PAREALISTDATA pAreaListData)
{
   if (!pAreaListData->bChange || (arealistoptions.ulFlags & AREALISTFLAG_SHOWTAGS))
      pRecord->RecordCore.flRecordAttr=CRA_RECORDREADONLY;
   else
      pRecord->RecordCore.flRecordAttr=0;
   pRecord->RecordCore.pszIcon=NULL;
   pRecord->RecordCore.pszText=NULL;
   pRecord->RecordCore.pszName=NULL;
   pRecord->RecordCore.pszTree=NULL;
   pRecord->RecordCore.hptrIcon= NULLHANDLE;
   if (arealistoptions.ulFlags & AREALISTFLAG_SHOWTAGS)
      pRecord->areadesc=pAreaDef->areadata.areatag;
   else
      pRecord->areadesc=pAreaDef->areadata.areadesc;
   pRecord->pAreaDef=pAreaDef;
   UpdateAreaNums(pRecord);

   return;
}

static void UpdateAreaNums(PAREARECORD pRecord)
{
   if (!pRecord->msgnotread)
      pRecord->msgnotread=malloc(12);
   if (!pRecord->totlmsgs)
      pRecord->totlmsgs=malloc(12);

   if (pRecord->pAreaDef->scanned)
   {
      ULONG count;

      if ((count=(pRecord->pAreaDef->maxmessages)-(pRecord->pAreaDef->currentmessage))==0)
         strcpy(pRecord->msgnotread, "-");
      else
         _itoa(count, pRecord->msgnotread, 10);
      _itoa(pRecord->pAreaDef->maxmessages, pRecord->totlmsgs, 10);
   }
   else
   {
      strcpy(pRecord->msgnotread, "-");
      strcpy(pRecord->totlmsgs, "-");
   }

   return;
}

/*----------------------------- SquishParamsProc ----------------------------*/
/* Window-Prozedur f. Squish-Parameter                                       */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY SquishParamsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   static SQUISHPARAMSPAR *pParams;
   extern HWND hwndhelp;
   extern WINDOWPOSITIONS windowpositions;

   switch(message)
   {
      case WM_INITDLG:
         pParams=(SQUISHPARAMSPAR*) mp2;
         WinAssociateHelpInstance(hwndhelp, parent);
         /* Anzahl der Messages 0-20000 */
         WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+5, SPBM_SETLIMITS,
                           MPFROMLONG(20000), MPFROMLONG(0));
         /* Skipped Messages 0-20000 */
         WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+6, SPBM_SETLIMITS,
                           MPFROMLONG(20000), MPFROMLONG(0));
         /* Anzahl der Tage 0-100 */
         WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+7, SPBM_SETLIMITS,
                           MPFROMLONG(100), MPFROMLONG(0));

         /* Werte setzen */
         WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+5, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pParams->SquishParams.ulMaxMsgs), NULL);
         WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+6, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pParams->SquishParams.ulSkipMsgs), NULL);
         WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+7, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pParams->SquishParams.usDaysToKeep), NULL);
         RestoreWinPos(parent, &windowpositions.sqparamspos, FALSE, TRUE);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
            if (SHORT1FROMMP(mp1)==DID_OK)
            {
               LONG lBuf;

               WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+5, SPBM_QUERYVALUE,
                                 &lBuf, MPFROM2SHORT(0, SPBQ_UPDATEIFVALID));
               pParams->SquishParams.ulMaxMsgs=lBuf;
               WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+6, SPBM_QUERYVALUE,
                                 &lBuf, MPFROM2SHORT(0, SPBQ_UPDATEIFVALID));
               pParams->SquishParams.ulSkipMsgs=lBuf;
               WinSendDlgItemMsg(parent, IDD_SQUISHBASEPARAM+7, SPBM_QUERYVALUE,
                                 &lBuf, MPFROM2SHORT(0, SPBQ_UPDATEIFVALID));
               pParams->SquishParams.usDaysToKeep=(USHORT)lBuf;

#if 0
               /* Werte pruefen */
               if (pParams->SquishParams.ulSkipMsgs>0 &&
                   pParams->SquishParams.ulMaxMsgs==0)
               {
                  MessageBox(parent, IDST_MSG_SQPARAMVALUES, 0, IDD_SQPARAMVALUES,
                             MB_OK | MB_ERROR);
                  return (MRESULT) FALSE;
               }
#endif
            }
         break;

      case WM_CONTROL:
         if ((SHORT1FROMMP(mp1)==IDD_SQUISHBASEPARAM+5 ||
             SHORT1FROMMP(mp1)==IDD_SQUISHBASEPARAM+6 ||
             SHORT1FROMMP(mp1)==IDD_SQUISHBASEPARAM+6) &&
             SHORT2FROMMP(mp1)==SPBN_CHANGE)
            SendMsg((HWND) mp2, SPBM_QUERYVALUE, NULL,
                       MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
         break;

      case WM_DESTROY:
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         QueryWinPos(parent, &windowpositions.sqparamspos);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DragOverAreaList                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet ein Drag-Over der Area-Liste                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Window-Handle der Area-Liste                          */
/*            pCnrDrag: Drag-Info von Container                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: MRESULT wie fuer CN_DRAGOVER                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT DragOverAreaList(HWND hwndDlg, PCNRDRAGINFO pCnrDrag)
{
   USHORT usDrop = DOR_DROP;
   USHORT usDefaultOp = DO_UNKNOWN;

   hwndDlg = hwndDlg;

   if (pCnrDrag->pRecord)
   {
      DRAGITEM dItem;

      DrgAccessDraginfo(pCnrDrag->pDragInfo);
      DrgQueryDragitem(pCnrDrag->pDragInfo, sizeof(dItem), &dItem, 0);

      if (WinQueryAnchorBlock(dItem.hwndItem) != anchor)
         usDrop = DOR_NEVERDROP;

      if (!DrgVerifyType(&dItem, "FleetStreet Template"))
         usDrop = DOR_NEVERDROP;

      DrgFreeDraginfo(pCnrDrag->pDragInfo);
   }
   else
      usDrop = DOR_NODROP;


   return MRFROM2SHORT(usDrop, usDefaultOp);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DropAreaList                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet ein Drop der Area-Liste                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Window-Handle der Area-Liste                          */
/*            pCnrDrag: Drag-Info von Container                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DropAreaList(HWND hwndDlg, PCNRDRAGINFO pCnrDrag)
{
   hwndDlg = hwndDlg;

   DrgAccessDraginfo(pCnrDrag->pDragInfo);

   if (pCnrDrag->pRecord)
   {
      DRAGITEM dItem;

      DrgQueryDragitem(pCnrDrag->pDragInfo, sizeof(dItem), &dItem, 0);

      if (WinQueryAnchorBlock(dItem.hwndItem) == anchor)
      {
         if (DrgVerifyType(&dItem, "FleetStreet Template"))
         {
            ((PAREARECORD)pCnrDrag->pRecord)->pAreaDef->areadata.ulTemplateID = dItem.ulItemID;
            ((PAREARECORD)pCnrDrag->pRecord)->pAreaDef->dirty = TRUE;
            arealiste.bDirty = TRUE;
         }
      }
   }
   DrgDeleteDraginfoStrHandles(pCnrDrag->pDragInfo);
   DrgFreeDraginfo(pCnrDrag->pDragInfo);

   return;
}

static void InitFolderContainer(HWND hwndDlg, PAREALISTDATA pAreaListData)
{
   CNRINFO cnrinfo;
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+4);
   PFOLDERRECORD pRecord;
   PAREAFOLDER pFolder;
   PAREADEFLIST pCurrentArea;
   LONG FolderToOpen = FOLDERID_ALL;

   /* Container-Info */
   SetFolderIcons(hwndDlg, pAreaListData);

   cnrinfo.cb = sizeof(cnrinfo);
   cnrinfo.pSortRecord = (PVOID) SortFolders;
   cnrinfo.cxTreeIndent=14;

   SendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo,
           MPFROMLONG(CMA_PSORTRECORD | CMA_CXTREEINDENT));

   if (pAreaListData->pchParamString && *pAreaListData->pchParamString)
   {
      pCurrentArea = AM_FindArea(&arealiste, *pAreaListData->pchParamString);
      if (pCurrentArea)
         FolderToOpen = pCurrentArea->areadata.ulFolderID;
   }

   /* Inhalt */
   pFolder = FM_FindFolder(&FolderAnchor, 0);
   if (pFolder)
   {
      pRecord=InsertFolderRecord(hwndCnr, pFolder, NULL, pAreaListData, FolderToOpen);
      if (pRecord)
         InsertChildFolders(hwndCnr, pRecord, pAreaListData, FolderToOpen);
      SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
   }

   if (pAreaListData->pOpenFolder)
   {
      pAreaListData->pOpenFolder->RecordCore.hptrIcon = pAreaListData->icnFolderOpen;
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pAreaListData->pOpenFolder,
              MPFROM2SHORT(TRUE, CRA_INUSE));
   }

   return;
}

static SHORT EXPENTRY SortFolders(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage)
{
   pStorage = pStorage;

   if (p1 && p2 && p1->pszIcon && p2->pszIcon)
      return stricmp(p1->pszIcon, p2->pszIcon);
   else
      return 0;
}

static void InsertChildFolders(HWND hwndCnr, PFOLDERRECORD pParentRecord, PAREALISTDATA pAreaListData, LONG FolderToOpen)
{
   PAREAFOLDER pChild=NULL;
   PFOLDERRECORD pRecord;

   while (pChild = FM_FindFolderWithParent(&FolderAnchor, pChild, pParentRecord->pAreaFolder->FolderID))
   {
      if (pChild->FolderID != pParentRecord->pAreaFolder->FolderID) /* keine Schleifen */
      {
         pRecord = InsertFolderRecord(hwndCnr, pChild, pParentRecord, pAreaListData, FolderToOpen);

         /* Kinder einfgen (Rekursion!) */
         if (pRecord)
            InsertChildFolders(hwndCnr, pRecord, pAreaListData, FolderToOpen);
      }
   }
}

static PFOLDERRECORD InsertFolderRecord(HWND hwndCnr, PAREAFOLDER pFolder, PFOLDERRECORD pParent, PAREALISTDATA pAreaListData, LONG FolderToOpen)
{
   RECORDINSERT RecordInsert;
   PFOLDERRECORD pRecord;

   pRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                          MPFROMLONG(sizeof(FOLDERRECORD)-sizeof(MINIRECORDCORE)),
                          MPFROMLONG(1));

   if (pRecord)
   {
      pRecord->pAreaFolder = pFolder;
      pRecord->RecordCore.pszIcon = pFolder->pchName;
      if (FolderHasUnreadAreas(pFolder))
         pRecord->RecordCore.hptrIcon = pAreaListData->icnFolderUnread;
      else
         pRecord->RecordCore.hptrIcon = pAreaListData->icnFolder;
      if (pFolder->ulFlags & FOLDER_EXPANDED)
         pRecord->RecordCore.flRecordAttr = CRA_EXPANDED;
      else
         pRecord->RecordCore.flRecordAttr = CRA_COLLAPSED;

      RecordInsert.cb = sizeof(RecordInsert);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent = (PRECORDCORE) pParent;
      RecordInsert.fInvalidateRecord = FALSE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = 1;

      SendMsg(hwndCnr, CM_INSERTRECORD, pRecord, &RecordInsert);

      if ((FolderToOpen != FOLDERID_ALL && pFolder->FolderID == FolderToOpen) ||
          (pFolder->FolderID == FolderAnchor.LastFolder))
      {
         pAreaListData->pOpenFolder = pRecord;
         pAreaListData->ulSort = pFolder->ulFlags & FOLDER_SORT_MASK;
      }
   }

   return pRecord;
}

static void OpenFolderContext(HWND hwndDlg, PAREALISTDATA pAreaListData, PFOLDERRECORD pRecord)
{
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+4);
   RECTL rectl;

   pAreaListData->selectFolder = pRecord;

   if (pRecord)
   {
      QUERYRECORDRECT qrecordc;

      qrecordc.cb=sizeof(QUERYRECORDRECT);
      qrecordc.pRecord=(PRECORDCORE) pRecord;
      qrecordc.fRightSplitWindow=FALSE;
      if ((FolderAnchor.ulFlags & AREAFOLDERS_ICONMASK) == AREAFOLDERS_NOICONS)
         qrecordc.fsExtent=CMA_TEXT;
      else
         qrecordc.fsExtent=CMA_ICON;
      SendMsg(hwndCnr, CM_QUERYRECORDRECT, &rectl, &qrecordc);

      ApplySourceEmphasis(hwndCnr, (PRECORDCORE) pRecord);
      WinMapWindowPoints(hwndCnr, HWND_DESKTOP, (PPOINTL) &rectl, 2);

      WinEnableMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_DELETE, pRecord->pAreaFolder->FolderID);
      WinCheckMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_SCAN, pRecord->pAreaFolder->ulFlags & FOLDER_AUTOSCAN);

      WinCheckMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_SORT_NONE, FALSE);
      WinCheckMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_SORT_NAME, FALSE);
      WinCheckMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_SORT_UNR, FALSE);
      switch(pRecord->pAreaFolder->ulFlags & FOLDER_SORT_MASK)
      {
         case FOLDER_SORT_UNSORTED:
            WinCheckMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_SORT_NONE, TRUE);
            break;

         case FOLDER_SORT_NAME:
            WinCheckMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_SORT_NAME, TRUE);
            break;

         case FOLDER_SORT_UNREAD:
            WinCheckMenuItem(pAreaListData->hwndFolderPopup, IDM_FP_SORT_UNR, TRUE);
            break;
      }

      WinPopupMenu(HWND_DESKTOP, hwndDlg, pAreaListData->hwndFolderPopup,
                   rectl.xRight, rectl.yBottom,
                   0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }
   else
   {
      if (!pAreaListData->bKeyboard)
      {
         POINTL pointl;

         WinQueryPointerPos(HWND_DESKTOP, &pointl);
         rectl.xLeft = pointl.x;
         rectl.yBottom = pointl.y;
      }
      else
      {
         WinQueryWindowRect(hwndCnr, &rectl);
         WinMapWindowPoints(hwndCnr, HWND_DESKTOP, (PPOINTL) &rectl, 2);
         rectl.xLeft = (rectl.xLeft + rectl.xRight)/2;
         rectl.yBottom = (rectl.yBottom + rectl.yTop)/2;
      }

      WinPopupMenu(HWND_DESKTOP, hwndDlg, pAreaListData->hwndSmallFolderPopup,
                   rectl.xLeft, rectl.yBottom,
                   0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, NULL, MPFROM2SHORT(TRUE, CRA_SOURCE));

   }
}

static PFOLDERRECORD CreateNewFolder(HWND hwndDlg, PAREALISTDATA pAreaListData)
{
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+4);
   PAREAFOLDER pNewFolder;
   AREAFOLDER AreaFolder;
   PFOLDERRECORD pRecord=NULL;

   memset(&AreaFolder, 0, sizeof(AreaFolder));
   AreaFolder.pchName = calloc(1, 50);
   LoadString(IDST_AL_NEWFOLDER, 50, AreaFolder.pchName);

   if (!pAreaListData->selectFolder)
   {
      /* Root-Folder nehmen */
      pAreaListData->selectFolder = SendMsg(hwndCnr, CM_QUERYRECORD, NULL, MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
   }

   if (pAreaListData->selectFolder)
      AreaFolder.ParentFolder = pAreaListData->selectFolder->pAreaFolder->FolderID;

   pNewFolder = FM_AddFolder(&FolderAnchor, &AreaFolder, ADDFOLDER_TAIL | ADDFOLDER_NEWID | ADDFOLDER_MARKDIRTY);

   if (pNewFolder)
   {
      pRecord = InsertFolderRecord(hwndCnr, pNewFolder, pAreaListData->selectFolder, pAreaListData, FOLDERID_ALL);
      SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
   }

   return pRecord;
}

static void OpenFolder(HWND hwndDlg, PAREALISTDATA pAreaListData, PFOLDERRECORD pFolder)
{
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+4);

   if (pAreaListData->pOpenFolder != pFolder)
   {
      if (pAreaListData->pOpenFolder)
      {
         if (FolderHasUnreadAreas(pAreaListData->pOpenFolder->pAreaFolder))
            pAreaListData->pOpenFolder->RecordCore.hptrIcon = pAreaListData->icnFolderUnread;
         else
            pAreaListData->pOpenFolder->RecordCore.hptrIcon = pAreaListData->icnFolder;
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pAreaListData->pOpenFolder,
                 MPFROM2SHORT(FALSE, CRA_INUSE));
         pAreaListData->pOpenFolder = NULL;
      }
      pAreaListData->pOpenFolder = pFolder;
      pAreaListData->ulSort = pFolder->pAreaFolder->ulFlags & FOLDER_SORT_MASK;
      pFolder->RecordCore.hptrIcon = pAreaListData->icnFolderOpen;
      SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pFolder,
              MPFROM2SHORT(TRUE, CRA_INUSE));

      UpdateAreaList(WinWindowFromID(hwndDlg, IDD_AREALIST+1), pAreaListData, NULL);
      ResortAreaList(hwndDlg, pAreaListData->ulSort);
      WinSendDlgItemMsg(hwndDlg, IDD_AREALIST+1, CM_INVALIDATERECORD, NULL, NULL);
   }
   else
      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDlg, IDD_AREALIST+1));

   return;
}

static void SetFolderIcons(HWND hwndDlg, PAREALISTDATA pAreaListData)
{
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+4);
   CNRINFO cnrinfo;
   ULONG ulStyle;

   cnrinfo.cb = sizeof(cnrinfo);
   cnrinfo.slTreeBitmapOrIcon.cx=16;
   cnrinfo.slTreeBitmapOrIcon.cy=16;
   cnrinfo.hptrExpanded=pAreaListData->hptrMinus;
   cnrinfo.hptrCollapsed=pAreaListData->hptrPlus;

   WinCheckMenuItem(pAreaListData->hwndSmallFolderPopup, IDM_SFP_ICONS_LARGE, FALSE);
   WinCheckMenuItem(pAreaListData->hwndSmallFolderPopup, IDM_SFP_ICONS_SMALL, FALSE);
   WinCheckMenuItem(pAreaListData->hwndSmallFolderPopup, IDM_SFP_ICONS_NONE, FALSE);

   ulStyle = WinQueryWindowULong(hwndCnr, QWL_STYLE) | CCS_MINIICONS;
   WinSetWindowULong(hwndCnr, QWL_STYLE, ulStyle);

   switch(FolderAnchor.ulFlags & AREAFOLDERS_ICONMASK)
   {
      case AREAFOLDERS_LARGEICONS:
         cnrinfo.flWindowAttr = CV_TREE | CA_TREELINE;
         WinCheckMenuItem(pAreaListData->hwndSmallFolderPopup, IDM_SFP_ICONS_LARGE, TRUE);
         break;

      case AREAFOLDERS_SMALLICONS:
         cnrinfo.hptrExpanded=NULLHANDLE;
         cnrinfo.hptrCollapsed=NULLHANDLE;
         cnrinfo.flWindowAttr = CV_TREE | CA_TREELINE | CV_MINI;
         WinCheckMenuItem(pAreaListData->hwndSmallFolderPopup, IDM_SFP_ICONS_SMALL, TRUE);
         break;

      case AREAFOLDERS_NOICONS:
         cnrinfo.flWindowAttr = CV_TREE | CV_TEXT | CA_TREELINE;
         WinCheckMenuItem(pAreaListData->hwndSmallFolderPopup, IDM_SFP_ICONS_NONE, TRUE);
         break;

      default:
         return;
   }

   SendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_TREEICON | CMA_FLWINDOWATTR));

   return;
}

static void InitAreaDrag(HWND hwndDlg, PAREALISTDATA pAreaListData, PCNRDRAGINIT pInit)
{
   PDRAGINFO pDraginfo;
   DRAGITEM dItem;
   DRAGIMAGE dImage[3];
   ULONG ulNum=0;
   PAREARECORD pRecord;
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+1);
   int i=0;
   HWND hwndDrop;

   if (pInit->pRecord)
   {
      if (pInit->pRecord->flRecordAttr & CRA_SELECTED)
      {
         /* alle selektierten */
         pRecord = NULL;
         while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                     MPFROMLONG(CRA_SELECTED)))
            ulNum++;
      }
      else
         ulNum = 1;

      pDraginfo = DrgAllocDraginfo(ulNum);
      pDraginfo->usOperation=DO_DEFAULT;
      pDraginfo->hwndSource=hwndDlg;

      if (ulNum > 1)
      {
         pRecord = NULL;
         while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                     MPFROMLONG(CRA_SELECTED)))
         {
            /* Source emphasis */
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

            FillDragItem(hwndDlg, pRecord, &dItem);
            DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), i);
            i++;
         }
      }
      else
      {
         /* nur einer */
         pRecord = (PAREARECORD) pInit->pRecord;

         /* Source emphasis */
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

         FillDragItem(hwndDlg, pRecord, &dItem);
         DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);
      }

      for (i=0; i<ulNum && i<3; i++)
      {
         /* Drag-Image vorbereiten */
         dImage[i].cb=sizeof(DRAGIMAGE);
         dImage[i].hImage=pAreaListData->icon;
         dImage[i].fl=DRG_ICON;
         dImage[i].cxOffset=i*10;
         dImage[i].cyOffset=i*10;
      }

      hwndDrop = DrgDrag(hwndDlg, pDraginfo, dImage, (ulNum<3)?ulNum:3, VK_ENDDRAG, NULL);
      DrgFreeDraginfo(pDraginfo);

      if (hwndDrop)
      {
         UpdateAreaList(hwndCnr, pAreaListData, NULL);
         ResortAreaList(hwndDlg, pAreaListData->ulSort);
         SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
      }
      else
         /* Source emphasis wegnehmen */
         if (ulNum > 1)
            RemoveSourceEmphasis(hwndCnr);
         else
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));
   }
}

static void FillDragItem(HWND hwndDlg, PAREARECORD pRecord, PDRAGITEM dItem)
{
   /* Drag-Item vorbereiten*/
   dItem->hwndItem=hwndDlg;
   dItem->ulItemID= (ULONG)pRecord;
   dItem->hstrType=DrgAddStrHandle(AREADRAGTYPE);
   dItem->hstrSourceName=DrgAddStrHandle(pRecord->pAreaDef->areadata.areatag);
   dItem->hstrTargetName=DrgAddStrHandle(pRecord->pAreaDef->areadata.areatag);
   dItem->hstrContainerName = NULLHANDLE;

   if (pRecord->pAreaDef->areadata.ulAreaOpt & AREAOPT_FROMCFG)
      dItem->hstrRMF=DrgAddStrHandle(AREARMF);
   else
      dItem->hstrRMF=DrgAddStrHandle(AREARMFDEL);
   dItem->fsControl= DC_CONTAINER;
   dItem->fsSupportedOps=DO_MOVEABLE;

   return;
}

static BOOL IsBetweenContainers(HWND hwndDlg, SHORT x, SHORT y)
{
   SWP swpFolders, swpList;

   WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_AREALIST+4), &swpFolders);
   WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_AREALIST+1), &swpList);

   if (y > swpFolders.y &&
       y < swpFolders.y+swpFolders.cy &&
       x >= swpFolders.x+swpFolders.cx &&
       x < swpList.x)
      return TRUE;
   else
      return FALSE;
}

static void SetPointer(void)
{
   HPOINTER hptr;

   hptr = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE);
   WinSetPointer(HWND_DESKTOP, hptr);

   return;
}

static void TrackSeparator(HWND hwndDlg)
{
   TRACKINFO TrackInfo;
   SWP swpFolders;

   /* Daten vorbelegen */
   TrackInfo.cxBorder = 2;
   TrackInfo.cyBorder = 1;
   TrackInfo.cxGrid = 1;
   TrackInfo.cyGrid = 1;
   TrackInfo.cxKeyboard = 1;
   TrackInfo.cyKeyboard = 1;

   WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_AREALIST+4), &swpFolders);
   WinQueryWindowRect(hwndDlg, &TrackInfo.rclBoundary);
   CalcClientRect(anchor, hwndDlg, &TrackInfo.rclBoundary);
   TrackInfo.rclBoundary.yBottom = swpFolders.y;
   TrackInfo.rclBoundary.yTop = swpFolders.y+swpFolders.cy;

   WinInflateRect(anchor, &TrackInfo.rclBoundary, -WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL), 0);

   TrackInfo.rclTrack.xLeft = swpFolders.x+swpFolders.cx;
   TrackInfo.rclTrack.xRight = swpFolders.x+swpFolders.cx+2;
   TrackInfo.rclTrack.yBottom = swpFolders.y;
   TrackInfo.rclTrack.yTop = swpFolders.y+swpFolders.cy;

   TrackInfo.ptlMinTrackSize.x = 1;
   TrackInfo.ptlMinTrackSize.y = 1;
   TrackInfo.ptlMaxTrackSize.x = 2;
   TrackInfo.ptlMaxTrackSize.y = swpFolders.cy;

   TrackInfo.fs = TF_ALLINBOUNDARY | TF_MOVE;

   /* Track */
   if (WinTrackRect(hwndDlg, NULLHANDLE, &TrackInfo))
   {
      FolderAnchor.lSplit = TrackInfo.rclTrack.xLeft - swpFolders.x;
      FolderAnchor.bDirty = TRUE;

      RepositionContainers(hwndDlg);
   }
   return;
}

static void RepositionContainers(HWND hwndDlg)
{
   RECTL rectl;
   SWP swp;

   WinQueryWindowRect(hwndDlg, &rectl);
   CalcClientRect(anchor, hwndDlg, &rectl);
   WinQueryWindowPos(WinWindowFromID(hwndDlg, DID_OK), &swp);
   rectl.yBottom += swp.y + swp.cy;
   WinSetWindowPos(WinWindowFromID(hwndDlg, IDD_AREALIST+4),
                   NULLHANDLE,
                   rectl.xLeft, rectl.yBottom,
                   FolderAnchor.lSplit, rectl.yTop-rectl.yBottom,
                   SWP_MOVE | SWP_SIZE);
   WinSetWindowPos(WinWindowFromID(hwndDlg, IDD_AREALIST+1),
                   NULLHANDLE,
                   rectl.xLeft+FolderAnchor.lSplit+CONTAINERSPACE, rectl.yBottom,
                   rectl.xRight-rectl.xLeft-FolderAnchor.lSplit-CONTAINERSPACE, rectl.yTop-rectl.yBottom,
                   SWP_MOVE | SWP_SIZE);

   return;
}

static MRESULT DragOverFolder(HWND hwndDlg, PCNRDRAGINFO pInfo)
{
   USHORT usDrop = DOR_NEVERDROP;
   USHORT usDefaultOp = DO_MOVE;

   if (pInfo->pRecord)
   {
      DrgAccessDraginfo(pInfo->pDragInfo);

      if (pInfo->pDragInfo->hwndSource == hwndDlg) /* nur innerhalb der Liste */
      {
         if (pInfo->pDragInfo->usOperation == DO_DEFAULT ||
             pInfo->pDragInfo->usOperation == DO_MOVE)
         {
            PDRAGITEM pItem;
            int i;

            for (i=0; i<pInfo->pDragInfo->cditem; i++)
            {
               pItem = DrgQueryDragitemPtr(pInfo->pDragInfo, i);
               if (pItem)
               {
                  if (DrgVerifyTrueType(pItem, AREADRAGTYPE))
                     continue;

                  if (DrgVerifyTrueType(pItem, FOLDERDRAGTYPE))
                  {
                     if ((PRECORDCORE)pItem->ulItemID == pInfo->pRecord)
                     {
                        usDrop = DOR_NODROP;
                        break;
                     }
                  }
                  else
                     break;
               }
            }
            if (i >= pInfo->pDragInfo->cditem)
               usDrop = DOR_DROP;
         }
         else
            usDrop = DOR_NODROPOP;
      }

      DrgFreeDraginfo(pInfo->pDragInfo);
   }
   else
      usDrop = DOR_NODROP;

   return MRFROM2SHORT(usDrop, usDefaultOp);
}

static void DropOnFolder(PCNRDRAGINFO pInfo)
{
   int i;
   PDRAGITEM pItem;

   DrgAccessDraginfo(pInfo->pDragInfo);

   for (i=0; i<pInfo->pDragInfo->cditem; i++)
   {
      pItem = DrgQueryDragitemPtr(pInfo->pDragInfo, i);
      if (pItem)
      {
         if (DrgVerifyTrueType(pItem, AREADRAGTYPE))
         {
            PAREARECORD pRecord = (PAREARECORD)pItem->ulItemID;

            /* Area verschieben */
            if (pRecord->pAreaDef->areadata.ulFolderID !=
                ((PFOLDERRECORD)pInfo->pRecord)->pAreaFolder->FolderID)
            {
               pRecord->pAreaDef->areadata.ulFolderID =
                ((PFOLDERRECORD)pInfo->pRecord)->pAreaFolder->FolderID;

               pRecord->pAreaDef->dirty = TRUE;
               arealiste.bDirty = TRUE;
            }
         }
         else
            if (DrgVerifyTrueType(pItem, FOLDERDRAGTYPE))
            {
               PFOLDERRECORD pRecord = (PFOLDERRECORD)pItem->ulItemID;

               if (pRecord->pAreaFolder->ParentFolder !=
                   ((PFOLDERRECORD)pInfo->pRecord)->pAreaFolder->FolderID)
               {
                  pRecord->pAreaFolder->ParentFolder = ((PFOLDERRECORD)pInfo->pRecord)->pAreaFolder->FolderID;
                  pRecord->pAreaFolder->bDirty = TRUE;
                  FolderAnchor.bDirty = TRUE;
               }
            }
      }
   }

   DrgDeleteDraginfoStrHandles(pInfo->pDragInfo);

   DrgFreeDraginfo(pInfo->pDragInfo);
}

static void DeleteFolder(HWND hwndDlg, PAREALISTDATA pAreaListData, PFOLDERRECORD pFolder)
{
   PAREADEFLIST pArea= arealiste.pFirstArea;
   PAREAFOLDER pAFolder = FolderAnchor.pList;

   /* Areas im Folder nach oben setzen */
   while (pArea)
   {
      if (pArea->areadata.ulFolderID == pFolder->pAreaFolder->FolderID)
      {
         pArea->areadata.ulFolderID = pFolder->pAreaFolder->ParentFolder;
         pArea->dirty = TRUE;
         arealiste.bDirty = TRUE;
      }
      pArea = pArea->next;
   }

   /* Folder im Folder nach oben setzen */
   while (pAFolder)
   {
      if (pAFolder->ParentFolder == pFolder->pAreaFolder->FolderID)
      {
         pAFolder->ParentFolder = pFolder->pAreaFolder->ParentFolder;
         pAFolder->bDirty = TRUE;
         FolderAnchor.bDirty = TRUE;
      }
      pAFolder = pAFolder->next;
   }

   if (pAreaListData->pOpenFolder == pFolder)  /* aktuellen loeschen */
   {
      pAreaListData->pOpenFolder = NULL;
      FolderAnchor.LastFolder = pFolder->pAreaFolder->ParentFolder;
   }

   FM_DeleteFolderDirect(&FolderAnchor, pFolder->pAreaFolder);

   /* neu anzeigen */
   WinSendDlgItemMsg(hwndDlg, IDD_AREALIST+4, CM_REMOVERECORD, NULL,
                     MPFROM2SHORT(0, CMA_FREE));
   InitFolderContainer(hwndDlg, pAreaListData);
   UpdateAreaList(WinWindowFromID(hwndDlg, IDD_AREALIST+1), pAreaListData, NULL);
   ResortAreaList(hwndDlg, pAreaListData->ulSort);
   WinSendDlgItemMsg(hwndDlg, IDD_AREALIST+1, CM_INVALIDATERECORD, NULL, NULL);

   return;
}

static void InitFolderDrag(HWND hwndDlg, PAREALISTDATA pAreaListData, PCNRDRAGINIT pInit)
{
   PDRAGINFO pDraginfo;
   DRAGITEM dItem;
   DRAGIMAGE dImage;
   PFOLDERRECORD pRecord;
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+4);
   HWND hwndDrop;

   if (pInit->pRecord)
   {
      pRecord = (PFOLDERRECORD) pInit->pRecord;

      if (pRecord->pAreaFolder->FolderID) /* nicht root */
      {
         pDraginfo = DrgAllocDraginfo(1);
         pDraginfo->usOperation=DO_DEFAULT;
         pDraginfo->hwndSource=hwndDlg;

         /* Source emphasis */
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

         /* Drag-Item vorbereiten*/
         dItem.hwndItem=hwndDlg;
         dItem.ulItemID= (ULONG)pRecord;
         dItem.hstrType=DrgAddStrHandle(FOLDERDRAGTYPE);
         dItem.hstrSourceName=DrgAddStrHandle(pRecord->pAreaFolder->pchName);
         dItem.hstrTargetName=DrgAddStrHandle(pRecord->pAreaFolder->pchName);
         dItem.hstrContainerName = NULLHANDLE;

         if (pRecord->pAreaFolder->FolderID)
            dItem.hstrRMF=DrgAddStrHandle(FOLDERRMF);
         else
            dItem.hstrRMF=DrgAddStrHandle(FOLDERRMFDEL);

         dItem.fsControl= DC_CONTAINER;
         dItem.fsSupportedOps=DO_MOVEABLE;
         DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);

         /* Drag-Image vorbereiten */
         dImage.cb=sizeof(DRAGIMAGE);
         dImage.hImage=pAreaListData->icnFolder;
         dImage.fl=DRG_ICON;
         dImage.cxOffset=0;
         dImage.cyOffset=0;

         hwndDrop = DrgDrag(hwndDlg, pDraginfo, &dImage, 1, VK_ENDDRAG, NULL);
         DrgFreeDraginfo(pDraginfo);

         if (hwndDrop)
         {
            if (pAreaListData->pOpenFolder)
            {
               FolderAnchor.LastFolder = pAreaListData->pOpenFolder->pAreaFolder->FolderID;
               pAreaListData->pOpenFolder = NULL;
            }

            /* neu anzeigen */
            SendMsg(hwndCnr, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));
            InitFolderContainer(hwndDlg, pAreaListData);
         }
         else
            /* Source emphasis wegnehmen */
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));
      }
   }
}

static void UpdateScannedFolders(HWND hwndDlg, PAREALISTDATA pAreaListData)
{
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_AREALIST+4);

   if (pAreaListData->pOpenFolder)
   {
      FolderAnchor.LastFolder = pAreaListData->pOpenFolder->pAreaFolder->FolderID;
      pAreaListData->pOpenFolder = NULL;
   }

   SendMsg(hwndCnr, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));
   InitFolderContainer(hwndDlg, pAreaListData);

   return;
}

static BOOL FolderHasUnreadAreas(PAREAFOLDER pFolder)
{
   PAREADEFLIST pArea = arealiste.pFirstArea;

   while (pArea)
   {
      if (pArea->areadata.ulFolderID == pFolder->FolderID &&
          pArea->scanned &&
          pArea->currentmessage < pArea->maxmessages)
         return TRUE;

      pArea = pArea->next;
   }

   return FALSE;
}
/*--------------------------------- Modulende -------------------------------*/


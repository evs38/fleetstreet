/*---------------------------------------------------------------------------+
 | Titel: ECHOMANAGER.C                                                      |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 16.04.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Echo-Manager von FleetStreet                                          |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#define INCL_WIN
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "utility.h"
#include "dialogids.h"
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "areadlg.h"
#include "setupdlg.h"
#include "savemsg.h"
#include "devkit\echoman.h"
#include "util\fltutil.h"
#include "util\addrcnv.h"
#include "dump\expt.h"

#include "echomanager.h"


/*--------------------------------- Defines ---------------------------------*/

#define ECHOMGR_STRINGLEN  50

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L
#endif

#define TAB_FONT    "8.Helv"
#define RGB_GREY    0x00cccccc

#define TEXTBLOCKSIZE   4096

#define CMD_LINK       "+%s\n"
#define CMD_LNKRESC    "+%s\n%%RESCAN %s\n"
#define CMD_UNLINK     "-%s\n"
#define CMD_RESCAN     "%%RESCAN %s\n"
#define CMD_REFRESH    "%LIST\n"
#define CMD_PAUSE      "%PAUSE\n"
#define CMD_RESUME     "%RESUME\n"

#define WM_SAVEDONE     WM_USER
#define WM_CHANGEDONE   (WM_USER+1)
#define WM_CURRENTDEL   (WM_USER+2)

/*---------------------------------- Typen ----------------------------------*/

typedef struct
{
   MINIRECORDCORE RecordCore;
   PCHAR pchEchoName;
   PCHAR pchStatus;
   PCHAR pchAction;
   ULONG ulAction;
   ULONG ulStatus;
} ECHOMGRRECORD, *PECHOMGRRECORD;

#define ACTION_NONE    0UL
#define ACTION_LINK    1UL
#define ACTION_UNLINK  2UL
#define ACTION_RESCAN  3UL
#define ACTION_PAUSE   4UL
#define ACTION_RESUME  5UL
#define ACTION_REFRESH 6UL
#define ACTION_LNKRESC 7UL

#define STATUS_UNLINKED 0UL
#define STATUS_LINKED   1UL

typedef struct
{
   HWND hwndDlg;
   FTNMESSAGE Message;
   MSGHEADER Header;
} SAVEPARAM, *PSAVEPARAM;

typedef struct
{
   USHORT  usID;    /* Menue-ID */
   PUPLINK pUplink; /* zugehoeriger Uplink */
} MENUTABLE, *PMENUTABLE;

typedef struct
{
   HWND hwndPopup;
   HWND hwndFolderPopup;
   char pchTitleEcho[ECHOMGR_STRINGLEN];
   char pchTitleStatus[ECHOMGR_STRINGLEN];
   char pchTitleAction[ECHOMGR_STRINGLEN];
   char pchStatusLinked[ECHOMGR_STRINGLEN];
   char pchStatusUnlinked[ECHOMGR_STRINGLEN];
   char pchActionLink[ECHOMGR_STRINGLEN];
   char pchActionUnlink[ECHOMGR_STRINGLEN];
   char pchActionRescan[ECHOMGR_STRINGLEN];
   char pchActionRefresh[ECHOMGR_STRINGLEN];
   char pchActionPause[ECHOMGR_STRINGLEN];
   char pchActionResume[ECHOMGR_STRINGLEN];
   ULONG ulAction;
   BOOL bNotify;
   BOOL bKeyboard;
   PECHOMGRRECORD pPopupRecord;
   SAVEPARAM SaveParam;
   PUPLINK pUplink;
   PMENUTABLE pMenuTable;
   ULONG ulCountUplinks;
} ECHOMGRDATA, *PECHOMGRDATA;

typedef struct
{
   USHORT cb;
   PUPLINK pUplink;
   HWND hwndSettingsDlg;
   PECHOMGRDATA pEchoMgrData;
} EMANSETTINGSDATA, *PEMANSETTINGSDATA;

typedef struct stringlist
{
   struct stringlist *next;
   char *pchString;
} STRINGLIST, *PSTRINGLIST;

typedef struct actionlist
{
   struct actionlist *next;
   ULONG ulAction;
   char pchAreaTag[LEN_AREATAG+1];
} ACTIONLIST, *PACTIONLIST;

typedef struct
{
   HWND hwndDlg;
   PACTIONLIST pActionList;
   PUPLINK pUplink;
} CHANGEPARAM, *PCHANGEPARAM;

typedef struct
{
  USHORT cb;
  PUPLINK pUplink;
} UPLINKPARAM, *PUPLINKPARAM;

typedef struct
{
   HMODULE hModule;
   ULONG (* APIENTRY QueryVersion)(VOID);
   ULONG (* APIENTRY QueryParamBlockSize)(VOID);
   ULONG (* APIENTRY SetupParams)(PVOID, ULONG, HWND, HAB, HMODULE);
   ULONG (* APIENTRY AddEcho)(PVOID, ULONG, PCHAR, PCHAR, PCHAR, PCHAR, ULONG);
   ULONG (* APIENTRY RemoveEcho)(PVOID, ULONG, PCHAR, PCHAR, PCHAR, PCHAR, ULONG);
} FUNCTABLE, *PFUNCTABLE;

#define LOADDLL_OK          0
#define LOADDLL_CANTLOAD    1
#define LOADDLL_VERSION     2
#define LOADDLL_FUNCMISSING 3

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HMODULE hmodLang;

extern ECHOMGROPT EchoMgrOpt;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/
static void InsertEchos(HWND hwndCnr, PECHOMGRDATA pEchoMgrData);
static void CleanupContainer(HWND hwndCnr);
static void OpenPopup(HWND hwndDlg, PECHOMGRRECORD pRecord, PECHOMGRDATA pEchoMgrData);
static MRESULT EXPENTRY EchoMgrSettings(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void InsertPages(HWND hwndDlg, PVOID pParam);
static MRESULT EXPENTRY UplinkSettings(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY DllSettings(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static char *CreateMessageText(HWND hwndCnr, PECHOMGRDATA pEchoMgrData);
static PACTIONLIST CreateActionList(HWND hwndCnr);
static int CreateMessage(HWND hwndCnr, PECHOMGRDATA pEchoMgrData, PFTNMESSAGE pMessage, PMSGHEADER pHeader);
static void _Optlink MessageSaveThread(PVOID pParam);
static void _Optlink CfgChangeThread(PVOID pParam);
static int LoadExtensionDLL(char *pchDllName, PFUNCTABLE pFuncTable);
static void ConfigureDLL(HWND hwndDlg, PFUNCTABLE pFuncTable);
static PUPLINK FindUplink(PECHOMGROPT pEchoMgrOpt, char *pchUplinkAddress);
static PUPLINK FindMatchingUplink(PECHOMGROPT pEchoMgrOpt, char *pchMyAddress);
static char *ExtractAreasFromMessage(char *pchMessageText, char *pchOldAreas);
static int ChangeUplink(HWND hwndDlg, SHORT sItem);
static int DeleteUplink(HWND hwndDlg, SHORT sItem, PECHOMGRDATA pEchoMgrData);
static MRESULT EXPENTRY UplinkProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static int RemoveEchoFromUplink(HWND hwndDlg, PECHOMGRDATA pEchoMgrData);
static void CreateUplinkMenu(PECHOMGRDATA pEchoMgrData, ECHOMGROPT *pEchoMgrOpt);
static void SwitchUplink(HWND hwndDlg, PECHOMGRDATA pEchoMgrData, USHORT usID);
static SHORT _System SortEchos(PRECORDCORE p1, PRECORDCORE p2, PVOID pData);
static BOOL HaveArea(PSTRINGLIST pList, char *pchTag);

/*---------------------------------------------------------------------------*/
/* Funktionsname: EchoMgrProc                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Prozedur des Echo-Managers                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY EchoMgrProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;
   extern char CurrentAddress[LEN_5DADDRESS+1];
   PECHOMGRDATA pEchoMgrData = WinQueryWindowPtr(hwnd, 0);
   PFIELDINFO pFieldInfo, pFirstFieldInfo;
   HWND hwndCnr;
   FIELDINFOINSERT FieldInfoInsert;
   CNRINFO CnrInfo;
   PACTIONLIST pActionList=NULL;

   switch(msg)
   {
      case WM_INITDLG:
         /* Instanzdaten */
         pEchoMgrData = calloc(1, sizeof(ECHOMGRDATA));
         WinSetWindowPtr(hwnd, 0, pEchoMgrData);

         /* Strings laden */
         LoadString(IDST_EM_ECHO, ECHOMGR_STRINGLEN, pEchoMgrData->pchTitleEcho);
         LoadString(IDST_EM_STATUS, ECHOMGR_STRINGLEN, pEchoMgrData->pchTitleStatus);
         LoadString(IDST_EM_ACTION, ECHOMGR_STRINGLEN, pEchoMgrData->pchTitleAction);
         LoadString(IDST_EM_LINKED, ECHOMGR_STRINGLEN, pEchoMgrData->pchStatusLinked);
         LoadString(IDST_EM_UNLINKED, ECHOMGR_STRINGLEN, pEchoMgrData->pchStatusUnlinked);
         LoadString(IDST_EM_LINK, ECHOMGR_STRINGLEN, pEchoMgrData->pchActionLink);
         LoadString(IDST_EM_UNLINK, ECHOMGR_STRINGLEN, pEchoMgrData->pchActionUnlink);
         LoadString(IDST_EM_RESCAN, ECHOMGR_STRINGLEN, pEchoMgrData->pchActionRescan);
         LoadString(IDST_EM_REFRESH, ECHOMGR_STRINGLEN, pEchoMgrData->pchActionRefresh);
         LoadString(IDST_EM_PAUSE, ECHOMGR_STRINGLEN, pEchoMgrData->pchActionPause);
         LoadString(IDST_EM_RESUME, ECHOMGR_STRINGLEN, pEchoMgrData->pchActionResume);

         /* Menues laden */
         pEchoMgrData->hwndPopup = WinLoadMenu(HWND_OBJECT, hmodLang, IDM_EM_POPUP);
         pEchoMgrData->hwndFolderPopup = WinLoadMenu(HWND_OBJECT, hmodLang, IDM_EMF_POPUP);

         /* Spalten im Container */
         hwndCnr = WinWindowFromID(hwnd, IDD_ECHOMANAGER+1);
         pFirstFieldInfo=(PFIELDINFO)SendMsg(hwndCnr, CM_ALLOCDETAILFIELDINFO,
                                                MPFROMLONG(3), NULL);

         pFieldInfo=pFirstFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         pFieldInfo->pTitleData= pEchoMgrData->pchTitleEcho;
         pFieldInfo->offStruct= FIELDOFFSET(ECHOMGRRECORD, pchEchoName);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         pFieldInfo->pTitleData= pEchoMgrData->pchTitleStatus;
         pFieldInfo->offStruct= FIELDOFFSET(ECHOMGRRECORD, pchStatus);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR;
         pFieldInfo->flTitle=0;
         pFieldInfo->pTitleData= pEchoMgrData->pchTitleAction;
         pFieldInfo->offStruct= FIELDOFFSET(ECHOMGRRECORD, pchAction);

         /* Felder des Containers einfuegen */
         FieldInfoInsert.cb=sizeof(FIELDINFOINSERT);
         FieldInfoInsert.pFieldInfoOrder=(PFIELDINFO) CMA_FIRST;
         FieldInfoInsert.fInvalidateFieldInfo=TRUE;
         FieldInfoInsert.cFieldInfoInsert=3;

         SendMsg(hwndCnr, CM_INSERTDETAILFIELDINFO,
                    pFirstFieldInfo, &FieldInfoInsert);

         pEchoMgrData->pUplink = FindMatchingUplink(&EchoMgrOpt, CurrentAddress);

         /* Container-Attribute setzen */
         CnrInfo.cb=sizeof(CNRINFO);
         CnrInfo.flWindowAttr=CV_DETAIL | CA_DETAILSVIEWTITLES | CA_CONTAINERTITLE |
                              CA_TITLEREADONLY | CA_TITLESEPARATOR;
         CnrInfo.pSortRecord = (PVOID) SortEchos;

         if (pEchoMgrData->pUplink)
            CnrInfo.pszCnrTitle=pEchoMgrData->pUplink->pchEchoMgrAddress;
         else
            CnrInfo.pszCnrTitle=NULL;

         SendMsg(hwndCnr, CM_SETCNRINFO, &CnrInfo,
                    MPFROMLONG(CMA_FLWINDOWATTR | CMA_CNRTITLE | CMA_PSORTRECORD));

         CreateUplinkMenu(pEchoMgrData, &EchoMgrOpt);

         if (pEchoMgrData->pUplink)
            InsertEchos(hwndCnr, pEchoMgrData);

         /* Fenstergroesse herstellen */
         SetForeground(hwndCnr, &EchoMgrOpt.lFolderFore);
         SetBackground(hwndCnr, &EchoMgrOpt.lFolderBack);
         SetFont(hwndCnr, EchoMgrOpt.pchFolderFont);

         RestoreWinPos(hwnd, &EchoMgrOpt.FolderPos, TRUE, TRUE);

         pEchoMgrData->bNotify=TRUE;
         break;

      case WM_DESTROY:
         {
            LONG lColor;
            char pchFont[FACESIZE+5];
            HWND hwndCnr = WinWindowFromID(hwnd, IDD_ECHOMANAGER+1);

            QueryForeground(hwndCnr, &lColor);
            if (EchoMgrOpt.lFolderFore != lColor)
            {
               EchoMgrOpt.lFolderFore = lColor;
               EchoMgrOpt.bDirty=TRUE;
            }
            QueryBackground(hwndCnr, &lColor);
            if (EchoMgrOpt.lFolderBack != lColor)
            {
               EchoMgrOpt.lFolderBack = lColor;
               EchoMgrOpt.bDirty=TRUE;
            }
            QueryFont(hwndCnr, pchFont);
            if (strcmp(EchoMgrOpt.pchFolderFont, pchFont))
            {
               strcpy(EchoMgrOpt.pchFolderFont, pchFont);
               EchoMgrOpt.bDirty=TRUE;
            }
            CleanupContainer(hwndCnr);
            if (pEchoMgrData->hwndPopup)
               WinDestroyWindow(pEchoMgrData->hwndPopup);
            if (pEchoMgrData->hwndFolderPopup)
               WinDestroyWindow(pEchoMgrData->hwndFolderPopup);
            if (pEchoMgrData->pMenuTable)
               free(pEchoMgrData->pMenuTable);
            free(pEchoMgrData);
         }
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               memset(&pEchoMgrData->SaveParam, 0, sizeof(SAVEPARAM));
               pEchoMgrData->SaveParam.hwndDlg = hwnd;
               if (pEchoMgrData->pUplink)
               {
                  switch(CreateMessage(WinWindowFromID(hwnd, IDD_ECHOMANAGER+1), pEchoMgrData,
                                    &pEchoMgrData->SaveParam.Message, &pEchoMgrData->SaveParam.Header))
                  {
                     case 0:
                        if (EchoMgrOpt.pchDllName[0])
                           pActionList = CreateActionList(WinWindowFromID(hwnd, IDD_ECHOMANAGER+1));

                        /* Message speichern */
                        WinEnableControl(hwnd, DID_OK, FALSE);
                        WinEnableControl(hwnd, DID_CANCEL, FALSE);

                        if (pActionList)
                        {
                           PCHANGEPARAM pChangeParam;

                           pChangeParam = malloc(sizeof(CHANGEPARAM));
                           pChangeParam->hwndDlg = hwnd;
                           pChangeParam->pUplink = pEchoMgrData->pUplink;
                           pChangeParam->pActionList = pActionList;
                           _beginthread(CfgChangeThread, NULL, 32768, pChangeParam);
                        }
                        else
                           _beginthread(MessageSaveThread, NULL, 32768, &pEchoMgrData->SaveParam);

                        return (MRESULT) FALSE;

                     case 1:
                        MessageBox(hwnd, IDST_MSG_NOACTIONS, 0, IDD_NOACTIONS, MB_ERROR | MB_OK);
                        return (MRESULT) FALSE;

                     case 2:
                        MessageBox(hwnd, IDST_MSG_EM_NOPASS, 0, IDD_EM_NOPASS, MB_ERROR | MB_OK);
                        return (MRESULT) FALSE;

                     default:
                        return (MRESULT) FALSE;
                  }
               }
               break;

            case DID_CANCEL:
               if (WinIsWindowEnabled(WinWindowFromID(hwnd, DID_CANCEL)))
                  break;
               else
                  return (MRESULT) FALSE;

            case IDM_EMF_SETTINGS:
               {
                  EMANSETTINGSDATA EManSettingsData;

                  EManSettingsData.cb = sizeof(EManSettingsData);
                  EManSettingsData.pEchoMgrData = pEchoMgrData;
                  if (WinDlgBox(HWND_DESKTOP, hwnd, EchoMgrSettings, hmodLang,
                                IDD_ECHOMGRSETTINGS, &EManSettingsData) == DID_OK)
                     WinDismissDlg(hwnd, DID_CANCEL);
               }
               return (MRESULT)FALSE;

            case IDM_EMF_REFRESH:
               pEchoMgrData->ulAction = ACTION_REFRESH;
               CnrInfo.cb=sizeof(CNRINFO);
               CnrInfo.pszCnrTitle = pEchoMgrData->pchActionRefresh;

               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_SETCNRINFO, &CnrInfo,
                          MPFROMLONG(CMA_CNRTITLE));
               return (MRESULT)FALSE;

            case IDM_EMF_PAUSE:
               pEchoMgrData->ulAction = ACTION_PAUSE;
               CnrInfo.cb=sizeof(CNRINFO);
               CnrInfo.pszCnrTitle = pEchoMgrData->pchActionPause;

               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_SETCNRINFO, &CnrInfo,
                          MPFROMLONG(CMA_CNRTITLE));
               return (MRESULT)FALSE;

            case IDM_EMF_RESUME:
               pEchoMgrData->ulAction = ACTION_RESUME;
               CnrInfo.cb=sizeof(CNRINFO);
               CnrInfo.pszCnrTitle = pEchoMgrData->pchActionResume;

               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_SETCNRINFO, &CnrInfo,
                          MPFROMLONG(CMA_CNRTITLE));
               return (MRESULT)FALSE;

            case IDM_EMF_RESET:
               pEchoMgrData->ulAction = ACTION_NONE;
               CnrInfo.cb=sizeof(CNRINFO);
               if (pEchoMgrData->pUplink)
                  CnrInfo.pszCnrTitle = pEchoMgrData->pUplink->pchEchoMgrAddress;
               else
                  CnrInfo.pszCnrTitle = NULL;

               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_SETCNRINFO, &CnrInfo,
                          MPFROMLONG(CMA_CNRTITLE));
               return (MRESULT)FALSE;

            case IDM_EM_LINK:
               pEchoMgrData->pPopupRecord->ulAction = ACTION_LINK;
               pEchoMgrData->pPopupRecord->pchAction = pEchoMgrData->pchActionLink;
               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_INVALIDATERECORD,
                                 &pEchoMgrData->pPopupRecord, MPFROM2SHORT(1, CMA_TEXTCHANGED));
               return (MRESULT)FALSE;

            case IDM_EM_UNLINK:
               pEchoMgrData->pPopupRecord->ulAction = ACTION_UNLINK;
               pEchoMgrData->pPopupRecord->pchAction = pEchoMgrData->pchActionUnlink;
               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_INVALIDATERECORD,
                                 &pEchoMgrData->pPopupRecord, MPFROM2SHORT(1, CMA_TEXTCHANGED));
               return (MRESULT)FALSE;

            case IDM_EM_RESCAN:
               if (pEchoMgrData->pPopupRecord->ulAction == ACTION_LINK)
                  pEchoMgrData->pPopupRecord->ulAction = ACTION_LNKRESC;
               else
                  pEchoMgrData->pPopupRecord->ulAction = ACTION_RESCAN;
               pEchoMgrData->pPopupRecord->pchAction = pEchoMgrData->pchActionRescan;
               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_INVALIDATERECORD,
                                 &pEchoMgrData->pPopupRecord, MPFROM2SHORT(1, CMA_TEXTCHANGED));
               return (MRESULT)FALSE;

            case IDM_EM_RESET:
               pEchoMgrData->pPopupRecord->ulAction = ACTION_NONE;
               pEchoMgrData->pPopupRecord->pchAction = NULL;
               WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_INVALIDATERECORD,
                                 &pEchoMgrData->pPopupRecord, MPFROM2SHORT(1, CMA_TEXTCHANGED));
               return (MRESULT)FALSE;

            case IDM_EM_REMOVE:
               RemoveEchoFromUplink(hwnd, pEchoMgrData);
               return (MRESULT)FALSE;

            default:
               SwitchUplink(hwnd, pEchoMgrData, SHORT1FROMMP(mp1));
               return (MRESULT)FALSE;
         }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_ECHOMANAGER+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               case CN_CONTEXTMENU:
                  if (WinIsWindowEnabled(WinWindowFromID(hwnd, DID_CANCEL)))
                     OpenPopup(hwnd, (PECHOMGRRECORD) mp2, pEchoMgrData);
                  break;

               default:
                  break;
            }
         }
         break;

      case WM_CLOSE:
         if (WinIsWindowEnabled(WinWindowFromID(hwnd, DID_CANCEL)))
            break;
         else
            return (MRESULT) FALSE;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CONTEXTMENU:
         if (WinIsWindowEnabled(WinWindowFromID(hwnd, DID_CANCEL)) &&
             WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(hwnd, IDD_ECHOMANAGER+1))
         {
            pEchoMgrData->bKeyboard=TRUE;
            WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, msg, mp1, mp2);
         }
         break;

      case WM_WINDOWPOSCHANGED:
         if (pEchoMgrData && pEchoMgrData->bNotify)
            SaveWinPos(hwnd, (PSWP) mp1, &EchoMgrOpt.FolderPos, &EchoMgrOpt.bDirty);
         break;

      case WM_ADJUSTFRAMEPOS:
         if (((PSWP)mp1)->fl & (SWP_SIZE|SWP_MAXIMIZE|SWP_MINIMIZE|SWP_RESTORE))
         {
            SWP swp;
            RECTL rectl;

            rectl.xLeft=0;
            rectl.xRight=((PSWP)mp1)->cx;
            rectl.yBottom=0;
            rectl.yTop=((PSWP)mp1)->cy;

            CalcClientRect(anchor, hwnd, &rectl);
            WinQueryWindowPos(WinWindowFromID(hwnd, DID_OK), &swp);
            rectl.yBottom += swp.y + swp.cy;
            WinSetWindowPos(WinWindowFromID(hwnd, IDD_ECHOMANAGER+1),
                            NULLHANDLE,
                            rectl.xLeft, rectl.yBottom,
                            rectl.xRight-rectl.xLeft, rectl.yTop-rectl.yBottom,
                            SWP_MOVE | SWP_SIZE);
         }
         break;

      case WM_QUERYTRACKINFO:
         {
            SWP swp;
            MRESULT resultbuf;

            /* Default-Werte aus Original-Prozedur holen */
            resultbuf=WinDefDlgProc(hwnd, msg, mp1, mp2);

            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_ECHOMANAGER+2), &swp);

            /* Minimale Fenstergroesse einstellen */
            ((PTRACKINFO)mp2)->ptlMinTrackSize.x=swp.x+swp.cx+5;
            ((PTRACKINFO)mp2)->ptlMinTrackSize.y=200;
            return resultbuf;
         }

      case WM_MENUEND:
         if ((HWND) mp2 == pEchoMgrData->hwndPopup ||
             (HWND) mp2 == pEchoMgrData->hwndFolderPopup)
            WinSendDlgItemMsg(hwnd, IDD_ECHOMANAGER+1, CM_SETRECORDEMPHASIS,
                              pEchoMgrData->pPopupRecord,
                              MPFROM2SHORT(FALSE, CRA_SOURCE));
         break;

      case WM_CHANGEDONE:
         switch((ULONG) mp1)
         {
            case LOADDLL_CANTLOAD:
               MessageBox(hwnd, IDST_MSG_EM_DLLLOAD, 0, IDD_EM_DLLLOAD, MB_ERROR | MB_OK);
               break;

            case LOADDLL_VERSION:
               MessageBox(hwnd, IDST_MSG_EM_DLLVER, 0, IDD_EM_DLLVER, MB_ERROR | MB_OK);
               break;

            case LOADDLL_FUNCMISSING:
               MessageBox(hwnd, IDST_MSG_EM_DLLFUNC, 0, IDD_EM_DLLFUNC, MB_ERROR | MB_OK);
               break;

            default:
               break;
         }
         switch((ULONG) mp1)
         {
            case ECHOMAN_OK:
               break;

            case ECHOMAN_PARAMSIZE:
            case ECHOMAN_FORMAT:
               MessageBox(hwnd, IDST_MSG_EM_INIT, 0, IDD_EM_INIT, MB_ERROR | MB_OK);
               break;

            case ECHOMAN_CFGNOTFOUND:
               MessageBox(hwnd, IDST_MSG_EM_CFGNOTF, 0, IDD_EM_CFGNOTF, MB_ERROR | MB_OK);
               break;

            case ECHOMAN_CFGREAD:
               MessageBox(hwnd, IDST_MSG_EM_CFGREAD, 0, IDD_EM_CFGREAD, MB_ERROR | MB_OK);
               break;

            case ECHOMAN_CFGWRITE:
               MessageBox(hwnd, IDST_MSG_EM_CFGWRITE, 0, IDD_EM_CFGWRITE, MB_ERROR | MB_OK);
               break;

            case ECHOMAN_CFGFORMAT:
               MessageBox(hwnd, IDST_MSG_EM_CFGFORMAT, 0, IDD_EM_CFGFORMAT, MB_ERROR | MB_OK);
               break;

            case ECHOMAN_ALREADYLINKED:
               MessageBox(hwnd, IDST_MSG_EM_CFGLINKED, 0, IDD_EM_CFGLINKED, MB_ERROR | MB_OK);
               break;

            case ECHOMAN_NOTLINKED:
               MessageBox(hwnd, IDST_MSG_EM_CFGUNLINKED, 0, IDD_EM_CFGUNLINKED, MB_ERROR | MB_OK);
               break;

            default:
               MessageBox(hwnd, IDST_MSG_EM_DLLINT, 0, IDD_EM_DLLINT, MB_ERROR | MB_OK);
               break;
         }
         _beginthread(MessageSaveThread, NULL, 32768, &pEchoMgrData->SaveParam);
         break;

      case WM_SAVEDONE:
         switch((ULONG) mp1)
         {
            case 0:
               break;

            default:
               MessageBox(hwnd, IDST_MSG_ERRORMSGSAVE, 0, IDD_ERRORMSGSAVE, MB_OK | MB_ERROR);
               break;
         }
         WinDismissDlg(hwnd, DID_OK);
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static SHORT _System SortEchos(PRECORDCORE p1, PRECORDCORE p2, PVOID pData)
{
   pData = pData;

   return stricmp(((PECHOMGRRECORD)p1)->pchEchoName, ((PECHOMGRRECORD)p2)->pchEchoName);
}

/*-----------------------------------------------------------------------------
 | Funktionsname: SwitchUplink
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Schaltet den aktuellen Uplink um
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwndDlg: Dialog-Window-Handle
 |            pEchoMgrData: Instanzdaten des Echo-Managers
 |            usID: ID des gewÑhlten Uplink-Menu-Punktes
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static void SwitchUplink(HWND hwndDlg, PECHOMGRDATA pEchoMgrData, USHORT usID)
{
   int iUplink=0;

   /* Uplink in Tabelle suchen */
   while (iUplink < pEchoMgrData->ulCountUplinks)
   {
      if (pEchoMgrData->pMenuTable[iUplink].usID == usID)
         break;
      iUplink++;
   }

   if (iUplink < pEchoMgrData->ulCountUplinks) /* gefunden */
   {
      if (pEchoMgrData->pMenuTable[iUplink].pUplink != pEchoMgrData->pUplink) /* anderer als aktueller */
      {
         CNRINFO CnrInfo;
         HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_ECHOMANAGER+1);

         /* umschalten */
         CleanupContainer(hwndCnr);
         pEchoMgrData->pUplink = pEchoMgrData->pMenuTable[iUplink].pUplink;
         InsertEchos(hwndCnr, pEchoMgrData);

         /* Aktionen zuruecksetzen */
         pEchoMgrData->ulAction = ACTION_NONE;

         CnrInfo.cb=sizeof(CNRINFO);

         if (pEchoMgrData->pUplink)
            CnrInfo.pszCnrTitle=pEchoMgrData->pUplink->pchEchoMgrAddress;
         else
            CnrInfo.pszCnrTitle=NULL;

         SendMsg(hwndCnr, CM_SETCNRINFO, &CnrInfo, MPFROMLONG(CMA_CNRTITLE));
      }
   }
   return;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertEchos                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt alle Echos in den Container ein                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            pEchoMgrData: Instanzdaten des Echomanagers                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InsertEchos(HWND hwndCnr, PECHOMGRDATA pEchoMgrData)
{
   extern AREALIST arealiste;
   ULONG ulNumAreas=0;
   PCHAR pchTemp;
   PCHAR pchCopy;
   PECHOMGRRECORD pRecord, pFirstRecord;
   RECORDINSERT RecordInsert;

   if (!pEchoMgrData->pUplink->pchUplinkAreas ||
       !pEchoMgrData->pUplink->pchUplinkAreas[0])
      return;

   pchTemp = pEchoMgrData->pUplink->pchUplinkAreas;
   while (*pchTemp == ' ')
      pchTemp++;

   while (*pchTemp)
   {
      if (*pchTemp == ' ')
      {
         ulNumAreas++;
         while (*pchTemp == ' ')
            pchTemp++;
      }
      else
         pchTemp++;
   }
   ulNumAreas++;

   pchCopy = strdup(pEchoMgrData->pUplink->pchUplinkAreas);

   pFirstRecord = SendMsg(hwndCnr, CM_ALLOCRECORD,
                             MPFROMLONG(sizeof(ECHOMGRRECORD) - sizeof(MINIRECORDCORE)),
                             MPFROMLONG(ulNumAreas));
   pRecord = pFirstRecord;

   pchTemp = strtok(pchCopy, " ");

   while (pchTemp)
   {
      pRecord->pchEchoName = strdup(pchTemp);
      if (AM_FindArea(&arealiste, pchTemp))
      {
         pRecord->pchStatus = pEchoMgrData->pchStatusLinked;
         pRecord->ulStatus = STATUS_LINKED;
      }
      else
      {
         pRecord->pchStatus = pEchoMgrData->pchStatusUnlinked;
         pRecord->ulStatus = STATUS_UNLINKED;
      }
      pRecord->pchAction = "";
      pRecord->ulAction= ACTION_NONE;

      pRecord = (PECHOMGRRECORD) pRecord->RecordCore.preccNextRecord;

      pchTemp = strtok(NULL, " ");
   }

   free(pchCopy);

   RecordInsert.cb = sizeof(RecordInsert);
   RecordInsert.pRecordOrder = (PRECORDCORE) CMA_FIRST;
   RecordInsert.pRecordParent = NULL;
   RecordInsert.fInvalidateRecord = TRUE;
   RecordInsert.zOrder = CMA_TOP;
   RecordInsert.cRecordsInsert = ulNumAreas;

   SendMsg(hwndCnr, CM_INSERTRECORD, pFirstRecord, &RecordInsert);

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: CreateUplinkMenu
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Erzeugt das Unter-MenÅ mit den Uplinks
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pEchoMgrData: Instanzdaten
 |            pEchoMgrOpt: Echo-Manager-Optionen
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static void CreateUplinkMenu(PECHOMGRDATA pEchoMgrData, ECHOMGROPT *pEchoMgrOpt)
{
   PUPLINK pUplink = pEchoMgrOpt->pUplinks;
   int iNumUplinks=0;
   int iUplink=0;
   MENUITEM SubMenu;
   MENUITEM NewItem = {MIT_END, MIS_TEXT, 0, 0, NULLHANDLE, NULLHANDLE};
   HWND hwndMenu;

   /* Uplinks zaehlen */
   while (pUplink)
   {
      iNumUplinks++;
      pUplink = pUplink->next;
   }

   /* Anzahl begrenzen */
   if (iNumUplinks >= (IDM_EM_POPUP - IDM_EMF_UPLINK))
      iNumUplinks = IDM_EM_POPUP - IDM_EMF_UPLINK - 1;

   if (iNumUplinks) /* nur, wenn wirklich welche da */
   {
      /* Feld anlegen */
      pEchoMgrData->pMenuTable = calloc(iNumUplinks, sizeof(MENUTABLE));
      pEchoMgrData->ulCountUplinks = iNumUplinks;

      /* Sub-Menu holen */
      SendMsg(pEchoMgrData->hwndFolderPopup, MM_QUERYITEM,
              MPFROM2SHORT(IDM_EMF_UPLINK, FALSE), &SubMenu);

      hwndMenu = SubMenu.hwndSubMenu;

      /* alle Uplinks durchgehen */
      pUplink = pEchoMgrOpt->pUplinks;
      while (pUplink && iUplink < iNumUplinks)
      {
         /* Menue-Eintrag erzeugen */
         NewItem.id = IDM_EMF_UPLINK+1+iUplink;

         SendMsg(hwndMenu, MM_INSERTITEM, &NewItem, pUplink->pchEchoMgrAddress);

         /* in Tabelle eintragen */
         pEchoMgrData->pMenuTable[iUplink].usID = NewItem.id;
         pEchoMgrData->pMenuTable[iUplink].pUplink = pUplink;

         /* weiter */
         iUplink++;
         pUplink = pUplink->next;
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CleanupContainer                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Entfernt alle Records vom Container                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CleanupContainer(HWND hwndCnr)
{
   PECHOMGRRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
   {
      if (pRecord->pchEchoName)
         free(pRecord->pchEchoName);
   }

   SendMsg(hwndCnr, CM_REMOVERECORD, NULL, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenPopup                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet das Popup-Menue des Echo-Managers                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Echo-Manager-Dialog                                   */
/*            pRecord: Record, fuer den das Popup-Menue geoeffnet wurde      */
/*            pEchoMgrData: Instanz-Daten                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void OpenPopup(HWND hwndDlg, PECHOMGRRECORD pRecord, PECHOMGRDATA pEchoMgrData)
{
   POINTL pointl;

   if (!pEchoMgrData->bKeyboard)
      WinQueryPointerPos(HWND_DESKTOP, &pointl);
   else
   {
      QUERYRECORDRECT qRecord;
      RECTL rectl;
      SWP swp;

      WinQueryWindowPos(WinWindowFromID(hwndDlg, IDD_ECHOMANAGER+1), &swp);

      if (pRecord)
      {
         qRecord.cb = sizeof(qRecord);
         qRecord.pRecord = (PRECORDCORE) pRecord;
         qRecord.fRightSplitWindow=FALSE;
         qRecord.fsExtent = CMA_TEXT;

         WinSendDlgItemMsg(hwndDlg, IDD_ECHOMANAGER+1, CM_QUERYRECORDRECT,
                           &rectl, &qRecord);

         pointl.x = swp.x + rectl.xLeft+ (rectl.xRight - rectl.xLeft)/2;
         pointl.y = swp.y + rectl.yBottom + (rectl.yTop - rectl.yBottom)/2;
      }
      else
      {
         pointl.x = swp.x + swp.cx/2;
         pointl.y = swp.y + swp.cy/2;
      }
      WinMapWindowPoints(hwndDlg, HWND_DESKTOP, &pointl, 1);
   }

   pEchoMgrData->pPopupRecord = pRecord;
   WinSendDlgItemMsg(hwndDlg, IDD_ECHOMANAGER+1, CM_SETRECORDEMPHASIS, pRecord,
                     MPFROM2SHORT(TRUE, CRA_SOURCE));

   if (pRecord)
   {
      if (pRecord->ulStatus == STATUS_LINKED)
      {
         WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_LINK, FALSE);
         WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_UNLINK, TRUE);
         WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_RESCAN, TRUE);
      }
      else
      {
         if (pRecord->ulAction)
            WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_LINK, FALSE);
         else
            WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_LINK, TRUE);
         WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_UNLINK, FALSE);
         if (pRecord->ulAction == ACTION_LINK)
            WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_RESCAN, TRUE);
         else
            WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_RESCAN, FALSE);
      }

      if (pRecord->ulAction)
         WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_RESET, TRUE);
      else
         WinEnableMenuItem(pEchoMgrData->hwndPopup, IDM_EM_RESET, FALSE);

      WinPopupMenu(HWND_DESKTOP, hwndDlg, pEchoMgrData->hwndPopup, pointl.x, pointl.y,
                   0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }
   else
   {
      if (pEchoMgrData->pUplink)
      {
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_REFRESH, TRUE);
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_PAUSE, TRUE);
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_RESUME, TRUE);
         if (pEchoMgrData->ulAction)
            WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_RESET, TRUE);
         else
            WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_RESET, FALSE);
      }
      else
      {
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_REFRESH, FALSE);
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_PAUSE, FALSE);
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_RESUME, FALSE);
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_RESET, FALSE);
      }
      if (EchoMgrOpt.pUplinks)
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_UPLINK, TRUE);
      else
         WinEnableMenuItem(pEchoMgrData->hwndFolderPopup, IDM_EMF_UPLINK, FALSE);
      WinPopupMenu(HWND_DESKTOP, hwndDlg, pEchoMgrData->hwndFolderPopup, pointl.x, pointl.y,
                   0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }

   pEchoMgrData->bKeyboard = FALSE;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: EchoMgrSettings                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fenster-Prozedur fuer die Echo-Manager-Settings             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Wenn der Dialog mit DID_OK beendet wird, wird angezeigt,       */
/*            da· die Liste des aktuellen Uplinks geloescht wurde. Der       */
/*            Echo-Manager muss sich dann mit DID_CANCEL beenden.            */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY EchoMgrSettings(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;

   switch(msg)
   {
      case WM_INITDLG:
         ((PEMANSETTINGSDATA)mp2)->hwndSettingsDlg = hwnd;
         InsertPages(hwnd, (PVOID) mp2);
         RestoreWinPos(hwnd, &EchoMgrOpt.SettingsPos, TRUE, TRUE);
         WinSetWindowULong(hwnd, QWL_USER, 1);
         break;

      case WM_DESTROY:
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_WINDOWPOSCHANGED:
         if (WinQueryWindowULong(hwnd, QWL_USER))
            SaveWinPos(hwnd, (PSWP) mp1, &EchoMgrOpt.SettingsPos, &EchoMgrOpt.bDirty);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_ECHOMGRSETTINGS+1);
         break;

      case WM_CURRENTDEL:
         WinDismissDlg(hwnd, DID_OK);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertPages                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt alle Seiten in das Notebook ein                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window                                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InsertPages(HWND hwndDlg, PVOID pParam)
{
   HWND hwndNotebook = WinWindowFromID(hwndDlg, IDD_ECHOMGRSETTINGS+1);

   SetNotebookParams(hwndNotebook, 120);

   InsertOnePage(hwndNotebook, IDD_EMS_UPLINKS, IDST_TAB_UPLINKS, UplinkSettings, pParam);
   InsertOnePage(hwndNotebook, IDD_EMS_EXTDLL, IDST_TAB_DLLEXT, DllSettings, pParam);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: UplinkSettings                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fenster-Prozedur der Seite "Uplinks"                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY UplinkSettings(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   SHORT sItem;
   PUPLINK pUplink;
   PEMANSETTINGSDATA pEManData = (PEMANSETTINGSDATA) WinQueryWindowULong(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_INITDLG:
         pEManData = (PEMANSETTINGSDATA) mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) mp2);

         pUplink = EchoMgrOpt.pUplinks;
         while (pUplink)
         {
            sItem = (SHORT) WinSendDlgItemMsg(hwnd, IDD_EMS_UPLINKS+1, LM_INSERTITEM,
                                              MPFROMSHORT(LIT_END), pUplink->pchEchoMgrAddress);
            if (sItem != LIT_MEMERROR &&
                sItem != LIT_ERROR)
               WinSendDlgItemMsg(hwnd, IDD_EMS_UPLINKS+1, LM_SETITEMHANDLE,
                                 MPFROMSHORT(sItem), pUplink);

            pUplink = pUplink->next;
         }

         if (EchoMgrOpt.pUplinks)
         {
            WinEnableControl(hwnd, IDD_EMS_UPLINKS+3, TRUE);
            WinEnableControl(hwnd, IDD_EMS_UPLINKS+4, TRUE);
         }
         break;

      case WM_DESTROY:
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_EMS_UPLINKS+3: /* Change */
               sItem = (SHORT) WinSendDlgItemMsg(hwnd, IDD_EMS_UPLINKS+1, LM_QUERYSELECTION,
                                                 MPFROMSHORT(LIT_FIRST), NULL);
               if (sItem != LIT_NONE)
                  ChangeUplink(hwnd, sItem);
               break;

            case IDD_EMS_UPLINKS+4: /* Delete */
               sItem = (SHORT) WinSendDlgItemMsg(hwnd, IDD_EMS_UPLINKS+1, LM_QUERYSELECTION,
                                                 MPFROMSHORT(LIT_FIRST), NULL);
               if (sItem != LIT_NONE)
                  DeleteUplink(hwnd, sItem, pEManData->pEchoMgrData);
               if (!EchoMgrOpt.pUplinks)
               {
                  WinEnableControl(hwnd, IDD_EMS_UPLINKS+3, FALSE);
                  WinEnableControl(hwnd, IDD_EMS_UPLINKS+4, FALSE);
               }
               break;

            default:
               break;
         }
         return (MRESULT) FALSE;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_EMS_UPLINKS+1)
         {
            if (SHORT2FROMMP(mp1) == LN_ENTER)
            {
               sItem = (SHORT) SendMsg((HWND)mp2, LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST), NULL);
               if (sItem != LIT_NONE)
                  ChangeUplink(hwnd, sItem);
            }
         }
         break;

      case WM_CURRENTDEL:
         WinPostMsg(pEManData->hwndSettingsDlg, WM_CURRENTDEL, NULL, NULL);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ChangeUplink                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet Dialog zur Einstellung eines Uplinks                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window (Owner)                                 */
/*            sItem: AusgewÑhltes Item in der Uplink-Liste                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 1  OK                                                      */
/*                0  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int ChangeUplink(HWND hwndDlg, SHORT sItem)
{
   UPLINKPARAM UplinkParam = {sizeof(UPLINKPARAM), NULL};

   UplinkParam.pUplink = WinSendDlgItemMsg(hwndDlg, IDD_EMS_UPLINKS+1, LM_QUERYITEMHANDLE,
                                           MPFROMSHORT(sItem), NULL);
   if (UplinkParam.pUplink)
   {
      if (WinDlgBox(HWND_DESKTOP, hwndDlg, UplinkProc, hmodLang, IDD_UPLINK,
                    &UplinkParam) == DID_OK)
         EchoMgrOpt.bDirty = TRUE;

      return 1;
   }
   else
      return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: UplinkProc                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Prozedur des Uplink-Settings-Dialogs                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY UplinkProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PUPLINK pUplink = (PUPLINK) WinQueryWindowULong(hwnd , QWL_USER);
   extern HWND hwndhelp;

   switch(msg)
   {
      case WM_INITDLG:
         pUplink = ((PUPLINKPARAM)mp2)->pUplink;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pUplink);
         WinSetWindowText(hwnd, pUplink->pchEchoMgrAddress);

         WinSendDlgItemMsg(hwnd, IDD_UPLINK+4, EM_SETTEXTLIMIT, MPFROMSHORT(LEN_USERNAME), NULL);
         WinSetDlgItemText(hwnd, IDD_UPLINK+4, pUplink->pchEchoMgrName);
         WinSendDlgItemMsg(hwnd, IDD_UPLINK+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(sizeof(pUplink->pchPassword)-1), NULL);
         WinSetDlgItemText(hwnd, IDD_UPLINK+2, pUplink->pchPassword);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)==DID_OK)
         {
            WinQueryDlgItemText(hwnd, IDD_UPLINK+4, LEN_USERNAME+1, pUplink->pchEchoMgrName);
            WinQueryDlgItemText(hwnd, IDD_UPLINK+2, sizeof(pUplink->pchPassword), pUplink->pchPassword);
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteUplink                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht einen Uplink aus der Liste                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window (Owner)                                 */
/*            sItem: AusgewÑhltes Item in der Uplink-Liste                   */
/*            pEchoMgrData: Echo-Manager-Instanzdaten                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 1  OK                                                      */
/*                0  Fehler                                                  */
/*                2  Abgebrochen                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int DeleteUplink(HWND hwndDlg, SHORT sItem, PECHOMGRDATA pEchoMgrData)
{
   PUPLINK pUplink;

   pUplink = WinSendDlgItemMsg(hwndDlg, IDD_EMS_UPLINKS+1, LM_QUERYITEMHANDLE,
                               MPFROMSHORT(sItem), NULL);
   if (pUplink)
   {
      int iUplink=0;

      /* Sicherheitsabfrage */
      if (MessageBox(hwndDlg, IDST_MSG_DELUPLINK, IDST_TITLE_DELUPLINK, IDD_DELUPLINK,
                     MB_QUERY | MB_YESNO) != MBID_YES)
         return 2;

      /* Im Menue suchen */
      while (iUplink < pEchoMgrData->ulCountUplinks)
      {
         if (pEchoMgrData->pMenuTable[iUplink].pUplink == pUplink)
            break;
         iUplink++;
      }
      if (iUplink < pEchoMgrData->ulCountUplinks)
      {
         /* Menue entfernen */
         SendMsg(pEchoMgrData->hwndFolderPopup, MM_REMOVEITEM,
                 MPFROM2SHORT(pEchoMgrData->pMenuTable[iUplink].usID, TRUE),
                 NULL);

         /* Tabelle updaten */
         if ((pEchoMgrData->ulCountUplinks - iUplink -1) > 0)
            memmove(&pEchoMgrData->pMenuTable[iUplink], &pEchoMgrData->pMenuTable[iUplink+1],
                    (pEchoMgrData->ulCountUplinks - iUplink -1) * sizeof(MENUTABLE));
         pEchoMgrData->ulCountUplinks--;
      }

      /* Aus Listbox entfernen */
      WinSendDlgItemMsg(hwndDlg, IDD_EMS_UPLINKS+1, LM_DELETEITEM,
                        MPFROMSHORT(sItem), NULL);

      /* Aus Liste aushaengen */
      if (pUplink->prev)
         pUplink->prev->next = pUplink->next;
      if (pUplink->next)
         pUplink->next->prev = pUplink->prev;
      if (EchoMgrOpt.pUplinks == pUplink)
         EchoMgrOpt.pUplinks = pUplink->next;
      if (EchoMgrOpt.pUplinksLast == pUplink)
         EchoMgrOpt.pUplinksLast = pUplink->prev;
      EchoMgrOpt.bDirty = TRUE;

      /* Element entfernen */
      if (pUplink->pchUplinkAreas)
         free(pUplink->pchUplinkAreas);
      free(pUplink);

      /* aktuellen Uplink geloescht? */
      if (pUplink == pEchoMgrData->pUplink)
         WinPostMsg(hwndDlg, WM_CURRENTDEL, NULL, NULL);

      return 1;
   }
   else
      return 0;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: DllSettings                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Notebook-Seite "Erweiterungs-DLL"                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY DllSettings(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg)
   {
      case WM_INITDLG:
         WinSendDlgItemMsg(hwnd, IDD_EMS_EXTDLL+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSetDlgItemText(hwnd, IDD_EMS_EXTDLL+2, EchoMgrOpt.pchDllName);
         if (EchoMgrOpt.pchDllName[0])
            WinEnableControl(hwnd, IDD_EMS_EXTDLL+4, TRUE);
         break;

      case WM_DESTROY:
         {
            char pchTemp[LEN_PATHNAME+1];

            WinQueryDlgItemText(hwnd, IDD_EMS_EXTDLL+2, sizeof(pchTemp), pchTemp);
            if (strcmp(EchoMgrOpt.pchDllName, pchTemp))
            {
               strcpy(EchoMgrOpt.pchDllName, pchTemp);
               EchoMgrOpt.bDirty = TRUE;
            }
         }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_EMS_EXTDLL+2)
            if (WinQueryDlgItemTextLength(hwnd, IDD_EMS_EXTDLL+2))
               WinEnableControl(hwnd, IDD_EMS_EXTDLL+4, TRUE);
            else
               WinEnableControl(hwnd, IDD_EMS_EXTDLL+4, FALSE);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_EMS_EXTDLL+3: /* Locate */
               {
                  char pchTemp[LEN_PATHNAME+1];

                  WinQueryDlgItemText(hwnd, IDD_EMS_EXTDLL+2, sizeof(pchTemp), pchTemp);
                  if (GetPathname(hwnd, pchTemp) == DID_OK)
                  {
                     WinSetDlgItemText(hwnd, IDD_EMS_EXTDLL+2, pchTemp);
                     strcpy(EchoMgrOpt.pchDllName, pchTemp);

                     EchoMgrOpt.bDirty=TRUE;
                  }
               }
               break;

            case IDD_EMS_EXTDLL+4: /* Configure */
               {
                  char pchTemp[LEN_PATHNAME+1];
                  FUNCTABLE FuncTable;

                  WinQueryDlgItemText(hwnd, IDD_EMS_EXTDLL+2, sizeof(pchTemp), pchTemp);
                  switch(LoadExtensionDLL(pchTemp, &FuncTable))
                  {
                     case LOADDLL_OK:
                        ConfigureDLL(hwnd, &FuncTable);
                        DosFreeModule(FuncTable.hModule);
                        break;

                     case LOADDLL_CANTLOAD:
                        MessageBox(hwnd, IDST_MSG_EM_DLLLOAD, 0, IDD_EM_DLLLOAD, MB_ERROR | MB_OK);
                        break;

                     case LOADDLL_VERSION:
                        MessageBox(hwnd, IDST_MSG_EM_DLLVER, 0, IDD_EM_DLLVER, MB_ERROR | MB_OK);
                        break;

                     case LOADDLL_FUNCMISSING:
                        MessageBox(hwnd, IDST_MSG_EM_DLLFUNC, 0, IDD_EM_DLLFUNC, MB_ERROR | MB_OK);
                        break;
                  }
               }
               break;

            default:
               break;
         }
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ConfigureDLL                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ruft die Konfigurations-Funktion der DLL auf                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window (Owner)                                 */
/*            pFuncTable: Funktionspointer-Tabelle der DLL                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ConfigureDLL(HWND hwndDlg, PFUNCTABLE pFuncTable)
{
   extern ECHOMGROPT EchoMgrOpt;
   ULONG ulSize=0;

   if (!EchoMgrOpt.pDllParams)
   {
      /* noch kein Parameter-Block, neu anfordern */
      ulSize = pFuncTable->QueryParamBlockSize();
      if (ulSize < MIN_PARAM_SIZE)
      {
         /* interner Fehler der DLL */
         MessageBox(hwndDlg, IDST_MSG_EM_DLLINT, 0, IDD_EM_DLLINT, MB_ERROR | MB_OK);
         return;
      }
      EchoMgrOpt.pDllParams = calloc(1, ulSize);
      EchoMgrOpt.ulParamLen = ulSize;
      EchoMgrOpt.bDirty = TRUE;
   }

   /* Setup aufrufen */
   switch(pFuncTable->SetupParams(EchoMgrOpt.pDllParams, EchoMgrOpt.ulParamLen, hwndDlg, anchor, pFuncTable->hModule))
   {
      case ECHOMAN_OK:
         EchoMgrOpt.bDirty = TRUE;
         return;

      case ECHOMAN_CANCEL:
         return;

      case ECHOMAN_PARAMSIZE:
      case ECHOMAN_FORMAT:
         /* nochmal versuchen, s.u. */
         break;

      default:
         /* interner Fehler der DLL */
         MessageBox(hwndDlg, IDST_MSG_EM_DLLINT, 0, IDD_EM_DLLINT, MB_ERROR | MB_OK);
         return;
   }

   /* falscher Parameter-Block, neu allokieren und nochmal probieren */
   ulSize = pFuncTable->QueryParamBlockSize();
   if (ulSize < MIN_PARAM_SIZE)
   {
      /* interner Fehler der DLL */
      MessageBox(hwndDlg, IDST_MSG_EM_DLLINT, 0, IDD_EM_DLLINT, MB_ERROR | MB_OK);
      return;
   }
   free(EchoMgrOpt.pDllParams);
   EchoMgrOpt.pDllParams = calloc(1, ulSize);
   EchoMgrOpt.ulParamLen = ulSize;
   EchoMgrOpt.bDirty = TRUE;

   /* Setup aufrufen */
   switch(pFuncTable->SetupParams(EchoMgrOpt.pDllParams, EchoMgrOpt.ulParamLen, hwndDlg, anchor, pFuncTable->hModule))
   {
      case ECHOMAN_OK:
         EchoMgrOpt.bDirty = TRUE;
         return;

      case ECHOMAN_CANCEL:
         return;

      default:
         /* interner Fehler der DLL */
         MessageBox(hwndDlg, IDST_MSG_EM_DLLINT, 0, IDD_EM_DLLINT, MB_ERROR | MB_OK);
         return;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ExtractAreasFromMessage                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt eine Area-Liste aus dem Messagetext                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchMessageText: Text mit Area-Liste                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: Zeiger auf den Area-String                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Area-String hat die Form "AREA1 AREA2 AREA3\0"                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static char *ExtractAreasFromMessage(char *pchMessageText, char *pchOldAreas)
{
   PSTRINGLIST pList=NULL, pLast=NULL;
   char *pchLine;
   char pchAreaTag[LEN_AREATAG+1];
   int iCopied=0;
   int i;
   ULONG ulReqLen=0;
   char *pchAreas=NULL;

   if (!pchMessageText)
      return NULL;

   pchLine = pchMessageText;

   while (*pchLine)
   {
      /* Zeilenenden Åbergehen */
      while (*pchLine == '\n')
         pchLine++;

      /* Whitespace Åbergehen */
      while (*pchLine == ' ' || *pchLine == '*' || *pchLine == '+')
         pchLine++;

      /* erstes Wort kopieren */
      iCopied=0;
      while (iCopied < LEN_AREATAG && *pchLine != '\n' && *pchLine != ' ')
         pchAreaTag[iCopied++]= *pchLine++;
      pchAreaTag[iCopied]=0;

      /* Ist das ein Area-Tag? */
      if (pchAreaTag[0])
      {
         i=0;
         while (pchAreaTag[i])
            if (isalnum(pchAreaTag[i]) ||
                pchAreaTag[i] == '.' ||
                pchAreaTag[i] == '-' ||
                pchAreaTag[i] == '_')
               i++;
            else
               break;
         if (!pchAreaTag[i] && strlen(pchAreaTag)>=3 )
         {
            strupr(pchAreaTag);
#if 0
            i=0;
            while (pchAreaTag[i])
            {
               if (isalpha(pchAreaTag[i]))
               {
#endif
                  if (!HaveArea(pList, pchAreaTag) &&
                      !AreaInAreaSet(pchOldAreas, pchAreaTag) )
                  {
                     /* Area-Tag gefunden */
                     if (pList)
                     {
                        pLast->next = malloc(sizeof(STRINGLIST));
                        pLast = pLast->next;
                     }
                     else
                        pList = pLast = malloc(sizeof(STRINGLIST));

                     pLast->next = NULL;
                     pLast->pchString = strdup(pchAreaTag);
                     ulReqLen += strlen(pchAreaTag)+1;
                  }

#if 0
                  break;
               }
               i++;
            }
#endif
         }
      }

      /* Zeilenende suchen */
      while (*pchLine && *pchLine != '\n')
         pchLine++;
   }

   if (ulReqLen)
   {
      pchAreas = malloc(ulReqLen);
      pchAreas[0]=0;

      while (pList)
      {
         if (pchAreas[0])
            strcat(pchAreas, " ");
         strcat(pchAreas, pList->pchString);

         free(pList->pchString);
         pLast = pList;
         pList = pList->next;
         free(pLast);
      }
      return pchAreas;
   }
   else
      return NULL;
}

static BOOL HaveArea(PSTRINGLIST pList, char *pchTag)
{
   while (pList)
      if (!stricmp(pList->pchString, pchTag))
         return TRUE;
      else
         pList = pList->next;

   return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateMessageText                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt Text mit Anweisungen aus den Echomanager-Kommandos  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            pEchoMgrData: Instanzdaten                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: Zeiger auf neuen Messagetext                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static char *CreateMessageText(HWND hwndCnr, PECHOMGRDATA pEchoMgrData)
{
   char *pchText=NULL, *pchTemp=NULL;
   char pchLine[100];
   ULONG ulAlloc=0;
   PECHOMGRRECORD pRecord=NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
   {
      if (pRecord->ulAction)
      {
         switch(pRecord->ulAction)
         {
            case ACTION_LINK:
               sprintf(pchLine, CMD_LINK, pRecord->pchEchoName);
               break;

            case ACTION_UNLINK:
               sprintf(pchLine, CMD_UNLINK, pRecord->pchEchoName);
               break;

            case ACTION_RESCAN:
               sprintf(pchLine, CMD_RESCAN, pRecord->pchEchoName);
               break;

            case ACTION_LNKRESC:
               sprintf(pchLine, CMD_LNKRESC, pRecord->pchEchoName, pRecord->pchEchoName);
               break;

            default:
               pchLine[0]=0;
               break;
         }
         if (!pchText)
         {
            pchText = malloc(TEXTBLOCKSIZE);
            strcpy(pchText, pchLine);
            ulAlloc = TEXTBLOCKSIZE;
         }
         else
         {
            if (strlen(pchText)+strlen(pchLine) > ulAlloc)
            {
               pchTemp = pchText;
               ulAlloc += TEXTBLOCKSIZE;
               pchText = malloc(ulAlloc);
               strcpy(pchText, pchTemp);
               free(pchTemp);
            }
            strcat(pchText, pchLine);
         }
      }
   }

   if (pEchoMgrData->ulAction)
   {
      switch(pEchoMgrData->ulAction)
      {
         case ACTION_REFRESH:
            strcpy(pchLine, CMD_REFRESH);
            break;

         case ACTION_PAUSE:
            strcpy(pchLine, CMD_PAUSE);
            break;

         case ACTION_RESUME:
            strcpy(pchLine, CMD_RESUME);
            break;

         default:
            pchLine[0]=0;
            break;
      }

      if (!pchText)
      {
         pchText = malloc(TEXTBLOCKSIZE);
         strcpy(pchText, pchLine);
         ulAlloc = TEXTBLOCKSIZE;
      }
      else
      {
         if (strlen(pchText)+strlen(pchLine) > ulAlloc)
         {
            pchTemp = pchText;
            ulAlloc += TEXTBLOCKSIZE;
            pchText = malloc(ulAlloc);
            strcpy(pchText, pchTemp);
            free(pchTemp);
         }
         strcat(pchText, pchLine);
      }
   }

   return pchText;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateActionList                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt eine Liste von Aktionen, die durch die Erweiterungs-*/
/*               DLL verarbeitet werden sollen                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: Zeiger auf Kopf der Liste                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PACTIONLIST CreateActionList(HWND hwndCnr)
{
   PECHOMGRRECORD pRecord=NULL;
   PACTIONLIST pList=NULL, pLast=NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord?CMA_NEXT:CMA_FIRST, CMA_ITEMORDER)))
   {
      if (pRecord->ulAction == ACTION_LINK ||
          pRecord->ulAction == ACTION_LNKRESC ||
          pRecord->ulAction == ACTION_UNLINK)
      {
         if (!pList)
            pList = pLast = malloc(sizeof(ACTIONLIST));
         else
         {
            pLast->next = malloc(sizeof(ACTIONLIST));
            pLast = pLast->next;
         }

         pLast->next = NULL;
         pLast->ulAction = pRecord->ulAction;
         strcpy(pLast->pchAreaTag, pRecord->pchEchoName);
      }
   }
   return pList;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateMessage                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt eine Message an den Uplink                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            pEchoMgrData: Instanzdaten                                     */
/*            pMessage: Erzeugte Message                                     */
/*            pHeader: Erzeugter Header                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte:  0   OK                                                    */
/*                 1   keine Aktion                                          */
/*                 2   Passwort o. Name fehlt                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int CreateMessage(HWND hwndCnr, PECHOMGRDATA pEchoMgrData, PFTNMESSAGE pMessage, PMSGHEADER pHeader)
{
   extern ECHOMGROPT EchoMgrOpt;
   extern char CurrentName[LEN_USERNAME+1];
   extern char CurrentArea[LEN_AREATAG+1];
   extern AREALIST arealiste;
   char *pchText;
   LONG lDummy;

   pchText = CreateMessageText(hwndCnr, pEchoMgrData);

   if (pchText)
   {
      if (pEchoMgrData->pUplink->pchEchoMgrName[0] &&
          pEchoMgrData->pUplink->pchEchoMgrAddress[0] &&
          pEchoMgrData->pUplink->pchPassword[0])
      {
         /* Header vorbereiten */
         MSG_NewMessage(pMessage, pHeader, &arealiste, CurrentArea, CurrentName, pEchoMgrData->pUplink->pchMyAddress, &lDummy);
         if (pMessage->pchMessageText)
            free(pMessage->pchMessageText);
         pMessage->pchMessageText = pchText;
         strcpy(pHeader->pchToName, pEchoMgrData->pUplink->pchEchoMgrName);
         strcpy(pHeader->pchSubject, pEchoMgrData->pUplink->pchPassword);
         StringToNetAddr(pEchoMgrData->pUplink->pchEchoMgrAddress, &pHeader->ToAddress, NULL);
         pHeader->ulAttrib |= ATTRIB_KILLSENT | ATTRIB_PRIVATE;

         return 0;
      }
      else
      {
         free(pchText);
         return 2;
      }
   }
   else
      return 1;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MessageSaveThread                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sichert die Uplink-Message in der Messagebase               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Datenblock (SAVEPARAM)                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void _Optlink MessageSaveThread(PVOID pParam)
{
   extern USERDATAOPT userdaten;
   extern GENERALOPT generaloptions;
   extern AREALIST arealiste;
   extern DRIVEREMAP driveremap;
   extern TEMPLATELIST templatelist;
   extern char CurrentArea[LEN_AREATAG+1];
   extern BOOL MailEntered[3];
   AREADEFLIST *pAreaDef;
   HAB hab;
   HMQ hmq;

   PSAVEPARAM pSaveParam = (PSAVEPARAM) pParam;
   ULONG ulRet=0;

   INSTALLEXPT("MsgSave");

   hab = WinInitialize(0);
   hmq=WinCreateMsgQueue(hab, 0);
   WinCancelShutdown(hmq, TRUE);

   if (pAreaDef = AM_FindArea(&arealiste, CurrentArea))
   {
      ulRet = MSG_AddMessage(&pSaveParam->Message, &pSaveParam->Header,
                             &arealiste, CurrentArea, &userdaten, &generaloptions,
                             &driveremap, generaloptions.lMaxMsgLen * 1024,
                             &templatelist, 0, SendAddMessage);
      MailEntered[pAreaDef->areadata.areatype]=TRUE;
      pAreaDef->mailentered = TRUE;
   }

   MSG_ClearMessage(&pSaveParam->Header, &pSaveParam->Message);

   WinPostMsg(pSaveParam->hwndDlg, WM_SAVEDONE, MPFROMLONG(ulRet), pSaveParam);

   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CfgChangeThread                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ruft die Erweiterungs-DLL auf, um die Config-Dateien zu     */
/*               aendern                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParam: Thread-Parameter (CHANGEPARAM)                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void _Optlink CfgChangeThread(PVOID pParam)
{
   extern ECHOMGROPT EchoMgrOpt;
   extern PATHNAMES pathnames;
   extern char CurrentAddress[LEN_5DADDRESS+1];
   PACTIONLIST pActionList = ((PCHANGEPARAM) pParam)->pActionList;
   PUPLINK pUplink = ((PCHANGEPARAM) pParam)->pUplink;
   PACTIONLIST pTemp;
   ULONG ulRet=0;
   FUNCTABLE FuncTable;

   INSTALLEXPT("CfgChange");

   switch(ulRet = LoadExtensionDLL(EchoMgrOpt.pchDllName, &FuncTable))
   {
      case LOADDLL_OK:
         while(pActionList)
         {
            if (pActionList->ulAction == ACTION_LINK ||
                pActionList->ulAction == ACTION_LNKRESC)
               ulRet = FuncTable.AddEcho(EchoMgrOpt.pDllParams, EchoMgrOpt.ulParamLen,
                                         pathnames.squishcfg, pUplink->pchMyAddress,
                                         pUplink->pchEchoMgrAddress,
                                         pActionList->pchAreaTag, 0);
            else
               ulRet = FuncTable.RemoveEcho(EchoMgrOpt.pDllParams, EchoMgrOpt.ulParamLen,
                                            pathnames.squishcfg, pUplink->pchMyAddress,
                                            pUplink->pchEchoMgrAddress,
                                            pActionList->pchAreaTag, 0);

            if (ulRet)
               break;
            else
            {
               pTemp = pActionList;
               pActionList = pActionList->next;
               free(pTemp);
            }
         }
         /* restliche Aktionen loeschen */
         while(pActionList)
         {
            pTemp = pActionList;
            pActionList = pActionList->next;
            free(pTemp);
         }
         DosFreeModule(FuncTable.hModule);
         break;

      default:
         WinPostMsg(((PCHANGEPARAM) pParam)->hwndDlg, WM_CHANGEDONE, MPFROMLONG(ulRet), NULL);
         free(pParam);
         DEINSTALLEXPT;
         return;
   }

   WinPostMsg(((PCHANGEPARAM) pParam)->hwndDlg, WM_CHANGEDONE, NULL, MPFROMLONG(ulRet));
   free(pParam);

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: LoadExtensionDLL                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Laedt die Erweiterungs-DLL, holt alle Funktionspointer      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchDllName: Dateiname der DLL                                  */
/*            pFuncTable: Funktionspointer-Tabelle                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: LOADDLL_OK:   DLL geladen                                  */
/*                LOADDLL_CANTLOAD: Kann DLL nicht laden                     */
/*                LOADDLL_FUNCMISSING: Funktion in der DLL fehlt             */
/*                LOADDLL_VERSION: Version der DLL stimmt nicht              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int LoadExtensionDLL(char *pchDllName, PFUNCTABLE pFuncTable)
{
   HMODULE hmod;
   char pchError[20];
   int iRet = LOADDLL_OK;

   if (DosLoadModule(pchError, sizeof(pchError), pchDllName, &hmod))
      return LOADDLL_CANTLOAD;

   pFuncTable->hModule = hmod;

   if (DosQueryProcAddr(hmod, 0, "QueryVersion", (PFN*) &pFuncTable->QueryVersion))
      iRet = LOADDLL_FUNCMISSING;
   else
      if (pFuncTable->QueryVersion() != DLL_VERSION)
         iRet = LOADDLL_VERSION;
      else
         if (DosQueryProcAddr(hmod, 0, "QueryParamBlockSize", (PFN*) &pFuncTable->QueryParamBlockSize))
            iRet = LOADDLL_FUNCMISSING;
         else
            if (DosQueryProcAddr(hmod, 0, "SetupParams", (PFN*) &pFuncTable->SetupParams))
               iRet = LOADDLL_FUNCMISSING;
            else
               if (DosQueryProcAddr(hmod, 0, "AddEcho", (PFN*) &pFuncTable->AddEcho))
                  iRet = LOADDLL_FUNCMISSING;
               else
                  if (DosQueryProcAddr(hmod, 0, "RemoveEcho", (PFN*) &pFuncTable->RemoveEcho))
                     iRet = LOADDLL_FUNCMISSING;

   if (iRet)
      DosFreeModule(hmod);

   return iRet;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindUplink                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht in der Liste der Uplinks den Eintrag mit der          */
/*               angegebenen Uplink-Adresse                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pEchoMgrOpt: Uplink-Liste                                      */
/*            pchUplinkAddress: FTN-Adresse des ges. Uplinks                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: NULL    nicht gefunden                                     */
/*                sonst   Pointer auf Uplink-Eintrag                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PUPLINK FindUplink(PECHOMGROPT pEchoMgrOpt, char *pchUplinkAddress)
{
   PUPLINK pTemp = pEchoMgrOpt->pUplinks;

   while (pTemp && stricmp(pTemp->pchEchoMgrAddress, pchUplinkAddress))
      pTemp = pTemp->next;

   return pTemp;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FindMatchingUplink                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht in der Liste der Uplinks den Eintrag, der zu der      */
/*               angegebenen aktuellen Adresse passt.                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pEchoMgrOpt: Uplink-Liste                                      */
/*            pchMyAddress: aktuelle Adresse                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: NULL    nicht gefunden                                     */
/*                sonst   Pointer auf Uplink-Eintrag                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PUPLINK FindMatchingUplink(PECHOMGROPT pEchoMgrOpt, char *pchMyAddress)
{
   PUPLINK pTemp = pEchoMgrOpt->pUplinks;

   while (pTemp && stricmp(pTemp->pchMyAddress, pchMyAddress))
      pTemp = pTemp->next;

   return pTemp;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ExtractUplinkFromMessage                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt einen neuen Uplink aus der aktuellen Message        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pHeader: Message-Header                                        */
/*            pMessage: aktuelle Message                                     */
/*            pEchoMgrOpt: Uplink-Liste                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 1   OK                                                     */
/*                0   keine Areas in der Message                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ExtractUplinkFromMessage(HWND hwndClient, PMSGHEADER pHeader, PFTNMESSAGE pMessage, PECHOMGROPT pEchoMgrOpt)
{
   PUPLINK pNewUplink;
   char pchAddress[LEN_5DADDRESS];
   char *pchAreas=NULL;
   char *pchSave=NULL;
   ULONG ulResponse;

   /* Suchen, ob schon vorhanden */
   NetAddrToString(pchAddress, &pHeader->FromAddress);
   pNewUplink = FindUplink(pEchoMgrOpt, pchAddress);

   if (pNewUplink)
      /* Schon vorhanden, Abfrage ob anhaengen */
      switch(ulResponse = MessageBox(hwndClient, IDST_MSG_APPENDAREAS, IDST_TITLE_DOEXTRACT,
                                     IDD_EM_APPENDAREAS, MB_YESNOCANCEL | MB_QUERY))
      {
         case MBID_YES:
            break;

         case MBID_NO:
            /* Areas ersetzen */
            if (pNewUplink->pchUplinkAreas)
               free(pNewUplink->pchUplinkAreas);
            pNewUplink->pchUplinkAreas = NULL;
            break;

         case MBID_CANCEL:
            return 1;
      }

   /* Text extrahieren */
   pchAreas = ExtractAreasFromMessage(pMessage->pchMessageText,
                                      pNewUplink?pNewUplink->pchUplinkAreas:NULL);

   if (!pchAreas)
      return 0;

   if (!pNewUplink)
   {
      /* Neuen Uplink */
      pNewUplink = calloc(1, sizeof(UPLINK));
      if (pEchoMgrOpt->pUplinks)
         pEchoMgrOpt->pUplinksLast->next = pNewUplink;
      else
         pEchoMgrOpt->pUplinks = pNewUplink;
      pNewUplink->prev = pEchoMgrOpt->pUplinksLast;
      pEchoMgrOpt->pUplinksLast = pNewUplink;

      /* Default-Daten */
      memcpy(pNewUplink->pchEchoMgrName, pHeader->pchFromName, LEN_USERNAME);
      memcpy(pNewUplink->pchEchoMgrAddress, pchAddress, LEN_5DADDRESS);
      NetAddrToString(pNewUplink->pchMyAddress, &pHeader->ToAddress);
      pNewUplink->pchUplinkAreas = pchAreas;
   }
   else
   {
      switch(ulResponse) /* von Abfrage Anhaengen */
      {
         case MBID_YES:
            pchSave = pNewUplink->pchUplinkAreas;
            if (pchSave)
            {
               pNewUplink->pchUplinkAreas = malloc(strlen(pchSave) + strlen(pchAreas) + 1);
               strcpy(pNewUplink->pchUplinkAreas, pchSave);
               strcat(pNewUplink->pchUplinkAreas, " ");
               strcat(pNewUplink->pchUplinkAreas, pchAreas);
               free(pchAreas);
            }
            else
               pNewUplink->pchUplinkAreas = pchAreas;
            break;

         case MBID_NO:
            /* Areas ersetzen */
            pNewUplink->pchUplinkAreas = pchAreas;
            NetAddrToString(pNewUplink->pchMyAddress, &pHeader->ToAddress);
            break;
      }
   }

   pEchoMgrOpt->bDirty = TRUE;

   return 1;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: RemoveEchoFromUplink
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Entfernt ein Echo aus der Echoliste eines Uplinks
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: hwndDlg: Dialog-Window
 |            pEchoMgrData: Instanzdaten
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: 0  OK
 |                1  Fehler (Echo nicht gefunden)
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: Das Echo wird durch pPopupRecord in den Instanzdaten bestimmt.
 |
 +---------------------------------------------------------------------------*/

static int RemoveEchoFromUplink(HWND hwndDlg, PECHOMGRDATA pEchoMgrData)
{
   if (pEchoMgrData->pUplink)
   {
      PCHAR pchEchos, pchNext;

      /* Popup-Echo ermitteln */
      pchEchos = pEchoMgrData->pUplink->pchUplinkAreas;

      while (pchEchos)
      {
         pchEchos = strstr(pchEchos, pEchoMgrData->pPopupRecord->pchEchoName);
         if (pchEchos)
         {
            int len = strlen(pEchoMgrData->pPopupRecord->pchEchoName);

            if (*(pchEchos+len) == ' ' || /* danach ein Trenner */
                *(pchEchos+len) == 0)     /* oder Ende */
            {
               /* gefunden */
               break;
            }
            else
               pchEchos += len;
         }
      }

      if (pchEchos)
      {
         /* Anfang des naechsten Echos suchen */
         pchNext = pchEchos;
         while (*pchNext && *pchNext != ' ')
            pchNext++;
         if (*pchNext)
         {
            pchNext++;
            if (pchEchos == pEchoMgrData->pUplink->pchUplinkAreas)
            {
               /* Erstes Echo */
               pchEchos = pEchoMgrData->pUplink->pchUplinkAreas;
               while (*pchNext)
                  *pchEchos++ = *pchNext++;
               *pchEchos = 0;
            }
            else
            {
               /* nach vorne kopieren */
               while (*pchNext)
                  *pchEchos++ = *pchNext++;
               *pchEchos = *pchNext;
            }
         }
         else
         {
            /* letztes Echo, abschneiden */
            if (pchEchos > pEchoMgrData->pUplink->pchUplinkAreas)
               pchEchos--;
            *pchEchos = 0;
         }

         /* Flags setzen */
         EchoMgrOpt.bDirty = TRUE;

         /* Record aus Container entfernen */
         free(pEchoMgrData->pPopupRecord->pchEchoName);
         WinSendDlgItemMsg(hwndDlg, IDD_ECHOMANAGER+1, CM_REMOVERECORD,
                           &pEchoMgrData->pPopupRecord,
                           MPFROM2SHORT(1, CMA_FREE | CMA_INVALIDATE));

         return 0;
      }

      /* nicht gefunden */
      return 1;
   }
   else
      /* kein Uplink !? */
      return 1;
}

/*-------------------------------- Modulende --------------------------------*/


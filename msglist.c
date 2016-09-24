/*---------------------------------------------------------------------------+
 | Titel:  MSGLIST.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 27.07.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Messageliste von Fleet Street                                         |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_GPILOGCOLORTABLE
#define INCL_BASE
#define INCL_PM
#define INCL_SPLDOSPRINT
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
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "msglist.h"
#include "utility.h"
#include "areadlg.h"
#include "savemsg.h"
#include "controls\mlist.h"
#include "controls\clrsel.h"
#include "printsetup.h"
#include "printmsg\printmsg.h"
#include "dump\expt.h"

/*--------------------------------- Defines ---------------------------------*/


typedef struct _MSGLISTDATA {
             ULONG         popupRecord;
             HWND          hwndpopup;
             HWND          hwndpopupsmall;
             HPOINTER      icon;
             char          pchCurrentArea[LEN_AREATAG+1];
             HSWITCH       hSwitch;       /* Switch-Entry-Handle        */
             ULONG         ulEnableCount; /* Enable-Counter             */
             BOOL          bKeyboard;     /* Kontextmenue per Tastatur? */
             BOOL          bNotifications;
             BOOL          bForeground;
          } MSGLISTDATA, *PMSGLISTDATA;

#define TAB_FONT    "8.Helv"
#define RGB_GREY    0x00cccccc

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HMODULE hmodLang;
extern char CurrentArea[LEN_AREATAG+1];
extern AREALIST arealiste;
extern BOOL MailEntered[3];
extern HWND client;
extern BOOL bDoingWork;
extern BOOL bStopWork;
extern MSGLISTOPTIONS msglistoptions;
extern DRIVEREMAP driveremap;

/*--------------------------- Funktionsprototypen ---------------------------*/
static void CleanupMsgList(HWND hwndContainer);
static int InsertHeaders(HWND hwndContainer);
static BOOL LoadItem(HWND hwnd, PMLISTRECORD pRecord);
static PULONG CollectMsgIDs(HWND hwndCnr, ULONG ulSelectedID, PULONG pulRecordsCollected);
static void ListAddMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader);
static void ListDeleteMessage(HWND hwndCnr, PMESSAGEID pMsgID);
static void ListMarkReadMessage(HWND hwndCnr, PMESSAGEID pMsgID);
static void ListMarkAllRead(HWND hwndCnr, PMESSAGEID pMsgID);
static void ListChangeMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader);
static MRESULT EXPENTRY MListSettingsProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);
static void InsertMListSettingsPages(HWND notebook);
static MRESULT EXPENTRY MListSettColProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);

/*---------------------------------------------------------------------------*/
/* Funktionsname: MsgListProc                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Dialog-Prozedur der Messageliste                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: (Window-Procedure )                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY MsgListProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp, frame;
   extern GENERALOPT generaloptions;
   extern PATHNAMES pathnames;
   extern int tidWorker;
   MRESULT resultbuf=0;
   SWP swp;
   PMSGLISTDATA pMsgListData;
   int iCurrent;
   MLISTCOLUMNS MCol;
   MLISTCOLORS  MColors;

   pMsgListData=(PMSGLISTDATA) WinQueryWindowULong(parent, QWL_USER);

   switch(message)
   {
      case WM_INITDLG:
         pMsgListData=malloc(sizeof(MSGLISTDATA));
         memset(pMsgListData, 0, sizeof(MSGLISTDATA));
         WinSetWindowULong(parent, QWL_USER, (ULONG) pMsgListData);

         pMsgListData->hSwitch=AddToWindowList(parent);

         pMsgListData->hwndpopup=WinLoadMenu(HWND_DESKTOP,
                                             hmodLang, IDM_MSGLISTPOPUP);
         pMsgListData->hwndpopupsmall=WinLoadMenu(WinWindowFromID(parent, IDD_MSGLIST+1),
                                             hmodLang, IDM_SMLISTPOPUP);
         if (pMsgListData->hwndpopupsmall)
            ReplaceSysMenu(parent, pMsgListData->hwndpopupsmall, 1);

         if (msglistoptions.ulFlags & SCRIPTS_FOREGROUND)
         {
            pMsgListData->bForeground = TRUE;
            WinCheckMenuItem(pMsgListData->hwndpopupsmall, IDM_SMP_FGROUND, TRUE);
            WinSetOwner(parent, client);
         }
         else
         {
            pMsgListData->bForeground = FALSE;
            WinCheckMenuItem(pMsgListData->hwndpopupsmall, IDM_SMP_FGROUND, FALSE);
            WinSetOwner(parent, HWND_DESKTOP);
         }

         RestoreWinPos(parent, &msglistoptions.ListPos, TRUE, FALSE);

         SetForeground(WinWindowFromID(parent,IDD_MSGLIST+1),
                       &msglistoptions.lForeClr);
         SetBackground(WinWindowFromID(parent,IDD_MSGLIST+1),
                       &msglistoptions.lBackClr);
         MColors.lUnreadClr = msglistoptions.lUnreadClr;
         MColors.lFromClr   = msglistoptions.lFromClr;
         MColors.lToClr     = msglistoptions.lToClr;
         WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_SETCOLORS, &MColors, NULL);

         SetFont(WinWindowFromID(parent,IDD_MSGLIST+1),
                 msglistoptions.mlistfont);

         MCol.ulNrPercent   = msglistoptions.ulNrPercent;
         MCol.ulFromPercent = msglistoptions.ulFromPercent;
         MCol.ulToPercent   = msglistoptions.ulToPercent;
         MCol.ulSubjPercent = msglistoptions.ulSubjPercent;
         MCol.ulStampWrittenPercent = msglistoptions.ulStampWrittenPercent;
         MCol.ulStampArrivedPercent = msglistoptions.ulStampArrivedPercent;

         WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_SETCOLUMNS,
                           &MCol, NULL);


         pMsgListData->icon=LoadIcon(IDB_MSGLIST);
         SendMsg(parent, WM_SETICON, (MPARAM) pMsgListData->icon, (MPARAM) 0);

         SendMsg(parent, TM_REREADTHREADS, NULL, NULL);
         SetInitialAccel(parent);
         WinShowWindow(parent, TRUE);
         pMsgListData->bNotifications = TRUE;
         break;

      case TM_REREADTHREADS:
         CleanupMsgList(WinWindowFromID(parent, IDD_MSGLIST+1));
         strcpy(pMsgListData->pchCurrentArea, CurrentArea);

         /* Header-Infos einfuegen */
         WinEnableWindowUpdate(WinWindowFromID(parent, IDD_MSGLIST+1), FALSE);
         iCurrent = InsertHeaders(WinWindowFromID(parent, IDD_MSGLIST+1));

         if (iCurrent)
         {
            /* Aktuelle Message anfahren */

            WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_SCROLLTO,
                              MPFROMLONG(iCurrent), NULL);
         }
         WinEnableWindowUpdate(WinWindowFromID(parent, IDD_MSGLIST+1), TRUE);
         break;

      case WM_QUERYTRACKINFO:
         WinQueryWindowPos(parent, &swp);
         if (swp.fl & SWP_MINIMIZE)
            break;

         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(parent,message,mp1,mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=290;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=160;
         return resultbuf;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
            WinShowWindow(WinWindowFromID(parent, IDD_MSGLIST+1), FALSE);
         if (((PSWP)mp1)->fl & (SWP_MAXIMIZE|SWP_RESTORE))
            WinShowWindow(WinWindowFromID(parent, IDD_MSGLIST+1), TRUE);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, parent, IDD_MSGLIST+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pMsgListData && pMsgListData->bNotifications)
         {
            extern DIRTYFLAGS dirtyflags;

            SaveWinPos(parent, (PSWP) mp1, &msglistoptions.ListPos, &dirtyflags.mlsettingsdirty);
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, parent);
         else
            WinAssociateHelpInstance(hwndhelp, frame);
         break;

      case WM_DESTROY:
         /* Fenster aufraeumen */
         RemoveFromWindowList(pMsgListData->hSwitch);
         WinDestroyPointer(pMsgListData->icon);
         WinDestroyWindow(pMsgListData->hwndpopup);
         WinDestroyWindow(pMsgListData->hwndpopupsmall);
         CleanupMsgList(WinWindowFromID(parent, IDD_MSGLIST+1));

         if (pMsgListData->bForeground)
         {
            if (!(msglistoptions.ulFlags & MLISTFLAG_FOREGROUND))
            {
               extern DIRTYFLAGS dirtyflags;

               msglistoptions.ulFlags |= MLISTFLAG_FOREGROUND;
               dirtyflags.mlsettingsdirty = TRUE;
            }
         }
         else
         {
            if (msglistoptions.ulFlags & MLISTFLAG_FOREGROUND)
            {
               extern DIRTYFLAGS dirtyflags;

               msglistoptions.ulFlags &= ~MLISTFLAG_FOREGROUND;
               dirtyflags.mlsettingsdirty = TRUE;
            }
         }
         free(pMsgListData);
         break;

      case WM_CLOSE:
         WinPostMsg(client, MSGLM_CLOSE, NULL, NULL);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_MENU)
         {
            if (bDoingWork || pMsgListData->ulEnableCount)
            {
               WinAlarm(HWND_DESKTOP, WA_WARNING);
               return (MRESULT) FALSE;
            }
            if (SHORT1FROMMP(mp1)==IDM_MP_DELETE)
            {
               PWORKDATA pWorkData;

               if (generaloptions.safety & SAFETY_DELMSG)
               {
                  if (MessageBox(parent, IDST_MSG_DELETE, IDST_TITLE_DELETE,
                                 IDD_DELETE, MB_YESNO | MB_ICONEXCLAMATION)!=MBID_YES)
                  {
                     if (SHORT2FROMMP(mp2)==FALSE)
                        SendMsg(parent, WM_MENUEND, NULL,
                                   (MPARAM) pMsgListData->hwndpopup);
                     return (MRESULT) FALSE;
                  }
               }
               pWorkData=malloc(sizeof(WORKDATA));
               pWorkData->next=NULL;
               pWorkData->MsgIDArray=CollectMsgIDs(WinWindowFromID(parent, IDD_MSGLIST+1),
                                                   pMsgListData->popupRecord, &pWorkData->ulArraySize);
               pWorkData->flWorkToDo=WORK_DELETE;
               pWorkData->pPrintDest = NULL;
               strcpy(pWorkData->pchDestArea, pMsgListData->pchCurrentArea);
               bDoingWork=TRUE;
               tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
            }
            if (SHORT1FROMMP(mp1)==IDM_MP_EXPORT)
            {
               /* Messages exportieren */
               PWORKDATA pWorkData;
               extern ULONG ulExportOptions;

               pWorkData=malloc(sizeof(WORKDATA));
               pWorkData->next=NULL;
               strcpy(pWorkData->pchDestFile, pathnames.lastexport);

               /* Filenamen holen */
               if (GetExportName(parent, pWorkData->pchDestFile, &ulExportOptions))
               {
                  strcpy(pathnames.lastexport, pWorkData->pchDestFile);

                  pWorkData->MsgIDArray=CollectMsgIDs(WinWindowFromID(parent, IDD_MSGLIST+1),
                                pMsgListData->popupRecord, &pWorkData->ulArraySize);
                  pWorkData->flWorkToDo=WORK_EXPORT;
                  pWorkData->pPrintDest = NULL;
                  pWorkData->ulExportOptions = ulExportOptions;
                  strcpy(pWorkData->pchSrcArea, pMsgListData->pchCurrentArea);
                  bDoingWork=TRUE;
                  tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
               }
               else
               {
                  free(pWorkData);
                  return (MRESULT) FALSE;
               }
            }
            if (SHORT1FROMMP(mp1)==IDM_MP_PRINT)
            {
               PWORKDATA pWorkData;

               pWorkData=malloc(sizeof(WORKDATA));
               pWorkData->next=NULL;

               pWorkData->MsgIDArray=CollectMsgIDs(WinWindowFromID(parent, IDD_MSGLIST+1),
                             pMsgListData->popupRecord, &pWorkData->ulArraySize);
               pWorkData->flWorkToDo=WORK_PRINT;
               pWorkData->pPrintDest = NULL;
               strcpy(pWorkData->pchSrcArea, pMsgListData->pchCurrentArea);
               bDoingWork=TRUE;
               tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
            }
            if (SHORT1FROMMP(mp1)==IDM_MP_COPY)
            {
               PWORKDATA pWorkData;
               AREALISTPAR AreaListPar;

               /* ZielArea holen */
               AreaListPar.cb=sizeof(AREALISTPAR);
               if (generaloptions.LastCopyArea[0])
                  AreaListPar.pchString=strdup(generaloptions.LastCopyArea);
               else
                  AreaListPar.pchString=strdup(CurrentArea);
               AreaListPar.idTitle = IDST_TITLE_AL_COPY;
               AreaListPar.ulIncludeTypes = INCLUDE_ALL;
               AreaListPar.bExtendedSel = FALSE;
               AreaListPar.bChange      = FALSE;

               if (WinDlgBox(HWND_DESKTOP, parent,
                             AreaListProc, hmodLang,
                             IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
               {
                  AREADEFLIST *pArea;

                  pWorkData=malloc(sizeof(WORKDATA));
                  pWorkData->next=NULL;
                  pWorkData->ulCopyMove = 0;

                  pArea = AM_FindArea(&arealiste, AreaListPar.pchString);
                  if (pArea && pArea->areadata.areatype != AREATYPE_LOCAL)
                     switch(MessageBox(parent, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                                       IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                     {
                        case MBID_YES:
                           pWorkData->ulCopyMove = COPYMOVE_RESEND;
                           break;

                        default:
                           break;
                     }

                  pWorkData->MsgIDArray=CollectMsgIDs(WinWindowFromID(parent, IDD_MSGLIST+1),
                                pMsgListData->popupRecord, &pWorkData->ulArraySize);
                  pWorkData->flWorkToDo=WORK_COPY;
                  pWorkData->pPrintDest = NULL;
                  strcpy(pWorkData->pchSrcArea, pMsgListData->pchCurrentArea);
                  strcpy(pWorkData->pchDestArea, AreaListPar.pchString);
                  bDoingWork=TRUE;
                  tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                  free(AreaListPar.pchString);
               }
               else
                  return (MRESULT) FALSE;
            }
            if (SHORT1FROMMP(mp1)==IDM_MP_MOVE)
            {
               PWORKDATA pWorkData;
               AREALISTPAR AreaListPar;

               /* ZielArea holen */
               AreaListPar.cb=sizeof(AREALISTPAR);
               if (generaloptions.LastMoveArea[0])
                  AreaListPar.pchString=strdup(generaloptions.LastMoveArea);
               else
                  AreaListPar.pchString=strdup(CurrentArea);
               AreaListPar.idTitle = IDST_TITLE_AL_MOVE;
               AreaListPar.ulIncludeTypes = INCLUDE_ALL;
               AreaListPar.bExtendedSel = FALSE;
               AreaListPar.bChange      = FALSE;

               if (WinDlgBox(HWND_DESKTOP, parent,
                             AreaListProc, hmodLang,
                             IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
               {
                  AREADEFLIST *pArea;

                  pWorkData=malloc(sizeof(WORKDATA));
                  pWorkData->next=NULL;
                  pWorkData->ulCopyMove = 0;

                  pArea = AM_FindArea(&arealiste, AreaListPar.pchString);
                  if (pArea && pArea->areadata.areatype != AREATYPE_LOCAL)
                     switch(MessageBox(parent, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                                       IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                     {
                        case MBID_YES:
                           pWorkData->ulCopyMove = COPYMOVE_RESEND;
                           break;

                        default:
                           break;
                     }

                  pWorkData->MsgIDArray=CollectMsgIDs(WinWindowFromID(parent, IDD_MSGLIST+1),
                                pMsgListData->popupRecord, &pWorkData->ulArraySize);
                  pWorkData->flWorkToDo=WORK_MOVE;
                  pWorkData->pPrintDest = NULL;
                  strcpy(pWorkData->pchSrcArea, pMsgListData->pchCurrentArea);
                  strcpy(pWorkData->pchDestArea, AreaListPar.pchString);
                  bDoingWork=TRUE;
                  tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
                  free(AreaListPar.pchString);
               }
               else
                  return (MRESULT) FALSE;
            }
            if (SHORT1FROMMP(mp1)==IDM_MP_SELECTALL)
            {
               WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_SELECTALL, NULL, NULL);
            }
            if (SHORT1FROMMP(mp1)==IDM_MP_SELECTNONE)
            {
               WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_SELECTNONE, NULL, NULL);
            }
            if (SHORT1FROMMP(mp1)==IDM_SMP_SETTINGS)
            {
               extern DIRTYFLAGS dirtyflags;

               /* Settings-Notebook aufmachen */
               WinDlgBox(HWND_DESKTOP, parent, MListSettingsProc,
                         hmodLang, IDD_MLISTSETTINGS, NULL);
               if (dirtyflags.mlsettingsdirty)
               {
                  WinEnableWindowUpdate(WinWindowFromID(parent, IDD_AREALIST+1), FALSE);
                  pMsgListData->bNotifications=FALSE;
                  SetForeground(WinWindowFromID(parent,IDD_MSGLIST+1),
                                &msglistoptions.lForeClr);
                  SetBackground(WinWindowFromID(parent,IDD_MSGLIST+1),
                                &msglistoptions.lBackClr);
                  MColors.lUnreadClr = msglistoptions.lUnreadClr;
                  MColors.lFromClr   = msglistoptions.lFromClr;
                  MColors.lToClr     = msglistoptions.lToClr;
                  WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_SETCOLORS, &MColors,
                                    NULL);
                  pMsgListData->bNotifications=TRUE;

                  WinEnableWindowUpdate(WinWindowFromID(parent, IDD_AREALIST+1), TRUE);
               }
            }
            if (SHORT1FROMMP(mp1)==IDM_SMP_FGROUND)
            {
               if (pMsgListData->bForeground)
               {
                  pMsgListData->bForeground = FALSE;
                  WinCheckMenuItem(pMsgListData->hwndpopupsmall, IDM_SMP_FGROUND, FALSE);
                  WinSetOwner(parent, HWND_DESKTOP);
               }
               else
               {
                  pMsgListData->bForeground = TRUE;
                  WinCheckMenuItem(pMsgListData->hwndpopupsmall, IDM_SMP_FGROUND, TRUE);
                  WinSetOwner(parent, client);
               }
            }
            return (MRESULT) FALSE;
         }
         if (SHORT1FROMMP(mp2)==CMDSRC_ACCELERATOR)
         {
            switch(SHORT1FROMMP(mp1))
            {
               case IDA_DELMSG:
                  {
                     /* Delete-Key abfangen */
                     LONG lItem;
                     MLISTRECORD MRecord;

                     if (bDoingWork || pMsgListData->ulEnableCount)
                        return (MRESULT) FALSE;

                     lItem = (LONG) WinSendDlgItemMsg(parent, IDD_MSGLIST+1,
                                                      MLIM_QUERYCRSITEM, NULL, NULL);

                     if (lItem == MLIT_NONE)
                        return (MRESULT) FALSE;

                     WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_QUERYITEM,
                                       MPFROMLONG(lItem), &MRecord);

                     pMsgListData->popupRecord= MRecord.ulMsgID;

                     if (!pMsgListData->popupRecord)
                        return (MRESULT) FALSE;

                     WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_EMPHASIZEITEM,
                                       MPFROMLONG(lItem), NULL);

                     SendMsg(parent, WM_COMMAND, MPFROMSHORT(IDM_MP_DELETE),
                                MPFROM2SHORT(CMDSRC_MENU, FALSE));

                  }
                  return (MRESULT) TRUE;

               default:
                  return RedirectCommand(mp1, mp2);
            }
         }

         WinPostMsg(client, MSGLM_CLOSE, NULL, NULL);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_MSGLIST+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               PMLCONTEXT pContext;

               case MLIN_ENTER:
                  if (pMsgListData->ulEnableCount == 0)
                  {
                     MLISTRECORD MRecord;
                     if (WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_QUERYITEM,
                                       mp2, &MRecord))
                     {
                        SendMsg(client, TM_JUMPTOMESSAGE,
                                   MPFROMLONG(MRecord.ulMsgID), (MRESULT) TRUE);
                        SetFocusControl(client, IDML_MAINEDIT);
                     }
                  }
                  break;

               case MLIN_CONTEXTMENU:
                  if (bDoingWork || pMsgListData->ulEnableCount)
                  {
                     WinAlarm(HWND_DESKTOP, WA_WARNING);
                     break;
                  }
                  else
                  {
                     MLISTRECORD MRecord;

                     pContext = (PMLCONTEXT) mp2;

                     if (pContext->lItem == MLIT_NONE)
                     {
                        pMsgListData->popupRecord = 0;
                        WinPopupMenu(HWND_DESKTOP, parent, pMsgListData->hwndpopupsmall,
                                     pContext->xContext,
                                     pContext->yContext,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
                     }
                     else
                     {
                        WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_QUERYITEM,
                                          MPFROMLONG(pContext->lItem), &MRecord);
                        pMsgListData->popupRecord = MRecord.ulMsgID;

                        WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_EMPHASIZEITEM,
                                          MPFROMLONG(pContext->lItem), NULL);

                        WinPopupMenu(HWND_DESKTOP, parent, pMsgListData->hwndpopup,
                                     pContext->xContext,
                                     pContext->yContext,
                                     0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
                     }
                  }
                  break;

               case MLIN_LOADITEM:
                  return (MRESULT) LoadItem(parent, (PMLISTRECORD) mp2);

               case MLIN_PPARAMCHANGED:
                  if (pMsgListData && pMsgListData->bNotifications)
                  {
                     extern DIRTYFLAGS dirtyflags;

                     QueryForeground(WinWindowFromID(parent,IDD_MSGLIST+1),
                                     &msglistoptions.lForeClr);
                     QueryBackground(WinWindowFromID(parent,IDD_MSGLIST+1),
                                     &msglistoptions.lBackClr);
                     QueryFont(WinWindowFromID(parent,IDD_MSGLIST+1),
                               msglistoptions.mlistfont);

                     dirtyflags.mlsettingsdirty = TRUE;
                  }
                  break;

               case MLIN_SEPACHANGED:
                  {
                     MLISTCOLUMNS MCol;
                     extern DIRTYFLAGS dirtyflags;

                     WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_QUERYCOLUMNS,
                                       &MCol, NULL);

                     msglistoptions.ulNrPercent   = MCol.ulNrPercent;
                     msglistoptions.ulFromPercent = MCol.ulFromPercent;
                     msglistoptions.ulToPercent   = MCol.ulToPercent;
                     msglistoptions.ulSubjPercent = MCol.ulSubjPercent;
                     msglistoptions.ulStampWrittenPercent = MCol.ulStampWrittenPercent;
                     msglistoptions.ulStampArrivedPercent = MCol.ulStampArrivedPercent;

                     dirtyflags.mlsettingsdirty = TRUE;
                  }
                  break;

               default:
                  break;
            }
         }
         break;

      case WM_CONTEXTMENU:
         if (!SHORT1FROMMP(mp1) &&
             WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(parent, IDD_MSGLIST+1))
         {
            pMsgListData->bKeyboard=TRUE;
            WinSendDlgItemMsg(parent, IDD_MSGLIST+1, WM_CONTEXTMENU,
                              mp1, mp2);
         }
         break;

      case WM_MENUEND:
         if ((HWND) mp2 == pMsgListData->hwndpopup)
         {
            pMsgListData->bKeyboard=FALSE;
            WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_EMPHASIZEITEM,
                              MPFROMLONG(MLIT_NONE), NULL);
         }
         if ((HWND) mp2 == pMsgListData->hwndpopupsmall)
            ResetMenuStyle(pMsgListData->hwndpopupsmall, parent);
         break;

      case WORKM_DELETED:
         if (stricmp(pMsgListData->pchCurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            break;
         ListDeleteMessage(WinWindowFromID(parent, IDD_MSGLIST+1),
                           (PMESSAGEID) mp1);
         break;

      case WORKM_ADDED:
         if (stricmp(pMsgListData->pchCurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            break;
         ListAddMessage(WinWindowFromID(parent, IDD_MSGLIST+1),
                        (PMESSAGEID) mp1, (PMSGHEADER) mp2);
         break;

      case WORKM_END:
         break;

      case WORKM_DISABLEVIEWS:
         pMsgListData->ulEnableCount++;
         break;

      case WORKM_ENABLEVIEWS:
         if (pMsgListData->ulEnableCount)
            pMsgListData->ulEnableCount--;
         break;

      case WORKM_READ:
         if (stricmp(pMsgListData->pchCurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            break;
         ListMarkReadMessage(WinWindowFromID(parent, IDD_MSGLIST+1),
                             (PMESSAGEID) mp1);
         break;

      case WORKM_CHANGED:
         if (stricmp(pMsgListData->pchCurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            break;
         ListChangeMessage(WinWindowFromID(parent, IDD_MSGLIST+1),
                           (PMESSAGEID) mp1, (PMSGHEADER) mp2);
         break;

      case WORKM_MARKEND:
         if (stricmp(pMsgListData->pchCurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            break;
         ListMarkAllRead(WinWindowFromID(parent, IDD_MSGLIST+1), (PMESSAGEID) mp1);
         break;

      case WORKM_TRACKMSG:
         if (!stricmp(pMsgListData->pchCurrentArea, MESSAGEIDFROMP(mp1)->pchAreaTag))
         {
            LONG lTracked;

            lTracked = (LONG) WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_FINDUMSGID,
                                                MPFROMLONG(MESSAGEIDFROMP(mp1)->ulMsgID), NULL);

            if (lTracked != MLIT_NONE)
               WinSendDlgItemMsg(parent, IDD_MSGLIST+1, MLIM_SHIFTINTOVIEW,
                                 MPFROMLONG(lTracked), NULL);
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

/*---------------------------------------------------------------------------*/
/* Funktionsname: LoadItem                                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Laedt ein Item zur Anzeige in der Liste                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Window-Handle der Liste                                  */
/*            pRecord: Zeiger auf den Item-Record in der Liste               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE  Erfolg                                               */
/*                FALSE Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL LoadItem(HWND hwnd, PMLISTRECORD pRecord)
{
   extern USERDATAOPT userdaten;
   int msgnum, i;
   MSGHEADER Header;

   hwnd = hwnd;

   msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, pRecord->ulMsgID, TRUE);
   if (!msgnum ||
       MSG_ReadHeader(&Header, &arealiste, CurrentArea, msgnum))
   {
      return FALSE;
   }
   else
   {
      /* Strings kopieren */
      strncpy(pRecord->pchFrom, Header.pchFromName, LEN_USERNAME);
      pRecord->pchFrom[LEN_USERNAME]=0;
      strncpy(pRecord->pchTo, Header.pchToName, LEN_USERNAME);
      pRecord->pchTo[LEN_USERNAME]=0;
      strncpy(pRecord->pchSubject, Header.pchSubject, LEN_SUBJECT);
      pRecord->pchSubject[LEN_SUBJECT]=0;
      pRecord->StampWritten = Header.StampWritten;
      pRecord->StampArrived = Header.StampArrived;

      /* Read-Flag */
      if (Header.ulAttrib & ATTRIB_READ)
         pRecord->flRecFlags |= LISTFLAG_READ;
      else
         pRecord->flRecFlags &= ~LISTFLAG_READ;

      /* From pruefen */
      i=0;
      while (i<MAX_USERNAMES && stricmp(userdaten.username[i], Header.pchFromName))
         i++;
      if (i<MAX_USERNAMES)
         pRecord->flRecFlags |= LISTFLAG_FROMME;
      else
         pRecord->flRecFlags &= ~LISTFLAG_FROMME;

      /* To pruefen */
      i=0;
      while (i<MAX_USERNAMES && stricmp(userdaten.username[i], Header.pchToName))
         i++;
      if (i<MAX_USERNAMES)
         pRecord->flRecFlags |= LISTFLAG_TOME;
      else
         pRecord->flRecFlags &= ~LISTFLAG_TOME;

      return TRUE;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertHeaders                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt alle Header in die Liste ein                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Window-Handle der Liste                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Nummer der aktuellen Message                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int InsertHeaders(HWND hwndContainer)
{
   extern char CurrentArea[LEN_AREATAG+1];
   AREADEFLIST *zeiger;
   PMLISTRECORD pRecords=NULL;
   int i;

   zeiger=AM_FindArea(&arealiste, CurrentArea);

   if (!zeiger)
      return MLIT_NONE;

   if (zeiger->maxmessages==0)
      return MLIT_NONE;

   pRecords= malloc(zeiger->maxmessages * sizeof(MLISTRECORD));

   for (i=1; i<=zeiger->maxmessages; i++)
   {
      pRecords[i-1].ulMsgID=MSG_MsgnToUid(&arealiste, CurrentArea, i);
      pRecords[i-1].flRecFlags = 0;
   }

   SendMsg(hwndContainer, MLIM_ADDITEMARRAY, pRecords,
              MPFROMLONG(zeiger->maxmessages));

   free (pRecords);

   return zeiger->currentmessage - 1;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CleanupMsgList                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht alle Eintraege in der Liste                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Window-Handle der Liste                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void CleanupMsgList(HWND hwndContainer)
{
   SendMsg(hwndContainer, MLIM_CLEARLIST, NULL, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CollectMsgIDs                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht aus der Messageliste alle MSGIDs, fuer die eine       */
/*               Operation durchgefuehrt werden soll                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            ulSelectedID: Record, fuer den das Contextmenue                */
/*                          geoeffnet wurde                                  */
/*            pulRecordsCollected: Zeiger auf Anzahl der ausgesuchten Records*/
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Zeiger auf das erzeugte Array                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Das erzeugte Array muss ausserhalb freigegeben werden          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PULONG CollectMsgIDs(HWND hwndCnr, ULONG ulSelectedID, PULONG pulRecordsCollected)
{
   PULONG pSelArray=NULL;
   PULONG pSaveArray=NULL;
   MLISTRECORD MRecord;
   ULONG ulCountRecords=0;
   LONG lItem;

   lItem = (LONG) SendMsg(hwndCnr, MLIM_FINDUMSGID, MPFROMLONG(ulSelectedID),
                             NULL);

   if (lItem == MLIT_NONE)
   {
      *pulRecordsCollected = 0;
      return NULL;
   }

   SendMsg(hwndCnr, MLIM_QUERYITEM, MPFROMLONG(lItem), &MRecord);

   if (MRecord.flRecFlags & LISTFLAG_SELECTED)
   {
      /* Alle selected Records ausw�hlen */
      lItem = (LONG) SendMsg(hwndCnr, MLIM_QUERYFSELECT, NULL, NULL);

      while (lItem != MLIT_NONE)
      {
         if (SendMsg(hwndCnr, MLIM_QUERYITEM, MPFROMLONG(lItem), &MRecord))
         {
            /* MSGID in Array aufnehmen */
            ulCountRecords++;
            pSaveArray=pSelArray;
            pSelArray=realloc(pSelArray, sizeof(ULONG)* ulCountRecords);
            if (pSelArray)
               pSelArray[ulCountRecords-1]=MRecord.ulMsgID;
            else
            {
               /* realloc schiefgelaufen, neu anlegen */
               pSelArray=malloc(sizeof(ULONG)* ulCountRecords);
               if (pSaveArray)
               {
                  memcpy(pSelArray, pSaveArray, sizeof(ULONG)* (ulCountRecords-1));
                  free(pSaveArray);
               }
               pSelArray[ulCountRecords-1]=MRecord.ulMsgID;
            }
         }
         lItem = (LONG) SendMsg(hwndCnr, MLIM_QUERYNSELECT,
                                   MPFROMLONG(lItem), NULL);
      }
      *pulRecordsCollected=ulCountRecords;
   }
   else
   {
      /* nur aktuellen Record auswaehlen */
      pSelArray=malloc(sizeof(ULONG));
      pSelArray[0]=MRecord.ulMsgID;
      *pulRecordsCollected=1;
   }
   return pSelArray;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: WorkerThread                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Arbeitsthread fuer die Messageliste                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pThreadData: Zeiger auf WORKDATA-Struktur                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void _Optlink WorkerThread(PVOID pThreadData)
{
   PWORKDATA pWorkData=(PWORKDATA)pThreadData;
   PWORKDATA pWorkData2;
   ULONG ulMsgNum=0;
   ULONG ulProgress;     /* 0-100 */
   ULONG ulNewProgress;
   int i;
   BOOL bStop=FALSE;
   FTNMESSAGE Message;
   MESSAGEID SMessageID, DMessageID;
   MSGHEADER Header;
   HMQ hmq = NULLHANDLE;
   HAB hab;
   HDC hdc;
   HPS hps;
   AREADEFLIST *pAreaDef;
   ULONG ulExportOptions=0;

   extern MISCOPTIONS miscoptions;
   extern HWND client;
   extern DRIVEREMAP driveremap;
   extern USERDATAOPT userdaten;
   extern PRINTSETUP PrintSetup;
   extern GENERALOPT generaloptions;

   INSTALLEXPT("Worker");

   memset(&Message, 0, sizeof(Message));
   memset(&Header, 0, sizeof(Header));
   bStopWork = FALSE;

   hab = WinInitialize(0);
   hmq=WinCreateMsgQueue(hab, 0);
   WinCancelShutdown(hmq, TRUE);

   /* Export-Optionen der ersten Area ｜ernehmen */
   if (pWorkData)
      ulExportOptions = pWorkData->ulExportOptions;

   while (pWorkData)
   {
      /* Arbeit verrichten */
      switch(pWorkData->flWorkToDo)
      {
         case WORK_DELETE:

            MSG_OpenArea(&arealiste, pWorkData->pchDestArea, miscoptions.lastreadoffset, &driveremap);
            SendMsg(client, WORKM_STARTWORKAREA, pWorkData->pchDestArea, NULL);
            i=pWorkData->ulArraySize-1;
            bStop = FALSE;
            ulProgress=0;
            while(!bStop && i >=0 && !bStopWork)
            {
               ulNewProgress= (100*(pWorkData->ulArraySize-i)) / pWorkData->ulArraySize;
               if (ulNewProgress != ulProgress)
               {
                  ulProgress= ulNewProgress;
                  SendMsg(client, WORKM_PROGRESS, MPFROMLONG(ulProgress), NULL);
               }

               ulMsgNum=MSG_UidToMsgn(&arealiste, pWorkData->pchDestArea, pWorkData->MsgIDArray[i], TRUE);
               if (ulMsgNum && !MSG_KillMessage(&arealiste, pWorkData->pchDestArea, ulMsgNum, &driveremap, SendKillMessage))
               {
                  i--;
               }
               else
                  switch((ULONG) SendMsg(client, WORKM_ERROR, NULL, NULL))
                  {
                     case WORK_ERROR_IGNORE:  /* ignore error */

                        i--;                  /* next message */
                        break;

                     case WORK_ERROR_RETRY:   /* retry operation, don't inc index */

                        break;

                     case WORK_ERROR_ABORT:   /* abort, set Stop flag */

                        bStop=TRUE;
                        break;

                     default:

                        i--;
                        break;
                  }
            }
            MSG_CloseArea(&arealiste, pWorkData->pchDestArea, TRUE, miscoptions.lastreadoffset, &driveremap);
            break;

         case WORK_EXPORT:
            MSG_OpenArea(&arealiste, pWorkData->pchSrcArea, miscoptions.lastreadoffset, &driveremap);
            strcpy(SMessageID.pchAreaTag, pWorkData->pchSrcArea);
            SendMsg(client, WORKM_STARTWORKAREA, pWorkData->pchSrcArea, NULL);
            i=0;
            bStop = FALSE;
            ulProgress=0;
            while(!bStop && i < pWorkData->ulArraySize && !bStopWork)
            {
               ulNewProgress= (100*i) / pWorkData->ulArraySize;
               if (ulNewProgress != ulProgress)
               {
                  ulProgress= ulNewProgress;
                  SendMsg(client, WORKM_PROGRESS, MPFROMLONG(ulProgress), NULL);
               }

               ulMsgNum=MSG_UidToMsgn(&arealiste, pWorkData->pchSrcArea, pWorkData->MsgIDArray[i], TRUE);
               if (ulMsgNum && !MSG_ReadNum(&Message, &Header, &arealiste, pWorkData->pchSrcArea, ulMsgNum) &&
                   !WriteMessage(pWorkData->pchDestFile, &Message,
                                 &Header, pWorkData->pchSrcArea,
                                 ulExportOptions))
               {

                  SMessageID.ulMsgID = pWorkData->MsgIDArray[i];
                  SendMsg(client, WORKM_EXPORTED, &SMessageID, NULL);
                  i++;
                  ulExportOptions |= EXPORT_APPEND;
               }
               else
                  switch((ULONG)SendMsg(client, WORKM_ERROR, NULL, NULL))
                  {
                     case WORK_ERROR_IGNORE:  /* ignore error */

                        i++;                  /* next message */
                        ulExportOptions |= EXPORT_APPEND;
                        break;

                     case WORK_ERROR_RETRY:   /* retry operation, don't inc index */

                        break;

                     case WORK_ERROR_ABORT:   /* abort, set Stop flag */

                        bStop=TRUE;
                        break;

                     default:

                        i++;
                        ulExportOptions |= EXPORT_APPEND;
                        break;
                  }
            }
            MSG_CloseArea(&arealiste, pWorkData->pchSrcArea, TRUE, miscoptions.lastreadoffset, &driveremap);
            break;

         case WORK_PRINT:

            MSG_OpenArea(&arealiste, pWorkData->pchSrcArea, miscoptions.lastreadoffset, &driveremap);
            strcpy(SMessageID.pchAreaTag, pWorkData->pchSrcArea);
            SendMsg(client, WORKM_STARTWORKAREA, SMessageID.pchAreaTag, NULL);
            i=0;
            bStop = FALSE;
            ulProgress=0;

            if (pWorkData->pPrintDest)
               OpenPrinterDM(anchor, &hdc, &hps, pWorkData->pPrintDest);
            else
               OpenPrinter(&PrintSetup, &hdc, &hps);

            if (pWorkData->pPrintDest)
               free (pWorkData->pPrintDest);

            while(!bStop && i < pWorkData->ulArraySize && !bStopWork)
            {
               int rc;

               ulNewProgress= (100*i) / pWorkData->ulArraySize;
               if (ulNewProgress != ulProgress)
               {
                  ulProgress= ulNewProgress;
                  SendMsg(client, WORKM_PROGRESS, MPFROMLONG(ulProgress), NULL);
               }


               ulMsgNum=MSG_UidToMsgn(&arealiste, pWorkData->pchSrcArea, pWorkData->MsgIDArray[i], TRUE);
               if (ulMsgNum && !MSG_ReadNum(&Message, &Header, &arealiste, pWorkData->pchSrcArea, ulMsgNum))
               {
                  rc = PrintMessage(hdc, hps, &Header, &Message,
                                    pWorkData->pchSrcArea, ulMsgNum, &arealiste,
                                    hmodLang, &PrintSetup);

                  if (!rc)
                  {

                     SMessageID.ulMsgID = pWorkData->MsgIDArray[i];
                     SendMsg(client, WORKM_PRINTED, &SMessageID, NULL);
                     i++;
                  }
                  else
                     switch((ULONG)SendMsg(client, WORKM_ERROR, NULL, NULL))
                     {
                        case WORK_ERROR_IGNORE:  /* ignore error */

                           i++;                  /* next message */
                           break;

                        case WORK_ERROR_RETRY:   /* retry operation, don't inc index */

                           break;

                        case WORK_ERROR_ABORT:   /* abort, set Stop flag */

                           bStop=TRUE;
                           break;

                        default:

                           i++;
                           break;
                     }
               }
            }
            ClosePrinter(hdc, hps);

            MSG_CloseArea(&arealiste, pWorkData->pchSrcArea, TRUE, miscoptions.lastreadoffset, &driveremap);
            break;

         case WORK_COPY:

            MSG_OpenArea(&arealiste, pWorkData->pchDestArea, miscoptions.lastreadoffset, &driveremap);
            MSG_OpenArea(&arealiste, pWorkData->pchSrcArea, miscoptions.lastreadoffset, &driveremap);
            SendMsg(client, WORKM_STARTWORKAREA, pWorkData->pchSrcArea, NULL);
            i=0;
            bStop=FALSE;
            ulProgress=0;
            while (!bStop && i < pWorkData->ulArraySize && !bStopWork)
            {
               BOOL bError=FALSE;

               ulNewProgress= (100*i) / pWorkData->ulArraySize;
               if (ulNewProgress != ulProgress)
               {
                  ulProgress= ulNewProgress;
                  SendMsg(client, WORKM_PROGRESS, MPFROMLONG(ulProgress), NULL);
               }


               ulMsgNum=MSG_UidToMsgn(&arealiste, pWorkData->pchSrcArea, pWorkData->MsgIDArray[i], TRUE);
               if (!ulMsgNum)
                  bError=TRUE;
               else
                  if (MSG_ReadNum(&Message, &Header, &arealiste, pWorkData->pchSrcArea, ulMsgNum))
                     bError=TRUE;
                  else
                     if (MSG_CopyMessage(&Message, &Header, &arealiste, pWorkData->pchDestArea,
                                         &driveremap, &userdaten, &generaloptions, SendAddMessage,
                                         pWorkData->ulCopyMove))
                        bError=TRUE;

               if (!bError)
               {

                  i++;
               }
               else
                  switch((ULONG)SendMsg(client, WORKM_ERROR, NULL, NULL))
                  {
                     case WORK_ERROR_IGNORE:  /* ignore error */

                        i++;                  /* next message */
                        break;

                     case WORK_ERROR_RETRY:   /* retry operation, don't inc index */

                        break;

                     case WORK_ERROR_ABORT:   /* abort, set Stop flag */

                        bStop=TRUE;
                        break;

                     default:

                        i++;
                        break;
                  }
            }
            MSG_CloseArea(&arealiste, pWorkData->pchSrcArea, TRUE, miscoptions.lastreadoffset, &driveremap);
            MSG_CloseArea(&arealiste, pWorkData->pchDestArea, TRUE, miscoptions.lastreadoffset, &driveremap);

            AM_FindArea(&arealiste, pWorkData->pchDestArea)->mailentered=TRUE;
            MailEntered[AM_FindArea(&arealiste, pWorkData->pchDestArea)->areadata.areatype]=TRUE;
            break;

         case WORK_MOVE:

            if (!stricmp(pWorkData->pchDestArea, pWorkData->pchSrcArea))
               break;
            MSG_OpenArea(&arealiste, pWorkData->pchDestArea, miscoptions.lastreadoffset, &driveremap);
            MSG_OpenArea(&arealiste, pWorkData->pchSrcArea, miscoptions.lastreadoffset, &driveremap);
            strcpy(SMessageID.pchAreaTag, pWorkData->pchSrcArea);
            SendMsg(client, WORKM_STARTWORKAREA, pWorkData->pchSrcArea, NULL);
            i=0;
            bStop=FALSE;
            ulProgress=0;
            while (!bStop && i < pWorkData->ulArraySize && !bStopWork)
            {
               BOOL bError=FALSE;

               ulNewProgress= (100*i) / pWorkData->ulArraySize;
               if (ulNewProgress != ulProgress)
               {
                  ulProgress= ulNewProgress;
                  SendMsg(client, WORKM_PROGRESS, MPFROMLONG(ulProgress), NULL);
               }

               ulMsgNum=MSG_UidToMsgn(&arealiste, pWorkData->pchSrcArea, pWorkData->MsgIDArray[i], TRUE);

               if (!ulMsgNum)
                  bError=TRUE;
               else
                  if (MSG_ReadNum(&Message, &Header, &arealiste, pWorkData->pchSrcArea, ulMsgNum))
                     bError=TRUE;
                  else
                     if (MSG_MoveMessage(&Message, &Header, &arealiste, pWorkData->pchSrcArea,
                                         pWorkData->pchDestArea, ulMsgNum,
                                         &driveremap, &userdaten, &generaloptions, SendAddMessage,
                                         SendKillMessage, pWorkData->ulCopyMove))
                        bError=TRUE;

               if (!bError)
               {

                  i++;
               }
               else
                  switch((ULONG)SendMsg(client, WORKM_ERROR, NULL, NULL))
                  {
                     case WORK_ERROR_IGNORE:  /* ignore error */

                        i++;                  /* next message */
                        break;

                     case WORK_ERROR_RETRY:   /* retry operation, don't inc index */

                        break;

                     case WORK_ERROR_ABORT:   /* abort, set Stop flag */

                        bStop=TRUE;
                        break;

                     default:

                        i++;
                        break;
                  }
            }
            MSG_CloseArea(&arealiste, pWorkData->pchSrcArea, TRUE, miscoptions.lastreadoffset, &driveremap);
            MSG_CloseArea(&arealiste, pWorkData->pchDestArea, TRUE, miscoptions.lastreadoffset, &driveremap);

            AM_FindArea(&arealiste, pWorkData->pchDestArea)->mailentered=TRUE;
            MailEntered[AM_FindArea(&arealiste, pWorkData->pchDestArea)->areadata.areatype]=TRUE;
            break;

         case WORK_MARK:

            MSG_OpenArea(&arealiste, pWorkData->pchSrcArea, miscoptions.lastreadoffset, &driveremap);
            strcpy(DMessageID.pchAreaTag, pWorkData->pchSrcArea);
            SendMsg(client, WORKM_STARTWORKAREA, pWorkData->pchSrcArea, NULL);
            i=0;
            bStop = FALSE;
            ulProgress=0;
            while(!bStop && i < pWorkData->ulArraySize && !bStopWork)
            {
               ulNewProgress= (100*i) / pWorkData->ulArraySize;
               if (ulNewProgress != ulProgress)
               {
                  ulProgress= ulNewProgress;
                  SendMsg(client, WORKM_PROGRESS, MPFROMLONG(ulProgress), NULL);
               }


               DMessageID.ulMsgID = pWorkData->MsgIDArray[i];
               ulMsgNum=MSG_UidToMsgn(&arealiste, pWorkData->pchSrcArea, pWorkData->MsgIDArray[i], TRUE);
               if (ulMsgNum && !MSG_MarkRead(&arealiste, pWorkData->pchSrcArea, ulMsgNum, "", &driveremap))
               {

                  SendMsg(client, WORKM_READ, &DMessageID, NULL);
                  i++;
               }
               else
                  switch((ULONG) SendMsg(client, WORKM_ERROR, NULL, NULL))
                  {
                     case WORK_ERROR_IGNORE:  /* ignore error */

                        i++;                  /* next message */
                        break;

                     case WORK_ERROR_RETRY:   /* retry operation, don't inc index */

                        break;

                     case WORK_ERROR_ABORT:   /* abort, set Stop flag */

                        bStop=TRUE;
                        break;

                     default:

                        i++;
                        break;
                  }
            }
            MSG_CloseArea(&arealiste, pWorkData->pchSrcArea, TRUE, miscoptions.lastreadoffset, &driveremap);
            break;

         case WORK_MARKALL:

            MSG_OpenArea(&arealiste, pWorkData->pchSrcArea, miscoptions.lastreadoffset, &driveremap);
            SendMsg(client, WORKM_STARTWORKAREA, pWorkData->pchSrcArea, NULL);
            pAreaDef = AM_FindArea(&arealiste, pWorkData->pchSrcArea);
            strcpy(DMessageID.pchAreaTag, pWorkData->pchSrcArea);
            pAreaDef->currentmessage = pAreaDef->maxmessages;
            SendMsg(client, WORKM_REREAD, pWorkData->pchSrcArea, NULL);
            i=1;
            bStop = FALSE;
            ulProgress=0;
            while(!bStop && i <= pAreaDef->maxmessages  && !bStopWork)
            {
               ulNewProgress= (100*i) / pAreaDef->maxmessages;
               if (ulNewProgress != ulProgress)
               {
                  ulProgress= ulNewProgress;
                  SendMsg(client, WORKM_PROGRESS, MPFROMLONG(ulProgress), NULL);
               }


               DMessageID.ulMsgID = MSG_MsgnToUid(&arealiste, pWorkData->pchSrcArea, i);
               if (!MSG_MarkRead(&arealiste, pWorkData->pchSrcArea, i, pAreaDef->areadata.username, &driveremap))
               {

                  i++;
               }
               else
                  switch((ULONG) SendMsg(client, WORKM_ERROR, NULL, NULL))
                  {
                     case WORK_ERROR_IGNORE:  /* ignore error */

                        i++;                  /* next message */
                        break;

                     case WORK_ERROR_RETRY:   /* retry operation, don't inc index */

                        break;

                     case WORK_ERROR_ABORT:   /* abort, set Stop flag */

                        bStop=TRUE;
                        break;

                     default:

                        i++;
                        break;
                  }
            }
            if (i>=pAreaDef->maxmessages)
               DMessageID.ulMsgID = MSG_MsgnToUid(&arealiste, pWorkData->pchSrcArea, pAreaDef->maxmessages);
            else
               DMessageID.ulMsgID = MSG_MsgnToUid(&arealiste, pWorkData->pchSrcArea, i);
            SendMsg(client, WORKM_MARKEND, &DMessageID, NULL);
            MSG_CloseArea(&arealiste, pWorkData->pchSrcArea, TRUE, miscoptions.lastreadoffset, &driveremap);
            break;

         default:
            break;
      }
      free (pWorkData->MsgIDArray);
      pWorkData2 = pWorkData;
      pWorkData = pWorkData->next;
      free(pWorkData2);
   }

   MSG_ClearMessage(&Header, &Message);

   WinPostMsg(client, WORKM_END, NULL, NULL);

   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);


   DEINSTALLEXPT;

   return;
}

void MarkAllMessages(char *pchAreaTag)
{
   extern int tidWorker;
   PWORKDATA pWorkData=malloc(sizeof(WORKDATA));

   pWorkData->next=NULL;
   pWorkData->MsgIDArray=NULL;
   pWorkData->flWorkToDo=WORK_MARKALL;
   pWorkData->pPrintDest = NULL;
   strcpy(pWorkData->pchSrcArea, pchAreaTag);
   strcpy(pWorkData->pchDestArea, pchAreaTag);
   bDoingWork=TRUE;
   tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ListAddMessage                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt eine neue Message in die Messageliste ein             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle des Containers                          */
/*            pMsgID:  Message-ID-Struktur                                   */
/*            pHeader: Neuer Header                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ListAddMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader)
{
   extern USERDATAOPT userdaten;
   extern char CurrentArea[LEN_AREATAG+1];
   MLISTRECORD MRecord;
   int i;

   /* Neue Daten setzen */
   /* Strings kopieren */
   strncpy(MRecord.pchFrom, pHeader->pchFromName, LEN_USERNAME);
   MRecord.pchFrom[LEN_USERNAME]=0;
   strncpy(MRecord.pchTo, pHeader->pchToName, LEN_USERNAME);
   MRecord.pchTo[LEN_USERNAME]=0;
   strncpy(MRecord.pchSubject, pHeader->pchSubject, LEN_SUBJECT);
   MRecord.pchSubject[LEN_SUBJECT]=0;
   MRecord.StampWritten = pHeader->StampWritten;
   MRecord.StampArrived = pHeader->StampArrived;

   MRecord.ulMsgID = pMsgID->ulMsgID;

   /* Read-Flag */
   MRecord.flRecFlags =0;
   if (pHeader->ulAttrib & ATTRIB_READ)
      MRecord.flRecFlags |= LISTFLAG_READ;
   else
      MRecord.flRecFlags &= ~LISTFLAG_READ;

   /* From pruefen */
   i=0;
   while (i<MAX_USERNAMES && stricmp(userdaten.username[i], pHeader->pchFromName))
      i++;
   if (i<MAX_USERNAMES)
      MRecord.flRecFlags |= LISTFLAG_FROMME;
   else
      MRecord.flRecFlags &= ~LISTFLAG_FROMME;

   /* To pruefen */
   i=0;
   while (i<MAX_USERNAMES && stricmp(userdaten.username[i], pHeader->pchToName))
      i++;
   if (i<MAX_USERNAMES)
      MRecord.flRecFlags |= LISTFLAG_TOME;
   else
      MRecord.flRecFlags &= ~LISTFLAG_TOME;

   /* Item anhaengen */
   SendMsg(hwndCnr, MLIM_ADDITEM, &MRecord, NULL);

   /* erstes Item pruefen */
   if (SendMsg(hwndCnr, MLIM_QUERYITEM, MPFROMLONG(MLIT_FIRST),
                  &MRecord))
   {
      if (MSG_UidToMsgn(&arealiste, CurrentArea, MRecord.ulMsgID, TRUE) == 0)
      {
         /* erster Record nicht mehr vorhanden, loeschen */
         SendMsg(hwndCnr, MLIM_DELITEM, MPFROMLONG(MLIT_FIRST), NULL);
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ListChangeMessage                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Aendert eine vorhandene Message ab                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle des Containers                          */
/*            pMsgID:  Message-ID-Struktur                                   */
/*            pHeader: Neuer Header                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ListChangeMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader)
{
   extern USERDATAOPT userdaten;
   LONG lItem;
   MLISTRECORD MRecord;
   int i;

   lItem = (LONG) SendMsg(hwndCnr, MLIM_FINDUMSGID,
                             MPFROMLONG(pMsgID->ulMsgID), NULL);

   if (lItem != MLIT_NONE)
   {
      if (SendMsg(hwndCnr, MLIM_QUERYITEM, MPFROMLONG(lItem),
                     &MRecord))
      {
         /* Neue Daten setzen */
         /* Strings kopieren */
         strncpy(MRecord.pchFrom, pHeader->pchFromName, LEN_USERNAME);
         MRecord.pchFrom[LEN_USERNAME]=0;
         strncpy(MRecord.pchTo, pHeader->pchToName, LEN_USERNAME);
         MRecord.pchTo[LEN_USERNAME]=0;
         strncpy(MRecord.pchSubject, pHeader->pchSubject, LEN_SUBJECT);
         MRecord.pchSubject[LEN_SUBJECT]=0;
         MRecord.StampWritten = pHeader->StampWritten;
         MRecord.StampArrived = pHeader->StampArrived;

         /* Read-Flag */
         if (pHeader->ulAttrib & ATTRIB_READ)
            MRecord.flRecFlags |= LISTFLAG_READ;
         else
            MRecord.flRecFlags &= ~LISTFLAG_READ;

         /* From pruefen */
         i=0;
         while (i<MAX_USERNAMES && stricmp(userdaten.username[i], pHeader->pchFromName))
            i++;
         if (i<MAX_USERNAMES)
            MRecord.flRecFlags |= LISTFLAG_FROMME;
         else
            MRecord.flRecFlags &= ~LISTFLAG_FROMME;

         /* To pruefen */
         i=0;
         while (i<MAX_USERNAMES && stricmp(userdaten.username[i], pHeader->pchToName))
            i++;
         if (i<MAX_USERNAMES)
            MRecord.flRecFlags |= LISTFLAG_TOME;
         else
            MRecord.flRecFlags &= ~LISTFLAG_TOME;

         SendMsg(hwndCnr, MLIM_UPDATEITEM, &MRecord, MPFROMLONG(lItem));
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ListDeleteMessage                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht eine vorhandene Message                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle der Liste                               */
/*            pMsgID:  Message-ID-Struktur                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ListDeleteMessage(HWND hwndCnr, PMESSAGEID pMsgID)
{
   LONG lItem;

   lItem = (LONG) SendMsg(hwndCnr, MLIM_FINDUMSGID,
                             MPFROMLONG(pMsgID->ulMsgID), NULL);

   if (lItem != MLIT_NONE)
      SendMsg(hwndCnr, MLIM_DELITEM, MPFROMLONG(lItem), NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ListMarkReadMessage                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert eine Message als gelesen                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle der Liste                               */
/*            pMsgID:  Message-ID-Struktur                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ListMarkReadMessage(HWND hwndCnr, PMESSAGEID pMsgID)
{
   LONG lItem;
   MLISTRECORD MRecord;

   lItem = (LONG) SendMsg(hwndCnr, MLIM_FINDUMSGID,
                             MPFROMLONG(pMsgID->ulMsgID), NULL);

   if (lItem != MLIT_NONE)
   {
      if (SendMsg(hwndCnr, MLIM_QUERYITEM, MPFROMLONG(lItem),
                     &MRecord))
      {
         MRecord.flRecFlags |= LISTFLAG_READ;
         SendMsg(hwndCnr, MLIM_UPDATEITEM, &MRecord, MPFROMLONG(lItem));
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ListMarkAllRead                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert alle Messages als gelesen                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle der Liste                               */
/*            pMsgID:  Message-ID-Struktur                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ListMarkAllRead(HWND hwndCnr, PMESSAGEID pMsgID)
{
   LONG lItem;
   MLISTRECORD MRecord;
   LONG lCount;

   lCount = (LONG) SendMsg(hwndCnr, MLIM_QUERYITEMCOUNT, NULL, NULL);

   for (lItem=0; lItem < lCount; lItem++)
   {
      if (SendMsg(hwndCnr, MLIM_QUERYITEM, MPFROMLONG(lItem),
                     &MRecord))
      {
         if (MRecord.ulMsgID > pMsgID->ulMsgID)
            break;

         if (!(MRecord.flRecFlags & LISTFLAG_READ))
         {
            MRecord.flRecFlags |= LISTFLAG_READ;
            SendMsg(hwndCnr, MLIM_UPDATEITEM, &MRecord, MPFROMLONG(lItem));
         }
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MListSettingsProc                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f. Msg-List-Settings                       */
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

static MRESULT EXPENTRY MListSettingsProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp;
   HWND notebook=NULLHANDLE;

   switch(message)
   {
      MRESULT resultbuf;

      case WM_INITDLG:
         notebook=WinWindowFromID(hwnd, IDD_MLISTSETTINGS+1);
         InsertMListSettingsPages(notebook);
         RestoreWinPos(hwnd, &windowpositions.mlistsettingspos, TRUE, TRUE);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_MLISTSETTINGS+1);
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(hwnd, message, mp1, mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=255;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=190;

         return resultbuf;

      case WM_DESTROY:
         QueryWinPos(hwnd, &(windowpositions.mlistsettingspos));
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
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InsertMListSettingsPages                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt die Settings-Pages in das Notebook fuer die           */
/*               Arealisten-Settings ein.                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: notebook: Window-handle des Notebooks                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void InsertMListSettingsPages(HWND notebook)
{
   SetNotebookParams(notebook, 80);

   InsertOnePage(notebook, IDD_ML_SETTINGS_COLORS, IDST_TAB_AL_COLORS, MListSettColProc, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MListSettColProc                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f. Msg-List-Color-Settings                 */
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

static MRESULT EXPENTRY MListSettColProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern DIRTYFLAGS dirtyflags;

   switch (message)
   {
      case WM_INITDLG:
         /* Farben im Value-Set initialisieren */
         WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(1, 1),
                           MPFROMLONG(msglistoptions.lBackClr));
         WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(2, 1),
                           MPFROMLONG(msglistoptions.lForeClr));
         WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(3, 1),
                           MPFROMLONG(msglistoptions.lUnreadClr));
         WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(4, 1),
                           MPFROMLONG(msglistoptions.lFromClr));
         WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_SETITEM,
                           MPFROM2SHORT(5, 1),
                           MPFROMLONG(msglistoptions.lToClr));

         /* Erstes Element im VS auswaehlen */
         WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_SELECTITEM,
                           MPFROM2SHORT(1, 1), NULL);

         /* Fadenkreuz im Color-Wheel entsprechend setzen */
         WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+3, CLSM_SETRGB,
                           &msglistoptions.lBackClr,
                           NULL);
         break;

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_ML_SETTINGS_COLORS+3:  /* Color-Wheel */
               if (SHORT2FROMMP(mp1) == CLSN_RGB)
               {
                  MRESULT selected;

                  /* selektiertes Item im Value-Set abfragen */
                  selected=WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_QUERYSELECTEDITEM,
                                             NULL, NULL);

                  /* Farbe updaten */
                  WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4, VM_SETITEM,
                                    selected, mp2);

                  /* Farbe in Settings eintragen */
                  switch(SHORT1FROMMR(selected))
                  {
                     case 1:
                        msglistoptions.lBackClr = (LONG) mp2;
                        break;

                     case 2:
                        msglistoptions.lForeClr = (LONG) mp2;
                        break;

                     case 3:
                        msglistoptions.lUnreadClr = (LONG) mp2;
                        break;

                     case 4:
                        msglistoptions.lFromClr = (LONG) mp2;
                        break;

                     case 5:
                        msglistoptions.lToClr = (LONG) mp2;
                        break;

                     default:
                        break;
                  }

                  /* Dirty-Flag setzen */
                  dirtyflags.mlsettingsdirty=TRUE;
               }
               break;

            case IDD_ML_SETTINGS_COLORS+4:  /* Value-Set */
               if (SHORT2FROMMP(mp1) == VN_SELECT)
               {
                  ULONG ulColor;

                  /* neue Selektion abfragen */
                  ulColor=(ULONG)WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+4,
                                                   VM_QUERYITEM, mp2, NULL);

                  /* Fadenkreuz setzen */
                  WinSendDlgItemMsg(hwnd, IDD_ML_SETTINGS_COLORS+3,
                                    CLSM_SETRGB, &ulColor, NULL);
               }
               break;

            default:
               break;
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*-------------------------------- Modulende --------------------------------*/


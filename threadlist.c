/*---------------------------------------------------------------------------+
 | Titel: THREADLIST.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 28.01.94                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Threadliste von FleetStreet                                             |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_GPILOGCOLORTABLE
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
#include "msgheader.h"
#include "dialogids.h"
#include "areaman\areaman.h"
#include "handlemsg\handlemsg.h"
#include "threadlist.h"
#include "msglist.h"
#include "utility.h"
#include "threadlistsettings.h"
#include "areadlg.h"
#include "dialogs.h"
#include "util\fltutil.h"
#include "dump\expt.h"

/*--------------------------------- Defines ---------------------------------*/

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L    /* 2.1-spezifisch !!! */
#endif

/* eigene Notification-Codes */
#define CN_CHORD 200
#define CN_COLORCHANGED 201

#define NO_INSERT     0
#define INSERT_PARENT 1
#define INSERT_CHILD  2
#define INSERT_THREAD 3

#define ISREAD(x) ((x).Header.ulAttrib & ATTRIB_READ)

/*---------------------------------- Typen ----------------------------------*/

typedef struct tracelist
{
   int msgid;
   struct tracelist *next;
} TRACELIST;

typedef struct
{
   PWORKDATA pWorkData;
   ULONG ulStartID;
} THREADWORK, *PTHREADWORK;

typedef struct
{
   ULONG ulFlags;              /* INSERT_* */
   ULONG ulNextParent;
   MSGHEADER Header;
} THREADHEADERS, *PTHREADHEADERS;

typedef struct _THREADRECORD
{
   RECORDCORE RecordCore;
   char       pchText[10+LEN_USERNAME+LEN_SUBJECT]; /* Platz f. pszTree */
   ULONG      ulMsgID;                              /* Message-ID */
   ULONG      ulFlags;                              /* Flags, s.u. */
   char       pchName[LEN_USERNAME+1];
   char       pchSubj[LEN_SUBJECT+1];
} THREADRECORD, *PTHREADRECORD;

#define THREADFLAG_NOREPLY     0x0001UL
#define THREADFLAG_READ        0x0002UL
#define THREADFLAG_PERSONAL    0x0004UL
#define THREADFLAG_THREADSTART 0x8000UL

typedef struct _THREADLISTDATA
{
   MSGLISTPAR MsgListPar;
   HPOINTER   icon;
   TRACELIST  *Trace;
   TRACELIST  *Trace2;
   ULONG      numbers[2];
   POINTL     pointl;
   HSWITCH    hSwitch;
   LONG       lDisableCount;
   HWND       hwndListPopup;
   HWND       hwndThreadPopup;
   BOOL       bKeyboard;
   PTHREADRECORD pPopupRecord;
   ULONG      dspmode;
   BOOL       bForeground;
   BOOL       bSenderName;
   char       pchEmptySubj[LEN_SUBJECT+1];
   BOOL       bNotify;
} THREADLISTDATA, *PTHREADLISTDATA;


/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HMODULE hmodLang;

extern int tidThreadList;
extern HWND hwndThreadList;
extern volatile BOOL DoingInsert;
extern volatile BOOL StopInsert;

extern AREALIST arealiste;
extern char CurrentArea[LEN_AREATAG+1];

static HPOINTER hptrPlus;
static HPOINTER hptrMinus;

static char pchWorking[50];
static char pchUnread[50];
static char pchAllThreads[50];
static char pchUnreadOnly[50];

static THREADWORK ThreadWork;

extern THREADLISTOPTIONS threadlistoptions;
extern DIRTYFLAGS dirtyflags;

extern int  tidWorker;
extern BOOL bDoingWork;

/*--------------------------- Funktionsprototypen ---------------------------*/

static PFNWP OldThContainerProc;

/*----------------------- interne Funktionsprototypen -----------------------*/

static void UpdateDspMenu(PTHREADLISTDATA pThreadListData);
static void CleanupThreadList(HWND hwndContainer);
static void _Optlink InsertUnreadThreads(void *p);
static THREADRECORD *ExpandThreads(HWND hwndContainer, PTHREADRECORD pParent, PTHREADLISTDATA pThreadListData);
static MRESULT EXPENTRY NewThContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static void ExpandBranch(HWND hwndContainer, PTHREADRECORD pParent);
static BOOL DrawTree(POWNERITEM Item);
static SHORT _System SortThreads(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage);
static PTHREADRECORD GetNextRecord(PTHREADRECORD pRecord, HWND hwndContainer);
static void MsgIDsToIndex(PTHREADHEADERS Headers, ULONG maxmsgs);
static void TraceAllThreads(PTHREADHEADERS Headers, ULONG maxmsgs);
static void TraceUnreadThreads(PTHREADHEADERS Headers, ULONG maxmsgs, PULONG pCurrentNum);
static void TraceUnreadMsgs(PTHREADHEADERS Headers, ULONG maxmsgs, PULONG pCurrentNum);
static void TraceSubjects(PTHREADHEADERS Headers, ULONG maxmsgs);
static void ThreadsReady(HWND parent, PTHREADLISTDATA pThreadListData, PTHREADHEADERS Headers, ULONG maxmsgs);
static void DeleteMessage(HWND hwndCnr, PMESSAGEID pMessageID);
static void ThreadAddMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader, PTHREADLISTDATA pThreadListData);
static void ThreadChangeMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader, PTHREADLISTDATA pThreadListData);
static void ThreadContextMenu(HWND hwndDlg, PTHREADLISTDATA pThreadListData, PTHREADRECORD pRecord);
static void SwitchThreadlistView(HWND hwndDlg, PTHREADLISTDATA pThreadListData, SHORT sCmdID);
static void SwitchSenderName(HWND hwndDlg, PTHREADLISTDATA pThreadListData, BOOL bNoUpdate);
static BOOL IsPersonalMessage(char *pchToName);
static void HeaderToRecord(PTHREADLISTDATA pThreadListData, PMSGHEADER pHeader, PTHREADRECORD pRecord, BOOL bStripRe);
static void HeaderToThread(PTHREADLISTDATA pThreadListData, PMSGHEADER pHeader, PTHREADRECORD pRecord, BOOL bStripRe);
static void InsertRecords(HWND hwndDlg, PTHREADRECORD pParent, PTHREADRECORD pInsert, ULONG ulNumInsert, BOOL bInvalidate);

static void WorkOnThread(HWND hwndDlg, PTHREADLISTDATA pThreadListData, USHORT usCmdID);
static void _Optlink CollectThread(void *pvWorkData);
static void RecurseThread(ULONG ulMsgID, PWORKDATA pWorkData);
static char *BuildThreadTitle(char *pchName, char *pchSubj, BOOL bUseSender, char *pchBuffer);

/*------------------------------ NewContainerProc ---------------------------*/
/* Neue Window-Prozedur f. Container, um OS/2-Bug zu umschiffen              */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewThContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   POINTL pointl;
   PRECORDCORE pRecord=NULL;
   QUERYRECFROMRECT qRecord;

   switch(message)
   {
      case DM_DRAGOVER:
         return (MRESULT) DOR_NEVERDROP;

      case WM_CHORD:
         /* Cursorposition ermitteln */
         WinQueryPointerPos(HWND_DESKTOP, &pointl);
         WinMapWindowPoints(HWND_DESKTOP, parent, &pointl, 1);

         /* Record an der Stelle ermitteln */
         qRecord.cb=sizeof(QUERYRECFROMRECT);
         qRecord.rect.xLeft=pointl.x;
         qRecord.rect.xRight=pointl.x+1;
         qRecord.rect.yBottom=pointl.y;
         qRecord.rect.yTop=pointl.y+1;
         qRecord.fsSearch=CMA_PARTIAL | CMA_ITEMORDER;

         pRecord=(PRECORDCORE)SendMsg(parent, CM_QUERYRECORDFROMRECT,
                                         MPFROMP(CMA_FIRST), &qRecord);

         /* Notify owner */
         SendMsg(WinQueryWindow(parent, QW_OWNER), WM_CONTROL,
                    MPFROM2SHORT(WinQueryWindowUShort(parent, QWS_ID),
                                 CN_CHORD),
                    pRecord);
         break;

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
   return OldThContainerProc(parent, message, mp1, mp2);
}

/*------------------------------ ThreadListProc -----------------------------*/
/* Fensterprozedur der Messageliste in Thread-Darstellung.                   */
/* In der RECORDCORE-Struktur werden folgende Felder als Flags verwendet:    */
/*  pszIcon: NULL    Die Message hat Replies, die noch nicht eingefuegt      */
/*                   worden sind,                                            */
/*           != NULL Die Message hat keine Replies oder sie wurden schon     */
/*                   eingefuegt,                                             */
/*  pszName: NULL    Message wurde noch nicht gelesene (f. Ownerdraw)        */
/*           != NULL Message wurde schon gelesen                             */
/*  pszText: MSGNUM  Message-ID                                              */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY ThreadListProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWFONTS windowfonts;
   extern char CurrentArea[LEN_AREATAG+1];
   extern MSGHEADER CurrentHeader;
   extern HWND hwndhelp, client;
   extern GENERALOPT generaloptions;
   PTHREADLISTDATA pThreadListData;
   CNRINFO cnrinfo;
   MSGHEADER Header;
   PTHREADRECORD pRecord;
   static ULONG MinorVersion=0;

   pThreadListData=(PTHREADLISTDATA)WinQueryWindowULong(parent, QWL_USER);

   switch(message)
   {
      case WM_INITDLG:
         pThreadListData=malloc(sizeof(THREADLISTDATA));
         memset(pThreadListData, 0, sizeof(THREADLISTDATA));
         WinSetWindowULong(parent, QWL_USER, (ULONG) pThreadListData);

         memcpy(&pThreadListData->MsgListPar, (MSGLISTPAR *)mp2, sizeof(MSGLISTPAR));
         pThreadListData->dspmode = threadlistoptions.dspmode;
         pThreadListData->bSenderName = threadlistoptions.shownames;
         LoadString(IDST_ML_NOSUBJ, LEN_SUBJECT+1, pThreadListData->pchEmptySubj);

         pThreadListData->hSwitch=AddToWindowList(parent);
         OldThContainerProc=WinSubclassWindow(WinWindowFromID(parent, IDD_THREADLIST+1),
                                            NewThContainerProc);
         LoadString(IDST_ML_WORKING, 50, pchWorking);
         LoadString(IDST_ML_UNREAD, 50, pchUnread);
         LoadString(IDST_ML_ALLTHREADS, 50, pchAllThreads);
         LoadString(IDST_ML_UNREADONLY, 50, pchUnreadOnly);
         pThreadListData->icon=LoadIcon(IDB_MSGTREE);
         SendMsg(parent, WM_SETICON, (MPARAM) pThreadListData->icon, (MPARAM) 0);

         RestoreWinPos(parent, &threadlistoptions.ListPos, TRUE, TRUE);
         SetForeground(WinWindowFromID(parent,IDD_THREADLIST+1),
                       &threadlistoptions.lReadClr);
         SetBackground(WinWindowFromID(parent,IDD_THREADLIST+1),
                       &threadlistoptions.lBackClr);
         SetFont(WinWindowFromID(parent,IDD_THREADLIST+1),
                                 windowfonts.threadlistfont);


         pThreadListData->hwndListPopup = WinLoadMenu(HWND_OBJECT, hmodLang, IDM_TL_POPUP);
         pThreadListData->hwndThreadPopup = WinLoadMenu(HWND_OBJECT, hmodLang, IDM_TH_POPUP);
         UpdateDspMenu(pThreadListData);

         if (pThreadListData->hwndListPopup)
            ReplaceSysMenu(parent, pThreadListData->hwndListPopup, 1);

         if (threadlistoptions.keepinfront)
         {
            WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_FGROUND, TRUE);
            pThreadListData->bForeground = TRUE;
            WinSetOwner(parent, client);
         }
         else
         {
            WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_FGROUND, FALSE);
            pThreadListData->bForeground = FALSE;
            WinSetOwner(parent, HWND_DESKTOP);
         }

         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_SENDER, pThreadListData->bSenderName);

         /* OS/2 3.0 and below: replace tree icons */
         DosQuerySysInfo(QSV_VERSION_MINOR, QSV_VERSION_MINOR, &MinorVersion,
                         sizeof(MinorVersion));

         if (MinorVersion < 40)
         {
            hptrPlus=LoadIcon(IDIC_PLUS);
            hptrMinus=LoadIcon(IDIC_MINUS);
         }

         cnrinfo.cb=sizeof(CNRINFO);
         cnrinfo.flWindowAttr=CV_TREE | CV_TEXT | CA_TREELINE | CA_OWNERDRAW |
                              CA_CONTAINERTITLE | CA_TITLESEPARATOR;
         cnrinfo.slTreeBitmapOrIcon.cx=16;
         cnrinfo.slTreeBitmapOrIcon.cy=16;
         if (MinorVersion < 40)
         {
            cnrinfo.hptrExpanded=hptrMinus;
            cnrinfo.hptrCollapsed=hptrPlus;
         }
         cnrinfo.cxTreeLine=1;
         cnrinfo.cxTreeIndent=14;
         cnrinfo.pszCnrTitle=pchWorking;
         cnrinfo.pSortRecord=(PVOID)SortThreads;
         WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_FLWINDOWATTR |
                                      ((MinorVersion<40)?CMA_TREEICON:0) |
                                      CMA_CXTREEINDENT |
                                      CMA_PSORTRECORD |
                                      CMA_CXTREELINE |
                                      CMA_CNRTITLE));
         SendMsg(parent, TM_REREADTHREADS, mp2, NULL);
         pThreadListData->bNotify = TRUE;
         SetInitialAccel(parent);
         break;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
            WinShowWindow(WinWindowFromID(parent, IDD_THREADLIST+1), FALSE);

         if (((PSWP)mp1)->fl & (SWP_RESTORE|SWP_MAXIMIZE))
            WinShowWindow(WinWindowFromID(parent, IDD_THREADLIST+1), TRUE);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, parent, IDD_THREADLIST+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pThreadListData && pThreadListData->bNotify)
            SaveWinPos(parent, (PSWP) mp1, &threadlistoptions.ListPos, &dirtyflags.threadsdirty);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_THREADLIST+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               case CN_EXPANDTREE:
                  if (pThreadListData->lDisableCount)
                  {
                     WinAlarm(HWND_DESKTOP, WA_NOTE);
                     break;
                  }
                  WinEnableWindowUpdate(WinWindowFromID(parent, IDD_THREADLIST+1), FALSE);
                  ExpandThreads(WinWindowFromID(parent, IDD_THREADLIST+1), mp2, pThreadListData);
                  WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_INVALIDATERECORD, NULL, NULL);
                  WinEnableWindowUpdate(WinWindowFromID(parent, IDD_THREADLIST+1), TRUE);
                  break;

               case CN_ENTER:
                  if (((PNOTIFYRECORDENTER)mp2)->pRecord)
                  {
                     if (pThreadListData->lDisableCount)
                     {
                        WinAlarm(HWND_DESKTOP, WA_ERROR);
                        return (MRESULT) FALSE;
                     }
                     pRecord=(PTHREADRECORD)WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                                               CM_QUERYRECORDEMPHASIS, (MPARAM)CMA_FIRST,
                                               MPFROMSHORT(CRA_SELECTED));
                     if (pRecord > (PTHREADRECORD)NULL)
                     {
                        if (pRecord->ulMsgID)
                        {
                           SendMsg(client, TM_JUMPTOMESSAGE,
                                   MPFROMLONG(pRecord->ulMsgID), NULL);
                           SetFocusControl(client, IDML_MAINEDIT);
                        }
                     }
                  }
                  break;

               case CN_HELP:
                  SendMsg(parent, WM_HELP, MPFROMSHORT(IDD_THREADLIST+1), NULL);
                  break;

               case CN_CHORD:
                  if (pThreadListData->lDisableCount)
                  {
                     WinAlarm(HWND_DESKTOP, WA_NOTE);
                     return (MRESULT) FALSE;
                  }
                  if (DoingInsert)
                     WinAlarm(HWND_DESKTOP, WA_NOTE);
                  else
                  {
                     WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
                                                                    SPTR_WAIT, FALSE));
                     pRecord=(PTHREADRECORD) mp2;
                     if (pRecord > (PTHREADRECORD)NULL)
                     {
                        WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_EXPANDTREE,
                                          pRecord, NULL);
                        ExpandBranch(WinWindowFromID(parent, IDD_THREADLIST+1), pRecord);
                     }
                     WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_INVALIDATERECORD,
                                       NULL, NULL);
                     WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
                                                                    SPTR_ARROW, FALSE));
                  }
                  break;

               case CN_CONTEXTMENU:
                  ThreadContextMenu(parent, pThreadListData, (PTHREADRECORD) mp2);
                  break;

               case CN_COLORCHANGED:
                  if (pThreadListData && pThreadListData->bNotify)
                  {
                     QueryForeground(WinWindowFromID(parent, IDD_THREADLIST+1),
                                     &threadlistoptions.lReadClr);
                     QueryBackground(WinWindowFromID(parent, IDD_THREADLIST+1),
                                     &threadlistoptions.lBackClr);
                     dirtyflags.threadsdirty=TRUE;
                  }
                  break;

               default:
                  break;
            }
         }
         break;

      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1))
         {
            case IDM_TLP_SETTINGS:
               WinDlgBox(HWND_DESKTOP, parent, ThreadListSettingsProc,
                         hmodLang, IDD_THRLISTSETTINGS, NULL);
               if (dirtyflags.threadsdirty)
               {
                  WinEnableWindowUpdate(WinWindowFromID(parent, IDD_THREADLIST+1), FALSE);
                  pThreadListData->bNotify=FALSE;

                  SetForeground(WinWindowFromID(parent,IDD_THREADLIST+1),
                                &threadlistoptions.lReadClr);
                  SetBackground(WinWindowFromID(parent,IDD_THREADLIST+1),
                                &threadlistoptions.lBackClr);

                  WinInvalidateRect(WinWindowFromID(parent, IDD_THREADLIST+1),
                                    NULL, TRUE);
                  pThreadListData->bNotify=TRUE;
                  WinEnableWindowUpdate(WinWindowFromID(parent, IDD_THREADLIST+1), TRUE);

                  WinPostMsg(parent, WM_COMMAND, MPFROMSHORT(IDM_TLP_REFRESH), NULL);
               }
               return (MRESULT) FALSE;

            case IDM_TLP_VIEWALL:
            case IDM_TLP_VIEWTHR:
            case IDM_TLP_VIEWUNR:
               SwitchThreadlistView(parent, pThreadListData, SHORT1FROMMP(mp1));
               return (MRESULT) FALSE;

            case IDM_TLP_REFRESH:
               if (DoingInsert)
               {
                  StopInsert=TRUE;
                  DosWaitThread((PTID) &tidThreadList, DCWW_WAIT);
               }
               SendMsg(parent, TM_REREADTHREADS, &pThreadListData->MsgListPar, NULL);
               return (MRESULT) FALSE;

            case IDM_TLP_FGROUND:
               if (!pThreadListData->bForeground)
               {
                  WinSetOwner(parent, client);
                  pThreadListData->bForeground = TRUE;
                  WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_FGROUND, TRUE);
               }
               else
               {
                  WinSetOwner(parent, HWND_DESKTOP);
                  pThreadListData->bForeground = FALSE;
                  WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_FGROUND, FALSE);
               }
               return (MRESULT) FALSE;

            case IDM_TLP_SENDER:
               SwitchSenderName(parent, pThreadListData, DoingInsert);
               return (MRESULT) FALSE;

            case IDM_TLP_CATCHUP:
               if (bDoingWork)
               {
                  /* Fehlermeldung */
                  MessageBox(parent, IDST_MSG_DOINGWORK, 0, IDD_DOINGWORK,
                             MB_OK);
                  return (MRESULT) FALSE;
               }
               if (generaloptions.safety & SAFETY_CATCHUP)
                  if (MessageBox(parent, IDST_MSG_CATCHUP, IDST_TITLE_CATCHUP,
                                 IDD_CATCHUP, MB_YESNO) != MBID_YES)
                     return (MRESULT) FALSE;
               MarkAllMessages(CurrentArea);
               return (MRESULT) FALSE;

            case IDM_THP_DELETE:
            case IDM_THP_MOVE:
            case IDM_THP_COPY:
            case IDM_THP_EXPORT:
            case IDM_THP_PRINT:
            case IDM_THP_MARK:
               WorkOnThread(parent, pThreadListData, SHORT1FROMMP(mp1));
               return (MRESULT) FALSE;

            case IDM_THP_EXPAND:
               WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
                                                              SPTR_WAIT, FALSE));
               if (pThreadListData->pPopupRecord)
               {
                  WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_EXPANDTREE,
                                    pThreadListData->pPopupRecord, NULL);
                  ExpandBranch(WinWindowFromID(parent, IDD_THREADLIST+1),
                               pThreadListData->pPopupRecord);
               }
               WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_INVALIDATERECORD,
                                 NULL, NULL);
               WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
                                                              SPTR_ARROW, FALSE));
               return (MRESULT) FALSE;

            case DID_CANCEL:
               if (DoingInsert)
               {
                  StopInsert=TRUE;
                  DosWaitThread((PTID) &tidThreadList, DCWW_WAIT);
               }
               CleanupThreadList(WinWindowFromID(parent, IDD_THREADLIST+1));
               WinPostMsg(client, TM_THREADLISTCLOSE, NULL, NULL);
               break;

            default:
               return RedirectCommand(mp1, mp2);
         }
         break;

      case WM_CLOSE:
         if (DoingInsert)
         {
            StopInsert=TRUE;
            DosWaitThread((PTID) &tidThreadList, DCWW_WAIT);
         }
         CleanupThreadList(WinWindowFromID(parent, IDD_THREADLIST+1));
         WinPostMsg(client, TM_THREADLISTCLOSE, NULL, NULL);
         break;

      case WM_DESTROY:
         RemoveFromWindowList(pThreadListData->hSwitch);
         QueryFont(WinWindowFromID(parent,IDD_THREADLIST+1),
                   windowfonts.threadlistfont);
         if (pThreadListData->bForeground != threadlistoptions.keepinfront)
         {
            threadlistoptions.keepinfront=pThreadListData->bForeground;
            dirtyflags.threadsdirty=TRUE;
         }
         WinDestroyPointer(pThreadListData->icon);
         if (MinorVersion < 40)
         {
            WinDestroyPointer(hptrPlus);
            WinDestroyPointer(hptrMinus);
         }
         if (pThreadListData->hwndListPopup)
            WinDestroyWindow(pThreadListData->hwndListPopup);
         if (pThreadListData->hwndThreadPopup)
            WinDestroyWindow(pThreadListData->hwndThreadPopup);
         while(pThreadListData->Trace)
         {
            pThreadListData->Trace2=pThreadListData->Trace->next;
            free(pThreadListData->Trace);
            pThreadListData->Trace=pThreadListData->Trace2;
         }
         free(pThreadListData);
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, parent);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_DRAWITEM:
         if (SHORT1FROMMP(mp1)==IDD_THREADLIST+1)
            return (MRESULT) DrawTree((POWNERITEM) mp2);
         else
            return FALSE;

      case WM_CHAR:
        if ((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
            !(SHORT1FROMMP(mp1) & KC_KEYUP))
           if (SHORT2FROMMP(mp2) == VK_SPACE)
           {
              if (pThreadListData->lDisableCount)
              {
                 WinAlarm(HWND_DESKTOP, WA_NOTE);
                 return (MRESULT) FALSE;
              }
              if (DoingInsert)
                 WinAlarm(HWND_DESKTOP, WA_NOTE);
              else
              {
                 WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
                                                                SPTR_WAIT, FALSE));
                 pRecord=(PTHREADRECORD)WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                                           CM_QUERYRECORDEMPHASIS, (MPARAM)CMA_FIRST,
                                           MPFROMSHORT(CRA_SELECTED));
                 if (pRecord > (PTHREADRECORD)NULL)
                 {
                    WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_EXPANDTREE,
                                      pRecord, NULL);
                    ExpandBranch(WinWindowFromID(parent, IDD_THREADLIST+1), pRecord);
                 }
                 WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_INVALIDATERECORD,
                                   NULL, NULL);
                 WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
                                                                SPTR_ARROW, FALSE));
              }
           }
         break;

      case TM_THREADSREADY:
         ThreadsReady(parent, pThreadListData, (PTHREADHEADERS)mp1, (ULONG)mp2);
         break;

      case TM_REREADTHREADS:
         if (DoingInsert)
         {
            StopInsert=TRUE;
            DosWaitThread((PTID) &tidThreadList, DCWW_WAIT);
         }
         if (DoingInsert) /* immer noch */
         {
            QMSG qmsg;

            if (WinPeekMsg(anchor, &qmsg, parent, TM_THREADSREADY, TM_THREADSREADY,
                           PM_REMOVE))
            {
               /* Message gefunden, Speicher freigeben */
               if (qmsg.mp1)
                  free(qmsg.mp1);
            }
         }

         memcpy(&pThreadListData->MsgListPar, (MSGLISTPAR *)mp1, sizeof(MSGLISTPAR));
         CleanupThreadList(WinWindowFromID(parent, IDD_THREADLIST+1));
         if (pThreadListData->MsgListPar.msgnum==0)
            break;
         DoingInsert=TRUE;

         /* Top-Level der aktuellen Message suchen */
         memcpy(&Header, &CurrentHeader, sizeof(Header));
         pThreadListData->Trace=malloc(sizeof(TRACELIST));
         pThreadListData->Trace->msgid=pThreadListData->MsgListPar.msgnum;
         pThreadListData->Trace->next=NULL;

         /* Anfang des Threads suchen */
         while(Header.ulReplyTo &&
               (!(Header.ulAttrib & ATTRIB_READ) || pThreadListData->dspmode!=DSPTHREADS_UNREADONLY))
         {
            int msgnum;
            TRACELIST *pTemp = pThreadListData->Trace;

            /* Endlosschleife verhindern */
            while (pTemp && pTemp->msgid != Header.ulReplyTo)
               pTemp = pTemp->next;
            if (pTemp) /* waren schonmal da */
               break;

            /* @@ das m≪te mal aufgerеmt werden, mieser Code! */
            pThreadListData->Trace2=malloc(sizeof(TRACELIST));
            pThreadListData->Trace2->msgid=Header.ulReplyTo;

            msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, Header.ulReplyTo, TRUE);
            if (msgnum)
            {
               if (MSG_ReadHeader(&Header, &arealiste, CurrentArea, msgnum))
               {
                  free(pThreadListData->Trace2);
                  break;
               }
               else
               {
                  pThreadListData->Trace2->next=pThreadListData->Trace;
                  pThreadListData->Trace=pThreadListData->Trace2;
               }
            }
            else
            {
               free(pThreadListData->Trace2);
               break;
            }
         }
         /* Top-Level-Message gefunden, Trace enth�lt die Messagenummern bis zur */
         /* aktuellen Message */
         cnrinfo.pszCnrTitle=pchWorking;
         WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_CNRTITLE));

         pThreadListData->numbers[0]=pThreadListData->MsgListPar.msgnum;
         pThreadListData->numbers[1]=pThreadListData->Trace->msgid;

         /* Thread-Anfaenge suchen und einfuegen, zweite Stufe mitlesen */
         tidThreadList=_beginthread(InsertUnreadThreads, NULL, 32768, pThreadListData);
         break;

      case WORKM_READ:
         /* Message im Container suchen */
         pRecord=WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_QUERYRECORD, NULL,
                            MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
         while (pRecord && pRecord->ulMsgID != ((PMESSAGEID) mp1)->ulMsgID)
            pRecord=GetNextRecord(pRecord, WinWindowFromID(parent, IDD_THREADLIST+1));

         if (pRecord && !(pRecord->ulFlags & THREADFLAG_READ))
         {
            pRecord->ulFlags |= THREADFLAG_READ;
            WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_INVALIDATERECORD, &pRecord,
                              MPFROM2SHORT(1, CMA_TEXTCHANGED | CMA_NOREPOSITION));
         }
         break;

      case WORKM_DISABLEVIEWS:
         pThreadListData->lDisableCount++;
         break;

      case WORKM_ENABLEVIEWS:
         pThreadListData->lDisableCount--;
         if (pThreadListData->lDisableCount < 0)
            pThreadListData->lDisableCount = 0;
         break;

      case WORKM_MARKEND:
         if (!stricmp(CurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
         {
            HWND hwndCnr = WinWindowFromID(parent, IDD_THREADLIST+1);

            /* Alle Records als gelesen markieren */
            pRecord=SendMsg(hwndCnr, CM_QUERYRECORD, NULL,
                               MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
            do
            {
               if (pRecord)
               {
                  if (pRecord->ulMsgID <= ((PMESSAGEID) mp1)->ulMsgID)
                     pRecord->ulFlags|=THREADFLAG_READ;
                  pRecord=GetNextRecord(pRecord, hwndCnr);
               }
            }  while (pRecord);
            SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
         }
         break;

      case WORKM_DELETED:
         if (!stricmp(CurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            DeleteMessage(WinWindowFromID(parent, IDD_THREADLIST+1), (PMESSAGEID) mp1);
         break;

      case WORKM_ADDED:
         if (!stricmp(CurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            ThreadAddMessage(WinWindowFromID(parent, IDD_THREADLIST+1), (PMESSAGEID) mp1,
                             (MSGHEADER*) mp2, pThreadListData);
         break;

      case WORKM_CHANGED:
         if (!stricmp(CurrentArea, ((PMESSAGEID) mp1)->pchAreaTag))
            ThreadChangeMessage(WinWindowFromID(parent, IDD_THREADLIST+1), (PMESSAGEID) mp1,
                                (MSGHEADER*) mp2, pThreadListData);
         break;

      case WM_MENUEND:
         if ((HWND) mp2 == pThreadListData->hwndListPopup ||
             (HWND) mp2 == pThreadListData->hwndThreadPopup)
         {
            pThreadListData->bKeyboard=FALSE;

            /* Source-Emphasis loeschen */
            WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                              CM_SETRECORDEMPHASIS, pThreadListData->pPopupRecord,
                              MPFROM2SHORT(FALSE, CRA_SOURCE));
            if ( (HWND) mp2 == pThreadListData->hwndListPopup)
               ResetMenuStyle(pThreadListData->hwndListPopup, parent);
         }
         break;

      case WM_INITMENU:
         if ((HWND) mp2 == pThreadListData->hwndListPopup)
            pThreadListData->pPopupRecord=NULL;
         if ((HWND) mp2 == pThreadListData->hwndListPopup ||
             (HWND) mp2 == pThreadListData->hwndThreadPopup)
         {
            /* Emphasis setzen */
            WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_SETRECORDEMPHASIS,
                              pThreadListData->pPopupRecord,
                              MPFROM2SHORT(TRUE, CRA_SOURCE));
         }
         break;

      case WM_CONTEXTMENU:
         if (!SHORT1FROMMP(mp1) &&
             WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(parent, IDD_THREADLIST+1))
         {
            pThreadListData->bKeyboard = TRUE;
            WinSendDlgItemMsg(parent, IDD_THREADLIST+1, WM_CONTEXTMENU, mp1, mp2);
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
/* Funktionsname: UpdateDspMenu                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt die Checks beim Popup-Menu je nach Display-Modus      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pThreadListData: Instanzdaten                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void UpdateDspMenu(PTHREADLISTDATA pThreadListData)
{
   switch(pThreadListData->dspmode)
   {
      case DSPTHREADS_ALL:
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWALL, TRUE);
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWTHR, FALSE);
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWUNR, FALSE);
         break;

      case DSPTHREADS_WITHUNREAD:
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWALL, FALSE);
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWTHR, TRUE);
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWUNR, FALSE);
         break;

      case DSPTHREADS_UNREADONLY:
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWALL, FALSE);
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWTHR, FALSE);
         WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_VIEWUNR, TRUE);
         break;

      default:
         break;
   }
   return;
}

/*--------------------------- SortThreads          --------------------------*/
/* Sortierfunktion fuer die Thread-Liste                                     */
/*---------------------------------------------------------------------------*/

static SHORT _System SortThreads(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage)
{
   SHORT result;

   /* Compiler beruhigen */
   pStorage=pStorage;

   result=stricmp(((PTHREADRECORD)p1)->pchSubj, ((PTHREADRECORD)p2)->pchSubj);

   if (!result)
   {
      /* Subjects sind gleich, jetzt nach Messagenummer gehen */
      if (((PTHREADRECORD)p1)->ulMsgID < ((PTHREADRECORD)p1)->ulMsgID)
         result=-1;
      else
         result=1;
   }

   return result;
}


/*------------------------------ ExpandBranch   -----------------------------*/
/* Erweitert einen gesamten Unterbaum                                        */
/*---------------------------------------------------------------------------*/

static void ExpandBranch(HWND hwndContainer, PTHREADRECORD pParent)
{
   PTHREADRECORD pChild;

   pChild=(PTHREADRECORD)SendMsg(hwndContainer, CM_QUERYRECORD,
                                    pParent, MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
   while (pChild)
   {
      SendMsg(hwndContainer, CM_EXPANDTREE, pChild, NULL);
      ExpandBranch(hwndContainer, pChild);
      pChild=SendMsg(hwndContainer, CM_QUERYRECORD,
                               pChild, MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
   }
   return;
}

/*------------------------------ ExpandThreads  -----------------------------*/
/* Laedt neue Header beim Aufklappen eines Zweiges                           */
/*---------------------------------------------------------------------------*/

static THREADRECORD *ExpandThreads(HWND hwndContainer, PTHREADRECORD pParent, PTHREADLISTDATA pThreadListData)
{
   PTHREADRECORD pRecord, pRecord2;
   ULONG uid, uid2;
   int msgnum, msgnum2;
   MSGHEADER Header, Header2;
   extern char CurrentArea[LEN_AREATAG+1];
   int i;

   /* ersten Child-Record holen */
   pRecord=pParent;
   SendMsg(hwndContainer, CM_INVALIDATERECORD, &pRecord,
              MPFROM2SHORT(1, CMA_TEXTCHANGED /*| CMA_NOREPOSITION*/));
   pRecord=SendMsg(hwndContainer, CM_QUERYRECORD, pRecord,
                      MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
   if (threadlistoptions.compact && pParent && (pParent->ulFlags & THREADFLAG_THREADSTART))
      pRecord=SendMsg(hwndContainer, CM_QUERYRECORD, pRecord,
                      MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
   while(pRecord)
   {
      if (!(pRecord->ulFlags & THREADFLAG_NOREPLY))   /* Noch nicht bearbeitet */
      {
         pRecord->ulFlags |= THREADFLAG_NOREPLY;

         SendMsg(hwndContainer, CM_INVALIDATERECORD, &pRecord,
                    MPFROM2SHORT(1, CMA_TEXTCHANGED /* | CMA_NOREPOSITION*/));

         /* Nummer der Message holen */
         uid= pRecord->ulMsgID;
         msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, uid, TRUE);
         if (uid && !MSG_ReadHeader(&Header, &arealiste, CurrentArea, msgnum))
         {
            /* Alle Replies bearbeiten */
            i=0;
            while (i<NUM_REPLIES && Header.ulReplies[i]!=0)
            {
               uid2=Header.ulReplies[i++];
               if (uid2==0)
                  continue;   /* Reply nicht mehr da */
               msgnum2=MSG_UidToMsgn(&arealiste, CurrentArea, uid2, TRUE);
               if (MSG_ReadHeader(&Header2, &arealiste, CurrentArea, msgnum2))
                  continue;    /* Fehler beim Lesen */

               if (pThreadListData->dspmode==DSPTHREADS_UNREADONLY
                   && (Header2.ulAttrib & ATTRIB_READ))
                  continue;

               pRecord2=SendMsg(hwndContainer, CM_ALLOCRECORD,
                                   MPFROMLONG(sizeof(THREADRECORD)-sizeof(RECORDCORE)),
                                   MPFROMLONG(1));
               Header2.ulMsgID = uid2;
               HeaderToRecord(pThreadListData, &Header2, pRecord2, TRUE);

               InsertRecords(WinQueryWindow(hwndContainer, QW_PARENT),
                             pRecord, pRecord2, 1, TRUE);
            }
         }
      }
      /* naechsten Record holen, bis Stufe zu Ende */
      pRecord=SendMsg(hwndContainer, CM_QUERYRECORD, pRecord,
                         MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
   }
   return NULL;
}

/*------------------------------ CleanupThreadList --------------------------*/
/* Gibt den Speicher fuer die Textfelder wieder frei, entfernt alle Records  */
/*---------------------------------------------------------------------------*/

static void CleanupThreadList(HWND hwndContainer)
{
   SendMsg(hwndContainer, CM_REMOVERECORD, NULL,
              MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));
   return;
}

/*------------------------------ GetNextRecord     --------------------------*/
/* Liefert Zeiger auf naechsten Record im Tree-View                          */
/*---------------------------------------------------------------------------*/

static PTHREADRECORD GetNextRecord(PTHREADRECORD pRecord, HWND hwndContainer)
{
   PRECORDCORE record =NULL;
   PRECORDCORE recordP=NULL;
   record=(PRECORDCORE) SendMsg(hwndContainer,
                                   CM_QUERYRECORD,
                                   MPFROMP(pRecord),
                                   MPFROM2SHORT( CMA_FIRSTCHILD, CMA_ITEMORDER));
   if (record==NULL)
   {
      /* no child ? - try the twin... */
      record=(PRECORDCORE) SendMsg(hwndContainer,
                                      CM_QUERYRECORD,
                                      MPFROMP(pRecord),
                                      MPFROM2SHORT( CMA_NEXT, CMA_ITEMORDER));
      if (record==NULL)
      {
         /* no twin ? - try to go back and then next ... */
         record=(PRECORDCORE)pRecord;
         while (record!=NULL)
         {
            recordP=(PRECORDCORE)SendMsg(hwndContainer,
                                            CM_QUERYRECORD,
                                            MPFROMP(record),
                                            MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER));
            if (recordP==NULL)   /* no parent - nothing left ? */
               return NULL;
            record=(PRECORDCORE) SendMsg(hwndContainer,
                                            CM_QUERYRECORD,
                                            MPFROMP(recordP),
                                            MPFROM2SHORT( CMA_NEXT, CMA_ITEMORDER));
            if (record==NULL)
               record=recordP;
            else
               return (PTHREADRECORD) record;  /* PARENT -> NEXT */
         }
         return NULL;         /* PARENT -> PARENT */
      }
      else
         return (PTHREADRECORD) record;  /* NEXT */
   }
   else
      return (PTHREADRECORD) record;    /* FIRST_CHILD */
}

/*------------------------------ DrawTree      ------------------------------*/
/* Owner-Draw-Funktion fuer den Thread-Tree                                  */
/*---------------------------------------------------------------------------*/

static BOOL DrawTree(POWNERITEM Item)
{
   if (Item->idItem != CMA_TEXT)
      return FALSE;              /* nur Text zeichnen */

   if (Item->fsAttribute & CRA_SELECTED)
      return FALSE;      /* nur normale Darstellung zeichnen */

   if (!((PCNRDRAWITEMINFO)Item->hItem)->pRecord)
      return FALSE;       /* nur Records zeichnen */

   GpiCreateLogColorTable(Item->hps, 0, LCOLF_RGB, 0, 0, 0);

   if ((((PTHREADRECORD)((PCNRDRAWITEMINFO)Item->hItem)->pRecord)->ulFlags & THREADFLAG_PERSONAL) &&
       !(((PTHREADRECORD)((PCNRDRAWITEMINFO)Item->hItem)->pRecord)->ulFlags & THREADFLAG_READ))
   {
      WinDrawText(Item->hps, -1, ((PCNRDRAWITEMINFO)Item->hItem)->pRecord->pszTree,
                  &Item->rclItem,
                  threadlistoptions.lPersonalClr,
                  threadlistoptions.lBackClr,
                  DT_LEFT | DT_VCENTER);
   }
   else
      if (((PTHREADRECORD)((PCNRDRAWITEMINFO)Item->hItem)->pRecord)->ulFlags & THREADFLAG_READ)   /* gelesen */
      {
         WinDrawText(Item->hps, -1, ((PCNRDRAWITEMINFO)Item->hItem)->pRecord->pszTree,
                     &Item->rclItem,
                     threadlistoptions.lReadClr,
                     threadlistoptions.lBackClr,
                     DT_LEFT | DT_VCENTER);
      }
      else          /* ungelesen */
      {
         WinDrawText(Item->hps, -1, ((PCNRDRAWITEMINFO)Item->hItem)->pRecord->pszTree,
                     &Item->rclItem,
                     threadlistoptions.lUnreadClr,
                     threadlistoptions.lBackClr,
                     DT_LEFT | DT_VCENTER);
      }

   return TRUE;
}

/*--------------------------- InsertUnreadThreads  --------------------------*/
/* Sucht die Message-Threads mit ungelesener Mail und fuegt sie in den       */
/* Container ein.                                                            */
/*---------------------------------------------------------------------------*/

static void _Optlink InsertUnreadThreads(void *p)
{
   int msgnum;
   ULONG maxmsgs;
   AREADEFLIST *zeiger;
   PTHREADHEADERS Headers=NULL;
   extern char CurrentArea[LEN_AREATAG+1];
   extern MISCOPTIONS miscoptions;
   extern DRIVEREMAP driveremap;
   char pchThisArea[LEN_AREATAG+1];
   PTHREADLISTDATA pThreadListData = p;

   INSTALLEXPT("InsertThreads");

   DoingInsert=TRUE;
   StopInsert=FALSE;

   strcpy(pchThisArea, CurrentArea);

   zeiger=AM_FindArea(&arealiste, pchThisArea);

   /* Leere oder falsche Area */
   if (!zeiger || zeiger->maxmessages==0)
   {
      DEINSTALLEXPT;
      return;
   }

   MSG_OpenArea(&arealiste, pchThisArea, miscoptions.lastreadoffset, &driveremap);

   /* Speicher fuer Header belegen */
   maxmsgs=zeiger->maxmessages;
   Headers=calloc(maxmsgs, sizeof(THREADHEADERS));

   /* alle Header lesen */
   for (msgnum=1; msgnum<=maxmsgs && !StopInsert; msgnum++)
   {
      MSG_ReadHeader(&Headers[msgnum-1].Header, &arealiste, pchThisArea, msgnum);
      StripRe(Headers[msgnum-1].Header.pchSubject);
   }

   MSG_CloseArea(&arealiste, pchThisArea, FALSE, miscoptions.lastreadoffset, &driveremap);

   if (!StopInsert)
   {
      /* MSGIDs in Index umwandeln */
      MsgIDsToIndex(Headers, maxmsgs);

      /* einzufuegende Header bestimmen */
      switch(pThreadListData->dspmode)
      {
         case DSPTHREADS_ALL:
            TraceAllThreads(Headers, maxmsgs);
            break;

         case DSPTHREADS_WITHUNREAD:
            TraceUnreadThreads(Headers, maxmsgs, pThreadListData->numbers);
            break;

         case DSPTHREADS_UNREADONLY:
            TraceUnreadMsgs(Headers, maxmsgs, pThreadListData->numbers);
            break;

         default:
            break;
      }

      if (threadlistoptions.compact)
         TraceSubjects(Headers, maxmsgs);

      WinPostMsg(hwndThreadList, TM_THREADSREADY, Headers, MPFROMLONG(maxmsgs));
   }
   else
   {
      free(Headers);
      DoingInsert=FALSE;
   }

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MsgIDsToIndex                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Wandelt die MSGIDs in den Reply-Verweisen in Array-Index    */
/*               um                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: Headers: Header-Array                                          */
/*            maxmsgs: Anzahl der Message-Header im Array                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: ungueltige Verweise werden zu 0                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void MsgIDsToIndex(PTHREADHEADERS Headers, ULONG maxmsgs)
{
   ULONG msgnr, ulSeekID, nr2;
   int reply;

   for (msgnr=0; msgnr<maxmsgs; msgnr++)
   {
      /* replyto suchen */
      ulSeekID=Headers[msgnr].Header.ulReplyTo;
      nr2=0;
      while(nr2<maxmsgs && Headers[nr2].Header.ulMsgID < ulSeekID)
         nr2++;
      if (nr2<maxmsgs && Headers[nr2].Header.ulMsgID == ulSeekID)
         Headers[msgnr].Header.ulReplyTo=nr2+1;
      else
         Headers[msgnr].Header.ulReplyTo=0;

      /* replies absuchen */
      reply=0;
      while (reply<NUM_REPLIES && Headers[msgnr].Header.ulReplies[reply])
      {
         ulSeekID=Headers[msgnr].Header.ulReplies[reply];
         nr2=0;
         while(nr2<maxmsgs && Headers[nr2].Header.ulMsgID < ulSeekID)
            nr2++;
         if (nr2<maxmsgs && Headers[nr2].Header.ulMsgID == ulSeekID)
            Headers[msgnr].Header.ulReplies[reply]=nr2+1;
         else
            Headers[msgnr].Header.ulReplies[reply]=0;
         reply++;
      }
      /* restliche replies niederbuegeln */
      while(reply<NUM_REPLIES)
         Headers[msgnr].Header.ulReplies[reply++]=0;
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TraceAllThreads                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert die Header fuer "Alle Threads"                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: Headers: Header-Array                                          */
/*            maxmsgs: Anzahl der Message-Header im Array                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rueckgabewerte: -                                                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TraceAllThreads(PTHREADHEADERS Headers, ULONG maxmsgs)
{
   ULONG msgnr=0;

   for (msgnr=0; msgnr<maxmsgs; msgnr++)
   {
      if (!Headers[msgnr].Header.ulReplyTo)
         Headers[msgnr].ulFlags=INSERT_PARENT;
      else
         Headers[msgnr].ulFlags=INSERT_CHILD;
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TraceUnreadThreads                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert die Header fuer "unread threads"                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: Headers: Header-Array                                          */
/*            maxmsgs: Anzahl der Message-Header im Array                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TraceUnreadThreads(PTHREADHEADERS Headers, ULONG maxmsgs, PULONG pCurrentNum)
{
   ULONG msgnr, num2, i;

   for (msgnr=0; msgnr<maxmsgs; msgnr++)
   {
      if (! ISREAD(Headers[msgnr]) ||
          (Headers[msgnr].Header.ulMsgID == pCurrentNum[0]))
      {
         /* ungelesene Message */
         num2=msgnr;
         while(Headers[num2].Header.ulReplyTo && !Headers[num2].ulFlags)
         {
            Headers[num2].ulFlags=INSERT_CHILD;
            num2=Headers[num2].Header.ulReplyTo-1;
         }
         if (!Headers[num2].Header.ulReplyTo)
         {
            /* oben angekommen, einsetzen und alle Children auch */
            Headers[num2].ulFlags=INSERT_PARENT;
            for (i=0; i<NUM_REPLIES; i++)
               if (Headers[num2].Header.ulReplies[i])
                  Headers[Headers[num2].Header.ulReplies[i]-1].ulFlags=INSERT_CHILD;
         }
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TraceUnreadMsgs                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Markiert die Header fuer "unread messages"                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: Headers: Header-Array                                          */
/*            maxmsgs: Anzahl der Message-Header im Array                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TraceUnreadMsgs(PTHREADHEADERS Headers, ULONG maxmsgs, PULONG pCurrentNum)
{
   ULONG msgnr, num2, i;

   for (msgnr=0; msgnr<maxmsgs; msgnr++)
   {
      if (! ISREAD(Headers[msgnr]) ||
          (Headers[msgnr].Header.ulMsgID == pCurrentNum[0]))
      {
         /* ungelesene Message */
         num2=msgnr;
         while(Headers[num2].Header.ulReplyTo &&
               (!Headers[Headers[num2].Header.ulReplyTo-1].ulFlags) &&
                !ISREAD(Headers[Headers[num2].Header.ulReplyTo-1]))
         {
            Headers[num2].ulFlags=INSERT_CHILD;
            num2=Headers[num2].Header.ulReplyTo-1;
         }

         if (!Headers[num2].Header.ulReplyTo ||
             ISREAD(Headers[Headers[num2].Header.ulReplyTo-1]))
         {
            /* oben angekommen */
            Headers[num2].ulFlags=INSERT_PARENT;
            for (i=0; i<NUM_REPLIES; i++)
               if (Headers[num2].Header.ulReplies[i] &&
                   !ISREAD(Headers[Headers[num2].Header.ulReplies[i]-1]))
                  Headers[Headers[num2].Header.ulReplies[i]-1].ulFlags=INSERT_CHILD;
         }
         else
            Headers[num2].ulFlags=INSERT_CHILD;
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TraceSubjects                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht die Threads mit gleichem Subject zusammen             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: Headers: Header-Array                                          */
/*            maxmsgs: Anzahl der Message-Header im Array                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TraceSubjects(PTHREADHEADERS Headers, ULONG maxmsgs)
{
   int iStart, iLast, iNext;

   for (iStart = 0; iStart < maxmsgs; iStart++)
   {
      if (Headers[iStart].ulFlags == INSERT_PARENT)
      {
         /* Start eines Threads */
         Headers[iStart].ulFlags = INSERT_THREAD;
         iLast = iStart;

         /* gleiches Subject zusammensuchen */
         for (iNext = iStart+1; iNext < maxmsgs; iNext++)
         {
            if (Headers[iNext].ulFlags == INSERT_PARENT &&
                !stricmp(Headers[iStart].Header.pchSubject,
                         Headers[iNext].Header.pchSubject))
            {
               /* gleiches Subject */
               Headers[iNext].ulFlags = INSERT_THREAD;
               Headers[iLast].ulNextParent = iNext;
               iLast = iNext;
            }
         }
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteMessage                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht eine Message im Baum                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle des Containers                          */
/*            pMessageID: MESSAGEID-Struktur vom Worker-Thread               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void DeleteMessage(HWND hwndCnr, PMESSAGEID pMessageID)
{
   PTHREADRECORD pDelRecord=NULL;

   /* Message im Container suchen */
   pDelRecord=SendMsg(hwndCnr, CM_QUERYRECORD, NULL,
                         MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
   while (pDelRecord && pDelRecord->ulMsgID != pMessageID->ulMsgID)
      pDelRecord=GetNextRecord(pDelRecord, hwndCnr);

   if (pDelRecord)
   {
      PTHREADRECORD pChRecord=pDelRecord;
      PTHREADRECORD pThreadRecord = NULL, pTempRec=pDelRecord;

      if (threadlistoptions.compact)
      {
         /* Subject-Record suchen */
         while (pTempRec = SendMsg(hwndCnr, CM_QUERYRECORD, pTempRec,
                                   MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER)))
         {
            pThreadRecord = pTempRec;
         }
      }

      /* Alle Child-Records abklappern, falls vorhanden */
      while(pChRecord=SendMsg(hwndCnr, CM_QUERYRECORD, pChRecord,
                          MPFROM2SHORT((pChRecord!=pDelRecord)? CMA_NEXT : CMA_FIRSTCHILD,
                                       CMA_ITEMORDER)))
      {
         PTHREADRECORD pNewTop=NULL;

         /* Record auf Top-Level kopieren */
         if (pNewTop=SendMsg(hwndCnr, CM_ALLOCRECORD, MPFROMLONG(sizeof(THREADRECORD)-sizeof(RECORDCORE)),
                                              MPFROMSHORT(1)))
         {
            PTHREADRECORD  pGChRecord=pChRecord;

            pNewTop->RecordCore.flRecordAttr = pChRecord->RecordCore.flRecordAttr;
            pNewTop->RecordCore.pszTree=pNewTop->pchText;
            strcpy(pNewTop->pchText, pChRecord->pchText);
            pNewTop->ulMsgID = pChRecord->ulMsgID;
            pNewTop->ulFlags = pChRecord->ulFlags;
            pNewTop->RecordCore.pTreeItemDesc= pChRecord->RecordCore.pTreeItemDesc;

            InsertRecords(WinQueryWindow(hwndCnr, QW_PARENT), pThreadRecord, pNewTop, 1, FALSE);

            /* Alle Grand-Child-Records abklappern und an Child-Record anhaengen */
            while(pGChRecord=SendMsg(hwndCnr, CM_QUERYRECORD, pGChRecord,
                                    MPFROM2SHORT((pGChRecord!=pChRecord)? CMA_NEXT : CMA_FIRSTCHILD,
                                                 CMA_ITEMORDER)))
            {
               PTHREADRECORD pNewRecord=NULL;

               if (pNewRecord=SendMsg(hwndCnr, CM_ALLOCRECORD, MPFROMLONG(sizeof(THREADRECORD)-sizeof(RECORDCORE)),
                                                        MPFROMSHORT(1)))
               {
                  pNewRecord->RecordCore.flRecordAttr = pGChRecord->RecordCore.flRecordAttr;
                  pNewRecord->RecordCore.pszTree=pNewRecord->pchText;
                  strcpy(pNewRecord->pchText, pGChRecord->pchText);
                  pNewRecord->ulMsgID = pGChRecord->ulMsgID;
                  pNewRecord->ulFlags = pGChRecord->ulFlags;
                  pNewRecord->RecordCore.pTreeItemDesc= pGChRecord->RecordCore.pTreeItemDesc;

                  InsertRecords(WinQueryWindow(hwndCnr, QW_PARENT), pNewTop, pNewRecord, 1, FALSE);
               }
            }
         }
      }

      /* Record loeschen, loescht auch dessen Child-Records */
      SendMsg(hwndCnr, CM_REMOVERECORD, &pDelRecord, MPFROM2SHORT(1, CMA_INVALIDATE | CMA_FREE));

      /* Schauen, ob Thread-Record noch Nachfolger hat, sonst auch loeschen */
      if (pThreadRecord)
      {
         if (!SendMsg(hwndCnr, CM_QUERYRECORD, pThreadRecord, MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER)))
            SendMsg(hwndCnr, CM_REMOVERECORD, &pThreadRecord, MPFROM2SHORT(1, CMA_INVALIDATE | CMA_FREE));
      }
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ThreadAddMessage                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt eine neue Message in den Baum ein                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle des Containers                          */
/*            pMesgID: MESSAGEID-Struktur vom Worker-Thread                  */
/*            pHeader: Neuer Message-Header                                  */
/*            ulDispMode: Display-Mode                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ThreadAddMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader, PTHREADLISTDATA pThreadListData)
{
   PTHREADRECORD pParentRec=NULL, pNewRec=NULL;

   switch(pThreadListData->dspmode)
   {
      case DSPTHREADS_UNREADONLY:
         /* Nichts machen, neue Messages sind immer "read" */
         break;

      case DSPTHREADS_WITHUNREAD:
      case DSPTHREADS_ALL:
         if (pHeader->ulReplyTo)
         {
            /* neue Message ist ein Reply, Parent-Record suchen */
            pParentRec = SendMsg(hwndCnr, CM_QUERYRECORD, NULL,
                                    MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
            while (pParentRec && pParentRec->ulMsgID != pHeader->ulReplyTo)
               pParentRec=GetNextRecord(pParentRec, hwndCnr);

            if (!pParentRec)
               /* nicht gefunden, wird evtl. erst spaeter eingefuegt,
                  die neue Message wird dann autom. beruecksichtigt, nix machen */
               return;
         }
         /* nun haben wir entweder den Parent-Record, oder die neue Msg ist
            kein Reply, Record einfuegen */
         if (threadlistoptions.compact && !pParentRec)
         {
            /* nach Subject suchen */
            pParentRec = SendMsg(hwndCnr, CM_QUERYRECORD, NULL,
                                    MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
            while (pParentRec && stricmp(pParentRec->pchSubj, pHeader->pchSubject))
            {
               pParentRec = SendMsg(hwndCnr, CM_QUERYRECORD, pParentRec,
                                    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
            }

            if (!pParentRec)
            {
               /* Neuen Thread einfuegen */
               if (pParentRec=SendMsg(hwndCnr, CM_ALLOCRECORD, MPFROMLONG(sizeof(THREADRECORD)-sizeof(RECORDCORE)),
                                                        MPFROMSHORT(1)))
               {
                  HeaderToThread(pThreadListData, pHeader, pParentRec, TRUE);

                  InsertRecords(WinQueryWindow(hwndCnr, QW_PARENT), NULL, pParentRec, 1, TRUE);
               }
            }

         }

         if (pNewRec=SendMsg(hwndCnr, CM_ALLOCRECORD, MPFROMLONG(sizeof(THREADRECORD)-sizeof(RECORDCORE)),
                                                  MPFROMSHORT(1)))
         {
            pHeader->ulMsgID = pMsgID->ulMsgID;
            HeaderToRecord(pThreadListData, pHeader, pNewRec, TRUE);

            InsertRecords(WinQueryWindow(hwndCnr, QW_PARENT), pParentRec, pNewRec, 1, TRUE);
         }
         break;

      default:
         break;
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ThreadChangeMessage                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Aendert eine Message in der Threadliste                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Window-Handle des Containers                          */
/*            pMesgID: MESSAGEID-Struktur vom Worker-Thread                  */
/*            pHeader: Neuer Message-Header                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ThreadChangeMessage(HWND hwndCnr, PMESSAGEID pMsgID, MSGHEADER *pHeader, PTHREADLISTDATA pThreadListData)
{
   PTHREADRECORD pRecord=NULL;

   /* Message im Container suchen */
   pRecord=SendMsg(hwndCnr, CM_QUERYRECORD, NULL,
                      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
   while (pRecord && pRecord->ulMsgID != pMsgID->ulMsgID)
      pRecord=GetNextRecord(pRecord, hwndCnr);

   if (pRecord)
   {
      pHeader->ulMsgID = pMsgID->ulMsgID;
      HeaderToRecord(pThreadListData, pHeader, pRecord, TRUE);

      SendMsg(hwndCnr, CM_INVALIDATERECORD, &pRecord,
                 MPFROM2SHORT(1, CMA_TEXTCHANGED));
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ThreadContextMenu                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet CN_CONTEXTMENU vom Container                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window                                         */
/*            pThreadListData: Instanzdaten                                  */
/*            pRecord: Record fuer das Kontext-Menue                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ThreadContextMenu(HWND hwndDlg, PTHREADLISTDATA pThreadListData, PTHREADRECORD pRecord)
{
   RECTL rectl, rectl2;

   if (bDoingWork || pThreadListData->lDisableCount > 0)
   {
      WinAlarm(HWND_DESKTOP, WA_NOTE);
      return;
   }

   if (pRecord &&
       (pRecord->ulFlags & THREADFLAG_THREADSTART))
   {
      WinAlarm(HWND_DESKTOP, WA_NOTE);
      return;
   }

   pThreadListData->pPopupRecord= pRecord;

   if (pRecord)
   {
      QUERYRECORDRECT qRecord;

      qRecord.cb=sizeof(QUERYRECORDRECT);
      qRecord.pRecord=(PRECORDCORE) pRecord;
      qRecord.fRightSplitWindow=FALSE;
      qRecord.fsExtent=CMA_TEXT;
      WinSendDlgItemMsg(hwndDlg, IDD_THREADLIST+1, CM_QUERYRECORDRECT,
                        &rectl, &qRecord);
   }
   else
      WinQueryWindowRect(WinWindowFromID(hwndDlg, IDD_THREADLIST+1), &rectl);

   if (pThreadListData->bKeyboard)
   {
      WinQueryWindowRect(WinWindowFromID(hwndDlg, IDD_THREADLIST+1), &rectl2);
      WinMapWindowPoints(WinWindowFromID(hwndDlg, IDD_THREADLIST+1),
                         HWND_DESKTOP, (PPOINTL) &rectl2, 2);

      if (pRecord)
         WinPopupMenu(HWND_DESKTOP, hwndDlg, pThreadListData->hwndThreadPopup,
                      rectl2.xLeft+rectl.xLeft+(rectl.xRight-rectl.xLeft)/2,
                      rectl2.yBottom+rectl.yBottom+(rectl.yTop-rectl.yBottom)/2,
                      0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
      else
         WinPopupMenu(HWND_DESKTOP, hwndDlg, pThreadListData->hwndListPopup,
                      rectl2.xLeft+(rectl2.xRight-rectl2.xLeft)/2,
                      rectl2.yBottom+rectl.yBottom+(rectl.yTop-rectl.yBottom)/2,
                      0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }
   else
   {
      POINTL pointl;

      WinQueryPointerPos(HWND_DESKTOP, &pointl);
      if (pRecord)
         WinPopupMenu(HWND_DESKTOP, hwndDlg, pThreadListData->hwndThreadPopup,
                      pointl.x, pointl.y,
                      0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
      else
         WinPopupMenu(HWND_DESKTOP, hwndDlg, pThreadListData->hwndListPopup,
                      pointl.x, pointl.y,
                      0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SwitchThreadlistView                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Schaltet den View-Modus der Threadliste um, liest Threads   */
/*               neu ein.                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window                                         */
/*            pThreadListData: Instanzdaten                                  */
/*            sCmdID: ID des Menues                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SwitchThreadlistView(HWND hwndDlg, PTHREADLISTDATA pThreadListData, SHORT sCmdID)
{
   if (DoingInsert)
   {
      StopInsert=TRUE;
      DosWaitThread((PTID) &tidThreadList, DCWW_WAIT);
   }

   switch(sCmdID)
   {
      case IDM_TLP_VIEWALL:
         pThreadListData->dspmode = DSPTHREADS_ALL;
         break;

      case IDM_TLP_VIEWTHR:
         pThreadListData->dspmode = DSPTHREADS_WITHUNREAD;
         break;

      case IDM_TLP_VIEWUNR:
         pThreadListData->dspmode = DSPTHREADS_UNREADONLY;
         break;

      default:
         break;
   }
   UpdateDspMenu(pThreadListData);

   SendMsg(hwndDlg, TM_REREADTHREADS, &pThreadListData->MsgListPar, NULL);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SwitchSenderName                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Schaltet die Namens-Ansicht um                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window                                         */
/*            pThreadListData: Instanzdaten                                  */
/*            bNoUpdate: Vorhandene Records nicht updaten                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void SwitchSenderName(HWND hwndDlg, PTHREADLISTDATA pThreadListData, BOOL bNoUpdate)
{
   PTHREADRECORD pRecord=NULL;
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_THREADLIST+1);

   pThreadListData->bSenderName = !pThreadListData->bSenderName;
   WinCheckMenuItem(pThreadListData->hwndListPopup, IDM_TLP_SENDER, pThreadListData->bSenderName);

   if (!bNoUpdate)
   {
      /* Messages im Container updaten */
      pRecord=SendMsg(hwndCnr, CM_QUERYRECORD, NULL,
                      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
      while (pRecord)
      {
         if (!threadlistoptions.compact || !(pRecord->ulFlags & THREADFLAG_THREADSTART))
            BuildThreadTitle(pRecord->pchName, pRecord->pchSubj,
                             pThreadListData->bSenderName, pRecord->RecordCore.pszTree);

         pRecord=GetNextRecord(pRecord, hwndCnr);
      }
      SendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, NULL);
   }


   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: IsPersonalMessage                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Stellt fest, ob ein Name einer der eigenen Namen ist        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchToName: gesuchter Name                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE Name ist eigener Name                                 */
/*                FALSE Name ist nicht eigener Name                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static BOOL IsPersonalMessage(char *pchToName)
{
   extern USERDATAOPT userdaten;
   int i=0;

   while (i < MAX_USERNAMES && userdaten.username[i][0])
   {
      if (!stricmp(pchToName, userdaten.username[i]))
         return TRUE;
      else
         i++;
   }
   return FALSE;

}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CollectThread                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sammelt die MSGIDs eines Threads.                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pThreadWork: Zeiger auf statische THREADWORK-Struktur          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Benoetigt evtl. viel Stack! 200 KB empfohlen.                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void _Optlink CollectThread(void *pThreadWork)
{
   extern MISCOPTIONS miscoptions;
   extern DRIVEREMAP driveremap;
   PWORKDATA pWorkData = ((PTHREADWORK) pThreadWork)->pWorkData;
   ULONG ulStartID = ((PTHREADWORK) pThreadWork)->ulStartID;
   AREADEFLIST *pSrcArea;

   INSTALLEXPT("CollectThread");

   bDoingWork=TRUE;

   pSrcArea = AM_FindArea(&arealiste, pWorkData->pchSrcArea);

   if (pSrcArea)
   {
      MSG_OpenArea(&arealiste, pWorkData->pchSrcArea, miscoptions.lastreadoffset, &driveremap);

      if (pSrcArea->maxmessages)
      {
         pWorkData->MsgIDArray = malloc(pSrcArea->maxmessages * sizeof(ULONG));
         pWorkData->ulArraySize=0; /* noch leer */
         pWorkData->next=NULL;

         /* UMSGIDs sammeln */
         RecurseThread(ulStartID, pWorkData);

         /* Worker-Thread starten */
         tidWorker = _beginthread(WorkerThread, NULL, 32768, pWorkData);
      }
      else
         bDoingWork=FALSE;
      MSG_CloseArea(&arealiste, pWorkData->pchSrcArea, FALSE, miscoptions.lastreadoffset, &driveremap);
   }
   else
      bDoingWork=FALSE;

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RecurseThread                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sammelt die MSGIDs eines Threads rekursiv                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ulMsgID: Ab dieser Message sammeln                             */
/*            pWorkData: WORKDATA-Struktur                                   */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Benoetigt evtl. viel Stack! 200 KB empfohlen.                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void RecurseThread(ULONG ulMsgID, PWORKDATA pWorkData)
{
   ULONG msgnr;

   /* UMSGID hinzufuegen */
   pWorkData->MsgIDArray[pWorkData->ulArraySize] = ulMsgID;
   pWorkData->ulArraySize++;

   msgnr = MSG_UidToMsgn(&arealiste, pWorkData->pchSrcArea, ulMsgID, TRUE);

   if (msgnr)
   {
      MSGHEADER *pHeader = malloc(sizeof(MSGHEADER));

      if (!MSG_ReadHeader(pHeader, &arealiste, pWorkData->pchSrcArea, msgnr))
      {
         char i=0;

         while (i < NUM_REPLIES && pHeader->ulReplies[i])
         {
            RecurseThread(pHeader->ulReplies[i], pWorkData);
            i++;
         }
      }
      free(pHeader);
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: WorkOnThread                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Leitet die Bearbeitung eines Threads ein                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Window-Handle der Threadliste                         */
/*            pThreadListData: Instanzdaten der Liste                        */
/*            usCmdID: ID von WM_COMMAND                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void WorkOnThread(HWND hwndDlg, PTHREADLISTDATA pThreadListData, USHORT usCmdID)
{
   extern PATHNAMES pathnames;
   extern GENERALOPT generaloptions;
   AREALISTPAR AreaListPar;
   extern ULONG ulExportOptions;
   AREADEFLIST *pArea;

   if (bDoingWork)
   {
      /* Fehlermeldung */
      MessageBox(hwndDlg, IDST_MSG_DOINGWORK, 0, IDD_DOINGWORK,
                 MB_OK);
      return;
   }

   ThreadWork.ulStartID = pThreadListData->pPopupRecord->ulMsgID;
   ThreadWork.pWorkData = malloc(sizeof(WORKDATA));
   memset(ThreadWork.pWorkData , 0, sizeof(WORKDATA));

   ThreadWork.pWorkData->pPrintDest = NULL;
   strcpy(ThreadWork.pWorkData->pchSrcArea, CurrentArea);
   strcpy(ThreadWork.pWorkData->pchDestArea, CurrentArea);
   ThreadWork.pWorkData->ulCopyMove = 0;

   /* Destination-Area ermitteln */
   AreaListPar.cb=sizeof(AREALISTPAR);
   AreaListPar.pchString = NULL;

   switch(usCmdID)
   {
      case IDM_THP_DELETE:
         ThreadWork.pWorkData->flWorkToDo = WORK_DELETE;
         break;

      case IDM_THP_MOVE:
         ThreadWork.pWorkData->flWorkToDo = WORK_MOVE;

         AreaListPar.idTitle = IDST_TITLE_AL_MOVE;
         AreaListPar.ulIncludeTypes = INCLUDE_ALL;
         AreaListPar.bExtendedSel = FALSE;
         AreaListPar.bChange      = FALSE;
         if (generaloptions.LastMoveArea[0])
            AreaListPar.pchString=strdup(generaloptions.LastMoveArea);
         else
            AreaListPar.pchString=strdup(CurrentArea);

         /* ZielArea holen */
         if (WinDlgBox(HWND_DESKTOP, hwndDlg,
                       AreaListProc, hmodLang,
                       IDD_AREALIST, &AreaListPar)!=DID_OK || !AreaListPar.pchString)
         {
            free(ThreadWork.pWorkData);
            return;
         }

         pArea = AM_FindArea(&arealiste, AreaListPar.pchString);
         if (pArea && pArea->areadata.areatype != AREATYPE_LOCAL)
            switch(MessageBox(hwndDlg, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                              IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
            {
               case MBID_YES:
                  ThreadWork.pWorkData->ulCopyMove = COPYMOVE_RESEND;
                  break;

               default:
                  break;
            }

         strcpy(ThreadWork.pWorkData->pchDestArea, AreaListPar.pchString);
         break;

      case IDM_THP_COPY:
         ThreadWork.pWorkData->flWorkToDo = WORK_COPY;

         AreaListPar.idTitle = IDST_TITLE_AL_COPY;
         AreaListPar.ulIncludeTypes = INCLUDE_ALL;
         AreaListPar.bExtendedSel = FALSE;
         AreaListPar.bChange      = FALSE;
         if (generaloptions.LastCopyArea[0])
            AreaListPar.pchString=strdup(generaloptions.LastCopyArea);
         else
            AreaListPar.pchString=strdup(CurrentArea);

         /* ZielArea holen */
         if (WinDlgBox(HWND_DESKTOP, hwndDlg,
                       AreaListProc, hmodLang,
                       IDD_AREALIST, &AreaListPar)!=DID_OK || !AreaListPar.pchString)
         {
            free(ThreadWork.pWorkData);
            return;
         }

         pArea = AM_FindArea(&arealiste, AreaListPar.pchString);
         if (pArea && pArea->areadata.areatype != AREATYPE_LOCAL)
            switch(MessageBox(hwndDlg, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                              IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
            {
               case MBID_YES:
                  ThreadWork.pWorkData->ulCopyMove = COPYMOVE_RESEND;
                  break;

               default:
                  break;
            }

         strcpy(ThreadWork.pWorkData->pchDestArea, AreaListPar.pchString);
         break;

      case IDM_THP_EXPORT:
         ThreadWork.pWorkData->flWorkToDo = WORK_EXPORT;

         strcpy(ThreadWork.pWorkData->pchDestFile, pathnames.lastexport);

         /* Filenamen holen */
         if (GetExportName(hwndDlg, ThreadWork.pWorkData->pchDestFile,
                           &ulExportOptions))
         {
            strcpy(pathnames.lastexport, ThreadWork.pWorkData->pchDestFile);
            ThreadWork.pWorkData->ulExportOptions = ulExportOptions;
         }
         else
         {
            free(AreaListPar.pchString);
            free(ThreadWork.pWorkData);
            return;
         }
         break;

      case IDM_THP_PRINT:
         ThreadWork.pWorkData->flWorkToDo = WORK_PRINT;
         break;

      case IDM_THP_MARK:
         if (generaloptions.safety & SAFETY_CATCHUP)
            if (MessageBox(hwndDlg, IDST_MSG_CATCHUP, IDST_TITLE_CATCHUP,
                           IDD_CATCHUP, MB_YESNO) != MBID_YES)
         {
            free(ThreadWork.pWorkData);
            return;
         }
         ThreadWork.pWorkData->flWorkToDo = WORK_MARK;
         break;

      default:
         free(ThreadWork.pWorkData);
         return;
   }
   free(AreaListPar.pchString);

   _beginthread(CollectThread, NULL, 200000, &ThreadWork);

   return;
}

static void HeaderToRecord(PTHREADLISTDATA pThreadListData, PMSGHEADER pHeader, PTHREADRECORD pRecord, BOOL bStripRe)
{
   char pchDup[LEN_SUBJECT+1];

   if (pHeader->ulReplies[0])
      pRecord->RecordCore.flRecordAttr=CRA_COLLAPSED;
   else
      pRecord->RecordCore.flRecordAttr=0;

   pRecord->ulFlags=0;
   pRecord->RecordCore.pszTree=pRecord->pchText;

   if (bStripRe)
      StripRe(pHeader->pchSubject);
   memcpy(pchDup, pHeader->pchSubject, LEN_SUBJECT+1);
   if (strtok(pchDup, " \t")==NULL)
      memcpy(pHeader->pchSubject, pThreadListData->pchEmptySubj, LEN_SUBJECT+1);
   BuildThreadTitle(pHeader->pchFromName, pHeader->pchSubject,
                    pThreadListData->bSenderName, pRecord->RecordCore.pszTree);
   memcpy(pRecord->pchName, pHeader->pchFromName, LEN_USERNAME+1);
   memcpy(pRecord->pchSubj, pHeader->pchSubject, LEN_SUBJECT+1);
   pRecord->ulMsgID= pHeader->ulMsgID;
   pRecord->RecordCore.pTreeItemDesc=NULL;
   if (pHeader->ulAttrib & ATTRIB_READ)
      pRecord->ulFlags |= THREADFLAG_READ;
   if (IsPersonalMessage(pHeader->pchToName))
      pRecord->ulFlags |= THREADFLAG_PERSONAL;

   return;
}

static void HeaderToThread(PTHREADLISTDATA pThreadListData, PMSGHEADER pHeader, PTHREADRECORD pRecord, BOOL bStripRe)
{
   char pchDup[LEN_SUBJECT+1];

   pRecord->RecordCore.flRecordAttr=CRA_COLLAPSED;
   pRecord->ulFlags=THREADFLAG_THREADSTART | THREADFLAG_READ;
   pRecord->RecordCore.pszTree=pRecord->pchText;

   if (bStripRe)
      StripRe(pHeader->pchSubject);
   memcpy(pchDup, pHeader->pchSubject, LEN_SUBJECT+1);
   if (strtok(pchDup, " \t")==NULL)
      memcpy(pHeader->pchSubject, pThreadListData->pchEmptySubj, LEN_SUBJECT+1);
   memcpy(pRecord->RecordCore.pszTree, pHeader->pchSubject, LEN_SUBJECT+1);
   memcpy(pRecord->pchSubj, pHeader->pchSubject, LEN_SUBJECT+1);
   pRecord->ulMsgID= 0;
   pRecord->RecordCore.pTreeItemDesc=NULL;

   return;
}

static void InsertRecords(HWND hwndDlg, PTHREADRECORD pParent, PTHREADRECORD pInsert, ULONG ulNumInsert, BOOL bInvalidate)
{
   RECORDINSERT RecordInsert;

   RecordInsert.cb=sizeof(RECORDINSERT);
   RecordInsert.pRecordOrder=(PRECORDCORE) CMA_END;
   RecordInsert.pRecordParent=(PRECORDCORE) pParent;
   RecordInsert.fInvalidateRecord=bInvalidate;
   RecordInsert.zOrder=CMA_TOP;
   RecordInsert.cRecordsInsert=ulNumInsert;

   WinSendDlgItemMsg(hwndDlg, IDD_THREADLIST+1, CM_INSERTRECORD, pInsert, &RecordInsert);

   return;
}

static char *BuildThreadTitle(char *pchName, char *pchSubj, BOOL bUseSender, char *pchBuffer)
{
   if (bUseSender)
      sprintf(pchBuffer, "%s (%s)", pchSubj, pchName);
   else
      strcpy(pchBuffer, pchSubj);

   return pchBuffer;
}

static void ThreadsReady(HWND parent, PTHREADLISTDATA pThreadListData, PTHREADHEADERS Headers, ULONG maxmsgs)
{
   PTHREADRECORD pMsgInsert, pFirstMsgInsert, pChildRecords;
   int insnum=0, threadnum=0;
   ULONG i, ulChildCount;
   PTHREADRECORD pRecord, pParentRecord, pThreadRecord=NULL;
   CNRINFO cnrinfo;
   int ThreadTopExpanded=FALSE;

   for (threadnum=0; threadnum<maxmsgs; threadnum++)
   {
      if (Headers[threadnum].ulFlags == (threadlistoptions.compact?INSERT_THREAD:INSERT_PARENT))
      {
         insnum = threadnum;

         if (threadlistoptions.compact)
         {
            /* Thread-Header vorbereiten */
            pThreadRecord=(PTHREADRECORD)WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                              CM_ALLOCRECORD,
                              MPFROMLONG(sizeof(THREADRECORD)-sizeof(RECORDCORE)),
                              MPFROMSHORT(1));
            HeaderToThread(pThreadListData, &Headers[insnum].Header, pThreadRecord, FALSE);
            InsertRecords(parent, NULL, pThreadRecord, 1, FALSE);
         }

         do
         {
            Headers[insnum].ulFlags=0;
            /* Anzahl der Child-Records zaehlen */

            i=0;
            ulChildCount=0;
            while (i<NUM_REPLIES &&
                   Headers[insnum].Header.ulReplies[i])
            {
               if (Headers[Headers[insnum].Header.ulReplies[i]-1].ulFlags == INSERT_CHILD)
                  ulChildCount++;
               i++;
            }
            pFirstMsgInsert=(PTHREADRECORD)WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                              CM_ALLOCRECORD,
                              MPFROMLONG(sizeof(THREADRECORD)-sizeof(RECORDCORE)),
                              MPFROMSHORT(1+ulChildCount));
            HeaderToRecord(pThreadListData, &Headers[insnum].Header, pFirstMsgInsert, FALSE);

            pParentRecord=pFirstMsgInsert;
            pChildRecords=(PTHREADRECORD)pFirstMsgInsert->RecordCore.preccNextRecord;

            InsertRecords(parent, pThreadRecord, pFirstMsgInsert, 1, FALSE);

            /* zweite Stufe (Replies) mit bearbeiten */
            if (ulChildCount)
            {
               pMsgInsert=pChildRecords;

               i=0;
               while (i<NUM_REPLIES &&
                      Headers[insnum].Header.ulReplies[i])
               {
                  ULONG num2;

                  num2=Headers[insnum].Header.ulReplies[i]-1;

                  if (Headers[num2].ulFlags == INSERT_CHILD)
                  {
                     Headers[num2].ulFlags=0;

                     HeaderToRecord(pThreadListData, &Headers[num2].Header, pMsgInsert, FALSE);

                     pMsgInsert=(PTHREADRECORD)pMsgInsert->RecordCore.preccNextRecord;
                  }
                  i++;
               }
               InsertRecords(parent, pParentRecord, pChildRecords, ulChildCount, FALSE);
            }

            /* naechsten record */
            if (threadlistoptions.compact)
               insnum = Headers[insnum].ulNextParent;
            else
               insnum=0;
         } while (insnum);
      }
   }

   free(Headers);
   WinEnableWindowUpdate(WinWindowFromID(parent, IDD_THREADLIST+1), FALSE);
   pRecord=NULL;

   pThreadListData->Trace2=pThreadListData->Trace;
   while(pThreadListData->Trace2)
   {
      /* Message im Container suchen */
      pRecord=WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_QUERYRECORD, NULL,
                         MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
      do
      {
         if (pRecord)
         {
            if (pRecord->ulMsgID == (ULONG) pThreadListData->Trace2->msgid)
               break;
            pRecord=GetNextRecord(pRecord, WinWindowFromID(parent, IDD_THREADLIST+1));
         }
      }  while (pRecord);

      /* Thread expanden */
      if (pRecord)
      {
         if (threadlistoptions.compact && /* erst noch Thread-Record expanden */
             !ThreadTopExpanded &&
             (pThreadRecord = WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                                                CM_QUERYRECORD, pRecord,
                                                MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER))) )
         {
            ThreadTopExpanded = TRUE;
            WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                              CM_EXPANDTREE, pThreadRecord, NULL);
         }

         if (pThreadListData->Trace2->next)
            WinSendDlgItemMsg(parent, IDD_THREADLIST+1,
                              CM_EXPANDTREE, pRecord, NULL);
      }

      /* naechste Message */
      pThreadListData->Trace2=pThreadListData->Trace2->next;
   }
   /* Baum mit der aktuellen Message ist nun voll expanded, */
   /* pRecord zeigt auf die aktuelle Message */

   switch(pThreadListData->dspmode)
   {
      case DSPTHREADS_ALL:
         cnrinfo.pszCnrTitle=pchAllThreads;
         break;

      case DSPTHREADS_WITHUNREAD:
         cnrinfo.pszCnrTitle=pchUnread;
         break;

      case DSPTHREADS_UNREADONLY:
         cnrinfo.pszCnrTitle=pchUnreadOnly;
         break;
   }
   WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_SETCNRINFO, &cnrinfo,
                      MPFROMLONG(CMA_CNRTITLE));

   WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_INVALIDATERECORD, NULL, NULL);
   WinEnableWindowUpdate(WinWindowFromID(parent, IDD_THREADLIST+1), TRUE);

   if (pRecord)
   {
      QUERYRECORDRECT qrecord;
      RECTL rectl, rectl2;

      /* Cursor auf den Record setzen */
      WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_SETRECORDEMPHASIS,
                        pRecord, MPFROM2SHORT(TRUE, CRA_CURSORED));

      /* Zum Record scrollen */
      WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_QUERYVIEWPORTRECT,
                        MPFROMP(&rectl2),
                        MPFROM2SHORT(CMA_WINDOW, FALSE));

      qrecord.cb=sizeof(QUERYRECORDRECT);
      qrecord.pRecord=(PRECORDCORE) pRecord;
      qrecord.fRightSplitWindow=TRUE;
      qrecord.fsExtent=CMA_TEXT;
      if (WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_QUERYRECORDRECT,
                            &rectl, &qrecord))
      {
         /*if (rectl.yBottom)*/
            WinSendDlgItemMsg(parent, IDD_THREADLIST+1, CM_SCROLLWINDOW,
                              MPFROMSHORT(CMA_VERTICAL),
                              MPFROMLONG((rectl2.yTop-rectl2.yBottom)/2-rectl.yBottom));
      }
   }

   DoingInsert = FALSE;
   return;
}

/*-------------------------------- Modulende --------------------------------*/


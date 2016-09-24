/*---------------------------------------------------------------------------+
 | Titel: Fleet Street Messagereader                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von:  Michael Hohner           | Am: 15.04.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM mit CSet++ 2.1                                        |
 +---------------------------------------------------------------------------+
 | Beschreibung:   Messagereader fuer Fido- und Squish-Messagebase           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#pragma strings(readonly)

#define INCL_GPI
#define INCL_WIN
#define INCL_BASE
#define INCL_SPL
#define INCL_SPLDOSPRINT
#include <os2.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "version.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman/areaman.h"
#include "areaman/folderman.h"
#include "dialogs.h"
#include "dialogids.h"
#include "resids.h"
#include "messages.h"
#include "controls/editwin.h"
#include "controls/statline.h"
#include "controls/msgviewer.h"
#include "controls/toolbar.h"
#include "handlemsg/handlemsg.h"
#include "handlemsg/kludgeapi.h"
#include "init.h"
#include "help.h"
#include "areadlg.h"
#include "areascan.h"
#include "mainwindow.h"
#include "msglist.h"
#include "setupdlg.h"
#include "finddlg.h"
#include "pipeserv.h"
#include "utility.h"
#include "cclist.h"
#include "templatedlg.h"
#include "savemsg.h"
#include "fltv7/fltv7.h"
#include "lookups.h"
#include "attachcheck.h"
#include "threadlist.h"
#include "secwin.h"
#include "ccmanage.h"
#include "rxfolder.h"
#include "rexxexec.h"
#include "markmanage.h"
#include "nlbrowser.h"
#include "nickmanage.h"
#include "nicknames.h"
#include "echomanager.h"
#include "printsetup.h"
#include "toolbarconfig.h"
#include "request.h"
#include "request_manage.h"
#include "dump/expt.h"
#include "dump/pmassert.h"
#include "util/addrcnv.h"

/*--------------------------------- Defines ---------------------------------*/

#define REQUIRED_LANGDLL_VERSION  24UL

#define WINDOWTITLE "FleetStreet " FLEETVER
#define WINDOWTITLESEC "FleetStreet " FLEETVER " [*]"

char pchWindowTitle[100];

#define TPLTYPE_NEW      0
#define TPLTYPE_QUOTE    1
#define TPLTYPE_FORWARD  2

/*---------------------------- Globale Variablen ----------------------------*/

/* Fensterhandles */
HAB anchor = 0;
HMQ queue = NULLHANDLE;
HMODULE hmodLang=NULLHANDLE;
HWND frame = NULLHANDLE;
HWND client = NULLHANDLE;
HWND hwndmenu = NULLHANDLE;
HWND hwndhelp = NULLHANDLE;
PFNWP OldFrameProc=NULL;                    /* Original-WP des Frame     */
volatile BOOL issecondinstance=FALSE;
volatile BOOL isregistered=FALSE;
volatile HWND hwndKludge=NULLHANDLE;
volatile HWND hwndAreaDlg=NULLHANDLE;
HACCEL hAccel1=NULLHANDLE;
HACCEL hAccel2=NULLHANDLE;
HACCEL hAccel3=NULLHANDLE;
volatile ULONG ProfileError;
BOOL bProfileSaved=FALSE;
BOOL bNoHook=FALSE;

volatile HWND hwndTemplates = NULLHANDLE;
volatile HWND hwndCCLists = NULLHANDLE;
volatile HWND hwndRxFolder = NULLHANDLE;

/* Nodelist-Browser */
volatile HWND hwndNLBrowser = NULLHANDLE;
volatile BOOL bDoingBrowse = FALSE;

/* Merker fuer den Exit */
BOOL MailEntered[3]={FALSE, FALSE, FALSE};

/* Scan-Thread */
int tidAreaScan=0;
volatile BOOL DoingAreaScan=FALSE;
volatile BOOL StopAreaScan=FALSE;

/* Find-Thread */
int tidFind=0;
volatile HWND hwndFindDlg=NULLHANDLE;
volatile HWND hwndFindResults=NULLHANDLE;
volatile BOOL DoingFind=FALSE;
volatile BOOL StopFind=FALSE;

/* Worker-Thread */
volatile int  tidWorker=0;
volatile BOOL bDoingWork=FALSE;
volatile BOOL bStopWork = FALSE;

/* Thread-Liste */
volatile int tidThreadList=0;
volatile HWND hwndThreadList=NULLHANDLE;
volatile BOOL DoingInsert=FALSE;
volatile BOOL StopInsert=FALSE;

/* Message-Liste */
volatile HWND hwndMsgList=NULLHANDLE;

/* Request-Dialog */
volatile HWND hwndRequester=NULLHANDLE;

volatile BOOL bIgnoreActivate=FALSE;

/* Script-Ausfuehrung */
volatile HWND hwndMonitor=NULLHANDLE;
volatile PRXSCRIPT pExecScript=NULL;
volatile int tidRexxExec=0;

/* Messages schreiben */
BOOL NewMessage=FALSE;
char NewArea[LEN_AREATAG+1]="";
char *pchXPostList=NULL;
ULONG ulCCSelected=0;
char SaveToName[LEN_USERNAME+1];
char SaveToAddress[LEN_5DADDRESS+1];
int QuotedMsgNum=0;
LONG iptInitialPos=0;
LONG iptInitialPos2=0;
BOOL bTemplateProcessed=TRUE;
ULONG ulTemplateType=0;
BOOL bOldMsgLocal=FALSE;

/* Alle Programmoptionen */
AREALIST arealiste;
NICKNAMELIST NickNameList;
USERDATAOPT userdaten;
WINDOWCOLORS windowcolors;
WINDOWFONTS windowfonts;
WINDOWPOSITIONS windowpositions;
PATHNAMES pathnames;
MISCOPTIONS miscoptions;
MACROTABLEOPT macrotable;
GENERALOPT generaloptions;
NODELISTOPT nodelist;
ECHOTOSSOPT echotossoptions;
PDOMAINS domains=NULL;
OUTBOUND outbound[MAX_ADDRESSES];
INTLSETTING intlsetting;
THREADLISTOPTIONS threadlistoptions;
LOOKUPOPTIONS lookupoptions;
RESULTSOPTIONS resultsoptions;
REQUESTOPT requestoptions;
AREALISTOPTIONS arealistoptions;
MSGLISTOPTIONS msglistoptions;
DRIVEREMAP driveremap;
TEMPLATELIST templatelist;
CCANCHOR ccanchor;
SCRIPTLIST scriptlist;
PCCLIST pQuickCCList=NULL;
REXXHOOKS rexxhooks;
FINDJOB FindJob;
MARKERLIST MarkerList;
BOOL bSaveResults;
BROWSEROPTIONS BrowserOptions;
ULONG ulExportOptions;
TOOLBAROPTIONS ToolbarOptions;
ECHOMGROPT EchoMgrOpt;
SEARCHPAR SearchPar;
PRINTSETUP PrintSetup;
TOOLBARCONFIG ToolbarConfig;
OPENWIN OpenWindows;
FOLDERANCHOR FolderAnchor;

/* Derzeitige Message */
FTNMESSAGE CurrentMessage;
MSGHEADER CurrentHeader;
char CurrentAddress[LEN_5DADDRESS+1]="";
char CurrentName[LEN_USERNAME+1]="";
char CurrentArea[LEN_AREATAG+1]="";
int  CurrentStatus=PROGSTATUS_NOSETUP;

int MonoDisp=FALSE;
int TempMono=FALSE;  /* temp. monospaced (wg. NPD-Flag) */

/*--------------------------- Funktionsprototypen ---------------------------*/

static MRESULT EXPENTRY clientwndproc(HWND fenster, ULONG message, MPARAM mp1, MPARAM mp2);
static void ProcessCommands(HWND client, USHORT CommandID);
static BOOL ProcessKeys(HWND client, USHORT usVK);
static MRESULT ProcessWorkMessages(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);

void StartQuickRexx(USHORT menuID);
static void EditMenuCommands(HWND client, ULONG message);
static int SwitchToArea(char *NewArea);
static void ShutdownProgram(void);
static void HandleAttachAttrib(HWND hwndClient);
static int QuoteCurrentMessage(PWINDOWDATA pWindowData, BOOL bDiffArea, ULONG ulReplyDest,
                               ULONG ulOptions);
static int GetReplyArea(PFTNMESSAGE pCurrentMessage, char *pchDestArea);
static void ShowSecondaryWindows(BOOL bShow);
static BOOL SearchInMessage(HWND hwndClient, PSEARCHPAR pSearchPar);

/*---------------------------------- main -----------------------------------*/

int main(int argc, char **argv)
{
   QMSG qmsg;
   HELPINIT hinit;
   char pchModError[100];
   HWND hwndAbout=NULLHANDLE;
   HEV hevInit=NULLHANDLE;
   ULONG ulTimerID=0;
   HWND hwnd;
   PFNWP pPrevProc;
   int rc = RET_NOACTION;
   ULONG ulCurrentCP, ulRet;


   ULONG frameflags= FCF_SIZEBORDER | FCF_MINMAX  | FCF_ICON |
                     FCF_TASKLIST   | FCF_SYSMENU | FCF_TITLEBAR;

   INSTALLEXPT("Main");

   /* erkennen, ob schon eine Instanz laeuft*/
   if (issecondinstance=AlreadyRunning())
      strcpy(pchWindowTitle, WINDOWTITLESEC);
   else
      strcpy(pchWindowTitle, WINDOWTITLE);

   anchor=WinInitialize(0);
   PMASSERT(anchor != NULLHANDLE, "Can't init PM");

   queue=WinCreateMsgQueue(anchor,100);
   PMASSERT(queue != NULLHANDLE, "Can't create msg queue");

   DosQueryCp(sizeof(ulCurrentCP), &ulCurrentCP, &ulRet);
   MSG_SetCPInfo(ulCurrentCP);

   if (!RegisterSecondaryWindows(anchor))
      return RET_ERROR;

   OpenMarkerList(&MarkerList);

   /* Language-DLL laden */
   if (DosLoadModule(pchModError, 100, "FLEETLNG", &hmodLang))
   {
      /* DLL nicht erfolgreich geladen */
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                    "FLEETLNG.DLL not found! Run INSTALL.EXE.",
                    "Fatal error!", 50000, MB_OK | MB_ERROR | MB_MOVEABLE);
      return RET_ERROR;
   }
   else
   {
      ULONG (* _System QueryLangVersion)(void);

      if (DosQueryProcAddr(hmodLang, 0, "FLTLNG_QueryVersion", (PFN *) &QueryLangVersion) ||
          QueryLangVersion() != REQUIRED_LANGDLL_VERSION)
      {
         /* falsche DLL-Version */
         WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                       "The language DLL is not suited for this version"
                       " of FleetStreet. Run INSTALL.EXE to update your installation.",
                       "Fatal error!", 50000, MB_OK | MB_ERROR | MB_MOVEABLE);
         return RET_ERROR;
      }
   }

   if (!WinRegisterClass(anchor,
                         "FleetStreet",
                         clientwndproc,
                         CS_SIZEREDRAW | CS_SYNCPAINT,
                         sizeof(PVOID)))
      return RET_ERROR;       /* Register fehlgeschlagen */

   /* Semaphore fuer Init */
   DosCreateEventSem(NULL, &hevInit, 0, FALSE);

   /* Timer starten f. About-Dialog*/
   ulTimerID=WinStartTimer(anchor, 0, 0, 500);

   if (LogoDisplayEnabled())
   {
      /* About-Dialog anzeigen */
      hwndAbout=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, AboutProc, hmodLang,
                           IDD_HELPABOUT, NULL);

      PMASSERT(hwndAbout != NULLHANDLE, "Can't load about dialog");
      WinShowWindow(hwndAbout, TRUE);
   }

   /* INI-Thread starten */
   ParseArgs(argc, argv);

   if (!_beginthread(ReadIniThread, NULL, 65536, &hevInit))
      return RET_ERROR;

   /* erste Message-Loop */
   while(WinGetMsg(anchor,&qmsg,0,0,0))
      if ((qmsg.msg != WM_TIMER) || (LONGFROMMP((qmsg.mp1)) != ulTimerID))
         WinDispatchMsg(anchor, &qmsg);
      else
      {
         if (!WinWaitEventSem(hevInit, SEM_IMMEDIATE_RETURN))
         {
            WinStopTimer(anchor, 0, ulTimerID);
            break;
         }
      }

   if (hwndAbout)
      WinDestroyWindow(hwndAbout);
   DosCloseEventSem(hevInit);

   CreatePipes();
   InitPrintSetup(&PrintSetup, anchor);

   memset(&CurrentMessage, 0, sizeof(CurrentMessage));

   if (!(windowpositions.mainwindowpos.uchFlags & WINPOS_VALID))
      frameflags |= FCF_SHELLPOSITION;

   if (!(frame=WinCreateStdWindow(HWND_DESKTOP,
                                  0,
                                  &frameflags,
                                  "FleetStreet",
                                  pchWindowTitle,
                                  WS_SYNCPAINT,
                                  NULLHANDLE,       /* Ressourcen */
                                  ID_WINDOW,
                                  &client)))
      return RET_ERROR;       /* Fenster kann nicht erzeugt werden */

#if 0
   AddToWindowList(frame);
#endif

   InitMainWindow(client);

   /* Frame-Window ableiten */
   pPrevProc= WinSubclassWindow(frame, ToolbarFrameProc);
   WinSetWindowPtr(frame, 0, (PVOID) pPrevProc);

   /* Menue laden */
   hwndmenu=WinLoadMenu(frame, hmodLang, IDM_MENU);
   PMASSERT(hwndmenu != NULLHANDLE, "Can't load menu");

   CreateToolbar(frame);
   CreateStatusLine(frame);

   /* Rexx-Menue-Handle holen */
   hwnd = WinWindowFromID(frame, FID_MENU);
   if (hwnd)
   {
      MENUITEM MenuItem;

      if (SendMsg(hwnd, MM_QUERYITEM, MPFROM2SHORT(IDM_REXX, FALSE), &MenuItem))
      {
         PWINDOWDATA pWindowData;

         pWindowData=(PWINDOWDATA) WinQueryWindowULong(client, QWL_USER);
         pWindowData->hwndRexxMenu = MenuItem.hwndSubMenu;
         UpdateRexxMenu(pWindowData->hwndRexxMenu);
      }
   }

   SetFont(hwndmenu, windowfonts.menufont);
   SendMsg(frame, WM_UPDATEFRAME, (MPARAM)FCF_MENU, NULL);

   RestoreWinPos(frame, &windowpositions.mainwindowpos, TRUE, TRUE);

   WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_SETTEXTCOLOR,
                     MPFROMLONG(windowcolors.editfore), NULL);

   /* Helpsystem initialisieren */
   memset(&hinit, 0, sizeof(hinit));
   hinit.cb=sizeof(HELPINIT);
   hinit.phtHelpTable=(PHELPTABLE) MAKEULONG(FLEET_HELP_TABLE, 0xffff);
   hinit.pszHelpWindowTitle="FleetStreet";     /* Titel der Hilfe */
   hinit.fShowPanelId=CMIC_HIDE_PANEL_ID;
   hinit.pszHelpLibraryName="FLTSTRT.HLP";     /* Helpfile */

   if (!(hwndhelp=WinCreateHelpInstance(anchor,&hinit)) ||
       hinit.ulReturnCode)
   {
      char msgbuf[200];

      LoadString(IDST_MSG_NOHELP, 200, msgbuf);

      WinMessageBox(HWND_DESKTOP,
                    HWND_DESKTOP,
                    msgbuf,
                    NULL,
                    IDD_NOHELP,
                    MB_OK | MB_ERROR | MB_MOVEABLE);
   }
   else
      WinAssociateHelpInstance(hwndhelp, frame);

   if (ProfileError)
   {
      if (HandleInitErrors(client, ProfileError) != MBID_OK)
      {
         bProfileSaved = TRUE;
         return RET_ERROR;
      }

      if (ProfileError == INIERROR(INIFILE_NEW))
      {
         WinPostMsg(client, WM_COMMAND, MPFROM2SHORT(IDM_OPCONFIG,0),
                    MPFROM2SHORT(CMDSRC_MENU, 0));

         if (hwndhelp)
            SendMsg(hwndhelp, HM_GENERAL_HELP, NULL, NULL);
      }
   }

   hAccel1=WinLoadAccelTable(anchor, NULLHANDLE, ID_ACCEL1);
   hAccel2=WinLoadAccelTable(anchor, NULLHANDLE, ID_ACCEL2);
   hAccel3=WinLoadAccelTable(anchor, NULLHANDLE, ID_ACCEL3);

   StatusChanged(client, CurrentStatus);
   TplSetIntl(&intlsetting);

   if (MSG_OpenApi(userdaten.address[0]))
      return RET_ERROR;

   if (generaloptions.uselastarea)
      SwitchToArea(miscoptions.lastarearead);
   else
      SwitchToArea(generaloptions.startarea);

   if (generaloptions.scanatstartup)
   {
      MarkAllAreas(&arealiste, FOLDERID_ALL, WORK_SCAN);
      if ((tidAreaScan=_beginthread(ScanAreas, NULL, 32768, &arealiste))==-1)
         WinAlarm(HWND_DESKTOP, WA_ERROR);
   }

   if (!issecondinstance)
   {
      /* Pipe starten */
      _beginthread(NewPipeServer, NULL, 32768, NULL);
   }

   RestoreOpenWindows(&OpenWindows);
   SetFocusControl(client, IDML_MAINEDIT);

   /* Message-Loop */
   do
   {
      WinGetMsg(anchor, &qmsg, 0, 0, 0);
      if (qmsg.msg == WM_QUIT &&
          qmsg.hwnd)
         qmsg.msg = WM_CLOSE;
      WinDispatchMsg(anchor, &qmsg);
   } while(qmsg.msg != WM_QUIT);

   ShutdownProgram();

   MSG_CloseArea(&arealiste, CurrentArea, TRUE, miscoptions.lastreadoffset, &driveremap);
   MSG_CloseApi(&arealiste, &driveremap);

   TermPrintSetup(&PrintSetup);

   if (hwndhelp)
   {
      WinAssociateHelpInstance(NULLHANDLE, hwndhelp);
      WinDestroyHelpInstance(hwndhelp);
   }

   if (hmodLang)
      DosFreeModule(hmodLang);
   if (frame)
      WinDestroyWindow(frame);
   if (queue)
      WinDestroyMsgQueue(queue);
   if (anchor)
      WinTerminate(anchor);

   AM_DeleteAllAreas(&arealiste);
   CleanupDomains(&domains);
   CloseMarkerList(&MarkerList);

   /* Errorlevel erzeugen */
   if (MailEntered[AREATYPE_NET])
      rc += RET_NEWNETMAIL;
   if (MailEntered[AREATYPE_ECHO])
      rc += RET_NEWECHOMAIL;
   if (MailEntered[AREATYPE_LOCAL])
      rc += RET_NEWLOCALMAIL;

   DEINSTALLEXPT;

   return rc;
}


/*------------------------------ clientwndroc  ------------------------------*/
/* Die Fensterprozedur                                                       */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY clientwndproc(HWND fenster, ULONG message, MPARAM mp1, MPARAM mp2)
{
   char chBuffer[LEN_AREATAG+1+20];
   AREADEFLIST *zeiger=NULL;
   static char pchFormatScanning[50];
   REQUESTPAR *pRequestPar=NULL;
   ULONG ulResult=0;
   PWINDOWDATA pWindowData;
   BOOL bInsert;
   MESSAGEID MessageID;
   HWND hwnd;

   pWindowData=(PWINDOWDATA) WinQueryWindowULong(fenster, QWL_USER);

   switch(message)
   {
     /* Bei Drag-Drop aus der Farbpalette */
     case WM_PRESPARAMCHANGED:
        if (((ULONG)mp1)==PP_BACKGROUNDCOLOR) /* andere Aenderungen nicht */
        {                                     /* beachten */
           WinQueryPresParam(fenster,
                             (ULONG)mp1, 0,
                             NULL,
                             sizeof(LONG),
                             (PVOID)&windowcolors.windowback,
                             0);
           WinInvalidateRect(fenster, 0, TRUE);
        }
        return 0;

     case WM_MOUSEMOVE:
        DisplayStatusText(0);
        break;

     case WM_MINMAXFRAME:
        if (((PSWP)mp1)->fl & SWP_MINIMIZE)
        {
           ShowSecondaryWindows(FALSE);
        }
        if (((PSWP)mp1)->fl & (SWP_MAXIMIZE | SWP_RESTORE))
        {
           ShowSecondaryWindows(TRUE);
        }
        break;

     case WM_SAVEAPPLICATION:
        if (!bProfileSaved)
        {
           if (echotossoptions.useechotoss &&
               echotossoptions.pchEchoToss[0])
              WriteEchotoss(&arealiste, echotossoptions.pchEchoToss);

           QueryLayout(fenster);
           QueryOpenWindows(&OpenWindows);
           /* zweite Instanz darf nicht speichern */
           if (!issecondinstance)
           {
              SaveIniProfile(anchor);
              if (arealiste.bDirty)
                 SaveIniAreas(anchor);
           }
        }
        break;


     /* PM soll Hintergrund loeschen */
     case WM_PAINT:
        {
           HPS hps;
           RECTL rectl;

           hps=WinBeginPaint(fenster, NULLHANDLE, &rectl);

           /* Color Table auf RGB umschalten */
           GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);
           WinFillRect(hps, &rectl, windowcolors.windowback);

           WinEndPaint(hps);
        }
        return (MRESULT) FALSE;

     /* Alle Controls einrichten */
     case WM_CREATE:
        LoadString(IDST_FORMAT_SCANNING, 50, pchFormatScanning);
        break;

     /* Menue klappt auf */
     case WM_INITMENU:
        if ((HWND) mp2 != pWindowData->hwndPopup)
           InitMenus(SHORT1FROMMP(mp1), (HWND) mp2);
        break;

     case WM_MENUSELECT:
        if ((HWND) mp2 != pWindowData->hwndPopup)
           DisplayMenuHelp(client, SHORT1FROMMP(mp1));
        break;

     case WM_CONTEXTMENU:
        if (mp2)
        {
           hwnd=WinQueryFocus(HWND_DESKTOP);
           if (hwnd)
              SendMsg(hwnd, message, mp1, mp2);
        }
        break;

     /* Groesse veraendert, anpassen */
     case WM_SIZE:
        FLTLAY_ResizeMainWindow(fenster, SHORT1FROMMP(mp2),SHORT2FROMMP(mp2));
        return (MRESULT) FALSE;

     case WM_CLOSE:
        if (tidRexxExec || bDoingWork || DoingFind || bDoingBrowse)
           return (MRESULT) FALSE;
        MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
        if (CurrentStatus == PROGSTATUS_EDITING)
        {
           if (MessageBox(client, IDST_MSG_EXITNOSAVE, IDST_TITLE_DISCARD,
                          IDD_EXITNOSAVE,
                          MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2)!=MBID_YES)
              return (MRESULT) FALSE;
        }
        if (!bProfileSaved && CurrentStatus != PROGSTATUS_CLEANUP)
        {
           if (echotossoptions.useechotoss &&
               echotossoptions.pchEchoToss[0])
              if (WriteEchotoss(&arealiste, echotossoptions.pchEchoToss))
              {
                 MessageBox(client, IDST_MSG_ETOSSERROR, 0, IDD_ETOSSERROR,
                            MB_ERROR | MB_OK);
                 return (MRESULT) FALSE;
              }

           QueryLayout(fenster);
           QueryOpenWindows(&OpenWindows);
           CurrentStatus = PROGSTATUS_CLEANUP;
           UpdateDisplay(FALSE, FALSE);
           ShutdownProgram();
           StatusChanged(client, CurrentStatus);

           if (!bNoHook && rexxhooks.ulExitID && !StartRexxScript(rexxhooks.ulExitID, NULL))
           {
              pWindowData->ulCurrentHook = CURRENTHOOK_EXIT;
              return (MRESULT) FALSE;
           }

           if (!issecondinstance)
           {
              _beginthread(SaveIniProfileThread, NULL, 32768, &queue);
           }
           else
              break;
        }
        return (MRESULT) FALSE;

     case KM_KLUDGEWNDCLOSE:
        WinDestroyWindow(hwndKludge);
        hwndKludge=NULLHANDLE;
        if (pWindowData->pArea &&
            pWindowData->pArea->maxmessages>0)
           WinSendDlgItemMsg(frame, FID_TOOLBAR, TBM_ENABLECMD, MPFROMLONG(IDB_SHOWKLUDGES), NULL);
        break;

     case FM_FINDRESULTSCLOSE:
        WinDestroyWindow(hwndFindResults);
        hwndFindResults=NULLHANDLE;
        break;

     case MSGLM_CLOSE:
        WinDestroyWindow(hwndMsgList);
        hwndMsgList=NULLHANDLE;
        break;

     case TM_THREADLISTCLOSE:
         WinDestroyWindow(hwndThreadList);
         hwndThreadList=NULLHANDLE;
         break;

     case ALM_CLOSE:
        if (hwndAreaDlg == (HWND) mp1)
        {
           WinDestroyWindow(hwndAreaDlg);
           hwndAreaDlg = NULLHANDLE;
        }
        break;

     case TPLF_CLOSE:
        WinDestroyWindow(hwndTemplates);
        hwndTemplates=NULLHANDLE;
        break;

     case CCFM_CLOSE:
        WinDestroyWindow(hwndCCLists);
        hwndCCLists=NULLHANDLE;
        break;

     case RXF_CLOSE:
        WinDestroyWindow(hwndRxFolder);
        hwndRxFolder=NULLHANDLE;
        break;

     case BRSM_CLOSE:
        if (hwndNLBrowser)
        {
           WinDestroyWindow(hwndNLBrowser);
           hwndNLBrowser=NULLHANDLE;
        }
        break;

     case REXXM_CLOSE:
        WinDestroyWindow((HWND) mp1);
        if (hwndRxFolder)
           SendMsg(hwndRxFolder, REXXM_CLOSE, mp1, mp2);
        hwndMonitor = NULLHANDLE;
        switch (pWindowData->ulCurrentHook)
        {
           case CURRENTHOOK_NONE:
              break;

           case CURRENTHOOK_PRESAVE:
              pWindowData->ulCurrentHook = CURRENTHOOK_NONE;
              bNoHook=TRUE;
              ProcessCommands(client, IDB_OK);
              bNoHook=FALSE;
              break;

           case CURRENTHOOK_EXIT:
              pWindowData->ulCurrentHook = CURRENTHOOK_NONE;
              CurrentStatus = PROGSTATUS_READING;
              bNoHook=TRUE;
              SendMsg(client, WM_CLOSE, NULL, NULL);
              bNoHook=FALSE;
              break;
        }
        break;

     case RXM_UPDATEMENU:
        if (pWindowData->hwndRexxMenu)
           UpdateRexxMenu(pWindowData->hwndRexxMenu);
        break;

     case ALM_SWITCHAREA:
        /* Source-Window */
        pWindowData->ulSourceWindow = SOURCEWIN_AREA;

        if (CurrentStatus == PROGSTATUS_READING &&
            stricmp(CurrentArea, (PCHAR) mp1))
        {
           MSGLISTPAR MsgListPar;


           MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
           strcpy(MessageID.pchAreaTag, CurrentArea);
           MessageID.ulMsgID=pWindowData->ulCurrentID;
           SendMsg(client, WORKM_READ, &MessageID, NULL);

           /* Aktuelle Area schliessen */
           MSG_CloseArea(&arealiste, CurrentArea, TRUE, miscoptions.lastreadoffset, &driveremap);

           /* Neue Area oeffnen */
           NewMessage=FALSE;
           SwitchToArea(mp1);
           MsgListPar.msgnum=MSG_MsgnToUid(&arealiste, CurrentArea, QueryCurrent(&arealiste, CurrentArea));
           pWindowData->ulCurrentID= MsgListPar.msgnum;
           if (hwndThreadList)
              SendMsg(hwndThreadList, TM_REREADTHREADS, &MsgListPar, NULL);
           if (hwndMsgList)
              SendMsg(hwndMsgList, TM_REREADTHREADS, &MsgListPar, NULL);
        }
        break;

     case WM_DESTROY:
        if (pWindowData->hwndPopup)
           WinDestroyWindow(pWindowData->hwndPopup);
        break;

     case WM_CONTROLPOINTER:
        DisplayStatusText(SHORT1FROMMP(mp1));
        break;

     case WM_COMMAND:
        ProcessCommands(fenster, SHORT1FROMMP(mp1));
        return (MRESULT) FALSE;

     case WM_CONTROL:
        switch (SHORT1FROMMP(mp1))
        {
           case FID_STATUSLINE:
              switch(SHORT2FROMMP(mp1))
              {
                 case STLN_CONTEXTMENU:
                    break;

                 case STLN_OPEN:
                    if ((ULONG) mp2 == pWindowData->idAddressField)
                       ProcessCommands(client, IDM_OPNAMEADDR);
                    if ((ULONG) mp2 == pWindowData->idCheckField &&
                        CurrentStatus == PROGSTATUS_READING)
                       ProcessCommands(client, IDA_MARKTOGGLE);
                    if ((ULONG) mp2 == pWindowData->idProgressField)
                       ProcessCommands(client, IDA_CANCEL);
                    break;

                 default:
                    break;
              }
              break;

           case FID_TOOLBAR:
              switch(SHORT2FROMMP(mp1))
              {
                 case TBN_CONTEXTMENU:
                    OpenToolbarContext(client, pWindowData, (ULONG) mp2);
                    break;

                 default:
                    break;
              }
              break;

           case IDE_TOADDRESS:
              if (SHORT2FROMMP(mp1)==EN_SETFOCUS)
                 SendMsg((HWND)mp2, EM_SETSEL, MPFROM2SHORT(0, LEN_5DADDRESS+1),
                            NULL);
              if (SHORT2FROMMP(mp1)==EN_KILLFOCUS && !ulCCSelected && !pQuickCCList &&
                  CurrentStatus == PROGSTATUS_EDITING)
              {
                 char pchTemp[LEN_5DADDRESS+1];
                 FTNADDRESS tempAddr, tempAddr2;
                 int iMatch;

                 WinQueryWindowText((HWND)mp2, LEN_5DADDRESS+1, pchTemp);
                 StringToNetAddr(pchTemp, &tempAddr, CurrentAddress);
                 WinSetWindowText((HWND)mp2, NetAddrToString(pchTemp, &tempAddr));
                 WinQueryWindowText(WinWindowFromID(client, IDE_FROMADDRESS), LEN_5DADDRESS+1, pchTemp);
                 StringToNetAddr(pchTemp, &tempAddr2, NULL);
                 iMatch = MSG_MatchAddress(&tempAddr, &userdaten, &tempAddr2);
                 if (iMatch>=0)
                    WinSetDlgItemText(client, IDE_FROMADDRESS, userdaten.address[iMatch]);
              }
              if (SHORT2FROMMP(mp1) == EN_CONTEXTMENU)
              {
                 pWindowData->usPopupControl = SHORT1FROMMP(mp1);

                 OpenEditPopup(client, pWindowData, (BOOL) mp2);
                 return 0;
              }
              if (SHORT2FROMMP(mp1) == EN_NODEDROPPED && mp2)
              {
                 PCHAR pchTemp=strchr((PCHAR)mp2, ' ');
                 if (pchTemp)
                 {
                    pchTemp++;
                    WinSetDlgItemText(client, IDE_TONAME, pchTemp);
                 }
              }
              break;

           case IDE_FROMADDRESS:
              if (SHORT2FROMMP(mp1)==EN_SETFOCUS)
                 SendMsg((HWND)mp2, EM_SETSEL, MPFROM2SHORT(0, LEN_5DADDRESS+1),
                            NULL);
              if (SHORT2FROMMP(mp1)==EN_KILLFOCUS && CurrentStatus == PROGSTATUS_EDITING)
              {
                 char pchTemp[LEN_5DADDRESS+1];
                 FTNADDRESS tempAddr;

                 WinQueryWindowText((HWND)mp2, LEN_5DADDRESS+1, pchTemp);
                 StringToNetAddr(pchTemp, &tempAddr, CurrentAddress);
                 WinSetWindowText((HWND)mp2, NetAddrToString(pchTemp, &tempAddr));
              }
              if (SHORT2FROMMP(mp1) == EN_CONTEXTMENU)
              {
                 pWindowData->usPopupControl = SHORT1FROMMP(mp1);

                 OpenEditPopup(client, pWindowData, (BOOL) mp2);
                 return 0;
              }
              if (SHORT2FROMMP(mp1) == EN_NODEDROPPED && mp2)
              {
                 PCHAR pchTemp=strchr((PCHAR)mp2, ' ');
                 if (pchTemp)
                 {
                    pchTemp++;
                    WinSetDlgItemText(client, IDE_FROMNAME, pchTemp);
                 }
              }
              break;

           case IDE_SUBJTEXT:
              if (SHORT2FROMMP(mp1) == EN_CONTEXTMENU)
              {
                 pWindowData->usPopupControl = SHORT1FROMMP(mp1);

                 OpenEditPopup(client, pWindowData, (BOOL) mp2);
                 return 0;
              }
              if (SHORT2FROMMP(mp1) == EN_FILEATTACH)
              {
                 CurrentHeader.ulAttrib &= ~ATTRIB_FILEATTACHED;
                 WinPostMsg(client, WM_COMMAND, MPFROMSHORT(IDA_TOGGLE_FILE),
                            MPFROM2SHORT(CMDSRC_ACCELERATOR, FALSE));
                 return 0;
              }
              break;

           case IDE_FROMNAME:
           case IDE_TONAME:
              if (SHORT2FROMMP(mp1) == EN_CONTEXTMENU)
              {
                 pWindowData->usPopupControl = SHORT1FROMMP(mp1);

                 OpenEditPopup(client, pWindowData, (BOOL) mp2);
                 return 0;
              }
              if (SHORT2FROMMP(mp1) == EN_NODEDROPPED && mp2)
              {
                 PCHAR pchTemp=strdup((PCHAR)mp2);
                 PCHAR pchTemp2=strchr(pchTemp, ' ');
                 if (pchTemp2)
                 {
                    *pchTemp2=0;
                    if (SHORT1FROMMP(mp1) == IDE_FROMNAME)
                       WinSetDlgItemText(client, IDE_FROMADDRESS, pchTemp);
                    else
                       WinSetDlgItemText(client, IDE_TOADDRESS, pchTemp);
                 }
                 free(pchTemp);
              }
              break;

           case IDML_MAINEDIT:
              if (SHORT2FROMMP(mp1)==MLN_SETFOCUS)
              {
                 if (!bTemplateProcessed)
                 {
                    char pchTemp[LEN_5DADDRESS+1];

                    /* Template fertig verarbeiten und einsetzen */
                    WinQueryDlgItemText(client, IDE_FROMNAME, LEN_USERNAME, CurrentHeader.pchFromName);
                    if (!ulCCSelected && !pQuickCCList)
                       WinQueryDlgItemText(client, IDE_TONAME, LEN_USERNAME, CurrentHeader.pchToName);
                    else
                       CurrentHeader.pchToName[0]='\0';
                    WinQueryDlgItemText(client, IDE_FROMADDRESS, LEN_5DADDRESS, pchTemp);
                    StringToNetAddr(pchTemp, &CurrentHeader.FromAddress, CurrentAddress);
                    WinQueryDlgItemText(client, IDE_TOADDRESS, LEN_5DADDRESS, pchTemp);
                    StringToNetAddr(pchTemp, &CurrentHeader.ToAddress, CurrentAddress);
                    switch(ulTemplateType)
                    {
                       case TPLTYPE_NEW:
                          MSG_NewMessageStep2(&templatelist, &arealiste, NewArea,
                                              &CurrentMessage, &CurrentHeader, &iptInitialPos);
                          break;

                       case TPLTYPE_QUOTE:
                          MSG_QuoteMessageStep2(&templatelist, &CurrentMessage, &CurrentHeader, &arealiste,
                                                CurrentArea, NewArea,
                                                CurrentName, &iptInitialPos);
                          break;

                       case TPLTYPE_FORWARD:
                          MSG_ForwardMessageStep2(&templatelist, &CurrentMessage, &CurrentHeader,
                                                  &arealiste, NewArea, CurrentName, &iptInitialPos);
                          break;

                       default:
                          break;
                    }
                    DisplayMsgText(client, &CurrentMessage);
                    WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_SETSEL,
                                      MPFROMLONG(iptInitialPos+iptInitialPos2),
                                      MPFROMLONG(iptInitialPos+iptInitialPos2));
                    bTemplateProcessed=TRUE;
                 }
              }
              if (SHORT2FROMMP(mp1)==MLN_CONTEXTMENU)
              {
                 pWindowData->usPopupControl = SHORT1FROMMP(mp1);


                 OpenEditPopup(client, pWindowData, (BOOL) mp2);
                 return 0;
              }
              if (SHORT2FROMMP(mp1)==MLN_FILEDROPPED)
              {
                 /* File importieren */
                 switch(ImportFile(client, (PCHAR) mp2,
                        !(pWindowData->pArea->areadata.ulAreaOpt & AREAOPT_HIGHASCII), FALSE))
                 {
                    /* Lesefehler */
                    case 1:
                       MessageBox(client, IDST_MSG_ERRORLOAD, 0, IDD_ERRORLOAD,
                                  MB_OK | MB_ERROR);
                       break;

                    /* Falscher Filename usw. */
                    case 2:
                       MessageBox(client, IDST_MSG_INVALIDFILE, 0, IDD_INVALIDFILE,
                                  MB_OK | MB_ERROR);
                       break;

                    /* File leer */
                    case 3:
                       MessageBox(client, IDST_MSG_EMPTYFILE, 0, IDD_EMPTYFILE,
                                  MB_OK | MB_ERROR);
                       break;

                    default:
                       break;
                 }
                 free(mp2);
              }
              if (SHORT2FROMMP(mp1)==MSGVN_PRINTCURRENT)
              {
                 PPRINTDEST pPrintDest = (PPRINTDEST) mp2;
                 PWORKDATA pWorkData;

                 /* Aktuelle Message drucken, mp2 ist PPRINTDEST */
                 if (CurrentStatus==PROGSTATUS_READING &&
                     pWindowData->pArea &&
                     pWindowData->pArea->maxmessages > 0)
                 {
                    pWorkData=malloc(sizeof(WORKDATA));
                    PMASSERT(pWorkData != NULL, "Out of memory");

                    pWorkData->next=NULL;
                    pWorkData->MsgIDArray=malloc(sizeof(ULONG));
                    PMASSERT(pWorkData->MsgIDArray != NULL, "Out of memory");
                    pWorkData->MsgIDArray[0] = pWindowData->ulCurrentID;
                    pWorkData->ulArraySize = 1;
                    pWorkData->flWorkToDo=WORK_PRINT;
                    pWorkData->pPrintDest = malloc(pPrintDest->cb);
                    PMASSERT(pWorkData->pPrintDest != NULL, "Out of memory");
                    memcpy(pWorkData->pPrintDest, pPrintDest, pPrintDest->cb);
                    strcpy(pWorkData->pchSrcArea, CurrentArea);
                    bDoingWork=TRUE;
                    _beginthread(WorkerThread, NULL, 32768, pWorkData);
                    Notify(client, IDST_NOT_PRINTED);
                 }
              }
              if (SHORT2FROMMP(mp1)==MSGVN_DISCARDCURRENT)
              {
                 /* Aktuelle Message loeschen */
                 if (CurrentStatus==PROGSTATUS_READING &&
                     pWindowData->pArea &&
                     pWindowData->pArea->maxmessages > 0)
                 {
                    int rc;

                    if (generaloptions.safety & SAFETY_SHREDMSG)
                    {
                       if (MessageBox(client, IDST_MSG_DELETE, IDST_TITLE_DELETE,
                                      IDD_DELETE, MB_YESNO | MB_ICONEXCLAMATION)!=MBID_YES)
                          break;
                    }
                    /* Momentane Message loeschen */
                    rc=MSG_KillMessage(&arealiste, CurrentArea, 0, &driveremap, SendKillMessage);
                    if (rc != OK &&
                        rc != NO_MESSAGE)
                       WinAlarm(HWND_DESKTOP, WA_ERROR);
                 }
              }
              if (SHORT2FROMMP(mp1)==MSGVN_EXPORTCURRENT)
              {
                 /* Aktuelle Message exportieren, mp2 PDRAGTRANSFER */
                 PRENDERPAR pRenderPar=mp2;
                 ULONG ulOptions = 0x01 | 0x02;

                 ExportFile(client, pRenderPar->pchFileName, FALSE, &ulOptions);
                 free(pRenderPar->pchFileName);

                 SendMsg(WinWindowFromID(client, IDML_MAINEDIT),
                            MLM_EXPORTED, pRenderPar->pDragTransfer, NULL);
              }
              if (SHORT2FROMMP(mp1)==MLN_CURSORPOS)
              {
                 char pchTemp[20];
                 PWINDOWDATA pWindowData;

                 sprintf(pchTemp, "%d:%d", SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
                 pWindowData=(PWINDOWDATA)WinQueryWindowULong(client, QWL_USER);
                 WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                                   MPFROMLONG(pWindowData->idCursorField),
                                   pchTemp);
              }
              break;

           default:
              break;
        }
        break;

     case SM_AREASCANNED:
        sprintf(chBuffer, pchFormatScanning, (char *) mp1);
        WinSetDlgItemText(frame, FID_STATUSLINE, chBuffer);
        break;

     case SM_SCANENDED:
        if (hwndAreaDlg)
           SendMsg(hwndAreaDlg, message, mp1, mp2);
        WinSetDlgItemText(frame, FID_STATUSLINE, "");
        break;

     case HM_QUERY_KEYS_HELP:
        return (MRESULT) PANEL_KEYS;

     case WM_CHAR:
        if ((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
            !(SHORT1FROMMP(mp1) & KC_KEYUP))
           if (SHORT2FROMMP(mp2) == VK_TAB ||
               SHORT2FROMMP(mp2) == VK_BACKTAB ||
               SHORT2FROMMP(mp2) == VK_NEWLINE ||
               SHORT2FROMMP(mp2) == VK_ENTER)
              return (MRESULT) ProcessKeys(client, SHORT2FROMMP(mp2));

        if (CurrentStatus == PROGSTATUS_EDITING)
        {
           bInsert=(BOOL)WinQuerySysValue(HWND_DESKTOP, SV_INSERTMODE);
           if (bInsert != pWindowData->bInsert)
           {
              if (bInsert)
                 WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                                   MPFROMLONG(pWindowData->idKeybField),
                                   "INS");
              else
                 WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                                   MPFROMLONG(pWindowData->idKeybField),
                                   "OVR");
           }
           pWindowData->bInsert=bInsert;
        }
        return (MRESULT) FALSE;

     case WM_ACTIVATE:
        if (mp1)
           WinAssociateHelpInstance(hwndhelp, frame);
        else
        {
           WinAssociateHelpInstance(hwndhelp, NULLHANDLE);

           if (CurrentStatus == PROGSTATUS_READING && MSG_IsApiOpen() &&
               !bIgnoreActivate)
           {
              MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);

              strcpy(MessageID.pchAreaTag, CurrentArea);
              MessageID.ulMsgID=pWindowData->ulCurrentID;
              SendMsg(client, WORKM_READ, &MessageID, NULL);
           }
        }
        break;

     case FM_JUMPTOMESSAGE:
        if (CurrentStatus != PROGSTATUS_READING)
        {
           WinAlarm(HWND_DESKTOP, WA_WARNING);
           break;
        }
        if (zeiger=AM_FindArea(&arealiste, ((PMESSAGEID)mp1)->pchAreaTag))
        {
           ULONG ulMsgNum;
           PJUMPINFO pJumpInfo = (PJUMPINFO) mp2;

           /* Source-Window vermerken */
           pWindowData->ulSourceWindow = SOURCEWIN_BOOKMARKS;

           MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);

           if (stricmp(CurrentArea, ((PMESSAGEID)mp1)->pchAreaTag))
           {
              MSGLISTPAR MsgListPar;

              /* Aktuelle Area schliessen */
              MSG_CloseArea(&arealiste, CurrentArea, TRUE, miscoptions.lastreadoffset, &driveremap);

              /* Neue Area oeffnen */
              SwitchToArea(((PMESSAGEID)mp1)->pchAreaTag);
              MsgListPar.msgnum=MSG_MsgnToUid(&arealiste, CurrentArea, QueryCurrent(&arealiste, CurrentArea));
              if (hwndThreadList)
                 SendMsg(hwndThreadList, TM_REREADTHREADS, &MsgListPar, NULL);
              if (hwndMsgList)
                 SendMsg(hwndMsgList, TM_REREADTHREADS, &MsgListPar, NULL);
           }
           /* gefundene Message lesen */
           ulMsgNum=MSG_UidToMsgn(&arealiste, CurrentArea, ((PMESSAGEID)mp1)->ulMsgID, TRUE);
           if (ulMsgNum == 0)
           {
              MessageBox(client, IDST_MSG_NOTTHERE, 0, IDD_NOTTHERE,
                         MB_OK | MB_ERROR);
              break;
           }
           zeiger->currentmessage=ulMsgNum;
           MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
           pWindowData->ulCurrentID=((PMESSAGEID)mp1)->ulMsgID;
           WinEnableWindowUpdate(WinWindowFromID(client, IDML_MAINEDIT), FALSE);
           UpdateDisplay(FALSE, TRUE);
           if (pJumpInfo)
           {
              if ((pJumpInfo->ulWhere & FINDWHERE_TEXT) &&
                  ((pJumpInfo->ulHow & FINDHOW_METHOD_MASK) == FINDHOW_CASE ||
                   (pJumpInfo->ulHow & FINDHOW_METHOD_MASK) == FINDHOW_SENS))
                 WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_FINDTEXT, pJumpInfo->pchText,
                                   MPFROMLONG(((pJumpInfo->ulHow & FINDHOW_METHOD_MASK)==FINDHOW_CASE)?FALSE:TRUE));
           }
           WinEnableWindowUpdate(WinWindowFromID(client, IDML_MAINEDIT), TRUE);
        }
        else
           MessageBox(client, IDST_MSG_NOTTHERE, 0, IDD_NOTTHERE,
                      MB_OK | MB_ERROR);
        break;

     case TM_JUMPTOMESSAGE:
        if (CurrentStatus != PROGSTATUS_READING)
        {
           WinAlarm(HWND_DESKTOP, WA_WARNING);
           break;
        }
        if (pWindowData->pArea && pWindowData->pArea->areahandle)
        {
           int msgnum;

           /* Source-Window vermerken */
           if (mp2)
              pWindowData->ulSourceWindow = SOURCEWIN_MSGLIST;
           else
              pWindowData->ulSourceWindow = SOURCEWIN_THREAD;

           MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
           strcpy(MessageID.pchAreaTag, CurrentArea);
           MessageID.ulMsgID=pWindowData->ulCurrentID;
           SendMsg(client, WORKM_READ, &MessageID, NULL);

           msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, (ULONG) mp1, TRUE);
           if (msgnum)
           {
              pWindowData->pArea->currentmessage=msgnum;
              MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
              pWindowData->ulCurrentID= (ULONG) mp1;
              UpdateDisplay(FALSE, TRUE);
           }
        }
        break;

     case REQM_REQUEST:
        pRequestPar=(REQUESTPAR*)mp1;
        if (pRequestPar->pReqList)
        {
           int rc;

           if (pRequestPar->bDirect)
              rc=MSG_RequestDirect(pRequestPar->pchReqAddr, userdaten.address[0],
                                   pRequestPar->pReqList, outbound, &driveremap,
                                   pRequestPar->ulAttrib);
           else
              rc=MSG_RequestFiles(&arealiste, pRequestPar->pchDestArea,
                                  pRequestPar->pchReqAddr,
                                  pRequestPar->pchReqName,
                                  pRequestPar->pReqList,
                                  pRequestPar->ulAttrib,
                                  CurrentName, CurrentAddress,
                                  &userdaten, &generaloptions,
                                  miscoptions.lastreadoffset,
                                  &driveremap, &templatelist, SendAddMessage);
           if (rc)
              WinAlarm(HWND_DESKTOP, WA_ERROR);
           else
           {
              Notify(fenster, IDST_NOT_REQUESTED);

              zeiger=AM_FindArea(&arealiste, pRequestPar->pchDestArea);
              if (zeiger)
                 zeiger->mailentered = TRUE;
              MailEntered[zeiger->areadata.areatype] = TRUE;
           }
        }
        break;

     case REQM_CLOSE:
        pRequestPar=(REQUESTPAR*)mp1;
        ulResult=(ULONG) mp2;
        if (ulResult==DID_OK)
        {
           if (stricmp(requestoptions.pchDestArea, pRequestPar->pchDestArea))
           {
              strcpy(requestoptions.pchDestArea, pRequestPar->pchDestArea);
              requestoptions.bDirty=TRUE;
           }
           if (requestoptions.bDirectReq != pRequestPar->bDirect)
           {
              requestoptions.bDirectReq = pRequestPar->bDirect;
              requestoptions.bDirty=TRUE;
           }
           if (requestoptions.ulAttrib != pRequestPar->ulAttrib)
           {
              requestoptions.ulAttrib = pRequestPar->ulAttrib;
              requestoptions.bDirty=TRUE;
           }
        }
        SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
        FreeRequestList(pRequestPar->pReqList);
        free(pRequestPar);
        WinDestroyWindow(hwndRequester);
        hwndRequester=NULLHANDLE;
        break;

     case WORKM_ERROR:
     case WORKM_DELETED:
     case WORKM_MOVED:
     case WORKM_COPIED:
     case WORKM_PRINTED:
     case WORKM_EXPORTED:
     case WORKM_ADDED:
     case WORKM_CHANGED:
     case WORKM_READ:
     case WORKM_END:
     case WORKM_DISABLEVIEWS:
     case WORKM_ENABLEVIEWS:
     case WORKM_REREAD:
     case WORKM_SHOWMSG:
     case WORKM_AREADEFCHANGED:
     case WORKM_AREASCANNED:
     case WORKM_MARKEND:
     case WORKM_STARTWORKAREA:
     case WORKM_PROGRESS:
     case WORKM_STARTFIND:
     case WORKM_FINDAREA:
     case WORKM_FINDPROGRESS:
     case WORKM_FINDAREAEND:
     case WORKM_STOPFIND:
     case WORKM_MSGMARKED:
     case WORKM_MSGUNMARKED:
     case WORKM_TRACKMSG:
     case WORKM_SWITCHACCELS:
        return (ProcessWorkMessages(client, message, mp1, mp2));

     case REXXM_STOPSCRIPT:
     case REXXM_ERROR:
        if (hwndMonitor)
           SendMsg(hwndMonitor, message, mp1, mp2);
        else
           if (message == REXXM_ERROR)
              DisplayRexxError(client, (PCHAR) mp1, (PCHAR) mp2);

        switch (pWindowData->ulCurrentHook)
        {
           case CURRENTHOOK_NONE:
              break;

           case CURRENTHOOK_PRESAVE:
              pWindowData->ulCurrentHook = CURRENTHOOK_NONE;
              bNoHook=TRUE;
              ProcessCommands(client, IDB_OK);
              bNoHook=FALSE;
              break;

           case CURRENTHOOK_EXIT:
              pWindowData->ulCurrentHook = CURRENTHOOK_NONE;
              CurrentStatus = PROGSTATUS_READING;
              bNoHook=TRUE;
              tidRexxExec=0;
              SendMsg(client, WM_CLOSE, NULL, NULL);
              bNoHook=FALSE;
              break;
        }
        tidRexxExec=0;
        SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
        break;

     case REXXM_OUTLINE:
     case REXXM_STARTSCRIPT:
        if (hwndMonitor)
           SendMsg(hwndMonitor, message, mp1, mp2);
        break;

     /* Alle anderen Messages */
     default:
        break;
   }
   return WinDefWindowProc(fenster,message,mp1,mp2);
}

static void ProcessCommands(HWND client, USHORT CommandID)
{
   CURRNAMEPAR CurrNamePar;
   AREALISTPAR AreaListPar;
   CCSELECTPAR CCSelectPar;
   REQUESTPAR *pRequestPar;
   PWINDOWDATA pWindowData;
   MESSAGEID MessageID;
   AREADEFLIST *zeiger=NULL;
   REPLYLISTPAR ReplyListPar;
   MSGLISTPAR   MsgListPar;
   ATTRIBPAR AttribPar;
   REPLYPAR ReplyPar;
   int msgnum;


   pWindowData=(PWINDOWDATA) WinQueryWindowULong(client, QWL_USER);

   switch(CommandID)
   {
      case IDB_CHANGEATTR:
      case IDA_ATTRIB:
         AttribPar.cb=sizeof(ATTRIBPAR);
         MSG_QueryAttribCaps(&arealiste, NewMessage?NewArea:CurrentArea, &AttribPar.ulAttribMask);
         AttribPar.ulAttrib=CurrentHeader.ulAttrib;
         AttribPar.bKeepRead = TRUE;
         if (WinDlgBox(HWND_DESKTOP,
                       client,
                       AttribProc,
                       hmodLang,
                       IDD_ATTRIB,
                       &AttribPar)==DID_OK)
         {
            /* Attribute uebernehmen */
            CurrentHeader.ulAttrib=AttribPar.ulAttrib;

            HandleAttachAttrib(client);
         }
         break;

      case IDB_FIND:
      case IDM_MSGFIND:
      case IDA_FIND:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            hwndFindDlg=WinLoadDlg(HWND_DESKTOP,
                                   client,
                                   FindProc,
                                   hmodLang,
                                   IDD_FINDTEXT,
                                   NULL);
            WinProcessDlg(hwndFindDlg);
            WinDestroyWindow(hwndFindDlg);
            hwndFindDlg=NULLHANDLE;
         }
         break;

      case IDB_CUT:
      case IDM_EDITCUT:
         EditMenuCommands(client, IDM_EDITCUT);
         break;

      case IDB_COPY:
      case IDM_EDITCOPY:
         EditMenuCommands(client, IDM_EDITCOPY);
         break;

      case IDB_PASTE:
      case IDM_EDITPASTE:
         EditMenuCommands(client, IDM_EDITPASTE);
         break;

      case IDM_EDITUNDO:
      case IDM_EDITCLEAR:
      case IDM_EDITDELLINE:
         EditMenuCommands(client, CommandID);
         break;

      case IDB_AREA:
      case IDA_AREA:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (!hwndAreaDlg)
            {
               AreaListPar.cb=sizeof(AREALISTPAR);
               AreaListPar.pchString=strdup(CurrentArea);
               AreaListPar.idTitle = 0;
               AreaListPar.ulIncludeTypes = INCLUDE_ALL;
               AreaListPar.bExtendedSel = TRUE;
               AreaListPar.bChange      = TRUE;

               hwndAreaDlg=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                                      AreaListProc, hmodLang,
                                      IDD_AREALIST, &AreaListPar);
               WinShowWindow(hwndAreaDlg, TRUE);
            }
            else
            {
               SWP swp;

               SetFocusControl(hwndAreaDlg, IDD_AREALIST+1);
               WinQueryWindowPos(hwndAreaDlg, &swp);
               if (swp.fl & SWP_MINIMIZE)
                  WinSetWindowPos(hwndAreaDlg,
                                  NULLHANDLE,
                                  0, 0, 0, 0,
                                  SWP_RESTORE);
            }
         }
         break;

      case IDB_MSGLIST:
      case IDA_MSGLIST:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            if (!hwndMsgList)
            {
               if (pWindowData->pArea)
               {
                  MsgListPar.cb=sizeof(MSGLISTPAR);
                  MsgListPar.msgnum=pWindowData->pArea->currentmessage;

                  hwndMsgList= WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                                          MsgListProc, hmodLang,
                                          IDD_MSGLIST, &MsgListPar);
                  SetFocusControl(hwndMsgList, IDD_MSGLIST+1);
               }
            }
            else
            {
               SWP swp;

               SetFocusControl(hwndMsgList, IDD_MSGLIST+1);
               WinQueryWindowPos(hwndMsgList, &swp);
               if (swp.fl & SWP_MINIMIZE)
                  WinSetWindowPos(hwndMsgList,
                                  NULLHANDLE,
                                  0, 0, 0, 0,
                                  SWP_RESTORE);
            }
         }
         break;

      case IDB_REPLY:
      case IDM_MSGREPLY:
         ReplyPar.cb=sizeof(REPLYPAR);
         if (MSG_FindKludge(&CurrentMessage, KLUDGE_AREA, NULL))
            ReplyPar.diffarea = TRUE;
         else
            ReplyPar.diffarea = FALSE;
         if (MSG_FindKludge(&CurrentMessage, KLUDGE_FWDFROM, NULL))
            ReplyPar.isfwd = TRUE;
         else
            ReplyPar.isfwd = FALSE;
         if (WinDlgBox(HWND_DESKTOP, client,
                       ReplyProc, hmodLang, IDD_REPLY,
                       &ReplyPar)!=DID_OK)
            break;

         QuoteCurrentMessage(pWindowData, ReplyPar.diffarea, ReplyPar.replydest, ReplyPar.quoteoptions);
         break;

      case IDB_NEXTMSG:
      case IDA_NEXTMSG:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (!pWindowData->pArea->maxmessages || (pWindowData->pArea->maxmessages && (pWindowData->pArea->currentmessage == pWindowData->pArea->maxmessages)))
            {
               ShowFleetWindow(IDM_WINAREAS);
            }
            else
            {
               if (MSG_ReadNext(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea))
                  WinAlarm(HWND_DESKTOP, WA_WARNING);
               pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
               UpdateDisplay(FALSE, TRUE);
            }
         }
         break;

      case IDB_PREVMSG:
      case IDA_PREVMSG:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (MSG_ReadPrev(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea))
               WinAlarm(HWND_DESKTOP, WA_WARNING);
            pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
            UpdateDisplay(FALSE, TRUE);
         }
         break;

      case IDB_LASTMSG:
      case IDA_LASTMSG:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (pWindowData->pArea &&  (pWindowData->pArea->maxmessages > 0))
            {
               pWindowData->pArea->currentmessage=pWindowData->pArea->maxmessages;
               if(MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea))
                  WinAlarm(HWND_DESKTOP, WA_WARNING);
               pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
               UpdateDisplay(FALSE, TRUE);
            }
         }
         break;

      case IDB_FIRSTMSG:
      case IDA_FIRSTMSG:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (pWindowData->pArea && (pWindowData->pArea->maxmessages > 0))
            {
               pWindowData->pArea->currentmessage=1;
               if (MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea))
                  WinAlarm(HWND_DESKTOP, WA_WARNING);
               pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
               UpdateDisplay(FALSE, TRUE);
            }
         }
         break;

      case IDB_PREVREPLY:
      case IDA_PREVREPLY:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (pWindowData->pArea)
            {
               msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, CurrentHeader.ulReplyTo, TRUE);
               if (msgnum)
               {
                  pWindowData->pArea->currentmessage=msgnum;
                  MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                  pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
                  UpdateDisplay(FALSE, TRUE);
               }
               else
                  if (hwndThreadList)
                     SetFocusControl(hwndThreadList, IDD_THREADLIST+1);
                  else
                     WinAlarm(HWND_DESKTOP, WA_WARNING);
            }
            else
               WinAlarm(HWND_DESKTOP, WA_WARNING);
         }
         break;

      case IDB_NEXTREPLY:
      case IDA_NEXTREPLY:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (CurrentHeader.ulReplies[1]==0)  /* nur ein Reply da */
               ReplyListPar.Selection=CurrentHeader.ulReplies[0];
            else
            {
               ReplyListPar.cb=sizeof(REPLYLISTPAR);
               ReplyListPar.pHeader=&CurrentHeader;
               if (WinDlgBox(HWND_DESKTOP,
                         client,
                         ReplyListProc,
                         hmodLang,
                         IDD_REPLYLIST,
                         &ReplyListPar)!=DID_OK)
                  break;
            }
            if (pWindowData->pArea)
            {
               msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, ReplyListPar.Selection, TRUE);
               if (msgnum)
               {
                  pWindowData->pArea->currentmessage=msgnum;
                  MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                  pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
                  UpdateDisplay(FALSE, TRUE);
               }
               else
                  if (hwndThreadList)
                     SetFocusControl(hwndThreadList, IDD_THREADLIST+1);
                  else
                     WinAlarm(HWND_DESKTOP, WA_WARNING);
            }
            else
               WinAlarm(HWND_DESKTOP, WA_WARNING);
         }
         break;

      case IDB_SHOWKLUDGES:
      case IDA_KLUDGES:
         if ((CurrentStatus == PROGSTATUS_READING ||
              CurrentStatus == PROGSTATUS_EDITING) &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            if (!hwndKludge)
            {
               hwndKludge=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                                     KludgesProc,
                                     hmodLang,
                                     IDD_KLUDGEINFO,
                                     NULL);
               SendMsg(hwndKludge, KM_SHOWKLUDGES,
                          MPFROMP(&CurrentMessage), NULL);
               WinShowWindow(hwndKludge, TRUE);
            }
            else
            {
               SWP swp;

               SetFocus(hwndKludge);
               WinQueryWindowPos(hwndKludge, &swp);
               if (swp.fl & SWP_MINIMIZE)
                  WinSetWindowPos(hwndKludge,
                                  NULLHANDLE,
                                  0, 0, 0, 0,
                                  SWP_RESTORE);
            }
         }
         break;

      case IDB_DELMSG:
      case IDM_MSGDELETE:
      case IDA_DELMSG:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            int rc;

            if (generaloptions.safety & SAFETY_DELMSG)
            {
               if (MessageBox(client, IDST_MSG_DELETE, IDST_TITLE_DELETE,
                              IDD_DELETE, MB_YESNO | MB_ICONEXCLAMATION)!=MBID_YES)
                  break;
            }
            /* Momentane Message loeschen */
            rc=MSG_KillMessage(&arealiste, CurrentArea, 0, &driveremap, SendKillMessage);
            if (rc != OK &&
                rc != NO_MESSAGE)
               WinAlarm(HWND_DESKTOP, WA_ERROR);
         }
         break;

      case IDB_EDITMSG:
      case IDM_MSGCHANGE:
      case IDA_CHANGEMSG:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            if ((generaloptions.safety & SAFETY_EDITSENT) &&
                ((CurrentHeader.ulAttrib & ATTRIB_SENT) || !(CurrentHeader.ulAttrib & ATTRIB_LOCAL)))
            {
               if (MessageBox(client, IDST_MSG_BEENSENT, IDST_TITLE_BEENSENT,
                              IDD_BEENSENT, MB_YESNO | MB_ICONQUESTION)!=MBID_YES)
                  break;
            }
            if (CurrentHeader.ulAttrib & ATTRIB_LOCAL)
               bOldMsgLocal=TRUE;
            else
               bOldMsgLocal=FALSE;
            NewMessage=FALSE;
            SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
            strcpy(NewArea, CurrentArea);
            SwitchEditor(client, NewArea, TRUE);
            CurrentStatus = PROGSTATUS_EDITING;
            DisplayMsgText(client, &CurrentMessage);
            StatusChanged(client, CurrentStatus);
            SetFocusControl(client, IDML_MAINEDIT);
         }
         break;

      case IDB_NEWMSG:
      case IDM_MSGNEW:
      case IDA_ENTER:
         if (CurrentStatus == PROGSTATUS_READING &&
             CurrentArea[0] )
         {
            NewMessage=TRUE;
            SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);
            strcpy(NewArea, CurrentArea);
            MSG_NewMessage(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea,
                           CurrentName, CurrentAddress, &iptInitialPos);
            bTemplateProcessed=FALSE;
            ulTemplateType=TPLTYPE_NEW;
            iptInitialPos2=0;

            SwitchEditor(client, NewArea, TRUE);
            CurrentStatus = PROGSTATUS_EDITING;
            UpdateDisplay(FALSE, FALSE);
            StatusChanged(client, CurrentStatus);
            SetFocusControl(client, IDE_TONAME);
         }
         break;

      case IDB_MSGTREE:
      case IDA_THREADLIST:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            MsgListPar.cb=sizeof(MSGLISTPAR);
            MsgListPar.msgnum=MSG_MsgnToUid(&arealiste, CurrentArea, QueryCurrent(&arealiste, CurrentArea));

            if (hwndThreadList)
            {
               SWP swp;

               SetFocusControl(hwndThreadList, IDD_THREADLIST+1);
               WinQueryWindowPos(hwndThreadList, &swp);
               if (swp.fl & SWP_MINIMIZE)
                  WinSetWindowPos(hwndThreadList,
                                  NULLHANDLE,
                                  0, 0, 0, 0,
                                  SWP_RESTORE);
            }
            else
            {
               MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
               hwndThreadList=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                                         ThreadListProc,
                                         hmodLang, IDD_THREADLIST,
                                         &MsgListPar);
               WinShowWindow(hwndThreadList, TRUE);
            }
         }
         break;

      case IDB_IMPORT:
      case IDM_FILEIMPORT:
      case IDA_IMPORT:
         if (CurrentStatus==PROGSTATUS_EDITING)
            switch(ImportFile(client, pathnames.lastimport,
                   !(pWindowData->pArea->areadata.ulAreaOpt & AREAOPT_HIGHASCII), TRUE))
            {
               /* Lesefehler */
               case 1:
                  MessageBox(client, IDST_MSG_ERRORLOAD, 0, IDD_ERRORLOAD,
                             MB_OK | MB_ERROR);
                  break;

               /* Falscher Filename usw. */
               case 2:
                  MessageBox(client, IDST_MSG_INVALIDFILE, 0, IDD_INVALIDFILE,
                             MB_OK | MB_ERROR);
                  break;

               /* File leer */
               case 3:
                  MessageBox(client, IDST_MSG_EMPTYFILE, 0, IDD_EMPTYFILE,
                             MB_OK | MB_ERROR);
                  break;

               default:
                  break;
            }
         break;

      case IDB_EXPORT:
      case IDM_FILEEXPORT:
      case IDA_EXPORT:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);
            switch(ExportFile(client, pathnames.lastexport, TRUE, &ulExportOptions))
            {
               /* Alles OK */
               case 0:
                  Notify(client, IDST_NOT_EXPORTED);
                  break;

               /* Fehler beim Schreiben */
               case 1:
                  MessageBox(client, IDST_MSG_ERRORSAVE, 0, IDD_ERRORSAVE,
                             MB_OK | MB_ERROR);
                  break;

               /* Falscher Filename */
               case 2:
                  MessageBox(client, IDST_MSG_INVALIDFILENAME, 0, IDD_INVALIDFILENAME,
                             MB_OK | MB_ERROR);
                  break;

               /* Abbruch */
               case 3:
                  break;

               default:
                  break;
            }
            SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
         }
         break;

      case IDB_PRINTMSG:
      case IDM_FILEPRINT:
      case IDA_PRINT:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            PWORKDATA pWorkData;

            pWorkData=malloc(sizeof(WORKDATA));
            PMASSERT(pWorkData != NULL, "Out of memory");

            pWorkData->next=NULL;
            pWorkData->MsgIDArray=malloc(sizeof(ULONG));
            PMASSERT(pWorkData->MsgIDArray != NULL, "Out of memory");
            pWorkData->MsgIDArray[0] = pWindowData->ulCurrentID;
            pWorkData->ulArraySize = 1;
            pWorkData->flWorkToDo=WORK_PRINT;
            pWorkData->pPrintDest=NULL;
            strcpy(pWorkData->pchSrcArea, CurrentArea);
            bDoingWork=TRUE;
            _beginthread(WorkerThread, NULL, 32768, pWorkData);
            Notify(client, IDST_NOT_PRINTED);
         }
         break;

      case IDB_CANCEL:
      case IDA_CANCEL:
         if (CurrentStatus == PROGSTATUS_EDITING)
         {
            if (generaloptions.safety & SAFETY_DISCARD)
            {
               if (MessageBox(client, IDST_MSG_CANCEL, IDST_TITLE_CANCEL,
                              IDD_DISCARD, MB_YESNO | MB_ICONQUESTION)!=MBID_YES)
                  break;
            }
            NewMessage=FALSE;
            SwitchEditor(client, CurrentArea, FALSE);
            CurrentStatus = PROGSTATUS_READING;
            StatusChanged(client, CurrentStatus);
            MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
            UpdateDisplay(TRUE, TRUE);
            if (pchXPostList)
            {
               free(pchXPostList);
               pchXPostList=NULL;
            }
            if (ulCCSelected)
            {
               ulCCSelected=0;
               WinSendDlgItemMsg(client, IDE_TONAME, EM_SETREADONLY, (MPARAM) FALSE, NULL);
               WinSendDlgItemMsg(client, IDE_TOADDRESS, EM_SETREADONLY, (MPARAM) FALSE, NULL);
            }
            if (pQuickCCList)
            {
               while(pQuickCCList->pEntries)
                  DeleteCCEntry(NULL, pQuickCCList, pQuickCCList->pEntries);
               free(pQuickCCList->pchListName);
               free(pQuickCCList);
               pQuickCCList=NULL;
            }
            SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
         }
         else
            if (CurrentStatus == PROGSTATUS_READING)
            {
               bStopWork=TRUE;
               StopAreaScan = TRUE;
            }
         break;

      case IDB_OK:
      case IDA_SAVE:
         if (pWindowData->ulCurrentHook != CURRENTHOOK_NONE)
            break;
         if (!bNoHook && rexxhooks.ulPreSaveID && !StartRexxScript(rexxhooks.ulPreSaveID, NULL))
         {
            pWindowData->ulCurrentHook = CURRENTHOOK_PRESAVE;
            break;
         }

         if (NewMessage)
         {
            if (pchXPostList)
            {
               if (!SaveCrosspostMessage(client, &CurrentMessage, &CurrentHeader,
                                         NewArea, &pchXPostList))
               {
                  SwitchEditor(client, CurrentArea, FALSE);
                  CurrentStatus = PROGSTATUS_READING;
                  StatusChanged(client, CurrentStatus);
                  MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                  NewMessage=FALSE;
                  UpdateDisplay(TRUE, TRUE);
                  free(pchXPostList);
                  pchXPostList=NULL;
               }
               else
                  break;
            }
            else
            {
               if (ulCCSelected || pQuickCCList)
               {
                  PCCLIST pList;

                  if (ulCCSelected)
                     pList = QueryCCList(&ccanchor, ulCCSelected);
                  else
                     pList = pQuickCCList;

                  if (!pList)
                  {
                     MessageBox(client, IDST_MSG_NOCCLIST, 0, IDD_NOCCLIST,
                                MB_OK | MB_ERROR);
                     ulCCSelected=0;
                     WinSendDlgItemMsg(client, IDE_TONAME, EM_SETREADONLY, (MPARAM) FALSE, NULL);
                     WinSendDlgItemMsg(client, IDE_TOADDRESS, EM_SETREADONLY, (MPARAM) FALSE, NULL);
                     WinSetDlgItemText(client, IDE_TOADDRESS, SaveToAddress);
                     WinSetDlgItemText(client, IDE_TONAME, SaveToName);
                     break;
                  }

                  if (!pList->pEntries)
                  {
                     MessageBox(client, IDST_MSG_NOCCENTRIES, 0, IDD_NOCCENTRIES,
                                MB_OK | MB_ERROR);
                     break;
                  }

                  if (!SaveCCMessage(client, &CurrentMessage, &CurrentHeader,
                                     NewArea, pList))
                  {
                     SwitchEditor(client, CurrentArea, FALSE);
                     ulCCSelected=0;
                     if (pQuickCCList)
                     {
                        while(pQuickCCList->pEntries)
                           DeleteCCEntry(NULL, pQuickCCList, pQuickCCList->pEntries);
                        free(pQuickCCList->pchListName);
                        free(pQuickCCList);
                        pQuickCCList=NULL;
                     }
                     CurrentStatus = PROGSTATUS_READING;
                     StatusChanged(client, CurrentStatus);
                     MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                     NewMessage=FALSE;
                     UpdateDisplay(TRUE, TRUE);
                  }
                  else
                     break;
               }
               else
               {
                  if (!SaveMessage(client, &CurrentMessage, &CurrentHeader, NewArea))
                  {
                     /* Switch back to reading mode */
                     SwitchEditor(client, CurrentArea, FALSE);
                     CurrentStatus = PROGSTATUS_READING;
                     StatusChanged(client, CurrentStatus);
                     MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                     NewMessage=FALSE;
                     UpdateDisplay(TRUE, TRUE);
                  }
                  else
                     break;
               }
            }

            pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
            SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
         }
         else
         {
            int rc;

            if (CurrentHeader.ulAttrib & (ATTRIB_SENT | ATTRIB_SCANNED))
            {
               if (MessageBox(client, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                              IDD_RESEND, MB_YESNO | MB_ICONQUESTION)==MBID_YES)
                  CurrentHeader.ulAttrib &= ~(ATTRIB_SENT | ATTRIB_SCANNED);
            }

            switch(rc = ChangeMessage(client, &CurrentMessage, &CurrentHeader, CurrentArea, bOldMsgLocal))
            {
               /* Alles OK */
               case 0:
                  SwitchEditor(client, CurrentArea, FALSE);
                  CurrentStatus = PROGSTATUS_READING;
                  StatusChanged(client, CurrentStatus);
                  NewMessage=FALSE;
                  MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                  pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
                  UpdateDisplay(FALSE, TRUE);
                  break;

               default:
                  SaveErrorMessage(client, rc);
                  return;
            }
            SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
         }
         NewMessage=FALSE;
         break;

      case IDM_EXIT:
      case IDA_END:
         WinPostMsg(client, WM_CLOSE, NULL, NULL);
         break;

      case IDM_FILESHELL:
      case IDB_SHELL:
         bIgnoreActivate = TRUE;
         StartShell();
         bIgnoreActivate = FALSE;
         break;

      case IDM_FILEPRINTSETUP:
         PrintSetup.cb = sizeof(PrintSetup);
         WinDlgBox(HWND_DESKTOP, client, PrintSetupProc, hmodLang, IDD_PRINTSETUP,
                   &PrintSetup);
         break;

      case IDM_HELPABOUT:
         WinDlgBox(HWND_DESKTOP,
                   client,
                   AboutProc,
                   hmodLang,
                   IDD_HELPABOUT,
                   NULL);
         break;

      case IDM_HELPKEYS:
         if (hwndhelp)
            SendMsg(hwndhelp, HM_KEYS_HELP, NULL, NULL);
         break;

      case IDM_HELPUSING:
         if (hwndhelp)
            SendMsg(hwndhelp, HM_DISPLAY_HELP, NULL, NULL);
         break;

      case IDM_HELPINDEX:
         if (hwndhelp)
            SendMsg(hwndhelp, HM_HELP_INDEX, NULL, NULL);
         break;

      case IDM_HELPCONTENTS:
         if (hwndhelp)
            SendMsg(hwndhelp, HM_HELP_CONTENTS, NULL, NULL);
         break;

      case IDB_HELP:
      case IDM_HELPGENERAL:
         if (hwndhelp)
            SendMsg(hwndhelp, HM_GENERAL_HELP, NULL, NULL);
         break;

      case IDM_OPCONFIG:
         MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
         strcpy(MessageID.pchAreaTag, CurrentArea);
         MessageID.ulMsgID=pWindowData->ulCurrentID;
         SendMsg(client, WORKM_READ, &MessageID, NULL);

         /* Area-Scan stoppen */
         if (DoingAreaScan)
         {
            StopAreaScan=TRUE;
            DosWaitThread((PTID) &tidAreaScan, DCWW_WAIT);
         }
         /* Find stoppen */
         if (DoingFind)
         {
            StopFind=TRUE;
            DosWaitThread((PTID) &tidFind, DCWW_WAIT);
         }
         /* Thread-Listen-Thread beenden */
         if (DoingInsert)
         {
            StopInsert=TRUE;
            DosWaitThread((PTID) &tidThreadList, DCWW_WAIT);
         }
         if (hwndThreadList)
         {
            WinDestroyWindow(hwndThreadList);
            hwndThreadList=NULLHANDLE;
         }
         if (hwndMsgList)
         {
            WinDestroyWindow(hwndMsgList);
            hwndMsgList=NULLHANDLE;
         }
         if (hwndAreaDlg)
         {
            WinDestroyWindow(hwndAreaDlg);
            hwndAreaDlg = NULLHANDLE;
         }
         if (hwndRxFolder)
         {
            WinDestroyWindow(hwndRxFolder);
            hwndRxFolder = NULLHANDLE;
         }
         if (hwndFindResults)
         {
            WinDestroyWindow(hwndFindResults);
            hwndFindResults = NULLHANDLE;
         }
         if (hwndNLBrowser)
         {
            WinDestroyWindow(hwndNLBrowser);
            hwndNLBrowser = NULLHANDLE;
         }
         if (hwndRequester)
            SendMsg(hwndRequester, WM_CLOSE, NULL, NULL);
         /* Api schliessen, alle Handles freigeben */
         MSG_CloseApi(&arealiste, &driveremap);

         /* Farben f. Editor */
         QueryBackground(WinWindowFromID(client, IDML_MAINEDIT), &windowcolors.viewerback);
         QueryForeground(WinWindowFromID(client, IDML_MAINEDIT), &windowcolors.viewerfore);
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_QUERYCOLOR,
                           MPFROMLONG(MSGVCLR_QUOTE),
                           MPFROMLONG(&windowcolors.viewerquote));
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_QUERYCOLOR,
                           MPFROMLONG(MSGVCLR_TEARLINE),
                           MPFROMLONG(&windowcolors.viewertearline));
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_QUERYCOLOR,
                           MPFROMLONG(MSGVCLR_ORIGIN),
                           MPFROMLONG(&windowcolors.viewerorigin));
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_QUERYCOLOR,
                           MPFROMLONG(MSGVCLR_QUOTE2),
                           MPFROMLONG(&windowcolors.viewerquote2));

         WinDlgBox(HWND_DESKTOP,
                   client,
                   OptionsProc,
                   hmodLang,
                   IDD_SETUPOPTIONS,
                   NULL);
         if (CurrentStatus == PROGSTATUS_NOSETUP &&
             (userdaten.address[0][0] && userdaten.username[0][0]))
         {
            CurrentStatus = PROGSTATUS_READING;
            StatusChanged(client, CurrentStatus);
         }
         if (CurrentStatus == PROGSTATUS_READING &&
             (!userdaten.address[0][0] || !userdaten.username[0][0]))
         {
            CurrentStatus = PROGSTATUS_NOSETUP;
            StatusChanged(client, CurrentStatus);
         }

         /* Farben f. Editor */
         WinEnableWindowUpdate(WinWindowFromID(client, IDML_MAINEDIT), FALSE);
         SetBackground(WinWindowFromID(client, IDML_MAINEDIT), &windowcolors.viewerback);
         SetForeground(WinWindowFromID(client, IDML_MAINEDIT), &windowcolors.viewerfore);
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_SETCOLOR,
                           MPFROMLONG(MSGVCLR_QUOTE),
                           MPFROMLONG(windowcolors.viewerquote));
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_SETCOLOR,
                           MPFROMLONG(MSGVCLR_TEARLINE),
                           MPFROMLONG(windowcolors.viewertearline));
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_SETCOLOR,
                           MPFROMLONG(MSGVCLR_ORIGIN),
                           MPFROMLONG(windowcolors.viewerorigin));
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_SETCOLOR,
                           MPFROMLONG(MSGVCLR_QUOTE2),
                           MPFROMLONG(windowcolors.viewerquote2));

         WinEnableWindowUpdate(WinWindowFromID(client, IDML_MAINEDIT), TRUE);

         /* Api wieder oeffnen */
         MSG_OpenApi(userdaten.address[0]);
         SwitchToArea(CurrentArea);
         NewMessage=FALSE;
         break;

      case IDM_OPTEMPLATE:
         if (hwndTemplates)
            SetFocusControl(hwndTemplates, IDD_TPLFOLDER+1);
         else
            hwndTemplates = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, TemplateFolderProc, hmodLang,
                                       IDD_TPLFOLDER, NULL);
         break;

      case IDM_OPNICKNAMES:
         WinDlgBox(HWND_DESKTOP, client, AdrBookProc, hmodLang, IDD_NICKNAMES, NULL);
         break;

      case IDM_MSGMOVE:
      case IDB_MOVEMSG:
      case IDA_MOVE:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            AreaListPar.cb=sizeof(AREALISTPAR);
            if (generaloptions.LastMoveArea[0])
               AreaListPar.pchString=strdup(generaloptions.LastMoveArea);
            else
               AreaListPar.pchString=strdup(CurrentArea);
            AreaListPar.idTitle = IDST_TITLE_AL_MOVE;
            AreaListPar.ulIncludeTypes = INCLUDE_ALL;
            AreaListPar.bExtendedSel = FALSE;
            AreaListPar.bChange      = FALSE;

            if (WinDlgBox(HWND_DESKTOP, client,
                          AreaListProc, hmodLang,
                          IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
            {
               MSGLISTPAR MsgListPar;
               int rc;
               ULONG ulOptions=0;

               zeiger = AM_FindArea(&arealiste, AreaListPar.pchString);
               if (zeiger && zeiger->areadata.areatype != AREATYPE_LOCAL)
                  switch(MessageBox(client, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                                    IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                  {
                     case MBID_YES:
                        ulOptions = COPYMOVE_RESEND;
                        break;

                     default:
                        break;
                  }

               if (!MSG_OpenArea(&arealiste, AreaListPar.pchString, miscoptions.lastreadoffset, &driveremap))
               {
                  if (rc=MSG_MoveMessage(&CurrentMessage, &CurrentHeader, &arealiste,
                                         CurrentArea, AreaListPar.pchString, 0,
                                         &driveremap, &userdaten, &generaloptions, SendAddMessage,
                                         SendKillMessage, ulOptions))
                  {
                     char message[200];
                     sprintf(message, "Fehler %d beim Moven!", rc);
                     QuickMessage(client, message);
                  }
                  else
                  {
                     Notify(client, IDST_NOT_MOVED);
                     strcpy(generaloptions.LastMoveArea, AreaListPar.pchString);

                     AM_FindArea(&arealiste, AreaListPar.pchString)->mailentered=TRUE;
                     MailEntered[AM_FindArea(&arealiste, AreaListPar.pchString)->areadata.areatype] = TRUE;
                  }
                  MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                  MsgListPar.msgnum=MSG_MsgnToUid(&arealiste, CurrentArea, QueryCurrent(&arealiste, CurrentArea));
                  pWindowData->ulCurrentID= MsgListPar.msgnum;
                  NewMessage=FALSE;
                  UpdateDisplay(FALSE, TRUE);
                  MSG_CloseArea(&arealiste, AreaListPar.pchString, TRUE, miscoptions.lastreadoffset, &driveremap);
               }
               free(AreaListPar.pchString);
            }
         }
         break;

      case IDM_MSGCOPY:
      case IDB_COPYMSG:
      case IDA_COPY:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            AreaListPar.cb=sizeof(AREALISTPAR);
            if (generaloptions.LastCopyArea[0])
               AreaListPar.pchString=strdup(generaloptions.LastCopyArea);
            else
               AreaListPar.pchString=strdup(CurrentArea);
            AreaListPar.idTitle = IDST_TITLE_AL_COPY;
            AreaListPar.ulIncludeTypes = INCLUDE_ALL;
            AreaListPar.bExtendedSel = FALSE;
            AreaListPar.bChange      = FALSE;

            if (WinDlgBox(HWND_DESKTOP, client,
                          AreaListProc, hmodLang,
                          IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
            {
               ULONG ulOptions=0;

               zeiger = AM_FindArea(&arealiste, AreaListPar.pchString);
               if (zeiger && zeiger->areadata.areatype != AREATYPE_LOCAL)
                  switch(MessageBox(client, IDST_MSG_RESEND, IDST_TITLE_RESEND,
                                    IDD_RESEND, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
                  {
                     case MBID_YES:
                        ulOptions = COPYMOVE_RESEND;
                        break;

                     default:
                        break;
                  }

               if (!MSG_OpenArea(&arealiste, AreaListPar.pchString, miscoptions.lastreadoffset, &driveremap))
               {
                  if (MSG_CopyMessage(&CurrentMessage, &CurrentHeader, &arealiste,
                                      AreaListPar.pchString, &driveremap, &userdaten, &generaloptions, SendAddMessage, ulOptions))
                     QuickMessage(client, "Fehler Message kopieren!");
                  else
                  {
                     Notify(client, IDST_NOT_COPIED);
                     strcpy(generaloptions.LastCopyArea, AreaListPar.pchString);
                     AM_FindArea(&arealiste, AreaListPar.pchString)->mailentered=TRUE;
                     MailEntered[AM_FindArea(&arealiste, AreaListPar.pchString)->areadata.areatype]=TRUE;
                  }
                  NewMessage=FALSE;
                  MSG_CloseArea(&arealiste, AreaListPar.pchString, TRUE, miscoptions.lastreadoffset, &driveremap);
               }
               free(AreaListPar.pchString);
            }
         }
         break;

      case IDM_MSGFORWARD:
      case IDB_FORWARD:
      case IDA_FORWARD:
         if (CurrentStatus==PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            AreaListPar.cb=sizeof(AREALISTPAR);
            if (generaloptions.LastForwardArea[0])
               AreaListPar.pchString=strdup(generaloptions.LastForwardArea);
            else
               AreaListPar.pchString=strdup(CurrentArea);
            AreaListPar.idTitle = IDST_TITLE_AL_FORWARD;
            AreaListPar.ulIncludeTypes = INCLUDE_ALL;
            AreaListPar.bExtendedSel = FALSE;
            AreaListPar.bChange      = FALSE;

            if (WinDlgBox(HWND_DESKTOP, client,
                          AreaListProc, hmodLang,
                          IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
            {
               MSG_ForwardMessage(&templatelist, &CurrentMessage, &CurrentHeader,
                                  &arealiste, CurrentArea,
                                  AreaListPar.pchString,
                                  CurrentName, generaloptions.genFwdSubj);
               strcpy(NewArea, AreaListPar.pchString);
               strcpy(generaloptions.LastForwardArea, AreaListPar.pchString);
               NewMessage=TRUE;
               bTemplateProcessed=FALSE;
               ulTemplateType=TPLTYPE_FORWARD;
               iptInitialPos2=0;

               SwitchEditor(client, NewArea, TRUE);
               CurrentStatus = PROGSTATUS_EDITING;
               UpdateDisplay(TRUE, FALSE);
               StatusChanged(client, CurrentStatus);
               SetFocusControl(client, IDE_TONAME);
               free(AreaListPar.pchString);
            }
         }
         break;

      case IDM_MSGREQUEST:
      case IDB_REQUEST:
      case IDA_REQUEST:
         if (CurrentStatus==PROGSTATUS_READING && !hwndRequester &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);
            pRequestPar=calloc(1, sizeof(REQUESTPAR));
            PMASSERT(pRequestPar != NULL, "Out of memory");
            pRequestPar->cb=sizeof(STRINGPAR);
            pRequestPar->pchFiles = CurrentMessage.pchMessageText;
            WinQueryDlgItemText(client, IDE_FROMADDRESS, LEN_5DADDRESS+1, pRequestPar->pchReqAddr);
            WinQueryDlgItemText(client, IDE_FROMNAME, LEN_USERNAME+1, pRequestPar->pchReqName);
            strcpy(pRequestPar->pchDestArea, requestoptions.pchDestArea);
            pRequestPar->arealist=&arealiste;
            pRequestPar->bDirect = requestoptions.bDirectReq;
            pRequestPar->ulAttrib =  requestoptions.ulAttrib;

            hwndRequester=WinLoadDlg(HWND_DESKTOP, client, RequestProc,
                                     hmodLang, IDD_REQUESTER, pRequestPar);
         }
         break;

      case IDM_MSGXPOST:
      case IDA_CROSSPOST:
         if (CurrentStatus==PROGSTATUS_EDITING && NewMessage)
         {
            if (ulCCSelected || pQuickCCList)
               break;
            if (AM_FindArea(&arealiste, NewMessage?NewArea:CurrentArea)->areadata.areatype == AREATYPE_NET)
               break;
            if (pchXPostList==NULL)
            {
               AreaListPar.cb=sizeof(AREALISTPAR);
               AreaListPar.pchString=NULL;
               AreaListPar.idTitle = IDST_TITLE_AL_XPOST;
               AreaListPar.ulIncludeTypes = INCLUDE_ECHO | INCLUDE_LOCAL;
               AreaListPar.bExtendedSel = TRUE;
               AreaListPar.bChange      = FALSE;
               if (WinDlgBox(HWND_DESKTOP, client,
                             AreaListProc, hmodLang,
                             IDD_AREALIST, &AreaListPar)==DID_OK && AreaListPar.pchString)
               {
                  pchXPostList=AreaListPar.pchString;
               }
            }
            else
            {
               free(pchXPostList);
               pchXPostList=NULL;
               Notify(client, IDST_NOT_XPDESELECT);
            }
         }
         break;

      case IDM_EDITSEARCH:
         SearchPar.cb = sizeof(SearchPar);

         if (WinDlgBox(HWND_DESKTOP, client, SearchProc, hmodLang,
                       IDD_SEARCH, &SearchPar) == DID_OK)
         {
            if (!SearchInMessage(client, &SearchPar))
               Notify(client, IDST_NOT_STRINGNOTFOUND);
         }
         break;

      case IDM_OPNAMEADDR:
      case IDA_DEFAULTADDR:
         if (CurrentStatus == PROGSTATUS_NOSETUP)
            break;

         CurrNamePar.cb=sizeof(CURRNAMEPAR);
         strcpy(CurrNamePar.CurrName, CurrentName);
         strcpy(CurrNamePar.CurrAddr, CurrentAddress);
         if (WinDlgBox(HWND_DESKTOP, client,
                       CurrNameProc, hmodLang, IDD_CURRENTNAMEADDR,
                       &CurrNamePar)==DID_OK)
         {
            /* Name und Adresse uebernehmen */
            strcpy(CurrentName, CurrNamePar.CurrName);
            strcpy(CurrentAddress, CurrNamePar.CurrAddr);
            WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                              MPFROMLONG(pWindowData->idAddressField),
                              CurrentAddress);

            if (CurrentStatus==PROGSTATUS_EDITING)
            {
               WinSetDlgItemText(client, IDE_FROMNAME, CurrentName);
               WinSetDlgItemText(client, IDE_FROMADDRESS, CurrentAddress);
            }
         }
         break;

      case IDM_OPSAVE:
         if (!issecondinstance)
            _beginthread(SaveIniProfileThread, NULL, 32768, NULL);
         else
            WinAlarm(HWND_DESKTOP, WA_ERROR);
         break;

      case IDM_MSGMARK:
      case IDB_CATCHUP:
      case IDA_CATCHUP:
         if (CurrentStatus == PROGSTATUS_READING &&
             pWindowData->pArea && pWindowData->pArea->maxmessages)
         {
            if (bDoingWork)
               MessageBox(client, IDST_MSG_DOINGWORK, 0, IDD_DOINGWORK, MB_OK);
            else
            {
               if (generaloptions.safety & SAFETY_CATCHUP)
                  if (MessageBox(client, IDST_MSG_CATCHUP, IDST_TITLE_CATCHUP,
                                 IDD_CATCHUP, MB_YESNO) != MBID_YES)
                     break;
               MarkAllMessages(CurrentArea);
            }
         }
         break;

      case IDM_OPCCLISTS:
         if (!hwndCCLists)
            hwndCCLists = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, CCFolderProc,
                                     hmodLang, IDD_CCFOLDER, NULL);
         else
            SetFocus(hwndCCLists);
         break;

      case IDM_WINAREAS:
      case IDM_WINMSGLIST:
      case IDM_WINTHREADS:
      case IDM_WINKLUDGES:
      case IDM_WINPROGRESS:
         ShowFleetWindow(CommandID);
         break;

      case IDM_WINRESULTS:
      case IDB_BOOKMARKS:
      case IDA_BOOKMARKS:
         if (pWindowData->pArea)
            ShowFleetWindow(IDM_WINRESULTS);
         break;

      case IDM_MSGCCOPY:
      case IDA_CCOPY:
         if (CurrentStatus==PROGSTATUS_EDITING && NewMessage)
         {
            if (pchXPostList)
               break;             /* Kein CC und XP gleichzeitig */
            zeiger= AM_FindArea(&arealiste, NewMessage? NewArea:CurrentArea);
            if (zeiger->areadata.areatype == AREATYPE_ECHO)
               break;
            if (ulCCSelected)
            {
               /* CC abschalten */

               ulCCSelected=0;
               WinSendDlgItemMsg(client, IDE_TONAME, EM_SETREADONLY, (MPARAM) FALSE, NULL);
               WinSendDlgItemMsg(client, IDE_TOADDRESS, EM_SETREADONLY, (MPARAM) FALSE, NULL);
               WinSetDlgItemText(client, IDE_TOADDRESS, SaveToAddress);
               WinSetDlgItemText(client, IDE_TONAME, SaveToName);
               Notify(client, IDST_NOT_CCDESELECT);
            }
            else
            {
               CCSelectPar.cb=sizeof(CCSELECTPAR);
               CCSelectPar.ulSelectID = 0;
               CCSelectPar.bEmptyLists=FALSE;
               if (WinDlgBox(HWND_DESKTOP, client, CCListSelectProc, hmodLang,
                         IDD_CCLISTSELECT, &CCSelectPar)==DID_OK)
               {
                  if (CCSelectPar.ulSelectID)
                  {
                     char *pchTemp, *pchDst, *pchSrc;
                     PCCLIST pList;

                     ulCCSelected=CCSelectPar.ulSelectID;
                     pList = QueryCCList(&ccanchor, ulCCSelected);

                     /* Name vorbereiten */
                     pchTemp = malloc(strlen(pList->pchListName)+10);
                     PMASSERT(pchTemp != NULL, "Out of memory");
                     strcpy(pchTemp, "** ");
                     pchDst=strchr(pchTemp, 0);
                     pchSrc =pList->pchListName;
                     while (*pchSrc)
                     {
                        switch(*pchSrc)
                        {
                           case '\r':
                              break;

                           case '\n':
                              *pchDst++ = ' ';
                              break;

                           default:
                              *pchDst++ = *pchSrc;
                              break;
                        }
                        pchSrc++;
                     }
                     *pchDst=0;
                     strcat(pchTemp, " **");
                     WinSetDlgItemText(client, IDE_TONAME, pchTemp);
                     free(pchTemp);

                     WinQueryDlgItemText(client, IDE_TONAME, LEN_USERNAME+1, SaveToName);
                     WinQueryDlgItemText(client, IDE_TOADDRESS, LEN_5DADDRESS+1, SaveToAddress);
                     WinSendDlgItemMsg(client, IDE_TONAME, EM_SETREADONLY, (MPARAM) TRUE, NULL);
                     WinSendDlgItemMsg(client, IDE_TOADDRESS, EM_SETREADONLY, (MPARAM) TRUE, NULL);
                     WinSetDlgItemText(client, IDE_TOADDRESS, "");
                  }
#ifdef DEBUG
                  else
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
#endif
               }
            }
         }
         break;

      case IDM_MSGQUICKCC:
         if (!pQuickCCList)
         {
            CCLISTPAR CCListPar;

            pQuickCCList = malloc(sizeof(CCLIST));
            PMASSERT(pQuickCCList != NULL, "Out of memory");
            memset(pQuickCCList, 0, sizeof(CCLIST));

            pQuickCCList->pchListName = malloc(100);
            PMASSERT(pQuickCCList->pchListName != NULL, "Out of memory");
            LoadString(IDST_CC_QUICKCC, 100, pQuickCCList->pchListName);

            CCListPar.cb = sizeof(CCListPar);
            CCListPar.pList = pQuickCCList;
            CCListPar.pCCAnchor = NULL;

            if (WinDlgBox(HWND_DESKTOP, client, CCListContentsProc, hmodLang,
                      IDD_CCLIST, &CCListPar)!= DID_OK)
            {
               while(pQuickCCList->pEntries)
                  DeleteCCEntry(NULL, pQuickCCList, pQuickCCList->pEntries);
               free(pQuickCCList->pchListName);
               free(pQuickCCList);
               pQuickCCList=NULL;
               Notify(client, IDST_NOT_CCDESELECT);
            }
            else
            {
               char *pchTemp;

               pchTemp = malloc(strlen(pQuickCCList->pchListName)+10);
               PMASSERT(pchTemp != NULL, "Out of memory");
               strcpy(pchTemp, "** ");
               strcat(pchTemp, pQuickCCList->pchListName);
               strcat(pchTemp, " **");
               WinQueryDlgItemText(client, IDE_TONAME, LEN_USERNAME+1, SaveToName);
               WinQueryDlgItemText(client, IDE_TOADDRESS, LEN_5DADDRESS+1, SaveToAddress);
               WinSendDlgItemMsg(client, IDE_TONAME, EM_SETREADONLY, (MPARAM) TRUE, NULL);
               WinSendDlgItemMsg(client, IDE_TOADDRESS, EM_SETREADONLY, (MPARAM) TRUE, NULL);
               WinSetDlgItemText(client, IDE_TOADDRESS, "");
               WinSetDlgItemText(client, IDE_TONAME, pchTemp);
               free(pchTemp);
            }
         }
         else
         {
            SHORT sRet;

            sRet = MessageBox(client, IDST_MSG_MODQUICKCC, IDST_TITLE_MODQUICKCC,
                              IDD_MODQUICKCC, MB_YESNOCANCEL | MB_QUERY);
            switch(sRet)
            {
               CCLISTPAR CCListPar;

               case MBID_YES:
                  CCListPar.cb = sizeof(CCListPar);
                  CCListPar.pList = pQuickCCList;
                  CCListPar.pCCAnchor = NULL;

                  WinDlgBox(HWND_DESKTOP, client, CCListContentsProc, hmodLang,
                            IDD_CCLIST, &CCListPar);
                  break;

               case MBID_NO:
                  while(pQuickCCList->pEntries)
                     DeleteCCEntry(NULL, pQuickCCList, pQuickCCList->pEntries);
                  free(pQuickCCList->pchListName);
                  free(pQuickCCList);
                  pQuickCCList=NULL;

                  WinSendDlgItemMsg(client, IDE_TONAME, EM_SETREADONLY, (MPARAM) FALSE, NULL);
                  WinSendDlgItemMsg(client, IDE_TOADDRESS, EM_SETREADONLY, (MPARAM) FALSE, NULL);
                  WinSetDlgItemText(client, IDE_TOADDRESS, SaveToAddress);
                  WinSetDlgItemText(client, IDE_TONAME, SaveToName);
                  Notify(client, IDST_NOT_CCDESELECT);
                  break;

               default:
                  break;
            }
         }
         break;

      case IDM_FILEECHOTOSS:
         if (echotossoptions.useechotoss &&
             echotossoptions.pchEchoToss[0])
            if (WriteEchotoss(&arealiste, echotossoptions.pchEchoToss))
               MessageBox(client, IDST_MSG_ETOSSERROR, 0, IDD_ETOSSERROR,
                          MB_ERROR | MB_OK);
         break;

      case IDM_EP_COPY:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_COPY, NULL, NULL);
               break;

            case IDE_FROMNAME:
            case IDE_FROMADDRESS:
            case IDE_TONAME:
            case IDE_TOADDRESS:
            case IDE_SUBJTEXT:
               WinSendDlgItemMsg(client, pWindowData->usPopupControl,
                                 EM_COPY, NULL, NULL);
               break;

            default:
               break;
         }
         break;

      case IDM_EP_CUT:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_CUT, NULL, NULL);
               break;

            case IDE_FROMNAME:
            case IDE_FROMADDRESS:
            case IDE_TONAME:
            case IDE_TOADDRESS:
            case IDE_SUBJTEXT:
               WinSendDlgItemMsg(client, pWindowData->usPopupControl,
                                 EM_CUT, NULL, NULL);
               break;

            default:
               break;
         }
         break;

      case IDM_EP_PASTE:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_PASTE, NULL, NULL);
               break;

            case IDE_FROMNAME:
            case IDE_FROMADDRESS:
            case IDE_TONAME:
            case IDE_TOADDRESS:
            case IDE_SUBJTEXT:
               WinSendDlgItemMsg(client, pWindowData->usPopupControl,
                                 EM_PASTE, NULL, NULL);
               break;

            default:
               break;
         }
         break;

      case IDM_EP_UNDO:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_UNDO, NULL, NULL);
               break;

            default:
               break;
         }
         break;

      case IDM_EP_CLEAR:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_CLEAR, NULL, NULL);
               break;

            case IDE_FROMNAME:
            case IDE_FROMADDRESS:
            case IDE_TONAME:
            case IDE_TOADDRESS:
            case IDE_SUBJTEXT:
               WinSendDlgItemMsg(client, pWindowData->usPopupControl,
                                 EM_CLEAR, NULL, NULL);
               break;

            default:
               break;
         }
         break;

      case IDM_EP_DELLINE:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_DELETELINE, NULL, NULL);
               break;

            default:
               break;
         }
         break;

      case IDM_EP_HIGHLIGHT:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               if (WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_QUERYHIGHLIGHT, NULL, NULL))
                  WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_ENABLEHIGHLIGHT, MPFROMLONG(FALSE), NULL);
               else
                  WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_ENABLEHIGHLIGHT, MPFROMLONG(TRUE), NULL);
               break;

            default:
               break;
         }
         break;

      case IDA_HIGHLIGHT:
         if (WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(client,IDML_MAINEDIT) &&
             CurrentStatus != PROGSTATUS_EDITING)
         {
            if (WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_QUERYHIGHLIGHT, NULL, NULL))
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_ENABLEHIGHLIGHT, MPFROMLONG(FALSE), NULL);
            else
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_ENABLEHIGHLIGHT, MPFROMLONG(TRUE), NULL);
         }
         break;

      case IDM_EP_MONO:
         switch(pWindowData->usPopupControl)
         {
            case IDML_MAINEDIT:
               if (MonoDisp || TempMono)
               {
                  QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewermonofont);
                  SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewerfont);
                  MonoDisp = TempMono = FALSE;
               }
               else
               {
                  QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewerfont);
                  SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewermonofont);
                  MonoDisp = TRUE;
               }
               break;

            default:
               break;
         }
         break;

      case IDA_MONO:
        if (WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(client,IDML_MAINEDIT) &&
            (CurrentStatus == PROGSTATUS_EDITING || CurrentStatus == PROGSTATUS_READING))
        {
           if (MonoDisp || TempMono)
           {
              QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewermonofont);
              SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewerfont);
              MonoDisp = TempMono = FALSE;
           }
           else
           {
              QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewerfont);
              SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewermonofont);
              MonoDisp = TRUE;
           }
        }
        break;

      case IDM_MSGBCDELETE:
         if (!MSG_BroadcastDelete(&arealiste, CurrentArea, &CurrentHeader,
                                  &CurrentMessage, &userdaten, &generaloptions,
                                  CurrentName, CurrentAddress,
                                  &driveremap, &templatelist, SendAddMessage))
         {
            MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
            pWindowData->pArea->mailentered=TRUE;
            MailEntered[pWindowData->pArea->areadata.areatype]=TRUE;
         }
         else
         {
            /* Fehler */
            MessageBox(client, IDST_MSG_BCDELERROR, IDST_TITLE_BCDELERROR,
                       IDD_BCDELERROR, MB_OK | MB_ERROR);
         }
         MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
         break;

      case IDM_MSGBCMODIFY:
         MSG_BroadcastModify(&CurrentHeader, &CurrentMessage);
         NewMessage=TRUE;
         SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
         strcpy(NewArea, CurrentArea);
         SwitchEditor(client, NewArea, TRUE);
         DisplayMsgText(client, &CurrentMessage);
         CurrentStatus = PROGSTATUS_EDITING;
         StatusChanged(client, CurrentStatus);
         SetFocusControl(client, IDML_MAINEDIT);
         break;

      case IDM_RXSCRIPTS:
      case IDB_SCRIPTS:
         if (hwndRxFolder)
            SetFocusControl(hwndRxFolder, IDD_RXFOLDER+1);
         else
         {
            hwndRxFolder = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, ScriptFolderProc,
                                          hmodLang, IDD_RXFOLDER, NULL);
            WinShowWindow(hwndRxFolder, TRUE);
         }
         break;

      case IDM_SPCADDTONICK:
      case IDA_ADDTONICK:
         if (pWindowData->pArea && pWindowData->pArea->maxmessages>0)
            AddToNick(client, &CurrentHeader);
         break;

      case IDM_SPCADDTOCC:
      case IDA_ADDTOCC:
         if (pWindowData->pArea && pWindowData->pArea->maxmessages>0 && ccanchor.ulNumLists)
            AddToCCList(client, &CurrentHeader);
         break;

      case IDM_RXQUICK1:
      case IDM_RXQUICK2:
      case IDM_RXQUICK3:
      case IDM_RXQUICK4:
      case IDM_RXQUICK5:
      case IDM_RXQUICK6:
      case IDM_RXQUICK7:
      case IDM_RXQUICK8:
      case IDM_RXQUICK9:
      case IDM_RXQUICK10:
         StartQuickRexx(CommandID);
         break;

      case IDM_SPCBROWSER:
      case IDB_BROWSER:
      case IDA_NLBROWSER:
         if (domains)
         {
            if (!hwndNLBrowser)
               hwndNLBrowser = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, NLBrowserProc,
                                          hmodLang, IDD_NLBROWSER, NULL);
            else
            {
               SWP swp;

               SetFocusControl(hwndNLBrowser, IDD_NLBROWSER+1);
               WinQueryWindowPos(hwndNLBrowser, &swp);
               if (swp.fl & SWP_MINIMIZE)
                  WinSetWindowPos(hwndNLBrowser,
                                  NULLHANDLE,
                                  0, 0, 0, 0,
                                  SWP_RESTORE);
            }
         }
         break;

      case IDM_MSGMARKMSG:
         if (!IsMessageMarked(&MarkerList, CurrentArea, pWindowData->ulCurrentID, MARKFLAG_MANUAL))
         {
            MarkMessage(&MarkerList, CurrentArea, pWindowData->ulCurrentID, QueryCurrent(&arealiste, CurrentArea),
                        &CurrentHeader, NULL, MARKFLAG_MANUAL, 0, 0);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID = pWindowData->ulCurrentID;
            SendMsg(client, WORKM_MSGMARKED, &MessageID, &CurrentHeader);
         }
         break;

      case IDM_MSGUNMARKMSG:
         if (IsMessageMarked(&MarkerList, CurrentArea, pWindowData->ulCurrentID, MARKFLAG_MANUAL))
         {
            UnmarkMessage(&MarkerList, CurrentArea, pWindowData->ulCurrentID, MARKFLAG_MANUAL);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID = pWindowData->ulCurrentID;
            SendMsg(client, WORKM_MSGUNMARKED, &MessageID, NULL);
         }
         break;

      case IDM_TB_TOP:
      case IDM_TB_BOTTOM:
      case IDM_TB_LEFT:
      case IDM_TB_RIGHT:
         SwitchToolbarPos(client, CommandID);
         break;

      case IDM_TB_SMALL:
         SwitchToolbarSize(client);
         break;

      case IDM_TB_ADD:
         ToolbarConfig.cb = sizeof(ToolbarConfig);
         if (WinDlgBox(HWND_DESKTOP, client, ToolbarConfigProc, hmodLang,
                   IDD_TOOLBARCONFIG, &ToolbarConfig) == DID_OK)
         {
            RefreshToolbar(WinWindowFromID(frame, FID_TOOLBAR), &ToolbarConfig,
                           ToolbarOptions.bSmallToolbar);
            StatusChanged(client, CurrentStatus);
            UpdateButtons(pWindowData->pArea);
            SendMsg(frame, WM_UPDATEFRAME, NULL, NULL);
         }
         break;

      case IDM_OPECHOMAN:
         SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);
         WinDlgBox(HWND_DESKTOP, client, EchoMgrProc, hmodLang, IDD_ECHOMANAGER, NULL);
         SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
         break;

      case IDM_OPADDAREAS:
         if (generaloptions.safety & SAFETY_CHANGESETUP)
            if (MessageBox(client, IDST_MSG_DOEXTRACT, IDST_TITLE_DOEXTRACT, IDD_DOEXTRACT,
                           MB_YESNO | MB_QUERY) != MBID_YES)
               break;
         if (!ExtractUplinkFromMessage(client, &CurrentHeader, &CurrentMessage, &EchoMgrOpt))
            MessageBox(client, IDST_MSG_NOUPLINKAREAS, 0, IDD_NOUPLINKAREAS,
                       MB_ERROR | MB_OK);
         else
            Notify(client, IDST_NOT_EXTRACTED);
         break;

      case IDA_DELLINE:
         if (WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(client, IDML_MAINEDIT) &&
             CurrentStatus==PROGSTATUS_EDITING)
            WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_DELETELINE, NULL, NULL);
         break;

      case IDA_UNDO:
         if (WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(client, IDML_MAINEDIT) &&
             CurrentStatus==PROGSTATUS_EDITING)
            WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_UNDO, NULL, NULL);
         break;

      case IDA_KEYSHELP:
         if (hwndhelp)
            SendMsg(hwndhelp, HM_KEYS_HELP, NULL, NULL);
         break;

      case IDA_F2:
      case IDA_F3:
      case IDA_F4:
      case IDA_F5:
      case IDA_F6:
      case IDA_F7:
      case IDA_F8:
      case IDA_F9:
      case IDA_F10:
      case IDA_F11:
      case IDA_F12:
         if (WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(client, IDML_MAINEDIT) &&
             CurrentStatus==PROGSTATUS_EDITING)
            InsertMacro(WinWindowFromID(client, IDML_MAINEDIT), CommandID-IDA_F2);
         break;

      case IDA_REPLY:
      case IDA_REPLYNOJOIN:
         QuoteCurrentMessage(pWindowData, FALSE,
#if 1
                             REPLYDEST_FROM,
#else
                             MSG_FindKludge(&CurrentMessage, KLUDGE_FWDFROM, NULL)?REPLYDEST_ORIG:REPLYDEST_FROM,
#endif
                             (CommandID==IDA_REPLYNOJOIN)?(QUOTE_TEXT|QUOTE_NOJOIN):QUOTE_TEXT);
         break;

      case IDA_REPLYNET:
      case IDA_REPLYNETNOJOIN:
         QuoteCurrentMessage(pWindowData, TRUE,
#if 1
                             REPLYDEST_FROM,
#else
                             MSG_FindKludge(&CurrentMessage, KLUDGE_FWDFROM, NULL)?REPLYDEST_ORIG:REPLYDEST_FROM,
#endif
                             (CommandID==IDA_REPLYNETNOJOIN)?(QUOTE_TEXT|QUOTE_NOJOIN):QUOTE_TEXT);
         break;

      case IDA_TOGGLE_CRASH:
         if (CurrentStatus==PROGSTATUS_EDITING)
         {
            CurrentHeader.ulAttrib ^= ATTRIB_CRASH;
            DisplayAttrib(CurrentHeader.ulAttrib);
         }
         break;

      case IDA_TOGGLE_KILL:
         if (CurrentStatus==PROGSTATUS_EDITING)
         {
            CurrentHeader.ulAttrib ^= ATTRIB_KILLSENT;
            DisplayAttrib(CurrentHeader.ulAttrib);
         }
         break;

      case IDA_TOGGLE_FILE:
         if (CurrentStatus==PROGSTATUS_EDITING)
         {
            CurrentHeader.ulAttrib ^= ATTRIB_FILEATTACHED;
            HandleAttachAttrib(client);
         }
         break;

      case IDA_TOGGLE_REQ:
         if (CurrentStatus==PROGSTATUS_EDITING)
         {
            CurrentHeader.ulAttrib ^= ATTRIB_FREQUEST;
            DisplayAttrib(CurrentHeader.ulAttrib);
         }
         break;

      case IDA_TOGGLE_PRIV:
         if (CurrentStatus==PROGSTATUS_EDITING)
         {
            CurrentHeader.ulAttrib ^= ATTRIB_PRIVATE;
            DisplayAttrib(CurrentHeader.ulAttrib);
         }
         break;

      case IDA_TOGGLE_HOLD:
         if (CurrentStatus==PROGSTATUS_EDITING)
         {
            CurrentHeader.ulAttrib ^= ATTRIB_HOLD;
            DisplayAttrib(CurrentHeader.ulAttrib);
         }
         break;

      case IDA_TOGGLE_DIRECT:
         if (CurrentStatus==PROGSTATUS_EDITING)
         {
            CurrentHeader.ulAttrib ^= ATTRIB_DIRECT;
            DisplayAttrib(CurrentHeader.ulAttrib);
         }
         break;

      case IDA_HOMEMSG:
      case IDB_HOMEMSG:
         if (CurrentStatus==PROGSTATUS_READING)
         {
            ULONG ulHome=MSG_QueryHomeMsg(&arealiste, CurrentArea);

            if (ulHome)
            {
               if (pWindowData->pArea)
               {
                  pWindowData->pArea->currentmessage=ulHome;
                  MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                  pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
                  UpdateDisplay(FALSE, TRUE);
               }
               else
                  WinAlarm(HWND_DESKTOP, WA_ERROR);
            }
            else
               WinAlarm(HWND_DESKTOP, WA_ERROR);
         }
         break;

      case IDA_NEXTAREA:
      case IDB_NEXTAREA:
         {
            char *pchNextArea=NULL;
            MSGLISTPAR MsgListPar;
            MESSAGEID MessageID;

            MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
            strcpy(MessageID.pchAreaTag, CurrentArea);
            MessageID.ulMsgID=pWindowData->ulCurrentID;
            SendMsg(client, WORKM_READ, &MessageID, NULL);

            if (pchNextArea=QueryNextArea(&arealiste, CurrentArea))
            {
               MSG_CloseArea(&arealiste, CurrentArea, TRUE, miscoptions.lastreadoffset, &driveremap);
               SwitchToArea(pchNextArea);
               pWindowData->ulCurrentID=MsgListPar.msgnum=CurrentHeader.ulMsgID;
               if (hwndThreadList)
                  SendMsg(hwndThreadList, TM_REREADTHREADS, &MsgListPar, NULL);
               if (hwndMsgList)
                  SendMsg(hwndMsgList, TM_REREADTHREADS, &MsgListPar, NULL);
            }
            else
               MessageBox(client, IDST_MSG_NONEXTAREA, 0, IDD_NONEXTAREA,
                          MB_OK | MB_WARNING);
         }
         break;

      case IDA_MARKTOGGLE:
         if (pWindowData->pArea && pWindowData->pArea->maxmessages>0)
         {
            if (IsMessageMarked(&MarkerList, CurrentArea, pWindowData->ulCurrentID, MARKFLAG_MANUAL))
            {
               UnmarkMessage(&MarkerList, CurrentArea, pWindowData->ulCurrentID, MARKFLAG_MANUAL);
               strcpy(MessageID.pchAreaTag, CurrentArea);
               MessageID.ulMsgID = pWindowData->ulCurrentID;
               SendMsg(client, WORKM_MSGUNMARKED, &MessageID, NULL);
            }
            else
            {
               MarkMessage(&MarkerList, CurrentArea, pWindowData->ulCurrentID,
                           QueryCurrent(&arealiste, CurrentArea), &CurrentHeader, NULL,
                           MARKFLAG_MANUAL, 0, 0);
               strcpy(MessageID.pchAreaTag, CurrentArea);
               MessageID.ulMsgID = pWindowData->ulCurrentID;
               SendMsg(client, WORKM_MSGMARKED, &MessageID, &CurrentHeader);
            }
         }
         break;

      case IDA_RXQUICK1:
      case IDA_RXQUICK2:
      case IDA_RXQUICK3:
      case IDA_RXQUICK4:
      case IDA_RXQUICK5:
      case IDA_RXQUICK6:
      case IDA_RXQUICK7:
      case IDA_RXQUICK8:
      case IDA_RXQUICK9:
      case IDA_RXQUICK10:
         StartQuickRexx(IDM_RXQUICK1 + CommandID - IDA_RXQUICK1);
         break;

      case IDA_BACKWIN:
         BackToWindow(pWindowData);
         break;

      default:
#ifdef DEBUG
         WinAlarm(HWND_DESKTOP, WA_NOTE);
#endif
         break;
   }
   return;
}

/*------------------------------ ProcessKeys   ------------------------------*/
/* Bearbeitet Tastendruecke Enter, Tab und BackTab                           */
/* Rueckgabewerte: TRUE, Taste behandelt                                     */
/*                 FALSE, Taste nicht behandelt                              */
/*---------------------------------------------------------------------------*/

static BOOL ProcessKeys(HWND client, USHORT usVK)
{
   HWND hwndFocus;
   USHORT FocusID;
   AREADEFLIST *zeiger=NULL;

   if (!(hwndFocus=WinQueryFocus(HWND_DESKTOP)))
      return FALSE;

   FocusID=WinQueryWindowUShort(hwndFocus, QWS_ID);

   zeiger=AM_FindArea(&arealiste, NewMessage?NewArea:CurrentArea);

   switch (usVK)
   {
      case VK_NEWLINE:
      case VK_ENTER:
         if (CurrentStatus != PROGSTATUS_EDITING)
            return FALSE;
         if (FocusID == IDE_TONAME)
         {
            char name[LEN_USERNAME+1];
            char FoundAddress[LEN_5DADDRESS+1];

            WinQueryDlgItemText(client, IDE_TONAME, LEN_USERNAME+1, name);

            /* Nicknames evtl. ersetzen */
            if (!LookupNickname(client, name, &NickNameList))
            {
               if (domains &&
                   zeiger->areadata.areatype == AREATYPE_NET &&
                   !ulCCSelected &&
                   !pQuickCCList &&
                   WinQueryDlgItemTextLength(client, IDE_TONAME) >0)
               {
                  /* Nodelist-Lookup */
                  if (PerformNameLookup(name, client, LOOKUP_NORMAL, name, FoundAddress))
                  {
                     FTNADDRESS tempAddr, tempAddr2;
                     char FromAddress[LEN_5DADDRESS+1];
                     int iMatch;

                     WinSetDlgItemText(client, IDE_TOADDRESS, FoundAddress);
                     WinQueryDlgItemText(client, IDE_FROMADDRESS, LEN_5DADDRESS+1, FromAddress);
                     StringToNetAddr(FromAddress, &tempAddr2, NULL);
                     StringToNetAddr(FoundAddress, &tempAddr, NULL);
                     iMatch = MSG_MatchAddress(&tempAddr, &userdaten, &tempAddr2);
                     if (iMatch>=0)
                        WinSetDlgItemText(client, IDE_FROMADDRESS, userdaten.address[iMatch]);
                     WinSetDlgItemText(client, IDE_TONAME, name);
                     SetFocusControl(client, IDE_SUBJTEXT);
                  }
                  else
                     SetFocusControl(client, IDE_TOADDRESS);
               }
               else
                  SetFocusControl(client, IDE_SUBJTEXT);
            }
         }

         if (FocusID == IDE_TOADDRESS)
         {
            char name[LEN_USERNAME+1];

            /* Fokuswechsel erzwingt Adress-Ergaenzung */
            SetFocusControl(client, IDE_SUBJTEXT);

            WinQueryDlgItemText(client, IDE_TONAME, LEN_USERNAME+1, name);
            if (!name[0]) /* noch kein Name eingegeben */
            {
               if (domains &&
                   zeiger->areadata.areatype == AREATYPE_NET &&
                   !ulCCSelected &&
                   !pQuickCCList &&
                   WinQueryDlgItemTextLength(client, IDE_TOADDRESS) >0)
               {
                  /* Nodelist-Lookup */
                  char pchAddress[LEN_5DADDRESS+1];

                  WinQueryDlgItemText(client, IDE_TOADDRESS, LEN_5DADDRESS+1, pchAddress);

                  if (PerformNodeLookup(pchAddress, client, name))
                     WinSetDlgItemText(client, IDE_TONAME, name);
               }
            }
         }

         if (FocusID == IDE_FROMNAME ||
             FocusID == IDE_FROMADDRESS ||
             FocusID == IDE_SUBJTEXT)
         {
             if (zeiger)
                SetFocusControl(client, FLTLAY_QueryNextFocus(FocusID, zeiger->areadata.areatype != AREATYPE_NET));
         }
         if (FocusID == IDML_MAINEDIT)
            return FALSE;
         break;

      case VK_TAB:
         if (WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(client, IDML_MAINEDIT) &&
             CurrentStatus==PROGSTATUS_EDITING)
            return FALSE;
         if (zeiger)
            SetFocusControl(client, FLTLAY_QueryNextFocus(FocusID, zeiger->areadata.areatype != AREATYPE_NET));
         break;

      case VK_BACKTAB:
#if 0
         if (WinQueryFocus(HWND_DESKTOP)==WinWindowFromID(client, IDML_MAINEDIT) &&
             CurrentStatus==PROGSTATUS_EDITING)
            return FALSE;
#endif
         if (zeiger)
            SetFocusControl(client, FLTLAY_QueryPrevFocus(FocusID, zeiger->areadata.areatype != AREATYPE_NET));
         break;

      default:
         return FALSE;
   }
   return TRUE;
}


/*------------------------------ SwitchToArea   -----------------------------*/
/* Neue Area oeffnen, Userdaten fuer Area uebernehmen                        */
/*---------------------------------------------------------------------------*/

static int SwitchToArea(char *NewArea)
{
   AREADEFLIST *zeiger=NULL;
   BOOL bError=FALSE;
   PWINDOWDATA pWindowData;

   pWindowData=(PWINDOWDATA) WinQueryWindowULong(client, QWL_USER);

   /* Area noch vorhanden ? */
   zeiger=AM_FindArea(&arealiste, NewArea);
   if (!zeiger)
   {  /* erste Area nehmen */
      zeiger=arealiste.pFirstArea;
   }

   if (zeiger)
   {
      /* Area oeffnen und Message lesen */
      strcpy(CurrentArea, zeiger->areadata.areatag);
      strcpy(CurrentAddress, zeiger->areadata.address);
      if (zeiger->areadata.username[0])
         strcpy(CurrentName, zeiger->areadata.username);
      else
         strcpy(CurrentName, userdaten.username[0]);

      if (MSG_OpenArea(&arealiste, CurrentArea, miscoptions.lastreadoffset, &driveremap))
      {
         if (zeiger != arealiste.pFirstArea)
         {
            zeiger=arealiste.pFirstArea;
            strcpy(CurrentArea, zeiger->areadata.areatag);
            strcpy(CurrentAddress, zeiger->areadata.address);
            if (zeiger->areadata.username[0])
               strcpy(CurrentName, zeiger->areadata.username);
            else
               strcpy(CurrentName, userdaten.username[0]);

            if (!generaloptions.monospaced &&
                !(zeiger->areadata.ulAreaOpt & AREAOPT_MONOSPACED))
            {
               if (MonoDisp)
               {
                  QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewermonofont);
                  SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewerfont);
               }
               TempMono = MonoDisp = FALSE;
            }
            else
            {
               if (!MonoDisp)
               {
                  QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewerfont);
                  SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewermonofont);
               }
               MonoDisp = TRUE;
               TempMono = FALSE;
            }

            if (MSG_OpenArea(&arealiste, CurrentArea, miscoptions.lastreadoffset, &driveremap))
               bError=TRUE;
            else
            {
               pWindowData->pArea=zeiger;
               if (zeiger->maxmessages >0 && zeiger->currentmessage==0)
                  zeiger->currentmessage=1;

               if (!generaloptions.monospaced &&
                   !(zeiger->areadata.ulAreaOpt & AREAOPT_MONOSPACED))
               {
                  if (MonoDisp)
                  {
                     QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewermonofont);
                     SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewerfont);
                  }
                  MonoDisp = FALSE;
               }
               else
               {
                  if (!MonoDisp)
                  {
                     QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewerfont);
                     SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewermonofont);
                  }
                  MonoDisp = TRUE;
               }

               MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
               pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
               if (CurrentStatus == PROGSTATUS_EDITING)
               {
                  SetTranslateMode(!(zeiger->areadata.ulAreaOpt & AREAOPT_HIGHASCII));
               }
               else
                  if (CurrentStatus == PROGSTATUS_READING)
                  {
                     PMSGTEMPLATE pTemplate;

                     WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_ENABLEHIGHLIGHT,
                                       MPFROMLONG(!generaloptions.nohighlight && !(zeiger->areadata.ulAreaOpt & AREAOPT_NOHIGHLIGHT)),
                                       NULL);
                     pTemplate = M_FindTemplate(&templatelist, &arealiste, zeiger->areadata.areatag);
                     if (pTemplate)
                        WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_SETQUOTECHAR,
                                          MPFROMCHAR(pTemplate->chQuoteChar), NULL);
                  }
            }
         }
         else
            bError=TRUE;
      }
      else
      {
         pWindowData->pArea=zeiger;
         if (zeiger->maxmessages >0 && zeiger->currentmessage==0)
            zeiger->currentmessage=1;
         MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
         pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
         if (CurrentStatus == PROGSTATUS_EDITING)
         {
            SetTranslateMode(!(zeiger->areadata.ulAreaOpt & AREAOPT_HIGHASCII));
         }
         else
            if (CurrentStatus == PROGSTATUS_READING)
            {
               PMSGTEMPLATE pTemplate;
               WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_ENABLEHIGHLIGHT,
                                 MPFROMLONG(!generaloptions.nohighlight && !(zeiger->areadata.ulAreaOpt & AREAOPT_NOHIGHLIGHT)),
                                 NULL);
               pTemplate = M_FindTemplate(&templatelist, &arealiste, zeiger->areadata.areatag);
               if (pTemplate)
                  WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_SETQUOTECHAR,
                                    MPFROMCHAR(pTemplate->chQuoteChar), NULL);
            }
      }
   }
   else
      bError=TRUE;

   if (bError)
   {
      /* keine Area vorhanden */
      MSG_ClearMessage(&CurrentHeader, &CurrentMessage);
      strcpy(CurrentAddress, userdaten.address[0]);
      strcpy(CurrentName, userdaten.username[0]);
      CurrentArea[0]='\0';
      pWindowData->ulCurrentID=0;
      pWindowData->pArea=NULL;
   }
   strcpy(miscoptions.lastarearead, CurrentArea);
   UpdateDisplay(TRUE, TRUE);
   return (int) bError;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ProcessWorkMessages                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Verarbeitet alle WORKM_* Messages                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Client-Window                                            */
/*            message: Work-Message                                          */
/*            mp1: Parameter 1 (meistens PMESSAGEID)                         */
/*            mp2: Parameter 2 (meist XMSG*        )                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT ProcessWorkMessages(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PWINDOWDATA pWindowData;

   pWindowData=(PWINDOWDATA) WinQueryWindowULong(hwnd, QWL_USER);

   switch(message)
   {
      case WORKM_ERROR:
         switch(MessageBox(hwnd, IDST_MSG_WORKERROR, 0, IDD_WORKERROR,
                           MB_ERROR | MB_ABORTRETRYIGNORE))
         {
            case MBID_ABORT:
               return (MRESULT) WORK_ERROR_ABORT;

            case MBID_RETRY:
               return (MRESULT) WORK_ERROR_RETRY;

            case MBID_IGNORE:
               return (MRESULT) WORK_ERROR_IGNORE;
         }
         break;

      case WORKM_DELETED:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         if (hwndAreaDlg)
            SendMsg(hwndAreaDlg, message, mp1, mp2);
         if (hwndFindResults)
            SendMsg(hwndFindResults, message, mp1, mp2);
         UnmarkMessage(&MarkerList, ((PMESSAGEID)mp1)->pchAreaTag,
                       ((PMESSAGEID)mp1)->ulMsgID, MARKFLAG_ALL);
         if (!stricmp(CurrentArea, ((PMESSAGEID)mp1)->pchAreaTag) &&
             CurrentStatus == PROGSTATUS_READING)
         {
            if (pWindowData->ulCurrentID == ((PMESSAGEID)mp1)->ulMsgID)
            {
               if (pWindowData->pArea->currentmessage > pWindowData->pArea->maxmessages)
                  pWindowData->pArea->currentmessage = pWindowData->pArea->maxmessages;
               if (pWindowData->pArea->maxmessages > 0)
               {
                  MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
                  pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
                  UpdateDisplay(FALSE, TRUE);
               }
               else
               {
                  MSG_ClearMessage(&CurrentHeader, &CurrentMessage);
                  pWindowData->ulCurrentID = 0;
                  UpdateDisplay(FALSE, TRUE);
               }
            }
            else
            {
               pWindowData->pArea->currentmessage = MSG_UidToMsgn(&arealiste, CurrentArea, pWindowData->ulCurrentID, TRUE);
#if 0
               if (pWindowData->pArea->currentmessage == pWindowData->pArea->maxmessages ||
                   pWindowData->pArea->currentmessage == 1)
                  UpdateDisplay(FALSE, TRUE);
               else
#endif
                  UpdateMsgNum(hwnd, pWindowData);
            }
         }
         break;

      case WORKM_MOVED:
      case WORKM_AREASCANNED:
      case WORKM_COPIED:
      case WORKM_PRINTED:
      case WORKM_EXPORTED:
      case WORKM_READ:
      case WORKM_ENABLEVIEWS:
      case WORKM_DISABLEVIEWS:
      case WORKM_AREADEFCHANGED:
      case WORKM_MARKEND:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         if (hwndAreaDlg)
            SendMsg(hwndAreaDlg, message, mp1, mp2);
         if (hwndFindResults)
            SendMsg(hwndFindResults, message, mp1, mp2);
         break;

      case WORKM_SWITCHACCELS:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         if (hwndAreaDlg)
            SendMsg(hwndAreaDlg, message, mp1, mp2);
         if (hwndFindResults)
            SendMsg(hwndFindResults, message, mp1, mp2);
         if (hwndKludge)
            SendMsg(hwndKludge, message, mp1, mp2);
         if (hwndNLBrowser)
            SendMsg(hwndNLBrowser, message, mp1, mp2);
         SwitchAccelsMain(frame, (ULONG) mp1);
         break;

      case WORKM_CHANGED:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         if (hwndAreaDlg)
            SendMsg(hwndAreaDlg, message, mp1, mp2);
         if (hwndFindResults)
            SendMsg(hwndFindResults, message, mp1, mp2);
         ChangeMarkedMessage(&MarkerList, ((PMESSAGEID)mp1)->pchAreaTag,
                             ((PMESSAGEID)mp1)->ulMsgID, (PMSGHEADER) mp2);
         break;

      case WORKM_STARTWORKAREA:
         OpenProgressBar(pWindowData, (PCHAR) mp1);
         break;

      case WORKM_PROGRESS:
         ProgressBarProgress(pWindowData, (LONG) mp1);
         break;

      case WORKM_ADDED:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         if (hwndAreaDlg)
            SendMsg(hwndAreaDlg, message, mp1, mp2);
         if (!stricmp(CurrentArea, ((PMESSAGEID)mp1)->pchAreaTag))
         {
#if 1
            UpdateMsgNum(hwnd, pWindowData);
            UpdateButtons(pWindowData->pArea);
            UpdateCheckField(pWindowData->pArea, pWindowData);
#else
            UpdateDisplay(TRUE, FALSE);
#endif
         }
         break;

      case WORKM_END:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         if (hwndAreaDlg)
            SendMsg(hwndAreaDlg, message, mp1, mp2);
         CloseProgressBar(pWindowData);
         bDoingWork=FALSE;
         WinAlarm(HWND_DESKTOP, WA_NOTE);
         break;

      case WORKM_REREAD:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         if (!stricmp(CurrentArea, (PCHAR) mp1))
         {
            MSG_ReadAct(&CurrentMessage, &CurrentHeader, &arealiste, CurrentArea);
            pWindowData->ulCurrentID= CurrentHeader.ulMsgID;
            UpdateDisplay(FALSE, TRUE);
         }
         break;

      case WORKM_SHOWMSG:
         if (hwndThreadList)
            SendMsg(hwndThreadList, message, mp1, mp2);
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         break;

      case WORKM_STARTFIND:
         if (!hwndFindResults)
         {
            BOOKMARKSOPEN BMOpen={sizeof(BOOKMARKSOPEN), MARKFLAG_FIND};

            hwndFindResults=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                                       FindResultsProc,
                                       hmodLang, IDD_FINDRESULTS,
                                       &BMOpen);
            WinShowWindow(hwndFindResults, TRUE);
         }
         if (hwndFindResults)
            SendMsg(hwndFindResults, message, mp1, mp2);
         break;

      case WORKM_STOPFIND:
      case WORKM_FINDAREA:
      case WORKM_FINDPROGRESS:
      case WORKM_FINDAREAEND:
         if (hwndFindResults)
            SendMsg(hwndFindResults, message, mp1, mp2);
         break;

      case WORKM_MSGMARKED:
      case WORKM_MSGUNMARKED:
         if (hwndFindResults)
            SendMsg(hwndFindResults, message, mp1, mp2);
         if (((PMESSAGEID)mp1)->ulMsgID == pWindowData->ulCurrentID &&
             !stricmp(CurrentArea, ((PMESSAGEID)mp1)->pchAreaTag))
            WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                              MPFROMLONG(pWindowData->idCheckField),
                              (message==WORKM_MSGMARKED)?MPFROMLONG(TRUE):NULL);
         break;

      case WORKM_TRACKMSG:
         if (hwndMsgList)
            SendMsg(hwndMsgList, message, mp1, mp2);
         break;

      default:
         break;
   }
   return MRFROMLONG(0);
}

/*---------------------------------------------------------------------------*/
/* QuickMessage, Hilfsfunktion */

void QuickMessage(HWND hwndOwner, char *text)
{
   WinMessageBox(HWND_DESKTOP, hwndOwner, text, "Message to you",
                 50000, MB_OK | MB_MOVEABLE);
   return;
}

static void ShutdownProgram(void)
{
   /* Area-Scan-Thread beenden */
   if (DoingAreaScan)
   {
      StopAreaScan=TRUE;
      DosWaitThread((PTID) &tidAreaScan, DCWW_WAIT);
   }

   /* Find-Thread beenden */
   if (DoingFind)
   {
      StopFind=TRUE;
      DosWaitThread((PTID) &tidFind, DCWW_WAIT);
   }

   /* Thread-Listen-Thread beenden */
   if (DoingInsert)
   {
      StopInsert=TRUE;
      DosWaitThread((PTID) &tidThreadList, DCWW_WAIT);
   }

   if (bDoingWork)
   {
      bStopWork=TRUE;
      DosWaitThread((PTID) &tidWorker, DCWW_WAIT);
   }

   if (hAccel1)
      WinDestroyAccelTable(hAccel1);
   if (hAccel2)
      WinDestroyAccelTable(hAccel2);
   if (hAccel3)
      WinDestroyAccelTable(hAccel3);

   if (hwndAreaDlg)
      WinDestroyWindow(hwndAreaDlg);

   if (hwndTemplates)
      WinDestroyWindow(hwndTemplates);

   if (hwndCCLists)
      WinDestroyWindow(hwndCCLists);

   if (hwndRxFolder)
      WinDestroyWindow(hwndRxFolder);

   if (hwndKludge)
      WinDestroyWindow(hwndKludge);

   if (hwndFindResults)
      WinDestroyWindow(hwndFindResults);

   if (hwndThreadList)
      WinDestroyWindow(hwndThreadList);

   if (hwndMsgList)
      WinDestroyWindow(hwndMsgList);

   if (hwndNLBrowser)
      WinDestroyWindow(hwndNLBrowser);

   return;
}

void StartQuickRexx(USHORT menuID)
{
   USHORT sItem=IDM_RXQUICK1;
   PRXSCRIPT pScript = scriptlist.pScripts;

   if (tidRexxExec)
      return;

   while(sItem <= IDM_RXQUICK10 && pScript)
   {
      if (pScript->ulFlags & REXXFLAG_QUICKACCESS)
      {
         if (sItem == menuID)
         {
            if (StartRexxScript(pScript->ulScriptID, NULL))
               WinAlarm(HWND_DESKTOP, WA_ERROR);
            break;
         }
         sItem++;
      }
      pScript = pScript->next;
   }

   return;
}

static void EditMenuCommands(HWND client, ULONG message)
{
   HWND hwndFokus=WinQueryFocus(HWND_DESKTOP);
   USHORT usID = WinQueryWindowUShort(hwndFokus, QWS_ID);

   client=client;

   switch(usID)
   {
      case IDE_FROMNAME:
      case IDE_FROMADDRESS:
      case IDE_TONAME:
      case IDE_TOADDRESS:
      case IDE_SUBJTEXT:
         switch (message)
         {
            case IDM_EDITCUT:
               SendMsg(hwndFokus, EM_CUT, NULL, NULL);
               break;

            case IDM_EDITCOPY:
               SendMsg(hwndFokus, EM_COPY, NULL, NULL);
               break;

            case IDM_EDITPASTE:
               SendMsg(hwndFokus, EM_PASTE, NULL, NULL);
               break;

            case IDM_EDITCLEAR:
               SendMsg(hwndFokus, EM_CLEAR, NULL, NULL);
               break;
         }
         break;

      case IDML_MAINEDIT:
         switch (message)
         {
            case IDM_EDITUNDO:
               SendMsg(hwndFokus, MLM_UNDO, NULL, NULL);
               break;

            case IDM_EDITCUT:
               SendMsg(hwndFokus, MLM_CUT, NULL, NULL);
               break;

            case IDM_EDITCOPY:
               if (CurrentStatus == PROGSTATUS_EDITING)
               {
#if 0
                  MRESULT minmax;

                  minmax=SendMsg(hwndFokus, MLM_QUERYSEL,
                                    MPFROMSHORT(MLFQS_MINMAXSEL),
                                    NULL);
                  if (SHORT2FROMMR(minmax)-SHORT1FROMMR(minmax))
                  {
                     /* Text ist selektiert */
                     SendMsg(hwndFokus, MLM_COPY, NULL, NULL);
                  }
                  else
                  {
                     ULONG length;

                     /* Text ist nicht selektiert, gesamten Text kopieren */
                     SendMsg(hwndFokus, MLM_DISABLEREFRESH, NULL, NULL);
                     length=(ULONG)SendMsg(hwndFokus,
                                     MLM_QUERYTEXTLENGTH, NULL, NULL);
                     SendMsg(hwndFokus, MLM_SETSEL, NULL, MPFROMLONG(length));
                     SendMsg(hwndFokus, MLM_COPY, NULL, NULL);
                     SendMsg(hwndFokus, MLM_SETSEL, NULL, NULL);
                     SendMsg(hwndFokus, MLM_ENABLEREFRESH, NULL, NULL);
                  }
#else
                     SendMsg(hwndFokus, MLM_COPY, NULL, NULL);
#endif
               }
               else
                  SendMsg(hwndFokus, MSGVM_COPY, NULL, NULL);
               break;

            case IDM_EDITPASTE:
               SendMsg(hwndFokus, MLM_PASTE, NULL, NULL);
               break;

            case IDM_EDITCLEAR:
               SendMsg(hwndFokus, MLM_CLEAR, NULL, NULL);
               break;

            case IDM_EDITDELLINE:
               SendMsg(hwndFokus, MLM_DELETELINE, NULL, NULL);
               break;
         }
         break;
   }

   return;
}

static void HandleAttachAttrib(HWND hwndClient)
{
   /* neue Attribute anzeigen */
   DisplayAttrib(CurrentHeader.ulAttrib);

   if (CurrentHeader.ulAttrib & ATTRIB_FILEATTACHED)
   {
      char pchTemp[LEN_SUBJECT+1];

      WinQueryDlgItemText(hwndClient, IDE_SUBJTEXT, LEN_SUBJECT+1, pchTemp);
      if (!pchTemp[0])
      {
         /* noch keine Files eingetragen, Dialog aufrufen */
         if (GetAttachedFiles(hwndClient, pchTemp, generaloptions.attachpath))
         {
            WinSetDlgItemText(hwndClient, IDE_SUBJTEXT, pchTemp);
         }
         else
            return;
      }
      CheckAttaches(pchTemp, hwndClient);
   }
   return;
}

static int QuoteCurrentMessage(PWINDOWDATA pWindowData, BOOL bDiffArea, ULONG ulReplyDest,
                               ULONG ulOptions)
{
   MESSAGEID MessageID;
   PKLUDGE pKludge;

   if (!pWindowData->pArea || !pWindowData->pArea->maxmessages)
      return 1;

   MSG_MarkRead(&arealiste, CurrentArea, 0, CurrentName, &driveremap);
   strcpy(MessageID.pchAreaTag, CurrentArea);
   MessageID.ulMsgID=pWindowData->ulCurrentID;
   SendMsg(client, WORKM_READ, &MessageID, NULL);

   if ((pKludge = MSG_FindKludge(&CurrentMessage, KLUDGE_AREA, NULL)) &&
       AM_FindArea(&arealiste, pKludge->pchKludgeText))
      strcpy(NewArea, pKludge->pchKludgeText);
   else
      strcpy(NewArea, CurrentArea);

   if (bDiffArea)
   {
      if (GetReplyArea(&CurrentMessage, NewArea))
         return 2;
   }

   NewMessage=TRUE;
   SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);
   MSG_QuoteMessage(&templatelist, &CurrentMessage, &CurrentHeader, &arealiste,
                    CurrentArea, NewArea,
                    ulOptions | QUOTE_STRIPRE,
                    ulReplyDest,
                    CurrentName, CurrentAddress, &iptInitialPos);

   if (!stricmp(NewArea, CurrentArea))
   {
      /* ReplyTo einsetzen */
      CurrentHeader.ulReplyTo= MessageID.ulMsgID;
   }
   SwitchEditor(client, NewArea, TRUE);
   CurrentStatus = PROGSTATUS_EDITING;
   UpdateDisplay(TRUE, FALSE);
   bTemplateProcessed=FALSE;
   ulTemplateType=TPLTYPE_QUOTE;
   iptInitialPos2=0;

   /* Address-Matching */
   if (AM_FindArea(&arealiste, NewArea)->areadata.areatype == AREATYPE_NET)
   {
      int iMatch;

      iMatch=MSG_MatchAddress(&CurrentHeader.ToAddress, &userdaten, &CurrentHeader.FromAddress);
      if (iMatch>=0)
         WinSetDlgItemText(client, IDE_FROMADDRESS, userdaten.address[iMatch]);
   }
   StatusChanged(client, CurrentStatus);
   SetFocusControl(client, IDE_SUBJTEXT);

   return 0;
}

static int GetReplyArea(PFTNMESSAGE pCurrentMessage, char *pchDestArea)
{
   AREALISTPAR AreaListPar;

   /* Zielarea holen */
   PKLUDGE pKludge = MSG_FindKludge(pCurrentMessage, KLUDGE_AREA, NULL);

   AreaListPar.cb=sizeof(AREALISTPAR);
   if (pKludge && pKludge->pchKludgeText)
      AreaListPar.pchString=strdup(pKludge->pchKludgeText);
   else
      AreaListPar.pchString=NULL;
   AreaListPar.idTitle = IDST_TITLE_AL_REPLY;
   AreaListPar.ulIncludeTypes = INCLUDE_ALL;
   AreaListPar.bExtendedSel = FALSE;
   AreaListPar.bChange      = FALSE;

   if (WinDlgBox(HWND_DESKTOP, client,
                 AreaListProc, hmodLang,
                 IDD_AREALIST, &AreaListPar)!=DID_OK || !AreaListPar.pchString)
   {
      return 1;
   }
   else
   {
      strcpy(pchDestArea, AreaListPar.pchString);
      free(AreaListPar.pchString);
      return 0;
   }
}

static void ShowSecondaryWindows(BOOL bShow)
{
   if (hwndThreadList)
      WinShowWindow(hwndThreadList, bShow);
   if (hwndFindResults)
      WinShowWindow(hwndFindResults, bShow);
   if (hwndKludge)
      WinShowWindow(hwndKludge, bShow);
   if (hwndMsgList)
      WinShowWindow(hwndMsgList, bShow);
   if (hwndAreaDlg)
      WinShowWindow(hwndAreaDlg, bShow);
   if (hwndTemplates)
      WinShowWindow(hwndTemplates, bShow);
   if (hwndCCLists)
      WinShowWindow(hwndCCLists, bShow);
   if (hwndRxFolder)
      WinShowWindow(hwndRxFolder, bShow);
   if (hwndNLBrowser)
      WinShowWindow(hwndNLBrowser, bShow);

   return;
}

static BOOL SearchInMessage(HWND hwndClient, PSEARCHPAR pSearchPar)
{
   switch(CurrentStatus)
   {
      case PROGSTATUS_READING:
         return (BOOL) WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MSGVM_FINDTEXT,
                                         pSearchPar->pchSearchText,
                              MPFROMLONG(pSearchPar->ulSearchFlags & SEARCHFLAG_CASESENSITIVE));

      case PROGSTATUS_EDITING:
         {
            MLE_SEARCHDATA SearchData;
            ULONG ulStyle = MLFSEARCH_SELECTMATCH;

            if (pSearchPar->ulSearchFlags & SEARCHFLAG_CASESENSITIVE)
               ulStyle |= MLFSEARCH_CASESENSITIVE;

            memset(&SearchData, 0, sizeof(SearchData));
            SearchData.cb = sizeof(SearchData);
            SearchData.pchFind = pSearchPar->pchSearchText;
            SearchData.cchFind = strlen(pSearchPar->pchSearchText);
            SearchData.iptStart = -1;
            SearchData.iptStop = -1;

            return (BOOL) WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_SEARCH,
                                            MPFROMLONG(ulStyle),
                                            &SearchData);
         }

      default:
         return FALSE;
   }
}
/*------------------------------ Programmende -------------------------------*/

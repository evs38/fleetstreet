/*---------------------------------------------------------------------------+
 | Titel: MAINWINDOW.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 22.07.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Bildaufbau des Hauptfensters u. dgl.                                    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_GPI
#define INCL_WIN
#define INCL_BASE
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "resids.h"
#include "messages.h"
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "utility.h"
#include "mainwindow.h"
#include "controls\editwin.h"
#include "controls\msgviewer.h"
#include "controls\statline.h"
#include "finddlg.h"
#include "markmanage.h"
#include "controls\toolbar.h"
#include "toolbarconfig.h"
#include "util\addrcnv.h"
#include "dump\pmassert.h"

/*--------------------------------- Defines ---------------------------------*/

typedef struct
{
   ULONG   CtrlID;      /* ID des Controls */
   LONG    x;           /* x-Koordinate */
   LONG    y;           /* y-Koordinate */
   LONG    cx;          /* x-Groesse */
   LONG    cy;          /* y-Groesse */
   UCHAR   flags;
} RESIZEPARAM;

/* Flags */
#define XREL      0x01U
#define YREL      0x02U
#define CXREL     0x04U
#define CYREL     0x08U
#define VIS       0x10U

static RESIZEPARAM ResizeParam[]=
{
{IDE_FROMNAME,       90,  -24,  -210,   18, YREL | CXREL | VIS},
{IDE_TONAME,         90,  -44,  -210,   18, YREL | CXREL | VIS},
{IDE_FROMADDRESS,     5,  -24,   -10,   18, YREL | CXREL | VIS},
{IDE_TOADDRESS,       5,  -44,   -10,   18, YREL | CXREL},
{IDML_MAINEDIT,       0,    0,     0,  -89, CXREL | CYREL | VIS},
{IDS_FROM,            5,  -24,    80,   18, YREL | VIS},
{IDS_TO,              5,  -44,    80,   18, YREL | VIS},
{IDS_SUBJ,            5,  -64,    80,   18, YREL | VIS},
{IDS_DATEWRITTEN,  -135,  -24,   130,   16, XREL | YREL | VIS},
{IDS_DATERECEIVED, -135,  -44,   130,   16, XREL | YREL | VIS},
{IDE_SUBJTEXT,       90,  -64,  - 95,   18, YREL | CXREL | VIS},
{IDS_ATTRTEXT,       90, - 84,  -235,   18, YREL | CXREL | VIS},
{IDB_CHANGEATTR,      5, - 86,    80,   22, YREL | VIS},
{0, 0, 0, 0, 0, 0}
};

static USHORT FocusTable[]=
{
IDE_FROMNAME,
IDE_FROMADDRESS,
IDE_TONAME,
IDE_TOADDRESS,
IDE_SUBJTEXT,
IDML_MAINEDIT,
IDB_CHANGEATTR
};

typedef struct
{
   USHORT usCtrlID;
   ULONG  ulStringID;
} HELPSTRINGTABLE;


#define MSGTIMEOUT  3000

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HMODULE hmodLang;
extern int tidRexxExec;
extern int MonoDisp, TempMono;

static const HELPSTRINGTABLE MenuHelpStrings[] =
{
{IDM_FILE, IDST_HLP_FI_FILE},
{IDM_FILEIMPORT, IDST_HLP_FI_IMPORT},
{IDM_FILEEXPORT, IDST_HLP_FI_EXPORT},
{IDM_FILEECHOTOSS, IDST_HLP_FI_ECHOTOSS},
{IDM_FILEPRINT, IDST_HLP_FI_PRINT},
{IDM_EXIT, IDST_HLP_FI_EXIT},
{IDM_EDIT, IDST_HLP_ED_EDIT},
{IDM_EDITUNDO, IDST_HLP_ED_UNDO},
{IDM_EDITCUT, IDST_HLP_ED_CUT},
{IDM_EDITCOPY, IDST_HLP_ED_COPY},
{IDM_EDITPASTE, IDST_HLP_ED_PASTE},
{IDM_EDITCLEAR, IDST_HLP_ED_CLEAR},
{IDM_EDITDELLINE, IDST_HLP_ED_CLRLINE},
{IDM_MESSAGE, IDST_HLP_MS_MESSAGE},
{IDM_MSGMOVE, IDST_HLP_MS_MOVE},
{IDM_MSGXPOST, IDST_HLP_MS_XPOST},
{IDM_MSGREQUEST, IDST_HLP_MS_REQ},
{IDM_MSGCCOPY, IDST_HLP_MS_CCOPY},
{IDM_MSGMARK, IDST_HLP_MS_MARK},
{IDM_OPTIONS, IDST_HLP_SU_SETUP},
{IDM_OPCONFIG, IDST_HLP_SU_OPTIONS},
{IDM_OPTEMPLATE, IDST_HLP_SU_TEMPL},
{IDM_OPCCLISTS, IDST_HLP_SU_CCLIST},
{IDM_OPNAMEADDR, IDST_HLP_SU_NAMEAD},
{IDM_OPSAVE, IDST_HLP_SU_SAVE},
{IDM_WINDOWS, IDST_HLP_WI_WINDOWS},
{IDM_WINKLUDGES, IDST_HLP_WI_KLUDGE},
{IDM_WINTHREADS, IDST_HLP_WI_THREAD},
{IDM_WINRESULTS, IDST_HLP_WI_RESULTS},
{IDM_HELP, IDST_HLP_HE_HELP},
{IDM_HELPINDEX, IDST_HLP_HE_INDEX},
{IDM_HELPUSING, IDST_HLP_HE_USING},
{IDM_HELPKEYS, IDST_HLP_HE_KEYS},
{IDM_HELPABOUT, IDST_HLP_HE_ABOUT},
{IDM_HELPGENERAL, IDST_HLP_HE_GENERAL},
{IDM_MSGNEW, IDST_HLP_MS_NEW},
{IDM_MSGCHANGE, IDST_HLP_MS_CHANGE},
{IDM_MSGREPLY, IDST_HLP_MS_REPLY},
{IDM_MSGDELETE, IDST_HLP_MS_DELETE},
{IDM_MSGFIND, IDST_HLP_MS_FIND},
{IDM_FILESHELL, IDST_HLP_FI_SHELL},
{IDM_MSGCOPY, IDST_HLP_MS_COPY},
{IDM_MSGFORWARD, IDST_HLP_MS_FORWARD},
{IDM_WINAREAS, IDST_HLP_WI_AREAS},
{IDM_WINMSGLIST, IDST_HLP_WI_MLIST},
{IDM_HELPCONTENTS, IDST_HLP_HE_CONTENTS},
{IDM_MSGBCAST, IDST_HLP_MS_BCAST},
{IDM_MSGBCDELETE, IDST_HLP_MS_BCDELETE},
{IDM_MSGBCMODIFY, IDST_HLP_MS_BCMODIFY},
{IDM_SPECIAL, IDST_HLP_SP_SPECIAL},
{IDM_MSGQUICKCC, IDST_HLP_MS_QUICKCC},
{IDM_REXX, IDST_HLP_REXX},
{IDM_RXSCRIPTS, IDST_HLP_RXSCRIPTS},
{IDM_SPCADDTONICK, IDST_HLP_ADDTONICK},
{IDM_SPCADDTOCC, IDST_HLP_ADDTOCC},
{IDM_MSGMARKMSG, IDST_HLP_MARKMSG},
{IDM_MSGUNMARKMSG, IDST_HLP_UNMARKMSG},
{IDM_SPCBROWSER, IDST_HLP_SPCBROWSER},
{IDM_OPNICKNAMES, IDST_HLP_OPNICKNAMES},
{IDM_OPECHOMAN, IDST_HLP_OPECHOMAN},
{IDM_OPADDAREAS, IDST_HLP_OPADDAREAS},
{IDM_FILEPRINTSETUP, IDST_HLP_FI_PRINTSETUP},
{IDM_EDITSEARCH, IDST_HLP_ED_SEARCH},
{IDM_TB_ADD, IDST_HLP_TB_ADD},
{0, 0}
};

static const HELPSTRINGTABLE ButtonHelpStrings[] =
{
{IDB_PREVMSG, IDST_HLP_PREVMSG},
{IDB_NEXTMSG, IDST_HLP_NEXTMSG},
{IDB_PREVREPLY, IDST_HLP_PREVREPLY},
{IDB_NEXTREPLY, IDST_HLP_NEXTREPLY},
{IDB_FIRSTMSG, IDST_HLP_FIRSTMSG},
{IDB_LASTMSG, IDST_HLP_LASTMSG},
{IDB_FIND, IDST_HLP_FIND},
{IDB_MSGTREE, IDST_HLP_MSGTREE},
{IDB_NEWMSG, IDST_HLP_NEWMSG},
{IDB_REPLY, IDST_HLP_REPLY},
{IDB_IMPORT, IDST_HLP_IMPORT},
{IDB_EXPORT, IDST_HLP_EXPORT},
{IDB_DELMSG, IDST_HLP_DELMSG},
{IDB_EDITMSG, IDST_HLP_EDITMSG},
{IDB_AREA, IDST_HLP_AREA},
{IDB_MSGLIST, IDST_HLP_MSGLIST},
{IDB_PRINTMSG, IDST_HLP_PRINTMSG},
{IDB_SHOWKLUDGES, IDST_HLP_SHOWKLUDGES},
{IDB_OK, IDST_HLP_OK},
{IDB_CANCEL, IDST_HLP_CANCEL},
{IDB_CHANGEATTR, IDST_HLP_CHANGEATTR},
{IDB_HOMEMSG, IDST_HLP_HOMEMSG},
{IDB_NEXTAREA, IDST_HLP_NEXTAREA},
{IDB_BOOKMARKS, IDST_HLP_WI_RESULTS},
{IDB_HELP, IDST_HLP_HE_GENERAL},
{IDB_CUT, IDST_HLP_ED_CUT},
{IDB_COPY,    IDST_HLP_ED_COPY},
{IDB_PASTE,   IDST_HLP_ED_PASTE},
{IDB_COPYMSG, IDST_HLP_MS_COPY},
{IDB_SHELL,   IDST_HLP_FI_SHELL},
{IDB_SCRIPTS, IDST_HLP_RXSCRIPTS},
{IDB_MOVEMSG, IDST_HLP_MS_MOVE},
{IDB_BROWSER, IDST_HLP_SPCBROWSER},
{IDB_FORWARD, IDST_HLP_MS_FORWARD},
{IDB_CATCHUP, IDST_HLP_MS_MARK},
{IDB_REQUEST, IDST_HLP_MS_REQ},
{0, 0}
};

/*--------------------------- Funktionsprototypen ---------------------------*/
static void EnableToolbarItems(int num, ...);
static void DisableToolbarItems(int num, ...);
static void EnableMenuItems(HWND hwndMenu, BOOL bNewState, int num, ...);
static void SetMainTitle(AREADEFLIST *pAreaDef);
static MRESULT SetReadOnly(HWND hwnd, ULONG Control, BOOL bReadOnly);

/*------------------------------ UpdateDisplay  -----------------------------*/
/* Zeichnet den Bildschirm neu, fuellt alle Fehler, aktiviert Controls       */
/*---------------------------------------------------------------------------*/

void UpdateDisplay(BOOL isNewArea, BOOL withText)
{
   extern char CurrentArea[LEN_AREATAG+1];
   extern AREALIST arealiste;
   extern HWND client, frame;
   extern BOOL NewMessage;
   extern char NewArea[LEN_AREATAG+1];
   extern int CurrentStatus;

   AREADEFLIST *zeiger=NULL;
   PWINDOWDATA pWindowData=(PWINDOWDATA) WinQueryWindowULong(client, QWL_USER);

   DisplayMessage(withText);

   zeiger=AM_FindArea(&arealiste, NewMessage?NewArea:CurrentArea);

   if (!zeiger)
   {
      SetMainTitle(NULL);

      WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idAddressField), "");
      if (CurrentStatus != PROGSTATUS_EDITING)
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_DISABLEDRAG, NULL, NULL);
   }
   if (zeiger && CurrentStatus==PROGSTATUS_EDITING)
   {
      SetTranslateMode(!(zeiger->areadata.ulAreaOpt & AREAOPT_HIGHASCII));
   }

   if (zeiger)
   {
      /* Adressen bei Echo<->Netmail */
      if (zeiger->areadata.areatype != AREATYPE_NET)
         WinShowWindow(WinWindowFromID(client, IDE_TOADDRESS), FALSE);
      else
         WinShowWindow(WinWindowFromID(client, IDE_TOADDRESS), TRUE);

      if (isNewArea)
      {
         /* Areabeschreibung */
         SetMainTitle(zeiger);

         WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                           MPFROMLONG(pWindowData->idAddressField),
                           zeiger->areadata.address);
      }

      if (!NewMessage)
      {
         MESSAGEID MessageID;

         strcpy(MessageID.pchAreaTag, CurrentArea);
         MessageID.ulMsgID = pWindowData->ulCurrentID;
         SendMsg(client, WORKM_TRACKMSG, &MessageID, NULL);
      }
   }

   UpdateButtons(zeiger);

   return;
}

static void SetMainTitle(AREADEFLIST *pAreaDef)
{
   extern char pchWindowTitle[100];
   extern HWND frame;
   extern AREALISTOPTIONS arealistoptions;

   char pchTitle[30+LEN_AREADESC+1];
   char chType='?';

   if (!pAreaDef)
   {
      char pchTemp[21];

      LoadString(IDST_MW_NOAREA, 20, pchTemp);
      sprintf(pchTitle, "%s - %s", pchWindowTitle, pchTemp);
   }
   else
   {
      switch(pAreaDef->areadata.areatype)
      {
         case AREATYPE_NET:
            chType = 'N';
            break;

         case AREATYPE_ECHO:
            chType = 'E';
            break;

         case AREATYPE_LOCAL:
            chType = 'L';
            break;
      }

      sprintf(pchTitle, "%s - %s (%c)", pchWindowTitle,
              (arealistoptions.ulFlags & AREALISTFLAG_SHOWTAGS)?pAreaDef->areadata.areatag:pAreaDef->areadata.areadesc,
              chType);
   }
   WinSetWindowText(frame, pchTitle);

   return;
}

/*------------------------------ UpdateMsgNum   -----------------------------*/
/* Message-Nummer anpassen                                                   */
/*---------------------------------------------------------------------------*/

void UpdateMsgNum(HWND hwndClient, PWINDOWDATA pWindowData)
{
   AREADEFLIST *zeiger;
   static char rangebuf[15];
   static char pchFormatRange[50]="";
   extern char CurrentArea[LEN_AREATAG+1];
   extern AREALIST arealiste;
   int msgnum;

   zeiger=AM_FindArea(&arealiste, CurrentArea);

   if (zeiger)
   {
      msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, pWindowData->ulCurrentID, FALSE);
      if (msgnum)
         zeiger->currentmessage=msgnum;
      if (zeiger->maxmessages > 0 && zeiger->currentmessage == 0)
         zeiger->currentmessage = 1;

      if (pchFormatRange[0]=='\0')
         LoadString(IDST_FORMAT_RANGE, 50, pchFormatRange);

      sprintf(rangebuf, pchFormatRange, zeiger->currentmessage, zeiger->maxmessages);
      WinSendDlgItemMsg(WinQueryWindow(hwndClient, QW_PARENT), FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idNumberField),
                        rangebuf);
   }
   else
   {
      WinSendDlgItemMsg(WinQueryWindow(hwndClient, QW_PARENT), FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idNumberField), "");
   }
   return;
}

void UpdateButtons(AREADEFLIST *zeiger)
{
   extern MSGHEADER CurrentHeader;
   extern HWND client, frame;
   extern int CurrentStatus;
   extern char CurrentArea[LEN_AREATAG+1];
   extern AREALIST arealiste;

   /* Anfang der Messages */
   if (!zeiger || zeiger->currentmessage <=1)
      DisableToolbarItems(2, IDB_PREVMSG, IDB_FIRSTMSG);
   else
      EnableToolbarItems(2, IDB_PREVMSG, IDB_FIRSTMSG);

   /* Ende der Messages */
   if (!zeiger || zeiger->currentmessage == zeiger->maxmessages)
      DisableToolbarItems(2, IDB_NEXTMSG, IDB_LASTMSG);
   else
      EnableToolbarItems(2, IDB_NEXTMSG, IDB_LASTMSG);

   /* ist es ein Reply ? */
   if (zeiger && CurrentHeader.ulReplyTo && zeiger->maxmessages &&
       MSG_UidToMsgn(&arealiste, CurrentArea, CurrentHeader.ulReplyTo, TRUE))
      EnableToolbarItems(1, IDB_PREVREPLY);
   else
      DisableToolbarItems(1, IDB_PREVREPLY);

   /* sind Replies da ? */
   if (zeiger && CurrentHeader.ulReplies[0] && zeiger->maxmessages &&
       MSG_UidToMsgn(&arealiste, CurrentArea, CurrentHeader.ulReplies[0], TRUE))
      EnableToolbarItems(1, IDB_NEXTREPLY);
   else
      DisableToolbarItems(1, IDB_NEXTREPLY);

   /* Messages da? */
   if (!zeiger || zeiger->maxmessages==0)
   {
      DisableToolbarItems(13, IDB_REPLY, IDB_EDITMSG, IDB_MSGTREE, IDB_EXPORT,
                          IDB_DELMSG, IDB_MSGLIST, IDB_PRINTMSG, IDB_HOMEMSG,
                          IDB_MOVEMSG, IDB_COPYMSG, IDB_FORWARD, IDB_REQUEST, IDB_CATCHUP );
      if (!zeiger)
         DisableToolbarItems(1, IDB_NEWMSG);
      else
         EnableToolbarItems(1, IDB_NEWMSG);

      if (CurrentStatus != PROGSTATUS_EDITING)
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_DISABLEDRAG, NULL, NULL);
   }
   else
   {
      EnableToolbarItems(15, IDB_REPLY, IDB_EDITMSG, IDB_FIND, IDB_MSGTREE,
                             IDB_EXPORT, IDB_DELMSG, IDB_NEWMSG, IDB_MSGLIST,
                             IDB_PRINTMSG, IDB_HOMEMSG, IDB_MOVEMSG, IDB_COPYMSG,
                             IDB_FORWARD, IDB_CATCHUP, IDB_REQUEST);

      if (CurrentStatus != PROGSTATUS_EDITING)
         WinSendDlgItemMsg(client, IDML_MAINEDIT, MSGVM_ENABLEDRAG, NULL, NULL);
   }

   return;
}

/*------------------------------ DisplayMessage -----------------------------*/
/* Zeigt die Message-Daten an                                                */
/*---------------------------------------------------------------------------*/

void DisplayMessage(BOOL withText)
{
   extern FTNMESSAGE CurrentMessage;
   extern MSGHEADER  CurrentHeader;
   extern int CurrentStatus;
   extern GENERALOPT generaloptions;
   extern char CurrentArea[LEN_AREATAG+1];
   extern HWND client, frame;
   extern HWND hwndKludge;
   extern AREALIST arealiste;
   extern USERDATAOPT userdaten;
   extern BOOL NewMessage;
   extern WINDOWFONTS windowfonts;
   AREADEFLIST *zeiger;
   PWINDOWDATA pWindowData;

   static char addrbuf[LEN_5DADDRESS+1];
   static char datebuf[22];
   static char rangebuf[15];
   static char pchFormatRange[50]="";

   int i;
   HWND hwnd;

   zeiger=AM_FindArea(&arealiste, CurrentArea);
   pWindowData=(PWINDOWDATA) WinQueryWindowULong(client, QWL_USER);

   hwnd=WinWindowFromID(client, IDML_MAINEDIT);

   /* NPD-Flag auswerten */
   if (!MonoDisp) /* noch prop. */
      if ((CurrentHeader.ulAttrib & ATTRIB_NPD) &&
          !TempMono)
      {
         /* auf Monospaced umschalten */
         QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewerfont);
         SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewermonofont);
         TempMono = TRUE;
      }
      else
         if (!(CurrentHeader.ulAttrib & ATTRIB_NPD) &&
             TempMono)
         {
            /* auf proportional umschalten */
            QueryControlFont(client, IDML_MAINEDIT, windowfonts.viewermonofont);
            SetFont(WinWindowFromID(client, IDML_MAINEDIT), windowfonts.viewerfont);
            TempMono = FALSE;
         }

   if (withText)
   {
      if (zeiger && (zeiger->maxmessages!=0 || NewMessage) && CurrentMessage.pchMessageText)
         DisplayMsgText(client, &CurrentMessage);
      else
         WinSetWindowText(hwnd, "");
   }
   else
      WinSetWindowText(hwnd, "");

   if (zeiger && hwndKludge)
      SendMsg(hwndKludge, KM_SHOWKLUDGES,
                 MPFROMP(&CurrentMessage), NULL);

   if (!zeiger || (zeiger->maxmessages==0 && CurrentStatus!=PROGSTATUS_EDITING))
   {
      WinSetDlgItemText(client, IDE_FROMNAME, "");
      WinSetDlgItemText(client, IDE_TONAME, "");
      WinSetDlgItemText(client, IDE_SUBJTEXT, "");
   }
   else
   {
      WinSetDlgItemText(client, IDE_FROMNAME, CurrentHeader.pchFromName);
      WinSetDlgItemText(client, IDE_TONAME, CurrentHeader.pchToName);
      WinSetDlgItemText(client, IDE_SUBJTEXT, CurrentHeader.pchSubject);
   }

   /* Pieps bei pers. Mail */
   if (zeiger && zeiger->maxmessages &&
       generaloptions.beeponpersonal &&
       CurrentStatus == PROGSTATUS_READING &&
       !(CurrentHeader.ulAttrib & ATTRIB_READ) &&
       CurrentHeader.pchToName[0])
      for (i=0; i<MAX_USERNAMES; i++)
         if (!stricmp(CurrentHeader.pchToName, userdaten.username[i]))
            WinAlarm(HWND_DESKTOP, WA_NOTE);

   if (!zeiger)
   {
      WinSetDlgItemText(client, IDE_FROMADDRESS, "");
      WinSetDlgItemText(client, IDE_TOADDRESS, "");
   }
   else
   {
      if (CurrentStatus==PROGSTATUS_EDITING || (zeiger->maxmessages>0))
      {
         NetAddrToString(addrbuf, &CurrentHeader.FromAddress);
         WinSetDlgItemText(client, IDE_FROMADDRESS, addrbuf);
      }
      else
         WinSetDlgItemText(client, IDE_FROMADDRESS, "");

      if (zeiger->maxmessages>0 || CurrentStatus==PROGSTATUS_EDITING )
      {
         NetAddrToString(addrbuf, &CurrentHeader.ToAddress);
         WinSetDlgItemText(client, IDE_TOADDRESS, addrbuf);
      }
      else
         WinSetDlgItemText(client, IDE_TOADDRESS, "");
   }


   if (zeiger && (zeiger->maxmessages || CurrentStatus == PROGSTATUS_EDITING))
   {
      StampToString(datebuf, &CurrentHeader.StampWritten);
      WinSetDlgItemText(client, IDS_DATEWRITTEN, datebuf);
   }
   else
      WinSetDlgItemText(client, IDS_DATEWRITTEN, "");

   if (zeiger && zeiger->maxmessages && !(CurrentHeader.ulAttrib & ATTRIB_LOCAL))
   {
      StampToString(datebuf, &CurrentHeader.StampArrived);
      WinSetDlgItemText(client, IDS_DATERECEIVED, datebuf);
   }
   else
      WinSetDlgItemText(client, IDS_DATERECEIVED, "");

   if (zeiger && (zeiger->maxmessages>0 || CurrentStatus==PROGSTATUS_EDITING))
      DisplayAttrib(CurrentHeader.ulAttrib);
   else
      WinSetDlgItemText(client, IDS_ATTRTEXT, "");

   if (zeiger && CurrentStatus == PROGSTATUS_READING)
   {
      if (pchFormatRange[0]=='\0')
         LoadString(IDST_FORMAT_RANGE, 50, pchFormatRange);
      sprintf(rangebuf, pchFormatRange, zeiger->currentmessage, zeiger->maxmessages);
      WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idNumberField),
                        rangebuf);
   }
   else
   {
      WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idNumberField), "");
   }
   UpdateCheckField(zeiger, pWindowData);

   return;
}

void UpdateCheckField(AREADEFLIST *zeiger, PWINDOWDATA pWindowData)
{
   extern HWND frame;
   extern int CurrentStatus;

   if (zeiger && CurrentStatus == PROGSTATUS_READING)
   {
      extern MARKERLIST MarkerList;

      if (IsMessageMarked(&MarkerList, zeiger->areadata.areatag, pWindowData->ulCurrentID, MARKFLAG_MANUAL))
         WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                           MPFROMLONG(pWindowData->idCheckField), MPFROMLONG(TRUE));
      else
         WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                           MPFROMLONG(pWindowData->idCheckField), NULL);
   }
   else
      WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idCheckField), NULL);

   return;
}

/*------------------------------ DisplayAttrib  -----------------------------*/
/* Zeigt die Message-Attribute an                                            */
/*---------------------------------------------------------------------------*/

void DisplayAttrib(ULONG attr)
{
   extern HWND client;
   static char flagsbuf[200];

   MSG_AttribToText(attr, flagsbuf);
   WinSetDlgItemText(client, IDS_ATTRTEXT, flagsbuf);

   return;
}

/*------------------------------ DisplayMsgText -----------------------------*/
/* Zeigt den Message-Text an                                                 */
/*---------------------------------------------------------------------------*/

void DisplayMsgText(HWND hwndClient, FTNMESSAGE *msginfo)
{
   int inspoint=0;
   HWND hwnd;
   extern int CurrentStatus;
   ULONG ulRemainLen, ulInsertLen;
   PCHAR pchHelp, pchBuf;

   hwnd=WinWindowFromID(hwndClient, IDML_MAINEDIT);

   if (CurrentStatus == PROGSTATUS_EDITING)
   {
      ulRemainLen = strlen(msginfo->pchMessageText);
      pchHelp = msginfo->pchMessageText;

      DosAllocMem((PPVOID)&pchBuf, 65000, OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE);
      SendMsg(hwnd, MLM_FORMAT, (MPARAM) MLFIE_NOTRANS, NULL);
      SendMsg(hwnd, MLM_DISABLEREFRESH, NULL, NULL);

      while(ulRemainLen >0)
      {
         if (ulRemainLen>=65000)
            ulInsertLen=65000;
         else
            ulInsertLen= ulRemainLen;

         memcpy(pchBuf, pchHelp, ulInsertLen);

         inspoint=(int)SendMsg(hwnd, MLM_QUERYTEXTLENGTH, NULL, NULL);

         SendMsg(hwnd, MLM_SETIMPORTEXPORT, pchBuf, MPFROMLONG(ulInsertLen));
         SendMsg(hwnd, MLM_IMPORT, &inspoint, MPFROMLONG(ulInsertLen));
         pchHelp += ulInsertLen;
         ulRemainLen -= ulInsertLen;
      }

      SendMsg(hwnd, MLM_ENABLEREFRESH, NULL, NULL);
      DosFreeMem(pchBuf);
   }
   else
      if (msginfo->pchMessageText)
      {
         WNDPARAMS WndParams;

         WndParams.fsStatus = WPM_TEXT;
         WndParams.cchText = strlen(msginfo->pchMessageText)+1; /* incl. \0 */
         WndParams.pszText = msginfo->pchMessageText;

         SendMsg(hwnd, WM_SETWINDOWPARAMS, &WndParams, NULL);
      }
      else
         WinSetWindowText(hwnd, "");
   return;
}


/*------------------------------ QueryLayout    -----------------------------*/
/* Fragt Farben und Fonts und Position vom Hauptfenster ab.                  */
/*---------------------------------------------------------------------------*/

void QueryLayout(HWND fenster)
{
   extern WINDOWPOSITIONS windowpositions;
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS windowfonts;
   extern HWND hwndmenu;
   extern HWND frame;
   extern int CurrentStatus;

   QueryWinPos(frame, &(windowpositions.mainwindowpos));

   QueryControlForeground(frame, FID_STATUSLINE, &windowcolors.statusfore);
   if (CurrentStatus == PROGSTATUS_EDITING)
   {
      QueryControlForeground(fenster, IDML_MAINEDIT, &windowcolors.editfore);
      QueryControlBackground(fenster, IDML_MAINEDIT, &windowcolors.editback);
   }
   else
   {
      HWND hwndView = WinWindowFromID(fenster, IDML_MAINEDIT);

      if (MonoDisp || TempMono)
         QueryFont(hwndView, windowfonts.viewermonofont);
      else
         QueryFont(hwndView, windowfonts.viewerfont);
      SendMsg(hwndView, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_TEXT), &windowcolors.viewerfore);
      SendMsg(hwndView, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_BACKGROUND), &windowcolors.viewerback);
      SendMsg(hwndView, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_QUOTE), &windowcolors.viewerquote);
      SendMsg(hwndView, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_QUOTE2), &windowcolors.viewerquote2);
      SendMsg(hwndView, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_TEARLINE), &windowcolors.viewertearline);
      SendMsg(hwndView, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_ORIGIN), &windowcolors.viewerorigin);
   }
   QueryControlForeground(fenster, IDS_DATEWRITTEN, &windowcolors.msgtimefore);
   QueryControlForeground(fenster, IDE_FROMNAME, &windowcolors.fromfore);
   QueryControlForeground(fenster, IDE_TONAME, &windowcolors.tofore);
   QueryControlForeground(fenster, IDE_FROMADDRESS, &windowcolors.fromadfore);
   QueryControlForeground(fenster, IDE_TOADDRESS, &windowcolors.toadfore);
   QueryControlForeground(fenster, IDE_SUBJTEXT, &windowcolors.subjfore);
   QueryControlForeground(fenster, IDS_ATTRTEXT, &windowcolors.attribfore);
   QueryControlForeground(fenster, IDS_FROM, &windowcolors.fromtostaticfore);

   QueryBackground(fenster, &windowcolors.windowback);
   QueryControlBackground(fenster, IDE_FROMNAME, &windowcolors.fromback);
   QueryControlBackground(fenster, IDE_TONAME, &windowcolors.toback);
   QueryControlBackground(fenster, IDE_FROMADDRESS, &windowcolors.fromadback);
   QueryControlBackground(fenster, IDE_TOADDRESS, &windowcolors.toadback);
   QueryControlBackground(fenster, IDE_SUBJTEXT, &windowcolors.subjback);
   QueryControlBackground(frame, FID_STATUSLINE, &windowcolors.statusback);
   QueryControlBackground(fenster, IDS_FROM, &windowcolors.fromtostaticback);
   QueryControlBackground(fenster, IDS_DATEWRITTEN, &windowcolors.msgtimeback);
   QueryControlBackground(fenster, IDS_ATTRTEXT, &windowcolors.attribback);

   QueryFont(hwndmenu, windowfonts.menufont);
   QueryControlFont(fenster, IDE_FROMNAME, windowfonts.fromfont);
   QueryControlFont(fenster, IDE_TONAME, windowfonts.tofont);
   QueryControlFont(fenster, IDE_FROMADDRESS, windowfonts.fromadfont);
   QueryControlFont(fenster, IDE_TOADDRESS, windowfonts.toadfont);
   QueryControlFont(fenster, IDE_SUBJTEXT, windowfonts.subjfont);
   QueryControlFont(fenster, IDS_ATTRTEXT, windowfonts.attribfont);
   QueryControlFont(frame, FID_STATUSLINE, windowfonts.statusfont);
   QueryControlFont(fenster, IDS_DATEWRITTEN, windowfonts.datefont);
   QueryControlFont(fenster, IDB_CHANGEATTR, windowfonts.buttonfont);
   QueryControlFont(fenster, IDS_FROM, windowfonts.fromtofont);
   return;
}

/*------------------------------ InitMenus      -----------------------------*/
/* Initialisierungsfunktion fuer Menues                                      */
/*---------------------------------------------------------------------------*/

void InitMenus(USHORT usMenuID, HWND hwndMenuWnd)
{
   extern int CurrentStatus;
   extern HWND hwndhelp, client, hwndRequester, frame;
   extern BOOL issecondinstance;
   extern BOOL NewMessage, bDoingWork, bDoingBrowse, DoingFind;
   extern char CurrentArea[LEN_AREATAG+1];
   extern char NewArea[LEN_AREATAG+1];
   extern AREALIST arealiste;
   extern char *pchXPostList;
   extern CCANCHOR ccanchor;
   extern ULONG ulCCSelected;
   extern ECHOTOSSOPT echotossoptions;
   extern MSGHEADER CurrentHeader;
   extern FTNMESSAGE CurrentMessage;
   extern PCCLIST pQuickCCList;
   extern PDOMAINS domains;
   AREADEFLIST *zeiger;
   PWINDOWDATA pWindowData=(PWINDOWDATA) WinQueryWindowULong(client, QWL_USER);

   switch(usMenuID)
   {
      case IDM_FILE:
         switch(CurrentStatus)
         {
            case PROGSTATUS_READING:
               EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_FILEIMPORT);
               zeiger=AM_FindArea(&arealiste, CurrentArea);
               if (!zeiger || zeiger->maxmessages==0)
                  EnableMenuItems(hwndMenuWnd, FALSE, 2, IDM_FILEEXPORT, IDM_FILEPRINT);
               else
                  EnableMenuItems(hwndMenuWnd, TRUE, 2, IDM_FILEEXPORT, IDM_FILEPRINT);
               if (echotossoptions.useechotoss &&
                   echotossoptions.pchEchoToss[0])
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_FILEECHOTOSS);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_FILEECHOTOSS);
               break;

            case PROGSTATUS_NOSETUP:
            case PROGSTATUS_CLEANUP:
               EnableMenuItems(hwndMenuWnd, FALSE, 4,IDM_FILEIMPORT, IDM_FILEEXPORT,
                                                     IDM_FILEECHOTOSS, IDM_FILEPRINT);
               break;

            case PROGSTATUS_EDITING:
               EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_FILEIMPORT);
               EnableMenuItems(hwndMenuWnd, FALSE, 3, IDM_FILEEXPORT, IDM_FILEECHOTOSS,
                               IDM_FILEPRINT);
               break;
         }
         break;

      case IDM_EDIT:
         switch(CurrentStatus)
         {
            case PROGSTATUS_READING:
               EnableMenuItems(hwndMenuWnd, FALSE, 5, IDM_EDITCUT, IDM_EDITPASTE,
                                                      IDM_EDITCLEAR, IDM_EDITUNDO,
                                                      IDM_EDITDELLINE);
               zeiger=AM_FindArea(&arealiste, CurrentArea);
               if (!zeiger || zeiger->maxmessages==0)
                  EnableMenuItems(hwndMenuWnd, FALSE, 2, IDM_EDITCOPY, IDM_EDITSEARCH);
               if (zeiger && zeiger->maxmessages!=0)
                  EnableMenuItems(hwndMenuWnd, TRUE, 2, IDM_EDITCOPY, IDM_EDITSEARCH);
               break;

            case PROGSTATUS_NOSETUP:
            case PROGSTATUS_CLEANUP:
               EnableMenuItems(hwndMenuWnd, FALSE, 7, IDM_EDITCUT, IDM_EDITPASTE,
                                                     IDM_EDITCLEAR, IDM_EDITUNDO,
                                                     IDM_EDITCOPY, IDM_EDITDELLINE, IDM_EDITSEARCH);
               break;

            case PROGSTATUS_EDITING:
               EnableMenuItems(hwndMenuWnd, TRUE, 4, IDM_EDITCUT, IDM_EDITCLEAR, IDM_EDITDELLINE, IDM_EDITSEARCH);
               WinOpenClipbrd(anchor);
               if (WinQueryClipbrdFmtInfo(anchor, CF_TEXT, NULL))
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_EDITPASTE);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_EDITPASTE);
               WinCloseClipbrd(anchor);
               if (SHORT1FROMMR(WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_QUERYUNDO, NULL, NULL)))
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_EDITUNDO);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_EDITUNDO);
               break;
         }
         break;

      case IDM_OPTIONS:
         if (issecondinstance)
            EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_OPSAVE);
         else
            EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_OPSAVE);
         switch(CurrentStatus)
         {
            case PROGSTATUS_READING:
               if (!tidRexxExec && !bDoingWork && !bDoingBrowse && !DoingFind)
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_OPCONFIG);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_OPCONFIG);
               zeiger=AM_FindArea(&arealiste, CurrentArea);
               if (!zeiger)
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_OPNAMEADDR);
               else
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_OPNAMEADDR);
               if (zeiger && zeiger->areadata.areatype == AREATYPE_NET)
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_OPECHOMAN);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_OPECHOMAN);
               EnableMenuItems(hwndMenuWnd, TRUE, 3, IDM_OPCCLISTS, IDM_OPTEMPLATE, IDM_TB_ADD);
               if (zeiger && zeiger->areadata.areatype != AREATYPE_ECHO)
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_OPADDAREAS);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_OPADDAREAS);
               break;

            case PROGSTATUS_NOSETUP:
               EnableMenuItems(hwndMenuWnd, TRUE, 4, IDM_OPCONFIG, IDM_OPCCLISTS, IDM_OPTEMPLATE, IDM_TB_ADD);
               EnableMenuItems(hwndMenuWnd, FALSE, 3, IDM_OPNAMEADDR, IDM_OPECHOMAN, IDM_OPADDAREAS);
               break;

            case PROGSTATUS_EDITING:
               EnableMenuItems(hwndMenuWnd, FALSE, 3, IDM_OPCONFIG, IDM_OPECHOMAN, IDM_OPADDAREAS);
               EnableMenuItems(hwndMenuWnd, TRUE, 3, IDM_OPCCLISTS, IDM_OPTEMPLATE, IDM_TB_ADD);
               if (!AM_FindArea(&arealiste, NewArea))
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_OPNAMEADDR);
               else
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_OPNAMEADDR);
               break;

            case PROGSTATUS_CLEANUP:
               EnableMenuItems(hwndMenuWnd, FALSE, 7, IDM_OPCONFIG, IDM_OPCCLISTS, IDM_OPTEMPLATE,
                                                      IDM_OPNAMEADDR, IDM_OPECHOMAN, IDM_OPADDAREAS, IDM_TB_ADD);
               break;
         }
         break;

      case IDM_MESSAGE:
         switch(CurrentStatus)
         {
            case PROGSTATUS_READING:
               EnableMenuItems(hwndMenuWnd, TRUE, 2, IDM_MSGFIND, IDM_MSGNEW);
               zeiger=AM_FindArea(&arealiste, CurrentArea);
               if (!zeiger || zeiger->maxmessages==0)
               {
                  EnableMenuItems(hwndMenuWnd, FALSE, 9, IDM_MSGMOVE, IDM_MSGCOPY, IDM_MSGFORWARD,
                                                        IDM_MSGDELETE, IDM_MSGREPLY, IDM_MSGCHANGE,
                                                        IDM_MSGBCAST, IDM_MSGMARKMSG, IDM_MSGUNMARKMSG);
                  if (!zeiger)
                     EnableMenuItems(hwndMenuWnd, FALSE, 2, IDM_MSGFIND, IDM_MSGNEW);
               }
               else
               {
                  extern MARKERLIST MarkerList;

                  EnableMenuItems(hwndMenuWnd, TRUE, 6, IDM_MSGMOVE, IDM_MSGCOPY, IDM_MSGFORWARD,
                                                        IDM_MSGDELETE, IDM_MSGREPLY, IDM_MSGCHANGE);
                  if (!IsMessageMarked(&MarkerList, CurrentArea, pWindowData->ulCurrentID, MARKFLAG_MANUAL))
                     EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_MSGMARKMSG);
                  else
                     EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_MSGMARKMSG);
                  if (IsMessageMarked(&MarkerList, CurrentArea, pWindowData->ulCurrentID, MARKFLAG_MANUAL))
                     EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_MSGUNMARKMSG);
                  else
                     EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_MSGUNMARKMSG);
                  if (zeiger->areadata.areatype == AREATYPE_ECHO &&
                      (CurrentHeader.ulAttrib & ATTRIB_LOCAL) &&
                      (CurrentHeader.ulAttrib & (ATTRIB_SENT | ATTRIB_SCANNED)) &&
                      MSG_FindKludge(&CurrentMessage, KLUDGE_MSGID, NULL))
                     EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_MSGBCAST);
                  else
                     EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_MSGBCAST);
               }
               break;

            case PROGSTATUS_NOSETUP:
            case PROGSTATUS_CLEANUP:
               EnableMenuItems(hwndMenuWnd, FALSE, 11, IDM_MSGMOVE, IDM_MSGCOPY, IDM_MSGFORWARD,
                                                       IDM_MSGDELETE, IDM_MSGREPLY, IDM_MSGCHANGE,
                                                       IDM_MSGFIND, IDM_MSGNEW, IDM_MSGBCAST,
                                                       IDM_MSGMARKMSG, IDM_MSGUNMARKMSG);
               break;

            case PROGSTATUS_EDITING:
               EnableMenuItems(hwndMenuWnd, FALSE, 11, IDM_MSGMOVE, IDM_MSGCOPY, IDM_MSGFORWARD,
                                                       IDM_MSGDELETE, IDM_MSGREPLY, IDM_MSGCHANGE,
                                                       IDM_MSGFIND, IDM_MSGNEW, IDM_MSGBCAST,
                                                       IDM_MSGMARKMSG, IDM_MSGUNMARKMSG);
               break;
         }
         break;

      case IDM_SPECIAL:
         switch(CurrentStatus)
         {
            case PROGSTATUS_READING:
               EnableMenuItems(hwndMenuWnd, FALSE, 3, IDM_MSGXPOST, IDM_MSGCCOPY, IDM_MSGQUICKCC);
               WinCheckMenuItem(hwndMenuWnd, IDM_MSGXPOST, FALSE);
               WinCheckMenuItem(hwndMenuWnd, IDM_MSGCCOPY, FALSE);
               WinCheckMenuItem(hwndMenuWnd, IDM_MSGQUICKCC, FALSE);
               zeiger=AM_FindArea(&arealiste, CurrentArea);
               if (!zeiger || zeiger->maxmessages==0)
               {
                  EnableMenuItems(hwndMenuWnd, FALSE, 4, IDM_MSGREQUEST, IDM_MSGMARK,
                                                         IDM_SPCADDTONICK, IDM_SPCADDTOCC);
               }
               else
               {
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_SPCADDTONICK);
                  EnableMenuItems(hwndMenuWnd, ccanchor.ulNumLists, 1, IDM_SPCADDTOCC);
                  EnableMenuItems(hwndMenuWnd, !bDoingWork, 1, IDM_MSGMARK);
                  EnableMenuItems(hwndMenuWnd, !hwndRequester, 1, IDM_MSGREQUEST);
               }
               if (domains)
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_SPCBROWSER);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_SPCBROWSER);
               break;

            case PROGSTATUS_NOSETUP:
            case PROGSTATUS_CLEANUP:
               EnableMenuItems(hwndMenuWnd, FALSE, 8, IDM_MSGREQUEST, IDM_MSGXPOST, IDM_MSGCCOPY,
                                                      IDM_MSGMARK, IDM_MSGQUICKCC, IDM_SPCADDTONICK,
                                                      IDM_SPCADDTOCC, IDM_SPCBROWSER);
               WinCheckMenuItem(hwndMenuWnd, IDM_MSGXPOST, FALSE);
               WinCheckMenuItem(hwndMenuWnd, IDM_MSGCCOPY, FALSE);
               WinCheckMenuItem(hwndMenuWnd, IDM_MSGQUICKCC, FALSE);
               break;

            case PROGSTATUS_EDITING:
               EnableMenuItems(hwndMenuWnd, FALSE, 4, IDM_SPCADDTONICK, IDM_SPCADDTOCC,
                                                      IDM_MSGREQUEST, IDM_MSGMARK);
               if (domains)
                  EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_SPCBROWSER);
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_SPCBROWSER);
               if (NewMessage)
               {
                  zeiger=AM_FindArea(&arealiste, NewMessage ? NewArea : CurrentArea);
                  if (!zeiger || zeiger->areadata.areatype == AREATYPE_NET || ulCCSelected || pQuickCCList)
                     EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_MSGXPOST);
                  else
                     EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_MSGXPOST);

                  if (pchXPostList)
                     WinCheckMenuItem(hwndMenuWnd, IDM_MSGXPOST, TRUE);
                  else
                     WinCheckMenuItem(hwndMenuWnd, IDM_MSGXPOST, FALSE);

                  if (!zeiger || zeiger->areadata.areatype == AREATYPE_ECHO ||pchXPostList ||
                      !ccanchor.ulNumLists || pQuickCCList)
                     EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_MSGCCOPY);
                  else
                     EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_MSGCCOPY);

                  if (ulCCSelected)
                     WinCheckMenuItem(hwndMenuWnd, IDM_MSGCCOPY, TRUE);
                  else
                     WinCheckMenuItem(hwndMenuWnd, IDM_MSGCCOPY, FALSE);

                  if (!ulCCSelected && !pchXPostList && zeiger &&
                      zeiger->areadata.areatype != AREATYPE_ECHO )
                  {
                     EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_MSGQUICKCC);
                     if (pQuickCCList)
                        WinCheckMenuItem(hwndMenuWnd, IDM_MSGQUICKCC, TRUE);
                     else
                        WinCheckMenuItem(hwndMenuWnd, IDM_MSGQUICKCC, FALSE);
                  }
                  else
                  {
                     EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_MSGQUICKCC);
                     WinCheckMenuItem(hwndMenuWnd, IDM_MSGQUICKCC, FALSE);
                  }
               }
               else
                  EnableMenuItems(hwndMenuWnd, FALSE, 3, IDM_MSGXPOST, IDM_MSGCCOPY, IDM_MSGQUICKCC);
               break;
         }
         break;

      case IDM_REXX:
         if (CurrentStatus == PROGSTATUS_NOSETUP)
            EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_RXSCRIPTS);
         else
            EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_RXSCRIPTS);

         if (tidRexxExec || CurrentStatus == PROGSTATUS_NOSETUP)
         {
            EnableMenuItems(hwndMenuWnd, FALSE, 10, IDM_RXQUICK1, IDM_RXQUICK2, IDM_RXQUICK3,
                                                    IDM_RXQUICK4, IDM_RXQUICK5, IDM_RXQUICK6,
                                                    IDM_RXQUICK7, IDM_RXQUICK8, IDM_RXQUICK9,
                                                    IDM_RXQUICK10);
         }
         else
         {
            EnableMenuItems(hwndMenuWnd, TRUE, 10, IDM_RXQUICK1, IDM_RXQUICK2, IDM_RXQUICK3,
                                                   IDM_RXQUICK4, IDM_RXQUICK5, IDM_RXQUICK6,
                                                   IDM_RXQUICK7, IDM_RXQUICK8, IDM_RXQUICK9,
                                                   IDM_RXQUICK10);
         }
         break;

      case IDM_WINDOWS:
         if (CurrentStatus == PROGSTATUS_READING ||
             CurrentStatus == PROGSTATUS_EDITING)
            EnableMenuItems(hwndMenuWnd, TRUE, 2, IDM_WINKLUDGES, IDM_WINRESULTS);
         else
            EnableMenuItems(hwndMenuWnd, FALSE, 2, IDM_WINKLUDGES, IDM_WINRESULTS);

         if (WinSendDlgItemMsg(frame, FID_TOOLBAR, TBM_ISCMDENABLED, MPFROMLONG(IDB_MSGTREE), NULL))
            EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_WINTHREADS);
         else
            EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_WINTHREADS);

         if (WinSendDlgItemMsg(frame, FID_TOOLBAR, TBM_ISCMDENABLED, MPFROMLONG(IDB_AREA), NULL))
            EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_WINAREAS);
         else
            EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_WINAREAS);

         if (WinSendDlgItemMsg(frame, FID_TOOLBAR, TBM_ISCMDENABLED, MPFROMLONG(IDB_MSGLIST), NULL))
            EnableMenuItems(hwndMenuWnd, TRUE, 1, IDM_WINMSGLIST);
         else
            EnableMenuItems(hwndMenuWnd, FALSE, 1, IDM_WINMSGLIST);
         break;

      case IDM_HELP:
         if (!hwndhelp)
         {
            EnableMenuItems(hwndMenuWnd, FALSE, 5, IDM_HELPKEYS, IDM_HELPGENERAL, IDM_HELPINDEX,
                                                   IDM_HELPCONTENTS, IDM_HELPUSING);
         }
         break;

      default:
         break;
   }
   return;
}

/*------------------------------ StatusChanged  -----------------------------*/
/* Aktiviert/Deaktiviert Fensterelemente je nach Programmstatus              */
/*---------------------------------------------------------------------------*/

void StatusChanged(HWND hwndClient, int newstatus)
{
   extern BOOL bTemplateProcessed;

   switch(newstatus)
   {
      case PROGSTATUS_READING:
         WinEnableControl(hwndClient, IDB_CHANGEATTR, FALSE);

         EnableToolbarItems(23, IDB_FIND, IDB_MSGTREE, IDB_NEWMSG, IDB_REPLY, IDB_EXPORT,
                                IDB_DELMSG, IDB_EDITMSG, IDB_AREA, IDB_MSGLIST, IDB_PRINTMSG,
                                IDB_HOMEMSG, IDB_NEXTAREA, IDB_SHOWKLUDGES, IDB_BOOKMARKS,
                                IDB_HELP, IDB_COPY, IDB_SHELL, IDB_SCRIPTS,
                                IDB_BROWSER, IDB_FORWARD, IDB_REQUEST, IDB_CATCHUP);
         DisableToolbarItems(5, IDB_IMPORT, IDB_OK, IDB_CANCEL, IDB_CUT, IDB_PASTE);

         SetReadOnly(hwndClient, IDE_FROMNAME, TRUE);
         SetReadOnly(hwndClient, IDE_TONAME, TRUE);
         SetReadOnly(hwndClient, IDE_FROMADDRESS, TRUE);
         SetReadOnly(hwndClient, IDE_TOADDRESS, TRUE);
         SetReadOnly(hwndClient, IDE_SUBJTEXT, TRUE);

         SendMsg(hwndClient, WORKM_SWITCHACCELS, MPFROMLONG(ACCEL_READ), NULL);
         bTemplateProcessed=TRUE;
         SetFocusControl(hwndClient, IDML_MAINEDIT);
         break;

      case PROGSTATUS_EDITING:
         WinEnableControl(hwndClient, IDB_CHANGEATTR, TRUE);

         DisableToolbarItems(23, IDB_PREVMSG, IDB_NEXTMSG, IDB_PREVREPLY, IDB_NEXTREPLY,
                                 IDB_FIRSTMSG, IDB_LASTMSG, IDB_FIND, IDB_MSGTREE, IDB_NEWMSG,
                                 IDB_REPLY, IDB_EXPORT, IDB_DELMSG, IDB_EDITMSG, IDB_AREA,
                                 IDB_MSGLIST, IDB_PRINTMSG, IDB_HOMEMSG, IDB_NEXTAREA,
                                 IDB_COPYMSG, IDB_MOVEMSG, IDB_FORWARD, IDB_CATCHUP, IDB_REQUEST);

         EnableToolbarItems(9, IDB_SHOWKLUDGES, IDB_IMPORT, IDB_OK, IDB_CANCEL,
                               IDB_CUT, IDB_COPY, IDB_PASTE, IDB_SHELL, IDB_SCRIPTS,
                               IDB_BROWSER);

         SetReadOnly(hwndClient, IDE_FROMNAME, FALSE);
         SetReadOnly(hwndClient, IDE_TONAME, FALSE);
         SetReadOnly(hwndClient, IDE_FROMADDRESS, FALSE);
         SetReadOnly(hwndClient, IDE_TOADDRESS, FALSE);
         SetReadOnly(hwndClient, IDE_SUBJTEXT, FALSE);

         SendMsg(hwndClient, WORKM_SWITCHACCELS, MPFROMLONG(ACCEL_WRITE), NULL);

         WinSetSysValue(HWND_DESKTOP, SV_INSERTMODE, TRUE);
         break;

      case PROGSTATUS_NOSETUP:
         WinEnableControl(hwndClient, IDB_CHANGEATTR, FALSE);

         DisableToolbarItems(33, IDB_PREVMSG, IDB_NEXTMSG, IDB_PREVREPLY, IDB_NEXTREPLY,
                                 IDB_FIRSTMSG, IDB_LASTMSG, IDB_FIND, IDB_MSGTREE, IDB_NEWMSG,
                                 IDB_REPLY, IDB_EXPORT, IDB_DELMSG, IDB_EDITMSG, IDB_AREA,
                                 IDB_MSGLIST, IDB_PRINTMSG, IDB_IMPORT, IDB_OK, IDB_CANCEL,
                                 IDB_SHOWKLUDGES, IDB_HOMEMSG, IDB_NEXTAREA, IDB_BOOKMARKS,
                                 IDB_COPYMSG, IDB_MOVEMSG, IDB_CUT, IDB_PASTE, IDB_COPY,
                                 IDB_BROWSER, IDB_SCRIPTS, IDB_FORWARD, IDB_CATCHUP, IDB_REQUEST);

         SetReadOnly(hwndClient, IDE_FROMNAME, TRUE);
         SetReadOnly(hwndClient, IDE_TONAME, TRUE);
         SetReadOnly(hwndClient, IDE_FROMADDRESS, TRUE);
         SetReadOnly(hwndClient, IDE_TOADDRESS, TRUE);
         SetReadOnly(hwndClient, IDE_SUBJTEXT, TRUE);

         bTemplateProcessed=TRUE;
         SendMsg(hwndClient, WORKM_SWITCHACCELS, MPFROMLONG(ACCEL_NONE), NULL);
         break;

      case PROGSTATUS_CLEANUP:
         WinEnableControl(hwndClient, IDB_CHANGEATTR, FALSE);

         DisableToolbarItems(35, IDB_PREVMSG, IDB_NEXTMSG, IDB_PREVREPLY, IDB_NEXTREPLY,
                                 IDB_FIRSTMSG, IDB_LASTMSG, IDB_FIND, IDB_MSGTREE,
                                 IDB_NEWMSG, IDB_REPLY, IDB_EXPORT, IDB_DELMSG,
                                 IDB_EDITMSG, IDB_AREA, IDB_MSGLIST, IDB_PRINTMSG,
                                 IDB_IMPORT, IDB_OK, IDB_CANCEL, IDB_SHOWKLUDGES,
                                 IDB_HOMEMSG, IDB_NEXTAREA, IDB_BOOKMARKS, IDB_HELP,
                                 IDB_COPYMSG, IDB_MOVEMSG, IDB_COPY, IDB_CUT, IDB_PASTE, IDB_SHELL,
                                 IDB_SCRIPTS, IDB_BROWSER, IDB_FORWARD, IDB_CATCHUP, IDB_REQUEST);

         WinSetDlgItemText(hwndClient, IDE_FROMNAME, "");
         WinSetDlgItemText(hwndClient, IDE_TONAME, "");
         WinSetDlgItemText(hwndClient, IDE_FROMADDRESS, "");
         WinSetDlgItemText(hwndClient, IDE_TOADDRESS, "");
         WinSetDlgItemText(hwndClient, IDE_SUBJTEXT, "");
         WinSetDlgItemText(hwndClient, IDS_ATTRTEXT, "");
         WinSetDlgItemText(hwndClient, IDS_DATEWRITTEN, "");
         WinSetDlgItemText(hwndClient, IDS_DATERECEIVED, "");

         SetReadOnly(hwndClient, IDE_FROMNAME, TRUE);
         SetReadOnly(hwndClient, IDE_TONAME, TRUE);
         SetReadOnly(hwndClient, IDE_FROMADDRESS, TRUE);
         SetReadOnly(hwndClient, IDE_TOADDRESS, TRUE);
         SetReadOnly(hwndClient, IDE_SUBJTEXT, TRUE);

         bTemplateProcessed=TRUE;
         SendMsg(hwndClient, WORKM_SWITCHACCELS, MPFROMLONG(ACCEL_READ), NULL);
         break;

      default:
         break;
   }
   return;
}

static void EnableToolbarItems(int num, ...)
{
   extern HWND frame;
   va_list arg_ptr;

   va_start(arg_ptr, num);

   while(num>0)
   {
      WinSendDlgItemMsg(frame, FID_TOOLBAR, TBM_ENABLECMD, MPFROMLONG(va_arg(arg_ptr, ULONG)), NULL);
      num--;
   }
   va_end(arg_ptr);
   return;
}

static void DisableToolbarItems(int num, ...)
{
   extern HWND frame;
   va_list arg_ptr;

   va_start(arg_ptr, num);

   while(num>0)
   {
      WinSendDlgItemMsg(frame, FID_TOOLBAR, TBM_DISABLECMD, MPFROMLONG(va_arg(arg_ptr, ULONG)), NULL);
      num--;
   }
   va_end(arg_ptr);
   return;
}

static void EnableMenuItems(HWND hwndMenu, BOOL bNewState, int num, ...)
{
   va_list arg_ptr;

   va_start(arg_ptr, num);

   while(num>0)
   {
      WinEnableMenuItem(hwndMenu, va_arg(arg_ptr, ULONG), bNewState);
      num--;
   }
   va_end(arg_ptr);
   return;
}

/*------------------------------ InitMainWindow -----------------------------*/
/* Initialisierungsfunktion des Main Window                                  */
/*---------------------------------------------------------------------------*/

void InitMainWindow(HWND fenster)
{
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS windowfonts;
   extern WINDOWPOSITIONS windowpositions;

   HWND hwnd;
   char pchTemp[100];
   PWINDOWDATA pWindowData;

   /* Instanzdaten */
   pWindowData=malloc(sizeof(WINDOWDATA));
   PMASSERT(pWindowData != NULL, "Out of memory");
   memset(pWindowData, 0, sizeof(WINDOWDATA));
   WinSetWindowULong(fenster, QWL_USER, (ULONG) pWindowData);

   SetBackground(fenster, &windowcolors.windowback);

   /* Main Edit Window */
   hwnd=WinCreateWindow(fenster,
                        WC_MSGVIEWER,
                        NULL,
                        WS_VISIBLE |
                        MSGVS_VSCROLL |
                        MSGVS_BORDER,
                        5,30,
                        70,100,
                        fenster,
                        HWND_TOP,
                        IDML_MAINEDIT,
                        NULL,
                        NULL);

   SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_TEXT), MPFROMLONG(windowcolors.viewerfore));
   SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_BACKGROUND), MPFROMLONG(windowcolors.viewerback));
   SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_QUOTE), MPFROMLONG(windowcolors.viewerquote));
   SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_QUOTE2), MPFROMLONG(windowcolors.viewerquote2));
   SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_TEARLINE), MPFROMLONG(windowcolors.viewertearline));
   SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_ORIGIN), MPFROMLONG(windowcolors.viewerorigin));
   SetFont(hwnd, windowfonts.viewerfont);

   /* Message-Header-Elemente */

   LoadString(IDST_MW_FROM, 100, pchTemp);
   hwnd=WinCreateWindow(fenster,
                   WC_STATIC,
                   pchTemp,
                   WS_VISIBLE |
                   SS_TEXT |
                   DT_RIGHT |
                   DT_VCENTER,
                   5,160,
                   60,18,
                   fenster,
                   HWND_TOP,
                   IDS_FROM,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.fromtostaticfore);
   SetBackground(hwnd, &windowcolors.fromtostaticback);
   SetFont(hwnd, windowfonts.fromtofont);

   LoadString(IDST_MW_TO, 100, pchTemp);
   hwnd=WinCreateWindow(fenster,
                   WC_STATIC,
                   pchTemp,
                   WS_VISIBLE |
                   SS_TEXT |
                   DT_RIGHT |
                   DT_VCENTER,
                   5,140,
                   60,18,
                   fenster,
                   HWND_TOP,
                   IDS_TO,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.fromtostaticfore);
   SetBackground(hwnd, &windowcolors.fromtostaticback);
   SetFont(hwnd, windowfonts.fromtofont);

   LoadString(IDST_MW_SUBJ, 100, pchTemp);
   hwnd=WinCreateWindow(fenster,
                   WC_STATIC,
                   pchTemp,
                   WS_VISIBLE |
                   SS_TEXT |
                   DT_RIGHT |
                   DT_VCENTER,
                   5,120,
                   60,18,
                   fenster,
                   HWND_TOP,
                   IDS_SUBJ,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.fromtostaticfore);
   SetBackground(hwnd, &windowcolors.fromtostaticback);
   SetFont(hwnd, windowfonts.fromtofont);

   hwnd=WinCreateWindow(fenster,
                   WC_STATIC,
                   NULL,
                   WS_VISIBLE |
                   SS_TEXT |
                   DT_CENTER |
                   DT_VCENTER,
                   100,80,
                   130,16,
                   fenster,
                   HWND_TOP,
                   IDS_DATEWRITTEN,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.msgtimefore);
   SetBackground(hwnd, &windowcolors.msgtimeback);
   SetFont(hwnd, windowfonts.datefont);

   hwnd=WinCreateWindow(fenster,
                   WC_STATIC,
                   NULL,
                   WS_VISIBLE | WS_SYNCPAINT |
                   SS_TEXT |
                   DT_CENTER |
                   DT_VCENTER,
                   100,100,
                   130,16,
                   fenster,
                   HWND_TOP,
                   IDS_DATERECEIVED,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.msgtimefore);
   SetBackground(hwnd, &windowcolors.msgtimeback);
   SetFont(hwnd, windowfonts.datefont);


   hwnd=WinCreateWindow(fenster,
                   "FromToEntry",
                   NULL,
                   WS_VISIBLE |
                   ES_AUTOSCROLL |
                   ES_LEFT,
                   100,100,
                   100,18,
                   fenster,
                   HWND_TOP,
                   IDE_FROMNAME,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.fromfore);
   SetBackground(hwnd, &windowcolors.fromback);
   SetFont(hwnd, windowfonts.fromfont);

   SendMsg(hwnd, EM_SETTEXTLIMIT, MPFROMSHORT(LEN_USERNAME), (MPARAM) NULL);
   SendMsg(hwnd, EM_SETREADONLY, MPFROMLONG(TRUE), NULL);

   hwnd=WinCreateWindow(fenster,
                   "FromToEntry",
                   NULL,
                   WS_VISIBLE |
                   ES_AUTOSCROLL |
                   ES_LEFT,
                   80,100,
                   100,18,
                   fenster,
                   HWND_TOP,
                   IDE_TONAME,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.tofore);
   SetBackground(hwnd, &windowcolors.toback);
   SetFont(hwnd, windowfonts.tofont);

   SendMsg(hwnd, EM_SETTEXTLIMIT, MPFROMSHORT(LEN_USERNAME), (MPARAM) NULL);
   SendMsg(hwnd, EM_SETREADONLY, MPFROMLONG(TRUE), NULL);

   hwnd=WinCreateWindow(fenster,
                   "SubjectEntry",
                   NULL,
                   WS_VISIBLE |
                   ES_AUTOSCROLL |
                   ES_LEFT,
                   100,100,
                   100,18,
                   fenster,
                   HWND_TOP,
                   IDE_SUBJTEXT,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.subjfore);
   SetBackground(hwnd, &windowcolors.subjback);
   SetFont(hwnd, windowfonts.subjfont);

   SendMsg(hwnd, EM_SETTEXTLIMIT, MPFROMSHORT(LEN_SUBJECT), (MPARAM) NULL);
   SendMsg(hwnd, EM_SETREADONLY, MPFROMLONG(TRUE), NULL);

   hwnd=WinCreateWindow(fenster,
                   "FidoEntry",
                   NULL,
                   WS_VISIBLE | WS_SYNCPAINT |
                   ES_AUTOSCROLL |
                   ES_LEFT,
                   100,100,
                   95,18,
                   fenster,
                   HWND_TOP,
                   IDE_FROMADDRESS,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.fromadfore);
   SetBackground(hwnd, &windowcolors.fromadback);
   SetFont(hwnd, windowfonts.fromadfont);

   SendMsg(hwnd, EM_SETTEXTLIMIT, MPFROMSHORT(LEN_5DADDRESS), (MPARAM) NULL);
   SendMsg(hwnd, EM_SETREADONLY, MPFROMLONG(TRUE), NULL);

   hwnd=WinCreateWindow(fenster,
                   "FidoEntry",
                   NULL,
                   WS_SYNCPAINT |
                   ES_AUTOSCROLL |
                   ES_LEFT,
                   100,100,
                   95,18,
                   fenster,
                   HWND_TOP,
                   IDE_TOADDRESS,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.toadfore);
   SetBackground(hwnd, &windowcolors.toadback);
   SetFont(hwnd, windowfonts.toadfont);

   SendMsg(hwnd, EM_SETTEXTLIMIT, MPFROMSHORT(LEN_5DADDRESS), (MPARAM) NULL);
   SendMsg(hwnd, EM_SETREADONLY, MPFROMLONG(TRUE), NULL);

   hwnd=WinCreateWindow(fenster,
                   WC_STATIC,
                   NULL,
                   WS_VISIBLE |
                   SS_TEXT |
                   DT_VCENTER,
                   100,100,
                   100,18,
                   fenster,
                   HWND_TOP,
                   IDS_ATTRTEXT,
                   NULL,
                   NULL);

   SetForeground(hwnd, &windowcolors.attribfore);
   SetBackground(hwnd, &windowcolors.attribback);
   SetFont(hwnd, windowfonts.attribfont);

   LoadString(IDST_MW_ATTRIB, 100, pchTemp);
   hwnd=WinCreateWindow(fenster,
                   WC_BUTTON,
                   pchTemp,
                   WS_VISIBLE |
                   BS_NOPOINTERFOCUS |
                   BS_PUSHBUTTON,
                   100,100,
                   65,22,
                   fenster,
                   HWND_TOP,
                   IDB_CHANGEATTR,
                   NULL,
                   NULL);

   SetFont(hwnd, windowfonts.buttonfont);

   /* Popup-Menues laden */
   pWindowData->hwndPopup = WinLoadMenu(HWND_OBJECT, hmodLang, IDM_EDITPOPUP);
   pWindowData->hwndToolbarPopup = WinLoadMenu(HWND_OBJECT, hmodLang, IDM_TB_POPUP);

   return;
}

/*----------------------------- DisplayStatusText ---------------------------*/
/* Zeigt fuer eine Control-ID einen Hilfetext an                             */
/*---------------------------------------------------------------------------*/

void DisplayStatusText(SHORT ctrlID)
{
   static ULONG currentID=0;
   UCHAR text[100];
   ULONG stringID;
   extern HWND frame;
   int i=0;

   while (ButtonHelpStrings[i].usCtrlID && ButtonHelpStrings[i].usCtrlID != ctrlID)
      i++;

   if (ButtonHelpStrings[i].usCtrlID)
      stringID = ButtonHelpStrings[i].ulStringID;
   else
      stringID = 0;

   if (currentID!=stringID)
   {
      currentID=stringID;

      if (!stringID)
         text[0]='\0';
      else
         LoadString( stringID, sizeof(text), text);

      WinSetWindowText(WinWindowFromID(frame, FID_STATUSLINE),
                       text);
   }
   return;
}

/*----------------------------- DisplayMenuHelp   ---------------------------*/
/* Zeigt fuer einen Menuepunkt einen Hilfetext an                            */
/*---------------------------------------------------------------------------*/

void DisplayMenuHelp(HWND hwndClient, SHORT ctrlID)
{
   UCHAR text[100];
   int i=0;

   while (MenuHelpStrings[i].usCtrlID && MenuHelpStrings[i].usCtrlID != ctrlID)
      i++;

   if (MenuHelpStrings[i].usCtrlID)
      LoadString(MenuHelpStrings[i].ulStringID, sizeof(text), text);
   else
      text[0]='\0';

   WinSetDlgItemText(WinQueryWindow(hwndClient, QW_PARENT), FID_STATUSLINE, text);

   return;
}

/*----------------------------- InsertMacro ---------------------------------*/
/* Fuegt einen Macro-Text in das MLE ein                                     */
/*---------------------------------------------------------------------------*/

void InsertMacro(HWND hwndEdit, USHORT MacroID)
{
   extern MACROTABLEOPT macrotable;

   SendMsg(hwndEdit, MLM_INSERT, macrotable.macrotext[MacroID], NULL);

   return;
}

/*----------------------------- ShowFleetWindow -----------------------------*/
/* Zeigt eines der FleetStreet-Fenster an                                    */
/*---------------------------------------------------------------------------*/

void ShowFleetWindow(USHORT usMenuID)
{
   extern HWND hwndThreadList, hwndKludge, hwndFindResults,
               hwndMsgList, hwndAreaDlg, client;
   HWND hwndTarget=NULLHANDLE;
   HWND hwndFocus=NULLHANDLE;

   switch(usMenuID)
   {
      case IDM_WINTHREADS:
         if (!hwndThreadList)
         {
            SendMsg(client, WM_COMMAND, MPFROMSHORT(IDB_MSGTREE),
                       MPFROMSHORT(CMDSRC_PUSHBUTTON));
         }
         if (hwndThreadList)
         {
            hwndTarget=hwndThreadList;
            hwndFocus=WinWindowFromID(hwndThreadList, IDD_THREADLIST+1);
         }
         break;

      case IDM_WINKLUDGES:
         if (!hwndKludge)
         {
            SendMsg(client, WM_COMMAND, MPFROMSHORT(IDB_SHOWKLUDGES),
                       MPFROMSHORT(CMDSRC_PUSHBUTTON));
         }
         if (hwndKludge)
         {
            hwndTarget=hwndKludge;
            hwndFocus=hwndKludge;
         }
         break;

      case IDM_WINRESULTS:
         if (!hwndFindResults)
         {
            BOOKMARKSOPEN BMOpen={sizeof(BOOKMARKSOPEN), MARKFLAG_MANUAL};

            hwndFindResults=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                                       FindResultsProc,
                                       hmodLang, IDD_FINDRESULTS,
                                       &BMOpen);
            WinShowWindow(hwndFindResults, TRUE);
         }
         if (hwndFindResults)
         {
            hwndTarget=hwndFindResults;
            hwndFocus=WinWindowFromID(hwndFindResults, IDD_FINDRESULTS+1);
         }
         break;

      case IDM_WINAREAS:
         if (!hwndAreaDlg)
         {
            SendMsg(client, WM_COMMAND, MPFROMSHORT(IDB_AREA),
                       MPFROMSHORT(CMDSRC_PUSHBUTTON));
         }
         if (hwndAreaDlg)
         {
            hwndTarget=hwndAreaDlg;
            hwndFocus=WinWindowFromID(hwndAreaDlg, IDD_AREALIST+1);
         }
         break;

      case IDM_WINMSGLIST:
         if (!hwndMsgList)
         {
            SendMsg(client, WM_COMMAND, MPFROMSHORT(IDB_MSGLIST),
                       MPFROMSHORT(CMDSRC_PUSHBUTTON));
         }
         if (hwndMsgList)
         {
            hwndTarget=hwndMsgList;
            hwndFocus=WinWindowFromID(hwndMsgList, IDD_MSGLIST+1);
         }
         break;

      default:
         break;
   }
   if (hwndTarget)
   {
      SWP swp;

      SetFocus(hwndFocus);

      WinQueryWindowPos(hwndTarget, &swp);
      if (swp.fl & SWP_MINIMIZE)
         WinSetWindowPos(hwndTarget,
                         NULLHANDLE,
                         0, 0,
                         0, 0,
                         SWP_RESTORE);
   }
   return;
}

/*----------------------------- SwitchEditor    -----------------------------*/
/* Schaltet zwischen Viewer und Editor hin und her                           */
/*---------------------------------------------------------------------------*/

void SwitchEditor(HWND hwndClient, char *pchDestArea, BOOL bEdit)
{
   SWP swp;
   HWND hwnd;
   PWINDOWDATA pWindowData;
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS windowfonts;
   extern AREALIST arealiste;
   extern GENERALOPT generaloptions;
   extern TEMPLATELIST templatelist;
   extern HWND frame;
   AREADEFLIST *zeiger=AM_FindArea(&arealiste, pchDestArea);
   char *pchFont;

   hwnd=WinWindowFromID(hwndClient, IDML_MAINEDIT);

   /* Position und Groesse sichern */
   WinQueryWindowPos(hwnd, &swp);

   if (bEdit)
   {
      /* Editor erzeugen */
      BOOL bInsert;

      /* alte Farben und Font sichern */
      SendMsg(hwnd, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_TEXT), &windowcolors.viewerfore);
      SendMsg(hwnd, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_BACKGROUND), &windowcolors.viewerback);
      SendMsg(hwnd, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_QUOTE), &windowcolors.viewerquote);
      SendMsg(hwnd, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_QUOTE2), &windowcolors.viewerquote2);
      SendMsg(hwnd, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_TEARLINE), &windowcolors.viewertearline);
      SendMsg(hwnd, MSGVM_QUERYCOLOR, MPFROMLONG(MSGVCLR_ORIGIN), &windowcolors.viewerorigin);
      if (MonoDisp || TempMono)
         QueryFont(hwnd, pchFont=windowfonts.viewermonofont);
      else
         QueryFont(hwnd, pchFont=windowfonts.viewerfont);

      /* altes Fenster killen */
      WinDestroyWindow(hwnd);

      hwnd=WinCreateWindow(hwndClient,
                           "EditWin",
                           NULL,
                           WS_VISIBLE | MLS_VSCROLL | MLS_WORDWRAP | MLS_BORDER,
                           swp.x, swp.y,
                           swp.cx, swp.cy,
                           hwndClient,
                           HWND_TOP,
                           IDML_MAINEDIT,
                           NULL, NULL);

      SetBackground(hwnd, &windowcolors.editback);
      SetForeground(hwnd, &windowcolors.editfore);
      SetFont(hwnd, pchFont);
      if (zeiger)
      {
         SendMsg(hwnd, MLM_SETTRANSLATE,
                    (MPARAM) !(zeiger->areadata.ulAreaOpt & AREAOPT_HIGHASCII),
                    NULL);
      }
      SendMsg(hwnd, MLM_SETTABSIZE,
                 MPFROMLONG(generaloptions.lTabSize), NULL);

      pWindowData=(PWINDOWDATA)WinQueryWindowULong(hwndClient, QWL_USER);
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
   else   /* Viewer */
   {
      PMSGTEMPLATE pTemplate;

      /* alte Farben und Font sichern */
      QueryForeground(hwnd, &windowcolors.editfore);
      QueryBackground(hwnd, &windowcolors.editback);
      if (MonoDisp || TempMono)
         QueryFont(hwnd, pchFont=windowfonts.viewermonofont);
      else
         QueryFont(hwnd, pchFont=windowfonts.viewerfont);

      /* altes Fenster killen */
      WinDestroyWindow(hwnd);

      /* Viewer erzeugen */
      hwnd=WinCreateWindow(hwndClient,
                           WC_MSGVIEWER,
                           NULL,
                           WS_VISIBLE | MSGVS_VSCROLL | MSGVS_BORDER,
                           swp.x, swp.y,
                           swp.cx, swp.cy,
                           hwndClient,
                           HWND_TOP,
                           IDML_MAINEDIT,
                           NULL, NULL);

      SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_TEXT), MPFROMLONG(windowcolors.viewerfore));
      SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_BACKGROUND), MPFROMLONG(windowcolors.viewerback));
      SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_QUOTE), MPFROMLONG(windowcolors.viewerquote));
      SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_QUOTE2), MPFROMLONG(windowcolors.viewerquote2));
      SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_TEARLINE), MPFROMLONG(windowcolors.viewertearline));
      SendMsg(hwnd, MSGVM_SETCOLOR, MPFROMLONG(MSGVCLR_ORIGIN), MPFROMLONG(windowcolors.viewerorigin));
      SendMsg(hwnd, MSGVM_ENABLEHIGHLIGHT,
              MPFROMLONG(!generaloptions.nohighlight && !(zeiger->areadata.ulAreaOpt & AREAOPT_NOHIGHLIGHT)),
              NULL);
      pTemplate = M_FindTemplate(&templatelist, &arealiste, pchDestArea);
      if (pTemplate)
         SendMsg(hwnd, MSGVM_SETQUOTECHAR,
                 MPFROMCHAR(pTemplate->chQuoteChar), NULL);
      SetFont(hwnd, pchFont);

      pWindowData=(PWINDOWDATA)WinQueryWindowULong(hwndClient, QWL_USER);
      WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idCursorField), NULL);
      WinSendDlgItemMsg(frame, FID_STATUSLINE, STLM_SETFIELDTEXT,
                        MPFROMLONG(pWindowData->idKeybField), NULL);
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenEditPopup                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet das Popup-Menue fuer Clipboard-Operationen          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndClient: Client-Window-Handle                               */
/*            pWindowData: Zeiger auf die Client-Window-Daten                */
/*            bKeyboard: TRUE: Tastatur-Operation                            */
/*                       FALSE: Maus-Operation                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: usPopupControl in den Window-Daten muss gesetzt sein.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void OpenEditPopup(HWND hwndClient, PWINDOWDATA pWindowData, BOOL bKeyBoard)
{
   POINTL pointl;
   extern int CurrentStatus;
   MRESULT mr;

   switch(pWindowData->usPopupControl)
   {
      case IDML_MAINEDIT:
         if (CurrentStatus == PROGSTATUS_EDITING)
         {
            EnableMenuItems(pWindowData->hwndPopup, TRUE, 7,
                            IDM_EP_COPY, IDM_EP_UNDO, IDM_EP_CUT, IDM_EP_CLEAR,
                            IDM_EP_DELLINE, IDM_EP_PASTE, IDM_EP_MONO);
            EnableMenuItems(pWindowData->hwndPopup, FALSE, 1, IDM_EP_HIGHLIGHT);
            WinCheckMenuItem(pWindowData->hwndPopup, IDM_EP_HIGHLIGHT, FALSE);
         }
         else
         {
            BOOL bTemp;

            EnableMenuItems(pWindowData->hwndPopup, TRUE, 3,
                            IDM_EP_COPY, IDM_EP_HIGHLIGHT, IDM_EP_MONO);
            EnableMenuItems(pWindowData->hwndPopup, FALSE, 5,
                            IDM_EP_UNDO, IDM_EP_CUT, IDM_EP_CLEAR, IDM_EP_DELLINE,
                            IDM_EP_PASTE);
            bTemp = (BOOL) WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MSGVM_QUERYHIGHLIGHT, NULL, NULL);
            WinCheckMenuItem(pWindowData->hwndPopup, IDM_EP_HIGHLIGHT, bTemp);
         }
         WinCheckMenuItem(pWindowData->hwndPopup, IDM_EP_MONO, MonoDisp || TempMono);
         break;

      case IDE_FROMNAME:
      case IDE_TONAME:
      case IDE_FROMADDRESS:
      case IDE_TOADDRESS:
      case IDE_SUBJTEXT:
         if (CurrentStatus == PROGSTATUS_EDITING)
            EnableMenuItems(pWindowData->hwndPopup, TRUE, 1, IDM_EP_PASTE);
         else
            EnableMenuItems(pWindowData->hwndPopup, FALSE, 3,
                            IDM_EP_CUT, IDM_EP_CLEAR, IDM_EP_PASTE);

         EnableMenuItems(pWindowData->hwndPopup, FALSE, 4,
                         IDM_EP_UNDO, IDM_EP_DELLINE, IDM_EP_HIGHLIGHT, IDM_EP_MONO);
         WinCheckMenuItem(pWindowData->hwndPopup, IDM_EP_HIGHLIGHT, FALSE);
         WinCheckMenuItem(pWindowData->hwndPopup, IDM_EP_MONO, FALSE);
         mr = WinSendDlgItemMsg(hwndClient, pWindowData->usPopupControl, EM_QUERYSEL,
                                NULL, NULL);

         if ((SHORT2FROMMR(mr) - SHORT1FROMMR(mr)) == 0)
            EnableMenuItems(pWindowData->hwndPopup, FALSE, 3,
                            IDM_EP_COPY, IDM_EP_CUT, IDM_EP_CLEAR);
         else
            if (CurrentStatus == PROGSTATUS_EDITING)
               EnableMenuItems(pWindowData->hwndPopup, TRUE, 3,
                               IDM_EP_COPY, IDM_EP_CUT, IDM_EP_CLEAR);
            else
               EnableMenuItems(pWindowData->hwndPopup, TRUE, 1, IDM_EP_COPY);
         break;

      default:
         return;
   }

   if (bKeyBoard)
   {
      RECTL rectl;

      WinQueryWindowRect(WinWindowFromID(hwndClient, pWindowData->usPopupControl), &rectl);
      WinMapWindowPoints(WinWindowFromID(hwndClient, pWindowData->usPopupControl),
                         HWND_DESKTOP, (PPOINTL) &rectl, 2);

      WinPopupMenu(HWND_DESKTOP, hwndClient, pWindowData->hwndPopup,
                   rectl.xLeft+(rectl.xRight-rectl.xLeft)/2,
                   rectl.yBottom+(rectl.yTop-rectl.yBottom)/2,
                   IDM_EP_COPY,
                   PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1 | PU_POSITIONONITEM);
   }
   else
   {
      WinQueryPointerPos(HWND_DESKTOP, &pointl);

      WinPopupMenu(HWND_DESKTOP, hwndClient, pWindowData->hwndPopup,
                   pointl.x, pointl.y,
                   IDM_EP_COPY,
                   PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD |
                   PU_POSITIONONITEM | PU_MOUSEBUTTON1);
   }
   return;
}

void QueryOpenWindows(POPENWIN pOpenWindows)
{
   ULONG ulTemp=0;
   extern HWND hwndAreaDlg, hwndKludge, hwndTemplates, hwndCCLists, hwndRxFolder,
               hwndNLBrowser, hwndFindResults, hwndThreadList, hwndMsgList;

   if (hwndAreaDlg)
      ulTemp |= OPENWIN_AREA;

   if (hwndKludge)
      ulTemp |= OPENWIN_KLUDGES;

   if (hwndTemplates)
      ulTemp |= OPENWIN_TPL;

   if (hwndCCLists)
      ulTemp |= OPENWIN_CCLISTS;

   if (hwndRxFolder)
      ulTemp |= OPENWIN_REXX;

   if (hwndNLBrowser)
      ulTemp |= OPENWIN_BROWSER;

   if (hwndFindResults)
      ulTemp |= OPENWIN_BOOKMARKS;

   if (hwndThreadList)
      ulTemp |= OPENWIN_THRL;

   if (hwndMsgList)
      ulTemp |= OPENWIN_MSGL;

   pOpenWindows->ulOpenWindows = ulTemp;

   return;
}

void RestoreOpenWindows(POPENWIN pOpenWindows)
{
   extern HWND client;
   ULONG ulOpenMask;

   ulOpenMask = (pOpenWindows->ulOpenWindows & ~pOpenWindows->ulForceClose) | pOpenWindows->ulForceOpen;

   if (ulOpenMask & OPENWIN_AREA)
   {
      SendMsg(client, WM_COMMAND, MPFROMSHORT(IDA_AREA),
                 MPFROM2SHORT(CMDSRC_ACCELERATOR, FALSE));
   }

   if (ulOpenMask & OPENWIN_KLUDGES)
   {
      SendMsg(client, WM_COMMAND, MPFROMSHORT(IDA_KLUDGES),
                 MPFROM2SHORT(CMDSRC_ACCELERATOR, FALSE));
   }

#if 0
   if (ulOpenMask & OPENWIN_TPL)
   {
   }

   if (ulOpenMask & OPENWIN_CCLISTS)
   {
   }

   if (ulOpenMask & OPENWIN_REXX)
   {
   }

   if (ulOpenMask & OPENWIN_BROWSER)
   {
   }
#endif

   if (ulOpenMask & OPENWIN_BOOKMARKS)
   {
      SendMsg(client, WM_COMMAND, MPFROMSHORT(IDA_BOOKMARKS),
                 MPFROM2SHORT(CMDSRC_ACCELERATOR, FALSE));
   }

   if (ulOpenMask & OPENWIN_THRL)
   {
      SendMsg(client, WM_COMMAND, MPFROMSHORT(IDA_THREADLIST),
                 MPFROM2SHORT(CMDSRC_ACCELERATOR, FALSE));
   }

   if (ulOpenMask & OPENWIN_MSGL)
   {
      SendMsg(client, WM_COMMAND, MPFROMSHORT(IDA_MSGLIST),
                 MPFROM2SHORT(CMDSRC_ACCELERATOR, FALSE));
   }
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTLAY_ResizeMainWindow                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Passt die Groesse und Position der geaenderten Fenster-     */
/*               groesse an                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndClient:  Window-Handle des Client-Windows                  */
/*            cx:          Neue X-Groesse des Client-Windows                 */
/*            cy:          Neue Y-Groesse des Client-Windows                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: keine                                                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void FLTLAY_ResizeMainWindow(HWND hwndClient, SHORT cx, SHORT cy)
{
   int i=0;

   while (ResizeParam[i].CtrlID)
   {
      switch(ResizeParam[i].CtrlID)
      {
         case IDE_TOADDRESS:
            WinSetWindowPos(WinWindowFromID(hwndClient, ResizeParam[i].CtrlID),
                            NULLHANDLE,
                            (ResizeParam[i].flags & XREL)?  (cx+ResizeParam[i].x)  : ResizeParam[i].x,
                            (ResizeParam[i].flags & YREL)?  (cy+ResizeParam[i].y)  : ResizeParam[i].y,
                            (ResizeParam[i].flags & CXREL)? (cx+ResizeParam[i].cx) : ResizeParam[i].cx,
                            (ResizeParam[i].flags & CYREL)? (cy+ResizeParam[i].cy) : ResizeParam[i].cy,
                            SWP_SIZE | SWP_MOVE);
            break;

         default:
            WinSetWindowPos(WinWindowFromID(hwndClient, ResizeParam[i].CtrlID),
                            NULLHANDLE,
                            (ResizeParam[i].flags & XREL)?  (cx+ResizeParam[i].x)  : ResizeParam[i].x,
                            (ResizeParam[i].flags & YREL)?  (cy+ResizeParam[i].y)  : ResizeParam[i].y,
                            (ResizeParam[i].flags & CXREL)? (cx+ResizeParam[i].cx) : ResizeParam[i].cx,
                            (ResizeParam[i].flags & CYREL)? (cy+ResizeParam[i].cy) : ResizeParam[i].cy,
                            SWP_SIZE | SWP_MOVE |
                            ((ResizeParam[i].flags & VIS) ? SWP_SHOW : SWP_HIDE));
            break;
      }
      i++;
   }

   /* Spezial-Behandlungen */
   WinSetWindowPos(WinWindowFromID(hwndClient, IDE_FROMNAME),
                   NULLHANDLE,
                   ResizeParam[0].x, cy+ResizeParam[0].y,
                   (cx-85-140)*2/3, ResizeParam[0].cy,
                   SWP_MOVE | SWP_SIZE);

   WinSetWindowPos(WinWindowFromID(hwndClient, IDE_TONAME),
                   NULLHANDLE,
                   ResizeParam[1].x, cy+ResizeParam[1].y,
                   (cx-85-140)*2/3, ResizeParam[1].cy,
                   SWP_MOVE | SWP_SIZE);

   WinSetWindowPos(WinWindowFromID(hwndClient, IDE_FROMADDRESS),
                   NULLHANDLE,
                   cx-(cx-80-140)/3-135, cy+ResizeParam[2].y,
                   (cx-80-140)/3, ResizeParam[2].cy,
                   SWP_SIZE | SWP_MOVE);

   WinSetWindowPos(WinWindowFromID(hwndClient, IDE_TOADDRESS),
                   NULLHANDLE,
                   cx-(cx-80-140)/3-135, cy+ResizeParam[3].y,
                   (cx-80-140)/3, ResizeParam[3].cy,
                   SWP_SIZE | SWP_MOVE);

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTLAY_QueryNextFocus                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liefert fÅr eine Control-ID die ID des naechsten Controls   */
/*               in der Kette zurueck                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ulLayoutNum: Nummer des Layouts                                */
/*            usCurrent:   ID des momentanen Controls                        */
/*            bSkipAddresses: TRUE  Adressfelder ueberspringen               */
/*                            FALSE Adressfelder einbeziehen                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: ID des naechsten Controls                                  */
/*                0 bei einem Fehler                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FLTLAY_QueryNextFocus(USHORT usCurrent, BOOL bSkipAddresses)
{
   int i=0;

   while(FocusTable[i] &&
         FocusTable[i] != usCurrent)
     i++;

   if (FocusTable[i] == 0)
      return 0;
   else
   {
      i++;
      if (FocusTable[i] == 0)
         i=0;
      if (FocusTable[i] == IDE_TOADDRESS && bSkipAddresses)
         i++;
      if (FocusTable[i] == 0)
         i=0;
      return FocusTable[i];
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTLAY_QueryPrevFocus                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liefert fÅr eine Control-ID die ID des vorherigen Controls  */
/*               in der Kette zurueck                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: ulLayoutNum: Nummer des Layouts                                */
/*            usCurrent:   ID des momentanen Controls                        */
/*            bSkipAddresses: TRUE  Adressfelder ueberspringen               */
/*                            FALSE Adressfelder einbeziehen                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: ID des vorherigen Controls                                 */
/*                0 bei einem Fehler                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT FLTLAY_QueryPrevFocus(USHORT usCurrent, BOOL bSkipAddresses)
{
   int i=0;

   while(FocusTable[i] &&
         FocusTable[i] != usCurrent)
     i++;

   if (FocusTable[i] == 0)
      return 0;
   else
   {
      i--;
      if (i<0)
         i=0;
      if (FocusTable[i] == IDE_TOADDRESS && bSkipAddresses)
         i--;
      if (i<0)
         i=0;
      return FocusTable[i];
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ToolbarFrameProc                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Neue Frame-Prozedur zum einpassen der Toolbar unter dem     */
/*               normalen Menue                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Der Zeiger auf die Original-Prozedur mu· in den Window-Words   */
/*            stehen, sonst wird nur WinDefWindowProc aufgerufen.            */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY ToolbarFrameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern TOOLBAROPTIONS ToolbarOptions;
   USHORT usCount;
   MRESULT (* EXPENTRY OldFrameProc)(HWND, ULONG, MPARAM, MPARAM);
   MRESULT resultbuf;
   SWP swp;

   OldFrameProc= (PFNWP) WinQueryWindowPtr(hwnd, 0);

   if (!OldFrameProc)
      return WinDefWindowProc(hwnd, msg, mp1, mp2);

   switch(msg)
   {
      /* Fenster bewegen oder Groesse aendern */
      case WM_QUERYTRACKINFO:
        /* Default-Werte aus Original-Prozedur holen */
        resultbuf=OldFrameProc(hwnd, msg, mp1, mp2);
        WinQueryWindowPos(hwnd, &swp);
        if (!(swp.fl & SWP_MINIMIZE)) /* nicht bei minimized */
        {
           /* Minimale Fenstergroesse einstellen */
           ((PTRACKINFO)mp2)->ptlMinTrackSize.x=440;
           ((PTRACKINFO)mp2)->ptlMinTrackSize.y=420;
        }
        return resultbuf;

      case WM_QUERYFRAMECTLCOUNT:
         usCount=(USHORT)OldFrameProc(hwnd, msg, mp1, mp2);
         return (MRESULT)(usCount+2);

      case WM_FORMATFRAME:
         {
         PSWP     pSWP = (PSWP) mp1;
         USHORT   usItemCount;
         USHORT   usMenuIndex;
         USHORT   usToolbarIndex;
         USHORT   usStatusIndex;
         USHORT   usClientIndex;
         PRECTL   prectlClient;
         HWND     hwndToolBar = WinWindowFromID(hwnd, FID_TOOLBAR);
         HWND     hwndClient = WinWindowFromID(hwnd, FID_CLIENT);
         HWND     hwndMenu = WinWindowFromID(hwnd, FID_MENU);
         HWND     hwndStatus = WinWindowFromID(hwnd, FID_STATUSLINE);

         usItemCount = SHORT1FROMMR(OldFrameProc(hwnd, msg, mp1, mp2));

         /* ------------- locate SWP for client window  ---------- */
         for (usClientIndex = 0; usClientIndex < usItemCount; usClientIndex++)
            if (pSWP[usClientIndex].hwnd == hwndClient)
               break;

         for (usMenuIndex = 0; usMenuIndex < usItemCount; usMenuIndex++)
            if (pSWP[usMenuIndex].hwnd == hwndMenu)
               break;

         /* ------ the new SWP starts after standard control SWPs ----- */
         usToolbarIndex = usItemCount+1 ;
         usStatusIndex = usItemCount ;

         /* Statuszeile formatieren */
         pSWP[usStatusIndex].cx = pSWP[usClientIndex].cx;
         pSWP[usStatusIndex].x  = pSWP[usClientIndex].x;
         pSWP[usStatusIndex].cy = 30;
         pSWP[usStatusIndex].y  = pSWP[usClientIndex].y;
         pSWP[usStatusIndex].hwndInsertBehind = HWND_TOP;
         pSWP[usStatusIndex].hwnd = hwndStatus;
         pSWP[usStatusIndex].fl = pSWP[usMenuIndex].fl;

         /* --------  adjust Client window size for Statusline ------- */
         pSWP[usClientIndex].fl |= SWP_SIZE | SWP_MOVE;
         pSWP[usClientIndex].cy -= pSWP[usStatusIndex].cy ;
         pSWP[usClientIndex].y  += pSWP[usStatusIndex].cy ;

         /* Toolbar formatieren */
         if (WinQueryWindowULong(hwndToolBar, QWL_STYLE) & TBS_VERTICAL)
         {
            pSWP[usToolbarIndex].cx = pSWP[usClientIndex].cx;
            SendMsg(hwndToolBar, WM_ADJUSTWINDOWPOS, &pSWP[usToolbarIndex], NULL);
            pSWP[usToolbarIndex].cy = pSWP[usClientIndex].cy;
            pSWP[usToolbarIndex].y = pSWP[usStatusIndex].y + pSWP[usStatusIndex].cy;
            pSWP[usToolbarIndex].hwndInsertBehind = HWND_TOP;
            pSWP[usToolbarIndex].hwnd = hwndToolBar;
            pSWP[usToolbarIndex].fl   = pSWP[usMenuIndex].fl;
            if (ToolbarOptions.ulToolbarPos == TOOLBARPOS_RIGHT)
            {
               pSWP[usToolbarIndex].x  = pSWP[usClientIndex].x +pSWP[usClientIndex].cx -pSWP[usToolbarIndex].cx;

               /* --------  adjust Client window size for 2nd menu ------- */
               pSWP[usClientIndex].fl |= SWP_SIZE;
               pSWP[usClientIndex].cx -= pSWP[usToolbarIndex].cx ;
            }
            else
            {
               pSWP[usToolbarIndex].x  = pSWP[usClientIndex].x;

               /* --------  adjust Client window size for 2nd menu ------- */
               pSWP[usClientIndex].fl |= SWP_SIZE | SWP_MOVE;
               pSWP[usClientIndex].cx -= pSWP[usToolbarIndex].cx ;
               pSWP[usClientIndex].x += pSWP[usToolbarIndex].cx ;
            }
         }
         else
         {
            pSWP[usToolbarIndex].cx = pSWP[usClientIndex].cx;
            pSWP[usToolbarIndex].x  = pSWP[usClientIndex].x;
            SendMsg(hwndToolBar, WM_ADJUSTWINDOWPOS, &pSWP[usToolbarIndex], NULL);
            pSWP[usToolbarIndex].cx = pSWP[usClientIndex].cx;
            pSWP[usToolbarIndex].hwndInsertBehind = HWND_TOP;
            pSWP[usToolbarIndex].hwnd = hwndToolBar;
            pSWP[usToolbarIndex].fl   = pSWP[usMenuIndex].fl;

            if (ToolbarOptions.ulToolbarPos == TOOLBARPOS_TOP)
            {
               pSWP[usToolbarIndex].y = pSWP[usMenuIndex].y - pSWP[usToolbarIndex].cy;

               /* --------  adjust Client window size for 2nd menu ------- */
               pSWP[usClientIndex].fl |= SWP_SIZE;
               pSWP[usClientIndex].cy -= pSWP[usToolbarIndex].cy ;
            }
            else
            {
               pSWP[usToolbarIndex].y = pSWP[usStatusIndex].y + pSWP[usStatusIndex].cy;

               /* --------  adjust Client window size for 2nd menu ------- */
               pSWP[usClientIndex].fl |= SWP_SIZE | SWP_MOVE;
               pSWP[usClientIndex].cy -= pSWP[usToolbarIndex].cy ;
               pSWP[usClientIndex].y += pSWP[usToolbarIndex].cy;
            }
         }

         /* --------  set new client size in rectl param  ---------- */
         prectlClient = (PRECTL) mp2;
         if (prectlClient)
            prectlClient->yTop -=
                            (prectlClient->yTop - prectlClient->yBottom) -
                            pSWP[usClientIndex].cy;

         /* ---  return total count of controls ( +1 for 2nd menu ) --- */
         return MRFROMSHORT(usItemCount+2);
         }

      case WM_CALCFRAMERECT :
         {
         PRECTL   prectl;
         BOOL     fCalcClientRect;
         SWP      swp;

         /* ----- first call old proc for defaults  ------- */
         if (OldFrameProc(hwnd, msg, mp1, mp2))
         {
            /* ----- get the passed parameters  -------- */
            prectl = (PRECTL) mp1;
            fCalcClientRect = (BOOL) SHORT1FROMMP( mp2 );

            /* ---- determine height of second menu ---- */
            swp.cy = 0 ;
            WinQueryWindowPos(WinWindowFromID(hwnd, FID_TOOLBAR), &swp);
            if (fCalcClientRect)
            {
               /*
                *  we are calculating the client rect so we must SUBTRACT
                *  the height of the 2nd menu from the default client size
                */
                prectl->yTop -= swp.cy;
            }
            else
            {
               /*
                *  we are calculating the frame rect so we must ADD
                *  the height of the 2nd menu to the default frame size
                */
                prectl->yTop += swp.cy;
            }
         }
         else
           return FALSE ;
         }
         return (MRESULT) TRUE;

      default:
         break;
   }
   return OldFrameProc(hwnd, msg, mp1, mp2);
}

int CreateStatusLine(HWND hwndFrame)
{
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS windowfonts;

   HWND hwnd;
   STLFIELDINFO STLField;
   PWINDOWDATA pWindowData = (PWINDOWDATA) WinQueryWindowULong(WinWindowFromID(hwndFrame, FID_CLIENT), QWL_USER);

   hwnd=WinCreateWindow(hwndFrame,
                        WC_STATUSLINE,
                        NULL,
                        WS_VISIBLE | STLS_BORDER,
                        0,0,
                        200,30,
                        hwndFrame,
                        HWND_TOP,
                        FID_STATUSLINE,
                        NULL,
                        NULL);

   SetForeground(hwnd, &windowcolors.statusfore);
   SetBackground(hwnd, &windowcolors.statusback);
   SetFont(hwnd, windowfonts.statusfont);

   STLField.ulFlags=STLF_VARIABLE | STLF_3D;
   STLField.lFieldSize=100;
   STLField.lColorForeground=windowcolors.statusfore;
   STLField.pszFontName=NULL;
   STLField.ulTimeout = MSGTIMEOUT;
   pWindowData->idMessageField=(ULONG)SendMsg(hwnd, STLM_ADDFIELD, &STLField,
                                                 MPFROMLONG(STLI_LAST));

   STLField.ulFlags=STLF_FIXED | STLF_3D | STLF_CENTER | STLF_CHECK;
   STLField.lFieldSize=30;
   STLField.lColorForeground=windowcolors.statusfore;
   STLField.pszFontName=NULL;
   STLField.ulTimeout = 0;
   pWindowData->idCheckField=(ULONG)SendMsg(hwnd, STLM_ADDFIELD, &STLField,
                                                MPFROMLONG(STLI_LAST));

   STLField.ulFlags=STLF_FIXED | STLF_3D | STLF_CENTER;
   STLField.lFieldSize=60;
   STLField.lColorForeground=windowcolors.statusfore;
   STLField.pszFontName=NULL;
   STLField.ulTimeout = 0;
   pWindowData->idCursorField=(ULONG)SendMsg(hwnd, STLM_ADDFIELD, &STLField,
                                                MPFROMLONG(STLI_LAST));

   STLField.ulFlags=STLF_FIXED | STLF_3D | STLF_CENTER;
   STLField.lFieldSize=50;
   STLField.lColorForeground=windowcolors.statusfore;
   STLField.pszFontName=NULL;
   STLField.ulTimeout = 0;
   pWindowData->idKeybField=(ULONG)SendMsg(hwnd, STLM_ADDFIELD, &STLField,
                                              MPFROMLONG(STLI_LAST));

   STLField.ulFlags=STLF_FIXED | STLF_3D | STLF_CENTER;
   STLField.lFieldSize=110;
   STLField.lColorForeground=windowcolors.statusfore;
   STLField.pszFontName=NULL;
   STLField.ulTimeout = 0;
   pWindowData->idNumberField=(ULONG)SendMsg(hwnd, STLM_ADDFIELD, &STLField,
                                                 MPFROMLONG(STLI_LAST));

   STLField.ulFlags=STLF_FIXED | STLF_3D | STLF_CENTER;
   STLField.lFieldSize=120;
   STLField.lColorForeground=windowcolors.statusfore;
   STLField.pszFontName=NULL;
   STLField.ulTimeout = 0;
   pWindowData->idAddressField=(ULONG)SendMsg(hwnd, STLM_ADDFIELD, &STLField,
                                                 MPFROMLONG(STLI_LAST));

   return 0;
}

int CreateToolbar(HWND hwndFrame)
{
   TBCTLDATA CtlData;
   extern TOOLBAROPTIONS ToolbarOptions;
   extern TOOLBARCONFIG ToolbarConfig;
   ULONG ulStyle = TBS_BORDER | TBS_SCROLLABLE;

   if (ToolbarOptions.ulToolbarPos == TOOLBARPOS_LEFT ||
       ToolbarOptions.ulToolbarPos == TOOLBARPOS_RIGHT)
      ulStyle |= TBS_VERTICAL;


   CtlData.cb = sizeof(CtlData);
   CtlData.lButtonSpacing = 0;
   CtlData.lExtraSpacing = 5 ;
   CtlData.lBorderX =  2;
   CtlData.lBorderY =  2;
   WinCreateWindow(hwndFrame,
                   WC_TOOLBAR,
                   NULL,
                   ulStyle,
                   0, 0,
                   400, 80,
                   hwndFrame,
                   HWND_TOP,
                   FID_TOOLBAR,
                   &CtlData,
                   NULL);

   RefreshToolbar(WinWindowFromID(hwndFrame, FID_TOOLBAR), &ToolbarConfig, ToolbarOptions.bSmallToolbar);

   return 0;
}

int OpenToolbarContext(HWND hwndClient, PWINDOWDATA pWindowData, ULONG ulButtonID)
{
   POINTL pointl;
   extern TOOLBAROPTIONS ToolbarOptions;

   WinQueryPointerPos(HWND_DESKTOP, &pointl);

   if (!ulButtonID)
   {
      WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_TOP, FALSE);
      WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_BOTTOM, FALSE);
      WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_LEFT, FALSE);
      WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_RIGHT, FALSE);

      switch(ToolbarOptions.ulToolbarPos)
      {
         case TOOLBARPOS_TOP:
            WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_TOP, TRUE);
            break;

         case TOOLBARPOS_BOTTOM:
            WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_BOTTOM, TRUE);
            break;

         case TOOLBARPOS_LEFT:
            WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_LEFT, TRUE);
            break;

         case TOOLBARPOS_RIGHT:
            WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_RIGHT, TRUE);
            break;

         default:
            break;
      }
      if (ToolbarOptions.bSmallToolbar)
         WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_SMALL, TRUE);
      else
         WinCheckMenuItem(pWindowData->hwndToolbarPopup, IDM_TB_SMALL, FALSE);

      WinPopupMenu(HWND_DESKTOP, hwndClient, pWindowData->hwndToolbarPopup,
                   pointl.x, pointl.y,
                   0,
                   PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1);
   }

   return 0;
}

int SwitchToolbarPos(HWND hwndClient, ULONG ulCommand)
{
   extern TOOLBAROPTIONS ToolbarOptions;
   extern DIRTYFLAGS dirtyflags;
   HWND hwndFrame = WinQueryWindow(hwndClient, QW_PARENT);
   HWND hwndToolbar = WinWindowFromID(hwndFrame, FID_TOOLBAR);
   ULONG ulStyle = WinQueryWindowULong(hwndToolbar, QWL_STYLE);

   switch(ulCommand)
   {
      case IDM_TB_TOP:
         ulStyle &= ~TBS_VERTICAL;
         ToolbarOptions.ulToolbarPos = TOOLBARPOS_TOP;
         break;

      case IDM_TB_BOTTOM:
         ulStyle &= ~TBS_VERTICAL;
         ToolbarOptions.ulToolbarPos = TOOLBARPOS_BOTTOM;
         break;

      case IDM_TB_LEFT:
         ulStyle |= TBS_VERTICAL;
         ToolbarOptions.ulToolbarPos = TOOLBARPOS_LEFT;
         break;

      case IDM_TB_RIGHT:
         ulStyle |= TBS_VERTICAL;
         ToolbarOptions.ulToolbarPos = TOOLBARPOS_RIGHT;
         break;

      default:
         break;
   }
   WinSetWindowULong(hwndToolbar, QWL_STYLE, ulStyle);
   SendMsg(hwndFrame, WM_UPDATEFRAME, NULL, NULL);
   dirtyflags.toolbardirty = TRUE;

   return 0;
}

int SwitchToolbarSize(HWND hwndClient)
{
   extern TOOLBAROPTIONS ToolbarOptions;
   extern DIRTYFLAGS dirtyflags;
   HWND hwndFrame = WinQueryWindow(hwndClient, QW_PARENT);
   HWND hwndToolbar = WinWindowFromID(hwndFrame, FID_TOOLBAR);
   TOOLBARITEM TBItem;
   ULONG ulResult;

   ToolbarOptions.bSmallToolbar = !ToolbarOptions.bSmallToolbar;

   WinEnableWindowUpdate(hwndToolbar, FALSE);
   ulResult = (ULONG) SendMsg(hwndToolbar, TBM_QUERYFIRSTITEM, &TBItem, NULL);
   while (ulResult == TBQUERY_OK)
   {
      TBItem.ulBitmapID = QueryBitmap(TBItem.ulCommandID, ToolbarOptions.bSmallToolbar);
      SendMsg(hwndToolbar, TBM_SETITEMDATA, NULL, &TBItem);

      ulResult = (ULONG) SendMsg(hwndToolbar, TBM_QUERYNEXTITEM, &TBItem, NULL);
   }
   WinEnableWindowUpdate(hwndToolbar, TRUE);

   SendMsg(hwndFrame, WM_UPDATEFRAME, NULL, NULL);
   dirtyflags.toolbardirty = TRUE;

   return 0;
}

void SetTranslateMode(BOOL bTranslate)
{
   extern HWND client;

   WinSendDlgItemMsg(client, IDML_MAINEDIT, MLM_SETTRANSLATE,
                     (MPARAM) bTranslate, NULL);
   WinSendDlgItemMsg(client, IDE_FROMNAME, MLM_SETTRANSLATE,
                     (MPARAM) bTranslate, NULL);
   WinSendDlgItemMsg(client, IDE_TONAME, MLM_SETTRANSLATE,
                     (MPARAM) bTranslate, NULL);
   WinSendDlgItemMsg(client, IDE_SUBJTEXT, MLM_SETTRANSLATE,
                     (MPARAM) bTranslate, NULL);
   return;
}

void BackToWindow(PWINDOWDATA pWindowData)
{
   extern HWND hwndAreaDlg, hwndFindResults, hwndThreadList, hwndMsgList;

   switch(pWindowData->ulSourceWindow)
   {
      case SOURCEWIN_AREA:
         if (hwndAreaDlg)
            SetFocusControl(hwndAreaDlg, IDD_AREALIST+1);
         break;

      case SOURCEWIN_MSGLIST:
         if (hwndMsgList)
            SetFocusControl(hwndMsgList, IDD_MSGLIST+1);
         break;

      case SOURCEWIN_THREAD:
         if (hwndThreadList)
            SetFocusControl(hwndThreadList, IDD_THREADLIST+1);
         break;

      case SOURCEWIN_BOOKMARKS:
         if (hwndFindResults)
            SetFocusControl(hwndFindResults, IDD_FINDRESULTS+1);
         break;

      default:
         break;
   }

   pWindowData->ulSourceWindow = SOURCEWIN_NONE;

   return;
}

int OpenProgressBar(PWINDOWDATA pWindowData, PCHAR pchAreaTag)
{
   STLFIELDINFO STLField;
   extern HWND frame;
   extern WINDOWCOLORS windowcolors;
   HWND hwndStatus = WinWindowFromID(frame, FID_STATUSLINE);

   if (!pWindowData->idProgressField)
   {
      STLField.ulFlags=STLF_VARIABLE | STLF_3D | STLF_PROGRESS;
      STLField.lFieldSize=50;
      STLField.lColorForeground=windowcolors.statusfore;
      STLField.pszFontName=NULL;
      STLField.ulTimeout = 0;

      SendMsg(hwndStatus, STLM_SETTIMEOUT, MPFROMLONG(pWindowData->idMessageField), MPFROMLONG(0));
      SendMsg(hwndStatus, STLM_SETFIELDWIDTH, MPFROMLONG(pWindowData->idMessageField), MPFROMLONG(50));

      pWindowData->idProgressField = (ULONG) SendMsg(hwndStatus, STLM_ADDFIELD,
                                            &STLField, MPFROMLONG(pWindowData->idMessageField));
   }
   SendMsg(hwndStatus, STLM_SETFIELDTEXT, MPFROMLONG(pWindowData->idMessageField), pchAreaTag);
   SendMsg(hwndStatus, STLM_SETFIELDTEXT, MPFROMLONG(pWindowData->idProgressField), NULL);
   pWindowData->lLastProgress=0;

   return 0;
}

int CloseProgressBar(PWINDOWDATA pWindowData)
{
   extern HWND frame;
   HWND hwndStatus = WinWindowFromID(frame, FID_STATUSLINE);

   SendMsg(hwndStatus, STLM_SETFIELDWIDTH, MPFROMLONG(pWindowData->idMessageField), MPFROMLONG(100));
   SendMsg(hwndStatus, STLM_SETTIMEOUT, MPFROMLONG(pWindowData->idMessageField), MPFROMLONG(MSGTIMEOUT));
   SendMsg(hwndStatus, STLM_REMOVEFIELD, MPFROMLONG(pWindowData->idProgressField), NULL);
   pWindowData->idProgressField = 0;
   pWindowData->lLastProgress=0;
   SendMsg(hwndStatus, STLM_SETFIELDTEXT, MPFROMLONG(pWindowData->idMessageField), "");

   return 0;
}

int ProgressBarProgress(PWINDOWDATA pWindowData, LONG lPercent)
{
   extern HWND frame;
   HWND hwndStatus = WinWindowFromID(frame, FID_STATUSLINE);

   if (lPercent/5 != pWindowData->lLastProgress)
   {
      pWindowData->lLastProgress = lPercent/5;
      SendMsg(hwndStatus, STLM_SETFIELDTEXT, MPFROMLONG(pWindowData->idProgressField),
              MPFROMLONG(lPercent));
   }

   return 0;
}

static MRESULT SetReadOnly(HWND hwnd, ULONG Control, BOOL bReadOnly)
{
   return WinSendDlgItemMsg(hwnd, Control, EM_SETREADONLY, (MPARAM) bReadOnly, NULL);
}

/*-------------------------------- Modulende --------------------------------*/



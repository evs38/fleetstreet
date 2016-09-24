/*---------------------------------------------------------------------------+
 | Titel: NICKNAMES.C                                                        |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 03.04.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Setup der Nicknames                                                   |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_WIN
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "mainwindow.h"
#include "fltv7\fltv7.h"
#include "lookups.h"
#include "dialogids.h"
#include "utility.h"
#include "handlemsg\handlemsg.h"
#include "util\addrcnv.h"
#include "controls\attrselect.h"
#include "nickmanage.h"
#include "nicknames.h"

/*--------------------------------- Defines ---------------------------------*/

#define UNREG_NICKNAMES  10

/*---------------------------------- Typen ----------------------------------*/

typedef struct {
          RECORDCORE RecordCore;
          PSZ pszNickName;
          PSZ pszRealName;
          PSZ pszAddress;
        } NICKRECORD, *PNICKRECORD;

typedef struct {
           USHORT cb;
           char pchTitle[LEN_FIRSTLINE+1];
           char pchLine[LEN_FIRSTLINE+1];
         } PROMPTPAR;

typedef struct
{
   USHORT cb;
   char pchNickSel[LEN_USERNAME+1];
} NICKSELECT, *PNICKSELECT;

/*---------------------------- Globale Variablen ----------------------------*/

extern HMODULE hmodLang;
extern HAB anchor;

PFNWP OldContainerProc;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static void UpdateNickList(HWND parent);
static SHORT _System SortNicknames(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage);
static MRESULT EXPENTRY NickEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY NewNContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY PromptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

/*---------------------------------------------------------------------------*/
/* Funktionsname:                                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung:                                                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewNContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
      case DM_DRAGOVER:
         DrgAccessDraginfo(mp1);
         break;

      default:
         break;
   }
   return OldContainerProc(parent, message, mp1, mp2);
}

/*-----------------------------  AdrBookProc --------------------------------*/
/* Dialog-Prozedur des Adressbuch-Dialogs                                    */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY AdrBookProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern NICKNAMELIST NickNameList;
   extern GENERALOPT generaloptions;
   extern WINDOWFONTS windowfonts;
   extern WINDOWCOLORS windowcolors;
   extern HWND hwndhelp;
   static char pchTitleNickname[100];
   static char pchTitleRealname[100];
   static char pchTitleAddress[100];
   static bNotify;
   PFIELDINFO pFieldInfo, pFirstFieldInfo;
   FIELDINFOINSERT FieldInfoInsert;
   CNRINFO cnrinfo;
   PNICKRECORD pRecord;
   extern BOOL isregistered;
   MRESULT resultbuf;
   PNICKNAME zeiger;
   PNICKSELECT pNickSelect = WinQueryWindowPtr(parent, QWL_USER);

   switch(message)
   {
      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            NICKPAR NickPar;

            switch (SHORT1FROMMP(mp1))
            {
               /* Add-Button */
               case IDD_NICKNAMES+10:
                  NickPar.cb=sizeof(NICKPAR);
                  memset(&NickPar.entry, 0, sizeof(NickPar.entry));
                  NickPar.bNewEntry=TRUE;
                  NickPar.bDirectAdd=FALSE;

                  if (WinDlgBox(HWND_DESKTOP, parent, NickEntryProc,
                                hmodLang, IDD_NICKENTRY, &NickPar)!=DID_OK)
                     return (MRESULT) FALSE;
                  AddNickname(&NickNameList, &NickPar.entry, TRUE);
                  if (NickPar.entry.pchComment)
                     free(NickPar.entry.pchComment);
                  UpdateNickList(parent);
                  return (MRESULT) FALSE;

               /* Change-Button */
               case IDD_NICKNAMES+13:
                  pRecord=WinSendDlgItemMsg(parent, IDD_NICKNAMES+6, CM_QUERYRECORDEMPHASIS,
                                            (MPARAM) CMA_FIRST,
                                            MPFROMSHORT(CRA_SELECTED));
                  if (pRecord)
                  {
                     /* Eintrag suchen */
                     zeiger=FindNickname(&NickNameList, pRecord->pszNickName, NULL);
                     if (zeiger)
                     {
                        NickPar.cb=sizeof(NICKPAR);
                        memcpy(&NickPar.entry, zeiger, sizeof(NickPar.entry));
                        if (zeiger->pchComment)
                           NickPar.entry.pchComment = strdup(zeiger->pchComment);
                        NickPar.bNewEntry=FALSE;
                        NickPar.pEntry=zeiger;
                        if (WinDlgBox(HWND_DESKTOP, parent, NickEntryProc,
                                      hmodLang, IDD_NICKENTRY, &NickPar)!=DID_OK)
                           return (MRESULT) FALSE;
                        ChangeNickname(&NickNameList, zeiger, &NickPar.entry);
                        if (NickPar.entry.pchComment)
                           free(NickPar.entry.pchComment);
                        UpdateNickList(parent);
                     }
                     else
                        WinAlarm(HWND_DESKTOP, WA_ERROR);
                  }
                  else
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                  return (MRESULT) FALSE;

               /* Delete-Button */
               case IDD_NICKNAMES+11:
                  if (generaloptions.safety & SAFETY_CHANGESETUP)
                  {
                     if (MessageBox(parent, IDST_MSG_DELNICKNAME, IDST_TITLE_DELNICKNAME,
                                    IDD_DELNICKNAME, MB_YESNO | MB_ICONQUESTION)==MBID_NO)
                        return (MRESULT) FALSE;
                  }
                  pRecord=WinSendDlgItemMsg(parent, IDD_NICKNAMES+6, CM_QUERYRECORDEMPHASIS,
                                            (MPARAM) CMA_FIRST,
                                            MPFROMSHORT(CRA_SELECTED));
                  if (pRecord)
                  {
                     /* Eintrag loeschen */
                     DeleteNickname(&NickNameList, pRecord->pszNickName);
                  }
                  else
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                  UpdateNickList(parent);
                  return (MRESULT) FALSE;

               case DID_OK:
                  if (pNickSelect)
                  {
                     /* selektierten Nickname zurckgeben */
                     pRecord=WinSendDlgItemMsg(parent, IDD_NICKNAMES+6, CM_QUERYRECORDEMPHASIS,
                                               (MPARAM) CMA_FIRST,
                                               MPFROMSHORT(CRA_CURSORED));
                     if (pRecord)
                        strcpy(pNickSelect->pchNickSel, pRecord->pszNickName);
                     else
                        pNickSelect->pchNickSel[0]=0;
                  }
                  break;

               default:
                  break;
            }
         }
         break;

      /* Doppelklick auf Eintrag */
      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_NICKNAMES+6)
         {
            if (SHORT2FROMMP(mp1)==CN_ENTER)
            {
               if (pNickSelect)
                  WinPostMsg(parent, WM_COMMAND, MPFROMSHORT(DID_OK),
                             MPFROMSHORT(CMDSRC_PUSHBUTTON));
               else
                  if (((PNOTIFYRECORDENTER)mp2)->pRecord)
                     SendMsg(parent, WM_COMMAND, MPFROMSHORT(IDD_NICKNAMES+13),
                                MPFROMSHORT(CMDSRC_PUSHBUTTON));
            }

            if (SHORT2FROMMP(mp1)==CN_HELP)
               SendMsg(parent, WM_HELP, MPFROMSHORT(IDD_NICKNAMES+6), NULL);
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, parent);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_QUERYTRACKINFO:
         {
            SWP swp;

            /* Default-Werte aus Original-Prozedur holen */
            resultbuf=WinDefDlgProc(parent,message,mp1,mp2);

            WinQueryWindowPos(WinWindowFromID(parent, IDD_NICKNAMES+12),
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
            WinSetWindowPos(WinWindowFromID(parent, IDD_NICKNAMES+6),
                            NULLHANDLE,
                            rectl.xLeft, rectl.yBottom,
                            rectl.xRight-rectl.xLeft, rectl.yTop-rectl.yBottom,
                            SWP_MOVE | SWP_SIZE);
         }
         break;

      case WM_INITDLG:
         OldContainerProc=WinSubclassWindow(WinWindowFromID(parent, IDD_NICKNAMES+6),
                                            NewNContainerProc);

         WinSetWindowPtr(parent, QWL_USER, (PNICKSELECT) mp2);

         SetFont(WinWindowFromID(parent, IDD_NICKNAMES+6), windowfonts.nicknamesfont);
         SetForeground(WinWindowFromID(parent, IDD_NICKNAMES+6), &windowcolors.nicknamesfore);
         SetBackground(WinWindowFromID(parent, IDD_NICKNAMES+6), &windowcolors.nicknamesback);

         /* Titelstrings laden */
         LoadString(IDST_NICK_NICKNAME, 100, pchTitleNickname);
         LoadString(IDST_NICK_REALNAME, 100, pchTitleRealname);
         LoadString(IDST_NICK_ADDRESS,  100, pchTitleAddress);

         /* Container vorbereiten */
         pFirstFieldInfo = WinSendDlgItemMsg(parent, IDD_NICKNAMES+6,
                           CM_ALLOCDETAILFIELDINFO, MPFROMLONG(3), NULL);
         pFieldInfo=pFirstFieldInfo;

         pFieldInfo->cb = sizeof(FIELDINFO);
         pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_LEFT | CFA_SEPARATOR;
         pFieldInfo->flTitle = CFA_FITITLEREADONLY;
         pFieldInfo->pTitleData = pchTitleNickname;
         pFieldInfo->offStruct = FIELDOFFSET(NICKRECORD, pszNickName);
         pFieldInfo = pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb = sizeof(FIELDINFO);
         pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_LEFT | CFA_SEPARATOR;
         pFieldInfo->flTitle = CFA_FITITLEREADONLY;
         pFieldInfo->pTitleData = pchTitleRealname;
         pFieldInfo->offStruct = FIELDOFFSET(NICKRECORD, pszRealName);
         pFieldInfo = pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb = sizeof(FIELDINFO);
         pFieldInfo->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_LEFT;
         pFieldInfo->flTitle = CFA_FITITLEREADONLY;
         pFieldInfo->pTitleData = pchTitleAddress;
         pFieldInfo->offStruct = FIELDOFFSET(NICKRECORD, pszAddress);
         pFieldInfo = pFieldInfo->pNextFieldInfo;

         FieldInfoInsert.cb = (ULONG)(sizeof(FIELDINFOINSERT));
         FieldInfoInsert.pFieldInfoOrder = (PFIELDINFO)CMA_FIRST;
         FieldInfoInsert.cFieldInfoInsert = 3;
         FieldInfoInsert.fInvalidateFieldInfo = TRUE;

         WinSendDlgItemMsg(parent, IDD_NICKNAMES+6, CM_INSERTDETAILFIELDINFO,
                           MPFROMP(pFirstFieldInfo),
                           MPFROMP(&FieldInfoInsert));

         cnrinfo.cb=sizeof(CNRINFO);
         cnrinfo.flWindowAttr=CV_DETAIL | CA_DETAILSVIEWTITLES;
         cnrinfo.pSortRecord=(PVOID)SortNicknames;
         WinSendDlgItemMsg(parent,IDD_NICKNAMES+6, CM_SETCNRINFO, &cnrinfo,
                            MPFROMLONG(CMA_FLWINDOWATTR | CMA_PSORTRECORD ));

         UpdateNickList(parent);
         RestoreWinPos(parent, &NickNameList.FolderPos, TRUE, TRUE);
         bNotify = TRUE;
         break;

      case WM_DESTROY:
         QueryFont(WinWindowFromID(parent, IDD_NICKNAMES+6), windowfonts.nicknamesfont);
         QueryForeground(WinWindowFromID(parent, IDD_NICKNAMES+6), &windowcolors.nicknamesfore);
         QueryBackground(WinWindowFromID(parent, IDD_NICKNAMES+6), &windowcolors.nicknamesback);
         bNotify=FALSE;
         break;

      case WM_WINDOWPOSCHANGED:
         if (bNotify)
            SaveWinPos(parent, (PSWP) mp1, &NickNameList.FolderPos, &NickNameList.bDirty);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*-----------------------------  SortNicknames  -----------------------------*/
/* Nicknameliste sortieren                                                   */
/*---------------------------------------------------------------------------*/

static SHORT _System SortNicknames(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage)
{
   pStorage=pStorage;

   return stricmp(((NICKRECORD*)p1)->pszNickName, ((NICKRECORD*)p2)->pszNickName);
}


/*-----------------------------  UpdateNickList -----------------------------*/
/* Nicknameliste neu fuellen                                                 */
/*---------------------------------------------------------------------------*/

static void UpdateNickList(HWND parent)
{
   extern NICKNAMELIST NickNameList;
   PNICKNAME zeiger;
   HWND hwndListe=NULLHANDLE;
   RECORDINSERT RecordInsert;
   PNICKRECORD pRecord, pFirstRecord;

   hwndListe=WinWindowFromID(parent, IDD_NICKNAMES+6);

   WinEnableWindowUpdate(hwndListe, FALSE);

   /* Liste loeschen */
   SendMsg(hwndListe, CM_REMOVERECORD, NULL,
              MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));

   /* Elemente zaehlen */
   if (NickNameList.ulNumEntries)
   {
      /* Speicher anfordern */
      pFirstRecord=SendMsg(hwndListe, CM_ALLOCRECORD,
                              MPFROMLONG(sizeof(NICKRECORD)-sizeof(RECORDCORE)),
                              MPFROMLONG(NickNameList.ulNumEntries));
      pRecord=pFirstRecord;

      /* Records vorbereiten */
      zeiger=NULL;
      while (zeiger = FindNickname(&NickNameList, NULL, zeiger))
      {
         pRecord->RecordCore.flRecordAttr=0;
         pRecord->pszNickName=zeiger->usertag;
         pRecord->pszRealName=zeiger->username;
         pRecord->pszAddress=zeiger->address;

         pRecord=(PNICKRECORD)pRecord->RecordCore.preccNextRecord;
      }
      /* Records einfuegen */

      RecordInsert.cb=sizeof(RECORDINSERT);
      RecordInsert.pRecordOrder=(PRECORDCORE) CMA_FIRST;
      RecordInsert.pRecordParent=NULL;
      RecordInsert.fInvalidateRecord=TRUE;
      RecordInsert.zOrder=CMA_TOP;
      RecordInsert.cRecordsInsert=NickNameList.ulNumEntries;

      SendMsg(hwndListe, CM_INSERTRECORD, pFirstRecord, &RecordInsert);
   }

   WinEnableWindowUpdate(hwndListe, TRUE);

   /* Leere Liste */
   if (!NickNameList.ulNumEntries)
   {
      WinEnableControl(parent, IDD_NICKNAMES+11, FALSE);
      WinEnableControl(parent, IDD_NICKNAMES+13, FALSE);
   }
   else
   {
      WinEnableControl(parent, IDD_NICKNAMES+11, TRUE);
      WinEnableControl(parent, IDD_NICKNAMES+13, TRUE);
   }
   return;
}

/*----------------------------- NickEntryProc -------------------------------*/
/* Dialog-Prozedur des Nickname-Eintrag-Dialogs                              */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NickEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   static NICKPAR *NickPar;
   extern HWND hwndhelp;
   extern WINDOWPOSITIONS windowpositions;
   extern HAB anchor;
   extern NICKNAMELIST NickNameList;
   extern USERDATAOPT userdaten;

   switch(message)
   {
      case WM_INITDLG:
         NickPar=(NICKPAR*) mp2;
         WinAssociateHelpInstance(hwndhelp, parent);

         /* Textlimits */
         WinSendDlgItemMsg(parent,IDD_NICKENTRY+1, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_USERNAME), NULL);

         WinSendDlgItemMsg(parent,IDD_NICKENTRY+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_USERNAME), NULL);

         WinSendDlgItemMsg(parent,IDD_NICKENTRY+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_5DADDRESS), NULL);

         WinSendDlgItemMsg(parent,IDD_NICKENTRY+4, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_SUBJECT), NULL);

         WinSendDlgItemMsg(parent,IDD_NICKENTRY+16, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_FIRSTLINE), NULL);

         /* Texte */
         WinSetDlgItemText(parent, IDD_NICKENTRY+1, NickPar->entry.usertag);
         if (NickPar->entry.usertag[0])
            WinEnableControl(parent, DID_OK, TRUE);
         WinSetDlgItemText(parent, IDD_NICKENTRY+2, NickPar->entry.username);
         WinSetDlgItemText(parent, IDD_NICKENTRY+3, NickPar->entry.address);
         WinSetDlgItemText(parent, IDD_NICKENTRY+4, NickPar->entry.subjectline);
         WinSetDlgItemText(parent, IDD_NICKENTRY+16, NickPar->entry.firstline);

         /* Default-Flags */
         WinSendDlgItemMsg(parent, IDD_NICKENTRY+11, ATTSM_SETATTRIB,
                           MPFROMLONG(NickPar->entry.ulAttrib),
                           MPFROMLONG(ATTRIB_ALL));

         if (NickPar->entry.ulFlags & NICKFLAG_NOTEMPLATE)
            WinCheckButton(parent, IDD_NICKENTRY+18, TRUE);

         RestoreWinPos(parent, &windowpositions.nickentrypos, FALSE, TRUE);
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         QueryWinPos(parent, &windowpositions.nickentrypos);
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)==DID_OK)
         {
            WinQueryDlgItemText(parent, IDD_NICKENTRY+1, LEN_USERNAME+1,
                                NickPar->entry.usertag);
            if (NickPar->bNewEntry)
            {
               if (FindNickname(&NickNameList, NickPar->entry.usertag, NULL))
               {
                  if (!NickPar->bDirectAdd)
                  {
                     /* Nickname schon vorhanden */
                     MessageBox(parent, IDST_MSG_HAVENICKNAME, 0, IDD_HAVENICKNAME,
                                   MB_OK | MB_ERROR);
                     return (MRESULT) FALSE;
                  }
                  else
                  {
                     if (MessageBox(parent, IDST_MSG_REPLACENICK, IDST_TITLE_REPLACENICK,
                                    IDD_REPLACENICK, MB_YESNO | MB_QUERY)!=MBID_YES)
                        return (MRESULT) FALSE;
                  }
               }
            }
            else
            {
               if (FindNickname(&NickNameList, NickPar->entry.usertag, NULL) &&
                   FindNickname(&NickNameList, NickPar->entry.usertag, NULL) != NickPar->pEntry)
               {
                  /* Nickname schon vorhanden */
                  MessageBox(parent, IDST_MSG_HAVENICKNAME, 0, IDD_HAVENICKNAME,
                                MB_OK | MB_ERROR);
                  return (MRESULT) FALSE;
               }
            }
            WinQueryDlgItemText(parent, IDD_NICKENTRY+2, LEN_USERNAME+1,
                                NickPar->entry.username);
            WinQueryDlgItemText(parent, IDD_NICKENTRY+3, LEN_5DADDRESS+1,
                                NickPar->entry.address);
            WinQueryDlgItemText(parent, IDD_NICKENTRY+4, LEN_SUBJECT+1,
                                NickPar->entry.subjectline);
            WinQueryDlgItemText(parent, IDD_NICKENTRY+16, LEN_FIRSTLINE+1,
                                NickPar->entry.firstline);

            NickPar->entry.ulAttrib= (ULONG) WinSendDlgItemMsg(parent, IDD_NICKENTRY+11,
                                                ATTSM_QUERYATTRIB, NULL, NULL);

            NickPar->entry.ulFlags = 0;
            if (WinQueryButtonCheckstate(parent, IDD_NICKENTRY+18))
               NickPar->entry.ulFlags |= NICKFLAG_NOTEMPLATE;
         }

         if (SHORT1FROMMP(mp1)==IDD_NICKENTRY+17)
         {
            char FoundName[LEN_USERNAME+1];
            char FoundAddress[LEN_5DADDRESS+1];

            WinQueryDlgItemText(parent, IDD_NICKENTRY+2, sizeof(FoundName), FoundName);
            if (PerformNameLookup(FoundName, parent, LOOKUP_NORMAL, FoundName, FoundAddress))
            {
               WinSetDlgItemText(parent, IDD_NICKENTRY+2, FoundName);
               WinSetDlgItemText(parent, IDD_NICKENTRY+3, FoundAddress);
            }
            return (MRESULT) FALSE;
         }
         break;

      case WM_CHAR:
         if (WinQueryDlgItemTextLength(parent, IDD_NICKENTRY+1))
            WinEnableControl(parent, DID_OK, TRUE);
         else
            WinEnableControl(parent, DID_OK, FALSE);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_NICKENTRY+3)
            if (SHORT2FROMMP(mp1)==EN_KILLFOCUS)
            {
               FTNADDRESS NetAddr;
               char pchTemp[LEN_5DADDRESS+1];

               WinQueryDlgItemText(parent, SHORT1FROMMP(mp1),
                                   LEN_5DADDRESS+1, pchTemp);
               StringToNetAddr(pchTemp, &NetAddr, userdaten.address[0]);
               NetAddrToString(pchTemp, &NetAddr);
               WinSetDlgItemText(parent, SHORT1FROMMP(mp1), pchTemp);
            }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddToNick                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt aktuellen Absender zu Nicknames dazu                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndOwner: Owner-Window                                        */
/*            pHeader: zeiger auf aktuellen Header                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void AddToNick(HWND hwndOwner, MSGHEADER *pHeader)
{
   extern NICKNAMELIST NickNameList;
   extern HWND client;
   NICKPAR NickPar;
   PNICKNAME zeiger=NULL;
   extern BOOL isregistered;

   NickPar.cb=sizeof(NICKPAR);
   memset(&NickPar.entry, 0, sizeof(NickPar.entry));
   NickPar.bNewEntry=TRUE;
   NickPar.bDirectAdd=TRUE;

   /* Name und Adresse fuellen */
   memcpy(NickPar.entry.username, pHeader->pchFromName, LEN_USERNAME);
   NetAddrToString(NickPar.entry.address, &pHeader->FromAddress);

   SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);

   if (WinDlgBox(HWND_DESKTOP, hwndOwner, NickEntryProc,
                 hmodLang, IDD_NICKENTRY, &NickPar)!=DID_OK)
   {
      SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
      return;
   }

   zeiger= FindNickname(&NickNameList, NickPar.entry.usertag, NULL);
   if (!zeiger)
   {
      /* noch nicht vorhanden */
      AddNickname(&NickNameList, &NickPar.entry, TRUE);
   }
   else
      /* schon vorhanden, aendern */
      ChangeNickname(&NickNameList, zeiger, &NickPar.entry);

   SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);

   return;
}

/*----------------------------- LookupNickname  -----------------------------*/
/* sucht den Nickname in der Nicknameliste, ersetzt ggf. Name, Adresse,      */
/* und Subject, setzt Focus weiter.                                          */
/* Rueckgabewerte: TRUE Nickname gefunden                                    */
/*                 FALSE Nickname nicht gefunden                             */
/*---------------------------------------------------------------------------*/

BOOL LookupNickname(HWND hwndClient, char *pchNick, PNICKNAMELIST Liste)
{
   PNICKNAME zeiger=NULL;
   extern MSGHEADER CurrentHeader;
   extern LONG iptInitialPos2;
   extern BOOL bTemplateProcessed;
   extern USERDATAOPT userdaten;
   int iMatch;
   FTNADDRESS tempAddr, tempAddr2;
   char pchTemp[LEN_5DADDRESS+1];
   NICKSELECT NickSelect;

   if (!*pchNick)
   {
      memset(&NickSelect, 0, sizeof(NickSelect));
      NickSelect.cb = sizeof(NickSelect);

      if (WinDlgBox(HWND_DESKTOP, hwndClient, AdrBookProc, hmodLang,
                    IDD_NICKNAMES, &NickSelect) == DID_OK)
         pchNick = NickSelect.pchNickSel;
      else
         return TRUE;
   }

   zeiger=FindNickname(Liste, pchNick, NULL);

   /* nicht gefunden oder Liste leer */
   if (!zeiger)
      return FALSE;

   WinSetDlgItemText(hwndClient, IDE_TONAME, zeiger->username);
   WinSetDlgItemText(hwndClient, IDE_TOADDRESS, zeiger->address);
   StringToNetAddr(zeiger->address, &tempAddr, NULL);
   WinQueryWindowText(WinWindowFromID(hwndClient, IDE_FROMADDRESS), LEN_5DADDRESS+1, pchTemp);
   StringToNetAddr(pchTemp, &tempAddr2, NULL);
   iMatch = MSG_MatchAddress(&tempAddr, &userdaten, &tempAddr2);
   if (iMatch>=0)
      WinSetDlgItemText(hwndClient, IDE_FROMADDRESS, userdaten.address[iMatch]);

   if (zeiger->ulFlags & NICKFLAG_NOTEMPLATE)
      bTemplateProcessed=TRUE;
   if (zeiger->subjectline[0])
   {
      WinSetDlgItemText(hwndClient, IDE_SUBJTEXT, zeiger->subjectline);
      SetFocusControl(hwndClient, IDML_MAINEDIT);
   }
   else
      SetFocusControl(hwndClient, IDE_SUBJTEXT);

   /* Attribute */
   CurrentHeader.ulAttrib |= zeiger->ulAttrib;
   DisplayAttrib(CurrentHeader.ulAttrib);

   /* erste Zeile vorbereiten */

   if (zeiger->firstline[0])
   {
      char pchBuffer[1000];
      char *pchSrc, *pchDest;
      long ipt=0;

      pchDest=pchBuffer;
      pchSrc=zeiger->firstline;

      while(*pchSrc)
      {
         switch(*pchSrc)
         {
            case '%':
               pchSrc++;
               if (*pchSrc == '?')
               {
                  PROMPTPAR PromptPar;
                  char *pchDest2;

                  PromptPar.cb=sizeof(PROMPTPAR);
                  pchSrc++;

                  if (*pchSrc=='"')
                  {
                     pchSrc++;
                     pchDest2=PromptPar.pchTitle;

                     while(*pchSrc && *pchSrc!='"')
                        *pchDest2++ = *pchSrc++;
                     *pchDest2='\0';
                     if (*pchSrc)
                        pchSrc++;
                  }
                  else
                     PromptPar.pchTitle[0]='\0';

                  PromptPar.pchLine[0]='\0';
                  if (WinDlgBox(HWND_DESKTOP, hwndClient, PromptProc, hmodLang,
                                IDD_NICKPROMPT, &PromptPar)==DID_OK)
                  {
                     char *pchTemp=PromptPar.pchLine;

                     while (*pchTemp)
                        *pchDest++=*pchTemp++;
                  }
               }
               else
               {
                  *pchDest++='%';
                  *pchDest++ = *pchSrc++;
               }
               break;

            default:
               *pchDest++ = *pchSrc++;
               break;
         }
      }
      *pchDest++='\n';
      *pchDest++=' ';
      *pchDest++='\n';
      *pchDest='\0';

      /* Einfuegen */
      WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_FORMAT,
                        MPFROMSHORT(MLFIE_NOTRANS), NULL);
      WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_SETIMPORTEXPORT,
                        pchBuffer, MPFROMLONG(strlen(pchBuffer)));
      WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_IMPORT,
                        &ipt, MPFROMLONG(strlen(pchBuffer)));
      iptInitialPos2+=strlen(pchBuffer);
   }

   return TRUE;
}

/*------------------------------ PromptProc     -----------------------------*/
/* Fensterprozedur f. Nickname-Prompt                                        */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY PromptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND frame, hwndhelp;
   static PROMPTPAR *PromptPar;

   switch(message)
   {
      case WM_INITDLG:
         PromptPar=(PROMPTPAR*) mp2;
         WinAssociateHelpInstance(hwndhelp, parent);
         WinSetDlgItemText(parent, IDD_NICKPROMPT+2, PromptPar->pchTitle);
         WinSendDlgItemMsg(parent, IDD_NICKPROMPT+3, EM_SETTEXTLIMIT,
                           MPFROMLONG(LEN_FIRSTLINE), NULL);
         RestoreWinPos(parent, &windowpositions.promptpos, FALSE, TRUE);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)==DID_OK)
            WinQueryDlgItemText(parent, IDD_NICKPROMPT+3,
                                LEN_FIRSTLINE+1, PromptPar->pchLine);
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         WinAssociateHelpInstance(hwndhelp, frame);
         QueryWinPos(parent, &windowpositions.promptpos);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*-------------------------------- Modulende --------------------------------*/


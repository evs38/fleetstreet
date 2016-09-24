/*---------------------------------------------------------------------------+
 | Titel: SETUPDLG.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 01.08.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |       Setup-Dialoge von FleetStreet                                       |
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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogs.h"
#include "dialogids.h"
#include "init.h"
#include "setupdlg.h"
#include "utility.h"
#include "controls\editwin.h"
#include "controls\msgviewer.h"
#include "controls\clrsel.h"
#include "util\fltutil.h"
#include "util\addrcnv.h"
#include "cfgfile_interface.h"

/*--------------------------------- Defines ---------------------------------*/

#define TAB_FONT         "8.Helv"
#define RGB_GREY         0x00cccccc
#define NUM_PAGES_SETUP  17

typedef struct _domainentrypar
{
  USHORT cb;
  PDOMAINS pDomain;
} DOMAINENTRYPAR;

/*---------------------------- Globale Variablen ----------------------------*/

extern HMODULE hmodLang;
extern HAB anchor;

PFNWP OldContainerProc;

/*--------------------------- Funktionsprototypen ---------------------------*/

static void UpdateAdrList(HWND parent);
static void UpdateNameList(HWND parent);
static void InsertNotebookPages(HWND notebook, NBPAGE *Table);
static MRESULT EXPENTRY DomainEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY ModemTypesProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY AddressesProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY DefOriginProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY MacrosProc2(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY MacrosProc3(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY DomainsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY StartupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY EditorOptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY UserProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY MacrosProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY MsgOptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY SquishOptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TosserPathsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY NewUserProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY NewAddressProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY RemapSetupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY SafetyOptProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY OpenWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void UpdateDomains(HWND hwnd);
static ULONG GetCfgType(HWND hwndDlg);
static void SetCfgType(HWND hwndDlg, ULONG ulCfgType);
static int ReadNewCfgFile(HWND parent, HWND hwndSetupFrame, char *pchPathName);
static void RefreshModemTypeList(HWND hwndDlg, ULONG ulIdCnr, NODELISTOPT *pNodelist);
static void CheckOpenWinButton(HWND hwndDlg, ULONG ulID, POPENWIN pOpenWindows, ULONG ulMask);
static void QueryOpenWinButton(HWND hwndDlg, ULONG ulID, POPENWIN pOpenWindows, ULONG ulMask);

/*------------------------------ OptionsProc   ------------------------------*/
/* Hauptdialog-Prozedur Optionen                                             */
/* Initialisierung des Notebooks                                             */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY OptionsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp, frame;

   static NBPAGE PageTable[NUM_PAGES_SETUP];
   static HWND notebook=NULLHANDLE;
   int i;
   MRESULT resultbuf=0;

   switch(message)
   {
      case WM_INITDLG:
         /* Leere Seitentabelle */
         memset(PageTable, 0, sizeof(PageTable));
         notebook=WinWindowFromID(parent, IDD_SETUPOPTIONS+1);

         SetNotebookParams(notebook, 110);

         /* Leere Seiten einfuegen */
         InsertNotebookPages(notebook, PageTable);
         /* erste Seite gleich anzeigen */
         LoadPage(notebook, &(PageTable[0]), NULL);

         RestoreWinPos(parent, &windowpositions.optionspos, TRUE, TRUE);
         WinAssociateHelpInstance(hwndhelp, parent);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, parent, IDD_SETUPOPTIONS+1);
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(parent,message,mp1,mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=255;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=190;

         return resultbuf;

      case WM_CLOSE:
         QueryWinPos(parent, &windowpositions.optionspos);
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_SETUPOPTIONS+1)
            if (SHORT2FROMMP(mp1)==BKN_PAGESELECTED)
            {
               /* Seitenwechsel */
               int i=0;

               /* alte Seite in Seiten-Tabelle suchen */
               while (i<NUM_PAGES_SETUP)
               {
                  if (PageTable[i].PageID == ((PPAGESELECTNOTIFY)mp2)->ulPageIdCur)
                     break;
                  else
                     i++;
               }
               if (i<NUM_PAGES_SETUP && PageTable[i].resID==IDD_USERNAMES)
                  SendMsg(PageTable[i].hwndPage, SUM_CHECKINPUT, NULL, NULL);

               /* neue Seite in Seiten-Tabelle suchen */
               i=0;
               while (i<NUM_PAGES_SETUP)
               {
                  if (PageTable[i].PageID == ((PPAGESELECTNOTIFY)mp2)->ulPageIdNew)
                     break;
                  else
                     i++;
               }

               /* Seite ggf. Laden */
               if (i<NUM_PAGES_SETUP)
               {
                  if (PageTable[i].hwndPage==NULLHANDLE)
                     LoadPage(notebook, &(PageTable[i]), NULL);
                  if (PageTable[i].resID==IDD_SQUISHOPTIONS)
                     SendMsg(PageTable[i].hwndPage, SUM_CHECKSETUP, NULL, NULL);
               }
            }
         break;

      case SUM_REFRESHUSER:
         /* Message an die Dialogseiten weiterreichen */
         i=0;
         while (i<NUM_PAGES_SETUP)
         {
          if ((PageTable[i].resID == IDD_ADDRESSES ||
               PageTable[i].resID == IDD_USERNAMES ||
               PageTable[i].resID == IDD_DEFAULTORIGIN) &&
              PageTable[i].hwndPage)
            SendMsg(PageTable[i].hwndPage, SUM_REFRESHUSER, NULL, NULL);
            i++;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*---------------------------- InsertNotebookPages --------------------------*/
/* Fuegt alle Seiten in das Notebook ein, Einstellung der Darstellungs-      */
/* Parameter des Notebooks                                                   */
/*---------------------------------------------------------------------------*/

static const struct pagedef
{
   ULONG ulStringID;
   ULONG resID;
   PFNWP DlgProc;
} PageDef[] =
{
   { IDST_TAB_USER,        IDD_USERNAMES,       UserProc},
   { IDST_TAB_ADDRESSES,   IDD_ADDRESSES,       AddressesProc},
   { IDST_TAB_DEFORIGIN,   IDD_DEFAULTORIGIN,   DefOriginProc},
   { IDST_TAB_SQUISHOPT,   IDD_SQUISHOPTIONS,   SquishOptProc},
   { IDST_TAB_TOSSERPATHS, IDD_TOSSERPATHS,     TosserPathsProc},
   { IDST_TAB_MACROS,      IDD_MACROS1,         MacrosProc},
   { IDST_TAB_MACROS2,     IDD_MACROS2,         MacrosProc2},
   { IDST_TAB_MACROS3,     IDD_MACROS3,         MacrosProc3},
   { IDST_TAB_DOMAINS,     IDD_DOMAINS,         DomainsProc},
   { IDST_TAB_MODEMTYPES,  IDD_MODEMTYPES,      ModemTypesProc},
   { IDST_TAB_MSGOPT,      IDD_MSGOPT,          MsgOptProc},
   { IDST_TAB_STARTUP,     IDD_STARTUP,         StartupProc},
   { IDST_TAB_EDITOROPT,   IDD_EDITOROPT,       EditorOptProc},
   { IDST_TAB_SAFETY,      IDD_SAFETY,          SafetyOptProc},
   { IDST_TAB_SU_OPENWIN,  IDD_SU_OPENWIN,      OpenWinProc},
   { IDST_TAB_DRIVEREMAP,  IDD_DRIVEREMAP,      RemapSetupProc}
};

#define NUM_INS_PAGES (sizeof(PageDef)/sizeof(PageDef[0]))

static void InsertNotebookPages(HWND notebook, NBPAGE *Table)
{
   int i;

   /* Leere Seiten einfuegen, Tabelle fuellen */
   for (i=0; i < NUM_INS_PAGES; i++)
   {
      InsertEmptyPage(notebook, PageDef[i].ulStringID, &(Table[i]));
      Table[i].resID=PageDef[i].resID;
      Table[i].DlgProc=PageDef[i].DlgProc;
   }

   return;
}

/*------------------------------- InsertEmptyPage ---------------------------*/
/* Fuegt eine leere Seite in das Notebook ein                                */
/*---------------------------------------------------------------------------*/

void InsertEmptyPage(HWND notebook, ULONG stringID, NBPAGE *Page)
{
   extern HAB anchor;
   UCHAR tabtext[50];

   Page->PageID=(ULONG)SendMsg(notebook, BKM_INSERTPAGE, NULL,
                            MPFROM2SHORT(BKA_AUTOPAGESIZE | BKA_MAJOR,
                            BKA_LAST));

   LoadString(stringID, sizeof(tabtext), tabtext);

   SendMsg(notebook,
              BKM_SETTABTEXT,
              MPFROMLONG(Page->PageID),
              (MPARAM) tabtext);

   return;
}

/*---------------------------------- LoadPage -------------------------------*/
/* Laedt eine Seite aus den Ressourcen, fuegt in Notebook ein                */
/*---------------------------------------------------------------------------*/

void LoadPage(HWND notebook, NBPAGE *Page, PVOID pCreateParam)
{
   Page->hwndPage=WinLoadDlg(WinQueryWindow(notebook, QW_PARENT),
                             notebook,
                             Page->DlgProc,
                             hmodLang,
                             Page->resID,
                             pCreateParam);

   SendMsg(notebook,
              BKM_SETPAGEWINDOWHWND,
              MPFROMLONG(Page->PageID),
              MPFROMHWND(Page->hwndPage));

   return;
}

/*--------------------------------  UserProc --------------------------------*/
/* Dialog-Prozedur des User-Dialogs                                          */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY UserProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   STRINGPAR StringPar;
   extern USERDATAOPT userdaten;
   extern DIRTYFLAGS dirtyflags;
   extern GENERALOPT generaloptions;
   int i;
   USHORT usSelect;

   switch(message)
   {
      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            switch (SHORT1FROMMP(mp1))
            {
               /* Neuer Name */
               case IDD_USERNAMES+3:
                  StringPar.cb=sizeof(STRINGPAR);
                  StringPar.pchString=malloc(LEN_USERNAME+1);
                  StringPar.pchString[0]='\0';
                  if (WinDlgBox(HWND_DESKTOP,
                                parent,
                                NewUserProc,
                                hmodLang,
                                IDD_NEWUSER,
                                &StringPar)==DID_OK)
                  {
                     i=0;
                     while (i<MAX_USERNAMES &&
                            strcmp(userdaten.username[i], StringPar.pchString))
                        i++;
                     if (i<MAX_USERNAMES)
                     {
                        /* Name gibt es schon */
                        WinAlarm(HWND_DESKTOP, WA_ERROR);
                     }
                     else
                     {
                        i=0;
                        while(i<MAX_USERNAMES && userdaten.username[i][0])
                           i++;
                        if (i==MAX_USERNAMES)
                           /* kann nicht vorkommen */
                           WinAlarm(HWND_DESKTOP, WA_ERROR);
                        else
                        {
                           strcpy(userdaten.username[i], StringPar.pchString);
                           UpdateNameList(parent);
                           dirtyflags.userdirty=TRUE;
                        }
                     }
                  }
                  free(StringPar.pchString);
                  break;

               /* Name aendern */
               case IDD_USERNAMES+4:
                  StringPar.cb=sizeof(STRINGPAR);
                  StringPar.pchString=malloc(LEN_USERNAME+1);
                  usSelect=(USHORT)WinSendDlgItemMsg(parent, IDD_USERNAMES+2, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
                  if (usSelect != LIT_NONE)
                  {
                     strcpy(StringPar.pchString, userdaten.username[usSelect]);
                     if (WinDlgBox(HWND_DESKTOP,
                                   parent,
                                   NewUserProc,
                                   hmodLang,
                                   IDD_NEWUSER,
                                   &StringPar)==DID_OK)
                     {
                        i=0;
                        while ((i==usSelect) ||
                               (i<MAX_USERNAMES && strcmp(userdaten.username[i], StringPar.pchString)))
                           i++;
                        if (i<MAX_USERNAMES)
                        {
                           /* Name gibt es schon */
                           WinAlarm(HWND_DESKTOP, WA_ERROR);
                        }
                        else
                        {
                           strcpy(userdaten.username[usSelect], StringPar.pchString);
                           dirtyflags.userdirty=TRUE;
                           UpdateNameList(parent);
                        }
                     }
                  }
                  free(StringPar.pchString);
                  break;

               /* Name loeschen */
               case IDD_USERNAMES+5:
                  usSelect=(USHORT)WinSendDlgItemMsg(parent, IDD_USERNAMES+2, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
                  if (usSelect != LIT_NONE)
                  {
                     if (generaloptions.safety & SAFETY_CHANGESETUP)
                     {
                        if (MessageBox(parent, IDST_MSG_DELUNAME, IDST_TITLE_DELUNAME,
                                       IDD_DELUNAME, MB_YESNO | MB_ICONQUESTION)==MBID_NO)
                           break;
                     }
                     /* Name loeschen */
                     if (usSelect==MAX_USERNAMES-1)
                        userdaten.username[usSelect][0]='\0';
                     else
                     {
                        for (i=usSelect+1; i<MAX_USERNAMES && userdaten.username[i][0]; i++)
                           strcpy(userdaten.username[i-1], userdaten.username[i]);
                        userdaten.username[i-1][0]='\0';
                     }
                     dirtyflags.userdirty=TRUE;
                     UpdateNameList(parent);
                  }
                  break;

               /* Name default */
               case IDD_USERNAMES+6:
                  usSelect=(USHORT)WinSendDlgItemMsg(parent, IDD_USERNAMES+2, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
                  if (usSelect != LIT_NONE && usSelect != 0)
                  {
                     char buffer[LEN_USERNAME+1];

                     strcpy(buffer, userdaten.username[0]);
                     strcpy(userdaten.username[0], userdaten.username[usSelect]);
                     strcpy(userdaten.username[usSelect], buffer);
                     dirtyflags.userdirty=TRUE;
                     UpdateNameList(parent);
                  }
                  break;

               default:
                  break;
            }
         }
         return (MRESULT) FALSE;

      case WM_INITDLG:
      case SUM_REFRESHUSER:
         UpdateNameList(parent);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}
/*-----------------------------  UpdateNameList -----------------------------*/
/* Usernamenliste neu fuellen                                                */
/*---------------------------------------------------------------------------*/

static void UpdateNameList(HWND parent)
{
   HWND hwndListe=NULLHANDLE;
   extern USERDATAOPT userdaten;
   int i;

   hwndListe=WinWindowFromID(parent, IDD_USERNAMES+2);

   SendMsg(hwndListe, LM_DELETEALL, NULL, NULL);
   /* Namens-Liste auffuellen */
   for (i=0; i<MAX_USERNAMES && userdaten.username[i][0]; i++)
      SendMsg(hwndListe, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                 (MPARAM) userdaten.username[i]);
   SendMsg(hwndListe, LM_SELECTITEM, (MPARAM) 0, (MPARAM) TRUE);

   /* Keine Namen */
   if (!userdaten.username[0][0])
   {
      WinEnableControl(parent, IDD_USERNAMES+4, FALSE);
      WinEnableControl(parent, IDD_USERNAMES+5, FALSE);
      WinEnableControl(parent, IDD_USERNAMES+6, FALSE);
   }
   else
   {
      WinEnableControl(parent, IDD_USERNAMES+4, TRUE);
      WinEnableControl(parent, IDD_USERNAMES+5, TRUE);
      WinEnableControl(parent, IDD_USERNAMES+6, TRUE);
   }

   /* Namensliste voll */
   if (userdaten.username[MAX_USERNAMES-1][0])
      WinEnableControl(parent, IDD_USERNAMES+3, FALSE);
   else
      WinEnableControl(parent, IDD_USERNAMES+3, TRUE);
    return;
}

/*-----------------------------  NewUserProc  -------------------------------*/
/* Dialog-Prozedur des Neuer-User-Dialogs                                    */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewUserProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp;

   static char *EditText=NULL;

   switch(message)
   {
      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
            if (SHORT1FROMMP(mp1)==DID_OK)
               WinQueryDlgItemText(parent, IDD_NEWUSER+2,
                                   LEN_USERNAME+1,
                                   EditText);
         break;

      case WM_INITDLG:
         WinAssociateHelpInstance(hwndhelp, parent);
         WinSendDlgItemMsg(parent,IDD_NEWUSER+2,
                           EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_USERNAME),
                           (MPARAM) NULL);
         if (mp2) /* Pointer auf String f. Eingabefeld */
         {
            EditText= ((STRINGPAR *) mp2)->pchString;
            WinSetDlgItemText(parent, IDD_NEWUSER+2,
                              EditText);
            if (EditText[0])
               WinEnableControl(parent, DID_OK, TRUE);
            else
               WinEnableControl(parent, DID_OK, FALSE);
         }
         RestoreWinPos(parent, &(windowpositions.newuserpos), FALSE, TRUE);
         break;

      case WM_CHAR:
         WinEnableControl(parent, DID_OK,
                          WinQueryDlgItemTextLength(parent, IDD_NEWUSER+2));
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         QueryWinPos(parent, &(windowpositions.newuserpos));
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}


/*-------------------------------AddressesProc-------------------------------*/
/* Dialog-Prozedur des Adressen-Dialogs                                      */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY AddressesProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   STRINGPAR StringPar;
   extern USERDATAOPT userdaten;
   extern DIRTYFLAGS dirtyflags;
   extern GENERALOPT generaloptions;
   int i;
   USHORT usSelect;

   switch(message)
   {
      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            switch (SHORT1FROMMP(mp1))
            {
               /* neue Adresse */
               case IDD_ADDRESSES+3:
                  StringPar.cb=sizeof(STRINGPAR);
                  StringPar.pchString=malloc(LEN_5DADDRESS+1);
                  StringPar.pchString[0]='\0';
                  if (WinDlgBox(HWND_DESKTOP,
                                parent,
                                NewAddressProc,
                                hmodLang,
                                IDD_NEWADDRESS,
                                &StringPar)==DID_OK)
                  {
                     i=0;
                     while (i<MAX_ADDRESSES &&
                            strcmp(userdaten.address[i], StringPar.pchString))
                        i++;
                     if (i<MAX_ADDRESSES)
                     {
                        /* Adresse gibt es schon */
                        WinAlarm(HWND_DESKTOP, WA_ERROR);
                     }
                     else
                     {
                        i=0;
                        while(i<MAX_ADDRESSES && userdaten.address[i][0])
                           i++;
                        if (i==MAX_ADDRESSES)
                           /* kann nicht vorkommen */
                           WinAlarm(HWND_DESKTOP, WA_ERROR);
                        else
                        {
                           strcpy(userdaten.address[i], StringPar.pchString);
                           UpdateAdrList(parent);
                           dirtyflags.userdirty=TRUE;
                        }
                     }
                  }
                  free(StringPar.pchString);
                  break;

               /* Adresse aendern */
               case IDD_ADDRESSES+4:
                  StringPar.cb=sizeof(STRINGPAR);
                  StringPar.pchString=malloc(LEN_5DADDRESS+1);
                  usSelect=(USHORT)WinSendDlgItemMsg(parent, IDD_ADDRESSES+2, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
                  if (usSelect != LIT_NONE)
                  {
                     strcpy(StringPar.pchString, userdaten.address[usSelect]);
                     if (WinDlgBox(HWND_DESKTOP,
                                   parent,
                                   NewAddressProc,
                                   hmodLang,
                                   IDD_NEWADDRESS,
                                   &StringPar)==DID_OK)
                     {
                        i=0;
                        while ((i==usSelect) ||
                               (i<MAX_ADDRESSES && strcmp(userdaten.address[i], StringPar.pchString)))
                           i++;
                        if (i<MAX_ADDRESSES)
                        {
                           /* Adresse gibt es schon */
                           WinAlarm(HWND_DESKTOP, WA_ERROR);
                        }
                        else
                        {
                           strcpy(userdaten.address[usSelect], StringPar.pchString);
                           dirtyflags.userdirty=TRUE;
                           UpdateAdrList(parent);
                        }
                     }
                  }
                  free(StringPar.pchString);
                  break;

               /* Adresse loeschen */
               case IDD_ADDRESSES+5:
                  usSelect=(USHORT)WinSendDlgItemMsg(parent, IDD_ADDRESSES+2, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
                  if (usSelect != LIT_NONE)
                  {
                     if (generaloptions.safety & SAFETY_CHANGESETUP)
                     {
                        if (MessageBox(parent, IDST_MSG_DELADDRESS, IDST_TITLE_DELADDRESS,
                                       IDD_DELADDRESS, MB_YESNO | MB_ICONQUESTION)==MBID_NO)
                           break;
                     }
                     /* Adresse loeschen */
                     if (usSelect==MAX_ADDRESSES-1)
                        userdaten.address[usSelect][0]='\0';
                     else
                     {
                        for (i=usSelect+1; i<MAX_ADDRESSES && userdaten.address[i][0]; i++)
                           strcpy(userdaten.address[i-1], userdaten.address[i]);
                        userdaten.address[i-1][0]='\0';
                     }
                     dirtyflags.userdirty=TRUE;
                     UpdateAdrList(parent);
                  }
                  break;

               /* Adresse default */
               case IDD_ADDRESSES+6:
                  usSelect=(USHORT)WinSendDlgItemMsg(parent, IDD_ADDRESSES+2, LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST), NULL);
                  if (usSelect != LIT_NONE && usSelect != 0)
                  {
                     char buffer[LEN_5DADDRESS+1];

                     strcpy(buffer, userdaten.address[0]);
                     strcpy(userdaten.address[0], userdaten.address[usSelect]);
                     strcpy(userdaten.address[usSelect], buffer);
                     dirtyflags.userdirty=TRUE;
                     UpdateAdrList(parent);
                  }
                  break;

               default:
                  break;
            }
         }
         return (MRESULT) FALSE;

      case WM_INITDLG:
      case SUM_REFRESHUSER:
         UpdateAdrList(parent);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}
/*------------------------------ UpdateAdrList ------------------------------*/
/* Adressliste   neu fuellen                                                 */
/*---------------------------------------------------------------------------*/

static void UpdateAdrList(HWND parent)
{
   extern USERDATAOPT userdaten;
   HWND hwndList=NULLHANDLE;
   int i;

   hwndList=WinWindowFromID(parent, IDD_ADDRESSES+2);
   SendMsg(hwndList, LM_DELETEALL, NULL, NULL);
   /* Address-Liste auffuellen */
   for (i=0; i<MAX_ADDRESSES && userdaten.address[i][0]; i++)
      SendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                 (MPARAM) userdaten.address[i]);
   SendMsg(hwndList, LM_SELECTITEM, (MPARAM) 0, (MPARAM) TRUE);

   /* Keine Adressen */
   if (!userdaten.address[0][0])
   {
      WinEnableControl(parent, IDD_ADDRESSES+4, FALSE);
      WinEnableControl(parent, IDD_ADDRESSES+5, FALSE);
      WinEnableControl(parent, IDD_ADDRESSES+6, FALSE);
   }
   else
   {
      WinEnableControl(parent, IDD_ADDRESSES+4, TRUE);
      WinEnableControl(parent, IDD_ADDRESSES+5, TRUE);
      WinEnableControl(parent, IDD_ADDRESSES+6, TRUE);
   }

   /* Adressliste voll */
   if (userdaten.address[MAX_ADDRESSES-1][0])
      WinEnableControl(parent, IDD_ADDRESSES+3, FALSE);
   else
      WinEnableControl(parent, IDD_ADDRESSES+3, TRUE);
   return;
}

/*----------------------------- NewAddressProc ------------------------------*/
/* Dialog-Prozedur des Neue-Adresse-Dialogs                                  */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewAddressProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp;

   static char *EditText=NULL;

   switch(message)
   {
      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
            if (SHORT1FROMMP(mp1)==DID_OK)
               WinQueryDlgItemText(parent, IDD_NEWADDRESS+2,
                                   LEN_5DADDRESS+1,
                                   EditText);
         break;

      case WM_INITDLG:
         WinAssociateHelpInstance(hwndhelp, parent);
         WinSendDlgItemMsg(parent, IDD_NEWADDRESS+2,
                           EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_5DADDRESS),
                           (MPARAM) NULL);
         if (mp2) /* Pointer auf String f. Eingabefeld */
         {
            EditText= ((STRINGPAR *) mp2)->pchString;
            WinSetDlgItemText(parent, IDD_NEWADDRESS+2,
                              EditText);
            if (EditText[0])
               WinEnableControl(parent, DID_OK, TRUE);
            else
               WinEnableControl(parent, DID_OK, FALSE);
         }
         RestoreWinPos(parent, &windowpositions.newaddresspos, FALSE, TRUE);
         break;

      case WM_CHAR:
         WinEnableControl(parent, DID_OK,
                          WinQueryDlgItemTextLength(parent, IDD_NEWADDRESS+2));
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_NEWADDRESS+2)
            if (SHORT2FROMMP(mp1)==EN_KILLFOCUS)
            {
               FTNADDRESS NetAddr;
               char pchTemp[LEN_5DADDRESS+1];

               WinQueryDlgItemText(parent, SHORT1FROMMP(mp1),
                                   LEN_5DADDRESS+1, pchTemp);
               StringToNetAddr(pchTemp, &NetAddr, NULL);
               NetAddrToString(pchTemp, &NetAddr);
               WinSetDlgItemText(parent, SHORT1FROMMP(mp1), pchTemp);
            }
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         QueryWinPos(parent, &windowpositions.newaddresspos);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*----------------------------- DefOriginProc -------------------------------*/
/* Dialog-Prozedur des Default-Origin-Dialogs                                */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY DefOriginProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern USERDATAOPT userdaten;
   extern DIRTYFLAGS dirtyflags;
   char pchOBuffer[LEN_ORIGIN+1];

   switch(message)
   {
      case WM_INITDLG:
      case SUM_REFRESHUSER:
         WinSendDlgItemMsg(parent, IDD_DEFAULTORIGIN+2,
                           EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_ORIGIN),
                           (MPARAM) NULL);
         WinSetDlgItemText(parent, IDD_DEFAULTORIGIN+2,
                           userdaten.defaultorigin);
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         WinQueryDlgItemText(parent, IDD_DEFAULTORIGIN+2, LEN_ORIGIN+1, pchOBuffer);
         if (strcmp(pchOBuffer, userdaten.defaultorigin))
         {
            strcpy(userdaten.defaultorigin, pchOBuffer);
            dirtyflags.userdirty=TRUE;
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*-----------------------------  MacrosProc  --------------------------------*/
/* Dialog-Prozedur des Macro-Dialogs #1                                      */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY MacrosProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   int i;
   extern MACROTABLEOPT macrotable;
   extern DIRTYFLAGS dirtyflags;

   switch(message)
   {
      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_INITDLG:
         /* Feldlaengen */
         for (i=3; i<=6; i++)
         {
            WinSendDlgItemMsg(parent, IDD_MACROS1+i,
                              MLM_SETTEXTLIMIT,
                              MPFROMSHORT(LEN_MACRO),
                              (MPARAM) NULL);
            WinSetDlgItemText(parent, IDD_MACROS1+i,
                              macrotable.macrotext[i-3]);
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         for (i=3; i<=6; i++)
         {
            char pchTemp[LEN_MACRO+1];

            WinQueryDlgItemText(parent, IDD_MACROS1+i,
                                LEN_MACRO+1, pchTemp);
            if (strcmp(pchTemp, macrotable.macrotext[i-3]))
            {
               strcpy(macrotable.macrotext[i-3], pchTemp);
               dirtyflags.macrosdirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*-----------------------------  MacrosProc2 --------------------------------*/
/* Dialog-Prozedur des Macro-Dialogs #2                                      */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY MacrosProc2(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   int i;
   extern MACROTABLEOPT macrotable;
   extern DIRTYFLAGS dirtyflags;

   switch(message)
   {
      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_INITDLG:
         /* Feldlaengen */
         for (i=3; i<=6; i++)
         {
            WinSendDlgItemMsg(parent, IDD_MACROS2+i,
                              MLM_SETTEXTLIMIT,
                              MPFROMSHORT(LEN_MACRO),
                              (MPARAM) NULL);
            WinSetDlgItemText(parent, IDD_MACROS2+i,
                              macrotable.macrotext[i+1]);
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         for (i=3; i<=6; i++)
         {
            char pchTemp[LEN_MACRO+1];

            WinQueryDlgItemText(parent, IDD_MACROS2+i,
                                LEN_MACRO+1, pchTemp);
            if (strcmp(pchTemp, macrotable.macrotext[i+1]))
            {
               strcpy(macrotable.macrotext[i+1], pchTemp);
               dirtyflags.macrosdirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*-----------------------------  MacrosProc3 --------------------------------*/
/* Dialog-Prozedur des Macro-Dialogs #3                                      */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY MacrosProc3(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   int i;
   extern MACROTABLEOPT macrotable;
   extern DIRTYFLAGS dirtyflags;

   switch(message)
   {
      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_INITDLG:
         /* Feldlaengen */
         for (i=3; i<=5; i++)
         {
            WinSendDlgItemMsg(parent, IDD_MACROS3+i,
                              MLM_SETTEXTLIMIT,
                              MPFROMSHORT(LEN_MACRO),
                              (MPARAM) NULL);
            WinSetDlgItemText(parent, IDD_MACROS3+i,
                              macrotable.macrotext[i+5]);
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         for (i=3; i<=5; i++)
         {
            char pchTemp[LEN_MACRO+1];

            WinQueryDlgItemText(parent, IDD_MACROS3+i,
                                LEN_MACRO+1, pchTemp);
            if (strcmp(pchTemp, macrotable.macrotext[i+5]))
            {
               strcpy(macrotable.macrotext[i+5], pchTemp);
               dirtyflags.macrosdirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*-------------------------- DomainsProc ------------------------------------*/
/* Domain-Setup                                                              */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY DomainsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern PDOMAINS domains;
   extern DIRTYFLAGS dirtyflags;
   PDOMAINS pDomains, pTemp;
   DOMAINENTRYPAR DomainEntryPar;
   extern GENERALOPT generaloptions;
   SHORT sItem;
   char pchTemp[LEN_DOMAIN+1];

   switch(message)
   {
      case WM_INITDLG:
         /* Listbox mit Domains fuellen */
         UpdateDomains(parent);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            switch (SHORT1FROMMP(mp1))
            {
               /* Change-Button */
               case IDD_DOMAINS+5:
                  sItem=(SHORT)WinSendDlgItemMsg(parent, IDD_DOMAINS+3, LM_QUERYSELECTION,
                                          MPFROMSHORT(LIT_FIRST), NULL);
                  WinSendDlgItemMsg(parent, IDD_DOMAINS+3, LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sItem, LEN_DOMAIN+1), pchTemp);
                  DomainEntryPar.cb=sizeof(DOMAINENTRYPAR);
                  DomainEntryPar.pDomain=QueryDomain(domains, pchTemp);
                  if (WinDlgBox(HWND_DESKTOP, parent, DomainEntryProc,
                                hmodLang, IDD_DOMAINENTRY, &DomainEntryPar)==DID_OK)
                  {
                     UpdateDomains(parent);
                     dirtyflags.domainsdirty=TRUE;
                  }
                  break;

               /* Add-Button */
               case IDD_DOMAINS+4:
                  pTemp=malloc(sizeof(DOMAINS));
                  memset(pTemp, 0, sizeof(DOMAINS));
                  DomainEntryPar.cb=sizeof(DOMAINENTRYPAR);
                  DomainEntryPar.pDomain=pTemp;
                  if (WinDlgBox(HWND_DESKTOP, parent, DomainEntryProc,
                                hmodLang, IDD_DOMAINENTRY, &DomainEntryPar)==DID_OK)
                  {
                     pDomains=domains;
                     domains=pTemp;
                     pTemp->next=pDomains;
                     UpdateDomains(parent);
                     dirtyflags.domainsdirty=TRUE;
                  }
                  else
                     free(pTemp);
                  break;

               /* Delete-Button */
               case IDD_DOMAINS+6:
                  if (generaloptions.safety )
                  {
                     if (MessageBox(parent, IDST_MSG_DELDOMAIN, IDST_TITLE_DELDOMAIN,
                                    IDD_DELDOMAIN, MB_YESNO | MB_QUERY | MB_DEFBUTTON2)!=MBID_YES)
                        break;
                  }
                  sItem=(SHORT)WinSendDlgItemMsg(parent, IDD_DOMAINS+3, LM_QUERYSELECTION,
                                          MPFROMSHORT(LIT_FIRST), NULL);
                  WinSendDlgItemMsg(parent, IDD_DOMAINS+3, LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sItem, LEN_DOMAIN+1), pchTemp);
                  pTemp=domains;
                  pDomains=domains;
                  while(pTemp && stricmp(pchTemp, pTemp->domainname))
                  {
                     pDomains=pTemp;
                     pTemp=pTemp->next;
                  }
                  if (pTemp)
                  {
                     if (pTemp != domains)
                        pDomains->next=pTemp->next;
                     else
                        domains=pTemp->next;
                     free(pTemp);
                     UpdateDomains(parent);
                     dirtyflags.domainsdirty=TRUE;
                  }
                  else
                     WinAlarm(HWND_DESKTOP, WA_ERROR);
                  break;

               default:
                  break;
            }
         }
         return (MRESULT) FALSE;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_DOMAINS+3)
            if (SHORT2FROMMP(mp1)==LN_ENTER)
               SendMsg(parent, WM_COMMAND, MPFROMSHORT(IDD_DOMAINS+5),
                          MPFROMSHORT(CMDSRC_PUSHBUTTON));
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}


/*-------------------------- DomainEntryProc --------------------------------*/
/* Eingabe-Prozedur f. Domain-Eintrag                                        */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY DomainEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;
   extern PDOMAINS domains;
   extern WINDOWPOSITIONS windowpositions;
   char pchTemp[LEN_DOMAIN+1];
   static DOMAINENTRYPAR *DomainEntryPar;
   PDOMAINS pTemp;

   switch(message)
   {
      case WM_INITDLG:
         DomainEntryPar=(DOMAINENTRYPAR *) mp2;
         WinAssociateHelpInstance(hwndhelp, parent);
         WinSubclassWindow(WinWindowFromID(parent, IDD_DOMAINENTRY+4),
                           FileEntryProc);
         WinSubclassWindow(WinWindowFromID(parent, IDD_DOMAINENTRY+6),
                           FileEntryProc);
         /* Feldlaengen */
         WinSendDlgItemMsg(parent, IDD_DOMAINENTRY+2,
                           EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_DOMAIN),
                           NULL);
         WinSendDlgItemMsg(parent, IDD_DOMAINENTRY+4,
                           EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME),
                           NULL);
         WinSendDlgItemMsg(parent, IDD_DOMAINENTRY+6,
                           EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME),
                           NULL);

         WinSetDlgItemText(parent, IDD_DOMAINENTRY+2, DomainEntryPar->pDomain->domainname);
         WinSetDlgItemText(parent, IDD_DOMAINENTRY+4, DomainEntryPar->pDomain->indexfile);
         WinSetDlgItemText(parent, IDD_DOMAINENTRY+6, DomainEntryPar->pDomain->nodelistfile);
         if (DomainEntryPar->pDomain->domainname[0])
            WinEnableControl(parent, DID_OK, TRUE);
         RestoreWinPos(parent, &windowpositions.domainentrypos, FALSE, TRUE);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_DOMAINENTRY+2)
            if (SHORT2FROMMP(mp1)==EN_CHANGE)
               if (WinQueryDlgItemTextLength(parent, IDD_DOMAINENTRY+2))
                  WinEnableControl(parent, DID_OK, TRUE);
               else
                  WinEnableControl(parent, DID_OK, FALSE);
         break;


      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)==DID_OK)
         {
            WinQueryDlgItemText(parent, IDD_DOMAINENTRY+2, LEN_DOMAIN+1, pchTemp);
            pTemp=QueryDomain(domains, pchTemp);
            if (!pTemp || (pTemp == DomainEntryPar->pDomain))
            {
               strcpy(DomainEntryPar->pDomain->domainname, pchTemp);
               WinQueryDlgItemText(parent, IDD_DOMAINENTRY+4, LEN_PATHNAME+1,
                                   DomainEntryPar->pDomain->indexfile);
               WinQueryDlgItemText(parent, IDD_DOMAINENTRY+6, LEN_PATHNAME+1,
                                   DomainEntryPar->pDomain->nodelistfile);
            }
            else
            {
               MessageBox(parent, IDST_MSG_HAVEDOMAIN, 0, IDD_HAVEDOMAIN,
                          MB_OK | MB_ERROR);
               return (MRESULT) FALSE;
            }
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         QueryWinPos(parent, &windowpositions.domainentrypos);
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*------------------------------ UpdateDomains ------------------------------*/
/* Fllt die Listbox mit den Domains neu                                     */
/*---------------------------------------------------------------------------*/

static void UpdateDomains(HWND hwnd)
{
   PDOMAINS pDomains;
   extern PDOMAINS domains;

   WinSendDlgItemMsg(hwnd, IDD_DOMAINS+3, LM_DELETEALL, NULL, NULL);

   pDomains=domains;
   while (pDomains)
   {
      WinInsertLboxItem(WinWindowFromID(hwnd, IDD_DOMAINS+3), LIT_SORTASCENDING,
                        pDomains->domainname);
      pDomains=pDomains->next;
   }

   if (domains)  /* ueberhaupt Domains da? */
   {
      WinSendDlgItemMsg(hwnd, IDD_DOMAINS+3, LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));
      WinEnableControl(hwnd, IDD_DOMAINS+5, TRUE);
      WinEnableControl(hwnd, IDD_DOMAINS+6, TRUE);
   }
   else
   {
      WinEnableControl(hwnd, IDD_DOMAINS+5, FALSE);
      WinEnableControl(hwnd, IDD_DOMAINS+6, FALSE);
   }
   return;
}

/*-------------------------- ModemTypesProc  --------------------------------*/
/* Eingabe-Prozedur f. Modem-Typen                                           */
/*---------------------------------------------------------------------------*/

typedef struct
{
   MINIRECORDCORE RecordCore;
   ULONG ulNum;
   PCHAR pchDesc;
} MODEMTYPERECORD, *PMODEMTYPERECORD;

static MRESULT EXPENTRY ModemTypesProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern NODELISTOPT nodelist;
   extern DIRTYFLAGS dirtyflags;
   PFIELDINFO pFirstField, pField;
   static char pchTitleBit[50];
   static char pchTitleByte[50];
   static char pchTitleDesc[50];
   CNRINFO cnrinfo;

   switch(message)
   {
      case WM_INITDLG:
         /* Strings */
         LoadString(IDST_MT_BIT, sizeof(pchTitleBit), pchTitleBit);
         LoadString(IDST_MT_BYTE, sizeof(pchTitleByte), pchTitleByte);
         LoadString(IDST_MT_DESC, sizeof(pchTitleDesc), pchTitleDesc);

         /* Eingabe-Feld */
         WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+7, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_MODEMTYPE), NULL);

         /* Checkboxen */
         if (nodelist.ulOptions & MODEMFLAGS_BYTETYPE)
            WinCheckButton(hwnd, IDD_MODEMTYPES+5, TRUE);
         else
            WinCheckButton(hwnd, IDD_MODEMTYPES+4, TRUE);

         /* Container-Beschriftung */
         pFirstField = WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+3,
                                         CM_ALLOCDETAILFIELDINFO,
                                         MPFROMSHORT(2), NULL);
         pField = pFirstField;
         if (pField)
         {
            FIELDINFOINSERT InfoInsert;

            /* Nummern-Feld */
            pField->cb = sizeof(FIELDINFO);
            pField->flData = CFA_ULONG | CFA_SEPARATOR | CFA_HORZSEPARATOR | CFA_RIGHT;
            pField->flTitle = CFA_FITITLEREADONLY | CFA_CENTER;
            if (nodelist.ulOptions & MODEMFLAGS_BYTETYPE)
               pField->pTitleData = pchTitleByte;
            else
               pField->pTitleData = pchTitleBit;
            pField->offStruct = FIELDOFFSET(MODEMTYPERECORD, ulNum);

            /* Bezeichnungs-Feld */
            pField = pField->pNextFieldInfo;

            pField->cb = sizeof(FIELDINFO);
            pField->flData = CFA_STRING | CFA_HORZSEPARATOR | CFA_LEFT;
            pField->flTitle = CFA_FITITLEREADONLY | CFA_LEFT;
            pField->pTitleData = pchTitleDesc;
            pField->offStruct = FIELDOFFSET(MODEMTYPERECORD, pchDesc);

            /* Einfuegen */
            InfoInsert.cb = sizeof(InfoInsert);
            InfoInsert.pFieldInfoOrder = (PFIELDINFO) CMA_FIRST;
            InfoInsert.fInvalidateFieldInfo = TRUE;
            InfoInsert.cFieldInfoInsert = 2;

            WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+3, CM_INSERTDETAILFIELDINFO,
                              pFirstField,
                              &InfoInsert);
         }

         /* Container */
         cnrinfo.cb = sizeof(cnrinfo);
         cnrinfo.flWindowAttr = CV_DETAIL | CA_DETAILSVIEWTITLES;
         WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+3, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_FLWINDOWATTR));

         RefreshModemTypeList(hwnd, IDD_MODEMTYPES+3, &nodelist);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == IDD_MODEMTYPES+8) /* Aendern-Button */
         {
            PMODEMTYPERECORD pRecord;

            pRecord = WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+3, CM_QUERYRECORDEMPHASIS,
                                        MPFROMLONG(CMA_FIRST),
                                        MPFROMSHORT(CRA_CURSORED));
            if (pRecord)
            {
               WinQueryDlgItemText(hwnd, IDD_MODEMTYPES+7, LEN_MODEMTYPE+1,
                                   pRecord->pchDesc);
               WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+3, CM_INVALIDATERECORD,
                                 &pRecord, MPFROM2SHORT(1, CMA_TEXTCHANGED));
               dirtyflags.nodedirty = TRUE;
            }

         }
         return (MRESULT) FALSE;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_MODEMTYPES+4 || /* Typ-Radiobuttons */
             SHORT1FROMMP(mp1) == IDD_MODEMTYPES+5)
         {
            if (SHORT2FROMMP(mp1) == BN_CLICKED ||
                SHORT2FROMMP(mp1) == BN_DBLCLICKED)
            {
               if (WinQueryButtonCheckstate(hwnd, IDD_MODEMTYPES+4))
               {
                  nodelist.ulOptions &= ~MODEMFLAGS_BYTETYPE;
                  RefreshModemTypeList(hwnd, IDD_MODEMTYPES+3, &nodelist);
               }
               else
               {
                  nodelist.ulOptions |= MODEMFLAGS_BYTETYPE;
                  RefreshModemTypeList(hwnd, IDD_MODEMTYPES+3, &nodelist);
               }
               dirtyflags.nodedirty = TRUE;

               /* Beschreibung anpassen */
               pField = WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+3, CM_QUERYDETAILFIELDINFO,
                                          NULL, MPFROMSHORT(CMA_FIRST));
               if (pField)
               {
                  if (nodelist.ulOptions & MODEMFLAGS_BYTETYPE)
                     pField->pTitleData = pchTitleByte;
                  else
                     pField->pTitleData = pchTitleBit;

                  WinSendDlgItemMsg(hwnd, IDD_MODEMTYPES+3, CM_INVALIDATEDETAILFIELDINFO,
                                    NULL, NULL);
               }
            }
         }
         if (SHORT1FROMMP(mp1) == IDD_MODEMTYPES+3) /* Container */
            if (SHORT2FROMMP(mp1) == CN_EMPHASIS)
            {
               PNOTIFYRECORDEMPHASIS pNotify = mp2;

               if (pNotify->fEmphasisMask & CRA_CURSORED)
                  if (pNotify->pRecord->flRecordAttr & CRA_CURSORED)
                     WinSetDlgItemText(hwnd, IDD_MODEMTYPES+7, ((PMODEMTYPERECORD)pNotify->pRecord)->pchDesc);
            }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

static void RefreshModemTypeList(HWND hwndDlg, ULONG ulIdCnr, NODELISTOPT *pNodelist)
{
   HWND hwndCnr = WinWindowFromID(hwndDlg, ulIdCnr);
   PMODEMTYPERECORD pRecord, pFirstRecord;
   ULONG ulNum;
   int i;
   RECORDINSERT RecordInsert;

   /* alle Items loeschen */
   WinSendMsg(hwndCnr, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));

   /* neue Items einfuegen */
   if (pNodelist->ulOptions & MODEMFLAGS_BYTETYPE)
      ulNum = MAX_MODEMTYPES_BYTE;
   else
      ulNum = MAX_MODEMTYPES;

   pFirstRecord = WinSendMsg(hwndCnr, CM_ALLOCRECORD,
                             MPFROMLONG(sizeof(MODEMTYPERECORD) - sizeof(MINIRECORDCORE)),
                             MPFROMSHORT(ulNum));
   pRecord = pFirstRecord;

   for (i=0; i<ulNum; i++)
   {
      if (pRecord)
      {
         if (pNodelist->ulOptions & MODEMFLAGS_BYTETYPE)
         {
            pRecord->ulNum = i+1;
            pRecord->pchDesc = pNodelist->bytetypes[i];
         }
         else
         {
            pRecord->ulNum = i;
            pRecord->pchDesc = pNodelist->modemtype[i];
         }

         pRecord = (PMODEMTYPERECORD) pRecord->RecordCore.preccNextRecord;
      }
   }

   /* Einfuegen */
   RecordInsert.cb = sizeof(RecordInsert);
   RecordInsert.pRecordOrder = (PRECORDCORE) CMA_FIRST;
   RecordInsert.pRecordParent = NULL;
   RecordInsert.fInvalidateRecord = TRUE;
   RecordInsert.zOrder = CMA_TOP;
   RecordInsert.cRecordsInsert = ulNum;
   WinSendMsg(hwndCnr, CM_INSERTRECORD, pFirstRecord, &RecordInsert);

   return;
}

/*------------------------------- MsgOptProc --------------------------------*/
/* Dialog-Prozedur des Msg-Option-Dialogs                                    */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY MsgOptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern GENERALOPT generaloptions;
   extern DIRTYFLAGS dirtyflags;
   extern BOOL isregistered;
   BOOL booltemp;

   switch(message)
   {
      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_INITDLG:
         WinCheckButton(parent, IDD_MSGOPT+7, generaloptions.monospaced);
         WinCheckButton(parent, IDD_MSGOPT+6, generaloptions.tearinnet);
         WinCheckButton(parent, IDD_MSGOPT+1, generaloptions.origininnet);
         WinCheckButton(parent, IDD_MSGOPT+8, generaloptions.scanatstartup);
         WinCheckButton(parent, IDD_MSGOPT+10, generaloptions.beeponpersonal);
         WinCheckButton(parent, IDD_MSGOPT+3, generaloptions.nohighlight);
         WinCheckButton(parent, IDD_MSGOPT+4, generaloptions.genFwdSubj);
         WinCheckButton(parent, IDD_MSGOPT+5, generaloptions.usepid);
         SetFocusControl(parent, IDD_MSGOPT+5);
         return (MRESULT) TRUE;

      case WM_CLOSE:
      case WM_DESTROY:
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+6);
         if (booltemp != generaloptions.tearinnet)
         {
            generaloptions.tearinnet=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+8);
         if (booltemp != generaloptions.scanatstartup)
         {
            generaloptions.scanatstartup=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+10);
         if (booltemp != generaloptions.beeponpersonal)
         {
            generaloptions.beeponpersonal=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+1);
         if (booltemp != generaloptions.origininnet)
         {
            generaloptions.origininnet=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+5);
         if (booltemp != generaloptions.usepid)
         {
            generaloptions.usepid=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+3);
         if (booltemp != generaloptions.nohighlight)
         {
            generaloptions.nohighlight=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+4);
         if (booltemp != generaloptions.genFwdSubj)
         {
            generaloptions.genFwdSubj=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         booltemp=WinQueryButtonCheckstate(parent, IDD_MSGOPT+7);
         if (booltemp != generaloptions.monospaced)
         {
            generaloptions.monospaced=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*------------------------------- StartupProc -------------------------------*/
/* Dialog-Prozedur des Startup-Dialogs                                       */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY StartupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern GENERALOPT generaloptions;
   extern DIRTYFLAGS dirtyflags;
   BOOL booltemp;
   char pchTemp[LEN_AREATAG+1];

   switch(message)
   {
      case WM_INITDLG:
         WinCheckButton(parent, IDD_STARTUP+3, generaloptions.uselastarea);
         WinCheckButton(parent, IDD_STARTUP+4, !generaloptions.uselastarea);
         WinEnableControl(parent, IDD_STARTUP+5, !generaloptions.uselastarea);
         WinSendDlgItemMsg(parent, IDD_STARTUP+5, EM_SETTEXTLIMIT,
                           MPFROMLONG(LEN_AREATAG), NULL);
         WinSetDlgItemText(parent, IDD_STARTUP+5, generaloptions.startarea);
         SetFocusControl(parent, IDD_STARTUP+3);
         return (MRESULT) TRUE;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         if ((SHORT1FROMMP(mp1)==IDD_STARTUP+3 ||
              SHORT1FROMMP(mp1)==IDD_STARTUP+4) &&
             (SHORT2FROMMP(mp1)==BN_CLICKED ||
              SHORT2FROMMP(mp1)==BN_DBLCLICKED))
         {
            booltemp=WinQueryButtonCheckstate(parent, IDD_STARTUP+4);
            WinEnableControl(parent, IDD_STARTUP+5, booltemp);
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         booltemp=WinQueryButtonCheckstate(parent, IDD_STARTUP+3);
         if (booltemp != generaloptions.uselastarea)
         {
            generaloptions.uselastarea=booltemp;
            dirtyflags.optionsdirty=TRUE;
         }
         WinQueryDlgItemText(parent, IDD_STARTUP+5, LEN_AREATAG+1, pchTemp);
         if (strcmp(generaloptions.startarea, pchTemp))
         {
            strcpy(generaloptions.startarea, pchTemp);
            dirtyflags.optionsdirty=TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*------------------------------- EditorOptProc -----------------------------*/
/* Dialog-Prozedur des Editor-Optionen-Dialogs                               */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY EditorOptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern GENERALOPT generaloptions;
   extern DIRTYFLAGS dirtyflags;
   extern WINDOWCOLORS windowcolors;
   LONG lTemp;

   switch(message)
   {
      case WM_INITDLG:
         /* Tabulator-Settings */
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+4, SPBM_SETLIMITS,
                           MPFROMLONG(20), MPFROMLONG(1));
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+4, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(generaloptions.lTabSize), NULL);
         /* Message-laenge */
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+14, SPBM_SETLIMITS,
                           MPFROMLONG(500), MPFROMLONG(2));
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+14, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(generaloptions.lMaxMsgLen), NULL);

         /* Farben im Value-Set initialisieren */
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SETITEM,
                           MPFROM2SHORT(1, 1),
                           MPFROMLONG(windowcolors.viewerback));
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SETITEM,
                           MPFROM2SHORT(2, 1),
                           MPFROMLONG(windowcolors.viewerfore));
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SETITEM,
                           MPFROM2SHORT(3, 1),
                           MPFROMLONG(windowcolors.viewerquote));
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SETITEM,
                           MPFROM2SHORT(4, 1),
                           MPFROMLONG(windowcolors.viewerquote2));
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SETITEM,
                           MPFROM2SHORT(5, 1),
                           MPFROMLONG(windowcolors.viewertearline));
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SETITEM,
                           MPFROM2SHORT(6, 1),
                           MPFROMLONG(windowcolors.viewerorigin));

         /* Erstes Element im VS auswaehlen */
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SELECTITEM,
                           MPFROM2SHORT(1, 1), NULL);

         /* Fadenkreuz im Color-Wheel entsprechend setzen */
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+13, CLSM_SETRGB,
                           &windowcolors.viewerback,
                           NULL);
         break;

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_EDITOROPT+4:
               if (!WinSendDlgItemMsg(parent, IDD_EDITOROPT+4, SPBM_QUERYVALUE,
                                      NULL,
                                      MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE)))
                  WinAlarm(HWND_DESKTOP, WA_ERROR);
               break;

            case IDD_EDITOROPT+14:
               if (!WinSendDlgItemMsg(parent, IDD_EDITOROPT+14, SPBM_QUERYVALUE,
                                      NULL,
                                      MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE)))
                  WinAlarm(HWND_DESKTOP, WA_ERROR);
               break;

            case IDD_EDITOROPT+12:  /* Value-Set */
               if (SHORT2FROMMP(mp1) == VN_SELECT)
               {
                  ULONG ulColor;

                  /* neue Selektion abfragen */
                  ulColor=(ULONG)WinSendDlgItemMsg(parent, IDD_EDITOROPT+12,
                                                   VM_QUERYITEM, mp2, NULL);

                  /* Fadenkreuz setzen */
                  WinSendDlgItemMsg(parent, IDD_EDITOROPT+13,
                                    CLSM_SETRGB, &ulColor, NULL);
               }
               break;

            case IDD_EDITOROPT+13:  /* Color-Wheel */
               if (SHORT2FROMMP(mp1) == CLSN_RGB)
               {
                  MRESULT selected;
                  ULONG ulVer[2]={0,0};

                  DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MINOR, ulVer, sizeof(ulVer));

                  /* selektiertes Item im Value-Set abfragen */
                  selected=WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_QUERYSELECTEDITEM,
                                             NULL, NULL);

                  /* Farbe updaten */
                  WinSendDlgItemMsg(parent, IDD_EDITOROPT+12, VM_SETITEM,
                                    selected, mp2);

                  /* Farbe in Settings eintragen */
                  switch(SHORT1FROMMR(selected))
                  {
                     case 1:
                        windowcolors.viewerback= (LONG) mp2;
                        if (ulVer[0] > 20 || ulVer[1]>=11)
                        {
                           /* OS/2 2.11 verwendet RGB-Farben f. MLE */
                           windowcolors.editback=(LONG) mp2;
                        }
                        break;

                     case 2:
                        windowcolors.viewerfore= (LONG) mp2;
                        if (ulVer[0] > 20 || ulVer[1]>=11)
                        {
                           /* OS/2 2.11 verwendet RGB-Farben f. MLE */
                           windowcolors.editfore=(LONG) mp2;
                        }
                        break;

                     case 3:
                        windowcolors.viewerquote= (LONG) mp2;
                        break;

                     case 4:
                        windowcolors.viewerquote2= (LONG) mp2;
                        break;

                     case 5:
                        windowcolors.viewertearline= (LONG) mp2;
                        break;

                     case 6:
                        windowcolors.viewerorigin= (LONG) mp2;
                        break;

                     default:
                        break;
                  }
               }
               break;

            default:
               break;
         }
         break;


      case WM_DESTROY:
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+4, SPBM_QUERYVALUE,
                           &lTemp,
                           MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
         if (generaloptions.lTabSize != lTemp)
         {
            generaloptions.lTabSize = lTemp;
            dirtyflags. optionsdirty=TRUE;
         }
         WinSendDlgItemMsg(parent, IDD_EDITOROPT+14, SPBM_QUERYVALUE,
                           &lTemp,
                           MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
         if (generaloptions.lMaxMsgLen != lTemp)
         {
            generaloptions.lMaxMsgLen = lTemp;
            dirtyflags. optionsdirty=TRUE;
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*----------------------------- SquishOptProc -------------------------------*/
/* Dialog-Prozedur des Squish-Optionen-Dialogs                               */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY SquishOptProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   BOOL booltemp;
   char pathnamebuffer[LEN_PATHNAME+1];
   extern USERDATAOPT userdaten;
   extern PATHNAMES pathnames;
   extern MISCOPTIONS miscoptions;
   extern GENERALOPT generaloptions;
   extern DIRTYFLAGS dirtyflags;
   extern HAB anchor;
   extern HWND hwndhelp;
   static HWND hwndSetupFrame;
   ULONG ulTemp;

   switch(message)
   {
      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            switch (SHORT1FROMMP(mp1))
            {
               case IDD_SQUISHOPTIONS+5:
                  WinQueryDlgItemText(parent, IDD_SQUISHOPTIONS+3,
                                      LEN_PATHNAME+1, pathnamebuffer);
                  if (GetPathname(parent, pathnamebuffer)==DID_OK)
                  {
                     if (!ReadNewCfgFile(parent, hwndSetupFrame, pathnamebuffer))
                        WinSetDlgItemText(parent, IDD_SQUISHOPTIONS+3,
                                          pathnamebuffer);
                  }
                  break;

               default:
                  break;
            }
         }
         return (MRESULT) FALSE;

      case WM_INITDLG:
         hwndSetupFrame=WinQueryWindow(parent, QW_PARENT);
         WinSubclassWindow(WinWindowFromID(parent, IDD_SQUISHOPTIONS+3),
                           FileEntryProc);
         WinSendDlgItemMsg(parent,IDD_SQUISHOPTIONS+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSendDlgItemMsg(parent,IDD_SQUISHOPTIONS+3, EM_SETREADONLY,
                           MPFROMSHORT(TRUE), NULL);

         /* Tosser-Pfad */
         WinSetDlgItemText(parent, IDD_SQUISHOPTIONS+3,
                           pathnames.squishcfg);
         WinCheckButton(parent, IDD_SQUISHOPTIONS+2, miscoptions.readcfg);
         WinEnableControl(parent, IDD_SQUISHOPTIONS+3, miscoptions.readcfg);
         WinEnableControl(parent, IDD_SQUISHOPTIONS+5, miscoptions.readcfg);
         WinEnableControl(parent, IDD_SQUISHOPTIONS+7, miscoptions.readcfg);

         SetCfgType(parent, miscoptions.ulCfgType);

         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_SQUISHOPTIONS+2 &&
             (SHORT2FROMMP(mp1)==BN_CLICKED || SHORT2FROMMP(mp1)==BN_DBLCLICKED))
         {
            booltemp=WinQueryButtonCheckstate(parent, IDD_SQUISHOPTIONS+2);
            if (!userdaten.username[0][0])
               booltemp=FALSE;
            WinEnableControl(parent, IDD_SQUISHOPTIONS+3, booltemp);
            WinEnableControl(parent, IDD_SQUISHOPTIONS+5, booltemp);
            WinEnableControl(parent, IDD_SQUISHOPTIONS+7, booltemp);
            miscoptions.readcfg=booltemp;
         }
         if (SHORT1FROMMP(mp1)==IDD_SQUISHOPTIONS+3 &&
             SHORT2FROMMP(mp1)==EN_FILEDROPPED)
         {
            WinQueryDlgItemText(parent, IDD_SQUISHOPTIONS+3,
                                LEN_PATHNAME+1, pathnamebuffer);

            ReadNewCfgFile(parent, hwndSetupFrame, pathnamebuffer);
         }
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         WinQueryDlgItemText(parent, IDD_SQUISHOPTIONS+3, LEN_PATHNAME+1,
                             pathnamebuffer);
         if (strcmp(pathnames.squishcfg, pathnamebuffer))
            strcpy(pathnames.squishcfg, pathnamebuffer);

         ulTemp = GetCfgType(parent);
         if (miscoptions.ulCfgType != ulTemp)
         {
            miscoptions.ulCfgType = ulTemp;
         }
         break;

      case SUM_CHECKSETUP:
         if (!userdaten.username[0][0])
         {
            WinEnableControl(parent, IDD_SQUISHOPTIONS+3, FALSE);
            WinEnableControl(parent, IDD_SQUISHOPTIONS+5, FALSE);
            WinEnableControl(parent, IDD_SQUISHOPTIONS+7, FALSE);
            WinEnableControl(parent, IDD_SQUISHOPTIONS+2, FALSE);
         }
         else
         {
            if (miscoptions.readcfg)
            {
               WinEnableControl(parent, IDD_SQUISHOPTIONS+3, TRUE);
               WinEnableControl(parent, IDD_SQUISHOPTIONS+5, TRUE);
               WinEnableControl(parent, IDD_SQUISHOPTIONS+7, TRUE);
            }
            else
            {
               WinEnableControl(parent, IDD_SQUISHOPTIONS+3, FALSE);
               WinEnableControl(parent, IDD_SQUISHOPTIONS+5, FALSE);
               WinEnableControl(parent, IDD_SQUISHOPTIONS+7, FALSE);
            }
            WinEnableControl(parent, IDD_SQUISHOPTIONS+2, TRUE);
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

static int ReadNewCfgFile(HWND parent, HWND hwndSetupFrame, char *pchPathName)
{
   extern PATHNAMES pathnames;
   extern AREALIST arealiste;

   ULONG ulCfgType = GetCfgType(parent);
   ULONG rc;

   rc = ReReadAreas(anchor, pchPathName, ulCfgType);

   if (rc)
   {
      HandleInitErrors(parent, rc);

      return 1;
   }
   else
   {
      WinSetDlgItemText(parent, IDD_SQUISHOPTIONS+3, pchPathName);

      /* Squish-Pfad kopieren */
      strcpy(pathnames.squishcfg, pchPathName);
      arealiste.bDirty = TRUE;
   }
   SendMsg(hwndSetupFrame, SUM_REFRESHUSER, NULL, NULL);

   return 0;
}

static ULONG GetCfgType(HWND hwndDlg)
{
   SHORT sItem;

   sItem = (SHORT) WinSendDlgItemMsg(hwndDlg, IDD_SQUISHOPTIONS+7,
                                     LM_QUERYSELECTION,
                                     MPFROMSHORT(LIT_FIRST), NULL);
   if (sItem != LIT_NONE)
   {
      return (ULONG) WinSendDlgItemMsg(hwndDlg, IDD_SQUISHOPTIONS+7, LM_QUERYITEMHANDLE,
                                       MPFROMSHORT(sItem), NULL);
   }
   else
      return 0;
}

static void SetCfgType(HWND hwndDlg, ULONG ulCfgType)
{
   extern PCFGDLL pCfgDLLs;
   PCFGDLL pTemp=NULL;
   SHORT sItem;
   HWND hwndList = WinWindowFromID(hwndDlg, IDD_SQUISHOPTIONS+7);

   while (pTemp = CFG_FindFormat(pCfgDLLs, CFGTYPE_ANY, pTemp))
   {
      sItem = (SHORT) SendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING),
                              pTemp->pchFormatName);
      if (sItem >= 0)
         SendMsg(hwndList, LM_SETITEMHANDLE, MPFROMSHORT(sItem),
                 MPFROMLONG(pTemp->ulFormatID));
      if (ulCfgType == pTemp->ulFormatID)
         SendMsg(hwndList, LM_SELECTITEM, MPFROMSHORT(sItem), MPFROMSHORT(TRUE));
   }

   return;
}

static MRESULT EXPENTRY TosserPathsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   BOOL booltemp;
   char pathnamebuffer[LEN_PATHNAME+1];
   extern PATHNAMES pathnames;
   extern MISCOPTIONS miscoptions;
   extern ECHOTOSSOPT echotossoptions;
   extern DIRTYFLAGS dirtyflags;
   extern HAB anchor;
   extern GENERALOPT generaloptions;
   LONG spinbuffer;
   extern HWND hwndhelp;

   switch(message)
   {
      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_INITDLG:
         /* ECHOMAIL.JAM */
         WinSendDlgItemMsg(parent, IDD_TOSSERPATHS+6, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSetDlgItemText(parent, IDD_TOSSERPATHS+6, generaloptions.jampath);

         /* Attach */
         WinSendDlgItemMsg(parent, IDD_TOSSERPATHS+12, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSetDlgItemText(parent, IDD_TOSSERPATHS+12, generaloptions.attachpath);

         /* Offset-Spinbutton */
         WinSendDlgItemMsg(parent,IDD_TOSSERPATHS+9, SPBM_SETLIMITS,
                           MPFROMLONG(500), MPFROMLONG(0));
         WinSendDlgItemMsg(parent, IDD_TOSSERPATHS+9, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(miscoptions.lastreadoffset), NULL);

         /* Echotoss-Optionen */
         WinCheckButton(parent, IDD_TOSSERPATHS+3, echotossoptions.useechotoss);
         WinEnableControl(parent, IDD_TOSSERPATHS+4, echotossoptions.useechotoss);
         WinSendDlgItemMsg(parent,IDD_TOSSERPATHS+4, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSetDlgItemText(parent, IDD_TOSSERPATHS+4, echotossoptions.pchEchoToss);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_TOSSERPATHS+3 &&
             (SHORT2FROMMP(mp1)==BN_CLICKED || SHORT2FROMMP(mp1)==BN_DBLCLICKED))
         {
            booltemp=WinQueryButtonCheckstate(parent, IDD_TOSSERPATHS+3);
            WinEnableControl(parent, IDD_TOSSERPATHS+4, booltemp);
         }
         if (SHORT1FROMMP(mp1)==IDD_SQUISHOPTIONS+9)
         {
            if (!WinSendDlgItemMsg(parent, IDD_SQUISHOPTIONS+9, SPBM_QUERYVALUE,
                          MPFROMP(&spinbuffer),
                          MPFROM2SHORT((USHORT)0, SPBQ_ALWAYSUPDATE)))
               WinAlarm(HWND_DESKTOP, WA_ERROR);
         }
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         if (!WinSendDlgItemMsg(parent, IDD_TOSSERPATHS+9, SPBM_QUERYVALUE,
                       MPFROMP(&miscoptions.lastreadoffset),
                       MPFROM2SHORT((USHORT)0, SPBQ_ALWAYSUPDATE)))
            DosBeep(1000,100);

         WinQueryDlgItemText(parent, IDD_TOSSERPATHS+6, LEN_PATHNAME+1,
                             pathnamebuffer);
         RemoveBackslash(pathnamebuffer);
         if (strcmp(generaloptions.jampath, pathnamebuffer))
         {
            strcpy(generaloptions.jampath, pathnamebuffer);
            dirtyflags.optionsdirty = TRUE;
         }

         WinQueryDlgItemText(parent, IDD_TOSSERPATHS+12, LEN_PATHNAME+1,
                             pathnamebuffer);
         RemoveBackslash(pathnamebuffer);
         if (strcmp(generaloptions.attachpath, pathnamebuffer))
         {
            strcpy(generaloptions.attachpath, pathnamebuffer);
            dirtyflags.optionsdirty = TRUE;
         }

         booltemp=WinQueryButtonCheckstate(parent, IDD_TOSSERPATHS+3);
         if (echotossoptions.useechotoss != booltemp)
         {
            echotossoptions.useechotoss=booltemp;
            dirtyflags.echotossdirty=TRUE;
         }
         WinQueryDlgItemText(parent, IDD_TOSSERPATHS+4, LEN_PATHNAME+1,
                             pathnamebuffer);
         if (strcmp(echotossoptions.pchEchoToss, pathnamebuffer))
         {
            strcpy(echotossoptions.pchEchoToss, pathnamebuffer);
            dirtyflags.echotossdirty=TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*----------------------------- RemapSetupProc   ----------------------------*/
/* Dialog-Prozedur des Drive-Remap-Setups                                    */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY RemapSetupProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern DIRTYFLAGS dirtyflags;
   extern DRIVEREMAP driveremap;
   static char *pchDrives[] = { "C:", "D:", "E:", "F:", "G:", "H:", "I:", "J:",
                                "K:", "L:", "M:", "N:", "O:", "P:", "Q:", "R:",
                                "S:", "T:", "U:", "V:", "W:", "X:", "Y:", "Z:"};
   char chLocal;

   switch(message)
   {
      case WM_INITDLG:
         /* Remote Drive fuellen */
         WinSendDlgItemMsg(parent, IDD_DRIVEREMAP+5, SPBM_SETARRAY,
                           pchDrives, MPFROMSHORT(24));

         /* Local Drive fuellen */
         WinSendDlgItemMsg(parent, IDD_DRIVEREMAP+6, SPBM_SETARRAY,
                           pchDrives, MPFROMSHORT(24));

         /* Anfangswert setzen */
         chLocal = driveremap.pchRemapString[0] - 'C';
         WinSendDlgItemMsg(parent, IDD_DRIVEREMAP+6, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(chLocal), NULL);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_DRIVEREMAP+5) /* Remote Drive */
         {
            if (SHORT2FROMMP(mp1) == SPBN_UPARROW ||
                SHORT2FROMMP(mp1) == SPBN_DOWNARROW)
            {
               char pchTemp[3];

               WinSendDlgItemMsg(parent, IDD_DRIVEREMAP+5, SPBM_QUERYVALUE,
                                 pchTemp, MPFROM2SHORT(sizeof(pchTemp),
                                                       SPBQ_ALWAYSUPDATE));
               chLocal = driveremap.pchRemapString[pchTemp[0] - 'C'] - 'C';

               WinSendDlgItemMsg(parent, IDD_DRIVEREMAP+6, SPBM_SETCURRENTVALUE,
                                 MPFROMLONG(chLocal), NULL);
            }
         }
         if (SHORT1FROMMP(mp1) == IDD_DRIVEREMAP+6) /* Local Drive */
         {
            if (SHORT2FROMMP(mp1) == SPBN_UPARROW ||
                SHORT2FROMMP(mp1) == SPBN_DOWNARROW)
            {
               char pchTemp[3];
               char pchTemp2[3];
               WinSendDlgItemMsg(parent, IDD_DRIVEREMAP+5, SPBM_QUERYVALUE,
                                 pchTemp, MPFROM2SHORT(sizeof(pchTemp),
                                                       SPBQ_ALWAYSUPDATE));
               WinSendDlgItemMsg(parent, IDD_DRIVEREMAP+6, SPBM_QUERYVALUE,
                                 pchTemp2, MPFROM2SHORT(sizeof(pchTemp2),
                                                       SPBQ_ALWAYSUPDATE));
               driveremap.pchRemapString[pchTemp[0] - 'C'] = pchTemp2[0];
               dirtyflags.remapdirty = TRUE;
            }
         }
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent,message,mp1,mp2);
}

/*----------------------------- SafetyOptProc    ----------------------------*/
/* Dialog-Prozedur des Safety-Setups                                         */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY SafetyOptProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern DIRTYFLAGS dirtyflags;
   extern GENERALOPT generaloptions;
   ULONG ulTemp;

   switch(msg)
   {
      case WM_INITDLG:
         if (generaloptions.safety & SAFETY_DELMSG)
            WinCheckButton(hwnd, IDD_SAFETY+2, TRUE);
         if (generaloptions.safety & SAFETY_SHREDMSG)
            WinCheckButton(hwnd, IDD_SAFETY+3, TRUE);
         if (generaloptions.safety & SAFETY_CATCHUP)
            WinCheckButton(hwnd, IDD_SAFETY+4, TRUE);
         if (generaloptions.safety & SAFETY_EDITSENT)
            WinCheckButton(hwnd, IDD_SAFETY+5, TRUE);
         if (generaloptions.safety & SAFETY_CHANGESETUP)
            WinCheckButton(hwnd, IDD_SAFETY+6, TRUE);
         if (generaloptions.safety & SAFETY_DISCARD)
            WinCheckButton(hwnd, IDD_SAFETY+7, TRUE);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         ulTemp=0;
         if (WinQueryButtonCheckstate(hwnd, IDD_SAFETY+2))
            ulTemp |= SAFETY_DELMSG;
         if (WinQueryButtonCheckstate(hwnd, IDD_SAFETY+3))
            ulTemp |= SAFETY_SHREDMSG;
         if (WinQueryButtonCheckstate(hwnd, IDD_SAFETY+4))
            ulTemp |= SAFETY_CATCHUP;
         if (WinQueryButtonCheckstate(hwnd, IDD_SAFETY+5))
            ulTemp |= SAFETY_EDITSENT;
         if (WinQueryButtonCheckstate(hwnd, IDD_SAFETY+6))
            ulTemp |= SAFETY_CHANGESETUP;
         if (WinQueryButtonCheckstate(hwnd, IDD_SAFETY+7))
            ulTemp |= SAFETY_DISCARD;
         if (generaloptions.safety != ulTemp)
         {
            generaloptions.safety = ulTemp;
            dirtyflags.optionsdirty = TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*----------------------------- OpenWinProc ---------------------------------*/
/* Dialog-Prozedur des Fenster-Setups                                        */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY OpenWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   extern OPENWIN OpenWindows;

   switch(msg)
   {
      case WM_INITDLG:
         CheckOpenWinButton(hwnd, IDD_SU_OPENWIN+3, &OpenWindows, OPENWIN_AREA);
         CheckOpenWinButton(hwnd, IDD_SU_OPENWIN+4, &OpenWindows, OPENWIN_KLUDGES);
         CheckOpenWinButton(hwnd, IDD_SU_OPENWIN+5, &OpenWindows, OPENWIN_BOOKMARKS);
         CheckOpenWinButton(hwnd, IDD_SU_OPENWIN+6, &OpenWindows, OPENWIN_THRL);
         CheckOpenWinButton(hwnd, IDD_SU_OPENWIN+7, &OpenWindows, OPENWIN_MSGL);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_DESTROY:
         OpenWindows.ulForceClose = OpenWindows.ulForceOpen = 0;

         QueryOpenWinButton(hwnd, IDD_SU_OPENWIN+3, &OpenWindows, OPENWIN_AREA);
         QueryOpenWinButton(hwnd, IDD_SU_OPENWIN+4, &OpenWindows, OPENWIN_KLUDGES);
         QueryOpenWinButton(hwnd, IDD_SU_OPENWIN+5, &OpenWindows, OPENWIN_BOOKMARKS);
         QueryOpenWinButton(hwnd, IDD_SU_OPENWIN+6, &OpenWindows, OPENWIN_THRL);
         QueryOpenWinButton(hwnd, IDD_SU_OPENWIN+7, &OpenWindows, OPENWIN_MSGL);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static void CheckOpenWinButton(HWND hwndDlg, ULONG ulID, POPENWIN pOpenWindows, ULONG ulMask)
{
   if (pOpenWindows->ulForceOpen & ulMask)
      WinCheckButton(hwndDlg, ulID, 1UL);
   else
      if (pOpenWindows->ulForceClose & ulMask)
         WinCheckButton(hwndDlg, ulID, 0UL);
      else
         WinCheckButton(hwndDlg, ulID, 2UL);

   return;
}

static void QueryOpenWinButton(HWND hwndDlg, ULONG ulID, POPENWIN pOpenWindows, ULONG ulMask)
{
   switch(WinQueryButtonCheckstate(hwndDlg, ulID))
   {
      case 0:
         pOpenWindows->ulForceClose |= ulMask;
         break;

      case 1:
         pOpenWindows->ulForceOpen |= ulMask;
         break;

      /* default: kein Flag setzen */
   }
   return;
}

/*----------------------------- GetPathname     -----------------------------*/
/* Holt mit Hilfe des File-Dialogs einen Pfadnamen                           */
/*---------------------------------------------------------------------------*/

LONG GetPathname(HWND hwndOwner, char *pchPathname)
{
   FILEDLG dlgpar;

   dlgpar.cbSize=sizeof(dlgpar);
   dlgpar.fl= FDS_CENTER | FDS_CUSTOM | FDS_HELPBUTTON | FDS_OPEN_DIALOG;
   dlgpar.pszTitle="File";
   dlgpar.pszOKButton="OK";
   dlgpar.pfnDlgProc=WinDefFileDlgProc;
   dlgpar.pszIType=NULL;
   dlgpar.papszITypeList=NULL;
   dlgpar.pszIDrive=NULL;
   dlgpar.papszIDriveList=NULL;
   dlgpar.hMod=hmodLang;
   strcpy(dlgpar.szFullFile,pchPathname);
   dlgpar.usDlgId=IDD_FILEDLG;

   WinFileDlg(HWND_DESKTOP,
              hwndOwner,
              &dlgpar);
   if (dlgpar.lReturn == DID_OK)
      strcpy(pchPathname, dlgpar.szFullFile);
   return dlgpar.lReturn;
}

/*-------------------------------- Modulende --------------------------------*/


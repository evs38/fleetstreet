/*---------------------------------------------------------------------------+
 | Titel: INSTALL.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 04.10.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Installationsprogramm fuer FleetStreet                                 |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>
#include "install.h"

/*--------------------------------- Defines ---------------------------------*/

#define MONO_FONT  "10.System Monospaced"
#define SCRIPTSINI "\\SCRIPTS.INI"
#define FOLDERLOC  "<WP_DESKTOP>"
#define FOLDERNAME "FleetStreet"
#define DOCNAME "Docs"

#define FILETYPE_NONE      0
#define FILETYPE_FILE      1
#define FILETYPE_SCRIPT    2
#define FILETYPE_LANG      3
#define FILETYPE_HLP       4
#define FILETYPE_READ      5
#define FILETYPE_DOC       6
#define FILETYPE_OBSFILE   7

#define REQUIRED_FILEID 1150UL


/*---------------------------------- Typen ----------------------------------*/

/* Verzeichnis von INSTALL.FIL */
typedef struct directory {
            struct directory *next;
            ULONG  ulEntryID;
            PCHAR  pchFileName;
            PCHAR  pchParam1;
            PCHAR  pchParam2;
            ULONG  ulOffsetData;
            ULONG  ulLengthData;
            FDATE  FileDate;
            FTIME  FileTime;
            ULONG  ulFlags;
            PCHAR  pchCompletePath;
         } DIRENTRY, *PDIRENTRY;

typedef struct stringlist {
            struct stringlist *next;
            char *pchString;
         } STRINGLIST, *PSTRINGLIST;

/*---------------------------- Globale Variablen ----------------------------*/

char * const pchFilesName="INSTALL.FIL";

char pchInstallDir[300];
char pchLangSelected[100];
BOOL bCreateObjects=TRUE;
BOOL bDoingInstall=FALSE;

PDIRENTRY pDirectory=NULL;
ULONG ulFileID;

HAB hab;
HMQ hmq;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

MRESULT EXPENTRY MainWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void DisplayStatus(HWND hwndDlg, ULONG ulStringID, PCHAR pchText);
USHORT MessageBox(HWND hwndOwner, ULONG ulMessageID, ULONG ulStyle);
void _Optlink FilReadThread(PVOID pParam);
MRESULT EXPENTRY ReadmeProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void _Optlink ReadReadme(PVOID pParam);
void _Optlink InstallThread(PVOID pParam);
static int CreatePath(char *pchPath);
int InstallFile(HWND hwndMain, PDIRENTRY pEntry, PCHAR pchPath);
int InstallLangFile(PDIRENTRY pEntry, PCHAR pchPath, PCHAR pchDestFile);
int InstallScriptsINI(HWND hwndMain, PCHAR pchInstallDir, PCHAR pchScriptDir);
int CreateObjects(HWND hwndMain, PCHAR pchInstallDir);
int SetFileDate(char *pchFileName, PFDATE pDate, PFTIME pTime);
static void GetInstallDir(char *pchDir, ULONG ulBufLen);
static void StoreInstallDir(char *pchDir);
static void GetInstallLang(char *pchLang, ULONG ulBufLen);
static void StoreInstallLang(char *pchLang);
static int DelObsFiles(PCHAR pchInstallDir, PCHAR pchFileSpec);

/*---------------------------------- main -----------------------------------*/

int main(void)
{
   hab=WinInitialize(0);

   if (hab)
   {
      hmq=WinCreateMsgQueue(hab, 100);

      if (hmq)
      {
         WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, MainWindowProc, NULLHANDLE,
                   IDD_MAIN, NULL);

         WinDestroyMsgQueue(hmq);
      }
      WinTerminate(hab);
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MainWindowProc                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fensterprozedur des Hauptfensters                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY MainWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   RECTL rectl, rectl2;
   SWP swp;
   PDIRENTRY pDir;
   SWCNTRL SwitchCtl;
   static HSWITCH hSwitch;

   switch(msg)
   {
      case WM_INITDLG:
         /* Fenster in Fensterliste */
         SwitchCtl.hwnd=hwnd;
         SwitchCtl.hwndIcon=NULLHANDLE;
         SwitchCtl.hprog=NULLHANDLE;
         SwitchCtl.idProcess=0;
         SwitchCtl.idSession=0;
         SwitchCtl.uchVisibility=SWL_VISIBLE;
         SwitchCtl.fbJump=SWL_JUMPABLE;
         SwitchCtl.bProgType=PROG_PM;
         WinQueryWindowText(hwnd, MAXNAMEL+4, SwitchCtl.szSwtitle);
         hSwitch=WinAddSwitchEntry(&SwitchCtl);

         /* Bitmap zentrieren */
         WinQueryWindowRect(hwnd, &rectl);
         WinQueryWindowRect(WinWindowFromID(hwnd, IDBM_MAIN), &rectl2);
         WinQueryWindowPos(WinWindowFromID(hwnd, IDBM_MAIN),
                           &swp);
         WinSetWindowPos(WinWindowFromID(hwnd, IDBM_MAIN),
                         NULLHANDLE,
                         (rectl.xRight-rectl2.xRight)/2,
                         swp.y,
                         0,0,
                         SWP_MOVE);

         /* Fenster zentrieren */
         WinQueryWindowRect(HWND_DESKTOP, &rectl);
         WinQueryWindowRect(hwnd, &rectl2);
         WinSetWindowPos(hwnd,
                         NULLHANDLE,
                         (rectl.xRight-rectl2.xRight)/2,
                         (rectl.yTop-rectl2.yTop)/2,
                         0,0,
                         SWP_MOVE | SWP_SHOW);
         /* Eingabefeld */
         WinSendDlgItemMsg(hwnd, IDD_MAIN+5, EM_SETTEXTLIMIT,
                           MPFROMSHORT(250), NULL);
         GetInstallDir(pchInstallDir, sizeof(pchInstallDir));
         WinSetDlgItemText(hwnd, IDD_MAIN+5, pchInstallDir);

         /* Checkbox */
         WinCheckButton(hwnd, IDD_MAIN+11, bCreateObjects);

         /* Anfangen mit Script pruefen */
         DisplayStatus(hwnd, IDST_STATUS_READFILES, NULL);
         _beginthread(FilReadThread, NULL, 32768, (PVOID) hwnd);
         break;

      case WM_DESTROY:
         WinRemoveSwitchEntry(hSwitch);
         break;

      case WM_CLOSE:
         if (bDoingInstall)
            return (MRESULT) FALSE;
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            ULONG ulNumLanguages;
            PCHAR *pItemArray;
            PCHAR *pItem;
            ULONG ulLang, ulLangSel;

            case IDD_MAIN+3: /* Close */
               if (bDoingInstall)
                  return (MRESULT) FALSE;
               break;

            case IDD_MAIN+1: /* Readme */
               WinDlgBox(HWND_DESKTOP, hwnd, ReadmeProc, NULLHANDLE, IDD_README,
                         NULL);
               WinEnableControl(hwnd, IDD_MAIN+2, TRUE);
               WinEnableControl(hwnd, IDD_MAIN+5, TRUE);
               WinEnableControl(hwnd, IDD_MAIN+7, TRUE);
               WinEnableControl(hwnd, IDD_MAIN+8, TRUE);
               WinEnableControl(hwnd, IDD_MAIN+11, TRUE);
               DisplayStatus(hwnd, IDST_STATUS_DOINSTALL, NULL);

               /* Sprachen in Spin-Button eintragen */
               pDir = pDirectory;
               ulNumLanguages=0;
               while (pDir)
               {
                  if (pDir->ulEntryID == FILETYPE_LANG)
                     ulNumLanguages++;
                  pDir = pDir->next;
               }
               pItemArray=calloc(ulNumLanguages, sizeof(PCHAR));
               pItem = pItemArray;

               GetInstallLang(pchLangSelected, sizeof(pchLangSelected));
               ulLang = ulLangSel =0;

               pDir = pDirectory;
               while (pDir)
               {
                  if (pDir->ulEntryID == FILETYPE_LANG)
                  {
                     *pItem = pDir->pchParam1;

                     if (!strcmp(pchLangSelected, pDir->pchParam1))
                        ulLangSel = ulLang;

                     pItem++;
                     ulLang++;
                  }
                  pDir = pDir->next;
               }
               WinSendDlgItemMsg(hwnd, IDD_MAIN+7, SPBM_SETARRAY,
                                 pItemArray, MPFROMLONG(ulNumLanguages));
               WinSendDlgItemMsg(hwnd, IDD_MAIN+7, SPBM_SETCURRENTVALUE,
                                 MPFROMLONG(ulLangSel), NULL);
               free(pItemArray);
               WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDD_MAIN+2));
               return (MRESULT) FALSE;

            case IDD_MAIN+8: /* Select */
               return (MRESULT) FALSE;

            case IDD_MAIN+2: /* Install */
               WinQueryDlgItemText(hwnd, IDD_MAIN+5, sizeof(pchInstallDir),
                                   pchInstallDir);
               if (!pchInstallDir[0])
               {
                  MessageBox(hwnd, IDST_MSG_NODIRENTERED, MB_OK);
                  return (MRESULT) FALSE;
               }
               else
               {
                  StoreInstallDir(pchInstallDir);
                  WinSendDlgItemMsg(hwnd, IDD_MAIN+7, SPBM_QUERYVALUE,
                                    pchLangSelected,
                                    MPFROM2SHORT(sizeof(pchLangSelected), SPBQ_ALWAYSUPDATE));
                  if (pchLangSelected[0])
                  {
                     StoreInstallLang(pchLangSelected);

                     bCreateObjects = WinQueryButtonCheckstate(hwnd, IDD_MAIN+11);
                     WinEnableControl(hwnd, IDD_MAIN+1, FALSE);
                     WinEnableControl(hwnd, IDD_MAIN+2, FALSE);
                     WinEnableControl(hwnd, IDD_MAIN+3, FALSE);
                     WinEnableControl(hwnd, IDD_MAIN+5, FALSE);
                     WinEnableControl(hwnd, IDD_MAIN+7, FALSE);
                     WinEnableControl(hwnd, IDD_MAIN+8, FALSE);
                     WinEnableControl(hwnd, IDD_MAIN+11, FALSE);
                     DisplayStatus(hwnd, 0, NULL);
                     _beginthread(InstallThread, NULL, 32768, (PVOID) hwnd);
                  }
                  else
                     MessageBox(hwnd, IDST_MSG_NOLANGSELECTED, MB_OK);
               }
               return (MRESULT) FALSE;

            default:
               return (MRESULT) FALSE;
         }
         break;

      case WM_FILESREADY:
         DisplayStatus(hwnd, IDST_STATUS_READREADME, NULL);
         WinEnableControl(hwnd, IDD_MAIN+1, TRUE);
         WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDD_MAIN+1));
         break;

      case WM_FILNOTFOUND:
         MessageBox(hwnd, IDST_MSG_NOFIL, MB_OK);
         WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);
         break;

      case WM_FILERROR:
         MessageBox(hwnd, IDST_MSG_FILERROR, MB_OK);
         WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);
         break;

      case WM_CREATEDIR:
         DisplayStatus(hwnd, IDST_STATUS_CREATEDIR, (PCHAR) mp1);
         break;

      case WM_INSTALLDONE:
         WinEnableControl(hwnd, IDD_MAIN+3, TRUE);
         DisplayStatus(hwnd, IDST_STATUS_DONE, NULL);
         break;

      case WM_DIRERROR:
         return (MRESULT) MessageBox(hwnd, IDST_MSG_CREATEDIR, MB_RETRYCANCEL);

      case WM_INSTALLABORTED:
         WinEnableControl(hwnd, IDD_MAIN+3, TRUE);
         DisplayStatus(hwnd, IDST_STATUS_INSTALLABORTED, NULL);
         break;

      case WM_INSTALLFILE:
         DisplayStatus(hwnd, IDST_STATUS_COPYING, (PCHAR) mp1);
         break;

      case WM_INSTALLLANG:
         DisplayStatus(hwnd, IDST_STATUS_COPYLANG, NULL);
         break;

      case WM_INSTALLSCR:
         DisplayStatus(hwnd, IDST_STATUS_SCRIPTS, NULL);
         break;

      case WM_CREATEOBJ:
         DisplayStatus(hwnd, IDST_STATUS_CREATEOBJ, NULL);
         break;

      case WM_DELOBSFILES:
         DisplayStatus(hwnd, IDST_STATUS_OBSFILES, NULL);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

void DisplayStatus(HWND hwndDlg, ULONG ulStringID, PCHAR pchText)
{
   char pchMessage[500];

   if (ulStringID == 0)
      pchMessage[0]=0;
   else
   {
      if (pchText)
      {
         char pchTemplate[200];

         WinLoadString(hab, NULLHANDLE, ulStringID, sizeof(pchTemplate), pchTemplate);
         sprintf(pchMessage, pchTemplate, pchText);
      }
      else
         WinLoadString(hab, NULLHANDLE, ulStringID, sizeof(pchMessage), pchMessage);
   }
   WinSetDlgItemText(hwndDlg, IDD_MAIN+10, pchMessage);

   return;
}

USHORT MessageBox(HWND hwndOwner, ULONG ulMessageID, ULONG ulStyle)
{
   char pchMessage[500];

   WinLoadString(hab, NULLHANDLE, ulMessageID, sizeof(pchMessage), pchMessage);

   return WinMessageBox(HWND_DESKTOP, hwndOwner, pchMessage, NULL, IDD_ERROR,
                        MB_ERROR | MB_MOVEABLE | ulStyle);
}

void _Optlink FilReadThread(PVOID pParam)
{
   HWND hwndMain = (HWND) pParam;
   FILE *pfData;
   int i;
   char pchFileName[300];
   ULONG ulEntryID;
   PDIRENTRY pNewEntry;
   BOOL bError=FALSE;
   BOOL bReadFData=TRUE;

   if (pfData = fopen(pchFilesName, "rb"))
   {
      /* Kennung lesen */
      fread(&ulFileID, sizeof(ulFileID), 1, pfData);

      if (ulFileID != REQUIRED_FILEID)
      {
         WinPostMsg(hwndMain, WM_FILERROR, NULL, NULL);
         fclose(pfData);
         return;
      }

      while(!feof(pfData) && !bError)
      {
         /* File-Kennung lesen */
         fread(&ulEntryID, sizeof(ulEntryID), 1, pfData);

         bReadFData = TRUE;

         if (ulEntryID == FILETYPE_NONE)
            break;
         else
         {
            /* Neuer Eintrag */

            pNewEntry=calloc(1, sizeof(DIRENTRY));
            pNewEntry->next = pDirectory;
            pDirectory = pNewEntry;

            pNewEntry->ulEntryID = ulEntryID;

            switch(ulEntryID)
            {
               case FILETYPE_FILE:
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfData);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  pNewEntry->pchFileName = strdup(pchFileName);
                  break;

               case FILETYPE_SCRIPT:
               case FILETYPE_READ:
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfData);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  pNewEntry->pchFileName = strdup(pchFileName);
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfData);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  pNewEntry->pchParam1 = strdup(pchFileName);
                  break;

               case FILETYPE_LANG:
               case FILETYPE_HLP:
               case FILETYPE_DOC:
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfData);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  pNewEntry->pchParam1 = strdup(pchFileName);
                  break;

               case FILETYPE_OBSFILE:
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfData);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  pNewEntry->pchFileName = strdup(pchFileName);
                  bReadFData = FALSE;
                  break;

               default:
                  bError=TRUE;
                  break;
            }
            if (!bError && bReadFData)
            {
               fread(&pNewEntry->FileDate, sizeof(FDATE), 1, pfData);
               fread(&pNewEntry->FileTime, sizeof(FTIME), 1, pfData);
               fread(&pNewEntry->ulFlags,  sizeof(ULONG), 1, pfData);
               fread(&pNewEntry->ulLengthData, sizeof(pNewEntry->ulLengthData), 1, pfData);
               pNewEntry->ulOffsetData=ftell(pfData);
               fseek(pfData, pNewEntry->ulLengthData, SEEK_CUR);
            }
         }
      }
      fclose(pfData);
      if (pDirectory && !bError)
      {
         WinPostMsg(hwndMain, WM_FILESREADY, NULL, NULL);
      }
      else
         WinPostMsg(hwndMain, WM_FILERROR, NULL, NULL);
   }
   else
      WinPostMsg(hwndMain, WM_FILNOTFOUND, NULL, NULL);

   return;
}

MRESULT EXPENTRY ReadmeProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PCHAR pchReadmeText;
   PCHAR pchTemp;

   switch(msg)
   {
      case WM_INITDLG:
         WinSetPresParam(hwnd, PP_FONTNAMESIZE, strlen(MONO_FONT)+1, MONO_FONT);
         _beginthread(ReadReadme, NULL, 16384, (PVOID) hwnd);
         break;

      case WM_CLOSE:
         if (!WinIsWindowEnabled(WinWindowFromID(hwnd, DID_OK)))
            return (MRESULT) FALSE;
         else
            break;

      case WM_READMENOTFOUND:
         MessageBox(hwnd, IDST_MSG_READMENOTFOUND, MB_OK);
         WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);
         break;

      case WM_READMEREAD:
         pchReadmeText = (PCHAR) mp1;
         pchTemp = strtok(pchReadmeText, "\r");
         WinEnableWindowUpdate(WinWindowFromID(hwnd, IDD_README+1), FALSE);
         while (pchTemp)
         {
            if (*pchTemp == '\n')
               pchTemp++;
            WinSendDlgItemMsg(hwnd, IDD_README+1, LM_INSERTITEM,
                              MPFROMSHORT(LIT_END),
                              pchTemp);

            pchTemp = strtok(NULL, "\r");
         }
         WinEnableWindowUpdate(WinWindowFromID(hwnd, IDD_README+1), TRUE);
         free(pchReadmeText);
         WinEnableControl(hwnd, DID_OK, TRUE);
         break;

      case WM_FILNOTFOUND:
         MessageBox(hwnd, IDST_MSG_NOFIL, MB_OK);
         WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

void _Optlink ReadReadme(PVOID pParam)
{
   HWND hwndDlg= (HWND) pParam;
   PDIRENTRY pEntry=pDirectory;
   PCHAR pchReadmeText;
   FILE *pfData;
   size_t bytesread;

   while(pEntry && pEntry->ulEntryID != FILETYPE_READ)
      pEntry=pEntry->next;

   if (pEntry)
   {
      if (pfData=fopen(pchFilesName, "rb"))
      {
         pchReadmeText=malloc(pEntry->ulLengthData+1);
         fseek(pfData, pEntry->ulOffsetData, SEEK_SET);
         bytesread=fread(pchReadmeText, 1, pEntry->ulLengthData, pfData);
         pchReadmeText[bytesread]=0;
         fclose(pfData);

         WinPostMsg(hwndDlg, WM_READMEREAD, pchReadmeText, NULL);
      }
      else
         WinPostMsg(hwndDlg, WM_FILNOTFOUND, NULL, NULL);
   }
   else
      WinPostMsg(hwndDlg, WM_READMENOTFOUND, NULL, NULL);

   return;
}

void _Optlink InstallThread(PVOID pParam)
{
   HWND hwndMain = (HWND) pParam;
   HMQ hmq;
   PCHAR pchDup;
   struct stat filestat;
   int rc=0;
   char pchScriptDir[300];
   PDIRENTRY pEntry;

   bDoingInstall=TRUE;

   /* wegen WinSendMsg */
   hmq = WinCreateMsgQueue(hab, 0);

   /* Installations-Verzeichnis erstellen */
   if (pchInstallDir[strlen(pchInstallDir)-1] == '\\')
      pchInstallDir[strlen(pchInstallDir)-1] = 0;

   WinSendMsg(hwndMain, WM_CREATEDIR, pchInstallDir, NULL);

   while (TRUE)
   {
      pchDup = strdup(pchInstallDir);
      if (_stat(pchDup, &filestat))
          rc=CreatePath(pchDup);
      free(pchDup);

      if (rc)
      {
         switch((USHORT)WinSendMsg(hwndMain, WM_DIRERROR, NULL, NULL))
         {
            case MBID_CANCEL:
               WinSendMsg(hwndMain, WM_INSTALLABORTED, NULL, NULL);
               WinDestroyMsgQueue(hmq);
               bDoingInstall=FALSE;
               return;

            case MBID_RETRY:
               break;
         }
      }
      else
         break;
   }

   /* Alle weiteren Verzeichnisse */

   strcpy(pchScriptDir, pchInstallDir);
   strcat(pchScriptDir, "\\Scripts");

   WinSendMsg(hwndMain, WM_CREATEDIR, pchScriptDir, NULL);
   pchDup = strdup(pchScriptDir);
   if (_stat(pchDup, &filestat))
       rc=CreatePath(pchDup);
   free(pchDup);

   if (rc)
   {
      switch((USHORT)WinSendMsg(hwndMain, WM_DIRERROR, NULL, NULL))
      {
         case MBID_CANCEL:
            WinSendMsg(hwndMain, WM_INSTALLABORTED, NULL, NULL);
            WinDestroyMsgQueue(hmq);
            bDoingInstall=FALSE;
            return;

         case MBID_RETRY:
            break;
      }
   }

   /* Alle Files kopieren */
   pEntry = pDirectory;
   while (pEntry)
   {
      if (pEntry->ulEntryID == FILETYPE_FILE ||
          pEntry->ulEntryID == FILETYPE_READ)
         if (InstallFile(hwndMain, pEntry, pchInstallDir))
         {
            WinDestroyMsgQueue(hmq);
            bDoingInstall=FALSE;
            return;
         }
      pEntry = pEntry->next;
   }

   /* Alle Scripte */
   pEntry = pDirectory;
   while (pEntry)
   {
      if (pEntry->ulEntryID == FILETYPE_SCRIPT)
         if (InstallFile(hwndMain, pEntry, pchScriptDir))
         {
            WinDestroyMsgQueue(hmq);
            bDoingInstall=FALSE;
            return;
         }
      pEntry = pEntry->next;
   }

   /* Language-Files */
   WinSendMsg(hwndMain, WM_INSTALLLANG, NULL, NULL);
   pEntry = pDirectory;
   while (pEntry)
   {
      if (pEntry->ulEntryID == FILETYPE_LANG &&
          !stricmp(pEntry->pchParam1, pchLangSelected))
         InstallLangFile(pEntry, pchInstallDir, "FLEETLNG.DLL");
      if (pEntry->ulEntryID == FILETYPE_HLP &&
          !stricmp(pEntry->pchParam1, pchLangSelected))
         InstallLangFile(pEntry, pchInstallDir, "FLTSTRT.HLP");
      if (pEntry->ulEntryID == FILETYPE_DOC &&
          !stricmp(pEntry->pchParam1, pchLangSelected))
         InstallLangFile(pEntry, pchInstallDir, "FLTSTRT.INF");
      pEntry = pEntry->next;
   }

   /* Scripts.INI */
   InstallScriptsINI(hwndMain, pchInstallDir, pchScriptDir);

   if (bCreateObjects)
   {
      /* WPS-Objekte */
      CreateObjects(hwndMain, pchInstallDir);
   }

   /* obs. Files */
   WinSendMsg(hwndMain, WM_DELOBSFILES, NULL, NULL);
   pEntry = pDirectory;
   while (pEntry)
   {
      if (pEntry->ulEntryID == FILETYPE_OBSFILE)
         DelObsFiles(pchInstallDir, pEntry->pchFileName);

      pEntry = pEntry->next;
   }

   WinSendMsg(hwndMain, WM_INSTALLDONE, NULL, NULL);
   WinDestroyMsgQueue(hmq);

   bDoingInstall=FALSE;
   return;
}

static int CreatePath(char *pchPath)
{
   char *pchPartPath=strdup(pchPath);
   char *pchLastSlash;
   struct stat filestat;

   pchLastSlash = strrchr(pchPartPath, '\\');

   if (pchLastSlash)
   {
      *pchLastSlash = '\0';

      /* nun wurde hinten ein Teil abgeschnitten */
      /* existiert der Pfad bis dorthin? */

      if (_stat(pchPartPath, &filestat))
      {
         /* existiert nicht, weiter oben erzeugen */
         CreatePath(pchPartPath);
      }
   }
   free(pchPartPath);

   return _mkdir(pchPath);
}


int InstallFile(HWND hwndMain, PDIRENTRY pEntry, PCHAR pchPath)
{
   char pchPathName[300];
   FILE *pfData;
   PVOID pBuffer;
   size_t bytesread;

   WinSendMsg(hwndMain, WM_INSTALLFILE, pEntry->pchFileName, NULL);

   strcpy(pchPathName, pchPath);
   strcat(pchPathName, "\\");
   strcat(pchPathName, pEntry->pchFileName);

   if (pfData=fopen(pchFilesName, "rb"))
   {
      fseek(pfData, pEntry->ulOffsetData, SEEK_CUR);
      pBuffer = malloc(pEntry->ulLengthData);
      bytesread = fread(pBuffer, 1, pEntry->ulLengthData, pfData);
      fclose(pfData);

      if (pfData=fopen(pchPathName, "wb"))
      {
         pEntry->pchCompletePath = strdup(pchPathName);
         fwrite(pBuffer, 1, bytesread, pfData);
         fclose(pfData);
         SetFileDate(pchPathName, &pEntry->FileDate, &pEntry->FileTime);
      }
      free(pBuffer);

      return 0;
   }
   else
      return 1;
}

int InstallLangFile(PDIRENTRY pEntry, PCHAR pchPath, PCHAR pchDestFile)
{
   char pchPathName[300];
   FILE *pfData;
   PVOID pBuffer;
   size_t bytesread;

   strcpy(pchPathName, pchPath);
   strcat(pchPathName, "\\");
   strcat(pchPathName, pchDestFile);

   if (pfData=fopen(pchFilesName, "rb"))
   {
      fseek(pfData, pEntry->ulOffsetData, SEEK_CUR);
      pBuffer = malloc(pEntry->ulLengthData);
      bytesread = fread(pBuffer, 1, pEntry->ulLengthData, pfData);
      fclose(pfData);

      if (pfData=fopen(pchPathName, "wb"))
      {
         fwrite(pBuffer, 1, bytesread, pfData);
         fclose(pfData);
         SetFileDate(pchPathName, &pEntry->FileDate, &pEntry->FileTime);
      }
      free(pBuffer);

      return 0;
   }
   else
      return 1;
}

int InstallScriptsINI(HWND hwndMain, PCHAR pchInstallDir, PCHAR pchScriptDir)
{
   char pchScriptsIni[300];
   HINI hini;
   PDIRENTRY pEntry=pDirectory;
   char pchTemp[CCHMAXPATH+1];
   PSTRINGLIST pScriptsList=NULL, pTemp;

   WinSendMsg(hwndMain, WM_INSTALLSCR, NULL, NULL);

   strcpy(pchScriptsIni, pchInstallDir);
   strcat(pchScriptsIni, SCRIPTSINI);

   hini=PrfOpenProfile(hab, pchScriptsIni);

   if (hini)
   {
      ULONG ulSizeKeys;
      PCHAR pchKeys, pchCurrentKey;
      ULONG ulHighID=1;

      /* Keys lesen */
      if (PrfQueryProfileSize(hini, NULL, NULL, &ulSizeKeys))
      {
         if (ulSizeKeys)
         {
            pchKeys = malloc(ulSizeKeys);

            PrfQueryProfileData(hini, NULL, NULL, pchKeys, &ulSizeKeys);
            pchCurrentKey = pchKeys;
            while (*pchCurrentKey)
            {
               if (*pchCurrentKey == '#')
               {
                  ULONG ulTemp = strtoul(pchCurrentKey+1, NULL, 10);

                  if (ulTemp > ulHighID)
                     ulHighID = ulTemp;

                  /* Pfadname lesen */
                  PrfQueryProfileString(hini, pchCurrentKey, "File", "", pchTemp, sizeof(pchTemp));
                  /* in Liste einhaengen */
                  if (pchTemp[0])
                  {
                     pTemp = pScriptsList;
                     pScriptsList = malloc(sizeof(STRINGLIST));
                     pScriptsList->next = pTemp;
                     pScriptsList->pchString = strdup(pchTemp);
                  }
               }
               while (*pchCurrentKey)
                  pchCurrentKey++;
               pchCurrentKey++;
            }
            ulHighID++;

            free(pchKeys);
         }
      }

      /* Nun Script-Eintraege erzeugen */
      while (pEntry)
      {
         if (pEntry->ulEntryID == FILETYPE_SCRIPT)
         {
            char pchScriptName[300];
            char pchApp[20]="#";

            strcpy(pchScriptName, pchScriptDir);
            strcat(pchScriptName, "\\");
            strcat(pchScriptName, pEntry->pchFileName);

            /* Script in den vorhandenen suchen */
            pTemp = pScriptsList;
            while (pTemp && stricmp(pTemp->pchString, pchScriptName))
               pTemp = pTemp->next;

            if (!pTemp)
            {
               _itoa(ulHighID, pchApp+1, 10);

               PrfWriteProfileString(hini, pchApp, "File", pchScriptName);
               PrfWriteProfileString(hini, pchApp, "Name", pEntry->pchParam1);

               ulHighID++;
            }
         }
         pEntry = pEntry->next;
      }

      PrfCloseProfile(hini);
      return 0;
   }
   else
      return 1;
}

int CreateObjects(HWND hwndMain, PCHAR pchInstallDir)
{
   PDIRENTRY pEntry;
   char pchSetupStr[500];

   WinSendMsg(hwndMain, WM_CREATEOBJ, NULL, NULL);

   /* Folder erzeugen */
   WinCreateObject("WPFolder", FOLDERNAME, "OBJECTID=<FleetFldr>", FOLDERLOC,
                   CO_REPLACEIFEXISTS);

   WinCreateObject("WPFolder", DOCNAME, "OBJECTID=<FleetDocs>", "<FleetFldr>",
                   CO_REPLACEIFEXISTS);

   /* Programmobjekt */
   strcpy(pchSetupStr, "OBJECTID=<FleetPRG>;EXENAME=");
   strcat(pchSetupStr, pchInstallDir);
   strcat(pchSetupStr, "\\FLTSTRT.EXE;STARTUPDIR=");
   strcat(pchSetupStr, pchInstallDir);
   WinCreateObject("WPProgram", "FleetStreet", pchSetupStr,
                   "<FleetFldr>", CO_REPLACEIFEXISTS);

   /* Readme */
   pEntry=pDirectory;
   while (pEntry)
   {
      if (pEntry->ulEntryID == FILETYPE_READ)
      {
         strcpy(pchSetupStr, "SHADOWID=");
         strcat(pchSetupStr, pchInstallDir);
         strcat(pchSetupStr, "\\");
         strcat(pchSetupStr, pEntry->pchFileName);
         strcat(pchSetupStr, ";OBJECTID=<");
         strcat(pchSetupStr, pEntry->pchFileName);
         strcat(pchSetupStr, ">");

         WinCreateObject("WPShadow", pEntry->pchFileName, pchSetupStr, "<FleetFldr>", CO_REPLACEIFEXISTS);
      }
      else
         if (pEntry->ulEntryID == FILETYPE_DOC)
         {
            /* View-Objekt */
            strcpy(pchSetupStr, "OBJECTID=<FleetUDoc>;EXENAME=VIEW.EXE;");
            strcat(pchSetupStr, "PARAMETERS=");
            strcat(pchSetupStr, pchInstallDir);
            strcat(pchSetupStr, "\\FLTSTRT.INF;STARTUPDIR=");
            strcat(pchSetupStr, pchInstallDir);
            WinCreateObject("WPProgram", "FleetStreet^User's Guide", pchSetupStr,
                            "<FleetFldr>", CO_REPLACEIFEXISTS);
         }
         else
            if (pEntry->ulEntryID == FILETYPE_FILE &&
                pEntry->ulFlags &&
                pEntry->pchCompletePath)
            {
               strcpy(pchSetupStr, "SHADOWID=");
               strcat(pchSetupStr, pEntry->pchCompletePath);
               strcat(pchSetupStr, ";OBJECTID=<");
               strcat(pchSetupStr, pEntry->pchFileName);
               strcat(pchSetupStr, ">");

               WinCreateObject("WPShadow", pEntry->pchFileName, pchSetupStr, "<FleetDocs>", CO_REPLACEIFEXISTS);
            }
      pEntry=pEntry->next;
   }

   return 0;
}

int SetFileDate(char *pchFileName, PFDATE pDate, PFTIME pTime)
{
   FILESTATUS3 FileStatus;

   if (!DosQueryPathInfo(pchFileName, FIL_STANDARD, &FileStatus, sizeof(FileStatus)))
   {
      FileStatus.fdateLastWrite = *pDate;
      FileStatus.ftimeLastWrite = *pTime;
      if (!DosSetPathInfo(pchFileName, FIL_STANDARD, &FileStatus,
                          sizeof(FileStatus), 0))
         return 0;
      else
         return -1;

   }
   else
      return -1;
}

static void GetInstallDir(char *pchDir, ULONG ulBufLen)
{
   /* Dir aus INI lesen */
   PrfQueryProfileString(HINI_USER, "FleetStreet", "InstallDir", "", pchDir, ulBufLen);

   return;
}

static void StoreInstallDir(char *pchDir)
{
   /* Dir aus INI lesen */
   PrfWriteProfileString(HINI_USER, "FleetStreet", "InstallDir", pchDir);

   return;
}

static void GetInstallLang(char *pchLang, ULONG ulBufLen)
{
   /* Dir aus INI lesen */
   PrfQueryProfileString(HINI_USER, "FleetStreet", "Language", "", pchLang, ulBufLen);

   return;
}

static void StoreInstallLang(char *pchLang)
{
   /* Dir aus INI lesen */
   PrfWriteProfileString(HINI_USER, "FleetStreet", "Language", pchLang);

   return;
}

static int DelObsFiles(PCHAR pchInstallDir, PCHAR pchFileSpec)
{
   char pchSrcMask[300];
   char pchTemp[CCHMAXPATH+1];
   APIRET rc;
   HDIR hDir = HDIR_CREATE;
   FILEFINDBUF3 FindBuf;
   ULONG ulNumFind=1;

   strcpy(pchSrcMask, pchInstallDir);
   strcat(pchSrcMask, "\\");
   strcat(pchSrcMask, pchFileSpec);

   rc = DosFindFirst(pchSrcMask, &hDir, FILE_ARCHIVED, &FindBuf, sizeof(FindBuf),
                     &ulNumFind, FIL_STANDARD);

   while (!rc)
   {
      /* Delete File */
      strcpy(pchTemp, pchInstallDir);
      strcat(pchTemp, "\\");
      strcat(pchTemp, FindBuf.achName);

      DosDelete(pchTemp);

      ulNumFind= 1;
      rc = DosFindNext(hDir, &FindBuf, sizeof(FindBuf), &ulNumFind);
   }

   if (hDir != HDIR_CREATE)
      DosFindClose(hDir);

   return 0;
}
/*-------------------------------- Modulende --------------------------------*/




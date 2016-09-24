/*---------------------------------------------------------------------------+
 | Titel: ATTACHCHECK.C                                                      |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 06.01.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |  Pr’ung der File-Attaches von FleetStreet                                |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#define INCL_PM
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "resids.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "utility.h"
#include "attachcheck.h"

/*--------------------------------- Defines ---------------------------------*/

#define SIZE_FINDBUFFER  2048

/*---------------------------------- Typen ----------------------------------*/

typedef struct _FILELIST {
         struct _FILELIST *next;
         char pchFileName[LEN_PATHNAME+1];
         ULONG ulSize;
         BOOL bFound;
       } FILELIST, *PFILELIST;

typedef struct _CHECKATPAR {
         USHORT cb;
         PFILELIST pFileList;
         ULONG ulCountRecords;
       } CHECKATPAR, *PCHECKATPAR;

typedef struct {
         MINIRECORDCORE RecordCore;
         PCHAR pchFileName;
         ULONG ulSize;
         PCHAR pchStatus;
      } ATTACHRECORD, *PATTACHRECORD;

/*---------------------------- Globale Variablen ----------------------------*/

extern HWND hwndhelp;
extern HMODULE hmodLang;

PFNWP OldAttachContainerProc;

/*----------------------- interne Funktionsprototypen -----------------------*/

static PFILELIST CheckFileMask(char *pchFile);
static MRESULT EXPENTRY AttachStatProc(HWND hwnd, ULONG message, MPARAM mp1,
                                       MPARAM mp2);
static MRESULT EXPENTRY NewAttachContainerProc(HWND hwnd, ULONG message,
                                               MPARAM mp1, MPARAM mp2);


/*---------------------------------------------------------------------------+
 | Funktionsname: CheckAttaches                                              |
 +---------------------------------------------------------------------------+
 | Beschreibung: Prueft die Files, die in dem String enthalten sind          |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Parameter: pchFiles: Filenamen, durch Leerzeichen getrennt                |
 |                      (wie Subject)                                        |
 |            hwndClient: Fensterhandle des Client-Windows                   |
 +---------------------------------------------------------------------------+
 | R…kgabewerte: 0    alle Files OK                                         |
 |                1    keine Files                                           |
 |                2    mindestens ein File nicht OK                          |
 +---------------------------------------------------------------------------+
 | Sonstiges:                                                                |
 |                                                                           |
 +---------------------------------------------------------------------------*/

int CheckAttaches(PCHAR pchFiles, HWND hwndClient)
{
   PFILELIST pFileList=NULL, pLast=NULL;
   PFILELIST pTemp=NULL;
   PCHAR pchSingleFile=NULL;
   int returncode=0;
   CHECKATPAR CheckAtPar;
   ULONG ulCountRecords=0;

   if (pchFiles == NULL ||
       !pchFiles[0])
      return 1;

   pchSingleFile=strtok(pchFiles, " \t");

   if (!pchSingleFile)
      return 1;

   /* Jetzt sind echt Files da */
   do
   {
     /* Filedaten holen */
     pTemp = CheckFileMask(pchSingleFile);

     if (pFileList)
     {
        /* hinten anhaengen */
        pLast->next = pTemp;
     }
     else
        pFileList = pLast = pTemp;

     /* letzten Record wieder suchen */
     while(pLast->next)
        pLast = pLast->next;

   } while (pchSingleFile=strtok(NULL, " \t"));

   /* Ergebnis zaehlen */
   pLast = pFileList;
   while (pLast)
   {
      ulCountRecords++;
      pLast = pLast->next;
   }

   /* Ergebnis anzeigen */
   CheckAtPar.cb=sizeof(CHECKATPAR);
   CheckAtPar.pFileList=pFileList;
   CheckAtPar.ulCountRecords=ulCountRecords;

   WinDlgBox(HWND_DESKTOP, hwndClient, AttachStatProc, hmodLang,
             IDD_ATTACHSTAT, &CheckAtPar);

   /* Ganzen Krempel wieder freigeben */
   while (pFileList)
   {
      pTemp=pFileList->next;
      free(pFileList);
      pFileList=pTemp;
   }

   return returncode;
}


/*---------------------------------------------------------------------------+
 | Funktionsname: CheckFileMask                                              |
 +---------------------------------------------------------------------------+
 | Beschreibung: Prueft ein einzelnes File und traegt die Daten ein          |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Parameter: pchFile: Filename (evtl. mit Wildcards)                        |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | R…kgabewerte: Liste der Files                                            |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Sonstiges:                                                                |
 |                                                                           |
 +---------------------------------------------------------------------------*/

static PFILELIST CheckFileMask(char *pchFile)
{
   PFILELIST pFirst=NULL, pLast=NULL;
   PFILEFINDBUF3 pfindbuf, pResult;
   ULONG ulFindCount=SIZE_FINDBUFFER/sizeof(FILEFINDBUF3);
   HDIR hdir=HDIR_CREATE;

   /* Zeichen f. Delete und Trucate uebergehen */
   if (*pchFile == '^' ||
       *pchFile == '#')
      pchFile++;

   pfindbuf=malloc(SIZE_FINDBUFFER);

   if (DosFindFirst(pchFile, &hdir, FILE_ARCHIVED | FILE_READONLY, pfindbuf,
                    SIZE_FINDBUFFER, &ulFindCount, FIL_STANDARD))
   {
      /* nichts gefunden */
      pFirst = calloc(1, sizeof(FILELIST));

      strcpy(pFirst->pchFileName, pchFile);

      return pFirst;
   }
   do
   {
      pResult = pfindbuf;
      do
      {
         /* gefundenes File einhaengen */
         if (pFirst)
         {
            pLast->next = calloc(1, sizeof(FILELIST));
            pLast = pLast->next;
         }
         else
            pFirst = pLast = calloc(1, sizeof(FILELIST));

         if (pLast) /* only when allocated */
         {
            strcpy(pLast->pchFileName, pResult->achName);
            pLast->ulSize = pResult->cbFile;
            pLast->bFound = TRUE;
         }

         /* weiter im Suchpuffer */
         if (pResult->oNextEntryOffset)
            pResult = (PFILEFINDBUF3)(((PCHAR) pResult) + pResult->oNextEntryOffset);
         else
            pResult = NULL;
      } while (pResult);

      ulFindCount=SIZE_FINDBUFFER/sizeof(FILEFINDBUF3);
   } while (DosFindNext(hdir, pfindbuf, SIZE_FINDBUFFER, &ulFindCount)<ERROR_NO_MORE_FILES);

   DosFindClose(hdir);

   free(pfindbuf);

   return pFirst;
}

/*---------------------------------------------------------------------------+
 | Funktionsname: AttachStatProc                                             |
 +---------------------------------------------------------------------------+
 | Beschreibung: Fensterprozedur des Ergebnisfensters                        |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Parameter: (Window-Procedure)                                             |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | R…kgabewerte: MRESULT                                                    |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Sonstiges:                                                                |
 |                                                                           |
 +---------------------------------------------------------------------------*/

static MRESULT EXPENTRY AttachStatProc(HWND hwnd, ULONG message, MPARAM mp1,
                                       MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS windowfonts;
   extern HWND frame;
   extern HWND hwndhelp;
   PCHECKATPAR pCheckPar=NULL;
   PFIELDINFO pFieldInfo, pFirstFieldInfo;
   FIELDINFOINSERT FieldInfoInsert;
   CNRINFO CnrInfo;
   PATTACHRECORD pRecord, pFirstRecord;
   RECORDINSERT RecordInsert;
   HWND hwndCnr;
   PFILELIST pTemp;

   static char pchTitleFile[50];
   static char pchTitleSize[50];
   static char pchTitleStatus[50];
   static char pchOK[50];
   static char pchNotFound[50];

   switch(message)
   {
      case WM_INITDLG:
         pCheckPar=(PCHECKATPAR) mp2;
         LoadString(IDST_ATT_TITLEFILE,   50, pchTitleFile);
         LoadString(IDST_ATT_TITLESIZE,   50, pchTitleSize);
         LoadString(IDST_ATT_TITLESTATUS, 50, pchTitleStatus);
         LoadString(IDST_ATT_OK,          50, pchOK);
         LoadString(IDST_ATT_NOTF,        50, pchNotFound);

         WinAssociateHelpInstance(hwndhelp, hwnd);

         hwndCnr=WinWindowFromID(hwnd, IDD_ATTACHSTAT+2);
         OldAttachContainerProc=WinSubclassWindow(hwndCnr,
                                                  NewAttachContainerProc);

         SetFont(hwndCnr, windowfonts.attachfont);
         SetForeground(hwndCnr, &windowcolors.attachfore);
         SetBackground(hwndCnr, &windowcolors.attachback);

         /* Felder des Containers vorbereiten */
         pFirstFieldInfo=(PFIELDINFO)SendMsg(hwndCnr,
                                                CM_ALLOCDETAILFIELDINFO,
                                                MPFROMLONG(3), NULL);

         pFieldInfo=pFirstFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR;
         pFieldInfo->flTitle=0;
         pFieldInfo->pTitleData= pchTitleFile;
         pFieldInfo->offStruct= FIELDOFFSET(ATTACHRECORD, pchFileName);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_ULONG | CFA_HORZSEPARATOR | CFA_SEPARATOR |
                            CFA_RIGHT;
         pFieldInfo->flTitle=0;
         pFieldInfo->pTitleData= pchTitleSize;
         pFieldInfo->offStruct= FIELDOFFSET(ATTACHRECORD, ulSize);
         pFieldInfo=pFieldInfo->pNextFieldInfo;

         pFieldInfo->cb=sizeof(FIELDINFO);
         pFieldInfo->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR |
                            CFA_CENTER;
         pFieldInfo->flTitle=0;
         pFieldInfo->pTitleData= pchTitleStatus;
         pFieldInfo->offStruct= FIELDOFFSET(ATTACHRECORD, pchStatus);

         /* Felder des Containers einfuegen */
         FieldInfoInsert.cb=sizeof(FIELDINFOINSERT);
         FieldInfoInsert.pFieldInfoOrder=(PFIELDINFO) CMA_FIRST;
         FieldInfoInsert.fInvalidateFieldInfo=TRUE;
         FieldInfoInsert.cFieldInfoInsert=3;

         SendMsg(hwndCnr, CM_INSERTDETAILFIELDINFO,
                    pFirstFieldInfo, &FieldInfoInsert);

         /* Container-Attribute setzen */
         CnrInfo.cb=sizeof(CNRINFO);
         CnrInfo.pFieldInfoLast=NULL;
         CnrInfo.flWindowAttr=CV_DETAIL | CA_DETAILSVIEWTITLES;
         CnrInfo.xVertSplitbar=0;

         SendMsg(hwndCnr, CM_SETCNRINFO, &CnrInfo,
                    MPFROMLONG(CMA_FLWINDOWATTR));

         /* Elemente einfuegen */

         pFirstRecord=(PATTACHRECORD)SendMsg(hwndCnr, CM_ALLOCRECORD,
                           MPFROMLONG(sizeof(ATTACHRECORD)-sizeof(MINIRECORDCORE)),
                           MPFROMLONG(pCheckPar->ulCountRecords));
         pRecord=pFirstRecord;
         pTemp=pCheckPar->pFileList;
         while(pTemp)
         {
            pRecord->pchFileName=pTemp->pchFileName;
            pRecord->ulSize=pTemp->ulSize;
            if (pTemp->bFound)
                pRecord->pchStatus = pchOK;
            else
                pRecord->pchStatus = pchNotFound;

            pRecord=(PATTACHRECORD)pRecord->RecordCore.preccNextRecord;
            pTemp=pTemp->next;
         }

         RecordInsert.cb=sizeof(RECORDINSERT);
         RecordInsert.pRecordOrder=(PRECORDCORE) CMA_FIRST;
         RecordInsert.pRecordParent=NULL;
         RecordInsert.fInvalidateRecord=TRUE;
         RecordInsert.zOrder=CMA_TOP;
         RecordInsert.cRecordsInsert=pCheckPar->ulCountRecords;

         SendMsg(hwndCnr, CM_INSERTRECORD, pFirstRecord, &RecordInsert);

         RestoreWinPos(hwnd, &windowpositions.attachpos, FALSE, TRUE);
         break;

      case WM_DESTROY:
         QueryWinPos(hwnd, &windowpositions.attachpos);
         QueryFont(WinWindowFromID(hwnd, IDD_ATTACHSTAT+2),
                   windowfonts.attachfont);
         QueryForeground(WinWindowFromID(hwnd, IDD_ATTACHSTAT+2),
                         &windowcolors.attachfore);
         QueryBackground(WinWindowFromID(hwnd, IDD_ATTACHSTAT+2),
                         &windowcolors.attachback);
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------+
 | Funktionsname: NewAttachContainerProc                                     |
 +---------------------------------------------------------------------------+
 | Beschreibung: Neue Window-Procedure f. Container (wg. OS/2-Bug)           |
 +---------------------------------------------------------------------------+
 | Parameter: (Window-Procedure)                                             |
 +---------------------------------------------------------------------------+
 | R…kgabewerte: MRESULT                                                    |
 +---------------------------------------------------------------------------+
 | Sonstiges:                                                                |
 +---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewAttachContainerProc(HWND hwnd, ULONG message,
                                               MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
      case DM_DRAGOVER:
         DrgAccessDraginfo(mp1);
         break;

      default:
         break;
   }
   return OldAttachContainerProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: GetAttachedFiles                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Namen der Angehaengten Files per File-Dialog       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndOwner: Owner-Window fuer File-Dialog.                      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: TRUE: Files angehaengt                                     */
/*                FALSE: keine Files angehaengt (z.B. Abbruch)               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

BOOL GetAttachedFiles(HWND hwndOwner, char *pchSubject, char *pchDefaultDir)
{
   FILEDLG FileDlg;
   BOOL rc=FALSE;

   memset(&FileDlg, 0, sizeof(FileDlg));

   FileDlg.cbSize = sizeof(FileDlg);
   FileDlg.fl = FDS_CENTER | FDS_HELPBUTTON | FDS_MULTIPLESEL | FDS_OPEN_DIALOG;
   FileDlg.pszTitle = calloc(1, 200);
   LoadString(IDST_TITLE_ATTACHFILES, 200, FileDlg.pszTitle);

   if (pchDefaultDir[0])
   {
      strcpy(FileDlg.szFullFile, pchDefaultDir);
      strcat(FileDlg.szFullFile, "\\*");
   }

   if (WinFileDlg(HWND_DESKTOP, hwndOwner, &FileDlg) &&
       FileDlg.lReturn == DID_OK)
   {
      if (FileDlg.ulFQFCount && FileDlg.papszFQFilename)
      {
         int iFile=0;

         while (iFile < FileDlg.ulFQFCount)
         {
            if (strlen(pchSubject) +
                strlen(*FileDlg.papszFQFilename[iFile]) +
                (*pchSubject ? 1:0) <= LEN_SUBJECT)  /* noch genug Platz */
            {
               /* Name anhaengen */
               if (*pchSubject)
                  strcat(pchSubject, " ");
               strcat(pchSubject, *FileDlg.papszFQFilename[iFile]);
               iFile++;
            }
            else
               break; /* kein Platz, sofort beenden */
         }

         WinFreeFileDlgList(FileDlg.papszFQFilename);
         rc= TRUE;
      }
   }

   free(FileDlg.pszTitle);
   return rc;
}

/*-------------------------------- Modulende --------------------------------*/


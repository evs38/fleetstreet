/*---------------------------------------------------------------------------+
 | Titel: UTILITY.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 25.08.93                    |
 +-----------------------------------------+---------------------------------+
 | System:  OS/2 2.x PM                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |      Hilfsfunktionen fÅr Fleet Street                                     |
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
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>
#include <sys\stat.h>

#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogs.h"
#include "dialogids.h"
#include "utility.h"
#include "setupdlg.h"
#include "handlemsg\handlemsg.h"
#include "util\fltutil.h"
#include "util\addrcnv.h"
#include "mainwindow.h"

/*--------------------------------- Defines ---------------------------------*/

#define TAB_FONT    "8.Helv"
#define RGB_GREY    0x00cccccc

#define INSTANCESEM "\\SEM32\\FleetStreet"

/*---------------------------- Globale Variablen ----------------------------*/

extern HMODULE hmodLang;
extern HAB anchor;

static unsigned char ReplaceTable[256][5];

/*--------------------------- Funktionsprototypen ---------------------------*/

static void WriteSeparator(FILE *outfile);

/*------------------------------ QueryCurrent   -----------------------------*/
/* Gibt die aktuelle Message zurueck, im Fehlerfall 0.                       */
/*---------------------------------------------------------------------------*/

int QueryCurrent(PAREALIST AreaList, PCHAR tag)
{
   AREADEFLIST *zeiger;

   zeiger=AM_FindArea(AreaList, tag);
   if (zeiger)
      return zeiger->currentmessage;
   else
      return 0;
}

/*------------------------------ WriteEchoToss  -----------------------------*/
/* Schreibt wenn noetig das Echotoss-Logfile                                 */
/*---------------------------------------------------------------------------*/

int WriteEchotoss(PAREALIST AreaList, PCHAR pchEchoToss)
{
   FILE *fileEchoToss=NULL;
   AREADEFLIST *zeiger;

   /* Altes File einlesen */
   if (fileEchoToss = fopen(pchEchoToss, "r"))
   {
      char pchEcho[LEN_AREATAG+1];

      while (!feof(fileEchoToss))
      {
         if (fgets(pchEcho, sizeof(pchEcho), fileEchoToss))
         {
            StripWhitespace(pchEcho);
            if (pchEcho[0])
               if (zeiger = AM_FindArea(AreaList, pchEcho))
                  zeiger->mailentered = TRUE;
         }
      }
      fclose(fileEchoToss);
      fileEchoToss=NULL;
   }

   zeiger=AreaList->pFirstArea;
   while(zeiger)
   {
      if (zeiger->mailentered &&
          (zeiger->areadata.ulAreaOpt & AREAOPT_FROMCFG) &&
          zeiger->areadata.areatype == AREATYPE_ECHO)
      {
         if (!fileEchoToss)
            if ((fileEchoToss=fopen(pchEchoToss, "w"))==NULL)
               return 1;
         fprintf(fileEchoToss, "%s\n", zeiger->areadata.areatag);
         zeiger->mailentered=FALSE;
      }
      zeiger=zeiger->next;
   }
   if (fileEchoToss)
      fclose(fileEchoToss);

   return 0;
}

/*------------------------------ AlreadyRunning -----------------------------*/
/* Erkennt, ob Fleet Street bereits laeuft                                   */
/*---------------------------------------------------------------------------*/

BOOL AlreadyRunning(void)
{
   HEV hev;

   switch(DosCreateEventSem(INSTANCESEM, &hev, 0, FALSE))
   {
      case NO_ERROR:
         return FALSE; /* Sem existiert noch nicht: wir sind die ersten */

      case ERROR_DUPLICATE_NAME:
         return TRUE;  /* Sem gibt es schon: wir sind nur zweiter */

      default:
         return FALSE;
   }
}

/*-----------------------------  QueryWinPos   ------------------------------*/
/* Fragt die Fensterposition eines Fensters ab                               */
/*---------------------------------------------------------------------------*/

BOOL QueryWinPos(HWND hwnd, WINPOS *pWinPos)
{
   SWP swp;

   WinQueryWindowPos(hwnd, &swp);
   if ((swp.fl & SWP_MAXIMIZE) || (swp.fl & SWP_MINIMIZE))
   {
      pWinPos->x=(SHORT)WinQueryWindowUShort(hwnd, QWS_XRESTORE);
      pWinPos->y=(SHORT)WinQueryWindowUShort(hwnd, QWS_YRESTORE);
      pWinPos->cx=(SHORT)WinQueryWindowUShort(hwnd, QWS_CXRESTORE);
      pWinPos->cy=(SHORT)WinQueryWindowUShort(hwnd, QWS_CYRESTORE);
   }
   else
   {
      pWinPos->x= swp.x;
      pWinPos->y= swp.y;
      pWinPos->cx= swp.cx;
      pWinPos->cy= swp.cy;
   }
   if (swp.fl & SWP_MAXIMIZE)
      pWinPos->uchFlags |= WINPOS_MAX;
   else
      pWinPos->uchFlags &= ~WINPOS_MAX;

   if (swp.fl & SWP_MINIMIZE)
      pWinPos->uchFlags |= WINPOS_MIN;
   else
      pWinPos->uchFlags &= ~WINPOS_MIN;

   pWinPos->uchFlags |= WINPOS_VALID;

   return TRUE;
}

/*----------------------------- RestoreWinPos   -----------------------------*/
/* Setzt die Fensterposition eines Fensters                                  */
/*---------------------------------------------------------------------------*/

BOOL RestoreWinPos(HWND hwnd, WINPOS *pWinPos, BOOL SizeIt, BOOL bShow)
{
   BOOL bTemp;

   if (pWinPos->uchFlags & WINPOS_VALID)
   {
      ULONG flags = SWP_ZORDER | SWP_MOVE | SWP_SIZE;

      if (pWinPos->uchFlags & WINPOS_MAX)
         flags |= SWP_MAXIMIZE;
      else
         if (pWinPos->uchFlags & WINPOS_MIN)
            flags |= SWP_MINIMIZE;

      if (SizeIt)
      {
         bTemp=WinSetWindowPos(hwnd,
                               HWND_TOP,
                               pWinPos->x, pWinPos->y,
                               pWinPos->cx, pWinPos->cy,
                               flags);
      }
      else
         bTemp=WinSetWindowPos(hwnd,
                               HWND_TOP,
                               pWinPos->x, pWinPos->y,
                               0, 0,
                               SWP_MOVE | SWP_ZORDER);
   }

   if (bShow)
      WinShowWindow(hwnd, TRUE);

   return bTemp;
}

/*----------------------------- QueryForeground -----------------------------*/
/* Fragt die Vordergrundfarbe eines Fensters ab                              */
/*---------------------------------------------------------------------------*/

void QueryForeground(HWND hwnd, LONG *lColor)
{
   WinQueryPresParam(hwnd,
                     PP_FOREGROUNDCOLOR,
                     PP_FOREGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     lColor,
                     QPF_ID2COLORINDEX);
   return;
}

void QueryControlForeground(HWND hwndParent, ULONG id, LONG *lColor)
{
   QueryForeground(WinWindowFromID(hwndParent, id), lColor);
}

/*------------------------------ SetForeground ------------------------------*/
/* Setzt die Vordergrundfarbe eines Fensters                                 */
/*---------------------------------------------------------------------------*/

void SetForeground(HWND hwnd, LONG *lColor)
{
   WinSetPresParam(hwnd,
                   PP_FOREGROUNDCOLOR,
                   sizeof(LONG),
                   lColor);
   return;
}

/*----------------------------- QueryBackground -----------------------------*/
/* Fragt die Hintergrundfarbe eines Fensters ab                              */
/*---------------------------------------------------------------------------*/

void QueryBackground(HWND hwnd, LONG *lColor)
{
   WinQueryPresParam(hwnd,
                     PP_BACKGROUNDCOLOR,
                     PP_BACKGROUNDCOLORINDEX,
                     NULL,
                     sizeof(LONG),
                     lColor,
                     QPF_ID2COLORINDEX);
   return;
}

void QueryControlBackground(HWND hwndParent, ULONG id, LONG *lColor)
{
   QueryBackground(WinWindowFromID(hwndParent, id), lColor);
}

/*------------------------------ SetBackground ------------------------------*/
/* Setzt die Hintergrundfarbe eines Fensters                                 */
/*---------------------------------------------------------------------------*/

void SetBackground(HWND hwnd, LONG *lColor)
{
   WinSetPresParam(hwnd,
                   PP_BACKGROUNDCOLOR,
                   sizeof(LONG),
                   lColor);
   return;
}

/*----------------------------- QueryFont       -----------------------------*/
/* Fragt den Font eines Fensters ab                                          */
/*---------------------------------------------------------------------------*/

void QueryFont(HWND hwnd, char *pchFacename)
{
   WinQueryPresParam(hwnd,
                     PP_FONTNAMESIZE,
                     0L,
                     NULL,
                     FACESIZE+5,
                     pchFacename,
                     0L);
   return;
}

void QueryControlFont(HWND hwndParent, ULONG id, char *pchFacename)
{
   QueryFont(WinWindowFromID(hwndParent, id), pchFacename);
}

/*----------------------------- SetFont         -----------------------------*/
/* Setzt den Font eines Fensters                                             */
/*---------------------------------------------------------------------------*/

void SetFont(HWND hwnd, char *pchFacename)
{
   if (pchFacename[0])
      WinSetPresParam(hwnd,
                      PP_FONTNAMESIZE,
                      strlen(pchFacename)+1,
                      pchFacename);
   return;
}

/*------------------------------ ImportFile    ------------------------------*/
/* Liesst ein Textfile und fuegt es in den Text ein                          */
/* Returncodes: 0   alles OK                                                 */
/*              1   Fehler beim Einlesen                                     */
/*              2   Falscher Filename                                        */
/*              3   Leeres File                                              */
/*              4   Abbruch durch User                                       */
/*---------------------------------------------------------------------------*/

#define READ_BUFFER    20000
#define IMPORT_BUFFER  65000

int ImportFile(HWND hwndClient, PCHAR pchLastFileName, BOOL bConvert, BOOL bAsk)
{
   FILE *impfile;
   int iFile;
   char *textbuf;
   char namebuf[LEN_PATHNAME+1];
   PCHAR insptr;
   int i, iDest;
   IPT ipt=-1;
   int rc=0;
   int iRead;

   strcpy(namebuf, pchLastFileName);

   if (bAsk && GetPathname(hwndClient, namebuf)!=DID_OK)
      return 4;                   /* Userabbruch */

   /* File einlesen */
   iFile = _sopen(namebuf, O_RDONLY|O_BINARY, SH_DENYNO, S_IREAD|S_IWRITE);
   if (iFile == -1)
      return 2;

   if ((impfile = _fdopen(iFile, "rb"))==NULL)
   {
      /* Fehler beim ôffnen */
      _close(iFile);
      return 2;
   }

   textbuf=calloc(1, READ_BUFFER);
   DosAllocMem((PPVOID)&insptr, IMPORT_BUFFER, OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE);

   WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_DISABLEREFRESH, NULL, NULL);

   if (bConvert)
   {
      /* Tabelle vorbereiten */
      memset(ReplaceTable, 0, sizeof(ReplaceTable));

      strcpy(ReplaceTable['†'], "a");
      strcpy(ReplaceTable['É'], "a");
      strcpy(ReplaceTable['Ñ'], "ae");
      strcpy(ReplaceTable['é'], "Ae");
      strcpy(ReplaceTable['Ö'], "a");
      strcpy(ReplaceTable['ë'], "ae");
      strcpy(ReplaceTable['í'], "Ae");
      strcpy(ReplaceTable['‡'], "a");
      strcpy(ReplaceTable['è'], "A");
      strcpy(ReplaceTable['Ü'], "a");
      strcpy(ReplaceTable['·'], "ss");
      strcpy(ReplaceTable['á'], "c");
      strcpy(ReplaceTable['Ä'], "C");
      strcpy(ReplaceTable['Ø'], ">>");
      strcpy(ReplaceTable['¯'], "o");
      strcpy(ReplaceTable['ˆ'], "/");
      strcpy(ReplaceTable['˙'], ".");
      strcpy(ReplaceTable[''], "|");
      strcpy(ReplaceTable['Ç'], "e");
      strcpy(ReplaceTable['ê'], "E");
      strcpy(ReplaceTable['à'], "e");
      strcpy(ReplaceTable['â'], "e");
      strcpy(ReplaceTable['ä'], "e");
      strcpy(ReplaceTable['ü'], "f");
      strcpy(ReplaceTable[''], " ");
      strcpy(ReplaceTable['°'], "i");
      strcpy(ReplaceTable['å'], "i");
      strcpy(ReplaceTable['ã'], "i");
      strcpy(ReplaceTable['ç'], "i");
      strcpy(ReplaceTable['≠'], "!");
      strcpy(ReplaceTable['®'], "?");
      strcpy(ReplaceTable[''], "<-");
      strcpy(ReplaceTable['™'], "-|");
      strcpy(ReplaceTable['Ê'], "mc");
      strcpy(ReplaceTable['§'], "n");
      strcpy(ReplaceTable['•'], "N");
      strcpy(ReplaceTable['¢'], "o");
      strcpy(ReplaceTable['ì'], "o");
      strcpy(ReplaceTable['ï'], "o");
      strcpy(ReplaceTable['î'], "oe");
      strcpy(ReplaceTable['ô'], "Oe");
      strcpy(ReplaceTable['¨'], "1/4");
      strcpy(ReplaceTable['´'], "1/2");
      strcpy(ReplaceTable['Æ'], "<<");
      strcpy(ReplaceTable['Ò'], "+-");
      strcpy(ReplaceTable['ú'], "UKP");
      strcpy(ReplaceTable['˝'], "qd");
      strcpy(ReplaceTable['¸'], "trp");
      strcpy(ReplaceTable['£'], "u");
      strcpy(ReplaceTable['ñ'], "u");
      strcpy(ReplaceTable['ó'], "u");
      strcpy(ReplaceTable['Å'], "ue");
      strcpy(ReplaceTable['ö'], "Ue");
      strcpy(ReplaceTable['¶'], "a");
      strcpy(ReplaceTable['ß'], "o");
      strcpy(ReplaceTable['ò'], "y");

      for (i=180; i<=218; i++)
         strcpy(ReplaceTable[i], "+");

      strcpy(ReplaceTable[179], "|");
      strcpy(ReplaceTable[186], "|");
      strcpy(ReplaceTable[196], "-");
      strcpy(ReplaceTable[205], "-");

      strcpy(ReplaceTable[176], "#");
      strcpy(ReplaceTable[177], "#");
      strcpy(ReplaceTable[178], "#");
      strcpy(ReplaceTable[219], "#");
      strcpy(ReplaceTable[220], "#");
      strcpy(ReplaceTable[221], "#");
      strcpy(ReplaceTable[222], "#");
      strcpy(ReplaceTable[223], "#");
   }

   while (!feof(impfile))
   {
      if ((iRead = fread(textbuf, 1, READ_BUFFER, impfile)) > 0)
      {
         if (bConvert)
         {
            /* konvertieren */

            i=0;
            iDest=0;
            while((iDest < IMPORT_BUFFER) && (i < iRead))
            {
               if (ReplaceTable[textbuf[i]][0]==0)
                  insptr[iDest++]=textbuf[i];
               else
               {
                  strcpy(insptr+iDest, ReplaceTable[textbuf[i]]);
                  iDest+=strlen(ReplaceTable[textbuf[i]]);
               }
               i++;
            }
            insptr[iDest]='\0';
         }
         else
         {
            /* unkonvertiert uebernehmen */
            memcpy(insptr, textbuf, iRead);
            insptr[iRead]=0;
         }

         /* Portion einfuegen */
         WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_FORMAT,
                           MPFROMSHORT(MLFIE_CFTEXT), NULL);
         WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_SETIMPORTEXPORT,
                           insptr, MPFROMLONG(IMPORT_BUFFER));
         WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_IMPORT,
                           &ipt, MPFROMLONG(strlen(insptr)));
      }
      else
         if (ferror(impfile))
         {
            rc=1;
            break;
         }
   }
   fclose(impfile);

   WinSendDlgItemMsg(hwndClient, IDML_MAINEDIT, MLM_ENABLEREFRESH, NULL, NULL);

   strcpy(pchLastFileName, namebuf);

   DosFreeMem(insptr);
   free(textbuf);

   return rc;
}

/*----------------------------- ExportFile      -----------------------------*/
/* Exportiert eine Message in ein File                                       */
/* Returncodes: 0   alles OK                                                 */
/*              1   Fehler beim Schreiben                                    */
/*              2   Falscher Filename                                        */
/*              3   Abbruch durch User                                       */
/*---------------------------------------------------------------------------*/

int ExportFile(HWND hwndOwner, PCHAR pchLastExport, BOOL bAsk, PULONG pulOptions)
{
   extern FTNMESSAGE CurrentMessage;
   extern MSGHEADER  CurrentHeader;
   extern char CurrentArea[LEN_AREATAG+1];

   if (bAsk)
   {
      if (GetExportName(hwndOwner, pchLastExport, pulOptions))
      {
         return WriteMessage(pchLastExport, &CurrentMessage, &CurrentHeader, CurrentArea,
                             *pulOptions);
      }
      else
         return 3;
   }
   else
      return WriteMessage(pchLastExport, &CurrentMessage, &CurrentHeader, CurrentArea,
                          EXPORT_WITHHEADER | EXPORT_APPEND);
}

/*---------------------------- GetExportName    -----------------------------*/
/* Holt Dateinamen zum Export                                                */
/*---------------------------------------------------------------------------*/

BOOL GetExportName(HWND hwndOwner, PCHAR pchFileName, PULONG pulExportOptions)
{
   FILEDLG dlgpar;

   dlgpar.cbSize=sizeof(dlgpar);
   dlgpar.fl= FDS_CENTER | FDS_CUSTOM | FDS_HELPBUTTON |
              FDS_SAVEAS_DIALOG | FDS_ENABLEFILELB;
   dlgpar.pszTitle="File";
   dlgpar.pszOKButton="OK";
   dlgpar.pfnDlgProc=ExportProc;
   dlgpar.pszIType=NULL;
   dlgpar.papszITypeList=NULL;
   dlgpar.pszIDrive=NULL;
   dlgpar.papszIDriveList=NULL;
   dlgpar.hMod=hmodLang;
   strcpy(dlgpar.szFullFile, pchFileName);
   dlgpar.usDlgId=IDD_EXPORTFILE;
   dlgpar.ulUser = *pulExportOptions;

   WinFileDlg(HWND_DESKTOP,
              hwndOwner,
              &dlgpar);

   if (dlgpar.lReturn != DID_OK)
      return FALSE;

   strcpy(pchFileName, dlgpar.szFullFile);
   *pulExportOptions = dlgpar.ulUser;

   return TRUE;
}

/*----------------------------- WriteMessage    -----------------------------*/
/* Schreibt eine Message auf Disk                                            */
/* Returncodes: 0   alles OK                                                 */
/*              1   Fehler beim Schreiben                                    */
/*              2   Falscher Filename                                        */
/*---------------------------------------------------------------------------*/

int WriteMessage(PCHAR pchFileName, FTNMESSAGE *Message, MSGHEADER *Header, PCHAR tag,
                 ULONG ulOptions)
{
   FILE *outfile;
   char *lastspace;
   char *position;
   char *linebegin;
   char *runner;

   if (ulOptions & EXPORT_APPEND)
      outfile=fopen(pchFileName, "a");
   else
      outfile=fopen(pchFileName, "w");

   if (!outfile)
      return 2;

   if (ulOptions & EXPORT_WITHHEADER)
   {
      /* Header schreiben */
      char fromtext[10]="";
      char totext[10]="";
      char subjtext[10]="";
      char datetime[22]="";
      char addrtext[LEN_5DADDRESS+1]="";
      AREADEFLIST *zeiger=NULL;
      extern AREALIST arealiste;

      zeiger=AM_FindArea(&arealiste, tag);
      if (zeiger)
      {
         int i,l;

         l=strlen(zeiger->areadata.areadesc);
         for (i=1; i<= (80-l-2)/2; i++)
            fputc('=', outfile);
         fprintf(outfile, " %s ", zeiger->areadata.areadesc);
         for (; i<=(80-l-2); i++)
            fputc('=', outfile);
         fprintf(outfile, "\n");
      }
      else
         WriteSeparator(outfile);

      LoadString(IDST_MW_FROM, 10, fromtext);
      LoadString(IDST_MW_TO,   10, totext);
      LoadString(IDST_MW_SUBJ, 10, subjtext);

      NetAddrToString(addrtext, &Header->FromAddress);
      StampToString(datetime, &Header->StampWritten);
      fprintf(outfile, "%8s %-33s %-15s %s\n", fromtext, Header->pchFromName, addrtext, datetime);
      StampToString(datetime, &Header->StampArrived);
      if (zeiger && zeiger->areadata.areatype == AREATYPE_ECHO)
         addrtext[0]=0;
      else
         NetAddrToString(addrtext, &Header->ToAddress);
      fprintf(outfile, "%8s %-33s %-15s %s\n", totext, Header->pchToName, addrtext, datetime);
      fprintf(outfile, "%8s %-71s\n", subjtext, Header->pchSubject);
      WriteSeparator(outfile);
   }

   /* Messagetext schreiben */
   position=Message->pchMessageText;
   lastspace=position;
   linebegin=position;

   while (*position)
      switch(*position)
      {
         case ' ':
            lastspace=position;
            position++;
            break;

         case '\n':
            for (runner=linebegin; runner<position; runner++)
                putc(*runner, outfile);
            fputc('\n', outfile);
            linebegin=++position;
            break;

         default:
            position++;
            if ((position-linebegin) > 80)
            {
               if (lastspace<linebegin)   /* Letztes Space vor dem Zeilenanfang */
               {
                  /* ueberlange Zeile schreiben */
                  for (runner=linebegin; runner<position; runner++)
                     putc(*runner, outfile);
                  linebegin=position;
               }
               else
               {
                  /* schreiben bis zum letzten Space */
                  for (runner=linebegin; runner<=lastspace; runner++)
                     putc(*runner, outfile);
                  linebegin=lastspace+1;
               }
               fputc('\n', outfile);
            }
            break;
      }
   if (position>=linebegin)
      for (runner=linebegin; runner<position; runner++)
         putc(*runner, outfile);

   fputc('\n', outfile);

   if (ulOptions & EXPORT_SEPARATOR)
      WriteSeparator(outfile);

   fclose(outfile);
   return 0;
}

static void WriteSeparator(FILE *outfile)
{
   fprintf(outfile, "===================="
                    "===================="
                    "===================="
                    "====================\n");
   return;
}


/*----------------------------- Notify --------------------------------------*/
/* Zeigt den angegebenen String in der Statuszeile an                        */
/*---------------------------------------------------------------------------*/

void Notify(HWND hwndOwner, ULONG idString)
{
   char pchTemp[100];

   LoadString(idString, 100, pchTemp);
   WinSetDlgItemText(WinQueryWindow(hwndOwner, QW_PARENT), FID_STATUSLINE, pchTemp);

   return;
}

/*----------------------------- CleanupDomains  -----------------------------*/
/* Lîscht alle Domains                                                       */
/*---------------------------------------------------------------------------*/

void CleanupDomains(PDOMAINS *ppDomains)
{
   PDOMAINS pTemp;

   while(*ppDomains)
   {
      pTemp=(*ppDomains)->next;
      free(*ppDomains);
      (*ppDomains)=pTemp;
   }
   return;
}

/*-------------------------------- QueryDomain  -----------------------------*/
/* Sucht ein Domain                                                          */
/*---------------------------------------------------------------------------*/

PDOMAINS QueryDomain(PDOMAINS domains, char *pchDomainName)
{
   PDOMAINS pTemp=domains;

   while(pTemp && stricmp(pTemp->domainname, pchDomainName))
   {
      pTemp=pTemp->next;
   }
   return pTemp;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: StartShell                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Startet eine Command-Shell                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: -                                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 0  OK                                                      */
/*                1  Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int StartShell(void)
{
   STARTDATA    StartData;    /* Start session data structure */
   ULONG        SessID;       /* Session ID (returned) */
   PID          PID;          /* Process ID (returned) */
   UCHAR ObjBuf[100];         /* Object buffer */
   APIRET       rc;           /* Return code */

    /*  Specify the various session start parameters  */

    StartData.Length = sizeof(STARTDATA);
    StartData.Related = 0;                 /* unrelated */
    StartData.FgBg = 0;                    /* Foreground */
    StartData.TraceOpt = SSF_TRACEOPT_NONE;
    StartData.PgmTitle = "Shell";
    StartData.PgmName = NULL;              /* Default-Shell */
    StartData.PgmInputs = 0;               /* keine Parameter */
    StartData.TermQ = 0;
    StartData.Environment = 0;
    StartData.InheritOpt = SSF_INHERTOPT_SHELL; /* Environment erben */
    StartData.SessionType = SSF_TYPE_WINDOWABLEVIO;
    StartData.IconFile = 0;
    StartData.PgmHandle = 0;
    StartData.PgmControl = SSF_CONTROL_VISIBLE /*| SSF_CONTROL_MAXIMIZE*/;
    StartData.InitXPos = 30;
    StartData.InitYPos = 40;
    StartData.InitXSize = 200;
    StartData.InitYSize = 140;
    StartData.Reserved = 0;
    StartData.ObjectBuffer = ObjBuf;
    StartData.ObjectBuffLen = 100;
    rc = DosStartSession(&StartData, &SessID, &PID);

    if (rc != 0)
       return 1;
    else
       return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MessageBox                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Zeigt eine Message-Box an                                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: -                                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: Return-Wert v. WinMessageBox                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

USHORT MessageBox(HWND hwndOwner, ULONG ulIDMessage, ULONG ulIDTitle,
                  USHORT usWinID, ULONG flStyle)
{
   char message[200];
   char title[200];

   if (ulIDMessage)
      LoadString(ulIDMessage, 200, message);

   if (ulIDTitle)
      LoadString(ulIDTitle, 200, title);

   return WinMessageBox(HWND_DESKTOP, hwndOwner,
                        ulIDMessage?message:NULL,
                        ulIDTitle?title:NULL,
                        usWinID, MB_HELP | MB_MOVEABLE | flStyle);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryNextArea                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sucht die naechste Area mit ungel. Mail                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: arealist: Liste aller Areas                                    */
/*            pchCurrent: Area-Tag der aktuellen Area                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: NULL  keine solche Area gefunden                           */
/*                sonst  Zeiger auf Area-Tag                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

char *QueryNextArea(PAREALIST arealist, char *pchCurrent)
{
   AREADEFLIST *pThisArea=AM_FindArea(arealist, pchCurrent);
   AREADEFLIST *pNextArea=NULL;
   extern HWND hwndAreaDlg;
   extern AREALISTOPTIONS arealistoptions;

   if (pThisArea)
   {
      pNextArea=pThisArea->next;

      while (pNextArea != pThisArea) /* max. einmal rum */
      {
         if (pNextArea == NULL)
            pNextArea=arealist->pFirstArea;  /* wieder von vorne suchen */
         else
         {
            if (pNextArea->scanned &&
                !pNextArea->bLocked &&
                pNextArea->currentmessage < pNextArea->maxmessages)
               return pNextArea->areadata.areatag;
            else
               pNextArea=pNextArea->next;
         }
      }
   }
   return NULL;
}

HWND ReplaceSysMenu(HWND hwndDlg, HWND hwndPopupMenu, USHORT usSubID)
{
   HWND  hSysMenu, hSysSubMenu;
   SHORT idSysMenu;
   MENUITEM SysMenu;
   MENUITEM MenuItem;

   hSysMenu = WinWindowFromID(hwndDlg, FID_SYSMENU);
   idSysMenu = SHORT1FROMMR(SendMsg(hSysMenu, MM_ITEMIDFROMPOSITION, NULL, NULL));
   SendMsg(hSysMenu, MM_QUERYITEM, MPFROM2SHORT(idSysMenu, FALSE),
              MPFROMP(&SysMenu));
   hSysSubMenu = SysMenu.hwndSubMenu;

   SysMenu.hwndSubMenu = hwndPopupMenu;
   SendMsg(hSysMenu, MM_SETITEM, MPFROM2SHORT(0, FALSE),
              MPFROMP(&SysMenu));

   SendMsg(hwndPopupMenu, MM_QUERYITEM, MPFROM2SHORT(usSubID, FALSE),
              &MenuItem);

   MenuItem.afStyle |= MIS_SUBMENU;
   MenuItem.hwndSubMenu = hSysSubMenu;
   SendMsg(hwndPopupMenu, MM_SETITEM, MPFROM2SHORT(0, FALSE),
              &MenuItem);
   WinSetWindowUShort(hwndPopupMenu, QWS_ID, idSysMenu);
   WinSetWindowULong(hwndPopupMenu, QWL_STYLE,
                     WinQueryWindowULong(hSysSubMenu, QWL_STYLE));
   WinSetOwner(hwndPopupMenu, hSysMenu);
   WinSetParent(hwndPopupMenu, hSysMenu, FALSE);

   return hSysSubMenu;
}

void ResetMenuStyle(HWND hwndPopup, HWND hwndDialog)
{
   ULONG ulStyle;
   HWND hwndSys;

   ulStyle = WinQueryWindowULong(hwndPopup, QWL_STYLE);
   ulStyle &= 0xffff0000UL;
   WinSetWindowULong(hwndPopup, QWL_STYLE, ulStyle);
   hwndSys = WinWindowFromID(hwndDialog, FID_SYSMENU);
   WinSetOwner(hwndPopup, hwndSys);
   WinSetParent(hwndPopup, hwndSys, FALSE);

   return;
}

BOOL CalcClientRect(HAB hab, HWND hwndFrame, PRECTL prclResult)
{
   POINTL BorderSize={0, 0};

   SendMsg(hwndFrame, WM_QUERYBORDERSIZE, &BorderSize, NULL);

   WinInflateRect(hab, prclResult, -BorderSize.x, -BorderSize.y);
   prclResult->yTop -= WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);

   return TRUE;
}

BOOL SizeToClient(HAB hab, PSWP pSwp, HWND hwndDialog, ULONG ulControlID)
{
   if (pSwp->fl & (SWP_SIZE|SWP_MINIMIZE|SWP_MAXIMIZE|SWP_RESTORE))
   {
      RECTL rectl;

      rectl.xLeft=0;
      rectl.xRight=pSwp->cx;
      rectl.yBottom=0;
      rectl.yTop=pSwp->cy;

      CalcClientRect(hab, hwndDialog, &rectl);
      WinSetWindowPos(WinWindowFromID(hwndDialog, ulControlID),
                      NULLHANDLE,
                      rectl.xLeft, rectl.yBottom,
                      rectl.xRight-rectl.xLeft, rectl.yTop-rectl.yBottom,
                      SWP_MOVE | SWP_SIZE);
   }
   return TRUE;
}

BOOL SaveWinPos(HWND hwnd, PSWP pSwp, PWINPOS pWinPos, PBOOL pbDirty)
{
   UCHAR fl=WINPOS_VALID;

   if (pSwp->fl & SWP_MAXIMIZE)
      fl |= WINPOS_MAX;
   if (pSwp->fl & SWP_MINIMIZE)
      fl |= WINPOS_MIN;

   if (pSwp->x != pWinPos->x  ||
       pSwp->y != pWinPos->y  ||
       pSwp->cx!= pWinPos->cx ||
       pSwp->cy!= pWinPos->cy ||
       fl != pWinPos->uchFlags)
   {
      QueryWinPos(hwnd, pWinPos);
      if (pbDirty)
         *pbDirty = TRUE;
      return TRUE;
   }
   return FALSE;
}

char *CreateUniqueName(ULONG ulStringID, PVOID pData,
                       int (*CompareFunc)(PVOID, char*),
                       ULONG ulBufferLen, char *pchBuffer)
{
   int iTries=0;

   do
   {
      *pchBuffer=0;
      LoadString(ulStringID, ulBufferLen, pchBuffer);

      if (iTries)
      {
         char pchNum[20];

         strcat(pchBuffer, ":");
         _itoa(iTries, pchNum, 10);
         strcat(pchBuffer, pchNum);
      }
      iTries++;
   } while (CompareFunc(pData, pchBuffer));

   return pchBuffer;
}

void SetNotebookParams(HWND hwndNotebook, USHORT usTabX)
{
   SendMsg(hwndNotebook, BKM_SETDIMENSIONS,
              MPFROM2SHORT(usTabX, 25),
              MPFROMSHORT(BKA_MAJORTAB));

   SendMsg(hwndNotebook, BKM_SETDIMENSIONS,
              MPFROM2SHORT(usTabX, 25),
              MPFROMSHORT(BKA_MINORTAB));

   SendMsg(hwndNotebook, BKM_SETDIMENSIONS,
              MPFROM2SHORT(25,25),
              MPFROMSHORT(BKA_PAGEBUTTON));

   SendMsg(hwndNotebook, BKM_SETNOTEBOOKCOLORS,
              MPFROMLONG(RGB_GREY),
              MPFROMSHORT(BKA_BACKGROUNDPAGECOLOR));

   SendMsg(hwndNotebook, BKM_SETNOTEBOOKCOLORS,
              MPFROMLONG(RGB_GREY),
              MPFROMSHORT(BKA_BACKGROUNDMAJORCOLOR));

   SendMsg(hwndNotebook, BKM_SETNOTEBOOKCOLORS,
              MPFROMLONG(RGB_GREY),
              MPFROMSHORT(BKA_BACKGROUNDMINORCOLOR));

   WinSetPresParam(hwndNotebook, PP_FONTNAMESIZE, sizeof(TAB_FONT), TAB_FONT);

   return;
}

/*------------------------------- InsertOnePage -----------------------------*/
/* Fuegt eine Seite in das Notebook ein                                      */
/* Laden der Dialog-Seite aus Resourcen, Setzen des Tab-Textes, Einfuegen    */
/* der Seite.                                                                */
/*---------------------------------------------------------------------------*/

ULONG InsertOnePage(HWND notebook, ULONG resourceID, ULONG stringID, PFNWP dlgproc, PVOID dlgpar)
{
   HWND hwnddlg;
   ULONG pageID;
   UCHAR tabtext[50];

   pageID=(ULONG)SendMsg(notebook,
                            BKM_INSERTPAGE,
                            (MPARAM) NULL,
                            MPFROM2SHORT(BKA_AUTOPAGESIZE | BKA_MAJOR,
                            BKA_LAST));

   hwnddlg=WinLoadDlg(WinQueryWindow(notebook, QW_PARENT),
                      notebook,
                      dlgproc,
                      hmodLang,
                      resourceID,
                      dlgpar);

   SendMsg(notebook,
              BKM_SETPAGEWINDOWHWND,
              MPFROMLONG(pageID),
              MPFROMHWND(hwnddlg));

   LoadString( stringID, sizeof(tabtext), tabtext);

   SendMsg(notebook,
              BKM_SETTABTEXT,
              MPFROMLONG(pageID),
              (MPARAM) tabtext);

  return pageID;
}


BOOL SetFocus(HWND hwnd)
{
   return WinSetFocus(HWND_DESKTOP, hwnd);
}

BOOL SetFocusControl(HWND hwndParent, ULONG ulID)
{
   return SetFocus(WinWindowFromID(hwndParent, ulID));
}

MRESULT SendMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   return WinSendMsg(hwnd, msg, mp1, mp2);
}

LONG LoadString(ULONG idString, LONG lBufferMax, PSZ pszBuffer)
{
   return WinLoadString(anchor, hmodLang, idString, lBufferMax, pszBuffer);
}

HSWITCH AddToWindowList(HWND hwndDlg)
{
   SWCNTRL SwitchCtl;

   SwitchCtl.hwnd=hwndDlg;
   SwitchCtl.hwndIcon=NULLHANDLE;
   SwitchCtl.hprog=NULLHANDLE;
   SwitchCtl.idProcess=0;
   SwitchCtl.idSession=0;
   SwitchCtl.uchVisibility=SWL_VISIBLE;
   SwitchCtl.fbJump=SWL_JUMPABLE;
   SwitchCtl.bProgType=PROG_PM;
   WinQueryWindowText(hwndDlg, MAXNAMEL+4, SwitchCtl.szSwtitle);

   return WinAddSwitchEntry(&SwitchCtl);
}

ULONG RemoveFromWindowList(HSWITCH hSwitch)
{
   if (hSwitch)
      return WinRemoveSwitchEntry(hSwitch);
   else
      return 0;
}


HPOINTER LoadIcon(ULONG ulIconID)
{
   return WinLoadPointer(HWND_DESKTOP, NULLHANDLE, ulIconID);
}

MRESULT RedirectCommand(MPARAM mp1, MPARAM mp2)
{
   extern HWND client;

   switch(SHORT1FROMMP(mp1))
   {
      case IDA_AREA:
      case IDA_MSGLIST:
      case IDA_THREADLIST:
      case IDA_BOOKMARKS:
      case IDA_NLBROWSER:
      case IDA_KLUDGES:
#if 0
      case IDA_CATCHUP:
      case IDA_NEXTAREA:
#endif
         return SendMsg(client, WM_COMMAND, mp1, mp2);

      default:
         return (MRESULT) NULL;
   }
}

void SwitchAccels(HWND hwndFrame, ULONG ulAccelNum)
{
   extern HACCEL hAccel3;

   switch(ulAccelNum)
   {
      case ACCEL_NONE:
      case ACCEL_WRITE:
         WinSetAccelTable(anchor, NULLHANDLE, hwndFrame);
         break;

      case ACCEL_READ:
         WinSetAccelTable(anchor, hAccel3, hwndFrame);
         break;

      default:
         break;
   }
   return;
}

void SwitchAccelsMain(HWND hwndFrame, ULONG ulAccelNum)
{
   extern HACCEL hAccel1, hAccel2;

   switch(ulAccelNum)
   {
      case ACCEL_NONE:
         WinSetAccelTable(anchor, NULLHANDLE, hwndFrame);
         break;

      case ACCEL_WRITE:
         WinSetAccelTable(anchor, hAccel2, hwndFrame);
         break;

      case ACCEL_READ:
         WinSetAccelTable(anchor, hAccel1, hwndFrame);
         break;

      default:
         break;
   }
   return;
}

void SetInitialAccel(HWND hwndFrame)
{
   extern int CurrentStatus;
   extern HACCEL hAccel3;

   if (CurrentStatus == PROGSTATUS_READING)
      WinSetAccelTable(anchor, hAccel3, hwndFrame);

   return;
}

/*-------------------------------- Modulende --------------------------------*/


/*---------------------------------------------------------------------------+
 | Titel: ECHOMAN_SQ.C                                                       |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 19.04.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Echo-Manager (Squish) f. FleetStreet                                    |
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
#include <stdio.h>
#include <stdlib.h>
#include "..\devkit\echoman.h"
#include "..\util\fltutil.h"
#include "..\util\crc32.h"
#include "echoman_sq.h"

/*--------------------------------- Defines ---------------------------------*/

#define FORMAT_SQUISH    0
#define FORMAT_SDM       1

#define SQUISH_SIG       "sq_cfg"

/*---------------------------------- Typen ----------------------------------*/

typedef struct
{
   char  achSig[16];             /* Signatur */
   ULONG ulMaxMsgs;              /* die drei Squish-Parameter */
   ULONG ulDays;
   ULONG ulSkip;
   ULONG ulFormat;               /* Default-Format */
   char  achPath[CCHMAXPATH+1];  /* Default-Pfad f. Areas */
   ULONG ulFlags;                /* andere Flags, s.u. */
} SQUISHPARBLOCK, *PSQUISHPARBLOCK;

#define MANFLAG_HPFS         1UL

typedef struct
{
   USHORT cb;
   PSQUISHPARBLOCK pParamBlock;
} DLGPARAM, *PDLGPARAM;

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static MRESULT EXPENTRY SetupDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryVersion                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Gibt die Version der DLL-Schnittstelle zurueck              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: -                                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Version der DLL-Schnittstelle                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG APIENTRY QueryVersion(VOID)
{
   return DLL_VERSION;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryParamBlockSize                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Gibt die Groesse der Parameter-Blocks zurueck               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: -                                                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Groesse des Paramter-Blocks                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG APIENTRY QueryParamBlockSize(VOID)
{
   return sizeof(SQUISHPARBLOCK);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetupParams                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet den Dialog zum Einstellen der Parameter             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParamBlock: Zeiegr auf Parameter-Block                        */
/*            ulParamBlockSize: Groesse des Parameter-Blocks                 */
/*            hwndOwner: Owner-Window f. Dialog                              */
/*            hab: Anchor-Block                                              */
/*            hModule: Module-Handle dieser DLL                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: ECHOMAN_PARAMSIZE                                          */
/*                ECHOMAN_FORMAT                                             */
/*                ECHOMAN_OK                                                 */
/*                ECHOMAN_CANCEL                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG APIENTRY SetupParams(PVOID pParamBlock, ULONG ulParamBlockSize,
                           HWND hwndOwner, HAB hab, HMODULE hModule)
{
   PSQUISHPARBLOCK pParams = (PSQUISHPARBLOCK) pParamBlock;
   DLGPARAM DlgParam;

   hab = hab;

   if (!pParamBlock)
      return ECHOMAN_PARAMSIZE;

   if (ulParamBlockSize != sizeof(SQUISHPARBLOCK))
      return ECHOMAN_PARAMSIZE;

   if (pParams->achSig[0])
   {
      if (strcmp(pParams->achSig, SQUISH_SIG))
         return ECHOMAN_FORMAT;
   }
   else
      strcpy(pParams->achSig, SQUISH_SIG);

   DlgParam.cb=sizeof(DlgParam);
   DlgParam.pParamBlock = pParams;

   if (WinDlgBox(HWND_DESKTOP, hwndOwner, SetupDlgProc, hModule,
                 IDD_SETUPDLG, &DlgParam) == DID_OK)
      return ECHOMAN_OK;
   else
      return ECHOMAN_CANCEL;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SetupDlgProc                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure des Setup-Dialogs                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: WINPROC                                                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY SetupDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PSQUISHPARBLOCK pParams = (PSQUISHPARBLOCK) WinQueryWindowPtr(hwnd, 0);

   switch(msg)
   {
      case WM_INITDLG:
         pParams = ((PDLGPARAM)mp2)->pParamBlock;
         WinSetWindowPtr(hwnd, 0, pParams);

         /* Alte Daten eintragen */
         /* Pfad */
         WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+6, EM_SETTEXTLIMIT,
                           MPFROMSHORT(CCHMAXPATH), NULL);
         WinSetDlgItemText(hwnd, IDD_SETUPDLG+6, pParams->achPath);

         /* Format */
         if (pParams->ulFormat == FORMAT_SQUISH)
            WinCheckButton(hwnd, IDD_SETUPDLG+5, TRUE);
         else
            WinCheckButton(hwnd, IDD_SETUPDLG+4, TRUE);

         /* HPFS */
         if (pParams->ulFlags & MANFLAG_HPFS)
            WinCheckButton(hwnd, IDD_SETUPDLG+11, TRUE);

         /* Squish-Parameter */
         WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+1, SPBM_SETLIMITS,
                           MPFROMLONG(20000), MPFROMLONG(0));
         WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+1, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pParams->ulMaxMsgs), NULL);
         WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+2, SPBM_SETLIMITS,
                           MPFROMLONG(20000), MPFROMLONG(0));
         WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+2, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pParams->ulDays), NULL);
         WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+3, SPBM_SETLIMITS,
                           MPFROMLONG(20000), MPFROMLONG(0));
         WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+3, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pParams->ulSkip), NULL);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == DID_OK)
         {
            /* Daten aus dem Dialog rausholen */
            WinQueryDlgItemText(hwnd, IDD_SETUPDLG+6, sizeof(pParams->achPath), pParams->achPath);
            RemoveBackslash(pParams->achPath);
            if (WinQueryButtonCheckstate(hwnd, IDD_SETUPDLG+5))
               pParams->ulFormat = FORMAT_SQUISH;
            else
               pParams->ulFormat = FORMAT_SDM;
            WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+1, SPBM_QUERYVALUE,
                              &pParams->ulMaxMsgs,
                              MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
            WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+2, SPBM_QUERYVALUE,
                              &pParams->ulDays,
                              MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
            WinSendDlgItemMsg(hwnd, IDD_SETUPDLG+3, SPBM_QUERYVALUE,
                              &pParams->ulSkip,
                              MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

            if (WinQueryButtonCheckstate(hwnd, IDD_SETUPDLG+11))
               pParams->ulFlags |= MANFLAG_HPFS;
            else
               pParams->ulFlags &= ~MANFLAG_HPFS;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddEcho                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt ein Echo der Squish.Cfg hinzu                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParamBlock: Parameter-Block                                   */
/*            ulParamBlockSize: Groesse des Parameter-Blocks                 */
/*            pchCfgFile: Filename der SQUISH.CFG                            */
/*            pchCurrentAddress: Aktuelle Adresse                            */
/*            pchUplinkAddress: Adresse des Uplinks                          */
/*            pchAreaTag: Bestelltes Echo                                    */
/*            ulFlags: Flags                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: ECHOMAN_OK                                                 */
/*                ECHOMAN_PARAMSIZE                                          */
/*                ECHOMAN_FORMAT                                             */
/*                ECHOMAN_ERROR                                              */
/*                ECHOMAN_CFGNOTFOUND                                        */
/*                ECHOMAN_CFGREAD                                            */
/*                ECHOMAN_CFGWRITE                                           */
/*                ECHOMAN_CFGFORMAT                                          */
/*                ECHOMAN_ALREADYLINKED                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG APIENTRY AddEcho(PVOID pParamBlock, ULONG ulParamBlockSize,
                       PCHAR pchCfgFile,
                       PCHAR pchCurrentAddress,
                       PCHAR pchUplinkAddress,
                       PCHAR pchAreaTag,
                       ULONG ulFlags)
{
   PSQUISHPARBLOCK pParams = (PSQUISHPARBLOCK) pParamBlock;
   FILE *pfCfgFile;
   char pchLine[500];
   char *pchTemp;
   BOOL bLineEnd=FALSE;

   ulFlags = ulFlags;

   if (!pParamBlock)
      return ECHOMAN_PARAMSIZE;

   if (ulParamBlockSize != sizeof(SQUISHPARBLOCK))
      return ECHOMAN_PARAMSIZE;

   if (!pParams->achSig[0] || strcmp(pParams->achSig, SQUISH_SIG))
      return ECHOMAN_FORMAT;

   if (pfCfgFile = fopen(pchCfgFile, "a+"))
   {
      /* Echo erstmal suchen */
      while (!feof(pfCfgFile))
      {
         int len;

         if (!fgets(pchLine, sizeof(pchLine), pfCfgFile))
            continue;

         len = strlen(pchLine);
         if (len && pchLine[len-1]=='\n')
            bLineEnd=TRUE;
         else
            bLineEnd=FALSE;

         pchTemp = strtok(pchLine, " \t");
         if (pchTemp)
         {
            if (!stricmp(pchTemp, "EchoArea") ||
                !stricmp(pchTemp, "LocalArea") ||
                !stricmp(pchTemp, "NetArea") ||
                !stricmp(pchTemp, "BadArea") ||
                !stricmp(pchTemp, "DupeArea"))
            {
               pchTemp = strtok(NULL, " \t");
               if (pchTemp && !stricmp(pchTemp, pchAreaTag))
               {
                  /* Echo gefunden */
                  fclose(pfCfgFile);
                  return ECHOMAN_ALREADYLINKED;
               }
            }
         }
      }

      /* Bis zum Ende gelesen, Area noch nicht vorhanden, neue Area erzeugen */
      if (!bLineEnd) /* kein Linefeed am Dateiende */
         fputs("\n", pfCfgFile);
      if (pParams->ulFlags & MANFLAG_HPFS)
         sprintf(pchLine, "EchoArea %s %s\\%s -p%s ", pchAreaTag,
                 pParams->achPath, pchAreaTag, pchCurrentAddress);
      else
         sprintf(pchLine, "EchoArea %s %s\\%08X -p%s ", pchAreaTag,
                 pParams->achPath, Crc32(pchAreaTag, strlen(pchAreaTag), 0xFFFFFFFFUL),
                 pchCurrentAddress);
      fputs(pchLine, pfCfgFile);
      if (pParams->ulFormat == FORMAT_SQUISH)
      {
         if (pParams->ulMaxMsgs)
         {
            sprintf(pchLine, "-$m%d ", pParams->ulMaxMsgs);
            fputs(pchLine, pfCfgFile);
         }
         if (pParams->ulDays)
         {
            sprintf(pchLine, "-$d%d ", pParams->ulDays);
            fputs(pchLine, pfCfgFile);
         }
         if (pParams->ulSkip)
         {
            sprintf(pchLine, "-$s%d ", pParams->ulSkip);
            fputs(pchLine, pfCfgFile);
         }
         if (!pParams->ulMaxMsgs &&
             !pParams->ulDays &&
             !pParams->ulSkip)
            fputs("-$ ", pfCfgFile);
      }
      fputs(pchUplinkAddress, pfCfgFile);
      fputs("\n", pfCfgFile);

      fclose(pfCfgFile);

      return ECHOMAN_OK;
   }
   else
      return ECHOMAN_CFGNOTFOUND;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: RemoveEcho                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Entfernt ein abbestelltes Echo aus der SQUISH.CFG           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pParamBlock: Parameter-Block                                   */
/*            ulParamBlockSize: Groesse des Parameter-Blocks                 */
/*            pchCfgFile: Filename der SQUISH.CFG                            */
/*            pchCurrentAddress: Aktuelle Adresse                            */
/*            pchUplinkAddress: Adresse des Uplinks                          */
/*            pchAreaTag: Abbestelltes Echo                                  */
/*            ulFlags: Flags                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: ECHOMAN_OK                                                 */
/*                ECHOMAN_PARAMSIZE                                          */
/*                ECHOMAN_FORMAT                                             */
/*                ECHOMAN_ERROR                                              */
/*                ECHOMAN_CFGNOTFOUND                                        */
/*                ECHOMAN_CFGREAD                                            */
/*                ECHOMAN_CFGWRITE                                           */
/*                ECHOMAN_CFGFORMAT                                          */
/*                ECHOMAN_NOTLINKED                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:-                                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/

ULONG APIENTRY RemoveEcho(PVOID pParamBlock, ULONG ulParamBlockSize,
                          PCHAR pchCfgFile,
                          PCHAR pchCurrentAddress,
                          PCHAR pchUplinkAddress,
                          PCHAR pchAreaTag,
                          ULONG ulFlags)
{
   PSQUISHPARBLOCK pParams = (PSQUISHPARBLOCK) pParamBlock;
   FILE *pfCfgFile;
   FILE *pfNewFile;
   char *pchTempName;
   char pchLine[500];
   char *pchTemp, *pchDup;
   BOOL bFound=FALSE;

   ulFlags = ulFlags;
   pchCurrentAddress = pchCurrentAddress;
   pchUplinkAddress = pchUplinkAddress;

   if (!pParamBlock)
      return ECHOMAN_PARAMSIZE;

   if (ulParamBlockSize != sizeof(SQUISHPARBLOCK))
      return ECHOMAN_PARAMSIZE;

   if (!pParams->achSig[0] || strcmp(pParams->achSig, SQUISH_SIG))
      return ECHOMAN_FORMAT;

   if (pfCfgFile = fopen(pchCfgFile, "r"))
   {
      /* SQUISH.CFG in temp. File umkopieren */
      pchTempName = tmpnam(NULL);
      if (pchTempName && (pfNewFile = fopen(pchTempName, "w")))
      {
         while (!feof(pfCfgFile))
         {
            if (!fgets(pchLine, sizeof(pchLine), pfCfgFile))
               continue;

            pchDup = strdup(pchLine);
            pchTemp = strtok(pchDup, " \t");
            if (pchTemp)
            {
               if (!stricmp(pchTemp, "EchoArea"))
               {
                  pchTemp = strtok(NULL, " \t");
                  if (pchTemp && !stricmp(pchTemp, pchAreaTag))
                  {
                     free(pchDup);
                     bFound=TRUE;
                     fprintf(pfNewFile, "; %s", pchLine);
                     continue; /* Das ist die Zeile, also auslassen */
                  }
               }
            }
            /* nicht die gesuchte Zeile, kopieren */
            fputs(pchLine, pfNewFile);
            free(pchDup);
         }

         fclose(pfNewFile);
         fclose(pfCfgFile);

         if (bFound)
         {
            /* umkopieren */
            if (DosCopy(pchTempName, pchCfgFile, DCPY_EXISTING))
               return ECHOMAN_CFGWRITE;
         }

         DosDelete(pchTempName);

         if (bFound)
            return ECHOMAN_OK;
         else
            return ECHOMAN_NOTLINKED;
      }
      else
      {
         fclose(pfCfgFile);
         return ECHOMAN_CFGWRITE;
      }
   }
   else
      return ECHOMAN_CFGNOTFOUND;
}

/*-------------------------------- Modulende --------------------------------*/


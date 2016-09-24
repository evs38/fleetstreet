/*---------------------------------------------------------------------------+
 | Titel: PRINTMSG.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 08.07.94                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Drucken einer Message mit PM-Treiber                                   |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#define INCL_SPLDOSPRINT
#define INCL_GPI
#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\main.h"
#include "..\resids.h"
#include "..\structs.h"
#include "..\msgheader.h"
#include "..\areaman\areaman.h"
#include "..\handlemsg\handlemsg.h"
#include "..\printsetup.h"
#include "..\util\addrcnv.h"
#include "printmsg.h"

/*--------------------------------- Defines ---------------------------------*/

#define FONTID_HEADER  1
#define FONTID_TEXT    2

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int OpenDC(PPRINTSETUP pPrintSetup, PHDC pHDC);
static LONG PrintHeader(HPS hps, MSGHEADER *pHeader,
                        PCHAR pchAreaTag, ULONG ulMsgNum, PAREALIST pAreaList,
                        HMODULE hmod, PPRINTSETUP pPrintSetup, FONTMETRICS *pfm, PHCINFO pFormInfo);
static int PrintText(HPS hps, HDC hdc, LONG lTextStart,
                     PMSGHEADER pHeader, FTNMESSAGE *pMessage, PPRINTSETUP pPrintSetup,
                     FONTMETRICS *pfm_header, FONTMETRICS *pfm_text, LONG lSizeHeader, LONG lSizeText, PHCINFO pFormInfo);
static LONG QueryWrap(PRECTL prclLimit, char *pchText, PLONG plWidths);
static LONG CreateFont(char *pchFont, HPS hps, LONG lcid);
static int SwitchToFont(HPS hps, LONG lcid, LONG lSize);

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenPrinter                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Druckerdaten vom System, bereitet PS und DC vor.   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hab: Anker-Block der Anwendung                                 */
/*            plWidth: Breite der Ausgabeflaeche                             */
/*            plHeight: Hoehe der Ausgabeflaeche                             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: PRINTMSG_OK: kein Fehler                                   */
/*                PRINTMSG_NOPRINTER: Kein Drucker installiert               */
/*                PRINTMSG_NOPROP:    Keine Job-Properties                   */
/*                PRINTMSG_NODC:      DC kann nicht erzeugt werden           */
/*                PRINTMSG_NOPS:      PS kann nicht erzeugt werden           */
/*                PRINTMSG_ERROR:     Sonstiger Fehler                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int OpenPrinter(PPRINTSETUP pPrintSetup, PHDC pHDC, PHPS pHPS)
{
   if (!OpenDC(pPrintSetup, pHDC))
   {
      SIZEL size = {0, 0}; /* max. Groesse */

      *pHPS = GpiCreatePS(pPrintSetup->hab, *pHDC, &size,
                          pPrintSetup->lWorldCoordinates |
                          GPIF_LONG |    /* LONG-Koordinaten */
                          GPIT_NORMAL |  /* normal PS */
                          GPIA_ASSOC);   /* sofort mit DC assoziieren */
      if (*pHPS)
         return PRINTMSG_OK;
      else
      {
         DevCloseDC(*pHDC);
         return PRINTMSG_NOPS;
      }
   }
   else
      return PRINTMSG_NODC;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenPrinterDM                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Druckerdaten vom System, bereitet PS und DC vor.   */
/*               Wird bei Drop auf Printer verwendet                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hab: Anker-Block der Anwendung                                 */
/*            pHDC: Puffer f. HDC                                            */
/*            pHPS: Puffer f. HPS                                            */
/*            pPrintDest: Parameter von DM_PRINTOBJECT                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: PRINTMSG_OK: kein Fehler                                   */
/*                PRINTMSG_NOPRINTER: Kein Drucker installiert               */
/*                PRINTMSG_NOPROP:    Keine Job-Properties                   */
/*                PRINTMSG_NODC:      DC kann nicht erzeugt werden           */
/*                PRINTMSG_NOPS:      PS kann nicht erzeugt werden           */
/*                PRINTMSG_ERROR:     Sonstiger Fehler                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int OpenPrinterDM(HAB hab, PHDC pHDC, PHPS pHPS, PPRINTDEST pPrintDest)
{
   *pHDC = DevOpenDC(hab,
                     pPrintDest->lType,
                     pPrintDest->pszToken,
                     pPrintDest->lCount,
                     pPrintDest->pdopData,
                     NULLHANDLE);

   if (!*pHDC)
      return PRINTMSG_NODC;
   else
   {
      SIZEL size = {0, 0}; /* max. Groesse */

      *pHPS = GpiCreatePS(hab, *pHDC, &size,
                          PU_LOMETRIC |  /* 0.1 mm */
                          GPIF_LONG |    /* LONG-Koordinaten */
                          GPIT_NORMAL |  /* normal PS */
                          GPIA_ASSOC);   /* sofort mit DC assoziieren */
      if (*pHPS)
         return PRINTMSG_OK;
      else
      {
         DevCloseDC(*pHDC);
         return PRINTMSG_NOPS;
      }
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ClosePrinter                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Beendet das Drucken, gibt Resourcen zurueck                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hdc: DC-Handle                                                 */
/*            hps: PS-Handle                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: PRINTMSG_OK                                                */
/*                PRINTMSG_ERROR                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ClosePrinter(HDC hdc, HPS hps)
{
   GpiAssociate(hps, NULLHANDLE); /* dissoziieren */
   GpiDestroyPS(hps);
   DevCloseDC(hdc);

   return PRINTMSG_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PrintMessage                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Druckt eine Message aus                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hdc: DC-Handle                                                 */
/*            hps: PS-Handle                                                 */
/*            lWidth: Breite der Ausgabe                                     */
/*            lHeight: Hoehe der Ausgabe                                     */
/*            pHeader: Message-Header                                        */
/*            pMessage: Message-Text                                         */
/*            pchAreaTag: Area-Tag der Message                               */
/*            ulMsgNum: Message-Nummer                                       */
/*            pAreaList: Area-Liste                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: PRINTMSG_OK                                                */
/*                PRINTMSG_ERROR                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int PrintMessage(HDC hdc, HPS hps,
                 MSGHEADER *pHeader, FTNMESSAGE *pMessage,
                 PCHAR pchAreaTag, ULONG ulMsgNum, PAREALIST pAreaList,
                 HMODULE hmod, PPRINTSETUP pPrintSetup)
{
   char pchDocName[LEN_AREATAG+1 +30];
   LONG lData;
   USHORT usJobID;
   LONG lTextStart=0;
   FONTMETRICS fm_header, fm_text;
   LONG lSizeHeader, lSizeText;
   LONG lForms, i;
   HCINFO FormInfo;

   memset(&FormInfo, 0, sizeof(FormInfo));
   lForms = DevQueryHardcopyCaps(hdc, 0, 0, NULL);

   for (i=0; i<lForms; i++)
   {
      DevQueryHardcopyCaps(hdc, i, 1, &FormInfo);
      if (FormInfo.flAttributes & HCAPS_CURRENT)
         break;
      else
         memset(&FormInfo, 0, sizeof(FormInfo));
   }

   /* Job-Name */
   sprintf(pchDocName, "%s, #%ld", pchAreaTag, ulMsgNum);

   /* Dokument starten */
   lData = 0;
   if (DevEscape(hdc, DEVESC_STARTDOC, strlen(pchDocName)+1, pchDocName,
                 &lData, NULL) != DEV_OK)
   {
      return PRINTMSG_NOSTARTDOC;
   }

   GpiSetColor(hps, CLR_DEFAULT);
   lSizeHeader = CreateFont(pPrintSetup->pchHeaderFont, hps, FONTID_HEADER);
   GpiQueryFontMetrics(hps, sizeof(fm_header), &fm_header);

   /* Header zeichnen */
   lTextStart = PrintHeader(hps, pHeader, pchAreaTag, ulMsgNum, pAreaList,
                            hmod, pPrintSetup, &fm_header, &FormInfo);

   lSizeText = CreateFont(pPrintSetup->pchTextFont, hps, FONTID_TEXT);
   GpiQueryFontMetrics(hps, sizeof(fm_text), &fm_text);

   /* Text zeichnen */
   PrintText(hps, hdc, lTextStart, pHeader, pMessage, pPrintSetup,
             &fm_header, &fm_text, lSizeHeader, lSizeText, &FormInfo);

   /* Dokument beenden */
   lData = 2;
   DevEscape(hdc, DEVESC_ENDDOC, 0, NULL, &lData, (PBYTE) &usJobID);

   return PRINTMSG_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenDC                                                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet einen DC fuer den Drucker                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hab: Anchor-Block                                              */
/*            pQueueInfo: Queue-Infos                                        */
/*            pHDC: ermittelter HDC                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: PRINTMSG_OK                                                */
/*                PRINTMSG_ERROR                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int OpenDC(PPRINTSETUP pPrintSetup, PHDC pHDC)
{
   *pHDC = DevOpenDC(pPrintSetup->hab,
                     pPrintSetup->lDCType,
                     "*",
                     4,            /* 4 Felder von DevOpen verwendet */
                     pPrintSetup->pDevOpenData,
                     NULLHANDLE);

   if (!*pHDC)
      return PRINTMSG_NODC;
   else
      return PRINTMSG_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PrintHeader                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Druckt den Header                                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hps: PS                                                        */
/*            pHeader: Header                                                */
/*            pchAreaTag: Area-Tag                                           */
/*            ulMsgNum: Message-Nummer                                       */
/*            pAreaList: Area-Liste                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: PRINTMSG_OK                                                */
/*                PRINTMSG_ERROR                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG PrintHeader(HPS hps, MSGHEADER *pHeader,
                        PCHAR pchAreaTag, ULONG ulMsgNum, PAREALIST pAreaList,
                        HMODULE hmod, PPRINTSETUP pPrintSetup, FONTMETRICS *pfm, PHCINFO pFormInfo)
{
   POINTL ptl;
   char pchString[200];
   LONG lLineHeight;
   LONG lLeft=0, lBottom;
   LONG lLeftHeader, lTopHeader;
   AREADEFLIST *zeiger;

   /* Font-Hoehe holen */
   lLineHeight = pfm->lMaxAscender + pfm->lMaxDescender;

   /* Raender festlegen */
   lTopHeader = (pFormInfo->cy - pFormInfo->yBottomClip - pPrintSetup->lTop)*10;
   lLeftHeader = (pPrintSetup->lLeft - pFormInfo->xLeftClip)*10 + 10;

   /* Area-Text */
   WinLoadString(pPrintSetup->hab, hmod, IDST_MW_AREA, 100, pchString);

   ptl.x = lLeftHeader;
   ptl.y = lTopHeader - lLineHeight + pfm->lMaxDescender;
   GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);

   GpiQueryCurrentPosition(hps, &ptl);
   if (ptl.x > lLeft)
      lLeft = ptl.x;

   /* FROM-Text */
   WinLoadString(pPrintSetup->hab, hmod, IDST_MW_FROM, 100, pchString);

   ptl.x = lLeftHeader;
   ptl.y -= lLineHeight;
   GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);

   GpiQueryCurrentPosition(hps, &ptl);
   if (ptl.x > lLeft)
      lLeft = ptl.x;

   /* TO-Text */
   WinLoadString(pPrintSetup->hab, hmod, IDST_MW_TO, 100, pchString);

   ptl.x = lLeftHeader;
   ptl.y -= lLineHeight;
   GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);

   GpiQueryCurrentPosition(hps, &ptl);
   if (ptl.x > lLeft)
      lLeft = ptl.x;

   /* Subject-Text */
   WinLoadString(pPrintSetup->hab, hmod, IDST_MW_SUBJ, 100, pchString);

   ptl.x = lLeftHeader;
   ptl.y -= lLineHeight;
   GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);

   GpiQueryCurrentPosition(hps, &ptl);
   if (ptl.x > lLeft)
      lLeft = ptl.x;

   /* Attrib-Text */
   if (pPrintSetup->ulOutput & OUTPUT_ATTRIB)
   {
      WinLoadString(pPrintSetup->hab, hmod, IDST_MW_ATTRIB, 100, pchString);

      ptl.x = lLeftHeader;
      ptl.y -= lLineHeight;
      GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);

      GpiQueryCurrentPosition(hps, &ptl);
      if (ptl.x > lLeft)
         lLeft = ptl.x;
   }

   /* Area-Tag */
   ptl.x = lLeft + 50;
   ptl.y = lTopHeader - lLineHeight  + pfm->lMaxDescender;

   GpiCharStringAt(hps, &ptl, strlen(pchAreaTag), pchAreaTag);
   GpiCharString(hps, 3, ", #");

   _itoa(ulMsgNum, pchString, 10);
   GpiCharString(hps, strlen(pchString), pchString);

   /* From-Name */
   ptl.y -= lLineHeight;
   GpiCharStringAt(hps, &ptl, strlen(pHeader->pchFromName), pHeader->pchFromName);

   /* To-Name */
   ptl.y -= lLineHeight;
   GpiCharStringAt(hps, &ptl, strlen(pHeader->pchToName), pHeader->pchToName);

   /* Subject */
   ptl.y -= lLineHeight;
   GpiCharStringAt(hps, &ptl, strlen(pHeader->pchSubject), pHeader->pchSubject);

   /* Attribute */
   if (pPrintSetup->ulOutput & OUTPUT_ATTRIB)
   {
      ptl.y -= lLineHeight;

      MSG_AttribToText(pHeader->ulAttrib, pchString);
      GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);
   }

   /* y-Position f. Rahmen merken */
   lBottom = ptl.y - 10 - pfm->lMaxDescender;

   /* Datum */
   ptl.x = lLeftHeader +( (pFormInfo->cx - pPrintSetup->lRight - pPrintSetup->lLeft)*10 )/2;  /* ab Mitte */
   ptl.y = lTopHeader - lLineHeight + pfm->lMaxDescender;

   if (pPrintSetup->ulOutput & OUTPUT_DATE)
   {
      StampToString(pchString, &pHeader->StampWritten);
      GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);
   }

   /* Absender-Adresse */
   ptl.y -= lLineHeight;

   NetAddrToString(pchString, &pHeader->FromAddress);
   GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);

   zeiger = AM_FindArea(pAreaList, pchAreaTag);

   if (zeiger && zeiger->areadata.areatype == AREATYPE_NET)   /* nur in NM-Areas */
   {
      /* Empfaenger-Adresse */
      ptl.y -= lLineHeight;

      NetAddrToString(pchString, &pHeader->ToAddress);
      GpiCharStringAt(hps, &ptl, strlen(pchString), pchString);
   }

   /* Rahmen */
   ptl.x = pPrintSetup->lLeft*10 - pFormInfo->xLeftClip*10;
   ptl.y = lTopHeader + 10;

   /*GpiBeginPath(hps, 1);*/
   GpiSetLineWidth(hps, LINEWIDTH_THICK);

   /* linke obere Ecke */
   GpiMove(hps, &ptl);

   /* rechte untere Ecke */
   ptl.x = (pFormInfo->cx - pPrintSetup->lRight - pFormInfo->xLeftClip)*10;
   ptl.y = lBottom;
   GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);

   /*GpiEndPath(hps);

   GpiSetLineWidthGeom(hps, 5);
   GpiStrokePath(hps, 1, 0);
   */
   GpiSetLineWidth(hps, LINEWIDTH_NORMAL);

   return lBottom - 10;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: PrintText                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Druckt den Message-Text                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hps: PS                                                        */
/*            hdc: DC                                                        */
/*            pMessage: Message-Text                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: PRINTMSG_OK                                                */
/*                PRINTMSG_ERROR                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int PrintText(HPS hps, HDC hdc, LONG lTextStart,
                     PMSGHEADER pHeader, FTNMESSAGE *pMessage, PPRINTSETUP pPrintSetup,
                     FONTMETRICS *pfm_header, FONTMETRICS *pfm_text, LONG lSizeHeader, LONG lSizeText, PHCINFO pFormInfo)
{
   LONG lLineHeight;
   char *pchHelp;
   LONG lY, lChars;
   RECTL rectl;
   LONG lWidths[256];
   FIXED fxExtra;
   int i;
   LONG lTopText, lLeftText;
   LONG lPageNum=1;

   /* Font-Hoehe holen */
   lLineHeight = pfm_text->lMaxBaselineExt;

   /* Text drucken */
   if (!pMessage->pchMessageText || strlen(pMessage->pchMessageText) == 0)
      return PRINTMSG_OK;  /* Nix zu tun */

   /* Zeichenbreiten ermitteln */
   GpiQueryWidthTable(hps, 0, 256, lWidths);
   if (GpiQueryCharExtra(hps, &fxExtra) && fxExtra)
      for (i=0; i< 256; ++i)
         lWidths[i] += fxExtra /65536;

   /* Anfangs-Koordinaten */
   lY = lTextStart - lLineHeight;
   lLeftText = pPrintSetup->lLeft*10 - pFormInfo->xLeftClip*10;
   lTopText = (pFormInfo->cy - pFormInfo->yBottomClip - pPrintSetup->lTop)*10;

   pchHelp = pMessage->pchMessageText;

   while (*pchHelp)
   {
      /* Zeile drucken */

      /* Rechteck f. Test, Hoehe= Fonthoehe, Breite = Papierbreite */
      rectl.xLeft = lLeftText;
      rectl.xRight = (pFormInfo->cx - pFormInfo->xLeftClip - pPrintSetup->lRight)*10;
      rectl.yBottom = lY;
      rectl.yTop= lY + lLineHeight;

      lChars = QueryWrap(&rectl, pchHelp, lWidths);
      if (lChars)
         GpiCharStringAt(hps, (PPOINTL) &rectl, lChars, pchHelp);

      pchHelp += lChars;

      while (*pchHelp == ' ')  /* Leerzeichen uebergehen */
         pchHelp++;

      if (*pchHelp == '\n')
      {
         /* Zeilenende uebergehen */
         pchHelp++;
      }

      /* Naechste Zeile berechnen */
      lY -= lLineHeight;

      if (lY < (pPrintSetup->lBottom - pFormInfo->yBottomClip)*10 && *pchHelp)  /* unter unteren Rand und nochwas da */
      {
         /* neue Seite */
         LONG lData=0;

         DevEscape(hdc, DEVESC_NEWFRAME, 0, NULL, &lData, NULL);
         lPageNum++;

         if (pPrintSetup->ulOutput & OUTPUT_PAGENUM)
         {
            /* Seitenkopf f. Seite 2..n */
            POINTL ptl;
            char pchPageNum[20];
            POINTL TextBox[TXTBOX_COUNT];

            lY = lTopText - pfm_header->lMaxAscender;

            SwitchToFont(hps, FONTID_HEADER, lSizeHeader);

            /* From: Subject */
            ptl.x = lLeftText;
            ptl.y = lY;

            GpiCharStringAt(hps, &ptl, strlen(pHeader->pchFromName), pHeader->pchFromName);
            GpiCharString(hps, 2, ": ");
            GpiCharString(hps, strlen(pHeader->pchSubject), pHeader->pchSubject);

            /* Seitennummer (rechtsb］dig) */
            _itoa(lPageNum, pchPageNum, 10);
            GpiQueryTextBox(hps, strlen(pchPageNum), pchPageNum, TXTBOX_COUNT, TextBox);
            ptl.x = (pFormInfo->cx - pFormInfo->xLeftClip - pPrintSetup->lRight)*10 -
                    (TextBox[TXTBOX_TOPRIGHT].x - TextBox[TXTBOX_TOPLEFT].x);
            GpiCharStringAt(hps, &ptl, strlen(pchPageNum), pchPageNum);

            /* Trennlinie */
            ptl.x = lLeftText;
            ptl.y = lTopText - pfm_header->lMaxAscender - pfm_header->lMaxDescender;
            GpiSetCurrentPosition(hps, &ptl);
            ptl.x = (pFormInfo->cx - pFormInfo->xLeftClip - pPrintSetup->lRight)*10;
            GpiLine(hps, &ptl);

            SwitchToFont(hps, FONTID_TEXT, lSizeText);

            lY -= pfm_header->lMaxAscender + pfm_header->lMaxDescender + 20;
         }
         else
            lY = lTopText - pfm_text->lMaxAscender;

      }
   }

   return PRINTMSG_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: QueryWrap                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ermittelt die Anzahl der Zeichen, die ins Rechteck passen   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:                                                                */
/*            prclLimit: begrenzendes Rechteck                               */
/*            pchText: auszugebender Text                                    */
/*            plWidths: Array mit Breitenangaben                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Anzahl der Zeichen, die ins Rechteck passen                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static LONG QueryWrap(PRECTL prclLimit, char *pchText, PLONG plWidths)
{
   BOOL bInWord = FALSE;
   char *pchEndWord = NULL;
   char *pchCurrent = pchText;
   LONG lWidth = 0;

   while (*pchCurrent)
   {
      if (*pchCurrent == ' ')
      {
         if (bInWord)
         {
            bInWord = FALSE;
            pchEndWord = pchCurrent;
         }
      }
      else
         if (*pchCurrent == '\n')
         {
            pchEndWord = pchCurrent;
            break;
         }
         else
            bInWord = TRUE;

      lWidth += plWidths[*pchCurrent]+1;

      if (prclLimit->xLeft + lWidth > prclLimit->xRight)
         break;
      else
         pchCurrent++;
   }

   if (pchEndWord && *pchCurrent)
      return pchEndWord - pchText;
   else
      return pchCurrent - pchText;
}

static LONG CreateFont(char *pchFont, HPS hps, LONG lcid)
{
   FATTRS fat;
   char *pchTemp;
   ULONG ulRet, ulCP;

   fat.usRecordLength = sizeof(FATTRS); /* sets size of structure   */
   fat.fsSelection = 0;         /* uses default selection           */
   fat.lMatch = 0L;             /* does not force match             */
   fat.idRegistry = 0;          /* uses default registry            */
   DosQueryCp(sizeof(ulCP), &ulCP, &ulRet);
   fat.usCodePage = ulCP;       /* default code-page                */
   fat.lMaxBaselineExt = 0L;    /* requested font height is 12 pels */
   fat.lAveCharWidth = 0L;      /* requested font width is 12 pels  */
   fat.fsType = 0;              /* uses default type                */
   fat.fsFontUse = FATTR_FONTUSE_OUTLINE;

   /* Copy Font name to szFacename field */
   pchTemp = strchr(pchFont, '.');
   if (pchTemp)
   {
      pchTemp++;
      strcpy(fat.szFacename, pchTemp);
   }
   else
      return -1;

   if (GpiCreateLogFont(hps, NULL, lcid, &fat) == GPI_ERROR)
      return -1;
   else
   {
      long lSize = strtol(pchFont, &pchTemp, 10);

      SwitchToFont(hps, lcid, lSize);

      return lSize;
   }
}

static int SwitchToFont(HPS hps, LONG lcid, LONG lSize)
{
   SIZEF size;

   GpiSetCharSet(hps, lcid);
   size.cx = MAKEFIXED((lSize*254)/72, 0);
   size.cy = MAKEFIXED((lSize*254)/72, 0);
   GpiSetCharBox(hps, &size);

   return 0;
}

/*-------------------------------- Modulende --------------------------------*/


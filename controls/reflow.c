/*---------------------------------------------------------------------------+
 | Titel: REFLOW.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 30.07.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Textformatierung v. Message-Viewer                                     |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "viewer_int.h"
#include "util.h"
#include "reflow.h"

/*--------------------------------- Defines ---------------------------------*/

/* Strings f. Zeilentyp-Erkennung */
#define TEARLINE  "--- "
#define TEARLINE2 "---\n"
#define ORIGIN    " * Origin: "
#define UMLAUTE "îÑÅ·ôéö†Ö¢ï°ç"


/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static LONG lenTear = sizeof(TEARLINE)-1;
static LONG lenTear2 = sizeof(TEARLINE2)-1;
static LONG lenOrigin = sizeof(ORIGIN)-1;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static UCHAR GetLineType(PVIEWERPARAMS pViewerParams, char *pchLine);
static int QuerySegment(const char *pchMessageText, const char *pchText);
static PVIEWERLINE AddSegment(PVIEWERLINE pOldLine, PVIEWERLINE pAddSeg);
static PVIEWERLINE SegmentText(PVIEWERPARAMS pViewerParams, char *pchText, BOOL bHighlight);
static void WrapSegments(PVIEWERPARAMS pViewerParams, PVIEWERLINE pSegments, LONG lLimit);
static PVIEWERLINE SplitSegment(PVIEWERLINE pSegment, char *pchCurrentPos, char *pchLastSpace);
static void ConvertSegments(PVIEWERPARAMS pViewerParams, PVIEWERLINE pLines, PULONG pulLongestSeg);

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReflowText                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Formatiert den Text neu                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Fenster-Handle des Viewers                               */
/*            pViewerParams: Zeiger auf Control-Daten                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*---------------------------------------------------------------------------*/

void ReflowText(HWND hwnd, PVIEWERPARAMS pViewerParams)
{
   RECTL rectl;

   FreeLines(pViewerParams);

   WinQueryWindowRect(hwnd, &rectl);

   if ((rectl.xRight-rectl.xLeft) <=5  ||
       (rectl.yTop-rectl.yBottom) <=5)
      return;

   if (pViewerParams->pchMessageText)
   {
      LONG lMaxWidth =pViewerParams->recWindow.xRight-pViewerParams->recWindow.xLeft-2*pViewerParams->lBorder;
      PVIEWERLINE pLines;

      pLines = SegmentText(pViewerParams, pViewerParams->pchMessageText, pViewerParams->bHighlight);

      if (pLines)
      {
         ULONG ulLongestSeg;

         WrapSegments(pViewerParams, pLines, lMaxWidth);
         ConvertSegments(pViewerParams, pLines, &ulLongestSeg);
         pViewerParams->pIncrements = malloc(sizeof(LONG) * ulLongestSeg);
      }
   }

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: GetLineType
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Ermittelt den Typ (Text, Quote, ...) der Zeile
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pchLine: Zeiger auf Zeilenanfang
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: LINE_*
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static UCHAR GetLineType(PVIEWERPARAMS pViewerParams, char *pchLine)
{
   /* feststellen, welcher Zeilentyp */
   if (!strncmp(pchLine, TEARLINE, lenTear) ||
       !strncmp(pchLine, TEARLINE2, lenTear2))
      return LINE_TEAR;
   else
      if (!strncmp(pchLine, ORIGIN, lenOrigin))
         return LINE_ORIGIN;
      else
      {
         int i=0, j=0;
         int realchars=0;

         while (i < 10 && pchLine[i] && pchLine[i]!='\n')
         {
            if (pchLine[i] == pViewerParams->uchQuoteChar)
            {
               j++; i++;
               while (pchLine[i] == pViewerParams->uchQuoteChar)
               {
                  j++;
                  i++;
               }

               if ((j%2) == 0)
                  return LINE_QUOTE2;
               else
                  return LINE_QUOTE;
            }
            else
               if (isalpha(pchLine[i]) ||
                   strchr(UMLAUTE, pchLine[i]) ||
                   pchLine[i] == ' ')
               {
                  if (pchLine[i] != ' '&& ++realchars > 4)
                     break;
                  i++;
               }
               else
                  break;
         }
         return LINE_TEXT;
      }
}

#define LINEEND   "\n .,:;-!?\'\")]}>"
#define LINESTART "\n -\'\"([{<"

static int QuerySegment(const char *pchMessageText, const char *pchText)
{
   char chMask = *pchText;
   char chLastChar=0;
   int len=0;

   /* Startbedingung */
   if ((pchText == pchMessageText ||           /* am Anfang */
        strchr(LINESTART, *(pchText-1))) &&    /* oder Startzeichen davor */
       *(pchText+1) != ' ')                    /* und kein Leerzeichen danach */
   {
      pchText++;
      while (*pchText &&               /* bis Textende, */
             *pchText != '\n' &&       /* Zeilenende, */
             *pchText != chMask &&     /* Ende-Marke, */
             (*pchText == ' ' ||
              *pchText == '!' ||
              *pchText == '-' ||
              *pchText == '\'' ||
              (*pchText >=0x30 && *pchText <= 0xef))) /* und mit Zeichen dazwischen */
      {
         chLastChar = *pchText++;
         len++;
      }
      if (*pchText == chMask &&              /* Ende-Marke und */
          chLastChar != ' ' &&               /* kein Leerzeichen davor */
          (strchr(LINEEND, *(pchText+1)) ||   /* Ende-Zeichen oder */
           *(pchText+1) == 0))                /* Text-Ende */
         return len;
   }
   return 0; /* keine Hervorhebung */
}

/*-----------------------------------------------------------------------------
 | Funktionsname: AddSegment
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Fuegt ein neues Segment an eine Zeile an
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pOldLine: Letztes Segment der aktuellen Zeile (od. NULL)
 |            pAddSeg: Hinzuzufuegendes Segment
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: Zeiger auf das neue Segment
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static PVIEWERLINE AddSegment(PVIEWERLINE pOldLine, PVIEWERLINE pAddSeg)
{
   PVIEWERLINE pNewSeg = calloc(1, sizeof(VIEWERLINE));

   if (pOldLine)
      pOldLine->nextseg = pNewSeg;

   *pNewSeg = *pAddSeg;
   pNewSeg->nextseg=NULL;
   pNewSeg->prevseg = pOldLine;

   return pNewSeg;
}


/*-----------------------------------------------------------------------------
 | Funktionsname: SegmentText
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Zerlegt den gegebenen Text in eine Kette von Segmenten.
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pchText: Zu segmentierender Text
 |            bHighlight: Highlight-Switch
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: Anfang der Segmentkette
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: Die Segmente sind nicht umbrochen und enthalten noch keine
 |            Increment-Felder
 +---------------------------------------------------------------------------*/

static PVIEWERLINE SegmentText(PVIEWERPARAMS pViewerParams, char *pchText, BOOL bHighlight)
{
   PVIEWERLINE pFirstSeg=NULL, pCurrentSeg=NULL;
   VIEWERLINE LineSeg;
   char *pchTemp = pchText;
   ULONG ulLen=0;
   BOOL bTest=TRUE;
   ULONG ulType=0;

   if (!pchText)
      return NULL;

   LineSeg.pchLine = pchText;
   LineSeg.ulLineLen = 0;

   while (*pchTemp)
   {
      if (bTest)
      {
         ulType = GetLineType(pViewerParams, pchTemp);
         bTest = FALSE;
      }

      switch(*pchTemp)
      {
         case '_':
         case '*':
         case '/':
            if (bHighlight)
            {
               ulLen = QuerySegment(pchText, pchTemp);
               if (ulLen)
               {
                  /* Altes Segment */
                  if (LineSeg.ulLineLen)
                  {
                     LineSeg.ulFlags = ulType;
                     pCurrentSeg = AddSegment(pCurrentSeg, &LineSeg);
                     if (!pFirstSeg)
                        pFirstSeg = pCurrentSeg;
                  }

                  /* neues Segment */
                  LineSeg.ulFlags = ulType;
                  LineSeg.ulLineLen = ulLen;
                  LineSeg.pchLine = pchTemp+1;
                  switch(*pchTemp)
                  {
                     case '_':
                        LineSeg.ulFlags |= LINESEG_UNDER;
                        break;

                     case '/':
                        LineSeg.ulFlags |= LINESEG_ITALIC;
                        break;

                     default:
                        LineSeg.ulFlags |= LINESEG_BOLD;
                        break;
                  }
                  pCurrentSeg = AddSegment(pCurrentSeg, &LineSeg);
                  if (!pFirstSeg)
                     pFirstSeg = pCurrentSeg;

                  /* naechstes Segment */
                  pchTemp += 1+ulLen;
                  LineSeg.pchLine = pchTemp+1;
                  LineSeg.ulLineLen = 0;
                  LineSeg.ulFlags = ulType;
               }
               else
                  LineSeg.ulLineLen++;
            }
            else
               LineSeg.ulLineLen++;
            break;

         case '\n':
            LineSeg.ulFlags = LINESEG_NEWLINE | ulType;
            pCurrentSeg = AddSegment(pCurrentSeg, &LineSeg);
            if (!pFirstSeg)
               pFirstSeg = pCurrentSeg;
            LineSeg.pchLine = pchTemp+1;
            LineSeg.ulLineLen = 0;
            bTest=TRUE;
            break;

         default:
            LineSeg.ulLineLen++;
            break;
      }
      pchTemp++;
   }

   if (LineSeg.ulLineLen)
   {
      /* letztes Segment noch hinzufÅgen */

      LineSeg.ulFlags = LINESEG_NEWLINE | ulType;
      pCurrentSeg = AddSegment(pCurrentSeg, &LineSeg);
      if (!pFirstSeg)
         pFirstSeg = pCurrentSeg;
   }

   if (pCurrentSeg)
      /* sicherstellen, da· letztes Segment das Zeilenende-Flag hat. Kann vor-
         kommen, wenn letztes Segment ein fettes oder unterstrichenes
         Segment war */
      pCurrentSeg->ulFlags |= LINESEG_NEWLINE;

   return pFirstSeg;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: WrapSegments
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Bricht die Segmente um
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pViewerParams: Instanzdaten
 |            pSegments: Kette der Segmente
 |            lLimit: max. Zeilenbreite (in Pels)
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static void WrapSegments(PVIEWERPARAMS pViewerParams, PVIEWERLINE pSegments, LONG lLimit)
{
   PVIEWERLINE pCurrentSeg = pSegments;
   LONG lWidth=0;
   int i;
   char *pchTemp, *pchLastSpace;
   BOOL bInWord;
   BOOL bSplit;

   while (pCurrentSeg) /* alle Segmente */
   {
      pchLastSpace=NULL;
      pchTemp = pCurrentSeg->pchLine;
      bSplit=FALSE;

      for (i=0; i < pCurrentSeg->ulLineLen && !bSplit; i++, pchTemp++) /* Alle Zeichen im Segment */
      {
         if (*pchTemp == ' ' && bInWord)
         {
            pchLastSpace = pchTemp;
            bInWord = FALSE;
         }
         else
            bInWord = TRUE;

         if (pCurrentSeg->ulFlags & LINESEG_BOLD)
            lWidth += pViewerParams->lWidthsBold[*pchTemp];
         else
            lWidth += pViewerParams->lWidths[*pchTemp];

         if (lWidth > lLimit)
         {
            pCurrentSeg = SplitSegment(pCurrentSeg, pchTemp, pchLastSpace);
            lWidth=0;
            bSplit=TRUE;
         }
      }

      if (!bSplit)
      {
         if (pCurrentSeg->ulFlags & LINESEG_NEWLINE)
            lWidth=0;

         pCurrentSeg = pCurrentSeg->nextseg;
      }
   }

   return;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: SplitSegment
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Teilt ein Segment auf, fuegt ggf. ein Segment dahinter ein
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pSegment: Zu teilendes Segment
 |            pchCurrentPos: aktuelle Position im Text (Zeichen, das nicht mehr
 |                           in die Zeile passt).
 |            pchLastSpace: Stelle des letzten Leerzeichens (ggf. NULL)
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: Zeiger auf das neue Segment
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static PVIEWERLINE SplitSegment(PVIEWERLINE pSegment, char *pchCurrentPos, char *pchLastSpace)
{
   char *pchSplitPos;
   PVIEWERLINE pNewSegment;
   ULONG ulOldLen;

   if (pchLastSpace)
      pchSplitPos = pchLastSpace;
   else
   {
      /* kein Space im Segment */
      if (pSegment->prevseg &&
          !(pSegment->prevseg->ulFlags & LINESEG_NEWLINE))
      {
         /* vorheriges Segment ist nicht Zeilenende */
         /* => vor diesem Segment umbrechen */
         pSegment->prevseg->ulFlags |= LINESEG_NEWLINE;
         return pSegment;
      }
      else
         /* erstes Segment der Zeile an dieser Stelle umbrechen */
         pchSplitPos = pchCurrentPos;
   }

   pNewSegment = calloc(1, sizeof(VIEWERLINE));

   pNewSegment->nextseg = pSegment->nextseg;
   if (pNewSegment->nextseg)
      pNewSegment->nextseg->prevseg = pNewSegment;
   pNewSegment->prevseg = pSegment;
   pSegment->nextseg = pNewSegment;

   ulOldLen = pSegment->ulLineLen;
   pSegment->ulLineLen = (pchSplitPos - pSegment->pchLine);

   pNewSegment->pchLine = pchSplitPos;
   pNewSegment->ulLineLen = ulOldLen - pSegment->ulLineLen;

   /* Leerzeichen am Anfang */
   while (pNewSegment->ulLineLen && *pNewSegment->pchLine == ' ')
   {
      pNewSegment->pchLine++;
      pNewSegment->ulLineLen--;
   }

   pNewSegment->ulFlags = pSegment->ulFlags;
   pSegment->ulFlags |= LINESEG_NEWLINE;

   return pNewSegment;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: ConvertSegments
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Wandelt die Segmente in ein Zeilen-Array um
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pViewerParams: Instanzdaten
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: -
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

static void ConvertSegments(PVIEWERPARAMS pViewerParams, PVIEWERLINE pLines, PULONG pulLongestSeg)
{
   PVIEWERLINE pTemp=pLines;
   ULONG ulNumLines=0;

   *pulLongestSeg=0;

   /* Zeilen zaehlen */
   while (pTemp)
   {
      if (pTemp->ulFlags & LINESEG_NEWLINE)
         ulNumLines++;

      pTemp = pTemp->nextseg;
   }

   if (ulNumLines == 0 && pLines)
      ulNumLines = 1; /* eine Zeile mindestens */

   if (ulNumLines)
   {
      int i=0;
      BOOL bEnter=TRUE;

      pViewerParams->pLines = malloc(ulNumLines * sizeof(PVIEWERLINE));

      while (pLines)
      {
         if (pLines->ulLineLen > *pulLongestSeg)
            *pulLongestSeg = pLines->ulLineLen;

         if (bEnter)
         {
            pViewerParams->pLines[i] = pLines;
            bEnter=FALSE;
            i++;
         }

         if (pLines->ulFlags & LINESEG_NEWLINE)
            bEnter=TRUE;

         pLines = pLines->nextseg;
      }
   }

   pViewerParams->ulCountLines = ulNumLines;


   return;
}

/*-------------------------------- Modulende --------------------------------*/


/*---------------------------------------------------------------------------+
 | Titel:  EDITWIN.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von:  Michael Hohner           | Am:  13.05.93                   |
 +-----------------------------------------+---------------------------------+
 | System:  OS/2 2.x PM                                                      |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Editorfenster fuer Fleet Street                                         |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_GPI
#define INCL_GPI
#define INCL_WIN
#define INCL_DOSNLS
#include <os2.h>
#include <stdlib.h>
#include <string.h>

#include "..\main.h"
#include "..\resids.h"
#include "..\nodedrag.h"
#include "editwin.h"

/*#define NOEXTRA*/

/*--------------------------------- Defines ---------------------------------*/

typedef struct
{
   unsigned char   chOrigChar;     /* Original-Zeichen */
   unsigned char   chNewChars[4];  /* Ersatz-Zeichenfolge, max. 3 char */
} XLATTABLE;

/*---------------------------- Globale Variablen ----------------------------*/

static PFNWP OldEditProc=NULL;   /* Fensterprozedur des MLE */
static PFNWP OldEntryProc=NULL;  /* Fensterprozedur des Entry-Fields */
static const char *achAddr="0123456789:/.";  /* Zeichen in einer 4D-Adresse */
static LONG QWL_TABSIZE=0;        /* Klassenspezifisches Window-Word */
static LONG QWL_TRANSLATE=FALSE;

/* Uebersetzungstabelle */
static const XLATTABLE XLatTable[]={
  { '†', "a"},
  { 'É', "a"},
  { 'Ñ', "ae"},
  { 'é', "Ae"},
  { 'Ö', "a"},
  { 'ë', "ae"},
  { 'í', "Ae"},
  { '‡', "a"},
  { 'è', "A"},
  { 'Ü', "a"},
  { '·', "ss"},
  { 'á', "c"},
  { 'Ä', "C"},
  { 'Ø', ">>"},
  { '¯', "o"},
  { 'ˆ', "/"},
  { '˙', "."},
  { '', "|"},
  { 'Ç', "e"},
  { 'ê', "E"},
  { 'à', "e"},
  { 'â', "e"},
  { 'ä', "e"},
  { 'ü', "f"},
  { '', ""},
  { '°', "i"},
  { 'å', "i"},
  { 'ã', "i"},
  { 'ç', "i"},
  { '≠', "!"},
  { '®', "?"},
  { '', "<-"},
  { '™', "-|"},
  { 'Ê', "mc"},
  { '§', "n"},
  { '•', "N"},
  { '¢', "o"},
  { 'ì', "o"},
  { 'ï', "o"},
  { 'î', "oe"},
  { 'ô', "Oe"},
  { '¨', "1/4"},
  { '´', "1/2"},
  { 'Æ', "<<"},
  { 'Ò', "+-"},
  { 'ú', "UKP"},
  { '˝', "qd"},
  { '¸', "trp"},
  { '£', "u"},
  { 'ñ', "u"},
  { 'ó', "u"},
  { 'Å', "ue"},
  { 'ö', "Ue"},
  { '¶', "a"},
  { 'ß', "o"},
  { 'ò', "y"},
  { 0, ""}    /* Ende-Kennzeichen */
};

RENDERPAR RenderPar;

static HPOINTER hptrMessage;

static MRESULT EXPENTRY EditWinProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static void SendCursorPos(HWND hwnd);
static MRESULT EXPENTRY SubjectEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY FromToEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp);

/*----------------------------- RegisterEditWin -----------------------------*/
/* Registriert die neue Fensterklasse                                        */
/*---------------------------------------------------------------------------*/

BOOL RegisterEditWin(HAB anchor)
{
   CLASSINFO classinfo;

   /* Neues MLE registrieren */
   if (!WinQueryClassInfo(anchor, WC_MLE, &classinfo))
      return FALSE;

   OldEditProc=classinfo.pfnWindowProc;
   QWL_TABSIZE=classinfo.cbWindowData;
   QWL_TRANSLATE=classinfo.cbWindowData+4;

   if (!WinRegisterClass(anchor,
                         "EditWin",
                         EditWinProc,
                         CS_SYNCPAINT,
                         classinfo.cbWindowData+8L))
      return FALSE;

   /* Neues Entry-Field registrieren */
   if (!WinQueryClassInfo(anchor, WC_ENTRYFIELD, &classinfo))
      return FALSE;

   OldEntryProc=classinfo.pfnWindowProc;

   if (!WinRegisterClass(anchor,
                         "FidoEntry",
                         FidoEntryProc,
                         0L,
                         classinfo.cbWindowData+4L))
      return FALSE;

   if (!WinRegisterClass(anchor,
                         "FromToEntry",
                         FromToEntryProc,
                         CS_SYNCPAINT,
                         classinfo.cbWindowData+8L))
      return FALSE;

   if (!WinRegisterClass(anchor,
                         "SubjectEntry",
                         SubjectEntryProc,
                         0L,
                         classinfo.cbWindowData+8L))
      return FALSE;

   return TRUE;
}

/*------------------------------- EditWinProc -------------------------------*/
/* Fensterprozedur der neuen Fensterklasse                                   */

static MRESULT EXPENTRY EditWinProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   LONG position;
   LONG oldpos;
   LONG line;
   LONG length;
   PDRAGINFO pDraginfo;
   PDRAGITEM pditem;
   USHORT cItems, usReply, i;
   char *szFullSourceName;
   char *szSourceName;
   MRESULT mr;

   switch(message)
   {
      case WM_CREATE:
         /* Window-Words vorbelegen */
         WinSetWindowULong(parent, QWL_TABSIZE, 8);
         WinSetWindowULong(parent, QWL_TRANSLATE, FALSE);
         hptrMessage=WinLoadPointer(HWND_DESKTOP, NULLHANDLE, IDIC_MESSAGE);
         break;

      case WM_DESTROY:
         WinDestroyPointer(hptrMessage);
         break;

      case WM_PRESPARAMCHANGED:
         mr=OldEditProc(parent, message, mp1, mp2);
         if ( (ULONG) mp1 == PP_FONTNAMESIZE ||
              (ULONG) mp1 == PP_FONTHANDLE)
         {
            ULONG ulCPs[2], ulReturned;
            FATTRS fattr;
            HPS hps=WinGetPS(parent);

            WinSendMsg(parent, MLM_QUERYFONT, &fattr, NULL);
            GpiSetCharSet(hps, LCID_DEFAULT);
            WinReleasePS(hps);

            DosQueryCp(sizeof(ulCPs), ulCPs, &ulReturned);
            if (ulReturned == 4)
               fattr.usCodePage=ulCPs[0];
            else
               fattr.usCodePage=ulCPs[1];

            WinSendMsg(parent, MLM_SETFONT, &fattr, NULL);
         }
         return mr;

      case WM_CONTEXTMENU:
         Notify(parent, MLN_CONTEXTMENU, mp2);
         break;

      case MLM_SETTRANSLATE:   /* mp1: BOOL */
         WinSetWindowULong(parent, QWL_TRANSLATE, (ULONG) mp1);
         return (MRESULT) TRUE;

      case MLM_QUERYTRANSLATE:
         return (MRESULT) WinQueryWindowULong(parent, QWL_TRANSLATE);

      case MLM_SETTABSIZE:    /* mp1: TAB-Size */
         if (((ULONG) mp1) < 1 ||
             ((ULONG) mp1) >20)
            return (MRESULT) FALSE;
         else
         {
            WinSetWindowULong(parent, QWL_TABSIZE, (ULONG) mp1);
            return (MRESULT) TRUE;
         }

      case MLM_QUERYTABSIZE:
         return (MRESULT) WinQueryWindowULong(parent, QWL_TABSIZE);

      case MLM_DELETELINE:
         WinSendMsg(parent, MLM_DISABLEREFRESH, NULL, NULL);
         /* Cursorposition abfragen */
         position=(LONG)WinSendMsg(parent, MLM_QUERYSEL,
                                   MPFROMSHORT(MLFQS_CURSORSEL), NULL);
         /* Zeilennummer holen */
         line=(LONG)WinSendMsg(parent, MLM_LINEFROMCHAR,
                               MPFROMLONG(position), NULL);
         /* Zeilenanfang holen */
         position=(LONG)WinSendMsg(parent, MLM_CHARFROMLINE,
                                   MPFROMLONG(line), NULL);
         /* Zeilenlaenge holen */
         length=(LONG)WinSendMsg(parent, MLM_QUERYLINELENGTH,
                                 MPFROMLONG(position), NULL);
         WinSendMsg(parent, MLM_SETSEL,
                    MPFROMLONG(position+length),
                    MPFROMLONG(position));
         WinSendMsg(parent, MLM_CLEAR, NULL, NULL);
         WinSendMsg(parent, MLM_ENABLEREFRESH, NULL, NULL);
         return (MRESULT) TRUE;

      case MLM_SPLITQUOTED:
         WinSendMsg(parent, MLM_DISABLEREFRESH, NULL, NULL);
         oldpos=(LONG)WinSendMsg(parent, MLM_QUERYSEL,
                                 MPFROMSHORT(MLFQS_CURSORSEL), NULL);
         position=(LONG)WinSendMsg(parent, MLM_CHARFROMLINE,
                                   MPFROMLONG(-1), NULL);
         length=(LONG)WinSendMsg(parent, MLM_QUERYLINELENGTH,
                                 MPFROMLONG(position), NULL);
         if (length)
         {
            char pchLine[25];
            char *quote;
            LONG lCursor;
            LONG lQueryLen;

            /* Zeile holen */
            if (length<20)
               lQueryLen = length;
            else
               lQueryLen = 20;
            WinSendMsg(parent, MLM_SETSEL, MPFROMLONG(position),
                       MPFROMLONG(position+lQueryLen));
            quote=pchLine+lQueryLen-1;
            WinSendMsg(parent, MLM_QUERYSELTEXT, &pchLine, NULL);
            WinSendMsg(parent, MLM_SETSEL, MPFROMLONG(oldpos),
                       MPFROMLONG(oldpos));

            /* Zeilenwechsel einfuegen */
            WinSendMsg(parent, MLM_INSERT, "\r\n", NULL);

            /* Position f. spaeter sichern */
            lCursor=(LONG)WinSendMsg(parent, MLM_QUERYSEL,
                                     MPFROMSHORT(MLFQS_CURSORSEL), NULL);

            while(quote>pchLine && quote[0]!='>')
               quote--;
            if (quote[0]=='>' &&
                (oldpos-position)>(quote-pchLine) &&
                (oldpos-position)<(length-1))
            {
               quote[1]='\0';
               WinSendMsg(parent, MLM_INSERT, pchLine, NULL);

               /* Leerzeichen sicherstellen */
               oldpos=(LONG)WinSendMsg(parent, MLM_QUERYSEL,
                                       MPFROMSHORT(MLFQS_CURSORSEL), NULL);
               WinSendMsg(parent, MLM_SETSEL, MPFROMLONG(oldpos),
                          MPFROMLONG(oldpos+1));
               WinSendMsg(parent, MLM_QUERYSELTEXT, pchLine, NULL);
               WinSendMsg(parent, MLM_SETSEL, MPFROMLONG(oldpos),
                          MPFROMLONG(oldpos));
               if (pchLine[0] != ' ')
                  WinSendMsg(parent, MLM_INSERT, " ", NULL);

               /* Cursor an Zeilenanfang setzen (gesicherte Position) */
               WinSendMsg(parent, MLM_SETSEL, MPFROMLONG(lCursor), MPFROMLONG(lCursor));
            }
         }
         else
            WinSendMsg(parent, MLM_INSERT, "\r\n", NULL);
         WinSendMsg(parent, MLM_ENABLEREFRESH, NULL, NULL);
         return (MRESULT) FALSE;

#if 0  /* Wird nicht ben., da beim Editieren das MLE immer neu erzeugt wird */
      case MLM_CLEARALL:
         length=(LONG)WinSendMsg(parent,MLM_QUERYTEXTLENGTH,
                                 (MPARAM) 0,
                                 (MPARAM) 0);
         WinSendMsg(parent,MLM_SETSEL,
                    (MPARAM) 0,
                    (MPARAM) length);
         WinSendMsg(parent, MLM_CLEAR, NULL, NULL);
         WinSendMsg(parent, MLM_SETCHANGED, MPFROMSHORT((BOOL)FALSE), NULL);
         return (MRESULT) TRUE;
#endif

      case WM_CHAR:
         /* Enter zum Splitten */
         if ((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
             !(SHORT1FROMMP(mp1) & KC_KEYUP))
            if (SHORT2FROMMP(mp2) == VK_NEWLINE &&
                !WinSendMsg(parent, MLM_QUERYREADONLY, NULL, NULL))
            {
               WinSendMsg(parent, MLM_SPLITQUOTED, NULL, NULL);
               return (MRESULT) TRUE;
            }

         /* Tabulator bearbeiten */
         if ((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
             !(SHORT1FROMMP(mp1) & KC_SHIFT) &&
             !(SHORT1FROMMP(mp1) & KC_CTRL) &&
             !(SHORT1FROMMP(mp1) & KC_ALT) &&
             !(SHORT1FROMMP(mp1) & KC_KEYUP) &&
             SHORT2FROMMP(mp2) == VK_TAB)
         {
            char chBuffer[21];
            LONG lTabSize;

            if (!WinSendMsg(parent, MLM_QUERYREADONLY, NULL, NULL))
            {
               lTabSize=WinQueryWindowULong(parent, QWL_TABSIZE);
               chBuffer[lTabSize--]='\0';
               while(lTabSize>=0)
                  chBuffer[lTabSize--]=' ';
               WinSendMsg(parent, MLM_INSERT, chBuffer, NULL);
            }
            return (MRESULT) TRUE;
         }

         /* Uebersetzungstabelle */
         if (!WinSendMsg(parent, MLM_QUERYREADONLY, NULL, NULL) &&
             WinQueryWindowULong(parent, QWL_TRANSLATE) &&
             !(SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
             !(SHORT1FROMMP(mp1) & KC_KEYUP))
         {
            int i=0;

            while(XLatTable[i].chOrigChar &&
                  XLatTable[i].chOrigChar != (unsigned char)SHORT1FROMMP(mp2))
               i++;

            if (XLatTable[i].chOrigChar)
            {
               WinSendMsg(parent, MLM_INSERT, MPFROMP(XLatTable[i].chNewChars), NULL);
               return (MRESULT) TRUE;
            }
         }
         if (SHORT1FROMMP(mp1) & KC_KEYUP)
            SendCursorPos(parent);
         break;

      case WM_BUTTON1UP:
         mr=OldEditProc(parent, message, mp1, mp2);
         /*SendCursorPos(parent);*/
         return mr;

      case DM_DRAGOVER:
         if (WinSendMsg(parent, MLM_QUERYREADONLY, NULL, NULL))
            return (MRESULT) DOR_NEVERDROP;
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);
         usReply=DOR_DROP;
         cItems = DrgQueryDragitemCount (pDraginfo);
         for (i=0; i< cItems; i++)
         {
            pditem = DrgQueryDragitemPtr (pDraginfo, i);
            if (!DrgVerifyRMF(pditem, "DRM_OS2FILE", NULL)) /* Nur Files */
               usReply = DOR_NEVERDROP;
            if (pditem->fsControl & DC_CONTAINER)     /* keine Verzeichnisse */
               usReply = DOR_NEVERDROP;
            if (pDraginfo->hwndSource==parent)    /* nicht im gleichen Fenster */
               usReply = DOR_NEVERDROP;
         }
         DrgFreeDraginfo(pDraginfo);
         return MRFROM2SHORT(usReply, DO_UNKNOWN);

      case DM_DROP:
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);

         cItems = DrgQueryDragitemCount (pDraginfo);
         for (i=0; i< cItems; i++)
         {
            pditem = DrgQueryDragitemPtr (pDraginfo, i);
            szFullSourceName = malloc(LEN_PATHNAME+1);
            DrgQueryStrName(pditem->hstrContainerName, CCHMAXPATH, szFullSourceName);
            szSourceName=malloc(LEN_PATHNAME+1);
            DrgQueryStrName(pditem->hstrSourceName, CCHMAXPATH, szSourceName);
            strcat(szFullSourceName, szSourceName);
            free(szSourceName);
            WinPostMsg(WinQueryWindow(parent, QW_OWNER), WM_CONTROL,
                       MPFROM2SHORT(WinQueryWindowUShort(parent, QWS_ID), MLN_FILEDROPPED),
                       szFullSourceName);
            WinSendMsg(pditem->hwndItem, DM_ENDCONVERSATION, MPFROMLONG(i), NULL);
         }

         DrgDeleteDraginfoStrHandles(pDraginfo);
         DrgFreeDraginfo(pDraginfo);
         return (MRESULT) FALSE;

      default:
         break;
   }
   return OldEditProc(parent, message, mp1, mp2);
}

static void SendCursorPos(HWND hwnd)
{
   IPT ipt, fchar;
   LONG line;
   SHORT col;
   SHORT sline;

   ipt=(IPT)WinSendMsg(hwnd, MLM_QUERYSEL, MPFROMSHORT(MLFQS_CURSORSEL), NULL);
   line=(LONG)WinSendMsg(hwnd, MLM_LINEFROMCHAR, (MPARAM) ipt, NULL);
   fchar=(IPT)WinSendMsg(hwnd, MLM_CHARFROMLINE, MPFROMLONG(line), NULL);

   sline=(SHORT)line+1;
   col=(SHORT)(ipt-fchar+1);

   Notify(hwnd, MLN_CURSORPOS, MPFROM2SHORT(sline, col));

   return;
}

/*------------------------------- FidoEntryProc -----------------------------*/
/* Fensterprozedur des neuen Entry-Fields                                    */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY FidoEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PDRAGINFO pDraginfo;
   USHORT usReply;
   int i;
   ULONG cItems;
   PDRAGITEM pditem;
   RECTL rcl;
   HPS hps;
   LONG lClr;
   LONG lBackClr, lForeClr;
   char pchTemp[LEN_5DADDRESS+LEN_USERNAME+2];

   switch(message)
   {
      case WM_CHAR:
         if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY)
            if (SHORT2FROMMP(mp2)==VK_SPACE)
               return (MRESULT) FALSE;
            else
               break;
         if (SHORT1FROMMP(mp1) & KC_KEYUP)
            break;
         if (SHORT1FROMMP(mp1) & KC_CHAR)
         {
            i=0;
            while (achAddr[i] && CHAR1FROMMP(mp2)!=achAddr[i])
                  i++;
            if (achAddr[i])
               break;
            WinAlarm(HWND_DESKTOP, WA_NOTE);
            return (MRESULT) FALSE;
         }
         break;

      case WM_CONTEXTMENU:
         Notify(parent, EN_CONTEXTMENU, mp2);
         break;

      case DM_DRAGOVER:
         if (WinSendMsg(parent, EM_QUERYREADONLY, NULL, NULL))
            return (MRESULT) DOR_NEVERDROP;
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);
         usReply=DOR_DROP;
         cItems = DrgQueryDragitemCount (pDraginfo);
         for (i=0; i< cItems; i++)
         {
            pditem = DrgQueryDragitemPtr (pDraginfo, i);
            if (!DrgVerifyRMF(pditem, DRMFLEET, DRFNODE)) /* Nur Nodes */
               usReply = DOR_NEVERDROP;
            if (pditem->fsControl & DC_CONTAINER)     /* keine Verzeichnisse */
               usReply = DOR_NEVERDROP;
            if (pDraginfo->hwndSource==parent)    /* nicht im gleichen Fenster */
               usReply = DOR_NEVERDROP;
         }
         DrgFreeDraginfo(pDraginfo);

         if (WinQueryWindowULong(parent, QWL_USER) == 0)
         {
            POINTL ptl;

            /* Target-Emphasis */
            WinQueryWindowRect(parent, &rcl);
            hps = DrgGetPS(parent);

            lClr = GpiQueryColor(hps);
            GpiSetColor(hps, CLR_DEFAULT);

            ptl.x = 0;
            ptl.y = 0;
            GpiMove(hps, &ptl);

            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yTop-1;

            GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
            GpiSetColor(hps, lClr);
            DrgReleasePS(hps);

            WinSetWindowULong(parent, QWL_USER, 1);
         }
         return MRFROM2SHORT(usReply, DO_UNKNOWN);

      case DM_DRAGLEAVE:
         /* neu zeichnen */
         WinQueryWindowRect(parent, &rcl);
         hps = DrgGetPS(parent);
         GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

         WinQueryWindowText(parent, LEN_5DADDRESS+1, pchTemp);
         if (!WinQueryPresParam(parent,
                                PP_BACKGROUNDCOLOR,
                                PP_BACKGROUNDCOLORINDEX,
                                NULL,
                                sizeof(LONG),
                                (PVOID)&lBackClr,
                                QPF_ID2COLORINDEX))
            lBackClr = WinQuerySysColor(HWND_DESKTOP, SYSCLR_ENTRYFIELD, 0);
         if (!WinQueryPresParam(parent,
                              PP_FOREGROUNDCOLOR,
                              PP_FOREGROUNDCOLORINDEX,
                              NULL,
                              sizeof(LONG),
                              (PVOID)&lForeClr,
                              QPF_ID2COLORINDEX))
            lForeClr = WinQuerySysColor(HWND_DESKTOP, SYSCLR_WINDOWTEXT, 0);

         WinDrawText(hps, -1, pchTemp, &rcl, lForeClr, lBackClr,
                     DT_LEFT | DT_VCENTER | DT_ERASERECT);

         DrgReleasePS(hps);
         WinSetWindowULong(parent, QWL_USER, 0);
         break;

      case DM_DROP:
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);

         pditem = DrgQueryDragitemPtr(pDraginfo, 0);
         DrgQueryStrName(pditem->hstrSourceName, LEN_5DADDRESS, pchTemp);
         if (pchTemp[0])
         {
            PCHAR pchTemp2 = strchr(pchTemp, ' ');
            if (pchTemp2)
            {
               *pchTemp2=0;
               WinSetWindowText(parent, pchTemp);
               *pchTemp2=' '; /* wieder rueckgaengig */
               Notify(parent, EN_NODEDROPPED, pchTemp);
            }
         }
         else
            WinInvalidateRect(parent, NULL, TRUE);
         DrgDeleteDraginfoStrHandles(pDraginfo);
         DrgFreeDraginfo(pDraginfo);
         WinSetWindowULong(parent, QWL_USER, 0);
         return (MRESULT) FALSE;

      default:
         break;
   }
   return OldEntryProc(parent,message,mp1,mp2);
}

/*------------------------------- FromToEntryProc ---------------------------*/
/* Fensterprozedur des neuen Entry-Fields f. From-Name u. To-Name            */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY FromToEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PDRAGINFO pDraginfo;
   USHORT usReply;
   int i;
   ULONG cItems;
   PDRAGITEM pditem;
   RECTL rcl;
   HPS hps;
   LONG lClr;
   LONG lBackClr, lForeClr;
   char pchTemp[LEN_5DADDRESS+LEN_USERNAME+2];

   switch(message)
   {
      case WM_CONTEXTMENU:
         Notify(parent, EN_CONTEXTMENU, mp2);
         break;

      case WM_CHAR:
         /* Uebersetzungstabelle */
         if (!WinSendMsg(parent, EM_QUERYREADONLY, NULL, NULL) &&
             WinQueryWindowULong(parent, QWL_USER) &&
             !(SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
             !(SHORT1FROMMP(mp1) & KC_KEYUP))
         {
            int i=0;

            while(XLatTable[i].chOrigChar &&
                  XLatTable[i].chOrigChar != (unsigned char)SHORT1FROMMP(mp2))
               i++;

            if (XLatTable[i].chOrigChar)
            {
               WinPostMsg(parent, WM_CHAR, MPFROMSH2CH(KC_CHAR, 1, 0),
                          MPFROM2SHORT(XLatTable[i].chNewChars[0], 0));
               if (XLatTable[i].chNewChars[1])
                  WinPostMsg(parent, WM_CHAR, MPFROMSH2CH(KC_CHAR, 1, 0),
                             MPFROM2SHORT(XLatTable[i].chNewChars[1], 0));

               return (MRESULT) TRUE;
            }
         }
         break;

      case MLM_SETTRANSLATE:   /* mp1: BOOL */
         WinSetWindowULong(parent, QWL_USER, (ULONG) mp1);
         return (MRESULT) TRUE;

      case MLM_QUERYTRANSLATE:
         return (MRESULT) WinQueryWindowULong(parent, QWL_USER);

      case DM_DRAGOVER:
         if (WinSendMsg(parent, EM_QUERYREADONLY, NULL, NULL))
            return (MRESULT) DOR_NEVERDROP;
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);
         usReply=DOR_DROP;
         cItems = DrgQueryDragitemCount (pDraginfo);
         for (i=0; i< cItems; i++)
         {
            pditem = DrgQueryDragitemPtr (pDraginfo, i);
            if (!DrgVerifyRMF(pditem, DRMFLEET, DRFNODE)) /* Nur Nodes */
               usReply = DOR_NEVERDROP;
            if (pditem->fsControl & DC_CONTAINER)     /* keine Verzeichnisse */
               usReply = DOR_NEVERDROP;
            if (pDraginfo->hwndSource==parent)    /* nicht im gleichen Fenster */
               usReply = DOR_NEVERDROP;
         }
         DrgFreeDraginfo(pDraginfo);

         if (WinQueryWindowULong(parent, QWL_USER+1) == 0)
         {
            POINTL ptl;

            /* Target-Emphasis */
            WinQueryWindowRect(parent, &rcl);
            hps = DrgGetPS(parent);

            lClr = GpiQueryColor(hps);
            GpiSetColor(hps, CLR_DEFAULT);

            ptl.x = 0;
            ptl.y = 0;
            GpiMove(hps, &ptl);

            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yTop-1;

            GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
            GpiSetColor(hps, lClr);
            DrgReleasePS(hps);

            WinSetWindowULong(parent, QWL_USER+1, 1);
         }

         return MRFROM2SHORT(usReply, DO_UNKNOWN);

      case DM_DRAGLEAVE:
         /* neu zeichnen */
         WinQueryWindowRect(parent, &rcl);
         hps = DrgGetPS(parent);
         GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

         WinQueryWindowText(parent, LEN_USERNAME+1, pchTemp);
         WinQueryPresParam(parent,
                           PP_BACKGROUNDCOLOR,
                           PP_BACKGROUNDCOLORINDEX,
                           NULL,
                           sizeof(LONG),
                           (PVOID)&lBackClr,
                           QPF_ID2COLORINDEX);
         WinQueryPresParam(parent,
                           PP_FOREGROUNDCOLOR,
                           PP_FOREGROUNDCOLORINDEX,
                           NULL,
                           sizeof(LONG),
                           (PVOID)&lForeClr,
                           QPF_ID2COLORINDEX);

         WinDrawText(hps, -1, pchTemp, &rcl, lForeClr, lBackClr,
                     DT_LEFT | DT_VCENTER | DT_ERASERECT);

         DrgReleasePS(hps);
         WinSetWindowULong(parent, QWL_USER+1, 0);
         break;

      case DM_DROP:
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);

         pditem = DrgQueryDragitemPtr(pDraginfo, 0);
         DrgQueryStrName(pditem->hstrSourceName, LEN_USERNAME, pchTemp);
         if (pchTemp[0])
         {
            PCHAR pchTemp2 = strchr(pchTemp, ' ');
            if (pchTemp2)
            {
               pchTemp2++;
               WinSetWindowText(parent, pchTemp2);
               Notify(parent, EN_NODEDROPPED, pchTemp);
            }
         }
         else
            WinInvalidateRect(parent, NULL, TRUE);
         DrgDeleteDraginfoStrHandles(pDraginfo);
         DrgFreeDraginfo(pDraginfo);
         WinSetWindowULong(parent, QWL_USER+1, 0);
         return (MRESULT) FALSE;

      default:
         break;
   }
   return OldEntryProc(parent,message,mp1,mp2);
}

/*------------------------------ SubjectEntryProc ---------------------------*/
/* Fensterprozedur des neuen Entry-Fields f. Subject                         */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY SubjectEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   char pchTemp[LEN_SUBJECT+1];
   char szFullSourceName[CCHMAXPATH];
   char szSourceName[CCHMAXPATH];
   PDRAGINFO pDraginfo;
   PDRAGITEM pditem;
   USHORT cItems, usReply, i ;
   HPS hps;
   RECTL rcl;
   POINTL ptl;
   LONG lClr;
   LONG lBackClr, lForeClr;

   switch(message)
   {
      case WM_CONTEXTMENU:
         Notify(parent, EN_CONTEXTMENU, mp2);
         break;

      case DM_DRAGOVER:
         if (WinSendMsg(parent, EM_QUERYREADONLY, NULL, NULL))
            return (MRESULT) DOR_NEVERDROP;
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);
         usReply=DOR_DROP;
         cItems = DrgQueryDragitemCount (pDraginfo);
         for (i=0; i< cItems; i++)
         {
            pditem = DrgQueryDragitemPtr (pDraginfo, i);
            if (!DrgVerifyRMF(pditem, "DRM_OS2FILE", NULL)) /* Nur Files */
               usReply = DOR_NEVERDROP;
            if (pditem->fsControl & DC_CONTAINER)     /* keine Verzeichnisse */
               usReply = DOR_NEVERDROP;
            if (pDraginfo->hwndSource==parent)    /* nicht im gleichen Fenster */
               usReply = DOR_NEVERDROP;
         }
         DrgFreeDraginfo(pDraginfo);

         if (WinQueryWindowULong(parent, QWL_USER+1) == 0)
         {
            /* Target-Emphasis */
            WinQueryWindowRect(parent, &rcl);
            hps = DrgGetPS(parent);

            lClr = GpiQueryColor(hps);
            GpiSetColor(hps, CLR_DEFAULT);

            ptl.x = 0;
            ptl.y = 0;
            GpiMove(hps, &ptl);

            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yTop-1;

            GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
            GpiSetColor(hps, lClr);
            DrgReleasePS(hps);

            WinSetWindowULong(parent, QWL_USER+1, 1);
         }

         return MRFROM2SHORT(usReply, DO_UNKNOWN);

      case DM_DRAGLEAVE:
         /* neu zeichnen */
         WinQueryWindowRect(parent, &rcl);
         hps = DrgGetPS(parent);
         GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);

         WinQueryWindowText(parent, LEN_SUBJECT+1, pchTemp);
         WinQueryPresParam(parent,
                           PP_BACKGROUNDCOLOR,
                           PP_BACKGROUNDCOLORINDEX,
                           NULL,
                           sizeof(LONG),
                           (PVOID)&lBackClr,
                           QPF_ID2COLORINDEX);
         WinQueryPresParam(parent,
                           PP_FOREGROUNDCOLOR,
                           PP_FOREGROUNDCOLORINDEX,
                           NULL,
                           sizeof(LONG),
                           (PVOID)&lForeClr,
                           QPF_ID2COLORINDEX);

         WinDrawText(hps, -1, pchTemp, &rcl, lForeClr, lBackClr,
                     DT_LEFT | DT_VCENTER | DT_ERASERECT);

         DrgReleasePS(hps);
         WinSetWindowULong(parent, QWL_USER+1, 0);
         break;

      case DM_DROP:
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);
         pchTemp[0]='\0';

         cItems = DrgQueryDragitemCount (pDraginfo);
         for (i=0; i< cItems; i++)
         {
            szFullSourceName[0]='\0';
            szSourceName[0]='\0';
            pditem = DrgQueryDragitemPtr(pDraginfo, i);
            DrgQueryStrName(pditem->hstrContainerName, CCHMAXPATH, szFullSourceName);
            DrgQueryStrName(pditem->hstrSourceName, CCHMAXPATH, szSourceName);
            strcat(szFullSourceName, szSourceName);
            if ((strlen(pchTemp) + strlen(szFullSourceName)+1) <= LEN_SUBJECT)
            {
               if (pchTemp[0])
                  strcat(pchTemp, " ");
               strcat(pchTemp, szFullSourceName);
            }
            WinSendMsg(pditem->hwndItem, DM_ENDCONVERSATION, MPFROMLONG(i),
                       MPFROMLONG(DMFL_TARGETSUCCESSFUL));
         }
         if (pchTemp[0])
         {
            WinSetWindowText(parent, pchTemp);
            Notify(parent, EN_FILEATTACH, pchTemp);
         }
         else
            WinInvalidateRect(parent, NULL, TRUE);
         DrgDeleteDraginfoStrHandles(pDraginfo);
         DrgFreeDraginfo(pDraginfo);
         WinSetWindowULong(parent, QWL_USER+1, 0);
         return (MRESULT) FALSE;

      case WM_CHAR:
         /* Uebersetzungstabelle */
         if (!WinSendMsg(parent, EM_QUERYREADONLY, NULL, NULL) &&
             WinQueryWindowULong(parent, QWL_USER) &&
             !(SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
             !(SHORT1FROMMP(mp1) & KC_KEYUP))
         {
            int i=0;

            while(XLatTable[i].chOrigChar &&
                  XLatTable[i].chOrigChar != (unsigned char)SHORT1FROMMP(mp2))
               i++;

            if (XLatTable[i].chOrigChar)
            {
               WinPostMsg(parent, WM_CHAR, MPFROMSH2CH(KC_CHAR, 1, 0),
                          MPFROM2SHORT(XLatTable[i].chNewChars[0], 0));
               if (XLatTable[i].chNewChars[1])
                  WinPostMsg(parent, WM_CHAR, MPFROMSH2CH(KC_CHAR, 1, 0),
                             MPFROM2SHORT(XLatTable[i].chNewChars[1], 0));

               return (MRESULT) TRUE;
            }
         }
         break;

      case MLM_SETTRANSLATE:   /* mp1: BOOL */
         WinSetWindowULong(parent, QWL_USER, (ULONG) mp1);
         return (MRESULT) TRUE;

      case MLM_QUERYTRANSLATE:
         return (MRESULT) WinQueryWindowULong(parent, QWL_USER);

      default:
         break;
   }
   return OldEntryProc(parent,message,mp1,mp2);
}

/*------------------------------- FileEntryProc -----------------------------*/
/* Fensterprozedur des neuen Entry-Fields f. Filenamen                       */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY FileEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PDRAGINFO pDraginfo;
   USHORT cItems, usReply;
   PDRAGITEM pditem;
   char szFullSourceName[CCHMAXPATH];
   char szSourceName[CCHMAXPATH];

   switch(message)
   {
      case DM_DRAGOVER:
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);
         cItems = DrgQueryDragitemCount (pDraginfo);
         if (cItems > 1)   /* nicht mehr als 1 File */
            usReply=DOR_NEVERDROP;
         else
         {
            pditem = DrgQueryDragitemPtr (pDraginfo, 0);
            if (DrgVerifyRMF(pditem, "DRM_OS2FILE", NULL))
               usReply = DOR_DROP;
            else
               usReply = DOR_NEVERDROP;
         }
         DrgFreeDraginfo(pDraginfo);
         return MRFROM2SHORT(usReply, DO_UNKNOWN);

      case DM_DROP:
         pDraginfo=(PDRAGINFO)mp1;
         DrgAccessDraginfo(pDraginfo);
         pditem = DrgQueryDragitemPtr (pDraginfo, 0);
         DrgQueryStrName(pditem->hstrContainerName, CCHMAXPATH, szFullSourceName);
         DrgQueryStrName(pditem->hstrSourceName, CCHMAXPATH, szSourceName);
         strcat(szFullSourceName, szSourceName);
         WinSetWindowText(parent, szFullSourceName);
         WinPostMsg(WinQueryWindow(parent, QW_OWNER), WM_CONTROL,
                    MPFROM2SHORT(WinQueryWindowUShort(parent, QWS_ID), EN_FILEDROPPED),
                    NULL);

         DrgDeleteDraginfoStrHandles(pDraginfo);
         DrgFreeDraginfo(pDraginfo);
         return (MRESULT) FALSE;

      default:
         break;
   }
   return OldEntryProc(parent,message,mp1,mp2);
}

static MRESULT Notify(HWND hwnd, USHORT usCode, MPARAM mp)
{
   return WinSendMsg(WinQueryWindow(hwnd,QW_OWNER), WM_CONTROL,
                     MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID), usCode), mp);
}


/*------------------------------ Programmende -------------------------------*/


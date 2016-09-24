/*---------------------------------------------------------------------------+
 | Titel: TEMPLATEDLG.C                                                      |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 05.09.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Template-Setup-Dialoge                                                 |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "templatedlg.h"
#include "setupdlg.h"
#include "utility.h"

/*--------------------------------- Defines ---------------------------------*/

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L
#endif

#define NUM_PAGES_TEMPLATE  11
#define MAXLEN_STRING    512

#define UNREG_TEMPLATES  3

#define TPLDRAGTYPE "FleetStreet Template"
#define TPLDRAGRMF  "(DRM_FLEET,DRM_DISCARD)x(DRF_FLEETTEMPLATE)"
#define TPLDRAGRMFRO "<DRM_FLEET,DRF_FLEETTEMPLATE>"

typedef struct {
           MINIRECORDCORE RecordCore;
           HWND           hwndSettings;
           PMSGTEMPLATE   pTemplate;
        } TPLRECORD, *PTPLRECORD;

typedef struct {
           USHORT        cb;
           PMSGTEMPLATE  pTemplate;
        } OPENTEMPLATE, *POPENTEMPLATE;

typedef struct {
           PTPLRECORD    pPopupRecord;
           PTPLRECORD    pDragRecord;
           HWND          hwndPopup;
           HWND          hwndPopup2;
           HPOINTER      hptrTemplate;
           HPOINTER      hptrDefTemplate;
           HPOINTER      hptrTemplateFolder;
           BOOL          bNotify;
           HSWITCH       hSwitch;
           BOOL          bForeground;
        } TPLFOLDERDATA, *PTPLFOLDERDATA;

typedef struct {
           PMSGTEMPLATE pTemplate;
           HWND         notebook;
           NBPAGE       PageTable[NUM_PAGES_TEMPLATE];
           BOOL         bNotify;
        } TEMPLATEBOOKDATA, *PTEMPLATEBOOKDATA;


/*---------------------------- Globale Variablen ----------------------------*/

extern HMODULE hmodLang;
extern HAB anchor;
extern DIRTYFLAGS dirtyflags;
extern HWND hwndTemplates;

static PFNWP OldTplProc;

/*--------------------------- Funktionsprototypen ---------------------------*/

static void InsertTemplatePages(HWND notebook, NBPAGE *Table);
static MRESULT EXPENTRY TEQuoteProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEHeaderProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEFooterProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEReplyProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEDAreaProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEForwardProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEForwardFooterProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEForwardOrderProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEXPostProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TECCopyProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY TEOriginProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

static PTPLRECORD AddEmptyTemplate(HWND hwndContainer, HPOINTER hptr);
static int HaveTemplateName(PVOID pTemplateList, char *pchName);
static PTPLRECORD CopyTemplate(HWND hwndContainer, HPOINTER hptr, PMSGTEMPLATE pOldTpl);
static int FillTemplateFolder(HWND hwndContainer, HPOINTER hptr, HPOINTER hptrDef);
static int OpenTemplate(HWND hwndContainer, PTPLRECORD pTplRecord);
static int DeleteTemplate(HWND hwndContainer, PTPLRECORD pTplRecord);
static int CleanupTemplateFolder(HWND hwndContainer);
static int InitTemplateDrag(HWND hwndDlg, PCNRDRAGINIT pInit);
static MRESULT EXPENTRY NewTplProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);
static void TemplateClosed(HWND hwndCnr, ULONG ulTemplateID);
static MRESULT TemplateDragover(HWND hwnd, PCNRDRAGINFO pDragOver);
static void TemplateDrop(HWND hwnd, PCNRDRAGINFO pCnrDrop, PTPLFOLDERDATA pTplFolderData);


static const struct pagedef
{
   ULONG ulStringID;
   ULONG resID;
   PFNWP DlgProc;
} PageDef[] =
{
   { IDST_TAB_QUOTE,         IDD_TE_QUOTE,         TEQuoteProc},
   { IDST_TAB_HEADER,        IDD_TE_HEADER,        TEHeaderProc},
   { IDST_TAB_FOOTER,        IDD_TE_FOOTER,        TEFooterProc},
   { IDST_TAB_REPLY,         IDD_TE_REPLY,         TEReplyProc},
   { IDST_TAB_DAREA,         IDD_TE_DAREA,         TEDAreaProc},
   { IDST_TAB_FORWARD,       IDD_TE_FORWARD,       TEForwardProc},
   { IDST_TAB_FORWARDFOOTER, IDD_TE_FORWARDFOOTER, TEForwardFooterProc},
   { IDST_TAB_FORWARDORDER,  IDD_TE_FORWARDORDER,  TEForwardOrderProc},
   { IDST_TAB_XPOST,         IDD_TE_XPOST,         TEXPostProc},
   { IDST_TAB_CCOPY,         IDD_TE_CCOPY,         TECCopyProc},
   { IDST_TAB_ORIGIN,        IDD_TE_ORIGIN,        TEOriginProc}
};

/*------------------------------ TemplateBookProc ---------------------------*/
/* Notebook-Dialog des Message-Templates                                     */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY TemplateBookProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;

   POPENTEMPLATE pOpenTemplate;
   extern TEMPLATELIST templatelist;
   PTEMPLATEBOOKDATA pTplBookData = (PTEMPLATEBOOKDATA) WinQueryWindowULong(parent, QWL_USER);
   MRESULT resultbuf=0;

   switch(message)
   {
      case WM_INITDLG:
         /* Leere Seitentabelle */
         pTplBookData = malloc(sizeof(TEMPLATEBOOKDATA));
         memset(pTplBookData, 0, sizeof(TEMPLATEBOOKDATA));
         WinSetWindowULong(parent, QWL_USER, (ULONG) pTplBookData);

         pTplBookData->notebook = WinWindowFromID(parent, IDD_TEMPLATE+1);

         /* Leere Seiten einfuegen */
         InsertTemplatePages(pTplBookData->notebook, pTplBookData->PageTable);

         pOpenTemplate = (POPENTEMPLATE) mp2;
         pTplBookData->pTemplate = pOpenTemplate->pTemplate;

         /* Titel */
         WinSetWindowText(parent, pTplBookData->pTemplate->TName);

         /* erste Seite gleich anzeigen */
         LoadPage(pTplBookData->notebook, &(pTplBookData->PageTable[0]), pOpenTemplate);

         RestoreWinPos(parent, &pTplBookData->pTemplate->TPos, TRUE, TRUE);
         pTplBookData->bNotify = TRUE;
         break;

      case WM_DESTROY:
         free(pTplBookData);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, parent, IDD_TEMPLATE+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pTplBookData && pTplBookData->bNotify)
         {
            if (SaveWinPos(parent, (PSWP) mp1, &pTplBookData->pTemplate->TPos, &pTplBookData->pTemplate->bDirty))
               templatelist.bDirty = TRUE;
         }
         break;

      case WM_QUERYTRACKINFO:
         /* Default-Werte aus Original-Prozedur holen */
         resultbuf=WinDefDlgProc(parent,message,mp1,mp2);

         /* Minimale Fenstergroesse einstellen */
         ((PTRACKINFO)mp2)->ptlMinTrackSize.x=490;
         ((PTRACKINFO)mp2)->ptlMinTrackSize.y=350;

         return resultbuf;

      case WM_CLOSE:
         WinPostMsg(hwndTemplates, TPL_CLOSE,
                    MPFROMLONG(pTplBookData->pTemplate->ulID), NULL);
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, parent);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_TEMPLATE+1)
            if (SHORT2FROMMP(mp1)==BKN_PAGESELECTED)
            {
               int i=0;
               /* Seitenwechsel */
               /* neue Seite in Seiten-Tabelle suchen */
               while (i<NUM_PAGES_TEMPLATE)
               {
                  if (pTplBookData->PageTable[i].PageID == ((PPAGESELECTNOTIFY)mp2)->ulPageIdNew)
                     break;
                  else
                     i++;
               }

               /* Seite ggf. Laden */
               if (i<NUM_PAGES_TEMPLATE && pTplBookData->PageTable[i].hwndPage==NULLHANDLE)
               {
                  OPENTEMPLATE OpenTemplate;

                  OpenTemplate.cb = sizeof(OpenTemplate);
                  OpenTemplate.pTemplate = pTplBookData->pTemplate;
                  LoadPage(pTplBookData->notebook, &(pTplBookData->PageTable[i]), &OpenTemplate);
               }
            }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*---------------------------- InsertTemplatePages --------------------------*/
/* Fuegt alle Seiten in das Notebook ein, Einstellung der Darstellungs-      */
/* Parameter des Notebooks                                                   */
/*---------------------------------------------------------------------------*/

#define NUM_INS_PAGES (sizeof(PageDef)/sizeof(PageDef[0]))

static void InsertTemplatePages(HWND notebook, NBPAGE *Table)
{
   int i;

   SetNotebookParams(notebook, 120);

   /* Leere Seiten einfuegen, Tabelle fuellen */
   for (i=0; i<NUM_INS_PAGES; i++)
   {
      InsertEmptyPage(notebook, PageDef[i].ulStringID, &(Table[i]));
      Table[i].resID=PageDef[i].resID;
      Table[i].DlgProc=PageDef[i].DlgProc;
   }

   return;
}

/*------------------------------ TEQuoteProc      ---------------------------*/
/* Dialogseite Quotes                                                        */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEQuoteProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   BOOL bTemp;
   LONG lTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);
   static char *QuoteChars[] = {">", ":", "|"};

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         WinSendDlgItemMsg(parent,IDD_TE_QUOTE+6, SPBM_SETLIMITS,
                           MPFROMLONG(80), MPFROMLONG(50));
         WinSendDlgItemMsg(parent, IDD_TE_QUOTE+6, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pTemplate->quotelinelen), NULL);
         WinSendDlgItemMsg(parent,IDD_TE_QUOTE+9, SPBM_SETLIMITS,
                           MPFROMLONG(80), MPFROMLONG(50));
         WinSendDlgItemMsg(parent, IDD_TE_QUOTE+9, SPBM_SETCURRENTVALUE,
                           MPFROMLONG(pTemplate->joinlen), NULL);
         WinCheckButton(parent, IDD_TE_QUOTE+2, pTemplate->useinitials);
         WinCheckButton(parent, IDD_TE_QUOTE+3, !pTemplate->useinitials);

         /* Quote-Zeichen */
         WinSendDlgItemMsg(parent, IDD_TE_QUOTE+11, SPBM_SETARRAY,
                           QuoteChars,
                           MPFROMSHORT(sizeof(QuoteChars)/sizeof(QuoteChars[0])));
         for (lTemp = 0; lTemp <sizeof(QuoteChars)/sizeof(QuoteChars[0]); lTemp++)
         {
            if (QuoteChars[lTemp][0] == pTemplate->chQuoteChar)
            {
               WinSendDlgItemMsg(parent, IDD_TE_QUOTE+11, SPBM_SETCURRENTVALUE,
                                 MPFROMLONG(lTemp), NULL);
               break;
            }
         }

         SetFocusControl(parent, IDD_TE_QUOTE+6);
         return (MRESULT) TRUE;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         switch (SHORT1FROMMP(mp1))
         {
            /* Spin-Button */
            case IDD_TE_QUOTE+9:
            case IDD_TE_QUOTE+6:
               if (!SendMsg((HWND)mp2, SPBM_QUERYVALUE,
                                      MPFROMP(&lTemp),
                                      MPFROM2SHORT((USHORT)0, SPBQ_ALWAYSUPDATE)))
                  DosBeep(1000,100);
               break;

            default:
               break;
         }
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         bTemp=WinQueryButtonCheckstate(parent, IDD_TE_QUOTE+2);
         if (pTemplate->useinitials != bTemp)
         {
            pTemplate->useinitials = bTemp;
            pTemplate->bDirty=TRUE;
            templatelist.bDirty=TRUE;
         }
         WinSendDlgItemMsg(parent, IDD_TE_QUOTE+6, SPBM_QUERYVALUE,
                       MPFROMP(&lTemp),
                       MPFROM2SHORT((USHORT)0, SPBQ_ALWAYSUPDATE));
         if (pTemplate->quotelinelen != lTemp)
         {
            pTemplate->quotelinelen = lTemp;
            pTemplate->bDirty=TRUE;
            templatelist.bDirty=TRUE;
         }
         WinSendDlgItemMsg(parent, IDD_TE_QUOTE+9, SPBM_QUERYVALUE,
                       MPFROMP(&lTemp),
                       MPFROM2SHORT((USHORT)0, SPBQ_ALWAYSUPDATE));
         if (pTemplate->joinlen != lTemp)
         {
            pTemplate->joinlen = lTemp;
            pTemplate->bDirty=TRUE;
            templatelist.bDirty=TRUE;
         }
         {
            char chNewChar[2];

            WinSendDlgItemMsg(parent, IDD_TE_QUOTE+11, SPBM_QUERYVALUE,
                              chNewChar,
                              MPFROM2SHORT(sizeof(chNewChar), SPBQ_ALWAYSUPDATE));

            if (pTemplate->chQuoteChar != chNewChar[0])
            {
               pTemplate->chQuoteChar = chNewChar[0];
               pTemplate->bDirty=TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEHeaderProc     ---------------------------*/
/* Dialogseite Header                                                        */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEHeaderProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         if (pTemplate->THeader)
            WinSetDlgItemText(parent, IDD_TE_HEADER+3, pTemplate->THeader);
         WinSendDlgItemMsg(parent, IDD_TE_HEADER+5, EM_SETTEXTLIMIT, MPFROMSHORT(LEN_USERNAME), NULL);
         WinSetDlgItemText(parent, IDD_TE_HEADER+5, pTemplate->TAllSyn);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         {
            ULONG textlen;
            char *pchTemp;
            char pchAllSyn[LEN_USERNAME+1]="";

            textlen=WinQueryDlgItemTextLength(parent, IDD_TE_HEADER+3);
            if (textlen)
            {
               pchTemp=malloc(textlen+1);
               WinQueryDlgItemText(parent, IDD_TE_HEADER+3, textlen+1, pchTemp);
               if (strcmp(pchTemp, pTemplate->THeader))
               {
                  free(pTemplate->THeader);
                  pTemplate->THeader=pchTemp;
                  pTemplate->bDirty = TRUE;
                  templatelist.bDirty=TRUE;
               }
               else
                  free(pchTemp);
            }
            else
            {
               if (pTemplate->THeader[0])
               {
                  pTemplate->THeader[0]='\0';
                  pTemplate->bDirty = TRUE;
                  templatelist.bDirty=TRUE;
               }
            }

            WinQueryDlgItemText(parent, IDD_TE_HEADER+5, sizeof(pchAllSyn), pchAllSyn);
            if (strcmp(pTemplate->TAllSyn, pchAllSyn))
            {
               strcpy(pTemplate->TAllSyn, pchAllSyn);
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEFooterProc     ---------------------------*/
/* Dialogseite Footer                                                        */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEFooterProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   ULONG textlen;
   char *pchTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         if (pTemplate->TFooter)
            WinSetDlgItemText(parent, IDD_TE_FOOTER+3, pTemplate->TFooter);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         textlen=WinQueryDlgItemTextLength(parent, IDD_TE_FOOTER+3);
         if (textlen)
         {
            pchTemp=malloc(textlen+1);
            WinQueryDlgItemText(parent, IDD_TE_FOOTER+3, textlen+1, pchTemp);
            if (strcmp(pchTemp, pTemplate->TFooter))
            {
               free(pTemplate->TFooter);
               pTemplate->TFooter=pchTemp;
               pTemplate->bDirty=TRUE;
               templatelist.bDirty = TRUE;
            }
            else
               free(pchTemp);
         }
         else
         {
            if (pTemplate->TFooter[0])
            {
               pTemplate->TFooter[0]='\0';
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEReplyProc      ---------------------------*/
/* Dialogseite Reply                                                         */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEReplyProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   ULONG textlen;
   char *pchTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;
         if (pTemplate->TReply)
            WinSetDlgItemText(parent, IDD_TE_REPLY+3, pTemplate->TReply);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         textlen=WinQueryDlgItemTextLength(parent, IDD_TE_REPLY+3);
         if (textlen)
         {
            pchTemp=malloc(textlen+1);
            WinQueryDlgItemText(parent, IDD_TE_REPLY+3, textlen+1, pchTemp);
            if (strcmp(pchTemp, pTemplate->TReply))
            {
               free(pTemplate->TReply);
               pTemplate->TReply=pchTemp;
               pTemplate->bDirty=TRUE;
               templatelist.bDirty = TRUE;
            }
            else
               free(pchTemp);
         }
         else
         {
            if (pTemplate->TReply[0])
            {
               pTemplate->TReply[0]='\0';
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEDAreaProc      ---------------------------*/
/* Dialogseite DArea                                                         */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEDAreaProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   ULONG textlen;
   char *pchTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         WinSendDlgItemMsg(parent, IDD_TE_DAREA+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(80), NULL);
         if (pTemplate->TDArea)
            WinSetDlgItemText(parent, IDD_TE_DAREA+3, pTemplate->TDArea);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         textlen=WinQueryDlgItemTextLength(parent, IDD_TE_DAREA+3);
         if (textlen)
         {
            pchTemp=malloc(textlen+1);
            WinQueryDlgItemText(parent, IDD_TE_DAREA+3, textlen+1, pchTemp);
            if (strcmp(pchTemp, pTemplate->TDArea))
            {
               free(pTemplate->TDArea);
               pTemplate->TDArea=pchTemp;
               pTemplate->bDirty=TRUE;
               templatelist.bDirty=TRUE;
            }
            else
               free(pchTemp);
         }
         else
         {
            if (pTemplate->TDArea[0])
            {
               pTemplate->TDArea[0]='\0';
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEForwardProc    ---------------------------*/
/* Dialogseite Forward                                                       */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEForwardProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   ULONG textlen;
   char *pchTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;
         if (pTemplate->TForward)
            WinSetDlgItemText(parent, IDD_TE_FORWARD+3, pTemplate->TForward);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         textlen=WinQueryDlgItemTextLength(parent, IDD_TE_FORWARD+3);
         if (textlen)
         {
            pchTemp=malloc(textlen+1);
            WinQueryDlgItemText(parent, IDD_TE_FORWARD+3, textlen+1, pchTemp);
            if (strcmp(pchTemp, pTemplate->TForward))
            {
               free(pTemplate->TForward);
               pTemplate->TForward=pchTemp;
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
            else
               free(pchTemp);
         }
         else
         {
            if (pTemplate->TForward[0])
            {
               pTemplate->TForward[0]='\0';
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEForwardFooterProc-------------------------*/
/* Dialogseite Forward-Footer                                                */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEForwardFooterProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   ULONG textlen;
   char *pchTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         if (pTemplate->TForwardFooter)
            WinSetDlgItemText(parent, IDD_TE_FORWARDFOOTER+3, pTemplate->TForwardFooter);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         textlen=WinQueryDlgItemTextLength(parent, IDD_TE_FORWARDFOOTER+3);
         if (textlen)
         {
            pchTemp=malloc(textlen+1);
            WinQueryDlgItemText(parent, IDD_TE_FORWARDFOOTER+3, textlen+1, pchTemp);
            if (strcmp(pchTemp, pTemplate->TForwardFooter))
            {
               free(pTemplate->TForwardFooter);
               pTemplate->TForwardFooter=pchTemp;
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
            else
               free(pchTemp);
         }
         else
         {
            if (pTemplate->TForwardFooter[0])
            {
               pTemplate->TForwardFooter[0]='\0';
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEForwardOrderProc -------------------------*/
/* Dialogseite Forward-Order                                                 */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEForwardOrderProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   BOOL bTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         if (pTemplate->forwardfirst)
         {
            WinCheckButton(parent, IDD_TE_FORWARDORDER+3, FALSE);
            WinCheckButton(parent, IDD_TE_FORWARDORDER+4, TRUE);
         }
         else
         {
            WinCheckButton(parent, IDD_TE_FORWARDORDER+3, TRUE);
            WinCheckButton(parent, IDD_TE_FORWARDORDER+4, FALSE);
         }
         SetFocusControl(parent, IDD_TE_FORWARDORDER+3);
         return (MRESULT) TRUE;

      case WM_DESTROY:
         bTemp=WinQueryButtonCheckstate(parent, IDD_TE_FORWARDORDER+4);
         if (pTemplate->forwardfirst != bTemp)
         {
            pTemplate->forwardfirst = bTemp;
            pTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEXPostProc      ---------------------------*/
/* Dialogseite XPost                                                         */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEXPostProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   ULONG textlen;
   char *pchTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         WinSendDlgItemMsg(parent, IDD_TE_XPOST+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(80), NULL);
         if (pTemplate->TXPost)
            WinSetDlgItemText(parent, IDD_TE_XPOST+3, pTemplate->TXPost);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         textlen=WinQueryDlgItemTextLength(parent, IDD_TE_XPOST+3);
         if (textlen)
         {
            pchTemp=malloc(textlen+1);
            WinQueryDlgItemText(parent, IDD_TE_XPOST+3, textlen+1, pchTemp);
            if (strcmp(pchTemp, pTemplate->TXPost))
            {
               free(pTemplate->TXPost);
               pTemplate->TXPost=pchTemp;
               pTemplate->bDirty=TRUE;
               templatelist.bDirty=TRUE;
            }
            else
               free(pchTemp);
         }
         else
         {
            if (pTemplate->TXPost[0])
            {
               pTemplate->TXPost[0]='\0';
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TECCopyProc      ---------------------------*/
/* Dialogseite CCopy                                                         */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TECCopyProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   ULONG textlen;
   char *pchTemp;
   extern TEMPLATELIST templatelist;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         WinSendDlgItemMsg(parent, IDD_TE_CCOPY+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(80), NULL);
         if (pTemplate->TCCopy)
            WinSetDlgItemText(parent, IDD_TE_CCOPY+3, pTemplate->TCCopy);
         break;

      case WM_COMMAND:
         return (MRESULT) FALSE;

      case WM_CONTROL:
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         textlen=WinQueryDlgItemTextLength(parent, IDD_TE_CCOPY+3);
         if (textlen)
         {
            pchTemp=malloc(textlen+1);
            WinQueryDlgItemText(parent, IDD_TE_CCOPY+3, textlen+1, pchTemp);
            if (strcmp(pchTemp, pTemplate->TCCopy))
            {
               free(pTemplate->TCCopy);
               pTemplate->TCCopy=pchTemp;
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
            else
               free(pchTemp);
         }
         else
         {
            if (pTemplate->TCCopy[0])
            {
               pTemplate->TCCopy[0]='\0';
               pTemplate->bDirty = TRUE;
               templatelist.bDirty=TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ TEOriginProc     ---------------------------*/
/* Dialogseite Origin                                                        */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY TEOriginProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern TEMPLATELIST templatelist;
   extern BOOL isregistered;
   POPENTEMPLATE pOpenTemplate;
   PMSGTEMPLATE pTemplate = (PMSGTEMPLATE) WinQueryWindowULong(parent, QWL_USER);
   BOOL bTemp;
   char pchTemp[LEN_PATHNAME+1];

   switch (message)
   {
      case WM_INITDLG:
         pOpenTemplate = (POPENTEMPLATE) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pOpenTemplate->pTemplate);
         pTemplate = pOpenTemplate->pTemplate;

         WinSendDlgItemMsg(parent, IDD_TE_ORIGIN+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_ORIGIN), NULL);
         WinSendDlgItemMsg(parent, IDD_TE_ORIGIN+6, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSetDlgItemText(parent, IDD_TE_ORIGIN+3, pTemplate->TOrigin);
         WinSetDlgItemText(parent, IDD_TE_ORIGIN+6, pTemplate->TOriginFile);
         if (pTemplate->randomorigin)
         {
            WinCheckButton(parent, IDD_TE_ORIGIN+5, TRUE);
            WinEnableControl(parent, IDD_TE_ORIGIN+8, TRUE); /* Text */
            WinEnableControl(parent, IDD_TE_ORIGIN+6, TRUE); /* File */
            WinEnableControl(parent, IDD_TE_ORIGIN+7, TRUE); /* Button */

            WinEnableControl(parent, IDD_TE_ORIGIN+3, FALSE); /* Origin-Text */
         }
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == IDD_TE_ORIGIN+7)
         {
            WinQueryDlgItemText(parent, IDD_TE_ORIGIN+6, sizeof(pchTemp), pchTemp);
            if (GetPathname(parent, pchTemp) == DID_OK)
               WinSetDlgItemText(parent, IDD_TE_ORIGIN+6, pchTemp);
         }
         return (MRESULT) FALSE;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_TE_ORIGIN+5 &&
             (SHORT2FROMMP(mp1) == BN_CLICKED ||
              SHORT2FROMMP(mp1) == BN_DBLCLICKED))
         {
            bTemp= WinQueryButtonCheckstate(parent, IDD_TE_ORIGIN+5);

            WinEnableControl(parent, IDD_TE_ORIGIN+8, bTemp); /* Text */
            WinEnableControl(parent, IDD_TE_ORIGIN+6, bTemp); /* File */
            WinEnableControl(parent, IDD_TE_ORIGIN+7, bTemp); /* Button */

            WinEnableControl(parent, IDD_TE_ORIGIN+3, !bTemp); /* Origin-Text */
         }
         break;

      case WM_DESTROY:
         bTemp = WinQueryButtonCheckstate(parent, IDD_TE_ORIGIN+5);
         if (pTemplate->randomorigin != bTemp)
         {
            pTemplate->randomorigin = bTemp;
            pTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         WinQueryDlgItemText(parent, IDD_TE_ORIGIN+3, LEN_ORIGIN+1, pchTemp);
         if (strcmp(pTemplate->TOrigin, pchTemp))
         {
            strcpy(pTemplate->TOrigin, pchTemp);
            pTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         WinQueryDlgItemText(parent, IDD_TE_ORIGIN+6, LEN_PATHNAME+1, pchTemp);
         if (strcmp(pTemplate->TOriginFile, pchTemp))
         {
            strcpy(pTemplate->TOriginFile, pchTemp);
            pTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: AddEmptyTemplate                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt ein neues Template                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Templates                         */
/*            hptr: verwendetes Icon                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Pointer auf neuen Template-Record                          */
/*                NULL: Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PTPLRECORD AddEmptyTemplate(HWND hwndContainer, HPOINTER hptr)
{
   extern TEMPLATELIST templatelist;
   PMSGTEMPLATE pTemplate, pNewTemplate;
   RECORDINSERT RecordInsert;
   PTPLRECORD pNewRecord;
   ULONG ulNewID=0;
   int iCount=0;
   extern BOOL isregistered;

   /* Ende der Templatekette suchen */
   pTemplate = templatelist.pTemplates;
   while (pTemplate)
   {
      iCount++;

      if (pTemplate->ulID > ulNewID)
         ulNewID = pTemplate->ulID;
      pTemplate = pTemplate->next;
   }
   ulNewID++;

   /* neues Template erzeugen */
   pNewTemplate = calloc(1, sizeof(MSGTEMPLATE));

   pNewTemplate->TName=malloc(MAXLEN_STRING);
   CreateUniqueName(IDST_TPL_NEWNAME, &templatelist, HaveTemplateName, MAXLEN_STRING, pNewTemplate->TName);

   /* vorne anhaengen */
   pNewTemplate->next = templatelist.pTemplates;
   pNewTemplate->prev = NULL;
   templatelist.pTemplates->prev = pNewTemplate;
   templatelist.pTemplates = pNewTemplate;

   templatelist.ulNumTemplates++;
   templatelist.bDirty = TRUE;

   /* Default-Daten setzen */
   LoadDefaultTemplate(pNewTemplate);
   pNewTemplate->ulID = ulNewID;

   /* Record vom Container anfordern */
   pNewRecord = SendMsg(hwndContainer, CM_ALLOCRECORD,
                           MPFROMLONG(sizeof(TPLRECORD) - sizeof(MINIRECORDCORE)),
                           MPFROMLONG(1));

   if (pNewRecord)
   {
      pNewRecord->hwndSettings = NULLHANDLE;
      pNewRecord->pTemplate    = pNewTemplate;

      pNewRecord->RecordCore.flRecordAttr = 0;
      pNewRecord->RecordCore.pszIcon = pNewTemplate->TName;
      pNewRecord->RecordCore.hptrIcon = hptr;

      /* Record einfuegen */
      RecordInsert.cb = sizeof(RECORDINSERT);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent = NULL;
      RecordInsert.fInvalidateRecord = TRUE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = 1;

      SendMsg(hwndContainer, CM_INSERTRECORD, pNewRecord, &RecordInsert);

      return pNewRecord;
   }
   else
      return NULL;
}

void LoadDefaultTemplate(PMSGTEMPLATE pTemplate)
{
   extern USERDATAOPT userdaten;

   pTemplate->bDirty = TRUE;

   pTemplate->quotelinelen=75;
   pTemplate->joinlen=65;
   pTemplate->useinitials=TRUE;
   pTemplate->chQuoteChar = '>';

   pTemplate->THeader=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_HEADER, MAXLEN_STRING, pTemplate->THeader);
   pTemplate->TFooter=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_FOOTER, MAXLEN_STRING, pTemplate->TFooter);
   pTemplate->TReply=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_REPLY, MAXLEN_STRING, pTemplate->TReply);
   pTemplate->TDArea=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_DAREA, MAXLEN_STRING, pTemplate->TDArea);
   pTemplate->TForward=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_FORWARD, MAXLEN_STRING, pTemplate->TForward);
   pTemplate->TForwardFooter=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_FORWARDFOOTER, MAXLEN_STRING, pTemplate->TForwardFooter);
   pTemplate->TXPost=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_XPOST, MAXLEN_STRING, pTemplate->TXPost);
   pTemplate->TCCopy=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_CCOPY, MAXLEN_STRING, pTemplate->TCCopy);
   LoadString(IDST_TPL_ALLSYN, LEN_USERNAME+1, pTemplate->TAllSyn);
   strcpy(pTemplate->TOrigin, userdaten.defaultorigin);

   return;
}

static int HaveTemplateName(PVOID pTemplateList, char *pchName)
{
   PMSGTEMPLATE pTemplate = ((TEMPLATELIST*)pTemplateList)->pTemplates;

   while (pTemplate)
      if (!strcmp(pTemplate->TName, pchName))
         return TRUE;
      else
         pTemplate = pTemplate->next;

   return FALSE;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CopyTemplate                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt ein neues Template aus einem alten                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Templates                         */
/*            hptr: verwendetes Icon                                         */
/*            pOldTpl: Zeiger auf altes Template                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Pointer auf neuen Template-Record                          */
/*                NULL: Fehler                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static PTPLRECORD CopyTemplate(HWND hwndContainer, HPOINTER hptr, PMSGTEMPLATE pOldTpl)
{
   extern TEMPLATELIST templatelist;
   PMSGTEMPLATE pTemplate, pNewTemplate;
   RECORDINSERT RecordInsert;
   PTPLRECORD pNewRecord;
   ULONG ulNewID=0;
   int iCount=0;
   extern BOOL isregistered;

   /* Ende der Templatekette suchen */
   pTemplate = templatelist.pTemplates;
   while (pTemplate)
   {
      iCount++;
      if (pTemplate->ulID > ulNewID)
         ulNewID = pTemplate->ulID;
      pTemplate = pTemplate->next;
   }
   ulNewID++;

   /* neues Template erzeugen */
   pNewTemplate = malloc(sizeof(MSGTEMPLATE));
   memset(pNewTemplate, 0, sizeof(MSGTEMPLATE));

   /* vorne anhaengen */
   pNewTemplate->next = templatelist.pTemplates;
   pNewTemplate->prev = NULL;
   templatelist.pTemplates->prev = pNewTemplate;
   templatelist.pTemplates = pNewTemplate;

   templatelist.ulNumTemplates++;
   templatelist.bDirty = TRUE;

   /* Daten setzen */
   pNewTemplate->bDirty = TRUE;
   pNewTemplate->quotelinelen = pOldTpl->quotelinelen;
   pNewTemplate->joinlen = pOldTpl->joinlen;
   pNewTemplate->useinitials = pOldTpl->useinitials;
   pNewTemplate->forwardfirst = pOldTpl->forwardfirst;
   pNewTemplate->randomorigin = pOldTpl->randomorigin;
   pNewTemplate->ulID = ulNewID;

   /* Strings kopieren */
   pNewTemplate->TFooter=strdup(pOldTpl->TFooter);
   pNewTemplate->THeader=strdup(pOldTpl->THeader);
   pNewTemplate->TReply=strdup(pOldTpl->TReply);
   pNewTemplate->TDArea=strdup(pOldTpl->TDArea);
   pNewTemplate->TForward=strdup(pOldTpl->TForward);
   pNewTemplate->TForwardFooter=strdup(pOldTpl->TForwardFooter);
   pNewTemplate->TXPost=strdup(pOldTpl->TXPost);
   pNewTemplate->TCCopy=strdup(pOldTpl->TCCopy);
   pNewTemplate->TName=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_NEWNAME, MAXLEN_STRING, pNewTemplate->TName);
   strcpy(pNewTemplate->TOrigin, pOldTpl->TOrigin);
   strcpy(pNewTemplate->TOriginFile, pOldTpl->TOriginFile);
   strcpy(pNewTemplate->TAllSyn, pOldTpl->TAllSyn);

   /* Record vom Container anfordern */
   pNewRecord = SendMsg(hwndContainer, CM_ALLOCRECORD,
                           MPFROMLONG(sizeof(TPLRECORD) - sizeof(MINIRECORDCORE)),
                           MPFROMLONG(1));

   if (pNewRecord)
   {
      pNewRecord->hwndSettings = NULLHANDLE;
      pNewRecord->pTemplate    = pNewTemplate;

      pNewRecord->RecordCore.flRecordAttr = 0;
      pNewRecord->RecordCore.pszIcon = pNewTemplate->TName;
      pNewRecord->RecordCore.hptrIcon = hptr;

      /* Record einfuegen */
      RecordInsert.cb = sizeof(RECORDINSERT);
      RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent = NULL;
      RecordInsert.fInvalidateRecord = TRUE;
      RecordInsert.zOrder = CMA_TOP;
      RecordInsert.cRecordsInsert = 1;

      SendMsg(hwndContainer, CM_INSERTRECORD, pNewRecord, &RecordInsert);

      return pNewRecord;
   }
   else
      return NULL;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FillTemplateFolder                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt alle Objekte in den Template-Folder ein               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Templates                         */
/*            hptr: verwendetes Icon                                         */
/*            hptrDef: verwendetes Icon f. Default-Template                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0   OK                                                     */
/*                -1  Fehler                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int FillTemplateFolder(HWND hwndContainer, HPOINTER hptr, HPOINTER hptrDef)
{
   extern TEMPLATELIST templatelist;
   PMSGTEMPLATE pTemplate = templatelist.pTemplates;
   RECORDINSERT RecordInsert;
   PTPLRECORD pRecord, pFirstRecord;

   if (templatelist.ulNumTemplates == 0)
      return -1;

   /* Records vom Container anfordern */
   pFirstRecord = SendMsg(hwndContainer, CM_ALLOCRECORD,
                             MPFROMLONG(sizeof(TPLRECORD) - sizeof(MINIRECORDCORE)),
                             MPFROMLONG(templatelist.ulNumTemplates));
   pRecord = pFirstRecord;

   pTemplate = templatelist.pTemplates;
   while (pRecord)
   {
      pRecord->hwndSettings = NULLHANDLE;
      pRecord->pTemplate    = pTemplate;

      pRecord->RecordCore.flRecordAttr = 0;
      pRecord->RecordCore.pszIcon = pTemplate->TName;

      if (pTemplate->ulID == 0)
         pRecord->RecordCore.hptrIcon = hptrDef;
      else
         pRecord->RecordCore.hptrIcon = hptr;

      pRecord = (PTPLRECORD) pRecord->RecordCore.preccNextRecord;
      pTemplate = pTemplate->next;
   }

   /* Records einfuegen */
   RecordInsert.cb = sizeof(RECORDINSERT);
   RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
   RecordInsert.pRecordParent = NULL;
   RecordInsert.fInvalidateRecord = TRUE;
   RecordInsert.zOrder = CMA_TOP;
   RecordInsert.cRecordsInsert = templatelist.ulNumTemplates;

   SendMsg(hwndContainer, CM_INSERTRECORD, pFirstRecord, &RecordInsert);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenTemplate                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet ein Template-Objekt                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Objekt                            */
/*            pTplRecord: Record-Pointer des Templates                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 OK                                                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Ein bereits offenes Template bekommt lediglich den Fokus       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int OpenTemplate(HWND hwndContainer, PTPLRECORD pTplRecord)
{
   if (pTplRecord->hwndSettings)  /* schon offen ? */
      SetFocus(pTplRecord->hwndSettings);
   else
   {
      OPENTEMPLATE OpenTemplate;

      /* in-use-emphasis setzen */
      SendMsg(hwndContainer, CM_SETRECORDEMPHASIS, pTplRecord,
                 MPFROM2SHORT(TRUE, CRA_INUSE));

      OpenTemplate.cb = sizeof(OpenTemplate);
      OpenTemplate.pTemplate = pTplRecord->pTemplate;

      /* Notebook oeffnen */
      pTplRecord->hwndSettings= WinLoadDlg(HWND_DESKTOP,
                                           HWND_DESKTOP,
                                           TemplateBookProc,
                                           hmodLang,
                                           IDD_TEMPLATE,
                                           &OpenTemplate);
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteTemplate                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht ein Template-Objekt                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Objekt                            */
/*            pTplRecord: Record-Pointer des Templates                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 OK                                                       */
/*                -1 Default-Template nicht loeschbar                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int DeleteTemplate(HWND hwndContainer, PTPLRECORD pTplRecord)
{
   extern TEMPLATELIST templatelist;

   if (pTplRecord->pTemplate->ulID == 0)
      return -1;  /* Default nicht loeschbar */

   /* offenes Notebook schliessen */
   if (pTplRecord->hwndSettings)
      WinDestroyWindow(pTplRecord->hwndSettings);

   /* Record im Container loeschen */
   SendMsg(hwndContainer, CM_REMOVERECORD, &pTplRecord,
              MPFROM2SHORT(1, CMA_INVALIDATE));

   /* Template-Felder freigeben */
   if (pTplRecord->pTemplate->THeader)
      free(pTplRecord->pTemplate->THeader);
   if (pTplRecord->pTemplate->TFooter)
      free(pTplRecord->pTemplate->TFooter);
   if (pTplRecord->pTemplate->TReply)
      free(pTplRecord->pTemplate->TReply);
   if (pTplRecord->pTemplate->TDArea)
      free(pTplRecord->pTemplate->TDArea);
   if (pTplRecord->pTemplate->TForward)
      free(pTplRecord->pTemplate->TForward);
   if (pTplRecord->pTemplate->TForwardFooter)
      free(pTplRecord->pTemplate->TForwardFooter);
   if (pTplRecord->pTemplate->TXPost)
      free(pTplRecord->pTemplate->TXPost);
   if (pTplRecord->pTemplate->TCCopy)
      free(pTplRecord->pTemplate->TCCopy);
   if (pTplRecord->pTemplate->TName)
      free(pTplRecord->pTemplate->TName);

   /* Template selbst loeschen */
   if (pTplRecord->pTemplate->prev)
      pTplRecord->pTemplate->prev->next = pTplRecord->pTemplate->next;
   else
      templatelist.pTemplates = pTplRecord->pTemplate->next;
   if (pTplRecord->pTemplate->next)
     pTplRecord->pTemplate->next->prev = pTplRecord->pTemplate->prev;

   templatelist.ulNumTemplates--;
   templatelist.bDirty = TRUE;

   free(pTplRecord->pTemplate);

   /* endgueltig aus Container entfernen */
   SendMsg(hwndContainer, CM_FREERECORD, &pTplRecord, MPFROMLONG(1));

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CleanupTemplateFolder                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Entfernt alle Objekte aus dem Template-Folder               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Objekt                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 OK                                                       */
/*                -1  Fehler                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Schliesst auch alle offenen Templates                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int CleanupTemplateFolder(HWND hwndContainer)
{
   PTPLRECORD pRecord = NULL;

   /* alle offenen Templates schliessen */
   while (pRecord = SendMsg(hwndContainer, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->hwndSettings)
         WinDestroyWindow(pRecord->hwndSettings);
   }

   /* Folder leeren */
   SendMsg(hwndContainer, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TemplateFolderProc                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fensterprozedur des Template-Folders                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: (WinProc)                                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY TemplateFolderProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern TEMPLATELIST templatelist;
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS  windowfonts;
   extern GENERALOPT generaloptions;
   extern HWND hwndhelp, client;
   PTPLFOLDERDATA pTplFolderData = (PTPLFOLDERDATA) WinQueryWindowULong(hwnd, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         /* Instanzdaten anfordern */
         pTplFolderData = malloc(sizeof(TPLFOLDERDATA));
         memset(pTplFolderData, 0, sizeof(TPLFOLDERDATA));
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pTplFolderData);

         OldTplProc = WinSubclassWindow(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                                        NewTplProc);

         /* Icon laden */
         pTplFolderData->hptrTemplate = LoadIcon(IDIC_TEMPLATE);
         pTplFolderData->hptrDefTemplate = LoadIcon(IDIC_DEFTEMPLATE);
         pTplFolderData->hptrTemplateFolder = LoadIcon(IDIC_TEMPLATEFOLDER);
         SendMsg(hwnd, WM_SETICON, (MPARAM) pTplFolderData->hptrTemplateFolder, NULL);

         /* Switch-Entry */
         pTplFolderData->hSwitch=AddToWindowList(hwnd);

         /* Menues laden */
         pTplFolderData->hwndPopup = WinLoadMenu(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                                                 hmodLang, IDM_TPLF_POPUP);
         pTplFolderData->hwndPopup2 = WinLoadMenu(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                                                  hmodLang, IDM_TPLF_POPUP2);

         if (pTplFolderData->hwndPopup2)
            ReplaceSysMenu(hwnd, pTplFolderData->hwndPopup2, 1);

         if (templatelist.ulFlags & TEMPLATE_FOREGROUND)
         {
            pTplFolderData->bForeground = TRUE;
            WinCheckMenuItem(pTplFolderData->hwndPopup2, IDM_TPLF_FGROUND, TRUE);
            WinSetOwner(hwnd, client);
         }
         else
         {
            pTplFolderData->bForeground = FALSE;
            WinCheckMenuItem(pTplFolderData->hwndPopup2, IDM_TPLF_FGROUND, FALSE);
            WinSetOwner(hwnd, HWND_DESKTOP);
         }

         /* Farben und Font setzen */
         SetBackground(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), &windowcolors.tplfolderback);
         SetForeground(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), &windowcolors.tplfolderfore);
         SetFont(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), windowfonts.tplfolderfont);

         /* Icons einfuegen */
         FillTemplateFolder(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                            pTplFolderData->hptrTemplate,
                            pTplFolderData->hptrDefTemplate);
         RestoreWinPos(hwnd, &templatelist.FolderPos, TRUE, TRUE);
         pTplFolderData->bNotify = TRUE;
         break;

      case WM_DESTROY:
         /* Farben und Font */
         CleanupTemplateFolder(WinWindowFromID(hwnd, IDD_TPLFOLDER+1));
         RemoveFromWindowList(pTplFolderData->hSwitch);
         QueryBackground(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), &windowcolors.tplfolderback);
         QueryForeground(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), &windowcolors.tplfolderfore);
         QueryFont(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), windowfonts.tplfolderfont);
         if (pTplFolderData->hptrTemplate)
            WinDestroyPointer(pTplFolderData->hptrTemplate);
         if (pTplFolderData->hptrTemplateFolder)
            WinDestroyPointer(pTplFolderData->hptrTemplateFolder);
         if (pTplFolderData->hptrDefTemplate)
            WinDestroyPointer(pTplFolderData->hptrDefTemplate);
         if (pTplFolderData->hwndPopup)
            WinDestroyWindow(pTplFolderData->hwndPopup);
         if (pTplFolderData->hwndPopup2)
            WinDestroyWindow(pTplFolderData->hwndPopup2);
         if (pTplFolderData->bForeground)
         {
            if (!(templatelist.ulFlags & TEMPLATE_FOREGROUND))
            {
               templatelist.ulFlags |= TEMPLATE_FOREGROUND;
               templatelist.bDirty = TRUE;
            }
         }
         else
         {
            if (templatelist.ulFlags & TEMPLATE_FOREGROUND)
            {
               templatelist.ulFlags &= ~TEMPLATE_FOREGROUND;
               templatelist.bDirty = TRUE;
            }
         }
         free(pTplFolderData);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_TPLFOLDER+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               PCNREDITDATA pEdit;
               PTPLRECORD pRecord;
               PNOTIFYRECORDENTER pEnter;

               case CN_ENTER:
                  pEnter = (PNOTIFYRECORDENTER) mp2;
                  if (pEnter->pRecord)
                     OpenTemplate(pEnter->hwndCnr,
                                  (PTPLRECORD) pEnter->pRecord);
                  break;

               case CN_REALLOCPSZ:
                  pEdit = (PCNREDITDATA) mp2;
                  pRecord = (PTPLRECORD) pEdit->pRecord;
                  free (pRecord->pTemplate->TName);
                  pRecord->pTemplate->TName = malloc(pEdit->cbText+1);
                  pRecord->pTemplate->TName[0] = '\0';
                  pRecord->RecordCore.pszIcon = pRecord->pTemplate->TName;
                  pRecord->pTemplate->bDirty = TRUE;
                  templatelist.bDirty = TRUE;
                  return (MRESULT) TRUE;

               case CN_ENDEDIT:
                  /* Template offen ? */
                  pEdit = (PCNREDITDATA) mp2;
                  if (((PTPLRECORD)pEdit->pRecord)->hwndSettings)
                     WinSetWindowText(((PTPLRECORD)pEdit->pRecord)->hwndSettings, *pEdit->ppszText);
                  break;

               case CN_INITDRAG:
                  pTplFolderData->pDragRecord = (PTPLRECORD) ((PCNRDRAGINIT) mp2)->pRecord;
                  InitTemplateDrag(hwnd, (PCNRDRAGINIT) mp2);
                  break;

               case CN_DRAGOVER:
                  return TemplateDragover(hwnd, (PCNRDRAGINFO) mp2);

               case CN_DROP:
                  TemplateDrop(hwnd, (PCNRDRAGINFO) mp2, pTplFolderData);
                  break;

               case CN_CONTEXTMENU:
                  pTplFolderData->pPopupRecord = (PTPLRECORD) mp2;
                  if (pTplFolderData->pPopupRecord)
                  {
                     /* Popup-Menue eines Templates */
                     RECTL rcl;
                     POINTL ptl;
                     QUERYRECORDRECT QRecord;

                     if (pTplFolderData->pPopupRecord->pTemplate->ulID == 0)
                        WinEnableMenuItem(pTplFolderData->hwndPopup, IDM_TPLF_DELETE, FALSE);
                     else
                        WinEnableMenuItem(pTplFolderData->hwndPopup, IDM_TPLF_DELETE, TRUE);

                     QRecord.cb = sizeof(QUERYRECORDRECT);
                     QRecord.pRecord = (PRECORDCORE) pTplFolderData->pPopupRecord;
                     QRecord.fRightSplitWindow = FALSE;
                     QRecord.fsExtent = CMA_ICON;
                     WinSendDlgItemMsg(hwnd, IDD_TPLFOLDER+1, CM_QUERYRECORDRECT,
                                       &rcl, &QRecord);
                     ptl.x = rcl.xRight;
                     ptl.y = rcl.yBottom;
                     WinMapWindowPoints(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                                        HWND_DESKTOP, &ptl, 1);
                     WinPopupMenu(HWND_DESKTOP, hwnd, pTplFolderData->hwndPopup,
                                  ptl.x, ptl.y, 0,
                                  PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD |
                                  PU_MOUSEBUTTON1);
                  }
                  else
                  {
                     /* Popup-Menue des Folders */
                     POINTL ptl;

                     WinQueryPointerPos(HWND_DESKTOP, &ptl);
                     WinPopupMenu(HWND_DESKTOP, hwnd, pTplFolderData->hwndPopup2,
                                  ptl.x, ptl.y, 0,
                                  PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD |
                                  PU_MOUSEBUTTON1);
                  }
                  break;

               case CN_HELP:
                  SendMsg(hwnd, WM_HELP, MPFROMSHORT(IDD_TPLFOLDER+1), NULL);
                  break;

               default:
                  break;
            }
         }
         break;

      case DM_DISCARDOBJECT:
         if (mp1)
         {
            DrgAccessDraginfo((PDRAGINFO) mp1);
            DeleteTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                           pTplFolderData->pDragRecord);
            DrgFreeDraginfo((PDRAGINFO) mp1);
         }
         else
            WinAlarm(HWND_DESKTOP, WA_ERROR);
         return (MRESULT) DRR_SOURCE;

      case WM_MENUEND:
         if ((HWND) mp2 == pTplFolderData->hwndPopup ||
             (HWND) mp2 == pTplFolderData->hwndPopup2)
         {
            /* Emphasis wegnehmen */
            WinSendDlgItemMsg(hwnd, IDD_TPLFOLDER+1, CM_SETRECORDEMPHASIS,
                              pTplFolderData->pPopupRecord,
                              MPFROM2SHORT(FALSE, CRA_SOURCE));
            if ( (HWND) mp2 == pTplFolderData->hwndPopup2)
               ResetMenuStyle(pTplFolderData->hwndPopup2, hwnd);
         }
         break;

      case WM_INITMENU:
         if ((HWND) mp2 == pTplFolderData->hwndPopup2)
            pTplFolderData->pPopupRecord=NULL;
         if ((HWND) mp2 == pTplFolderData->hwndPopup ||
             (HWND) mp2 == pTplFolderData->hwndPopup2)
         {
            /* Emphasis setzen */
            WinSendDlgItemMsg(hwnd, IDD_TPLFOLDER+1, CM_SETRECORDEMPHASIS,
                              pTplFolderData->pPopupRecord,
                              MPFROM2SHORT(TRUE, CRA_SOURCE));
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CONTEXTMENU:
      case WM_TEXTEDIT:
         if (!SHORT1FROMMP(mp1) &&
             WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(hwnd, IDD_TPLFOLDER+1))
         {
            WinSendDlgItemMsg(hwnd, IDD_TPLFOLDER+1, message,
                              mp1, mp2);
         }
         break;

      case WM_CLOSE:
         WinPostMsg(client, TPLF_CLOSE, NULL, NULL);
         break;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
            WinShowWindow(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), FALSE);
         if (((PSWP)mp1)->fl & (SWP_MAXIMIZE|SWP_RESTORE))
            WinShowWindow(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), TRUE);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_TPLFOLDER+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pTplFolderData && pTplFolderData->bNotify)
            SaveWinPos(hwnd, (PSWP) mp1, &templatelist.FolderPos, &templatelist.bDirty);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            PTPLRECORD pNewTemplate;

            case IDM_TPLF_OPEN:
               OpenTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                            pTplFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case IDM_TPLF_CREATE:
               if (pNewTemplate = AddEmptyTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                                                   pTplFolderData->hptrTemplate))
                  OpenTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                               pNewTemplate);
               return (MRESULT) FALSE;

            case IDM_TPLF_COPY:
               if (pNewTemplate = CopyTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                                               pTplFolderData->hptrTemplate,
                                               pTplFolderData->pPopupRecord->pTemplate))
                  OpenTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                               pNewTemplate);
               return (MRESULT) FALSE;

            case IDM_TPLF_DELETE:
               if (generaloptions.safety & SAFETY_CHANGESETUP)
                  if (MessageBox(hwnd, IDST_MSG_DELTEMPLATE, IDST_TITLE_DELTEMPLATE,
                                 IDD_DELTEMPLATE, MB_YESNO | MB_WARNING) != MBID_YES)
                     return (MRESULT) FALSE;
               DeleteTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                              pTplFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case IDM_TPLF_FGROUND:
               if (pTplFolderData->bForeground)
               {
                  pTplFolderData->bForeground = FALSE;
                  WinCheckMenuItem(pTplFolderData->hwndPopup2, IDM_TPLF_FGROUND, FALSE);
                  WinSetOwner(hwnd, HWND_DESKTOP);
               }
               else
               {
                  pTplFolderData->bForeground = TRUE;
                  WinCheckMenuItem(pTplFolderData->hwndPopup2, IDM_TPLF_FGROUND, TRUE);
                  WinSetOwner(hwnd, client);
               }
               return (MRESULT) FALSE;

            case DID_CANCEL:
               break;

            default:
               return (MRESULT) FALSE;
         }
         return (MRESULT) FALSE;

      case TPL_CLOSE:
         TemplateClosed(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                        (ULONG) mp1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InitTemplateDrag                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Startet Drag f〉 ein Template                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndDlg: Dialog-Window-Handle                                  */
/*            pInit:   Drag-Init-Infos vom Container                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int InitTemplateDrag(HWND hwndDlg, PCNRDRAGINIT pInit)
{
   PDRAGINFO pDraginfo;
   DRAGITEM dItem;
   DRAGIMAGE dImage;

   if (!pInit->pRecord)
      return -1;

   WinSendDlgItemMsg(hwndDlg, IDD_TPLFOLDER+1, CM_SETRECORDEMPHASIS,
                     pInit->pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

   pDraginfo = DrgAllocDraginfo(1);
   pDraginfo->usOperation=DO_LINK;
   pDraginfo->hwndSource=hwndDlg;

   /* Drag-Item vorbereiten*/
   dItem.hwndItem=hwndDlg;
   dItem.ulItemID= ((PTPLRECORD) pInit->pRecord)->pTemplate->ulID;
   dItem.hstrType=DrgAddStrHandle(TPLDRAGTYPE);
   if (((PTPLRECORD) pInit->pRecord)->pTemplate->ulID == 0)
      dItem.hstrRMF=DrgAddStrHandle(TPLDRAGRMFRO);
   else
      dItem.hstrRMF=DrgAddStrHandle(TPLDRAGRMF);
   dItem.hstrSourceName=DrgAddStrHandle(pInit->pRecord->pszIcon);
   dItem.hstrTargetName=DrgAddStrHandle(pInit->pRecord->pszIcon);
   if (((PTPLRECORD) pInit->pRecord)->hwndSettings)
      dItem.fsControl= DC_OPEN;
   else
      dItem.fsControl=0;
   dItem.fsSupportedOps=DO_LINKABLE|DO_COPYABLE;
   DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);

   /* Drag-Image vorbereiten */
   dImage.cb=sizeof(DRAGIMAGE);
   dImage.hImage=pInit->pRecord->hptrIcon;
   dImage.fl=DRG_ICON;
   dImage.cxOffset=0;
   dImage.cyOffset=0;

   /* Und los gehts */
#if 0
   if (!DrgDrag(hwndDlg, pDraginfo, &dImage, 1, VK_ENDDRAG, NULL))
      DrgDeleteDraginfoStrHandles(pDraginfo);
#else
   DrgDrag(hwndDlg, pDraginfo, &dImage, 1, VK_ENDDRAG, NULL);
#endif
   DrgFreeDraginfo(pDraginfo);

   WinSendDlgItemMsg(hwndDlg, IDD_TPLFOLDER+1, CM_SETRECORDEMPHASIS,
                     pInit->pRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: NewTplProc                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Neue Window-Procedure f. Container wg. Dragover-Bug         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: (WinProc)                                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewTplProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch (message)
   {
      case DM_DRAGOVER:
         DrgAccessDraginfo((PDRAGINFO) mp1);
         break;

      case WM_BUTTON2DOWN:
         return (MRESULT) FALSE;

      default:
         break;
   }

   return OldTplProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TemplateClosed                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Raeumt nach dem Schliessen eines Templates wieder auf       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            ulTemplateID: Template-ID                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TemplateClosed(HWND hwndCnr, ULONG ulTemplateID)
{
   PTPLRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->pTemplate->ulID == ulTemplateID)
      {
         if (pRecord->hwndSettings)
            WinDestroyWindow(pRecord->hwndSettings);
         pRecord->hwndSettings = NULLHANDLE;

         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord,
                    MPFROM2SHORT(FALSE, CRA_INUSE));
         break;
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TemplateDragover                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Behandelt Drag-Over eines Templates                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Template-Folder                                          */
/*            pDragOver: Drag-Infos vom Container                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT wie fuer DM_DRAGOVER                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static MRESULT TemplateDragover(HWND hwnd, PCNRDRAGINFO pDragOver)
{
   USHORT usDrop = DOR_NEVERDROP;
   USHORT usDefaultOp = DO_UNKNOWN;

   hwnd = hwnd;

   if (!pDragOver->pRecord)
   {
      DrgAccessDraginfo(pDragOver->pDragInfo);
      if (pDragOver->pDragInfo->usOperation == DO_COPY)
      {
         if (WinQueryAnchorBlock(pDragOver->pDragInfo->hwndSource) == anchor)
         {
            DRAGITEM dItem;
            DrgQueryDragitem(pDragOver->pDragInfo, sizeof(dItem), &dItem, 0);

            if (DrgVerifyType(&dItem, "FleetStreet Template"))
               usDrop = DOR_DROP;
         }
      }
      else
         usDrop = DOR_NODROP;

      DrgFreeDraginfo(pDragOver->pDragInfo);
   }
   else
      usDrop = DOR_NODROP;

   return MRFROM2SHORT(usDrop, usDefaultOp);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: TemplateDrop                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Behandelt Drop eines Templates (kopiert Template)           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwnd: Template-Folder                                          */
/*            pCnrDrop: Drop-Infos vom Container                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: -                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void TemplateDrop(HWND hwnd, PCNRDRAGINFO pCnrDrop, PTPLFOLDERDATA pTplFolderData)
{
   PTPLRECORD pNewTemplate;

   if (!pCnrDrop->pRecord)
   {
      DrgAccessDraginfo(pCnrDrop->pDragInfo);
      if (pCnrDrop->pDragInfo->usOperation == DO_COPY)
      {
         if (WinQueryAnchorBlock(pCnrDrop->pDragInfo->hwndSource) == anchor)
         {
            DRAGITEM dItem;
            DrgQueryDragitem(pCnrDrop->pDragInfo, sizeof(dItem), &dItem, 0);

            if (DrgVerifyType(&dItem, "FleetStreet Template"))
            {
               /* Template suchen */
               extern TEMPLATELIST templatelist;
               PMSGTEMPLATE pTemplate = templatelist.pTemplates;

               while(pTemplate)
               {
                  if (pTemplate->ulID == dItem.ulItemID)
                     break;
                  pTemplate = pTemplate->next;
               }
               /* Template kopieren */
               pNewTemplate = CopyTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1),
                                           pTplFolderData->hptrTemplate,
                                           pTemplate);
               if (pNewTemplate)
                  OpenTemplate(WinWindowFromID(hwnd, IDD_TPLFOLDER+1), pNewTemplate);
            }
         }
      }

      DrgFreeDraginfo(pCnrDrop->pDragInfo);
   }

   return;
}
/*-------------------------------- Modulende --------------------------------*/


/*---------------------------------------------------------------------------+
 | Titel: DIALOGS.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von:  Michael Hohner           | Am:  06.05.93                   |
 +-----------------------------------------+---------------------------------+
 | System:  OS/2 2.x PM und CSet/2                                           |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Dialog-Funktionen von Fleet Street                                     |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_WIN
#define INCL_BASE
#define INCL_DOSEXCEPTIONS
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
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "controls\editwin.h"
#include "utility.h"
#include "fltv7\fltv7.h"
#include "lookups.h"
#include "nickmanage.h"
#include "dump\expt.h"

/*--------------------------------- Defines ---------------------------------*/

typedef struct
{
   LONG     lMinX;
   LONG     lMinY;
   HPOINTER icon;
   HSWITCH  hSwitch;
} KLUDGEWNDDATA, *PKLUDGEWNDDATA;

/*---------------------------- Globale Variablen ----------------------------*/

extern HAB anchor;
extern HMODULE hmodLang;

/*---------------------------- Lokale Funktionen ----------------------------*/
static void _Optlink RenumberThread(void *pRenumberPar);

/*------------------------------- AttribProc --------------------------------*/
/* Dialog-Prozedur fuer Message-Attribute                                    */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY AttribProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp, frame;
   static ATTRIBPAR *pAttribPar;
   ULONG ulTestMask;
   int i;

   switch(message)
   {
      case WM_INITDLG:
         pAttribPar=(ATTRIBPAR *)mp2;
         WinAssociateHelpInstance(hwndhelp, parent);

         for (i=0, ulTestMask=1UL; i<32; i++, ulTestMask<<= 1)
         {
            if (pAttribPar->ulAttribMask & ulTestMask)
               WinEnableControl(parent, IDD_ATTRIB+1+i, TRUE);

            if (pAttribPar->ulAttrib & ulTestMask)
               WinCheckButton(parent, IDD_ATTRIB+1+i, TRUE);
         }
         RestoreWinPos(parent, &windowpositions.attribpos, FALSE, TRUE);
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         QueryWinPos(parent, &windowpositions.attribpos);
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
            if (SHORT1FROMMP(mp1)==DID_OK)
            {
               if (pAttribPar->bKeepRead)
                  pAttribPar->ulAttrib &= (ATTRIB_SCANNED | ATTRIB_READ);
               else
                  pAttribPar->ulAttrib = 0;

               for (i=1, ulTestMask=1UL; ulTestMask<= ATTRIB_NPD; i++, ulTestMask <<= 1)
               {
                  if (WinQueryButtonCheckstate(parent, IDD_ATTRIB+i))
                     pAttribPar->ulAttrib |= ulTestMask;
               }
            }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}


/*-------------------------------  ReplyProc  -------------------------------*/
/* Dialog-Prozedur des Reply-Dialogs                                         */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY ReplyProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp, frame;
   static REPLYPAR *pReplyPar;

   switch(message)
   {
      case WM_INITDLG:
         pReplyPar=(REPLYPAR *)mp2;

         if (pReplyPar->diffarea)
            WinCheckButton(parent, IDD_REPLY+3, TRUE);

         if (pReplyPar->isfwd)
         {
            WinEnableControl(parent, IDD_REPLY+7, TRUE);
            WinCheckButton(parent, IDD_REPLY+1, TRUE);
            WinSetFocus(HWND_DESKTOP, WinWindowFromID(parent, IDD_REPLY+1));
         }
         else
         {
            WinCheckButton(parent, IDD_REPLY+1, TRUE);
            WinSetFocus(HWND_DESKTOP, WinWindowFromID(parent, IDD_REPLY+1));
         }

         RestoreWinPos(parent, &windowpositions.replypos, FALSE, TRUE);
         WinAssociateHelpInstance(hwndhelp, parent);
         return (MRESULT) TRUE;

      case WM_DESTROY:
      case WM_CLOSE:
         if (WinQueryButtonCheckstate(parent, IDD_REPLY+1))
            pReplyPar->replydest = REPLYDEST_FROM;
         else
            if (WinQueryButtonCheckstate(parent, IDD_REPLY+2))
               pReplyPar->replydest = REPLYDEST_TO;
            else
               pReplyPar->replydest = REPLYDEST_ORIG;
         pReplyPar->diffarea=WinQueryButtonCheckstate(parent, IDD_REPLY+3);
         pReplyPar->quoteoptions = 0;
         if (!WinQueryButtonCheckstate(parent, IDD_REPLY+5))
            pReplyPar->quoteoptions |= QUOTE_TEXT;
         if (WinQueryButtonCheckstate(parent, IDD_REPLY+6))
            pReplyPar->quoteoptions |= QUOTE_NOJOIN;
         QueryWinPos(parent, &windowpositions.replypos);
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      case WM_COMMAND:
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*-----------------------------  KludgesProc  -------------------------------*/
/* Dialog-Prozedur der Kludge-Seenby-Anzeige                                 */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY KludgesProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern HWND client, frame, hwndhelp;
   extern WINDOWPOSITIONS windowpositions;
   extern WINDOWFONTS windowfonts;
   extern WINDOWCOLORS windowcolors;
   ULONG length;
   HWND hwnd;
   int inspoint=0;
   char *ImBuffer;
   SWP swp;
   PKLUDGEWNDDATA pKludgeWndData = WinQueryWindowPtr(parent, QWL_USER);

   switch(message)
   {
      case WM_INITDLG:
         pKludgeWndData = calloc(1, sizeof(KLUDGEWNDDATA));
         WinSetWindowPtr(parent, QWL_USER, pKludgeWndData);

         SetFont(WinWindowFromID(parent, IDD_KLUDGEINFO+1), windowfonts.msginfofont);
         WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1, MLM_SETTEXTCOLOR,
                           MPFROMLONG(windowcolors.msginfofore), NULL);
         WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1, MLM_SETBACKCOLOR,
                           MPFROMLONG(windowcolors.msginfoback), NULL);
         pKludgeWndData->hSwitch=AddToWindowList(parent);

         /* minimale Fensterbreite berechnen */
         WinQueryWindowPos(WinWindowFromID(parent, IDD_KLUDGEINFO+2), &swp);
         pKludgeWndData->lMinX = swp.x+swp.cx+10;
         pKludgeWndData->lMinY = 100;

         pKludgeWndData->icon=LoadIcon(IDB_SHOWKLUDGES);
         SendMsg(parent, WM_SETICON, (MPARAM) pKludgeWndData->icon, (MPARAM) 0);

         RestoreWinPos(parent, &windowpositions.kludgeinfopos, TRUE, TRUE);
         WinAssociateHelpInstance(hwndhelp, parent);
         SetInitialAccel(parent);
         break;

      case WM_QUERYTRACKINFO:
         WinQueryWindowPos(parent, &swp);
         if (swp.fl & SWP_MINIMIZE)
            break;
         else
         {
            MRESULT resultbuf;

            /* Default-Werte aus Original-Prozedur holen */
            resultbuf=WinDefDlgProc(parent, message, mp1, mp2);

            /* Minimale Fenstergroesse einstellen */
            ((PTRACKINFO)mp2)->ptlMinTrackSize.x=pKludgeWndData->lMinX;
            ((PTRACKINFO)mp2)->ptlMinTrackSize.y=pKludgeWndData->lMinY;

            return resultbuf;
         }

      case WM_MINMAXFRAME:
         if (((PSWP) mp1)->fl & SWP_MINIMIZE)
            WinShowWindow(WinWindowFromID(parent, DID_CANCEL), FALSE);
         if (((PSWP) mp1)->fl & (SWP_MAXIMIZE | SWP_RESTORE))
            WinShowWindow(WinWindowFromID(parent, DID_CANCEL), TRUE);
         break;

      case WM_CLOSE:
         WinPostMsg(client, KM_KLUDGEWNDCLOSE, (MPARAM) 0, (MPARAM) 0);
         break;

      case WM_ADJUSTFRAMEPOS:
         if (((PSWP)mp1)->fl & (SWP_SIZE|SWP_MAXIMIZE|SWP_MINIMIZE|SWP_RESTORE))
         {
            RECTL rectl;

            rectl.xLeft=0;
            rectl.xRight=((PSWP)mp1)->cx;
            rectl.yBottom=0;
            rectl.yTop=((PSWP)mp1)->cy;

            CalcClientRect(anchor, parent, &rectl);
            WinQueryWindowPos(WinWindowFromID(parent, DID_CANCEL), &swp);
            rectl.yBottom += swp.y + swp.cy;
            WinSetWindowPos(WinWindowFromID(parent, IDD_KLUDGEINFO+1),
                            NULLHANDLE,
                            rectl.xLeft, rectl.yBottom,
                            rectl.xRight-rectl.xLeft, rectl.yTop-rectl.yBottom,
                            SWP_MOVE | SWP_SIZE);
         }
         break;

      case WM_DESTROY:
         WinDestroyPointer(pKludgeWndData->icon);
         RemoveFromWindowList(pKludgeWndData->hSwitch);
         QueryWinPos(parent, &windowpositions.kludgeinfopos);
         QueryFont(WinWindowFromID(parent, IDD_KLUDGEINFO+1), windowfonts.msginfofont);
         windowcolors.msginfofore=(LONG)WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                          MLM_QUERYTEXTCOLOR, NULL, NULL);
         windowcolors.msginfoback=(LONG)WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                          MLM_QUERYBACKCOLOR, NULL, NULL);
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
         {
            MRESULT minmax;

            switch(SHORT1FROMMP(mp1))
            {
               /* Close-Button */
               case DID_CANCEL:
                  WinPostMsg(parent, WM_CLOSE, (MPARAM) 0, (MPARAM) 0);
                  return (MRESULT) FALSE;

               /* Copy-Button */
               case IDD_KLUDGEINFO+4:
                  minmax=WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1, MLM_QUERYSEL,
                                    MPFROMSHORT(MLFQS_MINMAXSEL),
                                    (MPARAM) 0);
                  if (SHORT2FROMMR(minmax)-SHORT1FROMMR(minmax))
                  {
                     /* Text ist selektiert */
                     WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1, MLM_COPY,
                                       NULL, NULL);
                  }
                  else
                  {
                     /* Text ist nicht selektiert, gesamten Text kopieren */
                     WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                       MLM_DISABLEREFRESH, NULL, NULL);
                     length=(ULONG)WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                     MLM_QUERYTEXTLENGTH, (MPARAM) 0, (MPARAM) 0);
                     WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                       MLM_SETSEL, (MPARAM) 0, MPFROMLONG(length));
                     WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                       MLM_COPY, (MPARAM) 0, (MPARAM) 0);
                     WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                       MLM_SETSEL, (MPARAM) 0, (MPARAM) 0);
                     WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1,
                                       MLM_ENABLEREFRESH, NULL, NULL);
                  }
                  return (MRESULT) FALSE;

               default:
                  return (MRESULT) FALSE;
            }
         }
         if (SHORT1FROMMP(mp2)==CMDSRC_ACCELERATOR)
            return RedirectCommand(mp1, mp2);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_KLUDGEINFO+2)
         {
            if (WinQueryButtonCheckstate(parent, IDD_KLUDGEINFO+2))
               WinSetOwner(parent, client);
            else
               WinSetOwner(parent, HWND_DESKTOP);
         }
         break;

      case KM_SHOWKLUDGES:    /* mp1: PFTNMESSAGE */
         hwnd=WinWindowFromID(parent, IDD_KLUDGEINFO+1);
         SendMsg(hwnd, MLM_DISABLEREFRESH, NULL, NULL);
         length=(ULONG)SendMsg(hwnd, MLM_QUERYTEXTLENGTH, NULL, NULL);
         SendMsg(hwnd, MLM_DELETE, NULL, (MPARAM) length);

         DosAllocMem((PPVOID)&ImBuffer, 32768, OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE);
         ImBuffer[0]='\0';

         if (mp1)
         {
            PKLUDGE pKludge=NULL;

            while(pKludge = MSG_FindKludge((PFTNMESSAGE)mp1, KLUDGE_ANY, pKludge))
            {
               strcat(ImBuffer, "@");
               if (pKludge->ulKludgeType == KLUDGE_OTHER)
               {
                  strcat(ImBuffer, pKludge->pchKludgeText);
                  strcat(ImBuffer, "\n");
               }
               else
               {
                  strcat(ImBuffer, MSG_QueryKludgeName(pKludge->ulKludgeType));
                  strcat(ImBuffer, " ");
                  strcat(ImBuffer, pKludge->pchKludgeText);
                  strcat(ImBuffer, "\n");
               }
            }
            if (((PFTNMESSAGE)mp1)->pchSeenPath)
            {
               if (((PFTNMESSAGE)mp1)->pFirstKludge)
                  strcat(ImBuffer, "-\n");
               strcat(ImBuffer, ((PFTNMESSAGE)mp1)->pchSeenPath);
            }
            WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1, MLM_FORMAT,
                              (MPARAM) MLFIE_NOTRANS, NULL);
            WinSendDlgItemMsg(parent, IDD_KLUDGEINFO+1, MLM_SETIMPORTEXPORT,
                              MPFROMP(ImBuffer), MPFROMLONG(32768));
            SendMsg(hwnd, MLM_IMPORT, MPFROMP(&inspoint), MPFROMLONG(strlen(ImBuffer)));
         }

         SendMsg(hwnd, MLM_ENABLEREFRESH, NULL, NULL);
         DosFreeMem(ImBuffer);
         return (MRESULT) TRUE;

      case WORKM_SWITCHACCELS:
         SwitchAccels(parent, (ULONG) mp1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ AboutProc     ------------------------------*/
/* Dialog-Prozedur des About-Dialogs                                         */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY AboutProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp, frame;

   switch(message)
   {
      RECTL rectl, rectl2;
      SWP swp;

      case WM_INITDLG:
         WinQueryWindowRect(parent, &rectl);
         WinQueryWindowRect(WinWindowFromID(parent, IDD_HELPABOUT+5), &rectl2);
         WinQueryWindowPos(WinWindowFromID(parent, IDD_HELPABOUT+5),
                           &swp);
         WinSetWindowPos(WinWindowFromID(parent, IDD_HELPABOUT+5),
                         NULLHANDLE,
                         (rectl.xRight-rectl2.xRight)/2,
                         swp.y,
                         0,0,
                         SWP_MOVE);

         /* zentrieren */
         WinQueryWindowRect(HWND_DESKTOP, &rectl);
         WinQueryWindowRect(parent, &rectl2);
         WinSetWindowPos(parent,
                         NULLHANDLE,
                         (rectl.xRight-rectl2.xRight)/2,
                         (rectl.yTop-rectl2.yTop)/2,
                         0,0,
                         SWP_MOVE | SWP_SHOW);
         break;

      default:
         break;

   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ ReplyListProc ------------------------------*/
/* Dialog-Prozedur des Reply-List-Dialogs                                    */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY ReplyListProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   static REPLYLISTPAR *pReplyListPar;
   extern HWND hwndhelp, frame;
   extern char CurrentArea[LEN_AREATAG+1];
   extern AREALIST arealiste;
   SHORT sSelect=-1;

   switch(message)
   {
      int i;
      char pchTemp[LEN_USERNAME+10];
      int msgnum;
      MSGHEADER TempHeader;

      case WM_INITDLG:
         WinAssociateHelpInstance(hwndhelp, parent);
         pReplyListPar=(REPLYLISTPAR *) mp2;
         RestoreWinPos(parent, &windowpositions.replylistpos, FALSE, TRUE);
         for (i=0; i<NUM_REPLIES && pReplyListPar->pHeader->ulReplies[i]!=0; i++)
         {
            msgnum=MSG_UidToMsgn(&arealiste, CurrentArea, pReplyListPar->pHeader->ulReplies[i], TRUE);
            if (msgnum)
            {
               MSG_ReadHeader(&TempHeader, &arealiste, CurrentArea, msgnum);
               if (TempHeader.ulAttrib & ATTRIB_READ)
                  sprintf(pchTemp, " %5d: %s", msgnum, TempHeader.pchFromName);
               else
               {
                  sprintf(pchTemp, "*%5d: %s", msgnum, TempHeader.pchFromName);
                  if (sSelect < 0)
                     sSelect = i;
               }
               WinSendDlgItemMsg(parent, IDD_REPLYLIST+1, LM_INSERTITEM,
                                 MPFROMSHORT(LIT_END), MPFROMP(pchTemp));
            }
         }
         if (sSelect <0)
            sSelect = 0;
         WinSendDlgItemMsg(parent, IDD_REPLYLIST+1, LM_SELECTITEM,
                           MPFROMSHORT(sSelect), MPFROMSHORT(TRUE));
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
            if (SHORT1FROMMP(mp1)==DID_OK)
            {
               SHORT sSelect;

               sSelect=(SHORT)WinSendDlgItemMsg(parent, IDD_REPLYLIST+1,
                                                LM_QUERYSELECTION,
                                                MPFROMSHORT(LIT_FIRST), NULL);
               pReplyListPar->Selection=pReplyListPar->pHeader->ulReplies[sSelect];
            }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_REPLYLIST+1)
            if (SHORT2FROMMP(mp1)==LN_ENTER)
               WinPostMsg(parent, WM_COMMAND, MPFROMSHORT(DID_OK),
                          MPFROMSHORT(CMDSRC_PUSHBUTTON));
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         QueryWinPos(parent, &windowpositions.replylistpos);
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ CurrNameProc  ------------------------------*/
/* Dialog-Prozedur des Namens-Wechsel-Dialogs                                */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY CurrNameProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern USERDATAOPT userdaten;
   extern WINDOWPOSITIONS windowpositions;
   extern HWND frame, hwndhelp;
   HWND hwnd;
   int i;
   SHORT select;
   static CURRNAMEPAR *pCurrNamePar;

   switch(message)
   {
      case WM_INITDLG:
         pCurrNamePar=(CURRNAMEPAR *) mp2;
         WinAssociateHelpInstance(hwndhelp, parent);
         RestoreWinPos(parent, &windowpositions.currnamepos, FALSE, TRUE);

         /* Namensliste fuellen */
         hwnd=WinWindowFromID(parent, IDD_CURRENTNAMEADDR+2);
         for (i=0; i<MAX_USERNAMES; i++)
            if (userdaten.username[i][0])
            {
               SendMsg(hwnd, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                          userdaten.username[i]);
               if (!strcmp(userdaten.username[i], pCurrNamePar->CurrName))
                  select=i;
            }
         SendMsg(hwnd, LM_SELECTITEM, MPFROMSHORT(select), MPFROMSHORT(TRUE));

         /* Adressliste fuellen */
         hwnd=WinWindowFromID(parent, IDD_CURRENTNAMEADDR+4);
         for (i=0; i<MAX_ADDRESSES; i++)
            if (userdaten.address[i][0])
            {
               SendMsg(hwnd, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                          userdaten.address[i]);
               if (!strcmp(userdaten.address[i], pCurrNamePar->CurrAddr))
                  select=i;
            }
         SendMsg(hwnd, LM_SELECTITEM, MPFROMSHORT(select), MPFROMSHORT(TRUE));
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
            if (SHORT1FROMMP(mp1)==DID_OK)
            {
               /* Dropdownlisten auslesen */
               select=(SHORT)WinSendDlgItemMsg(parent, IDD_CURRENTNAMEADDR+2,
                                LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST),
                                NULL);
               WinSendDlgItemMsg(parent, IDD_CURRENTNAMEADDR+2, LM_QUERYITEMTEXT,
                                 MPFROM2SHORT(select, LEN_USERNAME+1),
                                 pCurrNamePar->CurrName);

               select=(SHORT)WinSendDlgItemMsg(parent, IDD_CURRENTNAMEADDR+4,
                                LM_QUERYSELECTION, MPFROMSHORT(LIT_FIRST),
                                NULL);
               WinSendDlgItemMsg(parent, IDD_CURRENTNAMEADDR+4, LM_QUERYITEMTEXT,
                                 MPFROM2SHORT(select, LEN_5DADDRESS+1),
                                 pCurrNamePar->CurrAddr);
            }
         break;

      case WM_DESTROY:
         QueryWinPos(parent, &windowpositions.currnamepos);
         WinAssociateHelpInstance(hwndhelp, frame);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*----------------------------- ExportProc      -----------------------------*/
/* File-Dialog fuer Message-Export                                           */
/* ulUser-Feld in der FILEDLG-Struktur:                                      */
/*  Bit 0: with Header;                                                      */
/*  Bit 1: Append to file;                                                   */
/*  Bit 2: Add separator line;                                               */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY ExportProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PFILEDLG pFileDlg;
   extern HWND hwndhelp, client;

   switch (message)
   {
      case WM_INITDLG:
         pFileDlg=(PFILEDLG)WinQueryWindowULong(parent, QWL_USER);
         if (pFileDlg->ulUser & EXPORT_WITHHEADER)
            WinCheckButton(parent, IDD_EXPORTFILE+1, TRUE);
         if (pFileDlg->ulUser & EXPORT_APPEND)
            WinCheckButton(parent, IDD_EXPORTFILE+2, TRUE);
         else
            WinCheckButton(parent, IDD_EXPORTFILE+3, TRUE);
         if (pFileDlg->ulUser & EXPORT_SEPARATOR)
            WinCheckButton(parent, IDD_EXPORTFILE+4, TRUE);
         WinAssociateHelpInstance(hwndhelp, parent);
         break;

      case WM_COMMAND:
         pFileDlg=(PFILEDLG)WinQueryWindowULong(parent, QWL_USER);
         pFileDlg->ulUser=0;
         if (WinQueryButtonCheckstate(parent, IDD_EXPORTFILE+1))
            pFileDlg->ulUser |= EXPORT_WITHHEADER;
         if (WinQueryButtonCheckstate(parent, IDD_EXPORTFILE+2))
            pFileDlg->ulUser |= EXPORT_APPEND;
         if (WinQueryButtonCheckstate(parent, IDD_EXPORTFILE+4))
            pFileDlg->ulUser |= EXPORT_SEPARATOR;
         break;

      case WM_DESTROY:
         WinAssociateHelpInstance(hwndhelp, client);
         break;

      default:
         break;
   }
   return WinDefFileDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ RenumberProc  ------------------------------*/
/* Dialog fuer den Area-Renumber                                             */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY RenumberProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp;
   static BOOL bDoingRenumber;
   static RENUMBERPAR *pRenumberPar;
   WNDPARAMS parameters;
   SLDCDATA  cdata;
   int i;

   switch(message)
   {
      case WM_INITDLG:
         pRenumberPar=(RENUMBERPAR*) mp2;
         pRenumberPar->hwndProgress=parent;

         WinAssociateHelpInstance(hwndhelp, parent);
         bDoingRenumber=FALSE;

         parameters.fsStatus=WPM_CTLDATA;
         parameters.cbCtlData=sizeof(cdata);
         parameters.pCtlData=&cdata;
         cdata.cbSize=sizeof(cdata);
         cdata.usScale2Increments=21;   /* Je 5% */
         cdata.usScale2Spacing=0;

         WinSendDlgItemMsg(parent, IDD_RENUMBER+3,
                           WM_SETWINDOWPARAMS,
                           MPFROMP(&parameters),
                           (MPARAM) NULL);

         WinSendDlgItemMsg(parent, IDD_RENUMBER+3,
                           SLM_SETTICKSIZE,
                           MPFROM2SHORT(SMA_SETALLTICKS,3),
                           (MPARAM) NULL);

         WinSendDlgItemMsg(parent, IDD_RENUMBER+3, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SHAFTDIMENSIONS, 0),
                           MPFROMSHORT(20));

         for (i=0; i<=20; i+=10)
            WinSendDlgItemMsg(parent, IDD_RENUMBER+3,
                              SLM_SETTICKSIZE,
                              MPFROM2SHORT(i,6),
                              (MPARAM) NULL);

         RestoreWinPos(parent, &windowpositions.renumberpos, FALSE, TRUE);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)==DID_OK)
         {
            WinEnableControl(parent, DID_CANCEL, FALSE);
            WinEnableControl(parent, DID_OK, FALSE);
            if (_beginthread(RenumberThread, NULL, 16384, pRenumberPar)== -1)
               WinAlarm(HWND_DESKTOP, WA_ERROR);
            else
               bDoingRenumber=TRUE;
            return (MRESULT) FALSE;
         }
         break;

      case WM_CLOSE:
         if (bDoingRenumber)
         {
            WinAlarm(HWND_DESKTOP, WA_ERROR);
            return (MRESULT) FALSE;
         }
         else
            break;

      case WM_DESTROY:
         QueryWinPos(parent, &windowpositions.renumberpos);
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         break;

      case RENM_STAGE:
         switch((ULONG) mp1)
         {
            char textline[200];

            case 0:
               LoadString(IDST_RENUM_STAGE1, 200, textline);
               WinSetDlgItemText(parent, IDD_RENUMBER+2, textline);
               break;

            case 1:
               LoadString(IDST_RENUM_STAGE2, 200, textline);
               WinSetDlgItemText(parent, IDD_RENUMBER+2, textline);
               break;

            default:
               WinSetDlgItemText(parent, IDD_RENUMBER+2, "?");
               break;
         }
         break;

      case RENM_PROGRESS:
         WinSendDlgItemMsg(parent, IDD_RENUMBER+3, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                        SMA_INCREMENTVALUE),
                           MPFROMLONG(((ULONG)mp1)*20/((ULONG)mp2)));
         break;

      case RENM_ENDED:
         WinSetDlgItemText(parent, IDD_RENUMBER+2, "");
         WinSendDlgItemMsg(parent, IDD_RENUMBER+3, SLM_SETSLIDERINFO,
                           MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                        SMA_INCREMENTVALUE),
                           MPFROMLONG(20));
         WinEnableControl(parent, DID_CANCEL, TRUE);
         bDoingRenumber=FALSE;
         WinPostMsg(parent, WM_CLOSE, NULL, NULL);
         break;

      case RENM_ERROR:
         WinEnableControl(parent, DID_CANCEL, TRUE);
         bDoingRenumber=FALSE;
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ RenumberThread -----------------------------*/
/* Threadfunktion fr Renumber                                               */
/*---------------------------------------------------------------------------*/

static void _Optlink RenumberThread(void *pRenumberPar)
{
   extern MISCOPTIONS miscoptions;
   extern DRIVEREMAP driveremap;

   INSTALLEXPT("Renumber");

   if (MSG_RenumberArea(((RENUMBERPAR*)pRenumberPar)->arealist,
                        ((RENUMBERPAR*)pRenumberPar)->pchArea,
                        ((RENUMBERPAR*)pRenumberPar)->hwndProgress,
                        miscoptions.lastreadoffset,
                        &driveremap))
      WinPostMsg(((RENUMBERPAR*)pRenumberPar)->hwndProgress, RENM_ERROR, NULL, NULL);
   else
      WinPostMsg(((RENUMBERPAR*)pRenumberPar)->hwndProgress, RENM_ENDED, NULL, NULL);

   DEINSTALLEXPT;

   return;
}

/*------------------------------- Modulende ---------------------------------*/


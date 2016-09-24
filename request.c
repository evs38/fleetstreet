/*---------------------------------------------------------------------------+
 | Titel: REQUEST.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 18.10.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x PM                                                   |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   File-Requests v. FleetStreet                                            |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "structs.h"
#include "dialogids.h"
#include "messages.h"
#include "resids.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "utility.h"
#include "dialogs.h"
#include "setupdlg.h"
#include "finddlg.h"
#include "controls\editwin.h"
#include "controls\listbox.h"
#include "fltv7\fltv7.h"
#include "lookups.h"

#include "request_manage.h"
#include "request.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

typedef struct
{
   PREQUESTPAR pRequestPar;
   SIZEL MinSize;
   PFILELIST pCurrentList;
   FILELISTREAD ListRead;
   int tidRead;
} FREQUESTDATA, *PFREQUESTDATA;

typedef struct
{
   USHORT cb;
   FILELIST FileList;
} LISTADDPAR, *PLISTADDPAR;

typedef struct
{
   USHORT cb;
   char pchFileName[LEN_REQFILE+1];
   char pchPassword[LEN_PASSWORD+1];
} FILEADDPAR, *PFILEADDPAR;

typedef struct
{
   USHORT cb;
   char pchPassword[LEN_PASSWORD+1];
} PASSWORDPAR, *PPASSWORDPAR;

/*---------------------------- Globale Variablen ----------------------------*/

extern HWND client, hwndhelp;
extern HMODULE hmodLang;
extern HAB anchor;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static void RefreshFileList(HWND hwndDlg, PFILELIST pList);
static void FillFilesWindow(HWND hwndDlg, PRAMLIST pRAMList);
static MRESULT EXPENTRY ListAddProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY FileAddProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void AddFile(HWND hwndDlg);
static PFILELIST QueryFileList(HWND hwndDlg);
static void RemovePasswords(HWND hwnd);
static void AddFileName(char *pchDest, char *pchName);
static MRESULT EXPENTRY PasswordProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void SetPassword(HWND hwndDlg);
static void SearchInFiles(HWND hwndDlg, BOOL bSearchNext);
static void ActivateControls(HWND hwndDlg, BOOL bActivate);
static void CreateRequest(HWND hwnd, PREQUESTPAR pRequestPar);
static int LookupOtherLines(HWND hwnd);

/*-----------------------------------------------------------------------------
 | Funktionsname:
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte:
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges:
 |
 +---------------------------------------------------------------------------*/

MRESULT EXPENTRY RequestProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PFREQUESTDATA pRequestData = WinQueryWindowPtr(hwnd, QWL_USER);
   HWND hwndList;
   AREADEFLIST *pTemp;
   BOOL bAreaSelected = FALSE;
   SWP swp;
   extern REQUESTOPT requestoptions;
   PRAMLIST pRAMList;
   ULONG ulStyle;

   switch(msg)
   {
      case WM_INITDLG:
         pRequestData = calloc(1, sizeof(FREQUESTDATA));
         pRequestData->pRequestPar = (PREQUESTPAR) mp2;

         /* Adresse */
         WinSubclassWindow(WinWindowFromID(hwnd, IDD_REQUESTER+7), FidoEntryProc);
         WinSendDlgItemMsg(hwnd, IDD_REQUESTER+7, EM_SETTEXTLIMIT,
                           MPFROMLONG(LEN_5DADDRESS), NULL);
         WinSetDlgItemText(hwnd, IDD_REQUESTER+7, pRequestData->pRequestPar->pchReqAddr);

         /* Areas in Drop-Down-Liste */
         pTemp=pRequestData->pRequestPar->arealist->pFirstArea;
         hwndList=WinWindowFromID(hwnd, IDD_REQUESTER+9);
         WinEnableWindowUpdate(hwndList, FALSE);
         while(pTemp)
         {
            if (pTemp->areadata.areatype == AREATYPE_NET)
            {
               SHORT sItem;

               sItem=(SHORT)SendMsg(hwndList, LM_INSERTITEM,
                                       MPFROMSHORT(LIT_END),
                                       pTemp->areadata.areatag);
               if (*(pRequestData->pRequestPar->pchDestArea) &&
                   !stricmp(pRequestData->pRequestPar->pchDestArea, pTemp->areadata.areatag))
               {
                  SendMsg(hwndList, LM_SELECTITEM,
                             MPFROMSHORT(sItem), MPFROMLONG(TRUE));
                  bAreaSelected=TRUE;
               }
            }

            pTemp=pTemp->next;
         }
         if (!bAreaSelected)
         {
            SHORT sItem=0;

            sItem=(SHORT)SendMsg(hwndList, LM_QUERYTOPINDEX, NULL, NULL);
            SendMsg(hwndList, LM_SELECTITEM,
                       MPFROMSHORT(sItem), MPFROMLONG(TRUE));
         }
         WinEnableWindowUpdate(hwndList, TRUE);

         /* Direct-Button bedienen */
         WinCheckButton(hwnd, IDD_REQUESTER+11, pRequestData->pRequestPar->bDirect);
         if (pRequestData->pRequestPar->bDirect)
            WinEnableControl(hwnd, IDD_REQUESTER+9, FALSE);
         else
            WinEnableControl(hwnd, IDD_REQUESTER+9, TRUE);

         /* File-Listen */
         RefreshFileList(hwnd, requestoptions.pFirstList);

         pRAMList = MessageToFileList(pRequestData->pRequestPar->pchFiles);

         FillFilesWindow(hwnd, pRAMList);
         FreeFileList(pRAMList);

         /* Min. Fenstergroesse */
         WinQueryWindowPos(WinWindowFromID(hwnd, IDD_REQUESTER+18), &swp);
         pRequestData->MinSize.cx = swp.x + swp.cx + 10;
         WinQueryWindowPos(WinWindowFromID(hwnd, IDD_REQUESTER+12), &swp);
         pRequestData->MinSize.cy = swp.y + swp.cy + 25;

         /* Font, Farben */
         hwndList = WinWindowFromID(hwnd, IDD_REQUESTER+13);
         SetForeground(hwndList, &requestoptions.lListFore);
         SetBackground(hwndList, &requestoptions.lListBack);
         SetFont(hwndList, requestoptions.pchListFont);

         ulStyle = WinQueryWindowULong(hwndList, QWL_STYLE);
         WinSetWindowULong(hwndList, QWL_STYLE, ulStyle | WS_TABSTOP);

         RestoreWinPos(hwnd, &requestoptions.ReqPos, TRUE, TRUE);
         WinSetWindowPtr(hwnd, QWL_USER, pRequestData);
         break;

      case WM_DESTROY:
         {
            LONG lTemp=0;
            char pchTemp[FACESIZE+5]="";

            hwndList = WinWindowFromID(hwnd, IDD_REQUESTER+13);

            QueryForeground(hwndList, &lTemp);
            if (requestoptions.lListFore != lTemp)
            {
               requestoptions.lListFore = lTemp;
               requestoptions.bDirty = TRUE;
            }
            QueryBackground(hwndList, &lTemp);
            if (requestoptions.lListBack != lTemp)
            {
               requestoptions.lListBack = lTemp;
               requestoptions.bDirty = TRUE;
            }
            QueryFont(hwndList, pchTemp);
            if (strcmp(requestoptions.pchListFont, pchTemp))
            {
               strcpy(requestoptions.pchListFont, pchTemp);
               requestoptions.bDirty = TRUE;
            }
         }

         RemovePasswords(hwnd);
         free(pRequestData);
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               CreateRequest(hwnd, pRequestData->pRequestPar);
               break;

            case DID_CANCEL:
               if (pRequestData->tidRead)
               {
                  /* Lese-Thread stoppen, dieser postet dann REQM_LISTREAD */

                  pRequestData->ListRead.bStop = TRUE;
                  return (MRESULT) FALSE;
               }
               WinPostMsg(client, REQM_CLOSE, pRequestData->pRequestPar, MPFROMLONG(DID_CANCEL));
               break;

            /* Attribute */
            case IDD_REQUESTER+10:
               {
                  ATTRIBPAR AttribPar;

                  AttribPar.cb = sizeof(AttribPar);
                  AttribPar.ulAttrib = pRequestData->pRequestPar->ulAttrib;
                  if (WinQueryButtonCheckstate(hwnd, IDD_REQUESTER+11))
                     AttribPar.ulAttribMask = ATTRIB_IMMEDIATE|ATTRIB_CRASH|ATTRIB_DIRECT|ATTRIB_HOLD;
                  else
                     AttribPar.ulAttribMask = 0xffffffff;
                  AttribPar.bKeepRead = FALSE;

                  if (WinDlgBox(HWND_DESKTOP, hwnd, AttribProc, hmodLang,
                                IDD_ATTRIB, &AttribPar) == DID_OK)
                     pRequestData->pRequestPar->ulAttrib = AttribPar.ulAttrib;

               }
               return (MRESULT) FALSE;

            case IDD_REQUESTER+4: /* Liste hinzufuegen */
               {
                  LISTADDPAR ListAddPar;

                  memset(&ListAddPar, 0, sizeof(ListAddPar));
                  ListAddPar.cb = sizeof(ListAddPar);

                  if (WinDlgBox(HWND_DESKTOP, hwnd, ListAddProc, hmodLang,
                                IDD_REQ_ADDLIST, &ListAddPar) == DID_OK)
                  {
                     AddNewFileList(&requestoptions.pFirstList, &ListAddPar.FileList,
                                    &requestoptions.bListDirty);

                     RefreshFileList(hwnd, requestoptions.pFirstList);
                  }
               }
               return (MRESULT) FALSE;

            case IDD_REQUESTER+5: /* Liste loeschen */
               {
                  PFILELIST pDelList = QueryFileList(hwnd);
                  extern GENERALOPT generaloptions;

                  /* Sicherheitsabfrage */
                  if (generaloptions.safety & SAFETY_CHANGESETUP)
                     if (MessageBox(hwnd, IDST_MSG_REQ_DELLIST, IDST_TITLE_REQ_DELLIST,
                                    IDD_REQ_DELLIST, MB_YESNO | MB_QUERY) != MBID_YES)
                        return (MRESULT) FALSE;

                  if (pDelList)
                  {
                     DeleteFileList(&requestoptions.pFirstList, pDelList,
                                    &requestoptions.bListDirty);
                     RefreshFileList(hwnd, requestoptions.pFirstList);
                  }
                  else
                     WinAlarm(HWND_DESKTOP, WA_NOTE);
               }
               return (MRESULT) FALSE;

            case IDD_REQUESTER+14: /* File hinzufuegen */
               AddFile(hwnd);
               return (MRESULT) FALSE;

            case IDD_REQUESTER+15: /* Suchen */
               SearchInFiles(hwnd, FALSE);
               return (MRESULT) FALSE;

            case IDD_REQUESTER+19: /* naechstes Suchen */
               SearchInFiles(hwnd, TRUE);
               return (MRESULT) FALSE;

            case IDD_REQUESTER+16: /* Passwort */
               SetPassword(hwnd);
               return (MRESULT) FALSE;

            case IDD_REQUESTER+17: /* Alle */
               WinSendDlgItemMsg(hwnd, IDD_REQUESTER+13, LM_SELECTITEM,
                                 MPFROMLONG(LIT_ALL), NULL);
               return (MRESULT) FALSE;

            case IDD_REQUESTER+18: /* Keines */
               WinSendDlgItemMsg(hwnd, IDD_REQUESTER+13, LM_SELECTITEM,
                                 MPFROMLONG(LIT_NONE), NULL);
               return (MRESULT) FALSE;

            case IDD_REQUESTER+20: /* Lookup */
               LookupOtherLines(hwnd);
               return (MRESULT) FALSE;

            default:
               return (MRESULT) FALSE;
         }
         break;

      case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
         {
            case IDD_REQUESTER+11: /* Checkbox */
               if (pRequestData->pRequestPar->bDirect = WinQueryButtonCheckstate(hwnd, IDD_REQUESTER+11))
                  WinEnableControl(hwnd, IDD_REQUESTER+9, FALSE);
               else
                  WinEnableControl(hwnd, IDD_REQUESTER+9, TRUE);
               break;

            case IDD_REQUESTER+3: /* File-Listen */
               switch(SHORT2FROMMP(mp1))
               {
                  case LN_ENTER:
                     {
                        PFILELIST pSelList = QueryFileList(hwnd);

                        /* Alten Inhalt l”schen */
                        FillFilesWindow(hwnd, NULL);

                        pRequestData->pCurrentList = pSelList;

                        if (pSelList)
                        {
                           /* echte Liste */
                           WinSetDlgItemText(hwnd, IDD_REQUESTER+7, pSelList->pchAddress);

                           /* Thread-Parameter */
                           memset(&pRequestData->ListRead, 0, sizeof(pRequestData->ListRead));

                           pRequestData->ListRead.pList = pSelList;
                           pRequestData->ListRead.hwndNotify = hwnd;

                           /* Thread starten */
                           ActivateControls(hwnd, FALSE);
                           pRequestData->tidRead = _beginthread(ListReadThread, NULL, 16384, &pRequestData->ListRead);

                           if (pRequestData->tidRead < 0)
                           {
                              pRequestData->tidRead=0;
                              ActivateControls(hwnd, TRUE);
                              WinAlarm(HWND_DESKTOP, WA_ERROR);
                           }
                        }
                        else
                        {
                           /* Pseudo-Liste */
                           pRAMList = MessageToFileList(pRequestData->pRequestPar->pchFiles);
                           FillFilesWindow(hwnd, pRAMList);
                           FreeFileList(pRAMList);
                           WinSetDlgItemText(hwnd, IDD_REQUESTER+7,
                                             pRequestData->pRequestPar->pchReqAddr);
                        }
                     }
                     break;

                  default:
                     break;
               }
               break;

            default:
               break;
         }
         break;

      case WM_ADJUSTFRAMEPOS:
         if (((PSWP)mp1)->fl & (SWP_SIZE|SWP_MAXIMIZE|SWP_RESTORE))
         {
            RECTL rectl;
            LONG lTemp;

            rectl.xLeft=0;
            rectl.xRight=((PSWP)mp1)->cx;
            rectl.yBottom=0;
            rectl.yTop=((PSWP)mp1)->cy;

            CalcClientRect(anchor, hwnd, &rectl);

            /* File-Titel */
            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_REQUESTER+12), &swp);
            WinSetWindowPos(WinWindowFromID(hwnd, IDD_REQUESTER+12),
                            NULLHANDLE,
                            swp.x, rectl.yTop - swp.cy,
                            0, 0,
                            SWP_MOVE);
            lTemp = swp.cy;

            /* File-Liste */
            WinQueryWindowPos(WinWindowFromID(hwnd, IDD_REQUESTER+13), &swp);
            WinSetWindowPos(WinWindowFromID(hwnd, IDD_REQUESTER+13),
                            NULLHANDLE,
                            0, 0,
                            rectl.xRight - swp.x,
                            rectl.yTop - swp.y - lTemp,
                            SWP_SIZE);
         }
         break;

       case WM_WINDOWPOSCHANGED:
         if (pRequestData)
            SaveWinPos(hwnd, (PSWP) mp1, &requestoptions.ReqPos, &requestoptions.bDirty);
         break;

      case WM_QUERYTRACKINFO:
         if (pRequestData)
         {
            MRESULT resultbuf;

            /* Default-Werte aus Original-Prozedur holen */
            resultbuf=WinDefDlgProc(hwnd, msg, mp1, mp2);

            /* Minimale Fenstergroesse einstellen */
            ((PTRACKINFO)mp2)->ptlMinTrackSize.x=pRequestData->MinSize.cx;
            ((PTRACKINFO)mp2)->ptlMinTrackSize.y=pRequestData->MinSize.cy;
            return resultbuf;
         }
         else
            break;

      case WM_CLOSE:
         if (pRequestData->tidRead)
         {
            /* Lese-Thread stoppen, dieser postet dann REQM_LISTREAD */

            pRequestData->ListRead.bStop = TRUE;
            return (MRESULT) FALSE;
         }

         WinPostMsg(client, REQM_CLOSE, pRequestData->pRequestPar, MPFROMLONG(DID_CANCEL));
         break;

      case REQM_LISTREAD:
         pRequestData->tidRead=0;

         /* evtl. Fehlermeldung ausgeben */
         switch(pRequestData->ListRead.ulRetCode)
         {
            case FILELIST_NOTF:
               MessageBox(hwnd, IDST_MSG_REQ_NOTF, 0, IDD_REQ_ERROR, MB_OK);
               break;

            case FILELIST_READERR:
               MessageBox(hwnd, IDST_MSG_REQ_READERR, 0, IDD_REQ_ERROR, MB_OK);
               break;

            case FILELIST_STOPPED:
               /* Abgebrochen durch Close oder Cancel, Close nochmal posten */
               WinPostMsg(hwnd, WM_CLOSE, NULL, NULL);
               break;

            default:
               break;
         }
         ActivateControls(hwnd, TRUE);

         /* gelesene Daten anzeigen */
         FillFilesWindow(hwnd, pRequestData->ListRead.pReadList);
         FreeFileList(pRequestData->ListRead.pReadList);
         return (MRESULT) FALSE;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static void ActivateControls(HWND hwndDlg, BOOL bActivate)
{
   WinEnableControl(hwndDlg, IDD_REQUESTER+3, bActivate);
   WinEnableControl(hwndDlg, IDD_REQUESTER+14, bActivate);
   WinEnableControl(hwndDlg, IDD_REQUESTER+15, bActivate);
   WinEnableControl(hwndDlg, IDD_REQUESTER+16, bActivate);
   WinEnableControl(hwndDlg, IDD_REQUESTER+17, bActivate);
   WinEnableControl(hwndDlg, IDD_REQUESTER+18, bActivate);
   WinEnableControl(hwndDlg, IDD_REQUESTER+19, bActivate);
   WinEnableControl(hwndDlg, DID_OK, bActivate);

   return;
}

static void CreateRequest(HWND hwnd, PREQUESTPAR pRequestPar)
{
   LONG sItem;
   HWND hwndList = WinWindowFromID(hwnd, IDD_REQUESTER+13);
   PREQUESTLIST pLast=NULL;

   sItem = LIT_FIRST;
   while((sItem=(LONG)SendMsg(hwndList, LM_QUERYSELECTION,
                              MPFROMLONG(sItem), NULL))!=LIT_NONE)
   {
      char pchTemp[50];
      char *pchPassword;

      SendMsg(hwndList, LM_QUERYITEMTEXT, MPFROMLONG(sItem), MPFROMP(pchTemp));
      if (IsFileName(pchTemp, FALSE))
      {
         if (pRequestPar->pReqList)
         {
            pLast->next = calloc(1, sizeof(REQUESTLIST));
            pLast = pLast->next;
         }
         else
            pLast = pRequestPar->pReqList = calloc(1, sizeof(REQUESTLIST));

         AddFileName(pLast->pchFileName, pchTemp);
         pchPassword = (PCHAR) SendMsg(hwndList, LM_QUERYITEMHANDLE,
                                                 MPFROMLONG(sItem), NULL);
         if (pchPassword)
            strncpy(pLast->pchPassword, pchPassword, LEN_PASSWORD);
      }
   }
   WinQueryDlgItemText(hwnd, IDD_REQUESTER+7, LEN_5DADDRESS+1, pRequestPar->pchReqAddr);
   sItem=(SHORT)WinSendDlgItemMsg(hwnd, IDD_REQUESTER+9, LM_QUERYSELECTION,
                                  MPFROMSHORT(LIT_FIRST), NULL);
   WinSendDlgItemMsg(hwnd, IDD_REQUESTER+9, LM_QUERYITEMTEXT,
                     MPFROM2SHORT(sItem, LEN_AREATAG+1), pRequestPar->pchDestArea);
   strcpy(pRequestPar->pchReqName, "Sysop");
   SendMsg(client, REQM_REQUEST, pRequestPar, NULL);
   WinPostMsg(client, REQM_CLOSE, pRequestPar, MPFROMLONG(DID_OK));

   return;
}

static void RemovePasswords(HWND hwnd)
{
   HWND hwndList = WinWindowFromID(hwnd, IDD_REQUESTER+13);
   LONG sItem, sItemCount;
   PVOID pTemp;

   /* Passwoerter freigeben */
   sItemCount = (LONG) SendMsg(hwndList, LM_QUERYITEMCOUNT, NULL, NULL);

   for (sItem =0; sItem < sItemCount; sItem++)
   {
      pTemp = SendMsg(hwndList, LM_QUERYITEMHANDLE, MPFROMLONG(sItem), NULL);
      if (pTemp)
         free(pTemp);
   }

   return;
}

static void RefreshFileList(HWND hwndDlg, PFILELIST pList)
{
   HWND hwndList = WinWindowFromID(hwndDlg, IDD_REQUESTER+3);
   SHORT sItem;
   PFILELIST pTemp = pList;

   WinSendMsg(hwndList, LM_DELETEALL, NULL, NULL);

   /* Dummy-Liste einfgen */
   WinSendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_END),
              /* @@ */
              "Message");

   /* alle Listen-Elemente */
   while (pTemp)
   {
      sItem = (SHORT) WinSendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                                 pTemp->pchDesc);

      if (sItem >= 0)
         WinSendMsg(hwndList, LM_SETITEMHANDLE, MPFROMSHORT(sItem), pTemp);

      pTemp = pTemp->next;
   }

   /* Loesch-Button */
   if (pList)
      WinEnableControl(hwndDlg, IDD_REQUESTER+5, TRUE);
   else
      WinEnableControl(hwndDlg, IDD_REQUESTER+5, FALSE);

   return;
}

static void FillFilesWindow(HWND hwndDlg, PRAMLIST pRAMList)
{
   HWND hwndList = WinWindowFromID(hwndDlg, IDD_REQUESTER+13);
   LBOXINFO InsertInfo = {LIT_END, 0, 0, 0};
   PRAMLIST pTemp = pRAMList;
   PCHAR *pInsertArray;
   ULONG ulNum;

   WinEnableWindowUpdate(hwndList, FALSE);
   SendMsg(hwndList, LM_DELETEALL, NULL, NULL);

   ulNum = WinQueryWindowULong(hwndList, QWL_STYLE);
   WinSetWindowULong(hwndList, QWL_STYLE, ulNum | WS_TABSTOP);

   while (pTemp)
   {
      InsertInfo.ulItemCount++;
      pTemp = pTemp->next;
   }
   pInsertArray = malloc(InsertInfo.ulItemCount * sizeof(PCHAR));
   for (pTemp = pRAMList, ulNum = 0; pTemp; pTemp = pTemp->next, ulNum++)
      pInsertArray[ulNum] = pTemp->pchLine;

   SendMsg(hwndList, LM_INSERTMULTITEMS, &InsertInfo, pInsertArray);

   free(pInsertArray);

   WinEnableWindowUpdate(hwndList, TRUE);

   return;
}

static MRESULT EXPENTRY ListAddProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PLISTADDPAR pListAddPar = WinQueryWindowPtr(hwnd, QWL_USER);
   extern REQUESTOPT requestoptions;

   switch(msg)
   {
      case WM_INITDLG:
         pListAddPar = (PLISTADDPAR) mp2;
         WinSubclassWindow(WinWindowFromID(hwnd, IDD_REQ_ADDLIST+5), FidoEntryProc);

         WinSendDlgItemMsg(hwnd, IDD_REQ_ADDLIST+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PATHNAME), NULL);
         WinSendDlgItemMsg(hwnd, IDD_REQ_ADDLIST+5, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_5DADDRESS), NULL);
         WinSendDlgItemMsg(hwnd, IDD_REQ_ADDLIST+7, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_LISTDESC), NULL);

         RestoreWinPos(hwnd, &requestoptions.ListAddPos, FALSE, TRUE);
         WinSetWindowPtr(hwnd, QWL_USER, pListAddPar);
         break;

       case WM_WINDOWPOSCHANGED:
         if (pListAddPar)
            SaveWinPos(hwnd, (PSWP) mp1, &requestoptions.ListAddPos, &requestoptions.bDirty);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               WinQueryDlgItemText(hwnd, IDD_REQ_ADDLIST+2, sizeof(pListAddPar->FileList.pchFileName),
                                   pListAddPar->FileList.pchFileName);
               WinQueryDlgItemText(hwnd, IDD_REQ_ADDLIST+5, sizeof(pListAddPar->FileList.pchAddress),
                                   pListAddPar->FileList.pchAddress);
               WinQueryDlgItemText(hwnd, IDD_REQ_ADDLIST+7, sizeof(pListAddPar->FileList.pchDesc),
                                   pListAddPar->FileList.pchDesc);
               break;

            case DID_CANCEL:
               break;

            case IDD_REQ_ADDLIST+3:   /* Suchen */
               {
                  char pchPath[LEN_PATHNAME+1]="";

                  WinQueryDlgItemText(hwnd, IDD_REQ_ADDLIST+2, sizeof(pchPath), pchPath);
                  if (GetPathname(hwnd, pchPath) == DID_OK)
                  {
                     WinSetDlgItemText(hwnd, IDD_REQ_ADDLIST+2, pchPath);

                     if (WinQueryDlgItemTextLength(hwnd, IDD_REQ_ADDLIST+2) &&
                         WinQueryDlgItemTextLength(hwnd, IDD_REQ_ADDLIST+5) &&
                         WinQueryDlgItemTextLength(hwnd, IDD_REQ_ADDLIST+7))
                        WinEnableControl(hwnd, DID_OK, TRUE);
                     else
                        WinEnableControl(hwnd, DID_OK, FALSE);
                  }
               }
               return (MRESULT) FALSE;

            default:
               break;
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CHAR:
         if (WinQueryDlgItemTextLength(hwnd, IDD_REQ_ADDLIST+2) &&
             WinQueryDlgItemTextLength(hwnd, IDD_REQ_ADDLIST+5) &&
             WinQueryDlgItemTextLength(hwnd, IDD_REQ_ADDLIST+7))
            WinEnableControl(hwnd, DID_OK, TRUE);
         else
            WinEnableControl(hwnd, DID_OK, FALSE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static MRESULT EXPENTRY FileAddProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PFILEADDPAR pFileAddPar = WinQueryWindowPtr(hwnd, QWL_USER);
   extern REQUESTOPT requestoptions;

   switch(msg)
   {
      case WM_INITDLG:
         pFileAddPar = (PFILEADDPAR) mp2;

         WinSendDlgItemMsg(hwnd, IDD_REQ_ADDFILE+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_REQFILE), NULL);
         WinSendDlgItemMsg(hwnd, IDD_REQ_ADDFILE+4, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PASSWORD), NULL);

         RestoreWinPos(hwnd, &requestoptions.FileAddPos, FALSE, TRUE);
         WinSetWindowPtr(hwnd, QWL_USER, pFileAddPar);
         break;

       case WM_WINDOWPOSCHANGED:
         if (pFileAddPar)
            SaveWinPos(hwnd, (PSWP) mp1, &requestoptions.FileAddPos, &requestoptions.bDirty);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               WinQueryDlgItemText(hwnd, IDD_REQ_ADDFILE+2, sizeof(pFileAddPar->pchFileName),
                                   pFileAddPar->pchFileName);
               WinQueryDlgItemText(hwnd, IDD_REQ_ADDFILE+4, sizeof(pFileAddPar->pchPassword),
                                   pFileAddPar->pchPassword);
               break;

            case DID_CANCEL:
               break;

            default:
               break;
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_CHAR:
         if (WinQueryDlgItemTextLength(hwnd, IDD_REQ_ADDFILE+2))
            WinEnableControl(hwnd, DID_OK, TRUE);
         else
            WinEnableControl(hwnd, DID_OK, FALSE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static void AddFile(HWND hwndDlg)
{
   FILEADDPAR FileAddPar;

   memset(&FileAddPar, 0, sizeof(FileAddPar));
   FileAddPar.cb = sizeof(FileAddPar);

   if (WinDlgBox(HWND_DESKTOP, hwndDlg, FileAddProc, hmodLang,
                 IDD_REQ_ADDFILE, &FileAddPar) == DID_OK)
   {
      LONG sItem;

      sItem = (LONG) WinSendDlgItemMsg(hwndDlg, IDD_REQUESTER+13, LM_INSERTITEM,
                                        MPFROMLONG(LIT_END), FileAddPar.pchFileName);
      if (sItem >= 0)
      {
         if (FileAddPar.pchPassword[0])
            WinSendDlgItemMsg(hwndDlg, IDD_REQUESTER+13, LM_SETITEMHANDLE,
                              MPFROMLONG(sItem), strdup(FileAddPar.pchPassword));
         WinSendDlgItemMsg(hwndDlg, IDD_REQUESTER+13, LM_SELECTITEM,
                              MPFROMLONG(sItem), MPFROMLONG(TRUE));
      }
   }

   return;
}

static MRESULT EXPENTRY PasswordProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PPASSWORDPAR pPasswordPar = WinQueryWindowPtr(hwnd, QWL_USER);
   extern REQUESTOPT requestoptions;

   switch(msg)
   {
      case WM_INITDLG:
         pPasswordPar = (PPASSWORDPAR) mp2;

         WinSendDlgItemMsg(hwnd, IDD_REQ_SETPASS+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_PASSWORD), NULL);
         WinSetDlgItemText(hwnd, IDD_REQ_SETPASS+2, pPasswordPar->pchPassword);


         RestoreWinPos(hwnd, &requestoptions.PasswdPos, FALSE, TRUE);
         WinSetWindowPtr(hwnd, QWL_USER, pPasswordPar);
         break;

       case WM_WINDOWPOSCHANGED:
         if (pPasswordPar)
            SaveWinPos(hwnd, (PSWP) mp1, &requestoptions.PasswdPos, &requestoptions.bDirty);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               WinQueryDlgItemText(hwnd, IDD_REQ_SETPASS+2, sizeof(pPasswordPar->pchPassword),
                                   pPasswordPar->pchPassword);
               break;

            case DID_CANCEL:
               break;

            default:
               break;
         }
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, hwnd);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static PFILELIST QueryFileList(HWND hwndDlg)
{
   HWND hwndList = WinWindowFromID(hwndDlg, IDD_REQUESTER+3);
   SHORT sItem;

   sItem = (SHORT) SendMsg(hwndList, LM_QUERYSELECTION, MPFROMSHORT(LIT_CURSOR),
                           NULL);

   if (sItem >= 0)
      return (PFILELIST) SendMsg(hwndList, LM_QUERYITEMHANDLE, MPFROMSHORT(sItem),
                                 NULL);
   else
      return NULL;
}

static void AddFileName(char *pchDest, char *pchName)
{
   pchDest = strchr(pchDest, '\0');
   while (*pchName == ' ')
      pchName++;

   while (*pchName && *pchName != ' ')
      *pchDest++ = *pchName++;

   *pchDest=0;

   return;

}

static void SetPassword(HWND hwndDlg)
{
   PASSWORDPAR PasswordPar;
   HWND hwndList = WinWindowFromID(hwndDlg, IDD_REQUESTER+13);
   LONG sItem;
   PCHAR pchPassword;

   memset(&PasswordPar, 0, sizeof(PasswordPar));
   PasswordPar.cb = sizeof(PasswordPar);

   sItem = (LONG) SendMsg(hwndList, LM_QUERYSELECTION, MPFROMLONG(LIT_CURSOR), NULL);

   if (sItem >= 0)
   {
      pchPassword = (PCHAR) SendMsg(hwndList, LM_QUERYITEMHANDLE,
                                    MPFROMLONG(sItem), NULL);
      if (pchPassword)
         strncpy(PasswordPar.pchPassword, pchPassword, LEN_PASSWORD);

      if (WinDlgBox(HWND_DESKTOP, hwndDlg, PasswordProc, hmodLang,
                    IDD_REQ_SETPASS, &PasswordPar) == DID_OK)
      {
         if (pchPassword)
            free(pchPassword);
         if (PasswordPar.pchPassword)
            pchPassword = strdup(PasswordPar.pchPassword);
         else
            pchPassword = NULL;
         SendMsg(hwndList, LM_SETITEMHANDLE, MPFROMLONG(sItem),
                 pchPassword);
      }
   }

   return;
}

static void SearchInFiles(HWND hwndDlg, BOOL bSearchNext)
{
   SEARCHPAR SearchPar;
   LONG sTop;
   HWND hwndList = WinWindowFromID(hwndDlg, IDD_REQUESTER+13);
   extern REQUESTOPT requestoptions;

   /* Top-Message abfragen */
   sTop = (LONG) SendMsg(hwndList, LM_QUERYTOPINDEX, NULL, NULL);

   if (sTop >= 0)
   {
      /* Begriff holen */
      memset(&SearchPar, 0, sizeof(SearchPar));
      SearchPar.cb = sizeof(SearchPar);
      SearchPar.DlgPos = requestoptions.SearchPos;
      SearchPar.ulSearchFlags = requestoptions.ulSearchFlags;
      strcpy(SearchPar.pchSearchText, requestoptions.pchLastSearch);

      if ((bSearchNext && SearchPar.pchSearchText[0]) ||
          (WinDlgBox(HWND_DESKTOP, hwndDlg, SearchProc, hmodLang,
                     IDD_SEARCH, &SearchPar) == DID_OK))
      {
         LONG sMatch;
         LBSEARCH LBSearch = { LSS_SUBSTRING, 0};

         LBSearch.lItemStart = sTop;

         if (SearchPar.ulSearchFlags & SEARCHFLAG_CASESENSITIVE)
            LBSearch.usCmd |= LSS_CASESENSITIVE;

         /* Suchen */
         sMatch = (LONG) SendMsg(hwndList, LM_SEARCHSTRING,
                                 &LBSearch,
                                 SearchPar.pchSearchText);

         /* gefundene Stelle anzeigen */
         if (sMatch >= 0)
            SendMsg(hwndList, LM_SETTOPINDEX, MPFROMLONG(sMatch), NULL);
         else
            WinAlarm(HWND_DESKTOP, WA_ERROR);

         /* Daten zurckholen */
         requestoptions.SearchPos = SearchPar.DlgPos;
         strcpy(requestoptions.pchLastSearch, SearchPar.pchSearchText);
         requestoptions.ulSearchFlags = SearchPar.ulSearchFlags;
         requestoptions.bDirty = TRUE;
      }
   }

   return;
}

void FreeRequestList(PREQUESTLIST pList)
{
   PREQUESTLIST pNext;

   while (pList)
   {
      pNext = pList->next;
      free(pList);
      pList = pNext;
   }
   return;
}

static int LookupOtherLines(HWND hwnd)
{
   char pchAddress[LEN_5DADDRESS+1];
   char pchSysop[LEN_USERNAME+1];


   WinQueryDlgItemText(hwnd, IDD_REQUESTER+7, sizeof(pchAddress), pchAddress);

   if (pchAddress[0])
   {
      if (PerformNodeLookup(pchAddress, hwnd, pchSysop))
      {
         if (PerformNameLookup(pchSysop, hwnd, LOOKUP_FORCESELECT, pchSysop, pchAddress))
         {
            WinSetDlgItemText(hwnd, IDD_REQUESTER+7, pchAddress);
            return 0;
         }
         else
            return 1;
      }
      else
         return 1;
   }
   else
      return 1;
}

/*-------------------------------- Modulende --------------------------------*/


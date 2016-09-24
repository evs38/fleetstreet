/*---------------------------------------------------------------------------+
 | Titel: CCLIST.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 05.09.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x PM                                                       |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |    Carbon Copy Lists Setup fuer FleetStreet                               |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "resids.h"
#include "messages.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "dialogids.h"
#include "controls\editwin.h"
#include "utility.h"
#include "cclist.h"
#include "fltv7\fltv7.h"
#include "nodedrag.h"
#include "lookups.h"
#include "util\fltutil.h"
#include "util\addrcnv.h"
#include "ccmanage.h"
#include "nickmanage.h"

/*--------------------------------- Defines ---------------------------------*/

#ifndef CRA_SOURCE
#define CRA_SOURCE  0x00004000L
#endif

#define UNREG_CCLISTS  3

/* Name der CC-Liste hat sich geaendert */
#define CCM_UPDATENAME       (WM_USER+1)

/* CC-Inhalts-Fenster schlie疸 sich, mp1: HWND */
#define CCM_CLOSE            (WM_USER+2)

#define CCM_UPDATELIST       (WM_USER+3)

#define CCDRAGTYPE "FleetStreet CCEntry"
#define CCRMF      "<DRM_FLEET,DRF_CCENTRY>,<DRM_DISCARD,DRF_UNKNOWN>"

#define CCLISTDRAGTYPE "FleetStreet CCList"
#define CCLISTRMF      "<DRM_FLEET,DRF_CCLIST>,<DRM_DISCARD,DRF_UNKNOWN>"

typedef struct ccentrypar {
            USHORT cb;
            CCENTRY CCEntry;
          } CCENTRYPAR, *PCCENTRYPAR;

typedef struct cclistdata {
            PCCLIST   pList;
            PCCANCHOR pCCAnchor;
            HPOINTER  hptrCCEntry;
            BOOL      bNotify;
          } CCLISTDATA, *PCCLISTDATA;

typedef struct cclistrecord {
            MINIRECORDCORE RecordCore;
            PSZ pchUserName;
            PSZ pchAddress;
            PSZ pchShow;
            PCCENTRY pEntry;
          } CCLISTRECORD, *PCCLISTRECORD;

typedef struct ccfolderrecord {
            MINIRECORDCORE RecordCore;
            PCCLIST        pList;
            HWND           hwndContents;
          } CCFOLDERRECORD, *PCCFOLDERRECORD;

typedef struct ccfolderdata {
            HWND            hwndListPopup;
            HWND            hwndFolderPopup;
            PCCFOLDERRECORD pPopupRecord;
            PCCFOLDERRECORD pDragRecord;
            HPOINTER        hptrCCList;
            HPOINTER        hptrCCFolder;
            BOOL            bNotify;
            HSWITCH         hSwitch;
            BOOL            bForeground;
          } CCFOLDERDATA, *PCCFOLDERDATA;

/*---------------------------- Globale Variablen ----------------------------*/

extern HWND hwndCCLists;
extern HMODULE hmodLang;
extern DIRTYFLAGS dirtyflags;
extern HAB anchor;
extern CCANCHOR ccanchor;
extern ULONG ulCCSelected;

static PFNWP OldContainerProc;

/*--------------------------- Funktionsprototypen ---------------------------*/

static void UpdateCCList(HWND hwndListbox, BOOL bEmptyLists);
static void CleanupCCList(HWND hwndContainer);
static void UpdateCCEntries(HWND hwndContainer, PCCLIST pCCList);
static MRESULT EXPENTRY CCEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static SHORT EXPENTRY SortCCContents(CCLISTRECORD *p1, CCLISTRECORD *p2, PVOID pStorage);
static MRESULT EXPENTRY NewCCContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY ImportCCProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);
static int ImportCCList(HWND hwndOwner, PCCANCHOR pCCAnchor, PCCLIST pDestList);
static int ReadCCFile(FILE *pfImport, PCCANCHOR pCCAnchor, PCCLIST pDestList, ULONG ulFlags);
static MRESULT ContentsDragover(HWND hwndDlg, PCNRDRAGINFO pCnrDraginfo);
static void ContentsDrop(HWND hwndDlg, PCCLISTDATA pListData, PCNRDRAGINFO pCnrDraginfo);
static void ContentsInitDrag(HWND hwndDlg, PCCLISTDATA pListData, PCNRDRAGINIT pDragInit);

static PCCFOLDERRECORD AddEmptyCCList(HWND hwndContainer, HPOINTER hptr);
static void CCListClosed(HWND hwndCnr, HWND hwndList);
static int InitCCListDrag(HWND hwndDlg, PCNRDRAGINIT pInit);
static int CleanupCCFolder(HWND hwndContainer);
static int DeleteCCListF(HWND hwndContainer, PCCFOLDERRECORD pRecord);
static int FillCCFolder(HWND hwndContainer, HPOINTER hptr);
static int OpenCCList(HWND hwndContainer, PCCFOLDERRECORD pRecord);
static MRESULT CCFolderDragOver(HWND hwndCnr, PCNRDRAGINFO pCnrDrag);
static void CCFolderDrop(HWND hwndCnr, PCNRDRAGINFO pCnrDrag);
static void CCListSendUpdate(HWND hwndCnr, ULONG ulListID);
static void InitCCFolder(HWND hwndCnr);
static SHORT EXPENTRY SortCCFolder(PCCFOLDERRECORD p1, PCCFOLDERRECORD p2, PVOID pStorage);

/*------------------------------ NewContainerProc ---------------------------*/
/* Neue Window-Prozedur f. Container, um OS/2-Bug zu umschiffen              */
/*---------------------------------------------------------------------------*/

static MRESULT EXPENTRY NewCCContainerProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   switch(message)
   {
      case DM_DRAGOVER:
         /*DrgAccessDraginfo(mp1);*/
         break;

      case WM_BUTTON2DOWN:
         return (MRESULT) FALSE;

      default:
         break;
   }
   return OldContainerProc(parent, message, mp1, mp2);
}

/*------------------------------ UpdateCCList    ----------------------------*/
/* Fuellt die Listbox mit den CC-List-Tags neu                               */
/*---------------------------------------------------------------------------*/

static  void UpdateCCList(HWND hwndListbox, BOOL bEmptyLists)
{
   PCCLIST pList;
   SHORT sItem;

   WinEnableWindowUpdate(hwndListbox, FALSE);
   SendMsg(hwndListbox, LM_DELETEALL, NULL, NULL);
   pList=ccanchor.pLists;
   while (pList)
   {
      if (bEmptyLists || pList->pEntries)
      {
         char *pchTempTitle;
         char *pchTemp;

         pchTempTitle = strdup(pList->pchListName);
         pchTemp = pchTempTitle;
         while (*pchTemp)
            if (*pchTemp == '\r' ||
                *pchTemp == '\n')
               *pchTemp++ = ' ';
            else
               pchTemp++;
         sItem = (SHORT)WinInsertLboxItem(hwndListbox, LIT_SORTASCENDING, pchTempTitle);
         free(pchTempTitle);
         SendMsg(hwndListbox, LM_SETITEMHANDLE, MPFROMSHORT(sItem), MPFROMLONG(pList->ulListID));
      }
      pList=pList->next;
   }
   SendMsg(hwndListbox, LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));
   WinEnableWindowUpdate(hwndListbox, TRUE);
   return;
}

/*------------------------------ CCListContentsProc -------------------------*/
/* Fenster-Prozedur fuer CC-List-Inhalt                                      */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY CCListContentsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp;
   extern WINDOWFONTS windowfonts;
   extern WINDOWCOLORS windowcolors;
   extern GENERALOPT generaloptions;
   static pchTitleName[50];
   static pchTitleAddress[50];
   static pchTitleShow[50];
   CNRINFO cnrinfo;
   PFIELDINFO pField, pFirstField;
   FIELDINFOINSERT FieldinfoInsert;
   PCCLISTDATA pListData = (PCCLISTDATA) WinQueryWindowULong(parent, QWL_USER);
   BOOL bTemp;

   switch (message)
   {
      case WM_INITDLG:
         /* Instanzdaten anfordern */
         pListData = malloc(sizeof(CCLISTDATA));
         memset(pListData, 0, sizeof(CCLISTDATA));
         WinSetWindowULong(parent, QWL_USER, (ULONG) pListData);

         /* Daten setzen */
         pListData->pList=((PCCLISTPAR) mp2)->pList;
         pListData->pCCAnchor=((PCCLISTPAR) mp2)->pCCAnchor;
         pListData->hptrCCEntry=LoadIcon(IDIC_CCENTRY);

         OldContainerProc=WinSubclassWindow(WinWindowFromID(parent, IDD_CCLIST+1),
                                            NewCCContainerProc);

         SetFont(WinWindowFromID(parent, IDD_CCLIST+1), windowfonts.cclistfont);
         SetBackground(WinWindowFromID(parent, IDD_CCLIST+1), &windowcolors.cccontentsback);
         SetForeground(WinWindowFromID(parent, IDD_CCLIST+1), &windowcolors.cccontentsfore);

         /* Felder einfuegen */
         pFirstField=WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_ALLOCDETAILFIELDINFO,
                                       MPFROMSHORT(3), NULL);
         pField=pFirstField;
         pField->cb=sizeof(FIELDINFO);
         pField->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR | CFA_FIREADONLY;
         pField->flTitle=CFA_FITITLEREADONLY;
         pField->pTitleData=pchTitleName;
         LoadString(IDST_CC_NAME, 50, pField->pTitleData);
         pField->offStruct=FIELDOFFSET(CCLISTRECORD, pchUserName);
         pField=pField->pNextFieldInfo;

         pField->cb=sizeof(FIELDINFO);
         pField->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_SEPARATOR | CFA_FIREADONLY;
         pField->flTitle=CFA_FITITLEREADONLY;
         pField->pTitleData=pchTitleAddress;
         LoadString(IDST_CC_ADDRESS, 50, pField->pTitleData);
         pField->offStruct=FIELDOFFSET(CCLISTRECORD, pchAddress);
         pField=pField->pNextFieldInfo;

         pField->cb=sizeof(FIELDINFO);
         pField->flData=CFA_STRING | CFA_HORZSEPARATOR | CFA_FIREADONLY;
         pField->flTitle=CFA_FITITLEREADONLY;
         pField->pTitleData=pchTitleShow;
         LoadString(IDST_CC_SHOW, 50, pField->pTitleData);
         pField->offStruct=FIELDOFFSET(CCLISTRECORD, pchShow);

         FieldinfoInsert.cb=sizeof(FIELDINFOINSERT);
         FieldinfoInsert.pFieldInfoOrder=(PFIELDINFO)CMA_FIRST;
         FieldinfoInsert.fInvalidateFieldInfo=TRUE;
         FieldinfoInsert.cFieldInfoInsert=3;

         WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_INSERTDETAILFIELDINFO,
                           pFirstField, &FieldinfoInsert);

         cnrinfo.cb=sizeof(CNRINFO);
         cnrinfo.flWindowAttr=CV_DETAIL | CA_TITLEREADONLY |
                              CA_TITLESEPARATOR | CA_DETAILSVIEWTITLES;
         cnrinfo.pSortRecord=(PVOID)SortCCContents;
         WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_SETCNRINFO, &cnrinfo,
                           MPFROMLONG(CMA_FLWINDOWATTR | CMA_PSORTRECORD));

         UpdateCCEntries(WinWindowFromID(parent, IDD_CCLIST+1), pListData->pList);

         if (pListData->pList->ulFlags & CCLIST_KILLSENT)
            WinCheckButton(parent, IDD_CCLIST+7, TRUE);

         if (pListData->pList->pEntries)
         {
            WinEnableControl(parent, IDD_CCLIST+4, TRUE);
            WinEnableControl(parent, IDD_CCLIST+5, TRUE);
         }
         WinSetWindowText(parent, pListData->pList->pchListName);
         RestoreWinPos(parent, &pListData->pList->ListPos, FALSE, FALSE);
         pListData->bNotify = TRUE;
         break;

      case WM_ACTIVATE:
         if (mp1)
            WinAssociateHelpInstance(hwndhelp, parent);
         else
            WinAssociateHelpInstance(hwndhelp, NULLHANDLE);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            CCENTRYPAR CCEntryPar;
            CCLISTRECORD *pRecord;

            /* Add-Button */
            case IDD_CCLIST+3:
               CCEntryPar.cb=sizeof(CCENTRYPAR);
               CCEntryPar.CCEntry.ulFlags= CCENTRY_MENTION;
               CCEntryPar.CCEntry.pchName[0]='\0';
               CCEntryPar.CCEntry.pchAddress[0]='\0';
               CCEntryPar.CCEntry.pchFirstLine[0]='\0';
               if (WinDlgBox(HWND_DESKTOP, parent, CCEntryProc, hmodLang,
                             IDD_CCLISTENTRY, &CCEntryPar)==DID_OK)
               {
                  /* Eintrag einfuegen */
                  if (AddCCEntry(pListData->pCCAnchor, pListData->pList, &CCEntryPar.CCEntry))
                  {
                    UpdateCCEntries(WinWindowFromID(parent, IDD_CCLIST+1), pListData->pList);
                    if (pListData->pList->pEntries)
                    {
                       WinEnableControl(parent, IDD_CCLIST+4, TRUE);
                       WinEnableControl(parent, IDD_CCLIST+5, TRUE);
                    }
                  }
               }
               return (MRESULT) FALSE;

            /* Delete-Button */
            case IDD_CCLIST+4:
               if (generaloptions.safety & SAFETY_CHANGESETUP)
               {
                  if (MessageBox(parent, IDST_MSG_DELCCENTRY, IDST_TITLE_DELCCENTRY,
                                 IDD_DELCCENTRY, MB_YESNO | MB_QUERY | MB_DEFBUTTON2)!=MBID_YES)
                     return (MRESULT) FALSE;
               }
               pRecord = NULL;
               while (pRecord=WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_QUERYRECORDEMPHASIS,
                                                pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                                MPFROMSHORT(CRA_SELECTED)))
               {
                  DeleteCCEntry(pListData->pCCAnchor, pListData->pList, pRecord->pEntry);
               }
               UpdateCCEntries(WinWindowFromID(parent, IDD_CCLIST+1), pListData->pList);
               if (pListData->pList->pEntries)
               {
                  WinEnableControl(parent, IDD_CCLIST+4, TRUE);
                  WinEnableControl(parent, IDD_CCLIST+5, TRUE);
               }
               return (MRESULT) FALSE;

            /* Change-Button */
            case IDD_CCLIST+5:
               pRecord=(CCLISTRECORD *)WinSendDlgItemMsg(parent, IDD_CCLIST+1,
                                         CM_QUERYRECORDEMPHASIS, (MPARAM) CMA_FIRST,
                                         MPFROMSHORT(CRA_SELECTED));
               if (pRecord)
               {
                  CCEntryPar.cb=sizeof(CCENTRYPAR);
                  strcpy(CCEntryPar.CCEntry.pchName, pRecord->pEntry->pchName);
                  strcpy(CCEntryPar.CCEntry.pchAddress, pRecord->pEntry->pchAddress);
                  strcpy(CCEntryPar.CCEntry.pchFirstLine, pRecord->pEntry->pchFirstLine);
                  CCEntryPar.CCEntry.ulFlags = pRecord->pEntry->ulFlags;
                  if (WinDlgBox(HWND_DESKTOP, parent, CCEntryProc, hmodLang,
                                IDD_CCLISTENTRY, &CCEntryPar)==DID_OK)
                  {
                     strcpy(pRecord->pEntry->pchName, CCEntryPar.CCEntry.pchName);
                     strcpy(pRecord->pEntry->pchAddress, CCEntryPar.CCEntry.pchAddress);
                     strcpy(pRecord->pEntry->pchFirstLine, CCEntryPar.CCEntry.pchFirstLine);
                     pRecord->pEntry->ulFlags = CCEntryPar.CCEntry.ulFlags;
                     if (CCEntryPar.CCEntry.ulFlags & CCENTRY_MENTION)
                        LoadString(IDST_CC_YES, 10, pRecord->pchShow);
                     else
                        LoadString(IDST_CC_NO, 10, pRecord->pchShow);
                     WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_INVALIDATERECORD,
                                       &pRecord, MPFROM2SHORT(1,CMA_TEXTCHANGED));
                     WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_SORTRECORD,
                                       MPFROMP(SortCCContents), NULL);
                     WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_INVALIDATERECORD, NULL, NULL);
                     pListData->pList->bDirty = TRUE;
                     if (pListData->pCCAnchor)
                        pListData->pCCAnchor->bDirty = TRUE;
                  }
               }
               return (MRESULT) FALSE;

            /* Import-Button */
            case IDD_CCLIST+6:
               if (!ImportCCList(parent, pListData->pCCAnchor, pListData->pList))
                  UpdateCCEntries(WinWindowFromID(parent, IDD_CCLIST+1), pListData->pList);
               if (pListData->pList->pEntries)
               {
                  WinEnableControl(parent, IDD_CCLIST+4, TRUE);
                  WinEnableControl(parent, IDD_CCLIST+5, TRUE);
               }
               return (MRESULT) FALSE;

            case DID_OK:
            case DID_CANCEL:
               if (hwndCCLists)
                  WinPostMsg(hwndCCLists, CCM_CLOSE, (MPARAM) parent,
                             NULL);
               break;

            default:
               return (MRESULT) FALSE;
         }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1)==IDD_CCLIST+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               case CN_ENTER:
                  if (mp2)
                     SendMsg(parent, WM_COMMAND, MPFROMSHORT(IDD_CCLIST+5), NULL);
                  break;

               case CN_INITDRAG:
                  ContentsInitDrag(parent, pListData, (PCNRDRAGINIT)mp2);
                  break;

               case CN_DRAGOVER:
                  return ContentsDragover(parent, (PCNRDRAGINFO) mp2);

               case CN_DROP:
                  ContentsDrop(parent, pListData, (PCNRDRAGINFO) mp2);
                  break;

               default:
                  break;
            }
         }
         break;

      case DM_DISCARDOBJECT:
         if (mp1)
         {
            PDRAGINFO pDraginfo=(PDRAGINFO) mp1;
            PDRAGITEM pItem;
            PCCENTRY pSource;
            CCLISTRECORD *pRecord;
            ULONG ulNum;
            int i;

            DrgAccessDraginfo(pDraginfo);
            ulNum = DrgQueryDragitemCount(pDraginfo);
            for (i=0; i< ulNum; i++)
            {
               pItem=DrgQueryDragitemPtr(pDraginfo, i);
               pSource=(PCCENTRY)pItem->ulItemID;

               /* Record aus Container loeschen */
               pRecord=(CCLISTRECORD *)WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_QUERYRECORD, NULL,
                                                         MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
               while (pRecord && pRecord->pEntry != pSource)
                  pRecord=(CCLISTRECORD *)WinSendDlgItemMsg(parent, IDD_CCLIST+1,
                                                            CM_QUERYRECORD, pRecord,
                                                            MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
               if (pRecord)
                  WinSendDlgItemMsg(parent, IDD_CCLIST+1, CM_ERASERECORD,
                                    pRecord, NULL);
               DeleteCCEntry(pListData->pCCAnchor, pListData->pList, pSource);
            }
            UpdateCCEntries(WinWindowFromID(parent, IDD_CCLIST+1), pListData->pList);
            DrgFreeDraginfo(pDraginfo);
         }
         return MRFROMLONG(DRR_SOURCE);

      case WM_CLOSE:
         if (hwndCCLists)
            WinPostMsg(hwndCCLists, CCM_CLOSE, (MPARAM) parent,
                       NULL);
         break;

      case WM_DESTROY:
         bTemp = WinQueryButtonCheckstate(parent, IDD_CCLIST+7);
         if (bTemp && !(pListData->pList->ulFlags & CCLIST_KILLSENT))
         {
            pListData->pList->ulFlags |= CCLIST_KILLSENT;
            pListData->pList->bDirty = TRUE;
            if (pListData->pCCAnchor)
                pListData->pCCAnchor->bDirty = TRUE;
         }
         else
            if (!bTemp && (pListData->pList->ulFlags & CCLIST_KILLSENT))
            {
               pListData->pList->ulFlags &= ~CCLIST_KILLSENT;
               pListData->pList->bDirty = TRUE;
               if (pListData->pCCAnchor)
                   pListData->pCCAnchor->bDirty = TRUE;
            }

         QueryFont(WinWindowFromID(parent, IDD_CCLIST+1), windowfonts.cclistfont);
         QueryBackground(WinWindowFromID(parent, IDD_CCLIST+1), &windowcolors.cccontentsback);
         QueryForeground(WinWindowFromID(parent, IDD_CCLIST+1), &windowcolors.cccontentsfore);
         CleanupCCList(WinWindowFromID(parent, IDD_CCLIST+1));
         if (pListData->hptrCCEntry)
            WinDestroyPointer(pListData->hptrCCEntry);
         free(pListData);
         break;

      case CCM_UPDATENAME:
         WinSetWindowText(parent, pListData->pList->pchListName);
         return (MRESULT) TRUE;

      case CCM_UPDATELIST:
         UpdateCCEntries(WinWindowFromID(parent, IDD_CCLIST+1), pListData->pList);
         if (pListData->pList->pEntries)
         {
            WinEnableControl(parent, IDD_CCLIST+4, TRUE);
            WinEnableControl(parent, IDD_CCLIST+5, TRUE);
         }
         return (MRESULT) FALSE;

      case WM_WINDOWPOSCHANGED:
         if (pListData && pListData->bNotify)
         {
            if (SaveWinPos(parent, (PSWP) mp1, &pListData->pList->ListPos, &pListData->pList->bDirty))
            {
               if (pListData->pCCAnchor)
                  pListData->pCCAnchor->bDirty = TRUE;
            }
         }
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

static void ContentsInitDrag(HWND hwndDlg, PCCLISTDATA pListData, PCNRDRAGINIT pDragInit)
{
   ULONG ulNum=0;
   int i=0;
   PCCLISTRECORD pRecord= (PCCLISTRECORD) pDragInit->pRecord;
   HWND hwndCnr = WinWindowFromID(hwndDlg, IDD_CCLIST+1);

   if (pRecord)
   {
      /* Record draggen */
      PDRAGINFO pDraginfo;
      DRAGITEM dItem;
      DRAGIMAGE dImage[3];

      if (pRecord->RecordCore.flRecordAttr & CRA_SELECTED)
      {
         /* zaehlen und Source-Emphasis */
         pRecord=NULL;
         while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                     MPFROMLONG(CRA_SELECTED)))
         {
            ulNum++;
            SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));
         }
         pDraginfo = DrgAllocDraginfo(ulNum);
         pDraginfo->usOperation=DO_COPY;
         pDraginfo->hwndSource=hwndDlg;

         pRecord = NULL;
         while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, pRecord?pRecord:MPFROMLONG(CMA_FIRST),
                                     MPFROMLONG(CRA_SELECTED)))
         {
            dItem.hwndItem=hwndDlg;
            dItem.ulItemID=(ULONG)pRecord->pEntry;
            dItem.hstrType=DrgAddStrHandle(CCDRAGTYPE);
            dItem.hstrRMF=DrgAddStrHandle(CCRMF);
            dItem.hstrContainerName=DrgAddStrHandle(pListData->pList->pchListName);
            dItem.hstrSourceName=0;
            dItem.hstrTargetName=0;
            dItem.cxOffset=pDragInit->cx;
            dItem.cyOffset=pDragInit->cx;
            dItem.fsControl=0;
            dItem.fsSupportedOps=DO_COPYABLE | DO_MOVEABLE;

            DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), i);
            i++;
         }
         for (i=0; i < ulNum && i <3; i++)
         {
            /* Drag-Image vorbereiten */
            dImage[i].cb=sizeof(DRAGIMAGE);
            dImage[i].hImage=pListData->hptrCCEntry;
            dImage[i].fl=DRG_ICON;
            dImage[i].cxOffset=i*10;
            dImage[i].cyOffset=i*10;
         }
         DrgDrag(hwndDlg, pDraginfo, dImage, (ulNum <3)?ulNum:3, VK_ENDDRAG, NULL);
         DrgFreeDraginfo(pDraginfo);

         /* Source-Emphasis aus */
         RemoveSourceEmphasis(hwndCnr);
      }
      else
      {
         /* nur ein Record */
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

         pDraginfo = DrgAllocDraginfo(1);
         pDraginfo->usOperation=DO_COPY;
         pDraginfo->hwndSource=hwndDlg;

         dItem.hwndItem=hwndDlg;
         dItem.ulItemID=(ULONG)pRecord->pEntry;
         dItem.hstrType=DrgAddStrHandle(CCDRAGTYPE);
         dItem.hstrRMF=DrgAddStrHandle(CCRMF);
         dItem.hstrContainerName=DrgAddStrHandle(pListData->pList->pchListName);
         dItem.hstrSourceName=0;
         dItem.hstrTargetName=0;
         dItem.cxOffset=pDragInit->cx;
         dItem.cyOffset=pDragInit->cx;
         dItem.fsControl=0;
         dItem.fsSupportedOps=DO_COPYABLE | DO_MOVEABLE;

         DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);

         /* Drag-Image vorbereiten */
         dImage[0].cb=sizeof(DRAGIMAGE);
         dImage[0].hImage=pListData->hptrCCEntry;
         dImage[0].fl=DRG_ICON;
         dImage[0].cxOffset=0;
         dImage[0].cyOffset=0;

         DrgDrag(hwndDlg, pDraginfo, dImage, 1, VK_ENDDRAG, NULL);
         DrgFreeDraginfo(pDraginfo);

         /* Source-Emphasis aus */
         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));
      }
   }

   return;
}

static MRESULT ContentsDragover(HWND hwndDlg, PCNRDRAGINFO pCnrDraginfo)
{
   MRESULT mrReply;
   PID pid, pid2;
   TID tid, tid2;
   ULONG ulNum;
   int i;

   DrgAccessDraginfo(pCnrDraginfo->pDragInfo);
   WinQueryWindowProcess(pCnrDraginfo->pDragInfo->hwndSource, &pid, &tid);
   WinQueryWindowProcess(hwndDlg, &pid2, &tid2);
   if (pid != pid2)
      mrReply=(MRESULT) DOR_NEVERDROP;
   else
      if (pCnrDraginfo->pDragInfo->hwndSource == hwndDlg)
         mrReply=(MRESULT) DOR_NODROP;
      else
      {
         if ((pCnrDraginfo->pDragInfo->usOperation == DO_LINK) ||
             (pCnrDraginfo->pDragInfo->usOperation >= DO_UNKNOWN))
            mrReply=(MRESULT) DOR_NODROPOP;
         else
         {
            ulNum = DrgQueryDragitemCount(pCnrDraginfo->pDragInfo);
            for (i=0; i< ulNum; i++)
            {
               PDRAGITEM pItem;

               pItem=DrgQueryDragitemPtr(pCnrDraginfo->pDragInfo, i);
               if (!DrgVerifyTrueType(pItem, CCDRAGTYPE) &&
                   !DrgVerifyTrueType(pItem, NODEDRAGTYPE))
                  mrReply=(MRESULT) DOR_NEVERDROP;
               else
                  if (!DrgVerifyNativeRMF(pItem, CCRMF) &&
                      !DrgVerifyNativeRMF(pItem, NODERMF))
                     mrReply=(MRESULT) DOR_NEVERDROP;
                  else
                     if (pCnrDraginfo->pDragInfo->usOperation == DO_DEFAULT)
                        mrReply=MRFROM2SHORT(DOR_DROP, DO_MOVE);
                     else
                        mrReply=MRFROM2SHORT(DOR_DROP, pCnrDraginfo->pDragInfo->usOperation);
            }
         }
      }
   DrgFreeDraginfo(pCnrDraginfo->pDragInfo);
   return mrReply;
}

static void ContentsDrop(HWND hwndDlg, PCCLISTDATA pListData, PCNRDRAGINFO pCnrDraginfo)
{
   ULONG ulNum;
   int i;
   PCCENTRY pSource;
   PDRAGITEM pItem;
   BOOL bNeedUpdate=FALSE;
   BOOL bMoved = FALSE;

   DrgAccessDraginfo(pCnrDraginfo->pDragInfo);
   ulNum = DrgQueryDragitemCount(pCnrDraginfo->pDragInfo);
   for (i=0; i<ulNum; i++)
   {
      pItem=DrgQueryDragitemPtr(pCnrDraginfo->pDragInfo, i);
      if (DrgVerifyTrueType(pItem, NODEDRAGTYPE))
      {
         /* Node-Eintrag, neu erzeugen */
         CCENTRY NewCC;
         char pchTemp[LEN_5DADDRESS+LEN_USERNAME+2];
         char *pchTemp2;

         DrgQueryStrName(pItem->hstrSourceName, sizeof(pchTemp)-1, pchTemp);
         pchTemp2 = strchr(pchTemp, ' ');
         *pchTemp2 = 0;
         pchTemp2++;

         strcpy(NewCC.pchName, pchTemp2);
         strcpy(NewCC.pchAddress, pchTemp);
         NewCC.ulFlags = CCENTRY_MENTION;

         /* einfuegen */
         AddCCEntry(pListData->pCCAnchor, pListData->pList, &NewCC);
         bNeedUpdate=TRUE;
      }
      else
         if (DrgVerifyTrueType(pItem, CCDRAGTYPE))
         {
            /* Carbon-Copy-Eintrag, uebertragen */
            switch (pCnrDraginfo->pDragInfo->usOperation)
            {
               case DO_MOVE:
               case DO_COPY:
                  pSource=(PCCENTRY)pItem->ulItemID;
                  AddCCEntry(pListData->pCCAnchor, pListData->pList, pSource);
                  bNeedUpdate=TRUE;
                  if (pCnrDraginfo->pDragInfo->usOperation == DO_MOVE)
                     bMoved = TRUE;
                  break;

               default:
                  WinAlarm(HWND_DESKTOP, WA_ERROR);
                  break;
            }
         }
   }
   if (bMoved)
      SendMsg(pCnrDraginfo->pDragInfo->hwndSource, DM_DISCARDOBJECT,
                 pCnrDraginfo->pDragInfo, NULL);
   DrgDeleteDraginfoStrHandles(pCnrDraginfo->pDragInfo);
   DrgFreeDraginfo(pCnrDraginfo->pDragInfo);

   if (bNeedUpdate)
   {
      UpdateCCEntries(WinWindowFromID(hwndDlg, IDD_CCLIST+1), pListData->pList);
      if (pListData->pList->pEntries)
      {
         WinEnableControl(hwndDlg, IDD_CCLIST+4, TRUE);
         WinEnableControl(hwndDlg, IDD_CCLIST+5, TRUE);
      }
   }

   return;
}

/*------------------------------ SortCCContents     -------------------------*/
/* Sortiert Die CC-Liste                                                     */
/*---------------------------------------------------------------------------*/

static  SHORT EXPENTRY SortCCContents(CCLISTRECORD *p1, CCLISTRECORD *p2, PVOID pStorage)
{
   pStorage=pStorage;

   return strcmp(p1->pchAddress, p2->pchAddress);
}

/*------------------------------ CleanupCCList      -------------------------*/
/* Loescht den Container fuer die CC-Liste                                   */
/*---------------------------------------------------------------------------*/

static  void CleanupCCList(HWND hwndContainer)
{
   CCLISTRECORD *pRecord;

   pRecord=(CCLISTRECORD *)SendMsg(hwndContainer, CM_QUERYRECORD, NULL,
                                      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
   while (pRecord)
   {
      free (pRecord->pchShow);
      pRecord=(CCLISTRECORD *)SendMsg(hwndContainer, CM_QUERYRECORD, pRecord,
                                         MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
   }
   SendMsg(hwndContainer, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));

   return;
}

/*------------------------------ UpdateCCEntries    -------------------------*/
/* Fuellt den Container neu mit den Eintraegen der Liste                     */
/*---------------------------------------------------------------------------*/

static  void UpdateCCEntries(HWND hwndContainer, PCCLIST pCCList)
{
   PCCENTRY pTemp;
   PCCLISTRECORD pRecord, pFirstRecord;
   RECORDINSERT RecordInsert;
   ULONG ulCount=0;

   CleanupCCList(hwndContainer);

   /* zaehlen */
   pTemp = pCCList->pEntries;
   while (pTemp)
   {
      ulCount++;
      pTemp = pTemp->next;
   }

   if (ulCount)
   {
      pFirstRecord=(CCLISTRECORD *)SendMsg(hwndContainer, CM_ALLOCRECORD,
                                              MPFROMLONG(sizeof(CCLISTRECORD)-sizeof(MINIRECORDCORE)),
                                              MPFROMLONG(ulCount));
      pRecord = pFirstRecord;

      pTemp=pCCList->pEntries;
      while (pTemp && pRecord)
      {
         pRecord->pEntry = pTemp;   /* Pointer sichern */
         pRecord->pchUserName=pTemp->pchName;
         pRecord->pchAddress=pTemp->pchAddress;
         pRecord->pchShow=malloc(10);
         if (pTemp->ulFlags & CCENTRY_MENTION)
            LoadString(IDST_CC_YES, 10, pRecord->pchShow);
         else
            LoadString(IDST_CC_NO, 10, pRecord->pchShow);

         pRecord = (CCLISTRECORD *) pRecord->RecordCore.preccNextRecord;
         pTemp=pTemp->next;
      }
      RecordInsert.cb=sizeof(RECORDINSERT);
      RecordInsert.pRecordOrder=(PRECORDCORE) CMA_END;
      RecordInsert.pRecordParent=NULL;
      RecordInsert.fInvalidateRecord=TRUE;
      RecordInsert.zOrder=CMA_TOP;
      RecordInsert.cRecordsInsert=ulCount;
      SendMsg(hwndContainer, CM_INSERTRECORD, pFirstRecord, &RecordInsert);
   }
   return;
}

/*--------------------------------- CCEntryProc -----------------------------*/
/* Fenster-Prozedur fuer CC-Eintrag                                          */
/*---------------------------------------------------------------------------*/

static  MRESULT EXPENTRY CCEntryProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWPOSITIONS windowpositions;
   extern HWND hwndhelp;
   PCCENTRYPAR pCCEntryPar;
   extern char CurrentAddress[LEN_5DADDRESS+1];

   switch(message)
   {
      case WM_INITDLG:
         pCCEntryPar=(PCCENTRYPAR) mp2;
         WinSetWindowULong(parent, QWL_USER, (ULONG) pCCEntryPar);
         WinAssociateHelpInstance(hwndhelp, parent);
         WinSubclassWindow(WinWindowFromID(parent, IDD_CCLISTENTRY+3), FidoEntryProc);
         RestoreWinPos(parent, &windowpositions.ccentrypos, FALSE, TRUE);
         WinSendDlgItemMsg(parent, IDD_CCLISTENTRY+2, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_USERNAME), NULL);
         WinSendDlgItemMsg(parent, IDD_CCLISTENTRY+3, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_5DADDRESS), NULL);
         WinSendDlgItemMsg(parent, IDD_CCLISTENTRY+9, EM_SETTEXTLIMIT,
                           MPFROMSHORT(LEN_FIRSTLINE), NULL);
         if (pCCEntryPar->CCEntry.ulFlags & CCENTRY_MENTION)
           WinCheckButton(parent, IDD_CCLISTENTRY+4, TRUE);

         WinSetDlgItemText(parent, IDD_CCLISTENTRY+2, pCCEntryPar->CCEntry.pchName);
         WinSetDlgItemText(parent, IDD_CCLISTENTRY+3, pCCEntryPar->CCEntry.pchAddress);
         WinSetDlgItemText(parent, IDD_CCLISTENTRY+9, pCCEntryPar->CCEntry.pchFirstLine);
         if (pCCEntryPar->CCEntry.pchName[0] &&
             pCCEntryPar->CCEntry.pchAddress[0])
           WinEnableControl(parent, DID_OK, TRUE);
         break;

      case WM_CHAR:
         if (WinQueryDlgItemTextLength(parent, IDD_CCLISTENTRY+2) &&
             WinQueryDlgItemTextLength(parent, IDD_CCLISTENTRY+3))
           WinEnableControl(parent, DID_OK, TRUE);
         else
           WinEnableControl(parent, DID_OK, FALSE);
         break;

      case WM_COMMAND:
         pCCEntryPar=(PCCENTRYPAR)WinQueryWindowULong(parent, QWL_USER);
         if (SHORT1FROMMP(mp1)==DID_OK)
         {
            WinQueryDlgItemText(parent, IDD_CCLISTENTRY+2, LEN_USERNAME+1, pCCEntryPar->CCEntry.pchName);
            WinQueryDlgItemText(parent, IDD_CCLISTENTRY+3, LEN_5DADDRESS+1, pCCEntryPar->CCEntry.pchAddress);
            WinQueryDlgItemText(parent, IDD_CCLISTENTRY+9, LEN_FIRSTLINE+1, pCCEntryPar->CCEntry.pchFirstLine);
            pCCEntryPar->CCEntry.ulFlags = 0;
            if (WinQueryButtonCheckstate(parent, IDD_CCLISTENTRY+4))
               pCCEntryPar->CCEntry.ulFlags |= CCENTRY_MENTION;
         }

         if (SHORT1FROMMP(mp1)==IDD_CCLISTENTRY+7)
         {
            /* Nodelist-Lookup */
            char FoundName[LEN_USERNAME+1];
            char FoundAddress[LEN_5DADDRESS+1];

            WinQueryDlgItemText(parent, IDD_CCLISTENTRY+2, sizeof(FoundName), FoundName);
            if (PerformNameLookup(FoundName, parent, LOOKUP_NORMAL, FoundName, FoundAddress))
            {
               WinSetDlgItemText(parent, IDD_CCLISTENTRY+2, FoundName);
               WinSetDlgItemText(parent, IDD_CCLISTENTRY+3, FoundAddress);
               WinEnableControl(parent, DID_OK, TRUE);
            }
            return (MRESULT) FALSE;
         }
         break;

      case WM_CONTROL:
         pCCEntryPar=(PCCENTRYPAR)WinQueryWindowULong(parent, QWL_USER);
         if (SHORT1FROMMP(mp1)==IDD_CCLISTENTRY+2 &&
             SHORT2FROMMP(mp1)==EN_KILLFOCUS)
         {
            char pchTemp[LEN_USERNAME+1];
            PNICKNAME pNickEntry;
            extern NICKNAMELIST NickNameList;

            WinQueryWindowText((HWND) mp2, LEN_USERNAME+1, pchTemp);

            if (pNickEntry=FindNickname(&NickNameList, pchTemp, NULL))
            {
               WinSetDlgItemText(parent, IDD_CCLISTENTRY+2,
                                 pNickEntry->username);
               WinSetDlgItemText(parent, IDD_CCLISTENTRY+3,
                                 pNickEntry->address);
               WinSetDlgItemText(parent, IDD_CCLISTENTRY+9,
                                 pNickEntry->firstline);
            }
         }
         if (SHORT1FROMMP(mp1)==IDD_CCLISTENTRY+3)
            if (SHORT2FROMMP(mp1)==EN_KILLFOCUS)
            {
               FTNADDRESS NetAddr;
               char pchTemp[LEN_5DADDRESS+1];

               WinQueryDlgItemText(parent, SHORT1FROMMP(mp1),
                                   LEN_5DADDRESS+1, pchTemp);
               StringToNetAddr(pchTemp, &NetAddr, CurrentAddress);
               NetAddrToString(pchTemp, &NetAddr);
               WinSetDlgItemText(parent, SHORT1FROMMP(mp1), pchTemp);
            }
         break;

      case WM_DESTROY:
      case WM_CLOSE:
         QueryWinPos(parent, &windowpositions.ccentrypos);
         WinAssociateHelpInstance(hwndhelp, WinQueryWindow(parent, QW_OWNER));
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*------------------------------ CCListSelectProc ---------------------------*/
/* Fenster-Prozedur der CC-List-Auswahl                                      */
/*---------------------------------------------------------------------------*/

MRESULT EXPENTRY CCListSelectProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern HWND hwndhelp, frame;
   extern WINDOWPOSITIONS windowpositions;
   static CCSELECTPAR *pCCSelectPar;

   switch (message)
   {
      case WM_INITDLG:
         pCCSelectPar=(CCSELECTPAR *)mp2;
         WinAssociateHelpInstance(hwndhelp, parent);
         RestoreWinPos(parent, &windowpositions.ccselectpos, FALSE, TRUE);
         if (!ccanchor.pLists)
            WinEnableControl(parent, DID_OK, FALSE);
         UpdateCCList(WinWindowFromID(parent, IDD_CCLISTSELECT+1), pCCSelectPar->bEmptyLists);
         break;

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1)==DID_OK)
         {
            USHORT usItem;

            usItem=(USHORT)WinSendDlgItemMsg(parent, IDD_CCLISTSELECT+1, LM_QUERYSELECTION,
                                             MPFROMSHORT(LIT_FIRST), NULL);
            pCCSelectPar->ulSelectID=(ULONG)WinSendDlgItemMsg(parent, IDD_CCLISTSELECT+1,
                                                              LM_QUERYITEMHANDLE,
                                                              MPFROMSHORT(usItem),
                                                              NULL);
         }
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_CCLISTSELECT+1)
         {
            if (SHORT2FROMMP(mp1) == LN_ENTER)
            {
               SendMsg(parent, WM_COMMAND, MPFROMSHORT(DID_OK), NULL);
            }
         }
         break;

      case WM_CLOSE:
      case WM_DESTROY:
         WinAssociateHelpInstance(hwndhelp, frame);
         QueryWinPos(parent, &windowpositions.ccselectpos);
         break;

      default:
         break;
   }
   return WinDefDlgProc(parent, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: AddToCCList                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt einen Absender in eine CC-Liste ein                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndOwner: Owner-Window des Auswahl-Dialogs                    */
/*            pHeader: Aktueller Header                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void AddToCCList(HWND hwndOwner, MSGHEADER *pHeader)
{
   CCSELECTPAR CCSelect;
   extern HWND client;

   if (ccanchor.ulNumLists == 0)
      return;

   SendMsg(client, WORKM_DISABLEVIEWS, NULL, NULL);

   CCSelect.cb=sizeof(CCSelect);
   CCSelect.ulSelectID = 0;
   CCSelect.bEmptyLists=TRUE;
   if (WinDlgBox(HWND_DESKTOP, hwndOwner, CCListSelectProc, hmodLang,
             IDD_CCLISTSELECT, &CCSelect)==DID_OK)
   {
      if (CCSelect.ulSelectID)
      {
         PCCLIST pList;

         pList = QueryCCList(&ccanchor, CCSelect.ulSelectID);

         if (pList)
         {
            CCENTRY CCEntry;

            memcpy(CCEntry.pchName, pHeader->pchFromName, LEN_USERNAME+1);
            NetAddrToString(CCEntry.pchAddress, &pHeader->FromAddress);
            CCEntry.pchFirstLine[0] = '\0';
            CCEntry.ulFlags = CCENTRY_MENTION;

            AddCCEntry(&ccanchor, pList, &CCEntry);

            if (hwndCCLists)
               SendMsg(hwndCCLists, CCM_UPDATELIST, MPFROMLONG(CCSelect.ulSelectID),
                          NULL);
         }
      }
   }

   SendMsg(client, WORKM_ENABLEVIEWS, NULL, NULL);
   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ImportCCList                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Importieren einer CC-Liste                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndOwner: Owner-Window des Auswahl-Dialogs                    */
/*            pDestList: Liste f〉 den Eintrag                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 OK                                                       */
/*                1 Abgebrochen                                              */
/*                2 Fehler beim Lesen des Files                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  int ImportCCList(HWND hwndOwner, PCCANCHOR pCCAnchor, PCCLIST pDestList)
{
   FILEDLG dlgpar;
   FILE *pfImport;

   /* Filename holen */

   dlgpar.cbSize=sizeof(dlgpar);
   dlgpar.fl= FDS_CENTER | FDS_CUSTOM | FDS_HELPBUTTON |
              FDS_OPEN_DIALOG;
   dlgpar.pszTitle=NULL;
   dlgpar.pszOKButton="OK";
   dlgpar.pfnDlgProc=ImportCCProc;
   dlgpar.pszIType=NULL;
   dlgpar.papszITypeList=NULL;
   dlgpar.pszIDrive=NULL;
   dlgpar.papszIDriveList=NULL;
   dlgpar.hMod=hmodLang;
   strcpy(dlgpar.szFullFile, "*");
   dlgpar.usDlgId=IDD_IMPORTCCLIST;

   WinFileDlg(HWND_DESKTOP,
              hwndOwner,
              &dlgpar);

   if (dlgpar.lReturn != DID_OK)
      return 1;

   /* Datei oeffnen */
   pfImport = fopen(dlgpar.szFullFile, "r");

   if (!pfImport)
      return 2;

   /* Eintraege lesen */
   if (ReadCCFile(pfImport, pCCAnchor, pDestList, dlgpar.ulUser))
      MessageBox(hwndOwner, IDST_MSG_INVALIDFILE, 0,
                 IDD_INVALIDFILE, MB_OK | MB_ERROR);

   /* Datei schliessen */
   fclose(pfImport);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ImportCCProc                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Window-Procedure f〉 den File-Dialog                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: (Window-Proc)                                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: MRESULT                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Bit 1 des User-Felds                                           */
/*               1 = Default "Yes"                                           */
/*               0 = Default "No"                                            */
/*---------------------------------------------------------------------------*/

static  MRESULT EXPENTRY ImportCCProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   PFILEDLG pFileDlg;

   switch (message)
   {
      case WM_INITDLG:
         WinCheckButton(hwnd, IDD_IMPORTCCLIST+2, TRUE);
         break;

      case WM_COMMAND:
         pFileDlg=(PFILEDLG)WinQueryWindowULong(hwnd, QWL_USER);
         pFileDlg->ulUser=0;

         /* Radio-Buttons auslesen */
         if (WinQueryButtonCheckstate(hwnd, IDD_IMPORTCCLIST+2))
            pFileDlg->ulUser |= CCENTRY_MENTION;
         else
            pFileDlg->ulUser &= ~CCENTRY_MENTION;
         break;

      default:
         break;
   }
   return WinDefFileDlgProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadCCFile                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liesst die Textdatei in die Liste ein                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter:    pfImport: File, das gelesen wird                            */
/*               pDestList: Ergebnis-Liste                                   */
/*               bMention: Default f. bMention                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                !=0 Fehler                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*---------------------------------------------------------------------------*/

static  int ReadCCFile(FILE *pfImport, PCCANCHOR pCCAnchor, PCCLIST pDestList, ULONG ulFlags)
{
   char pchLine[500];
   char *pchTemp;
   CCENTRY NewCC;
   FTNADDRESS Addr;

   while (!feof(pfImport))
   {
      if (!fgets(pchLine, 500, pfImport))
         continue;

      StripWhitespace(pchLine);

      /* leere Zeile uebergehen */
      if (!pchLine[0])
         continue;

      /* Kommentarzeile uebergehen */
      if (pchLine[0]==';')
         continue;

      pchTemp = strrchr(pchLine, ' ');

      if (!pchTemp)  /* Fehler */
         continue;

      NewCC.ulFlags = ulFlags;

      *pchTemp = '\0';
      pchTemp++;

      /* Adresse ermitteln */
      strncpy(NewCC.pchAddress, pchTemp, LEN_5DADDRESS);

      StringToNetAddr(NewCC.pchAddress, &Addr, NULL);
      NetAddrToString(NewCC.pchAddress, &Addr);

      /* Name */
      pchTemp = strtok(pchLine, "\t ");
      NewCC.pchName[0]=0;

      while (pchTemp && (strlen(pchTemp)+strlen(NewCC.pchName)+
                         (NewCC.pchName[0]?1:0) <= LEN_USERNAME))
      {
         if (NewCC.pchName[0])
            strcat(NewCC.pchName, " ");
         strcat(NewCC.pchName, pchTemp);

         pchTemp = strtok(NULL, "\t ");
      }

      if (!NewCC.pchName[0])  /* kein Name */
         continue;

      NewCC.pchFirstLine[0]=0;

      /* an Liste anhaengen */
      AddCCEntry(pCCAnchor, pDestList, &NewCC);
   }
   return 0;
}


/*---------------------------------------------------------------------------*/
/* Funktionsname: AddEmptyCCList                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Erzeugt eine neue CC-Liste                                  */
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

static PCCFOLDERRECORD AddEmptyCCList(HWND hwndContainer, HPOINTER hptr)
{
   PCCLIST pNewList;
   RECORDINSERT RecordInsert;
   PCCFOLDERRECORD pNewRecord;
   char pchNewName[50];

   pNewList = AddCCList(&ccanchor, CreateUniqueName(IDST_CC_NEWNAME, &ccanchor,
                                                    HaveCCListName,
                                                    sizeof(pchNewName), pchNewName));

   /* Record vom Container anfordern */
   pNewRecord = SendMsg(hwndContainer, CM_ALLOCRECORD,
                           MPFROMLONG(sizeof(CCFOLDERRECORD) - sizeof(MINIRECORDCORE)),
                           MPFROMLONG(1));

   if (pNewRecord)
   {
      pNewRecord->hwndContents = NULLHANDLE;
      pNewRecord->pList        = pNewList;

      pNewRecord->RecordCore.flRecordAttr = 0;
      pNewRecord->RecordCore.pszIcon = pNewList->pchListName;
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
/* Funktionsname: FillCCFolder                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuegt alle Objekte in den CC-Folder ein                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit CC-Listen                         */
/*            hptr: verwendetes Icon                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0   OK                                                     */
/*                -1  Fehler                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  int FillCCFolder(HWND hwndContainer, HPOINTER hptr)
{
   RECORDINSERT RecordInsert;
   PCCFOLDERRECORD pRecord, pFirstRecord;
   ULONG ulNumLists=0;
   PCCLIST pList;

   ulNumLists = ccanchor.ulNumLists;

   if (ulNumLists == 0)
      return -1;

   /* Records vom Container anfordern */
   pFirstRecord = SendMsg(hwndContainer, CM_ALLOCRECORD,
                             MPFROMLONG(sizeof(CCFOLDERRECORD) - sizeof(MINIRECORDCORE)),
                             MPFROMLONG(ulNumLists));
   pRecord = pFirstRecord;

   pList = ccanchor.pLists;
   while (pRecord)
   {
      pRecord->hwndContents = NULLHANDLE;
      pRecord->pList        = pList;

      pRecord->RecordCore.flRecordAttr = 0;
      pRecord->RecordCore.pszIcon = pList->pchListName;

      pRecord->RecordCore.hptrIcon = hptr;

      pRecord = (PCCFOLDERRECORD) pRecord->RecordCore.preccNextRecord;
      pList = pList->next;
   }

   /* Records einfuegen */
   RecordInsert.cb = sizeof(RECORDINSERT);
   RecordInsert.pRecordOrder = (PRECORDCORE) CMA_END;
   RecordInsert.pRecordParent = NULL;
   RecordInsert.fInvalidateRecord = TRUE;
   RecordInsert.zOrder = CMA_TOP;
   RecordInsert.cRecordsInsert = ulNumLists;

   SendMsg(hwndContainer, CM_INSERTRECORD, pFirstRecord, &RecordInsert);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: OpenCCList                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Oeffnet ein CC-Listen-Objekt                                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Objekt                            */
/*            pRecord: Record-Pointer der CC-Liste                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 OK                                                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Ein bereits offenes Template bekommt lediglich den Fokus       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  int OpenCCList(HWND hwndContainer, PCCFOLDERRECORD pRecord)
{
   CCLISTPAR CCListPar;

   if (pRecord->hwndContents)  /* schon offen ? */
      SetFocus(pRecord->hwndContents);
   else
   {
      /* in-use-emphasis setzen */
      SendMsg(hwndContainer, CM_SETRECORDEMPHASIS, pRecord,
                 MPFROM2SHORT(TRUE, CRA_INUSE));

      CCListPar.cb=sizeof(CCLISTPAR);
      CCListPar.pList=pRecord->pList;
      CCListPar.pCCAnchor=&ccanchor;

      pRecord->hwndContents=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, CCListContentsProc,
                                       hmodLang, IDD_CCLIST, &CCListPar);
      WinShowWindow(pRecord->hwndContents, TRUE);
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: DeleteCCList                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Loescht ein CC-List-Objekt                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Objekt                            */
/*            pRecord: Record-Pointer der CC-Liste                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 OK                                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  int DeleteCCListF(HWND hwndContainer, PCCFOLDERRECORD pRecord)
{
   /* offene Liste schliessen */
   if (pRecord->hwndContents)
      WinDestroyWindow(pRecord->hwndContents);

   /* Record im Container loeschen */
   SendMsg(hwndContainer, CM_REMOVERECORD, &pRecord,
              MPFROM2SHORT(1, CMA_INVALIDATE));

   DeleteCCList(&ccanchor, pRecord->pList);

   /* endgueltig aus Container entfernen */
   SendMsg(hwndContainer, CM_FREERECORD, &pRecord, MPFROMLONG(1));

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CleanupCCFolder                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Entfernt alle Objekte aus dem CC-List-Folder                */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndContainer: Container mit Objekt                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0 OK                                                       */
/*                -1  Fehler                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Schliesst auch alle offenen CC-Listen                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  int CleanupCCFolder(HWND hwndContainer)
{
   PCCFOLDERRECORD pRecord = NULL;

   /* alle offenen Templates schliessen */
   while (pRecord = SendMsg(hwndContainer, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->hwndContents)
         WinDestroyWindow(pRecord->hwndContents);
   }

   /* Folder leeren */
   SendMsg(hwndContainer, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE));

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CCFolderProc                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fensterprozedur des CC-List-Folders                         */
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

MRESULT EXPENTRY CCFolderProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   extern WINDOWCOLORS windowcolors;
   extern WINDOWFONTS  windowfonts;
   extern GENERALOPT generaloptions;
   extern HWND hwndhelp, client;
   PCCFOLDERDATA pFolderData = (PCCFOLDERDATA) WinQueryWindowULong(hwnd, QWL_USER);

   switch (message)
   {
      case WM_INITDLG:
         /* Instanzdaten anfordern */
         pFolderData = malloc(sizeof(CCFOLDERDATA));
         memset(pFolderData, 0, sizeof(CCFOLDERDATA));
         WinSetWindowULong(hwnd, QWL_USER, (ULONG) pFolderData);

         OldContainerProc = WinSubclassWindow(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                                              NewCCContainerProc);

         /* Icon laden */
         pFolderData->hptrCCList = LoadIcon(IDIC_CCLIST);
         pFolderData->hptrCCFolder = LoadIcon(IDIC_CCFOLDER);

         SendMsg(hwnd, WM_SETICON, (MPARAM) pFolderData->hptrCCFolder, NULL);

         /* Switch-Entry */
         pFolderData->hSwitch=AddToWindowList(hwnd);

         /* Menues laden */
         pFolderData->hwndListPopup = WinLoadMenu(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                                                  hmodLang, IDM_CCF_POPUP);
         pFolderData->hwndFolderPopup = WinLoadMenu(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                                                    hmodLang, IDM_CCF_POPUP2);

         if (pFolderData->hwndFolderPopup)
            ReplaceSysMenu(hwnd, pFolderData->hwndFolderPopup, 1);

         if (ccanchor.ulFlags & CCANCHOR_FOREGROUND)
         {
            pFolderData->bForeground = TRUE;
            WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_CCF_FGROUND, TRUE);
            WinSetOwner(hwnd, client);
         }
         else
         {
            pFolderData->bForeground = FALSE;
            WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_CCF_FGROUND, FALSE);
            WinSetOwner(hwnd, HWND_DESKTOP);
         }

         /* Farben und Font setzen */
         SetBackground(WinWindowFromID(hwnd, IDD_CCFOLDER+1), &windowcolors.ccfolderback);
         SetForeground(WinWindowFromID(hwnd, IDD_CCFOLDER+1), &windowcolors.ccfolderfore);
         SetFont(WinWindowFromID(hwnd, IDD_CCFOLDER+1), windowfonts.ccfolderfont);

         /* Icons einfuegen */
         InitCCFolder(WinWindowFromID(hwnd, IDD_CCFOLDER+1));
         FillCCFolder(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                      pFolderData->hptrCCList);
         RestoreWinPos(hwnd, &ccanchor.FolderPos, TRUE, TRUE);
         pFolderData->bNotify = TRUE;
         break;

      case WM_DESTROY:
         /* Farben und Font */
         CleanupCCFolder(WinWindowFromID(hwnd, IDD_CCFOLDER+1));
         RemoveFromWindowList(pFolderData->hSwitch);
         QueryBackground(WinWindowFromID(hwnd, IDD_CCFOLDER+1), &windowcolors.ccfolderback);
         QueryForeground(WinWindowFromID(hwnd, IDD_CCFOLDER+1), &windowcolors.ccfolderfore);
         QueryFont(WinWindowFromID(hwnd, IDD_CCFOLDER+1), windowfonts.ccfolderfont);
         if (pFolderData->hptrCCList)
            WinDestroyPointer(pFolderData->hptrCCList);
         if (pFolderData->hptrCCFolder)
            WinDestroyPointer(pFolderData->hptrCCFolder);
         if (pFolderData->hwndListPopup)
            WinDestroyWindow(pFolderData->hwndListPopup);
         if (pFolderData->hwndFolderPopup)
            WinDestroyWindow(pFolderData->hwndFolderPopup);

         if (pFolderData->bForeground)
         {
            if (!(ccanchor.ulFlags & CCANCHOR_FOREGROUND))
            {
               ccanchor.ulFlags |= CCANCHOR_FOREGROUND;
               ccanchor.bDirty = TRUE;
            }
         }
         else
         {
            if (ccanchor.ulFlags & CCANCHOR_FOREGROUND)
            {
               ccanchor.ulFlags &= ~CCANCHOR_FOREGROUND;
               ccanchor.bDirty = TRUE;
            }
         }

         free(pFolderData);
         break;

      case WM_CONTROL:
         if (SHORT1FROMMP(mp1) == IDD_CCFOLDER+1)
         {
            switch(SHORT2FROMMP(mp1))
            {
               PCNREDITDATA pEdit;
               PCCFOLDERRECORD pRecord;
               PNOTIFYRECORDENTER pEnter;

               case CN_ENTER:
                  pEnter = (PNOTIFYRECORDENTER) mp2;
                  if (pEnter->pRecord)
                     OpenCCList(pEnter->hwndCnr,
                                (PCCFOLDERRECORD) pEnter->pRecord);
                  break;

               case CN_REALLOCPSZ:
                  pEdit = (PCNREDITDATA) mp2;
                  pRecord = (PCCFOLDERRECORD) pEdit->pRecord;
                  free (pRecord->pList->pchListName);
                  pRecord->pList->pchListName = malloc(pEdit->cbText+1);
                  pRecord->pList->pchListName[0] = '\0';
                  pRecord->RecordCore.pszIcon = pRecord->pList->pchListName;
                  pRecord->pList->bDirty=TRUE;
                  ccanchor.bDirty = TRUE;
                  return (MRESULT) TRUE;

               case CN_ENDEDIT:
                  /* Template offen ? */
                  pEdit = (PCNREDITDATA) mp2;
                  if (((PCCFOLDERRECORD)pEdit->pRecord)->hwndContents)
                     SendMsg(((PCCFOLDERRECORD)pEdit->pRecord)->hwndContents, CCM_UPDATENAME, NULL, NULL);
                  WinSendDlgItemMsg(hwnd, IDD_CCFOLDER+1, CM_SORTRECORD, MPFROMP(SortCCFolder), NULL);
                  break;

               case CN_INITDRAG:
                  pFolderData->pDragRecord = (PCCFOLDERRECORD) ((PCNRDRAGINIT) mp2)->pRecord;
                  InitCCListDrag(hwnd, (PCNRDRAGINIT) mp2);
                  break;

               case CN_CONTEXTMENU:
                  pFolderData->pPopupRecord = (PCCFOLDERRECORD) mp2;
                  if (pFolderData->pPopupRecord)
                  {
                     /* Popup-Menue eines Templates */
                     RECTL rcl;
                     POINTL ptl;
                     QUERYRECORDRECT QRecord;

                     if (pFolderData->pPopupRecord->pList->ulListID == ulCCSelected)
                        WinEnableMenuItem(pFolderData->hwndListPopup, IDM_CCF_DELETE, FALSE);
                     else
                        WinEnableMenuItem(pFolderData->hwndListPopup, IDM_CCF_DELETE, TRUE);

                     QRecord.cb = sizeof(QUERYRECORDRECT);
                     QRecord.pRecord = (PRECORDCORE) pFolderData->pPopupRecord;
                     QRecord.fRightSplitWindow = FALSE;
                     QRecord.fsExtent = CMA_ICON;
                     WinSendDlgItemMsg(hwnd, IDD_CCFOLDER+1, CM_QUERYRECORDRECT,
                                       &rcl, &QRecord);
                     ptl.x = rcl.xRight;
                     ptl.y = rcl.yBottom;
                     WinMapWindowPoints(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                                        HWND_DESKTOP, &ptl, 1);
                     WinPopupMenu(HWND_DESKTOP, hwnd, pFolderData->hwndListPopup,
                                  ptl.x, ptl.y, 0,
                                  PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD |
                                  PU_MOUSEBUTTON1);
                  }
                  else
                  {
                     /* Popup-Menue des Folders */
                     POINTL ptl;

                     WinQueryPointerPos(HWND_DESKTOP, &ptl);
                     WinPopupMenu(HWND_DESKTOP, hwnd, pFolderData->hwndFolderPopup,
                                  ptl.x, ptl.y, 0,
                                  PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD |
                                  PU_MOUSEBUTTON1);
                  }
                  break;

               case CN_HELP:
                  SendMsg(hwnd, WM_HELP, MPFROMSHORT(IDD_CCFOLDER+1), NULL);
                  break;

               case CN_DROP:
                  CCFolderDrop(WinWindowFromID(hwnd, IDD_CCFOLDER+1), (PCNRDRAGINFO) mp2);
                  break;

               case CN_DRAGOVER:
                  return CCFolderDragOver(WinWindowFromID(hwnd, IDD_CCFOLDER+1), (PCNRDRAGINFO) mp2);

               default:
                  break;
            }
         }
         break;

      case DM_DISCARDOBJECT:
         DrgAccessDraginfo((PDRAGINFO) mp1);
         DeleteCCListF(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                       pFolderData->pDragRecord);
         DrgFreeDraginfo((PDRAGINFO) mp1);
         return (MRESULT) DRR_SOURCE;

      case WM_MENUEND:
         if ((HWND) mp2 == pFolderData->hwndListPopup ||
             (HWND) mp2 == pFolderData->hwndFolderPopup)
         {
            /* Emphasis wegnehmen */
            WinSendDlgItemMsg(hwnd, IDD_CCFOLDER+1, CM_SETRECORDEMPHASIS,
                              pFolderData->pPopupRecord,
                              MPFROM2SHORT(FALSE, CRA_SOURCE));
            if ( (HWND) mp2 == pFolderData->hwndFolderPopup)
               ResetMenuStyle(pFolderData->hwndFolderPopup, hwnd);
         }
         break;

      case WM_INITMENU:
         if ((HWND) mp2 == pFolderData->hwndFolderPopup)
            pFolderData->pPopupRecord=NULL;
         if ((HWND) mp2 == pFolderData->hwndListPopup ||
             (HWND) mp2 == pFolderData->hwndFolderPopup)
         {
            /* Emphasis setzen */
            WinSendDlgItemMsg(hwnd, IDD_CCFOLDER+1, CM_SETRECORDEMPHASIS,
                              pFolderData->pPopupRecord,
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
             WinQueryFocus(HWND_DESKTOP) == WinWindowFromID(hwnd, IDD_CCFOLDER+1))
         {
            WinSendDlgItemMsg(hwnd, IDD_CCFOLDER+1, message,
                              mp1, mp2);
         }
         break;

      case WM_CLOSE:
         WinPostMsg(client, CCFM_CLOSE, NULL, NULL);
         break;

      case WM_ADJUSTWINDOWPOS:
         if (((PSWP)mp1)->fl & SWP_MINIMIZE)
            WinShowWindow(WinWindowFromID(hwnd, IDD_CCFOLDER+1), FALSE);
         if (((PSWP)mp1)->fl & (SWP_MAXIMIZE|SWP_RESTORE))
            WinShowWindow(WinWindowFromID(hwnd, IDD_CCFOLDER+1), TRUE);
         break;

      case WM_ADJUSTFRAMEPOS:
         SizeToClient(anchor, (PSWP) mp1, hwnd, IDD_CCFOLDER+1);
         break;

      case WM_WINDOWPOSCHANGED:
         if (pFolderData && pFolderData->bNotify)
            SaveWinPos(hwnd, (PSWP) mp1, &ccanchor.FolderPos, &ccanchor.bDirty);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            PCCFOLDERRECORD pNewCCList;

            case IDM_CCF_OPEN:
               OpenCCList(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                          pFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case IDM_CCF_CREATE:
               if (pNewCCList = AddEmptyCCList(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                                               pFolderData->hptrCCList))
                  OpenCCList(WinWindowFromID(hwnd, IDD_CCFOLDER+1), pNewCCList);
               return (MRESULT) FALSE;

            case IDM_CCF_DELETE:
               if (generaloptions.safety & SAFETY_CHANGESETUP)
                  if (MessageBox(hwnd, IDST_TITLE_DELCCLIST, IDST_MSG_DELCCLIST,
                                 IDD_DELCCLIST, MB_YESNO| MB_QUERY| MB_DEFBUTTON2) != MBID_YES)
                     return (MRESULT) FALSE;
               DeleteCCListF(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                             pFolderData->pPopupRecord);
               return (MRESULT) FALSE;

            case DID_CANCEL:
               break;

            case IDM_CCF_FGROUND:
               if (pFolderData->bForeground)
               {
                  pFolderData->bForeground = FALSE;
                  WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_CCF_FGROUND, FALSE);
                  WinSetOwner(hwnd, HWND_DESKTOP);
               }
               else
               {
                  pFolderData->bForeground = TRUE;
                  WinCheckMenuItem(pFolderData->hwndFolderPopup, IDM_CCF_FGROUND, TRUE);
                  WinSetOwner(hwnd, client);
               }
               return (MRESULT) FALSE;

            default:
               return (MRESULT) FALSE;
         }
         return (MRESULT) FALSE;

      case CCM_CLOSE:
         CCListClosed(WinWindowFromID(hwnd, IDD_CCFOLDER+1),
                      (HWND) mp1);
         break;

      case CCM_UPDATELIST:
         CCListSendUpdate(WinWindowFromID(hwnd, IDD_CCFOLDER+1), (ULONG) mp1);
         break;

      default:
         break;
   }
   return WinDefDlgProc(hwnd, message, mp1, mp2);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: InitCCListDrag                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Startet Drag f〉 eine CC-Liste                              */
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

static  int InitCCListDrag(HWND hwndDlg, PCNRDRAGINIT pInit)
{
   PDRAGINFO pDraginfo;
   DRAGITEM dItem;
   DRAGIMAGE dImage;

   if (!pInit->pRecord)
      return -1;

   if (((PCCFOLDERRECORD)pInit->pRecord)->pList->ulListID == ulCCSelected)
      return -1;

   WinSendDlgItemMsg(hwndDlg, IDD_CCFOLDER+1, CM_SETRECORDEMPHASIS,
                     pInit->pRecord, MPFROM2SHORT(TRUE, CRA_SOURCE));

   pDraginfo = DrgAllocDraginfo(1);
   pDraginfo->usOperation=DO_MOVE;
   pDraginfo->hwndSource=hwndDlg;

   /* Drag-Item vorbereiten*/
   dItem.hwndItem=hwndDlg;
   dItem.ulItemID= ((PCCFOLDERRECORD)pInit->pRecord)->pList->ulListID;
   dItem.hstrType=DrgAddStrHandle(CCLISTDRAGTYPE);
   dItem.hstrRMF=DrgAddStrHandle(CCLISTRMF);
   dItem.hstrSourceName=DrgAddStrHandle(pInit->pRecord->pszIcon);
   dItem.hstrTargetName=DrgAddStrHandle(pInit->pRecord->pszIcon);
   if (((PCCFOLDERRECORD) pInit->pRecord)->hwndContents)
      dItem.fsControl= DC_OPEN;
   else
      dItem.fsControl=0;
   dItem.fsSupportedOps=DO_MOVEABLE;
   DrgSetDragitem(pDraginfo, &dItem, sizeof(dItem), 0);

   /* Drag-Image vorbereiten */
   dImage.cb=sizeof(DRAGIMAGE);
   dImage.hImage=pInit->pRecord->hptrIcon;
   dImage.fl=DRG_ICON;
   dImage.cxOffset=0;
   dImage.cyOffset=0;

   /* Und los gehts */
   DrgDrag(hwndDlg, pDraginfo, &dImage, 1, VK_ENDDRAG, NULL);
   DrgFreeDraginfo(pDraginfo);

   WinSendDlgItemMsg(hwndDlg, IDD_CCFOLDER+1, CM_SETRECORDEMPHASIS,
                     pInit->pRecord, MPFROM2SHORT(FALSE, CRA_SOURCE));

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CCListClosed                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Raeumt nach dem Schliessen einer CC-Liste  wieder auf       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            hwndList: Window-Handle der geschlossenen Liste                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  void CCListClosed(HWND hwndCnr, HWND hwndList)
{
   PCCFOLDERRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->hwndContents == hwndList)
      {
         if (pRecord->hwndContents)
            WinDestroyWindow(pRecord->hwndContents);
         pRecord->hwndContents = NULLHANDLE;

         SendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pRecord,
                    MPFROM2SHORT(FALSE, CRA_INUSE));
         break;
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CCListSendUpdate                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Sendet eine Updatemessage an die CC-Liste, falls sie offen  */
/*               ist                                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            uListID: List-ID                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  void CCListSendUpdate(HWND hwndCnr, ULONG ulListID)
{
   PCCFOLDERRECORD pRecord = NULL;

   while (pRecord = SendMsg(hwndCnr, CM_QUERYRECORD, pRecord,
                               MPFROM2SHORT(pRecord ? CMA_NEXT : CMA_FIRST,
                                            CMA_ITEMORDER)))
   {
      if (pRecord->pList->ulListID == ulListID)
      {
         if (pRecord->hwndContents)
            SendMsg(pRecord->hwndContents, CCM_UPDATELIST, NULL, NULL);
         break;
      }
   }

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CCFolderDragOver                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Behandelt Drag-Over des CC-Folders                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            pCnrDrag: Drag-Infos des Containers                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: siehe CN_DRAGOVER                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  MRESULT CCFolderDragOver(HWND hwndCnr, PCNRDRAGINFO pCnrDrag)
{
   USHORT usDrop = DOR_NEVERDROP;
   USHORT usDefaultOp = DO_UNKNOWN;

   hwndCnr = hwndCnr;

   if (pCnrDrag->pRecord)
   {
      DrgAccessDraginfo(pCnrDrag->pDragInfo);
      usDefaultOp = pCnrDrag->pDragInfo->usOperation;
      if (usDefaultOp == DO_COPY ||
          usDefaultOp == DO_MOVE ||
          usDefaultOp == DO_DEFAULT )
      {
         if (WinQueryAnchorBlock(pCnrDrag->pDragInfo->hwndSource) == anchor)
         {
            PDRAGITEM pdItem;
            pdItem = DrgQueryDragitemPtr(pCnrDrag->pDragInfo, 0);

            if (DrgVerifyType(pdItem, CCDRAGTYPE))
            {
               usDrop = DOR_DROP;
               if (usDefaultOp == DO_DEFAULT)
                  usDefaultOp = DO_MOVE;
            }
            else
               if (DrgVerifyType(pdItem, NODEDRAGTYPE))
               {
                  usDrop = DOR_DROP;
                  if (usDefaultOp == DO_DEFAULT)
                     usDefaultOp = DO_COPY;
               }
         }
      }
      else
         usDrop = DOR_NODROPOP;

      DrgFreeDraginfo(pCnrDrag->pDragInfo);
   }
   else
      usDrop = DOR_NODROP;

   return MRFROM2SHORT(usDrop, usDefaultOp);
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CCFolderDrop                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Behandelt Drop auf den CC-Folder                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hwndCnr: Container-Window                                      */
/*            pCnrDrag: Drag-Infos des Containers                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static  void CCFolderDrop(HWND hwndCnr, PCNRDRAGINFO pCnrDrag)
{
   PCCENTRY pSource;
   ULONG ulNum;
   int i;
   BOOL bNeedUpdate=FALSE;

   hwndCnr = hwndCnr;

   DrgAccessDraginfo(pCnrDrag->pDragInfo);

   if (pCnrDrag->pDragInfo->usOperation == DO_MOVE ||
       pCnrDrag->pDragInfo->usOperation == DO_COPY ||
       pCnrDrag->pDragInfo->usOperation == DO_DEFAULT)
   {
      PDRAGITEM pItem;

      ulNum = DrgQueryDragitemCount(pCnrDrag->pDragInfo);
      for (i=0; i < ulNum; i++)
      {
         pItem=DrgQueryDragitemPtr(pCnrDrag->pDragInfo, i);

         if (DrgVerifyType(pItem, CCDRAGTYPE))
         {
            pSource=(PCCENTRY)pItem->ulItemID;
            AddCCEntry(&ccanchor, ((PCCFOLDERRECORD)pCnrDrag->pRecord)->pList, pSource);
            bNeedUpdate=TRUE;

            if (pCnrDrag->pDragInfo->usOperation == DO_MOVE)
            {
               SendMsg(pCnrDrag->pDragInfo->hwndSource, DM_DISCARDOBJECT,
                          pCnrDrag->pDragInfo, NULL);
            }
         }
         else
            if (DrgVerifyType(pItem, NODEDRAGTYPE))
            {
               /* Node-Eintrag, neu erzeugen */
               CCENTRY NewCC;
               char pchTemp[LEN_5DADDRESS+LEN_USERNAME+2];
               char *pchTemp2;

               DrgQueryStrName(pItem->hstrSourceName, sizeof(pchTemp)-1, pchTemp);
               pchTemp2 = strchr(pchTemp, ' ');
               *pchTemp2 = 0;
               pchTemp2++;

               strcpy(NewCC.pchName, pchTemp2);
               strcpy(NewCC.pchAddress, pchTemp);
               NewCC.ulFlags = CCENTRY_MENTION;

               /* einfuegen */
               AddCCEntry(&ccanchor, ((PCCFOLDERRECORD)pCnrDrag->pRecord)->pList, &NewCC);
               bNeedUpdate = TRUE;
            }
      }
   }

   DrgDeleteDraginfoStrHandles(pCnrDrag->pDragInfo);
   DrgFreeDraginfo(pCnrDrag->pDragInfo);

   if (bNeedUpdate)
      if (((PCCFOLDERRECORD)pCnrDrag->pRecord)->hwndContents)
         SendMsg(((PCCFOLDERRECORD)pCnrDrag->pRecord)->hwndContents, CCM_UPDATELIST,
                    NULL, NULL);

   return;
}

static SHORT EXPENTRY SortCCFolder(PCCFOLDERRECORD p1, PCCFOLDERRECORD p2, PVOID pStorage)
{
   pStorage = pStorage;
   return stricmp(p1->RecordCore.pszIcon, p2->RecordCore.pszIcon);
}

static void InitCCFolder(HWND hwndCnr)
{
   CNRINFO cnrinfo;

   cnrinfo.cb = sizeof(cnrinfo);
   cnrinfo.pSortRecord=(PVOID)SortCCFolder;

   WinSendMsg(hwndCnr, CM_SETCNRINFO, &cnrinfo, MPFROMLONG(CMA_PSORTRECORD));
   return;
}

/*-------------------------------- Modulende --------------------------------*/

/*---------------------------------------------------------------------------+
 | Titel: FLTV7.C                                                            |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 03.12.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Version 7 Nodelist-Lookup                                               |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <share.h>
#include <sys\stat.h>
#include <string.h>
#include <io.h>
#include <ctype.h>

#ifdef STANDALONE
   #include "vers7.h"
#else
   #include "..\main.h"
   #include "..\structs.h"
   #include "..\msgheader.h"
   #include "..\util\addrcnv.h"
   #include "fltv7.h"
#endif

#include "fltv7structs.h"

#ifndef STANDALONE
   #include "v7browse.h"
#endif

/*--------------------------------- Defines ---------------------------------*/

#define is_alpha(x) ((x)>64 && (x)<91)

#ifndef min
#define min(a,b)     ((a)<=(b)?(a):(b))
#endif

#define BROWSEDELTA   1000

/*---------------------------- Globale Variablen ----------------------------*/

static const char unwrk[] = " EANROSTILCHBDMUGPKYWFVJXZQ-'0123456789";

/*----------------------- interne Funktionsprototypen -----------------------*/

static LONG BTreeInit(PV7LOOKUP pLookup);
static LONG BTreeFindFirst(PV7LOOKUP pLookup);
static LONG BTreeSiftDown(PV7LOOKUP pLookup);
static LONG BTreeTraverse(PV7LOOKUP pLookup);
static PNDX get7node(FILE *stream, LONG lPos, PNDX pNdx);
static int get_ver7_info (V7P_HEADER *pHeader, FILE *pfData, FILE *pfDataDTP, LONG lOffset, PNODEDATA pNodeData);
static int namecmp(void *pchName1, void *pchName2, unsigned int n);
static int addrcmp(void *addr1, void *addr2, unsigned int n);
static void unpk(char *instr, char *outp, int countin, int countout);
static char *convertcase(char *string);
static int PushNode(PV7LOOKUP pLookup, PINDEXNODE pIndexNode);
static int PopNode(PV7LOOKUP pLookup);
static int HexVal(char ch);
static int ReadV7PlusData(V7P_HEADER *pHeader, FILE *pfData, PNODEDATA pNodeData);
static void CopyConvert(char *pchDest, const char *pchSrc);

#ifndef STANDALONE
static int ReadNodeIndex(FILE *pfIndexFile, PNODEBROWSE pNodeBrowse);
static int CreateNodeIndex(PNODEBROWSE pNodeBrowse);
static int ReadNameIndex(FILE *pfIndexFile, PNAMEBROWSE pNameBrowse);
static int CreateNameIndex(PNAMEBROWSE pNameBrowse);
static int SortNodeIndex(PNODEBROWSE pNodeBrowse);
static int NodeCompare(const void *elem1, const void *elem2);
#endif

void QuickSort(void *, size_t, size_t,
                           int (*)(const void *, const void *));

#ifdef STANDALONE
static int M_ParseAddress(PCHAR pchAddress, FTNADDRESS *pNetAddr);
FTNADDRESS * StringToNetAddr(PCHAR buffer, FTNADDRESS *address, PCHAR Default);
#endif

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: FLTV7OpenSearch                                            º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Initialisiert das Handle zum Suchen in der Nodeliste        º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pHandle: Zeiger auf das Handle                                 º
 º            pchSearchName: Zu suchender Name                               º
 º            pchIndexFile: Filename des Sysop-Index-Files                   º
 º            pchDataFile:  Filename des Nodelist-Daten-Files                º
 º            ulSearchType: Art der Suche, siehe Header                      º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: siehe Header                                               º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges: Belegt Speicher fuer das Handle                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

ULONG FLTV7OpenSearch(HV7LOOKUP *pHandle, char *pchSearchName,
                      char *pchIndexFile, char *pchDataFile, ULONG ulSearchType)
{
   int iFile;
   char pchDTPFile[LEN_PATHNAME+1];
   char *pchDot;

   if (!(*pHandle = calloc(1, sizeof(V7LOOKUP))))
      return V7ERR_NOMEM;

   switch(ulSearchType)
   {
      case V7SEARCHTYPE_NAME:
      case V7SEARCHTYPE_ADDRESS:
#if 0
      case V7SEARCHTYPE_ADDRESS_JOKERS:
      case V7SEARCHTYPE_NAME_FUZZY:
#endif
         break;

      default:
         return V7ERR_INVSEARCHTYPE;
   }

   ((PV7LOOKUP)*pHandle)->ulSearchType = ulSearchType;

   /* Busy-File oeffnen */
   strcpy(pchDTPFile, pchDataFile);
   pchDot = strrchr(pchDTPFile, '.');
   if (pchDot)
   {
      strcpy(pchDot, ".BSY");
      ((PV7LOOKUP)*pHandle)->BusyFile = _sopen(pchDTPFile, O_CREAT|O_RDWR, SH_DENYWR, S_IREAD|S_IWRITE);
      if (((PV7LOOKUP)*pHandle)->BusyFile == -1)
         return V7ERR_DATOPENERR;
   }

   if ((iFile = _sopen(pchIndexFile, O_RDONLY|O_BINARY, SH_DENYWR, S_IREAD|S_IWRITE)) == -1)
      return V7ERR_IDXOPENERR;  /* Fehler beim Oeffnen */

   ((PV7LOOKUP)*pHandle)->streamIndex = _fdopen(iFile, "rb");

   if ((iFile = _sopen(pchDataFile, O_RDONLY|O_BINARY, SH_DENYWR, S_IREAD|S_IWRITE)) == -1)
      return V7ERR_DATOPENERR;  /* Fehler beim Oeffnen */

   ((PV7LOOKUP)*pHandle)->streamData = _fdopen(iFile, "rb");

   /* V7+ Datenfile oeffnen, Fehler dabei ignorieren (Kompatiblitaet) */
   strcpy(pchDTPFile, pchDataFile);
   pchDot = strrchr(pchDTPFile, '.');
   if (pchDot)
   {
      strcpy(pchDot, ".DTP");
      if ((iFile = _sopen(pchDTPFile, O_RDONLY|O_BINARY, SH_DENYWR, S_IREAD|S_IWRITE)) != -1)
      {
         ((PV7LOOKUP)*pHandle)->streamDTP = _fdopen(iFile, "rb");
         if (((PV7LOOKUP)*pHandle)->streamDTP)
         {
            if (fread(&((PV7LOOKUP)*pHandle)->V7PHeader, sizeof(V7P_HEADER), 1,
                      ((PV7LOOKUP)*pHandle)->streamDTP)<1)
            {
               fclose(((PV7LOOKUP)*pHandle)->streamDTP);
               ((PV7LOOKUP)*pHandle)->streamDTP = NULL;
            }

         }
      }
   }

   switch(ulSearchType)
   {
      case V7SEARCHTYPE_NAME:
         ((PV7LOOKUP)*pHandle)->CompareFunc=namecmp;
         strncpy(((PV7LOOKUP)*pHandle)->pchName, pchSearchName, LEN_USERNAME);
         ((PV7LOOKUP)*pHandle)->pCompElem = ((PV7LOOKUP)*pHandle)->pchName;
#if 0
      case V7SEARCHTYPE_ADDRESS_JOKERS:
      case V7SEARCHTYPE_NAME_FUZZY:
#endif
         break;

      case V7SEARCHTYPE_ADDRESS:
         ((PV7LOOKUP)*pHandle)->CompareFunc = addrcmp;
         ((PV7LOOKUP)*pHandle)->pCompElem = &((PV7LOOKUP)*pHandle)->Addr;
         /* Adresse umwandeln */
         StringToNetAddr(pchSearchName, (FTNADDRESS*)&((PV7LOOKUP)*pHandle)->Addr, NULL);
         break;

      default:
         return V7ERR_INVSEARCHTYPE;
   }

   return V7ERR_OK;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: FLTV7SearchNext                                            º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Sucht den naechsten passenden Eintrag im Index, liefert     º
 º               gefundene Daten zurueck                                     º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: hLookup: Such-Handle                                           º
 º            pNodeData: Zeiger auf gefundene Nodedaten                      º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: siehe Header                                               º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

ULONG FLTV7SearchNext(HV7LOOKUP hLookup, PNODEDATA pNodeData)
{
   LONG lRecNum=0;
   ULONG ulRange=0;
   ULONG ulFlags=0;
   PV7LOOKUP pLookup=(PV7LOOKUP) hLookup;

   /* Handle pruefen */
   ulRange=sizeof(V7LOOKUP);
   if (DosQueryMem(pLookup, &ulRange, &ulFlags))
      return V7ERR_INVHANDLE;

   if (ulRange < sizeof(V7LOOKUP))
      return V7ERR_INVHANDLE;

   if ((ulFlags & (PAG_COMMIT | PAG_READ | PAG_WRITE)) !=
       (PAG_COMMIT | PAG_READ | PAG_WRITE))
      return V7ERR_INVHANDLE;

   if (pLookup->bAtEnd)      /* Wir sind schon am Ende */
      return V7ERR_NOENTRY;

   if (!pLookup->bStarted)
   {
      /* Anfang der Suche, Wurzel holen */
      if (BTreeInit(pLookup))
         return V7ERR_IDXREADERR;
      /* Ersten Knoten Suchen */
      if (BTreeFindFirst(pLookup)== -1)
         return V7ERR_NOENTRY;
      /* Weitersiften bis zu den Blaettern */
      lRecNum=BTreeSiftDown(pLookup);
   }
   else
      do
      {
         lRecNum=BTreeTraverse(pLookup);
      } while(pLookup->lFoundRec == lRecNum); /* Doppelte Eintraege Uebergehen */


   if (lRecNum >= 0)
   {
      pLookup->lFoundRec=lRecNum;
      if (get_ver7_info(&pLookup->V7PHeader, pLookup->streamData, pLookup->streamDTP,
                        pLookup->lFoundRec, pNodeData))
         return V7ERR_DATREADERR;
   }
   else
   {
      pLookup->bAtEnd=TRUE;
      return V7ERR_NOENTRY;
   }

   return V7ERR_OK;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: FLTV7CloseSearch                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Beendet die Suche, gibt Handle frei                         º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: hLookup: Such-Handle                                           º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: siehe Header                                               º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/


ULONG FLTV7CloseSearch(HV7LOOKUP hLookup)
{
   ULONG ulRange=0;
   ULONG ulFlags=0;

   /* Handle pruefen */
   ulRange=sizeof(V7LOOKUP);
   if (DosQueryMem(hLookup, &ulRange, &ulFlags))
      return V7ERR_INVHANDLE;

   if (ulRange < sizeof(V7LOOKUP))
      return V7ERR_INVHANDLE;

   if ((ulFlags & (PAG_COMMIT | PAG_READ | PAG_WRITE)) !=
       (PAG_COMMIT | PAG_READ | PAG_WRITE))
      return V7ERR_INVHANDLE;

   PopNode(hLookup);

   fclose(((PV7LOOKUP)hLookup)->streamIndex);
   fclose(((PV7LOOKUP)hLookup)->streamData);
   if (((PV7LOOKUP)hLookup)->streamDTP)
      fclose(((PV7LOOKUP)hLookup)->streamDTP);
   close(((PV7LOOKUP)hLookup)->BusyFile);
   free(hLookup);

   return V7ERR_OK;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: BTreeInit                                                  º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Initialisiert das Handle fuer die Suche im B-Baum des       º
 º               Index                                                       º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pLookup: Such-Handle                                           º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: 0  OK                                                      º
 º                1  Fehler                                                  º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static LONG BTreeInit(PV7LOOKUP pLookup)
{
    INDEXNODE inode;
    ULONG ulParity=0;

    /* Control-Block lesen */
    if (get7node (pLookup->streamIndex, 0, &inode.node) != &inode.node)
       return 1;
    else
    {
       /* Indexfile-Prfung */

       ulParity= (ULONG) inode.node.CtlBlock.CtlBlkSize ^
                 (ULONG) inode.node.CtlBlock.CtlRoot ^
                 (ULONG) inode.node.CtlBlock.CtlHiBlk ^
                 (ULONG) inode.node.CtlBlock.CtlLoLeaf ^
                 (ULONG) inode.node.CtlBlock.CtlHiLeaf ^
                 (ULONG) inode.node.CtlBlock.CtlFree ^
                 (ULONG) inode.node.CtlBlock.CtlLvls;


       if ((USHORT)ulParity != inode.node.CtlBlock.CtlParity)
          return 1;

       pLookup->lRecNum = inode.node.CtlBlock.CtlRoot;
       pLookup->CtlBlkSize= inode.node.CtlBlock.CtlBlkSize;
       pLookup->bStarted=TRUE;

       /* ersten Indexknoten lesen */
       if (get7node(pLookup->streamIndex, pLookup->lRecNum * pLookup->CtlBlkSize,
                    &inode.node) != &inode.node)
          return 1;
       else
       {
          inode.iPos=0;
          inode.next=NULL;
          PushNode(pLookup, &inode);
          return 0;
       }
    }
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: BTreeFindFirst                                             º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Sucht den Baum ab, positioniert auf den ersten gefundenen   º
 º               Eintrag                                                     º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pLookup: Such-Handle                                           º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: -1     Fehler                                              º
 º                sonst  Record-Nummer im Datenfile                          º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static LONG BTreeFindFirst(PV7LOOKUP pLookup)
{
   char *pchItem;
   int iComp, iLen;

   /* erstmal den Indexknoten durchsuchen */
   while (pLookup->pNodeStack->node.INodeBlock.IndxFirst != -1)
   {
      if ((pLookup->pNodeStack->count = pLookup->pNodeStack->node.INodeBlock.IndxCnt) == 0)
         return -1;

      /* alle 20 Eintraege durchsuchen */
      for (; pLookup->pNodeStack->iPos < pLookup->pNodeStack->count; pLookup->pNodeStack->iPos++)
      {
         pchItem = (char*) &(pLookup->pNodeStack->node)+
                   pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxOfs;
         iLen = pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxLen;

         iComp = pLookup->CompareFunc(pchItem, pLookup->pCompElem, iLen);

         if (iComp > 0)   /* ueber's Ziel hinaus */
            break;

         if (iComp == 0)  /* gefunden! */
            return (pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxData);
      }

      if (pLookup->pNodeStack->iPos == 0)
         pLookup->lRecNum = pLookup->pNodeStack->node.INodeBlock.IndxFirst;
      else
         pLookup->lRecNum = pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos - 1].IndxPtr;

      if (get7node(pLookup->streamIndex, pLookup->lRecNum * pLookup->CtlBlkSize,
                   &(pLookup->pNodeStack->node)) != &(pLookup->pNodeStack->node))
         return -1;
      pLookup->pNodeStack->iPos = 0;
   }  /* while */


   /* bisher nix gefunden, jetzt muá der Eintrag im Leafnode sein */

   if ((pLookup->pNodeStack->count = pLookup->pNodeStack->node.LNodeBlock.IndxCnt) != 0)
   {
      /* nach hoeherem Schluessel suchen */
      for (; pLookup->pNodeStack->iPos < pLookup->pNodeStack->count; pLookup->pNodeStack->iPos++)
      {
         pchItem = (char*) &(pLookup->pNodeStack->node)+
                   pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyOfs;
         iLen = pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyLen;

         iComp = pLookup->CompareFunc(pchItem, pLookup->pCompElem, iLen);

         if (iComp > 0)
            break;

         if (iComp == 0)
            return pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyVal;
      }
   }

   return -1;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: BTreeSiftDown                                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Geht im Baum weiter nach unten, bis der echt erste Eintrag  º
 º               gefunden wurde. Danach liegen alle Nodes auf dem Nodestack, º
 º               der aktuelle Node oben.                                     º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pLookup: Such-Handle                                           º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: -1     Fehler                                              º
 º                sonst  Datensatznummer im Datenfile                        º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static LONG BTreeSiftDown(PV7LOOKUP pLookup)
{
   char *pchItem;
   int iComp, iLen;
   INDEXNODE inode;
   LONG lRecNum=0;

   inode.iPos=0;
   inode.count=0;

   /* nur Indexknoten durchsuchen */
   if (pLookup->pNodeStack->node.INodeBlock.IndxFirst != -1)
   {
      if (pLookup->pNodeStack->iPos == 0)
         lRecNum = pLookup->pNodeStack->node.INodeBlock.IndxFirst;
      else
         lRecNum = pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos - 1].IndxPtr;

      if (get7node(pLookup->streamIndex, lRecNum * pLookup->CtlBlkSize,
          &inode.node) != &inode.node)
         return -1;

      if (inode.node.INodeBlock.IndxFirst != -1)  /* Baumknoten */
      {
         inode.count=inode.node.INodeBlock.IndxCnt;
         for (inode.iPos=0; inode.iPos < inode.count; inode.iPos++)
         {
            pchItem = (char*) &(inode.node)+
                      inode.node.INodeBlock.IndxRef[inode.iPos].IndxOfs;
            iLen = inode.node.INodeBlock.IndxRef[inode.iPos].IndxLen;

            iComp = pLookup->CompareFunc(pchItem, pLookup->pCompElem, iLen);

            if (iComp > 0)   /* ueber's Ziel hinaus */
            {
               if ((inode.iPos>0)?(inode.node.INodeBlock.IndxRef[inode.iPos-1].IndxPtr):(inode.node.INodeBlock.IndxFirst))
               {
                  pLookup->lRecNum=lRecNum;
                  PushNode(pLookup, &inode);
                  BTreeSiftDown(pLookup);
                  break;
               }
               else
                  break;
            }

            if (iComp == 0)  /* gefunden! */
            {
                pLookup->lRecNum=lRecNum;
                PushNode(pLookup, &inode);
                BTreeSiftDown(pLookup);
            }
         }
         if (inode.iPos >= inode.count) /* Ende des Knotens, rechts absteigen */
         {
             pLookup->lRecNum = pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxPtr;
             PushNode(pLookup, &inode);
             BTreeSiftDown(pLookup);
         }
      }
      else
      {
         /* Blattknoten */
         inode.count=inode.node.LNodeBlock.IndxCnt;
         for (inode.iPos=0; inode.iPos < inode.count; inode.iPos++)
         {
            pchItem = (char*) &(inode.node)+
                      inode.node.LNodeBlock.LeafRef[inode.iPos].KeyOfs;
            iLen = inode.node.LNodeBlock.LeafRef[inode.iPos].KeyLen;

            iComp = pLookup->CompareFunc(pchItem, pLookup->pCompElem, iLen);

            if (iComp > 0)   /* ueber's Ziel hinaus */
               break;

            if (iComp == 0)  /* gefunden! */
            {
                PushNode(pLookup, &inode);
                break;    /* Pushen, aber abbrechen */
            }
         }
      }
   }
   else
      return pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyVal;

   if (pLookup->pNodeStack->node.INodeBlock.IndxFirst != -1)  /* Baumknoten */
      return pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxData;
   else
      return pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyVal;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: BTreeTraverse                                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Geht einen Knoten durch                                     º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pLookup: Such-Handle                                           º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: -1     Fehler                                              º
 º                sonst  Record-Nummer im Datenfile                          º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static LONG BTreeTraverse(PV7LOOKUP pLookup)
{
   char *pchItem;
   int iComp, iLen;
   LONG lRecNum=0;
   BOOL samelevel=TRUE;

   /* Naechsten Key */
   pLookup->pNodeStack->iPos++;
   while (pLookup->pNodeStack->iPos >= pLookup->pNodeStack->count)
   {
      /* Am Ende des Blocks */
      if ((pLookup->pNodeStack->node.INodeBlock.IndxFirst != -1))
      {
         /* Baumknoten -> wieder nach unten */
         if ((lRecNum=BTreeSiftDown(pLookup)) != -1)
             return lRecNum;
         else
         {
            /* rechts unten nix gefunden, wieder rauf */
            if (pLookup->stacksize <= 1)
               return -1;
            else
            {
               while (pLookup->pNodeStack->iPos >= pLookup->pNodeStack->count)
                 PopNode(pLookup);
               samelevel=FALSE;
            }
         }
      }
      else
      {
         /* Blattknoten -> zurueck */
         if (pLookup->stacksize <= 1)
            return -1;
         else
         {
            while (pLookup->pNodeStack->iPos >= pLookup->pNodeStack->count)
              PopNode(pLookup);
            samelevel=FALSE;
         }
      }
   }

   if (samelevel) /* Wenn nur auf der gleichen Ebene weiter, versuchen zu siften */
      BTreeSiftDown(pLookup);

   if (pLookup->pNodeStack->node.INodeBlock.IndxFirst != -1)
   {
      for (; pLookup->pNodeStack->iPos < pLookup->pNodeStack->count; pLookup->pNodeStack->iPos++)
      {
         pchItem = (char*) &(pLookup->pNodeStack->node)+
                   pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxOfs;
         iLen = pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxLen;

         iComp = pLookup->CompareFunc(pchItem, pLookup->pCompElem, iLen);

         if (iComp > 0)   /* ueber's Ziel hinaus */
            break;

         if (iComp == 0)  /* gefunden! */
            return pLookup->pNodeStack->node.INodeBlock.IndxRef[pLookup->pNodeStack->iPos].IndxData;
      }
   }
   else
   {
      /* Blattknoten */
      for (; pLookup->pNodeStack->iPos < pLookup->pNodeStack->count; pLookup->pNodeStack->iPos++)
      {
         pchItem = (char*) &(pLookup->pNodeStack->node)+
                   pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyOfs;
         iLen = pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyLen;

         iComp = pLookup->CompareFunc(pchItem, pLookup->pCompElem, iLen);

         if (iComp > 0)   /* ueber's Ziel hinaus */
            break;

         if (iComp == 0)  /* gefunden! */
            return pLookup->pNodeStack->node.LNodeBlock.LeafRef[pLookup->pNodeStack->iPos].KeyVal;
      }
   }
   return -1;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: get7node                                                   º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung:  Liesst einen Knoten im Indexfile ein                       º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: stream: Stream-Handle der Index-Datei                          º
 º            lPos:   Offset in der Index-Datei                              º
 º            pNdx:   Zeiger auf Index-Knoten                                º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: Zeiger auf Index-Knoten                                    º
 º                NULL  Fehler                                               º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static PNDX get7node(FILE *stream, LONG lPos, PNDX pNdx)
{
    fseek (stream, lPos, SEEK_SET);

    if (fread (pNdx, sizeof(NDX), 1, stream) < 1)
        return NULL;

    return pNdx;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: get_ver7_info                                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Holt die Nodedaten aus dem Datenfile                        º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pLookup: Such-Handle                                           º
 º            pNodeData: Zeiger auf Node-Daten                               º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: 0  OK                                                      º
 º                1  Fehler                                                  º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static int get_ver7_info (V7P_HEADER *pHeader, FILE *pfData, FILE *pfDataDTP, LONG lOffset, PNODEDATA pNodeData)
{
    VERS7 vers7;
    char unpackline[LEN_USERNAME+LEN_SYSTEMNAME+LEN_LOCATION+1+10];
    char *packline=NULL;
    char *stringptr;
    ULONG ulDTPOffset=0;
    int digit=0;

    if (fseek (pfData, lOffset, SEEK_SET))
        return 1;

    if (fread (&vers7, sizeof(VERS7), 1, pfData) < 1)
        return 1;

    /* Adresse */
    pNodeData->Address.usZone = vers7.Zone;
    pNodeData->Address.usNet  = vers7.Net;
    pNodeData->Address.usNode = vers7.Node;

    if (vers7.NodeFlags & B_point)
       pNodeData->Address.usPoint = vers7.HubNode;
    else
       pNodeData->Address.usPoint = 0;

    /* Namen entpacken */
    memset(pNodeData->PhoneNr, 0, LEN_PHONENR+1);
    fread (pNodeData->PhoneNr, vers7.Phone_len, 1, pfData);

    memset(pNodeData->Password, 0, LEN_PASSWORD+1);
    fread (pNodeData->Password, vers7.Password_len, 1, pfData);

    memset(unpackline, 0, sizeof(unpackline));

    packline=malloc(vers7.pack_len+1);
    memset(packline, 0, vers7.pack_len);

    fread (packline, vers7.pack_len, 1, pfData);

    unpk(packline, unpackline, vers7.pack_len, sizeof(unpackline));

    free(packline);

    memset(pNodeData->SystemName, 0, LEN_SYSTEMNAME+1);
    memset(pNodeData->Location, 0, LEN_LOCATION+1);
    memset(pNodeData->SysopName, 0, LEN_USERNAME+1);

    memcpy(pNodeData->SystemName, unpackline, min(LEN_SYSTEMNAME, vers7.Bname_len));
    convertcase(pNodeData->SystemName);

    stringptr= unpackline + vers7.Bname_len;
    memcpy(pNodeData->SysopName, stringptr, min(LEN_USERNAME, vers7.Sname_len));
    convertcase(pNodeData->SysopName);

    stringptr= unpackline + vers7.Bname_len + vers7.Sname_len;
    memcpy(pNodeData->Location, stringptr, min(LEN_LOCATION, vers7.Cname_len));
    convertcase(pNodeData->Location);

    /* Daten */
    pNodeData->ModemType= (ULONG) vers7.ModemType;
    pNodeData->BaudRate=  (ULONG) vers7.BaudRate * 300;
    pNodeData->UserCost=  (ULONG) vers7.MsgFee;
    pNodeData->CallCost=  (ULONG) vers7.CallCost;

    /* Flags */
    pNodeData->isZC=   (vers7.NodeFlags & B_zone);
    pNodeData->isRC=   (vers7.NodeFlags & B_region);
    pNodeData->isCM=   (vers7.NodeFlags & B_CM);
    pNodeData->isHost= (vers7.NodeFlags & B_host);
    pNodeData->isHub=  (vers7.NodeFlags & B_hub);
    pNodeData->isMO=   (vers7.NodeFlags & B_MO);

    if (pfDataDTP)
    {
       /* V7+ */
       stringptr= unpackline + vers7.Bname_len + vers7.Sname_len + vers7.Cname_len;

       while (isxdigit(*stringptr) && digit < 8)
       {
          ulDTPOffset = (ulDTPOffset << 4) | HexVal(*stringptr);

          stringptr++;
          digit++;
       }

       if (digit == 8) /* 8 hexzeichen gelesen */
       {
          if (fseek (pfDataDTP, ulDTPOffset, SEEK_SET))
              return 0; /* return without error */

          ReadV7PlusData(pHeader, pfDataDTP, pNodeData);
       }
    }

    return 0;
}

static int HexVal(char ch)
{
   if (ch >= '0' && ch <= '9')
      return ch-'0';
   else
      if (ch >= 'a' && ch <= 'f')
         return ch-'a'+10;
      else
         if (ch >= 'A' && ch <= 'F')
            return ch-'A'+10;
         else
            return -1;
}

static int ReadV7PlusData(V7P_HEADER *pHeader, FILE *pfData, PNODEDATA pNodeData)
{
   USHORT usFieldLen=0xffff;
   char *pchFieldData;
   char *pchField;
   char *pchNext;
   int FieldNum=0;

   /* Links ueberspringen */
   if (pNodeData->Address.usPoint > 0)
      fseek(pfData, pHeader->AllFixSize, SEEK_CUR);
   else
      fseek(pfData, pHeader->AllFixSize+pHeader->AddFixSize, SEEK_CUR);

   /* rohen Eintrag lesen */

   if (fread(&usFieldLen, sizeof(usFieldLen), 1, pfData)==1)
   {
      if (usFieldLen != 0xffff)
      {
         pchFieldData = malloc(usFieldLen);
         if (pchFieldData)
         {
            if (fread(pchFieldData, 1, usFieldLen, pfData)==usFieldLen)
            {
               /* Nodelist-Zeile parsen */
               pchField = pchFieldData;
               pchNext = pchField;
               while (*pchNext && *pchNext != ',')
                  pchNext++;
               if (*pchNext)
               {
                  *pchNext = 0;
                  pchNext++;
               }
               if (!*pchNext)
                  pchNext=NULL;

               while (pchField && FieldNum <=6)
               {
                  switch(FieldNum)
                  {
                     case 2: /* Node-Name */
                        CopyConvert(pNodeData->SystemName, pchField);
                        break;

                     case 3: /* Ort */
                        CopyConvert(pNodeData->Location, pchField);
                        break;

                     case 4: /* Sysop */
                        CopyConvert(pNodeData->SysopName, pchField);
                        break;

                     case 6: /* Baudrate */
                        pNodeData->BaudRate = strtoul(pchField, NULL, 10);
                        break;
                  }
                  FieldNum++;
                  pchField = pchNext;
                  if (pchNext)
                  {
                     pchNext++;
                     while (*pchNext && *pchNext != ',')
                        pchNext++;
                     if (*pchNext)
                     {
                        *pchNext = 0;
                        pchNext++;
                     }
                     if (!*pchNext)
                        pchNext=NULL;
                  }
               }
            }

            free(pchFieldData);
            return 0;
         }
         else
            return 2;
      }
      else
         return 0;
   }
   else
      return 1;
}

static void CopyConvert(char *pchDest, const char *pchSrc)
{
   while (*pchSrc)
   {
      if (*pchSrc == '_')
         *pchDest = ' ';
      else
         *pchDest = *pchSrc;
      pchDest++;
      pchSrc++;
   }
   *pchDest = 0;

   return;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: unpk                                                       º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Entpackt einen Base-40-String in einen normalen String      º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: instr: gepackter String                                        º
 º            outp:  entpackter String                                       º
 º            countin: Anzahl der gepackten Bytes                            º
 º            countout: Puffergroesse f. entpackten String                   º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: keine                                                      º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static void unpk(char *instr, char *outp, int countin, int countout)
{
    _Packed struct chars {
           unsigned char c1;
           unsigned char c2;
           };

    union {
          unsigned short w1;
          _Packed struct chars d;
          } u;

   int i, j, k;
   char obuf[3];
   char *outtemp=outp;

   if (countout == 0)
      return;

   *outtemp = '\0';

   if (countout == 1)
      return;

   while (countin && ((outtemp-outp) < (countout -1)))
   {
      u.d.c1 = *instr++;
      u.d.c2 = *instr++;
      countin -= 2;
      for(j=2;j>=0;j--)
      {
         i = u.w1 % 40;
         u.w1 /= 40;
         obuf[j] = unwrk[i];
      }
      k=0;
      while(((outtemp-outp) < (countout -1)) && (k<3))
         *outtemp++=obuf[k++];
      *outtemp='\0';
   }
   return;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: convertcase                                                º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Konvertiert einen Uppercase-String in einen gemischten      º
 º               String                                                      º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: string: zu konvertierender String                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: Pointer auf den String                                     º
 º                                                                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static char *convertcase(unsigned char *string)
{
   BOOL bFirstChar= TRUE;
   unsigned char *s;

   s = string;

   while (*string)
   {
      if (is_alpha(*string))
      {
         if (!bFirstChar)                /* Kein Wortanfang ? */
            *string = (*string) | 0x20;
         else
            bFirstChar= FALSE;
      }
      else
         bFirstChar= TRUE;
      string++;
   }

   return s;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: PushNode                                                   º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Legt einen Index-Node auf den Stack                         º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pLookup: Lookup-Handle;                                        º
 º            pIndexNode: zu Stackender Index-Node                           º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: 0 OK                                                       º
 º                1 Out of memory                                            º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static int PushNode(PV7LOOKUP pLookup, PINDEXNODE pIndexNode)
{
   PINDEXNODE pNewNode;

   if (DosAllocMem((PVOID)&pNewNode, sizeof(INDEXNODE),
                   PAG_COMMIT | PAG_READ | PAG_WRITE))
      return 1;

   memcpy(pNewNode, pIndexNode, sizeof(INDEXNODE));
   pNewNode->next= pLookup->pNodeStack;
   pLookup->pNodeStack=pNewNode;
   pLookup->stacksize++;

   return 0;
}

/*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º Funktionsname: PopNode                                                    º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Beschreibung: Holt einen Index-Node vom Stack                             º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Parameter: pLookup: Lookup-Handle;                                        º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Rckgabewerte: 0 OK                                                       º
 º                1 Stack empty                                              º
 ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
 º Sonstiges:                                                                º
 º                                                                           º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

static int PopNode(PV7LOOKUP pLookup)
{
   PINDEXNODE pOldNode;

   if (pLookup->stacksize == 0 ||
       pLookup->pNodeStack == NULL)
      return 1;

   pLookup->stacksize--;
   pOldNode=pLookup->pNodeStack;
   pLookup->pNodeStack= pOldNode->next;
   DosFreeMem(pOldNode);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: addrcmp                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Vergleicht zwei Adressen                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: addr1: erste Adresse                                           */
/*            addr2: zweite Adresse                                          */
/*            n: Datenlaenge                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int addrcmp(void *addr1, void *addr2, unsigned int n)
{
   int k;
   USHORT Point;

   k = ((PFTNADDRESS)addr1)->usZone - ((PFTNADDRESS)addr2)->usZone;
   if (k)
      return k;

   k = ((PFTNADDRESS)addr1)->usNet  - ((PFTNADDRESS)addr2)->usNet;
   if (k)
      return k;

   k = ((PFTNADDRESS)addr1)->usNode - ((PFTNADDRESS)addr2)->usNode;
   if (k)
      return k;

   if (n == 6)
      Point = 0;
   else
      Point = ((PFTNADDRESS)addr1)->usPoint;

   return Point - ((PFTNADDRESS)addr2)->usPoint;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: namecmp                                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Vergleicht zwei Namen                                       */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchName1: erster Name                                          */
/*            pchName2: zweiter Name                                         */
/*            n: Datenlaenge                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte:                                                            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int namecmp(void *pchName1, void *pchName2, unsigned int n)
{
   int len=n;
   int nlen;

   if ((nlen = strlen(pchName2)) < n)
      len = nlen;

   return strnicmp((char *) pchName1, (char *) pchName2, len);
}

#ifndef STANDALONE

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTV7OpenNodeBrowse                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Bereitet zum Browsen nach Nodenummern vor                   */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchIndexFile: Dateiname der Node-Index-Datei                   */
/*            pchDataFile:  Dateiname der Daten-Datei                        */
/*            pNodeBrowse:  Zeiger auf Browser-Struktur                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: V7ERR_*                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int FLTV7OpenNodeBrowse(char *pchIndexFile, char *pchDataFile,
                                 PNODEBROWSE pNodeBrowse)
{
   int iFile;
   FILE *pfIndexFile;
   char pchDTPFile[LEN_PATHNAME+1];
   char *pchDot;

   memset(pNodeBrowse, 0, sizeof(NODEBROWSE));

   /* Busy-File oeffnen */
   strcpy(pchDTPFile, pchDataFile);
   pchDot = strrchr(pchDTPFile, '.');
   if (pchDot)
   {
      strcpy(pchDot, ".BSY");
      pNodeBrowse->BusyFile = _sopen(pchDTPFile, O_CREAT|O_RDWR, SH_DENYWR, S_IREAD|S_IWRITE);
      if (pNodeBrowse->BusyFile == -1)
         return V7ERR_DATOPENERR;
   }

   iFile= _sopen(pchIndexFile, O_RDONLY | O_BINARY, SH_DENYWR, 0);

   if (iFile == -1)
      return V7ERR_IDXOPENERR;

   pfIndexFile = _fdopen(iFile, "rb");

   if (!pfIndexFile)
   {
      _close(iFile);
      return V7ERR_IDXOPENERR;
   }


   /* Index einlesen */
   if (ReadNodeIndex(pfIndexFile, pNodeBrowse))
   {
      fclose(pfIndexFile);
      return V7ERR_IDXREADERR;
   }
   SortNodeIndex(pNodeBrowse);

   CreateNodeIndex(pNodeBrowse);

   fclose(pfIndexFile);

   iFile= _sopen(pchDataFile, O_RDONLY | O_BINARY, SH_DENYWR, 0);

   if (iFile == -1)
   {
      FLTV7CloseNodeBrowse(pNodeBrowse);
      return V7ERR_DATOPENERR;
   }

   pNodeBrowse->pfDataFile = _fdopen(iFile, "rb");

   if (!pNodeBrowse->pfDataFile)
   {
      _close(iFile);
      FLTV7CloseNodeBrowse(pNodeBrowse);
      return V7ERR_DATOPENERR;
   }

   /* V7+ */
   strcpy(pchDTPFile, pchDataFile);
   pchDot = strrchr(pchDTPFile, '.');
   if (pchDot)
   {
      strcpy(pchDot, ".DTP");
      iFile= _sopen(pchDTPFile, O_RDONLY | O_BINARY, SH_DENYWR, 0);

      if (iFile != -1)
         if (pNodeBrowse->pfDataDTP = _fdopen(iFile, "rb"))
         {
            fread(&pNodeBrowse->V7PHeader, sizeof(V7P_HEADER), 1, pNodeBrowse->pfDataDTP);
         }
   }

   return V7ERR_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadNodeIndex                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liesst den Nodelist-Index ein                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pfIndexFile:  Index-File                                       */
/*            pNodeBrowse:  Zeiger auf Browser-Struktur                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: 0  OK                                                      */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int ReadNodeIndex(FILE *pfIndexFile, PNODEBROWSE pNodeBrowse)
{
   NDX Ndx;
   ULONG ulParity=0;
   LONG lBlock;
   LONG lBlockSize;
   int i;
   PFTNADDRESS pAddr;
   ULONG ulNumAlloc=0;

   /* Control-Block lesen */
   if (get7node(pfIndexFile, 0, &Ndx) != &Ndx)
      return -1;

   /* Indexfile-Prfung */
   ulParity= (ULONG) Ndx.CtlBlock.CtlBlkSize ^
             (ULONG) Ndx.CtlBlock.CtlRoot ^
             (ULONG) Ndx.CtlBlock.CtlHiBlk ^
             (ULONG) Ndx.CtlBlock.CtlLoLeaf ^
             (ULONG) Ndx.CtlBlock.CtlHiLeaf ^
             (ULONG) Ndx.CtlBlock.CtlFree ^
             (ULONG) Ndx.CtlBlock.CtlLvls;

   if ((USHORT)ulParity != Ndx.CtlBlock.CtlParity)
      return -1;

   /* Anzahl schaetzen */
   ulNumAlloc = (Ndx.CtlBlock.CtlHiLeaf - Ndx.CtlBlock.CtlLoLeaf)*35;
   pNodeBrowse->pNodeIndex=malloc(ulNumAlloc * sizeof(NODEINDEX));

   /* ersten Leaf-Node lesen */
   lBlock=Ndx.CtlBlock.CtlLoLeaf;
   lBlockSize=Ndx.CtlBlock.CtlBlkSize;
   while (lBlock)
   {
      /* Block einlesen */
      get7node(pfIndexFile, lBlock * lBlockSize, &Ndx);

      /* Speicher holen */
      if (pNodeBrowse->ulNumNodes + Ndx.LNodeBlock.IndxCnt > ulNumAlloc)
      {
         PNODEINDEX pNewIndex;

         pNewIndex = realloc(pNodeBrowse->pNodeIndex,
                             (ulNumAlloc + BROWSEDELTA)*sizeof(NODEINDEX));
         if (!pNewIndex)
         {
            /* realloc fehlgeschlagen, malloc */
            pNewIndex = malloc((ulNumAlloc + BROWSEDELTA)* sizeof(NODEINDEX));
            if (pNodeBrowse->pNodeIndex)
            {
               memcpy(pNewIndex, pNodeBrowse->pNodeIndex, pNodeBrowse->ulNumNodes * sizeof(NODEINDEX));
               free(pNodeBrowse->pNodeIndex);
            }
         }
         pNodeBrowse->pNodeIndex = pNewIndex;

         ulNumAlloc += BROWSEDELTA;
      }

      /* Nodes extrahieren */
      for (i=0; i < Ndx.LNodeBlock.IndxCnt; i++)
      {
         PFTNADDRESS pDestAddr;

         /* Adresse */
         pAddr = (PFTNADDRESS) (((PCHAR)&Ndx) + Ndx.LNodeBlock.LeafRef[i].KeyOfs);
         pDestAddr = &pNodeBrowse->pNodeIndex[pNodeBrowse->ulNumNodes].NodeAddr;

         pDestAddr->usZone = pAddr->usZone;
         pDestAddr->usNet  = pAddr->usNet;
         pDestAddr->usNode = pAddr->usNode;
         if (Ndx.LNodeBlock.LeafRef[i].KeyLen == 6)
         {
            /* Node */
            pDestAddr->usPoint = 0;
         }
         else
         {
            /* Point */
            pDestAddr->usPoint = pAddr->usPoint;
         }

         /* Daten-Zeiger */
         pNodeBrowse->pNodeIndex[pNodeBrowse->ulNumNodes].lDataOffs = Ndx.LNodeBlock.LeafRef[i].KeyVal;

         pNodeBrowse->ulNumNodes++;
      }

      lBlock = Ndx.LNodeBlock.IndxFLink;
   }

   return 0;
}

static int SortNodeIndex(PNODEBROWSE pNodeBrowse)
{
   int i;
   PNODEINDEX pIndex = pNodeBrowse->pNodeIndex;

   for (i=0; i<pNodeBrowse->ulNumNodes; i++, pIndex++)
   {
      if (pIndex->NodeAddr.usNet < 1000)
      {
         if (pIndex->NodeAddr.usNet < 100)
         {
            if (pIndex->NodeAddr.usNet < 10)
               pIndex->usNetComp = pIndex->NodeAddr.usNet * 100000000 + i;
            else
               pIndex->usNetComp = pIndex->NodeAddr.usNet * 10000000 + i;
         }
         else
            pIndex->usNetComp = pIndex->NodeAddr.usNet * 1000000 + i;
      }
      else
         pIndex->usNetComp = pIndex->NodeAddr.usNet * 100000 + i;
   }

   QuickSort(pNodeBrowse->pNodeIndex, pNodeBrowse->ulNumNodes, sizeof(NODEINDEX), NodeCompare);

   return 0;
}

static int NodeCompare(const void *elem1, const void *elem2)
{
   PFTNADDRESS pAd1 = (PFTNADDRESS) elem1;
   PFTNADDRESS pAd2 = (PFTNADDRESS) elem2;

   if (pAd1->usZone < pAd2->usZone)
      return -1;
   else
      if (pAd1->usZone > pAd2->usZone)
         return 1;
      else
      {
         if (pAd1->usNet == pAd2->usNet)
         {
            if (pAd1->usNode < pAd2->usNode)
               return -1;
            else
               if (pAd1->usNode > pAd2->usNode)
                  return 1;
               else
               {
                  if (pAd1->usPoint < pAd2->usPoint)
                     return -1;
                  else
                     if (pAd1->usPoint > pAd2->usPoint)
                        return 1;
                     else
                        return 0;
               }
         }
         else
            if (((PNODEINDEX)pAd1)->usNetComp < ((PNODEINDEX)pAd2)->usNetComp)
               return -1;
            else
               return 1; /* Fall "gleich" bereits oben abgefangen */
      }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CreateNodeIndex                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Indexiert den Nodelist-Index nochmal                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pNodeBrowse:  Zeiger auf Browser-Struktur                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: 0  OK                                                      */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int CreateNodeIndex(PNODEBROWSE pNodeBrowse)
{
   USHORT usPrevZone=0;
   USHORT usPrevNet=0;
   BOOL bFirstZone=TRUE;
   PZONEINDEX pZoneIndex=NULL;
   PNETINDEX pNetIndex=NULL;
   int i;

   for (i=0; i<pNodeBrowse->ulNumNodes; i++)
   {
      if (pNodeBrowse->pNodeIndex[i].NodeAddr.usZone != usPrevZone ||
          bFirstZone)
      {
         bFirstZone=FALSE;

         /* Neue Zone */
         if (!pNodeBrowse->pZoneIndex)
         {
            pNodeBrowse->pZoneIndex=calloc(1, sizeof(ZONEINDEX));
            pZoneIndex=pNodeBrowse->pZoneIndex;
         }
         else
         {
            pZoneIndex->next=calloc(1, sizeof(ZONEINDEX));
            pZoneIndex = pZoneIndex->next;
         }
         usPrevZone = pZoneIndex->usZone = pNodeBrowse->pNodeIndex[i].NodeAddr.usZone;
         pZoneIndex->pStart = &(pNodeBrowse->pNodeIndex[i]);

         /* Erstes Netz gleich mit anlegen */
         pNetIndex = pZoneIndex->pNets = calloc(1, sizeof(NETINDEX));
         usPrevNet = pZoneIndex->pNets->usNet = pNodeBrowse->pNodeIndex[i].NodeAddr.usNet;
         pZoneIndex->pNets->pStart = pZoneIndex->pStart;
      }

      if (pNodeBrowse->pNodeIndex[i].NodeAddr.usNet != usPrevNet)
      {
         /* Neues Netz */
         pNetIndex->next= calloc(1, sizeof(NETINDEX));
         pNetIndex = pNetIndex->next;
         usPrevNet = pNetIndex->usNet = pNodeBrowse->pNodeIndex[i].NodeAddr.usNet;
         pNetIndex->pStart = &(pNodeBrowse->pNodeIndex[i]);
      }
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTV7CloseNodeBrowse                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Beendet das Nodelist-Browsing                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pNodeBrowse:  Zeiger auf Browser-Struktur                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: V7ERR_*                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int FLTV7CloseNodeBrowse(PNODEBROWSE pNodeBrowse)
{
   /* Data-File schliessen */
   if (pNodeBrowse->pfDataFile)
   {
      fclose(pNodeBrowse->pfDataFile);
      pNodeBrowse->pfDataFile=NULL;
   }

   /* V7+-File */
   if (pNodeBrowse->pfDataDTP)
   {
      fclose(pNodeBrowse->pfDataDTP);
      pNodeBrowse->pfDataDTP=NULL;
   }

   /* Array freigeben */
   if (pNodeBrowse->pNodeIndex)
   {
      free(pNodeBrowse->pNodeIndex);
      pNodeBrowse->pNodeIndex=NULL;
   }

   /* Indexierung freigeben */
   while (pNodeBrowse->pZoneIndex)
   {
      PZONEINDEX pZone=pNodeBrowse->pZoneIndex;

      while(pNodeBrowse->pZoneIndex->pNets)
      {
         PNETINDEX pNet=pNodeBrowse->pZoneIndex->pNets;

         pNodeBrowse->pZoneIndex->pNets = pNodeBrowse->pZoneIndex->pNets->next;
         free(pNet);
      }
      pNodeBrowse->pZoneIndex = pNodeBrowse->pZoneIndex->next;
      free(pZone);
   }

   close(pNodeBrowse->BusyFile);

   return V7ERR_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTV7ReadNodeData                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Node-Daten zu einem Node                           */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pNodeBrowse:  Zeiger auf Browser-Struktur                      */
/*            pNodeIndex:   Node, zu dem die Daten geholt werden sollen      */
/*            pNodeData:    Puffer f. Node-Daten                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: V7ERR_*                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/


int FLTV7ReadNodeData(PNODEBROWSE pNodeBrowse, PNODEINDEX pNodeIndex, PNODEDATA pNodeData)
{
   if (get_ver7_info(&pNodeBrowse->V7PHeader, pNodeBrowse->pfDataFile, pNodeBrowse->pfDataDTP, pNodeIndex->lDataOffs, pNodeData))
      return V7ERR_DATREADERR;
   else
      return V7ERR_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTV7OpenNameBrowse                                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Bereitet das Browsen nach Namen vor                         */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchIndexFile: Dateiname des Index-Files                        */
/*            pchDataFile:  Dateiname des Daten-Files                        */
/*            pNameBrowse:  Browser-Struktur                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: V7ERR_*                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int FLTV7OpenNameBrowse(char *pchIndexFile, char *pchDataFile,
                                 PNAMEBROWSE pNameBrowse)
{
   int iFile;
   FILE *pfIndexFile;
   char pchDTPFile[LEN_PATHNAME+1];
   char *pchDot;

   memset(pNameBrowse, 0, sizeof(NAMEBROWSE));

   /* Busy-File oeffnen */
   strcpy(pchDTPFile, pchDataFile);
   pchDot = strrchr(pchDTPFile, '.');
   if (pchDot)
   {
      strcpy(pchDot, ".BSY");
      pNameBrowse->BusyFile = _sopen(pchDTPFile, O_CREAT|O_RDWR, SH_DENYWR, S_IREAD|S_IWRITE);
      if (pNameBrowse->BusyFile == -1)
         return V7ERR_DATOPENERR;
   }

   iFile= _sopen(pchIndexFile, O_RDONLY | O_BINARY, SH_DENYWR, 0);

   if (iFile == -1)
      return V7ERR_IDXOPENERR;

   pfIndexFile = _fdopen(iFile, "rb");

   if (!pfIndexFile)
   {
      _close(iFile);
      return V7ERR_IDXOPENERR;
   }

   /* Index einlesen */
   if (ReadNameIndex(pfIndexFile, pNameBrowse))
   {
      fclose(pfIndexFile);
      return V7ERR_IDXREADERR;
   }

   CreateNameIndex(pNameBrowse);

   fclose(pfIndexFile);

   iFile= _sopen(pchDataFile, O_RDONLY | O_BINARY, SH_DENYWR, 0);

   if (iFile == -1)
   {
      FLTV7CloseNameBrowse(pNameBrowse);
      return V7ERR_DATOPENERR;
   }

   pNameBrowse->pfDataFile = _fdopen(iFile, "rb");

   if (!pNameBrowse->pfDataFile)
   {
      _close(iFile);
      FLTV7CloseNameBrowse(pNameBrowse);
      return V7ERR_DATOPENERR;
   }

   /* V7+ */
   strcpy(pchDTPFile, pchDataFile);
   pchDot = strrchr(pchDTPFile, '.');
   if (pchDot)
   {
      strcpy(pchDot, ".DTP");
      iFile= _sopen(pchDTPFile, O_RDONLY | O_BINARY, SH_DENYWR, 0);

      if (iFile != -1)
         if (pNameBrowse->pfDataDTP = _fdopen(iFile, "rb"))
         {
            fread(&pNameBrowse->V7PHeader, sizeof(V7P_HEADER), 1, pNameBrowse->pfDataDTP);
         }
   }

   return V7ERR_OK;
}

static int ReadNameIndex(FILE *pfIndexFile, PNAMEBROWSE pNameBrowse)
{
   NDX Ndx;
   ULONG ulParity=0;
   LONG lBlock;
   LONG lBlockSize;
   int i;
   PCHAR pchName;
   ULONG ulNumAlloc=0;

   /* Control-Block lesen */
   if (get7node(pfIndexFile, 0, &Ndx) != &Ndx)
      return -1;

   /* Indexfile-Prfung */
   ulParity= (ULONG) Ndx.CtlBlock.CtlBlkSize ^
             (ULONG) Ndx.CtlBlock.CtlRoot ^
             (ULONG) Ndx.CtlBlock.CtlHiBlk ^
             (ULONG) Ndx.CtlBlock.CtlLoLeaf ^
             (ULONG) Ndx.CtlBlock.CtlHiLeaf ^
             (ULONG) Ndx.CtlBlock.CtlFree ^
             (ULONG) Ndx.CtlBlock.CtlLvls;

   if ((USHORT)ulParity != Ndx.CtlBlock.CtlParity)
      return -1;

   /* Anzahl schaetzen */
   ulNumAlloc = (Ndx.CtlBlock.CtlHiLeaf - Ndx.CtlBlock.CtlLoLeaf)*35;
   pNameBrowse->pNameIndex=malloc(ulNumAlloc * sizeof(NAMEINDEX));

   /* ersten Leaf-Node lesen */
   lBlock=Ndx.CtlBlock.CtlLoLeaf;
   lBlockSize=Ndx.CtlBlock.CtlBlkSize;
   while (lBlock)
   {
      /* Block einlesen */
      get7node(pfIndexFile, lBlock * lBlockSize, &Ndx);

      /* Speicher holen */
      if (pNameBrowse->ulNumNames + Ndx.LNodeBlock.IndxCnt > ulNumAlloc)
      {
         PNAMEINDEX pNewIndex;

         pNewIndex = realloc(pNameBrowse->pNameIndex,
                             (ulNumAlloc + BROWSEDELTA)*sizeof(NAMEINDEX));
         if (!pNewIndex)
         {
            /* realloc fehlgeschlagen, malloc */
            pNewIndex = malloc((ulNumAlloc + BROWSEDELTA)* sizeof(NAMEINDEX));
            if (pNameBrowse->pNameIndex)
            {
               memcpy(pNewIndex, pNameBrowse->pNameIndex, pNameBrowse->ulNumNames * sizeof(NAMEINDEX));
               free(pNameBrowse->pNameIndex);
            }
         }
         pNameBrowse->pNameIndex = pNewIndex;

         ulNumAlloc += BROWSEDELTA;
      }

      /* Namen extrahieren */
      for (i=0; i < Ndx.LNodeBlock.IndxCnt; i++)
      {
         /* Adresse */
         pchName = ((PCHAR)&Ndx) + Ndx.LNodeBlock.LeafRef[i].KeyOfs;
         /*memset(pNameBrowse->pNameIndex[pNameBrowse->ulNumNames].pchSysopName, 0, LEN_USERNAME+1);*/
         strncpy(pNameBrowse->pNameIndex[pNameBrowse->ulNumNames].pchSysopName, pchName, min(LEN_USERNAME, Ndx.LNodeBlock.LeafRef[i].KeyLen));
         pNameBrowse->pNameIndex[pNameBrowse->ulNumNames].pchSysopName[min(LEN_USERNAME, Ndx.LNodeBlock.LeafRef[i].KeyLen)]=0;

         /* Daten-Zeiger */
         pNameBrowse->pNameIndex[pNameBrowse->ulNumNames].lDataOffs = Ndx.LNodeBlock.LeafRef[i].KeyVal;

         pNameBrowse->ulNumNames++;
      }

      lBlock = Ndx.LNodeBlock.IndxFLink;
   }

   return 0;
}

static int CreateNameIndex(PNAMEBROWSE pNameBrowse)
{
   int i;
   char chAlpha=0;

   for (i=0; i<pNameBrowse->ulNumNames; i++)
   {
      chAlpha = toupper(pNameBrowse->pNameIndex[i].pchSysopName[0]);

      if (chAlpha < 'A' || chAlpha > 'Z')
         chAlpha = 0;
      else
         chAlpha = chAlpha - '@';

      if (chAlpha <= 27 && pNameBrowse->Alpha[chAlpha] == NULL)
            pNameBrowse->Alpha[chAlpha] = &(pNameBrowse->pNameIndex[i]);
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTV7CloseNameBrowse                                       */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Beendet das Nodelist-Browsing                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pNameBrowse:  Zeiger auf Browser-Struktur                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: V7ERR_*                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int FLTV7CloseNameBrowse(PNAMEBROWSE pNameBrowse)
{
   /* Data-File schliessen */
   if (pNameBrowse->pfDataFile)
   {
      fclose(pNameBrowse->pfDataFile);
      pNameBrowse->pfDataFile=NULL;
   }

   /* V7+-File */
   if (pNameBrowse->pfDataDTP)
   {
      fclose(pNameBrowse->pfDataDTP);
      pNameBrowse->pfDataDTP=NULL;
   }

   /* Array freigeben */
   if (pNameBrowse->pNameIndex)
   {
      free(pNameBrowse->pNameIndex);
      pNameBrowse->pNameIndex=NULL;
   }

   close(pNameBrowse->BusyFile);

   return V7ERR_OK;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: FLTV7ReadNameData                                          */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Holt die Node-Daten zu einem Namen                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pNameBrowse:  Zeiger auf Browser-Struktur                      */
/*            pNameIndex:   Name, zu dem die Daten geholt werden sollen      */
/*            pNodeData:    Puffer f. Node-Daten                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: V7ERR_*                                                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int FLTV7ReadNameData(PNAMEBROWSE pNameBrowse, PNAMEINDEX pNameIndex, PNODEDATA pNodeData)
{
   if (get_ver7_info(&pNameBrowse->V7PHeader, pNameBrowse->pfDataFile, pNameBrowse->pfDataDTP, pNameIndex->lDataOffs, pNodeData))
      return V7ERR_DATREADERR;
   else
      return V7ERR_OK;
}
/******************************************************************/
/* qsort.c  --  Non-Recursive ANSI Quicksort function             */
/*                                                                */
/* Public domain by Raymond Gardner, Englewood CO  February 1991  */
/*                                                                */
/* Usage:                                                         */
/*     qsort(base, nbr_elements, width_bytes, compare_function);  */
/*        void *base;                                             */
/*        size_t nbr_elements, width_bytes;                       */
/*        int (*compare_function)(const void *, const void *);    */
/*                                                                */
/* Sorts an array starting at base, of length nbr_elements, each  */
/* element of size width_bytes, ordered via compare_function,     */
/* which is called as  (*compare_function)(ptr_to_element1,       */
/* ptr_to_element2) and returns < 0 if element1 < element2,       */
/* 0 if element1 = element2, > 0 if element1 > element2.          */
/* Most refinements are due to R. Sedgewick. See "Implementing    */
/* Quicksort Programs", Comm. ACM, Oct. 1978, and Corrigendum,    */
/* Comm. ACM, June 1979.                                          */
/******************************************************************/

#define SWAP_INTS

/* prototypes */
void _Inline swap_chars(char *a, char *b, size_t nbytes);

/*
** Compile with -DSWAP_INTS if your machine can access an int at an
** arbitrary location with reasonable efficiency.  (Some machines
** cannot access an int at an odd address at all, so be careful.)
*/

#ifdef   SWAP_INTS
 void _Inline swap_ints(char *, char *, size_t);
 #define  SWAP(a, b)  (swap_func((char *)(a), (char *)(b), width))
#else
 #define  SWAP(a, b)  (swap_chars((char *)(a), (char *)(b), size))
#endif

#define  COMP(a, b)  ((*comp)((void *)(a), (void *)(b)))

#define  T           7    /* subfiles of T or fewer elements will */
                          /* be sorted by a simple insertion sort */
                          /* Note!  T must be at least 3          */

void QuickSort(void *basep, size_t nelems, size_t size,
                            int (*comp)(const void *, const void *))
{
   char *stack[40], **sp;       /* stack and stack pointer        */
   char *i, *j, *limit;         /* scan and limit pointers        */
   size_t thresh;               /* size of T elements in bytes    */
   char *base;                  /* base pointer as char *         */

#ifdef   SWAP_INTS
   size_t width;                /* width of array element         */
   void (*swap_func)(char *, char *, size_t); /* swap func pointer*/

   width = size;                /* save size for swap routine     */
   swap_func = swap_chars;      /* choose swap function           */
   if ( size % sizeof(int) == 0 ) {   /* size is multiple of ints */
      width /= sizeof(int);           /* set width in ints        */
      swap_func = swap_ints;          /* use int swap function    */
   }
#endif

   base = (char *)basep;        /* set up char * base pointer     */
   thresh = T * size;           /* init threshold                 */
   sp = stack;                  /* init stack pointer             */
   limit = base + nelems * size;/* pointer past end of array      */
   for ( ;; ) {                 /* repeat until break...          */
      if ( limit - base > thresh ) {  /* if more than T elements  */
                                      /*   swap base with middle  */
         SWAP((((limit-base)/size)/2)*size+base, base);
         i = base + size;             /* i scans left to right    */
         j = limit - size;            /* j scans right to left    */
         if ( COMP(i, j) > 0 )        /* Sedgewick's              */
            SWAP(i, j);               /*    three-element sort    */
         if ( COMP(base, j) > 0 )     /*        sets things up    */
            SWAP(base, j);            /*            so that       */
         if ( COMP(i, base) > 0 )     /*      *i <= *base <= *j   */
            SWAP(i, base);            /* *base is pivot element   */
         for ( ;; ) {                 /* loop until break         */
            do                        /* move i right             */
               i += size;             /*        until *i >= pivot */
            while ( COMP(i, base) < 0 );
            do                        /* move j left              */
               j -= size;             /*        until *j <= pivot */
            while ( COMP(j, base) > 0 );
            if ( i > j )              /* if pointers crossed      */
               break;                 /*     break loop           */
            SWAP(i, j);       /* else swap elements, keep scanning*/
         }
         SWAP(base, j);         /* move pivot into correct place  */
         if ( j - base > limit - i ) {  /* if left subfile larger */
            sp[0] = base;             /* stack left subfile base  */
            sp[1] = j;                /*    and limit             */
            base = i;                 /* sort the right subfile   */
         } else {                     /* else right subfile larger*/
            sp[0] = i;                /* stack right subfile base */
            sp[1] = limit;            /*    and limit             */
            limit = j;                /* sort the left subfile    */
         }
         sp += 2;                     /* increment stack pointer  */
      } else {      /* else subfile is small, use insertion sort  */
         for ( j = base, i = j+size; i < limit; j = i, i += size )
            for ( ; COMP(j, j+size) > 0; j -= size ) {
               SWAP(j, j+size);
               if ( j == base )
                  break;
            }
         if ( sp != stack ) {         /* if any entries on stack  */
            sp -= 2;                  /* pop the base and limit   */
            base = sp[0];
            limit = sp[1];
         } else                       /* else stack empty, done   */
            break;
      }
   }
}

/*
**  swap nbytes between a and b
*/

void _Inline swap_chars(char *a, char *b, size_t nbytes)
{
   char tmp;
   do {
      tmp = *a; *a++ = *b; *b++ = tmp;
   } while ( --nbytes );
}

#ifdef   SWAP_INTS

/*
**  swap nints between a and b
*/

void _Inline swap_ints(char *ap, char *bp, size_t nints)
{
   int *a = (int *)ap, *b = (int *)bp;
   int tmp;
   do {
      tmp = *a; *a++ = *b; *b++ = tmp;
   } while ( --nints );
}

#endif

#endif /* STANDALONE */

#ifdef STANDALONE
/*ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ StringToNetAddr ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸
  ³ Wandelt String in NetAddresse um. Unvollst„ndige Adressangaben werden   ³
  ³ aus der Default-Adresse bernommen.                                     ³
  ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¾*/
FTNADDRESS * StringToNetAddr(PCHAR buffer, FTNADDRESS *address, PCHAR Default)
{
   FTNADDRESS  defaddr={60000,60000,60000,60000};

   if (Default)
      M_ParseAddress(Default, &defaddr);

   memset(address, 0, sizeof(FTNADDRESS));
   M_ParseAddress(buffer, address);

   if (Default)
   {
      if (address->usZone==60000)
         address->usZone=defaddr.usZone;

      if (address->usNet==60000)
         address->usNet=defaddr.usNet;

      if (address->usNode==60000)
         address->usNode=defaddr.usNode;

      if (address->usPoint==60000)
         address->usPoint=0;
   }

   if (address->usZone==60000)
      address->usZone=0;

   if (address->usNet==60000)
      address->usNet=0;

   if (address->usNode==60000)
      address->usNode=0;

   if (address->usPoint==60000)
      address->usPoint=0;

   return address;
}

/*ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ M_ParseAddress ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸
  ³ M_ParseAddress verarbeitet einen Textstring, der eine evtl. unvoll-     ³
  ³ staendige Fido-Adresse enthaelt. Falls die Adresse formell falsch ist,  ³
  ³ wird 1 zurckgegeben, ansonsten 0. Bei unvollstaendigen Adressen ent-   ³
  ³ halten die nicht angegebenen Adressteile jeweils 0.                     ³
  ³                                                                         ³
  ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¾*/

static int M_ParseAddress(PCHAR pchAddress, FTNADDRESS *pNetAddr)
{
   char achNumBuffer[LEN_5DADDRESS+1]="";
   int iZone=60000, iNet=60000, iNode=60000, iPoint=60000;
   int iSource=0, iDest=0;
   BOOL bEnd=FALSE, bNodeProcessed=FALSE;

   for (iSource=0; iSource<=LEN_5DADDRESS && !bEnd; iSource++)
      switch(pchAddress[iSource])
      {
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            achNumBuffer[iDest++]=pchAddress[iSource];
            break;

         case '/':
            achNumBuffer[iDest]='\0';
            iDest=0;
            if (iNet!=60000 || iNode!=60000 || iPoint!=60000)     /* falsche Reihenfolge */
               return 1;
            if (achNumBuffer[0])
               iNet=strtol(achNumBuffer, NULL, 10);
            break;

         case ':':
            achNumBuffer[iDest]='\0';
            iDest=0;
            if (iZone!=60000 || iNet!=60000 || iNode!=60000 || iPoint!=60000)     /* falsche Reihenfolge */
               return 1;
            if (achNumBuffer[0])
               iZone=strtol(achNumBuffer, NULL, 10);
            break;

         case '.':
            achNumBuffer[iDest]='\0';
            iDest=0;
            if (iNode!=60000)     /* falsche Reihenfolge */
               return 1;
            if (achNumBuffer[0])
               iNode=strtol(achNumBuffer, NULL, 10);
            bNodeProcessed=TRUE;
            break;

         case '\0':
         default:
            achNumBuffer[iDest]='\0';
            iDest=0;
            if (achNumBuffer[0])
            {
               if (iNode!=60000 || bNodeProcessed)
                  iPoint=strtol(achNumBuffer, NULL, 10);
               else
                  iNode=strtol(achNumBuffer, NULL, 10);
            }
            bEnd=TRUE;
            break;
      }
   pNetAddr->usZone=iZone;
   pNetAddr->usNet=iNet;
   pNetAddr->usNode=iNode;
   pNetAddr->usPoint=iPoint;

   return 0;
}


#endif

/*-------------------------------- Modulende --------------------------------*/

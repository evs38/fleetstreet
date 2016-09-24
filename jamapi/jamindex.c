/*---------------------------------------------------------------------------+
 | Titel: JAMINDEX.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 26.06.1995                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Laedt den Index einer JAM-Area in den Speicher                          |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Moegl. Verbesserungen:                                                    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Geaendert:                                                                |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "..\main.h"
#include "..\structs.h"
#include "..\msgheader.h"
#include "jammb.h"

#include "jamindex.h"


/*--------------------------------- Defines ---------------------------------*/

#define SCANBLOCK  512   /* Anzahl Index-Records beim Scannen */
#define ALLOCBLOCK  2000 /* Anzahl Alloc-Records beim Scannen */

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

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


PULONG JAMLoadIndex(JAMAPIRECptr apirec, PULONG pulMaxMsgs)
{
   JAMIDXRECptr pIdx;
   int iRead, j;
   PULONG pulArray;
   int i=0;
   int id = apirec->HdrInfo.BaseMsgNum;

#if 0
   if (!apirec->HdrInfo.ActiveMsgs)
      return NULL;
#endif

   if (apirec->SeekFunc(apirec, apirec->IdxHandle, JAMSEEK_SET, 0)!= 0)
   {
      apirec->APImsg=JAMAPIMSG_SEEKERROR;
      return NULL;
   }

#if 0
   pulArray= calloc(apirec->HdrInfo.ActiveMsgs, sizeof(ULONG));
   if (!pulArray)
   {
      apirec->ulIdxAlloc = 0;
      apirec->APImsg=JAMAPIMSG_INVMSGNUM;
      return NULL;
   }
   apirec->ulIdxAlloc = apirec->HdrInfo.ActiveMsgs;
#else
   apirec->ulIdxAlloc = 0;
   pulArray = NULL;
#endif

   pIdx = malloc(sizeof(JAMIDXREC) * SCANBLOCK);
   if (!pIdx)
   {
      free(pulArray);
      apirec->ulIdxAlloc = 0;
      apirec->APImsg=JAMAPIMSG_INVMSGNUM;
      return NULL;
   }

   while (
#if 0
   i < apirec->HdrInfo.ActiveMsgs &&
#endif
          !__eof(apirec->IdxHandle))
   {
      iRead = apirec->ReadFunc(apirec, apirec->IdxHandle, pIdx, sizeof(JAMIDXREC)* SCANBLOCK);

      for (j=0; j < iRead/sizeof(JAMIDXREC); j++)
      {
         if (pIdx[j].UserCRC != 0xffffffffUL ||
             pIdx[j].HdrOffset != 0xffffffffUL)
         {
#if 0
            JAMHDR Hdr;

            if (JAMmbFetchHdrDirect(apirec, &Hdr, pIdx[j].HdrOffset))
               if (!(Hdr.Attribute & MSG_DELETED))
                  pulArray[i++] = id;
#else
            /* gueltiger Index */
            /* Speicher testen */
            if (apirec->ulIdxAlloc <= i)
            {
               /* mehr Speicher */
               PULONG pulTemp;

               pulTemp = realloc(pulArray, (apirec->ulIdxAlloc + ALLOCBLOCK) * sizeof(ULONG));
               if (!pulTemp)
               {
                  pulTemp = malloc((apirec->ulIdxAlloc + ALLOCBLOCK) * sizeof(ULONG));
                  memcpy(pulTemp, pulArray, apirec->ulIdxAlloc * sizeof(ULONG));
                  free(pulArray);
               }
               pulArray = pulTemp;
               apirec->ulIdxAlloc += ALLOCBLOCK;
            }

            pulArray[i++] = id;
#endif
         }

         id++;
      }
   }
   free(pIdx);

   *pulMaxMsgs = i;

   /* Got it OK */
   apirec->APImsg=JAMAPIMSG_NOTHING;
   return pulArray;
}

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

int JAMFreeIndex(PAREADEFLIST pAreaDef)
{
   if (pAreaDef->msgnumlist)
   {
      free(pAreaDef->msgnumlist);
      pAreaDef->msgnumlist = NULL;
   }

   ((JAMAPIRECptr) pAreaDef->areahandle)->ulIdxAlloc =0;

   return 0;
}

ULONG JAMFindNum(PAREADEFLIST pAreaDef, int msgnum)
{
   if (pAreaDef->msgnumlist /*&& msgnum <= pAreaDef->maxmessages*/)
      return pAreaDef->msgnumlist[msgnum-1];
   else
      return 0;
}

ULONG JAMFindUid(PAREADEFLIST pAreaDef, ULONG ulMsgID, BOOL exact)
{
   int i=0;
   int max;

   if (pAreaDef->maxmessages < ((JAMAPIRECptr)pAreaDef->areahandle)->ulIdxAlloc)
      max = pAreaDef->maxmessages;
   else
      max = ((JAMAPIRECptr)pAreaDef->areahandle)->ulIdxAlloc;

   while (i < max &&
          pAreaDef->msgnumlist[i] <= ulMsgID) /* lineare Suche */
   {
      if (pAreaDef->msgnumlist[i] == ulMsgID)
         return i+1;
      else
         i++;
   }

   if (!exact)
      return i;
   else
      return 0;
}

void JAMAddToIdx(PAREADEFLIST pAreaDef, ULONG ulMsgNum)
{
   if (((JAMAPIRECptr)pAreaDef->areahandle)->ulIdxAlloc < pAreaDef->maxmessages)
   {
      /* neu allokieren */

      PULONG pulSaveArray = pAreaDef->msgnumlist;

      pAreaDef->msgnumlist = malloc(pAreaDef->maxmessages * sizeof(ULONG));
      if (pulSaveArray)
      {
         /* alte Daten kopieren */
         memcpy(pAreaDef->msgnumlist, pulSaveArray, ((JAMAPIRECptr)pAreaDef->areahandle)->ulIdxAlloc * sizeof(ULONG));
         free(pulSaveArray);
      }
      ((JAMAPIRECptr)pAreaDef->areahandle)->ulIdxAlloc = pAreaDef->maxmessages;
   }

   pAreaDef->msgnumlist[pAreaDef->maxmessages - 1]= ulMsgNum;

   return;
}

void RemoveFromIdx(PAREADEFLIST pAreaDef, int num)
{
   if ((pAreaDef->maxmessages - num + 1) > 0)
      memmove(&pAreaDef->msgnumlist[num-1], &pAreaDef->msgnumlist[num],
              sizeof(ULONG) * (pAreaDef->maxmessages - num + 1));

   return;
}

/*-------------------------------- Modulende --------------------------------*/

/*---------------------------------------------------------------------------+
 | Titel: ADDRCNV.C                                                          |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 23.01.1996                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Adreแkonvertierungen ASCII <-> FTNADDRESS                               |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "..\main.h"
#include "..\msgheader.h"

#include "addrcnv.h"


/*--------------------------------- Defines ---------------------------------*/

#define ADDR_NONE 60000U

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

static int M_ParseAddress(PCHAR pchAddress, FTNADDRESS *pNetAddr);

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

/*ีอออออออออออออออออออออออออออ NetAddrToString อออออออออออออออออออออออออออออออธ
  ณ Wandelt eine NETADDR-Struktur in einen String um.                         ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
char * NetAddrToString(PCHAR buffer, FTNADDRESS *address)
{
   if (address->usPoint==0)
      sprintf(buffer,"%u:%u/%u",   address->usZone, address->usNet,
                                   address->usNode);
   else
      sprintf(buffer,"%u:%u/%u.%u",address->usZone, address->usNet,
                                   address->usNode, address->usPoint);
   return buffer;
}



/*ีอออออออออออออออออออออออออออออ StringToNetAddr อออออออออออออออออออออออออออธ
  ณ Wandelt String in NetAddresse um. Unvollstndige Adressangaben werden   ณ
  ณ aus der Default-Adresse bernommen.                                     ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/
FTNADDRESS * StringToNetAddr(PCHAR buffer, FTNADDRESS *address, PCHAR Default)
{
   FTNADDRESS defaddr={ADDR_NONE, ADDR_NONE, ADDR_NONE, ADDR_NONE};

   if (Default)
      M_ParseAddress(Default, &defaddr);

   memset(address, 0, sizeof(FTNADDRESS));
   if (M_ParseAddress(buffer, address))
      address->usZone= address->usNet= address->usNode= address->usPoint= ADDR_NONE;

   if (Default)
   {
      if (address->usZone==ADDR_NONE)
         address->usZone=defaddr.usZone;

      if (address->usNet==ADDR_NONE)
         address->usNet=defaddr.usNet;

      if (address->usNode==ADDR_NONE)
         address->usNode=defaddr.usNode;

      if (address->usPoint==ADDR_NONE)
         address->usPoint=0;
   }

   if (address->usZone==ADDR_NONE)
      address->usZone=0;

   if (address->usNet==ADDR_NONE)
      address->usNet=0;

   if (address->usNode==ADDR_NONE)
      address->usNode=0;

   if (address->usPoint==ADDR_NONE)
      address->usPoint=0;

   return address;
}

/*ีออออออออออออออออออออออออออออ M_ParseAddress อออออออออออออออออออออออออออออธ
  ณ M_ParseAddress verarbeitet einen Textstring, der eine evtl. unvoll-     ณ
  ณ staendige Fido-Adresse enthaelt. Falls die Adresse formell falsch ist,  ณ
  ณ wird 1 zurckgegeben, ansonsten 0. Bei unvollstaendigen Adressen ent-   ณ
  ณ halten die nicht angegebenen Adressteile jeweils 0.                     ณ
  ณ                                                                         ณ
  ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ*/

static int M_ParseAddress(PCHAR pchAddress, FTNADDRESS *pNetAddr)
{
   USHORT usTemp = ADDR_NONE;
   BOOL bEnd=FALSE, bNodeProcessed=FALSE;
   PCHAR pchSrc=pchAddress;

   pNetAddr->usZone= pNetAddr->usNet= pNetAddr->usNode= pNetAddr->usPoint=ADDR_NONE;

   for ( ; (pchSrc - pchAddress)<=LEN_5DADDRESS && !bEnd; pchSrc++)
      switch(*pchSrc)
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
            if (usTemp == ADDR_NONE)
               usTemp=0;
            usTemp = usTemp*10 + *pchSrc-'0';
            break;

         case '/':
            if (pNetAddr->usNet!=ADDR_NONE || pNetAddr->usNode!=ADDR_NONE || pNetAddr->usPoint!=ADDR_NONE) /* falsche Reihenfolge */
               return 1;
            pNetAddr->usNet=usTemp;
            usTemp=0;
            break;

         case ':':
            if (pNetAddr->usZone!=ADDR_NONE || pNetAddr->usNet!=ADDR_NONE || pNetAddr->usNode!=ADDR_NONE || pNetAddr->usPoint!=ADDR_NONE) /* falsche Reihenfolge */
               return 1;
            pNetAddr->usZone=usTemp;
            usTemp=0;
            break;

         case '.':
            if (pNetAddr->usNode!=ADDR_NONE)     /* falsche Reihenfolge */
               return 1;
            pNetAddr->usNode=usTemp;
            usTemp=0;
            bNodeProcessed=TRUE;
            break;

         case '\0':
         default:
            if (pNetAddr->usNode!=ADDR_NONE || bNodeProcessed)
               pNetAddr->usPoint=usTemp;
            else
               pNetAddr->usNode=usTemp;
            bEnd=TRUE;
            usTemp=0;
            break;
      }

   return 0;
}


/*-------------------------------- Modulende --------------------------------*/


/*---------------------------------------------------------------------------+
 | Titel: PIPESERV.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 24.8.93                     |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Pipe-Server fuer Fleet Street                                         |
 |                                                                           |
 | Kommandos:                                                                |
 |     SCAN  [*|area|@filename]                                              |
 |     ETOSS [filename]                                                      |
 |     LOCK  [*|area|@filename]                                              |
 |     UNLCK [*|area|@filename]                                              |
 |     DBG   [MSG|HD|KL|SB|USR|DOM|STA|AR|SPC] {non-public}                  |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys\stat.h>

#include "main.h"
#include "version.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "pipeserv.h"
#include "handlemsg\handlemsg.h"
#include "handlemsg\kludgeapi.h"
#include "utility.h"
#include "areadlg.h"
#include "areascan.h"
#include "asciitable.h"
#include "dump\expt.h"

/*--------------------------------- Defines ---------------------------------*/

#define PIPE_INVALID       0
#define PIPE_DISCONNECTED  1
#define PIPE_CONNECTED     2
#define PIPE_CLOSING       3
#define PIPE_END           4

#define COM_QUIET          0
#define COM_WAITCOM        1
#define COM_WAITCOM2       2
#define COM_RXCMD          3

#define PIPENAME "\\PIPE\\FleetStreetDoor"

/*---------------------------- Globale Variablen ----------------------------*/

/* Alle Programmoptionen */
extern AREALIST arealiste;
extern USERDATAOPT userdaten;
extern WINDOWCOLORS windowcolors;
extern WINDOWFONTS windowfonts;
extern WINDOWPOSITIONS windowpositions;
extern PATHNAMES pathnames;
extern MISCOPTIONS miscoptions;
extern MACROTABLEOPT macrotable;
extern GENERALOPT generaloptions;
extern ECHOTOSSOPT echotossoptions;
extern PDOMAINS domains;
extern OUTBOUND outbound[MAX_ADDRESSES];
extern INTLSETTING intlsetting;
extern THREADLISTOPTIONS threadlistoptions;
extern LOOKUPOPTIONS lookupoptions;
extern RESULTSOPTIONS resultsoptions;

/* Derzeitige Message */
extern FTNMESSAGE CurrentMessage;
extern MSGHEADER  CurrentHeader;
extern char CurrentAddress[LEN_5DADDRESS+1];
extern char CurrentName[LEN_USERNAME+1];
extern char CurrentArea[LEN_AREATAG+1];
extern int  CurrentStatus;

static const char *PipeCommands[]={"SCAN", "ETOSS", "DBG", "LOCK", "UNLCK", ""};
#ifdef DEBUG
static const char *DebugCommands[]={"MSG", "HD", "KL", "SB", "USR", "DOM",
                                    "STA", "AR", "SPC", ""};
#endif

/* Empfangspuffer */
static char pchRx[100]="";
static int iRead=0;
static ULONG bytesread=0;

/*--------------------------- Funktionsprototypen ---------------------------*/

#ifdef DEBUG
static void DumpStatus(HPIPE hPipe);
static void DumpMessageText(HPIPE hPipe);
static void DumpHeader(HPIPE hPipe);
static void DumpSeenby(HPIPE hPipe);
static void DumpKludges(HPIPE hPipe);
static void DumpUser(HPIPE hPipe);
static void DumpDomains(HPIPE hPipe);
static void DumpArea(HPIPE hPipe);
#endif

static int IsValidCommand(int parmc, char **parmv);
static int IsValidParameter(int CmdIndex, int parmc, char **parmv);
static void ExecuteCommand(HPIPE hPipe, int CmdIndex, int parmc, char **parmv);
#ifdef DEBUG
static void ExecuteDebug(HPIPE hPipe, int parmc, char **parmv);
#endif

int ReadPipeText(HFILE hPipe, char *pchBuf, BOOL bSingleByte);
int SendPipeText(HFILE hPipe, const char *pchText, BOOL bAddETX);

static int SeparateCommand(char *pchOrigString, int maxparts, char **retarray);
static int GetLogFile(const char *pchFileName, char **pchDestBuf);
static int ForEachWord(char *pchLines, int (* CallBack)(const char*));
static int MarkAreaToScan(const char *pchAreaTag);
static void ScanLoggedAreas(HPIPE hPipe, const char *pchFileName);

/*---------------------------------------------------------------------------*/
/* Funktionsname: NewPipeServer                                              */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Pipe-Server Threadfunktion                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pThreadParam: Dummy-Parameter                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: FSM eines Pipe-Servers                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void _Optlink NewPipeServer(PVOID pThreadParam)
{
   int PipeStatus = PIPE_INVALID;
   int ComStatus = COM_QUIET;
   char command[300];
   HPIPE DoorPipe=NULLHANDLE;
   int CmdIndex=0;
   ULONG written;

   INSTALLEXPT("Pipe");

   pThreadParam = pThreadParam;

   while (PipeStatus != PIPE_END)            /* Endlosschleife */
      switch (PipeStatus)
      {
         case PIPE_INVALID:
            /* Pipe erzeugen */
            switch(DosCreateNPipe(PIPENAME, &DoorPipe, NP_INHERIT | NP_ACCESS_DUPLEX,
                           NP_WAIT | NP_TYPE_BYTE | NP_READMODE_BYTE | 1,
                           1000, 100, 0))
            {
               case NO_ERROR:
                  PipeStatus=PIPE_DISCONNECTED;
                  break;

               default:
                  PipeStatus=PIPE_END;
                  break;
            }
            break;

         case PIPE_DISCONNECTED:
            /* Pipe connecten */
            switch (DosConnectNPipe(DoorPipe))
            {
               case NO_ERROR:
                  PipeStatus=PIPE_CONNECTED;
                  break;

               case ERROR_BROKEN_PIPE:
                  PipeStatus=PIPE_CLOSING;
                  break;

               case ERROR_INTERRUPT:
                  break;

               default:
                  DosBeep(1000,100);
                  PipeStatus=PIPE_END;
                  break;
            }
            break;

         case PIPE_CONNECTED:
            switch(ComStatus)
            {
               case COM_QUIET:
                  if (!ReadPipeText(DoorPipe, command, TRUE))
                  {
                     if (command[0] == C_ENQ)
                     {
                        /* Identifikation schicken */
                        ComStatus = COM_WAITCOM;
                        SendPipeText(DoorPipe, "FleetStreet", TRUE);
                     }
                  }
                  else
                  {
                     ComStatus = COM_QUIET;
                     PipeStatus=PIPE_CLOSING;
                  }
                  break;

               case COM_WAITCOM:
                  if (!ReadPipeText(DoorPipe, command, TRUE))
                  {
                     if (command[0] == C_ACK)
                     {
                        /* Versionsnummer schicken */
                        ComStatus = COM_WAITCOM2;
                        SendPipeText(DoorPipe, FLEETVER, TRUE);
                     }
                     else
                     {
                        /* NAK erhalten */
                        DosWrite(DoorPipe, S_EOT, 1, &written);
                        ComStatus = COM_QUIET;
                        PipeStatus = PIPE_CLOSING;
                     }
                  }
                  else
                  {
                     ComStatus = COM_QUIET;
                     PipeStatus=PIPE_CLOSING;
                  }
                  break;

               case COM_WAITCOM2:
                  if (!ReadPipeText(DoorPipe, command, TRUE))
                  {
                     if (command[0] == C_ACK)
                     {
                        /* Warten auf Befehl */
                        ComStatus = COM_RXCMD;
                     }
                     else
                     {
                        /* NAK erhalten */
                        DosWrite(DoorPipe, S_EOT, 1, &written);
                        ComStatus = COM_QUIET;
                        PipeStatus = PIPE_CLOSING;
                     }
                  }
                  else
                  {
                     ComStatus = COM_QUIET;
                     PipeStatus=PIPE_CLOSING;
                  }
                  break;

               case COM_RXCMD:
                  if (!ReadPipeText(DoorPipe, command, TRUE))
                  {
                     if (command[0] == C_EOT)
                     {
                        DosWrite(DoorPipe, S_EOT, 1, &written);
                        ComStatus = COM_QUIET;
                        PipeStatus = PIPE_CLOSING;
                     }
                     else
                     {
                        int paramtest;
                        int parmc;
                        char *parmv[5];

                        parmc = SeparateCommand(command, 5, parmv);

                        if (parmc == 0)
                           SendPipeText(DoorPipe, S_NAK "C", TRUE);
                        else
                           if ((CmdIndex = IsValidCommand(parmc, parmv)) == -1)
                              SendPipeText(DoorPipe, S_NAK "C", TRUE);
                           else
                              if (!(paramtest=IsValidParameter(CmdIndex, parmc, parmv)))
                                 SendPipeText(DoorPipe, S_NAK "P", TRUE);
                              else
                              {
                                 if (paramtest == -1)
                                 {
                                    SendPipeText(DoorPipe, S_NAK "S", TRUE);
                                 }
                                 else
                                 {
                                    SendPipeText(DoorPipe, S_ACK, TRUE);

                                    ExecuteCommand(DoorPipe, CmdIndex, parmc, parmv);
                                    /* Ende */
                                    DosWrite(DoorPipe, S_ETX, 1, &written);
                                 }
                              }
                      }
                  }
                  else
                  {
                     ComStatus = COM_QUIET;
                     PipeStatus=PIPE_CLOSING;
                  }
                  break;

               default:
                  break;
            }
            break;

         case PIPE_CLOSING:
            /* Puffer zur…ksetzen */
            pchRx[0]=0;
            bytesread=0;
            iRead=0;

            switch (DosDisConnectNPipe(DoorPipe))
            {
               case ERROR_BROKEN_PIPE:
               case NO_ERROR:
                  PipeStatus=PIPE_DISCONNECTED;
                  break;

               default:
                  PipeStatus=PIPE_END;
                  break;
            }
            break;

         default:
            break;
      }

   if (DoorPipe)
      DosClose(DoorPipe);

   DEINSTALLEXPT;

   return;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ReadPipeText                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liest einen Text aus der Pipe                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hPipe: Pipe-Handle                                             */
/*            pchBuf: Zeiger auf Puffer                                      */
/*            bSingleByte: Liesst auch ein einzelnes Byte                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  Text gelesen                                            */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: ETX wird abgetrennt, Puffer ist Nullterminiert                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ReadPipeText(HFILE hPipe, char *pchBuf, BOOL bSingleByte)
{
   char *pchBufPtr = pchBuf;
   static int i=100;

   while(1)  /* endlos */
   {
      if (i == 100 || iRead == bytesread)
      {
         /* Empfangspuffer leer, neu lesen */
         i=0;
         iRead=0;
         DosRead(hPipe, pchRx, 100, &bytesread);
      }
      if (bytesread > 0)
      {
         while (i < 100 && iRead < bytesread)
         {
            if (pchRx[i] != C_ETX)
            {
               if (bSingleByte &&
                   (pchRx[i] == C_ACK ||
                    pchRx[i] == C_NAK ||
                    pchRx[i] == C_ENQ))    /* Einzelbyte lesen, nicht auf ETX warten */
               {
                  *pchBufPtr++ = pchRx[i++];
                  *pchBufPtr = 0;
                  iRead++;
                  return 0;
               }
               else
               {
                  *pchBufPtr++ = pchRx[i++];
                  iRead++;
               }
            }
            else
            {
               *pchBufPtr = 0;
               i++;
               iRead++;
               return 0;
            }
         }
      }
      else  /* Lesefehler */
      {
         *pchBufPtr = 0;
         return -1;
      }
   }

}

/*---------------------------------------------------------------------------*/
/* Funktionsname: SendPipeText                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Schickt einen Text in die Pipe                              */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hPipe: Pipe-Handle                                             */
/*            pchText: Text, der gesendet wird.                              */
/*            bAddETX: Ein ETX wird nach dem Text gesendet.                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  Text gelesen                                            */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Das Nullbyte wird nicht gesendet                               */
/*---------------------------------------------------------------------------*/

int SendPipeText(HFILE hPipe, const char *pchText, BOOL bAddETX)
{
   ULONG written;

   DosWrite(hPipe, (PVOID) pchText, strlen(pchText), &written);
   if (bAddETX)
      DosWrite(hPipe, S_ETX, 1, &written);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: IsValidCommand                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Prueft den Kommandostring auf ein gueltiges Kommando        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: parmc: Anzahl der Parameter                                    */
/*            parmv: Parameter-Array                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -1 kein gueltiges Kommando                                 */
/*                sonst  Kommando-Index                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int IsValidCommand(int parmc, char **parmv)
{
   int iIndex=0;

   if (!parmc)
      return -1;

   while(PipeCommands[iIndex][0])
   {
      if (!stricmp(parmv[0], PipeCommands[iIndex]))
         break;
      iIndex++;
   }

   if (PipeCommands[iIndex][0])
      return iIndex;
   else
      return -1;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: IsValidParameter                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Prueft den Kommandostring auf gueltige Parameter            */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: CmdIndex: Kommando-Index                                       */
/*            parmc: Anzahl der Parameter                                    */
/*            parmv: Parameter-Array                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  kein gueltiger Parameter                                */
/*                1  Parameter gueltig                                       */
/*               -1  Zu viele Parameter                                      */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int IsValidParameter(int CmdIndex, int parmc, char **parmv)
{
#ifdef DEBUG
   int iCode=0;
#endif

   switch(CmdIndex)
   {
      case 0:  /* SCAN */
         if (parmc < 2)
            return 0;
         else
            if (parmc > 2)
               return -1;
            else
               if (parmv[1][0]=='*' ||       /* '*' oder Area-Tag */
                   parmv[1][0]=='@' ||
                   AM_FindArea(&arealiste, parmv[1]))
                  return 1;
               else
                  return 0;

      case 1:  /* ETOSS */
         if (parmc == 1)
            return 1;
         if (parmc > 2)
            return -1;
         return 1;

#ifdef DEBUG
      case 2:  /* DBG */
         if (parmc < 2)
            return 0;
         if (parmc > 2)
            return -1;

         while(DebugCommands[iCode][0])
            if(stricmp(parmv[1], DebugCommands[iCode]))
               iCode++;
            else
               break;

         if (DebugCommands[iCode][0])
            return 1;
         else
            return 0;
#endif

      default:
         return 0;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ExecuteCommand                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt das Kommando aus                                     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hPipe: Pipe-Handle (fuer Rueckgabewerte)                       */
/*            CmdIndex: Kommando-Index                                       */
/*            parmc: Anzahl der Parameter                                    */
/*            parmv: Parameter-Array                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Der Bestaetigungstring muss geschrieben werden, jedoch ohne    */
/*            ETX. Format: <ACK|NAK>"Ergebnis"                               */
/*---------------------------------------------------------------------------*/

static void ExecuteCommand(HPIPE hPipe, int CmdIndex, int parmc, char **parmv)
{
   extern BOOL DoingAreaScan;
   extern int tidAreaScan;

   switch(CmdIndex)
   {
      case 0: /* SCAN */
         if (parmv[1][0] == '*')
         {
            AREADEFLIST *zeiger = arealiste.pFirstArea;

            while (zeiger)
            {
               zeiger->flWork |= WORK_SCAN;
               zeiger = zeiger->next;
            }
            while(DoingAreaScan)  /* Warten bis alter Scan vorbei */
               DosSleep(1000);
            tidAreaScan = _beginthread(ScanAreas, NULL, 16386, &arealiste);
            SendPipeText(hPipe, S_ACK "Scan started", FALSE);
         }
         else
            if (parmv[1][0] == '@')
               ScanLoggedAreas(hPipe, &(parmv[1][1]));
            else
            {
               if (MarkAreaToScan(parmv[1]))
                  SendPipeText(hPipe, S_NAK "No areas", FALSE);
               else
               {
                  while(DoingAreaScan)  /* Warten bis alter Scan vorbei */
                     DosSleep(1000);
                  tidAreaScan = _beginthread(ScanAreas, NULL, 16386, &arealiste);
                  SendPipeText(hPipe, S_ACK "Scan started", FALSE);
               }
            }
         break;

      case 1:  /* ETOSS */
         if (echotossoptions.useechotoss &&
             strlen(echotossoptions.pchEchoToss))
         {
            int rc;

            if (parmc > 1)
               rc = WriteEchotoss(&arealiste, parmv[1]);
            else
               rc = WriteEchotoss(&arealiste, echotossoptions.pchEchoToss);

            if (rc)
               SendPipeText(hPipe, S_NAK "write error", FALSE);
            else
               SendPipeText(hPipe, S_ACK "written", FALSE);
         }
         else
            SendPipeText(hPipe, S_NAK "not active", FALSE);
         break;

#ifdef DEBUG
      case 2:  /* DBG */
         ExecuteDebug(hPipe, parmc, parmv);
         break;
#endif

      case 3:  /* LOCK */
         SendPipeText(hPipe, S_NAK "Not implemented", FALSE);
         break;

      case 4:  /* UNLCK */
         SendPipeText(hPipe, S_NAK "Not implemented", FALSE);
         break;

      default:
         SendPipeText(hPipe, S_NAK "internal error", FALSE);
         break;
   }
   return;
}

#ifdef DEBUG
/*---------------------------------------------------------------------------*/
/* Funktionsname: ExecuteDebug                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Fuehrt das Kommando DBG aus                                 */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hPipe: Pipe-Handle (fuer Rueckgabewerte)                       */
/*            parmc: Anzahl der Parameter                                    */
/*            parmv: Parameter-Array                                         */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Der Bestaetigungstring muss geschrieben werden, jedoch ohne    */
/*            ETX. Format: <ACK|NAK>"Ergebnis"                               */
/*---------------------------------------------------------------------------*/

static void ExecuteDebug(HPIPE hPipe, int parmc, char **parmv)
{
   if (parmc < 2)
   {
      SendPipeText(hPipe, S_NAK "No Code", FALSE);
      return;
   }
   else
   {
      int iCode=0;

      while(DebugCommands[iCode][0])
         if(stricmp(parmv[1], DebugCommands[iCode]))
            iCode++;
         else
            break;

      if (DebugCommands[iCode][0])
      {
         switch(iCode)
         {
            case 0:  /* Message */
               DumpMessageText(hPipe);
               break;

            case 1:  /* Header */
               DumpHeader(hPipe);
               break;

            case 2:  /* Kludges */
               DumpKludges(hPipe);
               break;

            case 3:  /* SEEN-BYs */
               DumpSeenby(hPipe);
               break;

            case 4:  /* User */
               DumpUser(hPipe);
               break;

            case 5:  /* Domains */
               DumpDomains(hPipe);
               break;

            case 6:  /* Status */
               DumpStatus(hPipe);
               break;

            case 7:  /* Area */
               DumpArea(hPipe);
               break;

            case 8:  /* Special */
               SendPipeText(hPipe, S_ACK "OK", FALSE);
               break;

            default:
               SendPipeText(hPipe, S_NAK "Not impl.", FALSE);
               break;
         }
         return;
      }
      else
      {
         SendPipeText(hPipe, S_NAK "Unknown code", FALSE);
         return;
      }
   }
}

static void DumpMessageText(HPIPE hPipe)
{
   ULONG written;

   if (CurrentMessage.pchMessageText)
   {
      DosWrite(hPipe, S_ACK "\r\n", 3, &written);
      DosWrite(hPipe, CurrentMessage.pchMessageText, strlen(CurrentMessage.pchMessageText), &written);
   }
   else
   {
      DosWrite(hPipe, S_NAK "*No Text*", 10, &written);
   }
   return;
}

static void DumpHeader(HPIPE hPipe)
{
   ULONG written;
   char buffer[200];
   int i;

   DosWrite(hPipe, S_ACK, 1, &written);
   DosWrite(hPipe, "\r\n", 2, &written);

   sprintf(buffer, "From: >%-35s<\n", CurrentHeader.pchFromName);
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   sprintf(buffer, "To:   >%-35s<\n", CurrentHeader.pchToName);
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   strcpy(buffer, "O-Address: ");
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   NetAddrToString(buffer, &CurrentHeader.FromAddress);
   DosWrite(hPipe, buffer, strlen(buffer), &written);
   DosWrite(hPipe, "\r\n", 2, &written);

   strcpy(buffer, "D-Address: ");
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   NetAddrToString(buffer, &CurrentHeader.ToAddress);
   DosWrite(hPipe, buffer, strlen(buffer), &written);
   DosWrite(hPipe, "\r\n", 2, &written);

   sprintf(buffer, "Subj: >%-71s<\n", CurrentHeader.pchSubject);
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   strcpy(buffer, "Date written: ");
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   StampToString(buffer, &CurrentHeader.StampWritten);
   DosWrite(hPipe, buffer, strlen(buffer), &written);
   DosWrite(hPipe, "\r\n", 2, &written);

   strcpy(buffer, "Date arrived: ");
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   StampToString(buffer, &CurrentHeader.StampArrived);
   DosWrite(hPipe, buffer, strlen(buffer), &written);
   DosWrite(hPipe, "\r\n", 2, &written);

   sprintf(buffer, "Flags: %08x\n", CurrentHeader.ulAttrib);
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   strcpy(buffer, "Reply to: ");
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   sprintf(buffer, "%08x\n", CurrentHeader.ulReplyTo);
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   strcpy(buffer, "Replies: ");
   DosWrite(hPipe, buffer, strlen(buffer), &written);

   i=0;
   while (i<NUM_REPLIES && CurrentHeader.ulReplies[i])
   {
      sprintf(buffer, "%08x ", CurrentHeader.ulReplies[i]);
      DosWrite(hPipe, buffer, strlen(buffer), &written);
      i++;
   }
   DosWrite(hPipe, "\r\n", 2, &written);
   return;
}

static void DumpSeenby(HPIPE hPipe)
{
   SendPipeText(hPipe, S_ACK "\r\n", FALSE);
   if (CurrentMessage.pchSeenPath)
      SendPipeText(hPipe, CurrentMessage.pchSeenPath, FALSE);
   else
      SendPipeText(hPipe, "*No SEEN-BYs", FALSE);
   return;
}

static void DumpKludges(HPIPE hPipe)
{
   PKLUDGE pKludge=NULL;
   SendPipeText(hPipe, S_ACK "\r\n", FALSE);
   while (pKludge = MSG_FindKludge(&CurrentMessage, KLUDGE_ANY, pKludge))
   {
      if (pKludge->ulKludgeType == KLUDGE_OTHER)
         SendPipeText(hPipe, pKludge->pchKludgeText, FALSE);
      else
      {
         SendPipeText(hPipe, MSG_QueryKludgeName(pKludge->ulKludgeType), FALSE);
         SendPipeText(hPipe, pKludge->pchKludgeText, FALSE);
      }
      SendPipeText(hPipe, "\r\n", FALSE);
   }
   return;
}

static void DumpUser(HPIPE hPipe)
{
   char buffer[200];
   int i;

   SendPipeText(hPipe, S_ACK "\r\n", FALSE);

   /* usernamen */
   for (i=0; i<MAX_USERNAMES; i++)
   {
      sprintf(buffer, "Name %2d: >%-35s<\r\n", i+1, userdaten.username[i]);
      SendPipeText(hPipe, buffer, FALSE);
   }

   /* adressen */
   for (i=0; i<MAX_ADDRESSES; i++)
   {
      sprintf(buffer, "Address %2d: >%-40s<\r\n", i+1, userdaten.address[i]);
      SendPipeText(hPipe, buffer, FALSE);
   }

   /* origin */
   sprintf(buffer, "Origin: >%-50s<\r\n", userdaten.defaultorigin);
   SendPipeText(hPipe, buffer, FALSE);
   return;
}

static void DumpDomains(HPIPE hPipe)
{
   PDOMAINS pDomain;
   char buffer[300];

   /* Domains */
   pDomain=domains;
   SendPipeText(hPipe, S_ACK "\r\n", FALSE);
   while (pDomain)
   {
      sprintf(buffer, "Domain: >%s<\r\n", pDomain->domainname);
      SendPipeText(hPipe, buffer, FALSE);
      sprintf(buffer, "Nodelist-Index: >%s<\r\n", pDomain->indexfile);
      SendPipeText(hPipe, buffer, FALSE);
      sprintf(buffer, "Nodelist-Daten: >%s<\r\n\r\n", pDomain->nodelistfile);
      SendPipeText(hPipe, buffer, FALSE);
      pDomain=pDomain->next;
   }
   return;
}

static void DumpStatus(HPIPE hPipe)
{
   SendPipeText(hPipe, S_ACK "\r\nStatus:\r\n", FALSE);
   SendPipeText(hPipe, "CurrentAddress: >", FALSE);
   SendPipeText(hPipe, CurrentAddress, FALSE);
   SendPipeText(hPipe, "<\r\n", FALSE);
   SendPipeText(hPipe, "CurrentName:    >", FALSE);
   SendPipeText(hPipe, CurrentName, FALSE);
   SendPipeText(hPipe, "<\r\n", FALSE);
   SendPipeText(hPipe, "CurrentArea:    >", FALSE);
   SendPipeText(hPipe, CurrentArea, FALSE);
   SendPipeText(hPipe, "<\r\n", FALSE);

   SendPipeText(hPipe, "CurrentStatus:  ", FALSE);
   switch(CurrentStatus)
   {
      case PROGSTATUS_NOSETUP:
         SendPipeText(hPipe, "No Setup", FALSE);
         break;

      case PROGSTATUS_READING:
         SendPipeText(hPipe, "Reading", FALSE);
         break;

      case PROGSTATUS_EDITING:
         SendPipeText(hPipe, "Editing", FALSE);
         break;

      case PROGSTATUS_CLEANUP:
         SendPipeText(hPipe, "Clean up", FALSE);
         break;

      default:
         SendPipeText(hPipe, "unknown", FALSE);
         break;
   }

   return;
}

static void DumpArea(HPIPE hPipe)
{
   AREADEFLIST *zeiger;
   char buffer[200];

   SendPipeText(hPipe, S_ACK "CurrentArea: >", FALSE);
   SendPipeText(hPipe, CurrentArea, FALSE);
   SendPipeText(hPipe, "<\r\n", FALSE);

   zeiger=AM_FindArea(&arealiste, CurrentArea);

   if (!zeiger)
      SendPipeText(hPipe, "Invalid Area!\r\n", FALSE);
   else
   {
      sprintf(buffer, "Area Description: >%s<\r\n", zeiger->areadata.areadesc);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Address:          >%s<\r\n", zeiger->areadata.address);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Name:             >%s<\r\n", zeiger->areadata.username);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Path/File:        >%s<\r\n", zeiger->areadata.pathfile);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Template:         %d\r\n", zeiger->areadata.ulTemplateID);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Areatype:         %d\r\n", zeiger->areadata.areatype);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Format:           %d\r\n", zeiger->areadata.areaformat);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Optionen:         %d\r\n", zeiger->areadata.usAreaOpt);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Default flags:    %d\r\n", zeiger->areadata.usDefAttrib);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Max messages:     %d\r\n", zeiger->maxmessages);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Current message:  %d\r\n", zeiger->currentmessage);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Area handle:      %08x\r\n", zeiger->areahandle);
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Scanned:          %c\r\n", (zeiger->scanned)? 'Y' : 'N');
      SendPipeText(hPipe, buffer, FALSE);

      sprintf(buffer, "Mail entered:     %c\r\n", (zeiger->mailentered)? 'Y' : 'N');
      SendPipeText(hPipe, buffer, FALSE);
   }
   return;
}
#endif

/*---------------------------------------------------------------------------*/
/* Funktionsname: SeparateCommand                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Zerlegt einen String wie argv                               */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchOrigString: Original-String                                 */
/*            maxparts: Maximal zu erkennende Teile                          */
/*            retarray: erzeugtes Array wie argv                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: Anzahl der Teile (0 == nix erzeugt)                        */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int SeparateCommand(char *pchOrigString, int maxparts, char **retarray)
{
   char *pchHelp;
   int iPart=0;

   if (! pchOrigString ||
       ! maxparts)
      return 0;     /* von nix kommt nix */

   pchHelp = pchOrigString;

   while (*pchHelp && iPart < maxparts)
   {
      while (*pchHelp && *pchHelp == ' ')  /* Leerzeichen uebergehen */
         pchHelp++;

      if (*pchHelp)
      {
         retarray[iPart] = pchHelp; /* Anfang speichern */

         while (*pchHelp && *pchHelp != ' ') /* Ende suchen */
            pchHelp++;

         if (*pchHelp)
            *pchHelp++= '\0';  /* terminieren */

         iPart++;
      }
   }
   return iPart;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: GetLogFile                                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liesst ein Textfile in einen Puffer ein.                    */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchFileName: Name der Textdatei                                */
/*            pchDestBuf: Zeiger auf den Pufferzeiger                        */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Speicher f〉 den Puffer wird in der Funktion mit malloc        */
/*            belegt.                                                        */
/*---------------------------------------------------------------------------*/

static int GetLogFile(const char *pchFileName, char **pchDestBuf)
{
   struct stat filestat;
   FILE *fLogFile;

   /* Groesse des Files ermitteln */
   if (stat(pchFileName, &filestat) || filestat.st_size == 0)
      return -1;
   else
   {
      if (!(fLogFile = fopen(pchFileName, "rb")))
         return -1;

      *pchDestBuf = malloc(filestat.st_size+1);
      if (fread(*pchDestBuf, filestat.st_size, 1, fLogFile) != 1)
      {
         free(*pchDestBuf);
         return -1;
      }
      fclose(fLogFile);

      (*pchDestBuf)[filestat.st_size]= '\0';
      return 0;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ForEachWord                                                */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Ruft eine Funktion fuer jedes Wort im Text auf.             */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchLines: Text mit Woertern                                    */
/*            CallBack: Zeiger auf aufzurufende Funktion                     */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: R…kgabewert von CallBack                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Der String wird zerstoert. Wenn CallBack != 0 zurueckliefert,  */
/*            dann wird die Funktion beendet mit Return-Wert von CallBack    */
/*---------------------------------------------------------------------------*/

static int ForEachWord(char *pchLines, int (* CallBack)(const char*))
{
   char *pchWord=NULL;
   int ret=0;

   pchWord = strtok(pchLines, " \t\r\n");

   while(pchWord)
   {
      if (ret=CallBack(pchWord))
         break;
      else
         pchWord = strtok(NULL, " \t\r\n");
   }
   return ret;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: MarkAreaToScan                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Setzt bei der angegeben Area das Scan-Flag                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pchAreaTag: Area-Tag der Area                                  */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: 0  OK                                                      */
/*                -1 Keine Areas                                             */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int MarkAreaToScan(const char *pchAreaTag)
{
   AREADEFLIST *zeiger;

   if (!arealiste.ulNumAreas)
      return -1;
   else
   {
      zeiger = AM_FindArea(&arealiste, pchAreaTag);
      if (zeiger)
         zeiger->flWork |= WORK_SCAN;
      return 0;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: ScanLoggedAreas                                            */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Scannt alle Areas, die in einem Logfile angegeben sind.     */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: hPipe: Pipe-Handle f. Meldungen                                */
/*            pchFileName: Name des Logfiles                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* R…kgabewerte: -                                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: Die Areas werden asynchron gescannt.                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void ScanLoggedAreas(HPIPE hPipe, const char *pchFileName)
{
   char *pchBuf=NULL;
   extern int tidAreaScan;
   extern BOOL DoingAreaScan;

   if (GetLogFile(pchFileName, &pchBuf))
   {
      /* Lesefehler */
      SendPipeText(hPipe, S_NAK "Error reading log file ", FALSE);
      SendPipeText(hPipe, pchFileName, FALSE);
      return;
   }
   else
   {
      if (ForEachWord(pchBuf, MarkAreaToScan))
      {
         /* keine Areas */
         free(pchBuf);
         SendPipeText(hPipe, S_NAK "No areas", FALSE);
         return;
      }
      else
      {
         free(pchBuf);
         while(DoingAreaScan)  /* Warten bis alter Scan vorbei */
            DosSleep(1000);
         tidAreaScan = _beginthread(ScanAreas, NULL, 16386, &arealiste);
         SendPipeText(hPipe, S_ACK "Scan started", FALSE);
         return;
      }
   }
}

/*-------------------------------- Modulende --------------------------------*/

/*---------------------------------------------------------------------------+
 | Titel: FLEETCOM.C                                                         |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 05.06.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   FleetStreet Pipe-Server Client Tool                                     |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/

#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../version.h"
#include "../asciitable.h"

/*--------------------------------- Defines ---------------------------------*/

#define RET_OK             0
#define RET_PIPEOPEN       1
#define RET_NOCMD          2
#define RET_INVCMD         3
#define RET_ERROR          4

#define PIPENAME  "\\PIPE\\FleetStreetDoor"
#define ENVNAME   "FLEETPIPE"
#define IDENT     "FleetStreet"

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static char pchRx[100]="";
static int iRead=0;
static ULONG bytesread=0;

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

int ReadPipeText(HFILE hPipe, char *pchBuf, BOOL bSingleByte, int maxbuf);
int SendPipeText(HFILE hPipe, char *pchText);
char *GetAllParams(int argc, char **argv);

/*---------------------------------- main -----------------------------------*/

int main(int argc, char **argv)
{
   char *pchPipeName;
   HFILE hPipe;
   ULONG ulAction;
   ULONG written;
   char RxBuf[5000];

   /* Banner ausgeben */
   printf("\nFleetCom " FLEETVER ", (C) 1994-1998 Michael Hohner\n\n");

   if (argc < 2)
   {
      /* Usage ausgeben */

      printf("FLEETCOM Err: No command specified\n\n");
      printf("Usage: FLEETCOM command [options]\n\n");
      return RET_NOCMD;
   }

   /* Pipename-Override durch Environment */
   if (!(pchPipeName=getenv(ENVNAME)))
      pchPipeName = PIPENAME;

   /* Open Pipe */
   if (!DosOpen(pchPipeName, &hPipe, &ulAction, 0, FILE_NORMAL,
                OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYNONE| OPEN_ACCESS_READWRITE, NULL))
   {
      printf("FLEETCOM Inf: Trying to connect...\n");
      DosWrite(hPipe, S_ENQ, 1, &written);
      if (!ReadPipeText(hPipe, RxBuf, FALSE, 5000))
      {
         if (stricmp(RxBuf, IDENT))
         {
            printf("FLEETCOM Err: Pipe is not FleetStreet!\n\n");
            DosWrite(hPipe, S_NAK, 1, &written);
            DosClose(hPipe);
            printf("FLEETCOM Inf: End.\n");
            return RET_ERROR;
         }
         else
         {
            printf("FLEETCOM Inf: Connected to FleetStreet\n");
            DosWrite(hPipe, S_ACK, 1, &written);
            if (!ReadPipeText(hPipe, RxBuf, FALSE, 5000))
            {
               char *allparm;

               printf("FLEETCOM Inf: FleetStreet Version is %s.\n", RxBuf);
               DosWrite(hPipe, S_ACK, 1, &written);

               /* Kommando senden */
               SendPipeText(hPipe, allparm=GetAllParams(argc, argv));
               free(allparm);

               printf("FLEETCOM Inf: Sent command, waiting...\n");
               if (!ReadPipeText(hPipe, RxBuf, FALSE, 5000))
               {
                  if (RxBuf[0] == C_ACK)
                  {
                     int rc;

                     printf("FLEETCOM Inf: OK, FleetStreet is working.\n");
                     if ((rc=ReadPipeText(hPipe, RxBuf, FALSE, 5000))!=-1)
                     {
                        if (RxBuf[0] == C_ACK)
                        {
                           if (RxBuf[1])
                              printf("FLEETCOM Out: ");
                           else
                              printf("FLEETCOM Inf: Done.\n");
                           while(rc != -1)
                           {
                              if (RxBuf[1])
                                 if (RxBuf[0] == C_ACK)
                                    printf("%s", RxBuf+1);
                                 else
                                    printf("%s", RxBuf);

                              if (rc == 0)
                              {
                                 printf("\n");
                                 break;
                              }

                              rc=ReadPipeText(hPipe, RxBuf, FALSE, 5000);
                           }

                           printf("FLEETCOM Inf: Closing Connection.\n");
                           DosWrite(hPipe, S_EOT, 1, &written);
                           DosClose(hPipe);
                           printf("FLEETCOM Inf: End.\n");
                           return RET_OK;
                        }
                        else
                        {
                           printf("FLEETCOM Err: %s\n", RxBuf+1);
                           printf("FLEETCOM Inf: Closing Connection.\n");
                           DosWrite(hPipe, S_EOT, 1, &written);
                           DosClose(hPipe);
                           printf("FLEETCOM Inf: End.\n");
                           return RET_ERROR;
                        }
                     }
                     else
                     {
                        printf("FLEETCOM Err: Communication error.\n");
                        DosClose(hPipe);
                        return RET_ERROR;
                     }
                  }
                  else
                  {
                     switch(RxBuf[1])
                     {
                        case 'C':
                           printf("FLEETCOM Err: Invalid command!\n");
                           break;

                        case 'P':
                           printf("FLEETCOM Err: Invalid parameter(s)!\n");
                           break;

                        case 'S':
                           printf("FLEETCOM Err: Too many parameters!\n");
                           break;

                        default:
                           printf("FLEETCOM Err: Unknown error!\n");
                           break;
                     }
                     printf("FLEETCOM Inf: Closing Connection.\n");
                     DosWrite(hPipe, S_EOT, 1, &written);
                     DosClose(hPipe);
                     printf("FLEETCOM Inf: End.\n");
                     return RET_INVCMD;
                  }
               }
               else
               {
                  printf("FLEETCOM Err: Communication error.\n");
                  DosClose(hPipe);
                  printf("FLEETCOM Inf: End.\n");
                  return RET_ERROR;
               }
            }
            else
            {
               printf("FLEETCOM Err: Communication error.\n");
               DosClose(hPipe);
               printf("FLEETCOM Inf: End.\n");
               return RET_ERROR;
            }
         }
      }
      else
      {
         printf("FLEETCOM Err: Communication error.\n");
         DosClose(hPipe);
         printf("FLEETCOM Inf: End.\n");
         return RET_ERROR;
      }
   }
   else
   {
      printf("FLEETCOM Wrn: Could not open '%s'.\n", pchPipeName);
      printf("FLEETCOM Wrn: FleetStreet may not be running.\n\n");
      printf("FLEETCOM Inf: End.\n");
      return RET_PIPEOPEN;
   }
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
/*            maxbuf: maximale Puffergroesse                                 */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: 0  Text gelesen, Ende                                      */
/*                1  Text gelesen, noch Daten                                */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: ETX wird abgetrennt, Puffer ist Nullterminiert                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ReadPipeText(HFILE hPipe, char *pchBuf, BOOL bSingleByte, int maxbuf)
{
   char *pchBufPtr = pchBuf;
   static int i=100;

   for(;;)  /* endlos */
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
         while (i < 100 && iRead < bytesread && (pchBufPtr - pchBuf)<maxbuf)
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
         if ((pchBufPtr - pchBuf)==maxbuf)
         {
            *pchBufPtr = 0;
            return 1;
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
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: 0  Text gelesen                                            */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges: es wird ein ETX an den Text angehaengt. Das Nullbyte wird      */
/*            nicht gesendet                                                 */
/*---------------------------------------------------------------------------*/

int SendPipeText(HFILE hPipe, char *pchText)
{
   ULONG written;

   DosWrite(hPipe, pchText, strlen(pchText), &written);
   DosWrite(hPipe, S_ETX, 1, &written);

   return 0;
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: GetAllParams                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Liefert einen Zeiger auf die Programmparameter zurueck      */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: argc: Anzahl der main-Parameter                                */
/*            argv: main-Parameter                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Rckgabewerte: Zeiger auf String                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

char *GetAllParams(int argc, char **argv)
{
   int i;
   int size=1;
   char *all;

   if (argc < 2)
      return "";
   else
   {
      for (i=1; i<argc; i++)
         size+=strlen(argv[i])+1;
      all=malloc(size);
      all[0]=0;
      for (i=1; i<argc; i++)
      {
         if (i>1)
            strcat(all, " ");
         strcat(all, argv[i]);
      }
      return all;
   }
}

/*-------------------------------- Modulende --------------------------------*/


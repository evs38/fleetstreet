/*---------------------------------------------------------------------------+
 | Titel: Build Install File                                                 |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 03.10.1994                  |
 +-----------------------------------------+---------------------------------+
 | System: ANSI                                                              |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |   Erstellt ein INSTALL.FIL aus den Files                                  |
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
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>

/*--------------------------------- Defines ---------------------------------*/

#define COPYBUFFER  65536

#define FILETYPE_NONE      0
#define FILETYPE_FILE      1
#define FILETYPE_SCRIPT    2
#define FILETYPE_LANG      3
#define FILETYPE_HLP       4
#define FILETYPE_READ      5
#define FILETYPE_DOC       6
#define FILETYPE_OBSFILE   7

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

static char *pchListFile="FILES.LST";
static char *pchOutputFile="INSTALL.FIL";
static unsigned long ulFormatID = 1150UL;

char *pchTokens[]={"File",
                   "Script",
                   "Lang",
                   "Help",
                   "Readme",
                   "Doc",
                   "ObsFile",
                   NULL};

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

int CopyAllFiles(FILE *pfListFile, FILE *pfOutFile);
int CopyOneFile(char *pchFileName, FILE *pfOutFile, unsigned long ulFileLen, unsigned long ulFlags);
unsigned long TokenType(char *pchToken);
int QueryFileDate(const char *pchFileName, PFDATE pDate, PFTIME pTime);

/*------------------------------- main --------------------------------------*/

int main(int argc, char **argv)
{
   FILE *pfListFile;
   FILE *pfOutFile;
   int rc=0;
   unsigned long ulTemp=FILETYPE_NONE;

   if (argc >= 2)
      pchListFile = argv[1];

   if (argc >= 3)
      pchOutputFile = argv[2];

   printf("Generating %s from %s\n\n", pchOutputFile, pchListFile);

   if (pfListFile=fopen(pchListFile, "r"))
   {
      if (pfOutFile=fopen(pchOutputFile, "wb"))
      {
         /* Kennung schreiben */
         fwrite(&ulFormatID, sizeof(ulFormatID), 1, pfOutFile);

         rc=CopyAllFiles(pfListFile, pfOutFile);

         /* Ende-Kennung schreiben */
         fwrite(&ulTemp, sizeof(ulTemp), 1, pfOutFile);

         fclose(pfOutFile);
         fclose(pfListFile);

         return rc;
      }
      else
      {
         fclose(pfListFile);
         printf("ERROR: Could not open output file\n");
         return 1;
      }
   }
   else
   {
      printf("ERROR: Could not open list file.\n");
      return 1;
   }
}

/*---------------------------------------------------------------------------*/
/* Funktionsname: CopyAllFiles                                               */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Beschreibung: Kopiert alle Files                                          */
/*                                                                           */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Parameter: pfListFile: Textfile mit Fileliste                             */
/*            pfOutFile:  File f. Ausgabe                                    */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* RÅckgabewerte: 0 OK                                                       */
/*                -1 Fehler                                                  */
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* Sonstiges:                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int CopyAllFiles(FILE *pfListFile, FILE *pfOutFile)
{
   char pchZeile[300];
   char *pchName;
   struct stat stat;
   char *pchToken;
   char *pchFileName;
   char *pchParam;
   char *pchHelp;
   unsigned long ulFileType;

   while(!feof(pfListFile))
   {
      /* Filenamen holen */
      if (fgets(pchZeile, sizeof(pchZeile), pfListFile))
      {
         pchToken = strtok(pchZeile, " \t\n");

         if (!pchToken || pchToken[0] == ';')
            continue;

         pchFileName = strtok(NULL, " \t\n");
         pchParam = strtok(NULL, " \t\n");

         /* Filenamen isolieren u. schreiben */
         pchName=strrchr(pchFileName, '\\');
         if (!pchName)
            pchName=pchFileName;
         else
            pchName++;

         switch(ulFileType=TokenType(pchToken))
         {
            case FILETYPE_FILE:
               if (!_stat(pchFileName, &stat))
                  printf("INFO: File: %s; Length: %d\n", pchName, stat.st_size);
               else
               {
                  printf("ERROR: Could not find file %s\n", pchFileName);
                  return 1;
               }

               fwrite(&ulFileType, sizeof(ulFileType), 1, pfOutFile);
               fputs(pchName, pfOutFile);
               fputc(0, pfOutFile);
               CopyOneFile(pchFileName, pfOutFile, stat.st_size, pchParam?1:0);
               break;

            case FILETYPE_SCRIPT:
               if (!_stat(pchFileName, &stat))
                  printf("INFO: File: %s; Length: %d\n", pchName, stat.st_size);
               else
               {
                  printf("ERROR: Could not find file %s\n", pchFileName);
                  return 1;
               }

               fwrite(&ulFileType, sizeof(ulFileType), 1, pfOutFile);
               fputs(pchName, pfOutFile);
               fputc(0, pfOutFile);
               pchHelp = pchParam;
               while (*pchHelp)
                  if (*pchHelp == '_')
                     *pchHelp++ = ' ';
                  else
                     pchHelp++;
               fputs(pchParam, pfOutFile);
               fputc(0, pfOutFile);
               CopyOneFile(pchFileName, pfOutFile, stat.st_size, 0);
               break;

            case FILETYPE_LANG:
            case FILETYPE_HLP:
            case FILETYPE_DOC:
               if (!_stat(pchFileName, &stat))
                  printf("INFO: File: %s; Length: %d\n", pchName, stat.st_size);
               else
               {
                  printf("ERROR: Could not find file %s\n", pchFileName);
                  return 1;
               }

               fwrite(&ulFileType, sizeof(ulFileType), 1, pfOutFile);
               fputs(pchParam, pfOutFile); /* Sprachen-Name */
               fputc(0, pfOutFile);
               CopyOneFile(pchFileName, pfOutFile, stat.st_size, 0);
               break;

            case FILETYPE_READ:
               if (!_stat(pchFileName, &stat))
                  printf("INFO: File: %s; Length: %d\n", pchName, stat.st_size);
               else
               {
                  printf("ERROR: Could not find file %s\n", pchFileName);
                  return 1;
               }

               fwrite(&ulFileType, sizeof(ulFileType), 1, pfOutFile);
               fputs(pchName, pfOutFile);
               fputc(0, pfOutFile);
               fputs(pchParam, pfOutFile); /* Sprachen-Name */
               fputc(0, pfOutFile);
               CopyOneFile(pchFileName, pfOutFile, stat.st_size, 0);
               break;

            case FILETYPE_OBSFILE:
               printf("INFO: ObsFile: %s\n", pchName);
               fwrite(&ulFileType, sizeof(ulFileType), 1, pfOutFile);
               fputs(pchName, pfOutFile);
               fputc(0, pfOutFile);
               break;

            default:
               printf("ERROR: Unknown token!\n");
               return -1;
         }

      }
   }
   return 0;
}

int CopyOneFile(char *pchFileName, FILE *pfOutFile, unsigned long ulFileLen, unsigned long ulFlags)
{
   FILE *pfSrcFile;
   void *pBuffer;
   size_t bytesread;
   unsigned long ulSum=0;
   FDATE FileDate;
   FTIME FileTime;

   if (QueryFileDate(pchFileName, &FileDate, &FileTime))
      return 1;

   fwrite(&FileDate, sizeof(FileDate), 1, pfOutFile);
   fwrite(&FileTime, sizeof(FileTime), 1, pfOutFile);

   /* Flags */
   fwrite(&ulFlags, sizeof(ulFlags), 1, pfOutFile);

   /* Source-File oeffnen */
   if (pfSrcFile=fopen(pchFileName, "rb"))
   {
      /* Puffer holen */
      pBuffer = malloc(COPYBUFFER);

      fwrite(&ulFileLen, sizeof(ulFileLen), 1, pfOutFile);

      while (bytesread = fread(pBuffer, 1, COPYBUFFER, pfSrcFile))
      {
         fwrite(pBuffer, 1, bytesread, pfOutFile);
         ulSum += bytesread;
      }

      free(pBuffer);
      fclose(pfSrcFile);

      if (ulSum != ulFileLen)
      {
         printf("ERROR: File size differs; Expected: %d, Actual: %d\n", ulFileLen, ulSum);
         return 1;
      }
      else
         return 0;
   }
   else
   {
      printf("ERROR: Could not open %s\n", pchFileName);
      return 1;
   }
}

unsigned long TokenType(char *pchToken)
{
   int i=0;

   while (pchTokens[i] && stricmp(pchToken, pchTokens[i]))
      i++;

   if (pchTokens[i])
      return i+1;
   else
      return 0;
}

int QueryFileDate(const char *pchFileName, PFDATE pDate, PFTIME pTime)
{
   FILESTATUS3 FileStatus;

   if (DosQueryPathInfo((char*)pchFileName, FIL_STANDARD,
                        &FileStatus, sizeof(FileStatus)))
      return -1;

   memcpy(pDate, &FileStatus.fdateLastWrite, sizeof(FDATE));
   memcpy(pTime, &FileStatus.ftimeLastWrite, sizeof(FTIME));

   return 0;
}

/*-------------------------------- Modulende --------------------------------*/


/* Analyse v. INSTALL.FIL */

#define INCL_BASE
#include <os2.h>
#include <stdio.h>

char *pchInstallFile="INSTALL.FIL";
char pchFileName[300];

int main(void)
{
   FILE *pfInstall;
   unsigned long ulFileID=0;
   unsigned long ulLen;
   unsigned long ulFlags;
   int i;
   FDATE FileDate;
   FTIME FileTime;

   if (pfInstall=fopen(pchInstallFile, "rb"))
   {
      /* Kennung lesen */
      fread(&ulFileID, sizeof(ulFileID), 1, pfInstall);
      printf("ID=%d\n", ulFileID);

      while(!feof(pfInstall))
      {
         fread(&ulFileID, sizeof(ulFileID), 1, pfInstall);

         if (ulFileID == 0)
         {
            printf("EOF\n");
            break;
         }
         else
            switch(ulFileID)
            {
               case 1: /* File */
                  printf("File: ");
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Name=%s; ", pchFileName);
                  break;

               case 2:
                  printf("Script: ");
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Name=%s; ", pchFileName);
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Desc=%s; ", pchFileName);
                  break;

               case 3:
                  printf("Language-File: ");
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Sprache=%s; ", pchFileName);
                  break;

               case 4:
                  printf("Help-File: ");
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Sprache=%s; ", pchFileName);
                  break;

               case 5:
                  printf("Readme: ");
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Name=%s; ", pchFileName);
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Sprache=%s; ", pchFileName);
                  break;

               case 6:
                  printf("Anleitung: ");
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Sprache=%s; ", pchFileName);
                  break;

               case 7:
                  i=0;
                  while(1)
                  {
                     pchFileName[i]=fgetc(pfInstall);
                     if (!pchFileName[i])
                        break;
                     i++;
                  }
                  printf("Obsoletes File: Name=%s\n", pchFileName);
                  continue;

            }

         fread(&FileDate, sizeof(FDATE), 1, pfInstall);
         fread(&FileTime, sizeof(FTIME), 1, pfInstall);

         printf("%d.%d.%d, ", FileDate.day, FileDate.month, FileDate.year+80);
         printf("%d:%d:%d, ", FileTime.hours, FileTime.minutes, FileTime.twosecs);

         fread(&ulFlags, sizeof(ulFlags), 1, pfInstall);
         printf("Flags: %d, ", ulFlags);

         fread(&ulLen, sizeof(ulLen), 1, pfInstall);
         printf("Len=%d\n", ulLen);

         fseek(pfInstall, ulLen, SEEK_CUR);
      }

      fclose(pfInstall);
      return 1;
   }
   else
   {
      printf("Can't open file\n");
      return 1;
   }
}

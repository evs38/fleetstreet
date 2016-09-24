#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "../main.h"
#include "../structs.h"
#include "../areaman/areaman.h"
#include "../cfgfile_interface.h"


USERDATAOPT UserData;
OUTBOUND Outbounds[MAX_ADDRESSES];
AREALIST List;
LOADEDCFGDLL LoadedDLL;

static int CFG_LoadDLL(PCHAR pchDLLName, PLOADEDCFGDLL pLoadedCfgDLL);

int main(int argc, char **argv)
{
   int i=0;
   int rc;
   AREADEFLIST *pArea;

   strcpy(UserData.username[0], "DefaultName");

   if (argc >= 2)
   {
      if (!(rc= CFG_LoadDLL(argv[1], &LoadedDLL)))
      {
         printf("\nCFG-Format: %d = %s\n\n", LoadedDLL.QueryFormatID(), LoadedDLL.QueryFormatName());

         if (argc >= 3)
         {
            rc = LoadedDLL.ReadCfgFile(argv[2], &UserData, Outbounds, &List, NULL, READCFG_ALL);

            if (!rc)
            {
               while (i < MAX_ADDRESSES && UserData.address[i][0])
               {
                  printf("Aka %d: %s\n", i, UserData.address[i]);
                  i++;
               }
               i=0;
               while (i < MAX_USERNAMES && UserData.username[i][0])
               {
                  printf("Name %d: %s\n", i, UserData.username[i]);
                  i++;
               }
               i=0;
               while (i < MAX_ADDRESSES && Outbounds[i].zonenum)
               {
                  printf("Outbound Zone %d: %s\n", Outbounds[i].zonenum, Outbounds[i].outbound);
                  i++;
               }

               pArea = List.pFirstArea;

               while (pArea)
               {
                  printf("\nArea: %s\n", pArea->areadata.areatag);
                  printf("    Desc: %s\n", pArea->areadata.areadesc);
                  printf("     Aka: %s\n", pArea->areadata.address);
                  printf("    Name: %s\n", pArea->areadata.username);
                  printf("    File: %s\n", pArea->areadata.pathfile);
                  printf("  Format: ");
                  switch(pArea->areadata.areaformat)
                  {
                     case AREAFORMAT_FTS:
                        printf("*.MSG\n");
                        break;
                     case AREAFORMAT_SQUISH:
                        printf("Squish\n");
                        break;
                     case AREAFORMAT_JAM:
                        printf("JAM\n");
                        break;
                     default:
                        printf("Unknown\n");
                        break;
                  }
                  printf("     Typ: ");
                  switch(pArea->areadata.areatype)
                  {
                     case AREATYPE_NET:
                        printf("Net\n");
                        break;
                     case AREATYPE_ECHO:
                        printf("Echo\n");
                        break;
                     case AREATYPE_LOCAL:
                        printf("Local\n");
                        break;
                     default:
                        printf("Unknown\n");
                        break;
                  }
                  printf("  Attrib: %08x\n", pArea->areadata.ulDefAttrib);
                  printf("     Opt: %08x\n", pArea->areadata.ulAreaOpt);
                  printf("   Flags: %08x\n", pArea->areadata.ulTempFlags);

                  pArea = pArea->next;
               }
            }
            else
            {
               switch(rc)
               {
                  case CFGFILE_OPEN:
                     printf("Error opening data file\n");
                     break;

                  case CFGFILE_READ:
                     printf("Error reading data\n");
                     break;

                  case CFGFILE_VERSION:
                     printf("Wrong CFG file version\n");
                     break;

                  case CFGFILE_GENDATA:
                     printf("Error in general CFG data\n");
                     break;

                  case CFGFILE_AREA:
                     printf("Error in area definition\n");
                     break;

                  case CFGFILE_NOAREA:
                     printf("No area defined\n");
                     break;

                  default:
                     printf("Read RC=%d\n", rc);
                     break;
               }
            }
         }

         DosFreeModule(LoadedDLL.hmodCfgDLL);

      }
      else
         printf("Load RC=%d\n", rc);
   }
   else
      printf("Usage: TEST <dllname> [<cfgfile>]\n");

   return 0;
}

/*-----------------------------------------------------------------------------
 | Funktionsname: CFG_LoadDLL
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Laedt eine CFG-DLL
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: pchDLLName: Name der DLL
 |            pLoadedCfgDLL: Lade-Block
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Rckgabewerte: LOADCFGDLL_OK          kein Fehler
 |                LOADCFGDLL_CANTLOAD    DLL nicht ladbar
 |                LOADCFGDLL_FUNCMISSING Funktion fehlt
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

static int CFG_LoadDLL(PCHAR pchDLLName, PLOADEDCFGDLL pLoadedCfgDLL)
{
   char fail[50]="";
   APIRET rc=0;

   if (!(rc = DosLoadModule(fail, sizeof(fail), pchDLLName, &pLoadedCfgDLL->hmodCfgDLL)))
   {
      if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_QUERYVER, NULL,
                                (PFN*)&pLoadedCfgDLL->QueryVer)))
      {
         if ((rc=pLoadedCfgDLL->QueryVer()) == CURRENT_CFGVER)
         {
            printf("Version: %08x\n", rc);

            if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_QUERYID, NULL,
                                  (PFN*)&pLoadedCfgDLL->QueryFormatID)))
               if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_QUERYNAME, NULL,
                                     (PFN*)&pLoadedCfgDLL->QueryFormatName)))
                  if (!(rc=DosQueryProcAddr(pLoadedCfgDLL->hmodCfgDLL, ORDINAL_READCFG, NULL,
                                        (PFN*)&pLoadedCfgDLL->ReadCfgFile)))
                     return LOADCFGDLL_OK;
                  else
                     printf("ReadCfg: RC=%d\n", rc);
               else
                  printf("QueryName: RC=%d\n", rc);
            else
               printf("QueryID: RC=%d\n", rc);
         }
         else
         {
            DosFreeModule(pLoadedCfgDLL->hmodCfgDLL);
            printf("Wrong Version: %08x!\n", rc);

            return LOADCFGDLL_VERSION;
         }
      }
      else
         printf("QueryVer: RC=%d\n", rc);

      /* Fehler */
      DosFreeModule(pLoadedCfgDLL->hmodCfgDLL);
      return LOADCFGDLL_FUNCMISSING;
   }
   else
   {
      printf("LoadModule: RC=%d\n", rc);
      return LOADCFGDLL_CANTLOAD;
   }
}

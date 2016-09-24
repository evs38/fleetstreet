/*---------------------------------------------------------------------------+
 | Titel: INIT.C                                                             |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 19.06.93                    |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x                                                          |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |  Initialisierung von Fleet Street,                                        |
 |  Area-Informationen                                                       |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Moegl. Verbesserungen:                                                    |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_BASE
#define INCL_PM
#define INCL_GPI
#define INCL_SPLDOSPRINT
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "resids.h"
#include "dialogids.h"
#include "structs.h"
#include "msgheader.h"
#include "areaman\areaman.h"
#include "areaman\folderman.h"
#include "init.h"
#include "utility.h"
#include "msglist.h"
#include "handlemsg\handlemsg.h"
#include "ccmanage.h"
#include "finddlg.h"
#include "markmanage.h"
#include "nlbrowser.h"

#include "cfgfile_interface.h"

#include "nickmanage.h"
#include "echomanager.h"
#include "templatedlg.h"
#include "printsetup.h"
#include "toolbarconfig.h"
#include "request_manage.h"
#include "dump\expt.h"

/*--------------------------------- Defines ---------------------------------*/

#define INIFILENAME      "FLTSTRT.INI"
#define CCFILENAME       "CCLISTS.INI"
#define AREAFILENAME     "AREAS.INI"
#define DOMAINFILENAME   "DOMAINS.INI"
#define TEMPLATEFILENAME "TEMPL.INI"
#define SCRIPTFILENAME   "SCRIPTS.INI"
#define NICKFILENAME     "NICKN.INI"
#define FOLDERFILENAME   "FOLDERS.INI"

#define RGB_GREY      0x00cccccc
#define RGB_DARKBLUE  0x00000080
#define RGB_DARKGREEN 0x00008000
#define SMA_FONT      "8.Helv"
#define MID_FONT      "10.Helv"
#define VIEW_FONT      "10.System Proportional"
#define VIEW_FONT_MONO "10.System Monospaced"
#define MAXLEN_STRING  1024

#define PROGVERSION "FleetStreet"
#define INIVERSION  "0.680"

/* Alte Strukturen zum Konvertieren */
#define O_INIVERSION13 "0.660"
#define O_INIVERSION14 "0.661"
#define O_INIVERSION15 "0.662"
#define O_INIVERSION16 "0.670"
#define O_INIVERSION17 "0.675"
#define O_INIVERSION18 "0.676"
#define O_INIVERSION19 "0.677"
#define O_INIVERSION20 "0.678"
#define O_INIVERSION21 "0.679"
#define O_PROGVERSION  "Fleet Street 0.2b"

/*---------------------------- Globale Variablen ----------------------------*/
extern AREALIST arealiste;
extern USERDATAOPT userdaten;
DIRTYFLAGS dirtyflags;
extern HMODULE hmodLang;

static char pchIniPath[LEN_PATHNAME+1]="";
PCFGDLL pCfgDLLs;

#pragma pack(1)
typedef struct
{
   char usertag[LEN_USERNAME+1];
   char username[LEN_USERNAME+1];
   char address[LEN_5DADDRESS+1];
   char subjectline[LEN_SUBJECT+1];
   char firstline[LEN_FIRSTLINE+1];
   UINT  private    :1;
   UINT  crash      :1;
   UINT  kill       :1;
   UINT  attach     :1;
   UINT  notemplate :1;
   UINT  request    :1;
} BOOKADDRESSOPT;

typedef struct
{
   char areatag[LEN_AREATAG+1];
   char areadesc[LEN_AREADESC+1];
   char address[LEN_5DADDRESS+1];
   char username[LEN_USERNAME+1];
   char pathfile[LEN_PATHNAME+1];
   ULONG ulTemplateID;
   SHORT areatype;
   UINT  isechoarea      :1; /* Echo oder Net */
   UINT  isexcluded      :1; /* exclude from reading */
   UINT  isfromcfg       :1; /* stammt aus SQUISH.CFG */
   UINT  islocal         :1; /* ist eine lokale Area */
   UINT  defaultprivate  :1; /* Default-Message-Attribute */
   UINT  defaultcrash    :1;
   UINT  defaulthold     :1;
   UINT  defaultkillsent :1;
   UINT  separator       :1; /* Trennlinie */
   UINT  eightbitascii   :1; /* Sonderzeichen erlaubt ? */
   UINT  res1            :1; /* Reserved 1 */
} OLD_AREADEFOPT;
#pragma pack()

/*--------------------------- Funktionsprototypen ---------------------------*/

static int QueryIniDir(char *pchPath);
int ReadIntlSetting(void);
static ULONG ReadIniProfile(HAB hab);
static ULONG InitAreas(HAB hab, char *CfgFileName, ULONG ReadCfg, ULONG ulCfgType);
static ULONG ReadIniAreas(HAB hab);
static void ImportNamesAndAddresses(USERDATAOPT *pSrc, USERDATAOPT *pDest);
static int ReadIniUser(HINI inifile);
static int SaveIniUser(HINI inifile);
static int ReadIniNick(HAB hab);
static int SaveIniNick(HAB hab);
static int ReadIniWinPos(HINI inifile);
static int SaveIniWinPos(HINI inifile);
static int ReadIniWinCol(HINI inifile);
static int SaveIniWinCol(HINI inifile);
static int ReadIniFonts(HINI inifile);
static int SaveIniFonts(HINI inifile);
static int ReadIniPaths(HINI inifile);
static int SaveIniPaths(HINI inifile);
static int ReadIniMisc(HINI inifile);
static int SaveIniMisc(HINI inifile);
static int ReadIniMacros(HINI inifile);
static int SaveIniMacros(HINI inifile);
static int ReadIniGeneral(HINI inifile);
static int SaveIniGeneral(HINI inifile);
static int ReadIniNodelist(HINI inifile);
static int SaveIniNodelist(HINI inifile);
static int ReadIniEchotoss(HINI inifile);
static int SaveIniEchotoss(HINI inifile);
static int ReadIniDomains(HAB hab);
static int SaveIniDomains(HAB hab);
static int ReadIniCCLists(HAB hab);
static int SaveIniCCLists(HAB hab);
static int ReadIniCCEntries(HINI ccini, char *pchApp, PCCLIST pList);
static int SaveIniCCEntries(HINI ccini, char *pchApp, PCCLIST pList);
static int ReadIniThreadopt(HINI inifile);
static int SaveIniThreadopt(HINI inifile);
static int ReadIniLookup(HINI inifile);
static int SaveIniLookup(HINI inifile);
static int ReadIniResults(HINI inifile);
static int SaveIniResults(HINI inifile);
static int ReadIniRequest(HINI inifile);
static int SaveIniRequest(HINI inifile);
static int ReadIniAreaList(HINI inifile);
static int SaveIniAreaList(HINI inifile);
static int ReadIniMsgList(HINI inifile);
static int SaveIniMsgList(HINI inifile);
static int SaveIniRemap(HINI inifile);
static int ReadIniRemap(HINI inifile);
static int SaveIniHooks(HINI inifile);
static int ReadIniHooks(HINI inifile);
static int SaveIniScripts(HAB hab);
static int ReadIniScripts(HAB hab);

static int ConvertProfile(HINI inifile, PCHAR pchThisVersion, HAB hab);
static void CopyAreaData(AREADEFLIST *zeiger, AREADEFOPT *Area);

static int ReadIniTemplates(HAB hab);
static int LoadEmptyTemplate(TEMPLATELIST *pTemplateList);
static int SaveIniTemplates(HAB hab);
static int ReadIniFind(HINI inifile);
static int SaveIniFind(HINI inifile);
static int ReadIniBrowse(HINI inifile);
static int SaveIniBrowse(HINI inifile);
static int ReadIniExport(HINI inifile);
static int SaveIniExport(HINI inifile);
static int ReadIniToolbar(HINI inifile);
static int SaveIniToolbar(HINI inifile);
static int ReadIniEchoman(HINI inifile);
static int SaveIniEchoman(HINI inifile);
static int ReadIniSearch(HINI inifile);
static int SaveIniSearch(HINI inifile);
static int ReadIniPrintSetup(HINI inifile);
static int SaveIniPrintSetup(HINI inifile);

static int ReadIniFolders(HAB hab);
static int SaveIniFolders(HAB hab);
static int AddDefaultFolder(PFOLDERANCHOR pFolderAnchor);

static BOOL HaveAddress(char *pchAddress);


/* Stub-Funktionen */
static BOOL QueryProfileData(HINI hini, PSZ pszApp, PSZ pszKey, PVOID pBuffer, PULONG pulBufferMax);
static LONG QueryProfileInt(HINI hini, PSZ pszApp, PSZ pszKey, LONG lDefault);
static BOOL QueryProfileSize(HINI hini, PSZ pszApp, PSZ pszKey, PULONG pDataLen);
static ULONG QueryProfileString(HINI hini, PSZ pszApp, PSZ pszKey, PSZ pszDefault, PVOID pBuffer,
                                ULONG cchBufferMax);
static BOOL WriteProfileData(HINI hini, PSZ pszApp, PSZ pszKey, PVOID pBuffer, ULONG cchBufferMax);
static BOOL WriteProfileString(HINI hini, PSZ pszApp, PSZ pszKey, PSZ pszData);
static BOOL WriteProfileInt(HINI hini, PSZ pszApp, PSZ pszKey, LONG lData);

/* Fehler */
static ULONG ProfileErrorMessage(HWND hwndOwner, ULONG ErrorType);
static ULONG CFGFileErrorMessage(HWND hwndOwner, ULONG ErrorType);
static ULONG CFGDLLErrorMessage(HWND hwndOwner, ULONG ErrorType);

/*------------------------------ ReadIniThread ------------------------------*/
/* Thread-Funktion, die die INIs liesst. Beim Ende wird eine Ende-Message    */
/* mit dem Ergebnis an die Queue gepostet (Thread-Parameter).                */
/*---------------------------------------------------------------------------*/

void _Optlink ReadIniThread(void *phev)
{
   extern PATHNAMES pathnames;
   extern int CurrentStatus;
   extern MISCOPTIONS miscoptions;
   extern ULONG ProfileError;
   extern MARKERLIST MarkerList;
   HAB hab;

   INSTALLEXPT("ReadINI");

   hab = WinInitialize(0);

   QueryIniDir(pchIniPath);

   pCfgDLLs = CFG_ReadFormatList();

   if (!(ProfileError=ReadIniProfile(hab)))
   {
      if (userdaten.address[0][0] && userdaten.username[0][0])
         CurrentStatus=PROGSTATUS_READING;

      /* Areas lesen */
      if (miscoptions.readcfg && pathnames.squishcfg[0])
         ProfileError = InitAreas(hab, pathnames.squishcfg,
                        READCFG_BOTH, miscoptions.ulCfgType);
      else
         ProfileError = InitAreas(hab, pathnames.squishcfg,
                        READCFG_INI, miscoptions.ulCfgType);

      CheckMarkerAreas(&MarkerList);
   }

   DosPostEventSem(*((PHEV)phev));

   WinTerminate(hab);

   DEINSTALLEXPT;

   return;
}

static int QueryIniDir(char *pchPath)
{
   ULONG ulDrive, ulMap;
   char pchTemp[LEN_PATHNAME+1];
   ULONG ulLen = LEN_PATHNAME+1;

   if (!pchPath[0])
   {
      strcpy(pchPath, "C:\\");
      DosQueryCurrentDisk(&ulDrive, &ulMap);
      *pchPath = (char) ulDrive + '@';

      DosQueryCurrentDir(0, pchTemp, &ulLen);
      strcat(pchPath, pchTemp);
      strcat(pchPath, "\\");
   }

   return 0;
}

int ParseArgs(int argc, char **argv)
{
   int i;
   int len;

   for (i=1; i<argc; i++)
      if (argv[i][0] == '-')
         switch(argv[i][1])
         {
            case 'c':
            case 'C':
               strcpy(pchIniPath, &argv[i][2]);
               len = strlen(pchIniPath);
               if (len && pchIniPath[len-1] != '\\')
                  strcat(pchIniPath, "\\");
               break;

            default:
               return 1;
         }

   return 0;
}


/*------------------------------ InitAreas ----------------------------------*/
/* InitAreas, liesst Area-Informationen aus der SQUISH.CFG und aus dem       */
/* INI-File, baut die Area-Liste auf.                                        */
/* Rueckgabewerte: LSB SHORT: Fehlercode des CFG-Files                       */
/*                 MSB SHORT: Fehlercode des INI-Files                       */
/*---------------------------------------------------------------------------*/

static ULONG InitAreas(HAB hab, char *CfgFileName, ULONG ReadCfg, ULONG ulCfgType)
{
   ULONG RetValue=0;
   extern DRIVEREMAP driveremap;
   extern OUTBOUND outbound[MAX_ADDRESSES];
   PCFGDLL pFormatDLL;
   USERDATAOPT CfgUser;

   if (ReadCfg & READCFG_CFG)
   {
      pFormatDLL = CFG_FindFormat(pCfgDLLs, ulCfgType, NULL);

      if (pFormatDLL)
      {
         LOADEDCFGDLL LoadedDLL;

         if (!(RetValue = CFG_LoadDLL(pFormatDLL->pchDLLName, &LoadedDLL)))
         {
            memset(&CfgUser, 0, sizeof(CfgUser));
            strcpy(CfgUser.username[0], userdaten.username[0]);

            RetValue = LoadedDLL.ReadCfgFile(CfgFileName, &CfgUser, outbound, &arealiste, &driveremap, READCFG_ALL);

            if (RetValue)
               RetValue = CFGFILEERROR(RetValue);

            DosFreeModule(LoadedDLL.hmodCfgDLL);

            ImportNamesAndAddresses(&CfgUser, &userdaten);
         }
         else
            return CFGDLLERROR(RetValue);
      }
      else
         return CFGDLLERROR(LOADCFGDLL_UNSUPPFORMAT);

   }

   if (!RetValue && (ReadCfg & READCFG_INI))
      RetValue = ReadIniAreas(hab);

   return RetValue;
}

static void ImportNamesAndAddresses(USERDATAOPT *pSrc, USERDATAOPT *pDest)
{
   int i=0;
   int j;

   /* Namen */
   while (i < MAX_USERNAMES && pSrc->username[i][0])
   {
      j=0;

      while (j < MAX_USERNAMES &&
             pDest->username[j][0] &&
             strcmp(pSrc->username[i], pDest->username[j]))
         j++;

      if (j < MAX_USERNAMES && !pDest->username[j][0])
         strcpy(pDest->username[j], pSrc->username[i]);

      i++;
   }

   /* Adressen */
   i=0;
   while (i < MAX_ADDRESSES && pSrc->address[i][0])
   {
      j=0;

      while (j < MAX_ADDRESSES &&
             pDest->address[j][0] &&
             strcmp(pSrc->address[i], pDest->address[j]))
         j++;

      if (j < MAX_ADDRESSES && !pDest->address[j][0])
         strcpy(pDest->address[j], pSrc->address[i]);

      i++;
   }

   /* Origin */
   if (!pDest->defaultorigin[0])
      strcpy(pDest->defaultorigin, pSrc->defaultorigin);

   return;
}

/*----------------------------- ReReadAreas ---------------------------------*/
/* Liesst die Areas bei einem SQUISH.CFG-Wechsel neu ein.                    */
/* Rueckgabewerte: 0 kein Fehler                                             */
/*                 1 Fehler beim Einlesen                                    */
/*                 2 Areas nicht eindeutig                                   */
/*---------------------------------------------------------------------------*/

ULONG ReReadAreas(HAB hab, char *CfgFileName, ULONG ulCfgType)
{
   AREADEFLIST *zeiger=NULL;
   AREADEFLIST *zeiger2=NULL;
   AREALIST saveareas;
   AREALIST privareas;
   AREADEFLIST *pPriv;
   ULONG rc;

   /* alte Areas retten */
   saveareas=arealiste;

   memset(&arealiste, 0, sizeof(arealiste));
   if (rc= InitAreas(hab, CfgFileName, READCFG_CFG, ulCfgType))
   {
      /* Fehler beim Einlesen */
      AM_DeleteAllAreas(&arealiste);
      arealiste=saveareas;
      return rc;
   }
   else
   {
      /* ohne Fehler */
      privareas=saveareas;

      /* private areas suchen */
      pPriv = privareas.pFirstArea;
      while(pPriv && (pPriv->areadata.ulAreaOpt & AREAOPT_FROMCFG))
         pPriv=pPriv->next;
      privareas.pFirstArea = pPriv;

      if (pPriv)
      {
         /* Duplikate suchen */
         zeiger=arealiste.pFirstArea;
         while (zeiger)
         {
            if (AM_FindArea(&privareas, zeiger->areadata.areatag))
            {
               /* Area doppelt */
               AM_DeleteAllAreas(&arealiste);
               arealiste=saveareas;

               return INIERROR(INIFILE_DUPAREAS);
            }
            zeiger=zeiger->next;
         }
      }

      /* ohne Fehler */
      /* Areadaten abgleichen */
      zeiger=arealiste.pFirstArea;
      while(zeiger)
      {
         zeiger2=AM_FindArea(&saveareas, zeiger->areadata.areatag);
         if (zeiger2)
         {
            /* alte Daten bernehmen */
            strcpy(zeiger->areadata.areadesc, zeiger2->areadata.areadesc);
            strcpy(zeiger->areadata.username, zeiger2->areadata.username);
            zeiger->areadata.ulTemplateID= zeiger2->areadata.ulTemplateID;
            zeiger->areadata.ulFolderID= zeiger2->areadata.ulFolderID;
            zeiger->areadata.ulAreaOpt &= AREAOPT_FROMCFG;
            zeiger->areadata.ulAreaOpt |= (zeiger2->areadata.ulAreaOpt & ~AREAOPT_FROMCFG);
            zeiger->areadata.ulDefAttrib= zeiger2->areadata.ulDefAttrib;
            zeiger->areadata.ulTempFlags= zeiger2->areadata.ulTempFlags;
            zeiger->areadata.areaformat= zeiger2->areadata.areaformat;
            zeiger->areadata.areatype= zeiger2->areadata.areatype;

            zeiger->dirty=TRUE;
            zeiger->flWork= WORK_NOTHING;
            zeiger->bLocked= FALSE;
         }
         zeiger=zeiger->next;
      }

      /* alte CFG-Areas loeschen */
      while (saveareas.pFirstArea)
      {
         zeiger=saveareas.pFirstArea;

         while(zeiger && !(zeiger->areadata.ulAreaOpt & AREAOPT_FROMCFG))
            zeiger = zeiger->next;
         if (zeiger)
            AM_DeleteAreaDirect(&saveareas, zeiger);
         else
            break;
      }

      /* alte Liste anhaengen */
      AM_MergeAreas(&arealiste, &saveareas);

      arealiste.bDirty = TRUE;

      return INIERROR(INIFILE_OK);
   }
}

/*------------------------------ ReadIniAreas -------------------------------*/
/* Liesst die Areainformationen aus dem INI-File, fuegt evtl. die Areas hinzu*/
/*---------------------------------------------------------------------------*/

static ULONG ReadIniAreas(HAB hab)
{
   HINI inifile;
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */
   AREADEFLIST *zeiger;       /* Hilfszeiger */
   AREADEFOPT Area;
   ULONG ulDataLen;
   char pchIniFile[LEN_PATHNAME+1];
   BOOL bMarkDirty = FALSE;
   extern FOLDERANCHOR FolderAnchor;

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, AREAFILENAME);

   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
      return INIERROR(INIFILE_OPEN);

   if (!QueryProfileSize(inifile, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
   {
      PrfCloseProfile(inifile);
      return 0; /* INIERROR(INIFILE_OPEN); nicht ausgeben, da auch
                                           bei leerem File */
   }

   pchKeys=malloc(ulKeysLen);

   if (!QueryProfileData(inifile, NULL, NULL, pchKeys, &ulKeysLen))
   {
      free(pchKeys);
      PrfCloseProfile(inifile);
      return INIERROR(INIFILE_OPEN);
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      /* Areadaten lesen */
      memset(&Area, 0, sizeof(Area));

      strcpy(Area.areatag, pchCurrentKey);
      QueryProfileString(inifile, pchCurrentKey, "Desc", NULL, Area.areadesc, LEN_AREADESC+1);
      QueryProfileString(inifile, pchCurrentKey, "Name", NULL, Area.username, LEN_USERNAME+1);

      ulDataLen=sizeof(ULONG);
      QueryProfileData(inifile, pchCurrentKey, "Template", &Area.ulTemplateID, &ulDataLen);

      ulDataLen=sizeof(ULONG);
      QueryProfileData(inifile, pchCurrentKey, "Options", &Area.ulAreaOpt, &ulDataLen);
      ulDataLen=sizeof(ULONG);
      QueryProfileData(inifile, pchCurrentKey, "Attrib", &Area.ulDefAttrib, &ulDataLen);

      QueryProfileString(inifile, pchCurrentKey, "Address", NULL, Area.address, LEN_5DADDRESS+1);

      Area.ulFolderID = QueryProfileInt(inifile, pchCurrentKey, "Folder", 0);

      if (!FM_FindFolder(&FolderAnchor, Area.ulFolderID))
         /* folder gibt's nicht mehr, in den root-Folder */
         Area.ulFolderID = 0;

      if (!(Area.ulAreaOpt & AREAOPT_FROMCFG))
      {
         ulDataLen=sizeof(USHORT);
         QueryProfileData(inifile, pchCurrentKey, "Format", &Area.areaformat, &ulDataLen);
         ulDataLen=sizeof(Area.areatype);
         Area.areatype = AREATYPE_LOCAL;
         QueryProfileData(inifile, pchCurrentKey, "Type", &Area.areatype, &ulDataLen);
         QueryProfileString(inifile, pchCurrentKey, "Path", NULL, Area.pathfile, LEN_PATHNAME+1);
      }

      /* prfen ob Adresse noch im Setup */
      if (!HaveAddress(Area.address))
      {
         /* nicht mehr da, Default nehmen */
         strcpy(Area.address, userdaten.address[0]);
         bMarkDirty = TRUE;
      }
      else
         bMarkDirty = FALSE;

      /* Area suchen */
      zeiger=AM_FindArea(&arealiste, Area.areatag);

      if (zeiger)
      {
         /* Area gefunden */
         /* Daten uebernehmen */
         CopyAreaData(zeiger, &Area);
         if (bMarkDirty)
            zeiger->dirty = TRUE;
      }
      else
      {
         /* Area nicht gefunden */
         if (!(Area.ulAreaOpt & AREAOPT_FROMCFG))     /* Eintrag aus alter CFG, verwerfen */
         {
            /* Eintrag hinzufuegen */
            AM_AddArea(&arealiste, &Area, ADDAREA_TAIL | (bMarkDirty ? ADDAREA_MARKDIRTY : 0));
         }
      }
      /* Naechsten Key suchen */
      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);
   PrfCloseProfile(inifile);

   return 0;
}

static BOOL HaveAddress(char *pchAddress)
{
   int i=0;

   while (i<MAX_ADDRESSES &&
          strcmp(userdaten.address[i], pchAddress))
      i++;
   if (i<MAX_ADDRESSES)
      return TRUE;
   else
      return FALSE;
}

/*------------------------------ CopyAreaData -------------------------------*/
/* Kopiert die Area-Informationen in ein Listenelement                       */
/*---------------------------------------------------------------------------*/

static  void CopyAreaData(AREADEFLIST *zeiger, AREADEFOPT *Area)
{
   strcpy(zeiger->areadata.areadesc, Area->areadesc);
   strcpy(zeiger->areadata.username, Area->username);
   if (Area->address[0])
      strcpy(zeiger->areadata.address, Area->address);
   zeiger->areadata.ulTemplateID=Area->ulTemplateID;
   zeiger->areadata.ulFolderID=Area->ulFolderID;
   zeiger->areadata.ulAreaOpt &= AREAOPT_FROMCFG;
   zeiger->areadata.ulAreaOpt |= (Area->ulAreaOpt & ~AREAOPT_FROMCFG);
   zeiger->areadata.ulDefAttrib=Area->ulDefAttrib;
   zeiger->areadata.ulTempFlags=Area->ulTempFlags;

   return;
}

/*------------------------------ SaveIniAreas -------------------------------*/
/* Speichert die Areainformationen in dem INI-File                           */
/*---------------------------------------------------------------------------*/

int SaveIniAreas(HAB hab)
{
   HINI inifile;
   AREADEFLIST *zeiger;       /* Hilfszeiger */
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, AREAFILENAME);

   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
      return 1;

   zeiger=arealiste.pFirstArea;

   while (zeiger)
   {
      if (zeiger->dirty)
      {
         WriteProfileString(inifile, zeiger->areadata.areatag, "Desc", zeiger->areadata.areadesc);
         WriteProfileString(inifile, zeiger->areadata.areatag, "Name", zeiger->areadata.username);
         WriteProfileData(inifile, zeiger->areadata.areatag, "Template", &(zeiger->areadata.ulTemplateID),
                             sizeof(ULONG));
         WriteProfileData(inifile, zeiger->areadata.areatag, "Options", &(zeiger->areadata.ulAreaOpt),
                             sizeof(ULONG));
         WriteProfileData(inifile, zeiger->areadata.areatag, "Attrib", &(zeiger->areadata.ulDefAttrib),
                             sizeof(ULONG));
         WriteProfileString(inifile, zeiger->areadata.areatag, "Address", zeiger->areadata.address);
         if (zeiger->areadata.ulFolderID)
            WriteProfileInt(inifile, zeiger->areadata.areatag, "Folder", zeiger->areadata.ulFolderID);
         else
            WriteProfileString(inifile, zeiger->areadata.areatag, "Folder", NULL);
         if (!(zeiger->areadata.ulAreaOpt & AREAOPT_FROMCFG))
         {
            WriteProfileData(inifile, zeiger->areadata.areatag, "Format", &(zeiger->areadata.areaformat),
                             sizeof(USHORT));
            WriteProfileData(inifile, zeiger->areadata.areatag, "Type", &(zeiger->areadata.areatype),
                             sizeof(zeiger->areadata.areatype));
            WriteProfileString(inifile, zeiger->areadata.areatag, "Path", zeiger->areadata.pathfile);
         }

         zeiger->dirty=FALSE;
      }
      zeiger=zeiger->next;
   }

   /* ueberfluessige Areas im INI loeschen */
   if (!QueryProfileSize(inifile, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
   {
      PrfCloseProfile(inifile);
      return 1;
   }

   pchKeys=malloc(ulKeysLen);

   if (!QueryProfileData(inifile, NULL, NULL, pchKeys, &ulKeysLen))
   {
      free(pchKeys);
      PrfCloseProfile(inifile);
      return 1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      zeiger=arealiste.pFirstArea;

      /* Case-Sensitive suchen */
      while (zeiger && strcmp(pchCurrentKey, zeiger->areadata.areatag))
         zeiger = zeiger->next;
      if (!zeiger)
         WriteProfileData(inifile, pchCurrentKey, NULL, NULL, 0);

      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);

   PrfCloseProfile(inifile);

   arealiste.bDirty = FALSE;

   return 0;
}

/*------------------------------ ReadIniTemplates ---------------------------*/
/* Liesst die Templates aus dem INI-File                                     */
/*---------------------------------------------------------------------------*/

static  int ReadIniTemplates(HAB hab)
{
   extern TEMPLATELIST templatelist;
   HINI inifile;
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */
   ULONG ulDataLen;
   ULONG datalen;
   PMSGTEMPLATE pNewTemplate, pTemplate=NULL;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, TEMPLATEFILENAME);

   memset(&templatelist, 0, sizeof(TEMPLATELIST));

   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
   {
      LoadEmptyTemplate(&templatelist);
      return 1;
   }

   if (!QueryProfileSize(inifile, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
   {
      PrfCloseProfile(inifile);
      LoadEmptyTemplate(&templatelist);
      return 1;
   }

   pchKeys=malloc(ulKeysLen);

   if (!QueryProfileData(inifile, NULL, NULL, pchKeys, &ulKeysLen))
   {
      free(pchKeys);
      PrfCloseProfile(inifile);
      LoadEmptyTemplate(&templatelist);
      return 1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      if (!strcmp(pchCurrentKey, "Pos"))
      {
         /* Folder-Position */
         ulDataLen = sizeof(templatelist.FolderPos);
         QueryProfileData(inifile, "Pos", "Pos", &templatelist.FolderPos, &ulDataLen);
         ulDataLen = sizeof(templatelist.ulFlags);
         QueryProfileData(inifile, "Pos", "Flags", &templatelist.ulFlags, &ulDataLen);
      }
      else
      {
         /* neues Template */
         pNewTemplate = malloc(sizeof(MSGTEMPLATE));
         memset(pNewTemplate, 0, sizeof(MSGTEMPLATE));

         /* hinten anhaengen */
         if (pTemplate)
         {
            pTemplate->next = pNewTemplate;
            pNewTemplate->prev = pTemplate;
         }
         else
         {
            /* erstes Template */
            templatelist.pTemplates = pNewTemplate;
         }
         pTemplate = pNewTemplate;
         templatelist.ulNumTemplates++;

         /* Key ist ID, umwandeln */
         pNewTemplate->ulID = strtoul(pchCurrentKey, NULL, 10);

         /* restliche Daten */
         datalen=sizeof(pNewTemplate->TPos);
         if (!inifile || !QueryProfileData(inifile, pchCurrentKey, "Pos",
                                              &pNewTemplate->TPos, &datalen))
         {
            memset(&pNewTemplate->TPos, 0, sizeof(pNewTemplate->TPos));
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }

         datalen=sizeof(pNewTemplate->quotelinelen);
         if (!inifile || !QueryProfileData(inifile, pchCurrentKey, "Linelen",
                                              &pNewTemplate->quotelinelen, &datalen))
         {
            pNewTemplate->quotelinelen=75;
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }

         datalen=sizeof(pNewTemplate->joinlen);
         if (!inifile || !QueryProfileData(inifile, pchCurrentKey, "Joinlen",
                                              &pNewTemplate->joinlen, &datalen))
         {
            pNewTemplate->joinlen=65;
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }

         datalen=sizeof(pNewTemplate->useinitials);
         if (!inifile || !QueryProfileData(inifile, pchCurrentKey, "UseInitials",
                                              &pNewTemplate->useinitials, &datalen))
         {
            pNewTemplate->useinitials=TRUE;
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }

         datalen=sizeof(pNewTemplate->forwardfirst);
         if (!inifile || !QueryProfileData(inifile, pchCurrentKey, "ForwardOrder",
                                              &pNewTemplate->forwardfirst, &datalen))
         {
            pNewTemplate->forwardfirst=FALSE;
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }

         datalen=sizeof(pNewTemplate->randomorigin);
         if (!inifile || !QueryProfileData(inifile, pchCurrentKey, "Random",
                                              &pNewTemplate->randomorigin, &datalen))
         {
            pNewTemplate->randomorigin=FALSE;
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "Name", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TName=malloc(datalen);
            LoadString(IDST_TPL_DEFNAME, datalen, pNewTemplate->TName);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TName=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "Name", NULL, pNewTemplate->TName,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "Header", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->THeader=malloc(datalen);
            LoadString(IDST_TPL_HEADER, datalen, pNewTemplate->THeader);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->THeader=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "Header", NULL, pNewTemplate->THeader,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "Footer", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TFooter=malloc(datalen);
            LoadString(IDST_TPL_FOOTER, datalen, pNewTemplate->TFooter);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TFooter=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "Footer", NULL, pNewTemplate->TFooter,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "Reply", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TReply=malloc(datalen);
            LoadString(IDST_TPL_REPLY, datalen, pNewTemplate->TReply);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TReply=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "Reply", NULL, pNewTemplate->TReply,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "DArea", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TDArea=malloc(datalen);
            LoadString(IDST_TPL_DAREA, datalen, pNewTemplate->TDArea);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TDArea=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "DArea", NULL, pNewTemplate->TDArea,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "Forward", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TForward=malloc(datalen);
            LoadString(IDST_TPL_FORWARD, datalen, pNewTemplate->TForward);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TForward=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "Forward", NULL, pNewTemplate->TForward,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "ForwardFooter", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TForwardFooter=malloc(datalen);
            LoadString(IDST_TPL_FORWARDFOOTER, datalen, pNewTemplate->TForwardFooter);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TForwardFooter=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "ForwardFooter", NULL, pNewTemplate->TForwardFooter,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "XPost", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TXPost=malloc(datalen);
            LoadString(IDST_TPL_XPOST, datalen, pNewTemplate->TXPost);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TXPost=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "XPost", NULL, pNewTemplate->TXPost,
                                  datalen);
         }

         if (!inifile || !QueryProfileSize(inifile, pchCurrentKey, "CCopy", &datalen) ||
             datalen==0)
         {
            datalen=MAXLEN_STRING;
            pNewTemplate->TCCopy=malloc(datalen);
            LoadString(IDST_TPL_CCOPY, datalen, pNewTemplate->TCCopy);
            pNewTemplate->bDirty = TRUE;
            templatelist.bDirty=TRUE;
         }
         else
         {
            pNewTemplate->TCCopy=malloc(datalen);
            QueryProfileString(inifile, pchCurrentKey, "CCopy", NULL, pNewTemplate->TCCopy,
                                  datalen);
         }

         if (inifile)
         {
            QueryProfileString(inifile, pchCurrentKey, "Origin", "", pNewTemplate->TOrigin, LEN_ORIGIN+1);
            QueryProfileString(inifile, pchCurrentKey, "OriginFile", "", pNewTemplate->TOriginFile, LEN_PATHNAME+1);
            QueryProfileString(inifile, pchCurrentKey, "AllSyn", "", pNewTemplate->TAllSyn, LEN_USERNAME+1);
            datalen = sizeof(pNewTemplate->chQuoteChar);
            pNewTemplate->chQuoteChar = '>';
            QueryProfileData(inifile, pchCurrentKey, "QChar", &pNewTemplate->chQuoteChar, &datalen);
         }
      }

      /* Naechsten Key suchen */
      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   if (!templatelist.pTemplates)
      LoadEmptyTemplate(&templatelist);

   free(pchKeys);
   PrfCloseProfile(inifile);
   return 0;
}

/*------------------------------ LoadEmptyTemplate --------------------------*/
/* Erzeugt das Default-Template an der Spitze mit den Daten aus den Resourcen */
/*---------------------------------------------------------------------------*/

static int LoadEmptyTemplate(TEMPLATELIST *pTemplateList)
{
   PMSGTEMPLATE pNewTemplate;

   /* neues Template */
   pNewTemplate = calloc(1, sizeof(MSGTEMPLATE));

   /* erstes Template */
   pTemplateList->pTemplates = pNewTemplate;
   pTemplateList->ulNumTemplates++;
   pTemplateList->bDirty = TRUE;

   /* Default setzen */
   LoadDefaultTemplate(pNewTemplate);
   pNewTemplate->ulID = 0;

   pNewTemplate->TName=malloc(MAXLEN_STRING);
   LoadString(IDST_TPL_DEFNAME, MAXLEN_STRING, pNewTemplate->TName);

   return 0;
}

/*------------------------------ SaveIniTemplates ---------------------------*/
/* Speichert die Templates in dem INI-File                                   */
/*---------------------------------------------------------------------------*/

static  int SaveIniTemplates(HAB hab)
{
   extern TEMPLATELIST templatelist;
   HINI inifile;
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */
   PMSGTEMPLATE pTemplate;
   char pchKey[20];
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, TEMPLATEFILENAME);

   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
      return 1;

   WriteProfileData(inifile, "Pos", "Pos", &(templatelist.FolderPos),
                       sizeof(templatelist.FolderPos));
   WriteProfileData(inifile, "Pos", "Flags", &(templatelist.ulFlags),
                       sizeof(templatelist.ulFlags));

   pTemplate = templatelist.pTemplates;

   while (pTemplate)
   {
      if (pTemplate->bDirty)
      {
         _itoa(pTemplate->ulID, pchKey, 10);

         WriteProfileData(inifile, pchKey, "Pos", &pTemplate->TPos,
                             sizeof(pTemplate->TPos));
         WriteProfileData(inifile, pchKey, "Linelen", &pTemplate->quotelinelen,
                             sizeof(pTemplate->quotelinelen));
         WriteProfileData(inifile, pchKey, "Joinlen", &pTemplate->joinlen,
                             sizeof(pTemplate->joinlen));
         WriteProfileData(inifile, pchKey, "UseInitials", &pTemplate->useinitials,
                             sizeof(pTemplate->useinitials));
         WriteProfileData(inifile, pchKey, "ForwardOrder", &pTemplate->forwardfirst,
                             sizeof(pTemplate->forwardfirst));
         WriteProfileData(inifile, pchKey, "Random", &pTemplate->randomorigin,
                             sizeof(pTemplate->randomorigin));

         if (pTemplate->TName)
            WriteProfileString(inifile, pchKey, "Name", pTemplate->TName);
         if (pTemplate->THeader)
            WriteProfileString(inifile, pchKey, "Header", pTemplate->THeader);
         if (pTemplate->TFooter)
            WriteProfileString(inifile, pchKey, "Footer", pTemplate->TFooter);
         if (pTemplate->TReply)
            WriteProfileString(inifile, pchKey, "Reply", pTemplate->TReply);
         if (pTemplate->TDArea)
            WriteProfileString(inifile, pchKey, "DArea", pTemplate->TDArea);
         if (pTemplate->TForward)
            WriteProfileString(inifile, pchKey, "Forward", pTemplate->TForward);
         if (pTemplate->TForwardFooter)
            WriteProfileString(inifile, pchKey, "ForwardFooter", pTemplate->TForwardFooter);
         if (pTemplate->TXPost)
            WriteProfileString(inifile, pchKey, "XPost", pTemplate->TXPost);
         if (pTemplate->TCCopy)
            WriteProfileString(inifile, pchKey, "CCopy", pTemplate->TCCopy);
         WriteProfileString(inifile, pchKey, "Origin", pTemplate->TOrigin);
         WriteProfileString(inifile, pchKey, "OriginFile", pTemplate->TOriginFile);
         WriteProfileString(inifile, pchKey, "AllSyn", pTemplate->TAllSyn);
         WriteProfileData(inifile, pchKey, "QChar", &pTemplate->chQuoteChar,
                             sizeof(pTemplate->chQuoteChar));

         pTemplate->bDirty=FALSE;
      }
      pTemplate = pTemplate->next;
   }

   /* ueberfluessige Areas im INI loeschen */
   if (!QueryProfileSize(inifile, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
   {
      PrfCloseProfile(inifile);
      return 1;
   }

   pchKeys=malloc(ulKeysLen);

   if (!QueryProfileData(inifile, NULL, NULL, pchKeys, &ulKeysLen))
   {
      free(pchKeys);
      PrfCloseProfile(inifile);
      return 1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      if (strcmp(pchCurrentKey, "Pos"))
      {
         ULONG ulID;
         PMSGTEMPLATE pTemplate;

         ulID = strtoul(pchCurrentKey, NULL, 10);
         pTemplate = templatelist.pTemplates;
         while (pTemplate && pTemplate->ulID != ulID)
            pTemplate = pTemplate->next;

         if (!pTemplate)
            WriteProfileData(inifile, pchCurrentKey, NULL, NULL, 0);
      }

      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);

   PrfCloseProfile(inifile);
   templatelist.bDirty = FALSE;
   return 0;
}



/*------------------------------ ReadIniProfile -----------------------------*/
/* Liesst alle Informationen aus dem Profile                                 */
/* Returnwerte: 0   alles OK                                                 */
/*              1   Fehler beim Oeffnen des Profile                          */
/*              2   neues Profile                                            */
/*              3   veraltetes Profile                                       */
/*---------------------------------------------------------------------------*/

static ULONG ReadIniProfile(HAB hab)
{
   extern MARKERLIST MarkerList;
   extern BOOL issecondinstance;
   HINI inifile=NULLHANDLE;
   ULONG retval=0;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, INIFILENAME);

   inifile=PrfOpenProfile(hab, pchIniFile);

   if (inifile)
   {
      char chBuffer[50];

      QueryProfileString(inifile, "Version", "ProgVersion",
                            "", chBuffer, 50);
      if (strcmp(chBuffer, PROGVERSION) &&
          strcmp(chBuffer, O_PROGVERSION))
      {
         /* Neues Profile */
         PrfCloseProfile(inifile);
         inifile=NULLHANDLE;
         retval= INIERROR(INIFILE_NEW);
      }

      if (inifile)
      {
         QueryProfileString(inifile, "Version", "IniVersion",
                               "", chBuffer, 50);
         if (strcmp(chBuffer, INIVERSION))
         {
            /* veraltetes Profile */
            if (issecondinstance || ConvertProfile(inifile, chBuffer, hab))
            {
               PrfCloseProfile(inifile);
               inifile=NULLHANDLE;
               retval= INIERROR(INIFILE_VERSION);
            }
         }
      }
   }
   else
      retval= INIERROR(INIFILE_OPEN);

   memset(&dirtyflags, 0, sizeof(dirtyflags));

   ReadIniUser(inifile);
   ReadIniNick(hab);
   ReadIniWinPos(inifile);
   ReadIniWinCol(inifile);
   ReadIniFonts(inifile);
   ReadIniPaths(inifile);
   ReadIniMisc(inifile);
   ReadIniMacros(inifile);
   ReadIniGeneral(inifile);
   ReadIniNodelist(inifile);
   ReadIniTemplates(hab);
   ReadIniEchotoss(inifile);
   ReadIniDomains(hab);
   ReadIniThreadopt(inifile);
   ReadIniResults(inifile);
   ReadIniRequest(inifile);
   ReadIniLookup(inifile);
   ReadIniAreaList(inifile);
   ReadIniMsgList(inifile);
   ReadIniRemap(inifile);
   ReadIniHooks(inifile);
   ReadIniFind(inifile);
   ReadIniMarkers(inifile, &MarkerList);
   ReadIniBrowse(inifile);
   ReadIniExport(inifile);
   ReadIniToolbar(inifile);
   ReadIniEchoman(inifile);
   ReadIniSearch(inifile);
   ReadIniPrintSetup(inifile);
   ReadIniScripts(hab);
   ReadIniCCLists(hab);
   ReadIntlSetting();
   ReadIniFolders(hab);

   if (inifile)
     PrfCloseProfile(inifile);
   return retval;
}

void _Optlink SaveIniProfileThread(void *pParam)
{
   extern BOOL bProfileSaved;
   HAB SaveHab;

   INSTALLEXPT("SaveINI");

   SaveHab = WinInitialize(0);

   SaveIniProfile(SaveHab);

   if (arealiste.bDirty)
      SaveIniAreas(SaveHab);
   memset(&dirtyflags, 0, sizeof(DIRTYFLAGS));

   if (pParam)
   {
      bProfileSaved=TRUE;
      WinPostQueueMsg(*((PHMQ)pParam), WM_QUIT, NULL, NULL);
   }
   WinTerminate(SaveHab);

   DEINSTALLEXPT;

   return;
}

/*------------------------------ SaveIniProfile -----------------------------*/
/* Speichert alle Informationen im Profile                                   */
/*---------------------------------------------------------------------------*/

int SaveIniProfile(HAB hab)
{
   extern TEMPLATELIST templatelist;
   extern CCANCHOR ccanchor;
   extern SCRIPTLIST scriptlist;
   extern MARKERLIST MarkerList;
   extern BOOL bSaveResults;
   extern NICKNAMELIST NickNameList;
   extern ECHOMGROPT EchoMgrOpt;
   extern PRINTSETUP PrintSetup;
   extern TOOLBARCONFIG ToolbarConfig;
   extern REQUESTOPT requestoptions;
   extern FOLDERANCHOR FolderAnchor;
   HINI inifile=NULLHANDLE;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, INIFILENAME);

   inifile=PrfOpenProfile(hab, pchIniFile);

   if (inifile)
   {
      if (dirtyflags.userdirty)
         SaveIniUser(inifile);
      if (NickNameList.bDirty)
         SaveIniNick(hab);
      if (FolderAnchor.bDirty)
         SaveIniFolders(hab);
      SaveIniWinPos(inifile);
      SaveIniWinCol(inifile);
      SaveIniFonts(inifile);
      SaveIniPaths(inifile);
      SaveIniMisc(inifile);
      if (dirtyflags.domainsdirty)
         SaveIniDomains(hab);
      if (dirtyflags.macrosdirty)
         SaveIniMacros(inifile);
      /*if (dirtyflags.optionsdirty)*/
         SaveIniGeneral(inifile);
      if (dirtyflags.nodedirty)
         SaveIniNodelist(inifile);
      if (templatelist.bDirty)
         SaveIniTemplates(hab);
      if (dirtyflags.echotossdirty)
         SaveIniEchotoss(inifile);
      if (ccanchor.bDirty)
         SaveIniCCLists(hab);
      if (dirtyflags.threadsdirty)
         SaveIniThreadopt(inifile);
      if (dirtyflags.lookupdirty)
         SaveIniLookup(inifile);
      if (dirtyflags.resultsdirty)
         SaveIniResults(inifile);
      if (requestoptions.bDirty || requestoptions.bListDirty)
         SaveIniRequest(inifile);
      if (dirtyflags.alsettingsdirty)
         SaveIniAreaList(inifile);
      if (dirtyflags.mlsettingsdirty)
         SaveIniMsgList(inifile);
      if (dirtyflags.remapdirty)
         SaveIniRemap(inifile);
      if (dirtyflags.hooksdirty)
         SaveIniHooks(inifile);
      if (dirtyflags.finddirty)
         SaveIniFind(inifile);
      if (scriptlist.bDirty)
         SaveIniScripts(hab);
      if (bSaveResults)
      {
         if (MarkerList.bDirty)
            SaveIniMarkers(inifile, &MarkerList);
      }
      else
         WriteProfileData(inifile, "Mark", NULL, NULL, 0);
      if (dirtyflags.browserdirty)
         SaveIniBrowse(inifile);
      if (dirtyflags.toolbardirty || ToolbarConfig.bDirty)
         SaveIniToolbar(inifile);
      SaveIniExport(inifile);
      if (EchoMgrOpt.bDirty)
         SaveIniEchoman(inifile);
      if (PrintSetup.bDirty)
         SaveIniPrintSetup(inifile);
      SaveIniSearch(inifile);
      WriteProfileString(inifile, "Version", "ProgVersion", PROGVERSION);
      WriteProfileString(inifile, "Version", "IniVersion", INIVERSION);
      PrfCloseProfile(inifile);
      return 0;
   }
   else
      return 1;
}

/*------------------------------ ReadIniUser  -------------------------------*/
/* Liesst die Userinformationen aus dem INI-File,                            */
/*---------------------------------------------------------------------------*/

static  int ReadIniUser(HINI inifile)
{
   ULONG datalen=0;
   int rc=0;

   datalen=sizeof(userdaten.username);
   memset(&(userdaten.username[0]), 0, sizeof(userdaten.username));

   if (!inifile || !QueryProfileData(inifile, "User", "Names",
                            &(userdaten.username[0]), &datalen))
   {
      /* Default-Werte */
      memset(&(userdaten.username[0]), 0, sizeof(userdaten.username));
      dirtyflags.userdirty=TRUE;
      rc=1;
   }

   datalen=sizeof(userdaten.address);
   memset(&(userdaten.address[0]), 0, sizeof(userdaten.address));

   if (!inifile || !QueryProfileData(inifile, "User", "Addresses",
                            &(userdaten.address[0]), &datalen))
   {
      /* Default-Werte */
      memset(&(userdaten.address[0]), 0, sizeof(userdaten.address));
      dirtyflags.userdirty=TRUE;
      rc=1;
   }

   memset(userdaten.defaultorigin, 0, sizeof(userdaten.defaultorigin));

   if (inifile)
      QueryProfileString(inifile, "User", "Origin", NULL, userdaten.defaultorigin,
                            sizeof(userdaten.defaultorigin));

   return rc;
}

/*------------------------------ SaveIniUser  -------------------------------*/
/* Schreibt die Userinformationen in das INI-File                            */
/*---------------------------------------------------------------------------*/

static  int SaveIniUser(HINI inifile)
{
   int i=0;
   int rc=0;

   if (!inifile)
      return 1;

   i=0;
   while(i<MAX_USERNAMES && userdaten.username[i][0])
      i++;

   if (i>0)
      if (!WriteProfileData(inifile, "User", "Names",
                            &(userdaten.username[0]), i*(LEN_USERNAME+1)))
         rc=1;

   i=0;
   while(i<MAX_ADDRESSES && userdaten.address[i][0])
      i++;

   if (i>0)
      if (!WriteProfileData(inifile, "User", "Addresses",
                            &(userdaten.address[0]), i*(LEN_5DADDRESS+1)))
         rc=1;

   if (!WriteProfileString(inifile, "User", "Origin", userdaten.defaultorigin))
      rc=1;

   return rc;
}

/*------------------------------ ReadIniNick  -------------------------------*/
/* Liesst die Nicknames aus dem INI-File                                     */
/*---------------------------------------------------------------------------*/

static int ReadIniNick(HAB hab)
{
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */
   extern NICKNAMELIST NickNameList;
   NICKNAME NickName;
   ULONG sizeNick;
   HINI hini;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, NICKFILENAME);

   hini=PrfOpenProfile(hab, pchIniFile);

   if (hini)
   {
      if (!QueryProfileSize(hini, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
      {
         PrfCloseProfile(hini);
         return 1;
      }

      pchKeys=malloc(ulKeysLen);

      if (!QueryProfileData(hini, NULL, NULL, pchKeys, &ulKeysLen))
      {
         free(pchKeys);
         PrfCloseProfile(hini);
         return 1;
      }

      pchCurrentKey=pchKeys;

      while (pchCurrentKey[0])
      {
         if (pchCurrentKey[0] == '#')
         {
            /* Folder-Daten */
            sizeNick = sizeof(NickNameList.FolderPos);
            QueryProfileData(hini, pchCurrentKey, "Pos", &NickNameList.FolderPos, &sizeNick);
         }
         else
         {
            /* Nickname-Daten */
            memset(&NickName, 0, sizeof(NickName));
            strncpy(NickName.usertag, pchCurrentKey, LEN_USERNAME);
            strlwr(NickName.usertag);
            if (!FindNickname(&NickNameList, NickName.usertag, NULL))
            {
               QueryProfileString(hini, pchCurrentKey, "Name", "", NickName.username, sizeof(NickName.username));
               QueryProfileString(hini, pchCurrentKey, "Address", "", NickName.address, sizeof(NickName.address));
               QueryProfileString(hini, pchCurrentKey, "Subj", "", NickName.subjectline, sizeof(NickName.subjectline));
               QueryProfileString(hini, pchCurrentKey, "First", "", NickName.firstline, sizeof(NickName.firstline));
               NickName.ulAttrib = QueryProfileInt(hini, pchCurrentKey, "Attrib", 0);
               NickName.ulFlags = QueryProfileInt(hini, pchCurrentKey, "Flags", 0);

               ulKeysLen=0;
               if (QueryProfileSize(hini, pchCurrentKey, "Comment", &ulKeysLen) && ulKeysLen)
               {
                  NickName.pchComment = malloc(ulKeysLen);
                  QueryProfileString(hini, pchCurrentKey, "Comment", "", NickName.pchComment, ulKeysLen);
               }

               /* Hinzufuegen */
               AddNickname(&NickNameList, &NickName, FALSE);
               if (NickName.pchComment)
                  free(NickName.pchComment);
            }
         }

         /* Naechsten Key suchen */
         while (pchCurrentKey[0])
            pchCurrentKey++;
         pchCurrentKey++;
      }
      free(pchKeys);
      PrfCloseProfile(hini);
      return 0;
   }
   else
      return 1;
}

/*------------------------------ SaveIniNick  -------------------------------*/
/* Speichert die Nicknames in dem INI-File                                   */
/*---------------------------------------------------------------------------*/

static  int SaveIniNick(HAB hab)
{
   PNICKNAME pNick=NULL;
   extern NICKNAMELIST NickNameList;
   HINI hini;
   char pchIniFile[LEN_PATHNAME+1];
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, NICKFILENAME);

   hini=PrfOpenProfile(hab, pchIniFile);

   if (hini)
   {
      /* Globale Daten */
      WriteProfileData(hini, "#List", "Pos", &NickNameList.FolderPos, sizeof(NickNameList.FolderPos));

      /* Die Nicknames */
      while (pNick = FindNickname(&NickNameList, NULL, pNick))
      {
         if (pNick->bDirty)
         {
            if (pNick->username[0])
               WriteProfileString(hini, pNick->usertag, "Name", pNick->username);
            else
               WriteProfileString(hini, pNick->usertag, "Name", NULL);
            if (pNick->address[0])
               WriteProfileString(hini, pNick->usertag, "Address", pNick->address);
            else
               WriteProfileString(hini, pNick->usertag, "Address", NULL);
            if (pNick->subjectline[0])
               WriteProfileString(hini, pNick->usertag, "Subj", pNick->subjectline);
            else
               WriteProfileString(hini, pNick->usertag, "Subj", NULL);
            if (pNick->firstline[0])
               WriteProfileString(hini, pNick->usertag, "First", pNick->firstline);
            else
               WriteProfileString(hini, pNick->usertag, "First", NULL);
            if (pNick->ulAttrib)
               WriteProfileInt(hini, pNick->usertag, "Attrib", pNick->ulAttrib);
            else
               WriteProfileString(hini, pNick->usertag, "Attrib", NULL);
            if (pNick->ulFlags)
               WriteProfileInt(hini, pNick->usertag, "Flags", pNick->ulFlags);
            else
               WriteProfileString(hini, pNick->usertag, "Flags", NULL);
            WriteProfileString(hini, pNick->usertag, "Comment", pNick->pchComment);

            pNick->bDirty = FALSE;
         }
      }
      NickNameList.bDirty = FALSE;

      /* evtl. geloeschte Eintraege entfernen */
      if (!QueryProfileSize(hini, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
      {
         PrfCloseProfile(hini);
         return 1;
      }

      pchKeys=malloc(ulKeysLen);

      if (!QueryProfileData(hini, NULL, NULL, pchKeys, &ulKeysLen))
      {
         free(pchKeys);
         PrfCloseProfile(hini);
         return 1;
      }

      pchCurrentKey=pchKeys;

      while (pchCurrentKey[0])
      {
         if (pchCurrentKey[0] != '#' && !FindNicknameSens(&NickNameList, pchCurrentKey))
            WriteProfileData(hini, pchCurrentKey, NULL, NULL, 0);

         /* Naechsten Key suchen */
         while (pchCurrentKey[0])
            pchCurrentKey++;
         pchCurrentKey++;
      }
      free(pchKeys);
      PrfCloseProfile(hini);

      return 0;
   }
   else
      return 1;
}

/*------------------------------ ReadIniWinPos ------------------------------*/
/* Liesst die Fensterpositionen aus dem INI-File,                            */
/*---------------------------------------------------------------------------*/

static  int ReadIniWinPos(HINI inifile)
{
   extern WINDOWPOSITIONS windowpositions;
   ULONG datalen=0;

   datalen=sizeof(windowpositions);
   memset(&windowpositions, 0, datalen);
   if (!inifile || !QueryProfileData(inifile, "Positions", "Windowpos",
                            &windowpositions, &datalen))
   {
      /* Default-Werte */
      /* fl auf 0 setzen, d.h. keine gespeicherte Pos. anzeigen */
      memset(&windowpositions, 0, sizeof(windowpositions));
      return 1;
   }
   return 0;
}

/*------------------------------ SaveIniWinPos ------------------------------*/
/* Speichert die Fensterpositionen im INI-File                               */
/*---------------------------------------------------------------------------*/

static  int SaveIniWinPos(HINI inifile)
{
   extern WINDOWPOSITIONS windowpositions;


   if (!inifile || !WriteProfileData(inifile, "Positions", "Windowpos",
                            &windowpositions, sizeof(windowpositions)))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniWinCol ------------------------------*/
/* Liesst die Farben aus dem INI-File                                        */
/*---------------------------------------------------------------------------*/

static  int ReadIniWinCol(HINI inifile)
{
   extern WINDOWCOLORS windowcolors;
   ULONG datalen=0;
   ULONG ulVer[2]={0,0};

   DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MINOR, ulVer, sizeof(ulVer));

   /* Default-Werte */
   /* alle Farben schwarz */
   memset(&windowcolors, 0, sizeof(windowcolors));

   /* sonst andere Defaults */
   windowcolors.windowback=RGB_GREY;
   if (ulVer[0] > 2 || (ulVer[0]==2 && ulVer[1]>=11))
   {
      /* OS/2 2.11 verwendet RGB-Farben f. MLE */
      windowcolors.editback=RGB_WHITE;
      windowcolors.editfore=RGB_BLACK;
   }
   else
   {
      windowcolors.editback=CLR_WHITE;
      windowcolors.editfore=CLR_BLACK;
   }
   windowcolors.statusback=RGB_GREY;
   windowcolors.scriptback=RGB_WHITE;
   windowcolors.areatitlefore=RGB_RED;
   windowcolors.areatitleback=RGB_GREY;
   windowcolors.monitorfore=RGB_BLACK;
   windowcolors.monitorback=RGB_WHITE;
   windowcolors.fromback=RGB_GREY;
   windowcolors.fromadback=RGB_GREY;
   windowcolors.toback=RGB_GREY;
   windowcolors.toadback=RGB_GREY;
   windowcolors.fromtostaticfore=RGB_BLUE;
   windowcolors.fromtostaticback=RGB_GREY;
   windowcolors.subjback=RGB_GREY;
   windowcolors.attribback=RGB_GREY;
   windowcolors.msgtimeback=RGB_GREY;
   windowcolors.menuback=RGB_GREY;
   windowcolors.arealistback=RGB_WHITE;
   windowcolors.tplfolderback=RGB_WHITE;
   windowcolors.threadlistback=RGB_WHITE;
   windowcolors.ccfolderback=RGB_WHITE;
   windowcolors.ccfolderfore=RGB_BLACK;
   windowcolors.nicknamesback=RGB_WHITE;
   windowcolors.msginfoback=CLR_WHITE;
   windowcolors.msginfofore=CLR_BLACK;
   windowcolors.resultsback=RGB_WHITE;
   windowcolors.cccontentsback=RGB_WHITE;
   windowcolors.lookupback=RGB_WHITE;
   windowcolors.attachback=RGB_WHITE;
   windowcolors.viewerback=RGB_WHITE;
   windowcolors.viewerfore=RGB_BLACK;
   windowcolors.viewerquote=RGB_BLUE;
   windowcolors.viewerquote2=0x008F00FFL;
   windowcolors.viewertearline=RGB_RED;
   windowcolors.viewerorigin=RGB_DARKGREEN;

   datalen=sizeof(windowcolors);
   if (!inifile || !QueryProfileData(inifile, "Colors", "Windowcol",
                            &windowcolors, &datalen))
   {
      return 1;
   }
   return 0;
}

/*------------------------------ SaveIniWinCol ------------------------------*/
/* Speichert die Farben im INI-File                                          */
/*---------------------------------------------------------------------------*/

static int SaveIniWinCol(HINI inifile)
{
   extern WINDOWCOLORS windowcolors;

   if (!inifile || !WriteProfileData(inifile, "Colors", "Windowcol",
                            &windowcolors, sizeof(windowcolors)))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniFonts  ------------------------------*/
/* Liesst die Fonts aus dem INI-File                                         */
/*---------------------------------------------------------------------------*/

static int ReadIniFonts(HINI inifile)
{
   extern WINDOWFONTS windowfonts;
   ULONG datalen=0;

   /* Default-Werte */
   /* Alle Fonts unveraendert */
   memset(&windowfonts, 0, sizeof(windowfonts));

   /* andere Fonts */
   strcpy(windowfonts.statusfont, SMA_FONT);
   strcpy(windowfonts.scriptfont, SMA_FONT);
   strcpy(windowfonts.datefont, SMA_FONT);
   strcpy(windowfonts.buttonfont, SMA_FONT);
   strcpy(windowfonts.arealistfont, SMA_FONT);
   strcpy(windowfonts.tplfolderfont, SMA_FONT);
   strcpy(windowfonts.threadlistfont, SMA_FONT);
   strcpy(windowfonts.findresultsfont, SMA_FONT);
   strcpy(windowfonts.ccfolderfont, SMA_FONT);
   strcpy(windowfonts.cclistfont, SMA_FONT);
   strcpy(windowfonts.lookupfont, SMA_FONT);
   strcpy(windowfonts.nicknamesfont, SMA_FONT);
   strcpy(windowfonts.attachfont, SMA_FONT);
   strcpy(windowfonts.viewerfont, VIEW_FONT);
   strcpy(windowfonts.viewermonofont, VIEW_FONT_MONO);
   strcpy(windowfonts.attribfont, SMA_FONT);

   datalen=sizeof(windowfonts);
   if (!inifile || !QueryProfileData(inifile, "Fonts", "Windowfonts",
                            &windowfonts, &datalen))
   {
      return 1;
   }
   return 0;
}

/*------------------------------ SaveIniFonts  ------------------------------*/
/* Speichert die Fonts im INI-File                                           */
/*---------------------------------------------------------------------------*/

static int SaveIniFonts(HINI inifile)
{
   extern WINDOWFONTS windowfonts;

   if (!inifile || !WriteProfileData(inifile, "Fonts", "Windowfonts",
                            &windowfonts, sizeof(windowfonts)))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniPaths  ------------------------------*/
/* Liesst die Pfadnamen aus dem INI-File                                     */
/*---------------------------------------------------------------------------*/

static int ReadIniPaths(HINI inifile)
{
   extern PATHNAMES pathnames;

   memset(&pathnames, 0, sizeof(pathnames));

   if (!inifile)
      return 1;

   QueryProfileString(inifile, "Paths", "Import", "",
                         pathnames.lastimport, LEN_PATHNAME+1);
   QueryProfileString(inifile, "Paths", "Squish", "",
                         pathnames.squishcfg, LEN_PATHNAME+1);

   return 0;
}

/*------------------------------ SaveIniPaths  ------------------------------*/
/* Speichert die Paths im INI-File                                           */
/*---------------------------------------------------------------------------*/

static int SaveIniPaths(HINI inifile)
{
   extern PATHNAMES pathnames;

   if (!inifile)
      return 1;

   WriteProfileString(inifile, "Paths", "Import", pathnames.lastimport);
   WriteProfileString(inifile, "Paths", "Squish", pathnames.squishcfg);

   return 0;
}

/*------------------------------ ReadIniMisc   ------------------------------*/
/* Liesst die Misc-Options aus dem INI-File                                  */
/*---------------------------------------------------------------------------*/

static int ReadIniMisc(HINI inifile)
{
   extern MISCOPTIONS miscoptions;
   extern OPENWIN OpenWindows;
   ULONG datalen=0;

   memset(&miscoptions, 0, sizeof(miscoptions));
   datalen=sizeof(miscoptions);
   QueryProfileData(inifile, "Options", "Misc", &miscoptions, &datalen);
   OpenWindows.ulOpenWindows = QueryProfileInt(inifile, "Windows", "Open", 0);
   OpenWindows.ulForceOpen   = QueryProfileInt(inifile, "Windows", "FOpen", 0);
   OpenWindows.ulForceClose  = QueryProfileInt(inifile, "Windows", "FClose", 0);

   return 0;
}

/*------------------------------ SaveIniMisc   ------------------------------*/
/* Speichert die Misc-Options im INI-File                                    */
/*---------------------------------------------------------------------------*/

static int SaveIniMisc(HINI inifile)
{
   extern MISCOPTIONS miscoptions;
   extern OPENWIN OpenWindows;

   if (!inifile || !WriteProfileData(inifile, "Options", "Misc",
                            &miscoptions, sizeof(miscoptions)))
      return 1;
   else
   {
      WriteProfileInt(inifile, "Windows", "Open", OpenWindows.ulOpenWindows);
      WriteProfileInt(inifile, "Windows", "FOpen", OpenWindows.ulForceOpen);
      WriteProfileInt(inifile, "Windows", "FClose", OpenWindows.ulForceClose);

      return 0;
   }
}

/*------------------------------ ReadIniMacros ------------------------------*/
/* Liesst die Macros aus dem INI-File                                        */
/*---------------------------------------------------------------------------*/

static int ReadIniMacros(HINI inifile)
{
   extern MACROTABLEOPT macrotable;
   int i;
   char pchKey[10];

   memset(&macrotable, 0, sizeof(macrotable));

   if (!inifile)
      return 1;

   for (i=0; i<11; i++)
   {
      sprintf(pchKey, "M%02d", i+1);
      QueryProfileString(inifile, "Macros", pchKey, "", macrotable.macrotext[i], LEN_MACRO+1);
   }
   return 0;
}

/*------------------------------ SaveIniMacros ------------------------------*/
/* Speichert die Macros im INI-File                                          */
/*---------------------------------------------------------------------------*/

static int SaveIniMacros(HINI inifile)
{
   extern MACROTABLEOPT macrotable;
   int i;
   char pchKey[10];

   if (!inifile)
      return 1;

   for (i=0; i<11; i++)
   {
      sprintf(pchKey, "M%02d", i+1);
      WriteProfileString(inifile, "Macros", pchKey, macrotable.macrotext[i]);
   }

   return 0;
}

/*------------------------------ ReadIniGeneral -----------------------------*/
/* Liesst die General-Optionen aus dem INI-File                              */
/*---------------------------------------------------------------------------*/

static int ReadIniGeneral(HINI inifile)
{
   extern GENERALOPT generaloptions;
   ULONG datalen=0;
   ULONG ulTime;

   DosQuerySysInfo(QSV_TIME_LOW, QSV_TIME_LOW, &ulTime, sizeof(ulTime));

   datalen=sizeof(generaloptions);
   memset(&generaloptions, 0, datalen);

   /* Default-Werte */
   generaloptions.safety=0xFFFFFFFF;
   generaloptions.uselastarea=TRUE;
   generaloptions.lTabSize=8;
   generaloptions.lMaxMsgLen=14;
   generaloptions.beeponpersonal=TRUE;
   generaloptions.ulInstallTime = ulTime;

   if (!inifile || !QueryProfileData(inifile, "Options", "General",
                            &generaloptions, &datalen))
   {
      dirtyflags.optionsdirty=TRUE;
      return 1;
   }
   if (generaloptions.lTabSize<1 || generaloptions.lTabSize>20)
      generaloptions.lTabSize=8;
   return 0;
}

/*------------------------------ SaveIniGeneral -----------------------------*/
/* Speichert die General-Optionen im INI-File                                */
/*---------------------------------------------------------------------------*/

static int SaveIniGeneral(HINI inifile)
{
   extern GENERALOPT generaloptions;

   if (!inifile || !WriteProfileData(inifile, "Options", "General",
                            &generaloptions, sizeof(generaloptions)))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniNodelist ----------------------------*/
/* Liesst die Nodelist-Optionen aus dem INI-File                             */
/*---------------------------------------------------------------------------*/

static  int ReadIniNodelist(HINI inifile)
{
   extern NODELISTOPT nodelist;
   ULONG ulTypesLen=0;
   PCHAR pchModemTypes=NULL, pchCurrType=NULL;

   memset(&nodelist.modemtype, 0, sizeof(nodelist.modemtype));

   if (QueryProfileSize(inifile, "Nodelist", "Modem", &ulTypesLen) && ulTypesLen>0)
   {
      int iType=0;

      pchModemTypes=malloc(sizeof(nodelist.modemtype)+1);
      memset(pchModemTypes, 0, sizeof(nodelist.modemtype)+1);
      pchCurrType=pchModemTypes;
      QueryProfileData(inifile, "Nodelist", "Modem", pchModemTypes, &ulTypesLen);

      while(iType<MAX_MODEMTYPES)
      {
         strncpy(nodelist.modemtype[iType], pchCurrType, LEN_MODEMTYPE);

         while(*pchCurrType)
            pchCurrType++;
         pchCurrType++;
         iType++;
      }
      free(pchModemTypes);
   }

   if (QueryProfileSize(inifile, "Nodelist", "ModemB", &ulTypesLen) && ulTypesLen>0)
   {
      char *pchTemp;
      int iType;

      pchTemp = pchModemTypes = malloc(ulTypesLen);
      QueryProfileData(inifile, "Nodelist", "ModemB", pchModemTypes, &ulTypesLen);

      while(*pchTemp)
      {
         iType = (*pchTemp)-1;

         pchTemp++;

         strncpy(nodelist.bytetypes[iType], pchTemp, LEN_MODEMTYPE);

         while(*pchTemp)
            pchTemp++;
         pchTemp++;
      }
      free(pchModemTypes);
   }


   ulTypesLen=sizeof(nodelist.ulOptions);

   QueryProfileData(inifile, "Nodelist", "Options", &nodelist.ulOptions, &ulTypesLen);

   return 0;
}

/*------------------------------ SaveIniNodelist ----------------------------*/
/* Speichert die Nodelist-Optionen im INI-File                               */
/*---------------------------------------------------------------------------*/

static int SaveIniNodelist(HINI inifile)
{
   extern NODELISTOPT nodelist;
   PCHAR pchModemTypes=NULL, pchCurrType=NULL, pchTemp;
   int i;
   ULONG ulSize=0;

   pchModemTypes=malloc(sizeof(nodelist.modemtype)+1);

   memset(pchModemTypes, 0, sizeof(nodelist.modemtype)+1);
   pchCurrType=pchModemTypes;

   for (i=0; i<MAX_MODEMTYPES; i++)
   {
      strcpy(pchCurrType, nodelist.modemtype[i]);
      while(*pchCurrType)
         pchCurrType++;
      pchCurrType++;
   }
   *pchCurrType='\0';

   WriteProfileData(inifile, "Nodelist", "Modem", pchModemTypes,
                       pchCurrType-pchModemTypes+1);

   free(pchModemTypes);

   /* Byte-Type */
   for (i=0; i<255; i++)
      if (nodelist.bytetypes[i][0])
         ulSize += 2 + strlen(nodelist.bytetypes[i]);

   pchTemp = pchModemTypes = malloc(ulSize + 1);

   for (i=0; i<255; i++)
      if (nodelist.bytetypes[i][0])
      {
         *pchTemp = i+1;
         pchTemp++;
         strcpy(pchTemp, nodelist.bytetypes[i]);
         while (*pchTemp)
            pchTemp++;
         pchTemp++;
      }
   *pchTemp = 0;

   WriteProfileData(inifile, "Nodelist", "ModemB", pchModemTypes, ulSize+1);

   free(pchModemTypes);

   WriteProfileData(inifile, "Nodelist", "Options", &nodelist.ulOptions,
                    sizeof(nodelist.ulOptions));

   return 0;
}

/*------------------------------ ReadIniEchotoss ----------------------------*/
/* Liesst die Echotoss-Optionen aus dem INI-File                             */
/*---------------------------------------------------------------------------*/

static  int ReadIniEchotoss(HINI inifile)
{
   extern ECHOTOSSOPT echotossoptions;
   ULONG datalen=0;
   int rc=0;

   memset(&echotossoptions, 0, sizeof(echotossoptions));

   datalen=4;
   if (!inifile || !QueryProfileData(inifile, "Echotoss", "Options",
                            &echotossoptions.useechotoss, &datalen))
   {
      /* Default-Werte */
      echotossoptions.useechotoss=FALSE;
      dirtyflags.echotossdirty=TRUE;
      rc=1;
   }

   if (!inifile || !QueryProfileString(inifile, "Echotoss", "Pathname",
                            "", echotossoptions.pchEchoToss, LEN_PATHNAME+1))
   {
      /* Default-Werte */
      dirtyflags.echotossdirty=TRUE;
      rc=1;
   }

   return rc;
}

/*------------------------------ SaveIniEchotoss ----------------------------*/
/* Speichert die Echotoss-Optionen im INI-File                               */
/*---------------------------------------------------------------------------*/

static  int SaveIniEchotoss(HINI inifile)
{
   extern ECHOTOSSOPT echotossoptions;

   if (!inifile)
      return 1;
   if (!WriteProfileData(inifile, "Echotoss", "Options",
                            &echotossoptions.useechotoss, 4))
      return 1;
   if (!WriteProfileString(inifile, "Echotoss", "Pathname",
                              echotossoptions.pchEchoToss))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniDomains  ----------------------------*/
/* Liesst die Domain-Optionen aus dem INI-File                               */
/*---------------------------------------------------------------------------*/

static  int ReadIniDomains(HAB hab)
{
   extern PDOMAINS domains;
   PDOMAINS pDomainData=NULL;
   HINI inifile;
   char *pchDomains=NULL, *pchCurrentDomain=NULL;
   ULONG ulKeysLen=0;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, DOMAINFILENAME);


   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
      return 1;

   if (!QueryProfileSize(inifile, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
   {
      PrfCloseProfile(inifile);
      return 1;
   }

   pchDomains=malloc(ulKeysLen);

   if (!QueryProfileData(inifile, NULL, NULL, pchDomains, &ulKeysLen))
   {
      free(pchDomains);
      PrfCloseProfile(inifile);
      return 1;
   }

   pchCurrentDomain=pchDomains;
   domains=NULL;

   while (pchCurrentDomain[0])
   {
      /* Neuen Speicher anfordern */
      if (domains)
      {
         pDomainData->next=malloc(sizeof(DOMAINS));
         pDomainData=pDomainData->next;
         pDomainData->next=NULL;
      }
      else
      {
         domains=malloc(sizeof(DOMAINS));
         domains->next=NULL;
         pDomainData=domains;
      }
      memset(pDomainData, 0, sizeof(DOMAINS));
      /* Daten holen */
      strncpy(pDomainData->domainname, pchCurrentDomain, LEN_DOMAIN);
      QueryProfileString(inifile, pchCurrentDomain, "Index", "", pDomainData->indexfile, LEN_PATHNAME+1);
      QueryProfileString(inifile, pchCurrentDomain, "Data", "", pDomainData->nodelistfile, LEN_PATHNAME+1);

      while(*pchCurrentDomain)
         pchCurrentDomain++;
      pchCurrentDomain++;
   }

   free(pchDomains);
   PrfCloseProfile(inifile);
   return 0;
}

/*------------------------------ SaveIniDomains -----------------------------*/
/* Speichert die Domain-Optionen im INI-File                                 */
/*---------------------------------------------------------------------------*/

static  int SaveIniDomains(HAB hab)
{
   extern PDOMAINS domains;
   PDOMAINS pTemp;
   HINI inifile;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, DOMAINFILENAME);

   pTemp=domains;

   if (DosDelete(pchIniFile)>2)
      return 1;

   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
      return 1;

   while(pTemp)
   {
      WriteProfileString(inifile, pTemp->domainname, "Index", pTemp->indexfile);
      WriteProfileString(inifile, pTemp->domainname, "Data", pTemp->nodelistfile);

      pTemp=pTemp->next;
   }

   PrfCloseProfile(inifile);

   return 0;
}

/*------------------------------ ReadIniThreadopt ---------------------------*/
/* Liesst die Settings fuer die Threadsliste ein                             */
/*---------------------------------------------------------------------------*/

static  int ReadIniThreadopt(HINI inifile)
{
   extern THREADLISTOPTIONS threadlistoptions;
   ULONG datalen=0;

   datalen=sizeof(threadlistoptions);
   memset(&threadlistoptions, 0, datalen);
   threadlistoptions.shownames=TRUE;
   threadlistoptions.compact=TRUE;
   threadlistoptions.dspmode=DSPTHREADS_UNREADONLY;
   threadlistoptions.keepinfront=FALSE;
   threadlistoptions.lBackClr = RGB_WHITE;
   threadlistoptions.lReadClr = RGB_BLACK;
   threadlistoptions.lUnreadClr = RGB_RED;
   threadlistoptions.lPersonalClr = RGB_BLUE;
   if (!inifile || !QueryProfileData(inifile, "Options", "Threadlist",
                              &threadlistoptions, &datalen))
   {
      /* Default-Werte */
      threadlistoptions.shownames=TRUE;
      threadlistoptions.dspmode=DSPTHREADS_UNREADONLY;
      threadlistoptions.keepinfront=FALSE;
      threadlistoptions.lBackClr = RGB_WHITE;
      threadlistoptions.lReadClr = RGB_BLACK;
      threadlistoptions.lUnreadClr = RGB_RED;
      threadlistoptions.lPersonalClr = RGB_BLUE;
      dirtyflags.threadsdirty=TRUE;
      return 1;
   }
   else
      return 0;
}

/*------------------------------ SaveIniThreadopt ---------------------------*/
/* Speichert die Settings fuer die Threadsliste                              */
/*---------------------------------------------------------------------------*/

static  int SaveIniThreadopt(HINI inifile)
{
   extern THREADLISTOPTIONS threadlistoptions;

   if (!inifile || !WriteProfileData(inifile, "Options", "Threadlist",
                              &threadlistoptions, sizeof(threadlistoptions)))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniCCLists  ----------------------------*/
/* Liesst die CCList-Optionen aus dem INI-File                               */
/*---------------------------------------------------------------------------*/

static  int ReadIniCCLists(HAB hab)
{
   extern CCANCHOR ccanchor;
   HINI ccini=NULLHANDLE;
   PCCLIST pList;
   char *pchApps;
   char *pchCurrentApp;
   ULONG datalen=0;
   ULONG ulAppsLen=0;
   ULONG ulID;
   ULONG namelen;
   char *pchName;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, CCFILENAME);

   memset(&ccanchor, 0, sizeof(CCANCHOR));

   if (!(ccini=PrfOpenProfile(hab, pchIniFile)))
      return 1;

   if (!QueryProfileSize(ccini, NULL, NULL, &ulAppsLen) || (ulAppsLen==0))
   {
      PrfCloseProfile(ccini);
      return 1;
   }

   pchApps=malloc(ulAppsLen);

   if (!QueryProfileData(ccini, NULL, NULL, pchApps, &ulAppsLen))
   {
      free(pchApps);
      PrfCloseProfile(ccini);
      return 1;
   }

   pchCurrentApp=pchApps;

   while (pchCurrentApp[0])
   {
      if (!strcmp(pchCurrentApp, "Folder"))
      {
         datalen = sizeof(ccanchor.FolderPos);
         QueryProfileData(ccini, "Folder", "Pos", &ccanchor.FolderPos, &datalen);
         datalen = sizeof(ccanchor.ulFlags);
         QueryProfileData(ccini, "Folder", "Flags", &ccanchor.ulFlags, &datalen);
      }
      else
      {
         /* ID holen */
         ulID = strtoul(pchCurrentApp, NULL, 10);
         if (ulID)
         {
            /* Name holen */
            QueryProfileSize(ccini, pchCurrentApp, "Name", &namelen);
            pchName = malloc(namelen+1);
            QueryProfileString(ccini, pchCurrentApp, "Name", "", pchName, namelen);

            /* neue CC-Liste anlegen */
            pList = AddCCList(&ccanchor, pchName);
            free(pchName);

            /* ID ueberschreiben */
            pList->ulListID = ulID;

            /* Daten lesen */
            datalen = sizeof(pList->ListPos);
            QueryProfileData(ccini, pchCurrentApp, "Pos", &pList->ListPos, &datalen);

            datalen=sizeof(pList->ulFlags);
            QueryProfileData(ccini, pchCurrentApp, "Flags", &pList->ulFlags, &datalen);

            /* jetzt alle Eintraege lesen */
            ReadIniCCEntries(ccini, pchCurrentApp, pList);

            /* Dirty-Flags zuruecksetzen */
            pList->bDirty = FALSE;
            ccanchor.bDirty = FALSE;
         }
      }
      /* Naechste App suchen */
      while (pchCurrentApp[0])
         pchCurrentApp++;
      pchCurrentApp++;
   }
   free(pchApps);
   PrfCloseProfile(ccini);
   return 0;
}

static  int ReadIniCCEntries(HINI ccini, char *pchApp, PCCLIST pList)
{
   extern CCANCHOR ccanchor;
   char *pchKeys=NULL;
   ULONG ulKeysLen=0, ulEntryLen=0;
   char *pchCurrentKey=NULL;
   char *pchEntry;
   char *pchTemp;
   PCCENTRY pNewEntry;

   QueryProfileSize(ccini, pchApp, NULL, &ulKeysLen);

   if (ulKeysLen == 0)
      return -1;

   pchKeys=malloc(ulKeysLen);

   if (!QueryProfileData(ccini, pchApp, NULL, pchKeys, &ulKeysLen))
   {
      free(pchKeys);
      return -1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      if (*pchCurrentKey == '#') /* Pos und Flags nicht lesen */
      {
         /* daten lesen */
         QueryProfileSize(ccini, pchApp, pchCurrentKey, &ulEntryLen);
         if (ulEntryLen)
         {
            pchEntry = malloc(ulEntryLen);
            memset(pchEntry, 0, ulEntryLen);
            QueryProfileData(ccini, pchApp, pchCurrentKey, pchEntry, &ulEntryLen);

            /* neuen Eintrag holen */
            pNewEntry = AddCCEntry(&ccanchor, pList, NULL);

            /* Daten uebernehmen */
            pchTemp = pchEntry;
            strncpy(pNewEntry->pchName, pchTemp, LEN_USERNAME);
            pchTemp = strchr(pchTemp, 0)+1;
            strncpy(pNewEntry->pchAddress, pchTemp, LEN_5DADDRESS);
            pchTemp = strchr(pchTemp, 0)+1;
            pNewEntry->ulFlags = strtoul(pchTemp, NULL, 16);
            pchTemp = strchr(pchTemp, 0)+1;
            if (pchTemp-pchEntry < ulEntryLen)
            {
               strncpy(pNewEntry->pchFirstLine, pchTemp, LEN_FIRSTLINE);
            }

            free(pchEntry);
         }
      }
      /* Naechsten Key suchen */
      while (pchCurrentKey[0])
         pchCurrentKey++;
      pchCurrentKey++;
   }
   return 0;
}

/*------------------------------ SaveIniCCLists -----------------------------*/
/* Speichert die CCList-Optionen im INI-File                                 */
/*---------------------------------------------------------------------------*/

static  int SaveIniCCLists(HAB hab)
{
   extern CCANCHOR ccanchor;
   HINI ccini=NULLHANDLE;
   PCCLIST pList;
   char pchBuf[15];
   char *pchKeys;
   ULONG ulKeysLen;
   char *pchCurrentKey;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, CCFILENAME);

   if (!(ccini=PrfOpenProfile(hab, pchIniFile)))
      return 1;

   /* Position sichern */
   WriteProfileData(ccini, "Folder", "Pos", &ccanchor.FolderPos, sizeof(ccanchor.FolderPos));
   WriteProfileData(ccini, "Folder", "Flags", &ccanchor.ulFlags, sizeof(ccanchor.ulFlags));

   pList=ccanchor.pLists;

   while(pList)   /* Alle Listen */
   {
      if (pList->bDirty)
      {
         /* Alte Eintraege loeschen */
         _itoa(pList->ulListID, pchBuf, 10);
         WriteProfileData(ccini, pchBuf, NULL, NULL, 0);

         /* Position, Name und Optionen */
         WriteProfileData(ccini, pchBuf, "Pos", &pList->ListPos, sizeof(pList->ListPos));
         WriteProfileData(ccini, pchBuf, "Flags", &pList->ulFlags, sizeof(pList->ulFlags));
         WriteProfileString(ccini, pchBuf, "Name", pList->pchListName);

         /* Eintraege sichern */
         SaveIniCCEntries(ccini, pchBuf, pList);

         pList->bDirty = FALSE;
      }
      pList = pList->next;
   }

   /* alle anderen Listen loeschen */
   QueryProfileSize(ccini, NULL, NULL, &ulKeysLen);

   pchKeys=malloc(ulKeysLen);

   QueryProfileData(ccini, NULL, NULL, pchKeys, &ulKeysLen);

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      if (strcmp(pchCurrentKey, "Folder"))
      {
         ULONG ulID;

         ulID = strtoul(pchCurrentKey, NULL, 10);
         if (!QueryCCList(&ccanchor, ulID))
            WriteProfileData(ccini, pchCurrentKey, NULL, NULL, 0);
      }

      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);

   ccanchor.bDirty = FALSE;
   PrfCloseProfile(ccini);
   return 0;
}

static  int SaveIniCCEntries(HINI ccini, char *pchApp, PCCLIST pList)
{
   char pchKey[20] = "#";
   char pchData[LEN_USERNAME+1+LEN_5DADDRESS+LEN_FIRSTLINE+1+10];
   char *pchDst;
   ULONG count = 1;
   PCCENTRY pEntry;

   pEntry = pList->pEntries;

   while (pEntry)
   {
      _itoa(count, pchKey+1, 10);
      pchDst = pchData;

      strcpy(pchDst, pEntry->pchName);
      pchDst = strchr(pchDst, 0)+1;
      strcpy(pchDst, pEntry->pchAddress);
      pchDst = strchr(pchDst, 0)+1;
      _itoa(pEntry->ulFlags, pchDst, 16);
      pchDst = strchr(pchDst, 0)+1;
      strcpy(pchDst, pEntry->pchFirstLine);
      pchDst = strchr(pchDst, 0)+1;

      WriteProfileData(ccini, pchApp, pchKey, pchData, pchDst-pchData);

      count++;
      pEntry = pEntry->next;
   }

   return 0;
}

/*------------------------------ ReadIntlSetting ----------------------------*/
/* Liesst die Settings fuer die Zeitanzeige ein                              */
/*---------------------------------------------------------------------------*/

static  int ReadIntlSetting(void)
{
   extern INTLSETTING intlsetting;
   char charbuf[10];

   intlsetting.DMY=QueryProfileInt(HINI_USERPROFILE, "PM_National",
                                      "iDate", 1);
   QueryProfileString(HINI_USERPROFILE, "PM_National", "sDate",
                         ".", charbuf, 10);
   intlsetting.h24=(BOOL)QueryProfileInt(HINI_USERPROFILE, "PM_National",
                                            "iTime", 1);
   intlsetting.datesep=charbuf[0];
   QueryProfileString(HINI_USERPROFILE, "PM_National", "sTime",
                         ":", charbuf, 10);
   intlsetting.timesep=charbuf[0];
   if (!QueryProfileString(HINI_USERPROFILE, "PM_National", "s1159", NULL,
                         intlsetting.amtext, 3))
      strcpy(intlsetting.amtext, "am");
   if (!QueryProfileString(HINI_USERPROFILE, "PM_National", "s2359", NULL,
                         intlsetting.pmtext, 3))
      strcpy(intlsetting.pmtext, "pm");
   return 0;
}

/*------------------------------ ReadIniLookup ------------------------------*/
/* Liesst die Lookup-Optionen aus dem INI-File                               */
/*---------------------------------------------------------------------------*/

static  int ReadIniLookup(HINI inifile)
{
   extern LOOKUPOPTIONS lookupoptions;
   ULONG datalen=0;

   datalen=sizeof(lookupoptions);
   if (!inifile || !QueryProfileData(inifile, "Options", "Lookup",
                            &lookupoptions, &datalen))
   {
      /* Default-Werte */
      lookupoptions.bBrief=TRUE;
      lookupoptions.lSplitBar=150;
      dirtyflags.lookupdirty=TRUE;
      return 1;
   }
   return 0;
}

/*------------------------------ SaveIniLookup ------------------------------*/
/* Speichert die Lookup-Optionen im INI-File                                 */
/*---------------------------------------------------------------------------*/

static  int SaveIniLookup(HINI inifile)
{
   extern LOOKUPOPTIONS lookupoptions;

   if (!inifile || !WriteProfileData(inifile, "Options", "Lookup",
                            &lookupoptions, sizeof(lookupoptions)))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniResults -----------------------------*/
/* Liesst die Result-Optionen aus dem INI-File                               */
/*---------------------------------------------------------------------------*/

static  int ReadIniResults(HINI inifile)
{
   extern RESULTSOPTIONS resultsoptions;
   ULONG datalen=0;

   datalen=sizeof(resultsoptions);
   memset(&resultsoptions, 0, datalen);
   if (!inifile || !QueryProfileData(inifile, "Options", "Results",
                            &resultsoptions, &datalen))
   {
      /* Default-Werte */
      resultsoptions.bScroll=TRUE;
      resultsoptions.lSplitBar=150;
      resultsoptions.keepinfront=FALSE;
      dirtyflags.resultsdirty=TRUE;
      return 1;
   }
   return 0;
}

/*------------------------------ SaveIniResults -----------------------------*/
/* Speichert die Result-Optionen im INI-File                                 */
/*---------------------------------------------------------------------------*/

static  int SaveIniResults(HINI inifile)
{
   extern RESULTSOPTIONS resultsoptions;

   if (!inifile || !WriteProfileData(inifile, "Options", "Results",
                            &resultsoptions, sizeof(resultsoptions)))
      return 1;
   else
      return 0;
}

/*------------------------------ ReadIniRequest -----------------------------*/
/* Liesst die Request-Optionen aus dem INI-File                              */
/*---------------------------------------------------------------------------*/

static  int ReadIniRequest(HINI inifile)
{
   extern REQUESTOPT requestoptions;
   ULONG datalen=0;
   ULONG keylen;

   datalen=sizeof(requestoptions);
   memset(&requestoptions, 0, datalen);
   requestoptions.ulAttrib = ATTRIB_LOCAL | ATTRIB_CRASH | ATTRIB_KILLSENT |
                             ATTRIB_FREQUEST;
   requestoptions.lListFore = RGB_BLACK;
   requestoptions.lListBack = RGB_WHITE;
   if (!inifile)
      return 1;
   QueryProfileString(inifile, "Request", "Area", "", requestoptions.pchDestArea,
                         LEN_AREATAG+1);
   datalen=sizeof(requestoptions.bDirectReq);
   QueryProfileData(inifile, "Request", "Direct", &requestoptions.bDirectReq,
                       &datalen);
   datalen=sizeof(requestoptions.ulAttrib);
   QueryProfileData(inifile, "Request", "Attrib", &requestoptions.ulAttrib,
                       &datalen);
   datalen=sizeof(requestoptions.ReqPos);
   QueryProfileData(inifile, "Request", "RPos", &requestoptions.ReqPos,
                       &datalen);
   datalen=sizeof(requestoptions.ListAddPos);
   QueryProfileData(inifile, "Request", "LPos", &requestoptions.ListAddPos,
                       &datalen);
   datalen=sizeof(requestoptions.FileAddPos);
   QueryProfileData(inifile, "Request", "FPos", &requestoptions.FileAddPos,
                       &datalen);
   datalen=sizeof(requestoptions.PasswdPos);
   QueryProfileData(inifile, "Request", "PPos", &requestoptions.PasswdPos,
                       &datalen);
   datalen=sizeof(requestoptions.SearchPos);
   QueryProfileData(inifile, "Request", "SPos", &requestoptions.SearchPos,
                       &datalen);
   datalen=sizeof(requestoptions.lListFore);
   QueryProfileData(inifile, "Request", "LFore", &requestoptions.lListFore,
                       &datalen);
   datalen=sizeof(requestoptions.lListBack);
   QueryProfileData(inifile, "Request", "LBack", &requestoptions.lListBack,
                       &datalen);
   QueryProfileString(inifile, "Request", "LFont", "8.Helv", requestoptions.pchListFont,
                      sizeof(requestoptions.pchListFont));
   QueryProfileString(inifile, "Request", "Search", "", requestoptions.pchLastSearch,
                      sizeof(requestoptions.pchLastSearch));
   datalen=sizeof(requestoptions.ulSearchFlags);
   QueryProfileData(inifile, "Request", "SearchF", &requestoptions.ulSearchFlags,
                    &datalen);

   /* Listen lesen */
   if (QueryProfileSize(inifile, "FileLists", NULL, &keylen) && keylen)
   {
      char *pchKeys, *pchCurrentKey;

      pchKeys = malloc(keylen);

      if (QueryProfileData(inifile, "FileLists", NULL, pchKeys, &keylen))
      {
         char pchVal[LEN_PATHNAME+1+LEN_5DADDRESS+1+LEN_LISTDESC+1];
         char *pchTemp;
         FILELIST FileList;

         pchCurrentKey = pchKeys;

         while (*pchCurrentKey)
         {
            if (*pchCurrentKey == '#')
            {
               datalen = sizeof(pchVal);
               QueryProfileData(inifile, "FileLists", pchCurrentKey, pchVal, &datalen);

               memset(&FileList, 0, sizeof(FileList));

               /* Daten kopieren */
               pchTemp = pchVal;
               strncpy(FileList.pchFileName, pchTemp, LEN_PATHNAME);
               pchTemp = strchr(pchTemp, '\0');
               pchTemp++;
               strncpy(FileList.pchAddress, pchTemp, LEN_5DADDRESS);
               pchTemp = strchr(pchTemp, '\0');
               pchTemp++;
               strncpy(FileList.pchDesc, pchTemp, LEN_LISTDESC);

               AddNewFileList(&requestoptions.pFirstList, &FileList, NULL);
            }
            while (*pchCurrentKey)
               pchCurrentKey++;
            pchCurrentKey++;
         }
      }

      free (pchKeys);
   }

   return 0;
}

/*------------------------------ SaveIniRequest -----------------------------*/
/* Speichert die Request-Optionen im INI-File                                */
/*---------------------------------------------------------------------------*/

static int SaveIniRequest(HINI inifile)
{
   extern REQUESTOPT requestoptions;

   if (!inifile)
      return 1;

   if (requestoptions.bDirty)
   {
      WriteProfileString(inifile, "Request", "Area", requestoptions.pchDestArea);
      WriteProfileData(inifile, "Request", "Direct", &requestoptions.bDirectReq,
                          sizeof(requestoptions.bDirectReq));
      WriteProfileData(inifile, "Request", "Attrib", &requestoptions.ulAttrib,
                          sizeof(requestoptions.ulAttrib));
      WriteProfileData(inifile, "Request", "RPos", &requestoptions.ReqPos,
                          sizeof(requestoptions.ReqPos));
      WriteProfileData(inifile, "Request", "LPos", &requestoptions.ListAddPos,
                          sizeof(requestoptions.ListAddPos));
      WriteProfileData(inifile, "Request", "FPos", &requestoptions.FileAddPos,
                          sizeof(requestoptions.FileAddPos));
      WriteProfileData(inifile, "Request", "PPos", &requestoptions.PasswdPos,
                          sizeof(requestoptions.PasswdPos));
      WriteProfileData(inifile, "Request", "SPos", &requestoptions.SearchPos,
                          sizeof(requestoptions.SearchPos));
      WriteProfileData(inifile, "Request", "LFore", &requestoptions.lListFore,
                          sizeof(requestoptions.lListFore));
      WriteProfileData(inifile, "Request", "LBack", &requestoptions.lListBack,
                          sizeof(requestoptions.lListBack));
      WriteProfileString(inifile, "Request", "LFont", requestoptions.pchListFont);
      WriteProfileString(inifile, "Request", "Search", requestoptions.pchLastSearch);
      WriteProfileData(inifile, "Request", "SearchF", &requestoptions.ulSearchFlags,
                          sizeof(requestoptions.ulSearchFlags));

      requestoptions.bDirty = FALSE;
   }

   if (requestoptions.bListDirty)
   {
      PFILELIST pTemp = requestoptions.pFirstList;
      int num=1;
      char pchKey[10]="#";
      char pchVal[LEN_PATHNAME+1+LEN_5DADDRESS+1+LEN_LISTDESC+1];
      char *pchTemp;

      /* alte Listen Loeschen */
      WriteProfileData(inifile, "FileLists", NULL, NULL, 0);

      /* Listen speichern */
      while (pTemp)
      {
         /* Value zusammenkopieren */

         pchTemp = pchVal;
         strcpy(pchTemp, pTemp->pchFileName);
         pchTemp = strchr(pchTemp, '\0');
         pchTemp++;
         strcpy(pchTemp, pTemp->pchAddress);
         pchTemp = strchr(pchTemp, '\0');
         pchTemp++;
         strcpy(pchTemp, pTemp->pchDesc);
         pchTemp = strchr(pchTemp, '\0');
         pchTemp++;

         /* Key erzeugen */
         _itoa(num, pchKey+1, 10);

         /* schreiben */
         WriteProfileData(inifile, "FileLists", pchKey, pchVal, pchTemp - pchVal);

         pTemp->bDirty = FALSE;

         pTemp = pTemp->next;
         num++;
      }

      requestoptions.bListDirty = FALSE;
   }

   return 0;
}

/*------------------------------ ReadIniAreaList ----------------------------*/
/* Liesst die Arealist-Optionen aus dem INI-File                             */
/*---------------------------------------------------------------------------*/

static  int ReadIniAreaList(HINI inifile)
{
   extern AREALISTOPTIONS arealistoptions;
   ULONG datalen=0;

   datalen=sizeof(arealistoptions);
   memset(&arealistoptions, 0, datalen);

   arealistoptions.lBackColor=RGB_WHITE;
   arealistoptions.lFolderBack=RGB_WHITE;
   arealistoptions.lNetAreaColor=RGB_BLACK;
   arealistoptions.lEchoAreaColor=RGB_BLACK;
   arealistoptions.lLocalAreaColor=RGB_BLACK;
   arealistoptions.lFolderFore=RGB_BLACK;

   if (!inifile || QueryProfileData(inifile, "Options", "Arealist",
                                       &arealistoptions,
                                       &datalen))
   {
      dirtyflags.alsettingsdirty=TRUE;
   }

   return 0;
}

/*------------------------------ SaveIniAreaList ----------------------------*/
/* Speichert die Arealist-Optionen im INI-File                               */
/*---------------------------------------------------------------------------*/

static  int SaveIniAreaList(HINI inifile)
{
   extern AREALISTOPTIONS arealistoptions;

   if (!inifile)
      return 1;

   WriteProfileData(inifile, "Options", "Arealist", &arealistoptions,
                       sizeof(arealistoptions));
   return 0;
}

/*------------------------------ ReadIniMsgList -----------------------------*/
/* Liesst die Messagelist-Optionen aus dem INI-File                          */
/*---------------------------------------------------------------------------*/

static  int ReadIniMsgList(HINI inifile)
{
   extern MSGLISTOPTIONS msglistoptions;
   ULONG datalen=0;

   datalen=sizeof(msglistoptions);
   memset(&msglistoptions, 0, datalen);

   msglistoptions.ulNrPercent   =  70;
   msglistoptions.ulFromPercent = 150;
   msglistoptions.ulToPercent   = 150;
   msglistoptions.ulSubjPercent = 350;
   msglistoptions.ulStampWrittenPercent = 140;
   msglistoptions.ulStampArrivedPercent = 140;

   msglistoptions.lBackClr   = RGB_WHITE;
   msglistoptions.lForeClr   = RGB_BLACK;
   msglistoptions.lUnreadClr = RGB_RED;
   msglistoptions.lFromClr   = RGB_RED;
   msglistoptions.lToClr     = RGB_RED;

   strcpy(msglistoptions.mlistfont, SMA_FONT);

   if (!inifile || QueryProfileData(inifile, "Options", "Msglist",
                                       &msglistoptions,
                                       &datalen))
   {
      dirtyflags.alsettingsdirty=TRUE;
   }

   return 0;
}

/*------------------------------ SaveIniMsgList -----------------------------*/
/* Speichert die Messagelist-Optionen im INI-File                            */
/*---------------------------------------------------------------------------*/

static  int SaveIniMsgList(HINI inifile)
{
   extern MSGLISTOPTIONS msglistoptions;

   if (!inifile)
      return 1;

   WriteProfileData(inifile, "Options", "Msglist", &msglistoptions,
                       sizeof(msglistoptions));
   return 0;
}

/*------------------------------ SaveIniRemap   -----------------------------*/
/* Speichert die Remap-Optionen im INI-File                                  */
/*---------------------------------------------------------------------------*/

static  int SaveIniRemap(HINI inifile)
{
   extern DRIVEREMAP driveremap;

   if (!inifile)
      return 1;

   WriteProfileString(inifile, "Options", "Remap",
                         driveremap.pchRemapString);

   return 0;
}

/*------------------------------ ReadIniRemap   -----------------------------*/
/* Laedt     die Remap-Optionen im INI-File                                  */
/*---------------------------------------------------------------------------*/

static  int ReadIniRemap(HINI inifile)
{
   extern DRIVEREMAP driveremap;

   /* Default belegen */
   strcpy(driveremap.pchRemapString, "CDEFGHIJKLMNOPQRSTUVWXYZ");

   if (!inifile)
      return 1;

   QueryProfileString(inifile, "Options", "Remap", "CDEFGHIJKLMNOPQRSTUVWXYZ",
                         driveremap.pchRemapString, 25);

   return 0;
}

/*------------------------------ SaveIniScripts -----------------------------*/
/* Speichert die Scripts im INI-File                                         */
/*---------------------------------------------------------------------------*/

static  int SaveIniScripts(HAB hab)
{
   extern SCRIPTLIST scriptlist;
   PRXSCRIPT pScript;
   HINI hini;
   char *pchApps, *pchCurrentApp;
   ULONG ulLen;
   char pchTemp[20]="#";
   ULONG ulID;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, SCRIPTFILENAME);


   if (!(hini = PrfOpenProfile(hab, pchIniFile)))
   {
      return 1;
   }

   WriteProfileData(hini, "Folder", "Pos", &scriptlist.FolderPos, sizeof(WINPOS));
   WriteProfileData(hini, "Folder", "SPos", &scriptlist.FolderSettingsPos, sizeof(WINPOS));
   WriteProfileData(hini, "Folder", "Flags", &scriptlist.ulFlags, sizeof(ULONG));

   pScript = scriptlist.pScripts;
   while (pScript)
   {
      if (pScript->bDirty)
      {
         /* App-Name erzeugen */
         _itoa(pScript->ulScriptID, pchTemp+1, 10);

         WriteProfileData(hini, pchTemp, "SettP", &pScript->SettingsPos, sizeof(WINPOS));
         WriteProfileData(hini, pchTemp, "MonP",  &pScript->MonitorPos, sizeof(WINPOS));

         WriteProfileString(hini, pchTemp, "Name", pScript->pchScriptName);
         WriteProfileString(hini, pchTemp, "File", pScript->pchPathName);

         WriteProfileData(hini, pchTemp, "Flags",  &pScript->ulFlags, sizeof(ULONG));

         pScript->bDirty = FALSE;
      }

      pScript = pScript->next;
   }

   /* geloeschte Scripts pruefen */
   QueryProfileSize(hini, NULL, NULL, &ulLen);

   if (ulLen == 0)
      return 1;

   pchApps = malloc(ulLen);

   QueryProfileData(hini, NULL, NULL, pchApps, &ulLen);

   pchCurrentApp = pchApps;
   while (*pchCurrentApp)
   {
      if (*pchCurrentApp == '#')
      {
         /* ID ermitteln */
         ulID = strtoul(pchCurrentApp+1, NULL, 10);

         pScript = scriptlist.pScripts;
         while (pScript && pScript->ulScriptID != ulID)
            pScript = pScript->next;

         if (!pScript)
            WriteProfileData(hini, pchCurrentApp, NULL, NULL, 0);
      }

      while (*pchCurrentApp)
         pchCurrentApp++;
      pchCurrentApp++;
   }

   free(pchApps);
   PrfCloseProfile(hini);

   scriptlist.bDirty = FALSE;

   return 0;
}

/*------------------------------ ReadIniScripts -----------------------------*/
/* Laedt die Scripts aus dem INI-File                                        */
/*---------------------------------------------------------------------------*/

static int ReadIniScripts(HAB hab)
{
   extern SCRIPTLIST scriptlist;
   HINI hini;
   PRXSCRIPT pScript=NULL, pNewScript, pLastScript=NULL;
   char *pchApps, *pchCurrentApp;
   ULONG ulLen;
   char pchIniFile[LEN_PATHNAME+1];

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, SCRIPTFILENAME);

   memset(&scriptlist, 0, sizeof(SCRIPTLIST));

   if (!(hini = PrfOpenProfile(hab, pchIniFile)))
   {
      return 1;
   }

   ulLen = sizeof(WINPOS);
   QueryProfileData(hini, "Folder", "Pos", &scriptlist.FolderPos, &ulLen);
   ulLen = sizeof(WINPOS);
   QueryProfileData(hini, "Folder", "SPos", &scriptlist.FolderSettingsPos, &ulLen);
   ulLen = sizeof(ULONG);
   QueryProfileData(hini, "Folder", "Flags", &scriptlist.ulFlags, &ulLen);

   QueryProfileSize(hini, NULL, NULL, &ulLen);
   if (ulLen == 0)
      return 1;
   pchApps = malloc(ulLen);
   QueryProfileData(hini, NULL, NULL, pchApps, &ulLen);

   pchCurrentApp = pchApps;
   while (*pchCurrentApp)
   {
      if (*pchCurrentApp == '#')
      {
         /* neues Script erzeugen */
         pNewScript = malloc(sizeof(RXSCRIPT));
         memset(pNewScript, 0, sizeof(RXSCRIPT));

         /* ID ermitteln */
         pNewScript->ulScriptID = strtoul(pchCurrentApp+1, NULL, 10);

         /* Einfuege-Position suchen */
         pScript = scriptlist.pScripts;

         while (pScript && pScript->ulScriptID < pNewScript->ulScriptID)
            pScript = pScript->next;

         if (pScript)
         {
            /* davor einhaengen */
            pNewScript->next = pScript;
            pNewScript->prev = pScript->prev;

            if (pScript->prev)
               pScript->prev->next = pNewScript;
            pScript->prev = pNewScript;

            if (scriptlist.pScripts == pScript) /* vor dem ersten, Anker anpassen */
               scriptlist.pScripts = pNewScript;
         }
         else
         {
            if (!scriptlist.pScripts) /* erstes Script ueberhaupt */
            {
               scriptlist.pScripts = pNewScript;
               pLastScript = pNewScript;
            }
            else
            {
               /* nicht gefunden, am Ende einhaengen */
               pLastScript->next = pNewScript;
               pNewScript->prev = pLastScript;
               pLastScript = pNewScript;
            }
         }


         scriptlist.ulNumScripts++;

         /* Pos lesen */
         ulLen = sizeof(WINPOS);
         QueryProfileData(hini, pchCurrentApp, "SettP", &pNewScript->SettingsPos, &ulLen);
         ulLen = sizeof(WINPOS);
         QueryProfileData(hini, pchCurrentApp, "MonP", &pNewScript->MonitorPos, &ulLen);

         /* Name lesen */
         QueryProfileSize(hini, pchCurrentApp, "Name", &ulLen);

         pNewScript->pchScriptName = malloc(ulLen+1);
         pNewScript->pchScriptName[0] = '\0';

         if (ulLen)
         {
            QueryProfileString(hini, pchCurrentApp, "Name", "", pNewScript->pchScriptName, ulLen+1);
         }

         /* Pfad lesen */
         QueryProfileString(hini, pchCurrentApp, "File", "", pNewScript->pchPathName, LEN_PATHNAME+1);

         /* Optionen */
         ulLen = sizeof(ULONG);
         QueryProfileData(hini, pchCurrentApp, "Flags", &pNewScript->ulFlags, &ulLen);
      }

      while (*pchCurrentApp)
         pchCurrentApp++;
      pchCurrentApp++;
   }

   free(pchApps);
   PrfCloseProfile(hini);


   return 0;
}

/*------------------------------ SaveIniHooks   -----------------------------*/
/* Speichert die Hook-Settings im INI-File                                   */
/*---------------------------------------------------------------------------*/

static  int SaveIniHooks(HINI inifile)
{
   extern REXXHOOKS rexxhooks;

   if (!inifile)
      return 1;

   WriteProfileData(inifile, "Options", "Hooks", &rexxhooks, sizeof(rexxhooks));

   return 0;
}

/*------------------------------ ReadIniHooks   -----------------------------*/
/* Laedt die Hook-Settings aus dem INI-File                                  */
/*---------------------------------------------------------------------------*/

static  int ReadIniHooks(HINI inifile)
{
   extern REXXHOOKS rexxhooks;
   ULONG ulLen;

   memset(&rexxhooks, 0, sizeof(rexxhooks));

   if (!inifile)
      return 1;

   ulLen = sizeof(rexxhooks);
   QueryProfileData(inifile, "Options", "Hooks", &rexxhooks, &ulLen);

   return 0;
}

/*------------------------------ ReadIniFind    -----------------------------*/
/* Laedt die Find-Settings aus dem INI-File                                  */
/*---------------------------------------------------------------------------*/

static int ReadIniFind(HINI inifile)
{
   extern FINDJOB FindJob;
   extern BOOL bSaveResults;
   ULONG datalen;
   char *pchBlock;
   char *pchSrc;
   int i=0;

   memset(&FindJob, 0, sizeof(FINDJOB));
   FindJob.ulWhere = FINDWHERE_FROM |
                     FINDWHERE_TO   |
                     FINDWHERE_SUBJ |
                     FINDWHERE_TEXT;
   FindJob.ulHow = FINDHOW_CASE;
   FindJob.ulWAreas = FINDAREAS_ECHO;
   bSaveResults = TRUE;

   if (!inifile)
      return 1;

   datalen=0;
   QueryProfileSize(inifile, "Find", "Text", &datalen);
   if (datalen)
   {
      pchBlock = calloc(datalen, 1);
      pchSrc = pchBlock;

      QueryProfileData(inifile, "Find", "Text", pchBlock, &datalen);

      while (i < NUM_BACKTEXTS)
      {
         strcpy(FindJob.pchBackTexts[i], pchSrc);
         pchSrc = strchr(pchSrc, 0);
         pchSrc++;
         i++;
      }
      free(pchBlock);
   }

   datalen=sizeof(FindJob.ulWhere);
   QueryProfileData(inifile, "Find", "Parts", &FindJob.ulWhere, &datalen);
   datalen=sizeof(FindJob.ulHow);
   QueryProfileData(inifile, "Find", "Method", &FindJob.ulHow, &datalen);
   datalen=sizeof(FindJob.ulWAreas);
   QueryProfileData(inifile, "Find", "Areas", &FindJob.ulWAreas, &datalen);
   FindJob.ulFuzzyLevel = QueryProfileInt(inifile, "Find", "Fuzz", 1);

   datalen=0;
   QueryProfileSize(inifile, "Find", "SelAreas", &datalen);
   if (datalen)
   {
      FindJob.pchAreas = calloc(1, datalen);
      QueryProfileString(inifile, "Find", "SelAreas", "", FindJob.pchAreas, datalen);
   }

   datalen=sizeof(FindJob.PersMailOpt);
   QueryProfileData(inifile, "Find", "PersMail", &FindJob.PersMailOpt, &datalen);

   datalen=sizeof(bSaveResults);
   QueryProfileData(inifile, "Find", "Save", &bSaveResults, &datalen);

   return 0;
}

/*------------------------------ SaveIniFind    -----------------------------*/
/* Speichert die Find-Settings im INI-File                                   */
/*---------------------------------------------------------------------------*/

static int SaveIniFind(HINI inifile)
{
   extern FINDJOB FindJob;
   extern BOOL bSaveResults;
   char *pchBlock;
   char *pchDest;
   int i=0;

   if (!inifile)
      return 1;

   pchBlock = malloc(sizeof(FindJob.pchBackTexts));
   pchDest = pchBlock;

   while (i< NUM_BACKTEXTS)
   {
      strcpy(pchDest, FindJob.pchBackTexts[i]);
      pchDest = strchr(pchDest, 0);
      pchDest++;
      i++;
   }

   WriteProfileData(inifile, "Find", "Text", pchBlock, pchDest - pchBlock);
   free(pchBlock);

   WriteProfileString(inifile, "Find", "SelAreas", FindJob.pchAreas);
   WriteProfileData(inifile, "Find", "Parts", &FindJob.ulWhere, sizeof(FindJob.ulWhere));
   WriteProfileData(inifile, "Find", "Method", &FindJob.ulHow, sizeof(FindJob.ulHow));
   WriteProfileData(inifile, "Find", "Areas", &FindJob.ulWAreas, sizeof(FindJob.ulWAreas));
   WriteProfileData(inifile, "Find", "PersMail", &FindJob.PersMailOpt, sizeof(FindJob.PersMailOpt));
   WriteProfileInt(inifile, "Find", "Fuzz", FindJob.ulFuzzyLevel);
   WriteProfileData(inifile, "Find", "Save", &bSaveResults, sizeof(bSaveResults));

   return 0;
}

static int ReadIniBrowse(HINI inifile)
{
   extern BROWSEROPTIONS BrowserOptions;
   ULONG datalen;

   memset(&BrowserOptions, 0, sizeof(BrowserOptions));
   BrowserOptions.lSplitbar = 150;
   BrowserOptions.bIcons    = TRUE;

   if (!inifile)
      return 1;

   datalen = sizeof(BrowserOptions.BrowserPos);
   QueryProfileData(inifile, "NLBrowse", "Pos", &BrowserOptions.BrowserPos, &datalen);

   BrowserOptions.ulLastMode = QueryProfileInt(inifile, "NLBrowse", "Mode", BROWSEMODE_NODE);
   BrowserOptions.lSplitbar  = QueryProfileInt(inifile, "NLBrowse", "Split", 150);
   BrowserOptions.bIcons     = QueryProfileInt(inifile, "NLBrowse", "Icons", 1);
   BrowserOptions.bNoPoints  = QueryProfileInt(inifile, "NLBrowse", "NoPoints", 0);

   QueryProfileString(inifile, "NLBrowse", "Domain", "", BrowserOptions.pchLastDomain, sizeof(BrowserOptions.pchLastDomain));

   return 0;
}

static int SaveIniBrowse(HINI inifile)
{
   extern BROWSEROPTIONS BrowserOptions;

   if (!inifile)
      return 1;

   if (!WriteProfileString(inifile, "NLBrowse", "Domain", BrowserOptions.pchLastDomain))
      DosBeep(100, 100);

   WriteProfileInt(inifile, "NLBrowse", "Mode", BrowserOptions.ulLastMode);
   WriteProfileInt(inifile, "NLBrowse", "Split", BrowserOptions.lSplitbar);
   WriteProfileInt(inifile, "NLBrowse", "Icons", BrowserOptions.bIcons);
   WriteProfileInt(inifile, "NLBrowse", "NoPoints", BrowserOptions.bNoPoints);
   WriteProfileData(inifile, "NLBrowse", "Pos", &BrowserOptions.BrowserPos, sizeof(BrowserOptions.BrowserPos));

   return 0;
}

static int SaveIniExport(HINI inifile)
{
   extern ULONG ulExportOptions;
   extern PATHNAMES pathnames;

   if (!inifile)
      return 1;

   WriteProfileInt(inifile, "Export", "Options", ulExportOptions);
   WriteProfileString(inifile, "Export", "Path", pathnames.lastexport);

   return 0;
}

static int ReadIniExport(HINI inifile)
{
   extern ULONG ulExportOptions;
   extern PATHNAMES pathnames;

   if (!inifile)
      return 1;

   ulExportOptions = QueryProfileInt(inifile, "Export", "Options", 0x01 | 0x02);

   QueryProfileString(inifile, "Export", "Path", "", pathnames.lastexport, LEN_PATHNAME+1);

   return 0;
}

static int ReadIniToolbar(HINI inifile)
{
   extern TOOLBAROPTIONS ToolbarOptions;
   extern TOOLBARCONFIG  ToolbarConfig;
   ULONG ulLen;

   if (!inifile)
   {
      LoadDefaultToolbar(&ToolbarConfig);
      return 1;
   }

   ToolbarOptions.ulToolbarPos = QueryProfileInt(inifile, "Toolbar", "Pos", 0);
   ToolbarOptions.bSmallToolbar = QueryProfileInt(inifile, "Toolbar", "Small", 0);

   ulLen = sizeof(WINPOS);
   QueryProfileData(inifile, "Toolbar", "CPos", &ToolbarConfig.SelectPos, &ulLen);
   ulLen = sizeof(LONG);
   ToolbarConfig.lLeftFore = RGB_BLACK;
   QueryProfileData(inifile, "Toolbar", "LFore", &ToolbarConfig.lLeftFore, &ulLen);
   ulLen = sizeof(LONG);
   ToolbarConfig.lRightFore = RGB_BLACK;
   QueryProfileData(inifile, "Toolbar", "RFore", &ToolbarConfig.lRightFore, &ulLen);
   ulLen = sizeof(LONG);
   ToolbarConfig.lLeftBack = RGB_WHITE;
   QueryProfileData(inifile, "Toolbar", "LBack", &ToolbarConfig.lLeftBack, &ulLen);
   ulLen = sizeof(LONG);
   ToolbarConfig.lRightBack = RGB_WHITE;
   QueryProfileData(inifile, "Toolbar", "RBack", &ToolbarConfig.lRightBack, &ulLen);
   QueryProfileString(inifile, "Toolbar", "LFont", "8.Helv", ToolbarConfig.pchLeftFont, sizeof(ToolbarConfig.pchLeftFont));
   QueryProfileString(inifile, "Toolbar", "RFont", "8.Helv", ToolbarConfig.pchRightFont, sizeof(ToolbarConfig.pchRightFont));

   if (!QueryProfileSize(inifile, "Toolbar", "Config", &ulLen))
      LoadDefaultToolbar(&ToolbarConfig);
   else
   {
      if (ulLen > 1)
      {
         ToolbarConfig.pButtons = malloc(ulLen);
         QueryProfileData(inifile, "Toolbar", "Config", ToolbarConfig.pButtons, &ulLen);
         ToolbarConfig.ulNumButtons = ulLen / sizeof(BUTTONCONFIG);
      }
      else
      {
         ToolbarConfig.pButtons = NULL;
         ToolbarConfig.ulNumButtons = 0;
      }
   }

   return 0;
}

static int SaveIniToolbar(HINI inifile)
{
   extern TOOLBAROPTIONS ToolbarOptions;
   extern TOOLBARCONFIG  ToolbarConfig;
   char pchBuffer[20];

   if (!inifile)
      return 1;

   if (dirtyflags.toolbardirty)
   {
      WriteProfileInt(inifile, "Toolbar", "Pos", ToolbarOptions.ulToolbarPos);
      WriteProfileInt(inifile, "Toolbar", "Small", ToolbarOptions.bSmallToolbar);
   }

   if (ToolbarConfig.bDirty)
   {
      WriteProfileData(inifile, "Toolbar", "CPos", &ToolbarConfig.SelectPos, sizeof(WINPOS));
      WriteProfileData(inifile, "Toolbar", "LFore", &ToolbarConfig.lLeftFore, sizeof(LONG));
      WriteProfileData(inifile, "Toolbar", "RFore", &ToolbarConfig.lRightFore, sizeof(LONG));
      WriteProfileData(inifile, "Toolbar", "LBack", &ToolbarConfig.lLeftBack, sizeof(LONG));
      WriteProfileData(inifile, "Toolbar", "RBack", &ToolbarConfig.lRightBack, sizeof(LONG));
      WriteProfileString(inifile, "Toolbar", "LFont", ToolbarConfig.pchLeftFont);
      WriteProfileString(inifile, "Toolbar", "RFont", ToolbarConfig.pchRightFont);

      if (ToolbarConfig.pButtons)
         WriteProfileData(inifile, "Toolbar", "Config", ToolbarConfig.pButtons, ToolbarConfig.ulNumButtons * sizeof(BUTTONCONFIG));
      else
      {
         pchBuffer[0]=0;
         WriteProfileData(inifile, "Toolbar", "Config", pchBuffer, 1);
      }

      ToolbarConfig.bDirty = FALSE;
   }


   return 0;
}

static int ReadIniEchoman(HINI inifile)
{
   extern ECHOMGROPT EchoMgrOpt;
   ULONG ulLen=0;
   PCHAR pchKeys, pchCurrentKey, pchEchoSeg;
   PCHAR pchData, pchTemp, pchBack;
   PUPLINK pUplink;
   long lUplink=0;

   EchoMgrOpt.lFolderBack = RGB_WHITE;
   strcpy(EchoMgrOpt.pchFolderFont, SMA_FONT);

   if (!inifile)
      return 1;

   /* Uplinks */
   if (QueryProfileSize(inifile, "Uplinks", NULL, &ulLen) && ulLen)
   {
      pchKeys = malloc(ulLen);
      QueryProfileData(inifile, "Uplinks", NULL, pchKeys, &ulLen);
      pchCurrentKey = pchKeys;
      pchData = malloc(0xffff);
      while (*pchCurrentKey)
      {
         if (*pchCurrentKey == '#') /* Uplink */
         {
            ulLen=0;
            if (QueryProfileSize(inifile, "Uplinks", pchCurrentKey, &ulLen) && ulLen)
            {
               pchTemp = pchData;
               QueryProfileData(inifile, "Uplinks", pchCurrentKey, pchData, &ulLen);

               /* Neuen Block */
               pUplink = calloc(1, sizeof(UPLINK));
               if (EchoMgrOpt.pUplinks)
                  EchoMgrOpt.pUplinksLast->next = pUplink;
               else
                  EchoMgrOpt.pUplinks = pUplink;
               pUplink->prev = EchoMgrOpt.pUplinksLast;
               EchoMgrOpt.pUplinksLast = pUplink;

               strcpy(pUplink->pchEchoMgrAddress, pchTemp);
               pchTemp = strchr(pchTemp, 0)+1;
               strcpy(pUplink->pchMyAddress, pchTemp);
               pchTemp = strchr(pchTemp, 0)+1;
               strcpy(pUplink->pchEchoMgrName, pchTemp);
               pchTemp = strchr(pchTemp, 0)+1;
               strcpy(pUplink->pchPassword, pchTemp);

               /* Echos zusammensuchen */
               lUplink = strtol(pchCurrentKey+1, NULL, 10);
               pchEchoSeg = pchKeys;
               while (*pchEchoSeg)
               {
                  if (*pchEchoSeg == '+' &&
                      strtol(pchEchoSeg+1, NULL, 10) == lUplink)
                  {
                     /* Echos anhaengen */
                     QueryProfileString(inifile, "Uplinks", pchEchoSeg, "", pchData, 0xffff);
                     ulLen = strlen(pchData)+1;
                     pchBack = pUplink->pchUplinkAreas;
                     if (pUplink->pchUplinkAreas)
                        ulLen += strlen(pUplink->pchUplinkAreas)+1;
                     pUplink->pchUplinkAreas = malloc(ulLen);
                     strcpy(pUplink->pchUplinkAreas, pchData);
                     if (pchBack)
                     {
                        strcat(pUplink->pchUplinkAreas, " ");
                        strcat(pUplink->pchUplinkAreas, pchBack);
                        free(pchBack);
                     }

                  }
                  while (*pchEchoSeg)
                     pchEchoSeg++;
                  pchEchoSeg++;
               }
            }
         }
         while (*pchCurrentKey)
            pchCurrentKey++;
         pchCurrentKey++;
      }
      free(pchData);
      free(pchKeys);
   }

   QueryProfileString(inifile, "EchoMan", "FFont", SMA_FONT, EchoMgrOpt.pchFolderFont, sizeof(EchoMgrOpt.pchFolderFont));
   QueryProfileString(inifile, "EchoMan", "DLL", "", EchoMgrOpt.pchDllName, sizeof(EchoMgrOpt.pchDllName));
   ulLen = sizeof(WINPOS);
   QueryProfileData(inifile, "EchoMan", "FPos", &EchoMgrOpt.FolderPos, &ulLen);
   QueryProfileData(inifile, "EchoMan", "SPos", &EchoMgrOpt.SettingsPos, &ulLen);
   ulLen = sizeof(LONG);
   QueryProfileData(inifile, "EchoMan", "FFore", &EchoMgrOpt.lFolderFore, &ulLen);
   ulLen = sizeof(LONG);
   QueryProfileData(inifile, "EchoMan", "FBack", &EchoMgrOpt.lFolderBack, &ulLen);

   if (QueryProfileSize(inifile, "EchoMan", "Param", &ulLen) && ulLen)
   {
      EchoMgrOpt.pDllParams = calloc(1, ulLen);
      EchoMgrOpt.ulParamLen = ulLen;
      QueryProfileData(inifile, "EchoMan", "Param", EchoMgrOpt.pDllParams, &ulLen);
   }

   return 0;
}

static int SaveIniEchoman(HINI inifile)
{
   extern ECHOMGROPT EchoMgrOpt;
   int iUpl=1, iEch;
   PUPLINK pUplink = EchoMgrOpt.pUplinks;
   char *pchBuffer, *pchTemp, *pchSrc, *pchLastSrc, *pchLastTemp;
   char pchNum[20]="#";
   char pchNumEchos[40];

   if (!inifile)
      return 1;

   WriteProfileData(inifile, "Uplinks", NULL, NULL, 0);
   pchBuffer = malloc(0xffff);
   while (pUplink)
   {
       /* Uplink-Daten */
       strcpy(pchBuffer, pUplink->pchEchoMgrAddress);
       pchTemp = strchr(pchBuffer, 0)+1;
       strcpy(pchTemp, pUplink->pchMyAddress);
       pchTemp = strchr(pchTemp, 0)+1;
       strcpy(pchTemp, pUplink->pchEchoMgrName);
       pchTemp = strchr(pchTemp, 0)+1;
       strcpy(pchTemp, pUplink->pchPassword);
       pchTemp = strchr(pchTemp, 0)+1;

       _itoa(iUpl, pchNum+1, 10);
       WriteProfileData(inifile, "Uplinks", pchNum, pchBuffer, pchTemp-pchBuffer);

       /* Echoliste */
       pchLastSrc = pchSrc = pUplink->pchUplinkAreas;
       iEch = 0;

       while (pchSrc && *pchSrc)
       {
          /* kopieren */
          pchLastTemp = pchTemp = pchBuffer;
          while (*pchSrc && (pchTemp - pchBuffer) < 0xfff0)
          {
             if (*pchSrc == ' ') /* Stelle merken */
             {
                pchLastSrc = pchSrc;
                pchLastTemp = pchTemp;
             }
             *pchTemp++ = *pchSrc++;
          }
          if (*pchSrc) /* Abbruch wg. Laenge */
          {
             /* wieder bei letztem Leerzeichen aufsetzen */
             pchSrc = pchLastSrc+1;
             pchTemp = pchLastTemp;
          }
          *pchTemp++ = 0;
          sprintf(pchNumEchos, "+%d %d", iUpl, ++iEch);
          WriteProfileData(inifile, "Uplinks", pchNumEchos, pchBuffer, pchTemp-pchBuffer);
       }

       iUpl++;
       pUplink = pUplink->next;
   }
   free(pchBuffer);

   if (EchoMgrOpt.pDllParams && EchoMgrOpt.ulParamLen)
      WriteProfileData(inifile, "EchoMan", "Param", EchoMgrOpt.pDllParams, EchoMgrOpt.ulParamLen);
   else
      WriteProfileData(inifile, "EchoMan", "Param", NULL, 0);

   WriteProfileString(inifile, "EchoMan", "FFont", EchoMgrOpt.pchFolderFont);
   WriteProfileString(inifile, "EchoMan", "DLL", EchoMgrOpt.pchDllName);
   WriteProfileData(inifile, "EchoMan", "FFore", &EchoMgrOpt.lFolderFore, sizeof(LONG));
   WriteProfileData(inifile, "EchoMan", "FBack", &EchoMgrOpt.lFolderBack, sizeof(LONG));
   WriteProfileData(inifile, "EchoMan", "FPos", &EchoMgrOpt.FolderPos, sizeof(WINPOS));
   WriteProfileData(inifile, "EchoMan", "SPos", &EchoMgrOpt.SettingsPos, sizeof(WINPOS));

   EchoMgrOpt.bDirty=FALSE;

   return 0;
}

static int ReadIniSearch(HINI inifile)
{
   extern SEARCHPAR SearchPar;
   ULONG ulLen;

   if (!inifile)
      return 1;

   ulLen = sizeof(SearchPar.DlgPos);
   QueryProfileData(inifile, "Search", "Pos", &SearchPar.DlgPos, &ulLen);
   ulLen = sizeof(SearchPar.ulSearchFlags);
   QueryProfileData(inifile, "Search", "Options", &SearchPar.ulSearchFlags, &ulLen);

   QueryProfileString(inifile, "Search", "Text", "", SearchPar.pchSearchText, sizeof(SearchPar.pchSearchText));

   return 0;
}

static int SaveIniSearch(HINI inifile)
{
   extern SEARCHPAR SearchPar;

   if (!inifile)
      return 1;

   WriteProfileData(inifile, "Search", "Pos", &SearchPar.DlgPos, sizeof(SearchPar.DlgPos));
   WriteProfileData(inifile, "Search", "Options", &SearchPar.ulSearchFlags, sizeof(SearchPar.ulSearchFlags));
   WriteProfileString(inifile, "Search", "Text", SearchPar.pchSearchText);

   return 0;
}

static int ReadIniPrintSetup(HINI inifile)
{
   extern PRINTSETUP PrintSetup;
   ULONG ulLen;
   UCHAR Borders[4]={20, 10, 5, 10};

   if (!inifile)
      return 1;

   QueryProfileString(inifile, "Print", "HeaderFont", "10.Courier", PrintSetup.pchHeaderFont, sizeof(PrintSetup.pchHeaderFont));
   QueryProfileString(inifile, "Print", "TextFont", "10.Courier", PrintSetup.pchTextFont, sizeof(PrintSetup.pchTextFont));
   QueryProfileString(inifile, "Print", "Queue", "", PrintSetup.szPreferredQueue, sizeof(PrintSetup.szPreferredQueue));

   ulLen = sizeof(PrintSetup.DlgPos);
   QueryProfileData(inifile, "Print", "Pos", &PrintSetup.DlgPos, &ulLen);

   ulLen = sizeof(PrintSetup.ulOutput);
   PrintSetup.ulOutput = OUTPUT_AREA | OUTPUT_ATTRIB | OUTPUT_DATE | OUTPUT_PAGENUM;
   QueryProfileData(inifile, "Print", "Output", &PrintSetup.ulOutput, &ulLen);

   ulLen = sizeof(Borders);
   QueryProfileData(inifile, "Print", "Borders", Borders, &ulLen);
   PrintSetup.lLeft   = Borders[0];
   PrintSetup.lRight  = Borders[1];
   PrintSetup.lTop    = Borders[2];
   PrintSetup.lBottom = Borders[3];

   ulLen=0;
   QueryProfileSize(inifile, "Print", "DData", &ulLen);
   if (ulLen)
   {
      PrintSetup.pDriverData = calloc(1, ulLen);
      QueryProfileData(inifile, "Print", "DData", PrintSetup.pDriverData, &ulLen);
   }

   return 0;
}

static int SaveIniPrintSetup(HINI inifile)
{
   extern PRINTSETUP PrintSetup;
   UCHAR Borders[4];

   if (!inifile)
      return 1;

   WriteProfileData(inifile, "Print", "Pos", &PrintSetup.DlgPos, sizeof(PrintSetup.DlgPos));
   WriteProfileData(inifile, "Print", "Output", &PrintSetup.ulOutput, sizeof(PrintSetup.ulOutput));
   Borders[0] = PrintSetup.lLeft;
   Borders[1] = PrintSetup.lRight;
   Borders[2] = PrintSetup.lTop;
   Borders[3] = PrintSetup.lBottom;
   WriteProfileData(inifile, "Print", "Borders", Borders, sizeof(Borders));
   if (PrintSetup.pDriverData)
      WriteProfileData(inifile, "Print", "DData", PrintSetup.pDriverData, PrintSetup.pDriverData->cb);
   else
      WriteProfileData(inifile, "Print", "DData", NULL, 0);
   WriteProfileString(inifile, "Print", "HeaderFont", PrintSetup.pchHeaderFont);
   WriteProfileString(inifile, "Print", "TextFont", PrintSetup.pchTextFont);
   WriteProfileString(inifile, "Print", "Queue", PrintSetup.szPreferredQueue);

   PrintSetup.bDirty = FALSE;

   return 0;
}


/*------------------------------ ConvertProfile  ----------------------------*/
/* Konvertiert ein Profile der vorherigen Version in ein neues               */
/* Rckgabewert: 0 altes Profile umgewandelt                                 */
/*               1 altes Profile unbekannt                                   */
/*---------------------------------------------------------------------------*/

static int ConvertProfile(HINI inifile, PCHAR pchThisVersion, HAB hab)
{
   BOOL converted=FALSE;

   if (!strcmp(pchThisVersion, O_INIVERSION13))
   {
      /* alte Marker-Items loeschen */
      WriteProfileData(inifile, "Mark", NULL, NULL, 0);
      converted = TRUE;
   }
   if (!strcmp(pchThisVersion, O_INIVERSION13) ||
       !strcmp(pchThisVersion, O_INIVERSION14))
   {
      /* Layout-Optionen loeschen */
      WriteProfileData(inifile, "Options", "Layout", NULL, 0);
      converted = TRUE;
   }

   if (!strcmp(pchThisVersion, O_INIVERSION13) ||
       !strcmp(pchThisVersion, O_INIVERSION14) ||
       !strcmp(pchThisVersion, O_INIVERSION15))
   {
      /* Nicknames in eigenes Ini */
      HINI hNewIni;

      if (hNewIni = PrfOpenProfile(hab, NICKFILENAME))
      {
         PCHAR pchKeys;
         ULONG ulKeysLen=0;
         PCHAR pchCurrentKey;
         BOOKADDRESSOPT OldNick;
         ULONG ulNewAttrib;
         ULONG ulNewFlags;
         ULONG ulReadSize;

         if (QueryProfileSize(inifile, "Nicknames", NULL, &ulKeysLen) && ulKeysLen)
         {
            pchKeys=malloc(ulKeysLen);

            if (QueryProfileData(inifile, "Nicknames", NULL, pchKeys, &ulKeysLen))
            {
               pchCurrentKey=pchKeys;

               while (*pchCurrentKey)
               {
                  ulReadSize = sizeof(OldNick);
                  QueryProfileData(inifile, "Nicknames", pchCurrentKey, &OldNick, &ulReadSize);

                  if (OldNick.username[0])
                     WriteProfileString(hNewIni, OldNick.usertag, "Name", OldNick.username);
                  if (OldNick.address[0])
                     WriteProfileString(hNewIni, OldNick.usertag, "Address", OldNick.address);
                  if (OldNick.subjectline[0])
                     WriteProfileString(hNewIni, OldNick.usertag, "Subj", OldNick.subjectline);
                  if (OldNick.firstline[0])
                     WriteProfileString(hNewIni, OldNick.usertag, "First", OldNick.firstline);

                  ulNewAttrib = 0;
                  if (OldNick.private)
                     ulNewAttrib |= ATTRIB_PRIVATE;
                  if (OldNick.crash)
                     ulNewAttrib |= ATTRIB_CRASH;
                  if (OldNick.kill)
                     ulNewAttrib |= ATTRIB_KILLSENT;
                  if (OldNick.attach)
                     ulNewAttrib |= ATTRIB_FILEATTACHED;
                  if (OldNick.request)
                     ulNewAttrib |= ATTRIB_FREQUEST;

                  if (ulNewAttrib)
                     WriteProfileInt(hNewIni, OldNick.usertag, "Attrib", ulNewAttrib);

                  ulNewFlags = 0;
                  if (OldNick.notemplate)
                     ulNewFlags |= NICKFLAG_NOTEMPLATE;

                  if (ulNewFlags)
                     WriteProfileInt(hNewIni, OldNick.usertag, "Flags", ulNewFlags);

                  /* Naechsten Key suchen */
                  while (*pchCurrentKey)
                     pchCurrentKey++;
                  pchCurrentKey++;
               }
               free(pchKeys);
               PrfCloseProfile(hNewIni);
            }
            else
            {
               free(pchKeys);
               PrfCloseProfile(hNewIni);
            }

         }
         /* Alte Nicknames loeschen */
         WriteProfileData(inifile, "Nicknames", NULL, NULL, 0);

         PrfCloseProfile(hNewIni);
      }
      converted = TRUE;
   }

   if (!strcmp(pchThisVersion, O_INIVERSION13) ||
       !strcmp(pchThisVersion, O_INIVERSION14) ||
       !strcmp(pchThisVersion, O_INIVERSION15) ||
       !strcmp(pchThisVersion, O_INIVERSION16))
   {
      /* Neue Area-Struktur */
      HINI AreasINI;
      ULONG ulKeysLen=0;

      if (AreasINI=PrfOpenProfile(hab, AREAFILENAME))
      {
         if (QueryProfileSize(AreasINI, NULL, NULL, &ulKeysLen) && ulKeysLen)
         {
            PCHAR pchKeys, pchCurrentKey;

            pchKeys=malloc(ulKeysLen);

            if (QueryProfileData(AreasINI, NULL, NULL, pchKeys, &ulKeysLen))
            {
               OLD_AREADEFOPT OldAreaOpt;
               USHORT usFormat;
               ULONG  ulAttrib;
               ULONG  ulOptions;
               ULONG  ulDataLen;

               pchCurrentKey=pchKeys;

               while (*pchCurrentKey)
               {
                  /* Alte Optionen lesen */
                  memset(&OldAreaOpt, 0, sizeof(OldAreaOpt));

                  ulDataLen=sizeof(SHORT)+2;
                  QueryProfileData(AreasINI, pchCurrentKey, "Options", &OldAreaOpt.areatype, &ulDataLen);

                  /* Optionen konvertieren */
                  usFormat = OldAreaOpt.areatype;

                  ulAttrib=0;
                  if (OldAreaOpt.defaultprivate)
                     ulAttrib |= ATTRIB_PRIVATE;
                  if (OldAreaOpt.defaultcrash)
                     ulAttrib |= ATTRIB_CRASH;
                  if (OldAreaOpt.defaultkillsent)
                     ulAttrib |= ATTRIB_KILLSENT;
                  if (OldAreaOpt.defaulthold)
                     ulAttrib |= ATTRIB_HOLD;

                  ulOptions=0;
                  if (OldAreaOpt.isfromcfg)
                     ulOptions |= AREAOPT_FROMCFG;
                  if (OldAreaOpt.separator)
                     ulOptions |= AREAOPT_SEPARATOR;
                  if (OldAreaOpt.eightbitascii)
                     ulOptions |= AREAOPT_HIGHASCII;

                  /* Optionen neu schreiben */
                  WriteProfileData(AreasINI, pchCurrentKey, "Format", &usFormat, sizeof(usFormat));
                  WriteProfileData(AreasINI, pchCurrentKey, "Attrib", &ulAttrib, sizeof(ulAttrib));
                  WriteProfileData(AreasINI, pchCurrentKey, "Options", &ulOptions, sizeof(ulOptions));

                  /* Naechsten Key suchen */
                  while (*pchCurrentKey)
                     pchCurrentKey++;
                  pchCurrentKey++;
               }
               free(pchKeys);
               PrfCloseProfile(AreasINI);
            }
            else
            {
               free(pchKeys);
               PrfCloseProfile(AreasINI);
            }

         }
         else
            PrfCloseProfile(AreasINI);
      }

      converted = TRUE;
   }

   if (!strcmp(pchThisVersion, O_INIVERSION13) ||
       !strcmp(pchThisVersion, O_INIVERSION14) ||
       !strcmp(pchThisVersion, O_INIVERSION15) ||
       !strcmp(pchThisVersion, O_INIVERSION16) ||
       !strcmp(pchThisVersion, O_INIVERSION17))
   {
      WriteProfileData(inifile, "Request", "Crash", NULL, 0);

      converted = TRUE;
   }

   if (!strcmp(pchThisVersion, O_INIVERSION13) ||
       !strcmp(pchThisVersion, O_INIVERSION14) ||
       !strcmp(pchThisVersion, O_INIVERSION15) ||
       !strcmp(pchThisVersion, O_INIVERSION16) ||
       !strcmp(pchThisVersion, O_INIVERSION17) ||
       !strcmp(pchThisVersion, O_INIVERSION18))
   {
      WriteProfileData(inifile, "Printer", NULL, NULL, 0);
      WriteProfileData(inifile, "Nodelist", "Options", NULL, 0);

      converted = TRUE;
   }

   if (!strcmp(pchThisVersion, O_INIVERSION13) ||
       !strcmp(pchThisVersion, O_INIVERSION14) ||
       !strcmp(pchThisVersion, O_INIVERSION15) ||
       !strcmp(pchThisVersion, O_INIVERSION16) ||
       !strcmp(pchThisVersion, O_INIVERSION17) ||
       !strcmp(pchThisVersion, O_INIVERSION18) ||
       !strcmp(pchThisVersion, O_INIVERSION19))
   {
      WriteProfileData(inifile, "Find", "Text", NULL, 0);

      converted = TRUE;
   }

   if (!strcmp(pchThisVersion, O_INIVERSION13) ||
       !strcmp(pchThisVersion, O_INIVERSION14) ||
       !strcmp(pchThisVersion, O_INIVERSION15) ||
       !strcmp(pchThisVersion, O_INIVERSION16) ||
       !strcmp(pchThisVersion, O_INIVERSION17) ||
       !strcmp(pchThisVersion, O_INIVERSION18) ||
       !strcmp(pchThisVersion, O_INIVERSION19) ||
       !strcmp(pchThisVersion, O_INIVERSION20) ||
       !strcmp(pchThisVersion, O_INIVERSION21))
   {
      WriteProfileData(inifile, "Options", "Msglist", NULL, 0);

      converted = TRUE;
   }

   if (converted)
      return 0;
   else
      return 1;
}

/* Stub-Funktionen */

static BOOL QueryProfileData(HINI hini, PSZ pszApp, PSZ pszKey, PVOID pBuffer, PULONG pulBufferMax)
{
   return PrfQueryProfileData(hini, pszApp, pszKey, pBuffer, pulBufferMax);
}

static LONG QueryProfileInt(HINI hini, PSZ pszApp, PSZ pszKey, LONG lDefault)
{
   char pchTemp[15];
   PrfQueryProfileString(hini, pszApp, pszKey, "", pchTemp, sizeof(pchTemp));
   if (pchTemp[0])
      return strtoul(pchTemp, NULL, 10);
   else
      return lDefault;
}

static BOOL QueryProfileSize(HINI hini, PSZ pszApp, PSZ pszKey, PULONG pDataLen)
{
   return PrfQueryProfileSize(hini, pszApp, pszKey, pDataLen);
}

static ULONG QueryProfileString(HINI hini, PSZ pszApp, PSZ pszKey, PSZ pszDefault, PVOID pBuffer,
                                ULONG cchBufferMax)
{
   return PrfQueryProfileString(hini, pszApp, pszKey, pszDefault, pBuffer, cchBufferMax);
}

static BOOL WriteProfileData(HINI hini, PSZ pszApp, PSZ pszKey, PVOID pBuffer, ULONG cchBufferMax)
{
   return PrfWriteProfileData(hini, pszApp, pszKey, pBuffer, cchBufferMax);
}

static BOOL WriteProfileString(HINI hini, PSZ pszApp, PSZ pszKey, PSZ pszData)
{
   return PrfWriteProfileString(hini, pszApp, pszKey, pszData);
}

static BOOL WriteProfileInt(HINI hini, PSZ pszApp, PSZ pszKey, LONG lData)
{
   char pchTemp[15];
   _itoa(lData, pchTemp, 10);
   return PrfWriteProfileString(hini, pszApp, pszKey, pchTemp);
}

static ULONG CFGFileErrorMessage(HWND hwndOwner, ULONG ErrorType)
{
   char pchFormat[100]="";
   char pchAddText[100]="";
   char pchMessage[200]="";

   LoadString(IDST_MSG_INITBASE, sizeof(pchFormat), pchFormat);
   LoadString(IDST_MSG_INITBASE+ErrorType, sizeof(pchAddText), pchAddText);
   sprintf(pchMessage, pchFormat, pchAddText);

   return WinMessageBox(HWND_DESKTOP, hwndOwner, pchMessage, NULL, IDD_INITERROR,
                        MB_OKCANCEL | MB_HELP | MB_ERROR | MB_MOVEABLE);
}

static ULONG CFGDLLErrorMessage(HWND hwndOwner, ULONG ErrorType)
{
   char pchFormat[100]="";
   char pchAddText[100]="";
   char pchMessage[200]="";

   LoadString(IDST_MSG_CFGDLLBASE, sizeof(pchFormat), pchFormat);
   LoadString(IDST_MSG_CFGDLLBASE+ErrorType, sizeof(pchAddText), pchAddText);
   sprintf(pchMessage, pchFormat, pchAddText);

   return WinMessageBox(HWND_DESKTOP, hwndOwner, pchMessage, NULL, IDD_CFGDLLERROR,
                        MB_OKCANCEL | MB_HELP | MB_ERROR | MB_MOVEABLE);
}

static ULONG ProfileErrorMessage(HWND hwndOwner, ULONG ErrorType)
{
   switch(ErrorType)
   {
      case INIFILE_DUPAREAS: /* Areas doppelt */
         return MessageBox(hwndOwner, IDST_MSG_DUPAREAS, 0,
                           IDD_DUPAREAS, MB_OKCANCEL | MB_ERROR);

      case INIFILE_OPEN:
         return MessageBox(hwndOwner, IDST_MSG_ERRORINIFILE, 0,
                           IDD_ERRORINIFILE, MB_OKCANCEL | MB_ERROR);

      case INIFILE_NEW:
         return MessageBox(hwndOwner, IDST_MSG_NOINIFILE, 0,
                           IDD_NOINIFILE, MB_OKCANCEL | MB_ERROR);

      case INIFILE_VERSION:
         return MessageBox(hwndOwner, IDST_MSG_OLDINIFILE, 0,
                           IDD_OLDINIFILE, MB_OKCANCEL | MB_ERROR);

      default:
         return 0;
   }
}

ULONG HandleInitErrors(HWND hwndClient, ULONG ulError)
{
   switch(ERRORCLASS(ulError))
   {
      case ERRORCLASS_INI:
         return ProfileErrorMessage(hwndClient, ERRORCODE(ulError));

      case ERRORCLASS_CFGDLL:
         return CFGDLLErrorMessage(hwndClient, ERRORCODE(ulError));

      case ERRORCLASS_CFGFILE:
         return CFGFileErrorMessage(hwndClient, ERRORCODE(ulError));

      default:
         return MBID_CANCEL;
   }
}

static int ReadIniFolders(HAB hab)
{
   HINI inifile;
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */
   ULONG ulDataLen;
   char pchIniFile[LEN_PATHNAME+1];
   extern FOLDERANCHOR FolderAnchor;
   AREAFOLDER AreaFolder;
   LONG HighID=0;

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, FOLDERFILENAME);

   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
   {
      AddDefaultFolder(&FolderAnchor);
      return 1;
   }

   if (!QueryProfileSize(inifile, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
   {
      PrfCloseProfile(inifile);
      AddDefaultFolder(&FolderAnchor);

      return 1;
   }

   pchKeys=malloc(ulKeysLen);

   if (!QueryProfileData(inifile, NULL, NULL, pchKeys, &ulKeysLen))
   {
      free(pchKeys);
      PrfCloseProfile(inifile);
      AddDefaultFolder(&FolderAnchor);
      return 1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      if (*pchCurrentKey == '#')
      {
         /* Folderdaten lesen */
         memset(&AreaFolder, 0, sizeof(AreaFolder));

         AreaFolder.FolderID = atoi(pchCurrentKey+1);
         if (AreaFolder.FolderID || *(pchCurrentKey+1) == '0')
         {
            AreaFolder.ParentFolder = QueryProfileInt(inifile, pchCurrentKey, "Parent", 0);
            AreaFolder.ulFlags = QueryProfileInt(inifile, pchCurrentKey, "Flags", 0);
            ulDataLen=0;
            QueryProfileSize(inifile, pchCurrentKey, "Title", &ulDataLen);
            if (ulDataLen)
            {
               AreaFolder.pchName = malloc(ulDataLen+1);
               QueryProfileString(inifile, pchCurrentKey, "Title", "noname", AreaFolder.pchName, ulDataLen+1);
            }
            else
               AreaFolder.pchName = strdup("noname");

            FM_AddFolder(&FolderAnchor, &AreaFolder, ADDFOLDER_TAIL);
            if (AreaFolder.FolderID > HighID)
               HighID = AreaFolder.FolderID;
         }
      }
      else
      {
         /* Folderliste */
         FolderAnchor.LastFolder = QueryProfileInt(inifile, pchCurrentKey, "Last", 0);
         FolderAnchor.HighID = QueryProfileInt(inifile, pchCurrentKey, "High", 0);
         FolderAnchor.ulFlags = QueryProfileInt(inifile, pchCurrentKey, "Flags", 0);
         FolderAnchor.lSplit = QueryProfileInt(inifile, pchCurrentKey, "Split", 50);
      }
      /* Naechsten Key suchen */
      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);
   PrfCloseProfile(inifile);

   /* notfalls high-ID anpassen */
   if (FolderAnchor.HighID < HighID)
      FolderAnchor.HighID = HighID;

   return 0;
}

static int SaveIniFolders(HAB hab)
{
   HINI inifile;
   char *pchKeys;             /* alle Keys */
   ULONG ulKeysLen;           /* Platz fuer die Keys */
   char *pchCurrentKey;       /* Aktueller Key */
   char pchIniFile[LEN_PATHNAME+1];
   extern FOLDERANCHOR FolderAnchor;
   PAREAFOLDER pAreaFolder;

   strcpy(pchIniFile, pchIniPath);
   strcat(pchIniFile, FOLDERFILENAME);

   if (!(inifile=PrfOpenProfile(hab, pchIniFile)))
      return 1;

   pAreaFolder=FolderAnchor.pList;

   while (pAreaFolder)
   {
      if (pAreaFolder->bDirty)
      {
         char pchTemp[20]="#";

         _itoa(pAreaFolder->FolderID, pchTemp+1, 10);

         WriteProfileString(inifile, pchTemp, "Title", pAreaFolder->pchName);
         if (pAreaFolder->ParentFolder)
            WriteProfileInt(inifile, pchTemp, "Parent", pAreaFolder->ParentFolder);
         else
            WriteProfileString(inifile, pchTemp, "Parent", NULL);
         if (pAreaFolder->ulFlags)
            WriteProfileInt(inifile, pchTemp, "Flags", pAreaFolder->ulFlags);
         else
            WriteProfileString(inifile, pchTemp, "Flags", NULL);

         pAreaFolder->bDirty=FALSE;
      }
      pAreaFolder=pAreaFolder->next;
   }

   /* ueberfluessige Areas im INI loeschen */
   if (!QueryProfileSize(inifile, NULL, NULL, &ulKeysLen) || (ulKeysLen==0))
   {
      PrfCloseProfile(inifile);
      return 1;
   }

   pchKeys=malloc(ulKeysLen);

   if (!QueryProfileData(inifile, NULL, NULL, pchKeys, &ulKeysLen))
   {
      free(pchKeys);
      PrfCloseProfile(inifile);
      return 1;
   }

   pchCurrentKey=pchKeys;

   while (*pchCurrentKey)
   {
      if (*pchCurrentKey == '#')
      {
         LONG id;

         id = atoi(pchCurrentKey+1);

         if (!FM_FindFolder(&FolderAnchor, id))
            WriteProfileData(inifile, pchCurrentKey, NULL, NULL, 0);
      }

      while (*pchCurrentKey)
         pchCurrentKey++;
      pchCurrentKey++;
   }
   free(pchKeys);

   /* allgemeine Daten */
   WriteProfileInt(inifile, "Folder", "Last", FolderAnchor.LastFolder);
   WriteProfileInt(inifile, "Folder", "High", FolderAnchor.HighID);
   WriteProfileInt(inifile, "Folder", "Flags", FolderAnchor.ulFlags);
   WriteProfileInt(inifile, "Folder", "Split", FolderAnchor.lSplit);

   FolderAnchor.bDirty = FALSE;

   PrfCloseProfile(inifile);

   return 0;
}

static int AddDefaultFolder(PFOLDERANCHOR pFolderAnchor)
{
   AREAFOLDER Folder;

   memset(&Folder, 0, sizeof(Folder));

   Folder.ulFlags = FOLDER_AUTOSCAN;
   Folder.pchName = malloc(50);
   Folder.pchName[0] = 0;
   LoadString(IDST_AL_DEFAULT, 50, Folder.pchName);

   FM_AddFolder(pFolderAnchor, &Folder, ADDFOLDER_HEAD | ADDFOLDER_MARKDIRTY);

   return 0;
}

BOOL LogoDisplayEnabled(void)
{
   LONG lLogo = QueryProfileInt(HINI_USER, "PM_ControlPanel", "LogoDisplayTime", 1);

   return (BOOL) lLogo;
}
/*------------------------------ Modulende ----------------------------------*/

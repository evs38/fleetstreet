/* CFGFILE_INTERFACE.H */

#define READCFG_AREAS        1UL
#define READCFG_USERDATA     2UL
#define READCFG_OUTBOUNDS    4UL
#define READCFG_ALL     (READCFG_AREAS |\
                         READCFG_USERDATA |\
                         READCFG_OUTBOUNDS)

#define CFGFILE_OK            0     /* kein Fehler */
#define CFGFILE_OPEN          1     /* Fehler beim ™ffnen eines Files */
#define CFGFILE_READ          2     /* Lesefehler */
#define CFGFILE_VERSION       3     /* Falsche CFG-File-Version */
#define CFGFILE_GENDATA       4     /* Fehler in allgemeinen Daten */
#define CFGFILE_AREA          5     /* Fehler in Area-Definition */
#define CFGFILE_NOAREA        6     /* keine Area definiert */


/* dynamisches Interface */

/* Ordinals */
#define ORDINAL_QUERYID        1
#define ORDINAL_QUERYNAME      2
#define ORDINAL_READCFG        3
#define ORDINAL_QUERYVER       4

/* Funktionstypen */
typedef ULONG (*QUERYFORMATID)(void);
typedef ULONG (*QUERYVER)(void);
typedef PCHAR (*QUERYFORMATNAME)(void);
typedef int   (*READCFGFILE)(char *, USERDATAOPT *, OUTBOUND *, PAREALIST,
                             PDRIVEREMAP, ULONG);

/* Versionsnummer */
#define CURRENT_CFGVER  0x46533032     /* == "FS02" */


/* Struktur */
typedef struct cfgdll
{
   struct cfgdll *next;

   ULONG ulFormatID;
   char  pchFormatName[50];
   char  pchDLLName[LEN_PATHNAME+1];
} CFGDLL, *PCFGDLL;

/* Funktionen */

PCFGDLL CFG_ReadFormatList(void);
PCFGDLL CFG_FindFormat(PCFGDLL pList, ULONG ulFormatID, PCFGDLL pPrev);

#define CFGTYPE_ANY 0xffffffff

typedef struct
{
   HMODULE         hmodCfgDLL;
   QUERYFORMATID   QueryFormatID;
   QUERYFORMATNAME QueryFormatName;
   QUERYVER        QueryVer;
   READCFGFILE     ReadCfgFile;
} LOADEDCFGDLL, *PLOADEDCFGDLL;

int CFG_LoadDLL(PCHAR pchDLLName, PLOADEDCFGDLL pLoadedCfgDLL);

#define LOADCFGDLL_OK           0
#define LOADCFGDLL_CANTLOAD     1
#define LOADCFGDLL_FUNCMISSING  2
#define LOADCFGDLL_VERSION      3
#define LOADCFGDLL_UNSUPPFORMAT 4

/* Ende CFGFILE_INTERFACE.H */

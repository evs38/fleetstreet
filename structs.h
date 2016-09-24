/* STRUCTS.H */
/* Alle eigenen Strukturen */

/* Kompakte Struktur f. Fensterposition */

#define WINPOS_VALID   0x01
#define WINPOS_MAX     0x02
#define WINPOS_MIN     0x04

#pragma pack(1)
typedef struct
{
   LONG  x;
   LONG  y;
   LONG  cx;
   LONG  cy;
   UCHAR uchFlags;
} WINPOS, *PWINPOS;

/* Strukturen fuer den Setup-Dialog */

/* Userdaten */

#define MAX_USERNAMES 50
#define MAX_ADDRESSES 100

typedef struct
{
   char username[MAX_USERNAMES][LEN_USERNAME+1];
   char address[MAX_ADDRESSES][LEN_5DADDRESS+1];
   char defaultorigin[LEN_ORIGIN+1];
} USERDATAOPT;

/* Message-Area-Definition */
#define AREAFORMAT_FTS    1
#define AREAFORMAT_SQUISH 2
#define AREAFORMAT_JAM    3
#define AREAFORMAT_XBBS   4
#define AREAFORMAT_RFC822 5
#define AREAFORMAT_OTHER  255

#define AREATYPE_ECHO   0
#define AREATYPE_NET    1
#define AREATYPE_LOCAL  2

#define AREAOPT_FROMCFG        0x0002UL
#define AREAOPT_SEPARATOR      0x0004UL
#define AREAOPT_HIGHASCII      0x0008UL
#define AREAOPT_NOHIGHLIGHT    0x0010UL
#define AREAOPT_MONOSPACED     0x0020UL

#define AREAFLAG_NOREMAP       0x0001UL

typedef struct
{
   char areatag[LEN_AREATAG+1];
   char areadesc[LEN_AREADESC+1];
   char address[LEN_5DADDRESS+1];
   char username[LEN_USERNAME+1];
   char pathfile[LEN_PATHNAME+1];
   ULONG ulTemplateID;
   ULONG ulFolderID;
   USHORT areaformat;    /* AREAFORMAT_* */
   USHORT areatype;      /* AREATYPE_* */
   ULONG  ulDefAttrib;   /* ATTRIB_* */
   ULONG  ulAreaOpt;     /* AREAOPT_* */
   ULONG  ulTempFlags;   /* AREAFLAG_* */
} AREADEFOPT, *PAREADEFOPT;

typedef struct _AREADEFLIST
{
   struct _AREADEFLIST *next;
   struct _AREADEFLIST *prev;
   AREADEFOPT areadata;
   ULONG  maxmessages;
   ULONG  currentmessage;
   BOOL   scanned;
   PVOID  areahandle;
   BOOL   mailentered;
   ULONG  oldlastread;
   PULONG msgnumlist;
   ULONG  usage;
   ULONG  flWork;
   BOOL   dirty;
   BOOL   bLocked;
} AREADEFLIST, *PAREADEFLIST;

/* Flags f. flWork */
#define WORK_NOTHING  0x00000000
#define WORK_SCAN     0x00000001
#define WORK_FIND     0x00000002
#define WORK_PERSMAIL 0x00000004
#define WORK_ETOSS    0x00000008

/* Macros */
typedef struct
{
   char macrotext[11][LEN_MACRO+1];
} MACROTABLEOPT;

/* Message-Template */
typedef struct _MSGTEMPLATE
{
   struct _MSGTEMPLATE *next;
   struct _MSGTEMPLATE *prev;
   SHORT quotelinelen;
   SHORT joinlen;
   BOOL useinitials;
   BOOL forwardfirst;
   BOOL randomorigin;
   PSZ THeader;
   PSZ TFooter;
   PSZ TReply;
   PSZ TDArea;
   PSZ TForward;
   PSZ TForwardFooter;
   PSZ TXPost;
   PSZ TCCopy;
   PSZ TName;
   char TOrigin[LEN_ORIGIN+1];
   char TOriginFile[LEN_PATHNAME+1];
   char TAllSyn[LEN_USERNAME+1];
   char chQuoteChar;
   WINPOS TPos;
   ULONG ulID;        /* Eindeutige ID, 0 = Default */
   BOOL bDirty;
} MSGTEMPLATE, *PMSGTEMPLATE;

typedef struct
{
   PMSGTEMPLATE pTemplates;  /* Liste der Templates      */
   BOOL    bDirty;           /* Dirty f. gesamte Liste   */
   ULONG   ulNumTemplates;   /* Anzahl der Templates     */
   WINPOS  FolderPos;        /* Template-Folder-Position */
   ULONG   ulFlags;          /* s.u.                     */
} TEMPLATELIST;

#define TEMPLATE_FOREGROUND     0x01UL

/* Nodelist */
#define MAX_MODEMTYPES      8
#define MAX_MODEMTYPES_BYTE 255

typedef struct
{
   ULONG ulOptions;  /* s.u. */

   /* bei Bit-Type: */
   char modemtype[MAX_MODEMTYPES][LEN_MODEMTYPE+1];

   /* bei Byte-Type: */
   char bytetypes[MAX_MODEMTYPES_BYTE][LEN_MODEMTYPE+1];
} NODELISTOPT;

#define MODEMFLAGS_BITTYPE    0x00UL
#define MODEMFLAGS_BYTETYPE   0x01UL


/* Allg. Optionen */
typedef struct
{
   BOOL dontuseapi;          /* unbenutzt */
   BOOL origininnet;
   BOOL usepid;
   BOOL tearinnet;
   BOOL scanatstartup;
   BOOL showarrive;          /* unbenutzt */
   BOOL beeponpersonal;
   BOOL namesinthreadlist;   /* unbenutzt */
   ULONG safety;
   BOOL uselastarea;
   char startarea[LEN_AREATAG+1];
   LONG lTabSize;
   LONG lMaxMsgLen;
   ULONG ulInstallTime;
   char jampath[LEN_PATHNAME+1];
   BOOL nohighlight;
   char LastCopyArea[LEN_AREATAG+1];
   char LastMoveArea[LEN_AREATAG+1];
   char LastForwardArea[LEN_AREATAG+1];
   char attachpath[LEN_PATHNAME+1];
   BOOL genFwdSubj;
   BOOL monospaced;
} GENERALOPT;

#define SAFETY_DELMSG       0x01UL
#define SAFETY_SHREDMSG     0x02UL
#define SAFETY_CATCHUP      0x04UL
#define SAFETY_EDITSENT     0x08UL
#define SAFETY_CHANGESETUP  0x10UL
#define SAFETY_DISCARD      0x20UL

typedef struct
{
   BOOL useechotoss;
   char pchEchoToss[LEN_PATHNAME+1];
} ECHOTOSSOPT;

/* Farbeinstellungen */
typedef struct
{
   LONG windowback;
   LONG editback;
   LONG editfore;
   LONG statusfore;
   LONG statusback;
   LONG framefore;
   LONG scriptfore;
   LONG scriptback;
   LONG areatitlefore;  /* unbenutzt */
   LONG areatitleback;  /* unbenutzt */
   LONG monitorfore;
   LONG monitorback;
   LONG fromfore;
   LONG fromback;
   LONG fromadfore;
   LONG fromadback;
   LONG tofore;
   LONG toback;
   LONG toadfore;
   LONG toadback;
   LONG subjfore;
   LONG subjback;
   LONG fromtostaticfore;
   LONG fromtostaticback;
   LONG attribfore;
   LONG attribback;
   LONG msgtimefore;
   LONG msgtimeback;
   LONG menufore;
   LONG menuback;
   LONG arealistfore;
   LONG arealistback;
   LONG tplfolderfore;
   LONG tplfolderback;
   LONG ccfolderfore;
   LONG ccfolderback;
   LONG threadlistfore;
   LONG threadlistback;
   LONG lookupfore;
   LONG lookupback;
   LONG cccontentsfore;
   LONG cccontentsback;
   LONG resultsfore;
   LONG resultsback;
   LONG nicknamesfore;
   LONG nicknamesback;
   LONG msginfofore;
   LONG msginfoback;
   LONG attachfore;
   LONG attachback;
   LONG viewerfore;
   LONG viewerback;
   LONG viewerquote;
   LONG viewertearline;
   LONG viewerorigin;
   LONG viewerquote2;
} WINDOWCOLORS;

/* Fonteinstellungen */

typedef struct
{
   char editfont[FACESIZE+5];       /* unbenutzt */
   char monitorfont[FACESIZE+5];
   char fromfont[FACESIZE+5];
   char fromadfont[FACESIZE+5];
   char tofont[FACESIZE+5];
   char toadfont[FACESIZE+5];
   char subjfont[FACESIZE+5];
   char attribfont[FACESIZE+5];
   char statusfont[FACESIZE+5];
   char scriptfont[FACESIZE+5];
   char datefont[FACESIZE+5];
   char buttonfont[FACESIZE+5];
   char fromtofont[FACESIZE+5];
   char msgnumberfont[FACESIZE+5];  /* unbenutzt */
   char menufont[FACESIZE+5];
   char arealistfont[FACESIZE+5];
   char tplfolderfont[FACESIZE+5];
   char findresultsfont[FACESIZE+5];
   char ccfolderfont[FACESIZE+5];
   char threadlistfont[FACESIZE+5];
   char cclistfont[FACESIZE+5];
   char lookupfont[FACESIZE+5];
   char msginfofont[FACESIZE+5];
   char nicknamesfont[FACESIZE+5];
   char attachfont[FACESIZE+5];
   char viewerfont[FACESIZE+5];
   char areafolderfont[FACESIZE+5];
   char viewermonofont[FACESIZE+5];
} WINDOWFONTS;

/* Fensterpositionen */

typedef struct
{
   WINPOS mainwindowpos;
   WINPOS optionspos;
   WINPOS aboutpos;
   WINPOS replypos;
   WINPOS replylistpos;
   WINPOS findpos;
   WINPOS persmailpos;
   WINPOS findresultspos;
   WINPOS attribpos;
   WINPOS importpos;
   WINPOS exportpos;
   WINPOS kludgeinfopos;
   WINPOS newuserpos;
   WINPOS newaddresspos;
   WINPOS areasetuppos;
   WINPOS requesterpos;
   WINPOS currnamepos;
   WINPOS ccentrypos;
   WINPOS ccselectpos;
   WINPOS nickentrypos;
   WINPOS promptpos;
   WINPOS sqparamspos;
   WINPOS lookuppos;
   WINPOS domainentrypos;
   WINPOS renumberpos;
   WINPOS attachpos;
   WINPOS mlistsettingspos;
} WINDOWPOSITIONS;


/* Pfadnamen */

typedef struct
{
   char lastimport[LEN_PATHNAME+1];
   char lastexport[LEN_PATHNAME+1];
   char squishcfg[LEN_PATHNAME+1];
} PATHNAMES;

/* alles andere */

typedef struct
{
   char lastfindtext[LEN_FINDTEXT+1];  /* unbenutzt */
   char lastarearead[LEN_AREATAG+1];
   LONG lastreadoffset;       /* Offset im SQL-File */
   BOOL readcfg;
   BOOL globalexclude;
   ULONG ulCfgType;
} MISCOPTIONS;

/* Infos fuer Uhrdarstellung */

typedef struct
{
   int DMY;        /* 0=MDY, 1=DMY, 2=YMD */
   char datesep;   /* Datumsseparator */
   char timesep;   /* Zeitseparator */
   BOOL h24;       /* 24-Stunden-Uhr */
   char amtext[4];     /* AM-Text */
   char pmtext[4];     /* PM-Text */
} INTLSETTING;

/* Dirty-Flags fuer das Speichern der Settings */

typedef struct
{
   BOOL userdirty;
   BOOL macrosdirty;
   BOOL templatedirty;
   BOOL nodedirty;
   BOOL optionsdirty;
   BOOL hooksdirty;
   BOOL echotossdirty;
   BOOL domainsdirty;
   BOOL finddirty;
   BOOL threadsdirty;
   BOOL lookupdirty;
   BOOL resultsdirty;
   BOOL alsettingsdirty;
   BOOL mlsettingsdirty;
   BOOL remapdirty;
   BOOL browserdirty;
   BOOL toolbardirty;
} DIRTYFLAGS;

/* Squish-Outbounds und Domains */

typedef struct
{
   int  zonenum;
   char outbound[LEN_PATHNAME+1];
} OUTBOUND;

typedef struct _domains
{
   struct _domains *next;
   char domainname[LEN_DOMAIN+1];
   char indexfile[LEN_PATHNAME+1];
   char nodelistfile[LEN_PATHNAME+1];
} DOMAINS, *PDOMAINS;

/* Threadlisten-Optionen */

#define DSPTHREADS_ALL         0
#define DSPTHREADS_WITHUNREAD  1
#define DSPTHREADS_UNREADONLY  2

typedef struct
{
   BOOL shownames;
   ULONG dspmode;
   BOOL keepinfront;
   WINPOS SettingsPos;
   LONG lBackClr;
   LONG lReadClr;
   LONG lUnreadClr;
   LONG lPersonalClr;
   WINPOS ListPos;
   BOOL compact;
} THREADLISTOPTIONS;

/* Squish-Parameter */

typedef struct
{
   ULONG ulSkipMsgs;
   ULONG ulMaxMsgs;
   USHORT usDaysToKeep;
} SQUISHPARAMS, *PSQUISHPARAMS;

typedef struct
{
   LONG  lSplitBar;
   BOOL  bBrief;
} LOOKUPOPTIONS;

typedef struct
{
   LONG lSplitBar;
   BOOL bScroll;
   BOOL keepinfront;
} RESULTSOPTIONS;

typedef struct
{
   ULONG ulDummy;
   ULONG ulDummy2;
   LONG  lBackColor;
   LONG  lNetAreaColor;
   LONG  lEchoAreaColor;
   LONG  lLocalAreaColor;
   ULONG ulFlags;
   WINPOS ListPos;
   WINPOS SettingsPos;
   WINPOS SelectPos;
   LONG  lFolderFore;
   LONG  lFolderBack;
} AREALISTOPTIONS, *PAREALISTOPTIONS;

#define AREALISTFLAG_FOREGROUND 0x01UL
#define AREALISTFLAG_SHOWTAGS   0x02UL

typedef struct
{
   char pchRemapString[25];
} DRIVEREMAP, *PDRIVEREMAP;

/* Carbon Copy listen */

typedef struct _ccentry
{
   struct _ccentry *next;
   struct _ccentry *prev;
   char   pchName[LEN_USERNAME+1];
   char   pchAddress[LEN_5DADDRESS+1];
   char   pchFirstLine[LEN_FIRSTLINE+1];
   ULONG  ulFlags;     /* siehe CCENTRY_* unten */
} CCENTRY, *PCCENTRY;

#define CCENTRY_MENTION  0x00000001   /* Name in CC-Zeilen zeigen */

typedef struct _cclist
{
   struct _cclist *next;
   struct _cclist *prev;
   char     *pchListName;
   PCCENTRY pEntries;
   WINPOS   ListPos;
   ULONG    ulListID;
   ULONG    ulFlags;   /* siehe CCLIST_* unten */
   BOOL     bDirty;
} CCLIST, *PCCLIST;

#define CCLIST_KILLSENT   0x00000001  /* Messages 2..n mit Kill/Sent */

typedef struct
{
   PCCLIST pLists;
   ULONG   ulNumLists;
   WINPOS  FolderPos;
   BOOL    bDirty;
   ULONG   ulFlags;
} CCANCHOR, *PCCANCHOR;

#define CCANCHOR_FOREGROUND     0x01UL


/* Rexx-Scripts */

typedef struct rxscript
{
   struct rxscript *next;
   struct rxscript *prev;
   char   *pchScriptName;
   char   pchPathName[LEN_PATHNAME+1];
   WINPOS SettingsPos;
   WINPOS MonitorPos;
   ULONG  ulScriptID;
   ULONG  ulFlags;
   BOOL   bDirty;
} RXSCRIPT, *PRXSCRIPT;

#define REXXFLAG_NOMONITOR        0x00000001
#define REXXFLAG_AUTOCLOSE        0x00000002
#define REXXFLAG_QUICKACCESS      0x00000004

typedef struct scriptlist
{
   PRXSCRIPT pScripts;
   ULONG     ulNumScripts;
   WINPOS    FolderPos;
   WINPOS    FolderSettingsPos;
   BOOL      bDirty;
   ULONG     ulFlags;
} SCRIPTLIST, *PSCRIPTLIST;

#define SCRIPTS_FOREGROUND     0x01UL

typedef struct
{
   ULONG    ulExitID;
   ULONG    ulPreSaveID;
} REXXHOOKS, *PREXXHOOKS;

typedef struct
{
   USHORT cb;
   ULONG  ulView;
} BOOKMARKSOPEN, *PBOOKMARKSOPEN;


typedef struct
{
   ULONG ulToolbarPos;
   BOOL  bSmallToolbar;
} TOOLBAROPTIONS, *PTOOLBAROPTIONS;

#define TOOLBARPOS_TOP       0
#define TOOLBARPOS_BOTTOM    1
#define TOOLBARPOS_LEFT      2
#define TOOLBARPOS_RIGHT     3

/* File-Requests */

typedef struct requestlist
{
   struct requestlist *next;
   char pchFileName[LEN_REQFILE+1];  /* Filename 8.3 */
   char pchPassword[LEN_PASSWORD+1]; /* "" fÅr 'kein Password' */
} REQUESTLIST, *PREQUESTLIST;

/* offene Fenster */

typedef struct
{
   ULONG ulOpenWindows;
   ULONG ulForceClose;
   ULONG ulForceOpen;
} OPENWIN, *POPENWIN;

#define OPENWIN_AREA            1UL
#define OPENWIN_MSGL            2UL
#define OPENWIN_THRL            4UL
#define OPENWIN_BOOKMARKS       8UL
#define OPENWIN_KLUDGES        16UL
#define OPENWIN_CCLISTS        32UL
#define OPENWIN_TPL            64UL
#define OPENWIN_REXX          128UL
#define OPENWIN_BROWSER       256UL


#pragma pack()


/*
   FMSTRUCT.H

        File structures for FMail 1.46
        Copyright (C) 1999 Folkert J. Wijnstra. All rights reserved.

	All information in this document is subject to change at any time
	without prior notice!

	Strings are NUL terminated arrays of char type.
	Path names always end on a \ character (followed by NUL).
*/


/**** Modify the type definitions below if necessary for your compiler ****/

#define fhandle signed int
#define u8    unsigned char
#define uchar unsigned char
#define schar char
#ifndef __OS2__
#define u16 unsigned int
#define s16 signed int
#else
#define u16 short unsigned int
#define s16 short signed int
#endif
#define u32 long unsigned int
#define s32 long signed int
#define udef unsigned int
#define sdef signed int

#define MAX_U32	0xFFFFFFFFL


/* ********** General structures ********** */

typedef struct
{  uchar programName[46];
   u16   memRequired;      } archiverInfo;

typedef uchar pathType[48];

typedef struct
{  u16   zone;
   u16   net;
   u16   node;
   u16   point; } nodeNumType;

typedef struct
{  unsigned readOnly   : 1;  /* Bit  0    */
   unsigned writeOnly  : 1;  /* Bit  1    */
   unsigned locked     : 1;  /* Bit  2    */
   unsigned reserved   : 13; /* Bits 3-15 */   } nodeFlagsType;

typedef struct
{  nodeNumType   nodeNum;
   nodeFlagsType flags;     } nodeNumXType;

typedef struct
{  nodeNumType nodeNum;
   u16         fakeNet; } nodeFakeType;


/* ********** File header structure ********** */

#define DATATYPE_CF    0x0102 /* not used yet			  */
#define DATATYPE_NO    0x0202 /* node file                        */
#define DATATYPE_AD    0x0401 /* area file for echo mail defaults */
#define DATATYPE_AE    0x0402 /* area file for echo mail          */

typedef struct
{  uchar versionString[32]; /* Always starts with 'FMail' */
   u16   revNumber;         /* Is now 0x0100 */
   u16   dataType;          /* See #defines above */
   u16   headerSize;
   s32   creationDate;
   s32   lastModified;
   u16   totalRecords;
   u16   recordSize;
} headerType;


/* The structure below is used by the Areas File and (only partly)
   by the Config File */

typedef struct
{  unsigned active      : 1; /* Bit  0 */
   unsigned tinySeenBy  : 1; /* Bit  1 */
   unsigned security    : 1; /* Bit  2 */
   unsigned             : 1; /* Bit  3 */
   unsigned allowPrivate: 1; /* Bit  4 */
   unsigned impSeenBy   : 1; /* Bit  5 */
   unsigned checkSeenBy : 1; /* Bit  6 */
   unsigned             : 1; /* Bit  7 */
   unsigned local       : 1; /* Bit  8 */
   unsigned disconnected: 1; /* Bit  9 */
   unsigned _reserved   : 1; /* Bit 10 */
   unsigned allowAreafix: 1; /* Bit 11 */
   unsigned export2BBS  : 1; /* Bit 12 */
   unsigned             : 1; /* Bit 13 */
   unsigned arrivalDate : 1; /* Bit 14 */
   unsigned sysopRead   : 1; /* Bit 15 */   } areaOptionsType;


/* ********** FMAIL.CFG ********** */

#define MAX_AKAS      32
#define MAX_AKAS_F    64
#define MAX_AKAS_OLD  16
#define MAX_NA_OLD    11
#define MAX_NETAKAS   32
#define MAX_NETAKAS_F 64
#define MAX_USERS     16
#define MAX_UPLREQ    32
#define MAX_MATCH     16           /* not used yet */

#define LOG_NEVER     0x0000
#define LOG_INBOUND   0x0001
#define LOG_OUTBOUND  0x0002
#define LOG_PKTINFO   0x0004
#define LOG_XPKTINFO  0x0008
#define LOG_UNEXPPWD  0x0010
#define LOG_SENTRCVD  0x0020
#define LOG_STATS     0x0040
#define LOG_PACK      0x0080
#define LOG_MSGBASE   0x0100
#define LOG_ECHOEXP   0x0200
#define LOG_NETIMP    0x0400
#define LOG_NETEXP    0x0800
#define LOG_OPENERR   0x1000
#define LOG_EXEC      0x2000
#define LOG_NOSCRN    0x4000
#define LOG_ALWAYS    0x8000
#define LOG_DEBUG     0x8000

typedef nodeFakeType _akaListType[MAX_AKAS_OLD];
typedef nodeFakeType akaListType[MAX_AKAS_F];

typedef struct
{
   unsigned useEMS       : 1; /* BIT 0 */
   unsigned checkBreak   : 1; /* BIT 1 */
   unsigned swap         : 1; /* BIT 2 */
   unsigned swapEMS      : 1; /* BIT 3 */
   unsigned swapXMS      : 1; /* BIT 4 */
   unsigned lfn          : 1; /* BIT 5 */
   unsigned monochrome   : 1; /* BIT 6 */
   unsigned commentFFD   : 1; /* BIT 7 */
   unsigned PTAreasBBS   : 1; /* BIT 8 */
   unsigned commentFRA   : 1; /* BIT 9 */
   unsigned              : 1; /* BIT 10 */
   unsigned incBDRRA     : 1; /* BIT 11 */
   unsigned ctrlcodes    : 1; /* BIT 12 */
   unsigned timeSliceFM  : 1; /* BIT 13 */
   unsigned timeSliceFT  : 1; /* BIT 14 */
   unsigned _RA2         : 1; /* BIT 15 */  } genOptionsType;

typedef struct
{
   unsigned removeNetKludges : 1; /* Bit 0 */
   unsigned addPointToPath   : 1; /* Bit 1 */
   unsigned checkPktDest     : 1; /* Bit 2 */
   unsigned neverARC060      : 1; /* Bit 3 */
   unsigned createSema       : 1; /* Bit 4 */
   unsigned dailyMail	     : 1; /* Bit 5 */
   unsigned warnNewMail      : 1; /* bit 6 */
   unsigned killBadFAtt      : 1; /* Bit 7 */
   unsigned dupDetection     : 1; /* Bit 8 */
   unsigned ignoreMSGID      : 1; /* Bit 9 */
   unsigned ARCmail060       : 1; /* Bit 10 */
   unsigned extNames         : 1; /* Bit 11 */
   unsigned persNetmail      : 1; /* Bit 12 */
   unsigned privateImport    : 1; /* Bit 13 */
   unsigned keepExpNetmail   : 1; /* Bit 14 */
   unsigned killEmptyNetmail : 1; /* Bit 15 */  } mailOptionsType;

typedef struct
{
   unsigned sortNew      : 1; /* bit  0   */
   unsigned sortSubject  : 1; /* bit  1   */
   unsigned updateChains : 1; /* bit  2   */
   unsigned reTear       : 1; /* bit  3   */
   unsigned              : 1; /* bit  4   */
   unsigned              : 1; /* bit  5   */
   unsigned removeRe     : 1; /* bit  6   */
   unsigned removeLfSr   : 1; /* bit  7   */
   unsigned scanAlways   : 1; /* bit  8   */
   unsigned scanUpdate   : 1; /* bit  9   */
   unsigned multiLine    : 1; /* bit 10   */
   unsigned              : 1; /* bit 11   */
   unsigned quickToss    : 1; /* bit 12   */
   unsigned checkNetBoard: 1; /* bit 13   */
   unsigned              : 1; /* bit 14   */
   unsigned sysopImport  : 1; /* bit 15   */ } mbOptionsType;

typedef struct
{
   unsigned keepRequest  : 1; /* Bit  0 */
   unsigned keepReceipt  : 1; /* Bit  1 */
   unsigned sendUplArList: 1; /* Bit  2 */
   unsigned              : 1; /* Bit  3 */
   unsigned autoDiscArea : 1; /* Bit  4 */
   unsigned autoDiscDel  : 1; /* Bit  5 has temp. no effect, rec is always deleted */
   unsigned              : 3; /* Bit 6-8 */
   unsigned allowAddAll  : 1; /* Bit  9 */
   unsigned allowActive  : 1; /* Bit 10 */
   unsigned allowBCL     : 1; /* Bit 11 */
   unsigned allowPassword: 1; /* Bit 12 */
   unsigned allowPktPwd  : 1; /* Bit 13 */
   unsigned allowNotify  : 1; /* Bit 14 */
   unsigned allowCompr   : 1; /* Bit 15 */  } mgrOptionsType;

typedef struct
{
   unsigned smtpImm : 1;  /* BIT 0 */
   unsigned         : 15; /* BIT 1-15 */  } inetOptionsType;

typedef struct
{
   unsigned addPlusPrefix :  1; /* BIT 0 */
   unsigned               :  3;
   unsigned unconditional :  1; /* BIT 4 */
   unsigned               : 11;    } uplOptType;

typedef struct
{
   uchar userName[36];
   uchar reserved[28];
} userType;

typedef struct
{
   nodeNumType node;
   uchar       program[9];
   uchar       password[17];
   uchar       fileName[13];
   uchar       fileType;
   u32         groups;
   uchar       originAka;
   uplOptType  options;
   uchar       reserved[9];  } uplinkReqType;

typedef struct
{  u16   valid;
   u16   zone;
   u16   net;
   u16   node;    } akaMatchNodeType;

typedef struct
{
   akaMatchNodeType amNode;
   u16              aka;    } akaMatchType;

/* ATTENTION: FMAIL.CFG does NOT use the new config file type yet (no header) !!! */

typedef struct
{
   uchar           versionMajor;
   uchar           versionMinor;
   s32             creationDate;
   u32             key;
   u32             reservedKey;
   u32             relKey1;
   u32             relKey2;
   uchar           reserved1[18];
   inetOptionsType inetOptions;
   u16             maxForward;
   mgrOptionsType  mgrOptions;
   _akaListType    _akaList;
   u16             _netmailBoard[MAX_NA_OLD];
   u16             _reservedNet[16-MAX_NA_OLD];
   genOptionsType  genOptions;
   mbOptionsType   mbOptions;
   mailOptionsType mailOptions;
   u16             maxPktSize;
   u16             kDupRecs;
   u16             mailer;
   u16             bbsProgram;
   u16             maxBundleSize;
   u16             extraHandles; /* 0-235 */
   u16             autoRenumber;
   u16             bufSize;
   u16             ftBufSize;
   u16             allowedNumNetmail;
   u16             logInfo;
   u16             logStyle;
   u16             activTimeOut;
   u16             maxMsgSize;
   uchar           reserved2[64];
   u16             colorSet;
   uchar           sysopName[36];
   u16             defaultArc;
   u16             _adiscDaysNode;
   u16             _adiscDaysPoint;
   u16             _adiscSizeNode;
   u16             _adiscSizePoint;
   uchar           reserved3[16];
   uchar           tearType;
   uchar           tearLine[25];
   pathType        summaryLogName;
   u16             recBoard;
   u16             badBoard;
   u16             dupBoard;
   uchar           topic1[16];
   uchar           topic2[16];
   pathType        bbsPath;
   pathType        netPath;
   pathType        sentPath;
   pathType        rcvdPath;
   pathType        inPath;
   pathType        outPath;
   pathType        securePath;
   pathType        logName;
   pathType        swapPath;
   pathType        semaphorePath;
   pathType        pmailPath;
   pathType        areaMgrLogName;
   pathType        autoRAPath;
   pathType        autoFolderFdPath;
   pathType        autoAreasBBSPath;
   pathType        autoGoldEdAreasPath;
   archiverInfo    unArc;
   archiverInfo    unZip;
   archiverInfo    unLzh;
   archiverInfo    unPak;
   archiverInfo    unZoo;
   archiverInfo    unArj;
   archiverInfo    unSqz;
   archiverInfo    GUS;
   archiverInfo    arc;
   archiverInfo    zip;
   archiverInfo    lzh;
   archiverInfo    pak;
   archiverInfo    zoo;
   archiverInfo    arj;
   archiverInfo    sqz;
   archiverInfo    customArc;
   pathType        autoFMail102Path;
   uchar           reserved4[35];
   areaOptionsType _optionsAKA[MAX_NA_OLD];
   uchar           _groupsQBBS[MAX_NA_OLD];
   u16             _templateSecQBBS[MAX_NA_OLD];
   uchar           _templateFlagsQBBS[MAX_NA_OLD][4];
   uchar           _attr2RA[MAX_NA_OLD];
   uchar           _aliasesQBBS[MAX_NA_OLD];
   u16             _groupRA[MAX_NA_OLD];
   u16             _altGroupRA[MAX_NA_OLD][3];
   uchar           _qwkName[MAX_NA_OLD][13];
   u16             _minAgeSBBS[MAX_NA_OLD];
   u16             _daysRcvdAKA[MAX_NA_OLD];
   uchar           _replyStatSBBS[MAX_NA_OLD];
   u16             _attrSBBS[MAX_NA_OLD];
   uchar           groupDescr[26][27];
   uchar           reserved5[9];
   uchar           _msgKindsRA[MAX_NA_OLD];
   uchar           _attrRA[MAX_NA_OLD];
   u16             _readSecRA[MAX_NA_OLD];
   uchar           _readFlagsRA[MAX_NA_OLD][4];
   u16             _writeSecRA[MAX_NA_OLD];
   uchar           _writeFlagsRA[MAX_NA_OLD][4];
   u16             _sysopSecRA[MAX_NA_OLD];
   uchar           _sysopFlagsRA[MAX_NA_OLD][4];
   u16             _daysAKA[MAX_NA_OLD];
   u16             _msgsAKA[MAX_NA_OLD];
   uchar           _descrAKA[MAX_NA_OLD][51];
   userType        users[MAX_USERS];
   akaMatchType    akaMatch[MAX_MATCH];     /* not used yet */
   uchar           reserved6[752-10*MAX_MATCH];
   uchar           emailAddress[80];        /* max 56 chars used */
   uchar           pop3Server[80];          /* max 56 chars used */
   uchar           smtpServer[80];          /* max 56 chars used */
   pathType        tossedAreasList;
   pathType        sentEchoPath;
   archiverInfo    preUnarc;
   archiverInfo    postUnarc;
   archiverInfo    preArc;
   archiverInfo    postArc;
   archiverInfo    unUc2;
   archiverInfo    unRar;
   archiverInfo    unJar;
   archiverInfo    resUnpack[5];
   archiverInfo    uc2;
   archiverInfo    rar;
   archiverInfo    jar;
   archiverInfo    resPack[5];
   uplinkReqType   uplinkReq[MAX_UPLREQ+32];
   archiverInfo    unArc32;
   archiverInfo    unZip32;
   archiverInfo    unLzh32;
   archiverInfo    unPak32;
   archiverInfo    unZoo32;
   archiverInfo    unArj32;
   archiverInfo    unSqz32;
   archiverInfo    unUc232;
   archiverInfo    unRar32;
   archiverInfo    GUS32;
   archiverInfo    unJar32;
   archiverInfo    resUnpack32[5];
   archiverInfo    preUnarc32;
   archiverInfo    postUnarc32;
   archiverInfo    arc32;
   archiverInfo    zip32;
   archiverInfo    lzh32;
   archiverInfo    pak32;
   archiverInfo    zoo32;
   archiverInfo    arj32;
   archiverInfo    sqz32;
   archiverInfo    uc232;
   archiverInfo    rar32;
   archiverInfo    customArc32;
   archiverInfo    jar32;
   archiverInfo    resPack32[5];
   archiverInfo    preArc32;
   archiverInfo    postArc32;
   uchar           descrAKA[MAX_NETAKAS][51];
   uchar           qwkName[MAX_NETAKAS][13];
   areaOptionsType optionsAKA[MAX_NETAKAS];
   uchar           msgKindsRA[MAX_NETAKAS];
   u16             daysAKA[MAX_NETAKAS];
   u16             msgsAKA[MAX_NETAKAS];
   uchar           groupsQBBS[MAX_NETAKAS];
   uchar           attrRA[MAX_NETAKAS];
   uchar           attr2RA[MAX_NETAKAS];
   u16             attrSBBS[MAX_NETAKAS];
   uchar           aliasesQBBS[MAX_NETAKAS];
   u16             groupRA[MAX_NETAKAS];
   u16             altGroupRA[MAX_NETAKAS][3];
   u16             minAgeSBBS[MAX_NETAKAS];
   u16             daysRcvdAKA[MAX_NETAKAS];
   uchar           replyStatSBBS[MAX_NETAKAS];
   u16             readSecRA[MAX_NETAKAS];
   uchar           readFlagsRA[MAX_NETAKAS][8];
   u16             writeSecRA[MAX_NETAKAS];
   uchar           writeFlagsRA[MAX_NETAKAS][8];
   u16             sysopSecRA[MAX_NETAKAS];
   uchar           sysopFlagsRA[MAX_NETAKAS][8];
   u16             templateSecQBBS[MAX_NETAKAS];
   uchar           templateFlagsQBBS[MAX_NETAKAS][8];
   uchar           reserved7[512];
   u16             netmailBoard[MAX_NETAKAS_F];
   akaListType     akaList;
   } configType;



/* ********** FMAIL.AR ********** */

#if defined(__FMAILX__) || defined(__OS2__)
#define MAX_AREAS   4096
#else
#define MAX_AREAS    512
#endif

/*#define MAX_FORWARD   64*/
#define MAX_FORWARDOLD  64
#define MAX_FORWARDDEF 256    /* NOTE: this is a dummy. 'config.maxForward' */
                              /* contains the real size. 256 is the maximum.*/
#define MAX_FORWARD    config.maxForward
#define RAWECHO_SIZE   sizeof(rawEchoType)

#define MB_PATH_LEN_OLD   19
#define MB_PATH_LEN       61
#define ECHONAME_LEN_090  25
#define ECHONAME_LEN      51
#define COMMENT_LEN       51
#define ORGLINE_LEN       59

typedef uchar areaNameType[ECHONAME_LEN];

/* NOTE: See above for the file header structure !!! */

typedef struct
{
   unsigned tossedTo     : 1;  /* BIT 0 */
   unsigned              : 15; /* BIT 1-15 */  } areaStatType;

typedef struct
{
   u16             signature; /* contains "AE" for echo areas in FMAIL.AR and */
			      /* "AD" for default settings in FMAIL.ARD       */
   u16             writeLevel;
   areaNameType    areaName;
   uchar           comment[COMMENT_LEN];
   areaOptionsType options;
   u16             boardNumRA;
   uchar           msgBaseType;
   uchar           msgBasePath[MB_PATH_LEN];
   u16             board;
   uchar           originLine[ORGLINE_LEN];
   u16             address;
   u32             group;
   u16             _alsoSeenBy; /* obsolete: see the 32-bit alsoSeenBy below */
   u16             msgs;
   u16             days;
   u16             daysRcvd;
/* old startposition of export[MAX_FORWARD] */
   u16             readSecRA;
   uchar           flagsRdRA[4];
   uchar           flagsRdNotRA[4];
   u16             writeSecRA;
   uchar           flagsWrRA[4];
   uchar           flagsWrNotRA[4];
   u16             sysopSecRA;
   uchar           flagsSysRA[4];
   uchar           flagsSysNotRA[4];
   u16             templateSecQBBS;
   uchar           flagsTemplateQBBS[4];
   uchar           _internalUse;
   u16             netReplyBoardRA;
   uchar           boardTypeRA;
   uchar           attrRA;
   uchar           attr2RA;
   u16             groupRA;
   u16             altGroupRA[3];
   uchar           msgKindsRA;
   uchar           qwkName[13];
   u16             minAgeSBBS;
   u16             attrSBBS;
   uchar           replyStatSBBS;
   uchar           groupsQBBS;
   uchar           aliasesQBBS;
   u32             lastMsgTossDat;
   u32             lastMsgScanDat;
   u32             alsoSeenBy;
   areaStatType    stat;
   uchar           reserved[180];

   nodeNumXType    export[MAX_FORWARDDEF];

} rawEchoType;



/* ********** FMAIL.NOD ********** */

#if defined __OS2__ || defined __FMAILX__
#define MAX_NODEMGR      1024
#else
#define MAX_NODEMGR      256
#endif

#define PKTOUT_PATH_LEN  53

#define PKT_TYPE_2PLUS   1
#define CAPABILITY       PKT_TYPE_2PLUS

typedef struct
{
   unsigned fixDate     : 1; /* Bit 0 */
   unsigned tinySeenBy  : 1; /* Bit 1 */
   unsigned             : 1; /* Bit 2 */
   unsigned ignorePwd   : 1; /* Bit 3 */
   unsigned active      : 1; /* Bit 4 */
   unsigned             : 1;
   unsigned routeToPoint: 1; /* Bit 6 */
   unsigned packNetmail : 1; /* Bit 7 */
   unsigned             : 1; /* Bit 8 */
   unsigned             : 3;
   unsigned forwardReq  : 1; /* Bit 12 */
   unsigned remMaint    : 1; /* Bit 13 */
   unsigned allowRescan : 1; /* Bit 14 */
   unsigned notify      : 1;   } nodeOptionsType;

typedef struct /* OLD !!! */
{
   nodeNumType     node;
   uchar           reserved1[2];
   u16             capability;
   u16             archiver;
   nodeOptionsType options;
   u32             groups;
   u16             outStatus;
   uchar           reserved2[32];
   uchar           password[18];
   uchar           packetPwd[10];
   uchar           reserved[2];
   nodeNumType     viaNode;
   uchar           sysopName[36];   } nodeInfoTypeOld;

/* NOTE: see above for the file header structure !!! */

typedef struct
{
   u16             signature; /* contains "ND" */
   u16             writeLevel;
   nodeNumType     node;
   nodeNumType     viaNode;
   u16             capability;
   nodeOptionsType options;
   u8              archiver;
   u8              reserved1;
   u32             groups;
   u16             outStatus;
   uchar           sysopName[36];
   uchar           password[18];
   uchar           packetPwd[9];
   uchar           useAka;
   u32             lastMsgRcvdDat;
   u32             lastMsgSentDat;
   u32             lastNewBundleDat;
   uchar           pktOutPath[PKTOUT_PATH_LEN];
   u16             passiveDays;
   u16             passiveSize;
   u32             lastSentBCL;
   u16             autoBCL;
   u32             _lastNewBundleDat;
   uchar           reserved2[17];        } nodeInfoType;



/* ********** FMAIL.PCK ********** */

#define PACK_STR_SIZE 64
#define MAX_PACK      128

typedef uchar packEntryType[PACK_STR_SIZE];
typedef packEntryType packType[MAX_PACK];



/* ********** FMAIL.BDE ********** */

#define MAX_BAD_ECHOS 50

typedef struct
{
   areaNameType badEchoName;
   nodeNumType  srcNode;
   s16          destAka;      } badEchoType;




/*
   FMSTRUCT.H

   File structures for FMail 1.02
   Copyright (C) 1994 Folkert J. Wijnstra. All rights reserved.

   All information in this document is subject to change at any time
   without prior notice!

   Strings are NUL terminated arrays of char type.
   Path names always end on a \ character (followed by NUL).
*/


/**** Modify the type definitions below if necessary for your compiler ****/

#define fhandle int
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
{  nodeNumType nodeNum;
   u16         fakeNet; } nodeFakeType;


/* ********** File header structure ********** */

#define DATATYPE_CF    0x0102
#define DATATYPE_NO    0x0202
#define DATATYPE_AD    0x0401
#define DATATYPE_AE    0x0402

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
   unsigned             : 2; /* Bit 12-13 */
   unsigned arrivalDate : 1; /* Bit 14 */
   unsigned sysopRead   : 1; /* Bit 15 */   } areaOptionsType;


/* ********** FMAIL.CFG ********** */

#define MAX_AKAS      16
#define MAX_NETAKAS   11
#define MAX_USERS     16
#define MAX_UPLREQ    16
#define MAX_MATCH     16

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
#define LOG_EXEC      0x2000
#define LOG_NOSCRN    0x4000
#define LOG_ALWAYS    0x8000
#define LOG_DEBUG     0x8000

typedef nodeFakeType akaListType[MAX_AKAS];

typedef struct
{
   unsigned useEMS       : 1; /* BIT 0 */
   unsigned checkBreak   : 1; /* BIT 1 */
   unsigned swap         : 1; /* BIT 2 */
   unsigned swapEMS      : 1; /* BIT 3 */
   unsigned swapXMS      : 1; /* BIT 4 */
   unsigned              : 1;
   unsigned monochrome   : 1; /* BIT 6 */
   unsigned commentFFD   : 1; /* BIT 7 */
   unsigned PTAreasBBS   : 1; /* BIT 8 */
   unsigned commentFRA   : 1; /* BIT 9 */
   unsigned              : 1; /* BIT 10 */
   unsigned incBDRRA     : 1; /* BIT 11 */
   unsigned              : 1; /* BIT 12 */
   unsigned              : 2;
   unsigned _RA2         : 1; /* BIT 15 */  } genOptionsType;

typedef struct
{
   unsigned removeNetKludges : 1; /* Bit 0 */
   unsigned                  : 1;
   unsigned checkPktDest     : 1; /* Bit 2 */
   unsigned                  : 1;
   unsigned createSema       : 1; /* Bit 4 */
   unsigned                  : 1;
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
   unsigned              : 1; /* bit 13   */
   unsigned              : 1; /* bit 14   */
   unsigned sysopImport  : 1; /* bit 15   */ } mbOptionsType;

typedef struct
{
   unsigned keepRequest  : 1; /* Bit  0 */
   unsigned keepReceipt  : 1; /* Bit  1 */
   unsigned              : 2; /* Bit 2-3 */
   unsigned autoDiscArea : 1; /* Bit  4 */
   unsigned autoDiscDel  : 1; /* Bit  5 has temp. no effect, rec is always deleted */
   unsigned              : 3; /* Bit 6-8 */
   unsigned allowAddAll  : 1; /* Bit  9 */
   unsigned allowActive  : 1; /* Bit 10 */
   unsigned              : 1; /* Bit 11 */
   unsigned allowPassword: 1; /* Bit 12 */
   unsigned allowPktPwd  : 1; /* Bit 13 */
   unsigned allowNotify  : 1; /* Bit 14 */
   unsigned allowCompr   : 1; /* Bit 15 */  } mgrOptionsType;

#if 0
typedef struct
{
   unsigned active      : 1; /* Bit  0 */
   unsigned tinySeenBy  : 1; /* Bit  1 */
   unsigned security    : 1; /* Bit  2 */
   unsigned             : 1; /* Bit  3 */
   unsigned allowPrivate: 1; /* Bit  4 */
   unsigned impSeenBy   : 1; /* Bit  5 */
   unsigned checkSeenBy : 1; /* Bit  6 */
   unsigned             : 1; /* Bit  7 */
   unsigned local       : 1; /* Bit  8 */
   unsigned             : 1; /* Bit  9 */
   unsigned passThrough : 1; /* Bit 10 */
   unsigned             : 3; /* Bit 11-13 */
   unsigned arrivalDate : 1; /* Bit 14 */
   unsigned sysopRead   : 1; /* Bit 15 */     } defaultOptionsType;
#endif
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
   uchar           reserved1[22];
   mgrOptionsType  mgrOptions;
   akaListType     akaList;
/* nodeFakeType    reservedAka[16-MAX_AKAS]; */
   u16             netmailBoard[MAX_NETAKAS];
   u16             reservedNet[16-MAX_NETAKAS];
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
   uchar           reserved3[68];
   u16             colorSet;
   uchar           sysopName[36];
   u16             defaultArc;
   uchar           reserved4[24];
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
   uchar           reserved5[83];
   areaOptionsType optionsAKA[MAX_NETAKAS]; /* 'areaOptionsType' below */
   uchar           groupsQBBS[MAX_NETAKAS];
   u16             templateSecQBBS[MAX_NETAKAS];
   uchar           templateFlagsQBBS[MAX_NETAKAS][4];
   uchar           attr2RA[MAX_NETAKAS];
   uchar           aliasesQBBS[MAX_NETAKAS];
   u16             groupRA[MAX_NETAKAS];
   u16             altGroupRA[MAX_NETAKAS][3];
   uchar           qwkNameSBBS[MAX_NETAKAS][13];
   u16             minAgeSBBS[MAX_NETAKAS];
   u16             daysRcvdAKA[MAX_NETAKAS];
   uchar           replyStatSBBS[MAX_NETAKAS];
   u16             attrSBBS[MAX_NETAKAS];
   uchar           groupDescr[26][27];
   uchar           reserved6[9];
   uchar           msgKindsRA[MAX_NETAKAS];
   uchar           attrRA[MAX_NETAKAS];
   u16             readSecRA[MAX_NETAKAS];
   uchar           readFlagsRA[MAX_NETAKAS][4];
   u16             writeSecRA[MAX_NETAKAS];
   uchar           writeFlagsRA[MAX_NETAKAS][4];
   u16             sysopSecRA[MAX_NETAKAS];
   uchar           sysopFlagsRA[MAX_NETAKAS][4];
   u16             daysAKA[MAX_NETAKAS];
   u16             msgsAKA[MAX_NETAKAS];
   uchar           descrAKA[MAX_NETAKAS][51];
   userType        users[MAX_USERS];
   akaMatchType    akaMatch[MAX_MATCH];
   uchar           reserved7[1040-10*MAX_MATCH];
   pathType        sentEchoPath;
   archiverInfo    preUnarc;
   archiverInfo    postUnarc;
   archiverInfo    preArc;
   archiverInfo    postArc;
   archiverInfo    unUc2;
   archiverInfo    unRar;
   archiverInfo    resUnpack[6];
   archiverInfo    uc2;
   archiverInfo    rar;
   archiverInfo    resPack[6];
   uplinkReqType   uplinkReq[MAX_UPLREQ];   } configType;



/* ********** FMAIL.AR ********** */

#if defined(__FMAILX__) || defined(__OS2__)
#define MAX_AREAS   4096
#else
#define MAX_AREAS    512
#endif
#define MAX_FORWARD   64

#define MB_PATH_LEN_OLD   19
#define MB_PATH_LEN       61
#define ECHONAME_LEN_090  25
#define ECHONAME_LEN      51
#define COMMENT_LEN       51
#define ORGLINE_LEN       59

typedef uchar areaNameType[ECHONAME_LEN];

typedef struct      /* OLD !!! */
{
   uchar           zero; /* Should always be zero */
   uchar           msgBasePath[MB_PATH_LEN_OLD];
   uchar           groupsQBBS;
   uchar           flagsTemplateQBBS[4];
   uchar           comment[COMMENT_LEN];
   u32             group;
   u16             board;
   u16             address;
   u16             alsoSeenBy;
   u16             groupRA;
   u16             altGroupRA[3];
   areaOptionsType options;
   u16             outStatus;
   u16             days;
   u16             msgs;
   u16             daysRcvd;
   u16             templateSecQBBS;
   u16             readSecRA;
   uchar           flagsRdRA[4];
   u16             writeSecRA;
   uchar           flagsWrRA[4];
   u16             sysopSecRA;
   uchar           flagsSysRA[4];
   uchar           attrRA;
   uchar           msgKindsRA;
   u16             attrSBBS;
   uchar           replyStatSBBS;
   areaNameType    areaName;
   uchar           qwkName[13];
   u16             minAgeSBBS;
   uchar           attr2RA;
   uchar           aliasesQBBS;
   uchar           originLine[ORGLINE_LEN];
   nodeNumType     export[MAX_FORWARD];  } rawEchoTypeOld;

/* See Area File for file header structure !!! */

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
   u16             alsoSeenBy;
   u16             msgs;
   u16             days;
   u16             daysRcvd;

   nodeNumType     export[MAX_FORWARD];

   u16             readSecRA;
   uchar           flagsRdNotRA[4];
   uchar           flagsRdRA[4];
   u16             writeSecRA;
   uchar           flagsWrNotRA[4];
   uchar           flagsWrRA[4];
   u16             sysopSecRA;
   uchar           flagsSysRA[4];
   uchar           flagsSysNotRA[4];
   u16             templateSecQBBS;
   uchar           flagsTemplateQBBS[4];
   uchar           reserved2;
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
   uchar           aliasesQBBS;  } rawEchoType;



/* ********** FMAIL.NOD ********** */

#if defined(__FMAILX__) || defined(__OS2__)
#define MAX_NODEMGR      512
#else
#define MAX_NODEMGR      256
#endif

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

/* See Area File for file header structure !!! */

typedef struct
{
   u16             signature; /* contains "ND" */
   u16             writeLevel;
   nodeNumType     node;
   nodeNumType     viaNode;
   u16             capability;
   nodeOptionsType options;
   u16             archiver;
   u32             groups;
   u16             outStatus;
   uchar           sysopName[36];
   uchar           password[18];
   uchar           packetPwd[9];
   uchar           useAka;        } nodeInfoType;



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


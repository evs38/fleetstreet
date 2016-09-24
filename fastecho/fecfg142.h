/********************************************************/
/* 'C' Structures of FastEcho 1.42 beta                 */
/* Copyright (c) 1995 by Tobias Burchhardt              */
/* Last update: 16 Jun 1995                             */
/********************************************************/

/********************************************************/
/* FASTECHO.CFG = <CONFIG>                              */
/*                + <optional extensions>               */
/*                + <CONFIG.NodeCnt * Node>             */
/*                + <CONFIG.AreaCnt * Area>             */
/********************************************************/

#define REVISION        6       /* current revision     */

/* Note: there is a major change in this revision - the
     Node records have no longer a fixed length !       */

#define MAX_AREAS       3072    /* max # of areas       */
#define MAX_NODES       1024    /* max # of nodes       */
#define MAX_GROUPS      26      /* max # of groups      */
#define MAX_AKAS        32      /* max # of akas        */
#define MAX_ROUTE       15      /* max # of 'vias'      */
#define MAX_ORIGINS     20      /* max # of origins     */

/*
  Note: The MAX_AREAS and MAX_NODES are only the absolute maximums
        as the handling is flexible. To get the maximums which are
        used for the config file you read, you have to examine the
        CONFIG.MaxAreas and CONFIG.MaxNodes variables !

  Note: The MAX_AREAS and MAX_NODES maximums are subject to change
        with any new version, therefore - if possible - make hand-
        ling as flexible  possible  and  use  CONFIG.MaxAreas  and
        .MaxNodes whereever possible. But be aware that you  might
        (under normal DOS and depending on the way you handle it)
        hit the 64kB segment limit pretty quickly!

        Same goes for the # of AKAs and Groups - use  the  values
        found in CONFIG.AkaCnt and CONFIG.GroupCnt!
*/

/********************************************************/
/* CONFIG.flags                                         */
/********************************************************/
#define RETEAR                  0x00000001l
#define AUTOCREATE              0x00000002l
#define KILLEMPTY               0x00000004l
#define KILLDUPES               0x00000008l
#define CLEANTEARLINE           0x00001000l
#define IMPORT_INNCLUDEUSERSBBS 0x00002000l
#define KILLSTRAYATTACHES       0x00004000l
#define PURGE_PROCESSDATE       0x00008000l
#define MAILER_RESCAN           0x00010000l
#define EXCLUDE_USERS           0x00020000l
#define EXCLUDE_SYSOPS          0x00040000l
#define CHECK_DESTINATION       0x00080000l
#define UPDATE_BBS_CONFIG       0x00100000l
#define KILL_GRUNGED_DATE       0x00200000l
#define NOT_BUFFER_EMS          0x00400000l
#define KEEP_NETMAILS           0x00800000l
#define NOT_UPDATE_MAILER       0x01000000l
#define NOT_CHECK_SEMAPHORES    0x02000000l
#define CREATE_SEMAPHORES       0x04000000l
#define CHECK_COMPLETE          0x08000000l
#define RESPOND_TO_RRQ          0x10000000l
#define TEMP_OUTB_HARDDISK      0x20000000l
#define FORWARD_PACKETS         0x40000000l
#define UNPACK_UNPROTECTED      0x80000000l

/********************************************************/
/* CONFIG.mailer                                        */
/********************************************************/
#define FrontDoor               0x0001
#define InterMail               0x0002
#define DBridge                 0x0004
#define Binkley                 0x0010
#define PortalOfPower           0x0020
#define McMail                  0x0040

/********************************************************/
/* CONFIG.BBSSoftware                                   */
/********************************************************/
enum BBSSoft { NoBBSSoft = 0, RemoteAccess111, QuickBBS,
               SuperBBS, ProBoard122 /* Unused */, TagBBS,
               RemoteAccess200, ProBoard130 /* Unused */,
               ProBoard200, ProBoard202 };

/********************************************************/
/* CONFIG.CC.what                                       */
/********************************************************/
#define CC_FROM                 1
#define CC_TO                   2
#define CC_SUBJECT              3
#define CC_KLUDGE               4

/********************************************************/
/* CONFIG.QuietLevel                                    */
/********************************************************/
#define QUIET_PACK              0x0001
#define QUIET_UNPACK            0x0002
#define QUIET_EXTERN            0x0004

/********************************************************/
/* CONFIG.Swapping                                      */
/********************************************************/
#define SWAP_TO_XMS             0x0001
#define SWAP_TO_EMS             0x0002
#define SWAP_TO_DISK            0x0004

/********************************************************/
/* CONFIG.Buffers                                       */
/********************************************************/
#define BUF_LARGE               0x0000
#define BUF_MEDIUM              0x0001
#define BUF_SMALL               0x0002

/********************************************************/
/* CONFIG.arcext.inb/outb                               */
/********************************************************/
enum ARCmailExt { ARCDigits = 0, ARCHex, ARCAlpha };

/********************************************************/
/* CONFIG.AreaFixFlags                                  */
/********************************************************/
#define ALLOWRESCAN             0x0001
#define KEEPREQUEST             0x0002
#define KEEPRECEIPT             0x0004
#define ALLOWREMOTE             0x0008
#define DETAILEDLIST            0x0010
#define ALLOWPASSWORD           0x0020
#define ALLOWPKTPWD             0x0040
#define ALLOWCOMPRESS           0x0080
#define SCANBEFORE              0x0100
#define ADDRECEIPTLIST          0x0200
#define NOTIFYPASSWORDS         0x0400

/********************************************************/
/* Area.board (1-200 = Hudson)                          */
/********************************************************/
#define NO_BOARD        0x4000u /* JAM/Sq/Passthru etc. */
#define AREA_DELETED    0x8000u /* usually never written*/

/********************************************************/
/* Area.flags.storage                                   */
/********************************************************/
#define QBBS                    0
#define FIDO                    1
#define SQUISH                  2
#define JAM                     3
#define PASSTHRU                7

/********************************************************/
/* Area.flags.atype                                     */
/********************************************************/
#define AREA_ECHOMAIL           0
#define AREA_NETMAIL            1
#define AREA_LOCAL              2
#define AREA_BADMAILBOARD       3
#define AREA_DUPEBOARD          4

/********************************************************/
/* Types and other definitions                          */
/********************************************************/
#ifdef TEST
typedef unsigned char byte;
typedef unsigned short word;     /* normal int = 16 bit */
typedef unsigned long dword;
#endif

enum ARCers { ARC_Unknown = -1, ARC_SeaArc, ARC_PkArc, ARC_Pak,
	      ARC_ArcPlus, ARC_Zoo, ARC_PkZip, ARC_Lha, ARC_Arj,
	      ARC_Sqz, ARC_RAR, ARC_UC2 }; /* for Unpackers */

enum NetmailStatus { NetNormal = 0, NetHold, NetCrash /*, NetImm */ };

enum AreaFixType { NoAreaFix = 0, NormalAreaFix, FSC57AreaFix };
enum AreaFixSendTo { AreaFix = 0, AreaMgr, AreaLink, EchoMgr };

/********************************************************/
/* Structures                                           */
/********************************************************/

#pragma pack(1)

typedef struct
{
 unsigned short zone,net,node,point;
} Address;

#define _MAXPATH 56

typedef struct CONFIGURATION
{
 unsigned short revision;
 unsigned long flags;
 unsigned short NodeCnt,AreaCnt,unused1;
 char NetMPath[_MAXPATH],
      MsgBase[_MAXPATH],
      InBound[_MAXPATH],
      OutBound[_MAXPATH],
      Unpacker[_MAXPATH],               /* DOS default decompression program */
      LogFile[_MAXPATH],
      unused2[336],
      Unpacker2[_MAXPATH],              /* OS/2 default decompression program */
      UnprotInBound[_MAXPATH],
      StatFile[_MAXPATH],
      SwapPath[_MAXPATH],
      SemaphorePath[_MAXPATH],
      BBSConfigPath[_MAXPATH],
      DBQueuePath[_MAXPATH],
      unused3[32],
      RetearTo[40],
      LocalInBound[_MAXPATH],
      ExtAfter[_MAXPATH-4],
      ExtBefore[_MAXPATH-4];
 unsigned char unused4[480];
 struct
 {
  unsigned char what;
  char object[31];
  unsigned short conference;
 } CC[10];
 unsigned char security,loglevel;
 unsigned short def_days,def_messages;
 unsigned char unused5[462];
 unsigned short autorenum;
 unsigned short def_recvdays;
 unsigned char openQQQs,Swapping;
 unsigned short compressafter;
 unsigned short afixmaxmsglen;
 unsigned short compressfree;
 char TempPath[_MAXPATH];
 unsigned char graphics,BBSSoftware;
 char AreaFixHelp[_MAXPATH];
 unsigned char unused6[504];
 unsigned short AreaFixFlags;
 unsigned char QuietLevel,Buffers;
 unsigned char FWACnt,GDCnt;             /* # of ForwardAreaFix records,
                                   # of Group Default records */
 struct
 {
  unsigned short flags;
  unsigned short days[2];
  unsigned short msgs[2];
 } rescan_def;
 unsigned long duperecords;
 struct
 {
  unsigned char inb;
  unsigned char outb;
 } arcext;
 unsigned short AFixRcptLen;
 unsigned char AkaCnt,resv;              /* # of Aka records stored */
 unsigned short maxPKT;
 unsigned char sharing,sorting;
 struct
 {
  char name[36];
  unsigned long resv;
 } sysops[11];
 char AreaFixLog[_MAXPATH];
 char TempInBound[_MAXPATH];
 unsigned short maxPKTmsgs;
 unsigned short RouteCnt;                 /* # of PackRoute records */
 unsigned short maxPACKratio;
 unsigned char PackerCnt,UnpackerCnt;    /* # of Packers and Unpackers records */
 unsigned char GroupCnt,OriginCnt;       /* # of GroupNames and Origin records */
 unsigned short mailer;
 char reserved[810];
 unsigned short AreaRecSize,GrpDefRecSize;      /* Size  of  Area  and  GroupDefaults
                                         records stored in this file        */
 unsigned short MaxAreas,MaxNodes;              /* Current max values for this config */
 unsigned short NodeRecSize;                    /* Size of each stored Node record    */
 unsigned long offset;                        /* This is the offset from the current
                                         file-pointer to the 1st Node       */
} CONFIG;

/* To directly access the 'Nodes' and/or 'Areas' while bypassing the */
/* Extensions, perform an absolute (from beginning of file) seek to  */
/*                   sizeof(CONFIG) + CONFIG.offset                  */
/* If you want to access the 'Areas', you have to add the following  */
/* value to the above formula:  CONFIG.NodeCnt * CONFIG.NodeRecSize  */

typedef struct
{
 Address addr;          /* Main address                                  */
 Address arcdest;       /* ARCmail fileattach address                    */
 unsigned char aka,autopassive,newgroup,resv1;
 struct
 {
  unsigned int passive          : 1;
  unsigned int dddd             : 1;    /* Type 2+/4D                            */
  unsigned int arcmail060       : 1;
  unsigned int tosscan          : 1;
  unsigned int umlautnet        : 1;
  unsigned int exportbyname     : 1;
  unsigned int allowareacreate  : 1;
  unsigned int disablerescan    : 1;
  unsigned int arc_status       : 2;    /* NetmailStatus for ARCmail attaches    */
  unsigned int arc_direct       : 1;    /* Direct flag for ARCmail attaches      */
  unsigned int noattach         : 1;    /* don't create a ARCmail file attach    */
  unsigned int mgr_status       : 2;    /* NetMailStatus for AreaFix receipts    */
  unsigned int mgr_direct       : 1;    /* Direct flag for ...                   */
  unsigned int not_help         : 1;
  unsigned int not_notify       : 1;
  unsigned int packer           : 4;    /* # of Packer used, 0xf = send .PKT     */
  unsigned int packpriority     : 1;    /* system has priority packing ARCmail   */
  unsigned int resv             : 2;
 } flags;                       /* 24 bits total !                       */
 struct
 {
  unsigned int type             : 2;    /* Type of AreaFix: None (human),
                                 Normal or Advanced (FSC-57)           */
  unsigned int noforward        : 1;    /* Don't forward AFix requests           */
  unsigned int allowremote      : 1;
  unsigned int allowdelete      : 1;    /* flags for different FSC-57 requests   */
  unsigned int allowrename      : 1;    /* all 3 reserved for future use         */
  unsigned int binarylist       : 1;
  unsigned int addplus          : 1;    /* add '+' when requesting new area      */
  unsigned int addtear          : 1;    /* add tearline to the end of requests   */
  unsigned int sendto           : 3;    /* name of this systems's AreaFix robot  */
  unsigned int resv             : 4;
 } afixflags;
 unsigned short resv2;
 char password[9];		/* .PKT password                         */
 char areafixpw[9];     /* AreaFix password                      */
 unsigned short sec_level;
 unsigned long groups;			/* Bit-field, Byte 0/Bit 7 = 'A' etc.    */
                        /* FALSE means group is active           */
 unsigned long resv3;
 unsigned short resv4;
 unsigned short maxarcsize;
 char name[36];         /* Name of sysop                         */
 unsigned char areas[1];			/* Bit-field with CONFIG.MaxAreas / 8
                           bits, Byte 0/Bit 7 is conference #0   */
} Node;                 /* Total size of each record is stored
                           in CONFIG.NodeRecSize                 */

typedef struct
{
 char name[52];
 unsigned short board;			/* 1-200 Hudson, others reserved/special */
 unsigned short conference;       /* 0 ... CONFIG.MaxAreas-1               */
 unsigned short read_sec,write_sec;
 struct
 {
  unsigned int aka    : 8;      /* 0 ... CONFIG.AkaCnt                   */
  unsigned int group  : 8;      /* 0 ... CONFIG.GroupCnt                 */
 } info;
 struct
 {
  unsigned int storage: 4;
  unsigned int atype  : 4;
  unsigned int origin : 5;      /* # of origin line                      */
  unsigned int resv   : 3;
 } flags;
 struct
 {
  unsigned int autoadded  : 1;
  unsigned int tinyseen   : 1;
  unsigned int cpd        : 1;
  unsigned int passive    : 1;
  unsigned int keepseen   : 1;
  unsigned int mandatory  : 1;
  unsigned int keepsysop  : 1;
  unsigned int killread   : 1;
  unsigned int disablepsv : 1;
  unsigned int keepmails  : 1;
  unsigned int hide       : 1;
  unsigned int nomanual   : 1;
  unsigned int umlaut     : 1;
  unsigned int resv       : 3;
 } advflags;
 unsigned short resv1;
 unsigned long seenbys;			/* LSB = Aka0, MSB = Aka31      	 */
 unsigned long resv2;
 short days;
 short messages;
 short recvdays;
 char path[_MAXPATH];
 char desc[52];
} Area;

/********************************************************/
/* Optional Extensions                                  */
/********************************************************/
/* These are the variable length extensions between     */
/* CONFIG and the first Node record. Each extension has */
/* a header which contains the info about the type and  */
/* the length of the extension. You can read the fields */
/* using the following algorithm:                       */
/*                                                      */
/* offset := 0;                                         */
/* while (offset<CONFIG.offset) do                      */
/*  read_header;                                        */
/*  if(header.type==EH_abc) then                        */
/*   read_and_process_data;                             */
/*    else                                              */
/*  if(header.type==EH_xyz) then                        */
/*   read_and_process_data;                             */
/*    else                                              */
/*   [...]                                              */
/*    else  // unknown or unwanted extension found      */
/*  seek_forward(header.offset); // Seek to next header */
/*  offset = offset + header.offset + sizeof(header);   */
/* end;                                                 */
/********************************************************/

typedef struct
{
 unsigned short type;             /* EH_...                           */
 unsigned long offset;          /* length of field excluding header */
} ExtensionHeader;


#define EH_AREAFIX      0x0001 /* CONFIG.FWACnt * <ForwardAreaFix> */

enum AreaFixAreaListFormat { Areas_BBS = 0, Area_List };

typedef struct
{
 unsigned short nodenr;
 struct
 {
  unsigned int newgroup : 8;
  unsigned int active   : 1;
  unsigned int valid    : 1;
  unsigned int uncond   : 1;
  unsigned int format   : 3;
  unsigned int resv     : 2;
 } flags;
 char file[_MAXPATH];
 unsigned short sec_level;
 unsigned short resv1;
 unsigned long groups;
 char resv2[4];
} ForwardAreaFix;

#define EH_GROUPS       0x000C /* CONFIG.GroupCnt * <GroupNames> */

typedef struct
{
 char name[36];
} GroupNames;

#define EH_GRPDEFAULTS  0x0006  /* CONFIG.GDCnt * <GroupDefaults> */
				/* Size of each full GroupDefault
				   record is CONFIG.GrpDefResSize */
typedef struct
{
 unsigned char group;
 unsigned char resv[15];
 Area area;
 unsigned char nodes[1];			/* variable, c.MaxNodes / 8 bytes */
} GroupDefaults;

#define EH_AKAS         0x0007  /* CONFIG.AkaCnt * <SysAddress> */

typedef struct
{
 Address main;
 char domain[28];
 unsigned short pointnet;
 unsigned long flags;           /* unused       */
} SysAddress;

#define EH_ORIGINS      0x0008  /* CONFIG.OriginCnt * <OriginLines> */

typedef struct
{
 char line[62];
} OriginLines;

#define EH_PACKROUTE    0x0009  /* CONFIG.RouteCnt * <PackRoute> */

typedef struct
{
 Address dest;
 Address routes[MAX_ROUTE];
} PackRoute;

#define EH_PACKERS      0x000A  /* CONFIG.Packers * <Packers> (DOS)  */
#define EH_PACKERS2     0x100A  /* CONFIG.Packers * <Packers> (OS/2) */

typedef struct
{
 char tag[6];
 char command[_MAXPATH];
 char list[4];
 unsigned char ratio;
 unsigned char resv[7];
} Packers;

#define EH_UNPACKERS    0x000B  /* CONFIG.Unpackers * <Unpackers> (DOS)  */
#define EH_UNPACKERS2   0x100B  /* CONFIG.Unpackers * <Unpackers> (OS/2) */

/* Calling convention:                                      */
/* 0 = change path to inbound directory, 1 = <path> *.PKT,  */
/* 2 = *.PKT <path>, 3 = *.PKT #<path>, 4 = *.PKT -d <path> */

typedef struct
{
 char command[_MAXPATH];
 unsigned char callingconvention;
 unsigned char resv[7];
} Unpackers;

#define EH_RA111_MSG    0x0100  /* Original records of BBS systems */
#define EH_QBBS_MSG     0x0101
#define EH_SBBS_MSG     0x0102
#define EH_TAG_MSG     	0x0104
#define EH_RA200_MSG    0x0105
#define EH_PB200_MSG    0x0106  /* See BBS package's documentation */
#define EH_PB202_MSG    0x0107  /* for details                     */

/********************************************************/
/* Routines to access Node.areas, Node.groups           */
/********************************************************/

#if 0

/********************************************************/
/* FASTECHO.DAT = <STATHEADER>                          */
/*                + <STATHEADER.NodeCnt * StatNode>     */
/*                + <STATHEADER.AreaCnt * StatArea>     */
/********************************************************/

#define STAT_REVISION       3   /* current revision     */

#ifdef TEST
struct date                     /* Used in FASTECHO.DAT */
{
 unsigned short year;
 unsigned char day;
 unsigned char month;
};
#endif

typedef struct
{
 char signature[10];            /* contains 'FASTECHO\0^Z'      */
 unsigned short revision;
 struct date lastupdate;        /* last time file was updated   */
 unsigned short NodeCnt,AreaCnt;
 unsigned long startnode,startarea;     /* unix timestamp of last reset */
 unsigned short NodeSize,AreaSize;        /* size of StatNode and StatArea records */
 char resv[32];
} STATHEADER;

typedef struct
{
 Address adr;
 unsigned long import,export;
 struct date lastimport,lastexport;
 unsigned long dupes;
 unsigned long importbytes,exprotbytes;
} StatNode;

typedef struct
{
 unsigned short conference;               /* conference # of area */
 unsigned long tagcrc;                  /* CRC32 of area tag    */
 unsigned long import,export;
 struct date lastimport,lastexport;
 unsigned long dupes;
} StatArea;
#endif

#pragma pack()

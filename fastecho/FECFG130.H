/********************************************************/
/* 'C' Structures of FastEcho 1.30, File: FASTECHO.CFG  */
/* (c)1993 by Tobias Burchhardt, Updated: 13 Sep 1993   */
/********************************************************/

/********************************************************/
/* FASTECHO.CFG = <CONFIG>                              */
/*                + <optional extensions>               */
/*                + <CONFIG.NodeCnt * Node>             */
/*                + <CONFIG.AreaCnt * Area>             */
/********************************************************/

#if defined(__IBMC__) || defined(__IBMCPP__)
#pragma pack(1)
#endif

#define REVISION        4       /* current revision     */

#define MAX_AREAS       960     /* max # of areas       */
#define MAX_NODES       200     /* max # of nodes       */
#define MAX_GROUPS      26      /* max # of groups      */
#define MAX_AKAS        16      /* max # of akas        */

/********************************************************/
/* CONFIG.flags                                         */
/********************************************************/
#define RETEAR                  0x00000001l
#define AUTOCREATE		0x00000002l
#define KILLEMPTY		0x00000004l
#define KILLDUPES		0x00000008l
#define FRONTDOOR		0x00000010l
#define DBRIDGE         	0x00000020l
#define BINKLEY         	0x00000040l
#define INTERMAIL		0x00000080l
#define SWAP_XMS		0x00000100l
#define SWAP_EMS		0x00000200l
#define SWAP_DISK		0x00000400l
#define PURGE_PROCESSDATE       0x00008000l
#define MAILER_RESCAN           0x00010000l
#define EXCLUDE_USERS		0x00020000l
#define EXCLUDE_SYSOPS		0x00040000l
#define CHECK_DESTINATION	0x00080000l
#define UPDATE_BBS_CONFIG       0x00100000l
#define KILL_GRUNGED_DATE       0x00200000l
#define NOT_BUFFER_EMS          0x00400000l
#define KEEP_NETMAILS		0x00800000l
#define NOT_UPDATE_MAILER       0x01000000l
#define NOT_CHECK_SEMAPHORES    0x02000000l
#define CREATE_SEMAPHORES       0x04000000l
#define CHECK_COMPLETE          0x08000000l

/********************************************************/
/* CONFIG.BBSSoftware                                   */
/********************************************************/
enum BBSSoft { NoBBSSoft = 0, RemoteAccess111, QuickBBS,
               SuperBBS, ProBoard122, TagBBS, RemoteAccess200,
               ProBoard130};

/********************************************************/
/* CONFIG.CC.what                                       */
/********************************************************/
#define FROM            1
#define TO		2
#define SUBJECT		3

/********************************************************/
/* CONFIG.QuietLevel                                    */
/********************************************************/
#define QUIET_PACK      0x0001
#define QUIET_UNPACK	0x0002
#define QUIET_EXTERN    0x0004

/********************************************************/
/* CONFIG.Buffers                                       */
/********************************************************/
#define BUF_LARGE       0x0000
#define BUF_MEDIUM      0x0001
#define BUF_SMALL       0x0002

/********************************************************/
/* CONFIG.arcext.inb/outb                               */
/********************************************************/
enum ARCmailExt { ARCDigits = 0, ARCHex, ARCAlpha };

/********************************************************/
/* CONFIG.AreaFixFlags                                  */
/********************************************************/
#define ALLOWRESCAN	0x0001
#define KEEPREQUEST	0x0002
#define KEEPRECEIPT	0x0004
#define ALLOWREMOTE	0x0008
#define DETAILEDLIST	0x0010
#define ALLOWPASSWORD	0x0020
#define ALLOWPKTPWD  	0x0040

/********************************************************/
/* Area.board (1-200 = QBBS)                            */
/********************************************************/
#define NO_BOARD        0x4000u /* everything but QBBS  */
#define AREA_DELETED    0x8000u /* never written        */

/********************************************************/
/* Area.flags.type                                      */
/********************************************************/
#define QBBS            0
#define FIDO		1
#define SQUISH		2
#define JAM             3
#define PT_BOARD	7

/********************************************************/
/* Area.type                                            */
/********************************************************/
#define AREA_ECHOMAIL        0
#define AREA_NETMAIL         1
#define AREA_LOCAL           2
#define AREA_BADMAILBOARD    3
#define AREA_DUPEBOARD	     4

/********************************************************/
/* Node.flags                                           */
/********************************************************/
#define DDDD            0x0001
#define TOSSCAN		0x0002
#define DIRECT		0x0004
#define	CRASH		0x0008
#define UMLAUT_NET	0x0010
#define ALLOW_AF_REMOTE 0x0020
#define RESCANDISABLED  0x0040
#define	HOLD 		0x0080
#define ARCMAIL060	0x0200
#define PASSIVE		0x0400
#define ALLOWAREACREATE 0x0800
#define PACKER          0xF000

#define PACKS(x) (int)(((x)&PACKER)>>12)        /* Index of used Packer   */
						/* 0xf = .PKT, don't pack */

/********************************************************/
/* Node.advflags                                        */
/********************************************************/
#define EXPORTBYNAME    0x0001
#define NOT_NOTIFY      0x0002
#define NOT_HELP        0x0004
#define NOATTACH        0x0008
#define NET_HOLD	0x0010
#define NET_CRASH       0x0020
#define NET_DIRECT      0x0040

/********************************************************/
/* Types                                                */
/********************************************************/
#ifdef __IBMC__
#define word unsigned short
#endif

enum ARCers { ARC_Unknown = -1, ARC_SeaArc, ARC_PkArc, ARC_Pak,
              ARC_ArcPlus, ARC_Zoo, ARC_PkZip, ARC_Lha, ARC_Arj,
              ARC_Sqz };        /* CONFIG.Unpackers[]   */

/********************************************************/
/* Structures                                           */
/********************************************************/

typedef struct
{
 word zone,net,node,point;
} Address;

#define _MAXPATH 56

typedef struct CONFIGURATION
{
 word revision;
 unsigned long flags;
 word NodeCnt,AreaCnt,unused1;
 char NetMPath[_MAXPATH],
      MsgBase[_MAXPATH],
      InBound[_MAXPATH],
      OutBound[_MAXPATH],
      Unpacker[_MAXPATH],
      LogFile[_MAXPATH],
      OriginLine[8][_MAXPATH],
      StatFile[_MAXPATH],
      SwapPath[_MAXPATH],
      SemaphorePath[_MAXPATH],
      BBSConfigPath[_MAXPATH],
      DBQueuePath[_MAXPATH],
      unused2[32],
      RetearTo[40],
      SecurePath[_MAXPATH],
      ExtAfter[_MAXPATH-4],
      ExtBefore[_MAXPATH-4];
 struct
 {
  char tag[4];
  char name[_MAXPATH-2];
  char list[2];				/* List prefix character */
 } Packer[8];
 struct
 {
  unsigned char what;
  char object[31];
  word conference;
 } CC[10];
 unsigned char security,loglevel;
 short def_days,def_messages;
 struct                                 /* now obsolete         */
 {
  Address main;
  char domain[28];
  word pointnet;
  unsigned long flags;
 } oldakas[11];                         /* but still maintained */
 word autorenum;
 short def_recvdays;
 word openQQQs;
 word oldduperecords;                   /* now obsolete         */
 word msglen;
 word unused3;
 char TempPath[_MAXPATH];
 unsigned char graphics,BBSSoftware;
 char AreaFixHelp[_MAXPATH];
 char Unpackers[9][_MAXPATH];
 word AreaFixFlags;
 unsigned char QuietLevel,Buffers;
 unsigned char FWACnt,GDCnt;     /* # of ForwardAreaFix records,
                           # of Group Default records   */
 struct
 {
  word flags;
  word days[2];
  word msgs[2];
 } rescan_def;
 unsigned long duperecords;
 struct
 {
  unsigned char inb;
  unsigned char outb;
 } arcext;
 word AFixRcptLen;
 word AkaCnt;
 word maxPKT;
 unsigned char sharing,sorting;
 struct
 {
  char name[36];
  unsigned long resv;
 } sysops[11];
 char AreaFixLog[_MAXPATH];
 char TempInBound[_MAXPATH];
 char resv2[832];
 unsigned long offset;                        /* This is the offset from the current
					 file-pointer to the 1st Node        */
} CONFIG;

/* To directly access the 'Nodes' and/or 'Areas' while bypassing the */
/* Extensions, perform an absolute (from beginning of file) seek to  */
/*                   sizeof(CONFIG) + CONFIG.offset                  */

typedef struct
{
 Address addr;
 unsigned char aka;                      /* 0 ... MAX_AKAS-1                      */
 unsigned char autopassive;              /* # of days                             */
 word flags;
 word sec_level;
 char password[9];              /* .PKT password                         */
 char newgroup;                 /* Default group for new areas           */
 Address routes[15];            /* netmail routing                       */
 unsigned char areas[124];               /* Bit-field with 992 bits, Byte 0/Bit 7 */
				/* is conference 0, etc.                 */
 unsigned long groups;                  /* Bit-field, Byte 0/Bit 7 = 'A' etc.    */
				/* FALSE means group is active           */
 char areafixpw[9];
 unsigned char advflags;
} Node;

typedef struct
{
 char name[41];
 unsigned char type;
 word board;
 short messages;
#undef word
#define word int
 struct
 {
  word origin : 3;
  word group  : 5;              /* 0 ... MAX_GROUPS-1   */
  word type   : 3;
  word umlaut : 1;
  word aka    : 4;              /* 0 ... MAX_AKAS-1     */
 } flags;
#undef word
#define word unsigned short
 short days;
 word conference;               /* 0 ... 991            */
 word read_sec,write_sec;
#undef word
#define word int
 struct
 {
  word autoadded  : 1;
  word tinyseen   : 1;
  word cpd        : 1;
  word passive    : 1;
  word keepseen   : 1;
  word mandatory  : 1;
  word keepsysop  : 1;
  word killread   : 1;
  word disablepsv : 1;
  word resv	  : 7;
 } advflags;
#undef word
#define word unsigned short
 char path[_MAXPATH];
 char desc[42];
 word seenbys;                  /* LSB = Aka0, MSB = Aka15      */
 short recvdays;
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
/*  offset = offset + header.offset;                    */
/* end;                                                 */
/********************************************************/

typedef struct
{
 word type;             /* EH_...                           */
 unsigned long offset;          /* length of field excluding header */
} ExtensionHeader;


#define EH_AREAFIX      0x0001 /* CONFIG.FWACnt * <ForwardAreaFix> */

enum AreaFixSendTo { AreaFix = 0, AreaMgr, AreaLink, EchoMgr };
enum AreaFixAreaListFormat { Areas_BBS = 0, Area_List };

typedef struct
{
 word nodenr;
#undef word
#define word int
 struct
 {
  word sendto  : 3;
  word newgroup: 5;
  word valid   : 1;
  word uncond  : 1;
  word addplus : 1;
  word addtear : 1;
  word format  : 3;
  word active  : 1;
 } flags;
#undef word
#define word unsigned short
 char file[_MAXPATH];
 unsigned long groups;
 word sec_level;
 char resv[6];
} ForwardAreaFix;

#define EH_GROUPS       0x0002  /* 1 record of <GroupNames>     */

typedef struct
{
 char name[26][35];
} GroupNames;

#define EH_SYSOPNAMES   0x0003  /* CONFIG.NodeCnt * <SysopNames> */

typedef struct
{
 char name[36];
} SysopNames;

#define EH_GRPDEFAULTS  0x0006  /* CONFIG.GDCnt * <GroupDefaults> */

typedef struct
{
 unsigned char group;
 Area area;
 unsigned char nodes[32];        /* 256 bits     */
} GroupDefaults;

#define EH_AKAS         0x0007  /* CONFIG.AkaCnt * <SysAddress> */

typedef struct
{
 Address main;
 char domain[28];
 word pointnet;
 unsigned long flags;           /* unused       */
} SysAddress;

#define EH_RA111_MSG    0x0100  /* Original records of BBS systems */
#define EH_QBBS_MSG     0x0101
#define EH_SBBS_MSG     0x0102
#define EH_PB122_MSG    0x0103
#define EH_TAG_MSG     	0x0104
#define EH_RA200_MSG    0x0105  /* See BBS package's documentation */
#define EH_PB130_MSG    0x0106  /* for details                     */

#if defined(__IBMC__) || defined(__IBMCPP__)
#pragma pack()
#endif


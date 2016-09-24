/***** Configuration file structures ***********************************/

/* Note: All char arrays contain Pascal-type strings, i.e. the first
         byte contains the length of the string, and the string
         contents begin at the second byte.

   This header file is ANSI compatible and can be used with both
   16 bit and 32 bit compilers.

   Make sure that the compiler packs structures at byte boundaries.
   This header file is prepared for IBM's CSet++/Visual Age C++ in
   this respect.

   char's are assumed to be unsigned.

   Converted from Pascal version by Michael Hohner 11.10.1995
*/

/* Version numbers */

#define WMAIL_VER20    4
#define WMAIL_VER21    5
#define WMAIL_VER22    9
#define WMAIL_VER23   10
#define WMAIL_VER24   11
#define WMAIL_VER30   12

/* Swap options */

#define SWAP_NONE   0
#define SWAP_DISK   1
#define SWAP_EMS    2
#define SWAP_XMS    3

/* Log levels */

#define LOGLEVEL_I       0  /* Important messages */
#define LOGLEVEL_I_N     1  /* Important+normal messages */
#define LOGLEVEL_I_N_D   2  /* Important+normal+detailed messages */
#define LOGLEVEL_DEBUG   3  /* Debug mode (all sort of messages) */

/* Log styles */

#define LOGSTYLE_BINKLEY   0
#define LOGSTYLE_FD        1

#ifdef __IBMC__
   #pragma pack(1) /* pack at byte boundaries */
#endif

struct FidoAka
{
   unsigned short Zone, Net, Node, Point;
   char Domain[21];
};

struct Packer
{
   char Name[11];
   char PackCmd[41];
   char UnPackCmd[41];
   char Offset;
   char IdString[7];
};

/* File WMAIL.PRM */

struct SetRec
{
   unsigned short Version;     /* WMAIL_VER* */
   char  Sysop[31];
   char  Key[17];

   struct FidoAka Aka[11];     /* Aka 0 is primary address */
   unsigned short PointNet;

   unsigned CreateNew1 : 1;       /* Create new areas */
   unsigned NewAreasBase : 3;    /* like in AreasRecord.General */

   char Filler1[39];

   unsigned ImportDupes : 1;
   unsigned ImportBad : 1;
   unsigned DoStatistics : 1;
   unsigned UseLog : 1;
   unsigned UseNodelist : 1;
   unsigned DistNetmail : 1;
   unsigned RemoveRe : 1;
   unsigned BinkleyMode : 1;

   unsigned char Swap;             /* SWAP_* */

   unsigned short MinSpace;
   char Filler2[61];
   char FilesDir[61];
   char OutDir[61];
   char ArcMailDir[61];
   char QuickBBSDir[61];

   char Filler3[183];

   char StatsDir[61];
   char NewAreasDir[61];
   char NodelistDir[61];

   char Origin[66][10];

   char AreaListFile[61];
   char LogFile[61];
   char LogLevel;               /* LOGLEVEL_* */
   char LogStyle;               /* LOGSTYLE_* */

   unsigned GoldBase : 1;
   unsigned CreateNew : 1;
   unsigned : 2;
   unsigned CheckDupes : 1;
   unsigned LinkTearline : 1;
   unsigned HonorRRQs : 1;
   unsigned KillPacked : 1;

   char C_WindowBorder,         /* Video attributes */
        C_WindowTitle,
        C_WindowNorm,
        C_WindowHi,
        C_SelBar,
        C_ShowField,
        C_EditField,
        C_MenuDeny,
        C_MessageNorm,
        C_Griglia,
        C_TopBar,
        C_BottomBar,
        C_InfoBar,
        C_WMailText;

   char SquishDir[61];
   char JamDir[61];

   struct Packer Packers[12];

   char Filler4[1402];

   char RouteFile[61];

   char Filler5[476];
   char DefPacker;        /* Default packer number (1-12) */

   unsigned ChangeOrigin : 1;
   unsigned ChangeTearline : 1;
   unsigned HideSeenBy : 1;
   unsigned Strip4d : 1;
   unsigned AddAkas : 1;
   unsigned PutDomain : 1;
   unsigned ForcePublic : 1;
   unsigned ForcePrivate : 1;

   char Rescan_File[61];
   char TrackFile[61];

   char Filler6[109];

   char Def_Group;
   char Def_Aka;
   unsigned short Def_KeepDays,
                  Def_KeepRecv,
                  Def_KeepMsgs,
                  Def_WMKFlags;

   char Def_Origin,
        Def_General,
        Af_DefLevel;

   unsigned KillProcessed : 1;
   unsigned SortNodes : 1;
   unsigned WriteNodesShort : 1;
   unsigned AllowRescan : 1;
   unsigned : 3;
   unsigned Notify : 1;

   unsigned PointInfos4d : 1;
   unsigned : 7;

   unsigned RRRSetCrash : 1;
   unsigned RRRSetHold : 1;
   unsigned RRRSetKillSent : 1;
   unsigned : 5;

   unsigned AFSetCrash : 1;
   unsigned AFSetHold : 1;
   unsigned AFSetKillSent : 1;
   unsigned : 5;

   unsigned MsgTracking : 1;
   unsigned : 7;

   unsigned MsgRemapping : 1;
   unsigned : 7;

   char Filler7[8];

   unsigned short WMKHudsonFlags;  /* These flags apply to Hudson msg.
                                      base; see AreasRecord.WMKFlags */
   unsigned short WMKFlags;        /* Unused at this time */

   char Unused[738];
};


#define AREAKIND_LOCAL       1
#define AREAKIND_ECHO        2
#define AREAKIND_NET         3
#define AREAKIND_BAD         4
#define AREAKIND_DUPES       5
#define AREAKIND_VIRTUAL_NET 6
#define AREAKIND_PERSONAL    7

#define FORMAT_FIDO          0
#define FORMAT_QUICKBBS      1
#define FORMAT_SQUISH        2
#define FORMAT_PASSTHRU      3
#define FORMAT_JAM           4
#define FORMAT_GOLDBASE      6

/* File AREAS.PRM */

struct AreasRecord
{
   char Title[41];
   char Path[81];
   char Kind;           /* AREAKIND_* */
   char WEdOrigin;      /* Used by WEdit */
   char Tag[41];

   unsigned AttribPvt : 1;
   unsigned AttribCrash : 1;
   unsigned : 2;
   unsigned AttribFA : 1;
   unsigned : 2;
   unsigned AttribKS : 1;

   char ForwardTo[61][3];

   char Aka;
   char Misc;  /* Bits: see SetRec.Def_Misc */
   char Level;
   char WMOrigin;
   char WEd_Aka;  /* Used by WEdit */

   unsigned Inactive : 1;
   unsigned Format : 3;      /* FORMAT_* */
   char  Misc2;  /* Bits: see SetRec.Def_Misc2 */
   char  Fill;
   unsigned short IsExtRec;  /* If 0xf00f record is EXTENDED */
   unsigned short RecSize;  /* Size of the whole record, consider
                                      only if record is extended */
   unsigned short Version;  /* 1 for WMail 3.0 */
   unsigned short KeepDays,
                  KeepRecv,
                  KeepMsgs;

   unsigned WMKNoManage : 1;
   unsigned WMKNoPurge : 1;
   unsigned WMKNoRelink : 1;
   unsigned WMKNoPack : 1;
   unsigned WMKNoSort : 1;
   unsigned : 11;

   char Group;  /* Area group identifier (@,A-Z) */

   char Reserved[501];
};

   /* Note: To preserve compatibility with all previous and (maybe) future
           version of WMail, just read first record of file till 'RecSize'
           and then, if record is extended, consider size of all records
           in file equal to value in 'RecSize'.
           For editors, please set as Echomail any area with a 'Kind' flag
           other than valid ones. */

/* File NODES.PRM */

struct NodesRecord
{
   unsigned short Version;       /* 1 for WMail 3.0 */
   short Zona,
         Net,
         Nodo,
         Point;
   char Packer;        /* Like in SetRec.Def_Packer */
   char PktInPw[9];
   char PktOutPw[9];

   char Filler1[3];

   char AfPw[21];
   char Level;      /* The level assigned to the node for areafix operations */

   unsigned DailyPackets : 1;
   unsigned AlertNode : 1;
   unsigned LinkNode : 1;
   unsigned CreatingNode : 1;
   unsigned : 12;

   char Aka;

   char RemapName1[33];
   char RemapName2[33];

   unsigned long Groups;   /* Bit mapped: (bit on=group on
                                           0: group '@'
                                           1: group 'A'
                                          ..
                                          26: group 'Z' */
   char  Filler2[91];
};


/***** Statistics files structures *************************************/

/* File WMSTAT1.DAT */

struct Stat1
{
   char TagArea[31];
   struct
   {
      unsigned short ImpMsg;
      unsigned short ExpMsg;
   } F[3];
};

/* TagArea contains the name of the area to which statistics refer.
  Record F contains number of imported (ImpMsg) and exported (ExpMsg) messages.
  F[1] contains yearly informations, F[2] monthly ones and F[3] weekly ones.
  From first record it is possible to obtain informations about the period
  for which statistics refer.
  F[1].ImpMsg contains a 4 digit number indicating the concerned year
  F[2].ImpMsg contains a number in {1,2,...12} indicating the concerned month
  F[3].ImpMsg contains number of days between 1970 and concerned period.
  Weekly statistics are made after first Monday import. */


/* File WMSTAT2.DAT */

struct Stat2
{
   short Net, Node, Point;

   struct
   {
      short NumPktImp;
      long  NumMsgImp, NumBytesImp;
      short NumPktExp;
      long  NumMsgExp, NumBytesExp;
   } F[3];
};

/* Net,Node,Point contain the address of node to which statistics refer.
  Record F contains informations about number of imported packets (NumPktImp),
  number of imported messages (NumMsgImp) and total dimension in bytes of
  imported packets (NumBytesImp). The same informations are also present in
  export (NumPktExp, NumMsgExp,NumBytesExp). F[1] contains yearly informations
  F[2] the monthly ones and F[3] the weekly ones.
  First record contains reserved informations and must be therefore skipped. */


/***** W-Nodelist files structures ***************************************

These four files are created by WNode:

*FileDat.WNL contains pathnames and dates of all nodelist source files */

struct FileDatRec
{
   char Name[61];    /* @@ :PathStr; */
   long Date;
};

/* *NodeLoc.WNL contains the position of concerned field, and the number of
             the nodelist containing informations about node */

struct NodeLocRec
{
   unsigned NodeZC : 1;
   unsigned NodeRC : 1;
   unsigned NodeHost : 1;
   unsigned NodeHub : 1;
   unsigned NodePvt : 1;
   unsigned NodeHold : 1;
   unsigned NodeDown : 1;
   unsigned NodeBoss : 1;

   short Zone,
         Net,
         Node,
         Point;

   char  FileNum;    /* 1: Primary nodelist
                        2..20: Alternative nodelists as in file FileDat.WNL */
   long  FilePos; /* Contains absolute node position in nodelist file */
};


/* *SysList.WNL contains an alphabetically sorted list of all Sysops and the
             location of relative node in file NodeLoc.WNL. */

struct SysopRec
{
   char Name[21];
   long BBSRecord; /* Position of node record in NodeLoc.WNL */
};

/* *NodeIdx.WNL contains indexes to obtain a quick search inside NodeLoc.WNL and
             SysList.WNL. First nodes for each zone and net, and first sysop
             matching a 4 letter field are in this file. */

struct NodeIdx
{
   char NodeType;     /* For mapping, see NodeLocRec.NodeType.
                                   Nodes reported in this field are only of
                                   the following types:
                                   Zone coordinator
                                   Region coordinator
                                   Host
                                   Boss */
    short Zone,
          Net,
          Node;

    long BBSRecord; /* Position of node record in NodeLoc.WNL */
    char Match[4];  /* The first 4 character of some of the sorted sysop names.
                       One out of about 30 names is taken. */
    long SysopRecord; /* Position of sysop record in SysList.WNL */
};

#ifdef __IBMC__
   #pragma pack()  /* restore default packing */
#endif

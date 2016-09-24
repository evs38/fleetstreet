/* *
 * *
 * *   I M A I L   D E F I N I T I O N S   A N D   S T R U C T U R E S
 * *
 * */

/*
 *
 * Copyright (C) 1992-1995 by Andreas Klein. All rights reserved.
 *
 * The information in this file may not be passed on to others
 * without prior permission from the author.
 *
 * The contents of this file are subject to change without notice!
 * Fields marked reserved should NOT be used.
 *
 * It is an ERROR to write to any of the configuration files
 * while IMAIL (or any of its companion programs) is running.
 *
 *
 * Internal change: struct expt, does only affect IMAIL sources itself
 *
 */

#include <direct.h>
#include <io.h>
#include <time.h>

#pragma pack(1)

#define IMAIL_MAJ_VERSION     1
#define IMAIL_MIN_VERSION     70
#define IMAIL_MAJ_VERSION_STR "1"
#define IMAIL_MIN_VERSION_STR "70"
#define STRUCT_MAJ_VERSION    5
#define STRUCT_MIN_VERSION    9
#define IM_PRD_CODE           0x4B

/*
 *
 *  Data type definitions
 *
 */

typedef unsigned char        byte;
typedef unsigned short       word;
typedef unsigned char        boolean;
typedef short                integer;
typedef unsigned             bit;

/*
 *
 *  Internal limits
 *
 */

#define MAXAKAS              50       /* Max # of addresses */
#define MAXPACKERS           11       /* Max # of packer def */
#define MAXEXPORT           200       /* Max export defs */
#define MAXVIA               40       /* max nodes packed via */
#define MAXGROUPS           255       /* max nodes packed via */
#define MAXEXCEPT            10       /* max EXCEPT nodes */
#define MAXPACK              32       /* max default pack cmd */
#define MAXFWDLINK           15       /* max fwd link structs */
#define MAXNOIMPT            20       /* max # names for IMPORT */
#define MAXSYSNAME           20       /* max # names for PERSMAIL */
#define ZONESDOM             10       /* zones per domain entry */
#define MAXTAG               51       /* max areatag length */
#define MAXNAME              37       /* max namefield lenght */
#define MAXPACKNAME          50       /* max packer length */
#define MAXORIGIN            64       /* max origin length */

/*
 *
 *  Log Style Definitions
 *
 */

#define LOG_FATAL            0x0001                   /* Fatal errors */
#define LOG_OTHER            0x0002                   /* Other errors */
#define LOG_ACCOUNT          0x0004                   /* Accounting info */
#define LOG_FILES            0x0008                   /* Sent/Rcvd files */
#define LOG_BRIEF            0x0010                   /* Brief messages */
#define LOG_TRIVIAL          0x0020                   /* Trivial messages */
#define LOG_TRANSACT         0x0040                   /* Transaction info */
#define LOG_PASSWORD         0x0080                   /* Unexpected passwords */
#define LOG_SECDUPE          0x0100                   /* Security/Dupe info */
#define LOG_SPAWN            0x0200                   /* Spawning info */
#define LOG_DEBUG            0x8000                   /* DEBUG: All of the above */

/*
 *
 *  Mailer Environment Type
 *  (FrontDoor, Binkley, InterMail or Portal of Power)
 *
 */

enum {ENV_FRODO, ENV_BINK, ENV_IM, ENV_DB, ENV_POP};

/*
 *
 *  BBS Environment Type (for the USERS.BBS format)
 *
 */

enum {BBS_RA2,                        /* RA 2.0x/PB 2.0x */
      BBS_OTHER};                     /* Other */

/*
 *
 *  File Attach Message Status (used in IMAIL.ND)
 *
 */

enum {S_NORMAL, S_HOLD, S_CRASH, S_IMMEDIATE, S_DIRECT,
      S_HOLD_DIR, S_CRASH_DIR, S_IMMEDIATE_DIR};

/*
 *
 *  Message Base Types
 *
 */

#ifndef MSGTYPE_SDM
  #define MSGTYPE_SDM     0x01
#endif
#ifndef MSGTYPE_SQUISH
  #define MSGTYPE_SQUISH  0x02
#endif
#ifndef MSGTYPE_HUDSON
  #define MSGTYPE_HUDSON  0x03
#endif
#ifndef MSGTYPE_JAM
  #define MSGTYPE_JAM     0x04
#endif
#ifndef MSGTYPE_PASSTH
  #define MSGTYPE_PASSTH  0x0F
#endif
#ifndef MSGTYPE_ECHO
  #define MSGTYPE_ECHO    0x80
#endif
#ifndef MSGTYPE_LOCAL
  #define MSGTYPE_LOCAL   0x90
#endif
#ifndef MSGTYPE_NET
  #define MSGTYPE_NET     0xA0
#endif

/*
 *
 *  Via Line Switches
 *
 */

enum {VIA_NONE=1, VIA_EXP, VIA_IMP, VIA_BOTH};

/*
 *
 *  Dupe ring check switches
 *
 */

enum {DUPE_NONE, DUPE_COPY, DUPE_KILL, DUPE_KILLCOPY};

/*
 *
 *  Circular path detection switches
 *
 */

enum {CPD_NONE, CPD_COPY, CPD_KILL, CPD_KILLCOPY};

/*
 *
 *  Unlink handling
 *
 */

enum {ULNK_NONE, ULNK_PASSTH, ULNK_ALL};

/*
 *
 *  Personal Mail handling
 *
 */

enum {PERSM_NONE, PERSM_LOG, PERSM_MSG, PERSM_COPY};

/*
 *
 *  AreaLink Request Handling
 *
 */

enum {KEEP_NONE, KEEP_MSG, KEEP_ALL};

/*
 *
 *  PKTs not for us Handling
 *
 */

enum {PKT_TOSS, PKT_FORWARD, PKT_RENAME};

/*
 *
 *  Kill Dead selection
 *
 */

enum {KILL_NONE, KILL_FWD, KILL_ALL};

/*
 *
 *  Macros to make life easier
 *
 */

#define BASEMASK          0x0F
#define TYPEMASK          0xF0

#define IsSdm(Type)          ((Type & BASEMASK) == MSGTYPE_SDM)
#define IsMsg(Type)          ((Type & BASEMASK) == MSGTYPE_SDM)
#define IsSquish(Type)       ((Type & BASEMASK) == MSGTYPE_SQUISH)
#define IsHudson(Type)       ((Type & BASEMASK) == MSGTYPE_HUDSON)
#define IsJam(Type)          ((Type & BASEMASK) == MSGTYPE_JAM)
#define IsPassth(Type)       ((Type & BASEMASK) == MSGTYPE_PASSTH)
#define IsEcho(Type)         ((Type & TYPEMASK) == MSGTYPE_ECHO)
#define IsLocal(Type)        ((Type & TYPEMASK) == MSGTYPE_LOCAL)
#define IsNet(Type)          ((Type & TYPEMASK) == MSGTYPE_NET)

#define grp_in_set(grp,set) ((set[(byte)(((byte)grp)/32)])&(long)(1L<<(((byte)grp)%32)))
#define set_grp(grp,set)    set[(byte)(((byte)grp)/32)] |= (long)(1L<<(((byte)grp)%32))
#define del_grp(grp,set)    set[(byte)(((byte)grp)/32)] &= ~(long)(1L<<(((byte)grp)%32))

/*
 *
 *  Exit Errorlevels & Error Messages
 *
 */

#define E_NOERR              0            /* no error */
#define E_NOPKTS             236          /* */
#define E_GOPEN              237          /* Error opening IMAIL.GI */
#define E_CRDIR              238          /* Error creating directory */
#define E_ELOCK              239          /* File locking error */
#define E_AOPEN              240          /* Error opening IMAIL.AI */
#define E_BOPEN              241          /* Error opening IMAIL.BI */
#define E_NOIDX              242          /* Index file missing/corrupt */
#define E_NOCFG              243          /* IMAIL.CF not found */
#define E_NOARE              244          /* IMAIL.AR not found */
#define E_NONOD              245          /* IMAIL.ND not found */
#define E_BADCF              246          /* Error in IMAIL.CF */
#define E_BADVR              247          /* Bad version of IMAIL.CF */
#define E_EOPEN              248          /* Error opening file */
#define E_EREAD              249          /* Error reading file */
#define E_EWRIT              250          /* Error writing file */
#define E_CMDPR              251          /* Command Line Parameter error */
#define E_FILNF              252          /* File not found */
#define E_MEMRY              253          /* Memory Allocation error */
#define E_DISKF              254          /* Insufficient disk space */
#define E_UNKWN              255          /* Unknown error */

/*
 *
 * Internal status flags for TOSS and SCAN
 *
 */

#define T_NOMAIL             0x00000000L    /* no mail processed */
#define T_NET                0x00010000L    /* Net mail */
#define T_ECHO               0x00020000L    /* Echo mail */
#define T_BAD                0x00040000L    /* Bad and/or dupe mail */
#define T_HUDSON             0x00080000L    /* Hudson message base changed */
#define T_MSG                0x00100000L    /* *.MSG message base changed */
#define T_SQUISH             0x00200000L    /* Squish message base changed */
#define T_JAM                0x00400000L    /* JAM message base changed */
#define T_PERS               0x00800000L    /* Mail to Sysop received */
#define T_NEWAREA            0x01000000L    /* New echos have been created */
#define T_MSGNR              0x02000000L    /* Abort due to too many msgs */

/*
 *
 *  Special values for 'ALL'
 *
 */

#define ZONE_ALL             56685u
#define NET_ALL              56685u
#define NODE_ALL             56685u
#define POINT_ALL            56685u

/*
 *
 *  Misc other definitions required
 *
 */

#define FA_ANYFILE FA_RDONLY+FA_HIDDEN+FA_SYSTEM+FA_ARCH
#define _A_ANYFILE _A_RDONLY+_A_HIDDEN+_A_SYSTEM+_A_ARCH

#define TRUE          1
#define FALSE          0
#define BLANK        ' '

typedef char str255[256];
typedef char str35[36];
typedef char str16[16];

typedef char pktdate[20];

struct array512
{
  word         len;
  char         longstring[512];
};

/* define some simple keyword replacements */

#define strdelete(s,p,num) strcpy(s+p-1,s+p+num)

#define stoz(s, z) memmove(z, ((char *)(s))+1, (size_t)((byte)*(s)))

#define stozn(s, z, n) memmove(z, ((char *)(s))+1, (size_t)(((byte)*(s)<=n) ? (byte)*(s) : n))

#define ztos(z, s) memmove((char *)(s)+1, z, strlen(z)); *(s)=(char)strlen(z)

#define STRSIZ               255                  /* default string length */

#define NORM_FP(p) MK_FP(FP_SEG(p)+(FP_OFF(p)/0x10), FP_OFF(p)%0x10)

/*
 *
 *  In case your compiler doesn't have these ...
 *
 */

#ifndef MAXPATH
  #define MAXPATH            80
#endif
#ifndef MAXDRIVE
  #define MAXDRIVE           3
#endif
#ifndef MAXDIR
  #define MAXDIR             66
#endif
#ifndef MAXFILE
  #define MAXFILE            9
#endif
#ifndef MAXEXT
  #define MAXEXT             5
#endif

/*
 *
 *  Structs used in IMAIL Configuration files
 *
 */

struct naddress                  /* std node number ... */
{
  word         zone;             /*  Zone Number */
  word         net;              /*  Net Number */
  word         node;             /*  Node Number */
  word         point;            /*  Point Number */
};

struct expt                                 /* used for IMAIL TOSS internal */
{
  struct naddress dest;
  bit    doexpt:1;                          /* export to this system */
  bit    add_seen:1;                        /* add this system to SEEN-BYs */
  bit    rsvd1:3;                           /* reserved */
};

struct eaddress                             /* used in Area Manager */
{
  struct naddress dstn;                     /* node number */
  bit             exp_only:1;               /* export only flag */
  bit             imp_only:1;               /* import only flag */
  bit             paused:1;                 /* echo mail paused */
  bit             denied:1;                 /* access is denied */
  bit             uplink:1;                 /* uplink */
  bit             rsvd1:3;                  /* reserved */
};

struct fwd_link                             /* used in Forward Manager */
{
  char         areasfile[MAXFILE+MAXEXT];   /* name of areas file */
  char         toprogram[10];               /* name of area manager */
  char         password[21];                /* area manager password */
  struct naddress uplink;                   /* address of uplink */
  byte         accessgroup;                 /* accessgroup for forwarding */
  byte         creategroup;                 /* creategroup for forwarding */
  char         filler[10];                  /* reserved */
};

struct dom {
  char           domain[21];                  /* name of domain */
  char           outbound[MAXPATH];           /* root outbound path */
  word           zones[ZONESDOM];             /* Zones in this domain */
  byte           akas[MAXAKAS];               /* =my= AKAs in this domain */
};

struct im_stats
{
  unsigned long  th_day_nr;                   /* nr this day */
  unsigned long  la_day_nr;                   /* nr last day */
  unsigned long  th_week_nr;                  /* nr this week */
  unsigned long  la_week_nr;                  /* nr last week */
  unsigned long  th_month_nr;                 /* nr this month */
  unsigned long  la_month_nr;                 /* nr last month */
  unsigned long  th_year_nr;                  /* nr this year */
  unsigned long  la_year_nr;                  /* nr last year */
  unsigned long  th_day_size;                 /* amount this day */
  unsigned long  la_day_size;                 /* amount last day */
  unsigned long  th_week_size;                /* amount this week */
  unsigned long  la_week_size;                /* amount last week */
  unsigned long  th_month_size;               /* amount this month */
  unsigned long  la_month_size;               /* amount last month */
  unsigned long  th_year_size;                /* amount this year */
  unsigned long  la_year_size;                /* amount last year */
};


/*
 *
 *  IMAIL.CF structure
 *
 */

struct im_config_type
{
  byte           im_ver_maj;                    /* Major Version */
  byte           im_ver_min;                    /* Minor Version */
  byte           struct_maj;                    /* reserved */
  byte           struct_min;                    /* reserved */
  char           sysop[MAXNAME];                /* name of sysop */
  struct naddress aka[MAXAKAS];                 /* the AKAs */
  struct dom     domains[MAXAKAS];              /* domain names & zones */
  byte           rsvd1[10];                     /* reserved */
  char           netmail[MAXPATH];              /* net mail subdirectory */
  char           sec_inbound[MAXPATH];          /* secure inbound files */
  char           in_pkt[MAXPATH];               /* Directory for inbound PKTs */
  char           out_pkt[MAXPATH];              /* Directory for outbound PKTs */
  char           outbound[MAXPATH];             /* outbound directory */
  char           quickbbs[MAXPATH];             /* QuickBBS system directory */
  char           unsec_inbound[MAXPATH];        /* Unsecure inbound files */
  char           echotoss[MAXPATH];             /* name of echotoss.log */
  char           dupebase[MAXPATH];             /* dupe data base directory */
  char           semaphor[MAXPATH];             /* Semaphor directory */
  char           logfilename[MAXPATH];          /* Log file name */
  char           before_toss[MAXPATH];          /* call before proc. a PKT */
  char           semaphor_net[MAXFILE+MAXEXT];  /* Netmail rescan semaphor file */
  char           alnk_help[MAXFILE+MAXEXT];     /* AreaLink help text */
  char           maint_help[MAXFILE+MAXEXT];    /* Alnk Remote Maint. Helptext */
  char           rsvd2[MAXFILE+MAXEXT];         /* reserved */
  char           dflt_origin[MAXORIGIN];        /* default origin line */
  bit            rtnrecpt:1;                    /* True if to send rtn recpt */
  bit            del_empty_msg:1;               /* delete empty netmails (TOSS) */
  bit            ARCmail06:1;                   /* ARCmail 0.6 compatibility */
  bit            use_crc_names:1;               /* use crc-names for auto-areas */
  bit            req_all_allowed:1;             /* allow arealink +* command */
  bit            multi_tasking:1;               /* TRUE if multi-tasking */
  bit            ignore_unknown:1;              /* ALNK ignores unknown systems */
  bit            singleextract:1;               /* extract 1 bundle at a time */
  bit            trunc_sent:1;                  /* 1 = Trunc 0 = Delete */
  bit            keep_alnk_answ:1;              /* keep arealink answer */
  bit            prod_names:1;                  /* use the FTSC product list */
  bit            swap_ems:1;                    /* swap to EMS */
  bit            swap_ext:1;                    /* swap to extended memory */
  bit            forward_everything:1;          /* forward req. not in fwd-lists */
  bit            direct_video:1;                /* use direct screen writing */
  bit            close_at_end:1;                /* close graphic window at end */
  bit            compr_after_pkt:1;             /* compress after each PKT? */
  bit            delete_bases:1;                /* when removing an area, */
                                                /* delete also squish/msg-base */
  bit            quiet_packers:1;               /* send packer output >NUL */
  bit            use_imcomp:1;                  /* call IMCOMP in case of tight */
                                                /* diskspace or abort at once */
  bit            sort_alnk_lists:1;             /* sort ALNK lists by areatag */
  bit            ulnk_hudson_passth:1;          /* unlinked Hudson areas passth */
  bit            rsvd3:2;                       /* reserved */
  time_t         last_run;                      /* last maintenance run */
  word           rsvd4;                         /* reserved */
  byte           rsvd5;                         /* reserved */
  byte           rsvd6;                         /* reserved */
  word           max_arcmail_size;              /* max size of arcmail bundles */
  word           pwd_expire_days;               /* days before pwd expr'd */
  word           max_pkt_size;                  /* max size of pkt to create */
  byte           max_add_pkt;                   /* PKTs to compress in one run */
  byte           pkt_not_for_us;                /* how to handle PKTs not for us */
  byte           environment;                   /* FroDo, Binkley or Intermail */
  byte           max_msg_size;                  /* max size of netmail (split) */
  byte           via_line;                      /* add Via Line to netmails */
  byte           dupe_ring;                     /* Check for possible d-rings */
  byte           cpd_check;                     /* circular path detection */
  byte           pers_mail;                     /* use personal mail feature */
  byte           unlink_req;                    /* Unlink areas without dlink */
  byte           keep_alnk_req;                 /* keep arealink request */
  byte           rsvd7;                         /* reserved */
  unsigned long  max_dupes;                     /* max dupes kept in dbase */
  word           max_files_per_dir;             /* max. nr files when autocreate */
  byte           deadlink_days;                 /* nr of days for a dealink req */
  byte           rsvd8;                         /* reserved */
  char           bbs_system;                    /* BBS software used */
  char           new_areas[MAXPATH];            /* name of file for new areas */
  word           sp_before_unpack;              /* min. diskspace required */
  word           sp_before_toss;                /* before decompress, toss */
  word           sp_before_compress;            /* and compress (in MB). */
  char           kill_dead;                     /* Kill Dead Selection */
  word           prod[20];                      /* Type2+ product codes */
  long           setup_pwd_crc;                 /* Setup password (CRC) */
  char           rule_path[MAXPATH];            /* path to the rule-files */
  char           local_inbound[MAXPATH];        /* path to local PKTs (protected) */
  char           rsvd9[556];                    /* reserved */
  struct fwd_link fwd[MAXFWDLINK];              /* forward link requests */
  char           echojam[MAXPATH];              /* path to ECHOMAIL.JAM */
  char           before_toss_ii[MAXPATH];       /* call before proc. the PKTs */
  char           userbase[MAXPATH];             /* path to the userbase */
  unsigned long  stoptossmsgs;                  /* stop tossing after xxxxx msgs */
  unsigned long  stoptossnetmsgs;               /* stop tossing after xxxxx net */
                                                /* msgs within a PKT or at all */
  char           ignorelist[MAXPATH];           /* list of areas to suppress */
  char           db_queue[MAXPATH];             /* D'Bridge queue directory */
  long           log_level;                     /* logging level */
  char           att_status;                    /* Def. status of attach msg */
  char           msg_status;                    /* Def. status of Alnk msgs */
  char           filler[278];                   /* reserved */
};


/*
 *
 *  IMAIL.AR structure
 *
 */

struct areas_record_type
{
  char           aname[MAXTAG];              /* area name */
  char           comment[61];                /* area comment */
  char           origin[MAXORIGIN];          /* origin line to use */
  byte           grp;                        /* area group */
  char           o_addr;                     /* address for origin */
  char           use_akas[MAXAKAS];          /* addresses for seen-bys */
  byte           msg_base_type;              /* message base type */
  byte           brd;                        /* board number */
  char           msg_path[MAXPATH];          /* MSG/Squish path */
  bit            active:1;                   /* flag area active */
  bit            zone_gate:1;                /* Zone-gate stripping */
  bit            tiny_seen:1;                /* tiny seen-by flag */
  bit            secure:1;                   /* secure flag */
  bit            import_seen:1;              /* import seen-by into base */
  bit            deleted:1;                  /* flag deleted area */
  bit            auto_added:1;               /* flag auto-added record */
  bit            mandatory:1;                /* area is mandatory */
  bit            read_only:1;                /* area is read only */
  bit            unlinked:1;                 /* area has been unlinked */
  bit            ulnk_req:1;                 /* perform unlinked requests? */
  bit            hidden:1;                   /* area is hidden */
  bit            to_link:1;                  /* should by processed by LINK */
  bit            check_dup:1;                /* check for dupes in this area? */
  bit            no_pause:1;                 /* %PAUSE not allowed in this echo? */
  bit            hide_seen:1;                /* Hide seens when importing */
  bit            manual:1;                   /* No changes via Arealink */
  bit            fwdreq_pending:1;           /* Requested but yet not arrived */
  bit            sqkillfly:1;                /* Squish 'Kill on the fly' */
  bit            dupe_msgid:1;               /* Dupecheck on MSGID only? */
  bit            deadlink_req:1;             /* Deadlink request has been sent */
  bit            rsvd:2;                     /* reserved */
  byte           user_bits;                  /* 8 user-available bits */
  byte           days;                       /* days to keep messages */
  word           msgs;                       /* num messages to keep */
  struct im_stats stats;                     /* statistics */
  time_t         creation;                   /* date/time of statistic start */
  time_t         update;                     /* last update by midnight update */
  time_t         marked;                     /* used by kill dead */
  byte           kill_dead;                  /* kill echos without traffic */
  word           read_sec;                   /* Security level for read access */
  word           write_sec;                  /* Security level for write access */
  char           rulename[MAXFILE+MAXEXT];   /* filename for the rule file */
  char           filler[16];
  struct eaddress export[MAXEXPORT];         /* export list */
};

    /* --- Notes --------------------------------------------------

    1) The entries in 'o_addr' and 'use_akas' are indexes into
       the list of AKAs in IMAIL.CF, minus 1. eg:
          im_config.aka[o_addr-1]
       A value of 0 means 'no address'
    2) the 'user_bits' entry allows third-part software to store
       extra information in IMAIL.AR. Their meaning is program-
       specific, so be careful when making use of them!

    3) IMAIL.AR knows three predefined areatags:

        BADMAIL       for the IMAIL badmail area,
        DUPES         for the IMAIL dupe area and
        PERSMAIL      for the IMAIL personal mail area.

       All three boards are treated as local areas and
       the BADMAIL area must be present and non-passthru
       otherwise IMAIL will not run.

    ------------------------------------------------------------- */


/*
 *
 *  IMAIL.AI & IMAIL.BI structures
 *
 */

    /* --- Notes ---------------------------------------------------

    1) These are "true" index files, created by BPlus routines;
       they cannot be used unless you have the BPlus source
       code as implemented in IMAIL.

    ------------------------------------------------------------- */

/*
 *
 *  IMAIL.ND structure
 *
 */

struct node_record_type
{
  struct naddress  dstn;                  /* Node to pack for */
  char             sysop[MAXNAME];        /* name of sysop */
  char             domain;                /* index to domain */
  char             pwd[21];               /* AreaLink password */
  char             att_status;            /* Status of file attach msg */
  char             program;               /* packer to use ("packers") */
  long             grp_set[8];            /* groups node can request */
  word             capability;            /* capability word for node */
  bit              allow_remote:1;        /* allow remote maint. */
  bit              check_pkt_pwd:1;       /* check PKT password */
  bit              auto_cap:1;            /* TRUE = auto-detect capability */
  bit              send_rules:1;          /* send rules when linking */
  bit              newarea_add:1;         /* auto add to new areas? */
  bit              newarea_create:1;      /* create new areas? */
  bit              rescan_ok:1;           /* allow node to rescan */
  bit              notify:1;              /* Send notify messages? */
  bit              alnk_with_plus:1;      /* Add '+' to Arealink request */
  bit              rsvd1:1;               /* reserved */
  bit              forward_req:1;         /* Forward ALNK requests? */
  bit              uplink:1;              /* Is this system an uplink? */
  bit              fsc_comp:1;            /* Is a FSC-0057 arealink used? */
  bit              change_packer:1;       /* system may change packer */
  bit              check_alnk:1;          /* check for Arealink-Msg in echos */
  bit              extarcmail:1;          /* use 'A'-'Z' for arcmail extension*/
  word             user_bits;             /* 16 user-available bits */
  byte             newarea_grp;           /* new areas default group */
  char             pkt_pwd[9];            /* PKT password */
  time_t           last_pwd_change;       /* time ALNK pwd last changed */
  char             toprogram[11];         /* name of area manager */
  char             msg_status;            /* Status of Alnk messages */
  char             pkt_o_addr;            /* Aka to use for this system */
  unsigned long    msgs_in;               /* nr of msgs received */
  unsigned long    bytes_in;              /* amount of msgs received */
  unsigned long    msgs_out;              /* nr of msgs sent */
  unsigned long    bytes_out;             /* amount of msgs sent */
  time_t           lrdate;                /* date/time of statistic start */
  byte             pack_priority;         /* priority (9-0) when compressing */
  word             security;              /* Security level for area access */
  word             max_arcmail_size;      /* max size of arcmail bundles */
  word             max_pkt_size;          /* max size of pkt to create */
  byte             max_add_pkt;           /* PKTs to compress in one run */
  struct naddress  pkt_routing;           /* Node to route PKTS */
  char             filler[30];
  struct node_record_type *next_node;
  struct node_record_type *prev_node;
};


    /* --- Notes ---------------------------------------------------

    1) The last 2 fields are not saved to IMAIL.ND
    2) the 'user_bits' entry allows third-part software to store
       extra information in IMAIL.ND. Their meaning is program-
       specific, so be careful when making use of them!
    3) The entry in 'pkt_o_addr' is an index into the list of AKAs
       in IMAIL.CF, minus 1. eg:
         im_config.aka[pkt_o_addr-1]
       A value of 0 means 'use o_addr of the echo'

    ------------------------------------------------------------- */


/*
 *
 *  IMAIL.GR structure
 *
 */

struct group_record_type                 /* used in Group Manager */
{
  char           grp_desc[27];             /* Group description */
  byte           msg_base_type;            /* message base type */
  char           msg_path[MAXPATH];        /* MSG/Squish/Jam path */
  bit            active:1;                 /* flag area active */
  bit            zone_gate:1;              /* Zone-gate stripping */
  bit            tiny_seen:1;              /* tiny seen-by flag */
  bit            secure:1;                 /* secure flag */
  bit            import_seen:1;            /* import seen-by into base */
  bit            mandatory:1;              /* area is mandatory */
  bit            read_only:1;              /* area is read only */
  bit            ulnk_req:1;               /* perform unlinked requests? */
  bit            hidden:1;                 /* area is hidden */
  bit            check_dup:1;              /* check for dupes in this area? */
  bit            no_pause:1;               /* %PAUSE not allowed in this echo? */
  bit            hide_seen:1;              /* Hide seens when importing */
  bit            sqkillfly:1;              /* Squish 'Kill on the fly' */
  bit            dupe_msgid:1;             /* Dupecheck on MSGID only? */
  bit            manual:1;                 /* Manual only */
  bit            rsvd:1;                   /* reserved */
  byte           days;                     /* days to keep messages */
  word           msgs;                     /* num messages to keep */
  char           o_addr;                   /* address for origin */
  char           use_akas[MAXAKAS];        /* addresses for seen-bys */
  byte           kill_dead;                /* kill echos without traffic */
  byte           low_board;                /* Hudson boards for autocreate */
  byte           high_board;               /* lowest and highest */
  word           read_sec;                 /* Security level for read access */
  word           write_sec;                /* Security level for write access */
  char           filler[44];               /* reserved */
};


/*
 *
 *  IMAIL.RO structure
 *
 */

struct pack_routing_type                    /* used in pack routing */
{
  struct naddress dst;                      /* pack via this node */
  struct naddress nodes[MAXVIA];            /* nodes to pack */
  struct naddress except[MAXEXCEPT];        /* exceptions */
};

  /* Note: In the current implementation, the pack routing has */
  /*       as much records as defined in MAXPACK. */

/*
 *
 *  IMAIL.PP structure
 *
 */

struct packers
{
  char           packname[5];
  char           packer[MAXPACKNAME];
};

struct compression_type
{
  struct packers prg[MAXPACKERS];               /* Packer defintions */
  char           arcunpak[MAXPACKNAME];         /* cmd to de-arc .ARC files */
  char           arjunpak[MAXPACKNAME];         /* cmd to de-arc .ARJ files */
  char           pkpakunpak[MAXPACKNAME];       /* cmd to de-arc .PKA files */
  char           pkzipunpak[MAXPACKNAME];       /* cmd to de-arc .ZIP files */
  char           lharcunpak[MAXPACKNAME];       /* cmd to de-arc .LZH files */
  char           zoounpak[MAXPACKNAME];         /* cmd to de-arc .ZOO files */
  char           pakunpak[MAXPACKNAME];         /* cmd to de-arc .PAK files */
  char           sqzunpak[MAXPACKNAME];         /* cmd to de-arc .SQZ files */
  char           uc2unpak[MAXPACKNAME];         /* cmd to de-arc .UC2 files */
  char           rarunpak[MAXPACKNAME];         /* cmd to de-arc .RAR files */
  char           unkunpak[MAXPACKNAME];         /* cmd to de-arc unknown files */
};


/*
 *
 *  IMAIL.DP/IMAIL.DPI structure
 *
 */

    /* --- Notes ---------------------------------------------------------

    1) The dupe database consists simply of a series of long-sized values,
       which are a two 32-bit CRC of (in order) the following data:

       CRC 1: Addressee's name
              Sender's name
              Date
              Subject
       CRC 2: Address and MSGID (if any otherwise 0L)

    -------------------------------------------------------------------- */

/*
 *
 *  IMAIL.NI structure
 *
 */

struct no_import_type
{
  char           names[MAXNOIMPT][MAXNAME];
};


/*
 *
 *  IMAIL.SN structure
 *
 */

struct sysop_names
{
  char           names[MAXSYSNAME][MAXNAME];
};

#pragma pack()

/*--------------------------------EOF-----------------------------------*/


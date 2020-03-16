/*****************************************************************************
                         Maximus Structure Definitions
 *****************************************************************************

 Copyright 1989-1991 by Scott J. Dudley.  All rights reserved.
 Portions copyright 1990 by Bit Bucket Software, Inc.  Used with permission.

 This file contains the C structures used by Maximus 2.0.  Most of the
 structures were designed to be compatible with future versions, so
 PLEASE write your programs to permit variable-length structures. When
 we finally changed the user file size in Max 2.0, we found that a
 number of poorly-written third-party programs would not work under
 the new version.  (This is talking collectively, as Max SysOps.)

 However, programs which were properly written to spec worked as
 advertised, with no problems encountered.  The user file, the area
 file, and a few others are all VARIABLE LENGTH.  This may make things
 difficult if you're coding in Pascal or BASIC, but it's a cinch under
 C, and it makes things much more expandable.

 Just to make sure that everyone follows this, I may make the next
 version Maximus generate a random user record size.  }:->

 This file isn't as clean as I'd like it to be; it was thrown together
 in about an hour or so, scraping together the various tidbits from
 all over the Max source.  I had planned to clean it up and publish
 some sort of developer's kit, but I don't have time.

 The format for the Squish message base will be published in a
 separate file, with a full source release of the Maximus level 0
 MsgAPI layer.  I indend to make this available in a short period of
 time, but no guarantees.

 Scott J. Dudley
 November 11th, 1991
 FidoNet 1:249/106
 Internet f106.n249.z1.fidonet.org

 *****************************************************************************/


#include <time.h>

/* Portability declarations */

#if defined(__386__) || defined(__FLAT__)
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned short word;
  typedef signed short   sword;

  typedef unsigned int  dword;
  typedef signed int    sdword;

  typedef unsigned short ushort;
  typedef   signed short sshort;

  typedef unsigned long  ulong;
  typedef   signed long  slong;
#else
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned int  word;
  typedef signed int    sword;

  typedef unsigned long dword;
  typedef signed long   sdword;

  typedef unsigned short ushort;
  typedef   signed short sshort;

  typedef unsigned long  ulong;
  typedef   signed long  slong;
#endif




/* DOS-style bitmapped datestamp */

struct _stamp   
{
  struct
  {
    unsigned int da : 5;
    unsigned int mo : 4;
    unsigned int yr : 7;
  } date;

  struct
  {
    unsigned int ss : 5;
    unsigned int mm : 6;
    unsigned int hh : 5;
  } time;
};


struct _dos_st
{
  word date;
  word time;
};

/* Union so we can access stamp as "int" or by individual components */

union stamp_combo   
{
  dword ldate;
  struct _stamp msg_st;
  struct _dos_st dos_st;
};

typedef union stamp_combo SCOMBO;



/* Access levels, as used in user file, PRM file, and others. */

#define  TWIT        -2 /* 0xFFFE */
#define  DISGRACE    0x0000
#define  LIMITED     0x0001  /**/
#define  NORMAL      0x0002
#define  WORTHY      0x0003  /**/
#define  PRIVIL      0x0004
#define  FAVORED     0x0005  /**/
#define  EXTRA       0x0006
#define  CLERK       0x0007  /**/
#define  ASSTSYSOP   0x0008
#define  SYSOP       0x000A
#define  HIDDEN      0x000b


/* Miscellaneous definitions */

#define MAX_KEYS           32
#define MAX_ALEN           10   /* Max length of usr.msg[] and usr.files[]  */
#define MAX_OVR            16   /* Maximum # of override privs per area     */
#define NUM_MENU          256   /* Max. # of options in a menu file         */
#define PATHLEN           120   /* Max. length of a path                    */
#define MAX_MENUNAME  PATHLEN   /* Max. length of menuname[].               */
#define MAXEXPAND          30   /* max wildcard expansion                   */
#define CHAR_BITS           8   /* Number of bits in a `char' variable      */
#define MAX_AREAS        1296   /* Maximum number of total areas -- obsolete*/



/* Enumeration `option' -- All possible values for menu.option[x].type,     *
 * below.                                                                   */

typedef enum
{
  nothing,

  MISC_BLOCK=100, display_menu, display_file, message, file, other,
                  o_press_enter, key_poke, clear_stacked, o_if,
                  o_menupath, area_change, o_cls,


  XTERN_BLOCK=200, xtern_erlvl, xtern_dos, xtern_run, xtern_chain,
                   xtern_concur,

  MAIN_BLOCK=300, goodbye, statistics, o_yell, userlist, o_version,
                  user_editor, leave_comment, climax,

  MSG_BLOCK=400, same_direction, read_next, read_previous,
                 enter_message, msg_reply, read_nonstop,
                 read_original, read_reply, msg_list, msg_scan,
                 msg_inquire, msg_kill, msg_hurl, forward, msg_upload,
                 xport, read_individual, msg_checkmail, msg_change,
                 msg_tag, msg_browse, msg_current, msg_edit_user,
                 msg_upload_qwk,

  FILE_BLOCK=500, locate, file_titles, file_type, upload, download, raw,
                  file_kill, contents, file_hurl, override_path,
                  newfiles, file_tag,

  /* Options generally found on the Change Setup menu */

  CHANGE_BLOCK=600, chg_city, chg_password, chg_help, chg_nulls,
                    chg_width, chg_length, chg_tabs, chg_more,
                    chg_video, chg_editor, chg_clear, chg_ibm,
                    chg_phone, chg_realname, chg_hotkeys,
                    chg_language, chg_userlist, chg_protocol,
                    chg_fsr, chg_archiver,

  EDIT_BLOCK=700, edit_save, edit_abort, edit_list, edit_edit,
                  edit_insert, edit_delete, edit_continue, edit_to,
                  edit_from, edit_subj, edit_handling, read_diskfile,
                  edit_quote,

  /* Stuff that was hacked on after the original implementation */

  CHAT_BLOCK=800, who_is_on, o_page, o_chat_cb, chat_toggle, o_chat_pvt,
    
  END_BLOCK,


  /* Everything below here is RESERVED by Maximus for future uses!         *
   * Also, everything ABOVE is fairly stable.  If changes have to be made, *
   * the old options above will NOT be re-used.  For example, if the       *
   * `edit_insert' command should become obsoleted for some reason, that   *
   * slot would either get retired and do nothing, or perform the NEW      *
   * edit_insert function.                                                 */

  rsvd=32766  /* This was stuck in to make sure that the `option'          *
               * enumeration uses a word, instead of a byte, in case we    *
               * really expand this structure sometime soon.               */

} option;


/* An individual menu option.  There are many of these contained in one    *
 * _menu file, following the _menu data header, optionally with some       *
 * NULL-terminated strings between each _opt structure, for the argument.  *
 * One of these for each option in *.MNU.                                  */

struct _opt
{
#ifdef __FLAT__
  /* force enum to be 16 bits */
  word type;
#else
  option type;  /* What this menu option does                              */
#endif

  word priv;    /* Priv level required to execute this command             */
  dword lock;   /* Bit-field locks for this particular menu option         */
  word flag;    /* See the OFLAG_xxx contants for more info.               */
  word name;    /* The menu option, as it appears to user                  */
  word keypoke; /* Auto-keypoke string                                     */
  word arg;     /* The argument for this menu option                       */
  byte areatype;/* If this particular option can only be used if the user  *
                 * is in a certain message-area type.                      */
  byte fill1;   /* Reserved by Maximus for future use                      */

  byte rsvd[8]; /* Reserved for future uses */
};


/* Header of each *.MNU file */

struct _menu
{
  word header,      /* What to display when the user enters menu, such as  *
                     * "The MESSAGE Section", "The CHG SETUP Section", etc */
       num_options, /* Total number of options (struct _opt's) in menu     */
       menu_length; /* Number of lines long the .?BS menu file is!         */

  sword hot_colour; /* What colour to display if a user uses hotkeys to    *
                     * bypass a .?BS menu display, before displaying the   *
                     * key.  -1 == display nothing.                        */

  word title;       /* Length of the title string, not counting \0.        */
  word headfile;    /* Length of the header filename, not counting \0      */
  word dspfile;     /* Name of file to display for menu, instead of        *
                     * generating menu from .Mnu file.                     */
  word flag;        /* See MFLAG_XXX, below.                               */
};



#define MFLAG_MF_NOVICE   0x0001u /* MenuFile for these levels only */
#define MFLAG_MF_REGULAR  0x0002u
#define MFLAG_MF_EXPERT   0x0004u
#define MFLAG_MF_HOTFLASH 0x0008u

#define MFLAG_MF_ALL      (MFLAG_MF_NOVICE | MFLAG_MF_REGULAR | \
                           MFLAG_MF_EXPERT | MFLAG_MF_HOTFLASH)

#define MFLAG_HF_NOVICE   0x0010u /* HeaderFile for these levels only */
#define MFLAG_HF_REGULAR  0x0020u
#define MFLAG_HF_EXPERT   0x0040u
#define MFLAG_HF_HOTFLASH 0x0080u

#define MFLAG_HF_ALL      (MFLAG_HF_NOVICE | MFLAG_HF_REGULAR | \
                           MFLAG_HF_EXPERT | MFLAG_HF_HOTFLASH)

#define MFLAG_SILENT      0x0100u /* Silent menuheader option */




/* Structure of BBSTATxx.BBS */

#define STATS_VER           1   /* Version number of the BBSTATxx.BBS file */

struct _bbs_stats
{
  byte    version;
  dword   num_callers;
  dword   quote_pos;
  dword   msgs_written;
  time_t  online_date;
  dword   total_dl;
  dword   total_ul;
  sword   today_callers;
  union stamp_combo date;
};

/* Structure for entries in PROTOCOL.MAX */

struct _proto
{
  #define P_ISPROTO 0x01  /* This bit always set                            */
  #define P_BATCH   0x02  /* Can handle batch transfers                     */
  #define P_OPUS    0x04  /* Write an Opus-style .CTL file                  */
  #define P_ERL     0x08  /* Exit with xtern_erlvl                          */
  #define P_BI      0x10  /* Bidirectional transfer                         */

  word flag;

  char desc[40];
  char log[PATHLEN];
  char ctl[PATHLEN];
  char dlcmd[PATHLEN];
  char ulcmd[PATHLEN];
  char dlstr[40];
  char ulstr[40];
  char dlkey[40];
  char ulkey[40];
    
  word fnamword;
  word descword;
};


/* Structure for IPCxx.BBS header */

struct _cstat
{
  word avail;

  byte username[36];
  byte status[80];

  word msgs_waiting;

  dword next_msgofs;
  dword new_msgofs;
};



/* Types for _cdat.type */

#define CMSG_PAGE       0x00   /* "You're being paged by another user!"     */
#define CMSG_ENQ        0x01   /* "Are you on this chat channel?"           */
#define CMSG_ACK        0x02   /* "Yes, I AM on this channel!"              */
#define CMSG_EOT        0x03   /* "I'm leaving this chat channel!"          */
#define CMSG_CDATA      0x04   /* Text typed by used while in chat          */
#define CMSG_HEY_DUDE   0x05   /* A normal messge.  Always displayed.       */
#define CMSG_DISPLAY    0x06   /* Display a file to the user                */

/* Message data element within IPCxx.BBS */

struct _cdat
{
  word tid;
  word type;
  word len;

  dword rsvd1;
  word  rsvd2;
};


/* Handle for saving CHAT status.  Mainly used internally, but also        *
 * in RESTARxx.BBS.                                                        */

struct _css
{
  word avail;
  byte status[80];
};

/***************************************************************************
                    Definitions for the .PRM file
 ***************************************************************************/

#define OFS word

#define MAX_DRIVES         26   /* Maximum number of drives on system      */
#define CHAR_BITS           8   /* Number of bits in a 'char'              */
#define CTL_VER             9   /* Version number of BBS.PRM               */

/* This macro is ONLY used for accessing *pointers* in the `prm' structure.
   This is required, due to the way Wynn has made OPUS_CTL write the strings
   out (which uses a lot less memory than some other ways).  If you want
   to access an INT, or a non-pointer in the structure, then you can use
   a `prm.var_name'-style reference.                                       */

#define PRM(s) (offsets+(prm.s))

#define MAX_LANG           8       /* Max. number of possible languages    */
#define MAX_YELL          10       /* Max number of yell slots             */
#define MAX_EXTERNP       16       /* max. number of external programs     */
#define MAXCLASS          12       /* number of possible priv levels       */
#define ALIAS_CNT         15       /* number of matrix addresses           */


          /** Definitions for the `prm.flags' variable **/

#define FLAG_keyboard    0x0001 /* If local mode is on by default          */
#define FLAG_watchdog    0x0002 /* Use watchdog for outside commands       */
#define FLAG_snoop       0x0004 /* If snoop is on by default               */
#define FLAG_norname     0x0008 /* If we should disable ^aREALNAME kludge  */
#define FLAG_close_sf    0x0010 /* Close all standard files for O)utside   */
#define FLAG_break_clr   0x0020 /* Send a break signal to dump modem's     *
                                 * internal buffer                         */
#define FLAG_log_echo    0x0040 /* Log user-written echomail               */
#define FLAG_no_ulist    0x0080 /* User can't press '?' to list users in   *
                                 * msg.                                    */
#define FLAG_no_magnet   0x0100 /* Disable the MagnEt editor               */
#define FLAG_autodate    0x0200 /* Automatically search directory for      */
                                /* file size & date.                       */
#define FLAG_statusline  0x0400 /* If SysOp wants a status line on screen  */
#define FLAG_ask_phone   0x0800 /* If we should ask user for phone number  */
#define FLAG_noyell      0x1000 /* If yell is toggled on or off by Sysop   */
#define FLAG_lbaud96     0x2000 /* If we should use 9600 baud for local    *
                                 * callers... (For Opus compatibility!)    */
#define FLAG_alias       0x4000 /* If we're running a system which allows  *
                                 * aliases or handles                      */
#define FLAG_ask_name    0x8000 /* If we should ask user for their alias   *
                                 * name too -- Only needed if using        *
                                 * FLAG_alias.                             */

#define FLAG2_gate       0x0001 /* Gate netmail messages, use zonegate!    */
#define FLAG2_has_snow   0x0002 /* Video adapter is slow CGA w/snow        */
#define FLAG2_msgread    0x0004 /* If arrow keys can be used for reading   */
#define FLAG2_ltimeout   0x0008 /* Local keyboard timeout                  */
#define FLAG2_noshare    0x0010 /* SHARE not used -- don't lock files!     */
#define FLAG2_CAPTURE    0x0020 /* Sysop chat capture automatically on     */
#define FLAG2_NOCRIT     0x0040 /* Don't use internal crit.err handler     */
#define FLAG2_CHECKDUPE  0x0080 /* Check for duplicate uploads             */
#define FLAG2_CHECKEXT   0x0100 /* Compare extension for duplicate uploads */

#define LOG_terse       0x02
#define LOG_verbose     0x04
#define LOG_trace       0x06

#define MULTITASKER_none        0
#define MULTITASKER_doubledos   1
#define MULTITASKER_desqview    2
#define MULTITASKER_topview     3
#define MULTITASKER_mlink       4
#define MULTITASKER_mswindows   5
#define MULTITASKER_os2         6
#define MULTITASKER_pcmos       7

#define VIDEO_DOS         0x00 /* Standard DOS output hooks */
#define VIDEO_FOSSIL      0x01 /* FOSSIL write-character function */
#define VIDEO_IBM         0x02 /* Direct screen writes */
#define VIDEO_FAST        0x03 /* Semi-fast undocumented DOS call */
#define VIDEO_BIOS        0x04 /* Semi-faster int 10h BIOS writes */



/* Special values for the character set byte */

#define CHARSET_SWEDISH   0x01
#define CHARSET_CHINESE   0x02

#define XTERNEXIT       0x40      /* If external protocl has erlvl exit */
#define XTERNBATCH      0x80      /* If protocol can do batch transfers */

#ifndef _ADDRESS_DEFINED
#define _ADDRESS_DEFINED
typedef struct _ADDRESS
{
  word Zone;
  word Net;
  word Node;
  word Point;
} ADDR;
#endif


struct   cl_rec
{
  sword    priv;
  word     max_time;      /* max cume time per day         */
  word     max_call;      /* max time for one call         */
  word     max_dl;        /* max dl kbytes per day         */
  word     ratio;         /* ul:dl ratio                   */
  word     min_baud;      /* speed needed for logon        */
  word     min_file_baud; /* speed needed for file xfer    */
};

/* Note: To read in the *.PRM structure, first read in the m_pointers       *
 * structure, which is always contained at the beginning of the file.       *
 * Then, seek to the offset prm.heap_offset, and read in everything         *
 * from there to EOF into heap[].  All of the 'OFS' type variables          *
 * are simply offsets into the variable-length heap which started at        *
 * heap_offset. To obtain a string from the .PRM heap, simply               *
 * add the offset in the m_pointers structure to the address of the         *
 * heap[] variable that the heap was read into.  For example, to access     *
 * the string for 'system_name', you'd use '(heap+prm.system_name)'.        *
 * Alternatively, you can declare a macro to do this, such as the           *
 * PRM() macro shown above.  (Maximus itself uses the variable              *
 * 'strings' instead of 'heap' to hold the varible-length strings,          *
 * but the concept is the same.)  When using the PRM() macro to             *
 * access the string for 'system_name', you'd simply write:                 *
 * 'PRM(system_name)', which is a lot clearer.  Also, please note that      *
 * NON-OFS variables should be accessed normally!  That means that          *
 * 'task_num', 'auto_kill', can be access with 'prm.task_num',              *
 * 'prm.auto_kill', etc.  The special heap manipulation is only needed      *
 * for strings.                                                             */

struct m_pointers
{

        /*-----------------------------------------------------------*/
        /* DATA                                                      */
        /*-----------------------------------------------------------*/

  byte  id;             /* Always equal to 'M'               STABLE  */
  byte  version;        /* for safety                        STABLE  */
  word  heap_offset;    /* OFFSET OF BEGINNING OF HEAP!      STABLE  */
  byte  task_num;       /* for multi-tasking systems         STABLE  */
  sword com_port;       /* Com1=0, Com2=1, etc               STABLE  */
  byte  noise_ok;       /* If yell noise is currently on     STABLE  */

  /* Miscellanious system information */

  byte  video;          /* Mode for local video display              */
  byte  log_mode;       /* What style of logging to use              */
  word  max_baud;    /* fastest speed we can use                  */
  byte  multitasker;    /* flag for DoubleDos (see below)            */
  byte  nlver;          /* Which nodelist version we use (5 or 6)    */
  word  min_ulist;      /* Min and max privs for the U)serlist cmd   */
  word  max_ulist;      /*    "                                      */

  /* Information about errorlevels */

  byte  exit_val;       /* Erl to use after caller if none of below  */
  byte  edit_exit;      /* erl to use after matrix mail written      */
  byte  echo_exit;      /* ERRORLEVEL for after inbound echomail     */
  byte  local_exit;     /* Errorlevel after entering local msgs      */

  /* Modem information */

  sword carrier_mask;
  sword handshake_mask;

  /* Log-on information */

  sword logon_priv;     /* Access level for new users                */
  word  logon_time;     /* time to give for logons                   */
  word  min_baud;    /* minimum baud to get on-line               */
  word  speed_graphics; /* min baud for graphics                  */

  /* Information about message areas */

  byte  auto_kill;      /* RECD PVT msgs. 0=no 1=ask 2=yes            */

  sword ctla_priv;      /* Priv to see CONTROL-A lines in messages    */
  sword seenby_priv;    /* Min priv to see SEEN-BY line               */
  sword pvt_priv;       /* Min priv to read pvt msgs                  */

  sword msg_ask[16];    /* Array of privs. for message attr ask's     */
  sword msg_assume[16]; /* Array of privs. for message attr assume's  */
  sword msg_fromfile;   /* Priv. for doing message from file          */
  byte  rsvd1[4];       /* used to be high_msgarea, begin_msgarea     */
  sword unlisted_priv;  /* Priv needed to send to unlisted node       */
  sword unlisted_cost;  /* Charge to send to unlisted node            */

  sword mc_reply_priv;   /* Priv to reply to msg with mailchecker     */
  sword mc_kill_priv;    /* Priv to kill msg with mailchecker         */


  /* Information about file areas */

  sword date_style;     /* Used for FILES.BBS display                */
  sword dlall_priv;     /* Priv. needed to DL file not in FILES.BBS  */
  sword ulbbs_priv;     /* Priv. needed to UL *.BBS files            */
  dword k_free;         /* The number of disk space (in K) which     *
                         * must be available before we let a user    *
                         * upload.                                   */
  word  ul_reward;      /* Percentage reward for uploads             */
  word  ratio_threshold;/* K which can DL before harass about ratio  */

  byte  rsvd2[4];       /* used to be high_filearea, begin_filearea  */

  /* Our matrix address(es) */

  ADDR address[ALIAS_CNT];


  /*  struct _yell yell[MAX_YELL]; */  /* Yell info moved to event file */
  byte rsvd3[60]; /* Reserved by Maximus for future use */


  /* About the users */

  struct cl_rec class[MAXCLASS];

  /* Flags for external protocols */

  sword protoexit;              /* Errorlevel for protocol exit      */
  char  protoflag[MAX_EXTERNP]; 

  /* General-purpose bit-flags  (See FLAGx_xxx definitions above.) */

  word  flags;
  word  flags2;
  word  flags3;
  word  flags4;

  /* Bit field containing drive letters to save when going outside */
  char  drives_to_save[(MAX_DRIVES/CHAR_BITS)+1];

  byte  fbbs_margin;      /* Margin to use for wrapping FILES.BBS comments */

  byte  rsvd999;

  word  max_ptrs;         /* Maximum size of pointers of ALL *.LTFs */
  word  max_heap;         /* Maximus heap size of all *.LTFs */
  byte  max_lang;         /* Current number of languages */
  byte  rsvd_lang;

  word  max_glh_ptrs;
  word  max_glh_len;

  word  max_syh_ptrs;
  word  max_syh_len;

  byte  input_timeout;   /* # of mins until Max hangs up due to no input    */

  byte  charset;         /* Character set support - see CHARSET_XXXX, above */
  word  max_pack;        /* Maximum # of msgs to pack into a .QWK packet    */
  
  byte  rsvd65[12];      /* Reserved by Maximus for future use              */



  /* --------------------------------------------------------------- */
  /* -------------------------- OFFSETS ---------------------------- */
  /* --------------------------------------------------------------- */

  /* About your system */

  OFS   sysop;          /* sysop's name. MUST be first offset in prm file */
  OFS   system_name;    /* board's name                              */

  /* Modem commands */

  OFS   m_busy;         /* mdm cmd to take modem off hook            */

  /* Paths to various places */

  OFS   sys_path;       /* path to SYSTEM?.BBS files                 */
  OFS   misc_path;      /* path to `F-key files'                     */
  OFS   net_info;       /* path to NODELIST files                    */
  OFS   temppath;       /* place to put temporary files              */
  OFS   ipc_path;       /* path for inter-process communications     */

  /* General files needed by the system */

  OFS   user_file;      /* path/filename of User.Bbs                 */
  OFS   log_name;       /* name of the log file                      */
  OFS   chat_prog;      /* External chat program, if any             */
  OFS   chat_fbegin;    /* File to display instead of "CHAT: begin"  */
  OFS   chat_fend;      /* File to display instead of "END CHAT"     */
  OFS   local_editor;   /* Command for local editor, if any          */
  OFS   notfound;       /* User name not found in user file          */
  OFS   junk;           /* Don't use this for anything!              */

  /* General *.?BS files needed everywhere */

  OFS   logo;           /* first file shown to a caller              */
  OFS   bad_logon;      /* if user's last logon attempt was bad      */
  OFS   welcome;        /* shown after logon                         */
  OFS   quote;          /* For displaying "random" quotes from       */
  OFS   newuser1;       /* Asks user to type in password             */
  OFS   newuser2;       /* Replaces `welcome' for a new user         */
  OFS   rookie;         /* Replaces `welcome' for rookies            */
  OFS   application;    /* new user questionnaire                    */
  OFS   byebye;         /* file displayed at logoff                  */
  OFS   out_leaving;    /* Bon Voyage                                */
  OFS   out_return;     /* Welcome back from O)utside                */
  OFS   daylimit;       /* Sorry, you've been on too long...         */
  OFS   timewarn;       /* warning about forced hangup               */
  OFS   tooslow;        /* explains minimum logon baud rate          */
  OFS   barricade;      /* Displayed before prompt for access code   */
  OFS   shelltodos;     /* Displayed when Sysop hits Alt-J           */
  OFS   backfromdos;    /* Displayed when Sysop returns from Alt-J   */
  OFS   areanotexist;   /* File to display instead of "That area     *
                         * doesn't exist!"                           */

  /* File-area items */

  OFS   rsvd6;          /* Reserved by Maximus for future use        */
  OFS   xferbaud;       /* explains minimum file transfer baud rate  */
  OFS   file_area_list; /* dump file... used instead of Dir.Bbs      */
  OFS   no_space;       /* File to display if trying to UL with      *
                         * less than k_free space left on drive.     */
  OFS   fname_format;   /* Essay on MS-DOS filenames for U)ploads    */
  OFS   ul_log;         /* Log file for uploads                      */

  OFS   file_header;    /* Format for file area's A)rea command      */
  OFS   file_format;    /* Format for A)rea command entries          */
  OFS   file_footer;    /* Format for footer for file.area menu      */

  OFS   proto_dump;      /* Dump file for protocol screen            */

  /* Message-area items */

  OFS   msgarea_list;   /* dump file... used instead of Dir.Bbs      */
  OFS   echotoss_name;  /* Name of your echomail tosslog             */
  OFS   nomail;         /* Display by mailchecker if no mail wtng.   */

  OFS   msg_header;     /* Format for msg.area's A)rea command       */
  OFS   msg_format;     /* Format for A)reas command entries         */
  OFS   msg_footer;     /* Format for footer for msg.area menu       */

  /* Help files:  Used to explain various things */

  OFS   hlp_editor;     /* intro to msg editor for novices.          */
  OFS   hlp_replace;    /* Explain the Msg.Editor E)dit command      */
  OFS   msg_inquire;    /* Explain the Msg. I)nquire command         */
  OFS   hlp_locate;     /* Explain the Files L)ocate command         */
  OFS   hlp_contents;   /* Explain the Files C)ontents command       */
  OFS   oped_help;      /* help file for the full-screen editor      */
  OFS   hlp_scan;       /* help file for S)can                       */
  OFS   hlp_list;       /* help file for L)ist                       */

  /* External protocols */

  OFS   protocols[MAX_EXTERNP]; /* external file protocol programs   */
  OFS   protoname[MAX_EXTERNP]; /* name of protocol, on menu         */

  /* Date/Time format strings */

  OFS   timeformat;
  OFS   dateformat;

  /* Paths/filenames of the AREAS.DAT and AREAS.IDX files */

  OFS   adat_name;
  OFS   aidx_name;

  /* Menu paths/names */

  OFS   menupath;        /* The default place to look for the menus */
  OFS   first_menu;      /* The name of the first menu to display */
  OFS   edit_menu;       /* Name of the EDIT menu */
  
  /* Miscellaneous */
  
  OFS   achg_keys;       /* Characters used to change area -/+ */
  OFS   tune_file;       /* Path to TUNES.MAX */
  OFS   lang_path;       /* Path to *.LTF files */
  
  OFS   lang_file[MAX_LANG]; /* Array of all *.LTF names */
  
  OFS   m_init;          /* Modem initialization string */
  OFS   m_ring;          /* Command modem sends when phone ringing */
  OFS   m_answer;        /* Cmd to send to modem when ring detect */
  OFS   m_connect;       /* Connect string, as returned by modem */

  OFS   high_msgarea;
  OFS   begin_msgarea;  /* Msg area to put new users in              */
  
  OFS   high_filearea;
  OFS   begin_filearea; /* File area to put new users in             */
  
  OFS   fidouser;       /* Name of FIDOUSER.LST file to use          */
  OFS   cmtarea;        /* Message area to put comments in           */

  OFS   arc_ctl;        /* Control file for archiving programs       */
  OFS   olr_name;       /* OLR: Filename to use for DL packets       */
  OFS   olr_dir;        /* OLR: Directory for off-line stuff         */
  OFS   phone_num;
  OFS   viruschk;       /* Name of batch file to call for virus check*/
};




/* Structure for AREA.DAT */

#ifdef NEVER

/*  NOTE:  The _area structure has a dynamic length!  To access this file, *
 *         you should read the first _area structure from the file, and    *
 *         check the struct_len byte.  Then, to access the file, you seek  *
 *         to each new location, instead of reading straight through.      *
 *                                                                         *
 *         For example, to read all of the _area file into an array, you   *
 *         MUST do it like this, for upward compatiblity:                  */

  {
    struct _area area[NUM_AREAS];

    int x,
        slen;

    if ((areafile=open(area_name,O_RDONLY | O_BINARY))==-1)
      Error();

    /* Read the first record of the file, to grab the structure-length     *
     * byte.                                                               */

    read(areafile,&area[0],sizeof(struct _area));
    slen=area[0].struct_len;

    for (x=0;! eof(area_data);x++)
    {
      /* Note this lseek() call, which positions the pointer to the        *
       * start of the next record, no matter how long the previous         *
       * record was.                                                       */

      lseek(areafile,x*(long)struct_len,SEEK_SET);
      read(areafile,&area[x],sizeof(struct _area));
    }

    close(areafile);
  }

#endif


struct _override
{
  sword priv;   /* Override priv level */
  dword lock;   /* Override lock setting */

  byte ch;      /* First letter of menu option to apply override to */
  byte fill;    /* Reserved by Maximus */
};



#define AREA_ID   0x54414441L /* "ADAT" */
#define AREA_id   AREA_ID

struct _area
{
  long id;              /* Unique identifier for AREA.DAT structure.       *
                         * Should be AREA_id, above.                       */

  word struct_len;      /* Length of _area structure -- this needs only    *
                         * to be read from the first record in an area     *
                         * data file, since it can be assumed to remain    *
                         * the same throughout the entire file.  This is   *
                         * GUARANTEED to be at offset four for this and    *
                         * all future versions of this structure.          */

  word areano;          /* OBSOLETE.  Two-byte integer representation of   *
                         * this area's name.  Use area.name instead.       */

  byte name[40];        /* String format of area's name.  USE THIS!        */

  /*************************************************************************/
  /**                        Message Area Information                     **/
  /*************************************************************************/

  word type;            /* Message base type.  MSGTYPE_SDM = *.MSG.        *
                         * MSGTYPE_SQUISH = SquishMail.  (Constants are    *
                         * in MSGAPI.H)                                    */

  byte msgpath[80];     /* Path to messages                                */
  byte msgname[40];     /* The 'tag' of the area, for use in ECHOTOSS.LOG  */
  byte msginfo[80];     /* The DIR.BBS-like description for msg section    */
  byte msgbar[80];      /* Barricade file for message area                 */
  byte origin[62];      /* The ORIGIN line for this area                   */

  sword msgpriv;        /* This is the priv required to access the msg     *
                         * section of this area.                           */
  byte fill0;           /* The lock for the message area (obsolete)        */

  byte fill1;

  sword origin_aka;     /* This is the AKA number to use on the origin     *
                         * line.  See the normal SysOp documentation on    *
                         * the "Origin" statement, for info on how this    *
                         * number is used.                                 */

  /*************************************************************************/
  /**                        File Area Information                        **/
  /*************************************************************************/


  byte filepath[80];    /* Path for downloads                              */
  byte uppath[80];      /* Path for uploads                                */
  byte filebar[80];     /* Barricade file for file areas                   */
  byte filesbbs[80];    /* Path to FILES.BBS-like catalog for this area    */
  byte fileinfo[80];    /* The DIR.BBS-like description for file section   */

  sword filepriv;       /* This is the priv required to access the file    *
                         * section of this area.                           */
  byte fill15;          /* The locks for the file area (obsolete)          */
  byte fill2;

  /*************************************************************************/
  /**                      Miscellaneous Information                      **/
  /*************************************************************************/


  byte msgmenuname[13]; /* Alternate *.MNU name to use for this msg.area   */
  byte filemenuname[13];/* Alternate *.MNU name to use for this file area  */

  word attrib[MAXCLASS];/* This is an array of attributes for the          *
                         * msg/file areas.  These are dependant on PRIV    *
                         * level.  Once you have the CLASS number for a    *
                         * particular user (via Find_Class_Number()), you  *
                         * can find the attributes for that particular     *
                         * priv level like this: "area.attrib[class]"      *
                         * ...which will get you the attribute for that    *
                         * priv level.                                     */

  /*************************************************************************/
  /**                      Stuff hacked on later                          **/
  /*************************************************************************/

  struct _override movr[MAX_OVR]; /* Override privs for msg/file areas */
  struct _override fovr[MAX_OVR];
  
  dword msglock;        /* 32-bit locks for message areas                  */
  dword filelock;       /* 32-bit locks for file areas                     */

  word killbyage;       /* MAXREN: max # of days to keep msgs in this area */
                        /*         (use 0 for no deletion by age)          */
  word killbynum;       /* MAXREN: max # of msgs to keep in area (use 0    */
                        /*         for no deletion by #msgs.)              */

};



/* New Max 2.xx format for AREA.NDX.  The file is simply an array of        *
 * these structures.                                                        */

struct _aidx
{
  dword offset;
  byte name[MAX_ALEN];
};


/* This is the old, Max 1.02 format for AREA.IDX.  This is obsolete, but    *
 * it is still written by SILT for backwards compatibility.                 */

struct _102aidx
{
  word  area;       /* Same format as area.areano */
  dword offset;
  dword rsvd;
};



/* Structure for MTAG.BBS */

struct _tagdata
{
  word struct_len;
  char name[36];
  char areas[348];
};


/* Structure for RESTARxx.BBS */

/* NOTE: The following structure is not completely stable.  Unless         *
 * rst.rst_ver is equal to RST_VER, then the ONLY items you're guaranteed  *
 * to be able to read are those marked with "*STABLE*".  Those items       *
 * are guaranteed to be stored at those offsets for all future versions    *
 * of Maximus, regardless of the version number.  However, everything      *
 * else is likely to change at a moment's notice.                          */

struct _restart
{
  byte rst_ver; /* Version number of restart data                 *STABLE* */

  sdword timeon;  /* Date user got on system, seconds since 1970  *STABLE* */
  sdword timeoff; /* Date user must be OFF system, secs since '70 *STABLE* */
  sdword restart_offset; /* Offset in .BBS file to restart at     *STABLE* */

  dword baud;             /* User's baud rate                   *STABLE*   */
  dword max_time;         /* Max time, as given by '-t' param   *STABLE*   */

  sword port;             /* Current COM port, 0=COM1, 1=COM2,  *STABLE*   */

  char written_echomail;  /* 0=user HASN'T written echomail     *STABLE*   */
  char written_matrix;    /* 0=user HASN'T entered matrix msg   *STABLE*   */
  char local;             /* 0=NOT local                        *STABLE*   */

  struct _stamp laston;   /* Time the user was last on system   *STABLE*   */
  
  word steady_baud;       /* Locked baud rate of user           *STABLE*   */

  sdword starttime;       /* Start time, for external protocol             */
  sdword timestart;       /* Time when MAX.EXE was started                 */
  sdword ultoday;         /* KB's the user has uploaded today              */

  union stamp_combo next_ludate;
  
  byte restart_type;      /* 1 if started via .BBS file, 0 otherwise       */
  char restart_name[PATHLEN]; /* Name of .BBS file to restart in           */
  char menuname[PATHLEN]; /* Name of current menu                          */
  char menupath[PATHLEN]; /* The current menu path                         */
  char firstname[36];     /* The user's first name                         */
  char last_onexit[PATHLEN]; /* The 'onexit' filename for current .BBS file*/
  char parm[PATHLEN];     /* Parms for external program, if any            */
  char fix_menupath[PATHLEN]; /* Readjust menu name                        */
  char last_name[MAX_MENUNAME]; /* Name of the last menu                   */

  char lastmenu;          /* Last ^oR menu choice                          */
  char snoop;             /* If snoop is currently on or off               */

  char locked;            /* If priv is locked via keyboard 'L' command    */
  char locked2;           /* If priv locked via barricade                  */

  char keyboard;          /* If the Sysop's keyboard is turned on          */
  char protocol_letter;   /* Letter representing current protocol choice   */

  char chatreq;           /* If user wanted to chat with SysOp             */
  char mn_dirty;          /* If menuname buf is dirty                      */

  char barricade_ok;      /* If current barricade area is OK               */
  char no_zmodem;         /* If zmodem not allowed                         */

  sword usr_time;         /* User's usr.time value                         */
  sword usernum;          /* User's user number                            */
  sword lockpriv;         /* If rst.locked (above), then this is real priv */
  sword lockpriv2;        /* If rst.locked2, then this is real priv        */
  sword ctltype;          /* Control-file type (for xternal protocol)      */

  word current_baud;      /* User's baud rate, as a mask for mdm_baud() */

  /* Bit flags for ECHOTOSS.LOG */
  char echo_written_in[(MAX_AREAS/CHAR_BITS)+1];

  struct _area area;
  struct _css css;
  
  char log_name[80];

  word fnames;

  char  filenames[MAXEXPAND][PATHLEN];
  dword filesizes[MAXEXPAND];
  word  fileflags[MAXEXPAND];
  
  char event_num;
  char rsvd;
  
  struct _tagdata tma;        /* Tagged message areas */
  sword last_protocol;
  long getoff;
  char returning[PATHLEN];
};

#ifndef _NODE_DEFINED
  #define _NODE_DEFINED
    
  struct _node  /* NODELIST.SYS */
  {
     sword number;        /* node number                                   */
     sword net;           /* net number                                    */
     word  cost;          /* cost of a message to this node                */
     word  rate;          /* baud rate                                     */
     byte  name[20];      /* node name                                     */
     byte  phone[40];     /* phone number                                  */
     byte  city[40];      /* city and state                                */
  };
#endif


#ifndef _NEWNODE_DEFINED
  #define _NEWNODE_DEFINED

  struct _newnode /* NODELIST.DAT */
  {
     word NetNumber;
     word NodeNumber;
     word Cost;                                 /* cost to user for a
                                                 * message */
     byte SystemName[34];                       /* node name */
     byte PhoneNumber[40];                      /* phone number */
     byte MiscInfo[30];                         /* city and state */
     byte Password[8];                          /* WARNING: not necessarily
                                                 * null-terminated */
     word RealCost;                             /* phone company's charge */
     word HubNode;                              /* node # of this node's hub
                                                 * or 0 if none */
     byte BaudRate;                             /* baud rate divided by 300 */
     byte ModemType;                            /* RESERVED for modem type */
     word NodeFlags;                            /* set of flags (see below) */
     word NodeFiller;
  };
#endif

/*------------------------------------------------------------------------*/
/* Values for the `NodeFlags' field                                       */
/*------------------------------------------------------------------------*/
#define B_hub      0x0001
#define B_host     0x0002
#define B_region   0x0004
#define B_zone     0x0008
#define B_CM       0x0010
#define B_ores1    0x0020
#define B_ores2    0x0040
#define B_ores3    0x0080
#define B_ores4    0x0100
#define B_ores5    0x0200
#define B_res1     0x0400
#define B_res2     0x0800
#define B_point    0x1000
#define B_res4     0x2000
#define B_res5     0x4000
#define B_res6     0x8000


/* Help levels */

#define  EXPERT      (byte)0x02  /* grizzled veteran, no menus at all       */
#define  REGULAR     (byte)0x04  /* experienced user, brief menus           */
#define  NOVICE      (byte)0x06  /* Full menus plus additional hand-holding */
#define  HOTFLASH    (byte)0x20  /* Hotkey, full-screen interface           */

/* Msg/file area attributes */

#define  SYSMAIL   0x0001 /* is a mail area                                */
#define  NOPUBLIC  0x0004 /* OPUS: Disallow public messages                */
#define  NOPRIVATE 0x0008 /* OPUS: Disallow private messages               */
#define  ANON_OK   0x0010 /* OPUS: Enable anonymous messages               */
#define  ECHO      0x0020 /* OPUS: Set=Echomail Clear=Not Echomail         */
#define  HIGHBIT   0x0040 /* MAX:  Allow high-bit chars in this area       */
#define  NREALNAME 0x0200 /* MAX:  Don't use ^aREALNAME for this area      */
#define  UREALNAME 0x0400 /* MAX:  Use usr.name instead of alias (if alsys)*/
#define  CONF      0x0800 /* MAX:  Conference-type area (no origin/sb's)   */
#define  UALIAS    0x1000 /* MAX:  Use usr.alias instead of usr.name       */

#define  SHARED     (CONF | ECHO)
#define  NOPVTORPUB (NOPRIVATE | NOPUBLIC)

/* Structure for COLOURS.MAX */

struct _maxcol
{
  byte menu_name;         /* yellow */
  byte menu_high;         /* yellow */
  byte menu_text;         /* gray */
  byte file_name;         /* yellow */
  byte file_size;         /* magenta */
  byte file_date;         /* green */
  byte file_desc;         /* cyan */
  byte file_find;         /* yellow */
  byte file_off;          /* red */
  byte file_new;          /* blinking green */
  byte msg_from;          /* cyan */
  byte msg_to;            /* cyan */
  byte msg_subj;          /* cyan */
  byte msg_from_txt;      /* yellow */
  byte msg_to_txt;        /* yellow */
  byte msg_subj_txt;      /* yellow */
  byte msg_date;          /* lightgreen */
  byte msg_attr;          /* lightgreen */
  byte addr_type;         /* cyan */
  byte addr_locus;        /* green */
  byte msg_text;          /* gray */
  byte msg_quote;         /* cyan */
  byte msg_kludge;        /* lightmagenta */
  byte hot_opt;           /* black on white */
  byte hot_more;          /* lightred on white */
  byte hot_clr;           /* white on white */
  byte status_bar;        /* black on white */
  byte status_cht;        /* blinking black on white */
  byte status_key;        /* blinking black on white */
  byte fsr_msgn;          /* lightred on blue */
  byte fsr_msglink;       /* yellow on blue */
  byte fsr_attr;          /* yellow on blue */
  byte fsr_msginfo;       /* yellow on blue */
  byte fsr_date;          /* white on blue */
  byte fsr_addr;          /* yellow on blue */
  byte fsr_static;        /* white on blue */
  byte fsr_border;        /* lightcyan on blue */
  byte pop_text;          /* white on blue */
  byte pop_border;        /* yellow on blue */
  byte pop_high;          /* yellow on blue */
  byte pop_list;          /* black on grey */
  byte pop_lselect;       /* grey on red */

  byte wfc_stat;          /* white on blue */
  byte wfc_stat_bor;      /* yellow on blue */
  byte wfc_modem;         /* gray on blue */
  byte wfc_modem_bor;     /* lgreen on blue */
  byte wfc_keys;          /* yellow on blue */
  byte wfc_keys_bor;      /* white on blue */
  byte wfc_activ;         /* white on blue */
  byte wfc_activ_bor;     /* lcyan on blue */
  byte wfc_name;          /* yellow on black */
  byte wfc_line;          /* white on black */
};



/* Structure for USER.BBS and LASTUSER.BBS */

/* NOTE:  This structure is semi-stable.  Although it is still compatible  *
 * with the old Opus 1.03 structure, don't expect it to stay that way      *
 * for long.  In a future version, Maximus will be using a dymaic-sized    *
 * user record, making it possible to make additions without breaking      *
 * preexisting software.  You can start to code for this now in your       *
 * software, as the usr.struct_len variable indicates the length of the    *
 * current user structure, divided by twenty.  This allows us to build up  *
 * a base of utilities, and be able to switch to a new format (while still *
 * not breaking anything) in the future.  Also, if usr.sruct_len==0, then  *
 * you MUST assume that the length of the structure is actually 180 bytes  *
 * long, as Opus (and Maximus v1.00 only) did not use this field.  In      *
 * other words:                                                            *
 *                                                                         *
 * len_of_struct=(usr.struct_len ? (usr.struct_len*20) : 180)              *
 *                                                                         *
 * In addition, you can assume that all user records in the user file are  *
 * the SAME size...  ie. You can just read the first user record out of    *
 * the file, and you are assured that the rest of the records in the file  *
 * area also the same size.                                                *
 *                                                                         *
 *                                                                         *
 * Example for reading in the dynamic-sized user structure:                *
 *                                                                         *
 *    {                                                                    *
 *      struct _usr users[MAX_USERS];                                      *
 *                                                                         *
 *      int x,                                                             *
 *          userfile,                                                      *
 *          s_len;                                                         *
 *                                                                         *
 *      if ((userfile=open(ufile_name,O_RDONLY | O_BINARY))==-1)           *
 *        Error();                                                         *
 *                                                                         *
 *      read(userfile,&users[0],sizeof(struct _usr));                      *
 *                                                                         *
 *      s_len=users[0].struct_len ? users[0].struct_len*20 : 180;          *
 *                                                                         *
 *      for (x=0;x < MAX_USERS;x++)                                        *
 *      {                                                                  *
 *        lseek(userfile,(long)x*(long)s_len,SEEK_SET);                    *
 *        read(userfile,&users[x],sizeof(struct _usr));                    *
 *      }                                                                  *
 *                                                                         *
 *      close(userfile);                                                   *
 *    }                                                                    *
 *                                                                         *
 * If anything is added to the user structure, it will be appended to the  *
 * END of the structure, so you can be assured that the offsets of each    *
 * individual variable will NOT change.                                    *
 *                                                                         *
 * Also, when ADDING or DELETING users, certain special operations have    *
 * to be performed, mainly those related to the lastread pointers.  When   *
 * adding a user, the procedure is fairly simple; just make sure that      *
 * usr.lastread_ptr is a unique number, different from all others in       *
 * USER.BBS.  Although Max uses a somewhat complicated algorithm to        *
 * fill gaps in the user file, most utility programs can just read through *
 * USER.BBS, and keep a running tally of the HIGHEST usr.struct_len        *
 * variable.  Once you have that, increment it by one, and stuff it into   *
 * the usr.struct_len of the user to be added.                             *
 *                                                                         *
 * When DELETING users, you must go through the process of "cleansing"     *
 * the lastread pointers for the user you deleted.  The procedure for this *
 * is simple:  For every area listed in AREAS.CTL, open the LASTREAD.BBS   *
 * file for that area, and seek to the offset...                           *
 *                                                                         *
 *    usr.lastread_ptr*(long)sizeof(int)                                   *
 *                                                                         *
 * ...and write *two* NUL bytes (ASCII 00).                                *
 *                                                                         *
 * Please note that you do NOT need to do anything special to sort the     *
 * user file...  Since the lastread offset is stored in usr.lastread_ptr,  *
 * you can sort the user file with impunity, and even use old Opus 1.03    *
 * sort utilities.                                                         */



/* Masks for usr.bits1, below */

#define BITS_HOTKEYS     0x0001 /* Hotkeys, independent of HOTFLASH level   */
#define BITS_NOTAVAIL    0x0002 /* If set, user is NOT normally available   *
                                 * for chat.                                */
#define BITS_FSR         0x0004 /* Full-screen reading in msg areas         */
#define BITS_NERD        0x0008 /* Yelling makes no noise on sysop console  */
#define BITS_NOULIST     0x0010 /* Don't display name in userlist           */
#define BITS_TABS        0x0020 /* Reserved                                 */
#define BITS_BIT6        0x0040 /* Reserved                                 */
#define BITS_BIT7        0x0080 /* Reserved                                 */
#define BITS_BIT8        0x0100 /* Used to be 'usr.msg'                     */
#define BITS_BIT9        0x0200 /* Used to be 'usr.msg'                     */
#define BITS_BITA        0x0400 /* Used to be 'usr.msg'                     */
#define BITS_BITB        0x0800 /* Used to be 'usr.msg'                     */
#define BITS_BITC        0x1000 /* Used to be 'usr.msg'                     */
#define BITS_BITD        0x2000 /* Used to be 'usr.msg'                     */
#define BITS_BITE        0x4000 /* Used to be 'usr.msg'                     */
#define BITS_BITF        0x8000 /* Used to be 'usr.msg'                     */


/* Masks for usr.bits2, below */

#define BITS2_BADLOGON   0x0001 /* MAX: if user's last logon attempt was bad*/
#define BITS2_IBMCHARS   0x0002 /* MAX: if user can receive high-bit chars  */
#define BITS2_RSVD1      0x0004 /* MAX: *obsolete* 1.02 avatar flag         */
#define BITS2_BORED      0x0008 /* Use the line-oriented editor             */
#define BITS2_MORE       0x0010 /* Wants the "MORE?" prompt                 */
#define BITS2_RSVD2      0x0020 /* OPUS: set=wants Ansi                     */
#define BITS2_CONFIGURED 0x0040 /* OPUS: set=used Maximus before            */
#define BITS2_CLS        0x0080 /* OPUS: set=transmit ^L, clear=ignore ^L   */
#define BITS2_BIT8       0x0100 /* used to be 'usr.keys'                    */
#define BITS2_BIT9       0x0200 /* used to be 'usr.keys'                    */
#define BITS2_BITA       0x0400 /* used to be 'usr.keys'                    */
#define BITS2_BITB       0x0800 /* used to be 'usr.keys'                    */
#define BITS2_BITC       0x1000 /* used to be 'usr.keys'                    */
#define BITS2_BITD       0x2000 /* used to be 'usr.keys'                    */
#define BITS2_BITE       0x4000 /* used to be 'usr.keys'                    */
#define BITS2_BITF       0x8000 /* used to be 'usr.keys'                    */


/* Masks for usr.delflag, below */

#define UFLAG_DEL   0x01
#define UFLAG_PERM  0x02

/* Masks for usr.xp_flag, below */

#define XFLAG_EXPDATE    0x0001 /* Use the xp_date to control access        */
#define XFLAG_EXPMINS    0x0002 /* Use the xp_mins number to control access */
#define XFLAG_DEMOTE     0x0004 /* Demote user to priv level in usr.xp_priv */
#define XFLAG_AXE        0x0008 /* Just hang up on user                     */

/* Constants for usr.video, below */

#define GRAPH_TTY         0x00 /* The current user's graphics setting...    */
#define GRAPH_ANSI        0x01 
#define GRAPH_AVATAR      0x02


struct   _usr
   {
      byte name[36];        /* Caller's name                                */
      byte city[36];        /* Caller's location                            */

      byte alias[21];       /* MAX: user's alias (handle)                   */
      byte phone[15];       /* MAX: user's phone number                     */

      word lastread_ptr;    /* MAX: a num which points to offset in LASTREAD*/
                            /* file -- Offset of lastread pointer will be   */
                            /* lastread_ptr*sizeof(int).                    */

      word timeremaining;   /* MAX: time left for current call (xtern prog) */

      byte pwd[16];         /* Password                                     */
      word times;           /* Number of previous calls to this system      */
      byte help;            /* Help level                                   */
/**/  byte rsvd1[2];        /* Reserved by Maximus for future use           */
      byte video;           /* user's video mode (see GRAPH_XXXX)           */
      byte nulls;           /* Number of Nulls (delays) after <cr>          */

      byte bits;            /* Bit flags for user (number 1)                */

/**/  word rsvd2;           /* Reserved by Maximus for future use           */

      word bits2;           /* Bit flags for user (number 2)                */

      sword priv;           /* Access level                                 */
/**/  byte rsvd3[19];       /* Reserved by Maximus for future use           */
      byte struct_len;      /* len of struct, divided by 20. SEE ABOVE!     */
      word time;            /* Time on-line so far today                    */

      word delflag;         /* Used to hold baud rate for O)utside command  */
                            /* In USER.BBS, usr.flag uses the constants     */
                            /* UFLAG_xxx, defined earlier in this file.     */
      
/**/  byte rsvd4[8];        /* Reserved by Maximus for future use           */

      byte width;           /* Width of the caller's screen                 */
      byte len;             /* Height of the caller's screen                */
      word credit;          /* Matrix credit, in cents                      */
      word debit;           /* Current matrix debit, in cents               */

      word  xp_priv;        /* Priv to demote to, when time or minutes run  */
                            /* out.                                         */

      union stamp_combo xp_date;  /* Bit-mapped date of when user expires.  */
                                  /* If zero, then no expiry date.          */
 
      dword xp_mins;        /* How many minutes the user has left before    *
                             * expiring.                                    */

      byte  xp_flag;        /* Flags for expiry.  See above XFLAG_XXX defs. */
      byte  xp_rsvd;

      union stamp_combo ludate;   /* Bit-mapped date of user's last call    */

      dword xkeys;          /* User's keys (all 32 of 'em)                  */
      byte  lang;           /* The user's current language #                */
      sbyte def_proto;      /* Default file-transfer protocol               */

      dword up;             /* K-bytes uploaded, all calls                  */
      dword down;           /* K-bytes downloaded, all calls                */
      dword downtoday;      /* K-bytes downloaded, today                    */

      byte msg[MAX_ALEN];   /* User's last msg area (string)                */
      byte files[MAX_ALEN]; /* User's last file area (string)               */

      byte compress;        /* Default compression program to use           */

/**/  byte rsvd5;
      dword extra;
   };



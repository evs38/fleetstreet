/* SHOTGUN.H */

#pragma pack(1)

typedef struct
{
   unsigned short Zone,
                  Net,
                  Node,
                  Point;
} AddrType;

/*=============================================================================
/* \SHOTGUN\DATA\NETWORK.DAT                                                                             }
/*                                                                                                       }
/*  - Defines system network addresses and domain names.                                                 }
/*  - Multiple record file.                                                                              }
/*
/*============================================================================*/
typedef struct
{
   AddrType Net_Address;    /* Network address - first should be primary. */
   char     Net_Domain[16]; /* Network domain name, Pascal string */
} Network_Record;

/*=============================================================================
/* \SHOTGUN\DATA\ORIGINS.DAT
/*
/*  - Defines system origin lines.
/*  - Multiple record file.
/*
/*============================================================================*/
typedef struct
{
   char Net_Origin[51]; /* Origin lines for email (in step with addresses).  */
} Origin_Record;

/*===========================================================================
/* \SHOTGUN\DATA\MAxxxxxx.DAT
/*
/*  - Definition of a single message area on the system.
/*  - Paths MUST contain a trailing backslash.
/*  - Multiple record file.
/*
/*===========================================================================*/
typedef struct
{
   unsigned short Scrap1;
   char           Area_Name[41];         /*The descriptive name of the message area.       */
   unsigned short Sub;                   /*The message sub group that the area resides.    */
                                         /*This must NOT be '0' and the group must exist.  */
   char           Dos_Name[51];          /*Path/Filename of the message base. For FIDO MSG */
                                         /*areas, this must be a directory name, and for   */
                                         /*the others, it is a path and a filename (with   */
                                         /*no extension) of the message base.              */
   char           Sysop[31];             /*The name of the sysop of the area.              */
   unsigned char  Base_Type;             /*Message base format:                            */
                                         /*1-Jam.                                          */
                                         /*2-Squish.                                       */
                                         /*3-Fido.                                         */
   unsigned short Read_Access;           /*Minimum access level required to read messages. */
   unsigned short Write_Access;          /*Minimum access level required to write messages.*/
   unsigned char  Area_Type;             /*Message type:                                   */
                                         /* Local:    1-Public  2-Priv  3-Public/Priv      */
                                         /*  Echo:   10-Public 11-Priv 12-Public/Priv      */
                                         /*   Net:   20-Public 21-Priv 22-Public/Priv      */
                                         /*  Uucp:   30-Public 31-Priv 32-Public/Priv      */
   unsigned char  Msg_Type;              /*Name to use for messages:                       */
                                         /*1-Real name.                                    */
                                         /*2-Alias name.                                   */
                                         /*3-Real/Alias name.                              */
   unsigned char  File_Attaches;         /*0-Normal, 1-Allow file attaches.                */
   unsigned short File_Attaches_Sec;     /*Minimum access level for attaching files to     */
                                         /*messages in this area.                          */
   char           Tags[10];              /*User tags required to see the message area.     */
   unsigned char  Origin;                /*Index to the origin line used for this area.    */
   unsigned char  Address;               /*Index to the network address used for this area.*/
   char           NodeList[51];          /*Path to the *.SG nodelist files (for netmail).  */
   unsigned char  DoGraphics;            /*0-Normal, 1-Allow colour codes in message base. */
   unsigned char  CleanLanguage;         /*0-Normal, 1-Censor language using BADLANG.      */
   unsigned short AreaID;                /*Unique message area ID number. Every area must  */
                                         /*have an area ID different from all other areas! */
   unsigned short MaxMsgs;               /*Maximum number of messages to keep in the area. */
   unsigned char  ForcedTo;              /*0-Normal, 1-User can't change the msg recipient.*/
   char           Area_Tag[41];          /*Mail echo tag (if applicable).                  */
   char           Scrap2[84];
} MessageArea_Record;

#define BASETYPE_JAM     1
#define BASETYPE_SQUISH  2
#define BASETYPE_FIDO    3

#define SGAREATYPE_LOCAL   1
#define SGAREATYPE_ECHO    10
#define SGAREATYPE_NET     20
#define SGAREATYPE_UUCP    30

#define USENAME_REAL       1
#define USENAME_ALIAS      2
#define USENAME_REAL_ALIAS 3

/*=============================================================================
{ \SHOTGUN\DATA\SYSTEM.DAT
{
{  - Holds many global settings that effect the entire system.
{  - Paths MUST contain a trailing backslash.
{  - Single record file.
{
{============================================================================*/
typedef struct
{
   char Sysop[31];
   char Alias[31];
   /* Rest nicht verwendet */
} SystemDat_Record;

#pragma pack()

/* Ende SHOTGUN.H */

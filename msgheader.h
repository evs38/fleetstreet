/* MSGHEADER.H */

#pragma pack(1)
/* Fido-Adresse */

typedef struct
{
   USHORT usZone;
   USHORT usNet;
   USHORT usNode;
   USHORT usPoint;
} FTNADDRESS, *PFTNADDRESS;

/* Zeitstempel */

typedef struct
{
   unsigned int day     :5; /* 1..31 */
   unsigned int month   :4; /* 1..12 */
   unsigned int year    :7; /* Jahr -80 */
   unsigned int seconds :5; /* 0..59 */
   unsigned int minutes :6; /* 0..59 */
   unsigned int hours   :5; /* 0..23 */
} TIMESTAMP, *PTIMESTAMP;


#define NUM_REPLIES     10

typedef struct
{
   ULONG       ulAttrib;                     /* s. u. */
   char        pchFromName[LEN_USERNAME+1];
   char        pchToName[LEN_USERNAME+1];
   char        pchSubject[LEN_SUBJECT+1];
   FTNADDRESS  FromAddress;
   FTNADDRESS  ToAddress;
   TIMESTAMP   StampWritten;
   TIMESTAMP   StampArrived;
   ULONG       ulReplyTo;                    /* ID der Original-Message */
   ULONG       ulReplies[NUM_REPLIES];       /* IDs der Replies */
   ULONG       ulMsgID;                      /* ID dieser Message */
} MSGHEADER, *PMSGHEADER;

/* Allgemeine Attribute, von allen Formaten unterstuetzt */
#define ATTRIB_PRIVATE       0x0001UL
#define ATTRIB_CRASH         0x0002UL
#define ATTRIB_RCVD          0x0004UL
#define ATTRIB_SENT          0x0008UL
#define ATTRIB_FILEATTACHED  0x0010UL
#define ATTRIB_INTRANSIT     0x0020UL
#define ATTRIB_ORPHAN        0x0040UL
#define ATTRIB_KILLSENT      0x0080UL
#define ATTRIB_LOCAL         0x0100UL
#define ATTRIB_HOLD          0x0200UL
#define ATTRIB_READ          0x0400UL
#define ATTRIB_FREQUEST      0x0800UL
#define ATTRIB_RRQ           0x1000UL
#define ATTRIB_RECEIPT       0x2000UL
#define ATTRIB_AUDIT         0x4000UL
#define ATTRIB_UPDATEREQ     0x8000UL

/* Weitere Attribute, muessen umgemappt werden */
/* Squish */
#define ATTRIB_SCANNED       0x00010000UL

/* JAM */
#define ATTRIB_ARCHIVESENT   0x00020000UL
#define ATTRIB_DIRECT        0x00040000UL
#define ATTRIB_TRUNCFILE     0x00080000UL
#define ATTRIB_KILLFILE      0x00100000UL
#define ATTRIB_IMMEDIATE     0x00200000UL
#define ATTRIB_GATE          0x00400000UL
#define ATTRIB_FORCEPICKUP   0x00800000UL

/* ^aFLAGS */
#define ATTRIB_HUBROUTE      0x01000000UL

#define ATTRIB_KEEP          0x02000000UL
#define ATTRIB_NPD           0x04000000UL
#define ATTRIB_DELETED       0x08000000UL

#define ATTRIB_ALL           0x07ffffffUL

typedef struct kludge
{
   struct kludge   *next;
   struct kludge   *prev;
   ULONG           ulKludgeType;     /* s.u. */
   char            *pchKludgeText;
} KLUDGE, *PKLUDGE;

#define KLUDGE_OTHER         0UL /* falls nicht eines der anderen */
#define KLUDGE_ANY           0UL /* beim Suchen */
#define KLUDGE_FMPT          1UL
#define KLUDGE_TOPT          2UL
#define KLUDGE_INTL          3UL
#define KLUDGE_MSGID         4UL
#define KLUDGE_REPLY         5UL
#define KLUDGE_REPLYTO       6UL
#define KLUDGE_REPLYADDR     7UL
#define KLUDGE_FLAGS         8UL
#define KLUDGE_SPLIT         9UL
#define KLUDGE_PID          10UL
#define KLUDGE_AREA         11UL
#define KLUDGE_APPEND       12UL
#define KLUDGE_REALADDRESS  13UL
#define KLUDGE_ACUPDATE     14UL
#define KLUDGE_CHRS         15UL
/* CHARSET                  16UL  Aquivalent zu CHRS */
#define KLUDGE_CISTO        17UL
#define KLUDGE_CISFROM      18UL
#define KLUDGE_CISMSGID     19UL
#define KLUDGE_CISREPLY     20UL
#define KLUDGE_FWDFROM      21UL
#define KLUDGE_FWDTO        22UL
#define KLUDGE_FWDORIG      23UL
#define KLUDGE_FWDDEST      24UL
#define KLUDGE_FWDSUBJ      25UL
#define KLUDGE_FWDAREA      26UL
#define KLUDGE_FWDMSGID     27UL


typedef struct
{
   char    *pchMessageText;
   char    *pchSeenPath;  /* Zeigt in pchMessageText */
   PKLUDGE pFirstKludge;  /* Anfang und Ende der Kette */
   PKLUDGE pLastKludge;
} FTNMESSAGE, *PFTNMESSAGE;

#pragma pack()

/* Ende von MSGHEADER.H */

/* Main.H */

/* Return-Codes */
#define RET_NOACTION        0  /* Nix passiert */
#define RET_NEWNETMAIL      1  /* Neue Netmail eingegeben */
#define RET_NEWECHOMAIL     2  /* Neue Echomail eingegeben */
#define RET_NEWLOCALMAIL    4  /* Neue Localmail eingegeben */
#define RET_ERROR         255  /* Init-Fehler */

/* Programmstatus */
#define PROGSTATUS_READING    0
#define PROGSTATUS_EDITING    1
#define PROGSTATUS_NOSETUP    2
#define PROGSTATUS_CLEANUP    3

/* Feldlaengen */
#define LEN_USERNAME       35  /* alle excl. Nullbyte */
#define LEN_SUBJECT        71
#define LEN_ORIGIN         61
#define LEN_AREADESC       40
#define LEN_AREATAG        50
#define LEN_PATHNAME       CCHMAXPATH-1
#define LEN_MACRO        1023
#define LEN_REGCODE        40
#define LEN_COMMANDLINE    LEN_PATHNAME
#define LEN_QUOTETEMP      20
#define LEN_TEMPLTEXT     100
#define LEN_5DADDRESS      40
#define LEN_FINDTEXT       50
#define LEN_LISTTAG        50
#define LEN_DOMAIN         50
#define LEN_FIRSTLINE      79
#define LEN_SYSTEMNAME    100
#define LEN_PHONENR        50
#define LEN_LOCATION       50
#define LEN_PASSWORD       20
#define LEN_MODEMTYPE      20
#define LEN_SEARCHTEXT    100
#define LEN_REQFILE        50

void QuickMessage(HWND hwndOwner, char *text);

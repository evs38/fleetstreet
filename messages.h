/* MESSAGES.H */

/* Neue Messages */

/* Request-Messages erzeugen, mp1: PREQUESTPARAM */
#define REQM_REQUEST           (WM_USER+1)

/* Request-Dialog zu Ende, mp1: PREQUESTPARAM, mp2: DID */
#define REQM_CLOSE             (WM_USER+2)

/* Anzeigen der Zusatzinfos, mp1: PSZ ctlinfo, mp2: PSZ seenpath */
#define KM_SHOWKLUDGES         (WM_USER+8)

/* Zusatzinfo-Fenster schliesst sich */
#define KM_KLUDGEWNDCLOSE      (WM_USER+9)

/* Suchergebnis in Liste eintragen, mp1: XMSG *Header, mp2: TAGNUM *TagNum */
#define FM_INSERTRESULT        (WM_USER+10)

/* Neue Area absuchen, mp1: PSZ newarea, mp2: USHORT msgnum */
#define FM_FIND_NEWAREA        (WM_USER+11)

/* Progress-Indicator updaten, mp1: USHORT msgnum */
#define FM_FIND_UPDATEPROGRESS (WM_USER+12)

/* Fenster schliessen sich */
#define FM_FINDPROGRESSCLOSE   (WM_USER+13)
#define FM_FINDRESULTSCLOSE    (WM_USER+14)

/* Suchen ist zu Ende */
#define FM_FIND_END            (WM_USER+15)

/* Springen zu Suchergebnis, mp1: TAGNUM *TagNum */
#define FM_JUMPTOMESSAGE       (WM_USER+16)

/* Area gescannt, mp1: PSZ areatag */
#define SM_AREASCANNED         (WM_USER+17)

/* Scannen beendet */
#define SM_SCANENDED           (WM_USER+18)

/* Eingabedaten einer Setup-Seite pruefen */
#define SUM_CHECKINPUT         (WM_USER+19)

/* Ueberpruefen, ob Squish-Setup moeglich */
#define SUM_CHECKSETUP         (WM_USER+20)

/* Userdaten neu anzeigen */
#define SUM_REFRESHUSER        (WM_USER+21)

/* Thread-Liste komplett */
#define TM_THREADSREADY        (WM_USER+25)

/* Thread-Liste schliesst sich */
#define TM_THREADLISTCLOSE     (WM_USER+26)

/* Threads neu einlesen, mp1: MSGLISTPAR *pMsgListPar*/
#define TM_REREADTHREADS       (WM_USER+27)

/* Threadliste zu Message scrollen, mp1: PRECORDCORE */
#define TM_SCROLLTOMSG         (WM_USER+28)

/* Springen zu Message, mp1: int msgnum */
#define TM_JUMPTOMESSAGE       (WM_USER+29)

/* Threadliste deaktivieren, mp1: BOOL activate */
#define TM_ACTIVATE            (WM_USER+30)

/* Messages Markieren beendet */
#define MAM_MARKENDED          (WM_USER+31)

/* Renumber-Stufe, mp1: ULONG stage */
#define RENM_STAGE             (WM_USER+33)

/* Renumber-Fortschritt, mp1: ULONG processed Msgs, mp2: ULONG totalmsgs */
#define RENM_PROGRESS          (WM_USER+34)

/* Renumber beendet */
#define RENM_ENDED             (WM_USER+35)

/* Renumber-Fehler */
#define RENM_ERROR             (WM_USER+36)

/* Titel aendern bei Area-Settings */
#define APM_SETTITLE           (WM_USER+37)

/* Message-Liste schliesst sich */
#define MSGLM_CLOSE            (WM_USER+38)

/* Arealiste schlieát sich , mp1: hwndClosingWindow */
#define ALM_CLOSE              (WM_USER+40)

/* Area-Wechsel, mp1: pchNewArea */
#define ALM_SWITCHAREA         (WM_USER+41)

/* Area geloescht, mp1: pchDelArea */
#define ALM_DELAREA            (WM_USER+42)

/* Template-Notebook geschlossen, mp1: ulTemplateID */
#define TPL_CLOSE              (WM_USER+43)

/* Template-Folder geschlossen */
#define TPLF_CLOSE             (WM_USER+44)

/* CC-Folder geschlossen */
#define CCFM_CLOSE             (WM_USER+45)

/* Rexx-Folder geschlossen */
#define RXF_CLOSE              (WM_USER+46)

/* Rexx-Script-Settings geschlossen, mp1: ulScriptID */
#define RXSET_CLOSE            (WM_USER+47)

/* Rexx-Script-Name hat sich geaendert, mp1: ulScriptID */
#define RXSET_NEWNAME          (WM_USER+48)

/* Rexx-Menue updaten */
#define RXM_UPDATEMENU         (WM_USER+49)

/* Rexx-Monitor-Close */
#define REXXM_CLOSE            (WM_USER+51)

#define REXXM_STARTSCRIPT      (WM_USER+52)
#define REXXM_STOPSCRIPT       (WM_USER+53)
#define REXXM_OUTLINE          (WM_USER+54)
#define REXXM_ERROR            (WM_USER+55)
#define REXXM_CLS              (WM_USER+56)

/* NL-Browser */
#define BRSM_CLOSE             (WM_USER+57)
#define BRSM_INDEX_READY       (WM_USER+58)
#define BRSM_INDEX_ERROR       (WM_USER+59)
#define BRSM_DATA_READY        (WM_USER+60)
#define BRSM_DATA_ERROR        (WM_USER+61)

#define APM_REQCLOSE           (WM_USER+62)
#define APM_CANCEL             (WM_USER+63)

/* Work-Messages */
#define WORKM_DELETED          (WM_USER+101)
#define WORKM_MOVED            (WM_USER+102)
#define WORKM_COPIED           (WM_USER+103)
#define WORKM_PRINTED          (WM_USER+104)
#define WORKM_EXPORTED         (WM_USER+105)
#define WORKM_ADDED            (WM_USER+106)
#define WORKM_CHANGED          (WM_USER+107)
#define WORKM_READ             (WM_USER+108)
#define WORKM_END              (WM_USER+109)
#define WORKM_DISABLEVIEWS     (WM_USER+110)
#define WORKM_ENABLEVIEWS      (WM_USER+111)
#define WORKM_REREAD           (WM_USER+112)
#define WORKM_SHOWMSG          (WM_USER+113)
#define WORKM_AREADEFCHANGED   (WM_USER+114)
#define WORKM_AREASCANNED      (WM_USER+115)
#define WORKM_STARTWORKAREA    (WM_USER+116)
#define WORKM_AREACHANGE       (WM_USER+117)
#define WORKM_PROGRESS         (WM_USER+118)
#define WORKM_MARKEND          (WM_USER+119)
#define WORKM_STARTFIND        (WM_USER+120)
#define WORKM_FINDAREA         (WM_USER+121)
#define WORKM_FINDPROGRESS     (WM_USER+122)
#define WORKM_FINDAREAEND      (WM_USER+123)
#define WORKM_STOPFIND         (WM_USER+124)
#define WORKM_MSGMARKED        (WM_USER+125)
#define WORKM_MSGUNMARKED      (WM_USER+126)
#define WORKM_TRACKMSG         (WM_USER+127)
#define WORKM_SWITCHACCELS     (WM_USER+128)

#define ACCEL_NONE  0L
#define ACCEL_READ  1L
#define ACCEL_WRITE 2L

/* Work-Errors */
#define WORKM_ERROR            (WM_USER+100)

/* Replies auf WORK_ERROR */
#define WORK_ERROR_IGNORE       0
#define WORK_ERROR_RETRY        1
#define WORK_ERROR_ABORT        2

/* Message-Identifier, ist Parameter 1 bei Work-Messages */
typedef struct _MESSAGEID {
           char          pchAreaTag[LEN_AREATAG+1];
           unsigned long ulMsgID;
        } MESSAGEID, *PMESSAGEID;

#define MESSAGEIDFROMP(x) ((PMESSAGEID)x)

/* Ende MESSAGES.H */

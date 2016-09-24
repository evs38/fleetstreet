{ Struktur der BBToss Area- und Link-Daten v2.20}

{ Die Dateien (*.BBT) sind prinzipiell wie folgt aufgebaut: }

{   +--------------------+
    | DATEIHEADER-RECORD |  Header vom Typ signrec
    +--------------------+
    | Datensaetze ...    |
    |             ...    |
    +--------------------+
    | DATEIHEADER-RECORD |  Wiederholung des Header-Records vom Anfang
    +--------------------+
}

Const
  {Bei Aenderungen der Datenstrukturen aendert sich der DataRevisionLevel}
  DataRevisionLevel = $000E;   { 000A ab v2.10 *.BBT Dateien Data-Revisionlevel}
                               { 000B ab v2.10B04  Netmaildefaultzone}
                               { 000C ab v2.10B08  Tosser: timed/Golded/Squish.cfg hinzu
                                                   singnrectyp:record length added}
                               { 000D ab v2.20     Tosser: NetmailSentArea}
                               { 000E ab v2.30B03  Area: Lastmsgrcvd}
{Configfile-Namen}

  bbtlngbin      = 'bbtlng.bbt';   {compiliertes Sprachfile (bbtlng.cfg)}
  bbtossini      = 'bbtoss.ini';   {Grundeinstellungen, Pfade und Flags}
                                   {war bis v2.09 bbtoss.cfg}

  bbtlinkini     = 'bbtlink.ini';  {Linkdaten als Text/Inidatei}
  bbtareaini     = 'bbtarea.ini';  {Areadaten als Text/Inidatei}
  bbtlinkbin     = 'bbtlink.bbt';  {Configdatei der Linkdaten (binÑr)}
  bbtareabin     = 'bbtarea.bbt';  {Configdatei der Areadaten (binÑr)}

  bbtgroupini    = 'bbtgroup.ini'; {Gruppendaten als Text/Inidatei}

{  bbtakacfg      = 'bbtaka.cfg';   {List eingener Akas, * obsolete *}
  bbtzipini      = 'bbtzip.ini';   {Packereinstellungen}
  bbtaliascfg    = 'bbtalias.cfg'; {Aliasnamen}

{ bbtuplinkcfg   = 'bbtuplnk.cfg'; {Uplink-Einstellungen -> obsolete now bbuplnk.ini}
  bbtuplinkini   = 'bbtuplnk.ini'; {Uplink-Einstellungen umgestellt v2.10B05}

  bbtroutecfg    = 'bbtroute.cfg'; {Netmail Routing-Einstellungen}

  bbtcrcfile     = 'bbtdupe.bbt';  {CRC Daten fuer Dupe-Check}
  bbtsayerrfile  = 'bbsayerr.bbt'; {Textfile fuer Errormsgs BTSayerr}
  bbtlockfile    = 'bbtoss.bsy';   {wird bei Programm Ende gelîscht}
  bbtimportlog   = 'import.log';   {Echotoss.log der importierten Echomails,
                                    wird bei jedem Tosslauf neu erstellt,
                                    - Default, in bbtoss.ini ueberschreibbar}


{----- Declarationen fuer Links und Link-Record---------------------}
{LinkFlag Definitionen fuer Link-Record}
Link_activ        = $00000001; {link ist aktiv}
Link_checkpktpwd  = $00000002; {checkpktpwd}
Link_sayerror     = $00000004; {automatische Nachricht bei Fehlern}
Link_autocreate   = $00000008; {berechtigt zum anlegen neuer Areas}
Link_forwardmgr   = $00000010; {Areamgr Requests forwarded zum uplink}
Link_packnetmail  = $00000020; {netmail mit einpacken}
Link_packholdnet  = $00000040; {netmail mit Hold-Status einpacken}
Link_nodailyzip   = $00000080; {Arcmail auffÅllen, nicht tÑglich neue Endung}
Link_autoconnect  = $00000100; {link zu neuen Areas automatisch connecten}
Link_notify       = $00000200; {send Statusmsgs}
Link_fattachok    = $00000400; {link darf f'attach Msgs schicken}
Link_mgrpwdchange = $00000800; {link darf per Areamgr Passwort aendern}
Link_areainfo     = $00001000; {link bekommt info bei neuen Areas seiner Gruppe}
                               {     wenn area per autocreate eingerichtet}
Link_arcmail060   = $00002000; {Arc mail packets entsprechend arcmail 0.60 Standard}
Link_readonly     = $00004000; {readonly link}

{----- Declarationen fÅr Areas und Area-Record----------------------}
{Arc-Mail Status und AntwortmsgStatus Verwendung in Link-Record}
mailstatus_normal =  0;  {arcmail_status = linkrec.mailstatus and $0F}
mailstatus_hold   =  1;
mailstatus_direct =  2;
mailstatus_crash  =  4;

{mgrmsg_normal  =  0;  {mgrmail_staus = linkrec.mailstatus and $F0}

{mgrmsg_hold    =$10;
 mgrmsg_direct  =$20;
 mgrmsg_crash   =$40;}

{AreaFlags Definitionen}
Area_killseenby  = $00000001; {seen-by zeilen lîschen  }
Area_killpath    = $00000002; {path-zeile zurÅcksetzen }
Area_resetscanaka= $00000004; {beim scan immer AreaAka verwenden + Msgid update}
Area_chkaka      = $00000008; {prÅfe Absender-Aka jeder Msg}
Area_passive     = $00000010; {Area temporÑr passiv    }
Area_importseenby= $00000020; {import seenby in msgbase}
Area_importpath  = $00000040; {import path in MsgBase  }
Area_excludebbs  = $00000080; {nicht in Box exportieren}
Area_Pflichtarea = $00000100; {abmelden nicht mîglich  }
Area_Readonly    = $00000200; {ausser Uplink alle links nur lesender Zugriff}

type  SignStr = array[1..10] of char;  {Keyword in SignRec}

const AreaSign   : SignStr = 'BBTAREACFG';
      LinkSign   : SignStr = 'BBTLINKCFG';
      LngSign    : SignStr = 'BBTOSSLANG';
      DupeSign   : SignStr = 'BBTDUPEREC';

type  SignRec = record    {Vorspann in Bit-Cfg Files}
        Sign : SignStr;   {bbtarea.bbt & bbtlink.bbt usw.}
        Rev  : SmallWord; {SmallWord = 2 Byte Word (16Bit), Datarevision}
        Count: SmallWord; {Anzahl folgender DatensÑtze}
        end;

{String sind im Turbo Pascal Format, also LÑngenByte+x_Zeichen}
{SmallWord = 16 Bit Word ohne Vorzeichen}
String60 = String[60];
String79 = String[79];

{
aktuelle Definitionen, in anderer Unit declariert ....

Addrtype = record
   Zone,
   Net,
   Node,
   Point : SmallWord;
   end;
}

{verwendete Zeichen fÅr Packer -> LinkFileRec.Zipper}
{      Z = ZIP     }
{      A = ARC     }
{      L = LHA/LZH }
{      J = ARJ     }
{      S = SQZ     }
{      R = RAR     }
{      H = HYP     }
{      O = ZOO     }
{      U = UC2     }

{ Structur fuer BBTLINK.BBT}
LinkFileRec = Record
    SysOp      : String[40]; {Sysop-Name}
    NodeNr     : Addrtype;   {Nodenummer des Links}
    UseAka     : Addrtype;   {Absender Aka fuer PKT-Files}
    PktPwd     : String[9];  {Packet Password}
    MgrPwd     : String[15]; {Areamgr Password}
    Level      : SmallWord;  {Level des Links}
    Zipper     : Char;       {Packer fuer AcrMails Z=Zip J=ARJ A=ARC L=LHA...}
    Group      : String[15]; {Buchstabe der angeschlossenen Gruppen A..Z,a..z,
                             0..9, *=alle}
    Mailstatus : Byte;       {normal hold direct crash..siehe arcmail_xxx}
                             {Arcmail = Mailstatus and $F, Msg = mailstatus and $F0}
    Capability : SmallWord;  {Packetformat, Type 2 type 2+ usw. (aktuell not used)}
    LinkFlags  : Longint;    {Link-Flags, siehe Link_xxxx Konstanten}
    Flow       : Longint;    {Anzahl gesendeter Echomails (aktuell not used)}
    FlowKB     : Longint;    {Anzahl KB gesendeter Echomails (uncomprimiert)}
    Comment    : String[40]; {Kommentar}
    MaxPktSize : Longint;    {Maximale Groesse eines Packets *.PKT Files in Byte}
    MaxZipSize : Longint;    {Maximale Grosse eines Arcmail Archives in Byte}
    StatDate   : Longint;    {Date FlowCount started}
    LastPktRcvd: Longint;    {Timestamp wann letztes Packet von Link verarbeitet}

    end;

{
Aufbau Area-Datei BBTAREA.BBT
   signrec
      areafilerec
        array [1..areafilerec.storedlinks] of addrtype; -> variable LÑnge
      areafilerec
        array [1..areafilerec.storedlinks] of addrtype;
      ...
        ...
      ...
        ...
   signrec

{verwendete Zeichen fÅr Messagebasetype -> AreaFileRec.Basetyp}
{   E = EzyCom              }
{   H = Hudson              }
{   J = JAM                 }
{   L = HTML                }
{   N = Fido Netmail *.MSG  }
{   S = Squish              }
{   P = Passthrough         }

{verwendete Zeichen fÅr Area-Typ -> AreaFileRec.Typ}
{   B = Badmail Area                  }
{   D = Dupemail Area                 }
{   E = Echo Area                     }
{   L = lokales Area (also ohne Links)}
{   N = Netmail                       }
{   * = neu bestellt per Uplinkmgr    }

{ Structur BBTAREA.BBT}
AreaFileRec = Record
   TagName    : String60;  {Echo-Area Name}
   Nr         : SmallWord; {Nr. des Messagearea (in der Mailbox)}
   MPath      : String79;  {Msgbasename, bei Hudson '' (mu· mit \ enden)}
   BaseTyp    : char;      {Msg-Basetype  J=Jam H=Hudson S=Squish usw.}
   Group      : char;      {Gruppe des Areas  'A'..'Z','a'..z','0'..'9','*'}
   Typ        : char;      {E=Echo L=Lokal B=Bad D=Dupe N=Netmail}
   MainAka    : addrtype;  {eigene Aka fuer das Area}
   UpLink     : addrtype;  {Aka des Uplinks des Areas * not activ *}
                           {neu in Datarevisionlevel 8}
   AreaFlags  : LongInt;   {Area-Flags}
   WriteLevel : SmallWord; {Schreiblevel }
   ReadLevel  : SmallWord; {Lese/Bestell-Level}
   MaxMsgs    : SmallWord; {max Anzahl Msgs fuer Clean}
   MinMsgs    : SmallWord; {min Anzahl Msgs fuer Clean}
   DaysOld    : SmallWord; {Anzahl Tage fuer Clean}
   DaysRcvd   : SmallWord; {Anzahl Tage fuer Clean}
   Flow       : LongInt;   {Anzahl verarbeiteter Msgs seit StatDate}
   TodayFlow  : LongInt;   {Anzahl heute verarbeiteter Msgs}
   Desc       : String60;  {Beschreibung fuer Area}
   EOrigin    : String60;  {Origin Zeile}
   StatDate   : LongInt;   {Date Statistik started (Julian)}
   LastMsgRcvd: LongInt;   {Date last message received}
   RuleFile   : String79;  {Name der Arearule Datei}
   StoredLinks: SmallWord; {anzahl aktuell angeschlossener Links}
   { gefolgt von   Storedlinks * Size(Addrtype)  Bytes, pro angeschlossenem
     Link eine Aka (= 8 Byte)}
   end;

{ Structur BBTGROUP.BBT}
PGroupRec = ^GroupRec;
GroupRec = record
   Tag         : char;      {Gruppen Kennbuchstabe (fÅr externe Progs)}
   Desc        : String;    {Beschreibung}
   Uplink      : addrtype;  {Uplink-Aka}
   AreaBaseTyp : char;      {Areabasetyp fÅr autom. anlegen}
   Areapath    : String;    {Verzeichnis fÅr autom. erzeugte Areas}
   Startarea   : SmallWord; {freie Areanummer suchen ab...}
   RulePath    : String79;  {Verzeichnis der Rule-Text-Files}
   StatArea    : String60;  {Area-Tag fÅr Tagesstatistik}
   RuleListFile: String;    {path+name rule list file}
   RuleListFileDate : Longint; {File date of rulelistfile, last imported}
   RuleListFormat   : Byte  {1= areatag rulefile 2=rulefile areatag}
   end;

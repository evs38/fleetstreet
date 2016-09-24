:userdoc.
:docprof toc=1234.
:title.FleetStreet anv„ndarhandbok

.***************************************************************************
.* Einfhrung - inledning
.* 1997-10-22
.* @1@

:h1.Allm„nt
:p.
:lines align=center.
V„lkommen till

:font facename='Tms Rmn' size=24x18.:color fc=red.
FleetStreet 1.27.1
:font facename=default size=0x0.:color fc=default.
:elines.
:p.
FleetStreet „r en FTS-kompatibel meddelanderedigerare f”r OS/2 2.x PM.
FleetStreet kan bearbeta meddelandebaser i *.MSG-, Squish och JAM-format.
:elines.
:p.
:p.
Denna dokumentation beskriver konfigurationen av Fleetstreet och
grundl„ggande anv„ndning av programmet. Detaljerad information om
FleetStreets olika dialogrutor hittar Du i programmets onlinehj„lp. Denna
dokumentation och onlinehj„lpen kompletterar varandra.
:p.
Denna dokumentation f”ruts„tter att Du redan har grundl„ggande kunskaper i
Fidonet eller Fidokompatibla n„t.
:p.
Open Source version
:p.
™vers„ttning (C) Copyright 1995-1998 Peter Karlsson

.***************************************************************************
.* Features - funktioner
.* @2@
.*
:h2.Funktioner
:p.
Funktioner i FleetStreet&colon.
:ul compact.
:li.OS/2 2.x/3.x PM-program
:li.100% 32-bitars-kod
:li.Optimierad f”r Pentium-processorer
:li.Multitr†dad
:li.WPS-liknande anv„ndargr„nssnitt
:li.Anv„ndning via
:ul compact.
:li.menyer
:li.snabbtangenter
:li.knapprad (valbar position, tv† storlekar)
:eul.
:li.Enkelt anv„ndande genom utf”rlig onlinehj„lp samt hj„lptexter i
statusraden
:li.Standardfunktioner som
:ul compact.
:li.l„sa meddelanden
:li.skriva meddelanden
:li.ta bort meddelanden
:li.f”r„ndra meddelanden
:li.kopiera meddelanden
:li.flytta meddelanden
:li.vidares„nda meddelanden
:li.svara p† meddelanden
:li.exportera meddelanden
:li.importera ASCII-texter till meddelanden
:eul.
:li.St”der f”ljande meddelandebasformat&colon.
:ul compact.
:li.*.MSG
:li.Squish
:li.JAM
:eul.
:li.L„ser in konfigurationsfiler fr†n
:ul compact.
:li.Squish
:li.Fastecho
:li.HPT
:li.IMail
.*:li.GEcho
:li.LoraBBS
:li.FMail
:li.TerMail
:li.WMail
:li.NewsWave
:li.BBToss
:li.ShotgunBBS
:eul.
:li.Utskrift av meddelande via PM-skrivardrivrutinen
:li.Skapar en ECHOTOSS.LOG-fil
:li.Visar meddelanden i f„rg
:li.Korspostar ett nytt meddelande i flera m”ten
:li.Skapar filbeg„ran fr†n meddelanden
:li.Skriver till flera mottagare via s„ndlistor
:li.Snabbs„ndlista
:li.Kortnamnslista
:li.St”der broadcast-funktionerna hos Squish 1.1x
:li.St”der version-7-nodlistor
:li.Nodlistevisare
:li.Text kan fritt kopplas till funktionstangenterna
:li.Omdirigering av enhetsbokst„ver f”r anv„ndning via n„tverk
:li.Fritt definierbara meddelandemallar
:li.Komfortabel s”kfunktion, „ven ”ver flera m”ten
:li.S”kning efter personliga meddelanden
:li.Tr„dlista f”r att l„sa enligt meddelandetr„d
:li.mneslista
:li.Flera inst„llningar f”r varje m”te
:li.Omvandling av nationella tecken vid skrivning
:li.St”der CHRS-kludgen
:li.Manuell markering av meddelanden
:li.Fj„rrstyrning via named pipe
:li.Makroprogrammering via Rexxprogram
:li.Flerfaldiga drag-and-drop-funktioner
:li.Program och anv„ndarhandbok p†
:ul compact.
:li.tyska
:li.engelska
:li.italienska
:li.svenska
:eul.
:eul.
:p.
&dot.&dot.&dot. och detta var bara en grov ”versikt &colon.-)

.* @2@ **********************************************************************
:h2.Systemkrav
:p.
FleetStreet har dessa krav p† Din h†rd- och mjukvara&colon.
:p.
:hp2.H†rdvara&colon.:ehp2.
:ul compact.
:li.PC som k”r under OS/2 (386DX, 6 MB)
:li.ca. 1,5 MB h†rddiskuttrymme, meddelandebaser tillkommer
:li.VGA-grafikkort
:eul.
:p.
:hp2.Mjukvara&colon.:ehp2.
:ul compact.
:li.OS/2 2.x/3.x (2.0 har ej testats, men borde fungera)
:eul.
:p.
:hp2.Testade mjukvarumilj”er&colon.:ehp2.
:ul compact.
:li.OS/2 2.1 (diverse spr†k)
:li.OS/2 2.11 (2.1 med Service Pack)
:li.OS/2 2.99 (WARP II)
:li.OS/2 Warp 3.0
:li.OS/2 Warp Connect (Peer To Peer)
:li.OS/2 Warp 4.0
:li.ZipStream 1.03 (med meddelandebas i en komprimerad katalog)
:li.LAN Server 4.0 Entry
:eul.

.***************************************************************************
.* Design Goals - designm†l
.* @2@
.*

:h2 id=design.Designm†l
:p.Vid utvecklingen av FleetStreet hade vi flera m†l i bakhuvudet. H„r
nedan kan du l„sa lite om dem.
:parml.
:pt.:hp2.Varf”r en PM-redigerare?:ehp2.
:pd.Presentation Manager (tm) „r visserligen l†ngsammare „n textl„ge (s†
l„nge det startas som en fullsk„rmsapplikation), men har vissa andra
f”rdelar framf”r dessa. Urklipp „r fullt anv„ndbart (Urklipp i
textl„gesapplikationer „r ett stort problem) och anv„ndaren kan fritt
v„lja typsnitt och f„rger t„mligen enkelt. Dessutom „r man inte begr„nsad
till 80x25 (m.fl) - formatet. F”rutom detta till†ter PM oss att implementera
alla de dialogrutor som g”r FleetStreet l„tt att arbeta med.
:pt.:hp2.Integrerad setup:ehp2.
:pd.I andra redigeringsprogram m†ste Du arbeta med „ndl”sa
ASCII-konfigurationsfiler. Om Du redan „r van vid att
anv„nda spiralblocken och dialogrutorna i Workplace Shell kommer inte
konfigureringen av FleetStreet v†lla n†gra som helst problem. Dessutom
beh”ver Du inte starta om FleetStreet bara f”r att p† nytt l„sa in
konfigurationen.
:p.
Om Du inte vet hur en inst„llning fungerar, trycker du helt enkelt p† F1,
eller p† en av hj„lpknapparna, som finns tillg„ngliga ”verallt. Detta
besparar „ven den tr†kiga bl„ddringen av l†nga textdokumentationer.
:pt.:hp2.Support f”r Squish (tm):ehp2.
:pd.FleetStreet st”der tossern Squish p† tv† s„tt. Det f”rsta „r att det
anv„nder Squish' API (MSGAPI.DLL), och det andra „r att FleetStreet direkt kan
l„sa in Squish' konfigurationsfil (SQUISH.CFG).
:i2 refid=squish.MSGAPI.DLL
:pt.:hp2.Enkelt att anv„nda:ehp2.
:pd.De flesta PM-applikationer „r enkla att anv„nda. Om Du kan anv„nda WPS
kan Du „ven anv„nda FleetStreet, eftersom vi anv„nder spiralblocken,
popup-menyer, "containers", osv.
:pt.:hp2.Alla funktioner Du beh”ver:ehp2.
:pd.Vi har implementerat en hel del funktioner vi tyckt vara anv„ndbara fr†n
andra meddelanderedigerare, och d„rut”ver n†gra vi inte hittade n†gon
annanstans.
:pt.:hp2.Kraftfullt:ehp2.
:pd.FleetStreet „r ett 32-bitars-program som anv„nder multipla tr†dar och
CUA'91.
:eparml.


.* @2@ **********************************************************************
:h2.Copyright o.s.v.
:p.
Squish och MsgAPI „r varum„rken hos Lanius Corporation.
:p.
OS/2 och Workplace Shell „r varum„rken hos IBM.
:p.
JAM(mbp) - Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch,
Mats Wallin. ALL RIGHTS RESERVED.

.* @2@ **********************************************************************
:h2.Tack
:p.
F”ljande personer har speciellt bidragit till uppkomsten och
vidareutvecklingen av FleetStreet&colon.
:sl.
:li.:hp4.Harry Herrmannsd”rfer:ehp4. - Echomailhantering, alfa/beta-test,
programmering
:li.:hp4.Harald Kamm:ehp4. - Italiensk ”vers„ttning, italiens
registreringshantering
:li.:hp4.Jason Meaden:ehp4. - tidigare registreringshantering Australien
:li.:hp4.Helmut Renner:ehp4. - Registreringshantering och echo-link i
™sterrike
:li.:hp4.Marty Duplissey:ehp4. - tidigare registreringshantering USA
:li.:hp4.Siegfried Hentschel, Jens Holm, Richard Douglas,
Jose M. Guglieri:ehp4. - Registreringshantering.
:li.:hp4.Rasmus Foged Hansen:ehp4. - tidigare registreringshantering
:li.:hp4.Peter Karlsson:ehp4. - Svensk ”vers„ttning [man tackar &colon.-)]
:li.:hp4.Alla deltagare i den slutna betafasen, speciellt Michael Siebke,
Joachim Loehrl-Thiel, Hajo Kirchhoff, Robert Gl”ckner:ehp4. m.fl.
:li.:hp4.Thomas Jachmann:ehp4. - Betatest och diverse Rexxprogram
:li.:hp4.Torsten Grimme:ehp4. - Test av IMail-st”det
:li.:hp4.Dirk Brenken:ehp4. - Test av Fastecho-st”det
:li.:hp4.Carsten Ellwart:ehp4. - Buggfinnare
:li.:hp4.speciellt tack till alla registerade anv„ndare:ehp4., utan vilka
shareware inte kan ”verleva.
:esl.
:p.
Den svenske ”vers„ttaren ”nskar tacka:
:sl.
:li.:hp4.Bengt ™sterdahl:ehp4. - F”r betatestning av den f”rsta
”vers„ttningen, och f”r all uppmuntran.
:li.:hp4.Deltagarna i R20_OS2:ehp4. - F”r v„nliga svar p† mina fr†gor om
vettiga svenska termer
:esl.

.***************************************************************************
.* Installation - Installation
.* @1@
.*

:h1.Grundinst„llning
:p.
N„r Du startar FleetStreet f”r f”rsta g†ngen visas meddelandet att
INI-filen inte kunde hittas. Detta „r normalt i denna situation, och „r
ingen sk„l till oro. Klicka helt enkelt p† :hp2.OK:ehp2.-knappen.
FleetStreet ”ppnar d† automatiskt de allm„nna inst„llningarna, och Du kan
p†b”rja den grundl„ggande konfigureringen.
:p.
Den h„r beskrivna grundl„ggande konfigureringen r„cker gott f”r att
installera FleetStreet i en vanlig nod- eller pointkonfiguration, och f”r
att ta de f”rsta stegen med den. Konfigurationen kan senare n„r som helst
„ndras f”r att anpassas till Dina speciella ”nskem†l.
:p.
Om Du inte kommer vidare i konfiguration klickar Du helt enkelt p† en av de
:hp2.Hj„lp:ehp2.-knappar som finns p† varje sida i inst„llningsblocket.

.* @2@ **********************************************************************
:h2.Namn
:p.
N„r Du skriver nya meddelanden infogar FleetStreet automatiskt Ditt namn
som avs„ndare.
:p.
P† inst„llningsblockets f”rsta sida kan Du ange alla de namn som Du ”nskar
anv„nda tillsammans med FleetStreet. Det f”rsta namnet i listan „r ditt
standardnamn. Det kommer alltid f”rst att anges p† de st„llen Ditt namn
efterfr†gas. Du kan g”ra valfritt namn ur listan till ditt standardnamn
genom att Du markerar det och klickar p† :hp2.Standard:ehp2.-knappen.

.* @2@ **********************************************************************
:h2.Adresser
:p.
P† den andra sidan i inst„llningsblocket kan Du ange samtliga n„tadresser
som Du ”nskar anv„nda tillsammans med FleetStreet. Den f”rsta adressen i
listan „r „ven h„r din standardadress.

.* @2@ **********************************************************************
:h2.Tosserkonfiguration
:p.
Om Du redan har installerat en tosser som st”ds av FleetStreet kan Du helt
enkelt l†ta FleetStreet l„sa in dess konfigurationsfil. FleetStreet l„ser
d„rur samtliga definierade adresser, m”ten och kataloger, s† att Du inte
beh”ver ange dina meddelandem”ten ytterligare en g†ng, i FleetStreet.
:p.
FleetStreet kommer „ven i framtiden att l„sa in den angivna filen vid
uppstart. P† s† s„tt kommer m”tena i FleetStreet alltid att ”verensst„mma
med de m”ten du har definierade i din tosser.
:p.
F”ljande tossers st”ds&colon.
:table cols='10 20 30'.
:row.
:c.Tosser
:c.Versioner
:c.Fil som anv„nds
:row.
:row.
:c.Squish
:c.1.01, 1.10, 1.11
:c.SQUISH.CFG (eller namnet p† en ekvivalent fil)
:row.
:c.Fastecho
:c.1.30, 1.41, 1.45, 1.46
:c.FASTECHO.CFG
:row.
:c.HPT
:c.?
:c.?
:row.
:c.IMail
:c.1.60, 1.70, 1.75
.* , 1.85, 1.87
:c.IMAIL.CF
:row.
:c.GEcho
:c.1.10, 1.20
:c.SETUP.GE
:row.
:c.LoraBBS
:c.2.34, 2.35, 2.40, 2.42
:c.CONFIG.DAT
:row.
:c.FMail
:c.0.98, 1.02ff
:c.FMAIL.CFG
:row.
:c.TerMail
:c.3.0
:c.TM.CFG
:row.
:c.WMail
:c.3.0
:c.WMAIL.PRM
:row.
:c.NewsWave PM
:c.0.99
:c.NEWSWAVE.INI
:row.
:c.BBToss
:c.2.06
:c.BBTOSS.CFG
:row.
:c.BBToss
:c.2.40
:c.BBTOSS.INI
:row.
:c.ShotgunBBS
:c.1.36+
:c.SYSTEM.DAT
:etable.
:p.
Aktivera h„rn„st inst„llningen :hp2."l„s":ehp2. p† den fj„rde sidan i
inst„llningsblocket. D„refter v„ljer Du ut den ”nskade tossern. Klicka
d„refter p† knappen :hp2."S”k...":ehp2., och s”k upp konfigurationsfilen.
N„r Du har hittat den klickar Du p† :hp2.OK:ehp2..
:p.
FleetStreet har nu l„st in filen. D„rmed „r grundinst„llningen f„rdig.
St„ng inst„llningsblocket. FleetStreet ska nu ”ppna den f”rsta m”tet, och
visa det f”rsta meddelandet i huvudf”nstret.

.* @1@ **********************************************************************
:h1.Anv„ndning
:p.

.* @2@ **********************************************************************
:h2 id=mainwin.Huvudf”nstret
:p.
N„r Du l„ser meddelanden visas f”ljande delar i huvudf”nstret&colon.
:p.
:hp2.Titelraden:ehp2.
:artwork name='titlebar.bmp' align=center.
:p.
H„r visas&colon.
:ul compact.
:li.Programmnamn och -version
:li.Aktuellt m”te
:eul.
:p.
:hp2.Meddelandehuvud:ehp2.
:artwork name='header.bmp' align=center.
:p.
H„r visas&colon.
:ul compact.
:li.Namn och adress p† avs„ndaren
:li.Namn och adress p† mottagaren (adress endast i netmailareor)
:li.mnesrad
:li.Meddelandeattribut
:li.Datum och tid f”r skapandet av meddelandet
:li.Datum och tid f”r meddelandets ankomst
:eul.
:p.
:hp2.Knapprad:ehp2.
:p.
Knapparnas betydelse „r&colon.
:parml compact break=none tsize=6.
:pt.:artwork runin name='home.bmp'.
:pd.Tillbaka till den tidigare lastreadpekaren i m”tet
:pt.:artwork runin name='nextarea.bmp'.
:pd.Till n„sta m”te med nya meddelanden
:pt.:artwork runin name='prevmsg.bmp'.
:pd.Till f”reg†ende meddelande
:pt.:artwork runin name='nextmsg.bmp'.
:pd.Till n„sta meddelande
:pt.:artwork runin name='prevreply.bmp'.
:pd.Till det meddelande p† vilket det aktuella svarar
:pt.:artwork runin name='nextreply.bmp'.
:pd.Till svaret p† detta meddelande
:pt.:artwork runin name='firstmsg.bmp'.
:pd.Till f”rsta meddelandet
:pt.:artwork runin name='lastmsg.bmp'.
:pd.Till sista meddelandet
:pt.:artwork runin name='ok.bmp'.
:pd.Spara meddelandet
:pt.:artwork runin name='cancel.bmp'.
:pd.Avbryt (spara inte) meddelandet eller „ndringarna
:pt.:artwork runin name='newmsg.bmp'.
:pd.Skriv nytt meddelande
:pt.:artwork runin name='edit2.bmp'.
:pd.ndra det aktuella meddelandet
:pt.:artwork runin name='importfile.bmp'.
:pd.Importera en textfil till meddelandet
:pt.:artwork runin name='exportfile.bmp'.
:pd.Exportera meddelandet till en textfil
:pt.:artwork runin name='reply2.bmp'.
:pd.Svara p† det aktuella meddelandet
:pt.:artwork runin name='printmsg.bmp'.
:pd.Skriv ut det aktuella meddelandet
:pt.:artwork runin name='find.bmp'.
:pd.Aktivera s”kfunktionen
:pt.:artwork runin name='msglist.bmp'.
:pd.™ppna „mneslistan
:pt.:artwork runin name='msgtree.bmp'.
:pd.™ppna meddelandetr„det
:pt.:artwork runin name='area.bmp'.
:pd.™ppna m”teslistan
:pt.:artwork runin name='showkludges.bmp'.
:pd.™ppna f”nstret med "kludge"-raderna
:pt.:artwork runin name='delmsg.bmp'.
:pd.Ta bort det aktuella meddelandet
:pt.:artwork runin name='bookm.bmp'.
:pd.V„xla till bokm„rkesf”nstret
:pt.:artwork runin name='help.bmp'.
:pd.Allm„n hj„lp f”r FleetStreet
:pt.:artwork runin name='cut.bmp'.
:pd.Kopiera markering till Urklipp, och ta sedan bort den
:pt.:artwork runin name='copy.bmp'.
:pd.Kopiera markering till Urklipp
:pt.:artwork runin name='paste.bmp'.
:pd.Infoga inneh†llet i Urklipp till mark”rens nuvarande position
:pt.:artwork runin name='copymsg.bmp'.
:pd.Kopiera det aktuella meddelandet till annat m”te
:pt.:artwork runin name='movemsg.bmp'.
:pd.Flytta meddelandet till ett annat m”te
:pt.:artwork runin name='fwdmsg.bmp'.
:pd.Vidarebefordra det aktuella meddelandet till annat m”te
:pt.:artwork runin name='shell.bmp'.
:pd.Starta en ny kommandoprompt
:pt.:artwork runin name='script.bmp'.
:pd.Rexxprogram
:pt.:artwork runin name='browser.bmp'.
:pd.Visa inneh†llet i nodlistan/-listorna
:pt.:artwork runin name='request.bmp'.
:pd.Efterfr†ga filer fr†n det aktuella meddelandet
:pt.:artwork runin name='catchup.bmp'.
:pd.Markera alla meddelanden som "l„sta"
:eparml.
:p.
:hp2.Statusrad:ehp2.
:artwork name='statline.bmp' align=center.
:p.
H„r visas (fr†n v„nster till h”ger)&colon.
:ul compact.
:li.Hj„lptexter, meddelanden
:li.Markeringstecken (n„r meddelandet „r markerat)
:li.Mark”rposition (endast vid skrivning)
:li.Infognings-/”verskrivningsl„ge (endast vid skrivning)
:li.Nummer p† det aktuella meddelandet, antal meddelanden i m”tet
:li.Aktuell adress
:eul.

.* @2@ **********************************************************************
.* :h2.Lesen
.* :p.
.* ---> L„sa meddelanden


.* @2@ **********************************************************************
:h2 id=writemsg.Skriva meddelanden
:p.Det finns tv† s„tt att skapa nya meddelanden p†&colon.
:parml.
:pt.:hp2.Skriva nytt meddelande:ehp2.
:pd.N„r du vill skriva ett nytt meddelande klickar du p† knappen
:artwork name='help01.bmp' runin. eller trycker p† INS.
Meddelandef”nstret rensas, och din avs„ndaradress l„ggs in i motsvarande
avs„ndarf„lt. Du kan nu skriva in mottagarnamnet, dennes n„tadress (se „ven
:link reftype=hd refid=enteraddr."Skriva in FTN-adresser":elink.) och
„mnesraden. Efter att Du gjort detta skriver du in texten i
meddelandef”nstret.
:p.N„r Du „r klar med ditt meddelande klickar du p† knappen
:artwork name='help04.bmp' runin. eller trycker p† tangenterna Ctrl-S.
Meddelandet kommer d† att sparas i Din meddelandebas. Du kan under
inskrivningen klicka p† :artwork name='help05.bmp' runin. eller trycka p†
Esc-knappen f”r att avbryta inskrivningen utan att spara.
:pt.:hp2.Svara p† ett meddelande:ehp2.
:pd.F”r att svara p† ett mottaget meddelande klickar Du p† knappen
:artwork name='help03.bmp' runin. eller trycker p† Ctrl-R, eller Ctrl-N om
Du vill svara i ett annat m”te (ex.vis privata brev). Om Du anv„nder
knappradsknappen kan Du nu v„lja om Du vill svara till avs„ndaren eller
mottagaren av meddelandet. Du har ocks† m”jligheten att v„lja att svara p†
meddelandet i ett annat m”te.
:p.N„r Du „r klar med ditt meddelande klickar du p† knappen
:artwork name='help04.bmp' runin. eller trycker p† tangenterna Ctrl-S.
Meddelandet kommer d† att sparas i Din meddelandebas. Du kan under
inskrivningen klicka p† :artwork name='help05.bmp' runin. eller trycka p†
Esc-knappen f”r att avbryta inskrivningen utan att spara.
:eparml.

.* @2@ **********************************************************************
:h2 id=changemsg.ndra meddelanden
:p.Du kan „ndra meddelanden efter det att de har sparats. Klicka p† knappen
:artwork name='help06.bmp' runin. f”r att redigera om Ditt meddelande. Du
kan „ndra b†de meddelandetext och -huvud.
:p.F”r att „ndra p† attributen klickar Du p† :hp2.ndra:ehp2.-knappen. Ett
dialogf”nster kommer att ”ppnas, i vilket Du kan s„tta p† eller st„nga av
de olika meddelandeattributen.
:p.N„r Du f”rs”ker „ndra ett meddelande som redan har s„nts, eller som inte
„r lokalt f†r Du ett varningsmeddelande.
:p.Klicka p† knappen :artwork name='help04.bmp' runin. f”r att spara
„ndringarna. Om Du ”nskar avbryta utan att spara „ndringarna klickar Du p†
knappen :artwork name='help05.bmp' runin.&dot.

.* @2@ **********************************************************************
:h2.Ta bort meddelanden
:p.
Du kan ta bort ett meddelande p† f”ljande s„tt&colon.
:ul.
:li.Tryck p† :hp2.Delete:ehp2.-tangenten. Efter en kontrollfr†ga kommer
meddelandet att tas bort.
:li.Klicka p† raderingsknappen p† :link reftype=hd refid=mainwin.knappraden:elink..
:li.Dra det aktuella meddelandet till tuggen. B”rja med dragningen fr†n
meddelandef”nstret.
.br
:artwork align=center name='shredmsg.bmp'.
:eul.
:p.
Meddelanden kan „ven raderas fr†n :link reftype=hd refid=msglist.„mneslistan:elink.,
:link reftype=hd refid=threadlist.tr„dlistan:elink. eller
:link reftype=hd refid=bookmarks.bokm„rkeslistan:elink..

.* @2@ **********************************************************************
:h2.Skriva ut meddelanden
:p.Du kan skriva ut det aktuella meddelandet p† f”ljande s„tt&colon.
:ul.
:li.Tryck p† tangentkombinationen :hp2.Skift-PrintScreen:ehp2.
:li.Klicka p† utskriftsknappen p† :link reftype=hd refid=mainwin.knappraden:elink..
:li.Dra ett meddelande till skrivarobjektet. B”rja med dragningen ”ver
meddelandef”nstret.
:eul.
:p.
Meddelanden kan „ven skrivas ut fr†n :link reftype=hd refid=msglist.„mneslistan:elink.,
:link reftype=hd refid=threadlist.tr„dlistan:elink. eller
:link reftype=hd refid=bookmarks.bokm„rkeslistan:elink..

.* @2@ **********************************************************************
:h2.M”teslista
:p.
M”teslistan inneh†ller alla de m”ten, som „r tillg„ngliga f”r
FleetStreet. Du kan v„xla till ett av dessa m”ten, skapa och ta bort m”ten,
samt st„lla in m”tenas attribut.
:p.
Klicka p† ett av m”tena, eller tryck p† :hp2.Enter:ehp2., f”r att v„xla till
detta m”te.
:p.
Tryck p† :hp2.Avs”k:ehp2., f”r att starta en avs”kning av samtliga m”ten. Detta
kan vara n”dv„ndigt om tossern har "tossat" (packat upp) nya meddelanden
efter det m”tena senast avs”ktes. Avs”kningen sker i en separat tr†d,
vilket betyder att Du kan forts„tta att l„sa meddelanden under tiden
avs”kningen genomf”rs i bakgrunden.
:p.
Se „ven&colon.
:ul compact.
:li.:link reftype=hd refid=areacon.M”teslistans inneh†ll:elink.
:li.:link reftype=hd refid=areacrea.Skapa och radera m”ten:elink.
:li.:link reftype=hd refid=areaset.St„lla in m”tenas attribut:elink.
:li.:link reftype=hd refid=arealistset.Anpassa m”teslistan:elink.
:li.:link reftype=hd refid=areafolders.M”tesmappar:elink.
:eul.

.***************************************************************************
.* Area List settings - M”teslisteinst„llningar
.*
:h3 id=areacon.M”teslistans inneh†ll
:p.
F”r varje m”te i listan kommer
:ul compact.
:li.m”tesbeskrivningen,
:li.antal meddelanden i m”tet, samt
:li.antal :link reftype=fn refid=unrmsg.ol„sta meddelanden:elink. i m”tet
:eul.
:fn id=unrmsg.
:p.Egentligen „r inte det h„r det :hp2.verkliga:ehp2. antalet ol„sta
meddelanden, det „r helt enkelt antal meddelanden efter det som senast
l„stes.
:efn.
:p.
att visas.
:p.
F”r ej avs”kta m”ten visas "-" i st„llet f”r antal meddelanden.
:p.
M”tesbeskrivningen „r fr†n b”rjan samma som m”testagen. M”tesbeskrivningen
visas i huvudf”nstret och kan :link reftype=fn refid=desccha.„ndras:elink..

:fn id=desccha.
:p.Detta kan g”ras i inst„llningsblocket f”r m”tet, eller genom att Du
klickar p† m”tesnamnet med v„nstra musknappen samtidigt som Du h†ller nere
Alt-tangenten. Du kan d† direkt redigera m”tesbeskrivningen.
:efn.


:h3 id=areacrea.Skapa och radera m”ten
:p.
:hp2.Skapa ett nytt m”te:ehp2.
:p.
Du kan skapa ett nytt m”te genom att v„lja :hp2."Skapa ny":ehp2. i
till„gsmenyn. D† ”ppnas ett tomt inst„llningsspiralblock. Fyll i alla
n”dv„ndiga f„lt och st„ng spiralblocket. Den nya arean kommer d† att fogas
in i m”teslistan.
:p.
Alla m”ten som skapas i FleetStreet „r :hp2.lokala m”ten:ehp2..

:p.
:hp2.Ta bort ett m”te:ehp2.
:p.
Du kan endast ta bort de m”ten Du har skapat i FleetStreet. De som „r
definierade i tosserns konfigurationsfil m†ste tas bort d„rifr†n.
:p.
F”r att ta bort ett m”te ”ppnar Du :link reftype=fn refid=areacon.m”tets
till„gsmeny:elink. och v„ljer :hp2."Ta bort":ehp2.. M”tet tas d† bort ur
m”teslistan.

:fn id=areacon.
:p.Klicka med den h”gra musknappen p† m”tet.
:efn.
:p.
Observera&colon. Filerna f”r arean
.* (*.MSG f”r *.MSG-areor, *.SQ? f”r Squishareor och .*JH? f”r JAM-areor)
kommer :hp2.inte:ehp2. att tas bort fr†n h†rddisken.

:h3 id=areaset.St„lla in m”tenas attribut
:p.
Varje m”te har sin egen upps„ttning attribut. Dessa „r&colon.
:ul compact.
:li.M”tesbeskrivning
:li.M”testag
:li.Standardanv„ndarnamn
:li.Standardadress
:li.S”kv„g
:li.Areaformat
:li.M”testyp
:li.Standardmeddelandeattribut
:li.Ytterligare inst„llningar
:eul.
:p.
N„r det g„ller areror som „r definierade i tosserns konfigruationsfil, kan
Du inte „ndra m”testag, standardadress, s”kv„g, areaformat eller
net/echomail-val.
:p.
F”r att „ndra m”tesattributen ”ppnar Du m”tets till„gsmeny och v„ljer
:hp2."Inst„llningar":ehp2..

:h3 id=arealistset.Anpassa m”teslistan
:p.
Sj„lva m”teslistan har „ven den diverse attribut, vilka har inflytande p†
dess utseende. F”r att „ndra dessa ”ppnar du m”teslistans till„gsmeny och
v„ljer :hp2."Inst„llningar":ehp2..
:p.
H„r kan du v„lja&colon.
:ul compact.
:li.standardvisning
:li.sortering
:li.f„rgerna som anv„nds f”r de olika m”testyperna.
:eul.

:h3 id=areafolders.M”tesmappar
:p.
M”ten kan grupperas i mappar. En mapp „r ”ppen †t g†ngen. M”teslistan visar
bara m”tena i den aktuella mappen.
:p.
Det finns en :hp2.standardmapp:ehp2. som har n†gra speciella
egenskaper&colon.
:ul compact.
:li.Den „r h”gst i mapphierarkin. Alla andra mappar grupperas under denna.
:li.Den kan inte tas bort eller flyttas.
:li.M”ten som inte har flyttats till n†gon mapp (d.v.s. nya m”ten l„sta
fr†n tosserkonfigurationen) hamnar i standardmappen.
:eul.
:p.
M”tesmappar har flera inst„llningsm”jligheter&colon.
:ul compact.
:li.Ett namn. Du kan „ndra det genom att ALT-klicka p† det.
:li.En sorteringsordning f”r m”ten inom mappen.
:li.En inst„llning f”r automatisk avs”kning.
:eul.
:p.
Du kan skapa nya eller ta bort existerande mappar genom att anv„nda
till„ggsmenyn. Du kan flytta mappar genom att dra dem till en annan mapp.
:p.
M”ten kan flyttas till en mapp genom att dra det till mappen.

.***************************************************************************
.* Message liste - mneslista
.* @2@
:h2 id=msglist.mneslista
:p.
mneslista visar samtliga meddelanden i m”tet. Meddelandena kommer att
visas i samma ordning som de st†r i meddelandebasen.
:p.
Ditt egen namn kommer att visas i en avvikande f„rg. L„sta och ol„sta
meddelanden har sina meddelandenummer i olika f„rger. De exakta f„rgerna
kan st„llas in i „mneslistans inst„llningsf”nster.
:p.
Du kan v„lja flera meddelanden med musen eller med tangentbordet och
manipulera dessa. Dessa manipulationsm”jligheter finns&colon.
:ul compact.
:li.Ta bort
:li.Kopiera
:li.Flytta
:li.Skriv ut
:li.Exportera
:eul.
:p.
Om det uppst†r ett fel vid l„sning av ett meddelande fr†n meddelandebasen,
kommer samtliga f„lt att visa en :hp2."*":ehp2..
:p.
Du kan flytta spaltavskiljaren, f”r att st„lla in bredden p† spalterna.
:p.
:artwork align=center name='movesepa.bmp'.

.***************************************************************************
.* Threadlist - Tr„dlista
.* @2@
.*
:h2 id=threadlist.Tr„dlistan
:p.
H„r visas meddelandetr„det f”r det aktuella m”tet. L„sta och ol„sta
meddelanden visas i olika f„rger.
:p.
Tr„d „r meddelanden i ett m”te vilka h„nger samman p† s„tt, att ett
meddelande „r svar p† ett annat meddelande, eller att det sj„lv har svar.
Om ett meddelande i tr„dlistan har svar visas ett :artwork name='plus.bmp'
runin. framf”r det. Du kan klicka p† :artwork name='plus.bmp'
runin.-tecknet med musen eller trycka p† :hp2.+:ehp2.-tangenten. Svaren
kommer d† att visas i en tr„dstruktur. :artwork name='plus.bmp'
runin.-tecknet kommer d† att bytas ut mot ett :artwork name='minus.bmp'
runin.&dot. Om du trycker p† :hp2.mellanslagstangenten:ehp2. kommer hela
tr„dstrukturen att komprimeras (d.v.s. bara "roten" visas).
:p.
Se „ven&colon.
:ul compact.
:li.:link reftype=hd refid=thdisp.Visningstyper:elink.
:li.:link reftype=hd refid=thmani.Manipuleringsm”jligheter:elink.
:li.:link reftype=hd refid=thlink.REPLY-l„nkare:elink.
:li.:link reftype=hd refid=markmsg.Markera samtliga meddelanden:elink.
:eul.

:h3 id=thdisp.Visningstyper
:p.
Det finns tre olika visningstyper&colon. Alla tr„d, tr„d med ol„sta
meddelanden, och endast ol„sta meddelanden.
:parml.
:pt.:hp2.Alla tr„d:ehp2.
:pd.Alla tr„d i m”tet visas i sin fullst„ndiga form.

:pt.:hp2.Tr„d med ol„sta meddelande:ehp2.
:pd.Endast de tr„d som har minst ett ol„st meddelande visas. Dessa
tr„d visas i sin fullst„ndiga form.

:pt.:hp2.Endast ol„sta meddelanden:ehp2.
:pd.Endast de ol„sta meddelandena visas. Om flera ol„sta meddelanden skapar
ett helt meddelandetr„d, kommer de att visas som en s†dan. Om ett tr„d
avbryts av ett l„st meddelande kommer delarna att visas som tv† olika tr„d.
:eparml.
:p.
™nskad visningstyp v„ljs i tr„dlistans till„ggsmeny. Standardvisningen
st„ller du in i inst„llningarna f”r tr„dlistan.
:p.

:h3 id=thmani.Manipuleringsm”jligheter
:p.
Om Du manipulerar meddelanden i tr„dlistan manipulerar du alltid
:hp2.tr„d:ehp2. eller :hp2.delar av tr„d:ehp2.. Det betyder att inte bara
ett enda meddelande manipuleras, utan „ven alla p† det f”ljande svar.
:p.
"Syster"-tr„d, eller meddelanden f”re det markerade meddelandet kommer dock
:hp2.inte:ehp2. att manipuleras.
:p.
Endast ett tr„d kan vid varje given tidpunkt manipuleras. Detta „r en
begr„nsning hos OS/2.
:p.
Du kan
:ul compact.
:li.Ta bort tr„d
:li.Flytta tr„d till ett annat m”te
:li.Kopiera tr„d till ett annat m”te
:li.Exportera tr„d till en fil
:li.Skriva ut ett tr„d
:li.Markera ett tr„d som "l„st"
:li.Expandera ett tr„d
:eul.

:h3 id=thlink.REPLY-l„nkare
:p.
FleetStreet utv„rderar endast den l„nkinformation som st†r att finna i
meddelandebasen, och l„nkar allts† inte sj„lvmant. F”r l„nkningen m†ste Du
anv„nda ett externt program, vilket l„mpligen anropas direkt efter
tossning av meddelanden i ett kommandoprogram ("batchfil").
:p.
REPLY-l„nkare „r bl.a.&colon.
:ul compact.
:li.Squish
:li.SqmLink
:li.SqLink
:li.QQLink
:eul.
:p.
Squish 1.01 l„nkar meddelanden m.h.a „mnesraden. Detta har f”rdelen att
„ven meddelanden utan MSGID/REPLY-kludge l„nkas. En nackdel „r dock att om
„mnesraden „ndras s† kan det inte l„ngre l„nkas till det brev p† vilket det
svarar. Det g†r inte heller att p† tr„dstrukturen se vem det „r som svarar
p† vilket brev, eller om ett meddelandet har mer „n ett svar (svaren l„nkas
alltid linj„rt).
:p.
Squish 1.10, SqmLink, SqLink och QQLink l„nkar meddelandena m.h.a
MSGID/REPLY-kludgarna. D† kan verkligen ett meddelande kopplas samman med
det p† vilket det svarar, samt med de som svarar p† det. Tr„den „r ej
heller l„ngre linj„ra, utan mer tr„dartade. Squishmeddelandebasformatet
till†ter att ett meddelande maximalt har 10 svar l„nkade till det.
:p.
Nackdelen med denna metod „r att meddelanden inte kan kopplas samman om
svaret inte inneh†ller n†gon REPLY-kludge (t.ex om det skrivits med en
QWK-l„sare, om det kommit via n†gon typ av gateway, eller om REPLY-kludgen
inte riktigt h†ller sig till standarden).


:h3 id=markmsg.terh„mta
:p.Om du tidigare inte anv„nt FleetStreet f”r att l„sa en speciell
meddelandebas, kommer n„stan alla meddelanden att vara markerade som
"ol„sta". D„rf”r kommer alla meddelanden att visas i „mneslistan. Efter det
att du utf”rt denna funktion kommer endast de meddelanden som verkligen „r
ol„sta att visas, d.v.s. meddelanden som tossas in i m”tet av tossern
senare.
:p.Denna funktion „r vanligtvis bara beh”vlig efter det att Du har bytt
till FleetStreet och vill uppdatera de gamla meddelandem”tena till nyaste
l„get. FleetStreet kommer i framtiden att hantera "ol„st"-attributet helt
automatiskt.

.* @2@ **********************************************************************
:h2 id=bookmarks.Bokm„rken
:p.Bokm„rkesf”nstret kan inneh†lla tre typer av meddelanden&colon.
:ul compact.
:li.S”kresultat
:li.Resultat av s”kning av personliga brev
:li.Markerade meddelanden
:eul.
:p.
Anv„nd listans till„gsmeny f”r att v„xla mellan de tre visningstyperna.
:p.
Om du v„ljer "Spara inneh†ll" i till„gsmenyn kommer inneh†llet i listan att
sparas p† h†rddisken n„r Du avslutar FleetStreet. Den kommer att laddas in
igen n„r du startar FleetStreet p† nytt n„sta g†ng.
:p.
Klicka p† ett meddelande, eller tryck p† :hp2.Till meddelande:ehp2.-knappen
f”r att visa hela meddelandet.
:p.
Tryck p† knappen :hp2.Ta bort alla:ehp2. f”r att nollst„lla den aktuella
visningen, d.v.s. alla meddelanden i den aktuella visningen tas bort ur
listan.
:p.
Anv„nd till„gsmenyn hos ett meddelande f”r att
:ul compact.
:li.ta bort meddelandet
:li.exportera meddelandet
:li.skriva ut meddelandet
:li.flytta meddelandet till ett annat m”te
:li.kopiera meddelandet till ett annat m”te
:li.ta bort meddelandet ur listan
:eul.

.* @2@ **********************************************************************
:h2.S”kfunktion
:p.Med s”kfunktionen kan meddelanden i ett eller flera m”ten avs”kas efter
en best„mt teckenkombination.
:p.S”kdialogrutan ”ppnas bl.a via menyvalet "Meddelande/S”k". Ange sedan
den efters”kta texten, och alla ”nskade s”kval. Med knappen "Start" kommer
s”kningen att startas. S”kningen sker i bakgrunden. Varje g†ng ett m”te har
genoms”ks kommer s”kresultaten att l„ggas in i
:link reftype=hd refid=bookmarks.bokm„rkeslistan:elink. (i visningstypen
"S”kresultat").
:p.Med s”kdialogen startas „ven s”kningen efter personliga meddelanden.
S”kresultatet kommer „ven det att l„ggas in i bokm„rkeslistan (i
visningstypen "Personliga meddelanden").

.* @1@ **********************************************************************
:h1.Avancerade funktioner

.* @2@ **********************************************************************
:h2 id=enteraddr.Skriva in FTN-adresser
:p.N„r du skriver meddelanden i netmailm”ten m†ste Du ange mottagarens
n„tadress. Detta sker vanligen genom att Du matar in dennes kompletta 3D- resp.
4D-adress.
:p.FleetStreet l†ter Dig „ven ange ofullst„ndiga adresser. De delar som
saknas kommer automatiskt att ers„ttas med motsvarande delar ur Din egen
adress.
:p.F”ljande exempel visar hur denna adresskomplettering fungerar. Som
standardadress anv„nds 2&colon.2490/2520.17&colon.
:table cols='12 15 30'.
:row.
:c.Angivet
:c.Resultat
:c.Kommentar
:row.
:c.2&colon.2490/2520
:c.2&colon.2490/2520
:c.3D-adress matades in
:row.
:c.2520
:c.2&colon.2490/2520
:c.Nodnummer inmatat, n„t &amp. zon kopierades
:row.
:c.247/2099
:c.2&colon.247/2099
:c.N„t &amp. nod inmatades, zon kopierades
:row.
:c.1030.42
:c.2&colon.2490/1030.42
:c.Nod &amp. punkt angavs, zon &amp. n„t kopierades
:row.
:c..42
:c.2&colon.2490/2520.42
:c.Punkt angavs, samma bossnod
:etable.
:p.:hp2.De globala reglerna „r&colon.:ehp2.
:ol.
:li.Ett ensamt tal g„ller som "nod".
:li.Om inget punktnummer anges kommer "0" automatiskt att fyllas i.
:eol.

.* @2@ **********************************************************************
:h2.Knappraden
:p.Knappraden i FleetStreet kan visas i tv† olika storlekar p† fyra olika
platser. Dessa inst„llningar st„ller du in med hj„lp av knappradens
till„ggsmeny. ™ppna denna genom att klicka med musknapp 2 (vanligen den
h”gra) p† ett tomt utrymme inom knapprades omr†de.


.* @2@ **********************************************************************
:h2.M”teshanteraren
:p.M”teshanteraren „r till f”r f”renklad kommunikation med
m”teshanterarprogrammet hos Din uppl„nk.
:p.Vanligtvis best„ller man echomailm”ten genom att man skickar ett netmail
till uppl„nkens m”teshanteringsprogram. I detta inneh†ller m”tesraden
l”senordet, och meddelandetexten inneh†ller namnen p† de ”nskade m”tena.
:p.Man kan „ven p† ett liknande s„tt avbest„lla m”ten igen, och „ven beg„ra
en lista ”ver f”rfogbara m”ten, m.m.
:p.FleetStreets m”teshanterare f”renklar denna procedur&colon.
:ul compact.
:li.Man beh”ver inte ha namnet, adressen och l”senordet f”r m”teshanteraren tillg„nligt,
:li.Man kan p† ett enkelt s„tt via till„ggsmenyer best„lla resp. avbest„lla m”ten,
:li.Meddelanden till m”teshanteraren kommer att skapas automatiskt,
:li.De best„llda m”tena l„ggs in i tosserns konfigurationsfil.
:eul.

:h3.Lista ”ver m”ten
:p.F”r att m”teshanteraren ska fungera beh”ver den en lista ”ver de m”ten
som finns tillg„ngliga hos Din uppl„nk. Om denna saknas, m†ste den manuellt
beg„ras via ett meddelande till uppl„nken.
:p.Svaret fr†n m”teshanteraren inneh†ller en lista ”ver echomailm”ten.
Dessa m†ste nu ”verf”ras i FleetStreets m”teshanterare. F”r att g”ra det
v„ljer man menyvalet "Alternativ/H„mta m”testitlar". FleetStreet genoms”ker
nu det aktuella meddelandet efter m”tesnamn och anv„nder dessa i sin
m”teshanterare.
:p.FleetStreet sparar undan avs„ndaradressen f”r varje m”teslista. N„r
m”testitlarna h„mtas och det redan finns en lista f”r denna avs„ndare,
kommer den gamla listan att ers„ttas med den nya. Annars kommer avs„ndaren
att infogas som en ny uppl„nk.

:h3.Konfiguration
:p.F”r att kommunicera med m”teshanteraren hos uppl„nken kr„vs dess namn
och adress. ™ppna f”rst FleetStreets m”teshanterare, sedan listans
till„ggsmeny och d„r v„ljer Du "Inst„llningar". P† inst„llningsblockets
f”rsta sida finns en lista ”ver k„nda uppl„nkar. Du kan f”r var och en av
dessa infoga namn och l”senord f”r deras m”teshanterare.
:p.
:hp8.Observera&colon.:ehp8. Posterna f”r namn och l”senord kr„vs f”r att Du
senare ska kunna anv„nda FleetStreets m”teshanterare.

:h3.Anv„nda m”teshanteraren
:p.F”r att best„lla eller avbest„lla m”ten med hj„lp av FleetStreets
m”teshanterare g”r du som f”ljer&colon.
:ol.
:li.St„ll som aktuell adress in den adress med vilken Du vill skriva till
uppl„nken. Om Du har flera uppl„nkar resp. n„t, och har separata
netmailm”ten f”r varje uppl„nk v„xlar du till motsvarande netmailm”te.
:li.™ppna FleetStreets m”teshanterare via menyn. Nu ska uppl„nkes
m”teslista visas. Om listan „r tom m†ste Du f”rst manuellt beg„ra en lista
fr†n uppl„nken.
:li.™ppna till„ggsmenyn f”r det m”te Du vill best„lla, och v„lj
:hp2."best„ll":ehp2..
:li.Om Du vill best„lla eller avbest„lla fler m”ten g”r Du p† samma s„tt.
De ”nskade kommandona visas p† samma rad som m”testitlarna.
:li.Klicka p† :hp2.OK:ehp2. s† skapas det netmailbrev som ska skickas till
uppl„nken.
:eol.

:h3.Ut”knings-DLL
:p.Squish kan inte automatiskt l„gga in m”ten i sin konfigurationsfil.
D„rf”r kan FleetStreet ladda en DLL vid best„llning eller avbest„llning av
m”ten, och anropa funktioner fr†n den. DLL&colon.en kan sedan l„gga in
m”tena †t Dig.
:p.
Vilken DLL som ska anv„ndas kan st„llas in i m”teshanterarens
inst„llningsblock.
:p.
Filen :hp2.FLTCF_SQ.DLL:ehp2. som kommer med FleetStreet kan anv„ndas som
ut”knings-DLL n„r (och endast n„r) Du anv„nder Squish som tosser,
tillsammans med en standard-SQUISH.CFG. F”r andra konstellationer (ex.vis
pointpaket, andra tossers osv.) finns ev. motsvarande DLL&colon.er fr†n
tredjepartsleverant”rer till f”rfogande.
:p.
Teknisk information om hur du skapar en ut”knings-DLL kan f†s fr†n
:link reftype=hd refid=support.f”rfattaren:elink..


.* @2@ **********************************************************************
:h2 id=cclists.Anv„nda s„ndlistor
:p.Att anv„nda en s„ndlista inneb„r att samma meddelande skickas
till mer „n en mottagare (p† engelska kallas detta vanligen "carbon copy").
S„ndlistor kan endast anv„ndas i netmail.
:p.Ponera f”ljande exempel&colon.
:p.Du sammanst„ller ett nyhetsbrev, vilket Du regelbundet skickar till ett
antal personer. F”r att l”sa denna uppgift med FleetStreet kan kan Du skapa
en s„ndlista med namnet "Newsletter". N„r Du gjort detta skriver
Du in alla personer som ska ta emot nyhetsbrevet i listan.
:p.N„r Du nu vill skicka nyhetsbrevet via netmail sammanst„ller Du ditt
brev, p† precis samma s„tt som Du skulle gjort det vanligtvis. Men ist„llet
f”r att skriva in en mottagare v„ljer Du :hp2.Skicka med s„ndlista:ehp2.
fr†n menyn. Nu kan Du v„lja s„ndlistan "Newsletter" som Du skapade
tidigare. Texten "*** Newsletter ***" kommer att visas i st„llet f”r
mottagarnamnet, vilket visar att Du anv„nder denna s„ndlista.
:p.N„r n„sta utg†va av nyhetsbrevet kommer ut beh”ver Du inte skapa n†gon
ny s„ndlista, Du kan enkelt anv„nda den Du tillverkade tidigare.

:h3.F”rvalta s„ndlistor
:p.Alla s„ndlistor finns i s„ndlistef”nstret. Denna kan
”ppnas fr†n meny via :hp2."Alternativ/S„ndlistor":ehp2..
S„ndlistorna visas d„r som ikoner.
:p.Anv„nd listornas till„ggsmenyer f”r att ta bort eller skapa en ny lista.
Du kan ”ppna en lista, f”r att visa och redigera inneh†llet, genom att
dubbelklicka p† den.
:p.H†ll nere Alt-tangenten och klicka p† en s„ndlista f”r att
„ndra dess namn.

:h3.Importera
:p.V„lj den fil som Du ska importera. Filen m†ste vara en textfil, och
m†ste ha f”ljande format&colon.
:ul compact.
:li.Varje rad inneh†ller precis ett namn och en adress
:li.F„ltena p† en rad m†ste avdelas av minst ett mellanslag
:li.Tomrader ignoreras
:li.Rader med inledande semikolon ignoreras
:li.Rader med ogiltigt format ignoreras.
:eul.
:p.
Exempel&colon.
:xmp.
; Kommentar
Michael Hohner 2&colon.2490/2520.17
Hans Dampf 1&colon.234/567
Peter Karlsson 2&colon.204/145.42
:exmp.

:h3.Snabbs„ndlista
:p."Normala" s„ndlistor sparas undan, och st†r alltid till Ditt
f”rfogande. Detta „r dock inte alltid ”nskv„rt. D„rf”r finns funktionen
:hp2."Snabbs„ndlista":ehp2. i menyn :hp2."Special":ehp2..
:p.Snabbs„ndlistan fungerar i princip likadant som de normala
s„ndlistorna, f”rutom att den skapas f”rst vid skrivning av det
meddelande som ska s„ndas till flera mottagare. Dessutom kommer den inte
att sparas efter det att meddelandet har sparats.

.* @2@ **********************************************************************
:h2 id=crosspost.Korspostning av meddelanden
:p.Korspostning betyder, att samma meddelande sparas i mer „n ett m”te.
:p.Du kan aktivera korspostning genom att v„lja :hp2.Korsposta:ehp2. i
menyn. Denna funktion „r bara tillg„nglig d† Du skriver ett meddelande.
:p.M”teslistan kommer att ”ppnas, och Du kan d„rur v„lja m”ten i vilka Du
”nskar spara meddelandet.
:p.:hp8.Observera&colon.:ehp8. Du ska inte v„lja det aktuella m”tet i
listan. Meddelandet kommer alltid att sparas i det aktuella m”tet. V„lj
helt enkelt bara ”vriga m”ten.
:p.N„r korspostningsl„get „r aktivt kommer det att visas med en bock
framf”r :hp2.Korsposta:ehp2. i menyn.
:p.Du kan sl† av korspostningsl„get igen genom att Du v„ljer
:hp2.Korsposta:ehp2. fr†n menyn en g†ng till. Bocken kommer d† att
f”rsvinna, vilket betyder att Du „r i det vanliga redigeringsl„get igen.

.* @2@ **********************************************************************
:h2.Kortnamn
:p.I FleetStreet „r det m”jligt att definiera kortnamn f”r de anv„ndare
till vilka man skriver ofta.
:p.
F”r att anv„nda dessa kortnamn skriver du helt enkelt kortnamnet i
mottagarf„ltet och trycker p† "Enter". Kortnamnet kommer d† automatiskt att
ers„ttas med det riktiga namnet, och anv„ndarens adress kommer att matas in
i adressf„ltet. Om Du har angett en „mnesrad f”r kortnamnet, kommer denna
„ven att infogas i „mnesf„ltet.
:p.:hp2.F”r att definiera ett kortnamn g”r Du f”ljande&colon.:ehp2.
:ol compact.
:li.Klicka p† "Infoga"-knappen,
:li.Fyll i f„lten,
:li.Tryck p† "Ok".
:eol.
:p.:hp2.F”r att „ndra ett kortnamn g”r Du f”ljande&colon.:ehp2.
:ol compact.
:li.Markera kortnamnet i listan,
:li.Tryck p† "ndra"-knappen, eller dubbelklicka p† posten,
:li.G”r ”nskade „ndringar,
:li.Tryck p† "Ok".
:eol.
:p.:hp2.F”r att ta bort ett kortnamn g”r Du f”ljande&colon.:ehp2.
:ol compact.
:li.V„lj kortnamnet i listan,
:li.Tryck p† "Ta bort"-knappen.
:eol.

.* @2@ **********************************************************************
:h2 id=nodelists.Anv„nda nodlistor
:p.:hp2.Vad „r de bra f”r?:ehp2.
:p.Nodlistor inneh†ller bl.a namn och n„tadresser f”r samtliga deltagare i
ett n„t. FleetStreet kan s”ka upp adressen f”r ett namn i en s†dan
nodlista, och „ven g”ra det motsatta.
:p.F”r att g”ra detta beh”ver Du en kompilerad nodlista i formatet "Version
7". Detta format skapas bl.a av FastLst. FleetStreet beh”ver datafilen och
sysopindexet. Nodlistekompilatorn m†ste st„llas in p† motsvarande s„tt.
:p.:hp2.Hur hittar FleetStreet nodlistorna?:ehp2.
:p.I inst„llningarna finns en sida avsedd f”r nodlistor. F”r varje nodlista
m†ste Du skapa en "dom„n". En dom„npost inneh†ller dom„nens titel, och
s”kv„g till nodlistefilerna. Du m†ste ange komplett s”kv„g, inklusive
enhetsbokstav och filnamnstill„gg.
:p.:hp2.Vad beh”ver jag mer g”ra?:ehp2.
:p.Om Du vill kan Du „ven ange beteckningarna f”r de olika nodlisteflaggorna.
:p.:hp2.Hur kan jag anv„nda nodlistan?:ehp2.
:p.N„r Du skriver ett netmail anger du mottagarens hela namn, eller en del
av efternamnet, f”ljt av Enter. Om namnet hittas kommer adressen direkt att
l„ggas in i adressf„ltet. Om flera snarlika poster hittas visas ett f”nster
d„r Du kan v„lja vilken du vill anv„nda.
:p.I vissa dialogf”nster som kr„ver en inmatad adress finns det „ven en
"?"-knapp. Klicka p† den f”r att s”ka tillh”rande adress f”r det namn Du
redan skrivit in.

.* @2@ **********************************************************************
:h2 id=templates.Mall
:p.Meddelandemallar anv„nds f”r att st„lla in ett speciellt utseende p† nya
meddelanden, svar eller vidarebefordrade brev. N„r Du skriver ett nytt
meddelande, svarar eller vidarebefordrar kommer motsvarande delar av mallen
att kombineras med meddelandetexten. Speciella :link reftype=hd
refid=tokens.f”rkortningar:elink. i mallen kommer att ers„ttas med
best„mda delar av originalmeddelandet n„r mallen bearbetas.
:p.Meddelandemallen bearbetas n„r Du vid skapande av ett nytt meddelande,
svarande eller vidarebefordran f”r f”rsta g†ngen betr„der
redigeringsf”nstret. Om Du innan det angett ett „mne eller ett mottagarnamn
kommer dessa att anv„ndas som ers„ttning f”r f”rkortningarna. Om de „nnu
inte har definierats kommer f”rkortningarna att ers„ttas med tomrum.
:p.:hp2.Allm„nn ordningsf”ljd&colon.:ehp2.
.br
:hp2.Nytt meddelande&colon.:ehp2.
:xmp.
[Huvudtext]
[Meddelandetext]
[Fottext]
:exmp.
:p.:hp2.Svar&colon.:ehp2.
:xmp.
[Huvudtext]
[Svarstext]
[Meddelandetext]
[Fottext]
:exmp.
:p.:hp2.Svar i annat m”te&colon.:ehp2.
:xmp.
[Text f”r svar i annat m”te]
[Huvudtext]
[Svarstext]
[Meddelandetext]
[Fottext]
:exmp.
:p.:hp2.Korspostning&colon.:ehp2.
:xmp.
[Korspostningstext]
[Huvudtext]
[Meddelandetext]
[Fottext]
:exmp.
:p.:hp2.S„ndlistetext&colon.:ehp2.
:xmp.
[S„ndlistetext]
[Huvudtext]
[Meddelandetext]
[Fottext]
:exmp.
:p.:hp2.Vidarebefordran&colon.:ehp2.
:xmp.
[Text f”r vidarebefordran]
[Text fr†n det ursprungliga meddelandet]
[Fottext f”r vidarebefordran]
[Huvudtext]
[Fottext]
:exmp.
:p.:hp2.Tilldela mallar till m”ten:ehp2.
:p.Du kan dra en mallikon till m”teslistan och sl„ppa den p† den ”nskade
m”testiteln. Den tilldelade mallen kommer att anv„ndas n„r meddelanden
skapas i destinationsm”tet.
:p.
:artwork align=center name='atttpl.bmp'.


:h3 id=tokens.F”rkortningar
:p.F”ljande f”rkortningar finns till ditt f”rfogande i
meddelandemallarna&colon.
:parml break=none.
:pt.:hp2.%T:ehp2.
:pd.Mottagarens namn (originalbrev).
:pt.:hp2.%Z:ehp2.
:pd.Mottagarens f”rnamn (originalbrev).
:pt.:hp2.%R:ehp2.
:pd.Mottagarens adress (originalbrev). Vid svar eller vidarebefordran av
echomail ignoreras denna f”rkortning.
:pt.:hp2.%O:ehp2.
:pd.Mottagarens namn (nytt brev).
:pt.:hp2.%P:ehp2.
:pd.Mottagarens f”rnamn (nytt brev).
:pt.:hp2.%F:ehp2.
:pd.Avs„ndarens namn.
:pt.:hp2.%G:ehp2.
:pd.Avs„ndarens f”rnamn.
:pt.:hp2.%J:ehp2.
:pd.Avs„ndarens adress.
:pt.:hp2.%A:ehp2.
:pd.M”testag. Om ett meddelande korspostas „r detta destinationsm”tet. N„r Du
svarar i ett annat m”te, eller om meddelandet vidarebefordras, „r detta
ursprungsm”tet.
:pt.:hp2.%E:ehp2.
:pd.M”tesbeskrivning. Om ett meddelande kortspost „r detta destinationsm”tet.
N„r Du svarar i ett annat m”te, eller om meddelandet vidarebefordras, „r detta
ursprungsm”tet.
:pt.:hp2.%U:ehp2.
:pd.Ditt eget namn.
:pt.:hp2.%I:ehp2.
:pd.Ditt f”rnamn.
:pt.:hp2.%W:ehp2.
:pd.Din egen adress.
:pt.:hp2.%C:ehp2.
:pd.Namn p† personerna i s„ndlistan.
:pt.:hp2.%D:ehp2.
:pd.Datum f”r det meddelande p† vilket Du svarar.
:pt.:hp2.%M:ehp2.
:pd.Tid f”r det meddelande p† vilket Du svarar.
:pt.:hp2.%S:ehp2.
:pd.mne p† det meddelande p† vilket Du svarar.
:pt.:hp2.%%:ehp2.
:pd.Ett %-tecken
:eparml.

.* @2@ **********************************************************************
:h2.Omdirigering
:p.
Om meddelandebaserna befinner sig p† en annan dator i ett lokalt n„t „r den
”nskv„rt att bara ha en tosserkonfigurationsfil p† den datorn att redigera.
Om denna konfigurationsfil ska anv„ndas i FleetStreet, och
n„tverksenheterna „r kopplade till andra enhetsbokst„ver p† de olika
datorerna, kommer FleetStreet att anv„nda fel enhetsbokst„ver vid anrop
till meddelandebaserna.
:p.
Enhetsomdirigeringen ger en l”sning till problemet. Du kan omdirigera
n„tverkseneheterna till andra lokala enhetsbokst„ver. FleetStreet ers„tter
d† enhetsbokst„verna f”r n„tverksenheterna med den angivna bokstaven innan
meddelandebasen anropas.
:p.
Exempel&colon.
:p.
Meddelandebasen befinner sig p† dator A p† enhet D&colon.. FleetStreet k”rs
p† dator B, d„r dator As enhet D&colon. befinner sig p† enhet E&colon..
SQUISH.CFG befinner sig dock p† dator A, s† areafilnamnen „r angivna med
D&colon. som enhetsbokstav.
:p.
Om FleetStreet „r s† installerat att enhet D&colon. ers„tts med enhet
E&colon. kommer enhetsbokst„verna i areafilnamnen fr†n den SQUISH.CFG som
l„ses fr†n dator A att „ndras till E&colon., och korrekta filnamn kommer
att anv„ndas.
:p.
Observera&colon. Bara de filnamns enhetsbokst„ver som l„ses fr†n SQUISH.CFG
kommer att omdirigeras. Om du skapar m”ten i FleetStreet m†ste du sj„lv
ange korrekt enhet, FleetStreet omdirigerar inte dessa filnamn.

.* @2@ **********************************************************************
:h2 id=colorsetup.ndra f„rger och typsnitt
:i2 refid=setup.F„rger och typsnitt
:p.Du kommer inte att hitta n†gon meny eller dialogruta f”r att „ndra
f„rg och typsnitt f”r huvudf”nstret. Det beror p† att FleetStreet anv„nder
WPS-objekten f”r att st„lla in dessa saker.
:parml tsize=3.
:pt.:hp2.F„rgpaletten:ehp2.
:pd.™ppna f„rgpaletten och dra en f„rg till ”nskat element i
FleetStreetf”nstret, och l†t den 'falla ned' d„r du vill ha den. Om det „r
f”rgrundsf„rgen som ska „ndras h†ller du nera Ctrl-tangenten samtidigt.
:pt.:hp2.Fontpaletten:ehp2.
:pd.™ppna fontpaletten och dra ett typsnitt till ”nskat element i
FleetStreetf”nstret, och l†t den 'falla ned' d„r du vill ha den.
:eparml.

.* @2@ **********************************************************************
:h2.Importera, exportera
:p.
N„r Du skriver ett meddelande kan Du infoga en textfil vid den aktuella
mark”rpositionen.
:ul.
:li.Dra filen fr†n ett WPS-f”nster till redigerarf”nstret.
:artwork align=center name='dropfile.bmp'.
:li.Anv„nd importeringsfunktionen i "Arkiv"-menyn.
:eul.
:p.
Vid l„sning kan Du skriva meddelandet till en vanlig textfil.
:ul.
:li.Dra meddelandet till ett WPS-f”nster. B”rja dragningen ”ver
meddelandef”nstret.
:li.Anv„nd exporteringsfunktionen i "Arkiv"-menyn.
:eul.


.* @1@ **********************************************************************
:h1.Avancerade „mnen, tips
:p.
F”ljande „mnen kan m”jligen vara intressanta, n„r du har vant dig vid FleetStreet&colon.
:ul.
:li.:link reftype=hd refid=multinst.Flera instanser:elink.
:li.:link reftype=hd refid=perform.F”rb„ttra prestanda:elink.
:li.:link reftype=hd refid=multuser.Fleranv„ndarfunktion:elink.
:eul.

.***************************************************************************
.* Mehrere Instanzen - Flera instanser
.* @2@
.*

:h2 id=multinst.Flera instanser
:p.
Det „r m”jligt att starta FleetStreet vid mer „n en g†ng samtidigt. Om du g”r
det b”r du beakta f”ljande punkter&colon.
:ul.
:li.Endast den f”rst startade instansen sparar sina inst„llningar vid
avslut. D„rf”r b”r Du endast „ndra inst„llningarna i den f”rsta instansen, om
du ”nskar bibeh†lla dem.
:li.Sekund„ra instanser k„nnetecknas av en :hp2.[*]:ehp2. i sin titelrad.
:li.Det „r inte m”jligt att vid k”rning av flera instanser av FleetStreet
arbeta med olika versioner. Det „r heller inte m”jligt att k”ra en svensk och en
engelsk version samtidigt. D† kommer DLL&colon.erna fr†n den f”rsta instansen att anv„ndas
f”r alla de p†f”ljande. Detta „r en begr„nsning hos OS/2. Upptr„dandet hos en
sekund„r instans vid anv„ndandet av flera versioner „r ej definierat.
:li.Sekund„ra instanser har ingen pipe-server.
:eul.

.***************************************************************************
.* Performance - prestanda
.* @2@
.*

:h2 id=perform.F”rb„ttra prestanda
:p.FleetStreets prestanda beror till stor del p† tv† olika faktorer&colon.
Prestanda hos Squish' MSGAPI, och Dina h†rddiskars prestanda. Medan MSGAPI st†r
utanf”r v†rt inflytande, kan f”r„ndringar av den andra punkten f”rb„ttrea
FleetStreets sammantagna prestanda.
:p.N†gra kortfattade tips&colon.
:ul.
:li.Anv„nd om m”jligt meddelandebaser av Squish-typ i st„llet f”r *.MSG.
:li.Packa Squish-meddelandebaserna regelbundet med SqPackP. Efter en
s†dan packning ligger meddelandena i meddelandebasen utan luckor, och ordnade i
stigande nummerordning.
:li.Anv„nd HPFS.
:li.Exkludera m”ten i vilka du inte l„ser sj„lv fr†n m”teslistan.
Aktivera "G”m exkluderade m”ten".
:li.Om m”jligt n„r du s”ker efter text, s”k endast i meddelandehuvudet,
inte i huvud och text.
:eul.

.***************************************************************************
.* Multi-User - Fleranv„ndarfunktion
.* @2@
.*

:h2 id=multuser.Fleranv„ndarfunktion
:p.FleetStreet kan st„llas in f”r att nyttjas av flera anv„ndare, men med vissa
begr„nsningar. F”ljande punkter m†ste beaktas f”r att g”ra detta&colon.
:ol.
:li.Skapa en programkatalog f”r FleetStreet och kopiera EXE-filen och alla
DLL&colon.er till denna katalog.
:li.Skapa en separat konfigurationsfilskatalog f”r varje anv„ndare.
:li.Skapa ett programobjekt f”r varje anv„ndare. Ange den kompletta s”kv„gen
till FLTSTRT.EXE som programnamn.
:li.Skriv den :link reftype=hd refid=cmdlin.kommandoradsparameter:elink. som
anger konfigurationskatalogen f”r varje anv„ndare, t.ex.
"-Cd&colon.\fleet\user1".
:li.FleetStreet kan nu konfigureras separat f”r varje anv„ndare. Varje
konfigurationskatalog inneh†ller sin egen upps„ttning INI-filer.
:li.Anv„nd olika lastread-offsets f”r varje anv„ndare.
:eol.
:p.:hp2.Begr„nsningar&colon.:ehp2.
:ul.
:li.Det finns bara en "l„st"-flagga per meddelande, den „r allts†
samma f”r varje anv„ndare.
:li."Privat"-flaggan behandlas inte p† n†got speciellt s„tt.
:eul.

.***************************************************************************
.* Howto - Hur g”r man...
.* @2@
.*
:h2.Hur g”r man f”r att...
:p.
F”ljande avsnitt beskriver ofta anv„nda procedurer, och hur man realiserar
dem med FleetStreet.

:h3.skicka filer med ett meddelande?
:p.
Filer kan skickas tillsammans med ett meddelande. Namnen p† de filerna
ska st† p† meddelandets „renderad. Flera filnamn separeras med minst ett
mellanslag. Dessutom m†ste Du s„tta attributet "Fil bifogad" f”r att
tossern och mailern ska tolka „mnesraden p† korrekt s„tt.
:p.
Du kan dra filer fr†n valfritt WPS-f”nster till „renderaden f”r att bifoga
dem med meddelandet. Filnamnen kommer automatiskt att l„ggas till, "Fil
bifogad"-attributet s„ttas och en sammanfattning ”ver de bifogade filerna
kommer att visas. Detta fungerar endast vid skrivning av meddelanden!
:artwork align=center name='attfile.bmp'.

.***************************************************************************
.* Rexx
.* @1@
.*

:h1.Rexxprogram
:p.FleetStreet kan utf”ra program (scripts) i spr†ket Rexx, vilket
medf”ljer OS/2. Rexxspr†ket har ut”kats med :hp2.f”rdefinierade
variabler:ehp2. samt extra :hp2.funktioner:ehp2..


.* @2@ **********************************************************************
:h2.Programmeringsreferens
:p.Denna referens listar samtliga :link reftype=hd refid=rexxvar.f”rdefinierade
variabler:elink., och extra :link reftype=hd refid=rexxfunc.funktioner:elink..
:p.L„s onlinehj„lpen f”r Rexx' standardfunktioner.

:h3.FleetStreetmilj”n
:p.Rexxprogram som k”rs under FleetStreet k”rs inte i standardmilj”n
CMD.EXE. Rexxmilj”n f”r program under FleetStreet heter
:hp2.FLEETSTREET:ehp2..
:p.Detta betyder, att om du anv„nder ett icke-Rexx-kommando i ett
Rexxprogram kommer det att analyseras av FleetStreet. Om Du vill att
CMD.EXE ska utv„rdera kommandot m†ste du explicit anropa CMD.EXE-milj”n med
Rexxkommandot :hp2.ADDRESS:ehp2..
:p.Exempel&colon.
:xmp.
/* FELAKTIGT! */
'dir'

/* riktigt */
address CMD 'dir'

/* ocks† riktigt */
address CMD
'dir'
address FLEETSTREET
:exmp.
:p.L„s Rexx' onlinehj„lp f”r vidare information om Rexxmilj”er och
ADDRESS-kommandot.

.***************************************************************************
.* Variablen - Variabler
.*

:h3 id=rexxvar.F”rdefinierade variabler
:p.N„r ett Rexxprogram startas „r redan n†gra variabler initierade med
v„rden. Dessa variabler och deras tillh”rande v„rden kan anv„ndas i
programmet.
:p.:hp8.Observera&colon.:ehp8. Om Du i ett program „ndrar v„rdet p† en
f”rdefinierade variablerna kommer inte f”r„ndringen att †terspeglas i
FleetStreet, om inte du anv„nder en av FleetStreets funktioner eller
kommandon som g”r „ndringen verksam.
:p.De f”rdefinierade variablerna „r&colon.
:sl compact.
:li.:hp4.:link reftype=hd refid=rvar01.FleetSetup.Names:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar02.FleetSetup.Addresses:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar03.FleetSetup.Echotoss:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar04.FleetSetup.Tosser:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar05.FleetStatus.Area:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar06.FleetStatus.DestArea:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar07.FleetStatus.Name:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar08.FleetStatus.Address:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar09.FleetStatus.Mode:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar10.FleetStatus.Monitor:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar11.FleetStatus.Cursor:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar12.FleetMsg.Header:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar13.FleetMsg.Text:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar16.FleetMsg.Kludges:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar17.FleetMsg.Seenbys:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar14.FleetCCopy:elink.:ehp4.
:li.:hp4.:link reftype=hd refid=rvar15.NewMail:elink.:ehp4.
:esl.

:h4 id=rvar01.FleetSetup.Names
:p.Detta „r en vektor med anv„ndarnamn. :hp4.FleetSetup.Names.0:ehp4.
inneh†ller antal element i vektorn. :hp4.FleetSetup.Names.1:ehp4. osv.
inneh†ller namnen.
:p.Exempel&colon.
:table cols='20 10'.
:row.
:c.Variabel
:c.V„rde
:row.
:row.
:c.FleetSetup.Names.0
:c.2
:row.
:c.FleetSetup.Names.1
:c.Joe User
:row.
:c.FleetSetup.Names.2
:c.Joe
:etable.

:h4 id=rvar02.FleetSetup.Addresses
:p.Detta „r en vektor med anv„ndaradresser.
:hp4.FleetSetup.Addresses.0:ehp4. inneh†ller antal element i vektorn.
:hp4.FleetSetup.Addresses.1:ehp4. osv. inneh†ller adresserna.
:p.Exempel&colon.
:table cols='22 14'.
:row.
:c.Variabel
:c.V„rde
:row.
:row.
:c.FleetSetup.Addresses.0
:c.2
:row.
:c.FleetSetup.Addresses.1
:c.2&colon.2490/2520.17
:row.
:c.FleetSetup.Addresses.2
:c.21&colon.100/1016.17
:etable.

:h4 id=rvar03.FleetSetup.Echotoss
:p.:hp4.FleetSetup.Echotoss:ehp4. inneh†ller namnet p† filen Echotoss.Log.

:h4 id=rvar04.FleetSetup.Tosser
:p.:hp4.FleetSetup.Tosser:ehp4. inneh†ller namnet p† din tossers
konfigurationsfil.

:h4 id=rvar05.FleetStatus.Area
:p.:hp4.FleetStatus.Area:ehp4. „r en grupp variabler som inneh†ller
information om det aktuella m”tet.
:p.Variablerna „r&colon.
:parml.
:pt.:hp4.FleetStatus.Area.Tag:ehp4.
:pd.M”testag
:pt.:hp4.FleetStatus.Area.Desc:ehp4.
:pd.M”tesbeskrivning
:pt.:hp4.FleetStatus.Area.File:ehp4.
:pd.S”kv„g och/eller filnamn f”r arean
:pt.:hp4.FleetStatus.Area.Format:ehp4.
:pd.Areaformat. V„rdet kan vara n†got av "*.MSG", "Squish" eller "JAM"
:pt.:hp4.FleetStatus.Area.Type:ehp4.
:pd.M”testyp. V„rdet kan vara n†got av "Echo", "Net", "Local" eller "Private".
:eparml.

:h4 id=rvar06.FleetStatus.DestArea
:p.:hp4.FleetStatus.DestArea:ehp4. inneh†ller m”testag&colon.en f”r det m”te i
vilket svaret eller det vidarebefordrade kommer att sparas.
:p.Denna variabel „r endast definierad n„r ett meddelande redigeras!

:h4 id=rvar07.FleetStatus.Name
:p.:hp4.FleetStatus.Name:ehp4. inneh†ller det f”r tillf„llet aktiva
anv„ndarnamnet.

:h4 id=rvar08.FleetStatus.Address
:p.:hp4.FleetStatus.Address:ehp4. inneh†ller den f”r tillf„llet aktiva
anv„ndaradressen.

:h4 id=rvar09.FleetStatus.Mode
:p.:hp4.FleetStatus.Mode:ehp4. inneh†ller aktuell programstatus. Den kan vara
n†gon av f”ljande&colon.
:sl compact.
:li.No Setup
:li.Edit Single
:li.Edit XPost
:li.Edit CCopy
:li.Read
:li.Cleanup
:esl.
:p.N„r du redigerar ett meddelande „r variabelns f”rsta ord "Edit". Det
andra ordet visar om det „r ett enkelt meddelande, ett brev till flera
mottagare (via en s„ndlista, carbon copy), eller om korspostningsfunktionen
anv„nds.

:h4 id=rvar10.FleetStatus.Monitor
:p.:hp4.FleetStatus.Monitor:ehp4. „r "0" om inget ”vervakningsf”nster
anv„nds, och "1" om programmet utf”rs i ett s†dant.
:hp4.FleetStatus.Monitor:ehp4. kan anv„ndas direkt som ett booleiskt uttryck.

:h4 id=rvar11.FleetStatus.Cursor
:p.N„r du redigerar ett meddelande inneh†llre :hp4.FleetStatus.Cursor:ehp4.
tv† tal, vilka beskriver den aktuella mark”rpositionen. Det f”rsta talet „r
styckenumret, det andra talet „r teckenpositionen inom stycket.
Exempelvis betyder "46 3" att mark”ren „r p† det tredje tecknet i det
fyrtiosj„tte stycket.
:p.F”ljande program visar texten efter mark”ren&colon.
:xmp.
para = word(FleetStatus.Cursor, 1)
offs = word(FleetStatus.Cursor, 2)
say substr(FleetMsg.Text.para, offs)
:exmp.
:p.
Vid l„sning av meddelanden „r :hp4.FleetStatus.Cursor:ehp4. ej definierat.

:h4 id=rvar12.FleetMsg.Header
:p.:hp4.FleetMsg.Header:ehp4. „r en grupp variabler vilka inneh†ller
information om huvudet (headern) f”r det aktuella meddelandet.
:p.Variablerna „r&colon.
:parml.
:pt.:hp4.FleetMsg.Header.Attrib:ehp4.
:pd.Meddelandeattribut, previs som p† "Attrib"-raden.
:pt.:hp4.FleetMsg.Header.From:ehp4.
:pd.Avs„ndarens namn.
:pt.:hp4.FleetMsg.Header.FromAddress:ehp4.
:pd.Avs„ndarens adress.
:pt.:hp4.FleetMsg.Header.To:ehp4.
:pd.Mottagarens namn.
:pt.:hp4.FleetMsg.Header.ToAddress:ehp4.
:pd.Mottagarens adress. F”rs”k inte att anv„nda denna variabel i ett
echomailm”te!
:pt.:hp4.FleetMsg.Header.Subj:ehp4.
:pd.mnesraden.
:pt.:hp4.FleetMsg.Header.DateWritten:ehp4.
:pd.Datum och tidpunkt f”r meddelandets skapelse.
:pt.:hp4.FleetMsg.Header.DateReceived:ehp4.
:pd.Datum och tidpunkt f”r meddelandets tossning.
:eparml.

:h4 id=rvar13.FleetMsg.Text
:p.:hp4.FleetMsg.Text:ehp4. „r en vektor med textparafrafer.
:hp4.FleetMsg.Text.0:ehp4. inneh†ller antal element i vektorn.
:hp4.FleetMsg.Text.1:ehp4. osv. inneh†ller meddelandets text.
:p.:hp8.Observera&colon.:ehp8. Vektorns element „r :hp2.inte:ehp2.
textrader, utan stycken. Ett stycke i originaltexten avslutas med ett
radbrytningstecken. Om Du exempelvis genom infogning eller borttagning
„ndrar ord kan texten ombrytas korrekt igen. Om Du beh”ver ett eget format
med en best„mt radl„ngd m†ste Du programmera egna funktioner f”r
radombrytning. Detta borde vara ganska enkelt i Rexx.

:h4 id=rvar16.FleetMsg.Kludges
:p.:hp4.FleetMsg.Kludges:ehp4. „r en vektor vilken inneh†ller meddelandets
"kludgar". Vektorn „r endast definierad vid l„sning av meddelanden.
:p.
:hp4.FleetMsg.Kludges:ehp4.' f„lt „r som f”ljer &colon.
:parml.
:pt.:hp2.FleetMsg.Kludges.0:ehp2.
:pd.Antal element i vektorn
:pt.:hp2.FleetMsg.Kludges.1:ehp2.
:pd.F”rsta kludgeraden
:pt.:hp2.FleetMsg.Kludges.*:ehp2.
:pd.Alla vidare kludgerader
:eparml.
:p.
Kludgeraderna „r oftast p† formen
:xmp.
NAMN: V„rde
:exmp.
:p.
eller
:xmp.
NAMN V„rde
:exmp.
:p.
En kludgerad kan f”rekomma p† flera platser. Kludgarna har ingen speciell
ordning. Det inledande specialtecknet :hp2.01 hex:ehp2. medtas inte i
variabeln.

:h4 id=rvar17.FleetMsg.Seenbys
:p.:hp4.FleetMsg.Seenbys:ehp4. „r en vektor som inneh†ller meddelandets
SEEN-BY- och @PATH-rader. Vektorn „r endast definierad vid l„sning av
meddelanden.
:p.
:hp4.FleetMsg.Seenbys:ehp4.' f„lt „r som f”ljer&colon.
:parml.
:pt.:hp2.FleetMsg.Seenbys.0:ehp2.
:pd.Antal element i vektorn
:pt.:hp2.FleetMsg.Seenbys.1:ehp2.
:pd.F”rsta raden
:pt.:hp2.FleetMsg.Seenbys.*:ehp2.
:pd.Alla vidare rader
:eparml.
:p.
SEEN-BY-raderna „r p† formen
:xmp.
SEEN-BY&colon. noder
:exmp.

:h4 id=rvar14.FleetCCopy
:p.N„r du redigerar ett meddelande och anv„nder n†gon av
s„ndlistorna (carbon copy lists) (eller
snabbs„ndfunktionen) inneh†ller vektorn :hp4.FleetCCopy:ehp4.
namnen och adresserna till personerna i s„ndlistan. Om ingen
s„ndlista anv„nds „r dessa variabler inte definierade.
:p.
:hp4.FleetCCopy:ehp4.s f„lt „r som f”ljer&colon.
:parml.
:pt.:hp2.FleetCCopy.0:ehp2.
:pd.Antal poster i s„ndlistan.
:pt.:hp2.FleetCCopy.1.Name:ehp2.
:pd.Namnet p† f”rsta posten.
:pt.:hp2.FleetCCopy.1.Address:ehp2.
:pd.Adressen f”r den f”rsta posten.
:pt.:hp2.&dot.&dot.&dot.:ehp2.
:pd.
:eparml.

:h4 id=rvar15.NewMail
:p.:hp4.NewMail:ehp4. kan anta en kombination av f”ljande v„rden (avdelade
med mellanslag)&colon.
:parml break=none.
:pt.:hp2.'Echo':ehp2.
:pd.Nya echomailmeddelanden har skrivits in
:pt.:hp2.'Net':ehp2.
:pd.Nya netmailmeddelanden har skrivits in
:pt.:hp2.'Local':ehp2.
:pd.Nya lokala meddelanden har skrivits in
:eparml.
:p.
Om inga meddelanden har skrivits „r variabeln tom.
:p.
:hp8.Observera&colon.:ehp8. Dessa variabler har bara v„rden n„r programmet
h†ller p† att avslutas, d.v.s. n„r programmet anropats som "Exit-hook".
Annars „r de ej definierade.

.***************************************************************************
.* Funktionen - funktioner
.*

:h3 id=rexxfunc.Funktioner
:p.FleetStreet tillhandah†ller n†gra nya Rexxfunktioner. Dessa „r&colon.
:sl compact.
:li.:hp4.FSCls:ehp4.
:li.:hp4.FSLookupAddress:ehp4.
:li.:hp4.FSLookupName:ehp4.
:li.:hp4.FSSetEntryField:ehp4.
:li.:hp4.FSSetHeader:ehp4.
:li.:hp4.FSSetText:ehp4.
:esl.

:h4.FSCls
:p.:hp4.FSCls:ehp4. rensar ”vervakningsf”nstret.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSCls()
:exmp.
:p.:hp2.Parametrar&colon.:ehp2.
:p.
FSCls beh”ver inga parametrar.
:p.:hp2.Svarskoder&colon.:ehp2.
:p.:hp4.FSCls:ehp4. ger svaret "OK" om ”vervakningsf”nstret rensades. Om
programmet inte k”rs i ett ”vervakningsf”nstret ges svaret "NoMonitor".
:p.:hp2.Exempel&colon.:ehp2.
:xmp.
call FSCls
:exmp.

:h4.FSSetHeader
:p.:hp4.FSSetHeader:ehp4. f”rv„ntar sig en Rexx-vektor som parameter, och
anv„nder inneh†llet i dessa variabler som nytt meddelandehuvud.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetHeader(stem)
:exmp.
:p.:hp2.Parametrar&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.En Rexx-vektor som inneh†ller meddelandehuvudet. stem har f”ljande
f„lt&colon.
:ul compact.
:li.From
:li.FromAddress
:li.To
:li.ToAddress
:li.Subj
:eul.
:p.
Det „r samma f„lt som :hp4.FleetMsg.Header:ehp4. anv„nder, men det „r bara
ovan angivna f„lt som kommer att anv„ndas.
:eparml.
:p.
:hp2.Svarskoder&colon.:ehp2.
:p.:hp4.FSSetHeader:ehp4. ger svaret "OK".
:p.
:p.:hp2.Anm„rkningar&colon.:ehp2.
:ul.
:li.Alla element i vektorn m†ste ha v„rden, „ven om det bara „r
nullstr„ngar.
:li.S„tt alltid stamnamnet inom cituationstecken, s† att det inte
ers„tts med dess v„rde.
:li.Vid l„sning sparas inte den nya headern p† h†rddisken. Vid
redigering kommer den nya headern endast att sparas n„r hela meddelandet
sparas (ctrl-S).
:eul.
:p.
:p.:hp2.Exempel&colon.:ehp2.
:xmp.
/* Ers„tt avs„ndarnamnet */
FleetMsg.Header.From = 'Joe user'
RetVal = FSSetHeader('FleetMsg.Header')
:exmp.

:h4.FSSetText
:p.:hp4.FSSetText:ehp4. f”rv„ntar sig en Rexx-vektor som parameter, och
anv„nder dess text som ny meddelandetext. Texten i vektorn ers„tter den
tidigarevarande texten.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetText(stem)
:exmp.
:p.
:p.:hp2.Parametrar&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.En Rexx-vektor som inneh†ller meddelandetexten. stem.0 inneh†ller
antalet stycken, stem.1 ... stem.n inneh†ller styckena.
:eparml.
:p.
:p.:hp2.Svarskoder&colon.:ehp2.
:p.:hp4.FSSetText:ehp4. ger svaret "OK".
:p.
:p.:hp2.Anm„rkningar&colon.:ehp2.
:ul.
:li.Formatet p† vektorn „r detsamma som hos FleetMsg.Text.
:li.Element 0 i vektorn m†ste ha ett numeriskt v„rde.
:li.Alla element i vektorn m†ste ha ett v„rde, „ven om det „r
nullstr„ngar.
:li.S„tt alltid stamnamnet inom cituationstecken, s† att det inte
ers„tts med dess v„rde.
:li.Vid l„sning sparas inte den nya texten p† h†rddisken. Vid
redigering kommer den nya texten endast att sparas n„r hela meddelandet
sparas (ctrl-S).
:eul.
:p.
:p.:hp2.Exempel&colon.:ehp2.
:xmp.
NewText.0 = 2
NewText.1 = 'Detta „r bara ett'
NewText.2 = 'kort meddelande.'
RetVal = FSSetText('NewText')
:exmp.


:h4.FSLookupAddress
:p.:hp4.FSLookupAddress:ehp4. s”ker efter en FTN-adress i nodlistan. Svaret
sparas i en stamvariabel.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSLookupAddress(ftnaddress, stem)
:exmp.
:p.
:p.:hp2.Parametrar&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.ftnaddress:ehp2.
:pd.FTN-adressen p† den nod som ska efters”kas.
:pt.:hp2.stem:ehp2.
:pd.Stamnamn f”r den variabel i vilken resultatet ska sparas.
:p.Variabeln har f”ljande f„lt&colon.
:parml.
:pt.:hp4.Stem.Address:ehp4.
:pd.Nodadressen
:pt.:hp4.Stem.Name:ehp4.
:pd.SysOpens namn
:pt.:hp4.Stem.System:ehp4.
:pd.Systemets namn
:pt.:hp4.Stem.Phone:ehp4.
:pd.Telefonnummer
:pt.:hp4.Stem.Location:ehp4.
:pd.Systemets hemort
:pt.:hp4.Stem.Password:ehp4.
:pd.Sessionsl”senord. Om detta f„lt „r tomt har Du inte definierat n†got
l”senord f”r denna nod.
:pt.:hp4.Stem.Modem:ehp4.
:pd.Modemtyp. Detta „r ett numeriskt v„rde.
:pt.:hp4.Stem.Baud:ehp4.
:pd.H”gsta baudhastighet.
:pt.:hp4.Stem.UserCost:ehp4.
:pd.Kostnad f”r anv„ndaren att skicka ett meddelande till noden.
:pt.:hp4.Stem.CallCost:ehp4.
:pd.Kostnad f”r att ringa upp noden.
:pt.:hp4.Stem.Flags:ehp4.
:pd.Nodflaggor, en kombination av "ZC, "RC", "MO", "Hub", "Host" och "CM".
:eparml.
:eparml.
:p.
:p.:hp2.Svarskoder&colon.:ehp2.
:p.:hp4.FSLookupAddress:ehp4. ger som svar ett av f”ljande v„rden&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Noden hittades. Data f”r noden har sparats i den andra parametern.
:pt."NotFound"
:pd.Noden hittades inte.
:pt."Error"
:pd.Ett fel uppstod vid s”kningen.
:eparml.
:p.
:p.:hp2.Anm„rkningar&colon.:ehp2.
:ul.
:li.Om adresstr„ngen „r f”r l†ng kommer den att trunkeras. F„ltet
"Stem.Address" inneh†ller den resulterande adressen.
:li.S„tt alltid stamnamnet inom cituationstecken, s† att det inte
ers„tts med dess v„rde.
:eul.
:p.
:p.:hp2.Exempel&colon.:ehp2.
:xmp.
RetVal = FSLookupAddress('2&colon.2490/2520', 'NodeData')
say 'Systemnamn&colon.' NodeData.System
:exmp.


:h4.FSLookupName
:p.:hp4.FSLookupName:ehp4. s”ker efter ett SysOpnamn i nodlistan. Svaret
sparas i en stamvariabel.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSLookupName(name, stem)
:exmp.
:p.
:p.:hp2.Parametrar&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.name:ehp2.
:pd.Det s”kta SysOpnamnet.
:pt.:hp2.stem:ehp2.
:pd.Stamnamn f”r den variabel i vilken resultatet ska sparas.
:p.Variabeln har f”ljande f„lt&colon.
:parml.
:pt.:hp4.Stem.0:ehp4.
:pd.Antal funna poster.
:pt.:hp4.Stem.1.Address:ehp4.
:pd.Nodadressen f”r (f”rsta posten)
:pt.:hp4.Stem.1.Name:ehp4.
:pd.SysOpens namn f”r (f”rsta posten)
:pt.:hp4.Stem.1.System:ehp4.
:pd.Systemets namn f”r (f”rsta posten)
:pt.:hp4.Stem.1.Phone:ehp4.
:pd.Telefonnummer f”r (f”rsta posten)
:pt.:hp4.Stem.1.Location:ehp4.
:pd.Systemets hemort f”r (f”rsta posten)
:pt.:hp4.Stem.1.Password:ehp4.
:pd.Sessionsl”senord (f”rsta posten). Om detta f„lt „r tomt har Du inte
definierat n†got l”senord f”r denna nod.
:pt.:hp4.Stem.1.Modem:ehp4.
:pd.Modemtyp (f”rsta posten). Detta „r ett numeriskt v„rde.
:pt.:hp4.Stem.1.Baud:ehp4.
:pd.H”gsta baudhastighet (f”rsta posten)
f”r den f”rsta posten
:pt.:hp4.Stem.1.UserCost:ehp4.
:pd.Kostnad f”r anv„ndaren att skicka ett meddelande till noden (f”rsta
posten)
:pt.:hp4.Stem.1.CallCost:ehp4.
:pd.Kostnad f”r att ringa upp noden (f”rsta posten)
:pt.:hp4.Stem.1.Flags:ehp4.
:pd.Nodflaggor (f”rsta posten), en kombination av "ZC, "RC", "MO", "Hub", "Host" och "CM".
:eparml.
:eparml.
:p.Stem.2 osv. inneh†ller ”vriga funna poster.
:p.
:p.:hp2.Svarskoder&colon.:ehp2.
:p.:hp4.FSLookupname:ehp4. ger som svar ett av f”ljande v„rden&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Noden hittades. Data f”r noden har sparats i den andra parametern.
:pt."NotFound"
:pd.Noden hittades inte.
:pt."Error"
:pd.Ett fel uppstod vid s”kningen.
:eparml.
:p.
:p.:hp2.Anm„rkningar&colon.:ehp2.
:ul.
:li.Om namnet „r f”r l†ngt kommer det att trunkeras. F„ltet
"Stem.x.Name" inneh†ller det resulterande namnet.
:li.S„tt alltid stamnamnet inom cituationstecken, s† att det inte
ers„tts med dess v„rde.
:li.Det g†r „ven bra att s”ka p† en del av efternamnet.
:eul.
:p.
:p.:hp2.Exempel&colon.:ehp2.
:xmp.
RetVal = FSLookupName('Joe User', 'NodeData')
do i = 1 to NodeData.0
  say 'Adress&colon.' NodeData.i.Address
end
:exmp.

:h4.FSSetEntryField
:p.:hp4.FSSetEntryField:ehp4. s„tter in en text i ”vervakningsf”nstrets
inmatningsf„lt. Detta kan anv„ndas f”r att ange ett standardv„rde f”r
indata.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetEntryField(text)
:exmp.
:p.
:p.:hp2.Parametrar&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.text:ehp2.
:pd.Ny text f”r inmatningsf„ltet.
:eparml.
:p.:hp2.Svarskoder&colon.:ehp2.
:p.:hp4.FSSetEntryField:ehp4. ger som svar ett av f”ljande v„rden&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Texten har satts in.
:pt."NoMonitor"
:pd.Programmet k”rs utan ”vervakningsf”nster. D„rf”r kunde texten i
inmatningsf„ltet inte s„ttas.
:eparml.
:p.
:p.:hp2.Anm„rkningar&colon.:ehp2.
:ul.
:li.Texten trunkeras till maximalt 500 tecken.
:eul.
:p.
:p.:hp2.Exempel&colon.:ehp2.
:xmp.
RetVal = FSSetEntryField('C&colon.\')
if RetVal = 'OK' then
   do
   say 'Var v„nlig ange s”kv„g'
   parse pull mypath .
   end
:exmp.

.***************************************************************************
.* Hooks
.* @2@
.*

:h2.Hooks
:p.FleetStreet kan automatiskt starta specifika program i olika
situationer.
:p.
F”r att tilldela en hook ett speciellt program g”r du som f”ljer&colon.
:ol compact.
:li.™pnna Rexxprogramhanteraren
:li.V„lj "Inst„llningar" i f”nstrets till„ggsmeny
:li.V„xla till "Hooks"-sidan i inst„llningsblocket
:li.V„lj i "drop-down"-listan f”r den hook du ska definiera ”nskat program.
:eol.
:p.
:p.F”r n„rvarande finns f”ljande "hooks"&colon.
:ul compact.
:li.Programslut
:li.Innan sparande
:eul.

.***************************************************************************
.* Pipe-Server
.* @1@
.*

:h1.FleetStreets pipe-server
:p.
Detta avsnitt beskriver hur FleetStreets pipe-server fungerar.
:p.
FleetStreet startar automatiskt en tr†d som uteslutande betj„nar en "named
pipe". ™ver denna kan FleetStreet styras.
:p.
Namnet p† pipe&colon.n „r
:xmp.
\PIPE\FleetStreetDoor
:exmp.
:p.
Pipe&colon.n „r bidirektionell. Applikationer som vill kommunicera med
FleetStreet ”ver denna pipe m†ste anv„nda ett speciellt protokoll, vilket
beskrivs i f”ljande avsnitt. I de flesta fall r„cker dock FleetCom.
FleetCom „r en speciellt utvecklad FleetStreetklient som l„tt kan anv„ndas
i kommandoprogram, m.m.
:p.
Se „ven&colon.
:ul compact.
:li.:link reftype=hd refid=proto.Protokollet:elink.
:li.:link reftype=hd refid=commands.Kommandon f”r pipe-servern:elink.
:li.:link reftype=hd refid=fleetcom.FleetCom:elink.
:eul.

.* @2@ **********************************************************************
:h2 id=proto.Protokollet
:p.
En session med FleetStreet best†r av tre delar&colon. Uppr„ttande av
f”rbindelse, kommandoutbyte och avveckling av f”rbindelsen.
:p.
H„rn„st kommer de olika delarna att beskrivas. Till detta anv„nds f”ljande
ASCII-tecken&colon.
:table cols='6 12 8'.
:row.
:c.Symbol
:c.Hexadecimalt
:c.Decimalt
:row.
:row.
:c.ACK
:c.06
:c.6
:row.
:c.NAK
:c.15
:c.21
:row.
:c.ENQ
:c.05
:c.5
:row.
:c.ETX
:c.03
:c.3
:row.
:c.EOT
:c.04
:c.4
:etable.
:p.
:hp2.Uppr„ttande av f”rbindelse:ehp2.
:p.
Efter det att klienten har ”ppnat pipe&colon.n s„nder den f”rst en
:hp2.<ENQ>:ehp2. till FleetStreet. Som svar s„nder FleetStreet str„ngen
:hp2."FleetStreet"<ETX>:ehp2. till klienten. Denna m†ste nu kontrollera om
det verkligen „r FleetStreet som g”mmer sig bakom pipe&colon.n, d.v.s. om den
s„nda str„ngen ”verensst„mmer med "FleetStreet". Om detta „r fallet s„nder
klienten en :hp2.<ACK>:ehp2. till servern. Om det „r fel s„nder klienten en
:hp2.<NAK>:ehp2., p† vilket FleetStreet svarar med :hp2.<EOT>:ehp2. och
avslutar f”rbindelsen.
:p.
Om FleetStreet fr†n en :hp2.<ACK>:ehp2., s„nder det d„rp† sitt
versionsnummer som text, t.ex :hp2."0.88"<ETX>:ehp2.. Om klienten bara
fungerar tillsammans med en speciell version kan den kontrollera denna
str„ng. D† s„nder klienten antingen :hp2.<ACK>:ehp2. eller :hp2.<NAK>:ehp2.
till FleetStreet. Om str„ngen inte kontrolleras m†ste :hp2.<ACK>:ehp2.
s„ndas tillbaka.
:p.
Den f”rsta fasen „r nu avslutad, och FleetStreet v„ntar p† kommandon fr†n
klienten.
:p.
Schema ”ver uppr„ttningsfasen&colon.
:xmp.

 Klient              FleetStreet

            ENQ
   ---------------------->

     "FleetStreet<ETX>"
   <----------------------

            ACK
   ---------------------->

        "0.88<ETX>"
   <----------------------

            ACK
   ---------------------->
:exmp.
:p.
Vid fel&colon.
:xmp.

            ENQ
   ---------------------->

     "FleetStreet<ETX>"
   <----------------------

            NAK
   ---------------------->

            EOT
   <----------------------
:exmp.


.* @2@ **********************************************************************
:hp2.Kommandoutbyte:ehp2.
:p.
Klienten s„nder kommandon som enkla ASCII-texter till FleetStreet. Varje
kommando avslutas med :hp2.<ETX>:ehp2..
:p.
Exempel&colon.
:xmp.
"SCAN *"<ETX>
:exmp.
:p.
Parametrar avdelas fr†n kommandot och sinsemellan med ett mellanslag.
:p.
FleetStreet kontrollerar sedan om kommandot „r giltigt. Om ett fel uppst†r
skickas sekvensen
:xmp.
<NAK><kod><ETX>
:exmp.
:p.
tillbaka. <Kod> „r en av de tre bokst„verna C, P och S. Betydelsen av de
olika bokst„verna „r&colon.
:parml break=none.
:pt.C
:pd.Kommandot k„ndes inte igen
:pt.P
:pd.Parametrar saknas eller „r felaktiga (f”r kommandot)
:pt.S
:pd.Fler parametrar angavs „n vad som f”rv„ntades.
:eparml.
:p.
Om allting fungerar s„nds sekvensen
:xmp.
<ACK><ETX>
:exmp.
:p.
I s† fall b”rjar FleetStreet med att utf”ra kommandot. Efter det att
kommandot utf”rts meddelas resultatet. Om det under utf”randet uppst†r ett
fel kommer sekvensen
:xmp.
<NAK>"felmeddelande"<ETX>
:exmp.
:p.
att s„ndas. "Felmeddelande" „r en beskrivning av felet i klartext. Vid
felfri bearbetning kommer sekvensen
:xmp.
<ACK>"resultat"<ETX>
:exmp.
:p.
att s„ndas. "Resultat" „r d† den efterfr†gade informationen, ett
statusmeddelande, eller kan eventuellt vara tom.
:p.
Efter det att ett kommando f„rdigbehandlats kan ytterligare kommandon
skickas till FleetStreet. Detta kan ske ett valfritt antal g†nger fram till
avveckling av f”rbindelsen.
:p.
Schematiskt&colon.
:xmp.
 Klient              FleetStreet

       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

      [Bearbetning]

     <ACK>"34 areas"<ETX>
   <----------------------
:exmp.
:p.
Felaktigt kommando&colon.
:xmp.
       "ABC XYZ"<ETX>
   ---------------------->

        <NAK>"C"<ETX>
   <----------------------
:exmp.
:p.
Fel vid utf”randet&colon.
:xmp.
       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

       [Bearbetning]

    <NAK>"disk error"<ETX>
   <----------------------
:exmp.
:p.
:hp2.Avveckling av f”rbindelsen:ehp2.
:p.
N„r det klientens sista kommando har avklarats m†ste f”rbindelsen
avvecklas. F”r att g”ra detta s„nder klienten en
:xmp.
EOT
:exmp.
:p.
till servern. Denna s„nder d† tillbaka en
:xmp.
EOT
:exmp.
:p.
och b†da avbryter f”rbindelsen.
:p.
Schematiskt&colon.
:xmp.
 Klient              FleetStreet

            EOT
   ---------------------->

            EOT
   <----------------------
:exmp.


.* @2@ **********************************************************************
:h2 id=commands.Kommandon f”r pipe-servern
:p.
F”r n„rvarande „r f”ljande kommandon implementerade&colon.
:lines.
   SCAN
   ETOSS
.*   LOCK
.*   UNLCK
:elines.

:h3.SCAN
:p.
:hp2.SCAN:ehp2.
:p.
De angivna m”tena kommer att p† nytt avs”kas. Som m”te kan du ange&colon.
:parml break=none.
:pt.*
:pd.Alla m”ten
:pt.M”testag
:pd.Bara de angivna m”tena, t.ex TUB
:pt.@filnamn
:pd.Alla m”ten som anges i filen. Filens format „r detsamma som vid en
Echotoss-fil, d.v.s. en m”testag per rad
:eparml.

:h3.ETOSS
:p.
:hp2.ETOSS:ehp2.
:p.
Skriver Echotossfilen. Om ingen parameter anges kommer den fil som angivits
i FleetStreet-inst„llningen att skrivas. En parameter kan anges, och
betecknar d† filnamnet hos den fil som ska skrivas.


:h3 hide.LOCK
:p.
:hp2.LOCK:ehp2.
:p.
De angivna m”tena sp„rras, d.v.s. de kommer inte l„ngra att anv„ndas av
FleetStreet. Om m”tet f”r tillf„llet „r i anv„ndning, kommer kommandot att
vara i anv„ndning tills dess m”tet inte l„ngre anv„nds.
:p.
M”ten kan anges p† samma format som vid SCAN

:h3 hide.UNLCK
:p.
:hp2.UNLCK:ehp2.
:p.
De sp„rrade m”tena kommer att "l†sas upp". M”ten anges p† samma format som
vid LOCK.


.* @2@ **********************************************************************
:h2 id=fleetcom.FleetCom
:p.
Ett speciellt verktyg vid namn FleetCom skickas med FleetStreet. FleetCom
„r en klient f”r FleetStreets pipe-server. Anropskonventionen „r&colon.
:xmp.
FLEETCOM kommando [parameter]
:exmp.
:p.
FleetCom uppr„ttar en f”rbindelse med pipe-servern, s„nder kommandot och
dess parametrar, och skriver ut mottagna data. FleetCom r„cker i de flesta
fall f”r att anv„nda FleetStreets pipe-server. FleetCom s„nder endast ett
kommando per session.
:p.
De felniv†er (errorlevels) som FleetCom kan l„mna „r&colon.
:parml break=none tsize=4.
:pt.0
:pd.Allt OK, inget fel.
:pt.1
:pd.Pipe&colon.n kunde inte ”ppnas. Detta fel upptr„der „ven om FleetStreet inte
„r startad vid anropet. Allts† kan Du oftast strunta i detta fel.
:pt.2
:pd.Inget kommando har angetts.
:pt.3
:pd.FleetStreet k„nner inte igen kommandot.
:pt.4
:pd.Fel vid utf”randet av kommandot, eller systemfel.
:eparml.
:p.
FleetCom anv„nder som standard pipe&colon.n \PIPE\FleetStreetDoor. Den kan dock
„ven anv„nda ett annat namn. Programmet kontrollerar milj”variabeln
FLEETPIPE. Exempel&colon.
:xmp.
SET FLEETPIPE=\PIPE\AnotherPipe
:exmp.
:p.
Om FleetCom †ter ska anv„nda standardnamnet m†ste variabeln rensas med
:xmp.
SET FLEETPIPE=
:exmp.
:p.
:hp2.Exempel p† anv„ndning av FleetCom&colon.:ehp2.
:p.
Avs”k alla m”ten i vilka tossern har tossat nya meddelanden&colon.
:xmp.
SquishP IN -f echotoss.log
FleetCom scan @echotoss.log
:exmp.
:p.
Skriv Echotoss.Log, s† att tossern „ven kan exportera meddelanden fr†n
dessa m”ten&colon.
:xmp.
FleetCom etoss pack.log
SquishP OUT SQUASH -f pack.log
:exmp.
:p.

.* @1@ **********************************************************************
:h1.Tankar kring meddelandebasformaten
:p.
Var och ett av de tre av FleetStreet st”dda meddelandebasformaten har sina
f”r- och nackdelar. Det h„r kapitlet beskriver hur FleetStreet hanterar de
olika formaten, och fungerar som en guide f”r att v„lja det format som
passar dig.

.* @2@ **********************************************************************
:h2.*.MSG
:p.
FleetStreet anv„nder Squish' MSGAPI32.DLL f”r hantering av *.MSG-areor.
Tyv„rr „r det s† att detta API maximalt l„ser 512 bytes av kludge-raderna i
varje meddelande. Om det finns mer kludgar i ett meddelandet (vilket inte „r
ovanligt n„r det exempelvis g„ller meddelanden fr†n Internet) kommer de helt
enkelt att huggas av efter 512 bytes. Resten av kludgarna f†s som vanlig
meddelandetext.
:p.
FleetStreet kommer i dessa fall att flytta de ”verblivna kludgeraderna fr†n
meddelandet. En avhuggen kludgrad kommer dock inte l„ngre att kunna k„nnas
igen som en dylik. D„rf”r kan det f”rekomma att det ibland hamnar en halv
kludgrerad i b”rjan av meddelandetexten.
:p.
I *.MSG-formatet finns bara att spara Fido-standardattributen. Alla ”vriga
attribut sparas av FleetStreet som en FLAGS-kludge. Dessa kommer dock inte
att k„nnas igen av Squish (tossern). Attribut som "Direkt" eller "Arkivera
efter s„ndning" kommer d„rf”r att vara verkningsl”sa n„r de anv„nds tillsammans
med Squish.
:p.
F”r att markera "L„st"-attributet anv„nder FleetStreet l„sr„knaren i
*.MSG-filen. Denna kommer dock att vid packning nollst„llas av Squish, s† d†
kommer meddelandet att „n en g†ng markeras som "ol„st".

.* @2@ **********************************************************************
:h2.Squish
:p.
MSGAPI32.DLL (vilket anv„nds f”r hantering av Squishareor) har ett stort
designfel&colon. N„r en area ”ppnas kommer areaindexet (*.SQI) att l„sas
in i minnet. N„r arean „ndras kommer dock inte indexet p† disken att direkt
s„ttas att inneh†lla den aktuella informationen. I st„llet kommer „ndringen
endast att genomf”ras i minnet, och kommer inte f”rr„n det att arean st„ngs
att skrivas tillbaka p† disken. Ponera nu att tv† program samtidigt arbetar
p† arean, och s† skriver de oberoende av varnadra „ndringar till index.
Detta kommer att leda till ett defekt areaindex, eller i v„rsta fall till
att arean f”rst”rs.
:p.
Precis som i *.MSG-areor finns i Squishareor endast standardattributen att
tillg†. De andra attributen kommer „ven h„r att skrivas i en FLAGS-kludge
av FleetStreet. Squish (tossern) beaktar :hp5.inte:ehp5. denna kludgerad.
:p.
Som "L„st"-attribut anv„nder FleetStreet den h”gsta biten i meddelandets
attributf„lt.
:p.
"L†s"-attribuet hanteras inte av :hp2.SqPack:ehp2.. Anv„nd :hp2.FESQ:ehp2.
f”r att packa dina Squish-areor i st„llet.


.* @2@ **********************************************************************
:h2.JAM
:p.
I *.JHR-filen f”r en JAM-area markeras antalet aktiva meddelanden i arean.
Detta f„lt skrivs dock inte korrekt av olika programvaror (IMail 175 s„tter
det h„r f„ltet till 0 vid packning av arean; GoldEd 2.50.Beta6 kan ibland
vid borttagning av meddelanden generera ett underspill i detta f„lt).
P† grund av detta kan inte inneh†llet i det h„r f„ltet att h†llas f”r
trov„rdigt. FleetStreet f”rlitar sig ist„llet p† areaindexet.
:p.
Indexet f”r JAM-areor har en d†lig design. Med hj„lp av endast indexposten
„r det om”jligt att best„mma om ett meddelande „r borttaget eller aktivt. Det
finns bara tv† m”jligheter att g† runt problemet&colon.
:ol.
:li.Man l„ser f”rutom indexet meddelandehuvudet, f”r att uppt„cka om
meddelandet „r raderat eller aktigt. Detta „r l†ngsamt och motarbetar
indexets syfte. FleetStreet anv„nder inte l„ngre denna metod.
:li.Man visar alla meddelanden, „ven om n†gra av dem „r markerade att
vara "borttagna".
:eol.
:p.
F”r att borttagning av meddelanden trots allt ska ha n†got genomslag, skriver
FleetStreet helt ”ver meddelandets indexpost. Detta st„mmer dock inte till
100% med JAM-specifikationen, men det „r trots allt det enda f”rnuftiga
tillv„gag†ngss„ttet. Meddelanden som en g†ng har tagits bort med FleetStreet
dyker i alla fall inte upp mer.
:p.
M†nga meddelandebas-"utilities" tar bort genom att bara markera meddelandet
som borttaget i huvudet, och „ndrar inte indexposten. I s†dana fall b”r man,
i anslutning till detta, packa arean f”r att slutgiltigt ta bort dessa
meddelanden.

.* @1@ **********************************************************************
:h1.Tankar kring tossers
:p.
FleetStreet st”der flera olika tossers direkt. I detta kapitel hittar
du n†gra tips p† hur samarbetet med olika tossers kan effektiviseras.

.* @2@ **********************************************************************
:h2.Squish
:p.
Samarbetet med Squish „r relativt problemfritt. Det finns dock n†gra
noteringar som b”r g”ras&colon.
:ul.
:li.Squish k„nner varken av eller behandlar FLAGS-kludgen, d.v.s bara
standardmeddelandeattributen har n†gon som helst effekt.
:li.Vid packning av meddelanden i *.MSG-areor kommer l„sr„knaren att
s„ttas tillbaka till 0, vilket betyder att meddelanden kommer att
markeras som "ol„st".
:eul.

.* @2@ **********************************************************************
:h2.Fastecho
:p.
:ul.
:li.Fastecho st”der inte Squish' Broadcast-("Fj„rrbeordran")-funktioner.
:li.En egendomlighet „r hanteringen av flera netmailareor. I motsats till
Squish kan inte Fastecho tossa till eller packa fr†n flera netmailareor.
Netmailen m†ste f”re packningen exporteras till en sekund„r netmailarea,
f”r att sedan d„rifr†n exporteras och packas. F”r att exportera anv„nds
samma kommando som vid export av echomail. FleetStreet behandlar dock
samliga netmailareor p† samma s„tt, d.v.s de kommer inte att skrivas till
ECHOTOSS.LOG-filen. Vid export kommer allts† inte netmail i de
sekund„ra netmailareorna att registreras.
:p.
L”sningen till detta problem „r att genomf”ra exporten i tv† steg&colon.
I det f”rsta steget exporteras echomail med hj„lp av ECHOTOSS.LOG-filen. I det
andra steget exporteras netmailen till den sekund„ra netmailarean med hj„lp
av en "dummy"-ECHOTOSS.LOG. Denna dummy-ECHOTOSS.LOG inneh†ller helt enkelt
bara en lista p† de sekund„ra netmailareorna. I det sista steget kommer
den exporterade posten att packas.
:p.
Exempel&colon.
:xmp.
FASTECH2 SCAN -Lechotoss.log
FASTECH2 SCAN -Lnmareas.log
FASTECH2 PACK -P

[NMAREAS.LOG]
GERNET
OS2NET
:exmp.
:eul.

.* @2@ **********************************************************************
:h2.IMail
:p.
Version 1.75 av IMail har tyv„rr ett fel&colon. I JAM-areor sparas
:hp2.PATH:ehp2.-kludgen och :hp2.SEEN-BY:ehp2.-raderna till meddelandetexten,
i st„llet f”r (som anges i specifikationen) i meddelandehuvudet. P† grund
av detta kommer dessa rader att i FleetStreet visas under meddelandetexten.


.* @1@ **********************************************************************
:h1.Bilagor
:p.

.***************************************************************************
.* Kludge-Lines - Kludgerader
.* @2@
.*

:h2 id=kludges.Kludgerader
:p.
F”ljande kludgerader genereras och igenk„nns av FleetStreet&colon.
:parml compact tsize=3 break=all.
:pt.:hp2.FMPT:ehp2.
:pt.:hp2.TOPT:ehp2.
:pt.:hp2.INTL:ehp2.
:pd.Dessa kludgar genereras av FleetStreet i enlighet med :hp4.FTS-0001 rev.
15:ehp4. om meddelandet „r i NetMail.
:p.Squish skriver om dessa kludges vid exportering av meddelandena. St”d f”r
dem „r allts† egentligen inte n”dv„ndigt.
:p.Vid l„sning av meddelanden, ignoreras dessa kludges.
:p.

:pt.:hp2.MSGID:ehp2.
:pd.Vid skrivning av meddelanden genereras i enlighet med :hp4.FTS-0009 rev.
1.:ehp4. Adresskomponenten „r fyrdimensionell.
:p.Vid l„sning av ett echomail-m”te anv„nds :hp2.MSGID:ehp2. f”r att ta reda
p† brevets avs„ndaradress. Om en :hp2.MSGID:ehp2. inte inneh†ller n†gon
FTN-adress, eller om den saknas, avs”ks Origin-linjen f”r en giltig adress.
:p.

:pt.:hp2.REPLY:ehp2.
:pd.Vid skrivning av svar skrivs originalets :hp2.MSGID:ehp2. som en
:hp2.REPLY:ehp2.-kludge.
:p.

:pt.:hp2.PID:ehp2.
:pd.Om du har aktiverat :hp2.PID:ehp2. i inst„llningarna, kommer
:hp2.PID:ehp2.-kludges att skrivas, tillsammans med en kort tear-rad. En
l†ng tear-rad l„ggs till om :hp2.PID:ehp2. inte „r aktiverat.
:p.FleetStreet f”ljer rekommendationerna i :hp4.FSC-0046 rev. 2:ehp4..
:p.

:pt.:hp2.REPLYTO:ehp2.
:pd.Vid besvarande av NetMail som inneh†ller en :hp2.REPLYTO:ehp2.-kludge
kommer den d„r angivna adressen att s„ttas in som mottagande anv„ndarnamn.
:p.I echomailm”ten ignoreras denna kludge. FleetStreet f”ljer rekommendationerna
i :hp4.FSC-0035 rev. 1.:ehp4.
:p.

:pt.:hp2.REPLYADDR:ehp2.
:pd.Om Du svarar p† ett NetMail med :hp2.REPLYADDR:ehp2. kommer den d„r
angivna adressen att l„ggas till i en "To&colon."-rad i b”rjan av svaret.
:p.I echomailm”ten ignoreras denna kludge. FleetStreet f”ljer rekommendationerna
i :hp4.FSC-0035 rev. 1.:ehp4.
:p.

:pt.:hp2.SPLIT:ehp2.
:pd.D† ett meddelande sparas, och det „r l„ngre „n 12 KB, kommer det att delas
i mindre delar i enlighet med :hp4.FSC-0047 rev. 1:ehp4.. F”ljande avvikelser „r
v„rda att notera&colon.
:ul.
:li.D† meddelandet aldrig funnits i meddelandebasen i sin helhet, s† „r
meddelandenumret i :hp2.SPLIT:ehp2.-kludgen beroende p† implementation (m.a.o
meddelandenumret f”r den f”rst skapade delen).
:li.:hp4.FSC-0047:ehp4. rekommenderar att det inte finns n†gon :hp2.MSGID:ehp2.
i del tv† och fram†t, s† att en kontroll av duplikat inte ska radera dessa
delar. FleetStreet genererar d„remot en ny :hp2.MSGID:ehp2. f”r varje del,
vilket ger samma resultat.
:li.D† delnumret i :hp2.SPLIT:ehp2.-kludgen „r tv†siffrigt, skapas
maximalt 99 delar. Meddelandel„ngden „r d„rmed begr„nsad till 1188 KB.
:li.Vid redigering kan det f”r„ndrade meddelandet maximalt vara 15
KB l†ngt. En ny uppdelning skulle kollidera med numreringen hos de p†f”ljande
delarna, vilket skulle medf”ra att sammanfogning av meddelandena ej l„ngre
vore m”jlig.
:li.Numreringen av meddelandedelarna g”rs i slutet av „mnesraden,
inte i b”rjan, vilket f”resl†s. Genom detta f”rfarande kommer delarna att visas i
r„tt ordning i „mneslistan.
:eul.
:p.
:pt.:hp2.APPEND:ehp2.
:pt.:hp2.REALADDRESS:ehp2.
:pd.Dessa kludgar beh†lls vid svar.
:p.
:pt.:hp2.CHARSET/CHRS:ehp2.
:pd.FleetStreet st”der dessa kludgar upp till och med niv† 2 i enlighet med
:hp4.FSC-0054 Rev. 4:ehp4.. Vid skrivning av meddelanden anv„nder FleetStreet
alltid :hp2.IBMPC 2:ehp2..
:p.
:pt.:hp2.ACUPDATE:ehp2.
:pd.Denna kludgerad anv„nds i Squish 1.10 f”r att „ndra eller radera
meddelanden hos andra system. Konsultera dokumentationen f”r Squish 1.10 f”r
ytterligare information.
:p.
:pt.:hp2.AREA:ehp2.
:pd.Om denna kludgerad hittas i ett meddelande, och m”tet k„nns igen av
FleetStreet kommer svaret automatiskt att hamna i det h„r angivna m”tet.
:p.
:pt.:hp2.FLAGS:ehp2.
:pd.:hp2.FLAGS:ehp2.-kludgen inneh†ller de meddelandeattribut som inte
st”ds direkt av meddelandebasen. Formatet p† dena kludge „r definierat i
:hp4.FSC-0053:ehp4..
:p.
:pt.:hp2.FWDFROM, FWDTO, FWDSUBJ, FWDORIG, FWDDEST, FWDAREA, FWDMSGID:ehp2.
:pd.Dessa kludgar genereras vid vidares„ndning av meddelanden. De
inneh†ller headerinformation f”r det ursprunliga meddelandet. N„r du svarar
p† ett s†dant meddelande anv„nder FleetStreet dessa poster f”r svaret.
:eparml.

.* @2@ **********************************************************************
:h2 id=cmdlin.Kommandoradsparametrar
:p.
FleetStreet godtar f”ljande kommandoradsparameter&colon.
:parml.
:pt.:hp2.-C<s”kv„g>:ehp2.
:pd.INI-filerna kommer inte att l„sas och skrivas fr†n den aktuella
katalogen, utan i st„llet fr†n den katalog du anger.
:p.:hp2.Exempel&colon.:ehp2.
:p.FLTSTRT.EXE -Cd&colon.\myinis
:eparml.

.* @2@ **********************************************************************
:h2.Svarskoder
:p.
FLTSTRT.EXE ger f”ljande svarskoder&colon.
:parml break=none.
:pt.:hp2.0:ehp2.
:pd.Inga nya meddelanden har skrivits.
:pt.:hp2.1:ehp2.
:pd.Nya netmails har skrivits.
:pt.:hp2.2:ehp2.
:pd.Nya echomailmeddelanden har skrivits.
:pt.:hp2.4:ehp2.
:pd.Nya lokala meddelanden har skrivits.
:pt.:hp2.255:ehp2.
:pd.Allvarligt fel
:eparml.
:p.
En kombination av 1, 2 och 4 betyder att meddelanden har skrivits in i
flera av m”testyperna. Exempelvis betyder 5 att meddelanden skrivits b†de i
netmail och lokala m”ten.

.* @2@ **********************************************************************
:h2.Buggrapporter
:p.
FleetStreet „r med s„kerhet inte felfritt. Jag kan d„rf”r bar uppmuntra
alla att meddela alla fel till mig. Jag kan d† efter min b„sta f”rm†ga
f”rs”ka avhj„lpa felet.
:p.
M†nga fel kan jag ev. inte (direkt) s„tta mig in i. F”ljande
fr†gest„llningar „r f”rhoppningsvis relevanta f”r att kunna avhj„lpa
felet&colon.
:ul.
:li.L†ter felet sig reproduceras?
:li.Upptr„der felet vid f”rsta f”rs”ket, eller f”rst efter flera g†nger?
:li.Vilka funktioner utf”rdes n„r felet uppstod?
:li.Utf”rdes funktionen fr†n tangentbordet, menyn eller knappraden? G”r det
n†gon skillnad om Du anv„nder en annan metod?
:li.Framtr„der n†got felmeddelande? Vilket?
:li.r FleetStreet riktigt konfigurerat?
:li.Vad exakt h„nder n„r felet uppst†r?
:eul.
:p.
Om FleetStreet st„ngs av p† grund av ett fel, kommer filien
:hp2.FLTSTRT.DMP:ehp2. att skapas i den aktuella katalogen. Med hj„lp av
denna fil kan felet oftast lokaliseras t„mligen enkelt.


.* @2@ **********************************************************************
:h2.Anv„nda program
:p.
FleetStreet skapades och testades med f”ljande program&colon.
:ul.
:li.Kompilator&colon. IBM VisualAge C++ 3.0 (C-l„ge)
:li.Debugger&colon. IBM C/C++ Debugger 3.0 (IPMD)
:li.Utvecklingsmilj”&colon. IBM Workframe 3.0
:li.L„nkare&colon. ILink
:li.Redigerare&colon. Enhanced Editor, Tiny Editor, LPEX
:li.Verktygsprogram&colon. IBM OS/2 Toolkit 3.0; GNU Grep; PMTree; ExeMap;
Hexdump; PMSpy; PM Camera
:eul.

.* @2@ **********************************************************************
:h2 id=support.Support
:p.Michael Hohner kan n†s p† f”ljande e-post-adresser&colon.
:parml compact break=none tsize=16.
:pt.Fidonet&colon.
:pd.Michael Hohner 2&colon.2490/1050.17 (new!)
:pt.OS2Net&colon.
:pd.Michael Hohner 81&colon.499/617.17 (new!)
:pt.Internet&colon.
:pd.miho@n-online.de (new!)
:eparml.
:p.
:hp2.Fido echomail&colon.:ehp2.
:p.
Det finns tv† Fidonet-m”ten hos 2&colon.2490/1050, FLEETBETA och
FLEETSTREET. FLEETBETA „r det tyskspr†kiga m”tet, FLEETSTREET „r det
internationella m”tet (engelskspr†kigt). Skriv ett netmail till Robert
Gloeckner 2&colon.2490/1050, om Du vill f† tag p† m”tet. D„r kan Du
f† information om andra noder som har m”tet. M”tet kan fritt vidares„ndas,
men s„nd oss information om anslutna noder.
:p.
Fr†gor om FleetStreet kan „ven st„llas i Fidonetm”tet OS2BBS.GER.

:euserdoc.

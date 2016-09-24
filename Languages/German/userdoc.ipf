:userdoc.
:docprof toc=1234.
:title.FleetStreet Benutzeranleitung

.* ************************** Einf�hrung ********************************
.* @1@ **********************************************************************
:h1.Allgemeines
:p.
:lines align=center.
Willkommen bei

:font facename='Tms Rmn' size=24x18.:color fc=red.
FleetStreet 1.27.1
:font facename=default size=0x0.:color fc=default.
:p.
FleetStreet ist ein FTS-kompatibler Nachrichten-Editor f�r OS/2 2.x PM.
FleetStreet kann *.MSG-, Squish- und JAM-Message-Bases bearbeiten.
:elines.
:p.
:p.
Diese Dokumentation beschreibt die Konfiguration von FleetStreet und grundlegende
Vorgehensweisen. Detailinformationen zu den einzelnen Dialogen von FleetStreet
befinden sich in der Online-Hilfe zum Programm. Diese Doku und die Online-Hilfe
erg�nzen sich.
:p.
Diese Dokumentation setzt voraus, da� Sie bereits grundlegende Kenntnisse �ber
das Fidonet oder Fido-kompatible Netze haben.
:p.
Dies ist ein Open-Source-Programm.


.* ************************** Features   ********************************
.* @2@ **********************************************************************
:h2.Features
:p.
Features von FleetStreet&colon.
:ul compact.
:li.OS/2 2.x/3.x PM-Programm
:li.100% 32-Bit-Code
:li.Optimiert f�r Pentium-Prozessoren
:li.Multi-Threaded
:li.WPS-�hnliche Bedienung
:li.Bedienung �ber
:ul compact.
:li.Men�s
:li.Tastenk�rzel
:li.Toolbar (Position w�hlbar, 2 Gr��en)
:eul.
:li.Benutzerunterst�tzung durch ausf�hrliche Online-Hilfe und Hilfetexte in
der Statuszeile
:li.Standard-Funktionen wie
:ul compact.
:li.Messages lesen
:li.Messages schreiben
:li.Messages l�schen
:li.Messages �ndern
:li.Messages kopieren
:li.Messages verschieben
:li.Messages weiterleiten
:li.Auf eine Message antworten
:li.Message exportieren
:li.ASCII-Text in Message importieren
:eul.
:li.Unterst�tzung der Area-Formate
:ul compact.
:li.*.MSG
:li.Squish
:li.JAM
:eul.
:li.Einlesen der Konfigurationsdateien von
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
:li.Drucken einer Message �ber den PM-Druckertreiber
:li.Erzeugen einer ECHOTOSS.LOG-Datei
:li.Farbige Anzeige der Messages
:li.Crossposten einer neuen Message in mehreren Areas
:li.Erzeugen von File-Requests aus einer Message
:li.Schreiben an mehrere Empf�nger �ber Verteilerlisten
:li.Schnellverteiler
:li.Kurznamenliste
:li.Unterst�tzung der Broadcast-Funktionen von Squish 1.1x
:li.Unterst�tzung von Version-7-Nodelisten
:li.Nodelist-Browser
:li.Frei mit Text belegbare Funktionstasten
:li.Umsetzung von Laufwerksbuchstaben f. Netzwerk-Betrieb
:li.Frei definierbare Message-Schablonen
:li.Komfortable Suchfunktion, auch �ber mehrere Areas
:li.Suche nach pers�nlichen Messages
:li.Themenliste zum Lesen nach Themenzeilen
:li.Messageliste
:li.Vielf�ltige Area-Einstellungen
:li.Umlautkonvertierung beim Schreiben
:li.Unterst�tzung der CHRS-Kludgeline
:li.Manuelles Markieren von Messages
:li.Fernsteuerung �ber Named Pipe
:li.Makro-programmierbar durch Rexx-Skripte
:li.Vielf�ltige Drag-and-Drop-Features
:li.Programm und Benutzeranleitung in den Sprachen
:ul compact.
:li.Deutsch
:li.Englisch
:li.Italienisch
:li.Schwedisch
:eul.
:eul.
:p.
&dot.&dot.&dot. und das ist nur eine grobe �bersicht &colon.-)

.* @2@ **********************************************************************
:h2.Voraussetzungen
:p.
FleetStreet setzt die folgende Hard- und Software voraus&colon.
:p.
:hp2.Hardware&colon.:ehp2.
:ul compact.
:li.PC, der OS/2 ausreichend schnell laufen lassen kann (386DX, 6 MB)
:li.ca. 1,5 MB Plattenspeicher, zzgl. Messagebase
:li.VGA-Grafikkarte
:eul.
:p.
:hp2.Software&colon.:ehp2.
:ul compact.
:li.OS/2 2.x/3.x (2.0 nicht getestet, sollte aber gehen)
:eul.
:p.
:hp2.Getestete Softwareumgebungen&colon.:ehp2.
:ul compact.
:li.OS/2 2.1 (verschiedene Sprachen)
:li.OS/2 2.11 (2.1 mit Service Pack)
:li.OS/2 2.99 (WARP II)
:li.OS/2 Warp 3.0
:li.OS/2 Warp Connect (Peer To Peer)
:li.OS/2 Warp 4.0
:li.ZipStream 1.03 (Messagebase in komprimiertem Verzeichnis)
:li.LAN Server 4.0 Entry
:eul.

.***************************************************************************
.* Design Goals                                                            *
.***************************************************************************

.* @2@ **********************************************************************
:h2 id=design.Design-Ziele
:p.Bei der Entwicklung von FleetStreet hatten wir mehrere Ziele im
Hinterkopf, �ber die Sie hier etwas lesen k�nnen.
:parml.
:pt.:hp2.Warum ein PM Editor?:ehp2.
:pd.Der Presentation Manager (tm) mag zwar langsamer als eine VIO-Applikation
sein (solange diese als Full-Screen-Applikation gestartet wird), hat aber
andererseits einige Vorteile gegen�ber dieser. Die Zwischenablage ist voll
verwendbar (das Clipboard der VIO-Applikationen ist eine Kr�cke), der
Benutzer kann sich die Schriftart und die Farben sehr leicht selbst ausw�hlen.
Au�erdem ist man nicht auf ein 80*25 (o.�.) - Format beschr�nkt. Der PM erlaubte
es uns zus�tzlich noch all die Dialogboxen zu implementieren, die den Umgang mit
FleetStreet so leicht machen.
:pt.:hp2.Integriertes Setup:ehp2.
:pd.Bei anderen Editoren haben Sie sich vielleicht mit endlosen
ASCII-Konfigurationsdateien herumgeschlagen. Wenn Sie schon an die verschiedenen
M�glichkeiten der Benutzung von Notebooks und Dialogen der Workplace-Shell
gew�hnt sind, dann sollte die Konfiguration von FleetStreet kein Problem
darstellen. Die Einstellungen k�nnen zur Laufzeit ge�ndert werden. FleetStreet
muss also nicht neu gestartet werden, um die Konfiguration zu �bernehmen.
:p.
Wenn Sie nicht wissen, was eine Einstellung bewirkt, dr�cken Sie einfach F1 oder
einen der Hilfe-Buttons, die �berall rumliegen. Das
erspart auch das langweilige Bl�ttern in l�nglichen Textdokumentationen.
:pt.:hp2.Support f�r Squish (tm):ehp2.
:pd.FleetStreet unterst�tzt den Squish-Tosser auf zwei Arten. Zum einen benutzt es
die API von Squish (MSGAPI.DLL), zum anderen liest FleetStreet die
Konfigurationsdatei von Squish (SQUISH.CFG) direkt ein.
:i2 refid=squish.MSGAPI.DLL
:pt.:hp2.Leichte Benutzbarkeit:ehp2.
:pd.Die meisten PM Applikationen sind einfach zu benutzen. Wenn Sie die WPS benutzen
k�nnen, dann auch FleetStreet, denn wir benutzen Notebooks, Popup-Men�s,
Container usw.
:pt.:hp2.All die Funktionen, die man so braucht:ehp2.
:pd.Wir haben eine ganze Reihe an 'Features' implementiert, die wir in anderen
Message-Editoren n�tzlich fanden und einige, die wir nirgendwo sonst fanden.
:pt.:hp2.Power:ehp2.
:pd.FleetStreet benutzt mehrere Threads, CUA'91 und 32 Bit-Verarbeitung.
:eparml.

.* @2@ **********************************************************************
:h2.Copyrights etc.
:p.
Squish und MsgAPI sind Warenzeichen von Lanius Corporation.
:p.
OS/2 und Workplace Shell sind Warenzeichen von IBM.
:p.
JAM(mbp) - Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, Mats Wallin.
ALL RIGHTS RESERVED.

.* @2@ **********************************************************************
:h2.Danksagungen
:p.
Die folgenden Personen haben zum Entstehen und zur Weiterentwicklung von
FleetStreet besonders beigetragen&colon.
:sl.
:li.:hp4.Harry Herrmannsd�rfer:ehp4. - Echo-Management, Alpha/Beta-Test, Programmierung
:li.:hp4.Harald Kamm:ehp4. - Italienische �bersetzung, Registration-Site Italien
:li.:hp4.Jason Meaden:ehp4. - Ehemalige Registration-Site Australien
:li.:hp4.Helmut Renner:ehp4. - Registration-Site und Echo-Link �sterreich
:li.:hp4.Marty Duplissey:ehp4. - Ehemalige Registration-Site
:li.:hp4.Siegfried Hentschel, Jens Holm, Richard Douglas,
Jose M. Guglieri:ehp4. - Registration-Sites
:li.:hp4.Rasmus Foged Hansen:ehp4. - Ehemalige Registration-Site
:li.:hp4.Peter Karlsson:ehp4. - Schwedische �bersetzung
:li.:hp4.Alle Teilnehmer der Closed-Beta-Phase, besonders Michael Siebke,
Joachim Loehrl-Thiel, Hajo Kirchhoff, Robert Gl�ckner:ehp4. u.a.
:li.:hp4.Thomas Jachmann:ehp4. - Beta-Test und diverse Rexx-Skripte
:li.:hp4.Torsten Grimme:ehp4. - Test der IMail-Unterst�tzung
:li.:hp4.Dirk Brenken:ehp4. - Test der Fastecho-Unterst�tzung
:li.:hp4.Carsten Ellwart:ehp4. - Bug-Finder
:li.:hp4.ganz besonders alle registrierte Benutzer:ehp4., ohne die Shareware
nicht �berleben kann.
:esl.

.* ************************* Installation **********************************

.* @1@ **********************************************************************
:h1.Basis-Konfiguration
:p.
Wenn Sie FleetStreet zum ersten Mal starten, dann erscheint die Meldung, da�
die INI-Datei nicht gefunden wurde. Dies ist in dieser Situation ganz
normal und kein Grund zur Besorgnis. Dr�cken Sie einfach den :hp2.OK:ehp2.-Button.
FleetStreet �ffnet dann das Setup-Notebook automatisch, und Sie k�nnen mit
der Basis-Konfiguration beginnen.
:p.
Die hier beschriebene Basis-Konfiguration gen�gt v�llig, um FleetStreet in
einer �blichen Node- oder Point-Konfiguration zu installieren und die ersten
Schritte damit zu wagen. Die Konfiguration kann sp�ter jederzeit ge�ndert und
den eigenen Bed�rfnissen angepa�t werden.
:p.
Falls Sie bei der Konfiguration einmal nicht weiterkommen, dann dr�cken Sie
einfach den :hp2.Hilfe:ehp2.-Button, der sich auf jeder Seite befindet.

.* @2@ **********************************************************************
:h2.Namen
:p.
Wenn Sie neue Messages schreiben, setzt FleetStreet als Absender automatisch
Ihren Namen ein.
:p.
Auf der ersten Seite des Setup-Notebooks k�nnen Sie alle Namen eingeben, die
Sie mit FleetStreet verwenden wollen. Der erste Name in der Liste ist dabei Ihr
Default-Name. Er wird immer zuerst eingesetzt, wenn irgendwo Ihr Name
ben�tigt wird. Sie k�nnen jeden Namen in der Liste zu Ihrem Default-Namen
machen, indem Sie den Namen in der Liste ausw�hlen und den
:hp2.Default:ehp2.-Button dr�cken.

.* @2@ **********************************************************************
:h2.Adressen
:p.
Auf der zweiten Seite des Setup-Notebooks k�nnen Sie alle Netz-Adressen eingeben,
die Sie mit FleetStreet verwenden wollen. Die erste Adresse in der Liste
ist wieder Ihre Default-Adresse.

.* @2@ **********************************************************************
:h2.Tosser-Konfiguration
:p.
Wenn Sie bereits einen Tosser installiert haben, der von FleetStreet
unterst�tzt wird, dann k�nnen sie ganz einfach dessen Konfigurations-Datei
einlesen lassen. FleetStreet entnimmt daraus alle definierten Adressen, Areas
und Verzeichnisse, so da� Sie Ihre Message-Areas nicht nochmal in FleetStreet
definieren m�ssen.
:p.
Auch in Zukunft wird FleetStreet beim Start die angegebene Datei einlesen,
so da� die Areas in FleetStreet immer mit den Areas, die Sie f�r Ihren
Tosser definiert haben, �bereinstimmen.
:p.
Die folgenden Tosser werden unterst�tzt&colon.
:table cols='10 20 30'.
:row.
:c.Tosser
:c.Versionen
:c.Auszuw�hlende Datei
:row.
:row.
:c.Squish
:c.1.01, 1.10, 1.11
:c.SQUISH.CFG (oder Name einer �quivalenten Datei)
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
Aktivieren Sie zun�chst die Einstellung :hp2."lesen":ehp2.
auf der vierten Seite des Setup-Notebooks. W�hlen Sie dann den gew�nschten
Tosser aus. Dr�cken Sie danach den Button :hp2."Suchen...":ehp2.. Suchen
Sie die Konfigurations-Datei, w�hlen Sie sie aus und dr�cken Sie :hp2.OK:ehp2..
:p.
FleetStreet hat nun die Datei eingelesen. Damit ist die Basis-Konfiguration
auch schon beendet. Schlie�en Sie das Setup-Notebook. FleetStreet sollte nun
die erste Area �ffnen und die erste Message im Hauptfenster anzeigen.

.* @1@ **********************************************************************
:h1.Bedienung
:p.

.* @2@ **********************************************************************
:h2 id=mainwin.Hauptfenster
:p.
Beim Lesen von Messages werden die folgenden Elemente des Hauptfensters
angezeigt&colon.
:p.
:hp2.Titelbalken:ehp2.
:artwork name='titlebar.bmp' align=center.
:p.
Es werden angezeigt&colon.
:ul compact.
:li.Programmname und -Version
:li.Aktuelle Area
:eul.
:p.
:hp2.Message-Kopf:ehp2.
:artwork name='header.bmp' align=center.
:p.
Es werden angezeigt&colon.
:ul compact.
:li.Name und Adresse des Absenders
:li.Name und Adresse des Empf�ngers (Adresse nur in Netmail-Areas)
:li.Themenzeile
:li.Attribute der Message
:li.Datum und Zeit des Erstellens der Messages
:li.Datum und Zeit der Ankunft der Message
:eul.
:p.
:hp2.Toolbar:ehp2.
:p.
Die Bedeutung der Buttons ist&colon.
:parml compact break=none tsize=6.
:pt.:artwork runin name='home.bmp'.
:pd.Zur�ck zum alten Lastread der Area
:pt.:artwork runin name='nextarea.bmp'.
:pd.Zur n�chsten Area mit neuen Messages
:pt.:artwork runin name='prevmsg.bmp'.
:pd.Zur vorherigen Message
:pt.:artwork runin name='nextmsg.bmp'.
:pd.Zur n�chsten Message
:pt.:artwork runin name='prevreply.bmp'.
:pd.Zum Original zu dieser Antwort
:pt.:artwork runin name='nextreply.bmp'.
:pd.Zur Antwort zu dieser Message
:pt.:artwork runin name='firstmsg.bmp'.
:pd.Zur ersten Message
:pt.:artwork runin name='lastmsg.bmp'.
:pd.Zur letzten Message
:pt.:artwork runin name='ok.bmp'.
:pd.Message speichern
:pt.:artwork runin name='cancel.bmp'.
:pd.Message oder �nderungen verwerfen
:pt.:artwork runin name='newmsg.bmp'.
:pd.Neue Message schreiben
:pt.:artwork runin name='edit2.bmp'.
:pd.Aktuelle Message �ndern
:pt.:artwork runin name='importfile.bmp'.
:pd.Textfile in Message importieren
:pt.:artwork runin name='exportfile.bmp'.
:pd.Message als Textfile exportieren
:pt.:artwork runin name='reply2.bmp'.
:pd.Auf aktuelle Message antworten
:pt.:artwork runin name='printmsg.bmp'.
:pd.Aktuelle Message drucken
:pt.:artwork runin name='find.bmp'.
:pd.Suchfunktion starten
:pt.:artwork runin name='msglist.bmp'.
:pd.Messageliste �ffnen
:pt.:artwork runin name='msgtree.bmp'.
:pd.Themenliste �ffnen
:pt.:artwork runin name='area.bmp'.
:pd.Arealiste �ffnen
:pt.:artwork runin name='showkludges.bmp'.
:pd.Fenster mit Kludge-Lines �ffnen
:pt.:artwork runin name='delmsg.bmp'.
:pd.Aktuelle Message l�schen
:pt.:artwork runin name='bookm.bmp'.
:pd.Zum Merkerfenster umschalten
:pt.:artwork runin name='help.bmp'.
:pd.Allgemeine Hilfe zu FleetStreet
:pt.:artwork runin name='cut.bmp'.
:pd.Auswahl ins Clipboard kopieren und Auswahl l�schen
:pt.:artwork runin name='copy.bmp'.
:pd.Auswahl ins Clipboard kopieren
:pt.:artwork runin name='paste.bmp'.
:pd.Inhalt des Clipboards an momentaner Position einf�gen
:pt.:artwork runin name='copymsg.bmp'.
:pd.Aktuelle Message in andere Area kopieren
:pt.:artwork runin name='movemsg.bmp'.
:pd.Aktuelle Message in andere Area verschieben
:pt.:artwork runin name='fwdmsg.bmp'.
:pd.Aktuelle Message in anderer Area weiterleiten
:pt.:artwork runin name='shell.bmp'.
:pd.Kommandozeilen-Shell starten
:pt.:artwork runin name='script.bmp'.
:pd.Rexx-Skripts
:pt.:artwork runin name='browser.bmp'.
:pd.Inhalt der Nodelisten anzeigen
:pt.:artwork runin name='request.bmp'.
:pd.Dateien in der aktuellen Message requesten
:pt.:artwork runin name='catchup.bmp'.
:pd.Alle Messages als "gelesen" markieren
:eparml.
:p.
:hp2.Statuszeile:ehp2.
:artwork name='statline.bmp' align=center.
:p.
Es werden angezeigt (von links nach rechts)&colon.
:ul compact.
:li.Hilfetexte, Meldungen
:li.Markierungszeichen (wenn Message markiert ist)
:li.Cursorposition (nur beim Schreiben)
:li.Einf�ge/�berschreib-Modus (nur beim Schreiben)
:li.Nummer der aktuellen Message, Anzahl der Messages in der Area
:li.Aktuelle Adresse
:eul.

.* @2@ **********************************************************************
.* :h2.Lesen
.* :p.



.* @2@ **********************************************************************
:h2 id=writemsg.Schreiben von Nachrichten
:p.Es gibt zwei M�glichkeiten, um neue Nachrichten einzugeben&colon.
:parml.
:pt.:hp2.Neue Nachrichten schreiben:ehp2.
:pd.Wenn Sie eine neue Nachricht eingeben wollen, dr�cken Sie auf den Button
:artwork name='help01.bmp' runin. oder dr�cken Sie EINFG.
Das Nachrichtenfenster wird geleert und Ihre Absenderadresse wird in die
entsprechenden Absender-Felder eingetragen. Sie k�nnen nun den Empf�ngernamen,
dessen Netz-Adresse (siehe auch :link reftype=hd refid=enteraddr."Eingabe von
FTN-Addressen":elink.) und die Betreff-Zeile eingeben. Danach k�nnen Sie den
Text in das Nachrichtenfenster eingeben.
:p.Wenn Sie mit Ihrer Nachricht fertig sind, dann dr�cken Sie auf den Knopf
:artwork name='help04.bmp' runin. oder dr�cken die Taste STRG-S. Die Nachricht
wird dann in Ihrer Message-Base gespeichert. Dr�cken Sie w�hrend der Eingabe
den Knopf :artwork name='help05.bmp' runin. oder die Taste ESC, wenn Sie die
Eingabe abbrechen wollen.
:pt.:hp2.Antworten auf eine Nachricht:ehp2.
:pd.Um auf eine empfangene Nachricht zu antworten, dr�cken Sie auf den Knopf
:artwork name='help03.bmp' runin. oder dr�cken Sie die Taste STRG-R oder
STRG-N, wenn Sie dem Absender in einer anderen Area (z.B. Privatpost) antworten
wollen.
�ber den Knopf k�nnen Sie jetzt entweder dem Absender oder dem Empf�nger der
Nachricht antworten. Es gibt auch die M�glichkeit, in einer anderen Area zu
antworten.
:p.Wenn Sie mit Ihrer Nachricht fertig sind, dann dr�cken Sie auf den Knopf
:artwork name='help04.bmp' runin. oder dr�cken die Taste STRG-S. Die Nachricht
wird dann in Ihrer Message-Base gespeichert. Dr�cken Sie w�hrend der Eingabe
den Knopf :artwork name='help05.bmp' runin. oder die Taste ESC, wenn Sie die
Eingabe abbrechen wollen.
:eparml.

.* @2@ **********************************************************************
:h2 id=changemsg.�ndern von Messages
:p.Sie k�nnen Messages �ndern, nachdem sie abgespeichert wurden. Dr�cken Sie den
:artwork name='help06.bmp' runin. Knopf, um in den Editier-Modus zu schalten.
Nun k�nnen sie Ihre Message �berarbeiten. Sie k�nnen den Message-Text und den
Message-Header �ndern.
:p.Um die Attribute der Message umschalten, dr�cken Sie den :hp2.�ndern:ehp2. Knopf.
Ein Dialogfenster wird ge�ffnet, wo Sie die Message-Attribute setzen oder l�schen
k�nnen.
:p.Wenn Sie eine Message �ndern, die bereits verschickt wurde oder die keine lokale
Message ist, erscheint eine Warnung.
:p.Dr�cken Sie den :artwork name='help04.bmp' runin. Knopf, um die �nderungen
zu speichern. Wenn Sie die ge�nderte Message verwerfen wollen, dr�cken Sie den
:artwork name='help05.bmp' runin. Knopf.

.* @2@ **********************************************************************
:h2.L�schen
:p.
Sie k�nnen eine Message auf folgende Arten l�schen&colon.
:ul.
:li.Dr�cken Sie die :hp2.ENTF:ehp2.-Taste. Nach einer Sicherheitsabfrage wird die Message
gel�scht.
:li.Dr�cken Sie den L�sch-Button in der :link reftype=hd refid=mainwin.Toolbar:elink..
:li.Ziehen Sie die aktuelle Message auf den Shredder. Beginnen
Sie mit dem Ziehen �ber dem Editor-Fenster.
.br
:artwork align=center name='shredmsg.bmp'.
:eul.
:p.
Messages k�nnen auch aus der :link reftype=hd refid=msglist.Messageliste:elink.,
:link reftype=hd refid=threadlist.Themenliste:elink. oder
:link reftype=hd refid=bookmarks.Merkerliste:elink. gel�scht werden.

.* @2@ **********************************************************************
:h2.Drucken
:p.Sie k�nnen die aktuelle Message auf folgende Arten ausdrucken&colon.
:ul.
:li.Dr�cken Sie die Tastenkombination :hp2.SHIFT-DRUCK:ehp2.
:li.Dr�cken Sie den Druck-Button in der :link reftype=hd refid=mainwin.Toolbar:elink..
:li.Ziehen Sie die Message auf ein Druckerobjekt.
Beginnen Sie mit dem Ziehen �ber dem Editor-Fenster.
:eul.
:p.
Messages k�nnen auch aus der :link reftype=hd refid=msglist.Messageliste:elink.,
:link reftype=hd refid=threadlist.Themenliste:elink. oder
:link reftype=hd refid=bookmarks.Merkerliste:elink. gedruckt werden.

.* @2@ **********************************************************************
:h2.Arealiste
:p.
Die Arealiste enth�lt alle Message-Areas, die f�r FleetStreet zug�nglich sind.
Sie k�nnnen zu einer dieser Areas umschalten, Areas erstellen und l�schen und
Attribute von Areas setzen.
:p.
Klicken Sie auf eine der Areas oder dr�cken Sie :hp2.ENTER:ehp2., um zu dieser
Area umzuschalten.
:p.
Dr�cken Sie :hp2.Scannen:ehp2., um das Scannen aller Areas zu starten. Dies
k�nnte notwendig sein, wenn neue Messages getosst wurden, seit die Areas zuletzt
gescannt worden sind. Das Scannen findet in einem extra Thread statt. Sie k�nnen
weiter Messages lesen, w�hrend das Scannen im Hintergrund weiterl�uft.
:p.
Weiterf�hrende Themen&colon.
:ul compact.
:li.:link reftype=hd refid=areacon.Inhalt der Arealiste:elink.
:li.:link reftype=hd refid=areacrea.Erstellen und L�schen von Areas:elink.
:li.:link reftype=hd refid=areaset.Attribute f�r Areas einstellen:elink.
:li.:link reftype=hd refid=arealistset.Anpassen der Arealiste:elink.
:li.:link reftype=hd refid=areafolders.Area-Ordner:elink.
:eul.

.* ***************************** Area List settings *************************
:h3 id=areacon.Inhalt der Arealiste
:p.
F�r jede Area in der Liste wird
:ul compact.
:li.die Areabeschreibung,
:li.die Anzahl der Messages in der Area, und
:li.die Anzahl der :link reftype=fn refid=unrmsg.ungelesenen Messages:elink. in der Area
:eul.
:fn id=unrmsg.
:p.Tats�chlich ist dies nicht die :hp2.wirkliche:ehp2. Anzahl der ungelesenen
Messages. Es ist einfach nur die Anzahl der Messages nach dem Lastread.
:efn.
:p.
angezeigt.
:p.
F�r ungescannte Areas wird "-" als Anzahl der Messages angezeigt.
:p.
Die Areabeschreibung ist anfangs die gleiche wie der Area-Tag. Die Areabeschreibung
wird im Hauptfenster angezeigt und kann :link reftype=fn refid=desccha.ge�ndert:elink.
werden.

:fn id=desccha.
:p.Sie k�nnen dies im Einstellungs-Notebook der Area tun, oder indem Sie
Sie mit dem linken Mausbutton auf die Area klicken, w�hrend Sie die ALT-Taste
gedr�ckt halten. Sie k�nnen dann die Area-Beschreibung direkt editieren.
:efn.

:h3 id=areacrea.Erstellen und L�schen von Areas
:p.
:hp2.Erstellen einer neuen Area:ehp2.
:p.
Sie k�nnen eine neue Area erstellen, indem Sie :hp2."Erstellen":ehp2. im Kontextmen�
w�hlen. Ein leeres Einstellungs-Notebook wird f�r die neue Area ge�ffnet.
F�llen Sie alle Felder aus und schlie�en Sie das Notebook. Die neue Area wird dann
in der Arealiste eingef�gt.
:p.
Alle in FleetStreet erzeugten Areas sind :hp2.lokale Areas:ehp2..

:p.
:hp2.L�schen von Areas:ehp2.
:p.
Sie k�nnen nur diejenigen Areas l�schen, die Sie auch in FleetStreet erstellt haben.
Areas, die in der Tosser-Konfigurations-Datei definiert sind, m�ssen Sie auch
dort l�schen.
:p.
Um eine Area zu l�schen, �ffnen Sie das :link reftype=fn refid=areacon.Kontext-Men� der Area:elink.
und w�hlen Sie :hp2."L�schen":ehp2.. Die Area wird aus der Liste entfernt.

:fn id=areacon.
:p.Klicken Sie mit dem rechten Mausbutton auf die Area.
:efn.
:p.
Beachte&colon. Die Dateien der Area werden :hp2.nicht:ehp2. von der Platte
gel�scht.

:h3 id=areaset.Attribute f�r Areas einstellen
:p.
Jede Area hat ihren eigenen Satz Attribute. Diese sind&colon.
:ul compact.
:li.Area-Beschreibung
:li.Area-Tag
:li.Default-Username
:li.Default-Adresse
:li.Pfadname
:li.Area-Format
:li.Area-Typ
:li.Default-Message-Attribute
:li.Weitere Kennzeichnungen
:eul.
:p.
Bei Areas, die in der Tosser-Konfigurationsdatei definiert sind, k�nnen
der Area-Tag, die Default-Adresse, Pfadname, Area-Format und Net/Echo-Area-Kennzeichnung
nicht ver�ndert werden.
:p.
Um die Area-Attribute zu �ndern, �ffnen Sie das Kontextmen� der Area und w�hlen
Sie :hp2."Einstellungen":ehp2..

:h3 id=arealistset.Anpassen der Arealiste
:p.
Die Arealiste selbst hat diverse Attribute, die das Erscheinungsbild der Arealiste
beeinflussen. Um diese Attribute zu �ndern, �ffnen Sie das Kontextmen� der
Arealiste und w�hlen Sie :hp2."Einstellungen":ehp2..
:p.
Sie k�nnen einstellen &colon.
:ul compact.
:li.die Default-Ansicht
:li.die Sortierung
:li.die Farben, die f�r die verschiedenene Area-Typen verwendet werden.
:eul.

:h3 id=areafolders.Area-Ordner
:p.
Areas k�nnen in Ordnern zusammengefa�t werden. Es ist immer ein Ordner zu
einem Zeitpunkt ge�ffnet. Die Arealiste zeigt immer nur die Areas des offenen
Ordners an.
:p.
Es gibt einen :hp2.Default-Ordner:ehp2. mit einigen Besonderheiten&colon.
:ul compact.
:li.Er steht an der Spitze der Ordner-Hierarchie. Alle anderen Ordner sind
darunter angeordnet.
:li.Er kann nicht gel�scht oder verschoben werden.
:li.Areas, die keinem Ordner zugewiesen wurden (z.B. neu aus der
Tosser-Konfiguration eingelesene Areas), sind im Default-Ordner zu finden.
:eul.
:p.
Area-Ordner haben bestimmte Eigenschaften&colon.
:ul compact.
:li.Einen Namen. Dieser kann mit ALT-Klick und nachfolgendem Editieren
ge�ndert werden.
:li.Sortierung der darin enthaltenen Areas.
:li.Kennzeichnung f�r automatisches Scannen.
:eul.
:p.
Mit dem Kontextmen� der Ordner k�nnen neue Ordner erstellt und gel�scht
werden. Ordner k�nnen mit Drag-Drop verschoben werden.
:p.
Areas werden durch Drag-Drop den Ordnern zugewiesen.

.* ****************************** Message liste ******************************
.* @2@ **********************************************************************
:h2 id=msglist.Messageliste
:p.
Die Messageliste zeigt alle Messages in der Area an. Die Messages werden
in der gleichen Reihenfolge dargestellt, wie sie in der Messagebase
stehen.
:p.
Der eigene Name wird jeweils in einer anderen Farbe dargestellt. Gelesene und
ungelesene
Messages haben eine Messagenummer in unterschiedlicher Farbe. Die jeweilige Farbe
kann im Einstellungs-Notebook der Messageliste eingestellt werden.
:p.
Sie k�nnen mehrere Messages mit der Maus oder mit der Tastatur ausw�hlen
und manipulieren. Manipulationsm�glichkeiten sind&colon.
:ul compact.
:li.L�schen
:li.Kopieren
:li.Verschieben
:li.Drucken
:li.Exportieren
:eul.
:p.
Falls beim Lesen einer Message in der Messagebase ein Fehler aufgetreten
ist, wird in allen Feldern ein :hp2."*":ehp2. angezeigt.
:p.
Sie k�nnen die Spalten-Trenner verschieben, um die Breite der Spalten einzustellen.
:p.
:artwork align=center name='movesepa.bmp'.


.* ****************************** Threadlist *********************************
.* @2@ **********************************************************************
:h2 id=threadlist.Threadliste
:p.
Es werden die Message-Threads in der aktuellen Area dargestellt. Gelesene
Messages und ungelesene Messages werden in unterschiedlichen Farben
dargestellt.
:p.
Threads sind Messages in einer Area, die dadurch zusammenh�ngen, da�
eine Message eine Antwort auf eine andere Message ist oder selbst Antworten
hat. Wenn eine Message in der Threadliste Antworten hat, dann wird vor
der Message ein :artwork name='plus.bmp' runin. angezeigt. Sie k�nnen mit der Maus auf das
:artwork name='plus.bmp' runin. dr�cken
oder die :hp2.+:ehp2.-Taste dr�cken. Die Antworten werden dann in einer Baumstruktur
dargestellt. Das :artwork name='plus.bmp' runin. �ndert sich dann in ein
:artwork name='minus.bmp' runin. . Wenn
Sie die :hp2.Leertaste:ehp2. dr�cken,
dann wird sofort der gesamte Ast aufgeklappt.
:p.
Weitere Themen&colon.
:ul compact.
:li.:link reftype=hd refid=thdisp.Darstellungsmodi:elink.
:li.:link reftype=hd refid=thmani.Manipulationsm�glichkeiten:elink.
:li.:link reftype=hd refid=thlink.Reply-Linker:elink.
:li.:link reftype=hd refid=markmsg.Alle Messages markieren:elink.
:eul.

:h3 id=thdisp.Darstellungsmodi
:p.
Es gibt drei verschiedene Darstellungsmodi&colon. Alle Threads, Threads mit
ungelesenen Messages und nur ungelesene Messages.
:parml.
:pt.:hp2.Alle Threads:ehp2.
:pd.Es werden alle Threads in der Area komplett angezeigt.

:pt.:hp2.Threads mit ungelesenen Messages:ehp2.
:pd.Es werden nur die Threads in der Area angezeigt, die mindestens
eine ungelesene Message enthalten. Die einzelnen Threads werden komplett
angezeigt.

:pt.:hp2.Nur ungelesene Messages:ehp2.
:pd.Es werden nur die ungelesenen Messages angezeigt. Falls mehrere ungelesene
Messages im Thread l�ckenlos zusammenh�ngen, werden sie auch so dargestellt.
Wenn ein Thread durch eine gelesene Message unterbrochen ist, werden die
beiden Teile als zwei Threads angezeigt.
:eparml.
:p.
Der jeweilige Darstellungsmodus wird �ber das Kontextmen� der Threadliste
ausgew�hlt. Der Default-Modus wird im Einstellungs-Notebook der Threadliste festgelegt.
:p.

:h3 id=thmani.Manipulationsm�glichkeiten
:p.
Wenn Sie die Messages in der Threadliste manipulieren, dann manipulieren Sie
immer :hp2.Threads:ehp2. oder :hp2.Teile von Threads:ehp2.. Das bedeutet, da�
nicht nur eine einzelne Message betroffen ist, sondern auch alle nachfolgenden
Antworten darauf.
:p.
"Geschwister"-Threads oder Messages vor der ausgew�hlten Message sind jedoch
:hp2.nicht:ehp2. betroffen
:p.
Es kann nur ein Thread zu einem Zeitpunkt manipuliert werden. Dies ist eine
Beschr�nkung von OS/2.
:p.
Sie k�nnen
:ul compact.
:li.Threads l�schen
:li.Threads in eine andere Area verschieben
:li.Threads in eine andere Area kopieren
:li.Threads in eine Datei exportieren
:li.Threads drucken
:li.Threads als "gelesen" markieren
:li.Threads aufklappen
:eul.

:h3 id=thlink.Reply-Linker
:p.
FleetStreet wertet ausschlie�lich die Link-Informationen aus, die in der
Messagebase stehen, linkt also nicht selbst. F�r das Linken mu� ein
anderes Programm verwendet werden, am besten im Mailer-Batch direkt nach
dem Tossen der Messages.
:p.
Reply-Linker sind u.a.&colon.
:ul compact.
:li.Squish
:li.SqmLink
:li.SqLink
:li.QQLink
:eul.
:p.
Squish 1.01 linkt die Messages anhand der Themenzeile. Das hat den Vorteil, da�
auch Messages ohne MSGID/REPLY-Kludge zusammengelinkt werden k�nnen. Nachteil
ist jedoch, da� bei einer �nderung des Themas die Kette aufrei�t. Man kann
auch nicht anhand der Links erkennen, wer wem antwortet, und ob eine Message
evtl. mehrere Antworten hat (die Replies sind immer linear gelinkt).
:p.
Squish 1.10, SqmLink, SqLink und QQLink linken die Messages anhand der MSGID/REPLY-
Kludges. So kann eine Message wirklich exakt der Originalmessage zugeordnet
werden. Auch bei einer �nderung des Themas bleiben die Links bestehen.
Die Threads sind dann nicht mehr linear, sondern baumartig angeordnet.
Die Squish-Messagebase erlaubt pro Message maximal 10 Verweise auf andere
Messages.
:p.
Der Nachteil dieses Verfahrens ist, da� Messages nicht verbunden
werden k�nnen, wenn die Antwort keine REPLY-Kludge enth�lt (z.B. wenn sie mit
QWK-Readern verfasst wurde, �ber ein Gateway in die Area kommt oder
die REPLY-Kludge nicht exakt dem Standard entsprechend erzeugt wurde).

:h3 id=markmsg.Aufholen
:p.Wenn Sie FleetStreet noch nicht verwendet haben, um eine bestimmte Message-Base
zu lesen, sind zun�chst alle Messages als "ungelesen" markiert. Deshalb werden
in der Themen-Liste alle Messages in der Area angezeigt. Da Sie die Messages
aber schon gelesen haben k�nnten, erm�glicht dieser Men�punkt, alle Messages
als "gelesen" zu markieren. Nachdem Sie dies getan haben, zeigt
die Themen-Liste nur noch die Messages an, die wirklich ungelesen sind, also Messages
die danach in die Area getosst wurden.
:p.Diese Funktion wird �blicherweise nur ben�tigt, nachdem Sie zu FleetStreet gewechselt
haben und alte Message-Areas auf den neuesten Stand bringen wollen. FleetStreet
verwaltet in Zukunft das "gelesen"-Attribut automatisch.

.* @2@ **********************************************************************
:h2 id=bookmarks.Merkerliste
:p.Das Merkerfenster enth�lt drei Arten von Messages&colon.
:ul compact.
:li.Suchergebnisse
:li.Ergebnisse der Suche nach pers�nlicher Post
:li.Markierte Messages
:li.Unversandte Messages
:eul.
:p.
Verwenden Sie das Kontextmen� der Liste, um zwischen den drei Ansichten umzuschalten.
:p.
Wenn Sie "Inhalt speichern" im Kontextmen� w�hlen, wird der Inhalt der Liste
auf der Platte gespeichert, wenn Sie FleetStreet beenden. Er wird wieder geladen,
wenn Sie FleetStreet neu starten.
:p.
Klicken Sie eine Message an oder dr�cken Sie den :hp2.Zur Message:ehp2.-Knopf, um
die komplette Message anzuzeigen.
:p.
Dr�cken Sie den :hp2.Alle entfernen:ehp2.-Knopf, um die aktuelle Ansicht zu leeren,
d.h. alle Messages der aktuellen Ansicht werden aus der Liste entfernt.
:p.
Verwenden Sie das Kontextmen� einer Message, um
:ul compact.
:li.die Message zu l�schen
:li.die Message zu exportieren
:li.die Message zu drucken
:li.die Message in eine andere Area zu verschieben
:li.die Message in eine andere Area zu kopieren
:li.die Message aus der Liste zu entfernen
:eul.

.* @2@ **********************************************************************
:h2.Suchfunktion
:p.Mit der Suchfunktion k�nnen die Messages einer oder mehrerer Areas nach einer
bestimmten Zeichenkette durchsucht werden.
:p.Der Suchdialog wird u.a. �ber den Men�punkt "Message/Suchen" ge�ffnet. Geben
Sie dann den gesuchten Text und alle Suchoptionen an. Mit dem Button "Start"
wird die Suche gestartet. Die Suche findet im Hintergrund statt. Jedesmal wenn eine
Area durchsucht wurde, werden die Suchergebnisse in der
:link reftype=hd refid=bookmarks.Merkerliste:elink. (Ansicht "Suchergebnisse") eingetragen.
:p.Mit dem Suchdialog wird auch die Suche nach pers�nlichen Messages gestartet. Die
Suchergebnisse werden ebenfalls in der Merkerliste eingetragen (Ansicht "pers�nliche
Messages").

:h3.Regul�re Ausdr�cke
:p.
(Ausschnitt aus dem "VisualAge C++ Programming Guide")&colon.
:p.
Regular Expressions (REs) are used to determine if a
character string of interest is matched somewhere in a set of
character strings.  You can specify more than one character
string for which you wish to determine if a match exists.
:p.
Within an RE&colon.
:ul.
:li.An ordinary character matches itself. The simplest form
of regular expression is a string of characters with no
special meaning.
:li.A special character preceded by a backslash matches
itself. The special characters are&colon.
.br
          . [ \ * ^ $ ( ) + ? { |

:li.A period (.) without a backslash matches any character
except the null character.
:li.An expression within square brackets ([ ]), called a
bracket expression, matches one or more characters or
collating elements.
:eul.
:p.
:hp2.Bracket Expressions:ehp2.
:p.
A bracket expression itself contains one or more expressions
that represent characters, collating symbols, equivalence or
character classes, or range expressions&colon.

:parml.
:pt.[string]
:pd. Matches any of the characters specified. For example,
[abc] matches any of a, b, or c.

:pt.[^string]
:pd.Does not match any of the characters in string. The
caret immediately following the left bracket ([)
negates the characters that follow.  For example,
[^abc] matches any character or collating element
except a, b, or c.

:pt.[collat_sym-collat_sym]
:pd.Matches any collating elements that fall between the
two specified collating symbols, inclusive. The two
symbols must be different, and the second symbol
must collate equal to or higher than the first. For
example, in the "C" locale, [r-t] would match any
of r, s, or t.
:p.
Note&colon.  To treat the hyphen (-) as itself, place it
either first or last in the bracket expression, for
example: [-rt] or [rt-]. Both of these expressions
would match -, r, or t.

:pt.[[.collat_symbl.]]
:pd.Matches the collating element represented by the
specified single or multicharacter collating symbol
collat_symbl. For example, assuming that <ch> is the
collating symbol for ch in the current locale,
[[.ch.]] matches the character sequence ch. (In
contrast, [ch] matches c or h.) If collat_symbl is
not a collating element in the current locale, or if it
has no characters associated with it, it is treated as
an invalid expression.

:pt.[[=collat_symbl=]]
:pd.Matches all collating elements that have a weight
equivalent to the specified single or multicharacter
collating symbol collat_symbl. For example, assuming
a, �, and � belong to the same equivalence class,
[[=a=]] matches any of the three.  If the collating
symbol does not have any equivalents, it is treated as
a collating symbol and matches its corresponding
collating element (as for [&dot.&dot.]).

:pt.[[&colon.char_class&colon.]]
:pd.Matches any characters that belong to the specified
character class char_class. For example,
[[&colon.alnum&colon.]] matches all alphanumeric characters
(characters for which isalnum would return
nonzero).
:p.
Note&colon. To use the right bracket (]) in a bracket expression,
you must specify it immediately following the left bracket ([)
or caret symbol (^).  For example, []x] matches the
characters ] and x; [^]x] does not match ] or x; [x]] is
not valid.
:eparml.
:p.
You can combine characters, special characters, and bracket
expressions to form REs that match multiple characters and
subexpressions. When you concatenate the characters and
expressions, the resulting RE matches any string that matches
each component within the RE. For example, cd matches
characters 3 and 4 of the string abcde; ab[[&colon.digit&colon.]]
matches ab3 but not abc. You can optionally enclose the
concatenation in parentheses.
:p.
You can also use other syntax within an RE to control what it
matches&colon.
:parml.
:pt.(expression)
:pd.Matches whatever expression matches.  You only need
to enclose an expression in these delimiters to use
operators (such as * or +) on it and to denote
subexpressions for backreferencing (explained later in
this section).

:pt.expression*
:pd.Matches zero or more consecutive occurrences of what
expression matches. expression can be a single
character or collating symbol or a subexpression.
For example, [ab]*
matches ab and ababab; b*cd matches characters
3 to 7 of cabbbcdeb.

:pt.expression{m}
:pd.Matches exactly m occurrences of what expression
matches. expression can be a single character or
collating symbol or a subexpression.
For example, c{3} matches characters
5 through 7 of ababccccd (the first 3 c characters
only).

:pt.expression{m,}
:pd.Matches at least m occurrences of what expression
matches. expression can be a single character or
collating symbol or a subexpression.
For example, (ab){3,} matches
abababab, but does not match ababac.

:pt.expression{m,u}
:pd.Matches any number of occurrences, between m and u
inclusive, of what expression matches. expression can
be a single character or collating symbol or a
subexpression. For example, bc{1,3} matches characters
2 through 4 of abccd and characters 3 through 6 of abbcccccd

:pt.^expression
:pd.Matches only sequences that match expression that
start at the first character of a string or after a
new-line character. For example, ^ab matches ab in
the string abcdef, but does not match it in the string
cdefab. The expression can be the entire RE or any
subexpression of it.
:p.
Portability Note&colon. When ^ is the first character of a
subexpression, other implemenations could interpret it
as a literal character. To ensure portability, avoid
using ^ at the beginning of a subexpression; to use it
as a literal character, precede it with a backslash.

:pt.expression$
:pd.Matches only sequences that match expression that
end the string or that precede the new-line character.
For example, ab$ matches ab in the string cdefab but
does not match it in the string abcdef. The expression
must be the entire RE.
:p.
Portability Note&colon. When $ is the last character of a
subexpression, it is treated as a literal character. Other
implementations could interpret is as described above.
To ensure portability, avoid using $ at the end of a
subexpression; to use it as a literal character, precede
it with a backslash.

:pt.^expression$
:pd.Matches only an entire string, or an entire line. For
example, ^abcde$ matches only abcde.

:pt.expression+
:pd.Matches what one or more occurrences of expression
matches.  For example, a+(bc) matches aaaaabc;
(bc)+ matches characters 1 through 6 of
bcbcbcbb.

:pt.expression?
:pd.Matches zero or one consecutive occurrences of what
expression matches. For example, b?c matches
character 2 of acabbb (zero occurrences of b
followed by c).

:pt.expression|expression
:pd.Matches a string that matches either expression.  For
example, a((bc)|d) matches both abd and ad.
:eparml.
:p.
The RE syntax specifiers are processed in a specific order.
The order of precedence for REs is described below, from highest
to lowest. The specifiers in each category are also listed in
order of precedence.
:table cols='33 33'.
:row.
:c.Collation-related bracket
:c.[==]  [&colon.&colon.]  [&dot.&dot.]
:row.
:c.symbols
:row.
:c.Special characters
:c.\spec_char
:row.
:c.Bracket expressions
:c.[�
:row.
:c.Grouping
:c.()
:row.
:c.Repetition
:c.*  +  ?  {m}  {m,}  {m,n}
:row.
:c.Concetenation
:row.
:c.Anchoring
:c.^  $
:row.
:c.Alternation
:c.|
:etable.

:p.
Copyright International Business Machines Corporation, 1992, 1995. All rights reserved.

.* @1@ **********************************************************************
:h1.Bedienung f�r Fortgeschrittene

.* @2@ **********************************************************************
:h2 id=enteraddr.Eingabe von FTN-Adressen
:p.Beim Schreiben von Nachrichten m�ssen Sie die Netzadresse des Empf�ngers
angeben. Dies geschieht �blicherweise durch Angabe der vollen 3D-  bzw. 4D-
Adresse.
:p.FleetStreet unterst�tzt Sie jedoch bei der Eingabe, indem es unvollst�ndige
Adressen erg�nzt. Die fehlenden Teile der Adresse werden automatisch durch die
entsprechenden Teile Ihrer eigenen Adresse ersetzt.
:p.Die folgenden Beispiele zeigen diese Erg�nzung. Als Default-Adresse wird
2&colon.2490/2520.17 verwendet&colon.
:table cols='12 15 30'.
:row.
:c.Eingabe
:c.Resultat
:c.Kommentar
:row.
:c.2&colon.2490/2520
:c.2&colon.2490/2520
:c.3D - Addresse angegeben
:row.
:c.2520
:c.2&colon.2490/2520
:c.Nodenummer angegeben, Netz &amp. Zone �bernommen
:row.
:c.247/2099
:c.2&colon.247/2099
:c.Netz &amp. Node angegeben, Zone �bernommen
:row.
:c.1030.42
:c.2&colon.2490/1030.42
:c.Node &amp. Point angegeben, Zone &amp. Netz �bernommen
:row.
:c..42
:c.2&colon.2490/2520.42
:c.Point angegeben, gleicher Boss-Node
:etable.
:p.:hp2.Die globalen Regeln sind&colon.:ehp2.
:ol.
:li.Eine einzelne Zahl gilt als "Node".
:li.Wenn keine Pointnummer angegeben ist, wird "0" angenommen.
:eol.

.* @2@ **********************************************************************
:h2.Toolbar
:p.Die FleetStreet-Toolbar kann in zwei verschiedenen Gr��en und an 4
verschiedenen Positionen dargestellt werden. Diese Optionen werden mit Hilfe
des Kontextmen�s der Toolbar eingestellt. �ffnen Sie dieses, indem Sie mit der
rechten Maustaste auf eine freie Stelle innerhalb der Umrandung der Toolbar
klicken.


.* @2@ **********************************************************************
:h2.Echo-Manager
:p.Der Echo-Manager dient zur vereinfachten Kommunikation mit dem Area-Verwaltungs-Programm
des Uplinks.
:p.�blicherweise bestellt man Echos, indem man Netmails an den Area-Verwalter des Uplinks
schickt. Dabei steht in der Themenzeile ein Passwort, der Message-Text enth�lt die Namen
der gew�nschten Echos.
:p.Auf die gleiche Art kann man die Echos wieder abbestellen, eine Liste der verf�gbaren
Echos anfordern etc.
:p.Der Echo-Manager von FleetStreet erleichtert diese Vorg�nge&colon.
:ul compact.
:li.Man mu� Name, Adresse und Passwort des Area-Verwalters nicht von Hand eingeben,
:li.Man kann Areas einfach per Kontextmen� ausw�hlen und bestellen bzw. abbestellen,
:li.Die Messages an den Area-Verwalter werden automatisch erstellt,
:li.Die bestellten Echos werden in der Konfigurationsdatei des Tossers eingetragen.
:eul.

:h3.Liste der Echos
:p.Damit der Echo-Manager arbeiten kann, ben�tigt er eine Liste der Echos, die
beim Uplink verf�gbar sind. Falls man diese noch nicht hat, mu� man sie zun�chst
per Message an den Uplink manuell anfordern.
:p.Die Antwort des Area-Verwalters enth�lt eine Liste der Echos. Diese mu� man
nun dem Echo-Manager �bergeben. Dazu w�hlt man den Men�punkt "Setup/Areas extrahieren".
FleetStreet durchsucht nun die aktuelle Message nach Echo-Namen und verwendet diese
dann im Echo-Manager.
:p.FleetStreet merkt sich zu jeder Echo-Liste die Adresse des Absenders. Wenn man
Areas extrahiert, und es ist bereits eine Liste dieses Absenders vorhanden, so wird
die alte Liste durch die neue ersetzt. Ansonsten wird der Absender als neuer Uplink
aufgenommen.

:h3.Konfiguration
:p.Um mit dem Area-Verwalter des Uplinks zu kommunizieren, wird dessen Name und
Passwort ben�tigt. �ffnen Sie den Echo-Manager, �ffnen Sie das Kontextmen� der Liste
und w�hlen Sie "Einstellungen". Auf der ersten Seite des Einstellungs-Notebooks
ist eine Liste der bekannten Uplinks. F�r diese kann jeweils der Name und das
Passwort f�r den Area-Verwalter eingetragen werden.
:p.
:hp8.Achtung&colon.:ehp8. Das Eintragen des Namens und des Passworts ist zwingend
notwendig, um den Echo-Manager sp�ter verwenden zu k�nnen.

:h3.Verwendung des Echo-Managers
:p.Um mit Hilfe des Echo-Managers Echos zu bestellen oder abzubestellen, gehen Sie
wie folgt vor&colon.
:ol.
:li.Stellen Sie als Ihre aktuelle Adresse die Adresse ein, mit der Sie den Uplink
anschreiben wollen. Wenn Sie mehrere Uplinks bzw. Netze und f�r jeden Uplink
eine eigene Netmail-Area haben, wechseln Sie in die f�r den Uplink passende Area.
Wenn Sie nur einen Uplink haben, wechseln Sie in die Netmail-Area.
:li.�ffnen Sie �ber das Men� den Echo-Manager. Nun sollte die Echo-Liste des
Uplinks angezeigt werden. Wenn die Liste leer ist, dann m�ssen Sie zun�chst
manuell eine Liste beim Uplink anfordern.
:li.�ffnen Sie f�r das zu bestellende Echo das Kontextmen�. W�hlen Sie dort
:hp2."bestellen":ehp2..
:li.Das Bestellen bzw. Abbestellen von weiteren Echos funktioniert analog. Die
gew�nschte Aktion wird jeweils in der Zeile des Echos angezeigt.
:li.Dr�cken Sie :hp2.OK:ehp2.. Die Netmail an den Uplink wird daraufhin erstellt.
:eol.

:h3.Erweiterungs-DLL
:p.Squish kann neue Echos nicht automatisch in seine Konfigurationsdatei eintragen.
Aus diesem Grund kann FleetStreet beim Bestellen oder Abbestellen eines Echos
eine DLL laden und Funktionen darin aufrufen. Die DLL kann dann die Echos
eintragen.
:p.
Die DLL, die verwendet werden soll, kann im Einstellungs-Notebook des
Echo-Managers angegeben werden.
:p.
Die Datei :hp2.FLTCF_SQ.DLL:ehp2., die mit FleetStreet mitgeliefert wird, kann
als Erweiterungs-DLL verwendet werden, wenn (und nur wenn) Sie Squish als
Tosser mit einer Standard-SQUISH.CFG verwenden. F�r andere Konstellationen
(z.B. Point-Pakete, andere Tosser etc.) sind evtl. entsprechende DLLs von
Drittanbietern verf�gbar.
:p.
Technische Informationen zum Erstellen von Erweiterungs-DLLs k�nnen beim
:link reftype=hd refid=support.Autor:elink. angefordert werden.


.* @2@ **********************************************************************
:h2 id=cclists.Benutzung von Verteilerlisten
:p.Benutzung einer Verteilerliste bedeuted, da� Sie die gleiche Message an
verschiedene Empf�nger schicken. Verteilerlisten k�nnen nur f�r Netmail verwendet
werden.
:p.Stellen Sie sich das folgende Beispiel vor&colon.
:p.Sie stellen ein Newsletter zusammen, das Sie regelm��ig einer Anzahl von Leuten
schicken. Um diese Aufgabe mit FleetStreet zu l�sen, k�nnen Sie eine Verteilerliste
namens "Newsletter" erstellen. Nun k�nnen Sie alle Leute in dieser Liste aufnehmen,
die das Newsletter erhalten sollen.
:p.Wenn Sie jetzt das Newsletter per Netmail verschicken wollen, stellen Sie ihre
Message zusammen, wie Sie es sonst auch tun w�rden. Anstatt jedoch einen Empf�nger
einzutragen, w�hlen Sie :hp2.Verteiler:ehp2. aus dem Men�. Sie k�nnen nun Ihre
Verteilerliste "Newsletter" ausw�hlen, die sie vorher zusammengestellt haben.
Es erscheint der Text "*** Newsletter ***" und zeigt an, da� sie diesen Verteiler
verwenden.
:p.Wenn Sie die n�chste Ausgabe des Newsletters verschicken, m�ssen Sie keine neue
Verteilerliste erstellen. Sie k�nnen einfach die vorherige wiederverwenden.

:h3.Verwalten von Verteilerlisten
:p.Alle Verteilerlisten befinden sich im Verteilerlisten-Ordner. Dieser wird mit
dem Men�punkt :hp2."Setup/Verteilerlisten":ehp2. ge�ffnet. Die Verteilerlisten
sind als Icons dargestellt.
:p.Verwenden Sie das Kontextmen� der Liste, um eine Liste zu l�schen oder eine neue
Liste zu erstellen. Ein Doppel-Klick auf eine Liste �ffnet diese, um den Inhalt
anzuzeigen und zu ver�ndern.
:p.Halten Sie die ALT-Taste gedr�ckt und klicken Sie auf eine Verteilerliste, um
deren Namen zu �ndern.

:h3.Import
:p.Der Inhalt einer Textdatei kann in eine Verteilerliste importiert werden.
:p.Die Datei mu� eine Textdatei im folgenden Format sein&colon.
:ul compact.
:li.Jede Zeile enth�lt genau einen Namen und eine Adresse
:li.Die Felder einer Zeile sind durch mindestens ein Leerzeichen getrennt
:li.Leerzeilen werden ignoriert
:li.Zeilen mit einem Semikolon am Anfang werden ignoriert
:li.Zeilen in einem ung�ltigen Format werden ignoriert.
:eul.
:p.
Beispiel&colon.
:xmp.
; Kommentar
Michael Hohner 2&colon.2490/2520.17
Hans Dampf 1&colon.234/567
:exmp.

:h3.Schnellverteiler
:p."Normale" Verteilerlisten werden beim Programmende dauerhaft gespeichert und
stehen immer wieder zur Verf�gung. Dies ist jedoch manchmal nicht w�nschenswert.
Aus diesem Grund gibt es die Funktion :hp2."Schnellverteiler":ehp2. im Men� :hp2."Spezial":ehp2..
:p.Der Schnellverteiler hat im Prinzip die gleiche Funktion wie eine normale
Verteilerliste. Sie wird jedoch erst beim Schreiben der Message erstellt. Au�erdem
wird der Inhalt nach dem Speichern der Messages wieder verworfen.

.* @2@ **********************************************************************
:h2 id=crosspost.'Crossposten' von Messages
:p.Crossposten bedeutet, da� die gleiche Message in mehr als einer Area gespeichert
wird.
:p.Sie k�nnen das Crossposten aktivieren, indem Sie :hp2.Crossposten:ehp2. im Men�
w�hlen. Diese Funktion steht nur zur Verf�gung, wenn Sie gerade eine Message
schreiben.
:p.Die Area-Liste wird ge�ffnet, und Sie
k�nnen die Areas ausw�hlen, in denen die Message gespeichert werden soll.
:p.:hp8.Beachte&colon.:ehp8. Sie m�ssen die aktuelle Area in der Liste nicht
ausw�hlen. Die Message wird immer in der aktuellen Area gespeichert. W�hlen Sie
einfach nur die zus�tzlichen Areas aus.
:p.Wenn der Crosspost-Modus aktiv ist, wird ein H�kchen vor dem :hp2.Crossposten:ehp2.
im Men� angezeigt.
:p.Sie k�nnen den Crosspost-Modus wieder abschalten, indem Sie :hp2.Crossposten:ehp2.
noch einmal im Men� w�hlen. Das H�kchen verschwindet und zeigt an, da� Sie sich
wieder im normalen Editier-Modus befinden.

.* @2@ **********************************************************************
:h2.Kurznamen
:p.Bei FleetStreet ist es m�glich, Kurznamen f�r Benutzer zu definieren, an die
man h�ufig schreibt.
:p.
Um diese Kurznamen zu verwenden, geben Sie den Kurznamen einfach im Empf�nger-Feld
ein und dr�cken Sie "Enter". Der Kurzname wird automatisch durch den echten Namen
ersetzt, und die Adresse des Benutzers wird im Adre�feld eingef�gt. Falls Sie ein
Thema f�r den Kurznamen definiert haben, wird dieses ebenfalls ins Thema-Feld
eingef�gt.
:p.:hp2.Um einen Kurznamen zu definieren, tun Sie folgendes&colon.:ehp2.
:ol compact.
:li.Dr�cken Sie den "Hinzuf�gen"-Knopf,
:li.F�llen Sie die Felder aus,
:li.Dr�cken Sie "OK".
:eol.
:p.:hp2.Um einen Kurznamen zu �ndern, tun Sie folgendes&colon.:ehp2.
:ol compact.
:li.W�hlen Sie den Kurznamen in der Liste aus,
:li.Dr�cken Sie den "�ndern"-Knopf, oder Klicken Sie den Eintrag zweimal an,
:li.Nehmen Sie die �nderungen vor,
:li.Dr�cken Sie "OK".
:eol.
:p.:hp2.Um einen Kurznamen zu l�schen, tun Sie folgendes&colon.:ehp2.
:ol compact.
:li.W�hlen Sie den Kurznamen in der Liste aus,
:li.Dr�cken Sie den "L�schen"-Knopf.
:eol.

.* @2@ **********************************************************************
:h2 id=nodelists.Benutzung von Nodelisten
:p.
:hp2.Worum gehts?:ehp2.
:p.Nodelisten enthalten unter anderem den Namen und die Netz-Adresse der
Teilnehmer eines Netzes. FleetStreet kann zu einem Namen eines Empf�ngers
die passende Adresse in einer solchen Nodeliste suchen und umgekehrt.
:p.Dazu ist die Nodeliste in kompilierter Form im Format "Version 7" n�tig.
Dieses Format wird u.a. von FastLst erzeugt. FleetStreet ben�tigt das Datenfile
und den Sysop-Index. Der Nodelisten-Compiler ist entsprechend zu konfigurieren.
:p.:hp2.Wie findet FleetStreet die Nodelisten?:ehp2.
:p.Im Setup gibt es Notebook-Seiten f�r die Nodelisten. F�r jede Nodeliste
mu� ein "Domain" angelegt werden. Ein Domain-Eintrag enth�lt den Namen
des Domains und die Pfadnamen der Nodelist-Dateien. Es m�ssen jeweils
die vollen Pfadnamen inclusive Laufwerk und Extension angegeben werden.
:p.:hp2.Was ist noch zu tun?:ehp2.
:p.Wahlweise k�nnen auch Bezeichnungen f�r
die Nodelist-Flags angegeben werden.
:p.:hp2.Wie kann ich die Nodelisten verwenden?:ehp2.
:p.Geben Sie beim Schreiben einer Netmail den kompletten Namen des Empf�ngers
oder einen Teil des Nachnamens ein. Dr�cken Sie dann ENTER. Wenn der Name
gefunden wird, wird die Adresse sofort im Adre�feld eingetragen. Wird er
mehrmals gefunden, so erscheint ein Auswahl-Dialog.
:p.Bei einigen weiteren Dialogen, bei denen die Eingabe einer Adresse verlangt
wird, gibt es einen "?"-Button. Dr�cken Sie diesen, um f�r einen schon eingegebenen
Namen die zugeh�rige Adresse zu suchen.

.* @2@ **********************************************************************
:h2 id=templates.Schablonen
:p.Message-Schablonen werden verwendet, um ein bestimmtes Aussehen von
neuen Messages, Antworten oder Weiterleitungen zu definieren. Wenn Sie eine neue
Message schreiben, antworten oder weiterleiten, werden die entsprechenden
Schablonenteile mit dem Messagetext kombiniert. Spezielle
:link reftype=hd refid=tokens.K�rzel:elink. in der Message-Schablone
werden durch bestimmte Teile der Original-Message ersetzt, wenn die
Schablone verarbeitet wird.
:p.Die Message-Schablone wird verarbeitet, wenn Sie das Editorfenster zum
ersten Mal betreten, wenn Sie eine Message schreiben, antworten oder
weiterleiten. Wenn Sie bis dahin ein Thema oder einen Empf�ngernamen
eingegeben haben, k�nnen diese f�r die K�rzel-Ersetzung verwendet
werden. Wenn Sie diese noch nicht eingegeben haben, werden die
entsprechenden K�rzel "leer" ersetzt.
:p.:hp2.Allgemeine Reihenfolge&colon.:ehp2.
.br
:hp2.Neue Message&colon.:ehp2.
:xmp.
[Kopftext]
[Message-Text]
[Fu�text]
:exmp.
:p.:hp2.Antwort&colon.:ehp2.
:xmp.
[Kopftext]
[Antwort-Text]
[Message-Text]
[Fu�text]
:exmp.
:p.:hp2.Antwort in anderer Area&colon.:ehp2.
:xmp.
[Antwort in anderer Area-Text]
[Kopftext]
[Antwort-Text]
[Message-Text]
[Fu�text]
:exmp.
:p.:hp2.Crosspost&colon.:ehp2.
:xmp.
[Crosspost-Text]
[Kopftext]
[Message-Text]
[Fu�text]
:exmp.
:p.:hp2.Verteiler-Text&colon.:ehp2.
:xmp.
[Verteiler-Text]
[Kopftext]
[Message-Text]
[Fu�text]
:exmp.
:p.:hp2.Weiterleiten&colon.:ehp2.
:xmp.
[Weiterleiten-Text]
[Text der Original-Message]
[Weiterleiten-Fu�Text]
[Kopftext]
[Fu�text]
:exmp.
:p.:hp2.Zuordnen von Schablonen zu Areas:ehp2.
:p.Sie k�nnen eine Schablone zur Arealiste ziehen und auf einer Area fallen
lassen. Die zugeordnete Schablone wird dann verwendet, wenn Messages in der Zielarea
erzeugt werden.
:p.
:artwork align=center name='atttpl.bmp'.

:h3 id=tokens.K�rzel
:p.The folgenden K�rzel sind in Message-Schablonen verf�gbar&colon.
:parml break=none.
:pt.:hp2.%T:ehp2.
:pd.Name des adressierten Benutzers (Original-Message)
:pt.:hp2.%Z:ehp2.
:pd.Vorname des adressierten Benutzers (Original-Message)
:pt.:hp2.%R:ehp2.
:pd.Adresse des adressierten Benutzers (Original-Message). Bei Antworten oder
Weiterleiten von Echomail wird dieses K�rzel ignoriert.
:pt.:hp2.%O:ehp2.
:pd.Name des adressierten Benutzers (neue Message)
:pt.:hp2.%P:ehp2.
:pd.Vorname des adressierten Benutzers (neue Message)
:pt.:hp2.%F:ehp2.
:pd.Name des Absenders
:pt.:hp2.%G:ehp2.
:pd.Vorname des Absenders
:pt.:hp2.%J:ehp2.
:pd.Adresse des Absenders
:pt.:hp2.%A:ehp2.
:pd.Area-Kennung. Wenn eine Message gecrossposted [was f�r ein Wort!] wird, ist
dies die Ziel-Area. Wenn in einer anderen Area geantwortet wird oder eine Message
weitergeleitet wird, ist dies die Original-Area.
:pt.:hp2.%E:ehp2.
:pd.Area-Beschreibung. Wenn eine Message gecrossposted wird, ist
dies die Ziel-Area. Wenn in einer anderen Area geantwortet wird oder eine Message
weitergeleitet wird, ist dies die Original-Area.
:pt.:hp2.%U:ehp2.
:pd.Ihr eigener Name
:pt.:hp2.%I:ehp2.
:pd.Ihr Vorname
:pt.:hp2.%W:ehp2.
:pd.Ihre eigene Adresse
:pt.:hp2.%C:ehp2.
:pd.Namen der Benutzer in einer Verteilerliste
:pt.:hp2.%D:ehp2.
:pd.Datum der Message, auf die Sie antworten
:pt.:hp2.%M:ehp2.
:pd.Zeit der Message, auf die Sie antworten
:pt.:hp2.%S:ehp2.
:pd.Thema der Message, auf die Sie antworten
:pt.:hp2.%%:ehp2.
:pd.Ein %
:eparml.


.* @2@ **********************************************************************
:h2.Laufwerksumsetzung
:p.
Wenn sich die Message-Base auf einem anderen Rechner in einem lokalen Netz
befindet, dann ist es w�nschenswert, nur eine einzige Tosser-Konfigurationsdatei auf dem
gleichen Rechner zu unterhalten. Wenn Sie diese Tosser-Konfigurationsdatei in FleetStreet
verwenden, und die Netz-Laufwerke sind auf andere lokale Laufwerksbuchstaben
gemountet, so w�rde FleetStreet die falschen Laufwerksbuchstaben beim
Zugriff auf die Message-Base verwenden.
:p.
Die Laufwerks-Umsetzung bietet eine L�sung f�r das Problem. Sie k�nnen Netzlaufwerken
andere lokale Laufwerksbuchstaben zuweisen. FleetStreet ersetzt die Laufwerks-
Buchstaben von Netzlaufwerken durch die zugewiesenen Buchstaben, bevor auf
die Messagebase zugegriffen wird.
:p.
Beispiel&colon.
:p.
Die Messagebase befindet sich auf dem Rechner A auf Laufwerk D&colon.. FleetStreet
l�uft auf Rechner B, das Laufwerk D&colon. des Rechners A ist hier auf Laufwerk
E&colon. gemountet. Die Tosser-Konfigurationsdatei befindet sich ebenfalls auf Rechner A, die
Area-Filenamen sind deshalb mit D&colon. als Laufwerksbuchstaben angegeben.
:p.
Wenn FleetStreet so installiert wird, da� Laufwerk D&colon.  durch Laufwerk E&colon. ersetzt
wird, werden die Laufwerksbuchstaben der Area-Files aus der Tosser-Konfigurationsdatei
durch E&colon. ersetzt, die korrekten Filenamen werden verwendet.
:p.
:hp8.Achtung&colon.:ehp8.  Es werden nur die Laufwerksbuchstaben von Filenamen umgesetzt, die
aus der Tosser-Konfigurationsdatei gelesen wurden. Wenn Sie in FleetStreet Areas erstellen,
m�ssen Sie selbst die korrekten Laufwerke angeben, FleetStreet nimmt keine
Umsetzung bei diesen Filenamen vor.

.* @2@ **********************************************************************
:h2 id=colorsetup.Farben und Schriftarten �ndern
:p.Sie werden kein Men� oder einen Dialog finden, um die Farben und Schriftarten
des Hauptfensters zu �ndern. Das liegt daran, da� FleetStreet die WPS-Objekte
nutzt, um diese Dinge einzustellen.
:parml tsize=25 break=none.
:pt.:hp2.Die Farbpalette:ehp2.
:pd.�ffnen Sie eine Farbpalette, ziehen Sie eine Farbe �ber das entsprechende Element
des FleetStreet-Fensters und lassen Sie sie dort 'fallen'. Um die
Vordergrundfarbe (Schriftfarbe) zu �ndern, halten Sie dabei die CTRL- (bzw.
STRG-) Taste gedr�ckt.
:pt.:hp2.Die Schriftpalette:ehp2.
:pd.�ffnen Sie eine Schriftpalette, ziehen Sie eine Schriftart �ber das entsprechende
Element des FleetStreet-Fensters und lassen Sie es dort 'fallen'.
:eparml.

.* @2@ **********************************************************************
:h2.Import, Export
:p.
Wenn Sie eine Message schreiben, k�nnen Sie eine Textdatei an der aktuellen
Cursorposition einf�gen.
:ul.
:li.Ziehen Sie die Datei aus einem WPS-Ordner auf das Editor-Fenster.
:artwork align=center name='dropfile.bmp'.
:li.Verwenden Sie die Import-Funktion im "Datei"-Men�.
:eul.
:p.
Beim Lesen k�nnen Sie eine Message in eine normale Textdatei schreiben.
:ul.
:li.Ziehen Sie die Message zu einem WPS-Ordner. Beginnen Sie mit dem Ziehen
�ber dem Editor-Fenster.
:li.Verwenden Sie den Export-Dialog im "Datei"-Men�.
:eul.


.* @1@ **********************************************************************
:h1.Weiterf�hrende Themen, Tips
:p.
Die folgenden Themen sind evtl. interessant, wenn Sie sich in FleetStreet
eingearbeitet haben&colon.
:ul.
:li.:link reftype=hd refid=multinst.Mehrere Instanzen:elink.
:li.:link reftype=hd refid=perform.Verbesserung der Performance:elink.
:li.:link reftype=hd refid=multuser.Multi-User-Betrieb:elink.
:eul.

.* ************************** Mehrere Instanzen ******************************
.* @2@ **********************************************************************
:h2 id=multinst.Mehrere Instanzen
:p.
Sie k�nnen FleetStreet mehr als einmal zur gleichen Zeit starten. Dabei sind
folgende Punkte zu beachten&colon.
:ul.
:li.Nur die erste gestartete Instanz speichert beim Programmende ihre
Einstellungen. �ndern Sie das Setup deshalb nur in der ersten Instanz, falls
die �nderungen beibehalten werden sollen.
:li.Sekund�re Instanzen sind am :hp2.[*]:ehp2. in der Titelzeile
zu erkennen.
:li.Sie k�nnen nicht mehrere Instanzen von FleetStreet unterschiedlicher
Versionen laufen lassen. Ebenso k�nnen nicht eine englische und eine deutsche
Version gleichzeitig laufen. In einem solchen Fall werden f�r jede Instanz die
DLLs der ersten Instanz verwendet. Dies ist eine Einschr�nkung von OS/2. Das
Verhalten der sekund�ren Instanzen bei verschiedenen Versionen ist undefiniert.
:li.Sekund�re Instanzen haben keinen Pipe-Server
:eul.

.* ************************** Performance       ******************************
.* @2@ **********************************************************************
:h2 id=perform.Performance-Verbesserungen
:p.Die Performance von FleetStreet h�ngt in gro�em Ma�e von zwei Faktoren ab&colon.
Performance der MSGAPI und Performance der Platten. W�hrend die MSGAPI au�erhalb
unseres Einflusses steht, k�nnen �nderungen beim zweiten Punkt die Gesamt-
Performance von FleetStreet verbessern.
:p.Die Hinweise im einzelnen&colon.
:ul.
:li.Verwenden Sie falls m�glich Squish-Areas statt *.MSG-Areas.
:li.Packen Sie Squish-Areas regelm��ig mit SqPackP. Dadurch liegen die
einzelnen Messages l�ckenlos und in aufsteigender Reihenfolge in der Messagebase.
:li.Verwenden Sie HPFS.
:li.Schlie�en Sie Areas, die Sie nicht selbst lesen wollen, aus der
Arealiste aus. Aktivieren Sie "ausgeschlossene Areas verstecken".
:li.Lassen Sie Text - falls m�glich - nur im Header suchen, nicht
im Header &amp. Text.
:eul.

.* ************************** Multi-User   ***********************************
.* @2@ **********************************************************************
:h2 id=multuser.Multi-User-Betrieb
:p.Der Betrieb von FleetStreet f�r mehrere Benutzer ist mit Einschr�nkungen
m�glich. Gehen Sie wie folgt vor&colon.
:ol.
:li.Legen Sie ein Programm-Verzeichnis f�r FleetStreet an. Kopieren Sie die
EXE-Datei und alle DLLs in dieses Verzeichnis.
:li.Legen Sie f�r jeden Benutzer ein Konfigurations-Verzeichnis an.
:li.Erstellen Sie f�r jeden Benutzer ein Programm-Objekt. Tragen Sie beim
Programmnamen den kompletten Pfadnamen von FLTSTRT.EXE ein.
:li.Geben Sie den :link reftype=hd refid=cmdlin.Kommandozeilen-Parameter:elink. "-C" an,
der das Konfigurations-Verzeichnis f�r jeden Benutzer angibt, z.B. "-Cd&colon.\fleet\user1".
:li.FleetStreet kann nun f�r jeden Benutzer getrennt konfiguriert werden. Die
Konfigurations-Verzeichnisse enthalten dann jeweils einen eigenen Satz INI-Dateien.
:li.Sehen Sie f�r jeden Benutzer unterschiedliche Lastread-Offsets vor.
:eol.
:p.:hp2.Einschr�nkungen&colon.:ehp2.
:ul.
:li.Das "gelesen"-Flag existiert nur einmal pro Message, ist also f�r
alle Benutzer gleich.
:li.Das "private"-Flag wird nicht gesondert behandelt.
:eul.

.* ************************** Howto        ***********************************
.* @2@ **********************************************************************
:h2.Wie kann man...
:p.
Die folgenden Abschnitte beschreiben h�ufig verwendete Vorg�nge und wie man
Sie mit FleetStreet realisiert.

:h3.Dateien mit einer Message verschicken?
:p.
Dateien k�nnen zusammen mit einer Message verschickt werden. Die Namen der Dateien
werden dazu in der Themenzeile der Message eingetragen. Mehrere Dateinamen werden
durch mindestens ein Leerzeichen getrennt. Au�erdem ist es n�tig, das Message-Attribut
"Datei angeh�ngt" zu setzen, damit der Tosser bzw. Mailer die Themenzeile
entsprechend interpretiert.
:p.
Sie k�nnen Dateien von jedem WPS-Ordner auf die Themenzeile ziehen, um sie
an die Message anzuh�ngen. Die Dateinamen werden automatisch eingetragen, das
"Datei angeh�ngt"- Attribut eingeschaltet und eine Zusammenfassung der angeh�ngten
Dateien ausgegeben. Dies funktioniert nur beim Schreiben von Messages!
:artwork align=center name='attfile.bmp'.


.* ************************* Rexx ******************************************

.* @1@ **********************************************************************
:h1.Rexx-Skript-Programmierung
:p.FleetStreet hat die F�higkeit, Skripts in der Sprache Rexx auszuf�hren. Die
Sprache Rexx wird durch :hp2.vordefinierte Variablen:ehp2. und zus�tzliche
:hp2.Funktionen:ehp2. erweitert.

.* @2@ **********************************************************************
:h2.Programmier-Referenz
:p.Diese Referenz listet alle :link reftype=hd refid=rexxvar.vordefinierten Variablen:elink. und
zus�tzliche :link reftype=hd refid=rexxfunc.Funktionen:elink. auf.
:p.Lesen Sie die Online-Hilfe zu Standard-Rexx-Features.

:h3.Das FleetStreet-Environment
:p.Rexx-Skripts, die unter FleetStreet ablaufen, laufen nicht in ihrem
vorgegebenen Environment CMD.EXE ab. Das Rexx-Environment f�r Skripts unter
FleetStreet hei�t :hp2.FLEETSTREET:ehp2..
:p.Wenn Sie deshalb nicht-Rexx-Befehle in einem Skript verwenden, werden diese
Befehle von FleetStreet ausgewertet. Wenn sie wollen, da� CMD.EXE die Befehle
auswertet, dann m�ssen Sie explizit das CMD.EXE-Environment ansprechen mit dem
Rexx-Befehl :hp2.ADDRESS:ehp2..
:p.Beispiel&colon.
:xmp.
/* FALSCH! */
'dir'

/* richtig */
address CMD 'dir'

/* auch richtg */
address CMD
'dir'
address FLEETSTREET
:exmp.
:p.Lesen Sie in der Rexx-Onlinehilfe mehr �ber Rexx-Environments und den ADDRESS-Befehl.

.* ***************************** Variablen  ************************************

:h3 id=rexxvar.Vordefinierte Variablen
:p.Wenn ein Rexx-Skript gestartet wird, dann sind einige Variablen bereits mit
Werten vorbelegt. Diese Variablen und die dazugeh�rigen Werte k�nnen in dem
Skript verwendet werden.
:p.:hp8.Beachte&colon.:ehp8. Wenn der Wert einer vordefinierten Variable im Skript
ge�ndert wird, dann wirken sich die �nderungen nicht auf FleetStreet aus, solange
Sie nicht eine Funktion oder einen Befehl von FleetStreet verwenden, um die
�nderungen wirksam zu machen.
:p.Die vordefinierten Variablen sind&colon.
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
:p.Dies ist ein Array der Benutzernamen. :hp4.FleetSetup.Names.0:ehp4. enth�lt die
Anzahl der Elemente im Array. :hp4.FleetSetup.Names.1:ehp4. etc. enthalten die Namen.
:p.Beispiel&colon.
:table cols='20 10'.
:row.
:c.Variable
:c.Wert
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
:p.Dies ist ein Array von Benutzeradressen. :hp4.FleetSetup.Addresses.0:ehp4.
enth�lt die Anzahl der Elemente im Array. :hp4.FleetSetup.Addresses.1:ehp4. etc.
enthalten die Adressen.
:p.Beispiel&colon.
:table cols='22 14'.
:row.
:c.Variable
:c.Wert
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
:p.:hp4.FleetSetup.Echotoss:ehp4. enth�lt den Namen der  Echotoss.Log-Datei.

:h4 id=rvar04.FleetSetup.Tosser
:p.:hp4.FleetSetup.Tosser:ehp4. enth�lt den Namen der Tosser-Konfigurations-Datei.

:h4 id=rvar05.FleetStatus.Area
:p.:hp4.FleetStatus.Area:ehp4. ist eine Gruppe von Variablen, die Informationen
�ber die aktuelle Area enthalten.
:p.Die Variablen sind&colon.
:parml.
:pt.:hp4.FleetStatus.Area.Tag:ehp4.
:pd.Area-Kennung
:pt.:hp4.FleetStatus.Area.Desc:ehp4.
:pd.Area-Beschreibung
:pt.:hp4.FleetStatus.Area.File:ehp4.
:pd.Pfad und/oder Dateiname der Area
:pt.:hp4.FleetStatus.Area.Format:ehp4.
:pd.Format der Area. Der Wert ist entweder "*.MSG", "Squish" oder "JAM"
:pt.:hp4.FleetStatus.Area.Type:ehp4.
:pd.Typ der Area. Der Wert ist einer aus "Echo", "Net", "Local" und "Private".
:eparml.

:h4 id=rvar06.FleetStatus.DestArea
:p.:hp4.FleetStatus.DestArea:ehp4. enth�lt den Area-Tag der Area, in der eine
Message beim Antworten oder Forwarden gespeichert werden soll.
:p.Diese Variable ist nur beim Editieren einer Message definiert!

:h4 id=rvar07.FleetStatus.Name
:p.:hp4.FleetStatus.Name:ehp4. enth�lt den momentan aktiven Benutzer-Namen.

:h4 id=rvar08.FleetStatus.Address
:p.:hp4.FleetStatus.Address:ehp4. enth�lt die momentan aktive Benutzer-Adresse.

:h4 id=rvar09.FleetStatus.Mode
:p.:hp4.FleetStatus.Mode:ehp4. enth�lt den momentanen Programm-Status. Sie kann eine
der folgenden Werte annehmen&colon.
:sl compact.
:li.No Setup
:li.Edit Single
:li.Edit XPost
:li.Edit CCopy
:li.Read
:li.Cleanup
:esl.
:p.Beim Schreiben einer Message ist das erste Wort in der Variable "Edit". Das
zweite Wort zeigt an, ob Sie eine einzelne Message schreiben oder eine Verteilerliste
oder die Crosspost-Funktion verwenden.

:h4 id=rvar10.FleetStatus.Monitor
:p.:hp4.FleetStatus.Monitor:ehp4. ist entweder "0", wenn kein Monitor-Fenster
verwendet wird, oder "1", wenn das Skript mit einem Monitor-Fenster abl�uft.
:hp4.FleetStatus.Monitor:ehp4. kann direkt in einem boole'schen Ausdruck verwendet
werden.

:h4 id=rvar11.FleetStatus.Cursor
:p.Wenn Sie eine Message schreiben, enth�lt :hp4.FleetStatus.Cursor:ehp4. zwei Zahlen,
die die aktuelle Cursor-Position anzeigen. Die erste Nummer ist der Absatz, die
zweite Zahl ist die Zeichenposition im Absatz. So bedeutet z.B. "46 3", da� der
Cursor am dritten Zeichen im 46. Absatz steht.
:p.
Das folgende Programm gibt den Text nach dem Cursor aus&colon.
:xmp.
para = word(FleetStatus.Cursor, 1)
offs = word(FleetStatus.Cursor, 2)
say substr(FleetMsg.Text.para, offs)
:exmp.
:p.
Beim Lesen von Messages ist :hp4.FleetStatus.Cursor:ehp4. nicht definiert.

:h4 id=rvar12.FleetMsg.Header
:p.:hp4.FleetMsg.Header:ehp4. ist eine Gruppe von Variablen, die Informationen
�ber den Header der aktuellen Message enthalten.
:p.Die Variablen sind&colon.
:parml.
:pt.:hp4.FleetMsg.Header.Attrib:ehp4.
:pd.Message-Attribute, genauso wie in der "Attrib"-Zeile.
:pt.:hp4.FleetMsg.Header.From:ehp4.
:pd.Name des Absenders.
:pt.:hp4.FleetMsg.Header.FromAddress:ehp4.
:pd.Adresse des Absenders.
:pt.:hp4.FleetMsg.Header.To:ehp4.
:pd.Name des Empf�ngers.
:pt.:hp4.FleetMsg.Header.ToAddress:ehp4.
:pd.Adresse des Empf�ngers. Verwenden Sie diese Variable in Echo-Areas nicht!
:pt.:hp4.FleetMsg.Header.Subj:ehp4.
:pd.Themenzeile.
:pt.:hp4.FleetMsg.Header.DateWritten:ehp4.
:pd.Datum und Zeit des Erstellen der Message.
:pt.:hp4.FleetMsg.Header.DateReceived:ehp4.
:pd.Datum und Zeit des Tossens der Message.
:eparml.

:h4 id=rvar13.FleetMsg.Text
:p.:hp4.FleetMsg.Text:ehp4. ist ein Array von Text-Abs�tzen. :hp4.FleetMsg.Text.0:ehp4.
enth�lt die Anzahl der Elemente im Array. :hp4.FleetMsg.Text.1:ehp4. etc.
enthalten den Message-Text.
:p.:hp8.Beachte&colon.:ehp8. Die Elemente des Arrays sind :hp2.nicht:ehp2. Text-Zeilen,
sondern Abs�tze. Ein Absatz im Original-Text endet mit einem Zeilenendezeichen. Wenn
Sie den Text z.B. durch Einf�gen oder L�schen von W�rtern �ndern, kann der Text
wieder richtig umbrochen werden. Wenn Sie Ihr eigenes Format mit einer bestimmten
Zeilenl�nge ben�tigen, dann m�ssen Sie Ihren eigenen Zeilenumbruch programmieren.
Das m��te in Rexx ziemlich leicht sein.

:h4 id=rvar16.FleetMsg.Kludges
:p.:hp4.FleetMsg.Kludges:ehp4. ist ein Array, das die Kludges der Message
enth�lt. Das Array ist nur beim Lesen von Messages definiert.
:p.
Die Felder von :hp4.FleetMsg.Kludges:ehp4. sind &colon.
:parml.
:pt.:hp2.FleetMsg.Kludges.0:ehp2.
:pd.Anzahl der Elemente im Array
:pt.:hp2.FleetMsg.Kludges.1:ehp2.
:pd.Erste Kludge-Line
:pt.:hp2.FleetMsg.Kludges.*:ehp2.
:pd.alle weiteren Kludge-Lines
:eparml.
:p.
Die Kludge-Lines haben meist die Gestalt
:xmp.
BEZEICHNUNG: Wert
:exmp.
:p.
oder
:xmp.
BEZEICHNUNG Wert
:exmp.
:p.
Eine Kludge-Line kann auch mehrfach vorkommen. Die Kludges haben keine ausgezeichnete
Ordnung. Das Zeichen :hp2.01 hex:ehp2. am Anfang der Kludge-Lines ist in den Variablen
nicht enthalten.

:h4 id=rvar17.FleetMsg.Seenbys
:p.:hp4.FleetMsg.Seenbys:ehp4. ist ein Array, das die SEEN-BY-Zeilen
der Message enth�lt. Das Array ist nur beim Lesen von Messages
definiert.
:p.
Die Felder von :hp4.FleetMsg.Seenbys:ehp4. sind &colon.
:parml.
:pt.:hp2.FleetMsg.Seenbys.0:ehp2.
:pd.Anzahl der Elemente im Array
:pt.:hp2.FleetMsg.Seenbys.1:ehp2.
:pd.Erste Zeile
:pt.:hp2.FleetMsg.Seenbys.*:ehp2.
:pd.alle weiteren Zeilen
:eparml.
:p.
SEEN-BY-Zeilen haben die Gestalt
:xmp.
SEEN-BY&colon. nodes
:exmp.
:p.
Die Zeilen haben die gleiche Reihenfolge wie in der Original-Message.

:h4 id=rvar14.FleetCCopy
:p.Beim Erstellen einer Message unter Verwendung einer Verteilerliste (oder des
Schnellverteilers) enth�lt das Rexx-Array :hp4.FleetCCopy:ehp4. die Namen
und Adressen der Verteilerliste. Wenn Sie keine Verteilerliste verwenden, sind die
Variablen nicht definiert.
:p.
Die Felder von :hp4.FleetCCopy:ehp4. sind&colon.
:parml.
:pt.:hp2.FleetCCopy.0:ehp2.
:pd.Anzahl der Eintr�ge in der Verteilerliste.
:pt.:hp2.FleetCCopy.1.Name:ehp2.
:pd.Name des ersten Eintrags.
:pt.:hp2.FleetCCopy.1.Address:ehp2.
:pd.Adresse des ersten Eintrags.
:pt.:hp2.&dot.&dot.&dot.:ehp2.
:pd.
:eparml.

:h4 id=rvar15.NewMail
:p.:hp4.NewMail:ehp4. kann eine Kombination der folgenden Werte annehmen (durch
ein Leerzeichen getrennt)&colon.
:parml break=none.
:pt.:hp2.'Echo':ehp2.
:pd.Es wurde neue Echomail eingegeben
:pt.:hp2.'Net':ehp2.
:pd.Es wurde neue Netmail eingegeben
:pt.:hp2.'Local':ehp2.
:pd.Es wurde neue lokale Mail eingegeben
:eparml.
:p.
Wenn keine Messages eingegeben wurden, ist die Variable leer.
:p.
:hp8.Beachte&colon.:ehp8. Diese Variable hat nur einen Wert w�hrend der Abarbeitung
beim Programmende, d.h. wenn das Skript als "Programmende"-Hook ausgef�hrt wird.
Ansonsten ist sie undefiniert.

.* ***************************** Funktionen ************************************

:h3 id=rexxfunc.Funktionen
:p.FleetStreet bietet einige neue Rexx-Funktionen. Diese sind&colon.
:sl compact.
:li.:hp4.FSCls:ehp4.
:li.:hp4.FSLookupAddress:ehp4.
:li.:hp4.FSLookupName:ehp4.
:li.:hp4.FSSetEntryField:ehp4.
:li.:hp4.FSSetHeader:ehp4.
:li.:hp4.FSSetText:ehp4.
:esl.

:h4.FSCls
:p.:hp4.FSCls:ehp4. l�scht das Monitor-Fenster.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSCls()
:exmp.
:p.:hp2.Parameter&colon.:ehp2.
:p.
FSCls ben�tigt keine Parameter.
:p.:hp2.R�ckgabewerte&colon.:ehp2.
:p.:hp4.FSCls:ehp4. gibt "OK" zur�ck, wenn das Monitor-Fenster gel�scht wurde.
"NoMonitor" wird zur�ckgegeben, wenn das Skript ohne Monitor-Fenster abl�uft.
:p.:hp2.Beispiel&colon.:ehp2.
:xmp.
call FSCls
:exmp.

:h4.FSSetHeader
:p.:hp4.FSSetHeader:ehp4. erwartet ein Rexx-Array als Parameter und verwendet die
Inhalte der Variablen als neuen Message-Header.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetHeader(stem)
:exmp.
:p.:hp2.Parameter&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.Rexx-Array, das den Inhalt des Message-Headers enth�lt. stem hat die folgenden
Felder&colon.
:ul compact.
:li.From
:li.FromAddress
:li.To
:li.ToAddress
:li.Subj
:eul.
:p.
Das sind die gleichen Felder wie in :hp4.FleetMsg.Header:ehp4., jedoch werden nur
die obigen Felder verwendet.
:eparml.
:p.
:hp2.R�ckgabewerte&colon.:ehp2.
:p.:hp4.FSSetHeader:ehp4. gibt "OK" zur�ck.
:p.
:p.:hp2.Anmerkungen&colon.:ehp2.
:ul compact.
:li.Alle Elemente des Arrays m�ssen einen Wert haben, auch wenn es nur
der Null-String ist.
:li.Setzen Sie den Stamm-Namen immer in Anf�hrungszeichen, so da� er nicht
durch dessen Wert ersetzt wird.
:li.Beim Lesen wird der neue Header nicht auf der Platte gespeichert. Beim
Schreiben wird der neue Header nur gespeichert, wenn Sie die gesamte Message
speichern (Strg-S).
:eul.
:p.
:p.:hp2.Beispiel&colon.:ehp2.
:xmp.
/* Replace sender name */
FleetMsg.Header.From = 'Joe user'
RetVal = FSSetHeader('FleetMsg.Header')
:exmp.

:h4.FSSetText
:p.:hp4.FSSetText:ehp4. erwartet ein Rexx-Array als Parameter und verwendet den
Text im Array als den aktuellen Message-Text. Der Text im Array ersetzt den
bisherigen Text.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetText(stem)
:exmp.
:p.
:p.:hp2.Parameter&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.Rexx-Array, das den Message-Text enth�lt. stem.0 enth�lt die Anzahl der Abs�tze,
stem.1 ... stem.n enthalten die Abs�tze.
:eparml.
:p.
:p.:hp2.R�ckgabewerte&colon.:ehp2.
:p.:hp4.FSSetText:ehp4. gibt "OK" zur�ck.
:p.
:p.:hp2.Anmerkungen&colon.:ehp2.
:ul compact.
:li.Das Format des Arrays ist das gleiche wie das von FleetMsg.Text.
:li.Element 0 des Arrays mu� einen numerischen Wert haben.
:li.Alle Elemente des Arrays m�ssen einen Wert haben, auch wenn es nur
der Null-String ist.
:li.Setzen Sie den Stamm-Namen immer in Anf�hrungszeichen, so da� er nicht
durch dessen Wert ersetzt wird.
:li.Beim Lesen wird der neue Text nicht auf der Platte gespeichert. Beim
Schreiben wird der neue Text nur gespeichert, wenn Sie die gesamte Message
speichern (Strg-S).
:eul.
:p.
:p.:hp2.Beispiel&colon.:ehp2.
:xmp.
NewText.0 = 2
NewText.1 = 'Dies ist nur eine'
NewText.2 = 'kurze Message.'
RetVal = FSSetText('NewText')
:exmp.


:h4.FSLookupAddress
:p.:hp4.FSLookupAddress:ehp4. sucht eine FTN-Adresse in der Nodeliste. Das Ergebnis
wird in einer Stamm-Variablen abgelegt.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSLookupAddress(ftnaddress, stem)
:exmp.
:p.
:p.:hp2.Parameter&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.ftnaddress:ehp2.
:pd.FTN-Adresse des zu suchenden Nodes.
:pt.:hp2.stem:ehp2.
:pd.Stamm-Name der Variablen, die das Ergebnis zugewiesen bekommen soll.
:p.Die Variable hat die folgenden Felder&colon.
:parml.
:pt.:hp4.Stem.Address:ehp4.
:pd.Adresse des Nodes
:pt.:hp4.Stem.Name:ehp4.
:pd.Name des SysOps
:pt.:hp4.Stem.System:ehp4.
:pd.Name des Systems
:pt.:hp4.Stem.Phone:ehp4.
:pd.Telefonnummer
:pt.:hp4.Stem.Location:ehp4.
:pd.Standort des Systems
:pt.:hp4.Stem.Password:ehp4.
:pd.Das Session-Passwort. Dieses Feld ist leer, wenn Sie f�r den Node kein
Passwort definiert haben.
:pt.:hp4.Stem.Modem:ehp4.
:pd.Modem-Typ. Dies ist ein numerischer Wert.
:pt.:hp4.Stem.Baud:ehp4.
:pd.Maximale Baud-Rate.
:pt.:hp4.Stem.UserCost:ehp4.
:pd.Kosten f�r den Benutzer, um eine Message an den Node zu schreiben.
:pt.:hp4.Stem.CallCost:ehp4.
:pd.Kosten f�r einen Anruf bei dem Node.
:pt.:hp4.Stem.Flags:ehp4.
:pd.Node-Flags, eine Kombination aus "ZC", "RC", "MO, "Hub", "Host" und "CM".
:eparml.
:eparml.
:p.
:p.:hp2.R�ckgabewerte&colon.:ehp2.
:p.:hp4.FSLookupAddress:ehp4. gibt einen der folgenden Werte zur�ck&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Der Node wurde gefunden. Die Node-Daten werden im zweiten Parameter abgelegt.
:pt."NotFound"
:pd.Der Node wurde nicht gefunden.
:pt."Error"
:pd.Bei der Suche ist ein Fehler aufgetreten.
:eparml.
:p.
:p.:hp2.Anmerkungen&colon.:ehp2.
:ul compact.
:li.Wenn die Adre�-Zeichenkette zu lang ist, wird sie abgeschnitten. Das Feld
"Stem.Address" enth�lt die resultierende Adresse.
:li.Setzen Sie den Stamm-Namen immer in Anf�hrungszeichen, so da� er nicht
durch dessen Wert ersetzt wird.
:eul.
:p.
:p.:hp2.Beispiel&colon.:ehp2.
:xmp.
RetVal = FSLookupAddress('2&colon.2490/2520', 'NodeData')
say 'System-Name&colon.' NodeData.System
:exmp.


:h4.FSLookupName
:p.:hp4.FSLookupName:ehp4. sucht einen Sysop-Namen in der Nodeliste. Das Ergebnis
wird in einer Stamm-Variablen abgelegt.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSLookupName(name, stem)
:exmp.
:p.
:p.:hp2.Parameter&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.name:ehp2.
:pd.Gesuchter Sysop-Name.
:pt.:hp2.stem:ehp2.
:pd.Stamm-Name der Variablen, die das Ergebnis zugewiesen bekommen soll.
:p.Die Variable hat die folgenden Felder&colon.
:parml.
:pt.:hp4.Stem.0:ehp4.
:pd.Anzahl der gefundenen Eintr�ge.
:pt.:hp4.Stem.1.Address:ehp4.
:pd.Adresse des Nodes (1. Eintrag)
:pt.:hp4.Stem.1.Name:ehp4.
:pd.Name des Sysops (1. Eintrag)
:pt.:hp4.Stem.1.System:ehp4.
:pd.Name des Systems(1. Eintrag)
:pt.:hp4.Stem.1.Phone:ehp4.
:pd.Telefonnummer (1. Eintrag)
:pt.:hp4.Stem.1.Location:ehp4.
:pd.Standort des Systems (1. Eintrag)
:pt.:hp4.Stem.1.Password:ehp4.
:pd.Das Session-Passwort (1. Eintrag). Dieses Feld ist leer, wenn Sie f�r den Node kein
Passwort definiert haben.
:pt.:hp4.Stem.1.Modem:ehp4.
:pd.Modem-Typ (1. Eintrag). Dies ist ein numerischer Wert.
:pt.:hp4.Stem.1.Baud:ehp4.
:pd.Maximale Baudrate (1. Eintrag)
:pt.:hp4.Stem.1.UserCost:ehp4.
:pd.Kosten f�r den Benutzer, um eine Message an den Node zu schreiben (1. Eintrag)
:pt.:hp4.Stem.1.CallCost:ehp4.
:pd.Kosten f�r einen Anruf bei dem Node (1. Eintrag)
:pt.:hp4.Stem.1.Flags:ehp4.
:pd.Node-Flags, eine Kombination aus "ZC", "RC", "MO", "Hub", "Host" und "CM" (1. Eintrag)
:eparml.
:eparml.
:p.Stem.2 etc. enthalten die weiteren Eintr�ge.
:p.
:p.:hp2.R�ckgabewerte&colon.:ehp2.
:p.:hp4.FSLookupName:ehp4. gibt einen der folgenden Werte zur�ck&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Der Node wurde gefunden. Die Node-Daten werden im zweiten Parameter abgelegt.
:pt."NotFound"
:pd.Der Node wurde nicht gefunden.
:pt."Error"
:pd.Bei der Suche ist ein Fehler aufgetreten.
:eparml.
:p.
:p.:hp2.Anmerkungen&colon.:ehp2.
:ul compact.
:li.Wenn der Name zu lang ist, so wird er abgeschnitten. Das Feld "Stem.x.Name"
enth�lt den resultierenden Namen.
:li.Setzen Sie den Stamm-Namen immer in Anf�hrungszeichen, so da� er nicht
durch dessen Wert ersetzt wird.
:li.Sie k�nnen auch nur einen Teil des Nachnamens angeben.
:eul.
:p.
:p.:hp2.Beispiel&colon.:ehp2.
:xmp.
RetVal = FSLookupName('Joe User', 'NodeData')
do i = 1 to NodeData.0
  say 'Address&colon.' NodeData.i.Address
end
:exmp.

:h4.FSSetEntryField
:p.:hp4.FSSetEntryField:ehp4. setzt den Text des Eingabefeldes im Monitor-Fenster.
Dies ist ein Mittel, um eine Vorgabe f�r die Benutzereingabe zu liefern.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetEntryField(text)
:exmp.
:p.
:p.:hp2.Parameter&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.text:ehp2.
:pd.Neuer Text f�r das Eingabefeld
:eparml.
:p.:hp2.R�ckgabewerte&colon.:ehp2.
:p.:hp4.FSSetEntryField:ehp4. gibt einen der folgenden Werte zur�ck&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Der Text wurde gesetzt.
:pt."NoMonitor"
:pd.Das Skript l�uft ohne Monitor-Fenster ab, deshalb konnte der Eingabetext nicht
gesetzt werden.
:eparml.
:p.
:p.:hp2.Anmerkungen&colon.:ehp2.
:ul compact.
:li.Der Text wird auf maximal 500 Zeichen gek�rzt.
:eul.
:p.
:p.:hp2.Beispiel&colon.:ehp2.
:xmp.
RetVal = FSSetEntryField('C&colon.\')
if RetVal = 'OK' then
   do
   say 'Bitte Pfad eingeben'
   parse pull mypath .
   end
:exmp.

.* **************************** Hooks ***************************************
.* @2@ **********************************************************************

:h2.Hooks
:p.
FleetStreet kann bestimmte Skripts in bestimmten Situationen automatisch
starten.
:p.
Um ein Skript einem bestimmten Hook zuzuweisen, gehen Sie wie folgt vor&colon.
:ol compact.
:li.�ffnen Sie den Rexx-Skript-Ordner
:li.W�hlen Sie "Einstellungen" im Kontextmen� des Skript-Ordners
:li.Wechseln Sie zur "Hooks"-Seite des Notebooks
:li.W�hlen Sie bei der Drop-Down-Liste des betreffenden Hooks das gew�nschte
Skript aus
:eol.
:p.
Derzeit sind folgende Hooks verf�gbar&colon.
:ul compact.
:li.Programmende
:li.Vor dem Speichern
:eul.

.* **************************** Pipe-Server ********************************
.* @1@ **********************************************************************
:h1.Der FleetStreet Pipe-Server
:p.
Dieser Abschnitt beschreibt die Funktionen des Pipeservers von FleetStreet.
:p.
FleetStreet startet automatisch einen Thread, der ausschlie�lich eine
Named Pipe bedient. �ber diese Pipe kann FleetStreet gesteuert werden.
:p.
Der Name der Pipe ist
:xmp.
\PIPE\FleetStreetDoor
:exmp.
:p.
Die Pipe ist bidirektional. Anwendungen, die �ber diese Pipe mit FleetStreet
kommunizieren wollen, m�ssen ein Protokoll verwenden. Dieses Protokoll
ist nachfolgend beschrieben. In den meisten F�llen wird jedoch FleetCom
ausreichen. FleetCom ist ein spezieller FleetStreet-Client, der leicht
in Batch-Programmen u.�. eingebaut werden kann.
:p.
Weiterf�hrende Themen&colon.
:ul compact.
:li.:link reftype=hd refid=proto.Das Protokoll:elink.
:li.:link reftype=hd refid=commands.Kommandos des Pipe-Servers:elink.
:li.:link reftype=hd refid=fleetcom.FleetCom:elink.
:eul.

.* @2@ **********************************************************************
:h2 id=proto.Das Protokoll
:p.
Eine Session mit FleetStreet teilt sich in 3 Abschnitte auf&colon. Verbindungsaufbau,
Befehlsabwicklung und Ende der Verbindung.
:p.
Nachfolgend werden die Abschnitte beschrieben. Dabei werden die folgenden ASCII-
Zeichen verwendet&colon.
:table cols='6 11 7'.
:row.
:c.Symbol
:c.Hexadezimal
:c.Dezimal
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
:hp2.Verbindungsaufbau:ehp2.
:p.
Nachdem der Client die Pipe ge�ffnet hat, sendet er zuerst ein :hp2.<ENQ>:ehp2. an
FleetStreet. Daraufhin sendet FleetStreet die Kennung
:hp2."FleetStreet"<ETX>:ehp2. an
den Client. Der Client mu� nun pr�fen, ob sich hinter der Pipe wirklich
FleetStreet verbirgt, ob also die gesendete Kennung mit "FleetStreet"
�bereinstimmt. Ist dies der Fall, so sendet der Client ein :hp2.<ACK>:ehp2. an den
Server. Im Fehlerfall sendet der Client ein :hp2.<NAK>:ehp2.. Daraufhin sendet
FleetStreet ein :hp2.<EOT>:ehp2. zur�ck und beendet die Verbindung.
:p.
Bekommt FleetStreet jedoch ein :hp2.<ACK>:ehp2., so sendet er daraufhin seine
Versionsnummer
als Text, z.B. :hp2."0.88"<ETX>:ehp2.. Falls der Client nur mit bestimmten Versionen
zusammenarbeitet, so kann er wieder die Kennung �berpr�fen. Er sendet dann
wiederum entweder :hp2.<ACK>:ehp2. oder :hp2.<NAK>:ehp2. an FleetStreet. Wenn die
Kennung nicht gepr�ft wird, mu� :hp2.<ACK>:ehp2. zur�ckgesendet werden.
:p.
Die erste Phase ist nun beendet, und FleetStreet wartet auf Befehle des
Clients.
:p.
Die Aufbauphase im Schema&colon.
:xmp.

 Client              FleetStreet

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
Fehlerfall&colon.
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


:hp2.Befehlsabwicklung:ehp2.
:p.
Der Client sendet die Befehle als einfachen ASCII-Text an FleetStreet. Jeder
Befehl wird mit :hp2.<ETX>:ehp2. abgeschlossen.
:p.
Beispiel&colon.
:xmp.
"SCAN *"<ETX>
:exmp.
:p.
Parameter sind durch einzelne Leerzeichen vom Befehl und untereinander
getrennt.
:p.
FleetStreet pr�ft dann, ob der Befehl g�ltig ist. Im Fehlerfall wird die
Sequenz
:xmp.
<NAK><Code><ETX>
:exmp.
:p.
zur�ckgesendet. <Code> ist dabei einer der drei Buchstaben C, P oder S.
Die Bedeutung der drei Buchstaben ist&colon.
:parml break=none.
:pt.C
:pd.Der Befehl wurde nicht erkannt
:pt.P
:pd.Die Parameter fehlen oder sind ung�ltig (jeweils f�r den Befehl)
:pt.S
:pd.Es wurden mehr Parameter angegeben als erwartet.
:eparml.
:p.
Im Gut-Fall wird die Sequenz
:xmp.
<ACK><ETX>
:exmp.
:p.
gesendet. In diesem Fall beginnt FleetStreet mit der Befehlsausf�hrung. Nach
dem Ende der Ausf�hrung wird das Ergebnis mitgeteilt. Wenn w�hrend der
Ausf�hrung ein Fehler aufgetreten ist, so wird die Sequenz
:xmp.
<NAK>"Fehlertext"<ETX>
:exmp.
:p.
gesendet. "Fehlertext" ist dabei eine Beschreibung des Fehlers im Klartext.
Bei fehlerfreier Bearbeitung wird
:xmp.
<ACK>"Ergebnis"<ETX>
:exmp.
:p.
gesendet. "Ergebnis" ist dabei die angeforderte Information, ein Statusbericht,
oder kann evtl. auch leer sein.
:p.
Nach der Abarbeitung eines Befehls kann ein weiterer Befehl an FleetStreet
gesendet werden. Dies kann beliebig oft bis zum Verbindungsende geschehen.
:p.
Im Schema&colon.
:xmp.
 Client              FleetStreet

       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

      [Bearbeitung]

     <ACK>"34 areas"<ETX>
   <----------------------
:exmp.
:p.
Falscher Befehl&colon.
:xmp.
       "ABC XYZ"<ETX>
   ---------------------->

        <NAK>"C"<ETX>
   <----------------------
:exmp.
:p.
Fehler bei Ausf�hrung&colon.
:xmp.
       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

      [Bearbeitung]

    <NAK>"disk error"<ETX>
   <----------------------
:exmp.
:p.
:hp2.Verbindungsende:ehp2.
:p.
Wenn der letzte Befehl des Clients komplett abgearbeitet wurde, mu� die
Verbindung aufgel�st werden. Dazu sendet der Client ein
:xmp.
EOT
:exmp.
:p.
an den Server. Dieser sendet darufhin ein
:xmp.
EOT
:exmp.
:p.
zur�ck, und beide brechen die Verbindung ab.
:p.
Im Schema&colon.
:xmp.
 Client              FleetStreet

            EOT
   ---------------------->

            EOT
   <----------------------
:exmp.


.* @2@ **********************************************************************
:h2 id=commands.Die Kommandos des Pipe-Servers
:p.
Derzeit sind die folgenden Kommandos implementiert&colon.
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
Die angegebenen Areas werden neu gescannt. Als Areas kann angegeben werden&colon.
:parml break=none tsize=20.
:pt.:hp2.*:ehp2.
:pd.Alle Areas
:pt.:hp2.Areatag:ehp2.
:pd.Nur die angegebene Area, z.B. TUB
:pt.:hp2.@filename:ehp2.
:pd.Alle Areas, die in dem angegebenen File stehen. Das Format des
Files ist das gleiche wie beim Echotoss-File, also ein Area-Tag
pro Zeile
:eparml.

:h3.ETOSS
:p.
:hp2.ETOSS:ehp2.
:p.
Das Echotoss-File wird geschrieben. Wenn kein Parameter angegeben wurde, wird
das File gem�� dem FleetStreet-Setup geschrieben. Ein Parameter kann angegeben
werden und bezeichnet dann den Filenamen des geschriebenen Files.


:h3 hide.LOCK
:p.
:hp2.LOCK:ehp2.
:p.
Die angegebenen Areas werden gesperrt, d.h. sie werden von FleetStreet nicht
mehr verwendet. Wenn die Area gerade in Verwendung ist, so bleibt der
Befehl so lange in Bearbeitung, bis die Area nicht mehr verwendet wird.
:p.
Areas k�nnen im gleichen Format angegeben werden wie bei SCAN

:h3 hide.UNLCK
:p.
:hp2.UNLCK:ehp2.
:p.
Die gesperrten Areas werden freigegeben. Areas werden im gleichen Format
angegeben wie bei LOCK.


.* @2@ **********************************************************************
:h2 id=fleetcom.FleetCom
:p.
Mit FleetStreet wird ein spezielles Tool namens FleetCom mitgeliefert.
FleetCom ist ein Client f�r den FleetStreet Pipe-Server. Der Aufruf ist&colon.
:xmp.
FLEETCOM Befehl [Parameter]
:exmp.
:p.
FleetCom baut die Verbindung zum Pipe-Server auf, sendet den Befehl und die
Parameter und wertet die zur�ckgegebenen Daten aus. FleetCom gen�gt in den
meisten F�llen, um den Pipe-Server von FleetStreet zu nutzen. FleetCom
sendet pro Session nur einen Befehl.
:p.
Die Return-Codes von FleetCom sind&colon.
:parml break=none tsize=4.
:pt.0
:pd.alles OK, kein Fehler
:pt.1
:pd.Pipe konnte nicht ge�ffnet werden. Dieser Fehler tritt auch auf, wenn
FleetStreet beim Aufruf nicht l�uft. Er kann also in den meisten
F�llen �bergangen werden.
:pt.2
:pd.Kein Befehl angegeben.
:pt.3
:pd.FleetStreet erkennt den Befehl nicht
:pt.4
:pd.Fehler bei der Ausf�hrung des Befehls, oder Systemfehler.
:eparml.
:p.
FleetCom verwendet per Default die Pipe \PIPE\FleetStreetDoor. Es kann jedoch
auch ein anderer Name verwendet werden. Er wird der Environment-Variable
FLEETPIPE entnommen. Beispiel&colon.
:xmp.
SET FLEETPIPE=\PIPE\AnotherPipe
:exmp.
:p.
Wenn FleetCom wieder den Default-Namen verwenden soll, so mu� die Variable mit
:xmp.
SET FLEETPIPE=
:exmp.
:p.
gel�scht werden.
:p.
:hp2.Beispiele zur Verwendung von FleetCom&colon.:ehp2.
:p.
Rescan von Areas, in die neue Messages getosst wurden&colon.
:xmp.
SquishP IN -f echotoss.log
FleetCom scan @echotoss.log
:exmp.
:p.
Schreiben des Echotoss.Log, damit Squish aus diesen Areas die Messages
exportieren kann&colon.
:xmp.
FleetCom etoss pack.log
SquishP OUT SQUASH -f pack.log
:exmp.
:p.

.* @1@ **********************************************************************
:h1.�berlegungen zu den Message-Base-Formaten
:p.
Jede der drei unterst�tzten Message-Base-Formate hat seine Vorz�ge und Probleme.
Dieser Abschnitt beschreibt, wie FleetStreet die unterschiedlichen Formate
handhabt und bietet eine Entscheidungshilfe.

.* @2@ **********************************************************************
:h2.*.MSG
:p.
FleetStreet verwendet die MSGAPI32.DLL von Squish zum Zugriff auf *.MSG-Areas.
Diese API liest nur maximal 512 Bytes an Kludge-Lines einer Message. Sollten
einmal mehr Kludges in einer Message enthalten sein (was bei Messages aus dem
Internet leicht der Fall sein kann), so wird bei 512 Bytes einfach abgeschnitten.
Der Rest der Kludges wird im Message-Text geliefert.
:p.
FleetStreet zieht in jedem Fall noch die weiteren Kludge-Lines aus der Message.
Eine durchtrennte Kludgeline ist jedoch nicht mehr als solche erkennbar. Deshalb
kann es vorkommen, da� am Anfang des Message-Textes manchmal eine halbe
Kludge-Line auftaucht.
:p.
Das *.MSG-Format sieht nur die Fido-Standard-Attribute zur Speicherung vor. Alle
anderen Attribute speichert FleetStreet in einer FLAGS-Kludge. Diese wird jedoch
von Squish (dem Tosser) nicht erkannt. Attribute wie "Direkt" oder "Archivieren
nach Senden" sind deshalb bei Squish wirkungslos.
:p.
FleetStreet verwendet f�r das "Gelesen"-Attribut den Lese-Z�hler der *.MSG-Files.
Dieser wird jedoch von Squish beim Packen der Message zur�ckgesetzt, so da� die
Message wieder als "ungelesen" erscheint.

.* @2@ **********************************************************************
:h2.Squish
:p.
Die MSGAPI32.DLL (die zum Zugriff auf Squish-Areas verwendet wird) hat einen
schweren Design-Fehler&colon. Beim �ffnen einer Area wird der Area-Index (*.SQI)
in den Speicher geladen. Bei �nderungen in der Area wird der Index auf der
Platte jedoch nicht sofort auf den aktuellen Stand gebracht. Die �nderung wird
vielmehr nur im Speicher vorgenommen und erst beim Schlie�en der Area auf die
Platte zur�ckgeschrieben. Wenn nun zwei Programme gleichzeitig eine Area
bearbeiten, dann �berschreiben sie sich gegenseitig die �nderungen im Index.
Das f�hrt zu einem defekten Area-Index oder sogar zur Zerst�rung der Area.
:p.
Wie in *.MSG-Areas werden auch in Squish-Areas nur die Standard-Attribute
angeboten. Die anderen Attribute speichert FleetStreet wieder in einer
FLAGS-Kludge. Squish (der Tosser) beachtet diese Kludge-Line :hp5.nicht:ehp5..
:p.
F�r das "Gelesen"-Attribut verwendet FleetStreet das h�chstwertige Bit im
Attribut-Feld der Messages.
:p.
Das "Behalten"-Attribut wird von :hp2.SqPack:ehp2. nicht beachtet. Verwenden
Sie :hp2.FESQ:ehp2. zum Packen von Squish-Areas.

.* @2@ **********************************************************************
:h2.JAM
:p.
Im *.JHR-File einer JAM-Area ist die Anzahl der aktiven Messages der Area
vermerkt. Dieses Feld wird jedoch von verschiedenen Programmen nicht
korrekt belegt (IMail 1.75 setzt das Feld beim Packen der Area auf 0;
GoldEd 2.50.Beta6 erzeugt beim L�schen von Messages gelegentlich
Unterl�ufe des Feldes). Deshalb ist der Inhalt des Feldes nicht zuverl�ssig
zu verwenden. FleetStreet verl��t sich deshalb auf den Index der Area.
:p.
Der Index von JAM-Areas hat ein schlechtes Design. Anhand des Index-Eintrages
alleine ist nicht zu erkennen, ob eine Message gel�scht oder aktiv ist.
Es gibt nun zwei M�glichkeiten, um damit umzugehen&colon.
:ol.
:li.Man liest zus�tzlich der Kopf der Message, um herauszufinden, ob die Message
gel�scht oder aktiv ist. Dies ist langsam und f�hrt den Zweck des Indexes
ad absurdum. FleetStreet verwendet diese Methode nicht mehr.
:li.Man zeigt alle Messages an, auch wenn einige davon als "gel�scht"
gekennzeichnet sind.
:eol.
:p.
Damit das L�schen von Messages trotzdem eine Wirkung hat, �berschreibt
FleetStreet den Index-Eintrag der Message komplett. Dies entspricht zwar
nicht 100% der JAM-Spezifikation, ist aber die einzig sinnvolle Vorgehensweise.
Einmal mit FleetStreet gel�schte Messages tauchen in keinem Fall wieder auf.
:p.
Manche Messagebase-Utilities l�schen Messages, indem sie nur den Header als
gel�scht kennzeichnen und �ndern den Index-Eintrag nicht. In diesem Fall sollte
man die Area anschlie�end packen, um die gel�schten Messages endg�ltig
loszuwerden.

.* @1@ **********************************************************************
:h1.�berlegungen zu Tossern
:p.
FleetStreet unterst�tzt viele verschiedene Tosser direkt. In diesem Abschnitt
finden sich einige Hinweise zur effektiven Zusammenarbeit mit verschiedenen
Tossern.

.* @2@ **********************************************************************
:h2.Squish
:p.
Die Zusammenarbeit mit Squish ist relativ problemlos. Dennoch einige
Anmerkungen&colon.
:ul.
:li.Squish erkennt bzw. behandelt die FLAGS-Kludge nicht, d.h. nur die
Standard-Message-Attribute haben eine Wirkung.
:li.Beim Packen von Messages aus *.MSG-Areas wird der Lesez�hler auf 0
zur�ckgesetzt, d.h. die Messages erscheinen wieder als "ungelesen".
:eul.

.* @2@ **********************************************************************
:h2.Fastecho
:p.
:ul.
:li.Fastecho unterst�tzt die Broadcast-Features von Squish nicht.
:li.Eine Besonderheit ist die Behandlung von mehreren Netmail-Areas. Im
Gegensatz zu Squish kann Fastecho Netmails nicht direkt in die Netmail-Areas
tossen oder daraus packen. Die Netmails m�ssen beim Packen erst aus den
sekund�ren Netmail-Areas exportiert und danach gepackt
werden. Zum Exportieren wird das gleiche Kommando verwendet wie zum Exportieren
von Echomail. FleetStreet behandelt aber alle Netmail-Areas gleich, d.h.
sie werden nicht in die ECHOTOSS.LOG-Datei geschrieben. Beim Export werden
also die Netmails in den sekund�ren Netmail-Areas nicht erfa�t.
:p.
Die L�sung ist, den Export-Vorgang in zwei Schritten durchzuf�hren&colon.
Im ersten Schritt wird Echomail mit Hilfe der ECHOTOSS.LOG-Datei exportiert.
Im zweiten Schritt wird die Netmail aus den sekund�ren Netmail-Areas mit
Hilfe einer Dummy-ECHOTOSS.LOG exportiert. Diese Dummy-ECHOTOSS.LOG listet
einfach nur die sekund�ren Netmail-Areas auf. Im letzten Schritt wird die
exportierte Mail gepackt.
:p.
Beispiel&colon.
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
Die Version 1.75 von IMail hat leider einen Fehler&colon. In JAM-Areas
sind die :hp2.PATH:ehp2.-Kludge und die :hp2.SEEN-BY:ehp2.-Zeilen im
Message-Text statt (wie vorgeschrieben) beim Message-Kopf gespeichert.
Dadurch werden diese Zeilen in FleetStreet unterhalb des Message-Textes
angezeigt.


.* @1@ **********************************************************************
:h1.Anhang
:p.

.* ************************** Kludge-Lines ***********************************
.* @2@ **********************************************************************
:h2 id=kludges.Kludge-Lines
:p.
Die folgenden Kludge-Lines werden von FleetStreet erzeugt bzw. erkannt&colon.
:parml compact tsize=3 break=all.
:pt.:hp2.FMPT:ehp2.
:pt.:hp2.TOPT:ehp2.
:pt.:hp2.INTL:ehp2.
:pd.Diese Kludges werden von FleetStreet nach :hp4.FTS-0001 Rev. 15:ehp4. erzeugt, falls
die erzeugte Message eine NetMail ist.
:p.Squish erzeugt die Kludges selbst nochmal beim Exportieren der Message
neu. Die Unterst�tzung dieser Kludges w�re also eigentlich nicht notwendig.
:p.Beim Lesen werden diese Kludges ignoriert.
:p.

:pt.:hp2.MSGID:ehp2.
:pd.Beim Schreiben einer Message wird diese Kludge nach :hp4.FTS-0009 Rev. 1:ehp4.
erzeugt. Die Adre�-Komponente ist 4D.
:p.Beim Lesen in einer Echomail-Area wird :hp2.MSGID:ehp2. zur Ermittlung
der Absender-Adresse herangezogen. Falls :hp2.MSGID:ehp2. keine FTN-Adresse
enth�lt oder fehlt, wird die Origin-Zeile untersucht.
:p.

:pt.:hp2.REPLY:ehp2.
:pd.Beim Schreiben einer Antwort wird die :hp2.MSGID:ehp2. des Originals
als :hp2.REPLY:ehp2. geschrieben.
:p.

:pt.:hp2.PID:ehp2.
:pd.Falls die Benutzung von :hp2.PID:ehp2. im Setup aktiviert ist, wird die
:hp2.PID:ehp2.-Kludge geschrieben und nur eine kurze Tearline angeh�ngt. Eine
lange Tearline wird angeh�ngt, wenn :hp2.PID:ehp2. deaktiviert ist.
:p.FleetStreet folgt den Empfehlungen von :hp4.FSC-0046 Rev. 2:ehp4..
:p.

:pt.:hp2.REPLYTO:ehp2.
:pd.Beim Antworten auf eine NetMail mit :hp2.REPLYTO:ehp2.-Kludge werden
die dort angegebene Adresse und der Username als Empf�nger eingesetzt.
:p.In Echos wird die Kludge ignoriert. FleetStreet folgt den Empfehlungen von
:hp4.FSC-0035 Rev. 1.:ehp4.
:p.

:pt.:hp2.REPLYADDR:ehp2.
:pd.Beim Antworten auf eine NetMail mit :hp2.REPLYADDR:ehp2. wird die
dort angegebene Adresse in einer "To&colon."-Zeile am Anfang der Antwort eingesetzt.
:p.In Echos wird die Kludge ignoriert. FleetStreet folgt den Empfehlungen von
:hp4.FSC-0035 Rev. 1.:ehp4.
:p.

:pt.:hp2.SPLIT:ehp2.
:pd.Wenn eine Message gespeichert wird und l�nger als 12 KB ist, wird sie
nach :hp4.FSC-0047 Rev. 1:ehp4. in mehrere Teile aufgeteilt. Folgende Abweichungen
gelten&colon.
:p.
:ul.
:li.Da die Message nie als Ganzes in einer Messagebase gespeichert
war, ist die in der :hp2.SPLIT:ehp2.-Kludge angegebene Message-Nummer
implemetierungsabh�ngig (derzeit&colon. Messagenummer der ersten erzeugten
Message).
:li.:hp4.FSC-0047:ehp4. empfielt, in der Teilen 2...n die :hp2.MSGID:ehp2.
wegzulassen, damit ein Dupe-Checker diese Teile nicht aussortiert. FleetStreet
erzeugt stattdessen f�r alle Teile eine eigene :hp2.MSGID:ehp2., was die
gleiche Wirkung hat.
:li.Da die Teilenummer in der :hp2.SPLIT:ehp2.-Kludge zweistellig
ist, werden maximal 99 Teile erzeugt. Die Messagel�nge ist damit auf
1188 KB begrenzt.
:li.Beim �ndern einer Message kann die ge�nderte Message nur max.
15 KB lang sein. Ein erneutes Aufteilen w�rde mit der Numerierung der anderen
Teile kollidieren. Bei der Gegenseite w�re ein Zusammensetzen der Message
dann nicht mehr m�glich.
:li.Die Numerierung der Messageteile erfolgt am Ende der Themenzeile,
nicht wie vorgeschlagen am Anfang. Dadurch erscheinen in der Themenliste die
Messageteile wieder in der korrekten Sortierung.
:eul.
:p.
:pt.:hp2.APPEND, REALADDRESS:ehp2.
:pd.Diese Kludges bleiben beim Reply erhalten.
:p.
:pt.:hp2.CHARSET/CHRS:ehp2.
:pd.FleetStreet unterst�tzt diese Kludges bis Level 2 gem�� :hp4.FSC-0054 Rev. 4:ehp4..
Beim Schreiben von Messages erzeugt FleetStreet immer :hp2.IBMPC 2:ehp2..
:p.
:pt.:hp2.ACUPDATE:ehp2.
:pd.Diese Kludgeline wird von Squish 1.10 verwendet, um Messages auf anderen
Systemen zu �ndern oder zu l�schen. Lesen Sie die Squish 1.10-Dokumentation
f�r weitergehende Informationen.
:p.
:pt.:hp2.AREA:ehp2.
:pd.Wenn diese Kludgeline in einer Message gefunden wird und die Area FleetStreet
bekannt ist, so wird eine Antwort auf diese Message automatisch in der dort
angegebenen Area abgelegt.
:p.
:pt.:hp2.FLAGS:ehp2.
:pd.Die :hp2.FLAGS:ehp2.-Kludge enth�lt die Message-Attribute, die nicht
direkt von der Message-Base unterst�tzt werden. Das Format dieser Kludge ist in
:hp4.FSC-0053:ehp4. definiert.
:p.
:pt.:hp2.FWDFROM, FWDTO, FWDSUBJ, FWDORIG, FWDDEST, FWDAREA, FWDMSGID:ehp2.
:pd.Diese Kludges werden von FleetStreet erzeugt, wenn man eine Message
weiterleitet. Sie enthalten Header-Informationen der Original-Message.
Bei einer Antwort auf eine solche Message verwendet FleetStreet wieder
die Original-Daten.
:eparml.

.* @2@ **********************************************************************
:h2 id=cmdlin.Programm-Parameter
:p.
FleetStreet erkennt die folgenden Kommandozeilen-Parameter&colon.
:parml.
:pt.:hp2.-C<Pfad>:ehp2.
:pd.Die INI-Dateien werden nicht im aktuellen Verzeichnis gelesen und geschrieben,
sondern im angegebenen Verzeichnis.
:p.:hp2.Beispiel&colon.:ehp2.
:p.FLTSTRT.EXE -Cd&colon.\myinis
:eparml.

.* @2@ **********************************************************************
:h2.Return-Codes
:p.
FLTSTRT.EXE erzeugt die folgenden Return-Codes&colon.
:parml break=none.
:pt.:hp2.0:ehp2.
:pd.Keine neuen Messages eingegeben
:pt.:hp2.1:ehp2.
:pd.Neue Netmail eingegeben
:pt.:hp2.2:ehp2.
:pd.Neue Echomail eingegeben
:pt.:hp2.4:ehp2.
:pd.Neue lokale Messages eingegeben.
:pt.:hp2.255:ehp2.
:pd.Fataler Fehler
:eparml.
:p.
Eine Kombination von 1, 2 und 4 bedeutet, da� Messages in Areas dieser Typen
eingeben wurde, z.B. bedeutet 5, da� es neue Messages in Netmail- und
lokalen Areas gibt.


.* @2@ **********************************************************************
:h2.Bug-Reports
:p.
FleetStreet ist mit Sicherheit nicht fehlerfrei. Ich kann deshalb nur jeden
ermuntern, mir alle Fehler zu melden. Ich werde mich dann nach besten
Kr�ften bem�hen, die Fehler zu beseitigen.
:p.
Manche Fehler kann ich evtl. nicht (sofort) nachvollziehen. Die folgenden
Fragestellungen sind evtl. f�r die Beseitigung des Fehlers relevant&colon.
:ul.
:li.L��t sich der Fehler reproduzieren?
:li.Tritt der Fehler beim ersten Versuch auf oder erst nach mehrmaligem
Probieren?
:li.Welche Funktion wurde ausgef�hrt, als der Fehler auftrat?
:li.Wurde die Funktion mit der Tastatur, dem Men� oder der Toolbar aktiviert?
Macht die Bedienweise einen Unterschied?
:li.Kommt eine Fehlermeldung? Welche?
:li.Wurde FleetStreet richtig konfiguriert?
:li.Was ist die genaue Auswirkung des Fehlers?
:eul.
:p.
Wenn FleetStreet durch einen Fehler abst�rzt, dann wird im aktuellen Verzeichnis
die Datei :hp2.FLTSTRT.DMP:ehp2. erzeugt. Mit Hilfe dieser Datei kann der
Fehler meist recht leicht lokalisiert werden.


.* @2@ **********************************************************************
:h2.Verwendete Programme
:p.
FleetStreet wurde mit folgenden Programmen erstellt und getestet&colon.
:ul.
:li.Compiler&colon. IBM VisualAge C++ 3.0 (C-Modus)
:li.Debugger&colon. IBM C/C++ Debugger 3.0 (IPMD)
:li.Entwicklungsumgebung&colon. IBM Workframe 3.0
:li.Linker&colon. ILink
:li.Editor&colon. Enhanced Editor, Tiny Editor, LPEX
:li.Tools&colon. IBM OS/2 Toolkit 3.0; GNU Grep; PMTree; ExeMap;
Hexdump; PMSpy; PM Camera
:eul.

.* @2@ **********************************************************************
:h2 id=support.Support
:p.Michael Hohner ist unter den folgenden EMail-Adressen erreichbar&colon.
:parml compact break=none tsize=16.
:pt.Fidonet&colon.
:pd.Michael Hohner 2&colon.2490/1050.17 (neu!)
:pt.OS2Net&colon.
:pd.Michael Hohner 81&colon.499/617.17 (neu!)
:pt.Internet&colon.
:pd.miho@n-online.de (neu!)
:eparml.
:p.
:hp2.Fido-Echomail&colon.:ehp2.
:p.
Es gibt zwei Fido-Echos bei 2&colon.2490/1050, FLEETBETA und FLEETSTREET. FLEETBETA
ist das deutschsprachige Echo, FLEETSTREET ist das internationale Echo
(englischsprachig). Schreiben Sie eine NM an Robert Gloeckner
2&colon.2490/1050, wenn Sie das Echo beziehen wollen. Dort erfahren Sie auch
von weiteren Nodes, die das Echo bereitstellen k�nnen. Das Echo darf frei
weitergeroutet werden, aber bitte informieren Sie uns �ber angeschlossene
Nodes.
:p.
Fragen zu FleetStreet k�nnen auch im Fido-Echo OS2BBS.GER gestellt werden.

:euserdoc.

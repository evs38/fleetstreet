:userdoc.
:docprof toc=1234.
:title.FleetStreet Istruzioni per l'utente

.* ************************** Introduzione ********************************
.* @1@ **********************************************************************
:h1.In generale
:p.
:lines align=center.
Benvenuti da

:font facename='Tms Rmn' size=24x18.:color fc=red.
FleetStreet 1.27.1
:font facename=default size=0x0.:color fc=default.
:elines.
:p.
FleetStreet ä un editor FTS compatibile di messaggi per OS/2 2.x PM.
FleetStreet puï trattare *.MSG e basi di messaggio Squish e JAM ...
:p.
:p.
La presente documentazione descrive la configurazione di FleetStreet e l'approccio
generale. Informazioni pió dettagliate si trovano nei singoli dialoghi di FleetStreet
e nella guida online del programma. La documentazione e la guida online si completano
vicendevolmente.
:p.
Questa documentazione presuppone che tu possieda giÖ conoscenze approfondite sul Fidonet
o reti compatibili.
:p.
Open Source version.


.* ************************** Features   ********************************
.* @2@ **********************************************************************
:h2.Features
:p.
Features di FleetStreet&colon.
:ul compact.
:li.Programma PM OS/2 2.x/3.x
:li.100% 32-Bit-Code
:li.Ottimizzato per processori Pentium
:li.Multi-Threaded
:li.Integrato nella WPS
:li.Commandabile via
:ul compact.
:li.Menu
:li.Shortcuts
:li.Toolbar (con posizione selezionabile, 2 dimensioni)
:eul.
:li.L'utente viene aiutato dalla guida online esauriente e testi di aiuto
nella barra di stato
:li.Funzioni standard come
:ul compact.
:li.leggere messaggi
:li.scrivere messaggi
:li.cancellare messaggi
:li.modificare messaggi
:li.copiare messaggi
:li.spostare messaggi
:li.forwardare messaggi
:li.rispondere a messaggi
:li.esportare messaggi
:li.importare testi ASCII in messaggi
:eul.
:li.Si supportano i formati area
:ul compact.
:li.*.MSG
:li.Squish
:li.JAM
:eul.
:li.Lettura automatica dei files di configurazione
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
:li.Stampa un messaggio come testo ASCII oppure usando il driver stampante PM
:li.Crea un file ECHOTOSS.LOG
:li.Visualizza i messaggi a colori
:li.Crossposta un messaggi nuovo in aree diverse
:li.Crea richieste file da un messaggio
:li.Scrive a pió destinatari, usando liste di carbon copy
:li.Carbon copy rapido
:li.Lista nomi abbrevviati
:li.Supporta le funzioni broadcast di Squish 1.1x
:li.Supporta le nodelist Version-7
:li.Nodelist-Browser
:li.Tasti di funzione, liberamente configurabili con qualsiasi testo
:li.Rimappa drives per l'uso in rete
:li.Maschere messaggi, liberamente definibili
:li.Funzione efficace di ricerca, anche attraverso diverse aree
:li.Ricerca di messaggi personali
:li.Lista soggetti, per una lettura secondo le linee di soggetto
:li.Lista messaggi
:li.Configurazione variabile aree
:li.Conversione di caratteri speciali durante la scrittura
:li.Supporta la CHRS-Kludgeline
:li.Mettere messaggio in edidenza a mano
:li.Comandi a distanza via Named Pipe
:li.Programmabile via macro, usando scritti REXX
:li.Molte funzioni e feature Drag-and-Drop
:li.Il programma ä reperibile nelle seguenti lingue
:ul compact.
:li.Tedesco
:li.Inglese
:li.Italiano
:li.Svedese
:eul.
:eul.
:p.
&dot.&dot.&dot. e questo ä solamente un breve riassunto &colon.-)

.* @2@ **********************************************************************
:h2.Requisiti
:p.
Ci vogliono i requisiti successivi per usare FleetStreet&colon.
:p.
:hp2.Hardware&colon.:ehp2.
:ul compact.
:li.PC, capace di far girare OS/2 con una velocitÖ soddisfacente (386DX, 6 MB)
:li.circa 1,5 MB di spazio sul disco fisso, in pió ci vuole dello spazio per la base di messaggi
:li.scheda grafica VGA
:eul.
:p.
:hp2.Software&colon.:ehp2.
:ul compact.
:li.OS/2 2.x/3.x (non ä stato provata la versione 2.0, ma si dice che funzioni)
:eul.
:p.
:hp2.Ambienti software testati&colon.:ehp2.
:ul compact.
:li.OS/2 2.1 (lingue diverse)
:li.OS/2 2.11 (2.1 con Service Pack)
:li.OS/2 2.99 (WARP II)
:li.OS/2 Warp 3.0
:li.OS/2 Warp Connect (Peer To Peer)
:li.OS/2 Warp 4.0
:li.ZipStream 1.03 (Base messaggi in directory compressa)
:li.LAN Server 4.0 Entry
:eul.

.***************************************************************************
.* Design Goals                                                            *
.***************************************************************************

.* @2@ **********************************************************************
:h2 id=design.Scopi del progetto
:p.Durante lo sviluppo di FleetStreet intendevamo realizzare diverse idee
di cui parliamo in questa documentazione.
:parml.
:pt.:hp2.PerchÇ un Editor PM?:ehp2.
:pd.E' vero che il Presentation Manager (tm) ä pió lento di un'applicazione
VIO, (finchÇ l'ultima viene lanciata come applicazione full-screen), ma ä
altrettanto vero che ha alcuni vantaggi rispetto al PM. Il clipboard ä com-
pletamente utilizzabile (il clipboard delle applicazioni VIO ä un compromesso),
l'utente puï selezionare in modo molto semplice i colori ed i caratteri.
Inoltre non ä limitato all'uso di un formato 80*25 (o.s.) - . Il PM ci permetteva
inoltre implementare tutti i taccuini e dialoghi che facilitano tanto l'uso di FleetStreet.
:pt.:hp2.Setup integrato:ehp2.
:pd.Con altri editor con ogni probabilitÖ ti sei giÖ perso con files ASCII infiniti.
Essendo giÖ abituato all'uso di notebooks e dialoghi della Workplace-Shell, la
configurazione di FleetStreet non dovrebbe essere un problema. Le impostazioni
si possono anche modificare durante l'uso: cioä non sarÖ necessario lanciare
il programma un'altra volta per attivare la nuova configurazione.
:p.
Nel caso tu non sia sicuro dell'effetto di un'impostazione, bisogna premere
F1 oppure uno dei pulsanti d'aiuto che ci sono un po' dappertutto, risparmiandosi
lo sfogliare noioso nei testi di documentazione un po' lunghi.
:pt.:hp2.Supporto per Squish (tm):ehp2.
:pd.FleetStreet supporta il tosser Squish in due modi. Da una parte usa la
API di Squish (MSGAPI.DLL), dall'altra FleetStreet legge direttamente il file
di configurazione di Squish (SQUISH.CFG).
:i2 refid=squish.MSGAPI.DLL
:pt.:hp2.Uso semplice:ehp2.
:pd.La gran parte delle applicazione PM ä da usare in modo molto semplice. Nel caso
si sappia usare la WPS, si puï anche sfruttare FleetStreet, dato che usiamo
notebooks, menu popup, container ecc.
:pt.:hp2.Tutte le funzioni di cui si ha bisogno:ehp2.
:pd.Abbiamo implementato alcune 'features' trovate utili in altri editor di
messaggi, ed alcune inedite.
:pt.:hp2.Power:ehp2.
:pd.FleetStreet utilizza diversi threads, CUA'91 e processi a 32 bit.
:eparml.

.* @2@ **********************************************************************
:h2.Copyrights etc.
:p.
Squish e MsgAPI sono marchi depositati di Lanius Corporation.
:p.
OS/2 e Workplace Shell sono marchi depositati di IBM.
:p.
JAM(mbp) - Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, Mats Wallin.
ALL RIGHTS RESERVED.


.* @2@ **********************************************************************
:h2.Ringraziamenti
:p.
Le persone successive hanno contribuito alla creazione e al perfezionamento di
FleetStreet in modo particolare&colon.
:sl.
:li.:hp4.Harry Herrmannsdîrfer:ehp4. - Echo-Management, Alpha/Beta-Test, programmazione
:li.:hp4.Harald Kamm e Roberto Zanasi:ehp4. - Traduzione italiana, sede di registrazione Italia
.* @@
:li.:hp4.Jason Meaden:ehp4. - Ehemalige Registration-Site Australien
:li.:hp4.Helmut Renner:ehp4. - Sede di registrazione e echo-link Austria
.* @@
:li.:hp4.Marty Duplissey:ehp4. - Ehemalige Registration-Site
:li.:hp4.Siegfried Hentschel, Jens Holm, Richard Douglas,
Jose M. Guglieri:ehp4. - Sedi di registrazione
:li.:hp4.Rasmus Foged Hansen:ehp4. - GiÖ sede di registrazione
:li.:hp4.Peter Karlsson:ehp4. - Traduzione svedese
:li.:hp4.Tutti i partecipanti alla fase Closed-Beta, in particolare Michael Siebke,
Joachim Loehrl-Thiel, Hajo Kirchhoff, Robert Glîckner:ehp4. u.a.
:li.:hp4.Thomas Jachmann:ehp4. - Beta-Test e diversi scritti Rexx
:li.:hp4.Torsten Grimme:ehp4. - Test del supporto IMail
:li.:hp4.Dirk Brenken:ehp4. - Test del supporto Fastecho
:li.:hp4.Carsten Ellwart:ehp4. - Bug-Finder
:li.:hp4.e soprattutto tutti gli utenti registrati:ehp4., senza i quali lo Shareware
non riuscirebbe a sovravvivere.
:esl.

.* ************************* Installation **********************************

.* @1@ **********************************************************************
:h1.Configurazione di base
:p.
Quando si lancia FleetStreet per la prima volta, viene visualizzato il messaggio che
il file INI non ä stato trovato. In questa situazione ä del tutto normale e non c'ä
ragione per preoccuparsi. BasterÖ premere il pulsante :hp2.OK:ehp2..
Adesso FleetStreet apre automaticamente il taccuino del setup, dando la possibilitÖ
di impostare la configurazione di base.
:p.
La configurazione di base, descritta in queste righe, ä pió che sufficiente per
installare FleetStreet secondo una configurazione normale tipo point o tipo nodo.
E si possono osare i primi passi... La configurazione puï essere modificato in
qualsiasi momento ed essere adattata alle proprie esigenze.
:p.
Nel caso che nascano delle difficoltÖ durante la configurazione basterÖ premere
il pulsante :hp2.Guida:ehp2. che si trova su ogni pagina.

.* @2@ **********************************************************************
:h2.Nomi
:p.
Quando vengono scritti messaggi nuovi, FleetStreet inserisce automaticamente il
tuo nome come mittente.
:p.
Sulla prima pagina del taccuino di setup si possono inserire tutti i nomi da
usare con FleetStreet. Il primo nome nella lista sarÖ quello di default e viene
inserito ogni volta quando necessita il tuo nome. Qualsiasi nome puï essere
usato come nome di default, selezionando il nome nella lista e premendo il pulsante
:hp2.Default:ehp2..

.* @2@ **********************************************************************
:h2.Indirizzi
:p.
Sulla seconda pagina del taccuino di setup si possono impostare gli indirizzi
di rete da usare con FleetStreet. Il primo indirizzo nella lista viene usato
come indirizzo di default.

.* @2@ **********************************************************************
:h2.Configurazione tosser
:p.
Con un tosser installato che viene supportato da FleetStreet basterÖ far leggere
il rispettivo file di configurazione. FleetStreet ne estrae tutti gli indirizzi
definiti, aree e directories. Non sarÖ pió necessario ridefinire le aree messaggi
un'altra volta in FleetStreet.
:p.
Anche in futuro FleetStreet leggerÖ il file indicato dopo l'avvio. Cosç le aree
di FleetStreet corrisponderanno sempre alle aree definite per il tosser.
:p.
Si supportano i seguenti tosser&colon.
:table cols='10 20 30'.
:row.
:c.Tosser
:c.Versioni
:c.File da selezionare
:row.
:row.
:c.Squish
:c.1.01, 1.10, 1.11
:c.SQUISH.CFG (oppure il nome di un file equivalente)
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
Bisogna attivare l'impostazione :hp2."leggi":ehp2. sulla quarta pagina del
taccuino di setup. Dopo aver selezionato il tosser desiderato ä sufficiente
premere il pulsante :hp2."Cerca...":ehp2.. Dopo aver selezionato il file di
configurazione bisogna premere :hp2.OK:ehp2..
:p.
Dopo la lettura del file di configurazione da parte di FleetStreet, la configurazione
di base ä terminata. Bisogna chiudere il taccuino di setup e adesso FleetStreet dovrebbe
aprire la prima area e visualizzarne il primo messaggio nella finestra principale.

.* @1@ **********************************************************************
:h1.Uso
:p.

.* @2@ **********************************************************************
:h2 id=mainwin.Finestra principale
:p.
Durante la lettura di messaggi vengono visualizzati i seguenti elementi della
finestra principale&colon.
:p.
:hp2.Barra di titolo:ehp2.
:artwork name='titlebar.bmp' align=center.
:p.
Vengono visualizzati&colon.
:ul compact.
:li.Nome e versione del programma
:li.Area attuale
:eul.
:p.
:hp2.Message-header:ehp2.
:artwork name='header.bmp' align=center.
:p.
Vengono visualizzati&colon.
:ul compact.
:li.Nome e indirizzo del mittente
:li.Nome e indirizzo del destinatario (indirizzo solamente nelle aree netmail)
:li.Soggetto
:li.Attributi del messaggio
:li.Data e ora della creazione del messaggio
:li.Data e ora dell'arrivo del messaggio
:eul.
:p.
:hp2.:ehp2.
:p.
Il significato dei pulsanti&colon.
:parml compact break=none tsize=6.
:pt.:artwork runin name='home.bmp'.
:pd.Ritorno al vecchio lastread dell'area
:pt.:artwork runin name='nextarea.bmp'.
:pd.Alla prossima area con messaggi nuovi
:pt.:artwork runin name='prevmsg.bmp'.
:pd.Al messaggio precedente
:pt.:artwork runin name='nextmsg.bmp'.
:pd.Al prossimo messaggio
:pt.:artwork runin name='prevreply.bmp'.
:pd.All'originale di questa risposta
:pt.:artwork runin name='nextreply.bmp'.
:pd.Alla risposta di questo messaggio
:pt.:artwork runin name='firstmsg.bmp'.
:pd.Al primo messaggio
:pt.:artwork runin name='lastmsg.bmp'.
:pd.All'ultimo messaggio
:pt.:artwork runin name='ok.bmp'.
:pd.Salva messaggio
:pt.:artwork runin name='cancel.bmp'.
:pd.Annulla messaggio o modifica
:pt.:artwork runin name='newmsg.bmp'.
:pd.Scrivi messaggio nuovo
:pt.:artwork runin name='edit2.bmp'.
:pd.Modifica messaggio attuale
:pt.:artwork runin name='importfile.bmp'.
:pd.Importa file di testo in messaggio
:pt.:artwork runin name='exportfile.bmp'.
:pd.Esporta messaggio come file di testo
:pt.:artwork runin name='reply2.bmp'.
:pd.Rispondi al messaggio attuale
:pt.:artwork runin name='printmsg.bmp'.
:pd.Stampa messaggio attuale
:pt.:artwork runin name='find.bmp'.
:pd.Avvia funzione di ricerca
:pt.:artwork runin name='msglist.bmp'.
:pd.Apri lista messaggi
:pt.:artwork runin name='msgtree.bmp'.
:pd.Apri lista soggetti
:pt.:artwork runin name='area.bmp'.
:pd.Apri lista aree
:pt.:artwork runin name='showkludges.bmp'.
:pd.Apri finestra con kludge-lines
:pt.:artwork runin name='delmsg.bmp'.
:pd.Cancella messaggio attuale
:pt.:artwork runin name='bookm.bmp'.
:pd.Passa alla finestra segnalibro
:pt.:artwork runin name='help.bmp'.
:pd.Guida generale per FleetStreet
:pt.:artwork runin name='cut.bmp'.
:pd.Copia selezione nel clipboard e cancellala
:pt.:artwork runin name='copy.bmp'.
:pd.Copia selezione nel clipboard
:pt.:artwork runin name='paste.bmp'.
:pd.Inserisci contenuto del clipboard nella posizione attuale
:pt.:artwork runin name='copymsg.bmp'.
:pd.Copia messaggio attuale in un'altra area
:pt.:artwork runin name='movemsg.bmp'.
:pd.Sposta, copia o forwarda messaggio attuale
:pt.:artwork runin name='fwdmsg.bmp'.
:pd.Forward messaggio attuale in un'altra area
:pt.:artwork runin name='shell.bmp'.
:pd.Lancia shell linea comando
:pt.:artwork runin name='script.bmp'.
:pd.Rexx-Scripts
:pt.:artwork runin name='browser.bmp'.
:pd.Visualizza il contenuto delle nodelist
:pt.:artwork runin name='request.bmp'.
:pd.Richiedi files del messaggio attuale
:pt.:artwork runin name='catchup.bmp'.
:pd.Segna tutti i messaggi come "letti"
:eparml.
:p.
:hp2.Barra di stato:ehp2.
:artwork name='statline.bmp' align=center.
:p.
Vengono visualizzati (da sinistra a destra)&colon.
:ul compact.
:li.Testi di auiuto, messaggi
:li.Segnalibro (con messaggio marcato)
:li.Posizione cursore (solo durante scrittura)
:li.Modo d'inserire/sovrascrivere (solo durante scrittura)
:li.Numero del messaggio attuale, numero dei messaggio nell'area
:li.Indirizzo attuale
:eul.

.* @2@ **********************************************************************
.* :h2.Lettura
.* :p.

.* @2@ **********************************************************************
:h2 id=writemsg.Compilazione di messaggi
:p.Ci sono due possibilitÖ per scrivere messaggi nuovi&colon.
:parml.
:pt.:hp2.Scrivi messaggi nuovi:ehp2.
:pd.Volendo scrivere un messaggio nuovo bisogna premere il pulsante
:artwork name='help01.bmp' runin. oppure INS.
La finestra messaggi viene svuotata e si inserisce l'indirizzo del mittente
nel campo corrispondente. Adesso ä possibile inserire il nome del destinatario,
il suo indirizzo di rete (vedi anche :link reftype=hd refid=enteraddr."Inserimento
di indirizzi FTN":elink.) e la linea del soggetto. Dopo di che sarÖ possibile
scrivere il testo nella finestra del messaggio.
:p.Dopo aver finito il messaggio bisogna premere il pulsante
:artwork name='help04.bmp' runin. oppure i tasti CTRL-S. Il messaggio viene
salvato nella base di messaggi. Nel caso si desideri interrompere l'inserimento
bisogna premere il pulsante :artwork name='help05.bmp' runin. oppure il tasto ESC.
:pt.:hp2.Risposta ad un messaggio:ehp2.
:pd.Per rispondere ad un messaggio ricevuto bisogna premere il pulsante
:artwork name='help03.bmp' runin. oppure i tasti CTRL-R o CTRL-N, nel caso si
desideri rispondere al mittente in un'altra area.
Attraverso il pulsante ä possibile rispondere o al mittente o al destinatario del
messaggio. Esiste anche la possibilitÖ di rispondere in un'altra area.
:p.Dopo aver finito il messaggio bisogna premere il pulsante
:artwork name='help04.bmp' runin. oppure i tasti CTRL-S. Il messaggio viene
salvato nella base di messaggi. Premendo il pulsante :artwork name='help05.bmp' runin.
oppure il tasto ESC durante l'inserimento, si interrompe la scrittura del messaggio.
:eparml.

.* @2@ **********************************************************************
:h2 id=changemsg.Modifica messaggi
:p.E' possibile modificare messaggi dopo averli salvati. Bisogna premere
il pulsante :artwork name='help06.bmp' runin. per passare al modo editor. Adesso
ä possibile riprocessare il messaggio. Si possono modificare il testo e lo header
del messaggio.
:p.Per modificare gli attributi del messaggio bisogna premere il pulsante :hp2.Modifica:ehp2..
Si visualizza una finestra di dialogo dov'ä possibile stabilire o modificare gli attributi
del messaggio.
:p.Nel caso si modifichi un messaggio giÖ spedito o non locale, si visualizza un avvertimento.
:p.Bisogna premere il pulsante :artwork name='help04.bmp' runin. per salvare
le modifiche. Nel caso si rifiuti il messaggio modificato bisogna premere il
pulsante :artwork name='help05.bmp' runin.&dot.

.* @2@ **********************************************************************
:h2.Cancella
:p.
Ci sono i seguenti modi per cancellare un messaggio&colon.
:ul.
:li.Bisogna premere il tasto :hp2.CANC:ehp2.. Dopo una domanda di sicurezza il messaggio
viene cancellato.
:li.Bisogna premere il pulsante per cancellare nella :link reftype=hd refid=mainwin.Toolbar:elink..
:li.Bisogna trascinare il messaggio attuale sulla trinciatrice. Si comincia a trascinarlo
sopra la finestra dell'editor.
.br
:artwork align=center name='shredmsg.bmp'.
:eul.
:p.
E' anche possibile cancellare messaggi dalla :link reftype=hd refid=msglist.Lista messaggi:elink.,
:link reftype=hd refid=threadlist.Lista soggetti:elink. oppure dalla
:link reftype=hd refid=bookmarks.Lista segnalibro:elink..

.* @2@ **********************************************************************
:h2.Stampa
:p.Ci sono i seguenti modi per stampare un messaggio&colon.
:ul.
:li.Bisogna premere la combinazione di tasti :hp2.SHIFT-PRINT:ehp2.
:li.Bisogna premere il pulsante per stampare nella :link reftype=hd refid=mainwin.Toolbar:elink..
:li.Bisogna trascinare il messaggio su un oggetto stampante.
Si comincia a trascinarlo sopra la finestra dell'editor.
:eul.
:p.
E' anche possibile stampare messaggi dalla lista :link reftype=hd refid=msglist.Lista messaggi:elink.,
:link reftype=hd refid=threadlist.Lista soggetti:elink. oppure dalla
:link reftype=hd refid=bookmarks.Lista segnalibro:elink..

.* @2@ **********************************************************************
:h2.Lista aree
:p.
La lista aree contiene tutte le aree di messaggi accessibili da FleetStreet.
E' possibile passare ad una di queste aree, crearne, cancellarne e mettere gli
attributi di aree specifiche.
:p.
Bisogna fare un click sull'area in questione oppure premere :hp2.ENTER:ehp2. per
passarci.
:p.
Bisogna premere :hp2.Scan:ehp2. per avviare lo scan di tutte le aree. Potrebbe
essere necessario nel caso in cui il tosser utilizzato abbia tossato messaggi nuovi dall'ultimo
scan. Lo scan si effettua in un thread separato, per cui ä possibile continuare
a leggere messaggi, mentre lo scan separato viene eseguito sullo sfondo.
:p.
Argomenti supplementari&colon.
:ul compact.
:li.:link reftype=hd refid=areacon.Contenuto della lista delle aree:elink.
:li.:link reftype=hd refid=areacrea.Crea e cancella aree:elink.
:li.:link reftype=hd refid=areaset.Imposta attributi per aree:elink.
:li.:link reftype=hd refid=arealistset.Adatta la lista delle aree:elink.
:li.:link reftype=hd refid=areafolders.Cartella aree:elink.
:eul.

.* ***************************** Area List settings *************************

:h3 id=areacon.Contenuto della lista delle aree
:p.
Per ogni area nella lista si visualizza
:ul compact.
:li.la descrizione dell'area,
:li.il numero di messaggi nell'area e
:li.il numero di messaggi :link reftype=fn refid=unrmsg.non letti:elink.
:eul.
:fn id=unrmsg.
:p.Effettivamente non si tratta del numero :hp2.reale:ehp2. di messaggi non letti.
Si tratta semplicemente del numero di messaggi dopo il lastread.
:efn.
:p.
Per aree su cui non ä stato effettuato uno scan, si usa "-" come numero di messaggi.
:p.
Inizialmente la descrizione delle aree ä identica all'area-tag. La descrizione dell'area
viene visualizzata nella finestra principale e puï essere modificata :link reftype=fn refid=desccha.:elink.

:fn id=desccha.
:p.Le modifiche si possono applicare nel notebook dei settaggi dell'area in questione,
oppure facendo un click con il pulsante sinistro del mouse mentre si tiene premuto
il tasto ALT. Adesso ä possibile editare direttamente la descrizione dell'area.
:efn.


:h3 id=areacrea.Crea e cancella aree
:p.
:hp2.Crea un'area nuova:ehp2.
:p.
E' possibile creare un'area nuova, selezionando :hp2."Crea":ehp2. nel menu di contesto
Per un'area nuova si apre un notebook di configurazione vuoto. Bisogna riempire
tutti i campi per poi chiudere il notebook L'area nuova viene inserita nella
lista delle aree.
:p.
Tutte le aree create in FleetStreet sono aree :hp2.locali:ehp2..

:p.
:hp2.Cancella aree:ehp2.
:p.
Si possono cancellare esclusivamente aree create in FleetStreet.
Aree definite nel file di configurazione del tosser utilizzato devono essere
cancellate nel file di configurazione del tosser.
:p.
Per cancellare un'area bisogna aprire il :link reftype=fn refid=areacon.menu di contesto dell'area:elink.
per selezionare :hp2."Cancella":ehp2.. Di seguito l'area viene cancellata dalla lista.

:fn id=areacon.
:p.Bisogna fare click con il tasto destro del mouse sull'area.
:efn.
:p.
Considera&colon. I files corrispondenti all'area
:hp2.non:ehp2. vengono cancellati dal disco fisso.

:h3 id=areaset.Configura gli attributi per aree
:p.
Ogni area ha un suo set di attributi. Si tratta di&colon.
:ul compact.
:li.Descrizione dell'area
:li.Area-tag
:li.Nome dell'utente di default
:li.Indirizzo di default
:li.Path
:li.Formato dell'area
:li.Tipo di area
:li.Attributi di default del messaggio
:li.Ulteriori settaggi
:eul.
:p.
Non ä possibile modificare l'area-tag, l'indirizzo di default, il path, il formato dell'area ed i settaggi
dell'area net/echo di aree definite nel file di configurazione del tosser utilizzato.
:p.
Per modificare gli attributi dell'area, bisogna aprire il menu di contesto dell'area
per selezionare :hp2."Configurazione":ehp2..

:h3 id=arealistset.Adattamento della lista delle aree
:p.
La lista delle aree stessa ha diversi attributi che ne influenzano la visualizzazione.
Per modificare gli attributi in questione, bisogna aprire il menu di contesto della
lista della aree per selezionare :hp2."Configurazione":ehp2..
:p.
Si possono impostare &colon.
:ul compact.
:li.la visualizzazione di default
:li.l'ordine
:li.i colori da usare per i diversi tipi di area.
:eul.

:h3 id=areafolders.Cartella aree
:p.
Aree possono essere raccolte in cartelle. C'ä sempre una cartella aperta.
La lista aree visualizza solamente le aree contenute nella cartella aperta.
:p.
C'ä una :hp2.cartella di default:ehp2. con alcuni particolari&colon.
:ul compact.
:li.Sta sempre in cima alla gerarchia delle cartelle. Tutte le altre cartelle
sono subordinate.
:li.Non puï essere cancellata o spostata.
:li.Aree non assegnate ad una cartella (per esempio aree appena estratte dalla
configurazione del tosser) vengono collocate nella cartella di default.
:eul.
:p.
Cartelle di aree hanno certe caratteristiche&colon.
:ul compact.
:li.Un nome. Questo puï essere modificato con ALT-click editare successivo.
:li.Un ordine preciso delle aree contenute..
:li.Segnalibro per lo scan automatico.
:eul.
:p.
Con il menu contestuale delle cartelle ä possibile crearne nuove oppure
cancellarne. Cartelle possono essere spostate tramite drag-drop.
:p.
Aree vengono assegnate alle cartelle tramite drag-drop.

.* ****************************** Lista messaggi ****************************
.* @2@ **********************************************************************
:h2 id=msglist.Lista messaggi
:p.
La lista messaggi visualizza tutti i messaggi dell'area in questione. I messaggi
vengono visualizzati nello stesso ordine come si trovano nella base di messaggi.
:p.
Il proprio nome viene rispettivamente visualizzato in un colore diverso. Messaggi
letti e non letti si hanno un numero messaggio a colori diversi. Il colore rispettivo
puï essere stabilito nel taccuino di configurazione della lista messaggi.
:p.
E' possibile selezionare e manipolare pió messaggi con il mouse oppure con la tastiera.
Le possibilitÖ di manipolazione sono&colon.
:ul compact.
:li.Cancella
:li.Copia
:li.Muovi
:li.Stampa
:li.Esporta
:eul.
:p.
Nel caso che sia avvenuto un errore durante la lettura di un messaggio nella base
di messaggi, si visualizza in tutti i casi un :hp2."*":ehp2..
:p.
E' possibile spostare i separatori delle colonne per stabilire la larghezza delle
colonne.
:p.
:artwork align=center name='movesepa.bmp'.

.* ****************************** Threadlist *********************************
.* @2@ **********************************************************************
:h2 id=threadlist.Threadlist
:p.
Si visualizzano i threads dei messaggi nell'area attuale. Messaggi letti
e non letti si visualizzano in colori diversi.
:p.
Threads sono messaggi di un'area legati tra loro, trattandosi di risposte ad
un altro messaggio oppure di messaggi con risposte. Nel caso un messaggio abbia
risposte in una lista di thread, di visualizza un :artwork name='plus.bmp' runin.
davanti al messaggio. E' possibile fare un click con il mouse su
:artwork name='plus.bmp' runin. oppure premere i tasti +-. Le risposte si
visualizzano come struttura ad albero. Il :artwork name='plus.bmp' runin.
diventa un :artwork name='minus.bmp' runin. Nel caso si prema il tasto *, si
apre tutto il ramo.
:p.
Ulteriori argomenti&colon.
:ul compact.
:li.:link reftype=hd refid=thdisp.Modi di visulizzazione:elink.
:li.:link reftype=hd refid=thmani.PossibilitÖ di manipolazione:elink.
:li.:link reftype=hd refid=thlink.Reply-Linker:elink.
:li.:link reftype=hd refid=markmsg.Mettere in evidena tutti i messaggi:elink.
:eul.

:h3 id=thdisp.Modi di visualizzazione
:p.
Esistono tre modi di visualizzazione&colon. Tutti i threads, threads con
messaggi non letti e messaggi non letti.
:parml.
:pt.:hp2.Tutti i threads:ehp2.
:pd.Tutti i threads di un'area si visualizzano completamente.

:pt.:hp2.Threads con messaggi non letti:ehp2.
:pd.Si visualizzano esclusivamente threads nell'area che contengono come
minimo un messaggio non letto. I singoli threads si visualizzano completamente.

:pt.:hp2.Esclusivamente messaggi non letti:ehp2.
:pd.Si visualizzano esclusivamente i messaggi non ancora letti. Nel caso che
esistano messaggi non letti di uno thread senza interruzione, si visualizzano
in tale modo.
Se un thread viene interrotto da un messaggio giÖ letto, tutte le due parti
si visualizzano come due threads.
:eparml.
:p.
Il modo di visualizzazione viene selezionato attraverso il menu di contesto
della lista dei threads. Il modo di default viene stabilito nel setup.
:p.

:h3 id=thmani.PossibilitÖ di manipolazione
:p.
Manipolando i messaggi nella lista dei threads, si manipola sempre
:hp2.Threads:ehp2. oppure :hp2.Parti di threads:ehp2.. Questo significa che
la manipolazione non riguarda solamente un messaggio singolo, ma tutti i
messaggi successivi.
:p.
Threads "imparentati" oppure messaggi prima del messaggio selezionati perï
:hp2.non:ehp2. vengono considerati.
:p.
E' possibile manipolare solamente un thread alla volta. In questo caso si
tratta di una limitazione di OS/2.
:p.
E' possibile
:ul compact.
:li.cancellare threads
:li.muovere threads in un'altra area
:li.copiare threads in un'altra area
:li.esportare threads in un file
:li.stampare threads
:li.mettere in evidenza threads come "letti"
:li.aprire threads
:eul.


:h3 id=thlink.Reply-Linker
:p.
FleetStreet sfrutta esclusivamente informazioni di link che si trovano nella
base di messaggi, cioä non effettua dei links propri. Per il linkare bisogna usare
un altro programma, preferibilmente nel batch file del mailer, immediatamente
dopo il tossing dei messaggi.
:p.
Esistono reply-linker come per esempio&colon.
:ul compact.
:li.Squish
:li.SqmLink
:li.SqLink
:li.QQLink
:eul.
:p.
Squish 1.01 effettua i links dei messaggi, usando la linea di soggetto. C'ä il
vantaggio di poter linkare messaggi anche senza MSGID/REPLY-kludge. Purtroppo
c'ä lo svantaggio che si perde il concatenamento appena si modifica il soggetto.
Inoltre non ä possibile riconoscere dai links, chi risponde a quale persona e
se un messaggio eventualmente ha diverse risposte (le risposte sono sempre
legate in modo lineare).
:p.
Squish 1.10, SqmLink, SqLink e QQLink effettua i links dei messaggi usando i
MSGID/REPLY-kludges, permettendo di abbinare un messaggio esattamente al
messaggio originale. Anche dopo una modifica del soggetto i links si
salvano.
In questo caso i threads non sono pió disposte in modo lineare, ma rispecchiano
un ordine a forma di albero.
La base di messaggi Squish permette al massimo dieci legami per ogni messaggio.
:p.
Bisogna considerare perï lo svantaggio di questo metodo: non si possono
legare messaggi la cui risposta non contiene un REPLY-kludge (per esempio,
se sono stati creati da un QWK-reader, se sono arrivati nell'area attraverso
un gateway o se il REPLY-kludge non corrisponde esattamente agli standard).

:h3 id=markmsg.Aggiorna
:p.Se FleetStreet non ä ancora stato utilizzato per leggere una certa base di messaggi,
tutti i messaggi vengono segnati come "non letti". Perciï tutti i messaggi nell'area
vengono visualizzati nella lista dei threads. Dato che sono giÖ stati letti, questa
voce di menu permette di segnare tutti i messaggi fino a quella attuale come "letti".
Dopo aver fatto questo, la lista dei threads visualizza esclusivamente i messaggi
che non sono ancora stati letti veramente, cioä messaggi che sono stati tossati
nell'area dal tosser dopo la prima lettura della base di messaggi.
:p.Questa funzione di solito occore esclusivamente quando si desidera aggiornare
le aree di messaggi dopo essere passato a FleetStreet. In futuro FleetStreet
amministrerÖ automaticamente l'attributo "letto".

.* @2@ **********************************************************************
:h2 id=bookmarks.Segnalibro
:p.La finestra di segnalibro contiene tre tipi di messaggi&colon.
:ul compact.
:li.Risultati di ricerca
:li.Risultati della ricerca per posta personale
:li.Messaggi segnati
:li.Messaggi non inviati
:eul.
:p.
Usa il menu di contesto della lista per saltare tra i tre tipi di visualizzazione.
:p.
Selezionando "Salva contenuto" nel menu di contesto, il contenuto della lista viene
salvato sul disco fisso quando si termina FleetStreet. Esso viene ricaricato quando
si riavvia FleetStreet.
:p.
Bisogna fare click sul messaggio oppure premere il pulsante :hp2.Al messaggio:ehp2.
per visualizzare il messaggio intero.
:p.
Bisogna premere il pulsante :hp2.Cancella tutto:ehp2. per cancellare la visualizzazione
attuale, cioä tutti i messaggi della visualizzazione vengono tolti dalla lista.
:p.
Bisogna usare il menu di contesto di un messaggio per
:ul compact.
:li.cancellare il messaggio
:li.esportare il messaggio
:li.stampare il messaggio
:li.spostare il messaggio in un'altra area
:li.copiare il messaggio in un'altra area
:li.togliere il messaggio dalla lista
:eul.

.* @2@ **********************************************************************
:h2.Funzione di ricerca
:p.Con la funzione di ricerca ä possibile rintracciare messaggi di una o pió aree
che contengono una data seguenza di caratteri.
:p.Il dialogo di ricerca viene attivato con la voce di menu "Messaggio/Cerca". Bisogna
inserire il testo desiderato e tutte le opzioni di ricerca. Con il pulsante "Start"
viene avviata la ricerca. La ricerca stessa viene effettuato sullo sfondo. Ogni volta
che un'area ä stata controllata, i risultati di ricerca vengono inseriti nella
:link reftype=hd refid=bookmarks.Lista segnalibro:elink. (visualizzazione "Risultati di
ricerca).
:p.Sempre con il dialogo di ricerca viene avviata la ricerca per messaggi personali. I
risultati di ricerca vengono inseriti nella lista segnalibro (visualizzazione "Messaggi
personali").

.* @1@ **********************************************************************
:h1.Uso avanzato

.* @2@ **********************************************************************
:h2 id=enteraddr.Inserimento di indirizzi FTN
:p.Se si scrivono messaggi bisogna indicare l'indirizzo di rete del destinatario.
Di solito si usa l'indirizzo completo 3D oppure 4D.
:p.FleetStreet da una mano completando indirizzo incompleti. Le parti che mancano
vengono completate automaticamente con le parti corrispondenti del proprio indirizzo.
:p.Gli esempi successivi illustrano questo completamento. Come indirizzo di default
si usa
2&colon.2490/2520.17 &colon.
:table cols='12 15 30'.
:row.
:c.Inserimento
:c.Risultato
:c.Commento
:row.
:c.2&colon.2490/2520
:c.2&colon.2490/2520
:c.Indica l'indirizzo 3D
:row.
:c.2520
:c.2&colon.2490/2520
:c.Indica il numero di nodo, rete  &amp. Zona copiata
:row.
:c.247/2099
:c.2&colon.247/2099
:c.Indica rete &amp. Nodo, zona copiati
:row.
:c.1030.42
:c.2&colon.2490/1030.42
:c.Nodo &amp. Indica point, zone &amp. Rete copiata
:row.
:c..42
:c.2&colon.2490/2520.42
:c.Indica point, nodo di boss identico
:etable.
:p.:hp2.Le regole globali sono&colon.:ehp2.
:ol.
:li.Un numero singolo vale come "nodo".
:li.Nel caso che non si indichi un numero di point, si suppone "0".
:eol.

.* @2@ **********************************************************************
:h2.Toolbar
:p.La toolbar di FleetStreet puï essere visualizzata in due dimensioni e postata
in 4 posizioni diverse. Queste opzioni vengono impostate con il menu contestuale
della toolbar. L'ultimo viene aperto facendo click con il tasto destro del mouse
su un posto libero dentro alla cornice della toolbar.


.* @2@ **********************************************************************
:h2.Echo-Manager
:p.L'Echo-Manager serve alla comunicazione semplificato con i programmi di amminstrazione
aree dell'uplink.
:p.Usualmente echos vengono ordinati inviando netmails all'amministratore aree dell'uplink.
Nella linea soggetto viene inserita una password, il testo del messaggio contiene i nomi
degli echos desiderati.
:p.Allo stesso modo ä possibile sganciare echos, richiedere una lista degli echos
disponibili ecc.
:p.L'Echo-Manager facilita questi processi&colon.
:ul compact.
:li.Non ä pió necessario inserire a mano nome,indirizzo e password dell'amminstratore aree.
:li.Aree possono essere selezionate, agganciate oppure sganciate usando il menu contestuale.
:li.I messaggi all'amministratore aree vengono create automaticamente.
:li.Gli echos agganciati vengono inseriti nel file di configurazione del tosser.
:eul.

:h3.Lista echos
:p.Per funzionare bene l'Echo-Manager ha bisogno di una lista echos disponibili
dall'uplink. Nel caso che manchi bisogna procararsela scrivendo un messaggio all'uplink
per richiederla.
:p.La risposta dell'amminstratore aree contiene una lista degli echos disponibili.
Bisogna passare questa lista all'amministratore echo aree selezionando la voce di
menu "Setup/Estrai aree". Adesso FleetStreet cerca nel messaggio attuale per nomi
echo utilizzandoli nell'Echo-Manager.
:p.FleetStreet memorizza l'indirizzo del mittente per ogni lista echos in questione.
Quando vengono estratte aree essendo giÖ presente una lista dell'identico mittente,
la lista vecchia viene sostituita da quella nuova. Altrimenti il mittente viene
inserito come uplink nuovo.

:h3.Configurazione
:p.Per comunicare con l'amministratore aree dell'uplink ci vuole una password. Bisogna
aprire l'Echo-Manager, il menu contestuale della lista e selezionare "Impostazioni".
Sulla prima pagina di questo taccuino impostazioni si trova una lista dell'uplink
conosciuto, per il quale si possono inserire il nome e la password per l'amministratore aree.
:p.
:hp8.Attenzione&colon.:ehp8. Ci vuole assolutamente l'inserimento del nome e della password
per poter utilizzare l'Echo-Manager pió tardi.

:h3.Uso dell'Echo-Manager
:p.Per agganciare oppure sganciare echos con l'aiuto dell'Echo-Manager bisogna eseguire
i seguenti passi&colon.
:ol.
:li.Bisogna impostare come indirizzo attuale quello da utilizzare per scrivere un
messaggio all'uplink. Nel caso ci siano pió di uno per reti diverse e ci siano aree netmail
separate per ognuna, bisogna passare all'area adatta. Con un solo uplink bisogna passare
all'area appropriata di netmail.
:li.Aprendo il menu dell'Echo-Manager, viene visualizzata la lista echos dell'uplink in
questione. Se la lista ä vuota bisogna prima ordinarne una dall'uplink manualmente.
:li.Bisogna aprire il menu contestuale dell'echo da agganciare e selezionareci
:hp2."Aggancia":ehp2..
:li.L'agganciare oppure lo sganciare di ulteriori echos funziona allo stesso modo.
L'azione desiderata viene visualizzata nella riga dell'echo.
:li.Bisogna premere :hp2.OK:ehp2.. La netmail all'uplink viene creata.
:eol.

:h3.DLL di ampliamento
:p.Squish non ä capace di inserire automaticamente echos nuovi nel suo file di configurazione.
Per questa ragione FleetStreet puï caricare una DLL, quando viene agganciato o sganciato un echo,
e lanciare certe funzione. La DLL ä capace di inserire gli echos.
:p.
La DLL da utilizzare puï essere indicata nel taccuino delle impostazioni dell'Echo-Manager.
:p.
Il file :hp2.FLTCF_SQ.DLL:ehp2. fornito con FleetStreet puï essere utilizzato come DLL di
ampliamento se (e solamente se) Squish viene usato come tosser con una SQUISH.CFG standard.
Per altre combinazioni (pointkit, altri tosser, ecc.) esistono eventualmente DLL rispettive,
offerte da terzi.
:p.
Informazioni tecniche per creare DLL di ampliamento possono essere richieste dall'
:link reftype=hd refid=support.autore:elink..

.* @2@ **********************************************************************
:h2 id=cclists.Uso di liste di carbon copy
:p.L'uso di una lista di carbon copy significa che l'identico messaggio viene inviato
a diversi destinatari. Liste di carbon copy si possono utilizzare esclusivamente
per netmail.
:p.Si immagini l'esempio successivo&colon.
:p.Si compone una newsletter che periodicamente viene inviato ad un certo numero
di persone. Per risolvere questo compito con FleetStreet bastera' creare una
lista di carbon copy del nome "Newsletter" . Adesso nella lista si possono
inserire tutte le persone che devono ricevere la newsletter.
:p.Nel caso si desideri inviare questa newsletter via netmail, bisogna comporre
un normalissimo messaggio. Invece di inserire un destinatario si seleziona :hp2.Usa carbon copy:ehp2.
dal menu, per poi scegliere la lista di carbon copy "Newsletter" scritta prima.
Si visualizza il testo "*** Newsletter ***" per indicare l'uso di questa lista
di carbon copy.
:p.Nel caso si desideri inviare il prossimo numero della newsletter bastera'
riutilizzare questa lista, e non ä necessario crearne una nuova.

:h3.Amministrazione di liste carbon copy
:p.Tutte le liste carbon copy si trovano nella cartella delle liste carbon copy. L'ultimo viene
aperto con la voce di menu :hp2."Setup/Liste carbon copy":ehp2.. Le liste carbon copy vengono
rapresentate come icone.
:p.Bisogna utilizzare il menu contestuale della lista per cancellarne una, oppure per
creare una lista nuova. Un click doppio su una lista la apre per visualizzarne oppure
modificarne il contenuto.
:p.Bisogna tenere premuto il tasto ALT e fare click su una lista carbon copy per modificarne
il nome.

:h3.Importa
:p.Il contenuto di un file di testo puï essere importato in una lista carbon copy.
Il file deve essere un file di testo nel formato successivo&colon.
:ul compact.
:li.Ogni riga deve contenere precisamente un nome e un indirizzo
:li.I campi di una riga sono separati da almeno uno spazio vuoto
:li.Si ignorano righe vuote
:li.Si ignorano righe con un semicolon all'inizio
:li.Si ignorano righe in un formato non valido
:eul.
:p.
Esempio&colon.
:xmp.
; Commento
Michael Hohner 2&colon.2490/2520.17
Hans Dampf 1&colon.234/567
:exmp.

:h3.Distributore rapido
:p.Liste carbon copy "normali" vengono salvate con la terminazione del programma e possono,
perciï, sempre essere messe a disposizione. Spesso, perï, si desidera un utilizzo diverso,
per cui ä stata creata la funzione :hp2."Distributore rapido":ehp2. nel menu :hp2."Speciale":ehp2..
:p.Il distributore rapido principalmente possiede le stesse funzioni come la lista carbon copy,
ma viene creato quando si scrive un messaggio. Inoltre il contenuto non viene salvato dopo aver
salvato il messaggio.

.* @2@ **********************************************************************
:h2 id=crosspost.'Crosspostare' messaggi
:p.Crosspostare significa che lo stesso messaggio viene salvato in pió aree.
:p.E' possibile attivare il crossposting selezionando :hp2.Crossposting:ehp2.
nella voce menu. Questa funzione ä disponibile esclusivamente quando si scrive.
:p.La lista delle aree viene aperta, e
si possono selezionare le aree in cui si desidera salvare il messaggio.
:p.:hp8.Attenzione&colon.:ehp8. Non ä necessario selezionare l'area attuale nella
lista. Il messaggio viene sempre salvato nell'area attuale. Basta selezionare
le aree supplementari.
:p.Nel caso che il modo crosspostare sia attivato, nel menu si visualizza un
uncino prima del :hp2.Crosspostare:ehp2.
:p.E' possibile disattivare il modo crosspostare selezionando :hp2.Crosspostare:ehp2.
un'altra volta nel menu. L'uncino sparisce e visualizza che FleetStreet ä tornato
nel modo editor normale.

.* @2@ **********************************************************************
:h2.Nomi abbreviati
:p.Con FleetStreet ä possibile definire nomi abbreviati per utenti ai quali
si scrive spesso.
:p.
Per usare questi nomi abbreviati bisogna inserirne uno nel campo del destinatario
per poi premere "Enter". Il nome abbreviato viene automaticamente sostituito dal
nome vero, e l'indirizzo del destinatario viene inserito nel campo dell'indirizzo.
Viene anche inserito il soggetto campo corrispondente nel caso che sia stato
definito.
:p.:hp2.Per definire un nome abbreviato bisogna eseguire i passi successivi&colon.:ehp2.
:ol compact.
:li.Premi il pulsante "Aggiungi",
:li.riempi i campi,
:li.premi "OK".
:eol.
:p.:hp2.Per modificare un nome abbreviato bisogna eseguire i passi successivi&colon.:ehp2.
:ol compact.
:li.Seleziona il nome abbreviato dalla lista,
:li.premi il pulsante "Modifica", oppure fai un click doppio sull'inserimento,
:li.esegui le modifiche,
:li.premi "OK".
:eol.
:p.:hp2.Per cancellare un nome abbreviato bisogna eseguire i passi successivi&colon.:ehp2.
:ol compact.
:li.Seleziona un nome abbreviato dalla lista,
:li.premi il pulsante "Cancella".
:eol.

.* @2@ **********************************************************************
:h2 id=nodelists.Uso di nodelists
:p.:hp2.Di che cosa si tratta?:ehp2.
:p.Le nodelists contengono, tra l'altro, il nome e l'indirizzo di rete dei partecipanti
ad una rete.
FleetStreet ä capace di cercare in una nodelist del genere l'indirizzo
corrispondente al nome di un destinatario, e viceverso.
:p.Ci vuole perï una nodelist in forma compilata, del tipo "versione 7".
Questo formato viene creato, per esempio, da FastLst.
FleetStreet ha bisogno del file dati e dell'indice sysop e di una configurazione
corrispondente del compilatore della nodelist.
:p.:hp2.Come riesce FleetStreet a trovre le nodelists?:ehp2.
:p.Nel setup ci sono pagine di notebook per le nodelists. Per ogni nodelist
bisogna stabilire un "domain". Un inserimento di domain contiene il nome del
domain e i nomi path dei files di nodelist. Bisogna indicare il path completo,
drive e estensione compresi.
:p.:hp2.Cosa ci vuole ancora?:ehp2.
:p.Volendo si possono anche indicare i nomi
per le nodelist-flags.
:p.:hp2.In che modo si possono usare le nodelists?:ehp2.
:p.Scrivendo un netmail conviene inserire il nome completo del destinatario
oppure una parte del cognome. Premi ENTER. Se il nome si trova, l'indirizzo
viene subito inserito nel campo dell'indirizzo. Se ce ne sono pió versioni,
si visualizza un dialogo da selezione.
:p.Alcuni dialoghi che richiedono l'inserimento di un indirizzo, visualizzano
un pulsante "?". Bisogna premerlo per rintracciare l'indirizzo corrispondente
ad un nome.

.* @2@ **********************************************************************
:h2 id=templates.Template
:p.Templates di messaggi si usano per definire un certo aspetto di messaggi nuovi,
risposte o carbon copies. Nel caso che si scriva un messaggio nuovo, una carbon
copy o si risponda, le parti predefinite di un messaggio si combinano con il testo
del messsaggio.
:link reftype=hd refid=tokens.Abbreviazioni particolari:elink. nel template messaggio
si sostituiscono con certe parti del messaggio originale, nel caso che si usi il template.
:p.Il template messaggio viene usato nel caso si acceda alla finestra dell'editor per la
prima volta, si scriva, si forwardi un messaggio, oppure si risponda a un messaggio.
Nel caso che si abbia giÖ inserito un soggetto, oppure un nome del destinatario,
questi si possono usare per la sostituzione delle abbreviazioni. Le abbreviazioni
si sostituiscono come "vuoti" nel caso che non siano state inserite.
:p.:hp2.Ordine generale&colon.:ehp2.
.br
:hp2.Messaggio nuovo&colon.:ehp2.
:xmp.
[Header]
[Testo messaggio]
[Footer]
:exmp.
:p.:hp2.Risposta&colon.:ehp2.
:xmp.
[Header]
[Testo risposta]
[Testo messaggio]
[Footer]
:exmp.
:p.:hp2.Risposta in un'altra area&colon.:ehp2.
:xmp.
[Testo risposta in un'altra area]
[Header]
[Testo risposta]
[Testo messaggio]
[Footer]
:exmp.
:p.:hp2.Crosspost&colon.:ehp2.
:xmp.
[Testo crosspost]
[Header]
[Testo messaggio]
[Footer]
:exmp.
:p.:hp2.Testo carbon copy&colon.:ehp2.
:xmp.
[Testo carbon copy]
[Header]
[Testo messaggio]
[Footer]
:exmp.
:p.:hp2.Forward&colon.:ehp2.
:xmp.
[Testo forward]
[Testo messaggio originale]
[Footer forward]
[Header]
[Footer]
:exmp.


:h3 id=tokens.Abbreviazioni
:p.Le abbreviazioni successive sono disponibili nei message-template&colon.
:parml break=none.
:pt.:hp2.%T:ehp2.
:pd.Nome del destinatario (messaggio originale)
:pt.:hp2.%Z:ehp2.
:pd.Cognome del destinatario (messaggio originale)
:pt.:hp2.%R:ehp2.
:pd.Indirizzo del destinatario (messaggio originale). Questa abbreviazione
viene ignorata con risposte o echomail forwardato.
:pt.:hp2.%O:ehp2.
:pd.Nome del destinatario (messaggio nuovo)
:pt.:hp2.%P:ehp2.
:pd.Cognome del destinatario (messaggio nuovo)
:pt.:hp2.%F:ehp2.
:pd.Nome del mittente
:pt.:hp2.%G:ehp2.
:pd.Cognome del mittente
:pt.:hp2.%J:ehp2.
:pd.Indirizzo del mittente
:pt.:hp2.%A:ehp2.
:pd.Area-tag. Nel caso si crossposti un messaggio [che parola!], questa ä l'area
di destinazione. Diventa l'area originale sei si risponde in un'area diversa
oppure si forwarda un messaggio.
:pt.:hp2.%E:ehp2.
:pd.Descrizione dell'area. Nel caso si crossposti un messaggio, questa ä l'area
di destinazione. Diventa l'area originale se si risponde in un'area diversa
oppure si forwarda un messaggio.
:pt.:hp2.%U:ehp2.
:pd.Il tuo nome
:pt.:hp2.%I:ehp2.
:pd.Il tuo cognome
:pt.:hp2.%W:ehp2.
:pd.Il tuo indirizzo
:pt.:hp2.%C:ehp2.
:pd.Nome dell'utente in una lista di carbon copy
:pt.:hp2.%D:ehp2.
:pd.Data del messaggio a cui si risponde
:pt.:hp2.%M:ehp2.
:pd.Tempo del messaggio a cui si risponde
:pt.:hp2.%S:ehp2.
:pd.Soggetto del messaggio a cui si risponde
:pt.:hp2.%%:ehp2.
:pd.Un %
:eparml.

.* @2@ **********************************************************************
:h2.Mapping
:p.
Se la base di messaggi si trova su un altro computer in una rete locale
ä opportuno mantenere un unico file di configurazione del tosser sullo
stesso computer. Usando questo file di configurazione del tosser in FleetStreet
se i drives di rete sono montati su altri caratteri di drive locali, FleetStreet
riconoscerebbe i drives sbagliati quando accede alla base di messaggi.
:p.
Il mapping dei drives offre una soluzione a questo problema. E' possibile
assegnare ai drive di rete altri caratteri di drive locali. FleetStreet
sostituisce i caratteri di drives di rete con i caratteri assegnati
prima che si acceda alla base di messaggi.
:p.
Esempio&colon.
:p.
La base di messaggi si trova sul computer A sul drive D&colon.. FleetStreet
ä avviato sul computer B, drive D&colon. del computer A qui ä montato sul
drive E&colon.. Il file SQUISH.CFG si trova sempre sul computer A, i nomi dei
files dell'area perciï vengono indicati con D&colon. come carattere di drive.
:p.
Se FleetStreet viene installato in modo che drive D viene sostituito da&colon. drive E&colon.
le lettere di drive dei area-files dalla SQUISH.CFG vengono sostituiti da E&colon.,
si usano i nomi corretti dei files.
:p.
Attenzione&colon. Si sostituiscono esclusivamente le lettere di drive di nomi file
le quali sono state estratte dalla SQUISH.CFG. Se in FleetStreet vengono create delle
aree bisogna indicare il drive corretto. FleetStreet non sostituisce questi nomi
file.


.* @2@ **********************************************************************
:h2 id=colorsetup.Mofificare colori e caratteri
:i2 refid=setup.Colori e caratteri
:p.Non si troverÖ un menu o un dialogo per modificare i colori o i caratteri
della finestra principale. FleetStreet sfrutta gli oggetti WPS per impostare
queste cose.
:parml tsize=3.
:pt.:hp2.La tavolozza dei colori:ehp2.
:pd.Bisogna aprire una tavolozza di colori, trascinare un colore sull'elemento
corrispondente della finestra di FleetStreet e lasciarlo 'cadere'. Per modificare
il colore di primo piano (colore dei caratteri), bisogna tenere premuto il tasto
CTRL.
:pt.:hp2.La tavolozza dei caratteri:ehp2.
:pd.Bisogna aprire una tavolozza di caratteri, trascinare un carattere sull'elemento
corrispondente della finestra di FleetStreet e lasciarlo 'cadere'.
:eparml.

.* @2@ **********************************************************************
:h2.Importa, Esporta
:p.
Quando si scrive un messaggio ä possibile importare un file di testo alla posizione
attuale del cursore.
:ul.
:li.Bisogna trascinare il file da una cartella WPS e lasciarla cadere sulla finestra
dell'editor.
:artwork align=center name='dropfile.bmp'.
:li.Bisogna usare la funzione Importa nel menu "File".
:eul.
:p.
Durante la letture ä possibile esportare un messaggio in un file di testo normale.
:ul.
:li.Bisogna trascinare il messaggio ad una cartella WPS e lasciarlo cadere sulla
finestra dell'editor.
:li.Bisogna usare il dialogo Esporta nel menu "File".
:eul.

.* @1@ **********************************************************************
:h1 id=advtopics.Argomenti avanzati
:p.
Gli argomenti successivi potrebbero interessarvi appena acquistata un po'
di esperienza con FleetStreet&colon.
:ul.
:li.:link reftype=hd refid=multinst.Istanze multiple:elink.
:li.:link reftype=hd refid=perform.Miglioramento della performance:elink.
:li.:link reftype=hd refid=multuser.Uso-Multi-User:elink.
:li.:link reftype=hd refid=kludges.Kludge-Lines:elink.
:eul.

.* ************************** Istanze multiple ******************************
.* @2@ **********************************************************************
:h2 id=multinst.Istanze multiple
:p.
E' possibile avviare FleetStreet pió di una volta contemporaneamente. Bisogna
perï considerare i seguenti punti&colon.
:ul.
:li.Solamente la prima istanza avviata salva le impostazioni del
programma, dopo averlo terminato. Nel caso si desideri mantenere le modifiche,
bisogna salvare il Setup nella prima istanza.
:li.Istanze secondarie si riconoscono dal :hp2.[*]:ehp2. nella linea
di titolo.
:li.Non ä possibile avviare istanze multiple con diverse versioni di
FleetStreet, come del resto non ä possibile lanciare versioni di lingue diverse.
In un caso del genere si usano le DLLs della prima istanza per ogni successiva.
Si tratta di una limitazione di OS/2. Il comportamento delle istanze secondarie
con versioni diverse non ä definito.
:li.Istanze secondarie non possiedono un Pipe-Server.
:eul.

.* ************************** Performance       ******************************
.* @2@ **********************************************************************
:h2 id=perform.Miglioramenti della performance
:p.La performance di FleetStreet dipende in gran parte da due fattori&colon.
Performance della MSGAPI e performance dei dischi fissi. Mentre la MSGAPI si
sottrae alla nostra influenza, ä possibile applicare modifiche che riguardano
il secondo punto e che possono migliorare la performance di FleetStreet.
:p.Ne seguono alcuni suggerimenti&colon.
:ul.
:li.Conviene usare, possibilmente, aree in formato Squish al posto di aree in formato *.MSG.
:li.E' opportuno compattare le aree Squish con SqPackP regolarmente,
garantendo un ordine dei singoli messaggi regolare e ascendente nella base di messaggi.
:li.Conviene usare HPFS.
:li.Conviene escludere le aree che non desideri leggere dalla lista delle aree
Conviene attivare "Nascondi aree escluse".
:li.Conviene far cercare un testo - possibilmente - esclusivamente nello header
e non nello header &amp. e testo.
:eul.

.* ************************** Multi-User   ***********************************
.* @2@ **********************************************************************
:h2 id=multuser.Avviamento utenti multipli
:p.Entro certi limiti FleetStreet ä utilizzabile da pió utenti. Bisogna fare
cosç&colon.
:ol.
:li.Si crei una directory per FleetStreet per copiarci il file *.EXE e tutte le
DLLs.
:li.Per ogni utente si crei una directory di configurazione.
:li.Per ogni utente si crei un oggetto di programma, inserendo come nome di
programma il path completo di FLTSTRT.EXE.
:li.Bisogna indicare il :link reftype=hd refid=cmdlin.parametro della linea di comando:elink. "-C"
che indica il file di configurazione per ogni utente, per esempio "-Cd&colon.\fleet\user1".
:li.Adesso ä possibile configurare FleetStreet per ogni utente separatemente.
Le directories di configurazione conterranno un set proprio di files *.INI.
:li.Bisogna considerare per ogni utente Lastread-Offsets separati.
:eol.
:p.:hp2.Limitazioni&colon.:ehp2.
:ul.
:li.Il flag "letto" esiste una sola volta per ogni messaggio per cui
ä identico per tutti gli utenti.
:li.Il flag "privato" non viene considerato separatamente.
:eul.

.* ************************** Howto        ***********************************
.* @2@ **********************************************************************
:h2.Come ä possibile...
:p.
I seguenti paragrafi descrivono azioni frequenti e come realizzarli con FleetStreet.

:h3.Inviare files con un messaggio?
:p.
Files possono essere inviati insieme con un messaggio. I nomi dei files vengono inseriti
nella linea di soggetto del messaggio. Bisogna separare i nomi di files diversi con almeno
uno spazio vuoto. Inoltre ä necessario mettere l'attributo di messaggio "File attaccato",
per comunicare al tosser oppure al mailer come interpretare la linea di soggetto.
:p.
E' possibile trascinare files da qualsiasi cartella WPS sulla linea di soggetto per attaccarli
al messaggio. I nomi files vengono inseriti automaticamente, l'attributo "File attaccato" viene
attivato e si visualizza un riassunto dei files attaccati. Questa funzione ä disponibile
solo durante la scrittura di un messaggio!
:artwork align=center name='attfile.bmp'.

.* ************************* Rexx ******************************************

.* @1@ **********************************************************************
:h1.Rexx scripts
:p.FleetStreet ha la capacitÖ di eseguire scripts nel linguaggio Rexx. Il linguaggio
Rexx viene ampliato da :hp2.variabili predefinite:ehp2. e :hp2.funzioni:ehp2..

.* ***************************** Referenz   ************************************

.* @2@ **********************************************************************
:h2.Riferimento per il programmatore
:p.Questa riferimento elenca tutte le :link reftype=hd refid=rexxvar.variabili predfinite:elink.,
in pió :link reftype=hd refid=rexxfunc.funzioni:elink..
:p.Bisogna leggere la documentazione online per informarsi delle proprietÖ di Rexx
standard.

:h3.Il FleetStreet-Environment
:p.Rexx-scripts avviati sotto FleetStreet non vengono eseguiti sotto lo
environment prestabilito CMD.EXE. Lo environment Rexx per scripts sotto
FleetStreet si chiama :hp2.FLEETSTREET:ehp2..
:p.Se si usano comandi non-Rexx in uno script, questi comandi vengono interpretati
da FleetStreet. Se vuoi che CMD.EXE interpreti i comandi in questione bisogna
indirizzare esplicitamente l'environment CMD.EXE, con il comando Rexx :hp2.ADDRESS:ehp2..
:p.Esempio&colon.
:xmp.
/* SBAGLIATO! */
'dir'

/* corretto */
address CMD 'dir'

/* anche corretto */
address CMD
'dir'
address FLEETSTREET
:exmp.
:p.Bisogna leggere nella guida online su Rexx per sapere di pió dell'environment Rexx
e il comando ADDRESS.

.* ***************************** Variabili  ************************************

:h3 id=rexxvar.Variabili predefinite
:p.Quando viene avviato uno script Rexx alcune variabili hanno giÖ dati valori.
Queste variabili ed i valori corrispondenti si possono usare nello script.
:p.:hp8.Attenzione&colon.:ehp8. Se il valore di una variabile predefinita nello script
viene modificata, le modifiche non hanno effetto su FleetStreet finchÇ non si
usa una funzione o un comando di FleetStreet per effettuare le modifiche.
:p.Le variabili predifinite sono&colon.
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
:p.Si tratta di un'array dei nomi degli utenti. :hp4.FleetSetup.Names.0:ehp4. contiene il
numero degli elementi nell'array. :hp4.FleetSetup.Names.1:ehp4. etc. contengono i nomi.
:p.Esempio&colon.
:table cols='20 10'.
:row.
:c.Variabile
:c.Valore
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
:p.Si tratta di un array di indirizzi degli utenti. :hp4.FleetSetup.Addresses.0:ehp4.
contiene il numero degli elementi nell'array. :hp4.FleetSetup.Addresses.1:ehp4. etc.
contengono gli indirizzi.
:p.Esempio&colon.
:table cols='22 14'.
:row.
:c.Variabile
:c.Valore
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
:p.:hp4.FleetSetup.Echotoss:ehp4. contiene il nome del file Echotoss.Log.

:h4 id=rvar04.FleetSetup.Tosser
:p.:hp4.FleetSetup.Tosser:ehp4. contiene il nome del file di configurazione del tosser.

:h4 id=rvar05.FleetStatus.Area
:p.:hp4.FleetStatus.Area:ehp4. ä un gruppo di variabili che contengono le informazioni
sull'area attuale.
:p.Le variabili sono&colon.
:parml.
:pt.:hp4.FleetStatus.Area.Tag:ehp4.
:pd.Area-tag
:pt.:hp4.FleetStatus.Area.Desc:ehp4.
:pd.Descrizione dell'area
:pt.:hp4.FleetStatus.Area.File:ehp4.
:pd.Path e/o nome file dell'area
:pt.:hp4.FleetStatus.Area.Format:ehp4.
:pd.Formato dell'area. Il valore puï essere "*.MSG", "Squish" oppure "JAM"
:pt.:hp4.FleetStatus.Area.Type:ehp4.
:pd.Tipo dell'area. Il valore puï essere "Echo", "Net", "Local" e "Private".
:eparml.

:h4 id=rvar06.FleetStatus.DestArea
:p.:hp4.FleetStatus.DestArea:ehp4. contiene l'area-tag dell'area dove si desidera
salvare il messaggio quando si risponde o si forwarda.
:p.Questa variabile ä definita esclusivamente quando si edita un messaggio!

:h4 id=rvar07.FleetStatus.Name
:p.:hp4.FleetStatus.Name:ehp4. contiene il nome dell'utente momentaneamente attivo.

:h4 id=rvar08.FleetStatus.Address
:p.:hp4.FleetStatus.Address:ehp4. contiene l'indirizzo dell'utente momentaneamente attivo.

:h4 id=rvar09.FleetStatus.Mode
:p.:hp4.FleetStatus.Mode:ehp4. contiene lo stato attuale del programma. La variabile
puï avere uno dei seguenti valori&colon.
:sl compact.
:li.No Setup
:li.Edit Single
:li.Edit XPost
:li.Edit CCopy
:li.Read
:li.Cleanup
:esl.
:p.Quando si scrive un messaggio, la prima parola nella variabile ä "Edit". La seconda
parola indica se si desidera scrivere un messaggio singolo oppure se si vuole
usare una lista di carbon copy o la funzione crosspost..

:h4 id=rvar10.FleetStatus.Monitor
:p.:hp4.FleetStatus.Monitor:ehp4. puï essere "0" se non viene usata una finestra
di monitor, oppure "1" se lo script viene avviato in una finestra di monitor.
:hp4.FleetStatus.Monitor:ehp4. puï essere usato direttamente in un'espressione booleana.

:h4 id=rvar11.FleetStatus.Cursor
:p.Quando si scrive un messaggio, :hp4.FleetStatus.Cursor:ehp4. contiene due numeri
che indicano la posizione attuale del cursore. Il primo numero indica il paragrafo,
il secondo la posizione del segno nel paragrafo. "46 3" per esempio significa
che il cursore sta alla posizione del terzo segno nel paragrafo 46.
:p.
Il seguente programma visualizza il testo dopo il cursore&colon.
:xmp.
para = word(FleetStatus.Cursor, 1)
offs = word(FleetStatus.Cursor, 2)
say substr(FleetMsg.Text.para, offs)
:exmp.
:p.
Durante la lettura di messaggi :hp4.FleetStatus.Cursor:ehp4. non ä definito.

:h4 id=rvar12.FleetMsg.Header
:p.:hp4.FleetMsg.Header:ehp4. ä un gruppo di variabili che contengono le informazioni
sullo header del messaggio attuale.
:p.Le variabili sono&colon.
:parml.
:pt.:hp4.FleetMsg.Header.Attrib:ehp4.
:pd.Gli attributi del messaggio, come nella linea "Attrib".
:pt.:hp4.FleetMsg.Header.From:ehp4.
:pd.Nome del mittente.
:pt.:hp4.FleetMsg.Header.FromAddress:ehp4.
:pd.Indirizzo del mittente.
:pt.:hp4.FleetMsg.Header.To:ehp4.
:pd.Nome del destinatario.
:pt.:hp4.FleetMsg.Header.ToAddress:ehp4.
:pd.Indirizzo del destinatario. Non usare questa variabile in echo-areas!
:pt.:hp4.FleetMsg.Header.Subj:ehp4.
:pd.Soggetto.
:pt.:hp4.FleetMsg.Header.DateWritten:ehp4.
:pd.Data e ora della stesura del messaggio.
:pt.:hp4.FleetMsg.Header.DateReceived:ehp4.
:pd.Data e ora del tossing di un messaggio.
:eparml.

:h4 id=rvar13.FleetMsg.Text
:p.:hp4.FleetMsg.Text:ehp4. ä un array di paragrafi di testo. :hp4.FleetMsg.Text.0:ehp4.
contiene il numero di elementi nell'array. :hp4.FleetMsg.Text.1:ehp4. etc.
contengono il testo del messaggio.
:p.:hp8.Attenzione&colon.:ehp8. Gli elementi dell'array :hp2.non:ehp2. sono righe di testo,
ma paragrafi. Un paragrafo nel testo originale termina con un carattere di fine riga. Se il
testo viene modificato - per esempio inserendo o cancellando parole - l'indentazione
dovrebbe venir corretto. Se hai bisogno di un formato proprio con una certa lunghezza
di riga bisogna programmare un'indentazione propria. Non dovrebbe essere difficile
in Rexx.

:h4 id=rvar16.FleetMsg.Kludges
:p.:hp4.FleetMsg.Kludges:ehp4. ä un array che contiene le kludge-line del messaggio. L'array
ä definito esclusivamente durante la lettura di messaggi.
:p.
I campi di :hp4.FleetMsg.Kludges:ehp4. sono &colon.
:parml.
:pt.:hp2.FleetMsg.Kludges.0:ehp2.
:pd.Numero degli elementi nell'array
:pt.:hp2.FleetMsg.Kludges.1:ehp2.
:pd.Prima kludge-line
:pt.:hp2.FleetMsg.Kludges.*:ehp2.
:pd.tutti le altre kludge-line
:eparml.
:p.
Le kludge-line hanno quasi sempre la forma
:xmp.
NOME: valore
:exmp.
:p.
oppure
:xmp.
NOME valore
:exmp.
:p.
Una kludge-line puï esserci anche alcune volte. Le kludge-line non hanno un ordine preciso.
Il carattere :hp2.01 hex:ehp2. all'inizio delle kludge-line non ä contenuto nelle variabili.

:h4 id=rvar17.FleetMsg.Seenbys
:p.:hp4.FleetMsg.Seenbys:ehp4. ä un array che contine le righe SEEN-BY
del messaggio. L'array ä definito esclusivamente durante la letturea di messaggi.
:p.
I campi di :hp4.FleetMsg.Seenbys:ehp4. sono &colon.
:parml.
:pt.:hp2.FleetMsg.Seenbys.0:ehp2.
:pd.Numero degli elementi nell'array
:pt.:hp2.FleetMsg.Seenbys.1:ehp2.
:pd.Prima riga
:pt.:hp2.FleetMsg.Seenbys.*:ehp2.
:pd.tutte le alre righe
:eparml.
:p.
Righe SEEN-BY hanno la forma
:xmp.
SEEN-BY: nodes
:exmp.
:p.
Le righe hanno lo stesso ordine come nel messaggio originale.

:h4 id=rvar14.FleetCCopy
:p.Quando si scrive un messaggio usando una lista di carbon copy (oppure
una carbon copy veloce), il Rexx-array :hp4.FleetCCopy:ehp4. contiene i nomi
e gli indirizzi della lista di carbon copy. Se non viene usata una lista di
carbon copy le variabili non sono definite.
:p.
I campi di :hp4.FleetCCopy:ehp4. sono&colon.
:parml.
:pt.:hp2.FleetCCopy.0:ehp2.
:pd.Numero degli inserimenti nella lista di carbon copy.
:pt.:hp2.FleetCCopy.1.Name:ehp2.
:pd.Nome del primo inserimento.
:pt.:hp2.FleetCCopy.1.Address:ehp2.
:pd.Indirizzo del primo inserimento.
:pt.:hp2.&dot.&dot.&dot.:ehp2.
:pd.
:eparml.

:h4 id=rvar15.NewMail
:p.:hp4.NewMail:ehp4. puï essere una combinazione dei seguenti valori (separati da
uno spazio vuoto)&colon.
:parml break=none.
:pt.:hp2.'Echo':ehp2.
:pd.E' stata scritta nuova echomail
:pt.:hp2.'Net':ehp2.
:pd.E' stata scritta nuova netmail
:pt.:hp2.'Local':ehp2.
:pd.E' stata scritta nuova mail locale
:eparml.
:p.
Nel caso che non siano stati scritti messaggi, la variabile resta vuota.
:p.
:hp8.Attenzione&colon.:ehp8. Questa variabile ha solamente un valore durante l'escuzione,
alla fine del programma, cioä se lo script viene eseguito come hook di fine programma.
Altrimenti resta non definita.

.* ***************************** Funzioni ************************************

:h3 id=rexxfunc.Funzioni
:p.FleetStreet offre alcune funzioni Rexx nuove. Si tratta di&colon.
:sl compact.
:li.:hp4.FSCls:ehp4.
:li.:hp4.FSLookupAddress:ehp4.
:li.:hp4.FSLookupName:ehp4.
:li.:hp4.FSSetEntryField:ehp4.
:li.:hp4.FSSetHeader:ehp4.
:li.:hp4.FSSetText:ehp4.
:esl.

:h4.FSCls
:p.:hp4.FSCls:ehp4. cancella la finestra di monitor.
:p.:hp2.Sintassi&colon.:ehp2.
:xmp.
result = FSCls()
:exmp.
:p.:hp2.Parametri&colon.:ehp2.
:p.
FSCls non ha bisogno di parametri.
:p.:hp2.Valori restituiti&colon.:ehp2.
:p.:hp4.FSCls:ehp4. restituisce "OK" se la finestra di monitor ä stata cancellata.
Viene restituito "NoMonitor" se lo script viene avviato senza finestra di monitor.
:p.:hp2.Esempio&colon.:ehp2.
:xmp.
call FSCls
:exmp.

:h4.FSSetHeader
:p.:hp4.FSSetHeader:ehp4. aspetta un Rexx-Array come parametro e usa il contenuto
della variabile come header nuovo del messaggio.
:p.:hp2.Sintassi&colon.:ehp2.
:xmp.
result = FSSetHeader(stem)
:exmp.
:p.:hp2.Parametri&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.Rexx-Array che contiene il contenuto dello header del messaggio. stem possiede
i seguenti campi&colon.
:ul compact.
:li.From
:li.FromAddress
:li.To
:li.ToAddress
:li.Subj
:eul.
:p.
Si tratta degli stessi campi come in :hp4.FleetMsg.Header:ehp4. ma si usano invece
esclusivamente i campi di cui sopra.
:eparml.
:p.
:hp2.Valori restituiti&colon.:ehp2.
:p.:hp4.FSSetHeader:ehp4. restituisce "OK".
:p.
:p.:hp2.Note&colon.:ehp2.
:ul.
:li.Tutti gli elementi dell'array devono avere un valore, anche se si tratta
solamente di una stringa zero.
:li.Bisogna sempre mettere il nome di default tra virgolette per non farlo
sostituire dal suo stesso valore.
:li.Durante la lettura lo header nuovo non viene salvato sul disco fisso.
Durante la scrittura lo header nuovo viene salvato solamente se si salva il messaggio
intero (Ctrl-S).
:eul.
:p.
:p.:hp2.Esempio&colon.:ehp2.
:xmp.
/* Replace sender name */
FleetMsg.Header.From = 'Joe user'
RetVal = FSSetHeader('FleetMsg.Header')
:exmp.


:h4.FSSetText
:p.:hp4.FSSetText:ehp4. aspetta una Rexx-array come parametro ed usa il testo
nell'array come testo di messaggio attuale. Il testo nell'array sostituisce il
testo precedente..
:p.:hp2.Sintassi&colon.:ehp2.
:xmp.
result = FSSetText(stem)
:exmp.
:p.
:p.:hp2.Parametri&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.Rexx-array che contiene il testo di messaggio. stem.0 contiene il numero dei
paragrafi, stem.1 ... stem.n contengono i paragrafi.
:eparml.
:p.
:p.:hp2.Valori restituiti&colon.:ehp2.
:p.:hp4.FSSetText:ehp4. restituisce "OK".
:p.
:p.:hp2.Note&colon.:ehp2.
:ul.
:li.Il formato dell array ä identico a quello di FleetMsg.Text.
:li.L'elemento 0 dell'array deve avere un valore numerico.
:li.Tutti gli elementi dell'array devono avere un valore, anche nel caso che
si tratti nella stringa zero.
:li.Bisogna sempre mettere il nome di base tra virgolette per evitare
che venga sostituito dal suo valore.
:li.Durante la lettura il testo nuovo non viene salvato sul disco. Durante
la scrittura il testo nuovo viene solamente salvato nel caso che venga salvato
il messaggio intero (Ctrl-S).
:eul.
:p.
:p.:hp2.Esempio&colon.:ehp2.
:xmp.
NewText.0 = 2
NewText.1 = 'Questo ä'
NewText.2 = 'un messaggio breve.'
RetVal = FSSetText('NewText')
:exmp.


:h4.FSLookupAddress
:p.:hp4.FSLookupAddress:ehp4. cerca un indirizzo FTN nella nodelist. Il risultato
viene depositato in una variabile di base.
:p.:hp2.Sintassi&colon.:ehp2.
:xmp.
result = FSLookupAddress(ftnaddress, stem)
:exmp.
:p.
:p.:hp2.Parametri&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.ftnaddress:ehp2.
:pd.Indirizzo FTN del nodo da cercare.
:pt.:hp2.stem:ehp2.
:pd.Nome di base della variabile alla quale si deve attribuire il risultato.
:p.La variabile ha i seguenti campi&colon.
:parml.
:pt.:hp4.Stem.Address:ehp4.
:pd.Indirizzo del nodo
:pt.:hp4.Stem.Name:ehp4.
:pd.Nome del SysOp
:pt.:hp4.Stem.System:ehp4.
:pd.Nome del sistema
:pt.:hp4.Stem.Phone:ehp4.
:pd.Numero di telefono
:pt.:hp4.Stem.Location:ehp4.
:pd.Locazione del sistema
:pt.:hp4.Stem.Password:ehp4.
:pd.La password di sessione. Questo campo resta vuoto se non ä stato definito
una password per il nodo in questione.
:pt.:hp4.Stem.Modem:ehp4.
:pd.Modem-Typ. Si tratta di un valore numerico.
:pt.:hp4.Stem.Baud:ehp4.
:pd.Baud-rate massima.
:pt.:hp4.Stem.UserCost:ehp4.
:pd.Costi per l'utente per scrivere un messaggio al nodo in questione.
:pt.:hp4.Stem.CallCost:ehp4.
:pd.Costi per una telefonata al nodo in questione.
:pt.:hp4.Stem.Flags:ehp4.
:pd.Node-flags, una combinazione di "ZC", "RC", "MO", "Hub", "Host" e "CM".
:eparml.
:eparml.
:p.
:p.:hp2.Valore restituito&colon.:ehp2.
:p.:hp4.FSLookupAddress:ehp4. restituisce uno dei seguenti valori&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Il nodo ä stato trovato. I dati del nodo vengono depositati nel secondo parametro.
:pt."NotFound"
:pd.Il nodo non ä stato trovato.
:pt."Error"
:pd.E' avvenuto un errore durante la ricerca.
:eparml.
:p.
:p.:hp2.Note&colon.:ehp2.
:ul.
:li.Se la stringa dell'indirizzo ä troppo lunga viene tagliata. Il campo
"Stem.Address" contiene l'indirizzo che risulta.
:li.Bisogna sempre mettere il nome di base tra virgolette, altrimenti viene
sostituito dal suo valore.
:eul.
:p.
:p.:hp2.Esempio&colon.:ehp2.
:xmp.
RetVal = FSLookupAddress('2&colon.2490/2520', 'NodeData')
say 'System-Name&colon.' NodeData.System
:exmp.


:h4.FSLookupName
:p.:hp4.FSLookupName:ehp4. cerca il nome di un SysOp nella nodelist. Il risultato
viene depositato in una variabile di base.
:p.:hp2.Sintassi&colon.:ehp2.
:xmp.
result = FSLookupName(name, stem)
:exmp.
:p.
:p.:hp2.Parametri&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.name:ehp2.
:pd.Nome cercato del SysOp.
:pt.:hp2.stem:ehp2.
:pd.Nome di base della variabile alla quale viene attribuito il risultato.
:p.La variabile ha i seguenti campi&colon.
:parml.
:pt.:hp4.Stem.0:ehp4.
:pd.Numero dei risultati trovati.
:pt.:hp4.Stem.1.Address:ehp4.
:pd.Indirizzo del nodo (I risultato)
:pt.:hp4.Stem.1.Name:ehp4.
:pd.Nome del SysOp (I. risultato)
:pt.:hp4.Stem.1.System:ehp4.
:pd.Nome del sistema (I. risultato)
:pt.:hp4.Stem.1.Phone:ehp4.
:pd.Numero di telefono (I. risultato)
:pt.:hp4.Stem.1.Location:ehp4.
:pd.Locazione del sistema (1. Eintrag)
:pt.:hp4.Stem.1.Password:ehp4.
:pd.Password della sessione (I. risultato). Questo campo resta vuoto nel caso che non
sia stata definita una password per il nodo in questione.
:pt.:hp4.Stem.1.Modem:ehp4.
:pd.Tipo di modem (I. risultato). Si tratta di un valore numerico.
:pt.:hp4.Stem.1.Baud:ehp4.
:pd.Baudrate massima (I. risultato)
:pt.:hp4.Stem.1.UserCost:ehp4.
:pd.Costi per l'utente per scrivere un messaggio al nodo (I. risultato)
:pt.:hp4.Stem.1.CallCost:ehp4.
:pd.Costi per una telefonata al nodo (I. risultato)
:pt.:hp4.Stem.1.Flags:ehp4.
:pd.Node-flags, una combinazione di "ZC", "RC", "MO", "Hub", "Host" e "CM" (I. risultato)
:eparml.
:eparml.
:p.Stem.2 etc. contengono gli ulteriori risultati.
:p.
:p.:hp2.Valori restituiti&colon.:ehp2.
:p.:hp4.FSLookupName:ehp4. restituisce uno dei seguenti valori&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Il nodo ä stato trovato. I dati del nodo vengono depositati nel secondo parametro.
:pt."NotFound"
:pd.Il nodo non ä stato trovato.
:pt."Error"
:pd.E' avvenuto un errore durante la ricerca.
:eparml.
:p.
:p.:hp2.Note&colon.:ehp2.
:ul.
:li.Se il nome ä troppo lungo viene tagliato. Il campo "Stem.x.Name"
contiene il nome che ne risulta.
:li.Bisogna sempre mettere il nome di base tra virgolette per evitare che
venga sostituito dal suo valore.
:li.E' anche possibile indicare solamente una parte del cognome.
:eul.
:p.
:p.:hp2.Esempio&colon.:ehp2.
:xmp.
RetVal = FSLookupName('Joe User', 'NodeData')
do i = 1 to NodeData.0
  say 'Address&colon.' NodeData.i.Address
end
:exmp.

:h4.FSSetEntryField
:p.:hp4.FSSetEntryField:ehp4. stabilisce il testo del campo di inserzione nella finestra di monitor.
Si tratta di un mezzo per fornire un nome prestabilito per l'utente.
:p.:hp2.Sintassi&colon.:ehp2.
:xmp.
result = FSSetEntryField(text)
:exmp.
:p.
:p.:hp2.Parametri&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.text:ehp2.
:pd.Testo nuovo per il campo di inserzione
:eparml.
:p.:hp2.Valori restituiti&colon.:ehp2.
:p.:hp4.FSSetEntryField:ehp4. restituisce uno dei seguenti valori&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.Il testo ä stato inserito.
:pt."NoMonitor"
:pd.Lo script viene avviato senza finestra di monitor per cui il testo da inserire
non ä stato messo.
:eparml.
:p.
:p.:hp2.Note&colon.:ehp2.
:ul.
:li.Il testo viene tagliato in modo da non superare la lunghezza di 500 caratteri.
:eul.
:p.
:p.:hp2.Esempio&colon.:ehp2.
:xmp.
RetVal = FSSetEntryField('C&colon.\')
if RetVal = 'OK' then
   do
   say 'Insirere path'
   parse pull mypath .
   end
:exmp.

.* **************************** Hooks ***************************************
.* @2@ **********************************************************************

:h2.Hooks
:p.
FleetStreet ä capace di avviare automaticamente certi scripts in certe situazioni.
:p.
Per assegnare uno script ad un certo hook bisogna eseguire i seguenti passi&colon.
:ol compact.
:li.Bisogna aprire la cartella Rexx-Scripts
:li.Bisogna selezionare "Impostazioni" nel menu contestuale della cartella scripts
:li.Bisogna passare alla pagina "Hooks" del taccuino
:li.Bisogna selezionare nella lista a tendina del hook preso in considerazione lo
script desiderato
:eol.
:p.
Per adesso sono disponibili i seguenti hooks&colon.
:ul compact.
:li.Fine programma
:li.Prima di salvare
:eul.


.* **************************** Pipe-Server ********************************
.* @1@ **********************************************************************
:h1.Il pipe-server di FleetStreet
:p.
Questo paragrafo descrive le funzioni del pipe-server di FleetStreet.
:p.
FleetStreet avvia automaticamente un thread che serve esclusivamente ad un
named pipe. Attraverso questo pipe ä possibile dirigere FleetStreet.
:p.
Il nome del pipe ä
:xmp.
\PIPE\FleetStreetDoor
:exmp.
:p.
Questo pipe e bidirezionale. Applicazioni che vogliono comunicare attraverso
questo pipe con FleetStreet devono usare un protocollo. Questo protocollo viene
descritto sulle pagine successive. Nella gran parte dei casi perï basterÖ rifarsi
a FleetCom. FleetCom ä un FleetStreet-Client speciale, il quale puï essere integrato
in modo molto semplice in programmi batch o simili.
:p.
Argomenti successivi&colon.
:ul compact.
:li.:link reftype=hd refid=proto.Il protocollo:elink.
:li.:link reftype=hd refid=commands.Comandi del pipe-server:elink.
:li.:link reftype=hd refid=fleetcom.FleetCom:elink.
:eul.

.* @2@ **********************************************************************
:h2 id=proto.Il protocollo
:p.
Una sessione con FleetStreet si struttura in tre passi&colon. Costruire una connesione,
lancio dei comandi e fine del collegamento.
:p.
Nei paragrafi successivi si descrivono questi passi., usando i caratteri ASCII
che seguono&colon.
:table cols='8 12 9'.
:row.
:c.Simbolo
:c.Esadecimale
:c.Decimale
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
:hp2.Stabilire un collegamento:ehp2.
:p.
Dopo l'apertura del pipe da parte del client si invia un :hp2.<ENQ>:ehp2. a
FleetStreet. Dopo questo FleetStreet manda il riconoscimento
:hp2."FleetStreet"<ETX>:ehp2. al
client. Adesso il client deve controllare se dietro il pipe si nasconde
veramente FleetStreet, cioä se il riconoscimento inviato corrisponde a "FleetStreet".
In questo caso il client invia un :hp2.<ACK>:ehp2. al server.
Nel caso di un errore il client manda un :hp2.<NAK>:ehp2.. Dopo questo
FleetStreet rimanda un :hp2.<EOT>:ehp2. e termina il collegamento.
:p.
Se FleetStreet riceve un :hp2.<ACK>:ehp2., invia il suo numero di versione
come testo, per esempio :hp2."0.88"<ETX>:ehp2.. Nel caso il client collabori
solamente con determinate versioni, puï ricontrollare il riconoscimento,
riinviando o :hp2.<ACK>:ehp2. o :hp2.<NAK>:ehp2. a FleetStreet. Se il riconoscimento
non viene controllato bisogna rimandare :hp2.<ACK>:ehp2..
:p.
Adesso la prima fase ä terminata, e FleetStreet aspetta per comandi del client.
:p.
La fase di stabilire un collegento schematicamente&colon.
:xmp.

 Client              FleetStreet

            ENQ
   ---------------------->

     "FleetStreet<ETX>"
   <----------------------

            ACK
   ---------------------->

        "0.90<ETX>"
   <----------------------

            ACK
   ---------------------->
:exmp.
:p.
Errore&colon.
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


:hp2.Esecuzione di comandi:ehp2.
:p.
Il client manda i comandi sotto forma di testo ASCII semplice a FleetStreet. Ogni
comando viene terminato con :hp2.<ETX>:ehp2..
:p.
Esempi&colon.
:xmp.
"SCAN *"<ETX>
:exmp.
:p.
I parametri sono separati da singoli spazi vuoti dal comando e tra loro.
:p.
FleetStreet controlla se il comando ä valido. Nel caso di un errore si rimanda
la sequenza
:xmp.
<NAK><Code><ETX>
:exmp.
:p.
rimandato. <Code> ä una delle tre lettere C, P oppure S.
Il significato delle lettere&colon.
:parml break=none.
:pt.C
:pd.il comando non ä stato riconosciuto
:pt.P
:pd.i parametri mancano o sono invalidi (per il comando in questione)
:pt.S
:pd.sono stati indicati pió parametri che si aspettavano.
:eparml.
:p.
Nel caso di successo si invia la sequenza
:xmp.
<ACK><ETX>
:exmp.
:p..
In questo caso FleetStreet inizia l'esecuzione dei comandi. Dopo aver terminato
l'esecuzione dei comandi si comunica il risultato. Se c'ä stato un errore durante
l'esecuzione, si invia la sequenza
:xmp.
<NAK>"Testo d'errore"<ETX>
:exmp.
:p..
"Testo d'errore" ä una descrizione dell'errore.
Nel caso che non ci sia nessun errore, si invia
:xmp.
<ACK>"Risultato"<ETX>
:exmp.
:p..
"Risultato" ä l'informazione richiesta, un resoconto di stato. Eventualmente
puï essere vuoto.
:p.
Dopo l'esecuzione di un comando ne puï essere inviato un altro a FleetStreet.
Questa procedura puï essere eseguita tanto spesso quanto si desidera, fino alla
terminazione del collegamento.
:p.
Schematicamente&colon.
:xmp.
 Client              FleetStreet

       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

      [Esecuzione]

     <ACK>"34 areas"<ETX>
   <----------------------
:exmp.
:p.
Comando sbagliato&colon.
:xmp.
       "ABC XYZ"<ETX>
   ---------------------->

        <NAK>"C"<ETX>
   <----------------------
:exmp.
:p.
Errore d'esecuzione&colon.
:xmp.
       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

      [Esecuzione]

    <NAK>"disk error"<ETX>
   <----------------------
:exmp.
:p.
:hp2.Fine del collegamento:ehp2.
:p.
Dopo l'esecuzione completa dell'ultimo comando del client, bisogna staccare il
collegamento. Per farlo il client invia un
:xmp.
EOT
:exmp.
:p.
al server. Questo rimanda un
:xmp.
EOT
:exmp.
:p.
e tutti e due interrompono il collegamento.
:p.
Schematicamente&colon.
:xmp.
 Client              FleetStreet

            EOT
   ---------------------->

            EOT
   <----------------------
:exmp.


.* @2@ **********************************************************************
:h2 id=commands.I comandi del pipe-server
:p.
Momentaneamente sono stati implementati i comandi successivi&colon.
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
Si effettua un rescan delle aree indicate. Come aree si possono indicare&colon.
:parml break=none tsize=20.
:pt.:hp2.*:ehp2.
:pd.tutte le aree
:pt.:hp2.areatag:ehp2.
:pd.esclusivamente l'area indicta, per esempio TUB
:pt.:hp2.@filename:ehp2.
:pd.tutte le aree che si trovano nel file indicato. Il formato del file ä identico
al file echotoss, cioä un area-tag
ogni riga
:eparml.

:h3.ETOSS
:p.
:hp2.ETOSS:ehp2.
:p.
Si scrive il file echotoss. Nel caso che non si indichi un parametro, il file
viene scritto secondo il FleetStreet-setup. Si puï indicare un parametro per
stabilire il nome del file da scrivere.


:h3 hide.LOCK
:p.
:hp2.LOCK:ehp2.
:p.
Si chiudono le aree indicate, cioä non vengono pió utilizzate da FleetStreet.
Se l'area in questione ä momentaneamente usata, il comando resta valido fino al
momento in cui l'area non si utilizzi pió.
:p.
Le aree si possono indicare allo stesso modo come con il comando SCAN.

:h3 hide.UNLCK
:p.
:hp2.UNLCK:ehp2.
:p.
Si liberano le aree chiuse. Le aree si indicano allo stesso modo come con il
comando LOCK.


.* @2@ **********************************************************************
:h2 id=fleetcom.FleetCom
:p.
Con FleetStreet si distribuisce un arnese particolare di nome FleetCom.
FleetCom ä un Client per il FleetStreet pipe-server. Si avvia con il comando&colon.
:xmp.
FLEETCOM comando [parametro]
:exmp.
:p.
FleetCom stabilisce un collegamento con il pipe-server, invia il comando ed i
parametri e controlla i dati restituiti. Nella gran parte dei casi FleetCom ä pió
che sufficiente per sfruttare il pipe-server di FleetStreet. FleetCom invia un
comando ogni session.
:p.
I return-codes di FleetCom sono&colon.
:parml break=none tsize=4.
:pt.0
:pd.tutto OK, nessun errore
:pt.1
:pd.non ä stato possibile aprire il pipe. Questo errore avviene se FleetStreet
non ä attivato quando si lancia FleetCom, per cui nella gran parte dei casi puï
essere ignorato.
:pt.2
:pd.non ä stato indicato nessun comando
:pt.3
:pd.FleetStreet non riconosce il comando
:pt.4
:pd.Errore d'esecuzione del comando, oppure errore di sistema.
:eparml.
:p.
Come default FleetCom usa il pipe \PIPE\FleetStreetDoor. E' comunque possibile
usare un nome diverso. Questo viene estratto dalla variabile d'ambiente FLEETPIPE. Esempio&colon.
:xmp.
SET FLEETPIPE=\PIPE\AnotherPipe
:exmp.
:p.
Se FleetCom deve riutilizzare il nome di default, bisogna cancellare la variabile con
:xmp.
SET FLEETPIPE=
:exmp.
:p..
:p.
:hp2.Esempi per l'utilizzo di FleetCom&colon.:ehp2.
:p.
Rescan di aree nelle quali il tosser utilizzato ha tossato messaggi nuovi&colon.
:xmp.
SquishP IN -f echotoss.log
FleetCom scan @echotoss.log
:exmp.
:p.
Scrittura dell'echotoss.log, per dare al tosser la possibilitÖ di esportare messaggi
dalle aree&colon.
:xmp.
FleetCom etoss pack.log
SquishP OUT SQUASH -f pack.log
:exmp.
:p.

.* @1@ **********************************************************************
:h1.Pensieri sui formati della base messaggi
:p.
Ognuno dei formati della base messaggi offre i suoi vantaggi e problemi.
Questo paragrafo descrive come FleetStreet tratta i diversi formati e offre
una guida per una decisione ragionata.

.* @2@ **********************************************************************
:h2.*.MSG
:p.
FleetStreet utilizza la MSGAPI32.DLL di Squish per accedere alle aree *.MSG.
Questa API legge al massimo solamente 512 bytes di kludge-lines di un messaggio.
Nel caso che un messaggio contenga pió kludges (facilmente possibile con messaggi
che derivano dall'internet), tutti i bytes che superino 512 bytes vengono tagliati.
Il resto dei kludges viene riportato nel testo del messaggio.
:p.
In ogni caso FleetStreet estrae le ulteriori kludge-lines dal messaggio.
Una kludge-line tagliata non ä pió riconoscibile come tale. Perciï puï
capitare che all'inizio del testo di messaggio appaia una mezza kludge-line.
:p.
Il formato *.MSG prevede solamente gli attributi standard di Fido da salvare. Tutti
gli altri attributi vengono salvati da FleetStreet in una FLAGS-kludge. L'ultima,
perï, non viene riconosciuta da Squish (il tosser). Attributi come "Diretto" oppure
"Archivia dopo l'invio" restano senza effetto con Squish..
:p.
FleetStreet utilizza come attributo "Letto" il contatore lettura dei files *.MSG.
Questo, perï, viene messo a zero da Squish durante la compattazione del messaggio,
per cui il messaggio riappare come "non letto".

.* @2@ **********************************************************************
:h2.Squish
:p.
La MSGAPI32.DLL (utilizzata durante l'accesso ad aree Squish) possiede un errore
gravissimo nel suo design.&colon. Durante l'apertura di un'area l'indice area (*.SQI)
viene caricato nella memoria. Modificando l'area, l'indice sul disco fisso non viene
aggiornato per rispecchiare lo stato attuale. La modifica viene effettuata solamente
nella memoria di lavoro e viene riscritta sul disco fisso solamente dopo la chiusura
dell'area in questione. Nel caso che due programmi lavorino sulla stessa area, si
sovrascrivono le modifiche vicendevolmente nell'indice, causando un indice area diffettoso
o distruggendo addirittura l'area in question.
:p.
Come in aree *.MSG le aree Squish offrono solamente gli attributi standard. Gli altri
attributi vengono salvati da FleetStreet in una FLAGS-kludge. Squish (il tosser) :hp5.non:ehp5.
considera questa kludge-line.
:p.
Come attributo "Letto" FleetStreet utilizza il bit pió alto nel campo attributi dei
messaggi.
:p.
L'attributo "salva" non viene preso in considerazione da :hp2.SqPack:ehp2.. Conviene
utilizzare :hp2.FESQ:ehp2. per compattare aree Squish.

.* @2@ **********************************************************************
:h2.JAM
:p.
Nel file *.JHR di un'area JAM viene salvato il numero dei messaggi attivi dell'area
in questione. Questo campo non viene considerato in modo corretto da diversi programmi
(IMail 1.75 mette il campo a 0 durante la compattazione;
GoldEd 2.50.Beta6 crea errori in questo campo durante la cancellazione di messaggi).
Perciï il contenuto di questo campo non ä utilizzabile in modo affidabile, per cui
FleetStreet si fida del indice area.
:p.
L'indice di aree JAM ha un design non buono. Non si puï riconoscere dall'indice se un
messaggio ä attivo o se ä stato cancellato.
Ci sono due possibilitÖ per trattare questo problema&colon.
:ol.
:li.Viene letto in pió la testa del messaggio, per capire se il messaggio
ä attivo o se ä stato cancellato. Questo metodo ä lento e rende assurdo l'uso
dell'indice. FleetStreet non utilizza pió questo metodo.
:li.Vengon visualizzati tutti i messaggi, anche se alcuni di loro vengono
caratterizzati come "cancellati".
:eol.
:p.
Per effettuare la cancellazione di messaggi, FleetStreet sovrascrive l'inserimento
indice del messaggio completamente. Questo approccio non corrisponde al 100 %
alle specificazioni JAM, ma ä l'unico metodo che faccia senso. In nessun caso
messaggi cancellati da FleetStreet riappaiono un'altra volta.
:p.
Alcune utilitÖ per la base messaggi cancellano messaggi considerando solament lo
header come cancellato, e non modificano l'indice. In questo caso bisogna compattare
successivamente l'area in questione, per liberarsi definitivamente dai messaggi
cancellati.

.* @1@ **********************************************************************
:h1.Considerazioni sui tosser
:p.
FleetStreet supporta direttamente molti tosser diversi. In questo paragrafo forniamo
alcuni suggerimenti per una collaborazione effettiva con i diversi tosser.

.* @2@ **********************************************************************
:h2.Squish
:p.
La collaborazione con Squish ä relativamente priva di problemi. Ne seguono lo
stesso alcuni suggerimenti&colon.
:ul.
:li.Squish non riconosce rispettivamente non tratta la FLAGS-kludge, cioä solamente
gli attributi messaggio standard hanno un effetto.
:li.Compattando messaggi da aree *.MSG, il contatore di lettura viene messo a 0,
cioä i messaggi riappaiono come "non letti.
:eul.

.* @2@ **********************************************************************
:h2.Fastecho
:p.
:ul.
:li.Fastecho non supporta le features di broadcasting di Squish.
:li.Un altro particolare ä il trattamento di pió aree netmail. Al contrario
di Squish, Fastecho non ä capace di tossare direttamente nelle aree netmail,
oppure di compattarne. Durante la compattazione occorre che le netmails vengano
esportati dalle aree netmail secondari prima di compattarli successivamente.
Per l'esporazione viene utilizzato lo stesso comando come per l'esportazione
di echomail. Ma FleetStreet considera tutte le aree netmail ugualmente,
cioä esse non vengono inserite nel file ECHOTOSS.LOG. Ne risulta che durante
l'esportazione le netmails nelle aree netmail secondarie non vengono considerate.
:p.
La soluzione consiste nel suddividere l'esportazione in due passi.&colon.
Nel primo passo, echomail viene esportata con l'aiuto del file ECHOTOSS.LOG.
Nel secondo passo la netmail viene esportata dalle aree netmail secondarie con
l'aiuto di un file dummy ECHOTOSS.LOG. Questo file dummy ECHOTOSS.LOG elenca
semplicemente le aree netmail secondarie. Nell'ultimo passo la mail esportata
viene compattata.
:p.
Esempio&colon.
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
La versione 1.75 di IMail purtroppo possiede un errore&colon. Nelle aree JAM
la :hp2.PATH:ehp2.-kludge e le righe :hp2.SEEN-BY:ehp2. vengono salvati nel
testo del messaggio e non nella testa del messaggio (come prescritto).
Perciï queste righe vengono visualizzate da FleetStreet al di sotto del
testo del messaggio.


.* @1@ **********************************************************************
:h1.Appendice
:p.

.* ************************** Kludge-Lines ***********************************
.* @2@ **********************************************************************
:h2 id=kludges.Kludge-Lines
:p.
Le Kludge-Lines successive vengono create, rispettivamente riconosciute, da FleetStreet&colon.
:parml compact tsize=3 break=all.
:pt.:hp2.FMPT:ehp2.
:pt.:hp2.TOPT:ehp2.
:pt.:hp2.INTL:ehp2.
:pd.Queste Kludges si creano secondo :hp4.FTS-0001 Rev. 15:ehp4., nel caso che
si tratti di una NetMail.
:p.Squish ricrea le Kludges quando esporta il messaggi. Sostenere queste Kludges
non sarebbe necessario.
:p.Durante la lettura queste Kludges vengono ignorate.
:p.

:pt.:hp2.MSGID:ehp2.
:pd.Un messaggio da scrivere si crea secondo :hp4.FTS-0009 Rev. 1:ehp4.
La componente indirizzo ä 4D.
:p.Durante la lettura di un'Echomail-Area si utilizza :hp2.MSGID:ehp2. per
stabilire l'indirizzo del mittente. Nel caso che :hp2.MSGID:ehp2. non contenga
un indirizo FTN oppure nel caso che manchi, si sfrutta la Originline a tale scopo.
:p.

:pt.:hp2.REPLY:ehp2.
:pd.Scrivendo una risposta la :hp2.MSGID:ehp2. dell'originale si scrive come
:hp2.REPLY:ehp2..
:p.

:pt.:hp2.PID:ehp2.
:pd.Nel caso che l'uso di :hp2.PID:ehp2. sia attivato nel Setup, si scrive la
:hp2.PID:ehp2.-Kludge aggiungendo solamente una breve Tearline. Si inserisce
una Tearline lunga nel caso che :hp2.PID:ehp2. sia disattivato.
:p.FleetStreet segue i suggerimenti di :hp4.FSC-0046 Rev. 2:ehp4..
:p.

:pt.:hp2.REPLYTO:ehp2.
:pd.Rispondendo ad una Netmail con :hp2.REPLYTO:ehp2.-Kludge, se ne usano
l'indirizzo e il nome dell'utente come destinatario.
:p.La Kludge si ignora negli Echos. FleetStreet segue i suggerimenti di
:hp4.FSC-0035 Rev. 1.:ehp4.
:p.

:pt.:hp2.REPLYADDR:ehp2.
:pd.Rispondendo ad una NetMail con :hp2.REPLYADDR:ehp2., l'indirizzo di essa
viene inserito in una linea "To&colon." all'inizio della risposta.
:p.La Kludge si ignora negli Echos. FleetStreet segue i suggerimenti di
:hp4.FSC-0035 Rev. 1.:ehp4.
:p.

:pt.:hp2.SPLIT:ehp2.
:pd.Salvando un messaggio pió lungo di 12 KB, questo si suddivide secondo
:hp4.FSC-0047 Rev. 1:ehp4. in pió parti. Bisogna considerare perï le seguenti
differenze&colon.
:ul.
:li.PoichÇ il messaggio non ä mai stato salvato interamente in una
messagebase, il numero messaggio indicato nella :hp2.SPLIT:ehp2.-Kludge
dipende dall'implementazione (per adesso&colon. il numero messaggio del primo
messaggio creato).
:li.:hp4.FSC-0047:ehp4. consiglia di togliere nelle parti 2...n la :hp2.MSGID:ehp2.
evitando che un Dupe-Checker scarti queste parti. FleetStreet invece crea per tutte le
parti una :hp2.MSGID:ehp2. propria, arrivando allo stesso effetto.
:li.PoichÇ il numero delle parti nella :hp2.SPLIT:ehp2.-Kludge consiste
di due cifre, possono essere create al massimo 99 parti, per cui la lunghezza di
un messaggio ä limitata a 1188 KB.
:li.Modificando un messagggio, il messaggio toccato non puï superare
i 15 KB. Un'ulteriore suddivisione risulterebbe in un conflitto con il numero delle
altre parti, impedendo una ricomposizione del messaggio da parte del destinatario.
:li.La numerazione delle parti del messaggio avviene alla fine della linea
dei soggetti, al contrario della proposta di metterla all'inizio del messaggio.
Perciï le parti del messaggio si visualizzano nell'ordine corretto nella lista
dei soggetti.
:eul.
:p.
:pt.:hp2.APPEND:ehp2.
:pt.:hp2.REALADDRESS:ehp2.
:pd.Queste kludges si mantengono con la risposta.
:p.
:pt.:hp2.CHARSET/CHRS:ehp2.
:pd.FleetStreet sostiene queste queste kludges fino al livello 2 secondo :hp4.FSC-0054 Rev. 4:ehp4..
Durante la scrittura di messaggi FleetStreet crea sempre :hp2.IBMPC 2:ehp2..
:p.
:pt.:hp2.ACUPDATE:ehp2.
:pd.Questa kludge-line viene usata da 1.10 per modificare o cancellare messaggi
su altri sistemi. Bisogna leggere la documentazione di Squish 1.10 per avere
ulteriori informazioni.
:p.
:pt.:hp2.AREA:ehp2.
:pd.Nel caso che questa kludge-line venga trovata in un messaggio e l'area ä
riconosciuta da FleetStreet, una risposta a questo messaggio viene automaticamente
collocata nell'area indicata.
:p.
:pt.:hp2.FLAGS:ehp2.
:pd.La :hp2.FLAGS:ehp2.-Kludge contiene gli attributi messaggio non supportati direttamente
dalla base di messaggio. Il formato della Kludge in questione viene definito in
:hp4.FSC-0053:ehp4..
:p.
:pt.:hp2.FWDFROM, FWDTO, FWDSUBJ, FWDORIG, FWDDEST, FWDAREA, FWDMSGID:ehp2.
:pd.Queste kludges vengono create da FleetStreet quando si forwarda un messaggio.
Esse contengono informazioni dello header del messaggio originale. Rispondendo
ad un tale messaggio, FleetStreet riutilizza i dati originali.
:eparml.

.* @2@ **********************************************************************
:h2 id=cmdlin.Parametri del programma
:p.
FleetStreet riconosce i seguenti parametri dalla linea di comando&colon.
:parml.
:pt.:hp2.-C<Path>:ehp2.
:pd.I files INI non vengono scritti e letti nella directory attuale, ma piuttosto
nella directory indicata.
:p.:hp2.Esempio&colon.:ehp2.
:p.FLTSTRT.EXE -Cd&colon.\myinis
:eparml.

.* @2@ **********************************************************************
:h2.Codici di ritorno
:p.
FLTSTRT.EXE crea i seguenti codici di ritorno&colon.
:parml break=none.
:pt.:hp2.0:ehp2.
:pd.Non ä stato scritto nessun messaggio nuovo
:pt.:hp2.1:ehp2.
:pd.E' stata scritta netmail nuova
:pt.:hp2.2:ehp2.
:pd.E' stata scritta nuova echomail
:pt.:hp2.4:ehp2.
:pd.Sono stati scritti nuovi messaggi  locali.
:pt.:hp2.255:ehp2.
:pd.Errore fatale
:eparml.
:p.
Una combinazione di 1, 2 e 4 significa che sono stati scritti messaggi in aree del tipo
indicato. 5 per esempio significa che sono stati scritti messaggi nuovi in aree di netmail
e aree locali.


.* @2@ **********************************************************************
:h2.Bug-Reports
:p.
Senza dubbio FleetStreet non ä senza errori. Perciï vi chiedo di comunicarmi tutti
gli errori e cercherï, secondo le mie capacitÖ, di eliminare tutti gli errori il pió
presto possibile.
:p.
Eventualmente non sono capace di riprodurre (immediatamente) qualche errore. Le seguenti
domande possono diventare importanti per l'eliminazione di un errore&colon.
:ul.
:li.E' possibile riprodurre l'errore?
:li.L'errore si verifica immediatmante con il primo tentativo, oppure bisogna fare alcune
prove?
:li.Quale funzione era stata avviata quando si ä verificato l'errore?
:li.La funzione in questione ä stata attivata con la tastiera, il menu oppure la toolbar?
C'ä qualche differenza tra i diversi modi d'attivazione?
:li.Ci risulta un messaggio d'errore? Quale?
:li.FleetStreet ä stato configurato in modo corretto?
:li.Quali sono gli effetti precisi dell'errore in questione?
:eul.
:p.
Se FleetStreet si pianta per un errore, nella directory attuale
viene creato il file :hp2.FLTSTRT.DMP:ehp2.. Con l'aiuto di questo file,
l'errore puï spesso essere rintracciato abbastanza facilmente.


.* @2@ **********************************************************************
:h2.Programmi utilizzati
:p.
FleetStreet ä stato creato e testato con i seguenti programmi&colon.
:ul.
:li.Compiler&colon. IBM VisualAge C++ 3.0 (C-Modus)
:li.Debugger&colon. IBM C/C++ Debugger 3.0 (IPMD)
:li.Ambiente di sviluppo&colon. IBM Workframe 3.0
:li.Linker&colon. ILink
:li.Editor&colon. Enhanced Editor, Tiny Editor, LPEX
:li.Tools&colon. IBM OS/2 Toolkit 3.0; GNU Grep; PMTree; ExeMap;
Hexdump; PMSpy; PM Camera
:eul.

.* @2@ **********************************************************************
:h2 id=support.Supporto
:p.Michael Hohner ä raggiungibile sotto i seguenti indirizzi EMail&colon.
:parml compact break=none tsize=16.
:pt.Fidonet&colon.
:pd.Michael Hohner 2&colon.2490/1050.17 (new!)
:pt.OS2Net&colon.
:pd.Michael Hohner 81&colon.499/617.17 (new!)
:pt.Internet&colon.
:pd.miho@n-online.de (new!)
:eparml.
:p.
:hp2.Fido-Echomail&colon.:ehp2.
:p.
Ci sono due echos Fido presso 2&colon.2490/1050, FLEETBETA e FLEETSTREET. FLEETBETA
ä l'echo in lingua tedesca, FLEETSTREET quello internazionale (in lingua inglese).
Bisogna scrivere una netmail a Robert Gloeckner 2&colon.2490/1050 per sapere
quali siano i nodi che mettono a disposizione gli echos di cui sopra. Gli echos possono
essere routati liberamente, basta informarci sui nodi attaccati.
:p.
Domande che riguardano FleetStreet possono essere poste nell'echo Fido OS2_APP.ITA.

:euserdoc.

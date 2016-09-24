:userdoc.
:docprof toc=1234.
:title.FleetStreet User's Guide

.* ************************** EinfÅhrung ********************************
.* @1@ **********************************************************************
:h1.Introduction
:p.
:lines align=center.
Welcome to

:font facename='Tms Rmn' size=24x18.:color fc=red.
FleetStreet 1.27.1
:font facename=default size=0x0.:color fc=default.
:p.
FleetStreet is a FTS compatible message editor for OS/2 2.x PM.
FleetStreet can handle *.MSG, Squish and JAM type message areas.
:elines.
:p.
:p.
This documentation describes how to configure FleetStreet and informs you
about basic approaches. Detailed information for the various dialogs of FleetStreet
can be found in the online help of the program. This documentation and
the online help add to each other.
:p.
This documentation assumes that you have a basic knowledge about Fidonet or
Fidonet compatible networks.
:p.
This is an Open Source program.


.* ************************** Features   ********************************
.* @2@ **********************************************************************
:h2.Features
:p.
Features of FleetStreet&colon.
:ul compact.
:li.OS/2 2.x/3.x PM programm
:li.100% 32 bit code
:li.Optimized for Pentium processors
:li.Multi-threaded
:li.WPS-alike user interface
:li.Usable via
:ul compact.
:li.Menus
:li.Accelerator keys
:li.Toolbar (position selectable, 2 sizes, configurable)
:eul.
:li.User support by extensive online help and help messages in the
status line
:li.Standard functionality, like
:ul compact.
:li.reading messages
:li.writing messages
:li.deleting messages
:li.modifying messages
:li.copying messages
:li.moving messages
:li.forwarding messages
:li.replying to a message
:li.exporting messages
:li.importing ASCII-Text into message
:eul.
:li.Support for areas in the formats
:ul compact.
:li.*.MSG
:li.Squish
:li.JAM
:eul.
:li.Support for configuration files of
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
:li.Printing messages via the PM printer driver
:li.Creating an ECHOTOSS.LOG file
:li.Coloured display of a message
:li.Crossposting a message in more than one area
:li.Creating file requests from a message or from file lists
:li.Sending a message to more than one recipient via Carbon Copy Lists
:li.Quick Carbon Copy List
:li.List of nicknames
:li.Support for Squish 1.1x's broadcast functions
:li.Support for Version-7 nodelists
:li.Nodelist browser
:li.Programmable function keys
:li.Remapping of drive letters in a networked environment
:li.Freely configurable message templates
:li.Comfortable find function, can search more than one area
:li.Personal mail scan
:li.Threadlist for threaded message reading
:li.Message list
:li.Numerous area settings
:li.Conversion of accented characters when writing a message
:li.Support for CHRS kludge lines
:li.Manual tagging of messages
:li.Remote control via named pipe
:li.Macro programmable with Rexx scripts
:li.Heavy use of Drag-and Drop
:li.Program and user's guide in
:ul compact.
:li.German
:li.English
:li.Italian
:li.Swedish
:eul.
:eul.
:p.
&dot.&dot.&dot. and this is just a rough overview &colon.-)

.* @2@ **********************************************************************
:h2.Requirements
:p.
FleetStreet requires the following hardware and software to run&colon.
:p.
:hp2.Hardware&colon.:ehp2.
:ul compact.
:li.PC, running under OS/2 (386DX, 6 MB)
:li.about 1.5 MB disk space plus message base
:li.VGA graphics card
:eul.
:p.
:hp2.Software&colon.:ehp2.
:ul compact.
:li.OS/2 2.x/3.x (2.0 not testet, but should work)
:eul.
:p.
:hp2.Tested software enviromnents&colon.:ehp2.
:ul compact.
:li.OS/2 2.1 (various languages)
:li.OS/2 2.11 (2.1 with Service Pack)
:li.OS/2 2.99 (WARP II)
:li.OS/2 Warp 3.0
:li.OS/2 Warp Connect (Peer To Peer)
:li.OS/2 Warp 4.0
:li.ZipStream 1.03 (message base in compressed directory)
:li.LAN Server 4.0 Entry
:eul.

.***************************************************************************
.* Design Goals                                                            *
.***************************************************************************

.* @2@ **********************************************************************
:h2 id=design.Design goals
:p.We had several design issues in mind while developing FleetStreet. Maybe
you want to read about some of them.
:parml.
:pt.:hp2.Why a PM editor?:ehp2.
:pd.The Presentation Manager (tm) interface may be slower than the VIO
interfae (but only when you use VIO in full screen). On the other hand, PM provides
many advantages to the user. The clipboard is now fully usable (VIO clipboard
is a kludge!), the user can select fonts and colors easily and isn't bound
to a 80x25 (or similar) screen format. PM also gives us the ability
to implement all the dialog boxes that make FleetStreet so easy to use.
:pt.:hp2.Integrated setup:ehp2.
:pd.With other editors you may have struggled with a lot of ASCII configuration
files with as many different keywords. If you are used to the various
notebook settings dialogs of the workplace shell, you will not have difficulties
setting up FleetStreet. You can change settings while FleetStreet is running
and don't have to restart your editor to make the changes take effect.
:p.
If you are in doubt about one setting, just press F1 or the HELP buttons around.
There's no need to read a lengthy ASCII documentation.
:pt.:hp2.Support for Squish (tm):ehp2.
:pd.FleetStreet supports Squish in two ways: At first, it uses the squish API
implemented in MSGAPI32.DLL. Secondly, FleetStreet can read your Squish.Cfg
configuration file.
:i2 refid=squish.MSGAPI.DLL
:pt.:hp2.Ease of use:ehp2.
:pd.Most PM applications are easy to use, but we've added to that. Generally
speaking, if you've used the WPS you will know how to use FleetStreet.
We have settings notebooks, popup menus, containers etc.
:pt.:hp2.All the features you ever need:ehp2.
:pd.We've implemented a lot of features that we found to be useful in other
editors, and some that we didn't find elsewhere.
:pt.:hp2.Power:ehp2.
:pd.FleetStreet makes use of multiple threads, CUA'91 and 32 Bit processing.
:eparml.

.* @2@ **********************************************************************
:h2.Copyrights etc.
:p.
Squish and MsgAPI are trademarks of Lanius Corporation.
:p.
OS/2 and Workplace Shell are trademarks of IBM.
:p.
JAM(mbp) - Copyright 1993 Joaquim Homrighausen, Andrew Milner, Mats Birch, Mats Wallin.
ALL RIGHTS RESERVED.


.* @2@ **********************************************************************
:h2.Acknowledgements
:p.
The following people contributed to the initial version and the development
of FleetStreet&colon.
:sl.
:li.:hp4.Harry Herrmannsdîrfer:ehp4. - Echo management, Alpha/Beta testing, coding
:li.:hp4.Harald Kamm:ehp4. - Italian translation, registration site Italy
:li.:hp4.Jason Meaden:ehp4. - Former registration site Australia
:li.:hp4.Helmut Renner:ehp4. - Registration site and echo link Austria
:li.:hp4.Marty Duplissey:ehp4. - Former registration site USA
:li.:hp4.Siegfried Hentschel, Jens Holm, Richard Douglas,
Jose M. Guglieri:ehp4. - Registration sites
:li.:hp4.Rasmus Foged Hansen:ehp4. - Former registration site
:li.:hp4.Peter Karlsson:ehp4. - Swedish translation
:li.:hp4.All participants of the closed beta test, especially Michael Siebke,
Joachim Loehrl-Thiel, Hajo Kirchhoff, Robert Glîckner:ehp4. and others
:li.:hp4.Thomas Jachmann:ehp4. - Beta test and some Rexx scripts
:li.:hp4.Torsten Grimme:ehp4. - Test of IMail support
:li.:hp4.Dirk Brenken:ehp4. - Test of Fastecho support
:li.:hp4.Carsten Ellwart:ehp4. - Bug finder
:li.:hp4.and last but not least, all registered users:ehp4., Shareware can't
survive without them.
:esl.

.* ************************* Installation **********************************

.* @1@ **********************************************************************
:h1.Basic configuration
:p.
When you start FleetStreet for the first time, a message about missing INI
files is displayed. This is just normal at this time, so don't be afraid.
Just press the :hp2.OK:ehp2. button. FleetStreet then opens the setup
notebook automatically to let you start with the basic configuration
of FleetStreet.
:p.
The configuration described in this chapter is sufficient to install
FleetStreet in a normal Node-System or Point-System and to allow the
first steps with FleetStreet. You can change the configuration at any time
later and adjust it to your needs.
:p.
If you don't know what to do next in a certain situation, just press the
:hp2.Help:ehp2. button that is on every notebook page.

.* @2@ **********************************************************************
:h2.Names
:p.
When you write new messages FleetStreet automatically inserts your own name
as name of the sender.
:p.
On the first notebook page you can enter all names that you intend to use
with FleetStreet. The first name in the list is your default name. Whenever
a user name is needed, it is inserted first. You can make any of the names
in the list your default name by selecting the name and pressing the
:hp2.Default:ehp2. button.

.* @2@ **********************************************************************
:h2.Addresses
:p.
On the second page you can enter all the network addresses that you intend to
use with FleetStreet. Again, the first address in the list is your default
address.

.* @2@ **********************************************************************
:h2.Tosser configuration
:p.
If you already installed a tosser that's supported by FleetStreet, you
can easily let its configuration file be read by FleetStreet. FleetStreet
extracts all addresses, areas and directories from it so that you don't have to
redefine all message areas in FleetStreet.
:p.
Every time at startup, FleetStreet will read the file so that the areas in
FleetStreet will always match those defined for your tosser.
:p.
The following tossers are supported&colon.
:table cols='10 12 20'.
:row.
:c.Tosser
:c.Versions
:c.File to select
:row.
:row.
:c.Squish
:c.1.01, 1.10, 1.11
:c.SQUISH.CFG (or name of equivalent file)
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
First check :hp2."read":ehp2. on the fourth page of the setup notebook.
Then press the :hp2."Locate":ehp2. button. Locate the configuration file,
select it and press :hp2.OK:ehp2..
:p.
FleetStreet has now read the file. This also is the final step of the
basic configuration. Close the setup notebook. FleetStreet should now
open the first area and display the first message in the main window.

.* @1@ **********************************************************************
:h1.Using FleetStreet
:p.

.* @2@ **********************************************************************
:h2 id=mainwin.Main window
:p.
When reading messages the following components of the main window are
displayed&colon.
:p.
:hp2.Title bar:ehp2.
:artwork name='titlebar.bmp' align=center.
:p.
It displays&colon.
:ul compact.
:li.Program name and version
:li.Current area
:eul.
:p.
:hp2.Message header:ehp2.
:artwork name='header.bmp' align=center.
:p.
It displays&colon.
:ul compact.
:li.Name and address of the sender
:li.Name and address of the recipient (address only displayed in Netmail areas)
:li.Subject line
:li.Attributes of the message
:li.Date and time when the message was written
:li.Date and time when the message was received
:eul.
:p.
:hp2.Toolbar:ehp2.
:p.
The meaning of the buttons is&colon.
:parml compact break=none tsize=6.
:pt.:artwork runin name='home.bmp'.
:pd.Go back to the old lastread pointer of the area
:pt.:artwork runin name='nextarea.bmp'.
:pd.Go to the next area with unread mail
:pt.:artwork runin name='prevmsg.bmp'.
:pd.Go to the previous message
:pt.:artwork runin name='nextmsg.bmp'.
:pd.Go to the next message
:pt.:artwork runin name='prevreply.bmp'.
:pd.Go to the original message of this reply
:pt.:artwork runin name='nextreply.bmp'.
:pd.Go to the reply of this message
:pt.:artwork runin name='firstmsg.bmp'.
:pd.Go to the first message
:pt.:artwork runin name='lastmsg.bmp'.
:pd.Go to the last message
:pt.:artwork runin name='ok.bmp'.
:pd.Save message
:pt.:artwork runin name='cancel.bmp'.
:pd.Discard messages or changes in the message
:pt.:artwork runin name='newmsg.bmp'.
:pd.Write a new message
:pt.:artwork runin name='edit2.bmp'.
:pd.Change the current message
:pt.:artwork runin name='importfile.bmp'.
:pd.Insert text file into current message
:pt.:artwork runin name='exportfile.bmp'.
:pd.Export message as text file
:pt.:artwork runin name='reply2.bmp'.
:pd.Reply to current message
:pt.:artwork runin name='printmsg.bmp'.
:pd.Print current message
:pt.:artwork runin name='find.bmp'.
:pd.Search for a message
:pt.:artwork runin name='msglist.bmp'.
:pd.Open message list
:pt.:artwork runin name='msgtree.bmp'.
:pd.Open thread list
:pt.:artwork runin name='area.bmp'.
:pd.Open area list
:pt.:artwork runin name='showkludges.bmp'.
:pd.Open window with kludge lines
:pt.:artwork runin name='delmsg.bmp'.
:pd.Delete current message
:pt.:artwork runin name='bookm.bmp'.
:pd.Switch to the bookmarks window
:pt.:artwork runin name='help.bmp'.
:pd.General help for FleetStreet
:pt.:artwork runin name='cut.bmp'.
:pd.Copy selection to clipboard and delete selection
:pt.:artwork runin name='copy.bmp'.
:pd.Copy selection to clipboard
:pt.:artwork runin name='paste.bmp'.
:pd.Insert contents of clipboard at current position
:pt.:artwork runin name='copymsg.bmp'.
:pd.Copy current message to a different area
:pt.:artwork runin name='movemsg.bmp'.
:pd.Move current message to a different area
:pt.:artwork runin name='fwdmsg.bmp'.
:pd.Forward current message
:pt.:artwork runin name='shell.bmp'.
:pd.Start a command line shell
:pt.:artwork runin name='script.bmp'.
:pd.Rexx scripts
:pt.:artwork runin name='browser.bmp'.
:pd.Display contents of nodelist
:pt.:artwork runin name='request.bmp'.
:pd.Request files in the current message
:pt.:artwork runin name='catchup.bmp'.
:pd.Mark all messages as "read"
:eparml.
:p.
:hp2.Status line:ehp2.
:artwork name='statline.bmp' align=center.
:p.
It displays (left to right)&colon.
:ul compact.
:li.Help texts, messages
:li.Marker symbol (if message is marked)
:li.Cursor position (only when writing)
:li.Insert/overwrite mode (only when writing)
:li.Number of current message, number of messages in the area
:li.Current address
:eul.

.* @2@ **********************************************************************
.* :h2.Reading
.* :p.

.* @2@ **********************************************************************
:h2 id=writemsg.Writing messages
:p.There are two different ways to add new messages to your message base&colon.
:parml.
:pt.:hp2.Writing new messages:ehp2.
:pd.If you want to enter a new message, click on the :artwork name='help01.bmp' runin.
button or press INS.
The message window will be cleared and your own name and address will be inserted
into the appropriate fields of the message header. You can now enter the name and
address of the receipient (see :link reftype=hd refid=enteraddr."Entering Fidonet
addresses":elink.) and write your message in the message window.
:p.After you've entered your message, click on the :artwork name='help04.bmp' runin.
button or press CTRL-S. The
message will then be saved to your message base. Click on the :artwork name='help05.bmp' runin.
button or
press ESC if you don't want to save your message.
:pt.:hp2.Replying to a message:ehp2.
:pd.To reply to a message you've received, click on the :artwork name='help03.bmp' runin.
button or press CTRL-R.
You may either reply to the sender of the message (the usual case), or to the
recepient. You may also place your reply in a different message area (usually a
Netmail area.
:p.After editing your reply message, click on the :artwork name='help04.bmp' runin.
button or press CTRL-S to save it.
Click on the :artwork name='help05.bmp' runin. button or press ESC to discard your reply.
:eparml.

.* @2@ **********************************************************************
:h2 id=changemsg.Changing Messages
:p.Messages can be changed after you've saved them. Press the
:artwork name='help06.bmp' runin. button to switch to edit mode. Now you can
re-edit your message. You can change the message text and the message header.
:p.To toggle the attributes of the message, press the :hp2.Change:ehp2. button.
A dialog window appears, where you can set or clear the message attributes.
:p.If you edit a message that was already sent or that is not not a local message,
you will get a warning.
:p.Press the :artwork name='help04.bmp' runin. button to save your changes.
If you want to discard the edited message, press the :artwork name='help05.bmp' runin.
button.

.* @2@ **********************************************************************
:h2.Deleting messages
:p.
You can delete a message in one of the following ways&colon.
:ul.
:li.Press the :hp2.DEL:ehp2. key. The message is deleted after a confirmation.
:li.Press the delete button in the :link reftype=hd refid=mainwin.toolbar:elink..
:li.Drag the message to the shredder. Start the dragging over the editor window.
.br
:artwork align=center name='shredmsg.bmp'.
:eul.
:p.
You can also delete messages from the
:link reftype=hd refid=msglist.message list:elink.,
:link reftype=hd refid=threadlist.thread list:elink. or
:link reftype=hd refid=bookmarks.bookmarks window:elink..

.* @2@ **********************************************************************
:h2.Printing messages
:p.
You can print a message in one of the following ways&colon.
:ul.
:li.Press the :hp2.SHIFT-PRINTSCR:ehp2. keys.
:li.Press the print button in the :link reftype=hd refid=mainwin.toolbar:elink..
:li.Drag the message to a printer object. Start the dragging over the editor window.
:eul.
:p.
You can also print messages from the
:link reftype=hd refid=msglist.message list:elink.,
:link reftype=hd refid=threadlist.thread list:elink. or
:link reftype=hd refid=bookmarks.bookmarks window:elink..

.* @2@ **********************************************************************
:h2.Area list
:p.
The area list contains all message areas accessible for FleetStreet.
You can switch to one of these areas, create and delete areas and
set attributes for areas.
:p.
Double click on the area or press :hp2.ENTER:ehp2. to switch to that message area.
:p.
Press :hp2.Scan:ehp2. to start a re-scan of all areas. This may be necessary if
new messages were tossed since the areas were last scanned. The scanning
takes place in a separate thread. You may continue reading while
the scanning continues in the background.
:p.
Related Topics&colon.
:ul compact.
:li.:link reftype=hd refid=areacon.Contents of the area list:elink.
:li.:link reftype=hd refid=areacrea.Creating and deleting areas:elink.
:li.:link reftype=hd refid=areaset.Setting attributes for areas:elink.
:li.:link reftype=hd refid=arealistset.Customizing the area list:elink.
:li.:link reftype=hd refid=areafolders.Area folders:elink.
:eul.

.* ***************************** Area List settings *************************

:h3 id=areacon.Contents of the area list
:p.
For each area in the list, the
:ul compact.
:li.area description
:li.number of messages in the area
:li.number of :link reftype=fn refid=unrmsg.unread messages:elink. in the area
:eul.
:fn id=unrmsg.
:p.Actually this is not the :hp2.real:ehp2. number of unread messages. It is just
the number of messages after the message last read.
:efn.
:p.
are displayed.
:p.
For unscanned areas, "-" is displayed as number of messages.
:p.
The area description initially is the same as the area tag. The area
description is displayed in the main window and can be
:link reftype=fn refid=desccha.changed:elink..

:fn id=desccha.
:p.You can do this in the settings notebook of the area or by clicking
on the area with the left mouse button and while holding down the ALT
key. You can then edit the description directly.
:efn.

:h3 id=areacrea.Creating and deleting areas
:p.
:hp2.Creating a new area:ehp2.
:p.
You can create new areas by selecting :hp2."Create another":ehp2.
in the context menu.
An empty settings notbook for the new area is opened. Fill in
all fields and close the notebook. The new area is then inserted in the
list.
:p.
All areas created in FleetStreet are :hp2.local areas:ehp2..
:p.
:hp2.Deleting areas:ehp2.
:p.
You can delete only those areas which you have created in FleetStreet. Areas
which are defined in the tosser configuration file can only be deleted
there.
:p.
To delete an area open the :link reftype=fn refid=areacon.context menu of the area:elink.
and select :hp2."delete":ehp2.. The
area will be removed from the list.

:fn id=areacon.
:p.Click on the area using the right mouse button
:efn.
:p.
Note&colon. The area files are :hp2.not:ehp2. deleted from disk.

:h3 id=areaset.Setting attributes for areas
:p.
Each area has its own set of attributes. These are&colon.
:ul compact.
:li.Area description
:li.Area tag
:li.Default user name
:li.Default address
:li.Path/File
:li.Area format
:li.Area type
:li.Default message attributes
:li.Other flags
:eul.
:p.
For areas defined in the tosser configuration file, area tag, default address,
path/file, area format and Net/Echo area flag can not be modified.
:p.
To change area attributes, open the context menu for the area and select
:hp2."settings":ehp2..

:h3 id=arealistset.Customizing the area list
:p.
The area list itself has several attributes which influence the appearance
of the list. To change these attributes open the context menu of the
area list and select :hp2."Settings":ehp2..
:p.
You can change
:ul compact.
:li.the default view
:li.the sorting order
:li.the colors used for the different area types
:eul.

:h3 id=areafolders.Area folders
:p.
Areas can be grouped together in folders. One folder is open at a time.
The area list only displays the areas of the open folder.
:p.
There is a :hp2.default folder:ehp2. having some special properties&colon.
:ul compact.
:li.It's on top of the folder hierarchy. All other folders are grouped below.
:li.It can't be deleted or moved.
:li.Areas that weren't moved to a folder (eg. new areas read from the tosser
configuration) can be found in the default folder.
:eul.
:p.
Area folders have several settings&colon.
:ul compact.
:li.A name. You can change it by ALT-clicking on it.
:li.A sorting order of the areas within the folder.
:li.A switch for automatic scanning.
:eul.
:p.
You can create new folders and delete existing folders using the context menu.
You can move folders by dragging them to another folder.
:p.
Areas are moved to a folder by dragging the area to the folder.

.* ****************************** Message liste ******************************
.* @2@ **********************************************************************
:h2 id=msglist.Message list
:p.
The message list displays all messages in an area. The messages are displayed
in the same order as they are stored in the message base.
:p.
Your own name is displayed in a different color. Read and unread messages are displayed
with a message number with a different color. The colors used for these fields
can be chosen in the settings notebook of the message list.
:p.
You can select more than one message using the mouse and manipulate them.
The following actions can be performed on selected messages&colon.
:ul compact.
:li.Delete
:li.Copy
:li.Move
:li.Print
:li.Export
:eul.
:p.
If an error occurs when reading the message in the message base,
:hp2."*":ehp2. is displayed in all fields.
:p.
You can move the column separators to adjust the width of the columns.
:p.
:artwork align=center name='movesepa.bmp'.

.* ****************************** Threadlist *********************************
.* @2@ **********************************************************************
:h2 id=threadlist.Thread list
:p.
The message threads in the current area are displayed. Read messages and
unread messages are displayed with different colors.
:p.
Threads are messages in an area, which are linked together, because one
message is a reply to another message, or because the message has replies
for itself. When a message has replies in the thread list, a :artwork name='plus.bmp' runin.
is displayed before the message. You can click on the :artwork name='plus.bmp' runin. or press
the :hp2.+:ehp2. key to display the replies in a tree-like structure. The :artwork name='plus.bmp' runin.
changes to a :artwork name='minus.bmp' runin. . When you press :hp2.SPACE:ehp2., the whole branch is
expanded immediately.
:p.
Related topics&colon.
:ul compact.
:li.:link reftype=hd refid=thdisp.Display modes:elink.
:li.:link reftype=hd refid=thmani.Manipulating the messages:elink.
:li.:link reftype=hd refid=thlink.Reply linkers:elink.
:li.:link reftype=hd refid=markmsg.Marking messages:elink.
:eul.

:h3 id=thdisp.Display modes
:p.
There are three different display modes&colon. All threads, threads with unread
messages and unread messages only.
:parml.
:pt.:hp2.All threads:ehp2.
:pd.All threads in the area are displayed.

:pt.:hp2.Threads with unread messages:ehp2.
:pd.Only those threads that contain at least one unread message are displayed.
These threads are displayed as a whole.

:pt.:hp2.Unread messages only:ehp2.
:pd.Only those messages, that are unread, are displayed. When several unread
messages in a thread are linked together without a intervening space, they
are displayed in that way. When a thread is interrupted by a read message,
the two parts before and after the read message are displayed as separate
threads.
:eparml.
:p.
You can select a display mode by using the context menu of the thread list.
You can select the default display mode in the settings notebook of the thread
list.
:p.

:h3 id=thmani.Manipulating the messages
:p.
When you manipulate the messages in the thread list, you always manipulate
:hp2.threads:ehp2. or :hp2.part of threads:ehp2.. That means that not only a single
message is affected, but all subsequent replies as well.
:p.
However, "sibling" threads and messages before the selected message are :hp2.not:ehp2.
affected.
:p.
Only one thread can be manipulated at a time. This is an OS/2 limitation.
:p.
You can
:ul compact.
:li.Delete threads
:li.Move threads to a different area
:li.Copy threads to a different area
:li.Export threads to a file on disk
:li.Print threads
:li.Mark threads as "read"
:li.Expand threads
:eul.

:h3 id=thlink.Reply linkers
:p.
FleetStreet exclusively uses the linkage information in the message base,
i.e. it does :hp2.not:ehp2. link messages by itself. You must use a
different program for linking messages. It's best to put such a program
into your mailer batch program and let it do its work immediately
after tossing messages.
:p.
Reply linkers are (among others)&colon.
:ul compact.
:li.Squish
:li.SqmLink
:li.SqLink
:li.QQLink
:eul.
:p.
Squish 1.01 links messages by their subject line. This method has the advantage,
that messages without MSGID/REPLY kludge lines can be linked together. The
disadvantage is, that changing the subject line tears the thread apart.
Also, you can't tell from the links who replied to whom, or whether a message
has more than one reply (the reply chain is always linear).
:p.
Squish 1.10, SqmLink, SqLink und QQLink link the messages by their MSGID/REPLY
kludge lines. Choosing this method allows a reply to be linked to its original
message. Even after changing the subject line, the messages remain linked
together. Threads are no longer linear, but have a tree-like structure.
The Squish message base allows a message to be linked to 10 replies.
:p.
The disadvantage of this method is, that messages can't be linked together
when the reply does not contain a REPLY kludge (e.g. when it is written using
a QWK reader, when it is imported into the area over a gateway, or when
the REPLY kludge was not generated following the standards).

:h3 id=markmsg.Catchup
:p.If you haven't used FleetStreet for reading in a particular message base before,
all messages are marked as "unread". Therefore the message thread list will show
you all messages in that area. Because you may have read these messages before,
this menu item provides a way to mark all the messages as "read".
After you've done that, the thread list will show only messages that are
"really" unread, i.e. messages that were added to this area after you've used
this function.
:p.This function is usually only needed after you've switched to FleetStreet
and want to bring "old" message areas up to date. FleetStreet will maintain the
"read" flag automatically in the future.

.* @2@ **********************************************************************
:h2 id=bookmarks.Bookmarks
:p.The bookmarks window contains three types of messages&colon.
:ul compact.
:li.Results of a find operation
:li.Results of a personal mail scan
:li.Marked messages
:li.Unsent messages
:eul.
:p.
Use the context menu of the list to switch between the three views.
:p.
If you select "Save contents" in the context menu, the contents of the list
are saved to disk when exiting FleetStreet. They are reloaded when re-starting
FleetStreet.
:p.
Double click on a message or press the :hp2.Go to message:ehp2. button to
display the whole message.
:p.
Press the :hp2.Clear:ehp2. button to clear the current view, i.e. all messages
of the current view are removed from the list.
:p.
Use the context menu of a message to
:ul compact.
:li.Delete the message
:li.Export the message
:li.Print the message
:li.Move the message to a different area
:li.Copy the message to a different area
:li.Remove the message from the current view
:eul.

.* @2@ **********************************************************************
:h2.Finding messages
:p.With the find function you can let the messages in one or more areas
be scanned for a text string.
:p.You can open the find dialog e.g. with the "Message/Find" menu. Then enter
your text and specify all search options. Start the search with the "Start"
button. The search takes place in a background thread. Each time an area is
scanned the results are inserted in the
:link reftype=hd refid=bookmarks.Bookmarks window:elink. ("Find results" view).
:p.You can also start the personal mail scan with the find dialog. The results
are inserted in the bookmarks window as well ("personal mail" view).

:h3.Regular Expressions
:p.
(Excerpt from the "VisualAge C++ Programming Guide")&colon.
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
a, Ö, and É belong to the same equivalence class,
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
:c.[˘
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
:h1.Advanced users

.* @2@ **********************************************************************
:h2 id=enteraddr.Entering Fidonet addresses
:p.When writing messages you must specify the network address of the addressee.
This is usually done by entering the full 3-D or 4-D address.
:p.However, FleetStreet can assist you by completing uncomplete addresses. If you
enter an uncomplete address, the missing components are automatically added using
your own address as the default.
:p.The following examples illustrate the process. The default address 2&colon.2490/2520.17
is assumed&colon.
:table cols='12 15 30'.
:row.
:c.Entered address
:c.Resulting address
:c.Comment
:row.
:c.2&colon.2490/2520
:c.2&colon.2400/2520
:c.3-D address specified
:row.
:c.2520
:c.2&colon.2490/2520
:c.Node specified, same net &amp. zone
:row.
:c.247/2099
:c.2&colon.247/2099
:c.Net &amp. Node specified, same zone
:row.
:c.1030.42
:c.2&colon.2490/1030.42
:c.Node &amp. point specified, same zone &amp. net
:row.
:c..42
:c.2&colon.2490/2520.42
:c.Point specified, same boss
:etable.
:p.:hp2.The general rules are&colon.:ehp2.
:ol.
:li.A single number is treated as "Net".
:li.If the point number is not specified, it is set to "0".
:eol.

.* @2@ **********************************************************************
:h2.Toolbar
:p.
The FleetStreet toolbar can be displayed in 4 different positions and 2 sizes.
Select these in the context menu of the toolbar. You can open it by clicking
with the right mouse button on free space inside the toolbar border.


.* @2@ **********************************************************************
:h2.Echo manager
:p.The echo manager is used to easily communicate with the area manager
program of your uplink.
:p.Usually echo areas are subscribed by sending a netmail message to the
area manager of your uplink. The password is placed in the subject line, the
message text contains the names of the areas to be subscribed.
:p.You can unsubscribe from echos or request a list of available echos
in a similar way.
:p.The echo manager now simplifies these procedures&colon.
:ul compact.
:li.You don't have to enter name, address and password of the area manager
by hand,
:li.You can subscribe to and unsubscribe from echos simply by using a
context menu,
:li.The messages to the area manager are created automatically,
:li.Subscribed echos are inserted into the configuration file of
your tosser.
:eul.

:h3.List of echos
:p.The echo manager needs a list of echos available at your uplink to operate.
If you don't already have one, you must first request such a list from your
uplink manually.
:p.The answer from the area manager contains a list of echos. This must
now be passed to FleetStreet's echo manager. Do this by selecting "Setup/Extract areas"
from the menu. FleetStreet now scans the current message for echo names and
uses the results in the echo manager.
:p.FleetStreet saves the address of the sender with each echo list. When you
extract areas and there already is a list from this address, the old list is
replaced by the new list. Otherwise, the sender of the message is added as
new uplink.

:h3.Configuration
:p.To communicate with the area manager of your uplink FleetStreet needs its
name and password. Open the echo manager, open the context menu of the list and
select "Settings". On the first page of the notebook you can find a list
of known uplinks. You can then enter the name and password of the area manager
of each uplink.
:p.
:hp8.Note&colon.:ehp8. Entering the name and password is required to be able
to use the echo manager later on.

:h3.Using the echo manager
:p.To subscribe to echos or unsubscribe from echos using the echo manager,
follow these steps&colon.
:ol.
:li.Select the address as your current address that you want to use for
sending a message to your uplink. If you have several uplinks or networks
and you have one separate netmail area for each one, switch to the
appropriate netmail area. If you only have one uplink, switch to your
netmail area.
:li.Open the echo manager from the menu. The list of available echo areas
is now displayed. If the list is empty you first have to request a list
from the uplink manually.
:li.Open the context menu of the appropriate echo. Select :hp2."link":ehp2..
:li.Subscribing to other echos or unsubscribing from echos works
similarly. The requested action is displayed in the line of the
respective echo area.
:li.Press :hp2.OK:ehp2.. The netmail to your uplink is now generated.
:eol.

:h3.Extension DLLs
:p.
Squish doesn't add new echos to its configuration file automatically. For that
reason FleetStreet can load a DLL and call DLL functions when linking or unlinking
echos. The DLL can then add the echos.
:p.
The DLL that is to be used can be specified in the setup notebook of the echo
manager.
:p.
The file :hp2.FLTCF_SQ.DLL:ehp2. that comes with FleetStreet can be used
as extension DLL if (and only if) you use Squish as your tosser with a plain
SQUISH.CFG file. Other DLLs may be available from third party vendors
when using different installations (e.g. for point packages, other tossers etc.).
:p.
Technical information necessary to write extension DLLs are available from the
:link reftype=hd refid=support.Author:elink..


.* @2@ **********************************************************************
:h2 id=cclists.Using Carbon Copy Lists
:p.Using a carbon copy list means that you're sending the same message to different
addressees. Carbon copy lists can only be used for Netmail.
:p.Imagine the following example&colon.
:p.You're compiling a newsletter that you're sending to a number of people
on a regular basis. To perform this task with FleetStreet, you can create
a carbon copy list named "Newsletter". Now you can add all the people who should
receive the newsletter to the list.
:p.Now, when you want to send the letter via Netmail, compose your message
as you would normally do it. But instead of entering a addressee, select
:hp2.Carbon copy:ehp2. from the menu. You can now select your carbon copy list
"Newsletter" that you've configured before. The text "*** Newsletter ***' appears,
indicating that you're using the list.
:p.When you're sending the next issue of your newsletter, you don't have to create
a new carbon copy list. You can simply re-use the previous one.

:h3.Managing carbon copy lists
:p.All carbon copy lists can be found in the carbon copy list folder. It can
be opened by selecting :hp2."Setup/Carbon Copy Lists":ehp2. from the menu.
Carbon copy lists are displayed as icons.
:p.Use the context menu of a list to delete it or to create a new list.
Double clicking on a list opens it to display and modify its contents.
:p.Hold down the ALT key and click on a carbon copy list to change its name.

:h3.Importing
:p.You can import the contents of a text file into a carbon copy list.
:p.The file must be of the following format&colon.
:ul compact.
:li.Each line contains exactly one name and address
:li.The fields of a line are separated by at least one space
:li.Empty lines are ignored
:li.Lines with a semicolon as the first character are ignored
:li.Lines with an invalid format are ignored.
:eul.
:p.
Example&colon.
:xmp.
; Comment
Michael Hohner 2&colon.2490/2520.17
Hans Dampf 1&colon.234/567
:exmp.

:h3.Quick carbon copy
:p."Normal" carbon copy lists are saved permanently when exiting FleetStreet and
are available later. However, that may not be desired sometimes. For that reason
there is the function :hp2."Quick carbon copy":ehp2. in the :hp2."Special":ehp2.
menu.
:p.The quick carbon copy list works like a normal carbon copy list. However,
it's only composed when you write a message. Its contents are not saved but
discarded after the message has been saved.

.* @2@ **********************************************************************
:h2 id=crosspost.Crossposting messages
:p.Crossposting a message means that the same message is saved in more than one
area.
:p.You can activate crossposting by selecting :hp2.Crosspost:ehp2. from the menu.
This function is available when you are writing a message.
:p.The area list is opened and you can
select the areas, where the message should be saved.
:p.:hp8.Note&colon.:ehp8. You don't have to select the current area in the list.
The message is always saved in the current area. Just select the additional areas.
:p.When crossposting mode is active, a checkmark is displayed in front of
:hp2.Crosspost:ehp2. in the menu.
:p.You can deactivate crossposting mode by selecting :hp2.Crosspost:ehp2. again.
The checkmark will disappear again, indicating that you're in normal editing mode.

.* @2@ **********************************************************************
:h2.Nicknames
:p.With FleetStreet it is possible to define nicknames for users you often
write to.
:p.To use these nicknames, just enter the nickname in the TO field of
the message and press "Enter". The nickname will be replaced by the real name,
and the address of the user will be entered automatically in the address field.
If you defined a subject line for this nickname, it will be inserted in the subject
field as well.
:p.:hp2.To define a nickname, do the following&colon.:ehp2.
:ol compact.
:li.Press the "Add" button,
:li.Fill in the fields,
:li.Press "OK".
:eol.
:p.:hp2.To change a nickname, do the following&colon.:ehp2.
:ol compact.
:li.Select the nickname in the list,
:li.Press the "Change" button or double click on the entry,
:li.Do your changes,
:li.Press "OK".
:eol.
:p.:hp2.To delete a nickname, do the following&colon.:ehp2.
:ol compact.
:li.Select the nickname in the list,
:li.Press the "Delete" button.
:eol.

.* @2@ **********************************************************************
:h2 id=nodelists.Using nodelists
:p.:hp2.What's it all about?:ehp2.
:p.Nodelists contain the names and addresses of all participants of a network.
FleetStreet can look up the address and name of the addressee in a nodelist.
:p.You need a compiled nodelists in "Version 7" format for doing so. This
format is generated by FastLst (among others). FleetStreet needs the data
file and the sysop index file. Configure your nodelist compiler accordingly.
:p.:hp2.How does FleetStreet find the nodelists?:ehp2.
:p.There is a page for the nodelists in the setup notebook. You must
create a "Domain" for each nodelist. A domain entry contains the name
of the domain and the pathnames of the nodelist files. You must enter
the full pathname including drive letter an file extension.
:p.:hp2.What else must be done?:ehp2.
:p.In addition you can enter
descriptions for the nodelist flags.
:p.:hp2.How can I use nodelists?:ehp2.
:p.When writing a NetMail message, enter the full name of the addressee
or enter part of the last name. Then press ENTER. If the name is found,
the address is used immediately. If it is found more than once, a
selection dialog appears.
:p.In some other dialogs where you need to enter an address there is a "?" button.
Press this button to look up the address(es) of the name that you've
entered already.

.* @2@ **********************************************************************
:h2 id=templates.Template
:p.Message templates are used to define a predefined appearence of
new messages, replies and forwards. When writing, replying or forwarding,
the appropriate template items are combined with the message text.
Special :link reftype=hd refid=tokens.tokens:elink. in the message template
are replaced by specific parts of the orignal message when the template
is processed.
:p.The message template is processed when you enter the editor window
for the first time when writing, replying or forwarding. When you have
entered a subject or name of the addressee 'till then, these parts
can be used for token replacement. When you didn't have entered these
parts, the appropriate tokens are replaced "empty".
:p.:hp2.General order&colon.:ehp2.
.br
:hp2.New Message&colon.:ehp2.
:xmp.
[Header template]
[Message text]
[Footer template]
:exmp.
:p.:hp2.Reply&colon.:ehp2.
:xmp.
[Header template]
[Reply template]
[Message text]
[Footer template]
:exmp.
:p.:hp2.Reply in different area&colon.:ehp2.
:xmp.
[Reply-in-different-area template]
[Header template]
[Reply template]
[Message text]
[Footer template]
:exmp.
:p.:hp2.Crosspost&colon.:ehp2.
:xmp.
[Crosspost template]
[Header template]
[Message text]
[Footer template]
:exmp.
:p.:hp2.Carbon copy&colon.:ehp2.
:xmp.
[Carbon copy template]
[Header template]
[Message text]
[Footer template]
:exmp.
:p.:hp2.Forward&colon.:ehp2.
:xmp.
[Forward template]
[Original message text]
[Forward footer template]
[Header template]
[Footer template]
:exmp.

:h3 id=tokens.Tokens
:p.The following tokens are available in message templates&colon.
:parml break=none.
:pt.:hp2.%T:ehp2.
:pd.Name of the addressed user (original message)
:pt.:hp2.%Z:ehp2.
:pd.First name of the addressed user (original message)
:pt.:hp2.%R:ehp2.
:pd.Address of the addressed user (original message). When replying to echomail
or when forwarding echomail, this token is ignored.
:pt.:hp2.%O:ehp2.
:pd.Name of the addressed user (new message)
:pt.:hp2.%P:ehp2.
:pd.First name of the addressed user (new message)
:pt.:hp2.%F:ehp2.
:pd.Name of the sending user
:pt.:hp2.%G:ehp2.
:pd.First name of the sending user
:pt.:hp2.%J:ehp2.
:pd.Address of the sending user
:pt.:hp2.%A:ehp2.
:pd.Area tag. When crossposting a message, this is the destination area. When
replying in a different area or forwarding a message, this is the original area.
:pt.:hp2.%E:ehp2.
:pd.Area description. When crossposting a message, this is the destination area. When
replying in a different area or forwarding a message, this is the original area.
:pt.:hp2.%U:ehp2.
:pd.Your own name
:pt.:hp2.%I:ehp2.
:pd.Your first name
:pt.:hp2.%W:ehp2.
:pd.Your own address
:pt.:hp2.%C:ehp2.
:pd.Names of the users in the carbon copy list
:pt.:hp2.%D:ehp2.
:pd.Date of the message you are replying to
:pt.:hp2.%M:ehp2.
:pd.Time of the message you are replying to
:pt.:hp2.%S:ehp2.
:pd.Subject of the message you area replying to
:pt.:hp2.%%:ehp2.
:pd.A literal %
:eparml.

.* @2@ **********************************************************************
:h2.Drive remapping
:p.
When the message base is residing on a different machine in a LAN, it is
desireable to maintain a single tosser configuration file that is residing on the same
machine as the message base. When you use this tosser configuration file in FleetStreet, and
the remote drives are mounted on different drive letters on your local machine,
FleetStreet would use incorrect drive letters when accessing the message base.
:p.
Drive Remapping provides a solution to this problem. You can assign remote
drive letters to local drive letters. FleetStreet replaces remote drive letters
by the assigned ones before accessing the message base.
:p.
Example&colon.
:p.
The message base is on machine A on drive D&colon.. FleetStreet is running on machine
B, drive D&colon. of machine A is mounted on drive E&colon.. The tosser configuration file also is on
machine A, all message area files are therefore specified with D&colon. as drive
letters.
:p.
When FleetStreet is configured to remap drive D&colon. to E&colon., the drive letters
of the area file names read from the tosser configuration file are replaced by E&colon.. When
FleetStreet accesses the area files, the correct file names are used.
:p.
Note&colon. Only the drive letters of file names read from the tosser configuration file are remapped.
When you create new areas in FleetStreet, you must use the correct drive
letter yourself, FleetStreet does not do any remapping with these file names.

.* @2@ **********************************************************************
:h2 id=colorsetup.Changing colors and fonts
:p.You won't find any menu item or dialog for changing colors or fonts
of the main window of FleetStreet. This is because FleetStreet uses
Workplace Shell objects to costumize its appearance.
:parml tsize=3.
:pt.:hp2.The color palette:ehp2.
:pd.Open a color palette, drag a color to FleetStreet and drop it on
the appropriate element in the window. To change the color of textual elements
hold down the CTRL key while dropping. If you don't hold down CTRL the
background color of the element will be changed.
:pt.:hp2.The font palette:ehp2.
:pd.Open a font palette, drag a font to FleetStreet and drop it on
the appropriate element in the window.
:eparml.

.* @2@ **********************************************************************
:h2.Import, Export
:p.
When you write a message you can insert a text file at the current
cursor position.
:ul.
:li.Drag the file from a WPS folder to the editor window.
:artwork align=center name='dropfile.bmp'.
:li.Use the import function in the "File" menu.
:eul.
:p.
When reading messages you can export a message to a normal
text file.
:ul.
:li.Drag the message to a WPS folder. Start the dragging over the
editor window.
:li.Use the export function in the "File" menu.
:eul.


.* @1@ **********************************************************************
:h1 id=advtopics.Advanced Topics, hints &amp. tips
:p.
The following topics might be interesting after you've get used with
FleetStreet&colon.
:ul compact.
:li.:link reftype=hd refid=multinst.Multiple instances:elink.
:li.:link reftype=hd refid=perform.Improving performance:elink.
:li.:link reftype=hd refid=multuser.Multi-User operation:elink.
:eul.

.* ************************** Mehrere Instanzen ******************************
:h2 id=multinst.Multiple instances
:p.
You can run FleetStreet more than once at a time. Follow these
guidelines when doing so&colon.
:ul.
:li.Only the instance that was started first saves its settings. Change
settings only within the first instance if you like to keep the changes.
:li.Secondary instances can be identified by a :hp2.[*]:ehp2. in the
title bar.
:li.You can't run multiple instances of FleetStreet if they're different
versions. Also, you can't run a english version and a german version at the
same time. In this case the DLLs of the first instance are used for all instances.
This is a limitation of OS/2. The behaviour of the secondary instances when
running different versions is undefined.
:li.Secondary instances don't have a pipe server.
:eul.

.* ************************** Performance       ******************************
:h2 id=perform.Improving performance
:p.The performance of FleetStreet highly depends on two factors&colon. Performance
of the Squish MSGAPI and performance of your hard disks. While the MSGAPI
lies beyond our influence, changes to the second factor can improve FleetStreet's
performance.
:p.Some hints in detail&colon.
:ul.
:li.Whenever possible use Squish style areas instead of *.MSG.
:li.Regularily pack your Squish style areas using SqPackP. After packing
an area, the messages are stored without intervening space in the message base and are
numbered contigously.
:li.Use HPFS.
:li.Exclude areas from the area list if you don't read them yourself.
Activate "Hide excluded areas".
:li.When searching text, let FleetStreet search only in the message
headers whenever possible, not in the whole mesage.
:eul.

.* ************************** Multi-User   ***********************************
:h2 id=multuser.Multi-User operation
:p.FleetStreet can be configured for operating with more than one user, with some
limitations. Here's how to do it&colon.
:ol.
:li.Create a program directory for FleetStreet. Copy the EXE file and all DLLs
to into this directory.
:li.Create a configuration directory for each user.
:li.Create a program object for each user. Specify the full pathname of FLTSTRT.EXE
as the program name.
:li.Enter the "-C" :link reftype=hd refid=cmdlin.command line parameter:elink.,
specifying the configuration directory of each user, e.g. "-Cd&colon.\fleet\user1".
:li.Now you can configure FleetStreet for each user. The configuration directory
of a user contains a complete set of INI files.
:li.Use different lastread offsets for each user.
:eol.
:p.:hp2.Limitations&colon.:ehp2.
:ul.
:li.The "read" flag only exists once for each message, i.e. it is the same
for all users.
:li.The "private" flag isn't treated in a special way.
:eul.


.* ************************** Howto        ***********************************
.* @2@ **********************************************************************
:h2.How can I...
:p.
The following chapters describe common problems and how to solve them
with FleetStreet.

:h3.send files with a message?
:p.
You can send files together with a message. To do this you must enter
the file names in the subject line. File names must be separated by
at least one blank. It's necessary to set the "file attached"
message attribute to make the tosser or mailer process the
message appropriately.
:p.
You can drag files from any WPS folder to the subject line to attach them to
the message. The file names are entered in the subject line, the "File Attach"
flag is switched on and a short summary of the attached files is displayed.
This works only when you write a message!
:artwork align=center name='attfile.bmp'.


.* ************************* Rexx ******************************************

.* @1@ **********************************************************************
:h1.Rexx script programming
:p.FleetStreet has the capability to run scripts in the Rexx language. The Rexx
language is extended by :hp2.predefined variables:ehp2. and additional :hp2.functions:ehp2..

.* @2@ **********************************************************************
:h2.Programming reference
:p.This reference lists all :link reftype=hd refid=rexxvar.predefined variables:elink. and
additional :link reftype=hd refid=rexxfunc.functions:elink..
:p.For standard Rexx features, see the online Rexx information.

:h3.The FleetStreet environment
:p.Rexx scripts running under FleetStreet don't run in their default environment,
CMD.EXE. The Rexx environment for scripts running under FleetStreet is called
:hp2.FLEETSTREET:ehp2..
:p.So when issueing non-Rexx commands, these commands are evaluated by FleetStreet.
If you want CMD.EXE to evaluate the command, you explicitly need to address the
CMD.EXE environment with the :hp2.ADDRESS:ehp2. command.
:p.Example&colon.
:xmp.
/* WRONG! */
'dir'

/* right */
address CMD 'dir'

/* also right */
address CMD
'dir'
address FLEETSTREET
:exmp.
:p.See the online Rexx documentation for more information about Rexx environments
and the ADDRESS command.

.* ***************************** Variablen  ************************************

:h3 id=rexxvar.Predefined variables
:p.When a Rexx script is started, some variables already have been initialized with
values. These variables and their associated values may be used in the script.
:p.:hp8.Note&colon.:ehp8. When the value of a predefined variable is changed in the script, the changes
do not affect FleetStreet, as long as you don't use a FleetStreet function or
command to apply the changes.
:p.The predefined variables are&colon.
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
:p.This is an array of user names. :hp4.FleetSetup.Names.0:ehp4. contains the number of
elements in the array. :hp4.FleetSetup.Names.1:ehp4. etc. contain the names.
:p.Example&colon.
:table cols='20 10'.
:row.
:c.Variable
:c.Value
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
:p.This is an array of user addresses. :hp4.FleetSetup.Addresses.0:ehp4. contains the number of
elements in the array. :hp4.FleetSetup.Addresses.1:ehp4. etc. contain the addresses.
:p.Example&colon.
:table cols='22 14'.
:row.
:c.Variable
:c.Value
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
:p.:hp4.FleetSetup.Echotoss:ehp4. contains the name of the Echotoss.Log file.

:h4 id=rvar04.FleetSetup.Tosser
:p.:hp4.FleetSetup.Tosser:ehp4. contains the name of the tosser configuration file.

:h4 id=rvar05.FleetStatus.Area
:p.:hp4.FleetStatus.Area:ehp4. is a group of variables, that contain information
about the current area.
:p.The variables are&colon.
:parml.
:pt.:hp4.FleetStatus.Area.Tag:ehp4.
:pd.Area tag
:pt.:hp4.FleetStatus.Area.Desc:ehp4.
:pd.Area description
:pt.:hp4.FleetStatus.Area.File:ehp4.
:pd.Path and/or file name of the area.
:pt.:hp4.FleetStatus.Area.Format:ehp4.
:pd.Format of the area. This is either "*.MSG", "Squish" or "JAM"
:pt.:hp4.FleetStatus.Area.Type:ehp4.
:pd.Type of the area. This is one of "Echo", "Net", "Local" and "Private".
:eparml.

:h4 id=rvar06.FleetStatus.DestArea
:p.:hp4.FleetStatus.DestArea:ehp4. contains the area tag of the area, in which
a message is to be saved when replying or forwarding.
:p.This variable is only defined when editing a message!

:h4 id=rvar07.FleetStatus.Name
:p.:hp4.FleetStatus.Name:ehp4. contains the currently active user name.

:h4 id=rvar08.FleetStatus.Address
:p.:hp4.FleetStatus.Address:ehp4. contains the currently active user address.

:h4 id=rvar09.FleetStatus.Mode
:p.:hp4.FleetStatus.Mode:ehp4. contains the current program status. It can have one
of the following values&colon.
:sl compact.
:li.No Setup
:li.Edit Single
:li.Edit XPost
:li.Edit CCopy
:li.Read
:li.Cleanup
:esl.
:p.
When writing a message, the first word in the variable is "Edit". The second word
indicates whether you're writing a single message or use a carbon copy list or the
crossposting feature.

:h4 id=rvar10.FleetStatus.Monitor
:p.:hp4.FleetStatus.Monitor:ehp4. is either "0" when no monitor window is used, or
is "1" when the script is running with a monitor window. :hp4.FleetStatus.Monitor:ehp4.
can be used directly in a boolean expression.

:h4 id=rvar11.FleetStatus.Cursor
:p.When editing a message, :hp4.FleetStatus.Cursor:ehp4. contains two numbers indicating
the current cursor position. The first number is the paragraph, the second number is
the character position in the paragraph. For example, "46 3" means that the cursor
is at the third character in the 46th paragraph.
:p.
The following program outputs the text after the cursor&colon.
:xmp.
para = word(FleetStatus.Cursor, 1)
offs = word(FleetStatus.Cursor, 2)
say substr(FleetMsg.Text.para, offs)
:exmp.
:p.
When reading messages, :hp4.FleetStatus.Cursor:ehp4. is undefined.


:h4 id=rvar12.FleetMsg.Header
:p.:hp4.FleetMsg.Header:ehp4. is a group of variables, that contain the header of the
current message.
:p.The variables are&colon.
:parml.
:pt.:hp4.FleetMsg.Header.Attrib:ehp4.
:pd.Message attributes, same as the "Attrib" line.
:pt.:hp4.FleetMsg.Header.From:ehp4.
:pd.Name of the sender of the message.
:pt.:hp4.FleetMsg.Header.FromAddress:ehp4.
:pd.Address of the sender of the message.
:pt.:hp4.FleetMsg.Header.To:ehp4.
:pd.Name of the recipient of the message.
:pt.:hp4.FleetMsg.Header.ToAddress:ehp4.
:pd.Address of the recipient of the message. Don't use this variable in EchoMail
areas!
:pt.:hp4.FleetMsg.Header.Subj:ehp4.
:pd.Subject line.
:pt.:hp4.FleetMsg.Header.DateWritten:ehp4.
:pd.Timestamp of the time when the message was written.
:pt.:hp4.FleetMsg.Header.DateReceived:ehp4.
:pd.Timestamp of the time when the message was tossed.
:eparml.

:h4 id=rvar13.FleetMsg.Text
:p.:hp4.FleetMsg.Text:ehp4. is an array of text paragraphs. :hp4.FleetMsg.Text.0:ehp4. contains the number
of elements in the array. :hp4.FleetMsg.Text.1:ehp4. etc. contain the message text.
:p.:hp8.Note&colon.:ehp8. The elements in the array are :hp2.not:ehp2. text lines, but paragraphs.
A paragraph in the original text ends with a hard Carriage Return. If you change the
text e.g. by deleting or adding words, the text can be word-wrapped again. If
you need your own format with lines of a specific length, you must perform your
own word wrapping. This should be quite easy in Rexx.

:h4 id=rvar16.FleetMsg.Kludges
:p.:hp4.FleetMsg.Kludges:ehp4. is an array that contains the kludge lines
of the current message. The array is only defined when reading messages.
:p.
The fields of :hp4.FleetMsg.Kludges:ehp4. are &colon.
:parml.
:pt.:hp2.FleetMsg.Kludges.0:ehp2.
:pd.Number of elements in the array
:pt.:hp2.FleetMsg.Kludges.1:ehp2.
:pd.First kludge line
:pt.:hp2.FleetMsg.Kludges.*:ehp2.
:pd.all other kludge lines
:eparml.
:p.
The kludge lines look like
:xmp.
NAME&colon. value
:exmp.
:p.
or
:xmp.
NAME value
:exmp.
:p.
A kludge line can appear more than once. The kludge lines don't have a specific order.
The character :hp2.01 hex:ehp2. at the beginning of a kludge line is not included
in the variables.

:h4 id=rvar17.FleetMsg.Seenbys
:p.:hp4.FleetMsg.Seenbys:ehp4. is an array that contains the SEEN-BY lines
of the current message. The array is only defined when reading messages.
:p.
The fields of :hp4.FleetMsg.Seenbys:ehp4. are &colon.
:parml.
:pt.:hp2.FleetMsg.Seenbys.0:ehp2.
:pd.Number of elements in the array
:pt.:hp2.FleetMsg.Seenbys.1:ehp2.
:pd.First line
:pt.:hp2.FleetMsg.Seenbys.*:ehp2.
:pd.all other lines
:eparml.
:p.
SEEN-BY lines look like
:xmp.
SEEN-BY: nodes
:exmp.
:p.
The lines appear in the same order as they do in the original message.

:h4 id=rvar14.FleetCCopy
:p.When writing a message and using a carbon copy list (or the quick carbon copy
feature), the Rexx array :hp4.FleetCCopy:ehp4. contains the names and addresses
in the carbon copy list. When not using a carbon copy list, the variables
are undefined.
:p.
The fields of :hp4.FleetCCopy:ehp4. are&colon.
:parml.
:pt.:hp2.FleetCCopy.0:ehp2.
:pd.Number of entries in the carbon copy list.
:pt.:hp2.FleetCCopy.1.Name:ehp2.
:pd.Name of the first entry.
:pt.:hp2.FleetCCopy.1.Address:ehp2.
:pd.Address of the first entry.
:pt.:hp2.&dot.&dot.&dot.:ehp2.
:pd.
:eparml.

:h4 id=rvar15.NewMail
:p.:hp4.NewMail:ehp4. can have a combination of the following values (separated
by one space character)&colon.
:parml break=none.
:pt.:hp2.'Echo':ehp2.
:pd.New EchoMail has been entered
:pt.:hp2.'Net':ehp2.
:pd.New NetMail has been entered
:pt.:hp2.'Local':ehp2.
:pd.New local mail has been entered
:eparml.
:p.
When no messages have been entered, the variable is empty.
:p.
:hp8.Note&colon.:ehp8. This variable only has a value during exit processing,
i.e. when the script is executed as the "Exit" hook. Otherwise it is undefined.


.* ***************************** Funktionen ************************************

:h3 id=rexxfunc.Functions
:p.FleetStreet adds some functions to Rexx. These are&colon.
:sl compact.
:li.:hp4.FSCls:ehp4.
:li.:hp4.FSLookupAddress:ehp4.
:li.:hp4.FSLookupName:ehp4.
:li.:hp4.FSSetEntryField:ehp4.
:li.:hp4.FSSetHeader:ehp4.
:li.:hp4.FSSetText:ehp4.
:esl.

:h4.FSCls
:p.:hp4.FSCls:ehp4. clears the monitor window.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSCls()
:exmp.
:p.:hp2.Parameters&colon.:ehp2.
:p.
FSCls doesn't need any parameters.
:p.:hp2.Return values&colon.:ehp2.
:p.:hp4.FSCls:ehp4. returns "OK", when the monitor window is cleared. "NoMonitor" is
returned, when the script is running without a monitor window.
:p.:hp2.Example&colon.:ehp2.
:xmp.
call FSCls
:exmp.

:h4.FSSetHeader
:p.:hp4.FSSetHeader:ehp4. takes a stem name of a Rexx array as parameter and updates
the message header of the current message with the contents of the variables.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetHeader(stem)
:exmp.
:p.:hp2.Parameters&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.Rexx array, which contains the message header. stem has the following fields&colon.
:ul compact.
:li.From
:li.FromAddress
:li.To
:li.ToAddress
:li.Subj
:eul.
:p.
These are the same fields as contained in :hp4.FleetMsg.Header:ehp4., however, only
the fields above are used.
:eparml.
:p.
:hp2.Return values&colon.:ehp2.
:p.:hp4.FSSetHeader:ehp4. returns "OK".
:p.
:p.:hp2.Notes&colon.:ehp2.
:ul.
:li.All elements in the array must have a value, even if it's just a null string.
:li.Always put the stem name in quotes, so that it is not replaced by its value.
:li.When reading, the new header is not saved on disk. When editing, the
new header is only saved, if you save the whole message (CTRL-S).
:eul.
:p.
:p.:hp2.Example&colon.:ehp2.
:xmp.
/* Replace sender name */
FleetMsg.Header.From = 'Joe user'
RetVal = FSSetHeader('FleetMsg.Header')
:exmp.

:h4.FSSetText
:p.:hp4.FSSetText:ehp4. takes a stem name of a Rexx array as parameter and uses the text
in the array as the current message text. The text in the array replaces the
previous text.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetText(stem)
:exmp.
:p.
:p.:hp2.Parameters&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.stem:ehp2.
:pd.Rexx array, which contains the message text. stem.0 contains the number or
paragraphs, stem.1 ... stem.n contain the paragraphs.
:eparml.
:p.
:p.:hp2.Return values&colon.:ehp2.
:p.:hp4.FSSetText:ehp4. returns "OK".
:p.
:p.:hp2.Notes&colon.:ehp2.
:ul.
:li.The format of the array is the same as the format of FleetMsg.Text.
:li.Element 0 of the array must have a numeric value.
:li.All elements in the array must have a value, even if it's just a null string.
:li.Always put the stem name in quotes, so that it is not replaced by its value.
:li.When reading, the new text is not saved on disk. When editing, the
new text is only saved, if you save the whole message (CTRL-S).
:eul.
:p.
:p.:hp2.Example&colon.:ehp2.
:xmp.
NewText.0 = 2
NewText.1 = 'This is just'
NewText.2 = 'a short message.'
RetVal = FSSetText('NewText')
:exmp.


:h4.FSLookupAddress
:p.:hp4.FSLookupAddress:ehp4. looks up a FTN address in the nodelist. The
result is put into a stemmed variable.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSLookupAddress(ftnaddress, stem)
:exmp.
:p.
:p.:hp2.Parameters&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.ftnaddress:ehp2.
:pd.FTN address of the node to look up.
:pt.:hp2.stem:ehp2.
:pd.Stem name of the variables receiving the results.
:p.The stemmed variable has the following fields&colon.
:parml.
:pt.:hp4.Stem.Address:ehp4.
:pd.Address of the node.
:pt.:hp4.Stem.Name:ehp4.
:pd.Name of the SysOp
:pt.:hp4.Stem.System:ehp4.
:pd.Name of the system
:pt.:hp4.Stem.Phone:ehp4.
:pd.Phone number
:pt.:hp4.Stem.Location:ehp4.
:pd.Location of the node
:pt.:hp4.Stem.Password:ehp4.
:pd.The session password. This field is empty if you haven't defined a password for the node.
:pt.:hp4.Stem.Modem:ehp4.
:pd.Modem type. This is a numeric value.
:pt.:hp4.Stem.Baud:ehp4.
:pd.Maximum baud rate.
:pt.:hp4.Stem.UserCost:ehp4.
:pd.Cost for a user to write a message to the node.
:pt.:hp4.Stem.CallCost:ehp4.
:pd.Cost for calling the node.
:pt.:hp4.Stem.Flags:ehp4.
:pd.Node flags, a combination of "ZC", "RC", "MO", "Hub", "Host" and "CM".
:eparml.
:eparml.
:p.
:p.:hp2.Return values&colon.:ehp2.
:p.:hp4.FSLookupAddress:ehp4. returns one of the following&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.The node was found. The node data is returned in the second parameter.
:pt."NotFound"
:pd.The node was not found.
:pt."Error"
:pd.An error occured during lookup.
:eparml.
:p.
:p.:hp2.Notes&colon.:ehp2.
:ul.
:li.When the address string is too long, it is truncated. The field "Stem.Address"
contains the resulting address.
:li.Always put the stem name in quotes, so that it is not replaced by its value.
:eul.
:p.
:p.:hp2.Example&colon.:ehp2.
:xmp.
RetVal = FSLookupAddress('2&colon.2490/2520', 'NodeData')
say 'System name&colon.' NodeData.System
:exmp.


:h4.FSLookupName
:p.:hp4.FSLookupName:ehp4. looks up a sysop name in the nodelist. The
result is put into a stemmed variable.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSLookupName(name, stem)
:exmp.
:p.
:p.:hp2.Parameters&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.name:ehp2.
:pd.Sysop name to look up.
:pt.:hp2.stem:ehp2.
:pd.Stem name of the variables receiving the results.
:p.The stemmed variable has the following fields&colon.
:parml.
:pt.:hp4.Stem.0:ehp4.
:pd.Number of found entries in the nodelist.
:pt.:hp4.Stem.1.Address:ehp4.
:pd.Address of the node (1st entry)
:pt.:hp4.Stem.1.Name:ehp4.
:pd.Name of the SysOp (1st entry)
:pt.:hp4.Stem.1.System:ehp4.
:pd.Name of the system (1st entry)
:pt.:hp4.Stem.1.Phone:ehp4.
:pd.Phone number (1st entry)
:pt.:hp4.Stem.1.Location:ehp4.
:pd.Location of the node (1st entry)
:pt.:hp4.Stem.1.Password:ehp4.
:pd.The session password (1st entry). This field is empty if you haven't defined a password for the node.
:pt.:hp4.Stem.1.Modem:ehp4.
:pd.Modem type (1st entry). This is a numeric value.
:pt.:hp4.Stem.1.Baud:ehp4.
:pd.Maximum baud rate (1st entry).
:pt.:hp4.Stem.1.UserCost:ehp4.
:pd.Cost for a user to write a message to the node (1st entry).
:pt.:hp4.Stem.1.CallCost:ehp4.
:pd.Cost for calling the node (1st entry).
:pt.:hp4.Stem.1.Flags:ehp4.
:pd.Node flags, a combination of "ZC", "RC", "MO", "Hub", "Host" and "CM" (1st entry).
:eparml.
:eparml.
:p.Stem.2 etc. contain further entries.
:p.
:p.:hp2.Return values&colon.:ehp2.
:p.:hp4.FSLookupName:ehp4. returns one of the following&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.The name was found. The node data is returned in the second parameter.
:pt."NotFound"
:pd.The name was not found.
:pt."Error"
:pd.An error occured during lookup.
:eparml.
:p.
:p.:hp2.Notes&colon.:ehp2.
:ul.
:li.When the name string is too long, it is truncated. The field "Stem.x.Name"
contains the resulting name.
:li.Always put the stem name in quotes, so that it is not replaced by its value.
:li.You can also specify only a part of the last name.
:eul.
:p.
:p.:hp2.Example&colon.:ehp2.
:xmp.
RetVal = FSLookupName('Joe User', 'NodeData')
do i = 1 to NodeData.0
  say 'Address&colon.' NodeData.i.Address
end
:exmp.

:h4.FSSetEntryField
:p.:hp4.FSSetEntryField:ehp4. sets the text of the entry field in the monitor window.
This is a means to provide default values for user input.
:p.:hp2.Syntax&colon.:ehp2.
:xmp.
result = FSSetEntryField(text)
:exmp.
:p.
:p.:hp2.Parameters&colon.:ehp2.
:parml break=none tsize=15.
:pt.:hp2.text:ehp2.
:pd.New text for the entry field
:eparml.
:p.:hp2.Return values&colon.:ehp2.
:p.:hp4.FSSetEntryField:ehp4. returns one of the following&colon.
:parml break=none tsize=15.
:pt."OK"
:pd.The text was set correctly.
:pt."NoMonitor"
:pd.The script is running without a monitor window, so the entry field text
could not be set.
:eparml.
:p.
:p.:hp2.Notes&colon.:ehp2.
:ul.
:li.The text is truncated to 500 characters.
:eul.
:p.
:p.:hp2.Example&colon.:ehp2.
:xmp.
RetVal = FSSetEntryField('C&colon.\')
if RetVal = 'OK' then
   do
   say 'Enter path'
   parse pull mypath .
   end
:exmp.

.* **************************** Hooks ***************************************
.* @2@ **********************************************************************

:h2.Hooks
:p.
FleetStreet can start certain scripts in certain situations automatically.
:p.
To assign a script to a certain hook, follow these steps&colon.
:ol compact.
:li.Open the Rexx script folder
:li.Select "Settings" in the context menu of the folder
:li.Switch to the "Hooks" page of the notebook
:li.Select the desired script in the drop-down list of the hook
:eol.
:p.
Currently the following hooks are available&colon.
:ul compact.
:li.Exit
:li.Pre-Save
:eul.

.* **************************** Pipe-Server ********************************
.* @1@ **********************************************************************
:h1.The FleetStreet pipe server
:p.This sections describes the functions of the pipe server of FleetStreet.
:p.
FleetStreet automagically starts a thread for serving a named pipe.
FleetStreet can be controlled through this pipe.
:p.
The name of the pipe is
:xmp.
\PIPE\FleetStreetDoor
:exmp.
:p.
The pipe is bi-directional. Applications that want to communicate with
FleetStreet over the pipe must use a special protocol. This protocol is
described below. However, in most cases FleetCom will be sufficient.
FleetCom is a special FleetStreet client, which can be easily integrated
in batch programs.
:p.
Related topics&colon.
:ul compact.
:li.:link reftype=hd refid=proto.The protocol:elink.
:li.:link reftype=hd refid=commands.Commands of the Pipe Server:elink.
:li.:link reftype=hd refid=fleetcom.FleetCom:elink.
:eul.

.* @2@ **********************************************************************
:h2 id=proto.The Protocol
:p.
A session with FleetStreet is devided into 3 Phases&colon. Session startup,
Command transaction and ending a session.
:p.
These 3 phases are described in this chapter. The following ASCII characters
are used&colon.
:p.
:table cols='8 11 7'.
:row.
:c.Symbol
:c.Hexadecimal
:c.Decimal
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
:hp2.Session startup:ehp2.
:p.
After a client has opened the pipe, it first sends an ENQ to FleetStreet.
FleetStreet responds with the ID string "FleetStreet"<ETX>. The client now
must check if it's really FleetStreet, that is serving the pipe, i.e.
if the ID string matches "FleetStreet". If thats the case, the client sends
an ACK to the server. Otherwise the client sends a NAK. In this case,
FleetStreet responds with an EOT and ends the session.
:p.
When FleetStreet receives an ACK, it replies by sending its version number
as ASCII text, e.g. "0.88"<ETX>. If the client can only communicate with
certain versions of FleetStreet, it may check the version number as well.
It then sends back an ACK or NAK as before. If it doesn't ceck the version
string, an ACK must be sent to the server.
:p.
The session startup phase is now finished and FleetStreet waits for the client
to send commands.
:p.
Session startup&colon.
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
Session startup with error&colon.
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
:p.
:hp2.Command transaction:ehp2.
:p.
The client sends command to FleetStreet as simple ASCII text. Echo command
must end with an ETX character.
:p.
Example&colon.
:xmp.
"SCAN *"<ETX>
:exmp.
:p.
Parameters are separated from each other and from the command by single
whitespace characters.
:p.
FleetStreet checks the validity of a command. When a command is not valid,
the sequence
:xmp.
<NAK><Code><ETX>
:exmp.
:p.
is sent back. <Code> is one of the three characters C, P or S.
The meaning of the three characters is&colon.
:p.
:parml break=none.
:pt.C
:pd.The command was not recognized by FleetStreet
:pt.P
:pd.Parameters are missing or they are invalid
:pt.S
:pd.More parameters than expected were specified.
:eparml.
:p.
When the command is valid, the sequence
:xmp.
<ACK><ETX>
:exmp.
:p.
is sent back. In this case, FleetStreet starts to execute the command. When
FleetStreet is finished, the result of the command is sent to the client.
When an error occured during execution of the command, the sequence
:xmp.
<NAK>"error text"<ETX>
:exmp.
:p.
is sent. "error text" is a description of the error in plain ASCII text.
When the command was processed without errors, the sequence
:xmp.
<ACK>"result"<ETX>
:exmp.
:p.
is sent. "result" is the desired information, a status report or may be empty
as well.
:p.
After processing a command, another command may be sent to FleetStreet.
This may happen as often as necessary, until the end of the session.
:p.
Schematic description&colon.
:xmp.
 Client              FleetStreet

       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

        [Processing]

     <ACK>"34 areas"<ETX>
   <----------------------
:exmp.
:p.
Invalid command&colon.
:xmp.
       "ABC XYZ"<ETX>
   ---------------------->

        <NAK>"C"<ETX>
   <----------------------
:exmp.
:p.
Error during processing&colon.
:xmp.
       "SCAN *"<ETX>
   ---------------------->

         <ACK><ETX>
   <----------------------

        [Processing]

    <NAK>"disk error"<ETX>
   <----------------------
:exmp.
:p.
:hp2.End of session:ehp2.
:p.
After the last command was processed, the session must be stopped. The client
sends an
:xmp.
      EOT
:exmp.
:p.
to the server. The server responds with an
:xmp.
      EOT
:exmp.
:p.
and both end the connection.
:p.
Schematic description&colon.
:xmp.
 Client              FleetStreet

            EOT
   ---------------------->

            EOT
   <----------------------
:exmp.

.* @2@ **********************************************************************
:h2 id=commands.The commands of the pipe server
:p.
Currently the following commands are implemented&colon.
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
The areas specified are re-scanned. You can specifiy areas with&colon.
:parml break=none tsize=20.
:pt.:hp2.*:ehp2.
:pd.All areas
:pt.:hp2.Area tag:ehp2.
:pd.Only this area, e.g. TUB
:pt.:hp2.@filename:ehp2.
:pd.All areas that are specified in the file. The format of the file
is the same as the format of ECHOTOSS.LOG, i.e. one area tag per line.
:eparml.

:h3.ETOSS
:p.
:hp2.ETOSS:ehp2.
:p.
The echotoss.log file is written to disk. If you don't specify parameters,
the file is created according to the FleetStreet setup. A parameter can
can specified and denotes the file name of the file created as echotoss.log.

:h3 hide.LOCK
:p.
:hp2.LOCK:ehp2.
:p.
The areas specified are locked, i.e. they are no longer used by FleetStreet.
When an area is currently used by FleetStreet, the command continues being
processed until the area is no longer used.
:p.
Areas may be specified in the same format as with SCAN.

:h3 hide.UNLCK
:p.
:hp2.UNLCK:ehp2.
:p.
The areas specified are unlocked. Areas may be specified in the same format
as with LOCK.

.* @2@ **********************************************************************
:h2 id=fleetcom.FleetCom
:p.
FleetStreet comes with a special tool named FleetCom. FleetCom is a client
program for the FleetStreet pipe server. Usage of FleetCom&colon.
:xmp.
FLEETCOM command [parameters]
:exmp.
:p.
FleetCom establishes a connection with the pipe server, sends the command
and parameters and processes data returned by the server. In most
cases FleetCom is sufficient for using the pipe server in FleetStreet.
:p.
FleetCom returns the following exit codes&colon.
:parml break=none tsize=4.
:pt.0
:pd.OK, no error
:pt.1
:pd.the pipe could not be opened. This error also occurs when FleetStreet
is not running, so it can be ignored in most cases.
:pt.2
:pd.no command specified
:pt.3
:pd.FleetStreet doesn't recognize the command
:pt.4
:pd.error while processing the command, or system error.
:eparml.
:p.
FleetCom uses the pipe \PIPE\FleetStreetDoor by default. However, a different
name may be used as well. It is taken from the environment variable
FLEETPIPE. Example&colon.
:xmp.
SET FLEETPIPE=\PIPE\AnotherPipe
:exmp.
:p.
When FleetCom is to use the default name again, delete the variable by using
:xmp.
SET FLEETPIPE=
:exmp.
:p.
:hp2.Examples for using FleetCom&colon.:ehp2.
:p.
Rescan areas after new mesages were tossed&colon.
:xmp.
SquishP IN -f echotoss.log
FleetCom scan @echotoss.log
:exmp.
:p.
Write Echotoss.Log to scan out message from these areas&colon.
:xmp.
FleetCom etoss pack.log
SquishP OUT SQUASH -f pack.log
:exmp.

.* @1@ **********************************************************************
:h1.Message base considerations
:p.
Each of the three message base formats supported by FleetStreet has its
own advantages and problems. This chapter describes how FleetStreet handles the
different formats and assists you to decide which format to use.

.* @2@ **********************************************************************
:h2.*.MSG
:p.
FleetStreet uses Squish's MSGAPI32.DLL to access *.MSG areas. This API reads
up to 512 bytes of kludge lines from a message. In case there are more
kludge lines in a message (which is often the case with messages gated from
the Internet), the kludges are truncated after 512 bytes and the remainder
is provided in the message text.
:p.
FleetStreet extracts the remaining kludge lines from the message text. A truncated
and thereby split kludge line, however, cannot be detected as such. Therefor
it's possible that the second half of the split kludge line appears on top of
the message text.
:p.
The *.MSG format only provides space for the Fido standard attributes. All other
attributes are saved in a FLAGS kludge line. This kludge line, however, is
neither recognized nor processed by Squish (the tosser program), so attributes
like "direct" or "archive/sent" don't have any effect when using Squish.
:p.
FleetStreet uses the "times read" field in the *.MSG files for its "read"
attribute. Squish resets this field to zero when packing the message, so
the message is displayed as "unread" again afterwards.

.* @2@ **********************************************************************
:h2.Squish
:p.
Squish's MSGAPI32.DLL that's used to access Squish areas has a design flaw&colon.
When opening the area, the area's index (*.SQI) is read into memory. When
modifying the area (e.g. by adding or deleting) messages, the index file is
not updated immediately. The modifications are instead done in memory and
only written back to disk when closing the area. When two programs update an
area simultanously, they overwrite each other's changes in the index file.
This leads to a corrupted index file or may even destroy the area altogether.
:p.
Like *.MSG areas, Squish areas only support the standard attributes. Again,
FleetStreet stores all other attributes in a FLAGS kludge. Squish (the tosser
program) does :hp5.not:ehp5. handle this kludge line.
:p.
The most significant bit in the attribute field of each message is used as
"read" attribute.
:p.
The "keep" attribute is not handled by :hp2.SqPack:ehp2.. Use :hp2.FESQ:ehp2.
to pack your Squish areas.

.* @2@ **********************************************************************
:h2.JAM
:p.
The number of active messages in an area is stored in the *.JHR file. This
field is not updated correctly by several programs (IMail 1.75 sets the field
to zero when packing the message; GoldEd 2.50.Beta6 sometimes produces
underflows of this field when deleting messages). Therefore, the contents of
this field can not be used safely. FleetStreet relies on the area index.
:p.
The index file of JAM areas is designed badly. When looking at an index entry
alone you cannot see if a message is deleted or active. There are two ways
to handle this situation&colon.
:ol.
:li.You read the message header in addition to the index entry to find out
whether the message is deleted or not. This is slow and defeats the purpose
of having an index at all. FleetStreet no longer uses this method.
:li.You display all message, even if some of them are marked as "deleted".
:eol.
:p.
To let deleting a message have any effect, FleetStreet overwrites the index
entry altogether. This may not conform to the JAM specifications by 100%, but
this is the only reasonable way of doing it. Once a message is deleted using
FleetStreet, it will never show up again.
:p.
Some message base utilities delete messages by just setting the header to
"deleted", but don't update the index entry. You should then pack the area
afterwards to completely remove deleted messages.

.* @1@ **********************************************************************
:h1.Tosser considerations
:p.
FleetStreet directly supports a number of tossers. This chapter contains
tips for effectively using several tosser together with FleetStreet.

.* @2@ **********************************************************************
:h2.Squish
:p.
The overall cooperation with Squish is problem-free. Here are some additional
hints&colon.
:ul.
:li.Squish doesn't recognize or handle the FLAGS kludge line, i.e.
only the standard attributes have any effect.
:li.When packing messages from *.MSG areas, the read counter is reset to zero,
i.e. the messages appear as "unread" again.
:eul.

.* @2@ **********************************************************************
:h2.Fastecho
:p.
:ul.
:li.Fastecho doesn't support Squish's broadcast features.
:li.Fastecho treats multiple netmail areas differently. In contrast to Squish,
Fastecho can't import or export netmail messages directly to or from secondary
netmail areas. When packing netmail messages from secondary netmail areas, you
have to export them from the areas first and pack them afterwards. The same
command is used as for exporting echomail messages. FleetStreet however treats
all netmail areas equal, i.e. they are not included in the ECHOTOSS.LOG file.
So messages in secondary netmail areas are not found when exporting messages.
:p.
A solution is to export messages in two steps&colon. Export echomail messages
using the ECHOTOSS.LOG file in the first step. In a second step, use a
dummy ECHOTOSS.LOG file to export messages from secondary netmail areas. This
dummy ECHOTOSS.LOG file lists your secondary netmail areas. Finally, pack
the exported messages.
:p.
Example&colon.
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
Version 1.75 of IMail unfortunately has a bug&colon. In JAM areas, the
:hp2.PATH:ehp2. kludge and :hp2.SEEN-BY:ehp2. lines are stored in the message
text instead of the message header (as required by the JAM specification).
As a result, these lines are displayed after the message text in FleetStreet.

.* @1@ **********************************************************************
:h1.Appendix
:p.

.* ************************** Kludge-Lines ***********************************
.* @2@ **********************************************************************
:h2 id=kludges.Kludge lines
:p.The following kludge lines are generated or recognized by FleetStreet&colon.
:parml compact tsize=3 break=all.
:pt.:hp2.FMPT:ehp2.
:pt.:hp2.TOPT:ehp2.
:pt.:hp2.INTL:ehp2.
:pd.These kludges are generated by FleetStreet according to :hp4.FTS-0001 Rev. 15:ehp4.
when the generated message is a NetMail message.
:p.Squish re-generates these kludges when exporting the messages. Support for
these kludges normally would not be necessary.
:p.When reading messages, these kludges are ignored.
:p.

:pt.:hp2.MSGID:ehp2.
:pd.When writing messages this kludge is generated according to :hp4.FTS-0009 Rev. 1:ehp4..
The address component is 4D.
:p.When reading in an EchoMail area, :hp2.MSGID:ehp2. is used for finding out
the address of the sender. When :hp2.MSGID:ehp2. does not contain a FTN address,
the origin line is searched for a valid address.
:p.

:pt.:hp2.REPLY:ehp2.
:pd.When writing messages, the :hp2.MSGID:ehp2. kludge of the original message
is written as :hp2.REPLY:ehp2. kludge.
:p.

:pt.:hp2.PID:ehp2.
:pd.If you activated using :hp2.PID:ehp2. in the setup, the :hp2.PID:ehp2. kludge
is generated and only a short tearline is appended to the message. A long tearline
is appended if :hp2.PID:ehp2. is not activated.
:p.FleetStreet follows the recommendations of :hp4.FSC-0046 Rev. 2:ehp4..
:p.

:pt.:hp2.REPLYTO:ehp2.
:pd.When replying to a NetMail with a :hp2.REPLYTO:ehp2. kludge, the address
and username specified there are used as the addressee of the message.
:p.In EchoMail areas this kludge is ignored. FleetStreet follows the recommendations
of :hp4.FSC-0035 Rev. 1:ehp4..
:p.

:pt.:hp2.REPLYADDR:ehp2.
:pd.When replying to a NetMail with a :hp2.REPLYADDR:ehp2. kludge, the address
specified there is inserted as a "To&colon." line at the beginning of the
new message.
:p.In EchoMail areas this kludge is ignored. FleetStreet follows the recommendations
of :hp4.FSC-0035 Rev. 1:ehp4..
:p.

:pt.:hp2.SPLIT:ehp2.
:pd.When you save a message that is longer than 12 KB, it is split into smaller pieces
according to :hp4.FSC-0047 Rev. 1:ehp4.. There are some deviations&colon.
:ul.
:li.Since the message never was is the message base as a whole, the
message number in the :hp2.SPLIT:ehp2. kludge depends on the current implementation
(currently it's the number of the first part of the message).
:li.:hp4.FSC-0047:ehp4. recommends to strip :hp2.MSGID:ehp2. in parts 2..n
so that dupe checkers don't throw away these parts. FleetStreet however generates
a new :hp2.MSGID:ehp2. for each part which has the same effect.
:li.The part number in the :hp2.SPLIT:ehp2. kludge only has two ciphers, so a
maximum of 99 parts is possible. The message length is therefore limited to 1188 KB.
:li.When changing a message the new message can only have a maximum length of
15 KB. Splitting the message again would make it impossible for the
addressee to join the parts automatically, because the numbering of the parts
would be corrupted.
:li.The part numbers are added to the end of the subject line instead of to
the beginning. Therefore the messages appear in the correct order in the thread list.
:eul.
:p.
:pt.:hp2.APPEND:ehp2.
:pt.:hp2.REALADDRESS:ehp2.
:pd.These kludges are preserved when replying.
:p.
:pt.:hp2.CHARSET/CHRS:ehp2.
:pd.FleetStreet supports these kludges up to Level 2 according to :hp4.FSC-0054 Rev. 4:ehp4..
When writing a message, FleetStreet always outputs :hp2.IBMPC 2:ehp2..
:p.
:pt.:hp2.ACUPDATE:ehp2.
:pd.This kludge line is used by Squish 1.10 to modify or delete messages on
remote systems. See the Squish 1.10 documentation for further information.
:p.
:pt.:hp2.AREA:ehp2.
:pd.When this kludge line is found in a message and the area is known to FleetStreet,
a reply to this message is automatically placed in the area specified in the kludge
line.
:p.
:pt.:hp2.FLAGS:ehp2.
:pd.The :hp2.FLAGS:ehp2. kludge holds message attributes that aren't supported
directly by the message base format. The format of this kludge is defined in
:hp4.FSC-0053:ehp4..
:p.
:pt.:hp2.FWDFROM, FWDTO, FWDSUBJ, FWDORIG, FWDDEST, FWDAREA, FWDMSGID:ehp2.
:pd.These kludges are generated when forwarding a message. They contain header
information of the original message. When replying to such a message,
FleetStreet uses these items for the reply.
:eparml.

.* @2@ **********************************************************************
:h2 id=cmdlin.Program parameters
:p.
FleetStreet recognizes the following command line parameters&colon.
:parml.
:pt.:hp2.-C<path>:ehp2.
:pd.The INI files are not read from and written to the current directory, but
the path specified is used for the INI files.
:p.:hp2.Example&colon.:ehp2.
:p.FLTSTRT.EXE -Cd&colon.\myinis
:eparml.


.* @2@ **********************************************************************
:h2.Return codes
:p.
FLTSTRT.EXE generates the following return codes&colon.
:parml break=none.
:pt.:hp2.0:ehp2.
:pd.No new messages
:pt.:hp2.1:ehp2.
:pd.New netmail messages entered
:pt.:hp2.2:ehp2.
:pd.New echomail messages entered
:pt.:hp2.4:ehp2.
:pd.New local messages entered
:pt.:hp2.255:ehp2.
:pd.Fatal error
:eparml.
:p.
A combination of 1, 2 and 4 means that messages have been entered in these
area types, e.g. 5 means that there are new messages in netmail areas and
local areas.


.* @2@ **********************************************************************
:h2.Reporting errors
:p.
FleetStreet is not completely bug-free (like any other software program
on earth). I encourage you to report all errors. I will then do my best to
fix the problem.
:p.
Some errors may not be reproducible by me immediately. The following
questions may be relevant for fixing the error&colon.
:ul.
:li.Can you reproduce the error?
:li.Does the error occur at first try or do you have to try several times
to reproduce it?
:li.Which functions did you use when the error occured?
:li.Did you invoke the function with the keyboard, menu or toolbar?
Does it make a difference which one you use?
:li.Is there an error message? What does it say?
:li.Did you configure FleetStreet correctly?
:li.What's the exact result of the error?
:eul.
:p.
When FleetStreet crashes because of an error, it creates the file
:hp2.FLTSTRT.DMP:ehp2. in the current directory. With this file the error
can usually be found quite easily.


.* @2@ **********************************************************************
:h2.Programs used for developing FleetStreet
:p.
FleetStreet was developed and tested using the following programs&colon.
:ul.
:li.Compiler&colon. IBM Visual Age C++ 3.0 (C mode)
:li.Debugger&colon. IBM C/C++ Debugger 3.0 (IPMD)
:li.Development environment&colon. IBM Workframe 3.0
:li.Linker&colon. ILink
:li.Editor&colon. Enhanced Editor, Tiny Editor, LPEX
:li.Tools&colon. IBM OS/2 Toolkit 3.0; GNU Grep; PMTree; ExeMap;
Hexdump; PMSpy; PM Camera
:eul.

.* @2@ **********************************************************************
:h2 id=support.Support
:p.You can reach Michael Hohner at one of the following EMail addresses&colon.
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
There are two Fido echos available at 2&colon.2490/1050, FLEETBETA and FLEETSTREET.
FLEETBETA
is the german language echo, FLEETSTREET is the international echo (english
language). Please write a netmail to Robert Gloeckner (2&colon.2490/1050)
if you want to carry the echo. You get a list of other nodes connected to
this echo to make distribution cheaper. This echo may be routed without
restrictions, but please inform us about connected nodes...

:euserdoc.

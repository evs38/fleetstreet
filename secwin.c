/*---------------------------------------------------------------------------+
 | Titel: SECWIN.C                                                           |
 +-----------------------------------------+---------------------------------+
 | Erstellt von: Michael Hohner            | Am: 05.01.1996                  |
 +-----------------------------------------+---------------------------------+
 | System: OS/2 2.x/3.x PM                                                   |
 +---------------------------------------------------------------------------+
 | Beschreibung:                                                             |
 |                                                                           |
 |     Registrieren der Fensterklassen                                       |
 |                                                                           |
 |                                                                           |
 +---------------------------------------------------------------------------+
 | Bemerkungen:                                                              |
 +---------------------------------------------------------------------------*/

/*----------------------------- Header-Dateien ------------------------------*/
#pragma strings(readonly)

#define INCL_PM
#include <os2.h>
#include "main.h"
#include "msgheader.h"

#include "controls\msgviewer.h"
#include "controls\statline.h"
#include "controls\clrsel.h"
#include "controls\mlist.h"
#include "controls\toolbar.h"
#include "controls\fontdisp.h"
#include "controls\editwin.h"
#include "controls\listbox.h"
#include "controls\attrselect.h"

/*--------------------------------- Defines ---------------------------------*/

/*---------------------------------- Typen ----------------------------------*/

/*---------------------------- Globale Variablen ----------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

/*----------------------- interne Funktionsprototypen -----------------------*/

/*-----------------------------------------------------------------------------
 | Funktionsname: RegisterSecondaryWindows
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Beschreibung: Registriert alle Fensterklassen
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Parameter: anchor: Anchor-Block
 |
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | RÅckgabewerte: TRUE   Erfolg
 |                FALSE  Fehler
 |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 | Sonstiges: -
 |
 +---------------------------------------------------------------------------*/

BOOL RegisterSecondaryWindows(HAB anchor)
{
   if (RegisterEditWin(anchor) &&
       RegisterMsgViewer(anchor) &&
       RegisterStatusLineClass(anchor) &&
       RegisterColorSelect(anchor) &&
       RegisterMessageList(anchor) &&
       RegisterToolbar(anchor) &&
       RegisterFontDisplay(anchor) &&
       RegisterAttribSelect(anchor) &&
       RegisterListBox(anchor))
      return TRUE;
   else
      return FALSE;
}

/*-------------------------------- Modulende --------------------------------*/

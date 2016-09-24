/* LISTBOX.H */
/* Listbox control */

/* Window Class */
#define WC_EXTLISTBOX  "ExtListBox"

/* Control IDs */
#define MLID_VSCROLL          200

/* Timer ID */
#define TID_AUTOSCROLL        100

/* functions */
BOOL EXPENTRY RegisterListBox(HAB hab);

#define LIT_ALL              (-5)

/* Structures */

typedef struct    /* mp1 von LM_SEARCHSTRING */
{
   USHORT   usCmd;
   LONG     lItemStart;
} LBSEARCH, *PLBSEARCH;

/* Ende LISTBOX.H */

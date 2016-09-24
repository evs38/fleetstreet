/* CLRSEL.H */

#define WC_COLORSELECT  "ColorSelect"

/* Window-IDs */
#define CSEL_ID_WHEEL     1
#define CSEL_ID_SLID      2

/* Messages */
#define CLSM_QUERYRGB    (WM_USER+1)
#define CLSM_SETRGB      (WM_USER+2)

/* Notification codes */
#define CLSN_RGB         1


/* Funktionen */
BOOL EXPENTRY RegisterColorSelect(HAB hab);


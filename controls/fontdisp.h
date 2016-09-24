/* FONTDISP.H */

/* Window Class */

#define WC_FONTDISPLAY  "FontDisplay"


/* Styles */

#define FDS_NO_BORDER   1UL


/* Messages */

#define FDM_SETFONT   (WM_USER+1)
#define FDM_QUERYFONT (WM_USER+2)


/* WM_CONTROL notification codes */

#define FDN_FONTCHANGED    1
#define FDN_OPEN           2
#define FDN_CONTEXTMENU    3


/* Functions */

BOOL RegisterFontDisplay(HAB hab);

/* Ende FONTDISP.H */

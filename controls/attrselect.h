/* ATTRSELECT.H */

/*--------------------------- Fensterklasse       ---------------------------*/

#define WC_ATTRIBSELECT "AttribSelect"

/*--------------------------- Messages            ---------------------------*/

#define ATTSM_SETATTRIB     (WM_USER+1)
#define ATTSM_QUERYATTRIB   (WM_USER+2)


/*--------------------------- Control-Codes       ---------------------------*/

#define ATTSN_ATTRIBCHANGED   1

#define ATTSID_VALUE    100


/*--------------------------- Funktionsprototypen ---------------------------*/

BOOL RegisterAttribSelect(HAB hab);

/* Ende ATTRSELECT.H */

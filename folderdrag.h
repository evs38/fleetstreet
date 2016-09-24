/* FOLDERDRAG.H */

/* Definitionen fÅr Folder-Drag-Drop */

#define FOLDERDRAGTYPE "FleetStreet Folder"
#define DRMFLEET       "DRM_FLEET"
#define DRFFOLDER      "DRF_FOLDER"
#define FOLDERRMF      "<" DRMFLEET "," DRFFOLDER ">"
#define FOLDERRMFDEL   "<" DRMFLEET "," DRFFOLDER ">,<DRM_DISCARD,DRF_UNKNOWN>"

/****************************************************************************/
/* Inhalt von DRAGITEM                                                      */
/*                                                                          */
/*  HWND      hwndItem;           Dialog-Window der Area-Liste              */
/*  ULONG     ulItemID;           Pointer auf FOLDERRECORD                  */
/*  HSTR      hstrType;           s.o.                                      */
/*  HSTR      hstrRMF;            s.o.                                      */
/*  HSTR      hstrContainerName;  -                                         */
/*  HSTR      hstrSourceName;     Name                                      */
/*  HSTR      hstrTargetName;     Name                                      */
/*  SHORT     cxOffset;                                                     */
/*  SHORT     cyOffset;                                                     */
/*  USHORT    fsControl;          DC_CONTAINER                              */
/*  USHORT    fsSupportedOps;     Moveable                                  */
/****************************************************************************/


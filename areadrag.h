/* AREADRAG.H */

/* Definitionen fÅr Area-Drag-Drop */

#define AREADRAGTYPE "FleetStreet Area"
#define DRMFLEET     "DRM_FLEET"
#define DRFAREA      "DRF_AREA"
#define AREARMF      "<" DRMFLEET "," DRFAREA ">"
#define AREARMFDEL   "<" DRMFLEET "," DRFAREA ">,<DRM_DISCARD,DRF_UNKNOWN>"

/****************************************************************************/
/* Inhalt von DRAGITEM                                                      */
/*                                                                          */
/*  HWND      hwndItem;           Dialog-Window der Area-Liste              */
/*  ULONG     ulItemID;           Pointer auf AREARECORD                    */
/*  HSTR      hstrType;           s.o.                                      */
/*  HSTR      hstrRMF;            s.o.                                      */
/*  HSTR      hstrContainerName;  -                                         */
/*  HSTR      hstrSourceName;     Area-Tag                                  */
/*  HSTR      hstrTargetName;     Area-Tag                                  */
/*  SHORT     cxOffset;                                                     */
/*  SHORT     cyOffset;                                                     */
/*  USHORT    fsControl;          DC_CONTAINER                              */
/*  USHORT    fsSupportedOps;     Moveable                                  */
/****************************************************************************/


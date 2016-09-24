/* MESSAGEDRAG.H */
/* Definitionen fÅr Drag-Drop einer Message innerhalb von FleetStreet */

#define MSGDRAGTYPE   "FleetStreet Message"
#define MSGDRM        "DRM_FLEET"
#define MSGDRF        "DRF_MESSAGE"
#define MSGRMF        "<" MSGDRM "," MSGDRF ">,<DRM_PRINT,DRF_UNKNOWN>,<DRM_DISCARD,DRF_UNKNOWN>"

/****************************************************************************/
/* Inhalt von DRAGITEM                                                      */
/*                                                                          */ 
/*  HWND      hwndItem;           Window-Handle des Source-Windows          */ 
/*  ULONG     ulItemID;           UMSGID der Message                        */ 
/*  HSTR      hstrType;           s.o.                                      */ 
/*  HSTR      hstrRMF;            s.o.                                      */ 
/*  HSTR      hstrContainerName;  Area-Tag der Message                      */ 
/*  HSTR      hstrSourceName;     leer                                      */ 
/*  HSTR      hstrTargetName;     leer                                      */ 
/*  SHORT     cxOffset;                                                     */ 
/*  SHORT     cyOffset;                                                     */ 
/*  USHORT    fsControl;          -                                         */ 
/*  USHORT    fsSupportedOps;     DO_COPYABLE | DO_MOVEABLE                 */ 
/****************************************************************************/


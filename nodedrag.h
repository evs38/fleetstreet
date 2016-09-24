/* NODEDRAG.H */

/* Definitionen fÅr Node-Drag-Drop */

#define NODEDRAGTYPE "FleetStreet Node"
#define DRMFLEET     "DRM_FLEET"
#define DRFNODE      "DRF_NODE"
#define NODERMF      "<" DRMFLEET "," DRFNODE ">"

/****************************************************************************/
/* Inhalt von DRAGITEM                                                      */
/*                                                                          */ 
/*  HWND      hwndItem;           Dialog-Window des NL-Browsers             */ 
/*  ULONG     ulItemID;           konstant 0                                */ 
/*  HSTR      hstrType;           s.o.                                      */ 
/*  HSTR      hstrRMF;            s.o.                                      */ 
/*  HSTR      hstrContainerName;  Name des Domains, aus dem der Node stammt */ 
/*  HSTR      hstrSourceName;     Node im Format "z:n/n.p Name"             */ 
/*  HSTR      hstrTargetName;     leer                                      */ 
/*  SHORT     cxOffset;                                                     */ 
/*  SHORT     cyOffset;                                                     */ 
/*  USHORT    fsControl;          -                                         */ 
/*  USHORT    fsSupportedOps;     Copyable                                  */ 
/****************************************************************************/
 


/* AREAMAN.H */

/* Typen */

typedef struct
{
   PAREADEFLIST  pFirstArea;    /* erste Area  */
   PAREADEFLIST  pLastArea;     /* letzte Area */
   ULONG         ulNumAreas;    /* Anzahl Areas */
   BOOL          bDirty;        /* Dirty-Flag */
} AREALIST, *PAREALIST;

typedef BOOL (*TRAVERSEFUNC)(PAREADEFLIST, PVOID);

#define TRAVERSE_DIRTYONLY      0x01UL

/* Prototypen */

PAREADEFLIST AM_FindArea(PAREALIST pList, const char *pchAreaTag);
PAREADEFLIST AM_AddArea(PAREALIST pList, AREADEFOPT *pAreaDef, ULONG ulOptions);
BOOL AM_DeleteArea(PAREALIST pList, char *pchAreaTag);
BOOL AM_DeleteAreaDirect(PAREALIST pList, PAREADEFLIST pDel);
BOOL AM_DeleteAllAreas(PAREALIST pList);
PAREADEFLIST AM_MergeAreas(PAREALIST pList1, PAREALIST pList2);
BOOL AM_TraverseAreas(PAREALIST pAreaList, ULONG ulOptions, TRAVERSEFUNC CallBack, PVOID pParam);

/* Optionen f. Add */

#define ADDAREA_HEAD        0UL
#define ADDAREA_TAIL        1UL
#define ADDAREA_MARKDIRTY   8UL
#define ADDAREA_UNIQUE     16UL   /* n. anhaengen, wenn schon vorhanden */

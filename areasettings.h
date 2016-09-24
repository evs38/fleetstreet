/* AREASETTINGS.H */

typedef struct { USHORT cb;
                 AREADEFLIST *pAreaDef;
                 BOOL bMultiple;
               } AREAPAR, *PAREAPAR;

MRESULT EXPENTRY AreaSettingsProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);

#define IDST_TAB_AS_GENERAL         450
#define IDST_TAB_AS_MSGBASE         451
#define IDST_TAB_AS_ATTRIB          452


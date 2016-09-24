/* AREALISTSETTINGS.H */

typedef struct _ALSETTINGSPARAM {
                 USHORT cb;
                 PAREALISTOPTIONS pAreaListOptions;
              } ALSETTINGSPARAM, *PALSETTINGSPARAM;

MRESULT EXPENTRY AreaListSettingsProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);

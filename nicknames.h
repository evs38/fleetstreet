/* NICKNAMES.H */


typedef struct {
          USHORT cb;
          NICKNAME entry;
          PNICKNAME pEntry;  /* Zeiger beim Aendern */
          BOOL bNewEntry;
          BOOL bDirectAdd;
        } NICKPAR;

/*--------------------------- Funktionsprototypen ---------------------------*/
MRESULT EXPENTRY AdrBookProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
void AddToNick(HWND hwndOwner, MSGHEADER *pHeader);
BOOL LookupNickname(HWND hwndClient, char *pchNick, PNICKNAMELIST Liste);


/* Ende NICKNAMES.H */

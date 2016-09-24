/* FLTUTIL.H */

BOOL AreaInAreaSet(const char *pchAreaSet, const char *pchAreaTag);
char *stristr(const char *pHaystack, const char *pNeedle);
char *StripWhitespace(char *string);
char *RemoveBackslash(char *pchPath);
char *StripRe(char *pchString);

/* Container-Funktionen */

ULONG CollectRecordPointers(HWND hwndCnr, PRECORDCORE **pppDest, PRECORDCORE pPopupRecord);
void ApplySourceEmphasis(HWND hwndCnr, PRECORDCORE pPopupRecord);
void RemoveSourceEmphasis(HWND hwndCnr);
void SelectAllRecords(HWND hwndCnr);
void DeselectAllRecords(HWND hwndCnr);


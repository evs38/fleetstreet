/* CCMANAGE.H */

/*---------------------------------- Typen ----------------------------------*/

/*--------------------------- Funktionsprototypen ---------------------------*/

PCCLIST QueryCCList(PCCANCHOR pAnchor, ULONG ulListID);
int HaveCCListName(void *pAnchor, char *pchName);
PCCLIST AddCCList(PCCANCHOR pAnchor, PCHAR pchListName);
BOOL DeleteCCList(PCCANCHOR pAnchor, PCCLIST pList);
PCCENTRY AddCCEntry(PCCANCHOR pAnchor, PCCLIST pList, PCCENTRY pEntry);
BOOL DeleteCCEntry(PCCANCHOR pAnchor, PCCLIST pList, PCCENTRY pEntry);

/* JAMINDEX.H */

PULONG JAMLoadIndex(JAMAPIRECptr apirec, PULONG pulMaxMsgs);
int JAMFreeIndex(PAREADEFLIST pAreaDef);
ULONG JAMFindUid(PAREADEFLIST pAreaDef, ULONG ulMsgID, BOOL exact);
ULONG JAMFindNum(PAREADEFLIST pAreaDef, int msgnum);
void JAMAddToIdx(PAREADEFLIST pAreaDef, ULONG ulMsgNum);
void RemoveFromIdx(PAREADEFLIST pAreaDef, int num);

/* Ende JAMINDEX.H */

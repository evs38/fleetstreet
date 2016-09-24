/* FTSAPI.H */

/*--------------------------- Funktionsprototypen ---------------------------*/

ULONG  FTS_QueryAttribMask(void);

PVOID  FTS_OpenArea(PAREADEFLIST pAreaDef, PCHAR pchPathName);
USHORT FTS_CloseArea(PAREADEFLIST pAreaDef);

ULONG  FTS_ReadLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset);
USHORT FTS_WriteLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset);

USHORT FTS_ReadHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum);
USHORT FTS_WriteHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum, PDRIVEREMAP pdriveremap, BOOL bReplyAndAttr);
USHORT FTS_LinkMessages(PAREADEFLIST pAreaDef, ULONG ulReplyID, ULONG ulOrigID, PDRIVEREMAP pdriveremap);
int    FTS_UnlinkMessage(PAREADEFLIST pAreaDef, int msgnum, PDRIVEREMAP pdriveremap);
USHORT FTS_MarkRead(PAREADEFLIST pAreaDef, int msgnum, BOOL bPersonal, PDRIVEREMAP pdriveremap);

USHORT FTS_ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);
USHORT FTS_ChangeMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);
USHORT FTS_AddMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, PDRIVEREMAP pDriveRemap);
USHORT FTS_KillMessage(PAREADEFLIST pAreaDef, int msgnum);

ULONG  FTS_UidToMsgn(PAREADEFLIST pAreaDef, ULONG msgID, BOOL exact);
ULONG  FTS_MsgnToUid(PAREADEFLIST pAreaDef, int msgnum);

int    FTS_RenumberArea(PAREADEFLIST pAreaDef, HWND hwndProgress, PDRIVEREMAP pDriveRemap);

/* Ende FTSAPI.H */

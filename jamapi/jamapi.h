/* JAMAPI.H */

/*--------------------------- Funktionsprototypen ---------------------------*/

ULONG  JAM_QueryAttribMask(void);

USHORT JAM_OpenApi(void);
USHORT JAM_CloseApi(void);

PVOID  JAM_OpenArea(PAREADEFLIST pAreaDef, PCHAR pchPathName);
USHORT JAM_CloseArea(PAREADEFLIST pAreaDef);

ULONG  JAM_ReadLastread(PAREADEFLIST pAreaDef, LONG lOffset);
USHORT JAM_WriteLastread(PAREADEFLIST pAreaDef, LONG lOffset);

USHORT JAM_ReadHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum);
USHORT JAM_LinkMessages(PAREADEFLIST pAreaDef, ULONG ulReplyID, ULONG ulOrigID);
USHORT JAM_UnlinkMessage(PAREADEFLIST pAreaDef, int msgnum);
USHORT JAM_MarkRead(PAREADEFLIST pAreaDef, int msgnum, BOOL bPersonal);

USHORT JAM_ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);
USHORT JAM_ChangeMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);
USHORT JAM_AddMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage);
USHORT JAM_KillMessage(PAREADEFLIST pAreaDef, int msgnum);

ULONG  JAM_UidToMsgn(PAREADEFLIST pAreaDef, ULONG msgID, BOOL exact);
ULONG  JAM_MsgnToUid(PAREADEFLIST pAreaDef, int msgnum);

/* Ende von JAMAPI.H */

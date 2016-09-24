/* SQUISHAPI.H */

/*--------------------------- Funktionsprototypen ---------------------------*/

ULONG  SQ_QueryAttribMask(void);

USHORT SQ_OpenApi(PCHAR mainaddress);
USHORT SQ_CloseApi(void);

PVOID  SQ_OpenArea(PAREADEFLIST pAreaDef, PCHAR pchPathName);
USHORT SQ_CloseArea(PAREADEFLIST pAreaDef);

ULONG  SQ_ReadLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset);
USHORT SQ_WriteLastread(PAREADEFLIST pAreaDef, PCHAR pchPathName, LONG lOffset);

USHORT SQ_ReadHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum);
USHORT SQ_WriteHeader(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, int msgnum);
USHORT SQ_LinkMessages(PAREADEFLIST pAreaDef, ULONG ulReplyID, ULONG ulOrigID);
int    SQ_UnlinkMessage(PAREADEFLIST pAreaDef, int msgnum);
USHORT SQ_MarkRead(PAREADEFLIST pAreaDef, int msgnum, BOOL bPersonal);

USHORT SQ_ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);
USHORT SQ_ChangeMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);
USHORT SQ_AddMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage);
USHORT SQ_KillMessage(PAREADEFLIST pAreaDef, int msgnum);

ULONG  SQ_UidToMsgn(PAREADEFLIST pAreaDef, ULONG msgID, BOOL exact);
ULONG  SQ_MsgnToUid(PAREADEFLIST pAreaDef, int msgnum);

int    SQ_ReadSquishParams(PAREADEFLIST pAreaDef, PSQUISHPARAMS pSquishParams, PDRIVEREMAP pDriveRemap);
int    SQ_WriteSquishParams(PAREADEFLIST pAreaDef, PSQUISHPARAMS pSquishParams, PDRIVEREMAP pDriveRemap);

/* Ende SQUISHAPI.H */

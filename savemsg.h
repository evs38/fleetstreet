/* SAVEMSG.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
int ChangeMessage(HWND hwndClient, FTNMESSAGE *OldMessage, MSGHEADER *OldHeader, char *pchAreaTag, BOOL bChangeKludges);
BOOL SaveMessage(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *Header, PCHAR pAreaTag);
BOOL SaveCrosspostMessage(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *Header,
                          PCHAR pAreaTag, char **pchAreaList);
BOOL SaveCCMessage(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *Header, PCHAR pAreaTag,
                   PCCLIST pCCList);
int GetMsgContents(HWND hwndClient, FTNMESSAGE *MsgInfo, MSGHEADER *MsgHeader,
                   BOOL isEcho);
void SaveErrorMessage(HWND hwndClient, ULONG ulMsg);
MRESULT SendAddMessage(AREADEFLIST *pAreaDef, ULONG ulMsgID, PMSGHEADER pHeader);
MRESULT SendChangeMessage(AREADEFLIST *pAreaDef, ULONG ulMsgID, PMSGHEADER pHeader);
MRESULT SendKillMessage(AREADEFLIST *pAreaDef, ULONG ulMsgID, PMSGHEADER pHeader);

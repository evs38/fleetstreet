/* HandleMsg.h */

/* -------------------------- Fehlerdefinition -------------------------- */
enum MSG_Error { OK,               ERROR,              OUT_OF_MEMORY,

                 NO_AREA_OPEN,     AREA_NOT_FOUND,     AREA_OPEN_ERROR,
                 AREA_CLOSE_ERROR,

                 NO_MESSAGE,       FIRST_MESSAGE,      LAST_MESSAGE,
                 MSG_OPEN_ERROR,   MSG_READ_ERROR,     MSG_WRITE_ERROR,
                 MSG_DELETE_ERROR,

                 WRONG_HEADER };

typedef MRESULT (*MsgCallback)(AREADEFLIST *, ULONG, PMSGHEADER);

typedef struct
{
   char  chFlag[5];
   ULONG ulAttrib;
} ATTRIBMAP;

/* -------------------------- Funktionsprototypen ----------------------- */
int MSG_OpenApi(PCHAR mainaddress);
int MSG_CloseApi(PAREALIST arealist, PDRIVEREMAP pDriveRemap);

BOOL MSG_IsApiOpen(void);

int MSG_OpenArea(PAREALIST arealist, PCHAR tag, LONG lOffset,
                           PDRIVEREMAP pDriveRemap);
int MSG_CloseArea(PAREALIST arealist, PCHAR tag, BOOL Write_LR, LONG lOffset, PDRIVEREMAP pDriveRemap);

int MSG_LockArea(PCHAR tag, PAREALIST arealist);
int MSG_UnlockArea(PCHAR tag, PAREALIST arealist);

int MSG_ReadAct(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag);
int MSG_ReadNum(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag, int number);
int MSG_ReadNext(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag);
int MSG_ReadPrev(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag);

int MSG_ReadHeader(PMSGHEADER pHeader, PAREALIST arealist, PCHAR tag, int number);
int MSG_LinkMessages(PAREALIST arealist, PCHAR tag, ULONG ulReplyID, ULONG ulOrigID, PDRIVEREMAP pdriveremap);

int MSG_AddMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                   PAREALIST arealist, PCHAR tag, USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                   PDRIVEREMAP pDriveRemap, LONG lSplitLen,
                   TEMPLATELIST *ptemplatelist, ULONG ulOptions, MsgCallback AddCallback);

#define ADDOPT_MATCHADDRESS  0x01UL

int MSG_MatchAddress(FTNADDRESS *pDestAddress, USERDATAOPT *userdaten, FTNADDRESS *pCurrentAddr);
int MSG_ChangeMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                      PAREALIST arealist, PCHAR tag,
                      USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                      BOOL bChangeKludges, TEMPLATELIST *ptemplatelist, MsgCallback ChangeCallback);

int MSG_NewMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                   PAREALIST arealist, PCHAR tag,
                   char *pchCurrentName, char *pchCurrentAddress, LONG *iptInitialPos);
int MSG_NewMessageStep2(TEMPLATELIST *ptemplatelist, PAREALIST arealist, PCHAR tag,
                        PFTNMESSAGE pMessage, PMSGHEADER pHeader, LONG *iptInitialPos);

int MSG_CopyMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist,
                    PCHAR dest_tag, PDRIVEREMAP pDriveRemap, USERDATAOPT *userdaten,
                    GENERALOPT *generaloptions, MsgCallback CopyCallback, ULONG ulOptions);
int MSG_MoveMessage(PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist,
                    PCHAR tag, PCHAR dest_tag, ULONG msgnum, PDRIVEREMAP pDriveRemap,
                    USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                    MsgCallback CopyCallback, MsgCallback KillCallback, ULONG ulOptions);

#define COPYMOVE_RESEND     0x01UL

int MSG_ForwardMessage(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                       PAREALIST arealist, PCHAR tag, PCHAR dest_tag, char *pchCurrentName,
                       BOOL bGenFwdSubj);
int MSG_ForwardMessageStep2(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                            PAREALIST arealist, PCHAR dest_tag,
                            char *pchCurrentName, LONG *iptInitialPos);

int MSG_KillMessage(PAREALIST arealist, PCHAR tag, int msgnum, PDRIVEREMAP pdriveremap,
                    MsgCallback KillCallback);

#define QUOTE_TEXT      0x02UL
#define QUOTE_NOJOIN    0x04UL
#define QUOTE_STRIPRE   0x08UL

#define QUOTE_FROM      0
#define QUOTE_TO        1
#define QUOTE_ORIG      2

int MSG_QuoteMessage(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader, PAREALIST arealist,
                     PCHAR tag, PCHAR desttag, ULONG ulFlags, ULONG ulDest,
                     char *pchCurrentName, char *pchCurrentAddress, LONG *iptInitialPos);

int MSG_QuoteMessageStep2(TEMPLATELIST *ptemplatelist, PFTNMESSAGE pMessage, PMSGHEADER pHeader,
                          PAREALIST arealist, PCHAR tag, PCHAR desttag,
                          char *pchCurrentName, LONG *iptInitialPos);

int MSG_RequestFiles(PAREALIST arealist, PCHAR tag,
                     PCHAR address, PCHAR name, PREQUESTLIST pFiles, ULONG ulAttrib,
                     char *pchCurrentName, char *pchCurrentAddress,
                     USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                     LONG lOffset, PDRIVEREMAP pDriveRemap, TEMPLATELIST *ptemplatelist,
                     MsgCallback ReqCallback);
int MSG_RequestDirect(PCHAR pchReqAddr, PCHAR pchCurrentAddress,
                      PREQUESTLIST pFiles, OUTBOUND *pOutbound,
                      PDRIVEREMAP pDriveRemap, ULONG ulAttrib);

int MSG_BroadcastDelete(PAREALIST arealist, PCHAR AreaTag,
                        MSGHEADER *pHeader, FTNMESSAGE *pMessage,
                        USERDATAOPT *userdaten, GENERALOPT *generaloptions,
                        char *pchCurrentName, char *pchCurrentAddress,
                        PDRIVEREMAP pDriveRemap, TEMPLATELIST *ptemplatelist,
                        MsgCallback BCDCallback);
int MSG_BroadcastModify(PMSGHEADER pHeader, PFTNMESSAGE pMessage);

int MSG_UidToMsgn(PAREALIST arealist, PCHAR AreaTag, ULONG msgID, BOOL exact);
ULONG MSG_MsgnToUid(PAREALIST arealist, PCHAR AreaTag, int msgn);

int MSG_MarkRead(PAREALIST arealist, PCHAR tag, int msgnum, char *pchName, PDRIVEREMAP pdriveremap);

int MSG_ReadSquishParams(PSQUISHPARAMS pSquishParams, PAREALIST arealist, PCHAR tag, PDRIVEREMAP pDriveRemap);
int MSG_WriteSquishParams(PSQUISHPARAMS pSquishParams, PAREALIST arealist, PCHAR tag, PDRIVEREMAP pDriveRemap);

int MSG_RenumberArea(PAREALIST arealist, PCHAR tag, HWND hwndProgress,
                     LONG lOffset, PDRIVEREMAP pDriveRemap);

ULONG MSG_QueryHomeMsg(PAREALIST arealist, PCHAR tag);
void MSG_ClearMessage(PMSGHEADER pHeader, PFTNMESSAGE pMessage);
char *MSG_AttribToText(ULONG ulAttrib, char *pchBuffer);
int MSG_QueryAttribCaps(PAREALIST arealist, PCHAR tag, PULONG pulAttribMask);

char *    StampToString(PCHAR buffer, TIMESTAMP *timestamp);

int MSG_RemapDrive(char *pchPathName, PDRIVEREMAP pRemapOptions);
int MSG_RemapArea(char *pchBuffer, AREADEFLIST *pAreaDef, PDRIVEREMAP pDriveRemap);
PMSGTEMPLATE M_FindTemplate(TEMPLATELIST *ptemplatelist, PAREALIST arealist, PCHAR tag);
void MSG_SetCPInfo(ULONG ulCP);

/* exportierte Template-Funktionen */
char * TplCCopy(TEMPLATELIST *ptemplatelist, char *pchBuffer, CCLIST *pCCList, PCCENTRY pDestEntry, PAREALIST arealist, PCHAR tag);
char * TplXPost(TEMPLATELIST *ptemplatelist, char *pchBuffer, char *pchAreaList, PAREALIST arealist, PCHAR tag);
void TplSetIntl(INTLSETTING *pIntl);

void FlagsToAttrib(PFTNMESSAGE pMessage, PMSGHEADER pHeader, ATTRIBMAP *pMap);
void AttribToFlags(PFTNMESSAGE pMessage, PMSGHEADER pHeader, ATTRIBMAP *pMap);

extern PCHAR months[];

/* Ende HANDLEMSG.H */

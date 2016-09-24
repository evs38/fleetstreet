/* COMMON.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
void Xmsg2Msgheader(PXMSG pXmsg, PMSGHEADER pHeader);
void Msgheader2Xmsg(PXMSG pXmsg, PMSGHEADER pHeader);

void ExtractKludges(PCHAR pchMessageText, PCHAR *pchCtlText, PULONG ctllen);
void QueryOrigin(PCHAR pchMessageText, ULONG textlen, PCHAR ctltext, PXMSG pXmsg);

USHORT WriteMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);
USHORT ReadMessage(PAREADEFLIST pAreaDef, PMSGHEADER pHeader, PFTNMESSAGE pMessage, int msgnum);

int WriteReadFlag(HMSG hmsg, ULONG ulAttrib);
BOOL QueryReadFlag(HMSG hmsg);

/* Attribute */

#define FLEET_READ   0x80000000UL
#define FLEET_KEEP   0x40000000UL

/* Ende von COMMON.H */

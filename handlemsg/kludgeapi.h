/* KLUDGEAPI.H */

PKLUDGE MSG_SetKludge(PFTNMESSAGE pMessage, ULONG ulKludgeType,
                      char *pchKludgeText, ULONG ulFlags);
PKLUDGE MSG_SetKludgeVar(PFTNMESSAGE pMessage, ULONG ulKludgeType,
                         ULONG ulFlags, char *pchFormat, ...);
PKLUDGE MSG_SetKludgeFromBuffer(PFTNMESSAGE pMessage, char *pchBuffer, ULONG ulFlags);

#define SETKLUDGE_UNIQUE   0UL
#define SETKLUDGE_MULTIPLE 1UL

PKLUDGE MSG_FindKludge(PFTNMESSAGE pMessage, ULONG ulKludgeType, PKLUDGE pFirst);
int MSG_RemoveKludge(PFTNMESSAGE pMessage, ULONG ulKludgeType);

#define KLUDGE_ALL    0xffffffffUL

const char *MSG_QueryKludgeName(ULONG ulKludgeType);
ULONG MSG_CalcKludgeBufferSize(PFTNMESSAGE pMessage);
char *MSG_KludgesToBuffer(PFTNMESSAGE pMessage, char *pchBuffer);
int MSG_BufferToKludges(PFTNMESSAGE pMessage, char *pchBuffer);

/* Ende von KLUDGEAPI.H */

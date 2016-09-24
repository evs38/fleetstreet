/* HANDLETEMPLATE.H */

/*--------------------------- Funktionsprototypen ---------------------------*/
char *TplHeader(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchToName);
char *TplFooter(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchUser);
char *TplReply(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchFromName, char *pchToName, char *pchNewTo,
               char *pchArea, TIMESTAMP *DateWritten, FTNADDRESS *pFromAd, FTNADDRESS *pToAd,
               char *pchSubject, char *pchAreaDes);
char *TplReplyOther(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchOrigArea, char *pchAreaDes);
char *TplForward(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchOrigArea, char *pchUser, char *pchFromName,
                 char *pchToName, TIMESTAMP *DateWritten, FTNADDRESS *pFromAd, FTNADDRESS *pToAd,
                 char *pchSubject, char *pchAreaDes, FTNADDRESS *pMyAddr);
char *TplForwardFooter(MSGTEMPLATE *msgtemplate, char *pchBuffer, char *pchOrigArea, char *pchUser, char *pchFromName,
                 char *pchToName);

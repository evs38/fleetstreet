/* REXXEXEC.H */

/*--------------------------- Funktionsprototypen ---------------------------*/

int GetRexxErrorMsg(PCHAR pMsg, PCHAR pchTitle, ULONG ulMsgNr);
int StartRexxScript(ULONG ulScriptID, PHWND phwndMonitor);
void DisplayRexxError(HWND hwndOwner, char *pchText, char *pchTitle);
int CreatePipes(void);

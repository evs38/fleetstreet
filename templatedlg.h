/* TEMPLATEDLG.H */

/*--------------------------- Funktionsprototypen ---------------------------*/

MRESULT EXPENTRY TemplateBookProc(HWND parent, ULONG message, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TemplateFolderProc(HWND hwnd, ULONG message, MPARAM mp1, MPARAM mp2);
void LoadDefaultTemplate(PMSGTEMPLATE pTemplate);

/* PRINTMSG.H */

/*--------------------------------- Defines ---------------------------------*/
/* Fehler-Codes */
#define PRINTMSG_OK           0
#define PRINTMSG_NOPRINTER    1
#define PRINTMSG_NOPROP       2
#define PRINTMSG_NODC         3
#define PRINTMSG_NOPS         4
#define PRINTMSG_NOSTARTDOC   5
#define PRINTMSG_ERROR        100

/*--------------------------- Funktionsprototypen ---------------------------*/
int OpenPrinter(PPRINTSETUP pPrintSetup, PHDC pHDC, PHPS pHPS);
int OpenPrinterDM(HAB hab, PHDC pHDC, PHPS pHPS, PPRINTDEST pPrintDest);
int ClosePrinter(HDC hdc, HPS hps);
int PrintMessage(HDC hdc, HPS hps,
                 MSGHEADER *pHeader, FTNMESSAGE *pMessage,
                 PCHAR pchAreaTag, ULONG ulMsgNum, PAREALIST pAreaList,
                 HMODULE hmod, PPRINTSETUP pPrintSetup);


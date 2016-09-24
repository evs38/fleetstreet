/* EXPT.H */

typedef struct
{
   EXCEPTIONREGISTRATIONRECORD Reg;
   int tid;
   char pchThreadName[50];
} EXPTBLOCK, *PEXPTBLOCK;

void RegisterExptHandler(PEXPTBLOCK pExptBlock, PCHAR pchThreadName);
void DeregisterExptHandler(PEXPTBLOCK pExptBlock);

/* Macros */

#define INSTALLEXPT(x)  EXPTBLOCK EBlock; RegisterExptHandler(&EBlock, (x))
#define DEINSTALLEXPT  DeregisterExptHandler(&EBlock)


/* Ende EXPT.H */

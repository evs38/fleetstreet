/* INIT.h */

#define READCFG_CFG  1
#define READCFG_INI  2
#define READCFG_BOTH 3


/* INI-Fehlercodes */

/* Fehlerklassen */
#define ERRORCLASS_INI     0UL
#define ERRORCLASS_CFGDLL  1UL
#define ERRORCLASS_CFGFILE 2UL

/* Makros */
#define INIERROR(x)     (x)
#define CFGDLLERROR(x)  ((ERRORCLASS_CFGDLL << 16) | (x))
#define CFGFILEERROR(x) ((ERRORCLASS_CFGFILE << 16) | (x))

#define ERRORCLASS(x)   ((x) >> 16)
#define ERRORCODE(x)    ((x) & 0xffff)


#define INIFILE_OK       0
#define INIFILE_OPEN     1
#define INIFILE_NEW      2
#define INIFILE_VERSION  3
#define INIFILE_DUPAREAS 4

/*--------------------------- Funktionsprototypen ---------------------------*/
int ParseArgs(int argc, char **argv);
ULONG ReReadAreas(HAB hab, char *CfgFileName, ULONG ulCfgType);
int SaveIniAreas(HAB hab);
int SaveIniProfile(HAB hab);
void _Optlink ReadIniThread(void *phmq);
void _Optlink SaveIniProfileThread(void *pParam);
ULONG HandleInitErrors(HWND hwndClient, ULONG ulError);
BOOL LogoDisplayEnabled(void);

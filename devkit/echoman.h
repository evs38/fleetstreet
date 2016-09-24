/* ECHOMAN.H */

#define DLL_VERSION     1UL

/* Error codes */

#define ECHOMAN_OK            0UL  /* no error                       */
#define ECHOMAN_PARAMSIZE     1UL  /* invalid parameter block size   */
#define ECHOMAN_FORMAT        2UL  /* invalid parameter block format */
#define ECHOMAN_CANCEL        3UL  /* User canceled action           */
#define ECHOMAN_CFGNOTFOUND   4UL  /* CFG file not found             */
#define ECHOMAN_CFGREAD       5UL  /* Error reading CFG file         */
#define ECHOMAN_CFGWRITE      6UL  /* Error writing CFG file         */
#define ECHOMAN_CFGFORMAT     7UL  /* invalid CFG file format        */
#define ECHOMAN_ALREADYLINKED 8UL  /* already linked to the echo     */
#define ECHOMAN_NOTLINKED     9UL  /* not linked to the echo         */
#define ECHOMAN_ERROR       255UL  /* other error                    */

#ifdef __cplusplus
extern "C" {
#endif

/* All functions must use the system calling convention. If you have a
   compiler other than IBM CSet++, declare your functions appropriately! */

ULONG APIENTRY QueryVersion(VOID);

#define MIN_PARAM_SIZE  16UL

ULONG APIENTRY QueryParamBlockSize(VOID);

ULONG APIENTRY SetupParams(PVOID pParamBlock, ULONG ulParamBlockSize,
                           HWND hwndOwner, HAB hab, HMODULE hModule);

#define ADDFLAGS_RESCAN   1UL
ULONG APIENTRY AddEcho(PVOID pParamBlock, ULONG ulParamBlockSize,
                       PCHAR pchCfgFile,
                       PCHAR pchCurrentAddress,
                       PCHAR pchUplinkAddress,
                       PCHAR pchAreaTag,
                       ULONG ulFlags);

ULONG APIENTRY RemoveEcho(PVOID pParamBlock, ULONG ulParamBlockSize,
                          PCHAR pchCfgFile,
                          PCHAR pchCurrentAddress,
                          PCHAR pchUplinkAddress,
                          PCHAR pchAreaTag,
                          ULONG ulFlags);

#ifdef __cplusplus
}
#endif


/* End of ECHOMAN.H */

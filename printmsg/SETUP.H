// setup.h
// Monte Copeland for DevCon 7

#if ! defined INCL_SPLDOSPRINT && !defined RC_INVOKED
#error Define INCL_SPLDOSPRINT and recompile.
#endif

#define LEN_WORKSTRING    196

// Error result codes from SetupPrinter(). (Returns zero for OK.)
#define SETUP_NOQUEUES       1
#define SETUP_NODC           2
#define SETUP_NOMEMORY       3

ULONG APIENTRY SetupPrinter( PPRINTSETUP pSetup);
ULONG APIENTRY CleanupPrinter( PPRINTSETUP pSetup );

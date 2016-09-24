#include <os2.h>
#include <string.h>
#include "echoman.h"

#define MY_SIGNATURE  "MySig"

struct myoptions
{
   CHAR achSig[16];
   ULONG ulFlags;
};

ULONG APIENTRY QueryVersion(VOID)
{
   return DLL_VERSION; /* return compile time version number */
}

ULONG APIENTRY QueryParamBlockSize(VOID)
{
   return sizeof(struct myoptions);
}

ULONG APIENTRY SetupParams(PVOID pParamBlock, ULONG ulParamBlockSize,
                           HWND hwndOwner, HAB hab, HMODULE hModule)
{
   struct myoptions *pOptions = (struct myoptions *) pParamBlock;

   if (pOptions ||
       ulParamBlockSize != sizeof(struct myoptions))
      return ECHOMAN_PARAMSIZE; /* No buffer or buffer too small */

   if (pOptions->achSig[0])
   {
      /* Old signature present, test if it's ours */
      if (strcmp(pOptions->achSig, MY_SIGNATURE))
         return ECHOMAN_FORMAT; /* This is someone else's signature */
   }
   else
   {
      /* This is an unitialized block */
      strcpy(pOptions->achSig, MY_SIGNATURE);

   }

   /* Set parameters, e.g. by popping up a dialog */

   return ECHOMAN_OK;
}

ULONG APIENTRY AddEcho(PVOID pParamBlock, ULONG ulParamBlockSize,
                      PCHAR pchCfgFile,
                      PCHAR pchCurrentAddress,
                      PCHAR pchUplinkAddress,
                      PCHAR pchAreaTag,
                      ULONG ulFlags)
{
   struct myoptions *pOptions = (struct myoptions *) pParamBlock;

   if (pOptions ||
       ulParamBlockSize != sizeof(struct myoptions))
      return ECHOMAN_PARAMSIZE; /* No buffer or buffer too small */

   if (pOptions->achSig[0])
   {
      /* Old signature present, test if it's ours */
      if (strcmp(pOptions->achSig, MY_SIGNATURE))
         return ECHOMAN_FORMAT; /* This is someone else's signature */
   }
   else
      return 1; /* block must be initialized */

   /* Add area to Cfg file */

   return ECHOMAN_OK;
}

ULONG APIENTRY RemoveEcho(PVOID pParamBlock, ULONG ulParamBlockSize,
                         PCHAR pchCfgFile,
                         PCHAR pchCurrentAddress,
                         PCHAR pchUplinkAddress,
                         PCHAR pchAreaTag,
                         ULONG ulFlags)
{
   struct myoptions *pOptions = (struct myoptions *) pParamBlock;

   if (pOptions ||
       ulParamBlockSize != sizeof(struct myoptions))
      return ECHOMAN_PARAMSIZE; /* No buffer or buffer too small */

   if (pOptions->achSig[0])
   {
      /* Old signature present, test if it's ours */
      if (strcmp(pOptions->achSig, MY_SIGNATURE))
         return ECHOMAN_FORMAT; /* This is someone else's signature */
   }
   else
      return ECHOMAN_FORMAT; /* block must be initialized */


   /* Remove area from Cfg file */

   return ECHOMAN_OK;
}


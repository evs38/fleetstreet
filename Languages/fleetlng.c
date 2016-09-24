/* FLEETLNG.C, DLL fuer Language-Resourcen */

#define DLL_VERSION   24UL

unsigned long _System FLTLNG_QueryVersion(void);

unsigned long _System FLTLNG_QueryVersion(void)
{
   return DLL_VERSION;
}

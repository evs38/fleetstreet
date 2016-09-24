// setup.c: printer setup routine
// Monte Copeland for DevCon 7

// os2 includes
#define INCL_BASE
#define INCL_PM
#define INCL_SPLDOSPRINT
#include <os2.h>

// c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// local includes
#include "..\main.h"
#include "..\structs.h"
#include "..\printsetup.h"
#include "setup.h"

// Private functions herein.
static PPRQINFO3 APIENTRY FindQueue( PPRINTSETUP );


// ---------------------------------------------------------------------------------------------------
// SetupPrinter() returns zero for success.

ULONG APIENTRY SetupPrinter( PPRINTSETUP pSetup)
{
  BOOL            fOK;
  CHAR            szDefaultQueue[ LEN_WORKSTRING ];
  CHAR            szWork[ LEN_WORKSTRING ];
  PCHAR           pch;
  PPRQINFO3       pqi;
  SIZEL           sizel;
  ULONG           cReturned;
  ULONG           cTotal;
  ULONG           cbNeeded;
  ULONG           ul;
  ULONG           ulrc;
  PCHAR           pchDevName;


  // no good unless I can open a PS
  pSetup->pDevOpenData = NULL;


  // Close the info DC's and PS's from any previous call.
  if( pSetup->hpsPrinterInfo )
  {
    GpiAssociate( pSetup->hpsPrinterInfo, (HDC)0 );
    GpiDestroyPS( pSetup->hpsPrinterInfo );
    pSetup->hpsPrinterInfo = (HPS)0;
  }

  if( pSetup->hdcPrinterInfo )
  {
    DevCloseDC( pSetup->hdcPrinterInfo );
    pSetup->hdcPrinterInfo = (HDC)0;
  }

  if( pSetup->pQueueInfo )
  {
    // Free the array of PRQINFO3 from previous call.
    free( pSetup->pQueueInfo );
    pSetup->pQueueInfo = NULL;
  }


  // Query how many queues exist on this computer and the
  // number of bytes needed to hold the array.
  ul = SplEnumQueue( NULL, 3, NULL, 0, &cReturned, &cTotal, &cbNeeded, NULL );
  if( cTotal == 0 )
  {
    // There are no queues on this computer!
    ulrc = SETUP_NOQUEUES;
    pSetup->cQueues = 0;
    goto depart;
  }


  // Allocate memory to store the newly enumerated queue information.
  pSetup->pQueueInfo = calloc(1, cbNeeded ) ;
  if( ! pSetup->pQueueInfo )
  {
    ulrc = SETUP_NOMEMORY;
    goto depart;
  }

  // Call system again to get the array of PRQINFO3 structures.
  ul = SplEnumQueue( NULL, 3, pSetup->pQueueInfo, cbNeeded, &cReturned, &cTotal, &cbNeeded, NULL );
  pSetup->cQueues = cReturned;

  /* Workaround: PDRIVDATA ist nicht korrekt, vom Treiber selbst holen */
  for (ul=0; ul<cReturned; ul++)
  {
     /* alte Daten wegwerfen */
     pSetup->pQueueInfo[ul].pDriverData = NULL;

     /* Strings vorbereiten */
     strcpy(szWork, pSetup->pQueueInfo[ul].pszDriverName);
     pch = strchr( szWork, '.' );
     if( pch )
     {
        *pch = 0;
        pchDevName = pch+1;
     }
     else
        pchDevName = "";

     pch = strchr( pSetup->pQueueInfo[ul].pszPrinters, ',' );
     if( pch )
       *pch = 0;

     /* Datengr”áe holen */
     cbNeeded = DevPostDeviceModes(pSetup->hab, NULL, szWork, pchDevName,
                                   pSetup->pQueueInfo[ul].pszPrinters,
                                   DPDM_QUERYJOBPROP);
     pSetup->pQueueInfo[ul].pDriverData = calloc(1, cbNeeded);
     if (pSetup->pQueueInfo[ul].pDriverData)
        /* Daten holen */
        DevPostDeviceModes(pSetup->hab, pSetup->pQueueInfo[ul].pDriverData,
                           szWork, pchDevName,
                           pSetup->pQueueInfo[ul].pszPrinters,
                           DPDM_QUERYJOBPROP);
  }


  // Establish a default queue -- might need it.
  // Profiled queue name ends with a semicolon.
  ul = PrfQueryProfileString( HINI_PROFILE, "PM_SPOOLER", "QUEUE", "", szDefaultQueue, LEN_WORKSTRING );
  if( ul > 1 )
  {
    // Trim off semicolon.
    pch = strchr( szDefaultQueue, ';' );
    if (pch)
       *pch = 0;
  }
  else
    // Hmmmm. Use the first one queue from the enumeration.
    strcpy( szDefaultQueue,  pSetup->pQueueInfo->pszName );

  if( !pSetup->szPreferredQueue[0])
    // No queue preference; use default.
    strcpy( pSetup->szPreferredQueue, szDefaultQueue );



  // There is a chance that the preferred queue has been recently deleted.
  // Ensure the preferred queue still exists.

  pqi = FindQueue( pSetup );
  if( ! pqi )
  {
    // Not found. Use the default queue.
    strcpy( pSetup->szPreferredQueue, szDefaultQueue );

    if( pSetup->pDriverData )
    {
      free( pSetup->pDriverData );
      pSetup->pDriverData = NULL;
    }
  }
  else
  {
    // Preferred queue was found in the array. Do some additional validation
    // because it may have changed since last time in this function.

    fOK = TRUE;

    if( pSetup->pDriverData )
    {
      // Is driver data the right length?
      fOK = fOK && ( pqi->pDriverData->cb == pSetup->pDriverData->cb );

      // Is this queue still driving the same device?
      fOK = fOK && ( 0 == strcmp( pqi->pDriverData->szDeviceName,  pSetup->pDriverData->szDeviceName ));
    }

    if( !fOK )
    {
      free( pSetup->pDriverData );
      pSetup->pDriverData = NULL;
    }
  }



  // Find the queue again. If the last find failed, preferred queue name
  // was changed to default. This find will absolutely always succeed.

  pqi = FindQueue( pSetup );

  if( !pSetup->pDriverData )
  {
    // Use driver data from the enumeration.
    pSetup->pDriverData = malloc( pqi->pDriverData->cb );
    if( ! pSetup->pDriverData )
    {
      ulrc = SETUP_NOMEMORY;
      goto depart;
    }
    memcpy( pSetup->pDriverData, pqi->pDriverData, pqi->pDriverData->cb );
  }

  /* Prepare a DEVOPENSTRUC for DevOpenDC(). Use it here to open an OD_INFO
  DC. Caller may use the same DEVOPENSTRUC to open an OD_QUEUED DC when it is
  time to print. There are 9 pointers in the DEVOPENSTRUC. This code
  prepares the first 4. The others should be NULL.
  */

  // Prepare logical address which is preferred queue name.
  pSetup->lDCType = OD_QUEUED;
  pSetup->devopenstruc.pszLogAddress = pSetup->szPreferredQueue;


  // Prepare .DRV file name. Truncate after the period.
  strcpy( szWork, pqi->pszDriverName );
  pch = strchr( szWork, '.' );
  if( pch )
    *pch = 0;

  if( pSetup->devopenstruc.pszDriverName )
    free( pSetup->devopenstruc.pszDriverName );

  pSetup->devopenstruc.pszDriverName = strdup( szWork );
  if( ! pSetup->devopenstruc.pszDriverName )
  {
    ulrc = SETUP_NOMEMORY;
    goto depart;
  }

  // Prepare pointer to driver data.
  pSetup->devopenstruc.pdriv = pSetup->pDriverData;

  // Prepare data type. Standard is the preferred way to go.
  pSetup->devopenstruc.pszDataType = "PM_Q_STD";

  // Open an OD_INFO DC.
  pSetup->hdcPrinterInfo = DevOpenDC( pSetup->hab,
                                      OD_INFO,
                                      "*",
                                      4,
                                      (PDEVOPENDATA)&pSetup->devopenstruc,
                                      (HDC)0 );
  if( !pSetup->hdcPrinterInfo )
  {
    // Unable to open info DC. WinGetLastError() can provide diagnostics.
    ulrc = SETUP_NODC;
    goto depart;
  }

  // Create PS associated with OD_INFO DC.
  sizel.cx = 0;
  sizel.cy = 0;
  pSetup->hpsPrinterInfo = GpiCreatePS( pSetup->hab,
                                        pSetup->hdcPrinterInfo,
                                        &sizel,
                                        pSetup->lWorldCoordinates | GPIA_ASSOC );

  if( GPI_ERROR ==  pSetup->hpsPrinterInfo )
  {
    // Problem with this setup.
    DevCloseDC( pSetup->hdcPrinterInfo );
    pSetup->hdcPrinterInfo = (HDC)0;
    pSetup->hpsPrinterInfo = (HPS)0;
    ulrc = SETUP_NODC;
    goto depart;
  }


  // OK to use.
  pSetup->pDevOpenData = (PDEVOPENDATA)&pSetup->devopenstruc;

  // Success.
  ulrc = 0;

depart:
  return ulrc;
}



// ---------------------------------------------------------------------------------------------------------
// FindQueue finds the preferred queue name in the PRQINFO3 array.
// Returns the index if found; returns -1 if not found.

static PPRQINFO3 APIENTRY FindQueue( PPRINTSETUP pSetup )
{
  LONG   i;

  for( i = 0; i < pSetup->cQueues; i++ )
  {
    if( 0 == strcmp( pSetup->szPreferredQueue, pSetup->pQueueInfo[ i ].pszName ) )
    {
      return &pSetup->pQueueInfo[ i ];
    }
  }
  return NULL;
}

//-----------------------------------------------------------------------------------------------------------
// close the PS's and DC's if they are open
// free any memory allocations

ULONG APIENTRY CleanupPrinter( PPRINTSETUP pSetup )
{

  // Close DC's and PS's.

  if( pSetup->hpsPrinterInfo )
  {
    GpiAssociate( pSetup->hpsPrinterInfo, (HDC)0 );
    GpiDestroyPS( pSetup->hpsPrinterInfo );
    pSetup->hpsPrinterInfo = (HPS)0;
  }

  if( pSetup->hdcPrinterInfo )
  {
    DevCloseDC( pSetup->hdcPrinterInfo );
    pSetup->hdcPrinterInfo = (HDC)0;
  }


  if( pSetup->pQueueInfo )
  {
    free( pSetup->pQueueInfo );
    pSetup->pQueueInfo = NULL;
  }

  if( pSetup->pDriverData )
  {
    free( pSetup->pDriverData );
    pSetup->pDriverData = NULL;
  }

  if( pSetup->devopenstruc.pszDriverName )
  {
    free( pSetup->devopenstruc.pszDriverName );
    pSetup->devopenstruc.pszDriverName = NULL;
  }

  return 0;
}



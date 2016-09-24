#pragma strings(readonly)

#define INCL_BASE
#define INCL_DOSEXCEPTIONS
#define INCL_WIN
#include <os2.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../version.h"
#include "../main.h"

#include "expt.h"
#include "pmassert.h"

#define SIZE_MODNAME 30

static ULONG APIENTRY FleetExptHandler(PEXCEPTIONREPORTRECORD pReport,
                                       PEXCEPTIONREGISTRATIONRECORD pReg,
                                       PCONTEXTRECORD pContext,
                                       PVOID pReserved);
static void DumpEBPTrace(PULONG pStart, int iFile, char *pchText);
static void WriteLine(int iFile, const char *pchOut);
static APIRET QueryModule(PVOID pAddress, PULONG pulObj,
                          PULONG pulOffs, PCHAR pchMod);
static int OpenDumpFile(void);

/* Prototyp v. undok. Funktion */

APIRET APIENTRY DosQueryModFromEIP(PHMODULE pMod, PULONG pObNum, ULONG ulBufSize,
                                   PCHAR pchBuf, PULONG pOffset, PVOID pAddress);

void RegisterExptHandler(PEXPTBLOCK pExptBlock, PCHAR pchThreadName)
{
   PTIB pTIB;
   PPIB pPIB;

   memset(pExptBlock, 0, sizeof(EXPTBLOCK));

   pExptBlock->Reg.ExceptionHandler = FleetExptHandler;

   if (!DosGetInfoBlocks(&pTIB, &pPIB))
      pExptBlock->tid = pTIB->tib_ptib2->tib2_ultid;

   strncpy(pExptBlock->pchThreadName, pchThreadName,
           sizeof(pExptBlock->pchThreadName)-1);

   DosSetExceptionHandler(&pExptBlock->Reg);

   return;
}

void DeregisterExptHandler(PEXPTBLOCK pExptBlock)
{
   DosUnsetExceptionHandler(&pExptBlock->Reg);

   return;
}

static ULONG APIENTRY FleetExptHandler(PEXCEPTIONREPORTRECORD pReport,
                                       PEXCEPTIONREGISTRATIONRECORD pReg,
                                       PCONTEXTRECORD pContext,
                                       PVOID pReserved)
{
   PEXPTBLOCK pExptBlock = (PEXPTBLOCK) pReg;

   if (!(pReport->fHandlerFlags & EH_NESTED_CALL) &&
       (pReport->ExceptionNum == XCPT_ACCESS_VIOLATION ||
        pReport->ExceptionNum == XCPT_ILLEGAL_INSTRUCTION ||
        pReport->ExceptionNum == XCPT_FLOAT_DENORMAL_OPERAND ||
        pReport->ExceptionNum == XCPT_FLOAT_DIVIDE_BY_ZERO ||
        pReport->ExceptionNum == XCPT_FLOAT_INEXACT_RESULT ||
        pReport->ExceptionNum == XCPT_FLOAT_INVALID_OPERATION ||
        pReport->ExceptionNum == XCPT_FLOAT_OVERFLOW ||
        pReport->ExceptionNum == XCPT_FLOAT_STACK_CHECK ||
        pReport->ExceptionNum == XCPT_FLOAT_UNDERFLOW ||
        pReport->ExceptionNum == XCPT_INTEGER_DIVIDE_BY_ZERO ||
        pReport->ExceptionNum == XCPT_INTEGER_OVERFLOW ||
        pReport->ExceptionNum == XCPT_PRIVILEGED_INSTRUCTION))
   {
      /* Diese X behandeln wir */
      int iDumpFile;

      /* File oeffnen */
      iDumpFile = OpenDumpFile();
      if (iDumpFile >= 0)
      {
         DATETIME DateTime;
         char pchText[100];
         ULONG ulOffs, ulObj;
         ULONG ulVer[3];

         WriteLine(iDumpFile, "\n=========\n");

         DosGetDateTime(&DateTime);

         sprintf(pchText, "Bad thing happened at %02d.%02d.%d, %02d:%02d:%02d\n\n",
                          DateTime.day, DateTime.month, DateTime.year,
                          DateTime.hours, DateTime.minutes, DateTime.seconds);
         WriteLine(iDumpFile, pchText);

         DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_REVISION, ulVer, sizeof(ulVer));
         sprintf(pchText, "OS/2 %d.%02d.%02d\n", ulVer[0], ulVer[1], ulVer[2]);
         WriteLine(iDumpFile, pchText);

         WriteLine(iDumpFile, "FS " FLEETVER "\n\n");

         sprintf(pchText, "TID=%d, %s\n", pExptBlock->tid, pExptBlock->pchThreadName);
         WriteLine(iDumpFile, pchText);

         sprintf(pchText, "Location: %08x (", pReport->ExceptionAddress);
         WriteLine(iDumpFile, pchText);

         if (!QueryModule(pReport->ExceptionAddress, &ulObj, &ulOffs, pchText))
         {
            WriteLine(iDumpFile, pchText);
            sprintf(pchText, " @ %04x:%08x)\n", ulObj+1, ulOffs);
            WriteLine(iDumpFile, pchText);
         }
         else
            WriteLine(iDumpFile, "?)\n");

         WriteLine(iDumpFile, "Exception: ");
         switch(pReport->ExceptionNum)
         {
            char chOp;

            case XCPT_ACCESS_VIOLATION:
               switch(pReport->ExceptionInfo[0])
               {
                  case XCPT_READ_ACCESS:
                     chOp = 'r';
                     break;

                  case XCPT_WRITE_ACCESS:
                     chOp = 'w';
                     break;

                  case XCPT_EXECUTE_ACCESS:
                     chOp = 'x';
                     break;

                  case XCPT_SPACE_ACCESS:
                     chOp = 's';
                     break;

                  case XCPT_LIMIT_ACCESS:
                     chOp = 'l';
                     break;

                  default:
                     chOp = '?';
                     break;
               }

               sprintf(pchText, "ACCESS_VIOLATION, Op=%c, Addr=%08x\n", chOp, pReport->ExceptionInfo[1]);
               WriteLine(iDumpFile, pchText);
               break;

            case XCPT_ILLEGAL_INSTRUCTION:
               WriteLine(iDumpFile, "ILLEGAL_INSTRUCTION\n");
               break;

            case XCPT_FLOAT_DENORMAL_OPERAND:
               WriteLine(iDumpFile, "FLOAT_DENORMAL_OPERAND\n");
               break;

            case XCPT_FLOAT_DIVIDE_BY_ZERO:
               WriteLine(iDumpFile, "FLOAT_DIVIDE_BY_ZERO\n");
               break;

            case XCPT_FLOAT_INEXACT_RESULT:
               WriteLine(iDumpFile, "FLOAT_INEXACT_RESULT\n");
               break;

            case XCPT_FLOAT_INVALID_OPERATION:
               WriteLine(iDumpFile, "FLOAT_INVALID_OPERATION\n");
               break;

            case XCPT_FLOAT_OVERFLOW:
               WriteLine(iDumpFile, "FLOAT_OVERFLOW\n");
               break;

            case XCPT_FLOAT_STACK_CHECK:
               WriteLine(iDumpFile, "FLOAT_STACK_CHECK\n");
               break;

            case XCPT_FLOAT_UNDERFLOW:
               WriteLine(iDumpFile, "FLOAT_UNDERFLOW\n");
               break;

            case XCPT_INTEGER_DIVIDE_BY_ZERO:
               WriteLine(iDumpFile, "INTEGER_DIVIDE_BY_ZERO\n");
               break;

            case XCPT_INTEGER_OVERFLOW:
               WriteLine(iDumpFile, "INTEGER_OVERFLOW\n");
               break;

            case XCPT_PRIVILEGED_INSTRUCTION:
               WriteLine(iDumpFile, "PRIVILEGED_INSTRUCTION\n");
               break;

            default:
               sprintf(pchText, "%08x\n", pReport->ExceptionNum);
               WriteLine(iDumpFile, pchText);
               break;
         }

         WriteLine(iDumpFile, "\nLotsofnumbers:\n\n");

         if (pContext->ContextFlags & CONTEXT_CONTROL)
         {
            sprintf(pchText, "SS:ESP=%04x:%08x, CS:EIP=%04x:%08x, EFLAGS=%08x, EBP=%08x\n",
                              pContext->ctx_SegSs, pContext->ctx_RegEsp,
                              pContext->ctx_SegCs, pContext->ctx_RegEip,
                              pContext->ctx_EFlags,
                              pContext->ctx_RegEbp);
            WriteLine(iDumpFile, pchText);
         }
         if (pContext->ContextFlags & CONTEXT_INTEGER)
         {
            sprintf(pchText, "EAX=%08x, EBX=%08x, ECX=%08x, EDX=%08x\n",
                              pContext->ctx_RegEax, pContext->ctx_RegEbx,
                              pContext->ctx_RegEcx, pContext->ctx_RegEdx);
            WriteLine(iDumpFile, pchText);
            sprintf(pchText, "ESI=%08x, EDI=%08x\n",
                              pContext->ctx_RegEsi, pContext->ctx_RegEdi);
            WriteLine(iDumpFile, pchText);
         }
         if (pContext->ContextFlags & CONTEXT_SEGMENTS)
         {
            sprintf(pchText, "DS=%04x, ES=%04x, FS=%04x, GS=%04x\n",
                              pContext->ctx_SegDs, pContext->ctx_SegEs,
                              pContext->ctx_SegFs, pContext->ctx_SegGs);
            WriteLine(iDumpFile, pchText);
         }

         DumpEBPTrace((PULONG) pContext->ctx_RegEbp, iDumpFile, pchText);

         WriteLine(iDumpFile, "\nThat's all, I quit...\n");

         close(iDumpFile);
      }
   }

   return XCPT_CONTINUE_SEARCH; /* auf jeden Fall weiter */
}

static void DumpEBPTrace(PULONG pStart, int iFile, char *pchText)
{
   PTIB pTIB;
   PPIB pPIB;

   if (!DosGetInfoBlocks(&pTIB, &pPIB))
   {
      ULONG ulObj, ulOffs;
      ULONG ulLen=sizeof(ULONG), ulAcc;

      sprintf(pchText, "\nEBP trace (%08x - %08x):\n\n", pTIB->tib_pstack, pTIB->tib_pstacklimit);
      WriteLine(iFile, pchText);

      while (pStart < (PULONG) pTIB->tib_pstacklimit)
      {
         /* Zugriff auf pStart pruefen */
         if (DosQueryMem(pStart, &ulLen, &ulAcc) ||
             (ulAcc & (PAG_COMMIT | PAG_READ)) != (PAG_COMMIT | PAG_READ))
            break;

         sprintf(pchText, "%08x: Call from %08x (", pStart, *(pStart+1)-1);
         WriteLine(iFile, pchText);
         if (!QueryModule((PVOID)(*(pStart+1)-1), &ulObj, &ulOffs, pchText))
         {
            WriteLine(iFile, pchText);
            sprintf(pchText, " @ %04x:%08x)\n", ulObj+1, ulOffs);
            WriteLine(iFile, pchText);
         }
         else
            WriteLine(iFile, "\n");

         pStart = (PULONG) *pStart;
      }
   }
   return;
}

static void WriteLine(int iFile, const char *pchOut)
{
   write(iFile, pchOut, strlen(pchOut));

   return;
}

static APIRET QueryModule(PVOID pAddress, PULONG pulObj, PULONG pulOffs, PCHAR pchMod)
{
   HMODULE Module;

   return DosQueryModFromEIP(&Module, pulObj, SIZE_MODNAME, pchMod,
                             pulOffs, pAddress);
}

void _PMAssert(const char *pCond, const char *pFile, int Line,
               const char *pComment)
{
   static char TextBuffer[1000];
   int iFile;

   sprintf(TextBuffer, "Assertion failed: %s\n"
                       "File: %s, Line: %d\n"
                       "(%s)\n",
                       pCond, pFile, Line, pComment);

   iFile = OpenDumpFile();
   if (iFile >= 0)
   {
      WriteLine(iFile, "\n=========\n");
      WriteLine(iFile, TextBuffer);
      close(iFile);
   }

   WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, TextBuffer,
                 "Internal error", 0,
                 MB_ENTER|MB_ERROR|MB_SYSTEMMODAL|MB_MOVEABLE);

   exit(RET_ERROR);
}

static int OpenDumpFile(void)
{
   return open("FLTSTRT.DMP", O_APPEND | O_CREAT | O_WRONLY | O_TEXT,
                              S_IREAD | S_IWRITE);
}

/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/fonts/fntsmp.c -- AsuaCtl raster font helper sample application.
 *
 * Demonstrates XpmCreateRasterFont(): the sample tries a list of
 * "[pointsize.]facename" specifications, uses the first one that yields
 * a usable image font, draws a line of text in that font and reports
 * the metrics returned in the FONTMETRICS block.  A deliberately bogus
 * face name is also attempted to show the honest-failure path (an
 * XpmERR_FNT_* code rather than a silent substitution).
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WINSYS
#include "expm.h"
#include <stdio.h>
#include <string.h>

#define CLSNAME  "expfonts"
#define TITLE    "EXPM XpmCreateRasterFont Test"

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* Candidate specifications, in order of preference.  The point size is
 * a hint: the nearest available image font of that face is used. */
static const char *apszSpec[] =
{
    "10.System Monospaced",
    "12.System Monospaced",
    "10.System Proportional",
    "12.Helv",
    NULL
};


/*-------------------------------------------------------------------------
 * DrawText - draw a string with its baseline at (x, y).
 *-------------------------------------------------------------------------*/
static void DrawText(HPS hps, LONG x, LONG y, const char *psz)
{
   POINTL pt;

   pt.x = x;  pt.y = y;
   GpiMove(hps, &pt);
   GpiCharString(hps, (LONG)strlen(psz), (PCH)psz);
}


int main(void)
{
   HAB   hab;
   HMQ   hmq;
   HWND  hFrame, hClient;
   QMSG  qmsg;
   ULONG flCreate;

   flCreate = FCF_SYSMENU | FCF_TITLEBAR | FCF_SIZEBORDER |
              FCF_MINMAX  | FCF_SHELLPOSITION | FCF_TASKLIST;

   hab = WinInitialize(0);
   hmq = WinCreateMsgQueue(hab, 0);

   WinRegisterClass(hab, CLSNAME, ClientWndProc, CS_SIZEREDRAW, 0);

   hFrame = WinCreateStdWindow(HWND_DESKTOP, 0L, &flCreate, CLSNAME,
                               TITLE, 0L, NULLHANDLE, 0, &hClient);

   if (hFrame != NULLHANDLE)
   {
      WinSetWindowPos(hFrame, HWND_TOP, 100, 100, 560, 400,
                      SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW);

      while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
         WinDispatchMsg(hab, &qmsg);
   }

   WinDestroyWindow(hFrame);
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
}


MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch (msg)
   {
      case WM_ERASEBACKGROUND:
      {
         HPS   hps = (HPS)mp1;
         RECTL rcl;

         GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         WinQueryWindowRect(hwnd, &rcl);
         WinFillRect(hps, &rcl,
                     (LONG)WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0));
         return (MRESULT)TRUE;
      }

      case WM_PAINT:
      {
         HPS          hps;
         RECTL        rcl;
         FONTMETRICS  fm;
         FONTMETRICS  fmBad;
         LONG         lOld;
         LONG         rc;
         LONG         lBad;
         LONG         i;
         int          iUsed = -1;
         char         szBuf[160];

         hps = WinBeginPaint(hwnd, NULLHANDLE, &rcl);
         WinQueryWindowRect(hwnd, &rcl);
         GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         WinFillRect(hps, &rcl,
                     (LONG)WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0));
         GpiSetColor(hps, CLR_BLACK);

         lOld = GpiQueryCharSet(hps);

         DrawText(hps, 8, rcl.yTop - 22,
                  "XpmCreateRasterFont() -- bind a raster font to a local id");

         /* Try each candidate until one yields an image font. */
         rc = XpmERR_FNT_CREATE;
         for (i = 0; apszSpec[i] != NULL; i++)
         {
            rc = XpmCreateRasterFont(hps, (PSZ)apszSpec[i], 1, &fm);
            if (rc == 0)
            {
               iUsed = i;
               break;
            }
         }

         if (iUsed >= 0)
         {
            sprintf(szBuf, "Using \"%s\": rc=0, bound to lcid 1", apszSpec[iUsed]);
            GpiSetColor(hps, CLR_DARKGREEN);
            DrawText(hps, 8, rcl.yTop - 48, szBuf);
            GpiSetColor(hps, CLR_BLACK);

            /* Select the new font and draw a sample line in it. */
            GpiSetCharSet(hps, 1);
            DrawText(hps, 8, rcl.yTop - 90,
                     "The quick brown fox jumps over the lazy dog  0123456789");
            GpiSetCharSet(hps, lOld);

            sprintf(szBuf, "face=\"%s\"  nominal=%ld decipoints  em=%ld  asc=%ld  desc=%ld",
                    fm.szFacename, (long)fm.sNominalPointSize,
                    (long)fm.lEmHeight, (long)fm.lMaxAscender,
                    (long)fm.lMaxDescender);
            DrawText(hps, 8, rcl.yTop - 120, szBuf);

            sprintf(szBuf, "aveCharWidth=%ld  maxCharInc=%ld  match=0x%08lx",
                    (long)fm.lAveCharWidth, (long)fm.lMaxCharInc,
                    (unsigned long)fm.lMatch);
            DrawText(hps, 8, rcl.yTop - 140, szBuf);
         }
         else
         {
            sprintf(szBuf, "No image font found; last rc=%ld", (long)rc);
            GpiSetColor(hps, CLR_RED);
            DrawText(hps, 8, rcl.yTop - 48, szBuf);
         }

         /* Show the honest-failure path: a face that does not exist.
          * Bound to lcid 2 so it cannot disturb the font above. */
         lBad = XpmCreateRasterFont(hps, (PSZ)"10.EXPM No Such Font", 2, &fmBad);
         GpiSetColor(hps, CLR_BLACK);
         sprintf(szBuf, "Bogus face \"10.EXPM No Such Font\" -> rc=%ld (XpmERR_FNT_CREATE=%ld)",
                 (long)lBad, (long)XpmERR_FNT_CREATE);
         DrawText(hps, 8, rcl.yTop - 170, szBuf);

         WinEndPaint(hps);
         break;
      }

      case WM_CLOSE:
         WinPostMsg(hwnd, WM_QUIT, 0L, 0L);
         break;

      default:
         return WinDefWindowProc(hwnd, msg, mp1, mp2);
   }
   return (MRESULT)FALSE;
}

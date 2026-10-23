/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/arrowhead/arsmp.c -- ArrowHead module sample application.
 *
 * Draws three groups of arrows in a resizable PM window to demonstrate
 * XpmDrawArrowHead() with different directions, half-angles and fill
 * styles.  All coordinates scale with the client size.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WINSYS
#define INCL_WINMENUS
#include "expm.h"
#include "arsmp.h"
#include <string.h>

#define CLSNAME  "exparrow"
#define TITLE    "EXPM ArrowHead Test"

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static BOOL bDark;
static HWND hwndThemeMenu;

/*-------------------------------------------------------------------------
 * DrawArrow - draw a shaft from (x1,y1) to (x2,y2) and put an arrowhead
 * at the (x2,y2) end.
 *-------------------------------------------------------------------------*/
static void DrawArrow(HPS hps, LONG x1, LONG y1, LONG x2, LONG y2,
                      LONG lLen, double dAngle, BOOL bFill)
{
   POINTL ptFrom, ptTo;

   ptFrom.x = x1;  ptFrom.y = y1;
   ptTo.x   = x2;  ptTo.y   = y2;

   GpiMove(hps, &ptFrom);
   GpiLine(hps, &ptTo);
   XpmDrawArrowHead(hps, ptFrom, ptTo, lLen, dAngle, bFill);
}


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

   /* FCF_ICON is omitted: the sample has no icon resource bound to the
    * executable, and WinCreateStdWindow fails if FCF_ICON is set without
    * one. */
   flCreate = FCF_SYSMENU | FCF_TITLEBAR | FCF_SIZEBORDER |
              FCF_MINMAX  | FCF_SHELLPOSITION | FCF_TASKLIST |
              FCF_MENU;

   hab = WinInitialize(0);
   hmq = WinCreateMsgQueue(hab, 0);

   WinRegisterClass(hab, CLSNAME, ClientWndProc, CS_SIZEREDRAW, 0);

   hFrame = WinCreateStdWindow(HWND_DESKTOP, 0L, &flCreate, CLSNAME,
                               TITLE, 0L, NULLHANDLE, ID_RES_MENU, &hClient);

   if (hFrame != NULLHANDLE)
   {
      hwndThemeMenu = WinWindowFromID(hFrame, FID_MENU);

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
         HPS       hps = (HPS)mp1;
         PXpmTheme pth = XpmQueryTheme();
         RECTL     rcl;

         GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         WinQueryWindowRect(hwnd, &rcl);
         WinFillRect(hps, &rcl, (LONG)pth->clrCanvas);
         return (MRESULT)TRUE;
      }

      case WM_PAINT:
      {
         HPS       hps;
         PXpmTheme pth;
         RECTL     rcl;
         LONG      cx, cy;
         LONG      head, arm, i, y;

         hps = WinBeginPaint(hwnd, NULLHANDLE, &rcl);
         pth = XpmQueryTheme();
         WinQueryWindowRect(hwnd, &rcl);
         GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         WinFillRect(hps, &rcl, (LONG)pth->clrCanvas);
         GpiSetColor(hps, (LONG)pth->clrText);

         cx   = rcl.xRight;
         cy   = rcl.yTop;
         head = cx / 16;      /* length of each arrowhead line */
         arm  = cx / 10;      /* half of the shaft length      */

         /* Row 1: open arrowheads in the four principal directions. */
         y = cy * 82 / 100;
         DrawText(hps, 8, cy * 90 / 100, "Open arrowheads (30 degrees)");
         DrawArrow(hps, cx/8   - arm, y,     cx/8   + arm, y,     head, 30.0, FALSE);
         DrawArrow(hps, cx*3/8,       y-arm, cx*3/8,       y+arm, head, 30.0, FALSE);
         DrawArrow(hps, cx*5/8 + arm, y,     cx*5/8 - arm, y,     head, 30.0, FALSE);
         DrawArrow(hps, cx*7/8,       y+arm, cx*7/8,       y-arm, head, 30.0, FALSE);

         /* Row 2: filled arrowheads on the four diagonals. */
         y = cy * 52 / 100;
         DrawText(hps, 8, cy * 60 / 100, "Filled arrowheads (30 degrees)");
         DrawArrow(hps, cx/8   - arm, y-arm, cx/8   + arm, y+arm, head, 30.0, TRUE);
         DrawArrow(hps, cx*3/8 - arm, y+arm, cx*3/8 + arm, y-arm, head, 30.0, TRUE);
         DrawArrow(hps, cx*5/8 + arm, y+arm, cx*5/8 - arm, y-arm, head, 30.0, TRUE);
         DrawArrow(hps, cx*7/8 + arm, y-arm, cx*7/8 - arm, y+arm, head, 30.0, TRUE);

         /* Row 3: half-angle comparison, all pointing right. */
         y = cy * 22 / 100;
         DrawText(hps, 8, cy * 30 / 100, "Half-angle 15 / 30 / 45 degrees");
         for (i = 0; i < 3; i++)
         {
            LONG   x = cx * (2 * i + 2) / 8;
            double d = (i == 0) ? 15.0 : (i == 1) ? 30.0 : 45.0;

            DrawArrow(hps, x - arm, y, x + arm, y, head, d, FALSE);
         }

         WinEndPaint(hps);
         break;
      }

      case WM_COMMAND:
         if (SHORT1FROMMP(mp1) == IDM_DARK)
         {
            XpmTheme th;

            bDark = !bDark;
            if (bDark)
               XpmThemeDark(&th);
            else
               XpmThemeDefault(&th);
            XpmSetTheme(&th);
            WinCheckMenuItem(hwndThemeMenu, IDM_DARK, bDark);
            WinInvalidateRect(hwnd, NULLHANDLE, TRUE);
            return (MRESULT)FALSE;
         }
         break;

      case XpmWM_THEMECHANGED:
         WinInvalidateRect(hwnd, NULLHANDLE, TRUE);
         break;

      case WM_CLOSE:
         WinPostMsg(hwnd, WM_QUIT, 0L, 0L);
         break;

      default:
         return WinDefWindowProc(hwnd, msg, mp1, mp2);
   }
   return (MRESULT)FALSE;
}

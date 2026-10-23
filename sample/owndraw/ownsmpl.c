/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/owndraw/ownsmpl.c -- Owner-Draw Listbox sample application.
 *
 * Creates a window holding an LS_OWNERDRAW listbox whose items are
 * tab-separated "Name / Size / Date" rows.  The two EXPM helpers keep
 * the columns aligned even though the listbox uses a proportional font:
 * WM_MEASUREITEM calls XpmInitLBWidthHeight() to fix the row geometry,
 * and WM_DRAWITEM calls XpmDrawLBItems() to paint each row.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#define INCL_WIN
#define INCL_WINMENUS
#define INCL_GPI

#include <os2.h>
#include <string.h>
#include "expm.h"

#define CLSNAME  "expondraw"
#define TITLE    "EXPM Owner-Draw Listbox Sample"

#define ID_LISTBOX  100

#define IDM_MENU    500
#define IDM_VIEW    501
#define IDM_DARK    502

/* Longest item string, in characters. */
#define ITEM_CHARS  30

/* Margin between the client edge and the listbox, in pels. */
#define MARGIN      8

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static HWND  hwndList   = NULLHANDLE;
static LONG  lCharWidth = 0;
static BOOL  bDark      = FALSE;
static HWND  hwndThemeMenu = NULLHANDLE;

/* Column boundaries as 1-based character positions, zero-terminated.
 * Column 1 is the leftmost; each further entry starts a new column.
 * Field 1 is at most 11 chars, so column 2 starts at 15. */
static SHORT asTabStops[] = { 15, 21, 0 };

/* One tab-separated row per listbox item. */
static PSZ apszItems[] =
{
   "ABOUT.HLP\t12 KB\t2026-07-24",
   "CHANGES.TXT\t4 KB\t2026-07-24",
   "INSTALL.CMD\t2 KB\t2026-07-25",
   "LICENSE\t2 KB\t2026-07-24",
   "OWNDRAW.DLL\t19 KB\t2026-07-24",
   "OWNDRAW.LIB\t3 KB\t2026-07-24",
   "README.MD\t5 KB\t2026-07-24",
   "SOURCE.ZIP\t88 KB\t2026-07-25",
   NULL
};

/*-------------------------------------------------------------------------
 * FillListBox - insert the sample rows, letting the listbox expand the
 * '\t' separators through the owner-draw path.
 *-------------------------------------------------------------------------*/
static void FillListBox(HWND hwndLb)
{
   int i;

   for (i = 0; apszItems[i] != NULL; i++)
      WinSendMsg(hwndLb, LM_INSERTITEM,
                 MPFROMSHORT(LIT_END),
                 MPFROMP(apszItems[i]));
}


/*-------------------------------------------------------------------------
 * ApplyOwndrawTheme - tint the client and the owner-draw listbox.
 *-------------------------------------------------------------------------*/
static void ApplyOwndrawTheme(HWND hwnd)
{
   PXpmTheme pth = XpmQueryTheme();
   RGB2      rgb;
   LONG      c;

   if (hwndList != NULLHANDLE)
   {
      c = (LONG)pth->clrCanvas;
      rgb.fcOptions = 0;
      rgb.bBlue  = (BYTE)(c & 0xFF);
      rgb.bGreen = (BYTE)((c >> 8) & 0xFF);
      rgb.bRed   = (BYTE)((c >> 16) & 0xFF);
      WinSetPresParam(hwndList, PP_BACKGROUNDCOLOR, sizeof(rgb), &rgb);

      c = (LONG)pth->clrText;
      rgb.bBlue  = (BYTE)(c & 0xFF);
      rgb.bGreen = (BYTE)((c >> 8) & 0xFF);
      rgb.bRed   = (BYTE)((c >> 16) & 0xFF);
      WinSetPresParam(hwndList, PP_FOREGROUNDCOLOR, sizeof(rgb), &rgb);
   }

   (void)hwnd;
}


int main(void)
{
   HAB   hab;
   HMQ   hmq;
   HWND  hwndFrame, hwndClient;
   QMSG  qmsg;
   ULONG flCreate;

   /* FCF_ICON is omitted: the sample binds no icon resource and
    * WinCreateStdWindow fails if FCF_ICON is set without one. */
   flCreate = FCF_SYSMENU | FCF_TITLEBAR | FCF_SIZEBORDER |
              FCF_MINMAX  | FCF_SHELLPOSITION | FCF_TASKLIST | FCF_MENU;

   hab = WinInitialize(0);
   hmq = WinCreateMsgQueue(hab, 0);

   WinRegisterClass(hab, CLSNAME, ClientWndProc, CS_SIZEREDRAW, 0);

   hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0L, &flCreate, CLSNAME,
                                   TITLE, 0L, NULLHANDLE, IDM_MENU,
                                   &hwndClient);

   if (hwndFrame != NULLHANDLE)
   {
      WinSetWindowPos(hwndFrame, HWND_TOP, 120, 120, 760, 360,
                      SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW);

      while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
         WinDispatchMsg(hab, &qmsg);
   }

   WinDestroyWindow(hwndFrame);
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
}


MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch (msg)
   {
      case WM_CREATE:
         hwndThemeMenu = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                                         FID_MENU);
         hwndList = WinCreateWindow(hwnd, WC_LISTBOX, "",
                        WS_VISIBLE | LS_OWNERDRAW,
                        0, 0, 0, 0,
                        hwnd, HWND_TOP, ID_LISTBOX, NULL, NULL);
         FillListBox(hwndList);
         ApplyOwndrawTheme(hwnd);
         return (MRESULT)FALSE;

      case WM_SIZE:
         WinSetWindowPos(hwndList, NULLHANDLE,
                         MARGIN, MARGIN,
                         SHORT1FROMMP(mp2) - 2 * MARGIN,
                         SHORT2FROMMP(mp2) - 2 * MARGIN,
                         SWP_MOVE | SWP_SIZE | SWP_SHOW);
         return (MRESULT)FALSE;

      /* The listbox asks its owner for the row geometry.  This arrives
       * during WinCreateWindow (before hwndList is assigned), so look the
       * listbox up by control id instead of relying on the static handle. */
      case WM_MEASUREITEM:
      {
         HWND hlb = WinWindowFromID(hwnd, ID_LISTBOX);

         if (hlb == NULLHANDLE)
            hlb = hwnd;
         return XpmInitLBWidthHeight(hlb, MAXIMUM_WIDTH, ITEM_CHARS,
                                     &lCharWidth);
      }

      /* The listbox asks its owner to paint each item. */
      case WM_DRAWITEM:
      {
         POWNERITEM poi = (POWNERITEM)mp2;
         PXpmTheme  pth = XpmQueryTheme();

         GpiCreateLogColorTable(poi->hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         GpiSetColor(poi->hps, (LONG)pth->clrText);
         GpiSetBackColor(poi->hps, (LONG)pth->clrCanvas);
         return XpmDrawLBItems(poi, lCharWidth, asTabStops);
      }

      case WM_ERASEBACKGROUND:
      {
         HPS   hps = (HPS)mp1;
         RECTL rcl;

         GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         WinQueryWindowRect(hwnd, &rcl);
         WinFillRect(hps, &rcl, (LONG)XpmQueryTheme()->clrCanvas);
         return (MRESULT)TRUE;
      }

      case WM_PAINT:
      {
         HPS   hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
         RECTL rcl;

         GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         WinQueryWindowRect(hwnd, &rcl);
         WinFillRect(hps, &rcl, (LONG)XpmQueryTheme()->clrCanvas);
         WinEndPaint(hps);
         return (MRESULT)FALSE;
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
            ApplyOwndrawTheme(hwnd);
            WinInvalidateRect(hwnd, NULLHANDLE, TRUE);
            return (MRESULT)FALSE;
         }
         break;

      case XpmWM_THEMECHANGED:
         ApplyOwndrawTheme(hwnd);
         WinInvalidateRect(hwnd, NULLHANDLE, TRUE);
         return (MRESULT)FALSE;

      case WM_CLOSE:
         WinPostMsg(hwnd, WM_QUIT, 0, 0);
         return (MRESULT)FALSE;
   }

   return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

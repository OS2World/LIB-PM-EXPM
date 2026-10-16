/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/shadebtn/shbsmp.c -- Shaded Buttons sample application.
 *
 * Shows the WC_shBUTTON window class: gradient-shaded push buttons
 * (with runtime and resource colour changes), rounded sides, vertical
 * buttons, icon / mini-icon buttons, check boxes, a three-state box and
 * radio buttons.  A non-pushable shaded button is used as a status bar.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WIN
#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include "shbsmp.h"
#include "expm.h"

MRESULT EXPENTRY MainDlg(HWND, ULONG, MPARAM, MPARAM);

static VOID SetStatus(HWND hwndDlg, PSZ pszText);
static VOID CenterDialog(HWND hwndDlg);
static VOID SelectRadio(HWND hwndDlg, USHORT idChecked);

#ifdef SHB_TRACE
#include <stdarg.h>
static FILE *g_log;
static PFNWP g_oldFrame, g_oldTB;
static VOID TLog(const char *fmt, ...) {
   va_list ap;
   if (!g_log) return;
   va_start(ap, fmt);
   vfprintf(g_log, fmt, ap);
   fprintf(g_log, "\n");
   va_end(ap);
   fflush(g_log);
}
static MRESULT EXPENTRY TraceFrame(HWND h, ULONG m, MPARAM p1, MPARAM p2) {
   if (m == WM_SYSCOMMAND) TLog("FRAME WM_SYSCOMMAND cmd=%04lx", (unsigned long)SHORT1FROMMP(p1));
   else if (m == WM_CLOSE) TLog("FRAME WM_CLOSE");
   else if (m == WM_ACTIVATE) TLog("FRAME WM_ACTIVATE f=%04lx", (unsigned long)SHORT1FROMMP(p1));
   else if (m == WM_MOVE) TLog("FRAME WM_MOVE");
   return g_oldFrame(h, m, p1, p2);
}
static MRESULT EXPENTRY TraceTB(HWND h, ULONG m, MPARAM p1, MPARAM p2) {
   if (m == WM_BUTTON1DOWN) TLog("TB WM_BUTTON1DOWN");
   else if (m == WM_BUTTON1UP) TLog("TB WM_BUTTON1UP");
   else if (m == WM_BUTTON1DBLCLK) TLog("TB WM_BUTTON1DBLCLK");
   else if (m == WM_BUTTON2DOWN) TLog("TB WM_BUTTON2DOWN");
   return g_oldTB(h, m, p1, p2);
}
static VOID APIENTRY ExitTrace(ULONG ulTerm) {
   TLog("EXITLIST term=%lu", (unsigned long)ulTerm);
   DosExitList(EXLST_EXIT, (PFNEXITLIST)NULL);
}
#endif

/*-------------------------------------------------------------------------
 * main() - register the class and run the sample window non-modally.
 *
 * The sample is a top-level dialog window loaded with WinLoadDlg and
 * driven by a normal message loop (as in the original Shaded Buttons
 * test programs).  Loading it non-modally - rather than with WinDlgBox -
 * lets the frame be dragged by its title bar normally.
 *-------------------------------------------------------------------------*/
int main(void) {
   HAB hab = WinInitialize(0);
   HMQ hmq = WinCreateMsgQueue(hab, 0);
   HWND hwndFrame = NULLHANDLE;
   QMSG qmsg;

#ifdef SHB_TRACE
   DosExitList(EXLST_ADD, ExitTrace);
#endif

   if (!WinShadeBtnInit()) {
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                    "Could not register the shaded-button window class.",
                    "Shaded Buttons", 0, MB_OK | MB_ERROR);
   } else {
      hwndFrame = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, MainDlg,
                             NULLHANDLE, DLG_MAIN, NULL);
      if (hwndFrame) {
         while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0)) {
            if (qmsg.msg == WM_QUIT)
               break;
            WinDispatchMsg(hab, &qmsg);
         }
#ifdef SHB_TRACE
         TLog("message loop ended");
#endif
         WinDestroyWindow(hwndFrame);
      }
   }

#ifdef SHB_TRACE
   TLog("main END");
#endif
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
}

/*-------------------------------------------------------------------------
 * MainDlg() - dialog procedure for the sample.
 *-------------------------------------------------------------------------*/
MRESULT EXPENTRY MainDlg(HWND hwndDlg, ULONG msg,
                         MPARAM mp1, MPARAM mp2) {
#ifdef SHB_TRACE
   TLog("DLG msg=%08lx mp1=%08lx mp2=%08lx", (unsigned long)msg,
        (unsigned long)LONGFROMMP(mp1), (unsigned long)LONGFROMMP(mp2));
#endif
   switch (msg) {

      case WM_INITDLG: {
         CHAR sz[80];
         ULONG ulVer = WinShadeBtnQueryVersion();

         sprintf(sz, "Shaded Buttons %d.%02d - press a button.",
                 (int)(ulVer / 100), (int)(ulVer % 100));
         SetStatus(hwndDlg, sz);

         /* Change a button colour at run time (shBM_SETCOLOR). */
         WinSendMsg(WinWindowFromID(hwndDlg, BTN_DEF),
                    shBM_SETCOLOR, MPFROMLONG(0x00FFEEAA),
                    MPFROMLONG(shBSC_BUTTONFACE));

         /* Start with the first radio button selected. */
         SelectRadio(hwndDlg, RAD_1);

         CenterDialog(hwndDlg);
#ifdef SHB_TRACE
         g_log = fopen("C:\\Home\\martin\\shbmsg.txt", "w");
         TLog("== session start ==");
         {
            CHAR cn[64];
            HWND fr = WinQueryWindow(hwndDlg, QW_PARENT);
            HWND gp = WinQueryWindow(fr, QW_PARENT);
            WinQueryClassName(hwndDlg, sizeof(cn), cn);
            TLog("hwndDlg=%08lx cls=%s", (unsigned long)hwndDlg, cn);
            WinQueryClassName(fr, sizeof(cn), cn);
            TLog("parent=%08lx cls=%s", (unsigned long)fr, cn);
            WinQueryClassName(gp, sizeof(cn), cn);
            TLog("grand =%08lx cls=%s", (unsigned long)gp, cn);
            TLog("tb(dlg)=%08lx tb(parent)=%08lx",
                 (unsigned long)WinWindowFromID(hwndDlg, FID_TITLEBAR),
                 (unsigned long)WinWindowFromID(fr, FID_TITLEBAR));
            g_oldFrame = WinSubclassWindow(fr, TraceFrame);
            g_oldTB = WinSubclassWindow(WinWindowFromID(fr, FID_TITLEBAR), TraceTB);
         }
#endif
         return (MRESULT)FALSE;
      }

      /* Push buttons send WM_COMMAND. */
      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1)) {
            case PB_OK:
            case PB_CANCEL:
               WinPostMsg(hwndDlg, WM_QUIT, MPVOID, MPVOID);
               return (MRESULT)FALSE;
            default: {
               CHAR szText[64], szMsg[96];
               HWND hwndBtn = WinWindowFromID(hwndDlg, SHORT1FROMMP(mp1));

               szText[0] = '\0';
               if (hwndBtn)
                  WinQueryWindowText(hwndBtn, sizeof(szText), szText);
               sprintf(szMsg, "Button %d pressed: %s",
                       (int)SHORT1FROMMP(mp1), szText);
               SetStatus(hwndDlg, szMsg);
               return (MRESULT)FALSE;
            }
         }
         return (MRESULT)FALSE;

      /* Check boxes and radio buttons send WM_CONTROL / BN_CLICKED. */
      case WM_CONTROL:
         if (SHORT2FROMMP(mp1) == BN_CLICKED) {
            USHORT id = SHORT1FROMMP(mp1);
            CHAR   szMsg[80];

            if (id >= RAD_1 && id <= RAD_3) {
               SelectRadio(hwndDlg, id);
            }

            switch (id) {
               case CHK_AUTO:
               case CHK_3STATE:
               case RAD_1:
               case RAD_2:
               case RAD_3: {
                  LONG lCheck = (LONG)WinSendMsg(WinWindowFromID(hwndDlg, id),
                                                 BM_QUERYCHECK, MPVOID, MPVOID);
                  PSZ  pszState = (lCheck == 1) ? "checked"
                                : (lCheck == 2) ? "indeterminate"
                                : "unchecked";
                  sprintf(szMsg, "Control %d is %s.", (int)id, pszState);
                  SetStatus(hwndDlg, szMsg);
                  break;
               }
               default:
                  break;
            }
         }
         return (MRESULT)FALSE;

      case WM_CLOSE:
         WinPostMsg(hwndDlg, WM_QUIT, MPVOID, MPVOID);
         return (MRESULT)FALSE;
   }
#ifdef SHB_TRACE
   {
      MRESULT mr;
      TLog("BEFORE defdlg msg=%08lx", (unsigned long)msg);
      mr = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
      TLog("AFTER  defdlg msg=%08lx mr=%08lx", (unsigned long)msg,
           (unsigned long)mr);
      return mr;
   }
#else
   return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
#endif
}

/*-------------------------------------------------------------------------
 * SetStatus() - write a message into the non-pushable status button.
 *-------------------------------------------------------------------------*/
static VOID SetStatus(HWND hwndDlg, PSZ pszText) {
   HWND hwndStatus = WinWindowFromID(hwndDlg, ST_BAR);

   if (hwndStatus)
      WinSetWindowText(hwndStatus, pszText);
}

/*-------------------------------------------------------------------------
 * SelectRadio() - make one radio button checked and clear the others.
 *
 * The shaded-button class toggles radio buttons independently (it does
 * not manage a radio group), so the dialog does it here.
 *-------------------------------------------------------------------------*/
static VOID SelectRadio(HWND hwndDlg, USHORT idChecked) {
   USHORT id;

   for (id = RAD_1; id <= RAD_3; id++)
      WinSendMsg(WinWindowFromID(hwndDlg, id), BM_SETCHECK,
                 MPFROMSHORT(id == idChecked ? 1 : 0), MPVOID);
}

/*-------------------------------------------------------------------------
 * CenterDialog() - position the dialog in the middle of the screen.
 *-------------------------------------------------------------------------*/
static VOID CenterDialog(HWND hwndDlg) {
   RECTL rclScreen;
   RECTL rclDlg;

   WinQueryWindowRect(HWND_DESKTOP, &rclScreen);
   WinQueryWindowRect(hwndDlg, &rclDlg);
   WinSetWindowPos(hwndDlg, HWND_TOP,
                   (rclScreen.xRight - rclDlg.xRight) / 2,
                   (rclScreen.yTop   - rclDlg.yTop)   / 2,
                   0, 0, SWP_MOVE | SWP_SHOW);
}


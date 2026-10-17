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
#include <os2.h>
#include <stdio.h>
#include "shbsmp.h"
#include "expm.h"

MRESULT EXPENTRY MainDlg(HWND, ULONG, MPARAM, MPARAM);

static VOID SetStatus(HWND hwndDlg, PSZ pszText);
static VOID CenterDialog(HWND hwndDlg);
static VOID SelectRadio(HWND hwndDlg, USHORT idChecked);

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

   if (!XpmShadeBtnInit()) {
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
         WinDestroyWindow(hwndFrame);
      }
   }

   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
}

/*-------------------------------------------------------------------------
 * MainDlg() - window procedure for the sample dialog.
 *-------------------------------------------------------------------------*/
MRESULT EXPENTRY MainDlg(HWND hwndDlg, ULONG msg,
                         MPARAM mp1, MPARAM mp2) {
   switch (msg) {

      case WM_INITDLG: {
         CHAR sz[80];
         ULONG ulVer = XpmShadeBtnQueryVersion();

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
   return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
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


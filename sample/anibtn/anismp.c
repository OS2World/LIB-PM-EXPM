/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/anibtn/anismp.c -- AniButtn sample application.
 *
 * Displays a small "toolbar" dialog holding twelve animated push
 * buttons made with XpmCreateAniButton().  Moving the pointer over a
 * button shows a help string in the status line at the bottom; clicking
 * a button pops up a message box (the EXIT button ends the program).
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WIN
#define INCL_WINMENUS
#define INCL_GPI
#include <os2.h>
#include "anismp.h"
#include "expm.h"

MRESULT EXPENTRY ToolBarDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static VOID CreateButtons(HWND hwndDlg);
static VOID DeleteButtons(HWND hwndDlg);
static VOID CenterDialog(HWND hwndDlg);
static VOID ApplyAniTheme(HWND hwndDlg);

static BOOL bDark = FALSE;
static HWND hwndMenu = NULLHANDLE;

static VOID SetRgb(HWND hwnd, ULONG idMode, ULONG ulColor) {
   RGB2 rgb;

   rgb.fcOptions = 0;
   rgb.bBlue  = (BYTE)(ulColor & 0xFF);
   rgb.bGreen = (BYTE)((ulColor >> 8) & 0xFF);
   rgb.bRed   = (BYTE)((ulColor >> 16) & 0xFF);
   WinSetPresParam(hwnd, idMode, sizeof(rgb), &rgb);
}

static VOID ApplyAniTheme(HWND hwndDlg) {
   PXpmTheme pth = XpmQueryTheme();
   HWND      hCtl;

   SetRgb(hwndDlg, PP_BACKGROUNDCOLOR, pth->clrCanvas);

   hCtl = WinWindowFromID(hwndDlg, TX_HELP);
   if (hCtl) {
      SetRgb(hCtl, PP_BACKGROUNDCOLOR, pth->clrCanvas);
      SetRgb(hCtl, PP_FOREGROUNDCOLOR, pth->clrText);
   }

   hCtl = WinWindowFromID(hwndDlg, ID_THEME_CHECK);
   if (hCtl) {
      SetRgb(hCtl, PP_BACKGROUNDCOLOR, pth->clrCanvas);
      SetRgb(hCtl, PP_FOREGROUNDCOLOR, pth->clrText);
   }
}

/*-------------------------------------------------------------------------
 * main() - run the toolbar dialog modally.
 *-------------------------------------------------------------------------*/
int main(void) {
   HAB hab = WinInitialize(0);
   HMQ hmq = WinCreateMsgQueue(hab, 0);

   WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, ToolBarDlg,
             NULLHANDLE, DLG_TOOLBAR, NULL);

   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
}

/*-------------------------------------------------------------------------
 * ToolBarDlg() - dialog procedure for the toolbar.
 *-------------------------------------------------------------------------*/
MRESULT EXPENTRY ToolBarDlg(HWND hwndDlg, ULONG msg,
                            MPARAM mp1, MPARAM mp2) {
   switch (msg) {

      /* Create the animated buttons, then centre the dialog. */
      case WM_INITDLG:
         CreateButtons(hwndDlg);
         ApplyAniTheme(hwndDlg);
         CenterDialog(hwndDlg);
         return (MRESULT)FALSE;

       /* A push button was clicked: report it, except for EXIT. */
       case WM_COMMAND:
          switch (SHORT1FROMMP(mp1)) {
             case ID_THEME_CHECK:
                {
                   XpmTheme th;

                   bDark = !bDark;
                   if (bDark)
                      XpmThemeDark(&th);
                   else
                      XpmThemeDefault(&th);
                   XpmSetTheme(&th);
                   WinCheckButton(hwndDlg, ID_THEME_CHECK, bDark);
                   ApplyAniTheme(hwndDlg);
                   WinInvalidateRect(hwndDlg, NULLHANDLE, TRUE);
                   return (MRESULT)FALSE;
                }
             case TBB_EXIT:
               WinSendMsg(hwndDlg, WM_CLOSE, MPVOID, MPVOID);
               break;
            case TBB_FILE:
            case TBB_NEW:
            case TBB_NEXT:
            case TBB_LAST:
            case TBB_NA1:
            case TBB_NA2:
            case TBB_NA3:
            case TBB_NA4:
            case TBB_NA5:
            case TBB_NA6:
            case TBB_NA7:
               WinMessageBox(HWND_DESKTOP, hwndDlg,
                             "AniButtn button clicked.",
                             "AniButtn sample",
                             0, MB_OK | MB_INFORMATION);
               break;
         }
         return (MRESULT)FALSE;

      case WM_ERASEBACKGROUND: {
         HPS   hps = (HPS)mp1;
         RECTL rcl;

         GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
         WinQueryWindowRect(hwndDlg, &rcl);
         WinFillRect(hps, &rcl, (LONG)XpmQueryTheme()->clrCanvas);
         return (MRESULT)TRUE;
      }

      case XpmWM_THEMECHANGED:
         ApplyAniTheme(hwndDlg);
         WinInvalidateRect(hwndDlg, NULLHANDLE, TRUE);
         return (MRESULT)FALSE;

      /* Release the buttons before the dialog goes away. */
      case WM_CLOSE:
         DeleteButtons(hwndDlg);
         WinDismissDlg(hwndDlg, 0);
         return (MRESULT)FALSE;
   }
   return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
}

/*-------------------------------------------------------------------------
 * CreateButtons() - turn the twelve user buttons into animated buttons.
 *
 * The five left-hand buttons get their own bitmap pair; the seven "not
 * available" buttons share one pair, as in the original sample.
 *-------------------------------------------------------------------------*/
static VOID CreateButtons(HWND hwndDlg) {
   XpmCreateAniButton(hwndDlg, TBB_FILE, BMP_ABT_UP, BMP_ABT_DN, BMP_CX, BMP_CY,
                      "About this program.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NEW,  BMP_INF_UP, BMP_INF_DN, BMP_CX, BMP_CY,
                      "Show information about the current item.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_EXIT, BMP_EXT_UP, BMP_EXT_DN, BMP_CX, BMP_CY,
                      "Close the AniButtn sample.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NEXT, BMP_NXT_UP, BMP_NXT_DN, BMP_CX, BMP_CY,
                      "Go to the next item.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_LAST, BMP_LST_UP, BMP_LST_DN, BMP_CX, BMP_CY,
                      "Go to the previous item.", TX_HELP);

   XpmCreateAniButton(hwndDlg, TBB_NA1, BMP_NA_UP, BMP_NA_DN, BMP_CX, BMP_CY,
                      "This button is not used in the sample.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NA2, BMP_NA_UP, BMP_NA_DN, BMP_CX, BMP_CY,
                      "This button is not used in the sample.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NA3, BMP_NA_UP, BMP_NA_DN, BMP_CX, BMP_CY,
                      "This button is not used in the sample.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NA4, BMP_NA_UP, BMP_NA_DN, BMP_CX, BMP_CY,
                      "This button is not used in the sample.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NA5, BMP_NA_UP, BMP_NA_DN, BMP_CX, BMP_CY,
                      "This button is not used in the sample.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NA6, BMP_NA_UP, BMP_NA_DN, BMP_CX, BMP_CY,
                      "This button is not used in the sample.", TX_HELP);
   XpmCreateAniButton(hwndDlg, TBB_NA7, BMP_NA_UP, BMP_NA_DN, BMP_CX, BMP_CY,
                      "This button is not used in the sample.", TX_HELP);
}

/*-------------------------------------------------------------------------
 * DeleteButtons() - release every animated button again.
 *-------------------------------------------------------------------------*/
static VOID DeleteButtons(HWND hwndDlg) {
   XpmDeleteAniButton(hwndDlg, TBB_FILE);
   XpmDeleteAniButton(hwndDlg, TBB_NEW);
   XpmDeleteAniButton(hwndDlg, TBB_EXIT);
   XpmDeleteAniButton(hwndDlg, TBB_NEXT);
   XpmDeleteAniButton(hwndDlg, TBB_LAST);
   XpmDeleteAniButton(hwndDlg, TBB_NA1);
   XpmDeleteAniButton(hwndDlg, TBB_NA2);
   XpmDeleteAniButton(hwndDlg, TBB_NA3);
   XpmDeleteAniButton(hwndDlg, TBB_NA4);
   XpmDeleteAniButton(hwndDlg, TBB_NA5);
   XpmDeleteAniButton(hwndDlg, TBB_NA6);
   XpmDeleteAniButton(hwndDlg, TBB_NA7);
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

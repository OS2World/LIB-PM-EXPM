/*=========================================================================
 * test.c - Sample application for the EDITCOL.DLL color wheel dialog.
 *
 * Creates a standard PM window.  Clicking any mouse button on the client
 * area opens the "Edit Color" dialog.  While the dialog is open, the
 * client window is repainted live as the user drags the color wheel
 * (WM_COLORWHEELCHANGED).  After the dialog closes, the window title
 * shows the returned RGB values for five seconds.
 *=========================================================================*/

#define SZ_CLIENTCLASS  "testclient"
#define SZ_TESTTITLE    "\"Edit color\" test"
#define TID_TITLE       100            /* timer ID for title reset */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <editcol.h>

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/*-------------------------------------------------------------------------
 * main() - Initialize PM, register the client class, create the standard
 * window, and run the message loop.
 *-------------------------------------------------------------------------*/
int main(void) {
   /* Standard window flags minus accelerator table and menu bar. */
   ULONG flFrame = FCF_STANDARD & ~(FCF_ACCELTABLE | FCF_MENU);
   HWND  hFrame, hClient;
   QMSG  qmsg;
   HAB   hab = WinInitialize(0);
   HMQ   hmq = WinCreateMsgQueue(hab, 0);

   WinRegisterClass(hab, SZ_CLIENTCLASS, ClientWndProc,
                    CS_SIZEREDRAW, 0);

   if (NULLHANDLE != (hFrame = WinCreateStdWindow(
                                  HWND_DESKTOP, WS_VISIBLE,
                                  &flFrame, SZ_CLIENTCLASS,
                                  SZ_TESTTITLE, 0L, NULLHANDLE,
                                  1, &hClient))) {
      while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
         WinDispatchMsg(hab, &qmsg);
   }

   WinDestroyWindow(hFrame);
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
   return 0;
}

/*-------------------------------------------------------------------------
 * ClientWndProc() - Window procedure for the test client area.
 *-------------------------------------------------------------------------*/
MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   static CLR clr;        /* current background color (persists across msgs) */
   HPS    hps;
   RECTL  rcl;
   char   achTxt[64];

   switch (msg) {

      /* Set the initial background color to a medium grey (204, 204, 204). */
      case WM_CREATE:
         clr.rgb.red = 204;
         clr.rgb.grn = 204;
         clr.rgb.blu = 204;
         break;

      /* Live notification from the "Edit Color" dialog: the user moved the
       * color wheel while the dialog is still open.  Update the background
       * and show the current RGB values in the title bar. */
      case WM_COLORWHEELCHANGED:
      case WM_COLORWHEELCHANGED3:
         clr.lClr = (LONG)mp1;
         WinInvalidateRect(hwnd, NULL, FALSE);
         sprintf(achTxt, "Color (RGB): %d - %d - %d",
                 clr.rgb.red, clr.rgb.grn, clr.rgb.blu);
         WinSetWindowText(WinQueryWindow(hwnd, QW_PARENT), achTxt);
         break;

      /* Any mouse button click opens the "Edit Color" dialog.
       * The dialog is parented to the desktop and owned by this client
       * window so that WM_COLORWHEELCHANGED is delivered here.
       * After the dialog closes, show the final color in the title bar
       * and start a 5-second timer to restore the original title. */
      case WM_BUTTON1CLICK:
      case WM_BUTTON2CLICK:
      case WM_BUTTON3CLICK:
         clr.lClr = WinEditColorDlg(HWND_DESKTOP, hwnd,
                                     (COLOR)clr.lClr, NULL);
         WinInvalidateRect(hwnd, NULL, FALSE);
         sprintf(achTxt, "Returned color (RGB): %d - %d - %d",
                 clr.rgb.red, clr.rgb.grn, clr.rgb.blu);
         WinSetWindowText(WinQueryWindow(hwnd, QW_PARENT), achTxt);
         WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_TITLE, 5000);
         return (MRESULT)TRUE;

      /* Timer fired: restore the window title and stop the timer. */
      case WM_TIMER:
         if ((USHORT)mp1 == TID_TITLE) {
            WinSetWindowText(WinQueryWindow(hwnd, QW_PARENT), SZ_TESTTITLE);
            WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, TID_TITLE);
         }
         break;

      /* Paint the client area with the current background color.
       * GpiCreateLogColorTable switches the HPS to RGB color mode so that
       * WinFillRect interprets clr.lClr as a direct 0x00RRGGBB value. */
      case WM_PAINT:
         hps = WinBeginPaint(hwnd, NULLHANDLE, &rcl);
         GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
         WinFillRect(hps, &rcl, clr.lClr);
         WinEndPaint(hps);
         break;

      default:
         return WinDefWindowProc(hwnd, msg, mp1, mp2);
   }
   return (MRESULT)FALSE;
}

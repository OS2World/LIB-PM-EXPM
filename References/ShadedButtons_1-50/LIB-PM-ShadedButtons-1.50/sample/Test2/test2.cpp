// Test program for the shaded button class

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include "..\..\h\shadebtn.h"
#include "Test2.H"

typedef union {
  ULONG Index;
  struct {
    UCHAR blue,green,red;
    UCHAR dummy;
  } rgb;
} CLR;

MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY AboutDlgProc(HWND, ULONG, MPARAM, MPARAM);

int main() {
  static CHAR szClientClass[] = "ButtonTest";
  static ULONG flFrameFlags= FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER | FCF_MINMAX | FCF_SHELLPOSITION | FCF_TASKLIST;
  HAB          hab;
  HMQ          hmq;
  HWND         hWndFrame, hWndClient;
  QMSG         qmsg;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);

  shBtnInit();
  WinRegisterClass(hab, szClientClass, ClientWndProc, CS_SIZEREDRAW, 0);
  hWndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &flFrameFlags, szClientClass, NULL, 0L, 0, 0, &hWndClient);

  while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
    WinDispatchMsg(hab, &qmsg);

  WinDestroyWindow(hWndFrame);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return 0;
}

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  static HWND hwndFrame, hwndButton[10];
  static INT  cxClient, cyClient, cyChar;
  static LONG ButtonWidth, ButtonHeight;
  static CLR  buttonColour;
  static ULONG WinBGColor;
  CHAR        szBuffer[32];
  FONTMETRICS fm;
  HAB         hab;
  HPS         hps;
  INT         id;
  RECTL       rcl;

  switch(msg) {
  case WM_CREATE:
    WinBGColor=0x00CCCCCC;
    hab = WinQueryAnchorBlock(hwnd);
    hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
    hps = WinGetPS(hwnd);
    GpiQueryFontMetrics(hps, sizeof(fm),&fm);
    cyChar = fm.lMaxBaselineExt;
    WinReleasePS(hps);
    buttonColour.Index = 0x00CCCCCC;

// Buttons should be big enough to hold the pointer, with a 4 pixel border
    ButtonWidth = WinQuerySysValue(HWND_DESKTOP, SV_CXPOINTER) + 4;
    ButtonHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYPOINTER) + 4;
    hwndButton[0] = WinCreateWindow(hwnd, WC_shBUTTON, "#100,110 ", WS_VISIBLE | shBS_NORIGHTLINE | shBS_ICON, 0, 0, ButtonWidth, ButtonHeight, hwnd, HWND_BOTTOM, 0, NULL, NULL);
    hwndButton[1] = WinCreateWindow(hwnd, WC_shBUTTON, "#101,111 ", WS_VISIBLE | shBS_NOLEFTLINE | shBS_ICON, 0, 0, ButtonWidth, ButtonHeight, hwnd, HWND_BOTTOM, 1, NULL, NULL);

// This is the status bar
    hwndButton[2] = WinCreateWindow(hwnd, WC_shBUTTON, "Status Bar", WS_VISIBLE | shBS_NOPUSH, 0, 0, 0, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 2, NULL, NULL);
    sprintf(szBuffer, "Red: %d Green: %d Blue: %d", buttonColour.rgb.red, buttonColour.rgb.green, buttonColour.rgb.blue);
    WinSetWindowText(hwndButton[2], szBuffer);

// Buttons to change the colour of the about button
    hwndButton[3] = WinCreateWindow(hwnd, WC_shBUTTON, "<", WS_VISIBLE | shBS_NORIGHTLINE, 0, 0, ButtonWidth, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 3, NULL, NULL);
    hwndButton[4] = WinCreateWindow(hwnd, WC_shBUTTON, ">", WS_VISIBLE | shBS_NOLEFTLINE, 0, 0, ButtonWidth, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 4, NULL, NULL);
    hwndButton[5] = WinCreateWindow(hwnd, WC_shBUTTON, "<", WS_VISIBLE | shBS_NORIGHTLINE, 0, 0, ButtonWidth, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 5, NULL, NULL);
    hwndButton[6] = WinCreateWindow(hwnd, WC_shBUTTON, ">", WS_VISIBLE | shBS_NOLEFTLINE, 0, 0, ButtonWidth, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 6, NULL, NULL);
    hwndButton[7] = WinCreateWindow(hwnd, WC_shBUTTON, "<", WS_VISIBLE | shBS_NORIGHTLINE, 0, 0, ButtonWidth, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 7, NULL, NULL);
    hwndButton[8] = WinCreateWindow(hwnd, WC_shBUTTON, ">", WS_VISIBLE | shBS_NOLEFTLINE, 0, 0, ButtonWidth, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 8, NULL, NULL);
// info about this program
    hwndButton[9] = WinCreateWindow(hwnd, WC_shBUTTON, "About", WS_VISIBLE | shBS_RIGHTROUND | shBS_LEFTROUND | shBS_HELP, 0, 0, 2 * ButtonWidth, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 9, NULL, NULL);

// change the colour of the buttons
    WinSendMsg(hwndButton[3], shBM_SETCOLOR, MPFROMLONG(0x00FFCCCC), MPFROMLONG(shBSC_BUTTONFACE));
    WinSendMsg(hwndButton[4], shBM_SETCOLOR, MPFROMLONG(0x00FFCCCC), MPFROMLONG(shBSC_BUTTONFACE));
    WinSendMsg(hwndButton[5], shBM_SETCOLOR, MPFROMLONG(0x00CCFFCC), MPFROMLONG(shBSC_BUTTONFACE));
    WinSendMsg(hwndButton[6], shBM_SETCOLOR, MPFROMLONG(0x00CCFFCC), MPFROMLONG(shBSC_BUTTONFACE));
    WinSendMsg(hwndButton[7], shBM_SETCOLOR, MPFROMLONG(0x00CCCCFF), MPFROMLONG(shBSC_BUTTONFACE));
    WinSendMsg(hwndButton[8], shBM_SETCOLOR, MPFROMLONG(0x00CCCCFF), MPFROMLONG(shBSC_BUTTONFACE));
    WinSendMsg(hwndButton[9], shBM_SETCOLOR, MPFROMLONG(buttonColour.Index), MPFROMLONG(shBSC_BUTTONFACE));
    return 0;

  case WM_SIZE:
    hps = WinGetPS(hwnd);
    GpiQueryFontMetrics(hps, sizeof(fm),&fm);
    cyChar = fm.lMaxBaselineExt;
    WinReleasePS(hps);
    cxClient = SHORT1FROMMP(mp2);
    cyClient = SHORT2FROMMP(mp2);

// Center the buttons
    for(id = 0; id < 2; id++)
      WinSetWindowPos(hwndButton[id], NULLHANDLE, (cxClient >> 1) + ButtonWidth * (id - 1), (cyClient - ButtonHeight) >> 1, 0, 0, SWP_MOVE);
    for(id = 3; id < 9; id++)
      WinSetWindowPos(hwndButton[id], NULLHANDLE, (cxClient >> 1) + ButtonWidth * (-(id % 2)), ((cyClient + ButtonHeight) >> 1) + (3 - (id-1) / 2) * cyChar * 5 / 4, 0, 0, SWP_MOVE);
    WinSetWindowPos(hwndButton[9], NULLHANDLE, (cxClient >> 1) - ButtonWidth, ((cyClient - ButtonHeight) >> 1) - cyChar * 15 / 8, 0, 0, SWP_MOVE);

// make the status bar span the width of the window
    WinSetWindowPos(hwndButton[2], NULLHANDLE, 0, 0, cxClient, cyChar * 5 / 4, SWP_SIZE);

// Enable "Smaller" button if window is large enough for the buttons
    if (!WinIsWindowEnabled(hwndButton[0]) && ((9 * cxClient / 10) >= (2 * ButtonWidth)) && ((9  * cyClient / 10) >= (ButtonHeight + cyChar * 15 / 2))) {
      WinEnableWindow(hwndButton[0], TRUE);
      WinEnableWindow(hwndButton[9], TRUE);
    }

// Disable "Smaller" button if window is too small for the buttons
    if (WinIsWindowEnabled(hwndButton[0]) && (((9 * cxClient / 10) < (2 * ButtonWidth)) || ((9  * cyClient / 10) < (ButtonHeight + cyChar * 15 / 2)))) {
      WinEnableWindow(hwndButton[0], FALSE);
      WinEnableWindow(hwndButton[9], FALSE);
    }
    return 0;

  case WM_COMMAND:
    WinQueryWindowRect(hwnd, &rcl);
    WinMapWindowPoints(hwnd, HWND_DESKTOP, (PPOINTL) &rcl, 2);

    switch(COMMANDMSG(&msg)->cmd) {
    case 0: // "Smaller"
      rcl.xLeft   += cxClient / 20;
      rcl.xRight  -= cxClient / 20;
      rcl.yBottom += cyClient / 20;
      rcl.yTop    -= cyClient / 20;
      break;
    case 1: // "Larger"
      rcl.xLeft   -= cxClient / 20;
      rcl.xRight  += cxClient / 20;
      rcl.yBottom -= cyClient / 20;
      rcl.yTop    += cyClient / 20;
      break;
    case 3: // change red
      buttonColour.rgb.red--;
      if (buttonColour.rgb.red == 0)
	WinEnableWindow(hwndButton[3], FALSE);
      if (!WinIsWindowEnabled(hwndButton[4]))
	WinEnableWindow(hwndButton[4], TRUE);
      break;
    case 4:
      buttonColour.rgb.red++;
      if (buttonColour.rgb.red == 255)
	WinEnableWindow(hwndButton[4], FALSE);
      if (!WinIsWindowEnabled(hwndButton[3]))
	WinEnableWindow(hwndButton[3], TRUE);
      break;
    case 5: // change green
      buttonColour.rgb.green--;
      if (buttonColour.rgb.green == 0)
	WinEnableWindow(hwndButton[5], FALSE);
      if (!WinIsWindowEnabled(hwndButton[6]))
	WinEnableWindow(hwndButton[6], TRUE);
      break;
    case 6:
      buttonColour.rgb.green++;
      if (buttonColour.rgb.green == 255)
	WinEnableWindow(hwndButton[6], FALSE);
      if (!WinIsWindowEnabled(hwndButton[5]))
	WinEnableWindow(hwndButton[5], TRUE);
      break;
    case 7:// change blue
      buttonColour.rgb.blue--;
      if (buttonColour.rgb.blue == 0)
	WinEnableWindow(hwndButton[7], FALSE);
      if (!WinIsWindowEnabled(hwndButton[8]))
	WinEnableWindow(hwndButton[8], TRUE);
      break;
    case 8:
      buttonColour.rgb.blue++;
      if (buttonColour.rgb.blue == 255)
	WinEnableWindow(hwndButton[8], FALSE);
      if (!WinIsWindowEnabled(hwndButton[7]))
	WinEnableWindow(hwndButton[7], TRUE);
      break;
    }
    sprintf(szBuffer, "Red: %d Green: %d Blue: %d", buttonColour.rgb.red, buttonColour.rgb.green, buttonColour.rgb.blue);
    WinSetWindowText(hwndButton[2], szBuffer);
    WinCalcFrameRect(hwndFrame, &rcl, FALSE);
    WinSetWindowPos(hwndFrame, NULLHANDLE, rcl.xLeft, rcl.yBottom, rcl.xRight - rcl.xLeft, rcl.yTop - rcl.yBottom, SWP_MOVE | SWP_SIZE);
    WinPostMsg(hwndButton[9], shBM_SETCOLOR, MPFROMLONG(buttonColour.Index), NULL);
    return 0;

  case WM_HELP:
    WinDlgBox(HWND_DESKTOP, hwnd, AboutDlgProc, NULLHANDLE, IDD_ABOUT, NULL);
    return 0;

  case WM_ERASEBACKGROUND:
    GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
    WinFillRect((HPS)LONGFROMMP(mp1), (PRECTL)PVOIDFROMMP(mp2), WinBGColor);
    return 0;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
	switch(msg) {
		case WM_COMMAND:
			switch(COMMANDMSG(&msg)->cmd) {
				case DID_OK:
					WinDismissDlg(hwnd, TRUE);
					return 0;
			}
			break;
	}
	return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

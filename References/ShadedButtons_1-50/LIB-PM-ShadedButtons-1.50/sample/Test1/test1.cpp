// Test program for the shaded button class

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include "..\..\h\shadebtn.h"
#include "Test1.H"

typedef union {
  ULONG Index;
  struct {
    UCHAR blue,green,red;
    UCHAR dummy;
  } rgb;
} CLR;

MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY AboutDlgProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY LineDlgProc(HWND, ULONG, MPARAM, MPARAM);

int main() {
  HAB          hab;
  HMQ          hmq;
  HWND         hWndFrame;
  QMSG         qmsg;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);

  shBtnInit();
  WinRegisterClass(hab, CLIENTCLASS, ClientWndProc, 0L, 0);
  hWndFrame = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, NULL, NULLHANDLE,
			 IDD_MAIN, NULL);
  WinSetFocus(HWND_DESKTOP, WinWindowFromID(hWndFrame, FID_CLIENT));

  while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
    WinDispatchMsg(hab, &qmsg);

  WinDestroyWindow(hWndFrame);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return 0;
}

ULONG Line, LineCl;
CLR Colour, LineClr;

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  static HWND    hwndStatusBar;
  static ULONG   cyChar, cxChar;
  static RECTL   rclColor;
  static POINTL  aptlLine[5];
  CHAR           szBuffer[32];
  FONTMETRICS    fm;
  HPS            hps;
  RECTL          rcl;

  switch(msg) {
  case WM_CREATE:
    hps = WinGetPS(hwnd);
    GpiQueryFontMetrics(hps, sizeof(fm),&fm);
    cyChar = fm.lMaxBaselineExt;
    cxChar = fm.lAveCharWidth;
    WinReleasePS(hps);
    rclColor.xLeft = cxChar * 25 / 2;
    rclColor.xRight = cxChar * 45 / 2;
    rclColor.yBottom = cyChar * 15 / 4;
    rclColor.yTop = cyChar * 55 / 8;
    aptlLine[0].x = aptlLine[3].x = aptlLine[4].x = rclColor.xLeft - 5;
    aptlLine[1].x = aptlLine[2].x = rclColor.xRight + 5;
    aptlLine[0].y = aptlLine[1].y = aptlLine[4].y = rclColor.yTop + 5;
    aptlLine[2].y = aptlLine[3].y = rclColor.yBottom - 5;

    Colour.Index = 0x00ffcccc;
    Line = 0;
    LineClr.Index = 0x00000000;
    LineCl = 0;

    hwndStatusBar = WinCreateWindow(hwnd, WC_shBUTTON, "Status Bar", WS_VISIBLE | shBS_NOPUSH, 0, 0, 0, cyChar * 5 / 4, hwnd, HWND_BOTTOM, 203, NULL, NULL);
    WinQueryWindowRect(hwnd, &rcl);
    WinSetWindowPos(hwndStatusBar, NULLHANDLE, 0, 0, rcl.xRight, cyChar * 5 / 4, SWP_SIZE);
    return 0;

  case WM_COMMAND:
    switch (COMMANDMSG(&msg)->cmd) {
    case 201:
      WinDlgBox(HWND_DESKTOP, hwnd, AboutDlgProc, NULLHANDLE, IDD_ABOUT, NULL);
      return 0;
    case 202:
      if(WinDlgBox(HWND_DESKTOP, hwnd, LineDlgProc, NULLHANDLE, IDD_OUTLINE, NULL))
	WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 211:
      Colour.rgb.red++;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 212:
      Colour.rgb.red += 10;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 213:
      Colour.rgb.red -= 10;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 214:
      Colour.rgb.red--;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 221:
      Colour.rgb.green++;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 222:
      Colour.rgb.green += 10;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 223:
      Colour.rgb.green -= 10;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 224:
      Colour.rgb.green--;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 231:
      Colour.rgb.blue++;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 232:
      Colour.rgb.blue += 10;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 233:
      Colour.rgb.blue -= 10;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    case 234:
      Colour.rgb.blue--;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;
    }
    break;

  case WM_ERASEBACKGROUND:
    return MRFROMSHORT(1);

  case WM_PAINT:
    hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
    GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
    WinFillRect(hps, &rclColor, Colour.Index);
    GpiSetLineType(hps, 0);
    GpiSetColor(hps, 0x00cccccc);
    GpiMove(hps, aptlLine);
    GpiPolyLine(hps, 4L, aptlLine+1);
    GpiSetColor(hps, LineClr.Index);
    GpiSetLineType(hps, Line);
    GpiPolyLine(hps, 4L, aptlLine+1);
    WinEndPaint(hps);
    sprintf(szBuffer, "Red: %d Green: %d Blue: %d", Colour.rgb.red, Colour.rgb.green, Colour.rgb.blue);
    WinSetWindowText(hwndStatusBar, szBuffer);
    return 0;

  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  static char szBuffer[18];
  switch(msg) {
  case WM_INITDLG:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DID_OK));
    sprintf(szBuffer, "DLL version: %4.2f", shBtnQueryVersion() / 100.0);
    WinSetWindowText(WinWindowFromID(hwnd, 301), szBuffer);
    return MRFROMSHORT(1);
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


MRESULT EXPENTRY LineDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  static ULONG curClr, curLine;

  switch(msg) {
  case WM_INITDLG:
    curClr = LineCl + IDB_BLACK;
    curLine = Line + IDB_SOLID;
    WinCheckButton(hwnd, curClr, TRUE);
    WinCheckButton(hwnd, curLine, TRUE);
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, curClr));
    return MRFROMSHORT(1);
  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) >= IDB_BLACK && SHORT1FROMMP(mp1) <= IDB_BLACK+15) {
      WinCheckButton(hwnd, curClr, FALSE);
      curClr = SHORT1FROMMP(mp1);
      WinCheckButton(hwnd, curClr, TRUE);
    } else
      if (SHORT1FROMMP(mp1) >= IDB_SOLID && SHORT1FROMMP(mp1) <= IDB_SOLID+7) {
	WinCheckButton(hwnd, curLine, FALSE);
	curLine = SHORT1FROMMP(mp1);
	WinCheckButton(hwnd, curLine, TRUE);
      }
    return 0;
  case WM_COMMAND:
    switch(COMMANDMSG(&msg)->cmd) {
    case DID_OK:
      LineCl = curClr - IDB_BLACK;
      Line = curLine - IDB_SOLID;
      switch (LineCl) {
      case 0:
	LineClr.Index = 0x00000000;
	break;
      case 1:
	LineClr.Index = 0x00000080;
	break;
      case 2:
	LineClr.Index = 0x00800000;
	break;
      case 3:
	LineClr.Index = 0x00800080;
	break;
      case 4:
	LineClr.Index = 0x00008000;
	break;
      case 5:
	LineClr.Index = 0x00008080;
	break;
      case 6:
	LineClr.Index = 0x00808000;
	break;
      case 7:
	LineClr.Index = 0x00cccccc;
	break;
      case 8:
	LineClr.Index = 0x00808080;
	break;
      case 9:
	LineClr.Index = 0x000000ff;
	break;
      case 10:
	LineClr.Index = 0x00ff0000;
	break;
      case 11:
	LineClr.Index = 0x00ff00ff;
	break;
      case 12:
	LineClr.Index = 0x0000ff00;
	break;
      case 13:
	LineClr.Index = 0x0000ffff;
	break;
      case 14:
	LineClr.Index = 0x00ffff00;
	break;
      case 15:
	LineClr.Index = 0x00ffffff;
	break;
      }
      WinDismissDlg(hwnd, TRUE);
      return 0;
    case DID_CANCEL:
      WinDismissDlg(hwnd, FALSE);
      return 0;
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


#define INCL_WIN
#include <os2.h>
#include <stdio.h>
#include "Test3.h"
#include "../../h/shadebtn.h"

#define ID_TIMER 1

MRESULT EXPENTRY ClientWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY AboutDlgProc(HWND, ULONG, MPARAM, MPARAM);

int curIco = 0;
char *Icons[] = {"#1", "#2", "#3", "#4"};

int main() {
  static CHAR szClientClass[] = "ButtonTest";
  static ULONG flFrameFlags= FCF_TITLEBAR | FCF_SYSMENU | FCF_MINBUTTON | FCF_SHELLPOSITION | FCF_TASKLIST | FCF_DLGBORDER;
  HAB          hab;
  HMQ          hmq;
  HWND         hWndFrame, hWndClient;
  QMSG         qmsg;
  ULONG        idTimer;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);

  shBtnInit();
  WinRegisterClass(hab, szClientClass, ClientWndProc, CS_SIZEREDRAW, 0);
  hWndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &flFrameFlags, szClientClass, NULL, 0L, 0, 0, &hWndClient);

  idTimer = WinStartTimer(hab, hWndClient, ID_TIMER, 250);

  while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
    WinDispatchMsg(hab, &qmsg);

  WinStopTimer(hab, hWndClient, ID_TIMER);

  WinDestroyWindow(hWndFrame);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return 0;
}

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  static HWND hwndButton;
  ULONG ButtonWidth, ButtonHeight;
  ULONG ButtonX, ButtonY;

  switch(msg) {
  case WM_CREATE:
    ButtonWidth = WinQuerySysValue(HWND_DESKTOP, SV_CXPOINTER);
    ButtonHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYPOINTER);
    ButtonX = (100 + ButtonWidth) / 2;
    ButtonY = (100 + ButtonHeight) / 2;

    curIco = 0;
    hwndButton = WinCreateWindow(hwnd, WC_shBUTTON, Icons[curIco], WS_VISIBLE | BS_ICON | BS_HELP , ButtonX, ButtonY, ButtonWidth, ButtonHeight, hwnd, HWND_BOTTOM, 1, NULL, NULL);

    return 0;

  case WM_HELP:
    WinDlgBox(HWND_DESKTOP, hwnd, AboutDlgProc, NULLHANDLE, DLG_ABOUT, NULL);
    return 0;

  case WM_TIMER:
    curIco = (curIco + 1) & 3;
    WinSetWindowText(hwndButton, Icons[curIco]);
    return 0;

  case WM_ERASEBACKGROUND:
    WinFillRect((HPS)LONGFROMMP(mp1), (PRECTL)PVOIDFROMMP(mp2), 0xCCCCCC);
    return 0;

  default:
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
  }
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
    case 101:
      WinDismissDlg(hwnd, TRUE);
      return 0;
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

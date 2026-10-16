/* Shaded Buttons PMCX
 * Hubert Chan, 1998
 * Wrapper DLL to make Shaded Buttons PMCX-compliant
 */

#define INCL_WIN

#include <os2.h>
#include <string.h>
#include <PMCX.h>
#include "ShadeBtn.h"
#include "ShBtn.h"

extern "C" {
BOOL EXPENTRY ShBtnRegister(HAB);
BOOL EXPENTRY ShBtnQuery(PUSERINFO);
MRESULT EXPENTRY ShBtnStyles(HWND, ULONG, MPARAM, MPARAM);
}

MRESULT EXPENTRY AboutDlgProc(HWND, ULONG, MPARAM, MPARAM);

BOOL EXPENTRY ShBtnRegister(HAB) {
  return shBtnInit();
}

BOOL EXPENTRY ShBtnQuery(PUSERINFO pUserInfo) {
  pUserInfo->ulMajor = 1UL;
  pUserInfo->ulMinor = 10UL;
  memcpy(pUserInfo->szAuthor, "Hubert Chan", 12);
  memcpy(pUserInfo->szClassname, "shButton", 9);
  memcpy(pUserInfo->szName, "ShBtn", 6);
  pUserInfo->utDefined[0].cx = 40;
  pUserInfo->utDefined[0].cy = 10;
  pUserInfo->utDefined[0].flStyle = WS_VISIBLE | WS_TABSTOP;
  pUserInfo->utDefined[0].cMaxText = CCHTEXTMAX;
  pUserInfo->utDefined[0].idDlg = DLG_SHBTNSTYLES;
  pUserInfo->utDefined[0].ulType = UTYPE_PRIVATE;
  pUserInfo->utDefined[0].flOptions = PMCXOPT_REFRESH | PMCXOPT_TABABLE;
  pUserInfo->utDefined[0].cCtlData = 0;
  pUserInfo->utDefined[0].cMasks = 14UL;
  pUserInfo->utDefined[0].flStyleType = STYLETYPE_BITFLAGS;
  pUserInfo->utDefined[0].stMasks[0].flStyleMask = shBS_BT0;
  pUserInfo->utDefined[0].stMasks[0].idStyle     = IDS_BS_BT0;
  pUserInfo->utDefined[0].stMasks[1].flStyleMask = shBS_BT1;
  pUserInfo->utDefined[0].stMasks[1].idStyle     = IDS_BS_BT1;
  pUserInfo->utDefined[0].stMasks[2].flStyleMask = shBS_BT2;
  pUserInfo->utDefined[0].stMasks[2].idStyle     = IDS_BS_BT2;
  pUserInfo->utDefined[0].stMasks[3].flStyleMask = shBS_ICON;
  pUserInfo->utDefined[0].stMasks[3].idStyle     = IDS_BS_ICON;
  pUserInfo->utDefined[0].stMasks[4].flStyleMask = shBS_MINIICON;
  pUserInfo->utDefined[0].stMasks[4].idStyle     = IDS_BS_MINIICON;
  pUserInfo->utDefined[0].stMasks[5].flStyleMask = shBS_HELP;
  pUserInfo->utDefined[0].stMasks[5].idStyle     = IDS_BS_HELP;
  pUserInfo->utDefined[0].stMasks[6].flStyleMask = shBS_SYSCOMMAND;
  pUserInfo->utDefined[0].stMasks[6].idStyle     = IDS_BS_SYSCOMMAND;
  pUserInfo->utDefined[0].stMasks[7].flStyleMask = shBS_DEFAULT;
  pUserInfo->utDefined[0].stMasks[7].idStyle     = IDS_BS_DEFAULT;
  pUserInfo->utDefined[0].stMasks[8].flStyleMask = shBS_NOLEFTLINE;
  pUserInfo->utDefined[0].stMasks[8].idStyle     = IDS_BS_NOLEFTLINE;
  pUserInfo->utDefined[0].stMasks[9].flStyleMask = shBS_LEFTROUND;
  pUserInfo->utDefined[0].stMasks[9].idStyle     = IDS_BS_LEFTROUND;
  pUserInfo->utDefined[0].stMasks[10].flStyleMask = shBS_NORIGHTLINE;
  pUserInfo->utDefined[0].stMasks[10].idStyle     = IDS_BS_NORIGHTLINE;
  pUserInfo->utDefined[0].stMasks[11].flStyleMask = shBS_RIGHTROUND;
  pUserInfo->utDefined[0].stMasks[11].idStyle     = IDS_BS_RIGHTROUND;
  pUserInfo->utDefined[0].stMasks[12].flStyleMask = shBS_VERTICAL;
  pUserInfo->utDefined[0].stMasks[12].idStyle     = IDS_BS_VERTICAL;
  pUserInfo->utDefined[0].stMasks[13].flStyleMask = shBS_NOPUSH;
  pUserInfo->utDefined[0].stMasks[13].idStyle     = IDS_BS_NOPUSH;
  pUserInfo->utDefined[0].stMasks[14].flStyleMask = shBS_BITMAP;
  pUserInfo->utDefined[0].stMasks[14].idStyle     = IDS_BS_BITMAP;
  pUserInfo->utDefined[0].stMasks[15].flStyleMask = shBS_TEXT;
  pUserInfo->utDefined[0].stMasks[15].idStyle     = IDS_BS_TEXT;
  memcpy(pUserInfo->utDefined[0].szDescription, "Shaded Button", 14);
  return (TRUE);
}

MRESULT EXPENTRY ShBtnStyles(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
  PUSERSTYLE pust;

  switch(msg) {
  case WM_INITDLG:
    PDATATODLG(hwnd);
    if((pust = (PUSERSTYLE) mp2) != NULL) {
      WinSetDlgItemText(hwnd, EF_TEXT, pust->pszText);
      ((BOOL (_System *)(HWND, ULONG, PUSERSTYLE))pust->pfnSetSymbolID)(hwnd, IDB_ID, pust);
    }

    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_DELETEALL, NULL, NULL);
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_INSERTITEM, NULL, MPFROMP("Auto 3 State"));
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_INSERTITEM, NULL, MPFROMP("3 State"));
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_INSERTITEM, NULL, MPFROMP("Auto Radio Button"));
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_INSERTITEM, NULL, MPFROMP("Radio Button"));
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_INSERTITEM, NULL, MPFROMP("Auto Check Box"));
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_INSERTITEM, NULL, MPFROMP("Check Box"));
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_INSERTITEM, NULL, MPFROMP("Push Button"));
    WinSendDlgItemMsg(hwnd, CX_TYPE, LM_SELECTITEM, MPFROMLONG(pust->flStyle & 7), MPFROMLONG(TRUE));

    WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_DELETEALL, NULL, NULL);
    WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_INSERTITEM, NULL, MPFROMP("No border"));
    WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_INSERTITEM, NULL, MPFROMP("Rounded"));
    WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_INSERTITEM, NULL, MPFROMP("Straight"));
    if(pust->flStyle & shBS_NOLEFTLINE)
      WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_SELECTITEM, MPFROMLONG(2UL), MPFROMLONG(TRUE));
    else if (pust->flStyle & shBS_LEFTROUND)
      WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_SELECTITEM, MPFROMLONG(1UL), MPFROMLONG(TRUE));
    else
      WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_SELECTITEM, MPFROMLONG(0UL), MPFROMLONG(TRUE));

    WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_DELETEALL, NULL, NULL);
    WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_INSERTITEM, NULL, MPFROMP("No border"));
    WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_INSERTITEM, NULL, MPFROMP("Rounded"));
    WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_INSERTITEM, NULL, MPFROMP("Straight"));
    if(pust->flStyle & shBS_NORIGHTLINE)
      WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_SELECTITEM, MPFROMLONG(2UL), MPFROMLONG(TRUE));
    else if (pust->flStyle & shBS_RIGHTROUND)
      WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_SELECTITEM, MPFROMLONG(1UL), MPFROMLONG(TRUE));
    else
      WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_SELECTITEM, MPFROMLONG(0UL), MPFROMLONG(TRUE));

    if(pust->flStyle & shBS_MINIICON)
      WinSendDlgItemMsg(hwnd, CB_MINIICON, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & shBS_ICON)
      WinSendDlgItemMsg(hwnd, CB_ICON, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & shBS_SYSCOMMAND)
      WinSendDlgItemMsg(hwnd, CB_SYSCOMMAND, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & shBS_HELP)
      WinSendDlgItemMsg(hwnd, CB_HELP, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & shBS_NOPUSH)
      WinSendDlgItemMsg(hwnd, CB_NOPUSH, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & shBS_BITMAP)
      WinSendDlgItemMsg(hwnd, CB_BITMAP, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & shBS_VERTICAL)
      WinSendDlgItemMsg(hwnd, CB_VERTICAL, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);

    if(pust->flStyle & WS_VISIBLE)
      WinSendDlgItemMsg(hwnd, CB_VISIBLE, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & WS_GROUP)
      WinSendDlgItemMsg(hwnd, CB_GROUP, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & WS_DISABLED)
      WinSendDlgItemMsg(hwnd, CB_DISABLED, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);
    if(pust->flStyle & WS_TABSTOP)
      WinSendDlgItemMsg(hwnd, CB_TABSTOP, BM_SETCHECK, MPFROMSHORT(TRUE), 0L);

    // centre dialog
    SWP swp;
    WinQueryWindowPos(hwnd, &swp);
    WinSetWindowPos(hwnd, HWND_TOP,
        (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) - swp.cx) / 2L,
        (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - swp.cy) / 2L,
        0L, 0L, SWP_MOVE);
    return 0;
  case WM_COMMAND:
    switch(SHORT1FROMMP(mp1)) {
    case DID_OK:
      if((pust=PDATAFROMDLG(hwnd)) != NULL) {
	if (!((BOOL (_System *)(HWND, ULONG, PUSERSTYLE))pust->pfnGetSymbolID)(hwnd, IDB_ID, pust))
	  break;
	pust->cText = WinQueryDlgItemText(hwnd, EF_TEXT, CCHTEXTMAX, pust->pszText);

	pust->flStyle = 0;
	if (WinSendDlgItemMsg(hwnd, CB_VISIBLE, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= WS_VISIBLE;
	if (WinSendDlgItemMsg(hwnd, CB_GROUP, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= WS_GROUP;
	if (WinSendDlgItemMsg(hwnd, CB_DISABLED, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= WS_DISABLED;
	if (WinSendDlgItemMsg(hwnd, CB_TABSTOP, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= WS_TABSTOP;

	if (WinSendDlgItemMsg(hwnd, CB_MINIICON, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= shBS_MINIICON;
	if (WinSendDlgItemMsg(hwnd, CB_ICON, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= shBS_ICON;
	if (WinSendDlgItemMsg(hwnd, CB_SYSCOMMAND, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= shBS_SYSCOMMAND;
	if (WinSendDlgItemMsg(hwnd, CB_HELP, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= shBS_HELP;
	if (WinSendDlgItemMsg(hwnd, CB_NOPUSH, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= shBS_NOPUSH;
	if (WinSendDlgItemMsg(hwnd, CB_BITMAP, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= shBS_BITMAP;
	if (WinSendDlgItemMsg(hwnd, CB_VERTICAL, BM_QUERYCHECK, 0L, 0L))
	  pust->flStyle |= shBS_VERTICAL;

	pust->flStyle |= (USHORT) WinSendDlgItemMsg(hwnd, CX_TYPE, LM_QUERYSELECTION, MPFROMLONG(0L), MPFROMLONG(0L));
	switch ((USHORT) WinSendDlgItemMsg(hwnd, CX_LSIDE, LM_QUERYSELECTION, MPFROMLONG(0L), MPFROMLONG(0L))) {
	case 0:
	  break;
	case 1:
	  pust->flStyle |= shBS_LEFTROUND;
	  break;
	case 2:
	  pust->flStyle |= shBS_NOLEFTLINE;
	}
	switch ((USHORT) WinSendDlgItemMsg(hwnd, CX_RSIDE, LM_QUERYSELECTION, MPFROMLONG(0L), MPFROMLONG(0L))) {
	case 0:
	  break;
	case 1:
	  pust->flStyle |= shBS_RIGHTROUND;
	  break;
	case 2:
	  pust->flStyle |= shBS_NORIGHTLINE;
	}

	WinDismissDlg(hwnd, TRUE);
      }
      return 0;
    case DID_CANCEL:
      WinDismissDlg(hwnd, FALSE);
      return 0;
    case PB_FONTCOLORS:
      if((pust=PDATAFROMDLG(hwnd)) != NULL)
	((VOID (_System *)(HWND))pust->pfnGetFontClr)(hwnd);
      return 0;
    case PB_ABOUT:
      return 0;
    }
    return 0;
  case WM_CLOSE:
    WinDismissDlg(hwnd, FALSE);
    return 0;
  default:
    return(WinDefDlgProc(hwnd, msg, mp1, mp2));
  }
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

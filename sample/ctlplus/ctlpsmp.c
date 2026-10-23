/*
 * sample/ctlplus/ctlpsmp.c -- Control Plus sample for EXPM.
 *
 * Demonstrates the four Control Plus controls:
 *   WC_METER   thermometer
 *   WC_SWITCH  push button / throw switch
 *   WC_RECT    draggable, sizeable rectangle
 *   WC_COUNTER chained 7-segment digit counter
 *
 * The layout mirrors the Controls+ demo (CTLDEMO.C): meter, both switch
 * styles, a sizing rect and three chained decimal digits, plus buttons
 * that drive the least significant digit.
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
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <string.h>

#include "ctlpsmp.h"
#include "expm.h"

/* Local procedures */

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* Static variables */

static HWND hwndCounter[3] = { NULLHANDLE, NULLHANDLE, NULLHANDLE };
static HWND hwndStatus = NULLHANDLE;
static BOOL bDark = FALSE;
static HWND hwndThemeMenu = NULLHANDLE;

/* ------------------------------------------------------------------ */

static HWND CreateCtl(HWND hwndParent, PSZ pszClass, PSZ pszText, ULONG id,
                      LONG x, LONG y, LONG cx, LONG cy, PVOID pCtlData)
{
    return WinCreateWindow(hwndParent, pszClass, pszText, WS_VISIBLE,
                           x, y, cx, cy,
                           hwndParent, HWND_TOP, id, pCtlData, NULL);
}

static void BuildControls(HWND hwnd)
{
    MTCREATE  mtc;
    SWCREATE  swc;
    RCCREATE  rcc;
    CNTCREATE cnc;
    int i;

    /* Throw switch (top left). */
    swc.swStyle   = SWS_THROW | SWS_NOTIFYOWNER;
    swc.swFgColor = CLR_BLUE;
    strcpy(swc.swUpText,   "Up");
    strcpy(swc.swDownText, "Down");
    CreateCtl(hwnd, WC_SWITCH, "", ID_SWITCH2, 30, 270, 130, 140, &swc);

    /* Button switch (bottom left). */
    swc.swStyle   = SWS_BUTTON | SWS_NOTIFYOWNER;
    swc.swFgColor = CLR_BLUE;
    strcpy(swc.swUpText,   "Up");
    strcpy(swc.swDownText, "Down");
    CreateCtl(hwnd, WC_SWITCH, "", ID_SWITCH1, 30, 60, 130, 130, &swc);

    /* Thermometer meter (top, second column). */
    mtc.mtStyle   = MTS_THERM | MTS_NOTIFYOWNER | MTS_HALFTICK | MTS_QUARTERTICK;
    mtc.mtFgColor = CLR_BLUE;
    CreateCtl(hwnd, WC_METER, "", ID_METER, 200, 270, 110, 140, &mtc);

    /* Sizing rect (top, third column). */
    rcc.rcStyle      = RCS_VERTHORZSIZE | RCS_KSQUARE | RCS_NOTIFYOWNER;
    rcc.rcFgColor    = CLR_BLACK;
    rcc.rcMinSize.cx = 40;
    rcc.rcMinSize.cy = 40;
    rcc.rcMaxSize.cx = 140;
    rcc.rcMaxSize.cy = 140;
    CreateCtl(hwnd, WC_RECT, "", ID_RECT, 340, 270, 150, 140, &rcc);

    /* Three chained decimal digits, most significant on the left. */
    cnc.cntStyle   = CTS_DEC | CTS_NOTIFYOWNER;
    cnc.cntFgColor = CLR_RED;
    cnc.cntBgColor = CLR_BLACK;
    cnc.cntNext    = NULLHANDLE;

    for (i = 0; i < 3; i++)
        hwndCounter[i] = CreateCtl(hwnd, WC_COUNTER, "", ID_CNT0 + i,
                                   510 + (2 - i) * 36, 282, 32, 74, &cnc);

    WinSendMsg(hwndCounter[0], CTM_SETNEXT, MPFROMP(hwndCounter[1]), 0);
    WinSendMsg(hwndCounter[1], CTM_SETNEXT, MPFROMP(hwndCounter[2]), 0);

    /* Driver buttons. */
    CreateCtl(hwnd, WC_BUTTON, "Inc",   ID_INC,   510, 236, 120, 26, NULL);
    CreateCtl(hwnd, WC_BUTTON, "Dec",   ID_DEC,   510, 204, 120, 26, NULL);
    CreateCtl(hwnd, WC_BUTTON, "Reset", ID_RESET, 510, 172, 120, 26, NULL);

    hwndStatus = CreateCtl(hwnd, WC_STATIC, "", ID_STATUS, 20, 8, 620, 22, NULL);
}

static void ReportControl(HWND hwnd, USHORT id, USHORT code)
{
    PSZ  pszName;
    PSZ  pszWhat;
    CHAR szText[160];
    LONG v0, v1, v2;

    (void)hwnd;

    switch (id)
    {
        case ID_METER:   pszName = "meter";   break;
        case ID_SWITCH1: pszName = "switch1"; break;
        case ID_SWITCH2: pszName = "switch2"; break;
        case ID_RECT:    pszName = "rect";    break;
        case ID_CNT0:    pszName = "digit0";  break;
        case ID_CNT1:    pszName = "digit1";  break;
        case ID_CNT2:    pszName = "digit2";  break;
        default:         pszName = "control"; break;
    }

    switch (id)
    {
        case ID_METER:
            pszWhat = (code == MTN_CHGRANGE) ? "range changed" : "value changed";
            break;
        case ID_SWITCH1:
        case ID_SWITCH2:
            pszWhat = "value changed";
            break;
        case ID_RECT:
            pszWhat = (code == RCN_SIZE) ? "sizing" :
                      (code == RCN_DRAG) ? "dragging" : "paint";
            break;
        case ID_CNT0:
        case ID_CNT1:
        case ID_CNT2:
            pszWhat = (code == CTN_CHGVALUE)  ? "value changed" :
                      (code == CTN_OVERFLOW)  ? "overflow" :
                      (code == CTN_UNDERFLOW) ? "underflow" : "notify";
            break;
        default:
            pszWhat = "notify";
            break;
    }

    v0 = hwndCounter[0] ? (LONG)WinSendMsg(hwndCounter[0], CTM_QUERYVALUE, 0, 0) : 0;
    v1 = hwndCounter[1] ? (LONG)WinSendMsg(hwndCounter[1], CTM_QUERYVALUE, 0, 0) : 0;
    v2 = hwndCounter[2] ? (LONG)WinSendMsg(hwndCounter[2], CTM_QUERYVALUE, 0, 0) : 0;

    sprintf(szText, "%-8s %-14s  counter = %ld%ld%ld",
            pszName, pszWhat, v2, v1, v0);

    if (hwndStatus)
        WinSetWindowText(hwndStatus, szText);
}

static void IncLeastSignificant(void)
{
    if (hwndCounter[0])
        WinSendMsg(hwndCounter[0], CTM_INC, 0, 0);
}

static void DecLeastSignificant(void)
{
    if (hwndCounter[0])
        WinSendMsg(hwndCounter[0], CTM_DEC, 0, 0);
}

static void ResetCounters(void)
{
    int i;

    for (i = 0; i < 3; i++)
        if (hwndCounter[i])
            WinSendMsg(hwndCounter[i], CTM_SETVALUE, MPFROMSHORT(0), 0);
}

/* ------------------------------------------------------------------ */

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            HWND hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
            hwndThemeMenu = WinWindowFromID(hwndFrame, FID_MENU);
            BuildControls(hwnd);
            ReportControl(hwnd, ID_CNT0, CTN_CHGVALUE);
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
        {
            HPS         hps = (HPS)mp1;
            RECTL       rcl;
            PXpmTheme   pth = XpmQueryTheme();

            GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
            WinQueryWindowRect(hwnd, &rcl);
            WinFillRect(hps, &rcl, (LONG)pth->clrCanvas);
            return (MRESULT)TRUE;
        }

        case WM_PAINT:
        {
            HPS         hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
            RECTL       rcl, rLab;
            PXpmTheme   pth = XpmQueryTheme();
            LONG        clrBack = (LONG)pth->clrCanvas;
            LONG        clrText = (LONG)pth->clrText;

            WinQueryWindowRect(hwnd, &rcl);
            GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
            WinFillRect(hps, &rcl, clrBack);

            rLab.xLeft =  20; rLab.xRight = 170; rLab.yBottom = 414; rLab.yTop = 438;
            WinDrawText(hps, -1, "Throw Switch", &rLab, clrText, clrBack,
                        DT_CENTER | DT_VCENTER | DT_ERASERECT);

            rLab.xLeft =  20; rLab.xRight = 170; rLab.yBottom = 194; rLab.yTop = 218;
            WinDrawText(hps, -1, "Button Switch", &rLab, clrText, clrBack,
                        DT_CENTER | DT_VCENTER | DT_ERASERECT);

            rLab.xLeft = 190; rLab.xRight = 320; rLab.yBottom = 414; rLab.yTop = 438;
            WinDrawText(hps, -1, "Therm Meter", &rLab, clrText, clrBack,
                        DT_CENTER | DT_VCENTER | DT_ERASERECT);

            rLab.xLeft = 330; rLab.xRight = 500; rLab.yBottom = 414; rLab.yTop = 438;
            WinDrawText(hps, -1, "Sizing Rect", &rLab, clrText, clrBack,
                        DT_CENTER | DT_VCENTER | DT_ERASERECT);

            rLab.xLeft = 505; rLab.xRight = 645; rLab.yBottom = 358; rLab.yTop = 382;
            WinDrawText(hps, -1, "Digit Counters", &rLab, clrText, clrBack,
                        DT_CENTER | DT_VCENTER | DT_ERASERECT);

            WinEndPaint(hps);
            return (MRESULT)FALSE;
        }

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_INC:
                    IncLeastSignificant();
                    break;
                case ID_DEC:
                    DecLeastSignificant();
                    break;
                case ID_RESET:
                    ResetCounters();
                    break;
                case IDM_ABOUT:
                    WinMessageBox(HWND_DESKTOP, hwnd,
                                  "EXPM Control Plus sample\n\n"
                                  "Meter, switch, rect and counter controls.",
                                  "About", 0, MB_OK | MB_INFORMATION);
                    break;
                case IDM_EXIT:
                    WinPostMsg(hwnd, WM_QUIT, 0, 0);
                    break;
                case IDM_DARK:
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
            }
            return (MRESULT)FALSE;

        case WM_CONTROL:
            ReportControl(hwnd, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
            return (MRESULT)FALSE;

        case XpmWM_THEMECHANGED:
            WinInvalidateRect(hwnd, NULLHANDLE, TRUE);
            return (MRESULT)FALSE;

        case WM_CLOSE:
            WinPostMsg(hwnd, WM_QUIT, 0, 0);
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

INT main(VOID)
{
    HAB   hab;
    HMQ   hmq;
    QMSG  qmsg;
    HWND  hwndFrame;
    HWND  hwndClient;
    ULONG flCreate = FCF_TITLEBAR | FCF_SYSMENU | FCF_MENU |
                     FCF_MINMAX   | FCF_SIZEBORDER | FCF_TASKLIST;

    hab = WinInitialize(0);
    if (!hab)
        return -1;

    hmq = WinCreateMsgQueue(hab, 0);
    if (!hmq)
    {
        WinTerminate(hab);
        return -2;
    }

    if (!WinRegisterClass(hab, "CtlPlusSample",
                          (PFNWP)ClientWndProc, CS_SIZEREDRAW, 0))
    {
        WinDestroyMsgQueue(hmq);
        WinTerminate(hab);
        return -3;
    }

    XpmCtlInit(hab);

    hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
                                   WS_VISIBLE,
                                   &flCreate,
                                   "CtlPlusSample",
                                   "EXPM Control Plus Sample",
                                   0,
                                   NULLHANDLE,
                                   IDM_MENU,
                                   &hwndClient);

    if (hwndFrame)
    {
        WinSetWindowPos(hwndFrame, NULLHANDLE, 60, 40, 700, 520,
                        SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);

        while (WinGetMsg(hab, &qmsg, 0, 0, 0))
            WinDispatchMsg(hab, &qmsg);

        WinDestroyWindow(hwndFrame);
    }

    XpmCtlTerminate();
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return 0;
}

/*
 * sample/cell/celmp.c -- EXPM CELL sample application.
 *
 * Demonstrates the Cell Toolkit merged into EXPM: a vertically split
 * view (list box / multi-line edit), a split bar, and an attached
 * toolbar with bubble help and bitmap buttons.
 *
 * Adapted from the SIMPLE sample of the Cell Toolkit 0.7b
 * (copyright (C) Sergey I. Yevtushenko, BSD-2 terms; see
 * src/cell/cell.c).  Toolkit calls use the Presentation Manager-style
 * EXPM names (XpmCellInit, XpmCreateCell, ...); see doc/cell.txt.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WIN
#define INCL_NLS
#define INCL_DOS
#define INCL_GPI

#include <os2.h>
#include <string.h>
#include <stdlib.h>

#include "celmp.h"
#include "expm.h"

/* Local procedures */

MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* Static variables */

CellDef cdLeftPane =
{
    CELL_WINDOW,
    WC_LISTBOX,
    "List",
    LS_NOADJUSTPOS | WS_VISIBLE,
    ID_LIST
};

CellDef cdRightPane =
{
    CELL_WINDOW,
    WC_MLE,
    "SampleText",
    MLS_BORDER | WS_VISIBLE,
    ID_MLE
};

CellDef mainClient =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT30x70,
    0,
    "Simple",
    FCF_TITLEBAR | FCF_SYSMENU  | FCF_MENU |
    FCF_MINMAX   | FCF_TASKLIST | FCF_SIZEBORDER |
    0,
    MAIN_FRAME,
    &cdLeftPane,
    &cdRightPane,
    0,
    MainClientProc
};

ULONG mainItems[] =
{
    IDB_FILENEW ,
    IDB_FILEOPEN,
    IDB_FILESAVE,
    IDB_FILSAVAS,
    TB_SEPARATOR,
    IDB_EXIT    ,
    TB_SEPARATOR,
    IDB_EDITCOPY,
    IDB_EDITCUT ,
    IDB_EDITPAST,
    IDB_EDITUNDO,
    TB_SEPARATOR,
    IDB_EDITFIND,
    IDB_EDITFNNX,
    IDB_EDITREPL,
    TB_SEPARATOR,
    IDB_HELP    ,
    IDB_ABOUT   ,
    0
};

TbDef mainTb =
{
    TB_ATTACHED_TP | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems
};

MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    WindowCellCtlData *pWCtlData;

    pWCtlData = (WindowCellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    switch(msg)
    {
        case WM_COMMAND:
            switch(SHORT1FROMMP(mp1))
            {
                case IDB_EXIT:
                    WinPostMsg(hwnd, WM_QUIT, 0L, 0L);
                    return ((MRESULT) NULL);
            }
            break;

        case WM_CLOSE:
            WinPostMsg(hwnd, WM_QUIT, 0L, 0L);
            return ((MRESULT) NULL);
    }

    if(pWCtlData)
        return pWCtlData->pOldProc(hwnd, msg, mp1, mp2);

    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

INT main(VOID)
{
    HAB hab;
    HMQ hmq;
    QMSG qmsg;
    HWND hwndFrame;
    HWND hwndTmp;
    LONG lColor = CLR_PALEGRAY;
    SWP swp;

    hab = WinInitialize(0);

    if(!hab)
    {
        return -1;
    }

    hmq = WinCreateMsgQueue(hab, 0);

    if(!hmq)
    {
        WinTerminate(hab);
        return -2;
    }

    XpmCellInit(hab);

    WinQueryWindowPos(HWND_DESKTOP, &swp);

    hwndFrame = XpmCreateCell(&mainClient, HWND_DESKTOP, 0);
    if(hwndFrame)
    {
        HWND hwndList;

        hwndList = XpmCellWindowFromID(hwndFrame, ID_LPANE);

        if(!hwndList)
            hwndList = XpmCellWindowFromID(hwndFrame, ID_LIST);

        if(hwndList)
        {
            CHAR cText[256];
            static char cListItem[] = "List item ";
            int i;

            strcpy(cText, cListItem);

            for(i = 0; i < 15; i++)
            {
                XpmGenResIDStr(cText + sizeof(cListItem) - 1, i);
                WinSendMsg(hwndList,
                           LM_INSERTITEM,
                           MPFROMSHORT(LIT_END),
                           MPFROMP(cText));
            }
        }

        WinSetWindowPos(hwndFrame, NULLHANDLE,
                        swp.x + swp.cx / 8,
                        swp.y + swp.cy / 8,
                        (swp.cx / 4) * 3,
                        (swp.cy / 4) * 3,
                        SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);

        XpmCreateToolbar(hwndFrame, &mainTb);

        /* Set MLE color */

        hwndTmp = XpmCellWindowFromID(hwndFrame, ID_MLE);

        WinSendMsg(hwndTmp,
                   MLM_SETBACKCOLOR,
                   MPFROMLONG(CLR_PALEGRAY),
                   MPFROMLONG(MLE_INDEX));

        /* Set list color */

        hwndTmp = XpmCellWindowFromID(hwndFrame, ID_LIST);

        WinSetPresParam(hwndTmp,
                        PP_BACKGROUNDCOLORINDEX,
                        sizeof(lColor),
                        &lColor);

        while (WinGetMsg(hab, &qmsg, 0, 0, 0))
            WinDispatchMsg(hab, &qmsg);

        WinDestroyWindow(hwndFrame);
    }

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return 0;
}

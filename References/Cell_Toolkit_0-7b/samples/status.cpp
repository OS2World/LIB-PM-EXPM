/*
** Module   :MULTIBAR.C
** Abstract :Test and sample application for Cell toolkit procedures.
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Sun  08/02/98   Refined
**
*/

#define INCL_WIN
#define INCL_NLS
#define INCL_DOS
#define INCL_GPI

#include <os2.h>
#include <multibar.h>
#include <cell.h>

/* Local procedures */

MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* Static Variables */

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

CellDef rPanel =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT30x70,
    0,
    "Subwindow",
    WS_VISIBLE,
    ID_TOP,
    &cdLeftPane,
    &cdRightPane,
};

CellDef Panel1 =
{
    CELL_WINDOW,
    "StatusLine",
    "",
    WS_VISIBLE,
    ID_STATUS
};

CellDef mainClient =
{
    CELL_HSPLIT | CELL_FIXED | CELL_SIZE1,
    0,
    "Status Line Sample",
    FCF_TITLEBAR | FCF_SYSMENU  | FCF_MENU |
    FCF_MINMAX   | FCF_TASKLIST | FCF_SIZEBORDER |
    0,
    MAIN_FRAME,
    &Panel1,
    &rPanel,
    0,                  //Frame subclass proc
    MainClientProc,     //Client subclass proc
    20                  //Status line hight
};

ULONG mainItems[]=
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
    IDB_EDITFIND,
    IDB_EDITFNNX,
    IDB_EDITPAST,
    IDB_EDITREPL,
    IDB_EDITUNDO,
    TB_SEPARATOR,
    IDB_HELP    ,
    IDB_ABOUT   ,
    0
};

TbDef mainTb =
{
    TB_VERTICAL | TB_ATTACHED_TP | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems
};

ULONG OKMsgBox(PSZ pszText)
{
    return WinMessageBox(HWND_DESKTOP,
                         HWND_DESKTOP,
                         pszText,
                         "Cell Demo",
                         0,
                         MB_OK | MB_INFORMATION | MB_APPLMODAL);
}

MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    WindowCellCtlData *pWCtlData = 0;

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

MRESULT EXPENTRY StatusLineProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch(msg)
    {
        case WM_PAINT:
            {
                HPS hpsPaint;
                RECTL rclPaint;
                POINTL ptlWork;

                hpsPaint = WinBeginPaint(hwnd, 0, 0);
                WinQueryWindowRect(hwnd, &rclPaint);

                WinFillRect(hpsPaint, &rclPaint, CLR_PALEGRAY);

                GpiSetColor(hpsPaint, CLR_BLACK);

                ptlWork.x = rclPaint.xLeft      ;
                ptlWork.y = rclPaint.yBottom    ;
                GpiMove(hpsPaint, &ptlWork);
                ptlWork.y = rclPaint.yTop    - 2;
                GpiLine(hpsPaint, &ptlWork);
                ptlWork.x = rclPaint.xRight  - 1;
                GpiLine(hpsPaint, &ptlWork);

                GpiSetColor(hpsPaint,CLR_WHITE);

                ptlWork.y = rclPaint.yBottom    ;
                GpiLine(hpsPaint, &ptlWork);
                ptlWork.x = rclPaint.xLeft      ;
                GpiLine(hpsPaint, &ptlWork);

                rclPaint.yTop    -= 3;
                rclPaint.yBottom += 1;
                rclPaint.xRight  -= 2;
                rclPaint.xLeft   += 1;

                WinDrawText(hpsPaint, -1, "Status message", &rclPaint,
                            CLR_BLACK,
                            0,
                            DT_LEFT | DT_VCENTER);

                WinEndPaint(hpsPaint);
            }
            return MRFROMSHORT(FALSE);
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

INT main(VOID)
{
	HAB hab;
	HMQ hmq;
	QMSG qmsg;
    HWND hwndFrame;
    HWND hwndTb;
    HWND hwndTmp;
    LONG lColor = CLR_PALEGRAY;
    SWP swp;
    static char cFontMy[]="8.Helv";

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

    ToolkitInit(hab);

    WinRegisterClass(hab,
                     "StatusLine",
                     StatusLineProc,
                     CS_SIZEREDRAW,
                     sizeof(ULONG));


    WinQueryWindowPos(HWND_DESKTOP, &swp);

    hwndFrame = CreateCell(&mainClient, HWND_DESKTOP, 0);

    if(hwndFrame)
    {
        HWND hwndSubframe = CellWindowFromID(hwndFrame, ID_LPANE);

        WinSetWindowPos(hwndFrame, NULLHANDLE,
        	            swp.x + swp.cx / 8,
            	        swp.y + swp.cy / 8,
                	    (swp.cx / 4) * 3,
                    	(swp.cy / 4) * 3,
	                    SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);


        CreateToolbar(hwndFrame, &mainTb);

        /* Set status line font */

        hwndTmp = CellWindowFromID(hwndFrame, ID_STATUS);

        WinSetPresParam(hwndTmp,
    	                PP_FONTNAMESIZE,
            	        sizeof(cFontMy),
                        cFontMy);

        /* Set MLE color */

        hwndTmp = CellWindowFromID(hwndFrame, ID_MLE);

        WinSendMsg(hwndTmp,
    	           MLM_SETBACKCOLOR,
                   MPFROMLONG(CLR_PALEGRAY),
                   MPFROMLONG(MLE_INDEX));

        /* Set list color */

        hwndTmp = CellWindowFromID(hwndFrame, ID_LIST);

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



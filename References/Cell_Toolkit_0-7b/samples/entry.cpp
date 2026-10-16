/*
** Module   :SIMPLE.C
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Sun  14/02/1998     Created
**
*/

#define INCL_WIN
#define INCL_NLS
#define INCL_DOS
#define INCL_GPI

#include <os2.h>
#include <multibar.h>
#include <cell.h>
#include <string.h>

/* Local procedures */

MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY LEProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* Static Variables */
#define ID_FINDEF   1060
#define ID_FINDPB   1061
#define ID_PRXCELL  1062
#define ID_MLE2     1063


CellDef cdLeftPane =
{
    CELL_WINDOW,
    WC_MLE,
    "SampleText",
    MLS_BORDER | WS_VISIBLE,
    ID_MLE2
};

CellDef findEf =
{
    CELL_WINDOW,
    WC_ENTRYFIELD,
    NULL,
    WS_VISIBLE | ES_ANY | ES_MARGIN,
    ID_FINDEF,
    0,0,
    LEProc,
    0,0
};

CellDef findPb =
{
    CELL_WINDOW,
    WC_BUTTON,
    "Find",
    WS_VISIBLE,
    ID_FINDPB
};

CellDef cdRightPane =
{
    CELL_VSPLIT | CELL_SIZE2 | CELL_SPLITBAR | CELL_FIXED,
    0,
    NULL,
    WS_VISIBLE,
    ID_PRXCELL,
    &findEf,
    &findPb,
    NULL, NULL,
    60
};

CellDef mainClient =
{
    CELL_HSPLIT | CELL_SIZE2 | CELL_FIXED,
    0,
    "Simple",
    FCF_TITLEBAR | FCF_SYSMENU  | FCF_MENU |
    FCF_MINMAX   | FCF_TASKLIST | FCF_SIZEBORDER |
    0,
    MAIN_FRAME,
    &cdLeftPane,
    &cdRightPane,
    0,
    MainClientProc,
    20
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
    TB_ATTACHED_TP /*| TB_BUBBLE*/,
    ID_TOOLBAR,
    mainItems
};

MRESULT EXPENTRY LEProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    WindowCellCtlData *pWCtlData = 0;

    pWCtlData = (WindowCellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    switch(msg)
    {
        case WM_BUTTON1DOWN:
            {
                WinPostMsg(hwnd, WM_BUTTON1DBLCLK, mp1, mp2);
            }
            break;
    }
    if(pWCtlData)
        return pWCtlData->pOldProc(hwnd, msg, mp1, mp2);

    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY MainClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    WindowCellCtlData *pWCtlData = 0;
    static iTimerStarted = 0;
/*
    if(!iTimerStarted)
    {
        APIRET rc = WinStartTimer(0, hwnd, 1, 1000);
        iTimerStarted = 0;
    }
*/
    pWCtlData = (WindowCellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    switch(msg)
    {
        case WM_TIMER:
            WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
                        	HWND_TOP, 0,0,0,0, SWP_ZORDER);
            break;

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
    HWND hwndTb;
    HWND hwndTmp;
    LONG lColor = CLR_PALEGRAY;
    static char cFontMy[]="8.Helv";
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

    ToolkitInit(hab);

    WinQueryWindowPos(HWND_DESKTOP, &swp);

    hwndFrame = CreateCell(&mainClient, HWND_DESKTOP, 0);
    if(hwndFrame)
    {
        WinSetWindowPos(hwndFrame, NULLHANDLE,
        	            swp.x + swp.cx / 8,
            	        swp.y + swp.cy / 8,
                	    (swp.cx / 4) * 3,
                    	(swp.cy / 4) * 3,
	                    SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);


        CreateToolbar(hwndFrame, &mainTb);

        /* Set MLE color */

        hwndTmp = CellWindowFromID(hwndFrame, ID_MLE2);

        WinSetPresParam(hwndTmp,
                        PP_BACKGROUNDCOLORINDEX,
                        sizeof(lColor),
                        &lColor);

        WinSetPresParam(hwndFrame,
                        PP_FONTNAMESIZE,
                        sizeof(cFontMy),
                        cFontMy);

        WinStartTimer(0, WinWindowFromID(hwndFrame, FID_CLIENT), 1, 1000);

        while (WinGetMsg(hab, &qmsg, 0, 0, 0))
        	WinDispatchMsg(hab, &qmsg);

        WinDestroyWindow(hwndFrame);
    }

	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);

    return 0;
}



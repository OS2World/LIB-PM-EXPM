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

CellDef Panel0 =
{
    CELL_WINDOW,
    WC_BUTTON,
    "Button",
    BS_PUSHBUTTON | WS_VISIBLE,
    ID_RPANE
};

CellDef Panel1 =
{
    CELL_WINDOW,
    WC_STATIC,
    "Button",
    SS_TEXT | DT_CENTER | DT_VCENTER | WS_VISIBLE,
    ID_LPANE
};

CellDef uPanel =
{
    CELL_HSPLIT | CELL_SPLITBAR | CELL_FIXED | CELL_SIZE2,
    0,
    "",
    WS_VISIBLE,
    ID_RPANE,
    &Panel0,
    &Panel1,
    0,
    0,
    40
};

CellDef dPanel =
{
    CELL_WINDOW,
    WC_STATIC,
    "Button",
    SS_TEXT | DT_CENTER | DT_VCENTER | WS_VISIBLE,
    ID_LPANE,
    0,
    0
};

CellDef rPanel =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT30x70,
    0,
    "Subwindow",
    WS_VISIBLE,
    ID_RPANE,
    &uPanel,
    &dPanel
};

CellDef lPanel =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT70x30,
    0,
    "",
    WS_VISIBLE,
    ID_LPANE,
    &uPanel,
    &dPanel
};

CellDef mainClient =
{
    CELL_HSPLIT | CELL_SPLITBAR,
    0,
    "Multiple Toolbars Sample",
    FCF_TITLEBAR | FCF_SYSMENU  | FCF_MENU |
    FCF_MINMAX   | FCF_TASKLIST | FCF_SIZEBORDER |
    0,
    MAIN_FRAME,
    &lPanel,
    &rPanel,
    0,                  //Frame subclass proc
    MainClientProc      //Client subclass proc
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

ULONG mainItems1[]=
{
    IDB_FILENEW ,
    TB_SEPARATOR,
    IDB_FILEOPEN,
    IDB_FILESAVE,
    IDB_FILSAVAS,
    0
};

ULONG mainItems2[]=
{
    IDB_EDITCOPY,
    IDB_EDITCUT ,
    IDB_EDITPAST,
    TB_SEPARATOR,
    IDB_EDITFIND,
    IDB_EDITFNNX,
    IDB_EDITREPL,
    TB_SEPARATOR,
    IDB_EDITUNDO,
    0
};

ULONG mainItems3[]=
{
    IDB_HELP ,
    IDB_ABOUT,
    0
};

ULONG mainItems4[]=
{
    IDB_EXIT    ,
    TB_SEPARATOR,
    IDB_HELP    ,
    IDB_ABOUT   ,
    0
};


TbDef mainTb =
{
    TB_VERTICAL | TB_ATTACHED_LT | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems
};

TbDef mainTb1 =
{
    TB_VERTICAL | TB_ATTACHED_RT | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems1
};

TbDef mainTb2 =
{
    0           | TB_ATTACHED_TP | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems2
};

TbDef mainTb3 =
{
    0           | TB_ATTACHED_BT | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems3
};

TbDef mainTb4 =
{
    TB_VERTICAL | TB_FLOATING | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems4
};

TbDef mainTb5 =
{
    0           | TB_FLOATING | TB_BUBBLE,
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

                case ID_LPANE  :OKMsgBox("Button ID_LPANE pressed!"); break;
                case ID_RPANE  :OKMsgBox("Button ID_RPANE pressed!"); break;
                case MAIN_FRAME:OKMsgBox("Button MAIN_FRAME pressed!"); break;
                case ID_TOOLBAR:OKMsgBox("Button ID_TOOLBAR pressed!"); break;
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
        HWND hwndSubframe = CellWindowFromID(hwndFrame, ID_LPANE);

        WinSetWindowPos(hwndFrame, NULLHANDLE,
        	            swp.x + swp.cx / 8,
            	        swp.y + swp.cy / 8,
                	    (swp.cx / 4) * 3,
                    	(swp.cy / 4) * 3,
	                    SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);


        CreateToolbar(hwndFrame, &mainTb);
        CreateToolbar(hwndFrame, &mainTb1);
        CreateToolbar(hwndFrame, &mainTb2);
        CreateToolbar(hwndFrame, &mainTb3);
        CreateToolbar(hwndFrame, &mainTb4);

        if(hwndSubframe)
            CreateToolbar(hwndSubframe, &mainTb5);

        while (WinGetMsg(hab, &qmsg, 0, 0, 0))
        	WinDispatchMsg(hab, &qmsg);

        WinDestroyWindow(hwndFrame);
    }

	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);

    return 0;
}



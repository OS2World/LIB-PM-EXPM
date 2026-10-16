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
    TB_ATTACHED_TP | TB_BUBBLE,
    ID_TOOLBAR,
    mainItems
};

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
        HWND hwndList;

        hwndList = CellWindowFromID(hwndFrame, ID_LPANE);

        if(hwndList)
        {
            CHAR cText[256];
            static char cListItem[] = "List item ";
            int i;

            strcpy(cText, cListItem);

            for(i = 0; i < 15; i++)
            {
                GenResIDStr(cText + sizeof(cListItem) - 1, i);
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


        CreateToolbar(hwndFrame, &mainTb);

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



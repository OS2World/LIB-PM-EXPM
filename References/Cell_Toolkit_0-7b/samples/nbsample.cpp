/*
** Module   :NBSAMPLE.C
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Fri  19/02/1998     Created
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
    ID_LPANE
};

CellDef cdRightPane =
{
    CELL_WINDOW,
    WC_MLE,
    "Sample Text",
    MLS_BORDER | WS_VISIBLE,
    ID_RPANE
};

CellDef nbClient =
{
    CELL_VSPLIT | CELL_SPLITBAR | CELL_SPLIT40x60,
    0,
    "Notebook page",
    0,
    MAIN_FRAME,
    &cdLeftPane,
    &cdRightPane,
    0,
    0
};

CellDef nbPanel   =
{
    CELL_WINDOW,
    WC_NOTEBOOK,
    "",
    WS_VISIBLE        |
    BKS_TABBEDDIALOG  |
    BKS_MAJORTABTOP   |
    0,
    ID_NOTEBOOK
};

CellDef mainFrame =
{
    CELL_HSPLIT,
    0,
    "Notebook Sample",
    FCF_TITLEBAR | FCF_SYSMENU  | FCF_MENU |
    FCF_MINMAX   | FCF_TASKLIST | FCF_SIZEBORDER |
    0,
    MAIN_FRAME,
    &nbPanel,
    0,
    0,
    MainClientProc
};

ULONG mainItems[]=
{
    IDB_FILENEW , IDB_FILEOPEN, IDB_FILESAVE, IDB_FILSAVAS, TB_SEPARATOR,
    IDB_EXIT    , TB_SEPARATOR, IDB_EDITCOPY, IDB_EDITCUT , IDB_EDITPAST,
    IDB_EDITUNDO, TB_SEPARATOR, IDB_EDITFIND, IDB_EDITFNNX, IDB_EDITREPL,
    TB_SEPARATOR, IDB_HELP    , IDB_ABOUT   , 0
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

    hwndFrame = CreateCell(&mainFrame, HWND_DESKTOP, 0);

    if(hwndFrame)
    {
        ULONG ulPageId;
        HWND hwndList;
	    HWND hwndTb;
    	HWND hwndNb;
	    HWND hwndPage;
        CHAR cText[256];
        static CHAR ppNbFont[] = "9.WarpSans";
        static CHAR cPage[] = "Page ";
        static CHAR cListItem[] = "List Item ";
        int i,j;

        hwndNb   = CellWindowFromID(hwndFrame, ID_NOTEBOOK);
        WinSetPresParam(hwndNb,
                        PP_FONTNAMESIZE,
                        sizeof(ppNbFont),
                        ppNbFont);

        for(i = 0; i < 15; i++)
        {
        	ulPageId = (ULONG) WinSendMsg(hwndNb,
            	                          BKM_INSERTPAGE,
                	                      0,
                    	                  MPFROM2SHORT(BKA_MAJOR |
                                                       BKA_AUTOPAGESIZE,
                                                       BKA_LAST));
            strcpy(cText, cPage);
            GenResIDStr(cText + sizeof(cPage) - 1, i + 1);
	        WinSendMsg(hwndNb,
    	               BKM_SETTABTEXT,
        	           MPFROMLONG(ulPageId),
                       MPFROMP(cText));

            hwndPage = CreateCell(&nbClient, hwndNb, hwndNb);
            hwndList = CellWindowFromID(hwndPage, ID_LPANE);

            for(j = 0; j < 15; j++)
            {
                strcpy(cText, cListItem);
                GenResIDStr(cText + sizeof(cListItem) - 1, j + 1);

                WinSendMsg(hwndList,
                           LM_INSERTITEM,
                           MPFROMSHORT(LIT_END),
                           MPFROMP(cText));
            }

        	WinSendMsg(hwndNb,
            	       BKM_SETPAGEWINDOWHWND,
                	   MPFROMLONG(ulPageId),
	                   MPFROMHWND(hwndPage));
        }

        WinSetWindowPos(hwndFrame, NULLHANDLE,
        	            swp.x + swp.cx / 8,
            	        swp.y + swp.cy / 8,
                	    (swp.cx / 4) * 3,
                    	(swp.cy / 4) * 3,
	                    SWP_ACTIVATE | SWP_MOVE | SWP_SIZE | SWP_SHOW);

        CreateToolbar(hwndFrame, &mainTb);

        while (WinGetMsg(hab, &qmsg, 0, 0, 0))
        	WinDispatchMsg(hab, &qmsg);

        WinDestroyWindow(hwndFrame);
    }

	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);

    return 0;
}



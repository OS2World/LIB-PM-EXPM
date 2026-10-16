/*
** Module   :CELL.CPP
** Abstract :Cell Toolkit
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Sun  08/02/98   Created.
**      Wed  25/10/2000 Updated to version 0.7b
*/

#define INCL_WIN
#define INCL_NLS
#define INCL_DOS
#define INCL_GPI

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <cell.h>

#define TKM_SEARCH_ID       WM_USER+0x1000
#define TKM_QUERY_FLAGS     WM_USER+0x1001
#define TKM_SEARCH_PARENT   WM_USER+0x1002

#define TB_ATTACHED         0x00F8

/*****************************************************************************
** Static data
*/

static CHAR CELL_CLIENT[] = "Uni.Cell.Client";
static CHAR TB_CLIENT[]   = "Uni.Tb.Client";
static CHAR TB_SEPCLASS[] = "Uni.Tb.Separator";
static CHAR ppFont[] = "9.WarpSans";

/* Color tables */

static LONG lColor[SPLITBAR_WIDTH] =
{
    CLR_BLACK,
#if (SPLITBAR_WIDTH>2)
    CLR_PALEGRAY,
#endif
    CLR_WHITE
};

static LONG lColor2[SPLITBAR_WIDTH] =
{
    CLR_WHITE,
#if (SPLITBAR_WIDTH>2)
    CLR_PALEGRAY,
#endif
    CLR_BLACK
};


/*****************************************************************************
** Internal prototypes
*/

MRESULT EXPENTRY CellProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CellClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TbProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TbClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TbSeparatorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY BtProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND CreateTb(TbDef* pTb, HWND hWndParent, HWND hWndOwner);
void RecalcTbDimensions(HWND hwnd, POINTL *pSize);
BOOL TrackRectangle(HWND hwndBase, RECTL* rclTrack, RECTL* rclBounds);

/*****************************************************************************
** Internal data types
*/

    /* Cell data, used by subclass proc of splitted window. */

typedef struct stCellTb
{
    HWND hWnd;
    struct stCellTb* pNext;
} CellTb;

typedef struct
{
    PFNWP pOldProc;
    RECTL rclBnd;
    LONG lType;
    LONG lSplit;
    LONG lSize;
    HWND hwndSplitbar;
    HWND hwndPanel1;
    HWND hwndPanel2;
    CellTb *pCellTb;
} CellCtlData;

	/* Toolbar data */

typedef struct stTbItemData
{
    PFNWP pOldProc;
    CHAR  cText[TB_BUBBLE_SIZE];
} TbItemData;

typedef struct stToolbarCtlData
{
    PFNWP pOldProc;
    HWND hwndParent;
    LONG lState;
    LONG lCount;

    BOOL bBubble;
    HWND hwndBubble;
    HWND hwndLast;

    HWND  hItems[1];
} TbCtlData;

static HAB hab;

/*
******************************************************************************
** Cell (Splitted view) implementation
******************************************************************************
*/

void ShowCell(HWND hwnd, LONG lID, BOOL bAction)
{
    HWND hwndMain = hwnd;
    CellCtlData* pCtlData = 0;
    LONG lCell = 0;

    hwnd = CellParentWindowFromID(hwnd, lID);

    if(!hwnd)
        return;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return;

    if(WinQueryWindowUShort(pCtlData->hwndPanel1, QWS_ID) == lID)
        lCell = CELL_HIDE_1;

    if(WinQueryWindowUShort(pCtlData->hwndPanel2, QWS_ID) == lID)
        lCell = CELL_HIDE_2;

    switch(lCell)
    {
        case CELL_HIDE_1:
            if(bAction == FALSE)
                pCtlData->lType |= CELL_HIDE_1;
            else
                pCtlData->lType &= ~CELL_HIDE_1;
            break;

        case CELL_HIDE_2:
            if(bAction == FALSE)
                pCtlData->lType |= CELL_HIDE_2;
            else
                pCtlData->lType &= ~CELL_HIDE_2;
            break;
    }

    if(lCell)
        WinSendMsg(hwnd, WM_UPDATEFRAME, 0, 0);
}

LONG GetSplit(HWND hwnd, LONG lID)
{
    CellCtlData* pCtlData = 0;

    hwnd = CellWindowFromID(hwnd, lID);

    if(!hwnd)
        return 0;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return 0;

    return pCtlData->lSplit;
}

LONG SetSplit(HWND hwnd, LONG lID, LONG lNewSplit)
{
    CellCtlData* pCtlData = 0;

    hwnd = CellWindowFromID(hwnd, lID);

    if(!hwnd)
        return 0;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return 0;

    if(!(pCtlData->lType & CELL_FIXED))
    {
        pCtlData->lSplit = lNewSplit;
        if(pCtlData->lSplit  > CELL_TOP_LIMIT)
            pCtlData->lSplit = CELL_TOP_LIMIT;

        if(pCtlData->lSplit  < CELL_BOTTOM_LIMIT)
            pCtlData->lSplit = CELL_BOTTOM_LIMIT;

        WinSendMsg(hwnd, WM_UPDATEFRAME, 0, 0);
    }

    return pCtlData->lSplit;
}

LONG GetSplitType(HWND hwnd, LONG lID)
{
    CellCtlData* pCtlData = 0;

    hwnd = CellWindowFromID(hwnd, lID);

    if(!hwnd)
        return 0;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return 0;

    return (pCtlData->lType & (CELL_VSPLIT | CELL_HSPLIT | CELL_SWAP));
}

void SetSplitType(HWND hwnd, LONG lID, LONG lNewSplit)
{
    CellCtlData* pCtlData = 0;

    hwnd = CellWindowFromID(hwnd, lID);

    if(!hwnd)
        return;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return;

    pCtlData->lType &= ~(CELL_VSPLIT | CELL_HSPLIT);
    pCtlData->lType |= lNewSplit & (CELL_VSPLIT | CELL_HSPLIT);

    if(lNewSplit & CELL_SWAP) //Swap required?
    {
        if(!(pCtlData->lType & CELL_SWAP)) //Not swapped yet
        {
            //Swap subwindows
            HWND hwndTmp = pCtlData->hwndPanel1;
            pCtlData->hwndPanel1 = pCtlData->hwndPanel2;
            pCtlData->hwndPanel2 = hwndTmp;
        }

        pCtlData->lType |= CELL_SWAP;
    }
    else
    {
        if(pCtlData->lType & CELL_SWAP) //Already swapped
        {
            //Restore original state
            HWND hwndTmp = pCtlData->hwndPanel1;
            pCtlData->hwndPanel1 = pCtlData->hwndPanel2;
            pCtlData->hwndPanel2 = hwndTmp;
        }

        pCtlData->lType &= ~CELL_SWAP;
    }
}

/* Function: CountControls
** Abstract: calculates number of additional controls in cell window
*/

USHORT CountControls(CellCtlData * pCtlData)
{
    USHORT itemCount = 0;
    CellTb *pCellTb = 0;

    if(pCtlData->hwndPanel1 && !(pCtlData->lType & CELL_HIDE_1))
        itemCount++;

    if(pCtlData->hwndPanel2 && !(pCtlData->lType & CELL_HIDE_2))
        itemCount++;

    pCellTb = pCtlData->pCellTb;

    while(pCellTb)
    {
        LONG lFlags = (LONG)WinSendMsg(pCellTb->hWnd,
                                       TKM_QUERY_FLAGS, 0, 0);
        if(lFlags & TB_ATTACHED)
            itemCount++;

        pCellTb = pCellTb->pNext;
    }

    return itemCount;
}

/* Function: CellProc
** Abstract: Subclass procedure for frame window
*/

MRESULT EXPENTRY CellProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    CellCtlData* pCtlData = 0;
    CellTb *pCellTb = 0;
    ULONG itemCount;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
    {
        return 0;
    }
    switch(msg)
    {
        case WM_ADJUSTWINDOWPOS:
            {
                HWND hwndBehind;
                PSWP pSwp;

                pCellTb = pCtlData->pCellTb;
                pSwp = (PSWP)mp1;

                if(!(pSwp->fl & SWP_ZORDER) || !pCellTb)
                   break;

                hwndBehind = pSwp->hwndInsertBehind;

                while(pCellTb)
                {
                    LONG lFlags = (LONG)WinSendMsg(pCellTb->hWnd,
                                                   TKM_QUERY_FLAGS, 0, 0);
                    if(!(lFlags & TB_ATTACHED))
                    {

                        WinSetWindowPos (pCellTb->hWnd,
                                         hwndBehind,
                                         0, 0, 0, 0, SWP_ZORDER);

                        hwndBehind = pCellTb->hWnd;
                    }

                    pCellTb = pCellTb->pNext;
                }
                pSwp->hwndInsertBehind = hwndBehind;
            }
            break;

        case TKM_SEARCH_PARENT:
            {
                HWND hwndRC;

                if(WinQueryWindowUShort(hwnd, QWS_ID) == (ULONG)mp1)
                    return MRFROMSHORT(FALSE);

                if(WinQueryWindowUShort(pCtlData->hwndPanel1, QWS_ID)
                    == (ULONG)mp1)
                    return (MPARAM)hwnd;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel1,
                                          TKM_SEARCH_PARENT,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;

                if(WinQueryWindowUShort(pCtlData->hwndPanel2, QWS_ID)
                    == (ULONG)mp1)
                    return (MPARAM)hwnd;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel2,
                                          TKM_SEARCH_PARENT,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;

            }
            return MRFROMSHORT(FALSE);

        case TKM_SEARCH_ID:
            {
                HWND hwndRC;
                pCellTb = pCtlData->pCellTb;

                while(pCellTb)
                {
                    if(WinQueryWindowUShort(pCellTb->hWnd, QWS_ID)
                        == (ULONG)mp1)
                        return (MPARAM)pCellTb->hWnd;

                    hwndRC = (HWND)WinSendMsg(pCellTb->hWnd,
                                              TKM_SEARCH_ID,
                                              mp1, 0);

                    if(hwndRC)
                        return (MPARAM)hwndRC;

                    pCellTb = pCellTb->pNext;
                }

                if(WinQueryWindowUShort(hwnd, QWS_ID) == (ULONG)mp1)
                    return (MPARAM)hwnd;

                if(WinQueryWindowUShort(pCtlData->hwndPanel1, QWS_ID)
                    == (ULONG)mp1)
                    return (MPARAM)pCtlData->hwndPanel1;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel1,
                                          TKM_SEARCH_ID,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;

                if(WinQueryWindowUShort(pCtlData->hwndPanel2, QWS_ID)
                    == (ULONG)mp1)
                    return (MPARAM)pCtlData->hwndPanel2;

                hwndRC = (HWND)WinSendMsg(pCtlData->hwndPanel2,
                                          TKM_SEARCH_ID,
                                          mp1, 0);
                if(hwndRC)
                    return (MPARAM)hwndRC;

            }
            return MRFROMSHORT(FALSE);

        case WM_QUERYFRAMECTLCOUNT:
            {
                itemCount = (ULONG)pCtlData->pOldProc(hwnd, msg, mp1, mp2);
                itemCount += CountControls(pCtlData);
            }
            return MRFROMSHORT(itemCount);

        case WM_FORMATFRAME:
            {
                PSWP   pSWP     = 0;
                USHORT usClient = 0;
                USHORT itemCount2;
                SWP    swp;
                HWND   hClient = HWND_TOP;

                itemCount  = (ULONG)pCtlData->pOldProc(hwnd, msg, mp1, mp2);
                itemCount2 = CountControls(pCtlData);

                if(!itemCount2 || itemCount < 1)
                    return MRFROMSHORT(itemCount);

                pSWP = (PSWP)PVOIDFROMMP(mp1);

                usClient = itemCount - 1;

                hClient  = pSWP[usClient].hwnd;

                /*
                ** Cutting client window.
                ** If there are any attached toolbars, cut client window
                ** regarding to attachment type
                */

                /* Toolbars attached to top and bottom sides */

                pCellTb = pCtlData->pCellTb;

                while(pCellTb)
                {
                    POINTL ptlSize;
                    LONG lFlags = (LONG)WinSendMsg(pCellTb->hWnd,
                                                   TKM_QUERY_FLAGS, 0, 0);

                    if(!(lFlags & TB_ATTACHED))
                    {
                        pCellTb = pCellTb->pNext;
                        continue;
                    }

                    RecalcTbDimensions(pCellTb->hWnd, &ptlSize);

                    switch(lFlags & TB_ATTACHED)
                    {
                        case TB_ATTACHED_TP:
                            pSWP[itemCount].x    = pSWP[usClient].x;
                            pSWP[itemCount].y    = pSWP[usClient].y +
                                                   pSWP[usClient].cy - ptlSize.y;
                            pSWP[itemCount].cx   = pSWP[usClient].cx;
                            pSWP[itemCount].cy   = ptlSize.y;
                            pSWP[itemCount].fl   = SWP_SIZE |
                                                   SWP_MOVE |
                                                   SWP_SHOW;

                            pSWP[itemCount].hwnd = pCellTb->hWnd;
                            pSWP[itemCount].hwndInsertBehind = hClient;
                            hClient = pSWP[itemCount].hwnd;

                            pSWP[usClient].cy -= ptlSize.y;
                            itemCount++;
                            break;

                        case TB_ATTACHED_BT:
                            pSWP[itemCount].x    = pSWP[usClient].x;
                            pSWP[itemCount].y    = pSWP[usClient].y;
                            pSWP[itemCount].cx   = pSWP[usClient].cx;
                            pSWP[itemCount].cy   = ptlSize.y;
                            pSWP[itemCount].fl   = SWP_SIZE | SWP_MOVE | SWP_SHOW;

                            pSWP[itemCount].hwnd = pCellTb->hWnd;
                            pSWP[itemCount].hwndInsertBehind = hClient;
                            hClient = pSWP[itemCount].hwnd;

                            pSWP[usClient].cy -= ptlSize.y;
                            pSWP[usClient].y  += ptlSize.y;
                            itemCount++;
                            break;
                    }
                    pCellTb = pCellTb->pNext;
                }

                /*Toolbars attached to left and right sides*/

                pCellTb = pCtlData->pCellTb;

                while(pCellTb)
                {
                    POINTL ptlSize;
                    LONG lFlags = (LONG)WinSendMsg(pCellTb->hWnd,
                                                   TKM_QUERY_FLAGS, 0, 0);

                    if(!(lFlags & TB_ATTACHED))
                    {
                        pCellTb = pCellTb->pNext;
                        continue;
                    }

                    RecalcTbDimensions(pCellTb->hWnd, &ptlSize);

                    switch(lFlags & TB_ATTACHED)
                    {
                        case TB_ATTACHED_LT:
                            pSWP[itemCount].x    = pSWP[usClient].x;
                            pSWP[itemCount].y    = pSWP[usClient].y;
                            pSWP[itemCount].cx   = ptlSize.x;
                            pSWP[itemCount].cy   = pSWP[usClient].cy;
                            pSWP[itemCount].fl   = SWP_SIZE | SWP_MOVE | SWP_SHOW;
                            pSWP[itemCount].hwnd = pCellTb->hWnd;
                            pSWP[itemCount].hwndInsertBehind = hClient;
                            hClient = pSWP[itemCount].hwnd;

                            pSWP[usClient].cx -= ptlSize.x;
                            pSWP[usClient].x  += ptlSize.x;
                            itemCount++;
                            break;

                        case TB_ATTACHED_RT:
                            pSWP[itemCount].x    = pSWP[usClient].x +
                                                   pSWP[usClient].cx - ptlSize.x;
                            pSWP[itemCount].y    = pSWP[usClient].y;
                            pSWP[itemCount].cx   = ptlSize.x;
                            pSWP[itemCount].cy   = pSWP[usClient].cy;
                            pSWP[itemCount].fl   = SWP_SIZE | SWP_MOVE | SWP_SHOW;
                            pSWP[itemCount].hwnd = pCellTb->hWnd;
                            pSWP[itemCount].hwndInsertBehind = hClient;
                            hClient = pSWP[itemCount].hwnd;

                            pSWP[usClient].cx -= ptlSize.x;
                            itemCount++;
                            break;
                    }
                    pCellTb = pCellTb->pNext;
                }

                /*
                ** Placing panels.
                ** Remember client rect for future use
                ** They will save time when we start moving splitbar
                */

                pCtlData->rclBnd.xLeft   = pSWP[usClient].x;
                pCtlData->rclBnd.xRight  = pSWP[usClient].x + pSWP[usClient].cx;
                pCtlData->rclBnd.yTop    = pSWP[usClient].y + pSWP[usClient].cy;
                pCtlData->rclBnd.yBottom = pSWP[usClient].y;

                if(!pCtlData->hwndPanel1 || !pCtlData->hwndPanel2 ||
                   (pCtlData->lType & CELL_HIDE))
                {
                    /*
                    **single subwindow;
                    **In this case we don't need a client window,
                    **because of lack of splitbar.
                    **Just copy all data from pSWP[usClient]
                    **and replace some part of it
                    */

                    pSWP[itemCount]     = pSWP[usClient];
                    pSWP[itemCount].fl |= SWP_MOVE | SWP_SIZE;
                    pSWP[itemCount].hwndInsertBehind = HWND_TOP;
                    pSWP[usClient].cy = 0;

                    pSWP[itemCount].hwnd = 0;

                    if(pCtlData->hwndPanel1 && !(pCtlData->lType & CELL_HIDE_1))
                        pSWP[itemCount].hwnd = pCtlData->hwndPanel1;

                    if(pCtlData->hwndPanel2 && !(pCtlData->lType & CELL_HIDE_2))
                        pSWP[itemCount].hwnd = pCtlData->hwndPanel2;

                    /* Increase number of controls */

                    if(pSWP[itemCount].hwnd)
                    {
                        pSWP[itemCount].hwndInsertBehind = hClient;
                        hClient = pSWP[itemCount].hwnd;
                        itemCount++;
                    }
                }
                else
                {
                    USHORT usPanel1 = itemCount;
                    USHORT usPanel2 = itemCount + 1;
                    USHORT usWidth1 = 0;
                    USHORT usWidth2 = 0;

                    /* Just like case of one panel */

                    pSWP[usPanel1] = pSWP[usClient];
                    pSWP[usPanel2] = pSWP[usClient];

                    pSWP[usPanel1].fl |= SWP_MOVE | SWP_SIZE;
                    pSWP[usPanel2].fl |= SWP_MOVE | SWP_SIZE;

                    pSWP[usPanel1].hwndInsertBehind = hClient;
                    pSWP[usPanel2].hwndInsertBehind = pCtlData->hwndPanel1;

                    pSWP[usPanel1].hwnd = pCtlData->hwndPanel1;
                    pSWP[usPanel2].hwnd = pCtlData->hwndPanel2;

                    hClient = pCtlData->hwndPanel2;

                    if(pCtlData->lType & CELL_VSPLIT)
                    {
                        if((pCtlData->lType & CELL_FIXED) &&
                           (pCtlData->lType & (CELL_SIZE1 | CELL_SIZE2)) &&
                           (pCtlData->lSize > 0))
                        {
                            /* Case of fixed panel with exact size */

                            if(pCtlData->lType & CELL_SIZE1)
                            {
                                usWidth1 = pCtlData->lSize;
                                usWidth2 = pSWP[usClient].cx - usWidth1;
                            }
                            else
                            {
                                usWidth2 = pCtlData->lSize;
                                usWidth1 = pSWP[usClient].cx - usWidth2;
                            }
                        }
                        else
                        {
                            usWidth1 = (pSWP[usClient].cx * pCtlData->lSplit)
                                        / 100;
                            usWidth2 = pSWP[usClient].cx - usWidth1;
                        }

                        if(pCtlData->lType & CELL_SPLITBAR)
                        {
                            if(!(pCtlData->lType & CELL_SIZE1))
                                usWidth2 -= SPLITBAR_WIDTH;
                            else
                                usWidth1 -= SPLITBAR_WIDTH;

                            pSWP[usClient].cx = SPLITBAR_WIDTH;
                            pSWP[usClient].x  = pSWP[usClient].x + usWidth1;
                        }
                        else
                        {
                            pSWP[usClient].cx = 0;
                            pSWP[usClient].cy = 0;
                        }

                        pSWP[usPanel1].cx  = usWidth1;
                        pSWP[usPanel2].x  += usWidth1 + pSWP[usClient].cx;
                        pSWP[usPanel2].cx  = usWidth2;
                    }
                    else
                    {
                        if((pCtlData->lType & CELL_FIXED) &&
                           (pCtlData->lType & (CELL_SIZE1 | CELL_SIZE2)) &&
                           (pCtlData->lSize > 0))
                        {
                            /* Case of fixed panel with exact size */

                            if(pCtlData->lType & CELL_SIZE1)
                            {
                                usWidth1 = pCtlData->lSize;
                                usWidth2 = pSWP[usClient].cy - usWidth1;
                            }
                            else
                            {
                                usWidth2 = pCtlData->lSize;
                                usWidth1 = pSWP[usClient].cy - usWidth2;
                            }
                        }
                        else
                        {
                            usWidth1 = (pSWP[usClient].cy * pCtlData->lSplit)
                                       / 100;
                            usWidth2 = pSWP[usClient].cy - usWidth1;
                        }

                        if(pCtlData->lType & CELL_SPLITBAR)
                        {
                            if(!(pCtlData->lType & CELL_SIZE1))
                                usWidth2 -= SPLITBAR_WIDTH;
                            else
                                usWidth1 -= SPLITBAR_WIDTH;
                            pSWP[usClient].cy = SPLITBAR_WIDTH;
                            pSWP[usClient].y  = pSWP[usClient].y + usWidth1;
                        }
                        else
                        {
                            pSWP[usClient].cx = 0;
                            pSWP[usClient].cy = 0;
                        }

                        pSWP[usPanel1].cy  = usWidth1;
                        pSWP[usPanel2].y  += usWidth1 + pSWP[usClient].cy;
                        pSWP[usPanel2].cy  = usWidth2;
                    }

                    itemCount += 2;
                }
            }
            return MRFROMSHORT(itemCount);
    }
    return pCtlData->pOldProc(hwnd, msg, mp1, mp2);
}

/* Function: CellClientProc
** Abstract: Window procedure for Cell Client Window Class (splitbar)
*/

MRESULT EXPENTRY CellClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = 0;
    CellCtlData *pCtlData = 0;

    hwndFrame = WinQueryWindow(hwnd, QW_PARENT);

    if(hwndFrame)
        pCtlData = (CellCtlData *)WinQueryWindowULong(hwndFrame, QWL_USER);

    if(!hwndFrame || !pCtlData)
        return WinDefWindowProc(hwnd, msg, mp1, mp2);

    switch (msg)
    {
        case WM_ACTIVATE:
        case WM_SETFOCUS:
            return (MRESULT)(FALSE);

        case WM_PAINT:
            {
                HPS hpsPaint;
                RECTL rclPaint;
                POINTL ptlStart[SPLITBAR_WIDTH];
                POINTL ptlEnd[SPLITBAR_WIDTH];
                PLONG pColorTable;
                USHORT i;

                hpsPaint = WinBeginPaint(hwnd, 0, 0);
                WinQueryWindowRect(hwnd, &rclPaint);

                if(pCtlData->lType & CELL_VSPLIT)
                {
                    for(i = 0; i < SPLITBAR_WIDTH; i++)
                    {
                        ptlStart[i].x = rclPaint.xLeft + i;
                        ptlStart[i].y = rclPaint.yTop;

                        ptlEnd[i].x = rclPaint.xLeft + i;
                        ptlEnd[i].y = rclPaint.yBottom;
                    }
                    pColorTable = (pCtlData->lType & CELL_FIXED)
                                   ? lColor
                                   : lColor2;
                }
                else
                {
                    for(i = 0; i < SPLITBAR_WIDTH; i++)
                    {
                        ptlStart[i].x = rclPaint.xLeft;
                        ptlStart[i].y = rclPaint.yBottom + i;

                        ptlEnd[i].x = rclPaint.xRight;
                        ptlEnd[i].y = rclPaint.yBottom + i;
                    }
                    pColorTable = (pCtlData->lType & CELL_FIXED)
                                   ? lColor2
                                   : lColor;
                }

                for(i = 0; i < SPLITBAR_WIDTH; i++)
                {
                    GpiSetColor( hpsPaint, pColorTable[i]);
                    GpiMove(hpsPaint, &ptlStart[i]);
                    GpiLine(hpsPaint, &ptlEnd[i]);
                }
                WinEndPaint(hpsPaint);
            }
            return MRFROMSHORT(FALSE);

        case WM_MOUSEMOVE:
            {
                if(pCtlData->lType & CELL_FIXED)
                    break;

                if(pCtlData->lType & CELL_VSPLIT)
                    WinSetPointer(HWND_DESKTOP,
                                  WinQuerySysPointer(HWND_DESKTOP,
                                                     SPTR_SIZEWE,
                                                     FALSE));
                else
                    WinSetPointer(HWND_DESKTOP,
                                  WinQuerySysPointer(HWND_DESKTOP,
                                                     SPTR_SIZENS,
                                                     FALSE));
            }
            return MRFROMSHORT(FALSE);

        case WM_BUTTON1DOWN:
            {
                APIRET rc;
                RECTL  rclFrame;
                RECTL  rclBounds;

                if(pCtlData->lType & CELL_FIXED)
                    break;

                WinQueryWindowRect(hwnd, &rclFrame);

                rclBounds = pCtlData->rclBnd;

                WinMapWindowPoints(hwndFrame, HWND_DESKTOP,
                                   (PPOINTL)&rclBounds, 2);

                rc = TrackRectangle(hwnd, &rclFrame, &rclBounds);

                if(rc == TRUE)
                {
                    USHORT usNewRB;
                    USHORT usSize;

                    if(pCtlData->lType & CELL_VSPLIT)
                    {
                        usNewRB = rclFrame.xLeft
                                - rclBounds.xLeft;
                        usSize  = rclBounds.xRight
                                - rclBounds.xLeft;
                    }
                    else
                    {
                        usNewRB = rclFrame.yBottom
                                - rclBounds.yBottom;
                        usSize  = rclBounds.yTop
                                - rclBounds.yBottom;
                    }

                    pCtlData->lSplit = (usNewRB * 100)/usSize;
                    if(pCtlData->lSplit  > CELL_TOP_LIMIT)
                        pCtlData->lSplit = CELL_TOP_LIMIT;

                    if(pCtlData->lSplit  < CELL_BOTTOM_LIMIT)
                        pCtlData->lSplit = CELL_BOTTOM_LIMIT;

                    WinSendMsg(hwndFrame, WM_UPDATEFRAME, 0, 0);
                }
            }
            return MRFROMSHORT(FALSE);

        case WM_BUTTON2DOWN:

            if(pCtlData->lType & CELL_FIXED)
                break;

            {
                ULONG lType = pCtlData->lType & (CELL_VSPLIT | CELL_HSPLIT);

                pCtlData->lType &= ~(CELL_VSPLIT | CELL_HSPLIT);

                if(lType & CELL_VSPLIT)
                {
                    pCtlData->lType |= CELL_HSPLIT;
                }
                else
                {
                    pCtlData->lType |= CELL_VSPLIT;
                }

                //Swap subwindows

                if(lType & CELL_VSPLIT)
                {
                    HWND hwndTmp = pCtlData->hwndPanel1;
                    pCtlData->hwndPanel1 = pCtlData->hwndPanel2;
                    pCtlData->hwndPanel2 = hwndTmp;

                    pCtlData->lType ^= CELL_SWAP;
                }

                if(pCtlData->lType & CELL_HIDE_1)
                {
                    pCtlData->lType &= ~CELL_HIDE_1;
                    pCtlData->lType |=  CELL_HIDE_2;
                }
                else
                {
                    if(pCtlData->lType & CELL_HIDE_2)
                    {
                        pCtlData->lType &= ~CELL_HIDE_2;
                        pCtlData->lType |=  CELL_HIDE_1;
                    }
                }

                if(pCtlData->lType & CELL_SIZE1)
                {
                    pCtlData->lType &= ~CELL_SIZE1;
                    pCtlData->lType |=  CELL_SIZE2;
                }
                else
                {
                    if(pCtlData->lType & CELL_SIZE2)
                    {
                        pCtlData->lType &= ~CELL_SIZE2;
                        pCtlData->lType |=  CELL_SIZE1;
                    }
                }

                WinSendMsg(hwndFrame, WM_UPDATEFRAME, 0, 0);
            }
            return MRFROMSHORT(FALSE);
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* Function: CreateCell
** Abstract: Creates a subwindows tree for a given CellDef
** Note: If hWndOwner == NULLHANDLE, and first CellDef is frame,
**       all subwindows will have this frame window as Owner.
*/

HWND CreateCell(CellDef* pCell, HWND hWndParent, HWND hWndOwner)
{
    HWND hwndFrame        = NULLHANDLE;
    CellCtlData* pCtlData = 0;
    WindowCellCtlData * pWCtlData = 0;

    if(!pCell)
        return hwndFrame;

    switch(pCell->lType & (CELL_VSPLIT | CELL_HSPLIT | CELL_WINDOW))
    {
        case CELL_WINDOW:
            hwndFrame = WinCreateWindow(hWndParent,
                                        pCell->pszClass,
                                        pCell->pszName,
                                        pCell->ulStyle,
                                    	0, 0, 0, 0,
                                        hWndOwner,
                                    	HWND_TOP,
                                        pCell->ulID,
                                    	NULL,
                                        NULL);

            if(pCell->pClassProc && hwndFrame)
            {
                pWCtlData = (WindowCellCtlData *)
                             malloc(sizeof(WindowCellCtlData));
                if(!pWCtlData)
                	return hwndFrame;

                memset(pWCtlData, 0, sizeof(WindowCellCtlData));
                pWCtlData->pOldProc = WinSubclassWindow(hwndFrame,
                                                        pCell->pClassProc);
                WinSetWindowULong(hwndFrame, QWL_USER, (ULONG)pWCtlData);
            }
            break;

        case CELL_HSPLIT:
        case CELL_VSPLIT:

            pCtlData = (CellCtlData *)malloc(sizeof(CellCtlData));
            if(!pCtlData)
                return hwndFrame;

            memset(pCtlData, 0, sizeof(CellCtlData));

            pCtlData->lType = pCell->lType & (CELL_SPLIT_MASK | CELL_HIDE);
            pCtlData->lSize = (pCell->lType & (CELL_SIZE1
                                               | CELL_SIZE2
                                               | CELL_FIXED))
                                ? pCell->lSize
                                : 0;

            pCtlData->lSplit = 50;

            switch(pCell->lType & CELL_SPLIT_REL)
            {
                case CELL_SPLIT10x90: pCtlData->lSplit = 10; break;
                case CELL_SPLIT20x80: pCtlData->lSplit = 20; break;
                case CELL_SPLIT30x70: pCtlData->lSplit = 30; break;
                case CELL_SPLIT40x60: pCtlData->lSplit = 40; break;
                case CELL_SPLIT50x50: pCtlData->lSplit = 50; break;
                case CELL_SPLIT60x40: pCtlData->lSplit = 60; break;
                case CELL_SPLIT70x30: pCtlData->lSplit = 70; break;
                case CELL_SPLIT80x20: pCtlData->lSplit = 80; break;
                case CELL_SPLIT90x10: pCtlData->lSplit = 90; break;
            }

            hwndFrame = WinCreateStdWindow(hWndParent,
                                           WS_VISIBLE,
                                           &pCell->ulStyle,
                                           CELL_CLIENT,
                                           "",
                                           0L, 0,
                                           pCell->ulID,
                                           &pCtlData->hwndSplitbar);

            WinSetOwner(hwndFrame, hWndOwner);

            if(pCell->pClassProc)
                pCtlData->pOldProc = WinSubclassWindow(hwndFrame,
                                                       pCell->pClassProc);
            else
                pCtlData->pOldProc = WinSubclassWindow(hwndFrame, CellProc);

            if(pCell->pClientClassProc)
            {
                pWCtlData = (WindowCellCtlData *)
                             malloc(sizeof(WindowCellCtlData));
                if(!pWCtlData)
                	return hwndFrame;

                memset(pWCtlData, 0, sizeof(WindowCellCtlData));

                pWCtlData->pOldProc = WinSubclassWindow(pCtlData->hwndSplitbar,
                                                       pCell->pClientClassProc);
                WinSetWindowULong(pCtlData->hwndSplitbar,
                                  QWL_USER,
                                  (ULONG)pWCtlData);
            }

            if(!hWndOwner)
                hWndOwner = hwndFrame;
            else
                WinSetOwner(pCtlData->hwndSplitbar, hWndOwner);

            pCtlData->hwndPanel1 = CreateCell(pCell->pPanel1,
                                              hwndFrame,
                                              hWndOwner);
            pCtlData->hwndPanel2 = CreateCell(pCell->pPanel2,
                                              hwndFrame,
                                              hWndOwner);

            WinSetWindowULong(hwndFrame, QWL_USER, (ULONG)pCtlData);
            break;
    }

    return hwndFrame;
}

/*****************************************************************************
** Toolbar implementation
*/

/* Function: CreateTb
** Abstract: Creates Toolbar for a gived TbDef
*/

HWND CreateTb(TbDef* pTb, HWND hWndParent, HWND hWndOwner)
{
    SWP   swp;
    HWND  hwndClient;
    HWND  hwndTb = NULLHANDLE;
    LONG  lCount;
    POINTL ptlSize;
    POINTL ptlFSize;
    ULONG  flCreate;
    TbCtlData  *pTbCtlData;
    TbItemData *pTbItemData;

    if(!pTb)
        return hwndTb;

    for(lCount = 0; pTb->tbItems[lCount]; )
        lCount++;

    pTbCtlData  = (TbCtlData *)malloc(sizeof(TbCtlData) +
                                      sizeof(HWND) * lCount);

    if(!pTbCtlData)
        return hwndTb;

    pTbItemData = (TbItemData *)malloc(sizeof(TbItemData) * lCount);

    if(!pTbItemData)
    {
        free(pTbCtlData);
        return hwndTb;
    }

    memset(pTbCtlData, 0, sizeof(TbCtlData) +
                          sizeof(HWND) * lCount);

    memset(pTbItemData, 0, sizeof(TbItemData) * lCount);

    pTbCtlData->lCount  = lCount;
    pTbCtlData->bBubble = (pTb->lType & TB_BUBBLE) ? 1:0;

    pTb->lType &= TB_ALLOWED;

    /*
    **Some checks:
    ** if toolbar attached, they should be properly
    ** oriented. I.e. toolbar attached to top or
    ** bottom, can't be vertical.
    */

    if(pTb->lType & (TB_ATTACHED_TP | TB_ATTACHED_BT))
        pTb->lType &= ~TB_VERTICAL;

    pTbCtlData->lState = pTb->lType;
    pTbCtlData->hwndParent = hWndParent;

    if(!(pTb->lType & TB_ATTACHED))
        hWndParent = HWND_DESKTOP;

    if(pTb->lType & TB_ATTACHED)
        flCreate = FCF_BORDER | FCF_NOBYTEALIGN;
    else
        flCreate = FCF_DLGBORDER | FCF_NOBYTEALIGN;

    hwndTb = WinCreateStdWindow(hWndParent,
                                WS_CLIPCHILDREN |
                                WS_CLIPSIBLINGS |
                                WS_PARENTCLIP |
                                0,
                                &flCreate,
                                TB_CLIENT,
                                "",
                                0L, 0,
                                pTb->ulID,
                                &hwndClient);
    if(!hwndTb)
    {
        free(pTbCtlData);
        free(pTbItemData);
        return hwndTb;
    }

    ptlSize.x = (pTbCtlData->lState & TB_VERTICAL) ? 0 : HAND_SIZE;
    ptlSize.y = (pTbCtlData->lState & TB_VERTICAL) ? HAND_SIZE : 0;

    for(lCount = 0; lCount < pTbCtlData->lCount; lCount++)
    {
        CHAR cButtText[256];

        if(pTb->tbItems[lCount] == TB_SEPARATOR)
            pTbCtlData->hItems[lCount] =
            	WinCreateWindow(hwndTb,
                                TB_SEPCLASS,
                                "",
                                0,
                                0, 0, TB_SEP_SIZE, TB_SEP_SIZE,
                                hwndTb,
                                HWND_TOP,
                                pTb->tbItems[lCount],
                                NULL,
                                NULL);
        else
        {
	        flCreate = BS_PUSHBUTTON | BS_BITMAP |
    	               BS_AUTOSIZE | BS_NOPOINTERFOCUS;

            GenResIDStr(cButtText, pTb->tbItems[lCount]);

            pTbCtlData->hItems[lCount] =
    	                            WinCreateWindow(hwndTb,
                                                    WC_BUTTON,
                    	                            cButtText,
            	                                    flCreate,
                                                    -1, -1, -1, -1,
                                                	hWndOwner,
                                                	HWND_TOP,
                                                    pTb->tbItems[lCount],
            	                                    NULL,
                                                    NULL);

            pTbItemData[lCount].pOldProc =
                      WinSubclassWindow(pTbCtlData->hItems[lCount],
                                        BtProc);

           WinSetWindowULong(pTbCtlData->hItems[lCount],
                             QWL_USER,
                             (ULONG)(&pTbItemData[lCount]));
        }

        WinQueryWindowPos(pTbCtlData->hItems[lCount], &swp);

        if(pTbCtlData->lState & TB_VERTICAL)
        {
            if(swp.cx > ptlSize.x)
                ptlSize.x = swp.cx;
            ptlSize.y += swp.cy;
        }
        else
        {
        	if(swp.cy > ptlSize.y)
            	ptlSize.y = swp.cy;
            ptlSize.x += swp.cx;
        }
    }

    /*
    ** Now we have calculated client window size for toolbar
    ** Recalculate its proper size
    */

    WinSendMsg(hwndTb, WM_QUERYBORDERSIZE, MPFROMP(&ptlFSize), 0);
    ptlSize.x += ptlFSize.x * 2;
    ptlSize.y += ptlFSize.y * 2;

    pTbCtlData->pOldProc = WinSubclassWindow(hwndTb, TbProc);
    WinSetWindowULong(hwndTb, QWL_USER, (ULONG)pTbCtlData);

    WinQueryWindowPos(hWndOwner, &swp);

    WinSetWindowPos(hwndTb, 0, swp.x + HAND_SIZE/2, swp.y + HAND_SIZE/2,
                    ptlSize.x, ptlSize.y, SWP_MOVE | SWP_SIZE | SWP_SHOW);
    return hwndTb;
}

/* Function: BtProc
** Abstract: Subclass procedure for buttons
*/

MRESULT EXPENTRY BtProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = 0;
    TbCtlData  *pTbCtlData;
    TbItemData *pTbItemData;

    hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
    pTbCtlData  = (TbCtlData *)WinQueryWindowULong(hwndFrame,
                                                   QWL_USER);

    pTbItemData = (TbItemData *)WinQueryWindowULong(hwnd,
                                                    QWL_USER);
    switch(msg)
    {
        case WM_TIMER:
            if (pTbCtlData->hwndBubble)
			{
                WinDestroyWindow(pTbCtlData->hwndBubble);
                pTbCtlData->hwndBubble = 0;
                WinStopTimer(hab, hwnd, 1);
			}
            break;

        case WM_MOUSEMOVE:

            if(!(pTbCtlData->lState & TB_BUBBLE))
                break;

            if(!pTbCtlData->bBubble)
                break;

            if(WinQueryActiveWindow(HWND_DESKTOP) != hwndFrame &&
               WinQueryActiveWindow(HWND_DESKTOP) != pTbCtlData->hwndParent)
            break;

            if(pTbCtlData->hwndLast == hwnd)
                break;

            if(pTbCtlData->hwndBubble)
            {
                WinDestroyWindow(pTbCtlData->hwndBubble);
                pTbCtlData->hwndBubble = 0;
                WinStopTimer(hab, pTbCtlData->hwndLast, 1);
            }

            if(!pTbCtlData->hwndBubble)
            {
                HWND  hwndBubbleClient;
                ULONG ulStyle = FCF_BORDER | FCF_NOBYTEALIGN;
                HPS   hpsTemp = 0;
                LONG  lHight;
                LONG  lWidth;
                POINTL txtPointl[TXTBOX_COUNT];
                POINTL ptlWork = {0,0};
                ULONG ulColor = CLR_PALEGRAY;

                pTbCtlData->hwndLast   = hwnd;
                pTbCtlData->hwndBubble = WinCreateStdWindow(HWND_DESKTOP,
                                                            0,
                                                            &ulStyle,
                                                            WC_STATIC,
                                                            "",
                                                            SS_TEXT |
                                                            DT_LEFT |
                                                            DT_VCENTER,
                                                            NULLHANDLE,
                                                            TB_BUBBLEID,
                                                            &hwndBubbleClient);

                WinSetPresParam(hwndBubbleClient,
                                PP_FONTNAMESIZE,
                                sizeof(ppFont),
                                ppFont);


                WinSetPresParam(hwndBubbleClient,
                                PP_BACKGROUNDCOLORINDEX,
                                sizeof(ulColor),
                                &ulColor);

                if(!pTbItemData->cText[0])
                {
                    WinLoadString(hab,
                                  0,
                                  WinQueryWindowUShort(hwnd, QWS_ID),
                                  sizeof(pTbItemData->cText),
                                  pTbItemData->cText);
                }

                WinSetWindowText(hwndBubbleClient,
                                 pTbItemData->cText);

                WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptlWork, 1);

                hpsTemp = WinGetPS(hwndBubbleClient);
                GpiQueryTextBox(hpsTemp,
                                strlen(pTbItemData->cText),
                                pTbItemData->cText,
                                TXTBOX_COUNT,
                                txtPointl);

                WinReleasePS(hpsTemp);

                lWidth = txtPointl[TXTBOX_TOPRIGHT].x -
                         txtPointl[TXTBOX_TOPLEFT ].x +
                         WinQuerySysValue(HWND_DESKTOP,
                                          SV_CYDLGFRAME) * 2;

                lHight = txtPointl[TXTBOX_TOPLEFT   ].y -
                         txtPointl[TXTBOX_BOTTOMLEFT].y +
                         WinQuerySysValue(HWND_DESKTOP,
                                          SV_CXDLGFRAME) * 2;

                if(!(pTbCtlData->lState & TB_VERTICAL))
                    ptlWork.y -= lHight;
                else
                {
                    RECTL rclButton;
                    WinQueryWindowRect(hwnd, &rclButton);
                    ptlWork.x += rclButton.xRight - rclButton.xLeft;
                }

                WinSetWindowPos(pTbCtlData->hwndBubble,
                                HWND_TOP,
                                ptlWork.x,
                                ptlWork.y,
                                lWidth,
                                lHight,
                                SWP_SIZE | SWP_MOVE | SWP_SHOW);

                WinStartTimer(hab, hwnd, 1, 1500);
            }
            break;
    }

    return pTbItemData->pOldProc(hwnd, msg, mp1, mp2);
}

/* Function: TbProc
** Abstract: Subclass procedure for toolbar window
*/

MRESULT EXPENTRY TbProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    TbCtlData* pTbCtlData = 0;
    ULONG itemCount;

    pTbCtlData = (TbCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pTbCtlData)
    {
        return 0;
    }
    switch(msg)
    {

/* Internal messages */

        case TKM_SEARCH_ID:
            {
                LONG lCount;

                for(lCount = 0; lCount < pTbCtlData->lCount; lCount++)
                {
                    if(WinQueryWindowUShort(pTbCtlData->hItems[lCount],
                                            QWS_ID) == (ULONG)mp1)
                        return (MRESULT)pTbCtlData->hItems[lCount];
                }
            }
            return MRFROMSHORT(FALSE);

        case TKM_QUERY_FLAGS:
            return MRFROMLONG(pTbCtlData->lState);

/* Standard messages */

        case WM_QUERYFRAMECTLCOUNT:
            {
                itemCount = (ULONG)(pTbCtlData->pOldProc(hwnd, msg, mp1, mp2));
                itemCount += pTbCtlData->lCount;
            }
            return MRFROMSHORT(itemCount);

        case WM_FORMATFRAME:
            {
                PSWP   pSWP      = 0;
                USHORT usClient  = 0;
                LONG   lOffset = 0;
                LONG   lCount;
                SWP    swp;

                itemCount = (ULONG)(pTbCtlData->pOldProc(hwnd, msg, mp1, mp2));

                pSWP = (PSWP)PVOIDFROMMP(mp1);

                while(pSWP[usClient].hwnd != WinWindowFromID(hwnd, FID_CLIENT))
                    usClient++;

                if(pTbCtlData->lState & TB_VERTICAL)
                    lOffset = pSWP[usClient].cy - HAND_SIZE;
                else
                    lOffset = HAND_SIZE + 1;


                for(lCount = 0; lCount < pTbCtlData->lCount; lCount++)
                {
                    WinQueryWindowPos(pTbCtlData->hItems[lCount], &swp);


                    if(pTbCtlData->lState & TB_VERTICAL)
                    {
                        pSWP[itemCount].x  = pSWP[usClient].x;
                        pSWP[itemCount].y  = lOffset +
                                             pSWP[usClient].y - swp.cy;
                	}
                    else
                    {
                    	pSWP[itemCount].x  = pSWP[usClient].x + lOffset;
                    	pSWP[itemCount].y  = pSWP[usClient].y;
                    }

                    pSWP[itemCount].cx = swp.cx;
                    pSWP[itemCount].cy = swp.cy;
                    pSWP[itemCount].fl   = SWP_SIZE | SWP_MOVE | SWP_SHOW;
                    pSWP[itemCount].hwnd = pTbCtlData->hItems[lCount];
                    pSWP[itemCount].hwndInsertBehind = HWND_TOP;

                    if(pTbCtlData->lState & TB_VERTICAL)
                        lOffset -= swp.cy;
                    else
                    	lOffset += swp.cx;

                    itemCount++;
                }

                if(pTbCtlData->lState & TB_VERTICAL)
                {
                    pSWP[usClient].y  += pSWP[usClient].cy - HAND_SIZE;
                    pSWP[usClient].cy = HAND_SIZE;
                }
                else
                    pSWP[usClient].cx = HAND_SIZE;
            }
            return MRFROMSHORT(itemCount);
    }
    return pTbCtlData->pOldProc(hwnd, msg, mp1, mp2);
}

/* Function: TbSeparatorProc
** Abstract: Window procedure for Toolbar Separator Window Class
*/

MRESULT EXPENTRY TbSeparatorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_PAINT:
			{
				HPS hpsPaint;
                RECTL rclPaint;

                hpsPaint = WinBeginPaint(hwnd, 0, 0);

                WinQueryWindowRect(hwnd, &rclPaint);
                WinFillRect(hpsPaint, &rclPaint, CLR_PALEGRAY);

                WinEndPaint(hpsPaint);
			}
            return MRFROMSHORT(FALSE);
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* Function: RecalcTbDimensions
** Abstract: Recalculate Toolbar window dimensions
*/

void RecalcTbDimensions(HWND hwnd, POINTL * pSize)
{
    LONG  lCount;
    POINTL ptlSize;
    POINTL ptlFSize;
    TbCtlData *pTbCtlData;

    pTbCtlData = (TbCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    ptlSize.x = (pTbCtlData->lState & TB_VERTICAL) ? 0 : HAND_SIZE;
    ptlSize.y = (pTbCtlData->lState & TB_VERTICAL) ? HAND_SIZE : 0;

    for(lCount = 0; lCount < pTbCtlData->lCount; lCount++)
    {
        SWP swp;

        WinQueryWindowPos(pTbCtlData->hItems[lCount], &swp);

        if(pTbCtlData->lState & TB_VERTICAL)
        {
            if(swp.cx > ptlSize.x)
                ptlSize.x = swp.cx;
            ptlSize.y += swp.cy;
        }
        else
        {
        	if(swp.cy > ptlSize.y)
            	ptlSize.y = swp.cy;
            ptlSize.x += swp.cx;
        }
    }

    WinSendMsg(hwnd, WM_QUERYBORDERSIZE, MPFROMP(&ptlFSize), 0);
    ptlSize.x += ptlFSize.x * 2;
    ptlSize.y += ptlFSize.y * 2;

    if(pSize)
        *pSize = ptlSize;
    else
    	WinSetWindowPos(hwnd, 0, 0, 0, ptlSize.x, ptlSize.y, SWP_SIZE);

}

/* Function: TrackRectangle
** Abstract: Tracks given rectangle.
**
** If rclBounds is NULL, then track rectangle on entire desktop.
** rclTrack is in window coorditates and will be mapped to
** desktop.
*/

BOOL TrackRectangle(HWND hwndBase, RECTL* rclTrack, RECTL* rclBounds)
{
    TRACKINFO track;
    APIRET rc;
    POINTL ptlSize;

    track.cxBorder = 1;
    track.cyBorder = 1;
    track.cxGrid   = 1;
    track.cyGrid   = 1;
    track.cxKeyboard = 1;
    track.cyKeyboard = 1;

    if(!rclTrack)
        return FALSE;

    if(rclBounds)
    {
        track.rclBoundary = *rclBounds;
    }
    else
    {
        track.rclBoundary.yTop    =
        track.rclBoundary.xRight  = 3000;
        track.rclBoundary.yBottom =
        track.rclBoundary.xLeft   = -3000;
    }

    track.rclTrack = *rclTrack;

    WinMapWindowPoints(hwndBase,
                       HWND_DESKTOP,
                       (PPOINTL)&track.rclTrack,
                       2);

    track.ptlMinTrackSize.x = track.rclTrack.xRight
                            - track.rclTrack.xLeft;
    track.ptlMinTrackSize.y = track.rclTrack.yTop
                            - track.rclTrack.yBottom;
    track.ptlMaxTrackSize.x = track.rclTrack.xRight
                            - track.rclTrack.xLeft;
    track.ptlMaxTrackSize.y = track.rclTrack.yTop
                            - track.rclTrack.yBottom;

    track.fs = TF_MOVE | TF_ALLINBOUNDARY | TF_GRID;

    rc = WinTrackRect(HWND_DESKTOP, 0, &track);

    if(rc)
    {
        if(!memcmp(&rclTrack, &track.rclTrack, sizeof(RECTL)))
            return -1;

        *rclTrack = track.rclTrack;
    }

    return rc;
}

/* Function: TbClientProc
** Abstract: Window procedure for Toolbar Client Window Class
*/

MRESULT EXPENTRY TbClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HWND hwndFrame = 0;
    TbCtlData* pTbCtlData;
    RECTL rclPaint;

    hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
    pTbCtlData = (TbCtlData *)WinQueryWindowULong(hwndFrame,
                                                  QWL_USER);
    switch (msg)
    {
        case WM_ERASEBACKGROUND :
            {
                WinFillRect((HPS)mp1, (PRECTL)mp2, SYSCLR_BUTTONMIDDLE);
            }
            return MRFROMSHORT(FALSE);

        case WM_PAINT:
			{
				HPS hpsPaint;
                POINTL ptlWork;
                int i;

                hpsPaint = WinBeginPaint(hwnd, 0, 0);
                WinQueryWindowRect(hwnd, &rclPaint);

                WinFillRect(hpsPaint, &rclPaint, CLR_PALEGRAY);

                GpiSetColor(hpsPaint,CLR_WHITE);

                ptlWork.x = rclPaint.xLeft   + 2;
                ptlWork.y = rclPaint.yBottom + 2;
                GpiMove(hpsPaint, &ptlWork);
                ptlWork.y = rclPaint.yTop    - 2;
                GpiLine(hpsPaint, &ptlWork);
                ptlWork.x = rclPaint.xRight  - 2;
                GpiLine(hpsPaint, &ptlWork);

                GpiSetColor(hpsPaint,CLR_BLACK);

                ptlWork.y = rclPaint.yBottom + 2;
                GpiLine(hpsPaint, &ptlWork);
                ptlWork.x = rclPaint.xLeft   + 2;
                GpiLine(hpsPaint, &ptlWork);

                WinEndPaint(hpsPaint);
			}
            return MRFROMSHORT(FALSE);

        case WM_MOUSEMOVE:
            {
                WinSetPointer(HWND_DESKTOP,
                              WinQuerySysPointer(HWND_DESKTOP,
                                                 SPTR_MOVE,
                                                 FALSE));
            }
            return MRFROMSHORT(FALSE);

        case WM_BUTTON2DBLCLK: /* Switch bubble help on/off */
            if(pTbCtlData->lState & TB_BUBBLE)
            {
                pTbCtlData->bBubble = 1 - pTbCtlData->bBubble;
            }
            break;

        case WM_BUTTON1DBLCLK: /* Flip horisontal/vertical */
            {
                POINTL ptlPoint;
                SWP swp;

                /* attached toolbar can't be flipped */

                if(pTbCtlData->lState & TB_ATTACHED)
                	return MRFROMSHORT(FALSE);

            	pTbCtlData->lState ^= TB_VERTICAL;
            	WinShowWindow(hwndFrame, FALSE);
                RecalcTbDimensions(hwndFrame, 0);

                /*
                ** Setup new position
                ** New positon should be aligned to mouse cursor
                */

                WinQueryPointerPos(HWND_DESKTOP,&ptlPoint);
                WinQueryWindowPos(hwndFrame, &swp);

                if(pTbCtlData->lState & TB_VERTICAL)
                	WinSetWindowPos(hwndFrame, 0,
                                    ptlPoint.x - swp.cx/2,
                                    ptlPoint.y - swp.cy + HAND_SIZE/2,
                            	    0,
                                	0,
                                    SWP_MOVE);
                else
                    WinSetWindowPos(hwndFrame, 0,
                                    ptlPoint.x - HAND_SIZE/2,
                                    ptlPoint.y - swp.cy/2,
                            	    0,
                                	0,
                                    SWP_MOVE);

            	WinShowWindow(hwndFrame, TRUE);
            }
            return MRFROMSHORT(FALSE);

        case WM_BUTTON1DOWN:
            {
                RECTL rclOwner;
                RECTL rclFrame;
                LONG  lState = 0;
                LONG  lBorderX;
                LONG  lBorderY;
                APIRET rc;
                POINTL ptlSize;

                RecalcTbDimensions(hwndFrame, &ptlSize);

                rclFrame.xLeft   = 0;
                rclFrame.yBottom = 0;
                rclFrame.yTop    = ptlSize.y;
                rclFrame.xRight  = ptlSize.x;

                if((pTbCtlData->lState & TB_ATTACHED) &&
                   (pTbCtlData->lState & TB_VERTICAL))
                {
                    int iShift;

                    WinQueryWindowRect(hwndFrame, &rclOwner);

                    iShift = rclOwner.yTop - rclOwner.yBottom - ptlSize.y;

                    rclFrame.yBottom += iShift;
                    rclFrame.yTop    += iShift;
                }

                rc = TrackRectangle(hwndFrame, &rclFrame, 0);

                if(rc != TRUE)
                    break;

                /*
                ** Check new position for the toolbar
                ** NOTE: order of checks is important
                */

                WinQueryWindowRect(pTbCtlData->hwndParent, &rclOwner);

                /* Map both points to the desktop */

                WinMapWindowPoints(pTbCtlData->hwndParent,
                                   HWND_DESKTOP,
                                   (PPOINTL)&rclOwner,
                                   2);

                /* Cut owner rect by titlebar and menu hight */

                lBorderX = WinQuerySysValue(HWND_DESKTOP, SV_CXDLGFRAME);
                lBorderY = WinQuerySysValue(HWND_DESKTOP, SV_CYDLGFRAME);

                if(WinWindowFromID(pTbCtlData->hwndParent, FID_MENU))
                    rclOwner.yTop -= WinQuerySysValue(HWND_DESKTOP,
                                                      SV_CYMENU);

                if(WinWindowFromID(pTbCtlData->hwndParent, FID_TITLEBAR))
                    rclOwner.yTop -= WinQuerySysValue(HWND_DESKTOP,
                                                      SV_CYTITLEBAR);

                lState = 0;
                if(rclFrame.xLeft >= rclOwner.xLeft - lBorderX * 2 &&
                   rclFrame.xLeft <= rclOwner.xLeft + lBorderX * 2)
                    lState = TB_ATTACHED_LT;

                if(rclFrame.yTop  >= rclOwner.yTop - lBorderY * 2  &&
                   rclFrame.yTop  <= rclOwner.yTop + lBorderY * 2  )
                    lState = TB_ATTACHED_TP;

                if(rclFrame.xRight >= rclOwner.xRight - lBorderX * 2 &&
                   rclFrame.xRight <= rclOwner.xRight + lBorderX * 2)
                    lState = TB_ATTACHED_RT;

                if(rclFrame.yBottom >= rclOwner.yBottom - lBorderY * 2 &&
                   rclFrame.yBottom <= rclOwner.yBottom + lBorderY * 2)
                    lState = TB_ATTACHED_BT;

                WinShowWindow(hwndFrame, FALSE);

                if(!(pTbCtlData->lState & TB_ATTACHED) && !lState)
                {
                    /* Toolbar is not attached and will not be attached
                       this time. Just change its position.
                     */

                    WinSetWindowPos(hwndFrame, 0,
                                    rclFrame.xLeft, rclFrame.yBottom,
                                    0, 0, SWP_MOVE);
                }

                if(pTbCtlData->lState & TB_ATTACHED)
                {
                    POINTL ptlPoint;
                    SWP swp;

                    WinSetWindowBits(hwndFrame, QWL_STYLE, 0, FS_BORDER);
                    WinSetWindowBits(hwndFrame, QWL_STYLE, FS_DLGBORDER,
                                     FS_DLGBORDER);
                    WinSendMsg(hwndFrame,
                               WM_UPDATEFRAME,
                               MPFROMLONG(FCF_SIZEBORDER), 0);

                    pTbCtlData->lState &= ~TB_ATTACHED;
                    WinSetParent(hwndFrame, HWND_DESKTOP, FALSE);
                    RecalcTbDimensions(hwndFrame, 0);

                    WinQueryPointerPos(HWND_DESKTOP,&ptlPoint);
                    WinQueryWindowPos(hwndFrame, &swp);

                    if(pTbCtlData->lState & TB_VERTICAL)
                        WinSetWindowPos(hwndFrame, 0,
                                        ptlPoint.x - swp.cx/2,
                                        ptlPoint.y - swp.cy + HAND_SIZE/2,
                                        0,
                                        0,
                                        SWP_MOVE);
                    else
                        WinSetWindowPos(hwndFrame, 0,
                                        ptlPoint.x - HAND_SIZE/2,
                                        ptlPoint.y - swp.cy/2,
                                        0,
                                        0,
                                        SWP_MOVE);
                }

                if(lState)
                {
                    pTbCtlData->lState |= lState;

                    WinSetWindowBits(hwndFrame, QWL_STYLE, 0, FS_DLGBORDER);

                    WinSetWindowBits(hwndFrame, QWL_STYLE, FS_BORDER,
                                     FS_BORDER);

                    WinSendMsg(hwndFrame,
                               WM_UPDATEFRAME,
                               MPFROMLONG(FCF_SIZEBORDER), 0);

                    WinSetFocus(HWND_DESKTOP, pTbCtlData->hwndParent);
                    WinSetParent(hwndFrame, pTbCtlData->hwndParent, FALSE);

                    if((lState & (TB_ATTACHED_LT | TB_ATTACHED_RT)) &&
                       !(pTbCtlData->lState & TB_VERTICAL))
                    {
                        /*
                        ** toolbar is horisontal, but we need to
                        ** attach them to vertical side
                        */
                        pTbCtlData->lState ^= TB_VERTICAL;
                    }

                    if((lState & (TB_ATTACHED_TP | TB_ATTACHED_BT)) &&
                       (pTbCtlData->lState & TB_VERTICAL))
                    {
                        /*
                        **toolbar is vertical, but we need to
                        **attach them to horizontal side
                        */
                        pTbCtlData->lState ^= TB_VERTICAL;
                    }
                    RecalcTbDimensions(hwndFrame, 0);
                }

                WinSendMsg(pTbCtlData->hwndParent, WM_UPDATEFRAME, 0, 0);
                WinShowWindow(hwndFrame, TRUE);
                WinSetWindowPos(hwndFrame, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER);
            }
            return MRFROMSHORT(FALSE);
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* Function: ToolkitInit
** Abstract: Registers classes needed for toolkit
*/

void ToolkitInit(HAB aHab)
{
    hab = aHab;
    WinRegisterClass(hab,
                     CELL_CLIENT,
                     CellClientProc,
                     CS_SIZEREDRAW,
                     sizeof(ULONG));

    WinRegisterClass(hab,
                     TB_CLIENT,
                     TbClientProc,
                     CS_SIZEREDRAW,
                     sizeof(ULONG));

    WinRegisterClass(hab,
                     TB_SEPCLASS,
                     TbSeparatorProc,
                     CS_SIZEREDRAW,
                     sizeof(ULONG));
}

/* Function: CreateToolbar
** Abstract: Creates toolbar for cell frame window
*/

void CreateToolbar(HWND hwnd, TbDef* pTb)
{
    CellCtlData* pCtlData = 0;
    CellTb*      pCellTb  = 0;
    HWND hwndTb;

    if(!pTb || !hwnd)
        return;

    pCtlData = (CellCtlData *)WinQueryWindowULong(hwnd, QWL_USER);

    if(!pCtlData)
        return;

    hwndTb = CreateTb(pTb, hwnd, hwnd);

    if(!hwndTb)
        return;

    pCellTb = (CellTb*)malloc(sizeof(CellTb));

    if(!pCellTb)
        return;

    memset(pCellTb, 0, sizeof(CellTb));

    pCellTb->hWnd     = hwndTb;
    pCellTb->pNext    = pCtlData->pCellTb;
    pCtlData->pCellTb = pCellTb;

    WinSendMsg(hwnd, WM_UPDATEFRAME, 0, 0);
}

/* Function: CellWindowFromID
** Abstract: Locate control window with given ID
*/

HWND CellWindowFromID(HWND hwndCell, ULONG ulID)
{
    return (HWND)WinSendMsg(hwndCell, TKM_SEARCH_ID, MPFROMLONG(ulID), 0);
}

/* Function: CellWindowFromID
** Abstract: Locate parent window for window with given ID
*/

HWND CellParentWindowFromID(HWND hwndCell, ULONG ulID)
{
    return (HWND)WinSendMsg(hwndCell, TKM_SEARCH_PARENT, MPFROMLONG(ulID), 0);
}

/* Function: GenResIDStr
** Abstract: Generate string '#nnnn' for a given ID for using with Button
**           controls
*/

void GenResIDStr(CHAR *buff, ULONG ulID)
{
    char *str;
    int  slen = 0;

    *buff++ = '#';

    str = buff;

    do
    {
        *str++ = (ulID % 10) + '0';
        ulID /= 10;
        slen++;
    }
    while(ulID);

    *str-- = 0;

    for(; str > buff; str--, buff++)
    {
        *buff ^= *str;
        *str  ^= *buff;
        *buff ^= *str;
    }
}


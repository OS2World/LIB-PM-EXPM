/*
 * EXPM - Extended Presentation Manager library
 *
 * src/shapewin/shapewin.c -- Shaped ("transparent") windows.
 *
 * A clean-room reimplementation of the classic OS/2 "shape window"
 * technique (region-based non-rectangular windows).  The mask bitmap is
 * parsed into rectangular spans and one child window is created per
 * span; the child windows are clipped to the opaque pixels so the
 * background shows through the transparent (key-coloured) parts.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define  INCL_WINDIALOGS
#define  INCL_WINSYS
#define  INCL_WINWINDOWMGR
#define  INCL_WINMESSAGEMGR
#define  INCL_WININPUT
#define  INCL_GPI
#define  INCL_DOS
#include <os2.h>
#include <stdlib.h>
#include <string.h>

#include "expm.h"
#include "shapewin.h"

/* forward declaration */
static MRESULT EXPENTRY regnProc( HWND, ULONG, MPARAM, MPARAM );

/* ------------------------------------------------------------------ */
/* Internal types                                                     */
/* ------------------------------------------------------------------ */

typedef struct _SHPRGNREC
{
    struct _SHPCTLREC *pCtrl;   /* link to parent control               */
    HWND              hwnd;     /* region child window                   */
    RECTL             rect;     /* rectangle in parent coords            */
} SHPRGNREC, *SHPRGNPTR;

typedef struct _SHPCTLREC
{
    HWND        hwndSelf;
    HWND        hwndParent;
    HWND        hwndOwner;
    USHORT      id;
    SHORT       x;
    SHORT       y;
    SHORT       cx;
    SHORT       cy;

    HPS         hpsDraw;        /* image to display                     */
    ULONG       nRegion;
    SHPRGNPTR   aRegion;
} SHPCTLREC, *SHPCTLPTR;

/* ------------------------------------------------------------------ */
/* Span (run-length) helpers                                          */
/* ------------------------------------------------------------------ */

#define SHP_STEP    1024

typedef struct _SPANREC
{
    ULONG   nSize;
    ULONG   nUsed;
    RECTL   aSpan[1];
} SPANREC, *SPANPTR;

static void freeSpan( SPANPTR p )
{
    if( p ) free( p );
}

static SPANPTR makeSpan( void )
{
    SPANPTR p;
    int     len;

    len = (int)( sizeof(SPANREC) + sizeof(RECTL) * SHP_STEP );
    p = (SPANPTR)malloc( len );
    if( p == NULL ) return NULL;
    memset( p, 0, len );
    p->nSize = SHP_STEP;
    p->nUsed = 0;
    return p;
}

static SPANPTR moreSpan( SPANPTR pOld )
{
    SPANPTR pNew;
    int     len;

    if( pOld == NULL ) return NULL;
    len = (int)( sizeof(SPANREC) + sizeof(RECTL) * (pOld->nSize + SHP_STEP) );
    pNew = (SPANPTR)malloc( len );
    if( pNew == NULL ) return NULL;
    memset( pNew, 0, len );
    pNew->nSize = pOld->nSize + SHP_STEP;
    pNew->nUsed = pOld->nUsed;
    memcpy( pNew->aSpan, pOld->aSpan, sizeof(RECTL) * pOld->nUsed );
    freeSpan( pOld );
    return pNew;
}

static SPANPTR putSpan( SPANPTR pSpan, int y, int x1, int x2 )
{
    int     i;
    PRECTL  p;
    SPANPTR pNew;

    /* Try to merge with an existing continuous span. */
    for( i = 0; i < (int)pSpan->nUsed; i++ )
    {
        p = &pSpan->aSpan[i];
        if( p->yTop == y && p->xLeft == x1 && p->xRight == x2 )
        {
            p->yTop += 1;
            return pSpan;
        }
    }

    if( (pSpan->nUsed + 1) >= pSpan->nSize )
    {
        pNew = moreSpan( pSpan );
        if( pNew == NULL ) return NULL;
        pSpan = pNew;
    }

    pSpan->aSpan[pSpan->nUsed].yTop    = y + 1;
    pSpan->aSpan[pSpan->nUsed].yBottom = y;
    pSpan->aSpan[pSpan->nUsed].xLeft   = x1;
    pSpan->aSpan[pSpan->nUsed].xRight  = x2;
    pSpan->nUsed += 1;
    return pSpan;
}

/* ------------------------------------------------------------------ */
/* Mask parsing                                                       */
/* ------------------------------------------------------------------ */

static SPANPTR parseMask( HPS hps, PBITMAPINFOHEADER2 bmih )
{
    int     blen, hlen;
    PUCHAR  buf;
    PBITMAPINFO2 pbm;
    SPANPTR pSpan, pNew;
    int     x, y, k;
    LONG    first, color;
    BOOL    inspan;
    int     left;

    hlen = (int)( sizeof(BITMAPINFO2) + sizeof(RGB) * 256 );
    blen = (int)( ((bmih->cBitCount * bmih->cx + 31) / 32) * bmih->cPlanes * 4 );

    pbm = (PBITMAPINFO2)malloc( hlen );
    buf = (PUCHAR)malloc( blen );
    if( pbm == NULL || buf == NULL )
    {
        if( pbm ) free( pbm );
        if( buf ) free( buf );
        return NULL;
    }
    memcpy( pbm, bmih, sizeof(BITMAPINFOHEADER2) );

    pSpan = makeSpan();
    if( pSpan == NULL )
    {
        free( pbm );
        free( buf );
        return NULL;
    }

    first = -1;

    for( y = 0; y < (int)bmih->cy; y++ )
    {
        if( GpiQueryBitmapBits( hps, y, 1, buf, pbm ) == GPI_ERROR )
            break;

        inspan = FALSE;
        left = 0;
        for( x = 0; x < (int)bmih->cx; x++ )
        {
            k = x * 3;
            color = ( (LONG)buf[k] << 16 ) | ( (LONG)buf[k+1] << 8 ) | (LONG)buf[k+2];
            if( first < 0 ) first = color;

            if( inspan == FALSE && color != first )
            {
                inspan = TRUE;
                left = x;
            }
            else if( inspan == TRUE && color == first )
            {
                inspan = FALSE;
                pNew = putSpan( pSpan, y, left, x );
                if( pNew != NULL ) pSpan = pNew;
            }
        }
        if( inspan == TRUE )
        {
            pNew = putSpan( pSpan, y, left, (int)bmih->cx );
            if( pNew != NULL ) pSpan = pNew;
        }
    }

    free( pbm );
    free( buf );
    return pSpan;
}

/* ------------------------------------------------------------------ */
/* Region management                                                  */
/* ------------------------------------------------------------------ */

static int ShpMakeRegion( SHPCTLPTR pCtrl, HPS hpsMask )
{
    HAB     hab;
    HDC     hdc;
    HPS     hps;
    SIZEL   siz;
    HBITMAP hbm;
    BITMAPINFOHEADER2 bmi;
    POINTL  apt[3];
    SPANPTR pSpan;
    SHPRGNPTR pRegn;
    int     i;

    if( pCtrl == NULL ) return -1;

    hab = WinQueryAnchorBlock( pCtrl->hwndSelf );

    hdc = DevOpenDC( hab, OD_MEMORY, "*", 0L, NULL, NULLHANDLE );
    siz.cx = 0; siz.cy = 0;
    hps = GpiCreatePS( hab, hdc, &siz,
            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );

    memset( &bmi, 0, sizeof(bmi) );
    bmi.cbFix          = sizeof(BITMAPINFOHEADER2);
    bmi.cx             = (USHORT)pCtrl->cx;
    bmi.cy             = (USHORT)pCtrl->cy;
    bmi.cPlanes        = 1;
    bmi.cBitCount      = 24;
    bmi.ulCompression  = 0;

    hbm = GpiCreateBitmap( hps, &bmi, 0L, NULL, NULL );
    GpiSetBitmap( hps, hbm );

    apt[0].x = 0; apt[0].y = 0;
    apt[1].x = pCtrl->cx; apt[1].y = pCtrl->cy;
    apt[2].x = 0; apt[2].y = 0;

    if( GpiBitBlt( hps, hpsMask, 3, apt, ROP_SRCCOPY, 0 ) == GPI_ERROR )
    {
        /* ignore; parsing may still work on whatever is there */
    }

    pSpan = parseMask( hps, &bmi );

    if( pSpan != NULL && pSpan->nUsed > 0 )
    {
        pRegn = (SHPRGNPTR)malloc( sizeof(SHPRGNREC) * pSpan->nUsed );
        if( pRegn != NULL )
        {
            pCtrl->nRegion = pSpan->nUsed;
            pCtrl->aRegion = pRegn;
            for( i = 0; i < (int)pSpan->nUsed; i++ )
            {
                pRegn[i].pCtrl = pCtrl;
                pRegn[i].hwnd  = NULLHANDLE;
                pRegn[i].rect.xLeft   = pSpan->aSpan[i].xLeft;
                pRegn[i].rect.xRight  = pSpan->aSpan[i].xRight;
                pRegn[i].rect.yTop    = pSpan->aSpan[i].yTop;
                pRegn[i].rect.yBottom = pSpan->aSpan[i].yBottom;
            }
        }
        freeSpan( pSpan );
    }

    GpiDeleteBitmap( hbm );
    GpiDestroyPS( hps );
    DevCloseDC( hdc );
    return 0;
}

static int ShpFreeRegion( SHPCTLPTR pCtrl )
{
    int i;
    if( pCtrl == NULL || pCtrl->aRegion == NULL ) return 0;
    for( i = 0; i < (int)pCtrl->nRegion; i++ )
    {
        if( pCtrl->aRegion[i].hwnd != NULLHANDLE )
            WinDestroyWindow( pCtrl->aRegion[i].hwnd );
    }
    free( pCtrl->aRegion );
    pCtrl->aRegion = NULL;
    pCtrl->nRegion = 0;
    return 0;
}

static int ShpOpenRegion( SHPCTLPTR pCtrl )
{
    int         i;
    SHPRGNPTR   pRgn;
    ULONG       flStyle;
    static BOOL registered = FALSE;

    if( pCtrl == NULL || pCtrl->nRegion == 0 || pCtrl->aRegion == NULL )
        return -1;

    if( registered == FALSE )
    {
        WinRegisterClass( WinQueryAnchorBlock( pCtrl->hwndSelf ),
                          "XpmShapeRegion", regnProc,
                          CS_PARENTCLIP | CS_CLIPSIBLINGS | CS_SYNCPAINT,
                          sizeof(PVOID) );
        registered = TRUE;
    }

    flStyle = 0;

    for( i = 0, pRgn = pCtrl->aRegion; i < (int)pCtrl->nRegion; i++, pRgn++ )
    {
        pRgn->hwnd = WinCreateWindow(
                pCtrl->hwndParent,
                "XpmShapeRegion",
                NULL,
                flStyle,
                (SHORT)(pCtrl->x + pRgn->rect.xLeft),
                (SHORT)(pCtrl->y + pRgn->rect.yBottom),
                (SHORT)(pRgn->rect.xRight - pRgn->rect.xLeft),
                (SHORT)(pRgn->rect.yTop - pRgn->rect.yBottom),
                pCtrl->hwndOwner,
                HWND_TOP,
                (USHORT)i,
                pRgn,
                NULL );
    }
    return 0;
}

static int ShpAdjustRegion( SHPCTLPTR pCtrl, PSWP pSwp )
{
    int         i;
    SHPRGNPTR   pRgn;
    ULONG       fl;
    PSWP        pswpArray;

    if( pCtrl == NULL || pCtrl->nRegion == 0 || pCtrl->aRegion == NULL )
        return -1;

    fl = pSwp->fl;

    if( fl & SWP_MOVE )
    {
        pCtrl->x = (SHORT)pSwp->x;
        pCtrl->y = (SHORT)pSwp->y;
    }

    pswpArray = (PSWP)malloc( sizeof(SWP) * pCtrl->nRegion );
    if( pswpArray == NULL ) return -1;

    for( i = 0, pRgn = pCtrl->aRegion; i < (int)pCtrl->nRegion; i++, pRgn++ )
    {
        pswpArray[i].fl = fl;
        pswpArray[i].cy = (SHORT)(pRgn->rect.yTop - pRgn->rect.yBottom);
        pswpArray[i].cx = (SHORT)(pRgn->rect.xRight - pRgn->rect.xLeft);
        pswpArray[i].y  = (SHORT)(pCtrl->y + pRgn->rect.yBottom);
        pswpArray[i].x  = (SHORT)(pCtrl->x + pRgn->rect.xLeft);
        pswpArray[i].hwndInsertBehind = pSwp->hwndInsertBehind;
        pswpArray[i].hwnd = pRgn->hwnd;
    }
    WinSetMultWindowPos( WinQueryAnchorBlock( pCtrl->hwndSelf ),
                         &pswpArray[0], pCtrl->nRegion );
    free( pswpArray );

    pSwp->cx = (SHORT)pCtrl->cx;
    pSwp->cy = (SHORT)pCtrl->cy;
    pSwp->fl &= ~SWP_SHOW;
    return 0;
}

static SHPCTLPTR ShpInit( HWND hwnd, PCREATESTRUCT pWin, PXpmShapeData pData )
{
    SHPCTLPTR pCtrl;

    if( pData == NULL || pData->hpsDraw == NULLHANDLE ||
        pData->hpsMask == NULLHANDLE )
        return NULL;

    pCtrl = (SHPCTLPTR)malloc( sizeof(SHPCTLREC) );
    if( pCtrl == NULL ) return NULL;
    memset( pCtrl, 0, sizeof(SHPCTLREC) );

    pCtrl->hwndSelf   = hwnd;
    pCtrl->hwndParent = pWin->hwndParent;
    pCtrl->hwndOwner  = (pData->hwndOwner != NULLHANDLE) ? pData->hwndOwner : pWin->hwndOwner;
    pCtrl->id         = pWin->id;
    pCtrl->x          = (SHORT)pWin->x;
    pCtrl->y          = (SHORT)pWin->y;
    pCtrl->cx         = (SHORT)pData->cx;
    pCtrl->cy         = (SHORT)pData->cy;
    pCtrl->hpsDraw    = pData->hpsDraw;

    ShpMakeRegion( pCtrl, pData->hpsMask );
    ShpOpenRegion( pCtrl );

    return pCtrl;
}

static void ShpTerm( SHPCTLPTR pCtrl )
{
    if( pCtrl == NULL ) return;
    ShpFreeRegion( pCtrl );
    free( pCtrl );
}

/* ------------------------------------------------------------------ */
/* Region child window procedure                                      */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY regnProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

static MRESULT EXPENTRY regnProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    SHPRGNPTR pRgn;
    HPS       hps;

    pRgn = (SHPRGNPTR)WinQueryWindowPtr( hwnd, 0 );

    switch( msg )
    {
        case WM_CREATE:
            pRgn = (SHPRGNPTR)PVOIDFROMMP( mp1 );
            WinSetWindowPtr( hwnd, 0, (PVOID)pRgn );
            return (MRESULT)FALSE;

        case WM_PAINT:
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );
            if( hps != NULLHANDLE && pRgn != NULL && pRgn->pCtrl != NULL )
            {
                POINTL apt[3];
                apt[0].x = 0;
                apt[0].y = 0;
                apt[1].x = pRgn->rect.xRight - pRgn->rect.xLeft;
                apt[1].y = pRgn->rect.yTop - pRgn->rect.yBottom;
                apt[2].x = pRgn->rect.xLeft;
                apt[2].y = pRgn->rect.yBottom;
                GpiBitBlt( hps, pRgn->pCtrl->hpsDraw, 3, apt, ROP_SRCCOPY, 0 );
            }
            if( hps != NULLHANDLE ) WinEndPaint( hps );
            return (MRESULT)0;

        case WM_MOUSEMOVE:
        case WM_BUTTON1DOWN:
        case WM_BUTTON1UP:
        case WM_BUTTON1CLICK:
        case WM_BUTTON1DBLCLK:
        case WM_BUTTON2DOWN:
        case WM_BUTTON2UP:
        case WM_BUTTON2CLICK:
        case WM_BUTTON2DBLCLK:
        case WM_BUTTON3DOWN:
        case WM_BUTTON3UP:
        case WM_BUTTON3CLICK:
        case WM_BUTTON3DBLCLK:
        case WM_CHAR:
        case WM_VIOCHAR:
        case WM_BEGINDRAG:
        case WM_ENDDRAG:
        case WM_SINGLESELECT:
        case WM_OPEN:
        case WM_CONTEXTMENU:
        case WM_CONTEXTHELP:
        case WM_TEXTEDIT:
        case WM_BEGINSELECT:
        case WM_ENDSELECT:
            if( pRgn != NULL && pRgn->pCtrl != NULL && pRgn->pCtrl->hwndOwner != NULLHANDLE )
                return WinSendMsg( pRgn->pCtrl->hwndOwner, msg, mp1, mp2 );
            break;
    }
    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */
/* Shape window procedure                                             */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY XpmShapeWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    SHPCTLPTR pCtrl;
    PSWP      pswp;

    pCtrl = (SHPCTLPTR)WinQueryWindowPtr( hwnd, 0 );

    switch( msg )
    {
        case WM_CREATE:
            pCtrl = ShpInit( hwnd, (PCREATESTRUCT)PVOIDFROMMP(mp2),
                             (PXpmShapeData)PVOIDFROMMP(mp1) );
            if( pCtrl == NULL )
                return (MRESULT)TRUE;
            WinSetWindowPtr( hwnd, 0, (PVOID)pCtrl );
            return (MRESULT)FALSE;

        case WM_DESTROY:
            ShpTerm( pCtrl );
            WinSetWindowPtr( hwnd, 0, NULL );
            return (MRESULT)0;

        case WM_ADJUSTWINDOWPOS:
            pswp = (PSWP)PVOIDFROMMP( mp1 );
            ShpAdjustRegion( pCtrl, pswp );
            return (MRESULT)0;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );
            if( hps != NULLHANDLE ) WinEndPaint( hps );
            return (MRESULT)0;
        }

        case WM_MOUSEMOVE:
        case WM_BUTTON1DOWN:
        case WM_BUTTON1UP:
        case WM_BUTTON1CLICK:
        case WM_BUTTON1DBLCLK:
        case WM_BUTTON2DOWN:
        case WM_BUTTON2UP:
        case WM_BUTTON2CLICK:
        case WM_BUTTON2DBLCLK:
        case WM_BUTTON3DOWN:
        case WM_BUTTON3UP:
        case WM_BUTTON3CLICK:
        case WM_BUTTON3DBLCLK:
        case WM_CHAR:
        case WM_VIOCHAR:
            if( pCtrl != NULL && pCtrl->hwndOwner != NULLHANDLE )
                return WinSendMsg( pCtrl->hwndOwner, msg, mp1, mp2 );
            break;
    }
    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

static BOOL g_registered = FALSE;

BOOL EXPENTRY XpmShapeInit( VOID )
{
    if( g_registered ) return TRUE;
    g_registered = TRUE;
    return TRUE;
}

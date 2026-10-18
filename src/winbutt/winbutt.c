/*
 * EXPM - Extended Presentation Manager library
 *
 * src/winbutt/winbutt.c -- XpmWinButtonClass window button control.
 *
 * Clean-room reimplementation for the EXPM project (2026) of the
 * AsuaCtl 1.0 window button (WC_WINBUTTON).  Written in C from the
 * published AsuaCtl header (include/winbutt.h) and the published help
 * text only; no AsuaCtl source was consulted, so the behaviour is
 * EXPM's own interpretation of that published API.  AsuaCtl is GPLv2,
 * EXPM is LGPLv3 (see doc\winbutt.txt for the name mapping and the
 * documented deviations).
 *
 * XpmWinButtonClass is a drop-in replacement for the system button
 * (WC_BUTTON) with extra presentation features: flat buttons (frame
 * and full colour only while the pointer is over them), bitmap faces,
 * thin frames, automatic sizing and mouse enter/leave notifications.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#define INCL_WIN
#define INCL_GPI
#define INCL_GPILOGCOLORTABLE
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "expm.h"

/* Metrics */
#define WBTN_MAXTEXT    256     /* maximum stored text length         */
#define WBTN_MARGIN     9       /* horizontal text margin, each side  */
#define WBTN_VMARGIN    5       /* vertical   text margin, each side  */

static BOOL g_bBtnRegistered = FALSE;

/* Per-window control data. */
typedef struct _XPMWBTNDATA
{
    BOOL    bReady;     /* TRUE once creation is complete    */
    ULONG   flStyle;    /* cached window style               */
    BOOL    fPressed;   /* currently drawn pressed            */
    BOOL    fHilited;   /* mouse pointer over the control    */
    BOOL    fDown;      /* button 1 held down                 */
    BOOL    fFocus;     /* has the keyboard focus             */
    HBITMAP hbm;        /* normal    colour bitmap, or NULL   */
    HBITMAP hbmDis;     /* disabled  colour bitmap, or NULL   */
    HBITMAP hbmPress;   /* pressed   colour bitmap, or NULL   */
    HBITMAP hbmHilite;  /* mouse-over colour bitmap, or NULL  */
    HBITMAP hbmMask;    /* normal    monochrome mask, or NULL */
    HBITMAP hbmDisMask; /* disabled  monochrome mask, or NULL */
    HBITMAP hbmPrsMask; /* pressed   monochrome mask, or NULL */
    HBITMAP hbmHilMask; /* mouse-over monochrome mask, or NULL*/
    CHAR    szText[WBTN_MAXTEXT];
} XPMWBTNDATA;

/* ------------------------------------------------------------------ */

static XPMWBTNDATA *BtnData( HWND hwnd )
{
    return (XPMWBTNDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static ULONG BtnStyle( HWND hwnd )
{
    XPMWBTNDATA *p = BtnData( hwnd );

    return p ? p->flStyle : WinQueryWindowULong( hwnd, QWL_STYLE );
}

/* Read one colour from a presentation parameter with a SYSCLR_* / RGB
 * fallback. */
static LONG QueryColor( HWND hwnd, ULONG id, ULONG ulSysColor, LONG lDefault )
{
    XpmPPTemplate ppt;
    LONG          lColor;

    ppt.ulPresParam = id;
    ppt.ulPresFlags = 0;
    ppt.ulSysColor  = ulSysColor;
    ppt.ulRgbColor  = (ULONG)lDefault;

    lColor = lDefault;
    XpmQueryPresColors( hwnd, 1, &ppt, &lColor );
    return lColor;
}

/* Background/foreground pair for the current enable state. */
static void QueryColors( HWND hwnd, BOOL bActive, LONG *plBg, LONG *plFg )
{
    if( bActive )
    {
        *plBg = QueryColor( hwnd, PP_BACKGROUNDCOLOR,
                            (ULONG)SYSCLR_BUTTONMIDDLE, 0x00C0C0C0 );
        *plFg = QueryColor( hwnd, PP_FOREGROUNDCOLOR,
                            (ULONG)SYSCLR_WINDOWTEXT, 0x00000000 );
    }
    else
    {
        *plBg = QueryColor( hwnd, PP_DISABLEDBACKGROUNDCOLOR,
                            (ULONG)SYSCLR_BUTTONMIDDLE, 0x00C0C0C0 );
        *plFg = QueryColor( hwnd, PP_DISABLEDFOREGROUNDCOLOR,
                            (ULONG)SYSCLR_BUTTONDARK, 0x00808080 );
    }
}

/* Total frame thickness (in pixels) for the given style. */
static LONG FrameWidth( ULONG flStyle )
{
    LONG t = ( flStyle & XpmBS_THINFRAME ) ? 1 : 2;

    if( flStyle & XpmBS_DEFAULT )
        t += 1;

    return t;
}

/* ------------------------------------------------------------------ */
/* Measurement                                                        */
/* ------------------------------------------------------------------ */

static BOOL MeasureText( HWND hwnd, const CHAR *psz, LONG cch,
                         LONG *plWidth, LONG *plHeight )
{
    HPS    hps;
    POINTL aptl[TXTBOX_COUNT];

    *plWidth  = 0;
    *plHeight = 0;

    if( cch <= 0 )
        return FALSE;

    hps = WinGetPS( hwnd );
    if( !hps )
        return FALSE;

    if( GpiQueryTextBox( hps, cch, (PCH)psz, TXTBOX_COUNT, aptl ) )
    {
        *plWidth  = aptl[TXTBOX_CONCAT].x;
        *plHeight = aptl[TXTBOX_TOPLEFT].y - aptl[TXTBOX_BOTTOMLEFT].y;
    }

    WinReleasePS( hps );
    return TRUE;
}

static void BitmapSize( HBITMAP hbm, LONG *plWidth, LONG *plHeight )
{
    BITMAPINFOHEADER bmp;

    *plWidth  = 0;
    *plHeight = 0;

    if( !hbm )
        return;

    bmp.cbFix = (ULONG)sizeof( BITMAPINFOHEADER );
    if( GpiQueryBitmapParameters( hbm, &bmp ) )
    {
        *plWidth  = (LONG)bmp.cx;
        *plHeight = (LONG)bmp.cy;
    }
}

/* Minimum width/height needed to show the current face. */
static LONG MinWidth( HWND hwnd, XPMWBTNDATA *p )
{
    LONG tw, th, bw, bh, w;

    MeasureText( hwnd, p->szText, (LONG)strlen( p->szText ), &tw, &th );
    BitmapSize( p->hbm ? p->hbm : p->hbmPress, &bw, &bh );

    w = ( bw > tw ) ? bw : tw;
    if( w <= 0 )
        w = 0;

    return w + 2 * WBTN_MARGIN + 2 * FrameWidth( p->flStyle );
}

static LONG MinHeight( HWND hwnd, XPMWBTNDATA *p )
{
    LONG tw, th, bw, bh, h;

    MeasureText( hwnd, p->szText, (LONG)strlen( p->szText ), &tw, &th );
    BitmapSize( p->hbm ? p->hbm : p->hbmPress, &bw, &bh );

    h = ( bh > th ) ? bh : th;
    if( h <= 0 )
        h = 0;

    return h + 2 * WBTN_VMARGIN + 2 * FrameWidth( p->flStyle );
}

/* Resize the window to its minimum size when XpmBS_AUTOSIZE is set. */
static void AutoSize( HWND hwnd )
{
    XPMWBTNDATA *p = BtnData( hwnd );
    LONG cx, cy;

    if( !p || !( p->flStyle & XpmBS_AUTOSIZE ) )
        return;

    cx = MinWidth( hwnd, p );
    cy = MinHeight( hwnd, p );

    WinSetWindowPos( hwnd, NULLHANDLE, 0, 0, cx, cy, SWP_SIZE );
}

/* ------------------------------------------------------------------ */
/* Drawing                                                            */
/* ------------------------------------------------------------------ */

static void DrawLine( HPS hps, LONG x1, LONG y1, LONG x2, LONG y2, LONG clr )
{
    POINTL ptl;

    GpiSetColor( hps, clr );
    ptl.x = x1;
    ptl.y = y1;
    GpiMove( hps, &ptl );
    ptl.x = x2;
    ptl.y = y2;
    GpiLine( hps, &ptl );
}

/* Draw the 3-D frame.  Raised when not pressed, sunken when pressed; a
 * one-pixel outer ring in the default colour is added for default
 * buttons. */
static void DrawFrame( HPS hps, PRECTL prcl, ULONG flStyle, BOOL fPressed,
                       LONG clrLight, LONG clrDark, LONG clrDefault )
{
    LONG xl = prcl->xLeft;
    LONG yb = prcl->yBottom;
    LONG xr = prcl->xRight  - 1;
    LONG yt = prcl->yTop    - 1;
    LONG t  = ( flStyle & XpmBS_THINFRAME ) ? 1 : 2;
    LONG c1, c2, i;

    if( flStyle & XpmBS_DEFAULT )
    {
        DrawLine( hps, xl, yt, xr, yt, clrDefault );
        DrawLine( hps, xl, yb, xr, yb, clrDefault );
        DrawLine( hps, xl, yb, xl, yt, clrDefault );
        DrawLine( hps, xr, yb, xr, yt, clrDefault );
        xl++; yb++; xr--; yt--;
    }

    if( fPressed )
    {
        c1 = clrDark;
        c2 = clrLight;
    }
    else
    {
        c1 = clrLight;
        c2 = clrDark;
    }

    for( i = 0; i < t; i++ )
    {
        DrawLine( hps, xl + i, yt - i, xr - i, yt - i, c1 );  /* top    */
        DrawLine( hps, xl + i, yt - i, xl + i, yb + i, c1 );  /* left   */
        DrawLine( hps, xl + i, yb + i, xr - i, yb + i, c2 );  /* bottom */
        DrawLine( hps, xr - i, yt - i, xr - i, yb + i, c2 );  /* right  */
    }
}

/* Draw the keyboard-focus indicator: a one-pixel rectangle inset from
 * the client edge. */
static void DrawFocus( HPS hps, PRECTL prcl, LONG clr )
{
    LONG xl = prcl->xLeft + 3;
    LONG yb = prcl->yBottom + 3;
    LONG xr = prcl->xRight - 4;
    LONG yt = prcl->yTop - 4;

    if( xr <= xl || yt <= yb )
        return;

    DrawLine( hps, xl, yb, xr, yb, clr );
    DrawLine( hps, xl, yt, xr, yt, clr );
    DrawLine( hps, xl, yb, xl, yt, clr );
    DrawLine( hps, xr, yb, xr, yt, clr );
}

/* Draw the text, honouring the alignment field and the XpmBS_AUTOTEXT
 * ellipsis option. */
static void DrawTextStr( HPS hps, PRECTL prcl, XPMWBTNDATA *p, LONG bg,
                         LONG fg )
{
    ULONG  flStyle = p->flStyle;
    RECTL  box;
    POINTL aptl[TXTBOX_COUNT];
    POINTL pos;
    CHAR   szDraw[WBTN_MAXTEXT + 4];
    LONG   cch;
    LONG   tw, th;
    LONG   bw, bh;
    ULONG  align;

    cch = (LONG)strlen( p->szText );
    if( cch <= 0 )
        return;

    box = *prcl;
    box.xLeft   += FrameWidth( flStyle ) + 2;
    box.xRight  -= FrameWidth( flStyle ) + 2;
    box.yBottom += FrameWidth( flStyle ) + 1;
    box.yTop    -= FrameWidth( flStyle ) + 1;

    bw = box.xRight - box.xLeft;
    bh = box.yTop   - box.yBottom;
    if( bw <= 0 || bh <= 0 )
        return;

    if( cch > WBTN_MAXTEXT - 1 )
        cch = WBTN_MAXTEXT - 1;
    memcpy( szDraw, p->szText, (size_t)cch );
    szDraw[cch] = '\0';

    tw = 0;
    th = 0;
    if( GpiQueryTextBox( hps, cch, (PCH)szDraw, TXTBOX_COUNT, aptl ) )
    {
        tw = aptl[TXTBOX_CONCAT].x;
        th = aptl[TXTBOX_TOPLEFT].y - aptl[TXTBOX_BOTTOMLEFT].y;
    }

    if( ( flStyle & XpmBS_AUTOTEXT ) && tw > bw )
    {
        LONG n = cch;

        while( n > 0 )
        {
            szDraw[n]     = '.';
            szDraw[n + 1] = '.';
            szDraw[n + 2] = '.';
            szDraw[n + 3] = '\0';

            if( GpiQueryTextBox( hps, n + 3, (PCH)szDraw,
                                 TXTBOX_COUNT, aptl ) )
            {
                tw = aptl[TXTBOX_CONCAT].x;
                if( tw <= bw )
                    break;
            }
            n--;
        }

        if( n <= 0 )
        {
            szDraw[0] = '\0';
            cch = 0;
        }
        else
        {
            cch = n + 3;
            th  = aptl[TXTBOX_TOPLEFT].y - aptl[TXTBOX_BOTTOMLEFT].y;
        }
    }

    if( cch <= 0 )
        return;

    align = flStyle & XpmBS_ALIGNMASK;

    if( align == XpmBS_ALIGNRIGHT )
        pos.x = box.xRight - tw - 1;
    else if( align == XpmBS_ALIGNTOP || align == XpmBS_ALIGNBOTTOM )
        pos.x = box.xLeft + ( bw - tw ) / 2;
    else
        pos.x = box.xLeft + 1;

    if( align == XpmBS_ALIGNTOP )
        pos.y = box.yTop - th - 1;
    else if( align == XpmBS_ALIGNBOTTOM )
        pos.y = box.yBottom + 1;
    else
        pos.y = box.yBottom + ( bh - th ) / 2;

    GpiSetColor( hps, fg );
    GpiSetBackColor( hps, bg );
    GpiSetBackMix( hps, BM_OVERPAINT );
    GpiCharStringPosAt( hps, &pos, &box, CHS_CLIP, cch, szDraw, NULL );
}

/* Draw the bitmap face, centred in the client rectangle. */
static void DrawBitmapFace( HPS hps, PRECTL prcl, HBITMAP hbm,
                            HBITMAP hbmMask )
{
    POINTL pos;
    LONG   bw, bh;

    BitmapSize( hbm, &bw, &bh );

    pos.x = prcl->xLeft + ( ( prcl->xRight - prcl->xLeft ) - bw ) / 2;
    pos.y = prcl->yBottom + ( ( prcl->yTop - prcl->yBottom ) - bh ) / 2;

    XpmDrawBitmap( hps, hbm, hbmMask, &pos, TRUE );
}

static void DrawButton( HWND hwnd, HPS hps )
{
    XPMWBTNDATA *p = BtnData( hwnd );
    RECTL        rcl;
    ULONG        flStyle;
    BOOL         fDisabled;
    BOOL         fDrawFrame;
    LONG         bg, fg, clrLight, clrDark, clrDefault;
    HBITMAP      hbm, hbmMask;

    if( !p )
        return;

    flStyle = p->flStyle;
    WinQueryWindowRect( hwnd, &rcl );

    /* A PS from WinBeginPaint is in colour index mode; we draw with RGB
     * values, so select RGB mode explicitly. */
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );

    fDisabled  = !WinIsWindowEnabled( hwnd );
    QueryColors( hwnd, !fDisabled, &bg, &fg );
    clrLight   = QueryColor( hwnd, PP_BORDERLIGHTCOLOR,
                             (ULONG)SYSCLR_BUTTONLIGHT, 0x00FFFFFF );
    clrDark    = QueryColor( hwnd, PP_BORDERDARKCOLOR,
                             (ULONG)SYSCLR_BUTTONDARK, 0x00808080 );
    clrDefault = QueryColor( hwnd, PP_BORDERDEFAULTCOLOR,
                             (ULONG)SYSCLR_BUTTONDEFAULT, 0x00000000 );

    /* Background: the owner may paint it when XpmBS_OWNERDRAW is set. */
    if( flStyle & XpmBS_OWNERDRAW )
    {
        HWND         hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
        XpmOwnerBack ob;
        BOOL         fDrawn;

        if( !hwndOwner )
            hwndOwner = WinQueryWindow( hwnd, QW_PARENT );

        ob.hwnd       = hwnd;
        ob.hps        = hps;
        ob.bound      = rcl;
        ob.refpoint.x = rcl.xLeft;
        ob.refpoint.y = rcl.yBottom;

        fDrawn = FALSE;
        if( hwndOwner )
            fDrawn = (BOOL)WinSendMsg( hwndOwner, WM_DRAWITEM,
                                       MPFROM2SHORT(
                                           WinQueryWindowUShort( hwnd, QWS_ID ),
                                           0 ),
                                       MPFROMP( &ob ) );

        if( !fDrawn )
            WinFillRect( hps, &rcl, bg );
    }
    else
    {
        WinFillRect( hps, &rcl, bg );
    }

    /* Frame: a flat button keeps its face plain until the pointer is
     * over it (or it has the focus / is pressed). */
    fDrawFrame = TRUE;
    if( ( flStyle & XpmBS_FLATBUTTON ) && !fDisabled &&
        !p->fHilited && !p->fPressed && !p->fFocus )
        fDrawFrame = FALSE;

    if( fDrawFrame )
        DrawFrame( hps, &rcl, flStyle, p->fPressed,
                   clrLight, clrDark, clrDefault );

    /* Pick the bitmap for the current state. */
    hbm     = p->hbm;
    hbmMask = p->hbmMask;
    if( fDisabled && p->hbmDis )
    {
        hbm     = p->hbmDis;
        hbmMask = p->hbmDisMask;
    }
    else if( p->fPressed && p->hbmPress )
    {
        hbm     = p->hbmPress;
        hbmMask = p->hbmPrsMask;
    }
    else if( p->fHilited && p->hbmHilite )
    {
        hbm     = p->hbmHilite;
        hbmMask = p->hbmHilMask;
    }

    if( hbm )
        DrawBitmapFace( hps, &rcl, hbm, hbmMask );

    if( p->szText[0] )
        DrawTextStr( hps, &rcl, p, bg, fg );

    if( p->fFocus )
        DrawFocus( hps, &rcl, fg );
}

/* ------------------------------------------------------------------ */
/* Notifications                                                      */
/* ------------------------------------------------------------------ */

/* Send a button notification to the owner via WM_COMMAND (or
 * WM_SYSCOMMAND / WM_HELP when the matching style is set). */
static void SendNotify( HWND hwnd, USHORT code, BOOL fPointer )
{
    HWND   hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
    USHORT id        = WinQueryWindowUShort( hwnd, QWS_ID );
    ULONG  flStyle   = BtnStyle( hwnd );

    if( !hwndOwner )
        return;

    if( code == XpmBN_CLICKED )
    {
        if( flStyle & XpmBS_SYSCOMMAND )
        {
            WinSendMsg( hwndOwner, WM_SYSCOMMAND, MPFROMSHORT( id ),
                        MPFROMSHORT( 0 ) );
            return;
        }
        if( flStyle & XpmBS_HELP )
        {
            WinSendMsg( hwndOwner, WM_HELP, MPFROMSHORT( id ),
                        MPFROMSHORT( 0 ) );
            return;
        }
    }

    WinSendMsg( hwndOwner, WM_COMMAND, MPFROM2SHORT( id, code ),
                MPFROM2SHORT( (USHORT)CMDSRC_OTHER,
                              (USHORT)( fPointer ? TRUE : FALSE ) ) );
}

/* Send a WM_CONTROL notification to the owner. */
static void NotifyControl( HWND hwnd, USHORT code, ULONG spec )
{
    HWND   hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
    USHORT id        = WinQueryWindowUShort( hwnd, QWS_ID );

    if( hwndOwner )
        WinSendMsg( hwndOwner, WM_CONTROL, MPFROM2SHORT( id, code ),
                    MPFROMLONG( spec ) );
}

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static BOOL PtInClient( HWND hwnd, LONG x, LONG y )
{
    RECTL rcl;

    if( !WinQueryWindowRect( hwnd, &rcl ) )
        return FALSE;

    return ( x >= rcl.xLeft && x < rcl.xRight &&
             y >= rcl.yBottom && y < rcl.yTop );
}

/* Store a bounded copy of pszText, stripping an AsuaCtl bitmap
 * specification ("#<id1>#<id2>\ttext") prefix if present.  EXPM does
 * not load the referenced resources; the bitmap handles must be passed
 * through XpmWBTNCDATA instead. */
static void StoreTitle( XPMWBTNDATA *p, const CHAR *psz )
{
    const CHAR *pt = psz;
    LONG        cch = 0;

    if( psz && psz[0] == '#' )
    {
        const CHAR *q = psz + 1;

        while( *q >= '0' && *q <= '9' )
            q++;
        if( *q == '#' )
        {
            q++;
            while( *q >= '0' && *q <= '9' )
                q++;
            if( *q == '\t' || *q == ' ' )
                q++;
            pt = q;
        }
    }

    if( pt )
    {
        cch = (LONG)strlen( pt );
        if( cch > WBTN_MAXTEXT - 1 )
            cch = WBTN_MAXTEXT - 1;
        memcpy( p->szText, pt, (size_t)cch );
    }
    p->szText[cch] = '\0';
}

/* ------------------------------------------------------------------ */
/* Window procedure                                                   */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY WinButtonProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                       MPARAM mp2 )
{
    XPMWBTNDATA *p = BtnData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (XPMWBTNDATA *)malloc( sizeof( XPMWBTNDATA ) );
            if( !p )
                return (MRESULT)TRUE;

            memset( p, 0, sizeof( XPMWBTNDATA ) );

            p->flStyle = pcs ? pcs->flStyle : BtnStyle( hwnd );

            if( pcs && pcs->pszText )
                StoreTitle( p, pcs->pszText );

            if( pcs && pcs->pCtlData )
            {
                PXpmWBTNCDATA pcd = (PXpmWBTNCDATA)pcs->pCtlData;

                p->hbm        = (HBITMAP)pcd->hColorImage;
                p->hbmDis     = (HBITMAP)pcd->hColorDisImage;
                p->hbmPress   = (HBITMAP)pcd->hColorPressImage;
                p->hbmHilite  = (HBITMAP)pcd->hColorHiliteImage;
                p->hbmMask    = (HBITMAP)pcd->hImage;
                p->hbmDisMask = (HBITMAP)pcd->hImageDis;
                p->hbmPrsMask = (HBITMAP)pcd->hImagePress;
                p->hbmHilMask = (HBITMAP)pcd->hImageHilite;
                p->fPressed   = pcd->fsHiliteState ? TRUE : FALSE;
            }

            if( pcs && pcs->pPresParams )
                XpmStorePresParams( hwnd, pcs->pPresParams, NULL, 0 );

            WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );

            p->bReady = TRUE;
            AutoSize( hwnd );
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawButton( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_ENABLE:
        case WM_SYSCOLORCHANGE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_PRESPARAMCHANGED:
            WinInvalidateRect( hwnd, NULL, FALSE );
            if( p && p->bReady )
                NotifyControl( hwnd, XpmBN_PRESPARAMCHANGED,
                               (ULONG)LONGFROMMP( mp1 ) );
            return (MRESULT)FALSE;

        case WM_SETFOCUS:
            if( p )
            {
                p->fFocus = ( (ULONG)mp1 != 0 ) ? TRUE : FALSE;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)FALSE;

        case WM_BUTTON1DOWN:
            if( p )
            {
                p->fDown    = TRUE;
                p->fPressed = TRUE;
                WinSetCapture( HWND_DESKTOP, hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_BUTTON1UP:
            if( p && p->fDown )
            {
                BOOL fInside = PtInClient( hwnd, SHORT1FROMMP( mp1 ),
                                           SHORT2FROMMP( mp1 ) );

                p->fDown    = FALSE;
                p->fPressed = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );
                WinInvalidateRect( hwnd, NULL, FALSE );

                if( fInside && WinIsWindowEnabled( hwnd ) )
                    SendNotify( hwnd, XpmBN_CLICKED, TRUE );
            }
            return (MRESULT)0;

        case WM_BUTTON1DBLCLK:
            if( WinIsWindowEnabled( hwnd ) )
                SendNotify( hwnd, XpmBN_DBLCLICKED, TRUE );
            return (MRESULT)0;

        case WM_BUTTON2DOWN:
            if( p )
            {
                p->fDown    = TRUE;
                p->fPressed = TRUE;
                WinSetCapture( HWND_DESKTOP, hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_BUTTON2UP:
            if( p && p->fDown )
            {
                BOOL fInside = PtInClient( hwnd, SHORT1FROMMP( mp1 ),
                                           SHORT2FROMMP( mp1 ) );

                p->fDown    = FALSE;
                p->fPressed = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );
                WinInvalidateRect( hwnd, NULL, FALSE );

                if( fInside && WinIsWindowEnabled( hwnd ) )
                    SendNotify( hwnd, XpmBN_BUTTON2CLICKED, TRUE );
            }
            return (MRESULT)0;

        case WM_MOUSEMOVE:
            if( p )
            {
                BOOL fInside = PtInClient( hwnd, SHORT1FROMMP( mp1 ),
                                           SHORT2FROMMP( mp1 ) );

                if( fInside != p->fHilited )
                {
                    p->fHilited = fInside;
                    if( p->bReady )
                        NotifyControl( hwnd,
                                       fInside ? XpmBN_MOUSEENTER
                                               : XpmBN_MOUSELEAVE, 0 );
                    WinInvalidateRect( hwnd, NULL, FALSE );
                }

                if( p->fDown && fInside != p->fPressed )
                {
                    p->fPressed = fInside;
                    WinInvalidateRect( hwnd, NULL, FALSE );
                }
            }
            return (MRESULT)0;

        case WM_CHAR:
            if( WinIsWindowEnabled( hwnd ) )
            {
                UCHAR ch = CHAR1FROMMP( mp1 );

                if( ch == ' ' || ch == '\r' )
                    SendNotify( hwnd, XpmBN_CLICKED, FALSE );
            }
            return (MRESULT)0;

        case WM_QUERYWINDOWPARAMS:
        {
            PWNDPARAMS pwp = (PWNDPARAMS)PVOIDFROMMP( mp1 );

            if( pwp && p && ( pwp->fsStatus & WPM_CCHTEXT ) )
                pwp->cchText = (ULONG)strlen( p->szText );

            if( pwp && p && ( pwp->fsStatus & WPM_TEXT ) && pwp->pszText )
                strcpy( pwp->pszText, p->szText );

            return (MRESULT)TRUE;
        }

        case WM_SETWINDOWPARAMS:
        {
            PWNDPARAMS pwp = (PWNDPARAMS)PVOIDFROMMP( mp1 );

            if( pwp && p && ( pwp->fsStatus & WPM_TEXT ) )
            {
                StoreTitle( p, pwp->pszText );
                AutoSize( hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)TRUE;
        }

        case BM_AUTOSIZE:
            AutoSize( hwnd );
            return (MRESULT)0;

        case BM_CLICK:
            if( WinIsWindowEnabled( hwnd ) )
                SendNotify( hwnd, XpmBN_CLICKED, FALSE );
            return (MRESULT)0;

        case XpmBM_QUERYHEIGHT:
            return (MRESULT)( p ? MinHeight( hwnd, p ) : 0 );

        case XpmBM_QUERYWIDTH:
            return (MRESULT)( p ? MinWidth( hwnd, p ) : 0 );
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */

LONG EXPENTRY XpmWinButtonInit( HAB hab )
{
    if( g_bBtnRegistered )
        return 0L;

    if( !WinRegisterClass( hab, XpmWinButtonClass, WinButtonProc,
                           CS_SIZEREDRAW | CS_HITTEST,
                           sizeof( XPMWBTNDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;

    g_bBtnRegistered = TRUE;
    return 0L;
}

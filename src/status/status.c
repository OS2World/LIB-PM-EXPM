/*
 * EXPM - Extended Presentation Manager library
 *
 * src/status/status.c -- XpmStatusClass status control.
 *
 * Clean-room reimplementation for the EXPM project (2026) of the
 * AsuaCtl 1.0 status window (WC_STATUS).  Written in C from the
 * published AsuaCtl status header (include/status.h) and the IPF
 * manual (doc/src/status/doc/status.inc); no AsuaCtl source was
 * copied.  AsuaCtl is GPLv2, EXPM is LGPLv3 (see doc\status.txt and
 * doc\ctlcommon.txt for the mapping of names).
 *
 * A status control behaves like a static text field whose text can be
 * aligned, automatically scaled and enabled/disabled, and whose
 * background can optionally be owner-drawn.  Like the original, the
 * control keeps its own copy of the window text and answers
 * WM_QUERYWINDOWPARAMS / WM_SETWINDOWPARAMS itself, so that
 * WinQueryWindowText and WinSetWindowText work on it as well.
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

#define STATUS_MAXTEXT  512
#define STATUS_MARGIN   1           /* text margin (pixels) each side   */
#define STATUS_WINSTYLE_SIZE 12     /* XpmSS_WINSTYLE square, pixels    */

/* Per-window control data. */
typedef struct _XPMSTATUSDATA
{
    BOOL        bActive;            /* highlight on (TRUE) / off (FALSE) */
    BOOL        bReady;             /* TRUE once creation is complete    */
    ULONG       flStyle;            /* cached window style               */
    CHAR        szText[STATUS_MAXTEXT]; /* own copy of the window text   */
    ULONG       ulTextWidth;        /* pixel width of szText             */
    FONTMETRICS fm;                 /* metrics of the current font       */
} XPMSTATUSDATA;

static BOOL g_bStatusRegistered = FALSE;

/* ------------------------------------------------------------------ */

static XPMSTATUSDATA *StatusData( HWND hwnd )
{
    return (XPMSTATUSDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static ULONG StatusStyle( HWND hwnd )
{
    XPMSTATUSDATA *p = StatusData( hwnd );

    return p ? p->flStyle : WinQueryWindowULong( hwnd, QWL_STYLE );
}

/* Width (in pixels) of one border edge, honouring the frame selection
 * bits and the thin-frame option. */
static LONG EdgeWidth( ULONG flStyle, USHORT usEdge )
{
    LONG t;

    if( !( flStyle & XpmSS_BORDER ) )
        return 0L;

    t = ( flStyle & XpmSS_THINFRAME ) ? 1 : 2;
    return ( flStyle & usEdge ) ? t : 0L;
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

/* Foreground/background pair for the current enable state. */
static void QueryColors( HWND hwnd, BOOL bActive, LONG *plBg, LONG *plFg )
{
    if( bActive )
    {
        *plBg = QueryColor( hwnd, PP_BACKGROUNDCOLOR,
                            (ULONG)SYSCLR_ENTRYFIELD, 0x00FFFFFF );
        *plFg = QueryColor( hwnd, PP_FOREGROUNDCOLOR,
                            (ULONG)SYSCLR_WINDOWTEXT, 0x00000000 );
    }
    else
    {
        *plBg = QueryColor( hwnd, PP_DISABLEDBACKGROUNDCOLOR,
                            (ULONG)SYSCLR_ENTRYFIELD, 0x00FFFFFF );
        *plFg = QueryColor( hwnd, PP_DISABLEDFOREGROUNDCOLOR,
                            0, 0x00808080 );
    }
}

/* ------------------------------------------------------------------ */
/* Text and font storage                                              */
/* ------------------------------------------------------------------ */

/* Measure szText with the window's current font. */
static ULONG MeasureText( HWND hwnd, const CHAR *psz, LONG cch )
{
    HPS    hps;
    POINTL aptl[TXTBOX_COUNT];
    ULONG  ulWidth = 0;

    if( cch <= 0 )
        return 0;

    hps = WinGetPS( hwnd );
    if( hps )
    {
        GpiQueryTextBox( hps, cch, (PCH)psz, TXTBOX_COUNT, aptl );
        ulWidth = (ULONG)aptl[TXTBOX_CONCAT].x;
        WinReleasePS( hps );
    }
    return ulWidth;
}

/* Store a new text (bounded copy) and recompute its pixel width. */
static void StoreText( HWND hwnd, XPMSTATUSDATA *p, const CHAR *psz )
{
    LONG cch = 0;

    if( psz )
    {
        cch = (LONG)strlen( psz );
        if( cch > STATUS_MAXTEXT - 1 )
            cch = STATUS_MAXTEXT - 1;
        memcpy( p->szText, psz, cch );
    }
    p->szText[cch] = '\0';
    p->ulTextWidth = MeasureText( hwnd, p->szText, cch );
}

/* Cache the metrics of the window's current font. */
static void StoreFont( HWND hwnd, XPMSTATUSDATA *p )
{
    HPS hps;

    memset( &p->fm, 0, sizeof( p->fm ) );

    hps = WinGetPS( hwnd );
    if( hps )
    {
        GpiQueryFontMetrics( hps, (LONG)sizeof( FONTMETRICS ), &p->fm );
        WinReleasePS( hps );
    }
}

/* ------------------------------------------------------------------ */
/* Automatic scaling                                                  */
/* ------------------------------------------------------------------ */

static void StatAutoscale( HWND hwnd, SWP *pswp )
{
    XPMSTATUSDATA *p = StatusData( hwnd );
    LONG           border;

    if( !p )
        return;

    if( p->flStyle & XpmSS_WINSTYLE )
    {
        pswp->cx = STATUS_WINSTYLE_SIZE;
        pswp->cy = STATUS_WINSTYLE_SIZE;
        return;
    }

    border = ( p->flStyle & XpmSS_THINFRAME ) ? 1 : 2;

    if( p->flStyle & XpmSS_XAUTOSCALE )
    {
        pswp->cx = (LONG)p->ulTextWidth + ( p->fm.lMaxCharInc >> 1 );
        if( p->flStyle & XpmSS_LEFTFRAME )
            pswp->cx += border;
        if( p->flStyle & XpmSS_RIGHTFRAME )
            pswp->cx += border;
    }

    if( p->flStyle & XpmSS_YAUTOSCALE )
    {
        pswp->cy = p->fm.lMaxBaselineExt;
        if( p->flStyle & XpmSS_TOPFRAME )
            pswp->cy += border;
        if( p->flStyle & XpmSS_BOTTOMFRAME )
            pswp->cy += border;
    }
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

/* Draw one rectangular ring, one pixel wide, at the given inset.  Top
 * and left edges use clrLight, bottom and right edges clrDark.  Only
 * the edges selected in flStyle are drawn. */
static void BorderRing( HPS hps, PRECTL prcl, ULONG flStyle, LONG inset,
                        LONG clrLight, LONG clrDark )
{
    LONG xl = prcl->xLeft   + inset;
    LONG yb = prcl->yBottom + inset;
    LONG xr = prcl->xRight  - inset - 1;    /* last addressable column */
    LONG yt = prcl->yTop    - inset - 1;    /* last addressable row    */

    if( flStyle & XpmSS_TOPFRAME )
        DrawLine( hps, xl, yt, xr, yt, clrLight );
    if( flStyle & XpmSS_LEFTFRAME )
        DrawLine( hps, xl, yb, xl, yt, clrLight );
    if( flStyle & XpmSS_BOTTOMFRAME )
        DrawLine( hps, xl, yb, xr, yb, clrDark );
    if( flStyle & XpmSS_RIGHTFRAME )
        DrawLine( hps, xr, yb, xr, yt, clrDark );
}

static void DrawBorder( HWND hwnd, HPS hps, PRECTL prcl, ULONG flStyle )
{
    LONG light, dark, light2, dark2;

    if( !( flStyle & XpmSS_BORDER ) )
        return;

    if( flStyle & XpmSS_THINFRAME )
    {
        dark2 = QueryColor( hwnd, PP_BORDER2DARKCOLOR, 0, 0x00000000 );
        BorderRing( hps, prcl, flStyle, 0, dark2, dark2 );
        return;
    }

    light  = QueryColor( hwnd, PP_BORDERLIGHTCOLOR,  0, 0x00FFFFFF );
    dark   = QueryColor( hwnd, PP_BORDERDARKCOLOR,   0, 0x00808080 );
    light2 = QueryColor( hwnd, PP_BORDER2LIGHTCOLOR, 0, 0x00CCCCCC );
    dark2  = QueryColor( hwnd, PP_BORDER2DARKCOLOR,  0, 0x00000000 );

    BorderRing( hps, prcl, flStyle, 0, light, dark );
    BorderRing( hps, prcl, flStyle, 1, light2, dark2 );
}

/* 'Window scale' grip drawn for XpmSS_WINSTYLE (lower-right corner). */
static void DrawGrip( HWND hwnd, HPS hps, PRECTL prcl )
{
    LONG dark = QueryColor( hwnd, PP_BORDER2DARKCOLOR, 0, 0x00000000 );
    LONG x0   = prcl->xRight - 2;
    LONG y0   = prcl->yBottom + 2;
    int  i;

    for( i = 1; i <= 3; i++ )
        DrawLine( hps, x0 - i * 3, y0, x0, y0 + i * 3, dark );
}

static void DrawText( HWND hwnd, HPS hps, PRECTL prcl, ULONG flStyle,
                      BOOL bActive )
{
    XPMSTATUSDATA *p = StatusData( hwnd );
    LONG           lBg, lFg;
    LONG           border;
    RECTL          box;
    POINTL         pos;
    LONG           textw;

    if( !p || p->szText[0] == '\0' )
        return;

    QueryColors( hwnd, bActive, &lBg, &lFg );

    border = ( flStyle & XpmSS_THINFRAME ) ? 1 : 2;

    box = *prcl;
    if( flStyle & XpmSS_LEFTFRAME )
        box.xLeft   += border;
    if( flStyle & XpmSS_RIGHTFRAME )
        box.xRight  -= border;
    if( flStyle & XpmSS_TOPFRAME )
        box.yTop    -= border;
    if( flStyle & XpmSS_BOTTOMFRAME )
        box.yBottom += border;

    if( box.xRight <= box.xLeft || box.yTop <= box.yBottom )
        return;

    /* Vertical position: centre the font box on the interior. */
    pos.y  = ( box.yTop + box.yBottom - p->fm.lMaxBaselineExt ) / 2;
    pos.y += p->fm.lMaxDescender;

    /* Horizontal position: honour the alignment style. */
    switch( flStyle & 0x0060 )
    {
        case XpmSS_CENTER:
            pos.x = ( box.xRight + box.xLeft - (LONG)p->ulTextWidth ) / 2;
            break;

        case XpmSS_RIGHT:
            pos.x = box.xRight - ( p->fm.lMaxCharInc >> 2 )
                              - (LONG)p->ulTextWidth;
            break;

        case XpmSS_AUTOSHIFT:
            textw = (LONG)p->ulTextWidth
                  + ( ( p->fm.lMaxCharInc & 0xFFFFFFFC ) >> 1 );
            if( textw < ( box.xRight - box.xLeft ) )
                pos.x = box.xLeft + ( p->fm.lMaxCharInc >> 2 );
            else
                pos.x = box.xRight - ( p->fm.lMaxCharInc >> 2 )
                                  - (LONG)p->ulTextWidth;
            break;

        default:
            pos.x = box.xLeft + ( p->fm.lMaxCharInc >> 2 );
            break;
    }

    box.xRight--;
    box.yTop--;

    GpiSetColor( hps, lFg );
    GpiSetBackColor( hps, lBg );
    GpiSetBackMix( hps, BM_OVERPAINT );
    GpiCharStringPosAt( hps, &pos, &box, CHS_CLIP | CHS_OPAQUE,
                        (LONG)strlen( p->szText ), p->szText, NULL );
}

static void DrawStatus( HWND hwnd, HPS hps )
{
    XPMSTATUSDATA *p = StatusData( hwnd );
    RECTL          rcl;
    ULONG          flStyle = StatusStyle( hwnd );
    BOOL           bActive;
    LONG           lBg, lFg;

    bActive = p ? p->bActive : TRUE;
    if( !WinIsWindowEnabled( hwnd ) )
        bActive = FALSE;

    WinQueryWindowRect( hwnd, &rcl );

    /* A PS from WinBeginPaint/WinGetPS is in colour *index* mode; we
     * draw with RGB values, so select RGB mode explicitly. */
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );

    QueryColors( hwnd, bActive, &lBg, &lFg );
    WinFillRect( hps, &rcl, lBg );

    DrawBorder( hwnd, hps, &rcl, flStyle );

    if( flStyle & XpmSS_OWNERDRAW )
    {
        HWND         hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
        XpmOwnerBack ob;

        if( !hwndOwner )
            hwndOwner = WinQueryWindow( hwnd, QW_PARENT );

        ob.hwnd       = hwnd;
        ob.hps        = hps;
        ob.bound      = rcl;
        ob.refpoint.x = rcl.xLeft;
        ob.refpoint.y = rcl.yBottom;

        if( hwndOwner )
            WinSendMsg( hwndOwner, WM_DRAWITEM,
                        MPFROM2SHORT( WinQueryWindowUShort( hwnd, QWS_ID ), 0 ),
                        MPFROMP( &ob ) );
    }

    DrawText( hwnd, hps, &rcl, flStyle, bActive );

    if( flStyle & XpmSS_WINSTYLE )
        DrawGrip( hwnd, hps, &rcl );
}

/* ------------------------------------------------------------------ */
/* Notifications                                                      */
/* ------------------------------------------------------------------ */

static void NotifyOwner( HWND hwnd, ULONG id )
{
    XPMSTATUSDATA *p = StatusData( hwnd );
    HWND           hwndOwner;

    if( !p || !p->bReady )
        return;

    if( id != PP_BACKGROUNDCOLOR && id != PP_FOREGROUNDCOLOR &&
        id != PP_FONTNAMESIZE )
        return;

    hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
    if( !hwndOwner )
        hwndOwner = WinQueryWindow( hwnd, QW_PARENT );

    if( hwndOwner )
        WinPostMsg( hwndOwner, WM_CONTROL,
                    MPFROM2SHORT( WinQueryWindowUShort( hwnd, QWS_ID ),
                                  XpmSMN_PRESPARAMCHANGED ),
                    MPFROMLONG( id ) );
}

/* ------------------------------------------------------------------ */
/* Window procedure                                                   */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY StatusProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    XPMSTATUSDATA *p = StatusData( hwnd );

    switch( msg )
    {
        case WM_ADJUSTWINDOWPOS:
            StatAutoscale( hwnd, (SWP *)PVOIDFROMMP( mp1 ) );
            break;

        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (XPMSTATUSDATA *)malloc( sizeof( XPMSTATUSDATA ) );
            if( !p )
                return (MRESULT)TRUE;

            memset( p, 0, sizeof( XPMSTATUSDATA ) );

            p->flStyle = pcs ? pcs->flStyle : StatusStyle( hwnd );
            p->bActive = TRUE;

            if( pcs && pcs->pPresParams )
                XpmStorePresParams( hwnd, pcs->pPresParams, NULL, 0 );

            StoreFont( hwnd, p );
            StoreText( hwnd, p, pcs ? pcs->pszText : NULL );

            WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );

            p->bReady = TRUE;
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

            DrawStatus( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_ENABLE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            break;

        case WM_HITTEST:
            if( StatusStyle( hwnd ) & XpmSS_MOUSETRANSPARENT )
                return (MRESULT)HT_TRANSPARENT;
            return (MRESULT)HT_NORMAL;

        case WM_PRESPARAMCHANGED:
        {
            ULONG id = (ULONG)LONGFROMMP( mp1 );

            if( p && id == PP_FONTNAMESIZE )
            {
                StoreFont( hwnd, p );
                StoreText( hwnd, p, p->szText );
            }

            WinInvalidateRect( hwnd, NULL, FALSE );
            NotifyOwner( hwnd, id );
            return (MRESULT)FALSE;
        }

        case WM_SYSCOLORCHANGE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_QUERYWINDOWPARAMS:
        {
            PWNDPARAMS pwp = (PWNDPARAMS)PVOIDFROMMP( mp1 );

            if( pwp && p )
            {
                if( pwp->fsStatus & WPM_CCHTEXT )
                    pwp->cchText = (ULONG)strlen( p->szText );

                if( pwp->fsStatus & WPM_TEXT )
                {
                    if( pwp->pszText )
                        strcpy( pwp->pszText, p->szText );
                }
            }
            return (MRESULT)TRUE;
        }

        case WM_SETWINDOWPARAMS:
        {
            PWNDPARAMS pwp = (PWNDPARAMS)PVOIDFROMMP( mp1 );

            if( pwp && p && ( pwp->fsStatus & WPM_TEXT ) )
            {
                StoreText( hwnd, p, pwp->pszText );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)TRUE;
        }

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case XpmSM_QUERYMAXCHARINC:
            return (MRESULT)( p ? p->fm.lMaxCharInc : 0 );

        case XpmSM_QUERYTEXTHEIGHT:
            return (MRESULT)( p ? (LONG)p->fm.lMaxBaselineExt
                                  + 2 * STATUS_MARGIN : 0 );

        case XpmSM_QUERYTEXTWIDTH:
            return (MRESULT)( p ? (LONG)p->ulTextWidth : 0 );

        case XpmSM_QUERYHEIGHT:
        {
            ULONG flStyle = StatusStyle( hwnd );

            return (MRESULT)( p ? (LONG)p->fm.lMaxBaselineExt
                                    + 2 * STATUS_MARGIN
                                    + EdgeWidth( flStyle, XpmSS_TOPFRAME )
                                    + EdgeWidth( flStyle, XpmSS_BOTTOMFRAME )
                                : 0 );
        }

        case XpmSM_QUERYWIDTH:
        {
            ULONG flStyle = StatusStyle( hwnd );

            return (MRESULT)( p ? (LONG)p->ulTextWidth
                                    + ( p->fm.lMaxCharInc >> 1 )
                                    + EdgeWidth( flStyle, XpmSS_LEFTFRAME )
                                    + EdgeWidth( flStyle, XpmSS_RIGHTFRAME )
                                : 0 );
        }

        case XpmSM_QUERYHILITE:
            return (MRESULT)( p ? p->bActive : FALSE );

        case XpmSM_SETHILITE:
            if( p )
            {
                p->bActive = ( (ULONG)mp1 != 0 ) ? TRUE : FALSE;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */

LONG EXPENTRY XpmStatusInit( HAB hab )
{
    if( g_bStatusRegistered )
        return 0L;

    if( !WinRegisterClass( hab, XpmStatusClass, StatusProc,
                           CS_SIZEREDRAW | CS_HITTEST,
                           sizeof( XPMSTATUSDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;

    g_bStatusRegistered = TRUE;
    return 0L;
}

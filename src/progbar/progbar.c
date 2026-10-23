/*
 * EXPM - Extended Presentation Manager library
 *
 * src/progbar/progbar.c -- XpmProgressBarClass progress bar control.
 *
 * Clean-room reimplementation for the EXPM project (2026) of the
 * AsuaCtl 1.0 progress bar (WC_PROGRESSBAR).  Written in C from the
 * published AsuaCtl header (include/progbar.h) only; no AsuaCtl source
 * was consulted, so the behaviour is EXPM's own interpretation of that
 * published API.  AsuaCtl is GPLv2, EXPM is LGPLv3 (see doc\progbar.txt
 * for the mapping of names).
 *
 * AsuaCtl's progress bar mirrors the Windows progress-bar API: a range
 * (low/high), a current position, a step, and PBM_* messages to drive
 * them, plus a "bar colour" and "background colour".  EXPM keeps those
 * semantics and prefixes every public name with Xpm.
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
#include <stdio.h>
#include <string.h>
#include "expm.h"

/* Default colours (GPI RGB values, 0x00RRGGBB). */
#define PROG_DEFAULT_BAR   0x000080C0UL   /* medium blue                     */
#define PROG_BORDER_LIGHT  0x00FFFFFFUL   /* white                           */
#define PROG_BORDER_DARK   0x00808080UL   /* 50% grey                        */
#define PROG_TEXT_COLOR    0x00000000UL   /* black                           */

#define PROG_DEFAULT_LOW   0
#define PROG_DEFAULT_HIGH  100
#define PROG_DEFAULT_STEP  10

/* Per-window control data. */
typedef struct _XPMPROGDATA
{
    BOOL   bReady;      /* TRUE once creation is complete    */
    ULONG  flStyle;     /* cached window style               */
    SHORT  sLow;        /* range low limit                   */
    SHORT  sHigh;       /* range high limit                  */
    SHORT  sPos;        /* current position (sLow..sHigh)    */
    SHORT  sStep;       /* step for XpmPBM_STEPIT            */
    LONG   clrBar;      /* bar colour, 0 = default           */
    LONG   clrBack;     /* background colour, 0 = default    */
} XPMPROGDATA;

static BOOL g_bProgRegistered = FALSE;

/* ------------------------------------------------------------------ */

static XPMPROGDATA *ProgData( HWND hwnd )
{
    return (XPMPROGDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static ULONG ProgStyle( HWND hwnd )
{
    XPMPROGDATA *p = ProgData( hwnd );

    return p ? p->flStyle : WinQueryWindowULong( hwnd, QWL_STYLE );
}

static void ClampPos( XPMPROGDATA *p )
{
    if( p->sPos < p->sLow )
        p->sPos = p->sLow;
    if( p->sPos > p->sHigh )
        p->sPos = p->sHigh;
}

static LONG BarColor( HWND hwnd, XPMPROGDATA *p )
{
    (void)hwnd;

    return p->clrBar ? p->clrBar : (LONG)PROG_DEFAULT_BAR;
}

static LONG BackColor( HWND hwnd, XPMPROGDATA *p )
{
    ULONG ulColor;

    if( p->clrBack )
        return p->clrBack;

    ulColor = 0;
    if( WinQueryPresParam( hwnd, PP_BACKGROUNDCOLOR, 0, NULL,
                           sizeof( ULONG ), (PVOID)&ulColor, 0 ) )
        return (LONG)ulColor;

    return WinQuerySysColor( HWND_DESKTOP, SYSCLR_ENTRYFIELD, 0 );
}

static LONG TextColor( HWND hwnd )
{
    ULONG ulColor;

    ulColor = 0;
    if( WinQueryPresParam( hwnd, PP_FOREGROUNDCOLOR, 0, NULL,
                           sizeof( ULONG ), (PVOID)&ulColor, 0 ) )
        return (LONG)ulColor;

    return (LONG)PROG_TEXT_COLOR;
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

/* Draw the frame: 2px raised, 2px sunken, or a 1px neutral ring. */
static void DrawFrame( HPS hps, PRECTL prcl, ULONG flStyle )
{
    LONG xl = prcl->xLeft;
    LONG yb = prcl->yBottom;
    LONG xr = prcl->xRight  - 1;
    LONG yt = prcl->yTop    - 1;
    LONG light, dark, i, n;

    if( flStyle & XpmPBS_RAISED )
    {
        light = (LONG)PROG_BORDER_LIGHT;
        dark  = (LONG)PROG_BORDER_DARK;
        n     = 2;
    }
    else if( flStyle & XpmPBS_DEPRESSED )
    {
        light = (LONG)PROG_BORDER_DARK;
        dark  = (LONG)PROG_BORDER_LIGHT;
        n     = 2;
    }
    else
    {
        light = (LONG)PROG_BORDER_DARK;
        dark  = (LONG)PROG_BORDER_DARK;
        n     = 1;
    }

    for( i = 0; i < n; i++ )
    {
        LONG a = xl + i;
        LONG b = yb + i;
        LONG c = xr - i;
        LONG d = yt - i;

        DrawLine( hps, a, d, c, d, light );   /* top    */
        DrawLine( hps, a, b, a, d, light );   /* left   */
        DrawLine( hps, a, b, c, b, dark );    /* bottom */
        DrawLine( hps, c, b, c, d, dark );    /* right  */
    }
}

static void DrawIndicator( HPS hps, PRECTL prcl, ULONG flStyle,
                           XPMPROGDATA *p, LONG clrBar )
{
    LONG  range;
    LONG  num;
    RECTL inner;

    if( p->sHigh <= p->sLow )
        return;

    range = (LONG)p->sHigh - (LONG)p->sLow;
    num   = (LONG)p->sPos - (LONG)p->sLow;
    if( num < 0 )
        num = 0;
    if( num > range )
        num = range;

    inner.xLeft   = prcl->xLeft   + 2;
    inner.yBottom = prcl->yBottom + 2;
    inner.xRight  = prcl->xRight  - 2;
    inner.yTop    = prcl->yTop    - 2;

    if( inner.xRight <= inner.xLeft || inner.yTop <= inner.yBottom )
        return;

    if( flStyle & XpmPBS_VERTICAL )
    {
        LONG iw = inner.yTop - inner.yBottom;

        if( flStyle & XpmPBS_SMOOTH )
        {
            RECTL fill = inner;

            fill.yTop = inner.yBottom + (LONG)( ( (LONG)iw * num ) / range );
            if( fill.yTop > inner.yBottom )
                WinFillRect( hps, &fill, clrBar );
        }
        else
        {
            LONG block = inner.xRight - inner.xLeft;
            LONG nblk  = ( block > 0 ) ? iw / block : 1;
            LONG nfill = (LONG)( ( (LONG)nblk * num ) / range );
            LONG i;

            if( nblk < 1 )
                nblk = 1;
            for( i = 0; i < nfill; i++ )
            {
                RECTL fill = inner;

                fill.yBottom = inner.yBottom + i * block;
                fill.yTop    = fill.yBottom + block - 1;
                if( fill.yTop > inner.yTop )
                    fill.yTop = inner.yTop;
                WinFillRect( hps, &fill, clrBar );
            }
        }
    }
    else
    {
        LONG iw = inner.xRight - inner.xLeft;

        if( flStyle & XpmPBS_SMOOTH )
        {
            RECTL fill = inner;

            fill.xRight = inner.xLeft + (LONG)( ( (LONG)iw * num ) / range );
            if( fill.xRight > inner.xLeft )
                WinFillRect( hps, &fill, clrBar );
        }
        else
        {
            LONG block = inner.yTop - inner.yBottom;
            LONG nblk  = ( block > 0 ) ? iw / block : 1;
            LONG nfill = (LONG)( ( (LONG)nblk * num ) / range );
            LONG i;

            if( nblk < 1 )
                nblk = 1;
            for( i = 0; i < nfill; i++ )
            {
                RECTL fill = inner;

                fill.xLeft  = inner.xLeft + i * block;
                fill.xRight = fill.xLeft + block - 1;
                if( fill.xRight > inner.xRight )
                    fill.xRight = inner.xRight;
                WinFillRect( hps, &fill, clrBar );
            }
        }
    }
}

static void DrawPercent( HPS hps, PRECTL prcl, XPMPROGDATA *p, LONG bg,
                         LONG fg )
{
    POINTL aptl[TXTBOX_COUNT];
    POINTL pos;
    RECTL  box;
    CHAR   szBuf[8];
    LONG   cch;
    LONG   width;
    LONG   range;
    LONG   num;
    LONG   pct;

    if( p->sHigh <= p->sLow )
        pct = 100;
    else
    {
        range = (LONG)p->sHigh - (LONG)p->sLow;
        num   = (LONG)p->sPos - (LONG)p->sLow;
        if( num < 0 )
            num = 0;
        if( num > range )
            num = range;
        pct = ( num * 100 ) / range;
    }

    sprintf( szBuf, "%ld%%", pct );
    cch = (LONG)strlen( szBuf );

    if( !GpiQueryTextBox( hps, cch, szBuf, TXTBOX_COUNT, aptl ) )
        return;

    width = aptl[TXTBOX_BOTTOMRIGHT].x - aptl[TXTBOX_TOPLEFT].x;

    pos.x = ( prcl->xRight + prcl->xLeft - width ) / 2;
    pos.y = ( prcl->yTop + prcl->yBottom ) / 2 - 4;

    box      = *prcl;
    box.xLeft  += 2;
    box.yBottom += 2;
    box.xRight -= 3;
    box.yTop   -= 3;

    GpiSetColor( hps, fg );
    GpiSetBackColor( hps, bg );
    GpiSetBackMix( hps, BM_OVERPAINT );
    GpiCharStringPosAt( hps, &pos, &box, CHS_CLIP | CHS_OPAQUE,
                        cch, szBuf, NULL );
}

static void DrawBar( HWND hwnd, HPS hps )
{
    XPMPROGDATA *p = ProgData( hwnd );
    RECTL        rcl;
    ULONG        flStyle = ProgStyle( hwnd );
    LONG         bg;
    LONG         bar;

    if( !p )
        return;

    WinQueryWindowRect( hwnd, &rcl );

    /* A PS from WinBeginPaint is in colour index mode; we draw with
     * RGB values, so select RGB mode explicitly. */
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );

    bg  = BackColor( hwnd, p );
    bar = BarColor( hwnd, p );

    WinFillRect( hps, &rcl, bg );

    DrawIndicator( hps, &rcl, flStyle, p, bar );

    if( flStyle & XpmPBS_TEXT )
        DrawPercent( hps, &rcl, p, bg, TextColor( hwnd ) );

    DrawFrame( hps, &rcl, flStyle );
}

/* ------------------------------------------------------------------ */
/* Window procedure                                                   */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY ProgBarProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                     MPARAM mp2 )
{
    XPMPROGDATA *p = ProgData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (XPMPROGDATA *)malloc( sizeof( XPMPROGDATA ) );
            if( !p )
                return (MRESULT)TRUE;

            memset( p, 0, sizeof( XPMPROGDATA ) );

            p->flStyle = pcs ? pcs->flStyle : ProgStyle( hwnd );
            p->sLow    = PROG_DEFAULT_LOW;
            p->sHigh   = PROG_DEFAULT_HIGH;
            p->sPos    = PROG_DEFAULT_LOW;
            p->sStep   = PROG_DEFAULT_STEP;

            if( pcs && pcs->pPresParams )
                XpmStorePresParams( hwnd, pcs->pPresParams, NULL, 0 );

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

            DrawBar( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_ENABLE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            break;

        case WM_PRESPARAMCHANGED:
        case WM_SYSCOLORCHANGE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case XpmPBM_SETRANGE:
            if( p )
            {
                p->sLow  = SHORT1FROMMP( mp1 );
                p->sHigh = SHORT2FROMMP( mp2 );
                if( p->sHigh < p->sLow )
                {
                    SHORT t = p->sLow;

                    p->sLow  = p->sHigh;
                    p->sHigh = t;
                }
                ClampPos( p );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case XpmPBM_SETPOS:
        {
            LONG old = p ? p->sPos : 0;

            if( p )
            {
                p->sPos = SHORT1FROMMP( mp1 );
                ClampPos( p );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)old;
        }

        case XpmPBM_DELTAPOS:
        {
            LONG old = p ? p->sPos : 0;

            if( p )
            {
                p->sPos = (SHORT)( p->sPos + SHORT1FROMMP( mp1 ) );
                ClampPos( p );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)old;
        }

        case XpmPBM_SETSTEP:
        {
            LONG old = p ? p->sStep : 0;

            if( p )
                p->sStep = SHORT1FROMMP( mp1 );
            return (MRESULT)old;
        }

        case XpmPBM_STEPIT:
            if( p )
            {
                p->sPos = (SHORT)( p->sPos + p->sStep );
                ClampPos( p );
                WinInvalidateRect( hwnd, NULL, FALSE );
                return (MRESULT)p->sPos;
            }
            return (MRESULT)0;

        case XpmPBM_GETRANGE:
        {
            PXpmPBRange pp = (PXpmPBRange)PVOIDFROMMP( mp1 );

            if( pp && p )
            {
                pp->sLow  = p->sLow;
                pp->sHigh = p->sHigh;
            }
            return (MRESULT)0;
        }

        case XpmPBM_GETPOS:
            return (MRESULT)( p ? p->sPos : 0 );

        case XpmPBM_SETBARCOLOR:
        {
            LONG old = p ? p->clrBar : 0;

            if( p )
            {
                p->clrBar = (LONG)LONGFROMMP( mp1 );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)old;
        }

        case XpmPBM_SETBKCOLOR:
        {
            LONG old = p ? p->clrBack : 0;

            if( p )
            {
                p->clrBack = (LONG)LONGFROMMP( mp1 );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)old;
        }
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */

LONG EXPENTRY XpmProgressBarInit( HAB hab )
{
    if( g_bProgRegistered )
        return 0L;

    if( !WinRegisterClass( hab, XpmProgressBarClass, ProgBarProc,
                           CS_SIZEREDRAW, sizeof( XPMPROGDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;

    g_bProgRegistered = TRUE;
    return 0L;
}

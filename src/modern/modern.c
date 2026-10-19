/*
 * EXPM - Extended Presentation Manager library
 *
 * src/modern/modern.c -- modern ("Material" / iOS style) controls.
 *
 * Original EXPM code (2026), released under the LGPLv3 like the rest of
 * the library.  No third-party source was used.  The module provides a
 * shared theme engine and five controls - XpmToggleSwitch, XpmSegmented,
 * XpmRingProgress, XpmChip and XpmIconButton - drawn with the ordinary
 * PM/GPI primitives (GpiBox with rounded corners, GpiPolyLine arcs,
 * GpiCharStringAt text).  See doc\modern.txt for the design.
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

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "expm.h"

/* ------------------------------------------------------------------ */
/* Constants                                                          */
/* ------------------------------------------------------------------ */

#define MXPM_MAXSEG     16              /* segmented items                */
#define MXPM_SEGLEN     64              /* bytes per segmented item       */
#define MXPM_MAXTEXT    256             /* own copy of the window text    */

#define MXPM_HOVER      125             /* 1/8 hover tint (per mille)     */
#define MXPM_PRESS      250             /* 1/4 pressed tint (per mille)   */

#define TIMER_ANIM      1               /* state animation (knob slide)   */
#define TIMER_SPIN      2               /* ring spinner                   */
#define TIMER_HOVER     3               /* pointer leave polling          */

#define MXPM_DEG2RAD    (3.14159265358979323846 / 180.0)
#define MXPM_NRING      180             /* max points used for an arc     */

/* Per-window control data, shared by every modern class. */
typedef struct _MXPMDATA
{
    ULONG    flStyle;                   /* cached create style           */
    XpmTheme theme;                     /* snapshot of the process theme */

    LONG     lCheck;                    /* toggle/chip/iconbtn on-off    */
    LONG     lSel;                      /* segmented selected index      */
    LONG     lHover;                    /* segmented hovered index (-1)  */
    LONG     lPress;                    /* segmented pressed index (-1)  */

    LONG     lLow, lHigh, lPos;         /* ring range / position         */
    LONG     lSpinAngle;                /* ring spinner start angle      */

    LONG     lAnimStart;                /* animation start state         */
    LONG     lAnimFrame;                /* animation progress 0..frames  */
    BOOL     fAnim;                     /* animation running             */
    BOOL     fSpin;                     /* spinner running               */

    BOOL     fHover;                    /* pointer is over the control   */
    BOOL     fHoverTimer;               /* hover poll timer running      */
    BOOL     fPressed;                  /* button held down              */
    BOOL     fTracking;                 /* captured the mouse            */

    HBITMAP  hbmIcon;                   /* optional leading bitmap       */

    LONG     cItems;                    /* segmented item count          */
    CHAR     aszItems[MXPM_MAXSEG][MXPM_SEGLEN];

    CHAR     szText[MXPM_MAXTEXT];      /* own copy of the window text   */
} MXPMDATA, *PMXPMDATA;

/* ------------------------------------------------------------------ */
/* Theme engine                                                       */
/* ------------------------------------------------------------------ */

static XpmTheme g_theme;
static BOOL     g_fThemeInit = FALSE;

LONG EXPENTRY XpmThemeDefault( PXpmTheme pt )
{
    if( !pt )
        return 0L;

    memset( pt, 0, sizeof( XpmTheme ) );

    pt->cbSize      = sizeof( XpmTheme );
    pt->ulStyle     = XpmTH_LIGHT;

    pt->clrCanvas     = 0x00F5F5F5;
    pt->clrSurface    = 0x00FFFFFF;
    pt->clrSurfaceDim = 0x00E0E0E0;
    pt->clrText       = 0x00212121;
    pt->clrTextDim    = 0x009E9E9E;
    pt->clrPrimary    = 0x00620EE6;
    pt->clrOnPrimary  = 0x00FFFFFF;
    pt->clrSecondary  = 0x0000BFA5;
    pt->clrOutline    = 0x00BDBDBD;
    pt->clrHover      = 0x00000000;     /* tint colour (blended in)      */
    pt->clrPressed    = 0x00000000;
    pt->clrShadow     = 0x00C8C8C8;

    pt->lRadius       = 6;
    pt->lRadiusSmall  = 4;
    pt->lBorder       = 1;
    pt->lPadX         = 8;
    pt->lPadY         = 4;
    pt->lRowHeight    = 20;
    pt->lGap          = 6;
    pt->lElevation    = 2;

    pt->lAnimStep     = 15;
    pt->lAnimFrames   = 6;

    pt->szFace[0]     = '\0';
    pt->lPtBody       = 10;
    pt->lPtLabel      = 10;

    return 0L;
}

LONG EXPENTRY XpmThemeDark( PXpmTheme pt )
{
    if( !pt )
        return 0L;

    XpmThemeDefault( pt );

    pt->ulStyle       = XpmTH_DARK;

    pt->clrCanvas     = 0x00121212;
    pt->clrSurface    = 0x001E1E1E;
    pt->clrSurfaceDim = 0x00303030;
    pt->clrText       = 0x00E0E0E0;
    pt->clrTextDim    = 0x00909090;
    pt->clrPrimary    = 0x00BB86FC;
    pt->clrOnPrimary  = 0x00000000;
    pt->clrSecondary  = 0x0003DAC6;
    pt->clrOutline    = 0x00484848;
    pt->clrHover      = 0x00FFFFFF;     /* lighten on the dark surface   */
    pt->clrPressed    = 0x00FFFFFF;
    pt->clrShadow     = 0x00000000;

    return 0L;
}

BOOL EXPENTRY XpmSetTheme( PXpmTheme pt )
{
    if( pt )
        g_theme = *pt;
    else
        XpmThemeDefault( &g_theme );

    g_fThemeInit = TRUE;

    WinBroadcastMsg( HWND_DESKTOP, XpmWM_THEMECHANGED, 0, 0,
                     BMSG_POST | BMSG_DESCENDANTS );
    return TRUE;
}

PXpmTheme EXPENTRY XpmQueryTheme( VOID )
{
    if( !g_fThemeInit )
    {
        XpmThemeDefault( &g_theme );
        g_fThemeInit = TRUE;
    }
    return &g_theme;
}

/* ------------------------------------------------------------------ */
/* Shared helpers                                                     */
/* ------------------------------------------------------------------ */

static PMXPMDATA GetData( HWND hwnd )
{
    return (PMXPMDATA)WinQueryWindowULong( hwnd, QWL_USER );
}

static ULONG GetStyle( HWND hwnd )
{
    PMXPMDATA p = GetData( hwnd );

    return p ? p->flStyle : WinQueryWindowULong( hwnd, QWL_STYLE );
}

static void StoreText( HWND hwnd, PMXPMDATA p, PCSZ psz )
{
    (void)hwnd;
    if( !p )
        return;

    if( !psz )
    {
        p->szText[0] = '\0';
        return;
    }

    strncpy( p->szText, (const char *)psz, MXPM_MAXTEXT - 1 );
    p->szText[MXPM_MAXTEXT - 1] = '\0';
}

/* Blend `over` into `base` by permil (0..1000), channel by channel.
 * The result is a direct 0x00RRGGBB colour. */
static ULONG Blend( ULONG base, ULONG over, LONG permil )
{
    LONG br, bg, bb, orr, og, ob, r, g, b;

    if( permil <= 0 )
        return base;
    if( permil >= 1000 )
        return over;

    br = (LONG)( ( base >> 16 ) & 0xFF );
    bg = (LONG)( ( base >>  8 ) & 0xFF );
    bb = (LONG)(   base         & 0xFF );
    orr = (LONG)( ( over >> 16 ) & 0xFF );
    og = (LONG)( ( over >>  8 ) & 0xFF );
    ob = (LONG)(   over         & 0xFF );

    r = br + ( ( orr - br ) * permil ) / 1000;
    g = bg + ( ( og  - bg ) * permil ) / 1000;
    b = bb + ( ( ob  - bb ) * permil ) / 1000;

    if( r < 0 ) r = 0; else if( r > 255 ) r = 255;
    if( g < 0 ) g = 0; else if( g > 255 ) g = 255;
    if( b < 0 ) b = 0; else if( b > 255 ) b = 255;

    return ( (ULONG)r << 16 ) | ( (ULONG)g << 8 ) | (ULONG)b;
}

/* Surface colour with the hover/pressed tint applied. */
static ULONG StateFill( PMXPMDATA p, ULONG base )
{
    if( p->fPressed )
        return Blend( base, p->theme.clrPressed, MXPM_PRESS );
    if( p->fHover )
        return Blend( base, p->theme.clrHover, MXPM_HOVER );
    return base;
}

/* Put the presentation space into direct RGB colour mode. */
static void SetupRGB( HPS hps )
{
    GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL );
}

static void FillRectColor( HPS hps, PRECTL prcl, ULONG clr )
{
    WinFillRect( hps, prcl, (LONG)clr );
}

static void FillRoundRect( HPS hps, PRECTL prcl, LONG radius, ULONG clr )
{
    POINTL ptlStart, ptlOpp;

    if( radius < 0 )
        radius = 0;

    ptlStart.x = prcl->xLeft;
    ptlStart.y = prcl->yBottom;
    ptlOpp.x   = prcl->xRight;
    ptlOpp.y   = prcl->yTop;

    GpiSetColor( hps, (LONG)clr );
    GpiMove( hps, &ptlStart );
    GpiBox( hps, DRO_FILL, &ptlOpp, radius, radius );
}

static void FrameRoundRect( HPS hps, PRECTL prcl, LONG radius, ULONG clr )
{
    POINTL ptlStart, ptlOpp;

    if( radius < 0 )
        radius = 0;

    ptlStart.x = prcl->xLeft;
    ptlStart.y = prcl->yBottom;
    ptlOpp.x   = prcl->xRight;
    ptlOpp.y   = prcl->yTop;

    GpiSetColor( hps, (LONG)clr );
    GpiSetLineType( hps, LINETYPE_SOLID );
    GpiSetLineWidthGeom( hps, 1 );
    GpiMove( hps, &ptlStart );
    GpiBox( hps, DRO_OUTLINE, &ptlOpp, radius, radius );
}

static void DrawCircleFill( HPS hps, LONG cx, LONG cy, LONG r, ULONG clr )
{
    RECTL rcl;

    if( r < 1 )
        r = 1;

    rcl.xLeft   = cx - r;
    rcl.xRight  = cx + r;
    rcl.yBottom = cy - r;
    rcl.yTop    = cy + r;

    FillRoundRect( hps, &rcl, r, clr );
}

/* Text extent helpers (pels). */
static LONG TextWidth( HPS hps, PCSZ psz )
{
    POINTL aptl[TXTBOX_COUNT];

    if( !psz || !*psz )
        return 0;
    if( !GpiQueryTextBox( hps, (LONG)strlen( (const char *)psz ),
                          (PCH)psz, TXTBOX_COUNT, aptl ) )
        return 0;
    return aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_BOTTOMLEFT].x;
}

static LONG TextHeight( HPS hps, PCSZ psz )
{
    POINTL aptl[TXTBOX_COUNT];

    if( !psz || !*psz )
        return 0;
    if( !GpiQueryTextBox( hps, (LONG)strlen( (const char *)psz ),
                          (PCH)psz, TXTBOX_COUNT, aptl ) )
        return 0;
    return aptl[TXTBOX_TOPLEFT].y - aptl[TXTBOX_BOTTOMRIGHT].y;
}

/* Draw psz centred (or left aligned) in prcl, transparently. */
static void DrawTextRect( HPS hps, PRECTL prcl, ULONG clr, PCSZ psz,
                          BOOL fLeft )
{
    POINTL aptl[TXTBOX_COUNT];
    POINTL ptl;
    LONG   len, cx, cy;

    if( !psz || !*psz )
        return;

    len = (LONG)strlen( (const char *)psz );
    if( !GpiQueryTextBox( hps, len, (PCH)psz, TXTBOX_COUNT, aptl ) )
        return;

    cx = aptl[TXTBOX_TOPRIGHT].x - aptl[TXTBOX_BOTTOMLEFT].x;
    cy = aptl[TXTBOX_TOPLEFT].y  - aptl[TXTBOX_BOTTOMRIGHT].y;

    if( fLeft )
        ptl.x = prcl->xLeft - aptl[TXTBOX_BOTTOMLEFT].x;
    else
        ptl.x = ( prcl->xLeft + prcl->xRight - cx ) / 2
                - aptl[TXTBOX_BOTTOMLEFT].x;

    ptl.y = ( prcl->yBottom + prcl->yTop - cy ) / 2
            - aptl[TXTBOX_BOTTOMRIGHT].y;

    GpiSetBackMix( hps, BM_LEAVEALONE );
    GpiSetColor( hps, (LONG)clr );
    GpiCharStringAt( hps, &ptl, len, (PCH)psz );
}

/* Draw a circular arc of `stroke` pels: centre (cx,cy), radius r,
 * starting at dStart degrees sweeping dSweep (negative = clockwise). */
static void DrawArc( HPS hps, LONG cx, LONG cy, LONG r, double dStart,
                     double dSweep, LONG stroke, ULONG clr )
{
    POINTL aptl[MXPM_NRING + 1];
    LONG   n, i;
    double step, a, sweep;

    if( r < 1 || stroke < 1 )
        return;

    sweep = ( dSweep < 0.0 ) ? -dSweep : dSweep;
    n = (LONG)( sweep / 4.0 ) + 1;
    if( n < 2 )
        n = 2;
    if( n > MXPM_NRING )
        n = MXPM_NRING;

    step = dSweep / (double)n;
    for( i = 0; i <= n; i++ )
    {
        a = ( dStart + step * (double)i ) * MXPM_DEG2RAD;
        aptl[i].x = cx + (LONG)( (double)r * cos( a ) );
        aptl[i].y = cy + (LONG)( (double)r * sin( a ) );
    }

    GpiSetColor( hps, (LONG)clr );
    GpiSetLineType( hps, LINETYPE_SOLID );
    GpiSetLineWidthGeom( hps, stroke );
    GpiSetLineEnd( hps, LINEEND_ROUND );
    GpiSetLineJoin( hps, LINEJOIN_ROUND );

    GpiMove( hps, &aptl[0] );
    GpiPolyLine( hps, n, &aptl[1] );

    GpiSetLineWidthGeom( hps, 1 );
    GpiSetLineEnd( hps, LINEEND_DEFAULT );
    GpiSetLineJoin( hps, LINEJOIN_DEFAULT );
}

/* Draw an "x" close glyph inside prcl. */
static void DrawCloseGlyph( HPS hps, PRECTL prcl, ULONG clr )
{
    POINTL p1, p2, p3, p4;
    LONG   m = ( prcl->xRight - prcl->xLeft ) / 3;

    if( m < 2 )
        m = 2;

    p1.x = prcl->xLeft  + m; p1.y = prcl->yBottom + m;
    p2.x = prcl->xRight - m; p2.y = prcl->yTop    - m;
    p3.x = prcl->xLeft  + m; p3.y = prcl->yTop    - m;
    p4.x = prcl->xRight - m; p4.y = prcl->yBottom + m;

    GpiSetColor( hps, (LONG)clr );
    GpiSetLineType( hps, LINETYPE_SOLID );
    GpiSetLineWidthGeom( hps, 1 );

    GpiMove( hps, &p1 );
    GpiLine( hps, &p2 );
    GpiMove( hps, &p3 );
    GpiLine( hps, &p4 );
}

static void BitmapSize( HBITMAP hbm, PLONG pcx, PLONG pcy )
{
    SIZEL sizl;

    *pcx = 0;
    *pcy = 0;
    if( hbm && GpiQueryBitmapDimension( hbm, &sizl ) )
    {
        *pcx = sizl.cx;
        *pcy = sizl.cy;
    }
}

static BOOL PointInRect( PRECTL prcl, LONG x, LONG y )
{
    return ( x >= prcl->xLeft && x < prcl->xRight &&
             y >= prcl->yBottom && y < prcl->yTop ) ? TRUE : FALSE;
}

/* Send a WM_CONTROL notification to the owner. */
static void Notify( HWND hwnd, USHORT code, ULONG payload )
{
    HWND hwndOwner = WinQueryWindow( hwnd, QW_OWNER );

    if( hwndOwner )
        WinSendMsg( hwndOwner, WM_CONTROL,
                    MPFROM2SHORT( WinQueryWindowUShort( hwnd, QWS_ID ), code ),
                    MPFROMLONG( payload ) );
}

/* Track pointer enter/leave.  Called from WM_MOUSEMOVE. */
static void HoverUpdate( HWND hwnd, PMXPMDATA p )
{
    POINTL ptl;
    RECTL  rcl;
    BOOL   fIn;

    WinQueryPointerPos( HWND_DESKTOP, &ptl );
    WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1 );
    WinQueryWindowRect( hwnd, &rcl );

    fIn = PointInRect( &rcl, ptl.x, ptl.y );

    if( fIn != p->fHover )
    {
        p->fHover = fIn;
        WinInvalidateRect( hwnd, NULL, FALSE );
    }
}

/* Start the hover poll timer if it is not already running. */
static void HoverStart( HWND hwnd, PMXPMDATA p )
{
    if( !p->fHoverTimer )
    {
        WinStartTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_HOVER, 100 );
        p->fHoverTimer = TRUE;
    }
}

/* Handle the hover poll timer; returns TRUE if consumed. */
static BOOL HoverTimer( HWND hwnd, PMXPMDATA p, MPARAM mp1 )
{
    if( (ULONG)SHORT1FROMMP( mp1 ) != (ULONG)TIMER_HOVER )
        return FALSE;

    HoverUpdate( hwnd, p );
    if( !p->fHover )
    {
        WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_HOVER );
        p->fHoverTimer = FALSE;
    }
    return TRUE;
}

/* Announce that the theme changed (refresh the snapshot + repaint). */
static void ThemeChanged( HWND hwnd, PMXPMDATA p )
{
    if( p )
    {
        p->theme = *XpmQueryTheme();
        WinInvalidateRect( hwnd, NULL, FALSE );
    }
}

/* Common WM_CREATE tail: commit QWL_USER and the theme snapshot. */
static void FinishCreate( HWND hwnd, PMXPMDATA p )
{
    p->theme = *XpmQueryTheme();
    WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );
}

/* WM_QUERYWINDOWPARAMS / WM_SETWINDOWPARAMS on the control's own copy
 * of the window text, so WinQueryWindowText / WinSetWindowText work. */
static MRESULT DoWindowParams( ULONG msg, MPARAM mp1, PMXPMDATA p )
{
    PWNDPARAMS pwp = (PWNDPARAMS)PVOIDFROMMP( mp1 );

    if( pwp && p )
    {
        if( msg == WM_QUERYWINDOWPARAMS )
        {
            if( pwp->fsStatus & WPM_CCHTEXT )
                pwp->cchText = (ULONG)strlen( p->szText );

            if( pwp->fsStatus & WPM_TEXT )
            {
                if( pwp->pszText )
                    strcpy( pwp->pszText, p->szText );
            }
        }
        else
        {
            if( pwp->fsStatus & WPM_TEXT )
                StoreText( HWND_DESKTOP, p, pwp->pszText );
        }
    }
    return (MRESULT)TRUE;
}

/* ================================================================== */
/* Toggle switch                                                      */
/* ================================================================== */

#define TOG_INSET(p)    ( (p)->theme.lRowHeight / 8 )

static LONG TogTrackH( HWND hwnd, PMXPMDATA p )
{
    RECTL rcl;
    LONG  h;

    WinQueryWindowRect( hwnd, &rcl );
    h = rcl.yTop - rcl.yBottom;
    if( h > p->theme.lRowHeight )
        h = p->theme.lRowHeight;
    if( h < 12 )
        h = 12;
    return h;
}

/* Fill track/label rects (in window coordinates) for the current size. */
static void TogRects( HWND hwnd, PMXPMDATA p, PRECTL prclTrack,
                      PRECTL prclLabel )
{
    RECTL rcl;
    HPS   hps;
    LONG  th, tw, lw, total, x;
    BOOL  fRight = ( p->flStyle & XpmTOG_LABELRIGHT ) ? TRUE : FALSE;

    WinQueryWindowRect( hwnd, &rcl );
    th = TogTrackH( hwnd, p );
    tw = th * 2;

    hps = WinGetPS( hwnd );
    lw  = TextWidth( hps, p->szText );
    WinReleasePS( hps );

    if( lw > 0 )
        total = lw + p->theme.lGap + tw;
    else
        total = tw;

    x = rcl.xLeft + ( ( rcl.xRight - rcl.xLeft ) - total ) / 2;
    if( x < rcl.xLeft )
        x = rcl.xLeft;

    prclLabel->xLeft  = x;
    prclLabel->xRight = x;

    if( fRight )
    {
        if( lw > 0 )
        {
            prclLabel->xLeft  = x;
            prclLabel->xRight = x + lw;
            x += lw + p->theme.lGap;
        }
        prclTrack->xLeft  = x;
        prclTrack->xRight = x + tw;
    }
    else
    {
        prclTrack->xLeft  = x;
        prclTrack->xRight = x + tw;
        x += tw + p->theme.lGap;
        if( lw > 0 )
        {
            prclLabel->xLeft  = x;
            prclLabel->xRight = x + lw;
        }
    }

    prclTrack->yBottom = rcl.yBottom + ( ( rcl.yTop - rcl.yBottom ) - th ) / 2;
    prclTrack->yTop    = prclTrack->yBottom + th;
    prclLabel->yBottom = prclTrack->yBottom;
    prclLabel->yTop    = prclTrack->yTop;
}

static void TogPaint( HWND hwnd, HPS hps, PMXPMDATA p )
{
    RECTL rcl, rclTrack, rclLabel;
    LONG  th, inset, kd, kcx, kOff, kOn, f, n;
    ULONG track, knob;
    double frac;

    SetupRGB( hps );
    WinQueryWindowRect( hwnd, &rcl );
    FillRectColor( hps, &rcl, p->theme.clrCanvas );

    rclTrack.xLeft = rclTrack.xRight = 0;
    rclTrack.yBottom = rclTrack.yTop = 0;
    rclLabel = rclTrack;
    TogRects( hwnd, p, &rclTrack, &rclLabel );

    th    = rclTrack.yTop - rclTrack.yBottom;
    inset = th / 8;
    kd    = th - 2 * inset;

    n = ( p->theme.lAnimFrames > 0 ) ? p->theme.lAnimFrames : 1;
    f = p->lAnimFrame;
    if( f > n ) f = n;

    frac = (double)f / (double)n;

    kOff = rclTrack.xLeft  + inset + kd / 2;
    kOn  = rclTrack.xRight - inset - kd / 2;
    kcx  = kOff + (LONG)( (double)( kOn - kOff ) * frac );

    if( p->lAnimStart != p->lCheck )
        track = ( p->lCheck ? Blend( p->theme.clrSurfaceDim,
                                     p->theme.clrPrimary,
                                     (LONG)( frac * 1000.0 ) )
                            : Blend( p->theme.clrPrimary,
                                     p->theme.clrSurfaceDim,
                                     (LONG)( frac * 1000.0 ) ) );
    else
        track = p->lCheck ? p->theme.clrPrimary : p->theme.clrSurfaceDim;

    track = StateFill( p, track );

    FillRoundRect( hps, &rclTrack, th / 2, track );

    knob = p->theme.clrSurface;
    DrawCircleFill( hps, kcx, ( rclTrack.yBottom + rclTrack.yTop ) / 2,
                    kd / 2, knob );

    if( p->szText[0] )
        DrawTextRect( hps, &rclLabel, p->theme.clrText, p->szText, TRUE );
}

static void TogSize( HWND hwnd, PMXPMDATA p )
{
    HPS  hps;
    LONG th, tw, lw, w, h;

    if( !( p->flStyle & XpmTOG_AUTOSIZE ) )
        return;

    hps = WinGetPS( hwnd );
    lw  = TextWidth( hps, p->szText );
    WinReleasePS( hps );

    th = p->theme.lRowHeight;
    tw = th * 2;
    w  = tw + ( ( lw > 0 ) ? lw + p->theme.lGap : 0 );
    h  = th;

    WinSetWindowPos( hwnd, NULLHANDLE, 0, 0, w, h, SWP_SIZE );
}

static MRESULT EXPENTRY ToggleProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    PMXPMDATA p = GetData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (PMXPMDATA)malloc( sizeof( MXPMDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            memset( p, 0, sizeof( MXPMDATA ) );

            p->flStyle = pcs ? pcs->flStyle : GetStyle( hwnd );
            p->lCheck  = ( p->flStyle & XpmTOG_CHECKED ) ? 1 : 0;
            p->lAnimStart = p->lCheck;
            StoreText( hwnd, p, pcs ? pcs->pszText : NULL );
            FinishCreate( hwnd, p );
            p->lAnimFrame = p->theme.lAnimFrames;
            TogSize( hwnd, p );
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            if( p )
                TogPaint( hwnd, hps, p );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_MOUSEMOVE:
            if( p )
            {
                HoverUpdate( hwnd, p );
                HoverStart( hwnd, p );
            }
            return (MRESULT)0;

        case WM_BUTTON1DOWN:
            if( p )
            {
                p->fPressed = TRUE;
                p->fTracking = TRUE;
                WinSetCapture( HWND_DESKTOP, hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_BUTTON1UP:
            if( p && p->fTracking )
            {
                RECTL rcl;
                POINTL ptl;

                p->fTracking = FALSE;
                p->fPressed  = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );

                WinQueryPointerPos( HWND_DESKTOP, &ptl );
                WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1 );
                WinQueryWindowRect( hwnd, &rcl );

                if( PointInRect( &rcl, ptl.x, ptl.y ) )
                    WinSendMsg( hwnd, XpmTOGM_TOGGLE, 0, 0 );

                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_TIMER:
            if( p && (ULONG)SHORT1FROMMP( mp1 ) == (ULONG)TIMER_ANIM )
            {
                LONG n = ( p->theme.lAnimFrames > 0 )
                         ? p->theme.lAnimFrames : 1;
                p->lAnimFrame++;
                if( p->lAnimFrame >= n )
                {
                    p->lAnimFrame = n;
                    p->fAnim = FALSE;
                    WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                                  TIMER_ANIM );
                }
                WinInvalidateRect( hwnd, NULL, FALSE );
                return (MRESULT)0;
            }
            if( p && HoverTimer( hwnd, p, mp1 ) )
                return (MRESULT)0;
            break;

        case WM_ENABLE:
        case XpmWM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
            if( msg == XpmWM_THEMECHANGED )
                ThemeChanged( hwnd, p );
            else
                WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_QUERYWINDOWPARAMS:
        case WM_SETWINDOWPARAMS:
            DoWindowParams( msg, mp1, p );
            if( msg == WM_SETWINDOWPARAMS )
                TogSize( hwnd, p );
            return (MRESULT)TRUE;

        case XpmTOGM_SETCHECK:
            if( p )
            {
                LONG v = ( (ULONG)mp1 != 0 ) ? 1 : 0;
                if( v != p->lCheck )
                {
                    p->lAnimStart = p->lCheck;
                    p->lCheck = v;
                    p->lAnimFrame = 0;
                    p->fAnim = TRUE;
                    WinStartTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                                   TIMER_ANIM, p->theme.lAnimStep );
                    WinInvalidateRect( hwnd, NULL, FALSE );
                }
            }
            return (MRESULT)0;

        case XpmTOGM_QUERYCHECK:
            return (MRESULT)( p ? p->lCheck : 0 );

        case XpmTOGM_TOGGLE:
            if( p )
            {
                p->lAnimStart = p->lCheck;
                p->lCheck = p->lCheck ? 0 : 1;
                p->lAnimFrame = 0;
                p->fAnim = TRUE;
                WinStartTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                               TIMER_ANIM, p->theme.lAnimStep );
                WinInvalidateRect( hwnd, NULL, FALSE );
                Notify( hwnd, XpmTOGN_CHANGED, (ULONG)p->lCheck );
            }
            return (MRESULT)0;

        case XpmMCM_QUERYMINWIDTH:
            if( p )
            {
                HPS hps = WinGetPS( hwnd );
                LONG th = p->theme.lRowHeight;
                LONG lw = TextWidth( hps, p->szText );
                WinReleasePS( hps );
                return (MRESULT)( th * 2 + ( ( lw > 0 )
                                             ? lw + p->theme.lGap : 0 ) );
            }
            return (MRESULT)0;

        case XpmMCM_QUERYMINHEIGHT:
            return (MRESULT)( p ? p->theme.lRowHeight : 0 );
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

LONG EXPENTRY XpmToggleSwitchInit( HAB hab )
{
    static BOOL fReg = FALSE;

    if( fReg )
        return 0L;
    if( !WinRegisterClass( hab, XpmToggleSwitchClass, ToggleProc,
                           CS_SIZEREDRAW, sizeof( MXPMDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;
    fReg = TRUE;
    return 0L;
}

/* ================================================================== */
/* Segmented control                                                  */
/* ================================================================== */

/* Compute per-segment left edge and width along the layout axis. */
static void SegLayout( HWND hwnd, PMXPMDATA p, PRECTL prcl,
                       PLONG plLeft, PLONG plWidth, BOOL fVertical )
{
    HPS  hps;
    LONG i, total, fixed, extent, gap;

    hps   = WinGetPS( hwnd );
    gap   = p->theme.lGap;
    fixed = 0;
    for( i = 0; i < p->cItems; i++ )
    {
        LONG w;
        if( p->flStyle & XpmSEG_EQUALWIDTH )
            w = 0;
        else
            w = TextWidth( hps, p->aszItems[i] ) + 2 * p->theme.lPadX;
        plWidth[i] = w;
        fixed += w;
    }
    WinReleasePS( hps );

    if( fVertical )
        extent = prcl->yTop - prcl->yBottom;
    else
        extent = prcl->xRight - prcl->xLeft;

    if( p->flStyle & XpmSEG_EQUALWIDTH )
    {
        LONG each = ( p->cItems > 0 )
                    ? ( extent - gap * ( p->cItems - 1 ) ) / p->cItems : 0;
        for( i = 0; i < p->cItems; i++ )
            plWidth[i] = each;
    }
    else if( fixed + gap * ( p->cItems - 1 ) < extent && p->cItems > 0 )
    {
        total = ( extent - fixed ) / p->cItems;
        if( total > 0 )
        {
            for( i = 0; i < p->cItems; i++ )
                plWidth[i] += total;
        }
    }

    if( fVertical )
    {
        LONG pos = prcl->yBottom;
        for( i = 0; i < p->cItems; i++ )
        {
            plLeft[i] = pos;
            pos += plWidth[i] + gap;
        }
    }
    else
    {
        LONG pos = prcl->xLeft;
        for( i = 0; i < p->cItems; i++ )
        {
            plLeft[i] = pos;
            pos += plWidth[i] + gap;
        }
    }
}

static LONG SegIndexAt( HWND hwnd, PMXPMDATA p, LONG x, LONG y )
{
    RECTL rcl;
    LONG  aLeft[MXPM_MAXSEG], aWidth[MXPM_MAXSEG];
    LONG  i;
    BOOL  fVert = ( p->flStyle & XpmSEG_VERTICAL ) ? TRUE : FALSE;

    if( p->cItems < 1 )
        return -1;

    WinQueryWindowRect( hwnd, &rcl );
    SegLayout( hwnd, p, &rcl, aLeft, aWidth, fVert );

    for( i = 0; i < p->cItems; i++ )
    {
        RECTL sr;
        if( fVert )
        {
            sr.xLeft = rcl.xLeft;
            sr.xRight = rcl.xRight;
            sr.yBottom = aLeft[i];
            sr.yTop = aLeft[i] + aWidth[i];
        }
        else
        {
            sr.xLeft = aLeft[i];
            sr.xRight = aLeft[i] + aWidth[i];
            sr.yBottom = rcl.yBottom;
            sr.yTop = rcl.yTop;
        }
        if( PointInRect( &sr, x, y ) )
            return i;
    }
    return -1;
}

static void SegPaint( HWND hwnd, HPS hps, PMXPMDATA p )
{
    RECTL rcl, cont;
    LONG  aLeft[MXPM_MAXSEG], aWidth[MXPM_MAXSEG];
    BOOL  fVert = ( p->flStyle & XpmSEG_VERTICAL ) ? TRUE : FALSE;
    LONG  i;

    SetupRGB( hps );
    WinQueryWindowRect( hwnd, &rcl );
    FillRectColor( hps, &rcl, p->theme.clrCanvas );

    if( p->cItems < 1 )
        return;

    SegLayout( hwnd, p, &rcl, aLeft, aWidth, fVert );

    cont = rcl;
    FillRoundRect( hps, &cont, p->theme.lRadiusSmall,
                   p->theme.clrSurfaceDim );

    for( i = 0; i < p->cItems; i++ )
    {
        RECTL sr;
        ULONG fill, text;
        BOOL  fSel = ( i == p->lSel );

        if( fVert )
        {
            sr.xLeft = rcl.xLeft + 2;
            sr.xRight = rcl.xRight - 2;
            sr.yBottom = aLeft[i];
            sr.yTop = aLeft[i] + aWidth[i];
        }
        else
        {
            sr.xLeft = aLeft[i];
            sr.xRight = aLeft[i] + aWidth[i];
            sr.yBottom = rcl.yBottom + 2;
            sr.yTop = rcl.yTop - 2;
        }

        if( fSel )
            fill = p->theme.clrSurface;
        else
            fill = p->theme.clrSurfaceDim;

        if( !fSel && i == p->lHover )
            fill = Blend( fill, p->theme.clrHover, MXPM_HOVER );
        if( i == p->lPress )
            fill = Blend( fill, p->theme.clrPressed, MXPM_PRESS );

        if( fSel || i == p->lHover || i == p->lPress )
            FillRoundRect( hps, &sr, p->theme.lRadiusSmall, fill );

        text = fSel ? p->theme.clrText : p->theme.clrTextDim;
        DrawTextRect( hps, &sr, text, p->aszItems[i], FALSE );
    }
}

static void SegNotifySel( HWND hwnd, PMXPMDATA p )
{
    Notify( hwnd, XpmSEGN_SELECT, (ULONG)p->lSel );
}

static MRESULT EXPENTRY SegmentedProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                       MPARAM mp2 )
{
    PMXPMDATA p = GetData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (PMXPMDATA)malloc( sizeof( MXPMDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            memset( p, 0, sizeof( MXPMDATA ) );

            p->flStyle = pcs ? pcs->flStyle : GetStyle( hwnd );
            p->lSel    = -1;
            p->lHover  = -1;
            p->lPress  = -1;
            FinishCreate( hwnd, p );
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            if( p )
                SegPaint( hwnd, hps, p );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_MOUSEMOVE:
            if( p )
            {
                LONG idx = SegIndexAt( hwnd, p, (LONG)SHORT1FROMMP( mp1 ),
                                       (LONG)SHORT2FROMMP( mp1 ) );
                if( idx != p->lHover )
                {
                    p->lHover = idx;
                    WinInvalidateRect( hwnd, NULL, FALSE );
                }
            }
            return (MRESULT)0;

        case WM_BUTTON1DOWN:
            if( p )
            {
                p->lPress = SegIndexAt( hwnd, p, (LONG)SHORT1FROMMP( mp1 ),
                                        (LONG)SHORT2FROMMP( mp1 ) );
                p->fTracking = TRUE;
                WinSetCapture( HWND_DESKTOP, hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_BUTTON1UP:
            if( p )
            {
                LONG idx = SegIndexAt( hwnd, p, (LONG)SHORT1FROMMP( mp1 ),
                                       (LONG)SHORT2FROMMP( mp1 ) );
                p->fTracking = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );
                p->lPress = -1;
                if( idx >= 0 && idx != p->lSel )
                {
                    p->lSel = idx;
                    SegNotifySel( hwnd, p );
                }
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_ENABLE:
        case XpmWM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
            if( msg == XpmWM_THEMECHANGED )
                ThemeChanged( hwnd, p );
            else
                WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case XpmSEGM_SETITEMS:
        {
            PCSZ *apsz = (PCSZ *)PVOIDFROMMP( mp1 );
            LONG  i = 0;

            if( p )
            {
                if( apsz )
                {
                    while( i < MXPM_MAXSEG && apsz[i] )
                    {
                        strncpy( p->aszItems[i], (const char *)apsz[i],
                                 MXPM_SEGLEN - 1 );
                        p->aszItems[i][MXPM_SEGLEN - 1] = '\0';
                        i++;
                    }
                }
                p->cItems = i;
                if( p->cItems < 1 )
                    p->lSel = -1;
                else if( p->lSel < 0 || p->lSel >= p->cItems )
                    p->lSel = 0;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;
        }

        case XpmSEGM_SETSELECT:
            if( p )
            {
                LONG i = (LONG)mp1;
                if( i >= 0 && i < p->cItems && i != p->lSel )
                {
                    p->lSel = i;
                    WinInvalidateRect( hwnd, NULL, FALSE );
                    SegNotifySel( hwnd, p );
                }
            }
            return (MRESULT)0;

        case XpmSEGM_QUERYSELECT:
            return (MRESULT)( p ? p->lSel : -1 );

        case XpmSEGM_QUERYCOUNT:
            return (MRESULT)( p ? p->cItems : 0 );

        case XpmMCM_QUERYMINWIDTH:
        case XpmMCM_QUERYMINHEIGHT:
        {
            HPS  hps;
            LONG i, total;

            if( !p || p->cItems < 1 )
                return (MRESULT)0;

            hps = WinGetPS( hwnd );
            total = 0;
            for( i = 0; i < p->cItems; i++ )
                total += TextWidth( hps, p->aszItems[i] )
                         + 2 * p->theme.lPadX;
            WinReleasePS( hps );

            if( msg == XpmMCM_QUERYMINWIDTH )
                return (MRESULT)( total
                                  + p->theme.lGap * ( p->cItems - 1 ) );
            return (MRESULT)( p->theme.lRowHeight
                              + p->theme.lGap * ( p->cItems - 1 ) );
        }
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

LONG EXPENTRY XpmSegmentedInit( HAB hab )
{
    static BOOL fReg = FALSE;

    if( fReg )
        return 0L;
    if( !WinRegisterClass( hab, XpmSegmentedClass, SegmentedProc,
                           CS_SIZEREDRAW, sizeof( MXPMDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;
    fReg = TRUE;
    return 0L;
}

/* ================================================================== */
/* Ring progress / spinner                                            */
/* ================================================================== */

static void RingPaint( HWND hwnd, HPS hps, PMXPMDATA p )
{
    RECTL rcl;
    LONG  w, h, cx, cy, stroke, r;
    CHAR  szPct[16];

    SetupRGB( hps );
    WinQueryWindowRect( hwnd, &rcl );
    FillRectColor( hps, &rcl, p->theme.clrCanvas );

    w = rcl.xRight - rcl.xLeft;
    h = rcl.yTop - rcl.yBottom;
    cx = ( rcl.xLeft + rcl.xRight ) / 2;
    cy = ( rcl.yBottom + rcl.yTop ) / 2;

    stroke = ( p->flStyle & XpmRNG_THICK ) ? ( ( w < h ) ? w : h ) / 5
                                           : ( ( w < h ) ? w : h ) / 8;
    if( stroke < 2 )
        stroke = 2;

    r = ( ( ( w < h ) ? w : h ) / 2 ) - stroke / 2 - 2;
    if( r < 1 )
        return;

    DrawArc( hps, cx, cy, r, 0.0, 360.0, stroke, p->theme.clrSurfaceDim );

    if( p->flStyle & XpmRNG_INDETERMINATE )
    {
        DrawArc( hps, cx, cy, r, (double)p->lSpinAngle, 100.0, stroke,
                 p->theme.clrPrimary );
    }
    else
    {
        LONG   span = p->lHigh - p->lLow;
        double frac = ( span > 0 )
                      ? ( (double)( p->lPos - p->lLow ) / (double)span )
                      : 0.0;

        if( frac < 0.0 )
            frac = 0.0;
        if( frac > 1.0 )
            frac = 1.0;

        if( frac > 0.0 )
            DrawArc( hps, cx, cy, r, 90.0, -360.0 * frac, stroke,
                     p->theme.clrPrimary );

        if( p->flStyle & XpmRNG_TEXT )
        {
            LONG pct = (LONG)( frac * 100.0 + 0.5 );
            sprintf( szPct, "%ld%%", pct );
            DrawTextRect( hps, &rcl, p->theme.clrText, szPct, FALSE );
        }
    }
}

static MRESULT EXPENTRY RingProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                  MPARAM mp2 )
{
    PMXPMDATA p = GetData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (PMXPMDATA)malloc( sizeof( MXPMDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            memset( p, 0, sizeof( MXPMDATA ) );

            p->flStyle = pcs ? pcs->flStyle : GetStyle( hwnd );
            p->lLow  = 0;
            p->lHigh = 100;
            p->lPos  = 0;
            FinishCreate( hwnd, p );

            if( p->flStyle & XpmRNG_INDETERMINATE )
            {
                p->fSpin = TRUE;
                WinStartTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_SPIN,
                               40 );
            }
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( p )
            {
                if( p->fSpin )
                    WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                                  TIMER_SPIN );
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            if( p )
                RingPaint( hwnd, hps, p );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_TIMER:
            if( p && (ULONG)SHORT1FROMMP( mp1 ) == (ULONG)TIMER_SPIN
                     && p->fSpin )
            {
                p->lSpinAngle += 20;
                if( p->lSpinAngle >= 360 )
                    p->lSpinAngle -= 360;
                WinInvalidateRect( hwnd, NULL, FALSE );
                return (MRESULT)0;
            }
            break;

        case WM_ENABLE:
        case XpmWM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
            if( msg == XpmWM_THEMECHANGED )
                ThemeChanged( hwnd, p );
            else
                WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case XpmRNGM_SETRANGE:
            if( p )
            {
                LONG lo = (LONG)mp1;
                LONG hi = (LONG)mp2;
                p->lLow  = lo;
                p->lHigh = hi;
                if( p->lPos < lo ) p->lPos = lo;
                if( p->lPos > hi ) p->lPos = hi;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case XpmRNGM_SETPOS:
            if( p )
            {
                p->lPos = (LONG)mp1;
                if( p->lPos < p->lLow )  p->lPos = p->lLow;
                if( p->lPos > p->lHigh ) p->lPos = p->lHigh;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case XpmRNGM_GETPOS:
            return (MRESULT)( p ? p->lPos : 0 );

        case XpmRNGM_START:
            if( p && !p->fSpin )
            {
                p->fSpin = TRUE;
                WinStartTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_SPIN,
                               40 );
            }
            return (MRESULT)0;

        case XpmRNGM_STOP:
            if( p && p->fSpin )
            {
                p->fSpin = FALSE;
                WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd, TIMER_SPIN );
            }
            return (MRESULT)0;

        case XpmMCM_QUERYMINWIDTH:
        case XpmMCM_QUERYMINHEIGHT:
            return (MRESULT)32;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

LONG EXPENTRY XpmRingProgressInit( HAB hab )
{
    static BOOL fReg = FALSE;

    if( fReg )
        return 0L;
    if( !WinRegisterClass( hab, XpmRingProgressClass, RingProc,
                           CS_SIZEREDRAW, sizeof( MXPMDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;
    fReg = TRUE;
    return 0L;
}

/* ================================================================== */
/* Chip                                                               */
/* ================================================================== */

/* Fill layout rects for the chip: pill, icon, label, close. */
static void ChipRects( HWND hwnd, PMXPMDATA p, PRECTL prclPill,
                       PRECTL prclIcon, PRECTL prclLabel, PRECTL prclClose )
{
    RECTL rcl;
    HPS   hps;
    LONG  cx = 0, cy = 0, tw, th, x, mid;

    WinQueryWindowRect( hwnd, &rcl );
    hps = WinGetPS( hwnd );
    tw  = TextWidth( hps, p->szText );
    th  = TextHeight( hps, p->szText );
    WinReleasePS( hps );

    if( th < p->theme.lRowHeight - 2 * p->theme.lPadY )
        th = p->theme.lRowHeight - 2 * p->theme.lPadY;

    *prclPill = rcl;

    x   = rcl.xLeft + p->theme.lPadX;
    mid = ( rcl.yBottom + rcl.yTop ) / 2;

    prclIcon->xLeft = prclIcon->xRight = 0;
    if( p->hbmIcon )
    {
        BitmapSize( p->hbmIcon, &cx, &cy );
        prclIcon->xLeft   = x;
        prclIcon->xRight  = x + cx;
        prclIcon->yBottom = mid - cy / 2;
        prclIcon->yTop    = prclIcon->yBottom + cy;
        x += cx + p->theme.lGap;
    }

    prclLabel->xLeft   = x;
    prclLabel->xRight  = x + tw;
    prclLabel->yBottom = mid - th / 2;
    prclLabel->yTop    = prclLabel->yBottom + th;
    x += tw;

    prclClose->xLeft = prclClose->xRight = 0;
    if( p->flStyle & XpmCHIP_CLOSE )
    {
        LONG cw = th;
        x += p->theme.lGap;
        prclClose->xLeft   = x;
        prclClose->xRight  = x + cw;
        prclClose->yBottom = mid - cw / 2;
        prclClose->yTop    = prclClose->yBottom + cw;
    }
}

static void ChipPaint( HWND hwnd, HPS hps, PMXPMDATA p )
{
    RECTL rcl, pill, icon, label, clos;
    ULONG face, text;

    SetupRGB( hps );
    WinQueryWindowRect( hwnd, &rcl );
    FillRectColor( hps, &rcl, p->theme.clrCanvas );

    ChipRects( hwnd, p, &pill, &icon, &label, &clos );

    if( p->lCheck )
        face = p->theme.clrPrimary;
    else
        face = p->theme.clrSurface;
    face = StateFill( p, face );

    FillRoundRect( hps, &pill, ( pill.yTop - pill.yBottom ) / 2, face );

    if( !p->lCheck )
        FrameRoundRect( hps, &pill, ( pill.yTop - pill.yBottom ) / 2,
                        p->theme.clrOutline );

    text = p->lCheck ? p->theme.clrOnPrimary : p->theme.clrText;

    if( p->hbmIcon )
    {
        POINTL ptl;
        ptl.x = icon.xLeft;
        ptl.y = icon.yBottom;
        XpmDrawBitmap( hps, p->hbmIcon, (HBITMAP)NULL, &ptl, FALSE );
    }

    if( p->szText[0] )
        DrawTextRect( hps, &label, text, p->szText, TRUE );

    if( p->flStyle & XpmCHIP_CLOSE )
        DrawCloseGlyph( hps, &clos, text );
}

static void ChipSize( HWND hwnd, PMXPMDATA p )
{
    HPS  hps;
    LONG cx = 0, cy = 0, tw, th, w, h;

    if( !( p->flStyle & XpmCHIP_AUTOSIZE ) )
        return;

    hps = WinGetPS( hwnd );
    tw  = TextWidth( hps, p->szText );
    th  = TextHeight( hps, p->szText );
    WinReleasePS( hps );

    if( p->hbmIcon )
        BitmapSize( p->hbmIcon, &cx, &cy );

    h = p->theme.lRowHeight;
    if( th + 2 * p->theme.lPadY > h )
        h = th + 2 * p->theme.lPadY;

    w = 2 * p->theme.lPadX + tw;
    if( p->hbmIcon )
        w += cx + p->theme.lGap;
    if( p->flStyle & XpmCHIP_CLOSE )
        w += h + p->theme.lGap;

    WinSetWindowPos( hwnd, NULLHANDLE, 0, 0, w, h, SWP_SIZE );
}

static MRESULT EXPENTRY ChipProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                  MPARAM mp2 )
{
    PMXPMDATA p = GetData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (PMXPMDATA)malloc( sizeof( MXPMDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            memset( p, 0, sizeof( MXPMDATA ) );

            p->flStyle = pcs ? pcs->flStyle : GetStyle( hwnd );
            p->lCheck  = ( p->flStyle & XpmCHIP_SELECTED ) ? 1 : 0;
            StoreText( hwnd, p, pcs ? pcs->pszText : NULL );

            if( ( p->flStyle & XpmCHIP_ICON ) && pcs )
                p->hbmIcon = (HBITMAP)pcs->pCtlData;

            FinishCreate( hwnd, p );
            ChipSize( hwnd, p );
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            if( p )
                ChipPaint( hwnd, hps, p );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_MOUSEMOVE:
            if( p )
            {
                HoverUpdate( hwnd, p );
                HoverStart( hwnd, p );
            }
            return (MRESULT)0;

        case WM_BUTTON1DOWN:
            if( p )
            {
                p->fPressed = TRUE;
                p->fTracking = TRUE;
                WinSetCapture( HWND_DESKTOP, hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_BUTTON1UP:
            if( p && p->fTracking )
            {
                RECTL  rcl, pill, icon, label, clos;
                POINTL ptl;

                p->fTracking = FALSE;
                p->fPressed  = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );

                WinQueryPointerPos( HWND_DESKTOP, &ptl );
                WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1 );
                WinQueryWindowRect( hwnd, &rcl );
                ChipRects( hwnd, p, &pill, &icon, &label, &clos );

                if( ( p->flStyle & XpmCHIP_CLOSE )
                    && PointInRect( &clos, ptl.x, ptl.y ) )
                {
                    Notify( hwnd, XpmCHIPN_CLOSE, (ULONG)p->lCheck );
                }
                else if( PointInRect( &rcl, ptl.x, ptl.y ) )
                {
                    if( p->flStyle & XpmCHIP_SELECTABLE )
                        p->lCheck = p->lCheck ? 0 : 1;
                    Notify( hwnd, XpmCHIPN_SEL, (ULONG)p->lCheck );
                }

                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_TIMER:
            if( p && HoverTimer( hwnd, p, mp1 ) )
                return (MRESULT)0;
            break;

        case WM_ENABLE:
        case XpmWM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
            if( msg == XpmWM_THEMECHANGED )
                ThemeChanged( hwnd, p );
            else
                WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_QUERYWINDOWPARAMS:
        case WM_SETWINDOWPARAMS:
            DoWindowParams( msg, mp1, p );
            if( msg == WM_SETWINDOWPARAMS )
                ChipSize( hwnd, p );
            return (MRESULT)TRUE;

        case XpmCHIPM_SETSEL:
            if( p )
            {
                p->lCheck = ( (ULONG)mp1 != 0 ) ? 1 : 0;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case XpmCHIPM_QUERYSEL:
            return (MRESULT)( p ? p->lCheck : 0 );

        case XpmCHIPM_SETICON:
            if( p )
            {
                p->hbmIcon = (HBITMAP)mp1;
                ChipSize( hwnd, p );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case XpmMCM_QUERYMINWIDTH:
        case XpmMCM_QUERYMINHEIGHT:
        {
            HPS  hps;
            LONG cx = 0, cy = 0, tw, th, w, h;

            if( !p )
                return (MRESULT)0;

            hps = WinGetPS( hwnd );
            tw  = TextWidth( hps, p->szText );
            th  = TextHeight( hps, p->szText );
            WinReleasePS( hps );

            if( p->hbmIcon )
                BitmapSize( p->hbmIcon, &cx, &cy );

            h = p->theme.lRowHeight;
            if( th + 2 * p->theme.lPadY > h )
                h = th + 2 * p->theme.lPadY;

            w = 2 * p->theme.lPadX + tw;
            if( p->hbmIcon )
                w += cx + p->theme.lGap;
            if( p->flStyle & XpmCHIP_CLOSE )
                w += h + p->theme.lGap;

            return (MRESULT)( msg == XpmMCM_QUERYMINWIDTH ? w : h );
        }
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

LONG EXPENTRY XpmChipInit( HAB hab )
{
    static BOOL fReg = FALSE;

    if( fReg )
        return 0L;
    if( !WinRegisterClass( hab, XpmChipClass, ChipProc,
                           CS_SIZEREDRAW, sizeof( MXPMDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;
    fReg = TRUE;
    return 0L;
}

/* ================================================================== */
/* Icon button                                                        */
/* ================================================================== */

static void IbPaint( HWND hwnd, HPS hps, PMXPMDATA p )
{
    RECTL rcl, face;
    ULONG fill, glyph;
    LONG  iw = 0, ih = 0, tw, th, cx, cy;

    SetupRGB( hps );
    WinQueryWindowRect( hwnd, &rcl );
    FillRectColor( hps, &rcl, p->theme.clrCanvas );

    face = rcl;

    if( p->lCheck )
        fill = p->theme.clrPrimary;
    else
        fill = p->theme.clrSurface;

    if( p->flStyle & XpmIB_FLAT )
    {
        if( p->fHover || p->fPressed || p->lCheck )
            fill = StateFill( p, fill );
        else
            fill = p->theme.clrCanvas;
    }
    else
    {
        fill = StateFill( p, fill );
    }

    if( p->flStyle & XpmIB_ROUND )
    {
        LONG d = ( ( rcl.xRight - rcl.xLeft ) < ( rcl.yTop - rcl.yBottom ) )
                 ? ( rcl.xRight - rcl.xLeft ) : ( rcl.yTop - rcl.yBottom );
        face.xLeft   = ( rcl.xLeft + rcl.xRight - d ) / 2;
        face.xRight  = face.xLeft + d;
        face.yBottom = ( rcl.yBottom + rcl.yTop - d ) / 2;
        face.yTop    = face.yBottom + d;
        DrawCircleFill( hps, ( face.xLeft + face.xRight ) / 2,
                        ( face.yBottom + face.yTop ) / 2, d / 2, fill );
    }
    else
    {
        FillRoundRect( hps, &face, p->theme.lRadius, fill );
        if( !( p->flStyle & XpmIB_FLAT ) && !p->lCheck )
            FrameRoundRect( hps, &face, p->theme.lRadius,
                            p->theme.clrOutline );
    }

    glyph = p->lCheck ? p->theme.clrOnPrimary : p->theme.clrText;

    if( p->hbmIcon )
        BitmapSize( p->hbmIcon, &iw, &ih );

    tw = 0;
    th = 0;
    if( p->szText[0] )
    {
        tw = TextWidth( hps, p->szText );
        th = TextHeight( hps, p->szText );
    }

    cx = ( rcl.xLeft + rcl.xRight ) / 2;
    cy = ( rcl.yBottom + rcl.yTop ) / 2;

    if( p->flStyle & XpmIB_ALIGNLEFT )
    {
        LONG  x = rcl.xLeft + p->theme.lPadX;
        POINTL ptl;

        if( p->hbmIcon )
        {
            ptl.x = x;
            ptl.y = cy - ih / 2;
            XpmDrawBitmap( hps, p->hbmIcon, (HBITMAP)NULL, &ptl, FALSE );
            x += iw + p->theme.lGap;
        }
        if( p->szText[0] )
        {
            RECTL rl;
            rl.xLeft = x;
            rl.xRight = x + tw;
            rl.yBottom = cy - th / 2;
            rl.yTop = rl.yBottom + th;
            DrawTextRect( hps, &rl, glyph, p->szText, TRUE );
        }
    }
    else
    {
        if( p->hbmIcon && p->szText[0] )
        {
            LONG total = iw + p->theme.lGap + tw;
            LONG x = cx - total / 2;
            POINTL ptl;

            ptl.x = x;
            ptl.y = cy - ih / 2;
            XpmDrawBitmap( hps, p->hbmIcon, (HBITMAP)NULL, &ptl, FALSE );

            {
                RECTL rl;
                rl.xLeft = x + iw + p->theme.lGap;
                rl.xRight = rl.xLeft + tw;
                rl.yBottom = cy - th / 2;
                rl.yTop = rl.yBottom + th;
                DrawTextRect( hps, &rl, glyph, p->szText, TRUE );
            }
        }
        else if( p->hbmIcon )
        {
            POINTL ptl;
            ptl.x = cx - iw / 2;
            ptl.y = cy - ih / 2;
            XpmDrawBitmap( hps, p->hbmIcon, (HBITMAP)NULL, &ptl, FALSE );
        }
        else if( p->szText[0] )
        {
            DrawTextRect( hps, &rcl, glyph, p->szText, FALSE );
        }
    }
}

static void IbSize( HWND hwnd, PMXPMDATA p )
{
    HPS  hps;
    LONG iw = 0, ih = 0, tw, th, w, h;

    if( !( p->flStyle & XpmIB_AUTOSIZE ) )
        return;

    hps = WinGetPS( hwnd );
    tw  = TextWidth( hps, p->szText );
    th  = TextHeight( hps, p->szText );
    WinReleasePS( hps );

    if( p->hbmIcon )
        BitmapSize( p->hbmIcon, &iw, &ih );

    if( p->hbmIcon && p->szText[0] )
        w = 2 * p->theme.lPadX + iw + p->theme.lGap + tw;
    else if( p->hbmIcon )
        w = 2 * p->theme.lPadX + iw;
    else
        w = 2 * p->theme.lPadX + tw;

    if( ih > th )
        h = 2 * p->theme.lPadY + ih;
    else
        h = 2 * p->theme.lPadY + th;
    if( h < p->theme.lRowHeight )
        h = p->theme.lRowHeight;

    if( p->flStyle & XpmIB_ROUND )
    {
        if( w > h )
            h = w;
        else
            w = h;
    }

    WinSetWindowPos( hwnd, NULLHANDLE, 0, 0, w, h, SWP_SIZE );
}

static MRESULT EXPENTRY IconButtonProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                        MPARAM mp2 )
{
    PMXPMDATA p = GetData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (PMXPMDATA)malloc( sizeof( MXPMDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            memset( p, 0, sizeof( MXPMDATA ) );

            p->flStyle = pcs ? pcs->flStyle : GetStyle( hwnd );
            StoreText( hwnd, p, pcs ? pcs->pszText : NULL );

            if( ( p->flStyle & XpmIB_ICON ) && pcs )
                p->hbmIcon = (HBITMAP)pcs->pCtlData;

            FinishCreate( hwnd, p );
            IbSize( hwnd, p );
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            if( p )
                IbPaint( hwnd, hps, p );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_MOUSEMOVE:
            if( p )
            {
                HoverUpdate( hwnd, p );
                HoverStart( hwnd, p );
            }
            return (MRESULT)0;

        case WM_BUTTON1DOWN:
            if( p )
            {
                p->fPressed = TRUE;
                p->fTracking = TRUE;
                WinSetCapture( HWND_DESKTOP, hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_BUTTON1UP:
            if( p && p->fTracking )
            {
                RECTL  rcl;
                POINTL ptl;

                p->fTracking = FALSE;
                p->fPressed  = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );

                WinQueryPointerPos( HWND_DESKTOP, &ptl );
                WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1 );
                WinQueryWindowRect( hwnd, &rcl );

                if( PointInRect( &rcl, ptl.x, ptl.y ) )
                {
                    if( p->flStyle & XpmIB_TOGGLE )
                        p->lCheck = p->lCheck ? 0 : 1;
                    Notify( hwnd, XpmIBN_CLICK, (ULONG)p->lCheck );
                }

                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case WM_TIMER:
            if( p && HoverTimer( hwnd, p, mp1 ) )
                return (MRESULT)0;
            break;

        case WM_ENABLE:
        case XpmWM_THEMECHANGED:
        case WM_SYSCOLORCHANGE:
            if( msg == XpmWM_THEMECHANGED )
                ThemeChanged( hwnd, p );
            else
                WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_QUERYWINDOWPARAMS:
        case WM_SETWINDOWPARAMS:
            DoWindowParams( msg, mp1, p );
            if( msg == WM_SETWINDOWPARAMS )
                IbSize( hwnd, p );
            return (MRESULT)TRUE;

        case XpmIBM_SETICON:
            if( p )
            {
                p->hbmIcon = (HBITMAP)mp1;
                IbSize( hwnd, p );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case XpmIBM_SETCHECK:
            if( p )
            {
                p->lCheck = ( (ULONG)mp1 != 0 ) ? 1 : 0;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)0;

        case XpmIBM_QUERYCHECK:
            return (MRESULT)( p ? p->lCheck : 0 );

        case XpmMCM_QUERYMINWIDTH:
        case XpmMCM_QUERYMINHEIGHT:
        {
            HPS  hps;
            LONG iw = 0, ih = 0, tw, th, w, h;

            if( !p )
                return (MRESULT)0;

            hps = WinGetPS( hwnd );
            tw  = TextWidth( hps, p->szText );
            th  = TextHeight( hps, p->szText );
            WinReleasePS( hps );

            if( p->hbmIcon )
                BitmapSize( p->hbmIcon, &iw, &ih );

            if( p->hbmIcon && p->szText[0] )
                w = 2 * p->theme.lPadX + iw + p->theme.lGap + tw;
            else if( p->hbmIcon )
                w = 2 * p->theme.lPadX + iw;
            else
                w = 2 * p->theme.lPadX + tw;

            h = ( ih > th ) ? ih : th;
            h += 2 * p->theme.lPadY;
            if( h < p->theme.lRowHeight )
                h = p->theme.lRowHeight;

            if( p->flStyle & XpmIB_ROUND )
            {
                if( w > h )
                    h = w;
                else
                    w = h;
            }

            return (MRESULT)( msg == XpmMCM_QUERYMINWIDTH ? w : h );
        }
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

LONG EXPENTRY XpmIconButtonInit( HAB hab )
{
    static BOOL fReg = FALSE;

    if( fReg )
        return 0L;
    if( !WinRegisterClass( hab, XpmIconButtonClass, IconButtonProc,
                           CS_SIZEREDRAW, sizeof( MXPMDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;
    fReg = TRUE;
    return 0L;
}

/* ================================================================== */
/* Umbrella                                                           */
/* ================================================================== */

LONG EXPENTRY XpmModernInit( HAB hab )
{
    LONG rc = 0L;

    if( XpmToggleSwitchInit( hab ) != 0L )
        rc = (LONG)XpmERR_PNL_CLASSREG;
    if( XpmSegmentedInit( hab ) != 0L )
        rc = (LONG)XpmERR_PNL_CLASSREG;
    if( XpmRingProgressInit( hab ) != 0L )
        rc = (LONG)XpmERR_PNL_CLASSREG;
    if( XpmChipInit( hab ) != 0L )
        rc = (LONG)XpmERR_PNL_CLASSREG;
    if( XpmIconButtonInit( hab ) != 0L )
        rc = (LONG)XpmERR_PNL_CLASSREG;

    return rc;
}

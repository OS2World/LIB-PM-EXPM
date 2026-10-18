/*
 * EXPM - Extended Presentation Manager library
 *
 * src/toolbar/toolbar.c -- XpmToolBarClass toolbar control.
 *
 * Clean-room reimplementation for the EXPM project (2026) of the
 * AsuaCtl 1.0 toolbar (WC_TOOLBAR).  Written in C from the published
 * AsuaCtl header (include/toolbar.h) and the published IPF manual
 * (doc/src/toolbar/doc/toolbar.inc) only; no AsuaCtl source was
 * consulted, so the behaviour is EXPM's own interpretation of that
 * published API.  AsuaCtl is GPLv2, EXPM is LGPLv3 (see doc\toolbar.txt
 * for the name mapping and the documented deviations).
 *
 * XpmToolBarClass owns a set of child control windows ("objects") plus
 * painted separators and, optionally, two internal controls: a title bar
 * (used to drag, minimise and dock/undock the toolbar) and a rotate
 * button.  Objects are laid out from the beginning, the centre or the
 * end of the toolbar along its horizontal or vertical axis.
 *
 * The toolbar is created docked (XpmTST_FIXED).  It can be minimised
 * (XpmTST_MINIMIZED, so that only its title bar remains), undocked into
 * the floating state (the toolbar is re-parented to the desktop and can
 * be moved with the mouse) and docked again.  Floating toolbars keep the
 * position they are dropped at; docked ones are stretched across the
 * parent edge selected by the XpmTBA_FIXED* attribute.
 *
 * This release implements the full original control surface.  The
 * remaining differences from AsuaCtl (no separate "object pad" child
 * window, an EXPM-specific rotate glyph, ...) are documented in
 * doc\toolbar.txt.
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
#define TBM_SEP         5       /* separator thickness, pixels        */
#define TBM_GAP         1       /* space between objects, pixels      */
#define TBM_BORDER      1       /* 3-D frame thickness, pixels        */
#define TBM_TIPMAX      128     /* maximum stored tool-tip length     */

/* Internal control metrics */
#define TBM_TITLE       12      /* title-bar strip thickness, pixels  */
#define TBM_TITLEBTN    10      /* dock (fix) button square, pixels   */
#define TBM_ROTATE      17      /* rotate-button square, pixels       */

/* Tool-tip timer */
#define TBM_TIMER_TIP   1       /* poll timer id (on the toolbar)     */
#define TBM_TIMER_HILITE 2      /* one-shot hover timer (on a child)  */
#define TBM_TIPPOLL     200     /* poll interval, milliseconds        */

/* Internal child-window class names (not part of the public API). */
#define XpmToolBarTitleClass    "XpmToolBarTitle"
#define XpmToolBarRotateClass   "XpmToolBarRotate"
#define XpmToolBarTipClass      "XpmToolBarTip"

static BOOL g_bToolRegistered = FALSE;

/* One object placed on the toolbar. */
typedef struct _XPMTOBJ
{
    ULONG   iPosition;      /* ordinal position from the beginning         */
    ULONG   ulGroup;        /* button group                                */
    ULONG   flAttribute;    /* XpmTBO_* / XpmSPS_* attributes              */
    ULONG   orgCx;          /* initial width                               */
    ULONG   orgCy;          /* initial height                              */
    ULONG   curCx;          /* width used by the last layout               */
    ULONG   curCy;          /* height used by the last layout              */
    ULONG   id;             /* object identifier                           */
    ULONG   ulUser;         /* available for the application               */
    HWND    hwnd;           /* object window, or NULLHANDLE (separator)    */
    RECTL   rcl;            /* current rectangle (toolbar client coords)   */
    CHAR    szTip[TBM_TIPMAX];
} XPMTOBJ;

/* Per-window control data. */
typedef struct _XPMTOOLDATA
{
    BOOL     bReady;        /* TRUE once creation is complete              */
    ULONG    flStyle;       /* cached window style                         */
    USHORT   flToolAttrs;   /* XpmTBA_* attributes                         */
    USHORT   usMaxLength;   /* maximum floating length, pixels (0 = none)  */
    ULONG    flState;       /* XpmTST_* state                              */
    BOOL     bRotateEnabled;/* rotate button requested (XpmTM_ENABLEROTATE)*/
    XPMTOBJ **pObj;         /* objects, ordered by position                */
    LONG     cObj;          /* number of objects                           */
    LONG     cAlloc;        /* allocated slots in pObj                     */

    /* Internal control windows */
    HWND     hwndTitle;     /* title bar, or NULLHANDLE                    */
    HWND     hwndRotate;    /* rotate button, or NULLHANDLE                */
    HWND     hwndTip;       /* tool-tip window, or NULLHANDLE              */

    /* Floating/docking */
    HWND     hwndDockParent;/* parent to dock back to when floating        */

    /* Drag tracking */
    BOOL     bTracking;     /* a drag operation is in progress             */
    POINTL   ptlTrack;      /* last pointer position (screen coords)       */
    RECTL    rclTrackBound; /* dragging boundary (parent coords)           */

    /* Tool-tips */
    BOOL     bTipOn;        /* the tool-tip is currently displayed         */
    ULONG    idTipObj;      /* object the tool-tip describes               */
    ULONG    idTipTimer;    /* poll timer id (0 = not running)             */
    LONG     cTipTicks;     /* consecutive ticks over the same object      */
    LONG     lTipDelay;     /* delay before showing the tip, milliseconds  */
} XPMTOOLDATA;

/* Per-window data of the internal title bar. */
typedef struct _XPMTITLE
{
    HWND   hwndToolBar;     /* owning toolbar                              */
    BOOL   bHilite;         /* mouse is over the control                   */
    BOOL   bDown;           /* primary button is down                      */
    BOOL   bDragging;       /* a drag started, so the up event is not a click */
} XPMTITLE;

/* Per-window data of the internal rotate button. */
typedef struct _XPMROTATE
{
    HWND   hwndToolBar;     /* owning toolbar                              */
    BOOL   bHilite;         /* mouse is over the control                   */
} XPMROTATE;

/* ------------------------------------------------------------------ */
/* Small accessors and helpers                                        */
/* ------------------------------------------------------------------ */

static XPMTOOLDATA *ToolData( HWND hwnd )
{
    return (XPMTOOLDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static XPMTITLE *TitleData( HWND hwnd )
{
    return (XPMTITLE *)WinQueryWindowULong( hwnd, QWL_USER );
}

static XPMROTATE *RotateData( HWND hwnd )
{
    return (XPMROTATE *)WinQueryWindowULong( hwnd, QWL_USER );
}

static BOOL IsVertical( XPMTOOLDATA *p )
{
    return ( p->flState & XpmTST_ROTATED ) ? TRUE : FALSE;
}

static BOOL IsMinimized( XPMTOOLDATA *p )
{
    return ( p->flState & XpmTST_MINIMIZED ) ? TRUE : FALSE;
}

static LONG BorderW( XPMTOOLDATA *p )
{
    return ( p->flToolAttrs & XpmTBA_FRAME ) ? TBM_BORDER : 0;
}

/* A title bar is present when the style asks for any of its uses. */
static BOOL HasTitle( XPMTOOLDATA *p )
{
    return ( p->flStyle & ( XpmTBS_MOVEBUTTON | XpmTBS_ENABLEMINIMIZE |
                            XpmTBS_FIXBUTTON ) ) ? TRUE : FALSE;
}

/* The rotate button is present with the style bit or after
 * XpmTM_ENABLEROTATE(TRUE). */
static BOOL HasRotate( XPMTOOLDATA *p )
{
    if( p->flStyle & XpmTBS_ROTATEBUTTON )
        return TRUE;
    return p->bRotateEnabled;
}

/* Dragging the title bar is available with the move-button style and,
 * always, once the toolbar is floating. */
static BOOL CanDrag( XPMTOOLDATA *p )
{
    if( p->flStyle & XpmTBS_MOVEBUTTON )
        return TRUE;
    return !( p->flState & ( XpmTST_FIXED | XpmTST_MINIMIZED ) );
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

/* Read a ULONG presentation parameter; returns the default when absent. */
static ULONG QueryULongPP( HWND hwnd, ULONG id, ULONG ulDefault )
{
    ULONG ulValue = ulDefault;

    if( WinQueryPresParam( hwnd, id, 0, NULL, sizeof( ulValue ),
                           &ulValue, 0 ) == 0 )
        return ulDefault;
    return ulValue;
}

/* Send a WM_CONTROL notification to the owner. */
static void NotifyOwner( HWND hwnd, USHORT code, ULONG spec )
{
    HWND   hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
    USHORT id        = WinQueryWindowUShort( hwnd, QWS_ID );

    if( hwndOwner )
        WinSendMsg( hwndOwner, WM_CONTROL, MPFROM2SHORT( id, code ),
                    MPFROMLONG( spec ) );
}

/* ------------------------------------------------------------------ */
/* Object array                                                       */
/* ------------------------------------------------------------------ */

static LONG FindById( XPMTOOLDATA *p, ULONG id )
{
    LONG i;

    if( !p )
        return -1;

    for( i = 0; i < p->cObj; i++ )
    {
        if( p->pObj[i]->id == id )
            return i;
    }
    return -1;
}

static void Renumber( XPMTOOLDATA *p )
{
    LONG i;

    for( i = 0; i < p->cObj; i++ )
        p->pObj[i]->iPosition = (ULONG)i;
}

static BOOL EnsureCapacity( XPMTOOLDATA *p, LONG need )
{
    XPMTOBJ **q;
    LONG      cap;

    if( need <= p->cAlloc )
        return TRUE;

    cap = p->cAlloc ? p->cAlloc * 2 : 8;
    if( cap < need )
        cap = need;

    q = (XPMTOBJ **)realloc( p->pObj, (size_t)cap * sizeof( XPMTOBJ * ) );
    if( !q )
        return FALSE;

    p->pObj   = q;
    p->cAlloc = cap;
    return TRUE;
}

static void DestroyObj( XPMTOBJ *o )
{
    if( o->hwnd )
        WinDestroyWindow( o->hwnd );
    o->hwnd = NULLHANDLE;
}

/* ------------------------------------------------------------------ */
/* Measurement                                                        */
/* ------------------------------------------------------------------ */

/* Length of an object along the toolbar axis. */
static LONG ObjectLength( XPMTOBJ *o, BOOL vertical )
{
    switch( o->flAttribute & XpmTBO_BASICTYPES )
    {
        case XpmTBO_SEPARATOR:
            return TBM_SEP;

        case XpmTBO_BUTTON:
            if( o->hwnd )
            {
                LONG m = vertical
                       ? (LONG)WinSendMsg( o->hwnd, XpmBM_QUERYHEIGHT, 0, 0 )
                       : (LONG)WinSendMsg( o->hwnd, XpmBM_QUERYWIDTH, 0, 0 );

                if( m > 0 )
                    return m;
            }
            /* fall through */

        default:
            return (LONG)( vertical ? o->orgCy : o->orgCx );
    }
}

/* Size of an object across the toolbar axis. */
static LONG ObjectCross( XPMTOBJ *o, BOOL vertical )
{
    if( ( o->flAttribute & XpmTBO_BASICTYPES ) == XpmTBO_BUTTON && o->hwnd )
    {
        LONG m = vertical
               ? (LONG)WinSendMsg( o->hwnd, XpmBM_QUERYWIDTH, 0, 0 )
               : (LONG)WinSendMsg( o->hwnd, XpmBM_QUERYHEIGHT, 0, 0 );

        if( m > 0 )
            return m;
    }

    return (LONG)( vertical ? o->orgCx : o->orgCy );
}

/* Cross-axis size needed for the given orientation/state. */
static LONG NatCross( XPMTOOLDATA *p, BOOL vertical, BOOL minimized )
{
    LONG i, cross, m;

    if( minimized )
        return TBM_TITLE + 2 * BorderW( p );

    cross = 0;
    for( i = 0; i < p->cObj; i++ )
    {
        if( ( p->pObj[i]->flAttribute & XpmTBO_BASICTYPES ) == XpmTBO_SEPARATOR )
            continue;

        m = ObjectCross( p->pObj[i], vertical );
        if( m > cross )
            cross = m;
    }

    if( HasRotate( p ) && TBM_ROTATE > cross )
        cross = TBM_ROTATE;

    return cross + 2 * BorderW( p );
}

/* Axis size needed for the given orientation/state. */
static LONG NatLength( XPMTOOLDATA *p, BOOL vertical, BOOL minimized )
{
    LONG i, len;

    if( minimized )
        return TBM_TITLE + 2 * BorderW( p );

    len = 0;
    for( i = 0; i < p->cObj; i++ )
    {
        len += ObjectLength( p->pObj[i], vertical );
        if( i < p->cObj - 1 )
            len += TBM_GAP;
    }

    if( HasTitle( p ) )
        len += TBM_TITLE;
    if( HasRotate( p ) )
        len += TBM_ROTATE;

    return len + 2 * BorderW( p );
}

/* ------------------------------------------------------------------ */
/* Controls and visibility                                            */
/* ------------------------------------------------------------------ */

static void ShowObjects( XPMTOOLDATA *p, BOOL bVisible )
{
    LONG i;

    for( i = 0; i < p->cObj; i++ )
    {
        XPMTOBJ *o = p->pObj[i];

        if( !o->hwnd )
            continue;

        if( !bVisible )
            WinShowWindow( o->hwnd, FALSE );
        else
            WinShowWindow( o->hwnd,
                           ( o->flAttribute & XpmTBO_HIDE ) ? FALSE : TRUE );
    }
}

static void HideObjects( XPMTOOLDATA *p )
{
    ShowObjects( p, FALSE );
}

/* Position the internal title bar and rotate button inside the client
 * area (called from LayoutToolBar). */
static void PositionControls( HWND hwnd, XPMTOOLDATA *p )
{
    RECTL rcl, in, r;
    LONG  b = BorderW( p );
    BOOL  vertical = IsVertical( p );

    WinQueryWindowRect( hwnd, &rcl );

    in.xLeft   = rcl.xLeft + b;
    in.xRight  = rcl.xRight - b;
    in.yBottom = rcl.yBottom + b;
    in.yTop    = rcl.yTop - b;
    if( in.xRight < in.xLeft )
        in.xRight = in.xLeft;
    if( in.yTop < in.yBottom )
        in.yTop = in.yBottom;

    if( p->hwndTitle )
    {
        if( vertical )
        {
            r.xLeft   = in.xLeft;
            r.xRight  = in.xRight;
            r.yTop    = in.yTop;
            r.yBottom = in.yTop - TBM_TITLE;
        }
        else
        {
            r.xLeft   = in.xLeft;
            r.xRight  = in.xLeft + TBM_TITLE;
            r.yBottom = in.yBottom;
            r.yTop    = in.yTop;
        }

        if( r.xRight < r.xLeft )
            r.xRight = r.xLeft;
        if( r.yTop < r.yBottom )
            r.yTop = r.yBottom;

        WinSetWindowPos( p->hwndTitle, HWND_TOP, r.xLeft, r.yBottom,
                         r.xRight - r.xLeft, r.yTop - r.yBottom,
                         SWP_MOVE | SWP_SIZE );
    }

    if( p->hwndRotate )
    {
        if( IsMinimized( p ) )
        {
            WinShowWindow( p->hwndRotate, FALSE );
        }
        else
        {
            if( vertical )
            {
                r.yBottom = in.yBottom;
                r.yTop    = in.yBottom + TBM_ROTATE;
                r.xLeft   = in.xLeft + ( ( in.xRight - in.xLeft ) - TBM_ROTATE ) / 2;
                r.xRight  = r.xLeft + TBM_ROTATE;
            }
            else
            {
                r.xRight  = in.xRight;
                r.xLeft   = in.xRight - TBM_ROTATE;
                r.yBottom = in.yBottom + ( ( in.yTop - in.yBottom ) - TBM_ROTATE ) / 2;
                r.yTop    = r.yBottom + TBM_ROTATE;
            }

            WinShowWindow( p->hwndRotate, TRUE );
            WinSetWindowPos( p->hwndRotate, HWND_TOP, r.xLeft, r.yBottom,
                             r.xRight - r.xLeft, r.yTop - r.yBottom,
                             SWP_MOVE | SWP_SIZE );
        }
    }
}

/* ------------------------------------------------------------------ */
/* Layout                                                             */
/* ------------------------------------------------------------------ */

static void LayoutToolBar( HWND hwnd )
{
    XPMTOOLDATA *p = ToolData( hwnd );
    RECTL        rcl;
    RECTL        rect;
    LONG         border;
    LONG         axLen;
    LONG         content;
    LONG         titleT;
    LONG         rotT;
    LONG         beginCur;
    LONG         endCur;
    LONG         centerCur;
    LONG         beginTotal;
    LONG         endTotal;
    LONG         centerTotal;
    LONG         i;
    BOOL         vertical;

    if( !p )
        return;

    PositionControls( hwnd, p );

    if( IsMinimized( p ) )
    {
        HideObjects( p );
        return;
    }

    ShowObjects( p, TRUE );

    WinQueryWindowRect( hwnd, &rcl );
    border = BorderW( p );

    rect = rcl;
    rect.xLeft   += border;
    rect.xRight  -= border;
    rect.yBottom += border;
    rect.yTop    -= border;

    vertical = IsVertical( p );
    titleT   = HasTitle( p )  ? TBM_TITLE  : 0;
    rotT     = HasRotate( p ) ? TBM_ROTATE : 0;

    if( vertical )
    {
        rect.yTop    -= titleT;
        rect.yBottom += rotT;
        axLen   = rect.yTop - rect.yBottom;
        content = rect.xRight - rect.xLeft;
    }
    else
    {
        rect.xLeft  += titleT;
        rect.xRight -= rotT;
        axLen   = rect.xRight - rect.xLeft;
        content = rect.yTop - rect.yBottom;
    }
    if( axLen < 0 )
        axLen = 0;
    if( content < 0 )
        content = 0;

    beginTotal  = 0;
    endTotal    = 0;
    centerTotal = 0;
    for( i = 0; i < p->cObj; i++ )
    {
        ULONG just = p->pObj[i]->flAttribute & XpmTBO_JUSTTYPES;
        LONG  len  = ObjectLength( p->pObj[i], vertical );

        if( len < 0 )
            len = 0;

        if( just == XpmTBO_ENDALIGN )
            endTotal += len + TBM_GAP;
        else if( just == XpmTBO_CENTERALIGN )
            centerTotal += len + TBM_GAP;
        else
            beginTotal += len + TBM_GAP;
    }
    if( beginTotal > 0 )
        beginTotal -= TBM_GAP;
    if( endTotal > 0 )
        endTotal -= TBM_GAP;
    if( centerTotal > 0 )
        centerTotal -= TBM_GAP;

    /* The centre group is centred in the free space between the begin
     * and end groups, so it never collides with them when there is
     * room. */
    beginCur  = 0;
    endCur    = axLen;
    centerCur = beginTotal +
                ( axLen - beginTotal - endTotal - centerTotal ) / 2;
    if( centerCur < beginTotal )
        centerCur = beginTotal;

    for( i = 0; i < p->cObj; i++ )
    {
        XPMTOBJ *o    = p->pObj[i];
        ULONG    just = o->flAttribute & XpmTBO_JUSTTYPES;
        LONG     len  = ObjectLength( o, vertical );
        LONG     axisPos;
        LONG     cx, cy;

        if( len < 0 )
            len = 0;

        if( just == XpmTBO_ENDALIGN )
        {
            endCur -= len;
            axisPos = endCur;
            endCur -= TBM_GAP;
        }
        else if( just == XpmTBO_CENTERALIGN )
        {
            axisPos = centerCur;
            centerCur += len + TBM_GAP;
        }
        else
        {
            axisPos = beginCur;
            beginCur += len + TBM_GAP;
        }

        if( vertical )
        {
            cx = content;
            cy = len;

            o->rcl.xLeft   = rect.xLeft;
            o->rcl.xRight  = rect.xLeft + cx;
            o->rcl.yTop    = rect.yTop - axisPos;
            o->rcl.yBottom = o->rcl.yTop - cy;
        }
        else
        {
            cx = len;
            cy = content;

            o->rcl.xLeft   = rect.xLeft + axisPos;
            o->rcl.xRight  = o->rcl.xLeft + cx;
            o->rcl.yBottom = rect.yBottom;
            o->rcl.yTop    = rect.yBottom + cy;
        }

        o->curCx = (ULONG)cx;
        o->curCy = (ULONG)cy;

        if( o->hwnd )
            WinSetWindowPos( o->hwnd, HWND_TOP,
                             o->rcl.xLeft, o->rcl.yBottom, cx, cy,
                             SWP_MOVE | SWP_SIZE );
    }
}

/* ------------------------------------------------------------------ */
/* Forward declarations for the state and tracking helpers            */
/* ------------------------------------------------------------------ */

static void DoMinimize( HWND hwnd, XPMTOOLDATA *p );
static void DoUnminimize( HWND hwnd, XPMTOOLDATA *p );
static void DoFloat( HWND hwnd, XPMTOOLDATA *p );
static void DoDock( HWND hwnd, XPMTOOLDATA *p );
static void TrackStart( HWND hwnd, XPMTOOLDATA *p );
static void TrackMove( HWND hwnd, XPMTOOLDATA *p );
static void TrackEnd( HWND hwnd, XPMTOOLDATA *p );

static BOOL AnyTips( XPMTOOLDATA *p )
{
    LONG i;

    for( i = 0; i < p->cObj; i++ )
    {
        if( p->pObj[i]->szTip[0] != '\0' )
            return TRUE;
    }
    return FALSE;
}

static void UpdateTipTimer( HWND hwnd, XPMTOOLDATA *p )
{
    if( AnyTips( p ) )
    {
        if( !p->idTipTimer )
        {
            p->idTipTimer  = TBM_TIMER_TIP;
            p->cTipTicks   = 0;
            WinStartTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                           TBM_TIMER_TIP, TBM_TIPPOLL );
        }
    }
    else if( p->idTipTimer )
    {
        WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd, TBM_TIMER_TIP );
        p->idTipTimer = 0;
    }
}

/* ------------------------------------------------------------------ */
/* Object management                                                  */
/* ------------------------------------------------------------------ */

static BOOL InsertObject( HWND hwnd, XPMTOOLDATA *p, PXpmTOOLOBJ po,
                          PXpmTOOLWIN pw )
{
    XPMTOBJ *o;
    ULONG    pos   = po->iPosition;
    ULONG    basic = po->flAttribute & XpmTBO_BASICTYPES;

    if( FindById( p, po->id ) >= 0 )
        return FALSE;

    if( pos == XpmTB_END || pos > (ULONG)p->cObj )
        pos = (ULONG)p->cObj;

    if( !EnsureCapacity( p, p->cObj + 1 ) )
        return FALSE;

    o = (XPMTOBJ *)malloc( sizeof( XPMTOBJ ) );
    if( !o )
        return FALSE;

    memset( o, 0, sizeof( XPMTOBJ ) );
    o->ulGroup     = po->ulGroup;
    o->flAttribute = po->flAttribute;
    o->orgCx       = po->cx;
    o->orgCy       = po->cy;
    o->id          = po->id;
    o->ulUser      = po->ulUser;

    if( basic == XpmTBO_SEPARATOR )
    {
        o->hwnd = NULLHANDLE;
    }
    else
    {
        HWND hwndOwner;

        if( !pw )
        {
            free( o );
            return FALSE;
        }

        hwndOwner = WinQueryWindow( hwnd, QW_OWNER );
        if( !hwndOwner )
            hwndOwner = hwnd;

        o->hwnd = WinCreateWindow( hwnd,
                                   pw->pszClass, pw->pszName,
                                   pw->flStyle | WS_VISIBLE,
                                   0, 0, (LONG)po->cx, (LONG)po->cy,
                                   hwndOwner, HWND_TOP, po->id,
                                   pw->pCtlData, pw->pPresParams );
        if( !o->hwnd )
        {
            free( o );
            return FALSE;
        }

        if( po->flAttribute & XpmTBO_DISABLE )
            WinEnableWindow( o->hwnd, FALSE );
        if( po->flAttribute & XpmTBO_HIDE )
            WinShowWindow( o->hwnd, FALSE );
        else if( IsMinimized( p ) )
            WinShowWindow( o->hwnd, FALSE );
    }

    if( pos < (ULONG)p->cObj )
        memmove( &p->pObj[pos + 1], &p->pObj[pos],
                 (size_t)( p->cObj - (LONG)pos ) * sizeof( XPMTOBJ * ) );
    p->pObj[pos] = o;
    p->cObj++;
    Renumber( p );

    LayoutToolBar( hwnd );
    WinInvalidateRect( hwnd, NULL, FALSE );
    return TRUE;
}

static BOOL DeleteObject( HWND hwnd, XPMTOOLDATA *p, ULONG id )
{
    LONG idx = FindById( p, id );

    if( idx < 0 )
        return FALSE;

    DestroyObj( p->pObj[idx] );
    free( p->pObj[idx] );

    if( idx < p->cObj - 1 )
        memmove( &p->pObj[idx], &p->pObj[idx + 1],
                 (size_t)( p->cObj - 1 - idx ) * sizeof( XPMTOBJ * ) );
    p->cObj--;
    Renumber( p );

    if( p->idTipObj == id )
    {
        p->idTipObj = 0;
        p->cTipTicks = 0;
        if( p->bTipOn && p->hwndTip )
        {
            WinShowWindow( p->hwndTip, FALSE );
            p->bTipOn = FALSE;
        }
    }
    UpdateTipTimer( hwnd, p );

    LayoutToolBar( hwnd );
    WinInvalidateRect( hwnd, NULL, FALSE );
    return TRUE;
}

static BOOL MoveObject( HWND hwnd, XPMTOOLDATA *p, ULONG id, ULONG newPos,
                        ULONG attrs )
{
    LONG     idx = FindById( p, id );
    XPMTOBJ *o;

    if( idx < 0 )
        return FALSE;

    o = p->pObj[idx];

    if( attrs & XpmTBO_JUSTTYPES )
        o->flAttribute = ( o->flAttribute & ~XpmTBO_JUSTTYPES ) |
                         ( attrs & XpmTBO_JUSTTYPES );

    if( p->cObj > 0 && newPos > (ULONG)( p->cObj - 1 ) )
        newPos = (ULONG)( p->cObj - 1 );

    if( (ULONG)idx != newPos )
    {
        if( newPos > (ULONG)idx )
            memmove( &p->pObj[idx], &p->pObj[idx + 1],
                     (size_t)( newPos - (ULONG)idx ) * sizeof( XPMTOBJ * ) );
        else
            memmove( &p->pObj[newPos + 1], &p->pObj[newPos],
                     (size_t)( (ULONG)idx - newPos ) * sizeof( XPMTOBJ * ) );
        p->pObj[newPos] = o;
        Renumber( p );
    }

    LayoutToolBar( hwnd );
    WinInvalidateRect( hwnd, NULL, FALSE );
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* Automatic sizing                                                   */
/* ------------------------------------------------------------------ */

/* Fill a XpmTOOLADJ with a suggested rectangle for the requested state
 * and attributes. */
static void CalcNewRect( HWND hwnd, XPMTOOLDATA *p, PXpmTOOLADJ padj )
{
    USHORT flAttrs = padj->flAttrs ? (USHORT)padj->flAttrs : p->flToolAttrs;
    ULONG  flState = padj->flState ? padj->flState : p->flState;
    HWND   hwndParent = WinQueryWindow( hwnd, QW_PARENT );
    RECTL  prcl;
    LONG   natLen;
    LONG   natCross;
    BOOL   vertical  = ( flState & XpmTST_ROTATED )   ? TRUE : FALSE;
    BOOL   minimized = ( flState & XpmTST_MINIMIZED ) ? TRUE : FALSE;
    BOOL   fixed     = ( flState & XpmTST_FIXED )     ? TRUE : FALSE;

    natLen   = NatLength( p, vertical, minimized );
    natCross = NatCross( p, vertical, minimized );

    if( minimized && hwndParent && WinQueryWindowRect( hwndParent, &prcl ) )
    {
        /* Minimised: a small square at the dock corner. */
        if( vertical )
        {
            padj->rclSizePos.xLeft = ( flAttrs & XpmTBA_FIXEDRIGHT )
                                   ? ( prcl.xRight - TBM_TITLE ) : prcl.xLeft;
            padj->rclSizePos.xRight  = padj->rclSizePos.xLeft + TBM_TITLE;
            padj->rclSizePos.yTop    = prcl.yTop;
            padj->rclSizePos.yBottom = prcl.yTop - TBM_TITLE;
        }
        else
        {
            padj->rclSizePos.xLeft  = prcl.xLeft;
            padj->rclSizePos.xRight = padj->rclSizePos.xLeft + TBM_TITLE;
            if( flAttrs & XpmTBA_FIXEDBOTTOM )
            {
                padj->rclSizePos.yBottom = prcl.yBottom;
                padj->rclSizePos.yTop    = prcl.yBottom + TBM_TITLE;
            }
            else
            {
                padj->rclSizePos.yTop    = prcl.yTop;
                padj->rclSizePos.yBottom = prcl.yTop - TBM_TITLE;
            }
        }
    }
    else if( fixed && hwndParent && WinQueryWindowRect( hwndParent, &prcl ) )
    {
        /* Docked: stretched across the selected parent edge. */
        if( vertical )
        {
            padj->rclSizePos.xLeft = ( flAttrs & XpmTBA_FIXEDRIGHT )
                                   ? ( prcl.xRight - natCross ) : prcl.xLeft;
            padj->rclSizePos.xRight  = padj->rclSizePos.xLeft + natCross;
            padj->rclSizePos.yBottom = prcl.yBottom;
            padj->rclSizePos.yTop    = prcl.yTop;
        }
        else
        {
            padj->rclSizePos.xLeft  = prcl.xLeft;
            padj->rclSizePos.xRight = prcl.xRight;
            if( flAttrs & XpmTBA_FIXEDBOTTOM )
            {
                padj->rclSizePos.yBottom = prcl.yBottom;
                padj->rclSizePos.yTop    = prcl.yBottom + natCross;
            }
            else
            {
                padj->rclSizePos.yTop    = prcl.yTop;
                padj->rclSizePos.yBottom = prcl.yTop - natCross;
            }
        }
    }
    else
    {
        LONG lim = (LONG)p->usMaxLength;
        SWP  swp;

        /* Floating: keep the position; only the size is adjusted to fit
         * the objects. */
        WinQueryWindowPos( hwnd, &swp );

        padj->rclSizePos.xLeft   = swp.x;
        padj->rclSizePos.yBottom = swp.y;

        if( vertical )
        {
            LONG h = natLen;

            padj->rclSizePos.xRight = swp.x + natCross;
            if( lim > 0 && h > lim )
                h = lim;
            padj->rclSizePos.yTop = swp.y + h;
        }
        else
        {
            LONG w = natLen;

            padj->rclSizePos.yTop = swp.y + natCross;
            if( lim > 0 && w > lim )
                w = lim;
            padj->rclSizePos.xRight = swp.x + w;
        }
    }

    padj->hwndToolBar = hwnd;
    padj->hwndParent  = hwndParent;
    padj->flAttrs     = flAttrs;
    padj->flState     = flState;
}

static BOOL AutoSizeToolBar( HWND hwnd, XPMTOOLDATA *p, LONG maxLen )
{
    XpmTOOLADJ adj;
    SWP        swpOld;
    BOOL       changed;

    if( !p )
        return FALSE;

    memset( &adj, 0, sizeof( adj ) );
    adj.flAttrs = p->flToolAttrs;
    adj.flState = p->flState;
    CalcNewRect( hwnd, p, &adj );

    /* A length limit passed to the message overrides the stored one. */
    if( !( p->flState & XpmTST_FIXED ) && !IsMinimized( p ) && maxLen > 0 )
    {
        BOOL vertical = IsVertical( p );

        if( vertical )
        {
            LONG h = adj.rclSizePos.yTop - adj.rclSizePos.yBottom;

            if( h > maxLen )
                adj.rclSizePos.yTop = adj.rclSizePos.yBottom + maxLen;
        }
        else
        {
            LONG w = adj.rclSizePos.xRight - adj.rclSizePos.xLeft;

            if( w > maxLen )
                adj.rclSizePos.xRight = adj.rclSizePos.xLeft + maxLen;
        }
    }

    /* Let the owner adjust the suggestion. */
    adj.flState = p->flState;
    NotifyOwner( hwnd, XpmTN_AUTOSIZE, (ULONG)&adj );

    WinQueryWindowPos( hwnd, &swpOld );

    if( swpOld.x  != adj.rclSizePos.xLeft   ||
        swpOld.y  != adj.rclSizePos.yBottom ||
        swpOld.cx != ( adj.rclSizePos.xRight - adj.rclSizePos.xLeft ) ||
        swpOld.cy != ( adj.rclSizePos.yTop - adj.rclSizePos.yBottom ) )
    {
        WinSetWindowPos( hwnd, HWND_TOP,
                         adj.rclSizePos.xLeft, adj.rclSizePos.yBottom,
                         adj.rclSizePos.xRight - adj.rclSizePos.xLeft,
                         adj.rclSizePos.yTop - adj.rclSizePos.yBottom,
                         SWP_MOVE | SWP_SIZE | SWP_ZORDER );
        changed = TRUE;
    }
    else
    {
        changed = FALSE;
    }

    LayoutToolBar( hwnd );
    return changed;
}

/* ------------------------------------------------------------------ */
/* State changes (dock / float / minimise)                            */
/* ------------------------------------------------------------------ */

/* Change to the state flNew, notify the owner with 'code' (0 for none),
 * let it adjust the suggestion and apply the result. */
static void ChangeState( HWND hwnd, XPMTOOLDATA *p, ULONG flNew, USHORT code )
{
    XpmTOOLADJ adj;
    XpmTOOLADJ calc;

    memset( &adj, 0, sizeof( adj ) );
    adj.hwndToolBar = hwnd;
    adj.hwndParent  = WinQueryWindow( hwnd, QW_PARENT );
    adj.flAttrs     = p->flToolAttrs;
    adj.flState     = p->flState;               /* previous state */

    memset( &calc, 0, sizeof( calc ) );
    calc.flAttrs = p->flToolAttrs;
    calc.flState = flNew;
    CalcNewRect( hwnd, p, &calc );
    adj.rclSizePos = calc.rclSizePos;

    if( code )
        NotifyOwner( hwnd, code, (ULONG)&adj );

    if( adj.hwndParent && adj.hwndParent != WinQueryWindow( hwnd, QW_PARENT ) )
        WinSetParent( hwnd, adj.hwndParent, TRUE );
    if( adj.flAttrs )
        p->flToolAttrs = (USHORT)adj.flAttrs;

    p->flState = flNew;

    if( flNew & XpmTST_MINIMIZED )
        HideObjects( p );
    else
        ShowObjects( p, TRUE );

    WinSetWindowPos( hwnd, HWND_TOP,
                     adj.rclSizePos.xLeft, adj.rclSizePos.yBottom,
                     adj.rclSizePos.xRight - adj.rclSizePos.xLeft,
                     adj.rclSizePos.yTop - adj.rclSizePos.yBottom,
                     SWP_MOVE | SWP_SIZE );

    LayoutToolBar( hwnd );
    WinInvalidateRect( hwnd, NULL, FALSE );
}

/* The real desktop-window handle.  HWND_DESKTOP is only a pseudo-handle
 * (1); a window's reported parent is the real desktop window, so tests
 * such as "am I floating?" must compare against this. */
static HWND DesktopWindow( HWND hwnd )
{
    return WinQueryDesktopWindow( WinQueryAnchorBlock( hwnd ), NULLHANDLE );
}

/* Dock the toolbar: re-parent back to the saved parent and stretch it
 * across the selected edge. */
static void DoDock( HWND hwnd, XPMTOOLDATA *p )
{
    HWND hwndCur = WinQueryWindow( hwnd, QW_PARENT );

    if( ( p->flState & XpmTST_FIXED ) && !IsMinimized( p ) )
        return;

    if( hwndCur == DesktopWindow( hwnd ) && p->hwndDockParent )
    {
        RECTL  rcl;
        POINTL ptl[2];

        WinQueryWindowRect( hwnd, &rcl );
        ptl[0].x = rcl.xLeft;  ptl[0].y = rcl.yBottom;
        ptl[1].x = rcl.xRight; ptl[1].y = rcl.yTop;
        WinMapWindowPoints( HWND_DESKTOP, p->hwndDockParent, ptl, 2 );
        WinSetParent( hwnd, p->hwndDockParent, TRUE );
        WinSetWindowPos( hwnd, HWND_TOP, ptl[0].x, ptl[0].y,
                         ptl[1].x - ptl[0].x, ptl[1].y - ptl[0].y,
                         SWP_MOVE | SWP_SIZE );
    }

    ChangeState( hwnd, p, ( p->flState | XpmTST_FIXED ) & ~XpmTST_MINIMIZED,
                 XpmTN_FIXTOOL );
}

/* Undock the toolbar: re-parent it to the desktop (keeping its screen
 * position) and switch to the floating state. */
static void DoFloat( HWND hwnd, XPMTOOLDATA *p )
{
    HWND hwndCur = WinQueryWindow( hwnd, QW_PARENT );

    if( !( p->flState & ( XpmTST_FIXED | XpmTST_MINIMIZED ) ) )
        return;                                 /* already floating */

    if( hwndCur != DesktopWindow( hwnd ) )
    {
        RECTL  rcl;
        POINTL ptl[2];

        p->hwndDockParent = hwndCur;

        WinQueryWindowRect( hwnd, &rcl );
        ptl[0].x = rcl.xLeft;  ptl[0].y = rcl.yBottom;
        ptl[1].x = rcl.xRight; ptl[1].y = rcl.yTop;
        WinMapWindowPoints( hwndCur, HWND_DESKTOP, ptl, 2 );
        WinSetParent( hwnd, HWND_DESKTOP, TRUE );
        WinSetWindowPos( hwnd, HWND_TOP, ptl[0].x, ptl[0].y,
                         ptl[1].x - ptl[0].x, ptl[1].y - ptl[0].y,
                         SWP_MOVE | SWP_SIZE );
    }

    ChangeState( hwnd, p, p->flState & ~( XpmTST_FIXED | XpmTST_MINIMIZED ),
                 XpmTN_RESTORE );
}

static void DoMinimize( HWND hwnd, XPMTOOLDATA *p )
{
    if( IsMinimized( p ) )
        return;

    if( !( p->flState & XpmTST_FIXED ) )
        DoDock( hwnd, p );

    ChangeState( hwnd, p, p->flState | XpmTST_MINIMIZED, XpmTN_MINIMIZE );
}

static void DoUnminimize( HWND hwnd, XPMTOOLDATA *p )
{
    if( !IsMinimized( p ) )
        return;

    ChangeState( hwnd, p, p->flState & ~XpmTST_MINIMIZED, XpmTN_RESTORE );
}

/* ------------------------------------------------------------------ */
/* Drag tracking                                                      */
/* ------------------------------------------------------------------ */

static void TrackStart( HWND hwnd, XPMTOOLDATA *p )
{
    XpmTOOLTRACK tr;
    HWND         hwndParent = WinQueryWindow( hwnd, QW_PARENT );
    SWP          swp;
    RECTL        brcl;

    memset( &tr, 0, sizeof( tr ) );
    tr.hwndToolBar = hwnd;
    tr.hwndParent  = hwndParent;

    WinQueryWindowPos( hwnd, &swp );
    tr.rclSizePos.xLeft   = swp.x;
    tr.rclSizePos.yBottom = swp.y;
    tr.rclSizePos.xRight  = swp.x + swp.cx;
    tr.rclSizePos.yTop    = swp.y + swp.cy;

    WinQueryWindowRect( hwndParent, &brcl );
    tr.rclBoundary = brcl;
    tr.flAttrs     = p->flToolAttrs;
    tr.flState     = p->flState;

    NotifyOwner( hwnd, XpmTN_BEGINTRACK, (ULONG)&tr );

    p->rclTrackBound = tr.rclBoundary;
    WinQueryPointerPos( HWND_DESKTOP, &p->ptlTrack );
    p->bTracking = TRUE;
}

static void TrackMove( HWND hwnd, XPMTOOLDATA *p )
{
    POINTL ptl;
    SWP    swp;
    LONG   nx, ny, minX, minY, maxX, maxY;
    POINTL rel;

    WinQueryPointerPos( HWND_DESKTOP, &ptl );
    WinQueryWindowPos( hwnd, &swp );

    nx = swp.x + ( ptl.x - p->ptlTrack.x );
    ny = swp.y + ( ptl.y - p->ptlTrack.y );

    minX = p->rclTrackBound.xLeft;
    minY = p->rclTrackBound.yBottom;
    maxX = p->rclTrackBound.xRight - swp.cx;
    maxY = p->rclTrackBound.yTop - swp.cy;
    if( maxX < minX )
        maxX = minX;
    if( maxY < minY )
        maxY = minY;

    if( nx < minX ) nx = minX;
    if( nx > maxX ) nx = maxX;
    if( ny < minY ) ny = minY;
    if( ny > maxY ) ny = maxY;

    p->ptlTrack = ptl;

    WinSetWindowPos( hwnd, HWND_TOP, nx, ny, swp.cx, swp.cy, SWP_MOVE );

    rel.x = nx;
    rel.y = ny;
    NotifyOwner( hwnd, XpmTN_TRACKING, (ULONG)&rel );
}

static void TrackEnd( HWND hwnd, XPMTOOLDATA *p )
{
    XpmTOOLTRACK tr;
    SWP          swp;

    p->bTracking = FALSE;

    memset( &tr, 0, sizeof( tr ) );
    tr.hwndToolBar = hwnd;
    tr.hwndParent  = WinQueryWindow( hwnd, QW_PARENT );

    WinQueryWindowPos( hwnd, &swp );
    tr.rclSizePos.xLeft   = swp.x;
    tr.rclSizePos.yBottom = swp.y;
    tr.rclSizePos.xRight  = swp.x + swp.cx;
    tr.rclSizePos.yTop    = swp.y + swp.cy;
    tr.rclBoundary = p->rclTrackBound;
    tr.flAttrs     = p->flToolAttrs;
    tr.flState     = p->flState;

    NotifyOwner( hwnd, XpmTN_ENDTRACK, (ULONG)&tr );

    WinSetWindowPos( hwnd, HWND_TOP,
                     tr.rclSizePos.xLeft, tr.rclSizePos.yBottom,
                     tr.rclSizePos.xRight - tr.rclSizePos.xLeft,
                     tr.rclSizePos.yTop - tr.rclSizePos.yBottom,
                     SWP_MOVE | SWP_SIZE );

    LayoutToolBar( hwnd );
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

static void DrawSeparator( HPS hps, XPMTOBJ *o, BOOL vertical, LONG clrLight,
                           LONG clrDark )
{
    ULONG style = o->flAttribute & XpmSPS_SEPARATORS;
    LONG  mid;

    if( style == XpmSPS_FLAT )
        return;

    if( vertical )
    {
        mid = ( o->rcl.yTop + o->rcl.yBottom ) / 2;

        if( style == XpmSPS_LINE )
        {
            DrawLine( hps, o->rcl.xLeft, mid, o->rcl.xRight - 1, mid, clrDark );
        }
        else
        {
            DrawLine( hps, o->rcl.xLeft, mid, o->rcl.xRight - 1, mid,
                      ( style == XpmSPS_CONVEX ) ? clrLight : clrDark );
            DrawLine( hps, o->rcl.xLeft, mid - 1, o->rcl.xRight - 1, mid - 1,
                      ( style == XpmSPS_CONVEX ) ? clrDark : clrLight );
        }
    }
    else
    {
        mid = ( o->rcl.xLeft + o->rcl.xRight ) / 2;

        if( style == XpmSPS_LINE )
        {
            DrawLine( hps, mid, o->rcl.yBottom, mid, o->rcl.yTop - 1, clrDark );
        }
        else
        {
            DrawLine( hps, mid, o->rcl.yBottom, mid, o->rcl.yTop - 1,
                      ( style == XpmSPS_CONVEX ) ? clrLight : clrDark );
            DrawLine( hps, mid - 1, o->rcl.yBottom, mid - 1, o->rcl.yTop - 1,
                      ( style == XpmSPS_CONVEX ) ? clrDark : clrLight );
        }
    }
}

static void DrawTriangle( HPS hps, LONG cx, LONG cy, LONG s, BOOL up )
{
    POINTL p;

    GpiSetColor( hps, CLR_BLACK );
    if( up )
    {
        p.x = cx - s; p.y = cy - s; GpiMove( hps, &p );
        p.x = cx + s; p.y = cy - s; GpiLine( hps, &p );
        p.x = cx;     p.y = cy + s; GpiLine( hps, &p );
        p.x = cx - s; p.y = cy - s; GpiLine( hps, &p );
    }
    else
    {
        p.x = cx - s; p.y = cy + s; GpiMove( hps, &p );
        p.x = cx + s; p.y = cy + s; GpiLine( hps, &p );
        p.x = cx;     p.y = cy - s; GpiLine( hps, &p );
        p.x = cx - s; p.y = cy + s; GpiLine( hps, &p );
    }
}

/* A small "rotate" glyph: an open circle with an arrow head. */
static void DrawRotateGlyph( HPS hps, LONG cx, LONG cy )
{
    static const POINTL aUnit[12] =
    {
        {  5,  0 }, {  4,  3 }, {  3,  4 }, {  0,  5 }, { -3,  4 }, { -4,  3 },
        { -5,  0 }, { -4, -3 }, { -3, -4 }, {  0, -5 }, {  3, -4 }, {  4, -3 }
    };
    POINTL aptl[12];
    POINTL p;
    LONG   i;

    for( i = 0; i < 12; i++ )
    {
        aptl[i].x = cx + aUnit[i].x;
        aptl[i].y = cy + aUnit[i].y;
    }

    GpiSetColor( hps, CLR_BLACK );
    GpiMove( hps, &aptl[2] );
    GpiPolyLine( hps, 9, &aptl[3] );

    /* Arrow head at the top-right end of the arc. */
    p.x = aptl[2].x + 2; p.y = aptl[2].y + 3; GpiMove( hps, &p );
    p.x = aptl[2].x + 5; p.y = aptl[2].y - 1; GpiLine( hps, &p );
    p.x = aptl[2].x;     p.y = aptl[2].y + 5; GpiLine( hps, &p );
    p.x = aptl[2].x + 2; p.y = aptl[2].y + 3; GpiLine( hps, &p );
}

static void DrawToolBar( HWND hwnd, HPS hps )
{
    XPMTOOLDATA *p = ToolData( hwnd );
    RECTL        rcl;
    LONG         bg, clrLight, clrDark;
    LONG         border;
    LONG         i;

    if( !p )
        return;

    WinQueryWindowRect( hwnd, &rcl );

    /* A PS from WinBeginPaint is in colour index mode; we draw with RGB
     * values, so select RGB mode explicitly. */
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );

    bg       = QueryColor( hwnd, PP_BACKGROUNDCOLOR,
                           (ULONG)SYSCLR_BUTTONMIDDLE, 0x00C0C0C0 );
    clrLight = QueryColor( hwnd, PP_BORDERLIGHTCOLOR,
                           (ULONG)SYSCLR_BUTTONLIGHT, 0x00FFFFFF );
    clrDark  = QueryColor( hwnd, PP_BORDERDARKCOLOR,
                           (ULONG)SYSCLR_BUTTONDARK, 0x00808080 );

    WinFillRect( hps, &rcl, bg );

    border = BorderW( p );
    for( i = 0; i < border; i++ )
    {
        if( p->flToolAttrs & XpmTBA_TOPFRAME )
            DrawLine( hps, rcl.xLeft + i, rcl.yTop - 1 - i,
                      rcl.xRight - 1 - i, rcl.yTop - 1 - i, clrLight );
        if( p->flToolAttrs & XpmTBA_LEFTFRAME )
            DrawLine( hps, rcl.xLeft + i, rcl.yBottom + i,
                      rcl.xLeft + i, rcl.yTop - 1 - i, clrLight );
        if( p->flToolAttrs & XpmTBA_BOTTOMFRAME )
            DrawLine( hps, rcl.xLeft + i, rcl.yBottom + i,
                      rcl.xRight - 1 - i, rcl.yBottom + i, clrDark );
        if( p->flToolAttrs & XpmTBA_RIGHTFRAME )
            DrawLine( hps, rcl.xRight - 1 - i, rcl.yBottom + i,
                      rcl.xRight - 1 - i, rcl.yTop - 1 - i, clrDark );
    }

    if( !IsMinimized( p ) )
    {
        for( i = 0; i < p->cObj; i++ )
        {
            if( ( p->pObj[i]->flAttribute & XpmTBO_BASICTYPES ) ==
                XpmTBO_SEPARATOR )
                DrawSeparator( hps, p->pObj[i], IsVertical( p ),
                               clrLight, clrDark );
        }
    }
}

static void DrawTitle( HWND hwndTitle, HPS hps )
{
    XPMTITLE    *t = TitleData( hwndTitle );
    XPMTOOLDATA *p = t ? ToolData( t->hwndToolBar ) : NULL;
    RECTL        rcl;
    LONG         bg, act;
    BOOL         vertical;

    WinQueryWindowRect( hwndTitle, &rcl );
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );

    bg  = QueryColor( hwndTitle, PP_BACKGROUNDCOLOR,
                      (ULONG)SYSCLR_BUTTONMIDDLE, 0x00C0C0C0 );
    act = QueryColor( hwndTitle, PP_ACTIVECOLOR, 0, 0x00C7D5FF );

    WinFillRect( hps, &rcl,
                 ( t && ( t->bHilite || t->bDown ) ) ? act : bg );

    if( p && ( p->flStyle & XpmTBS_FIXBUTTON ) )
    {
        BOOL floating = !( p->flState & ( XpmTST_FIXED | XpmTST_MINIMIZED ) );

        vertical = IsVertical( p );
        if( vertical )
            DrawTriangle( hps, rcl.xLeft + TBM_TITLEBTN / 2,
                          rcl.yTop - TBM_TITLE / 2, 4, floating );
        else
            DrawTriangle( hps, rcl.xLeft + TBM_TITLE / 2,
                          rcl.yTop - TBM_TITLEBTN / 2, 4, floating );
    }
}

static void DrawRotate( HWND hwndRotate, HPS hps )
{
    XPMROTATE *r = RotateData( hwndRotate );
    RECTL      rcl;
    LONG       bg, act;

    WinQueryWindowRect( hwndRotate, &rcl );
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );

    bg  = QueryColor( hwndRotate, PP_BACKGROUNDCOLOR,
                      (ULONG)SYSCLR_BUTTONMIDDLE, 0x00C0C0C0 );
    act = QueryColor( hwndRotate, PP_ACTIVECOLOR, 0, 0x00C7D5FF );

    WinFillRect( hps, &rcl, ( r && r->bHilite ) ? act : bg );

    DrawRotateGlyph( hps, ( rcl.xLeft + rcl.xRight ) / 2,
                          ( rcl.yBottom + rcl.yTop ) / 2 );
}

static void DrawTip( HWND hwndTip, HPS hps )
{
    RECTL rcl;
    LONG  bg, fg, fr;
    LONG  len;
    CHAR  szText[TBM_TIPMAX + 1];
    CHAR  szFont[64];
    POINTL ptl;
    BOOL  bFont = FALSE;
    FONTMETRICS fm;

    WinQueryWindowRect( hwndTip, &rcl );
    GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );

    bg = QueryColor( hwndTip, XpmPP_TOOLTIPBACKGROUND, 0, 0x00FFFFC8 );
    fg = QueryColor( hwndTip, XpmPP_TOOLTIPTEXT,       0, 0x00000000 );
    fr = QueryColor( hwndTip, XpmPP_TOOLTIPFRAME,      0, 0x00000000 );

    WinFillRect( hps, &rcl, bg );

    GpiSetColor( hps, fr );
    ptl.x = rcl.xLeft;      ptl.y = rcl.yBottom; GpiMove( hps, &ptl );
    ptl.x = rcl.xRight - 1; ptl.y = rcl.yTop - 1; GpiBox( hps, DRO_OUTLINE,
                                                          &ptl, 0, 0 );

    len = WinQueryWindowText( hwndTip, sizeof( szText ), szText );
    if( len > 0 )
    {
        ptl.x = rcl.xLeft + 3;
        ptl.y = rcl.yBottom + 3;

        if( WinQueryPresParam( hwndTip, PP_FONTNAMESIZE, 0, NULL,
                               sizeof( szFont ), szFont, 0 ) > 0 )
            bFont = ( XpmCreateRasterFont( hps, szFont, 1, &fm ) == 0 );

        GpiSetColor( hps, fg );
        if( bFont )
            GpiSetCharSet( hps, 1 );

        GpiCharStringAt( hps, &ptl, len, szText );

        if( bFont )
            GpiDeleteSetId( hps, 1 );
    }
}

/* ------------------------------------------------------------------ */
/* Tool-tip display                                                   */
/* ------------------------------------------------------------------ */

static void HideTip( XPMTOOLDATA *p )
{
    if( p->hwndTip && p->bTipOn )
    {
        WinShowWindow( p->hwndTip, FALSE );
        p->bTipOn = FALSE;
    }
}

static void ShowTip( HWND hwnd, XPMTOOLDATA *p, XPMTOBJ *o )
{
    POINTL aptl[TXTBOX_COUNT];
    POINTL ptl;
    HPS    hps;
    LONG   w, h;

    if( !p->hwndTip )
        return;

    hps = WinGetPS( p->hwndTip );
    if( hps )
    {
        GpiQueryTextBox( hps, (LONG)strlen( o->szTip ), o->szTip,
                         TXTBOX_COUNT, aptl );
        w = aptl[TXTBOX_TOPRIGHT].x    - aptl[TXTBOX_BOTTOMLEFT].x;
        h = aptl[TXTBOX_TOPLEFT].y     - aptl[TXTBOX_BOTTOMLEFT].y;
        WinReleasePS( hps );
    }
    else
    {
        w = (LONG)strlen( o->szTip ) * 8;
        h = 16;
    }

    if( w < 8 ) w = 8;
    if( h < 8 ) h = 8;
    w += 8;
    h += 6;

    ptl.x = o->rcl.xLeft;
    ptl.y = o->rcl.yBottom;
    WinMapWindowPoints( hwnd, HWND_DESKTOP, &ptl, 1 );
    ptl.y -= h;

    WinSetWindowText( p->hwndTip, o->szTip );
    WinSetWindowPos( p->hwndTip, HWND_TOP, ptl.x, ptl.y, w, h,
                     SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER );
    WinInvalidateRect( p->hwndTip, NULL, FALSE );
    p->bTipOn = TRUE;
}

static void ToolTipTick( HWND hwnd, XPMTOOLDATA *p )
{
    POINTL ptl;
    LONG   i, found = -1;

    WinQueryPointerPos( HWND_DESKTOP, &ptl );
    WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1 );

    for( i = 0; i < p->cObj; i++ )
    {
        XPMTOBJ *o = p->pObj[i];

        if( !o->hwnd || o->szTip[0] == '\0' )
            continue;
        if( !WinIsWindowVisible( o->hwnd ) )
            continue;
        if( ptl.x >= o->rcl.xLeft && ptl.x < o->rcl.xRight &&
            ptl.y >= o->rcl.yBottom && ptl.y < o->rcl.yTop )
        {
            found = i;
            break;
        }
    }

    if( found < 0 )
    {
        p->idTipObj  = 0;
        p->cTipTicks = 0;
        HideTip( p );
        return;
    }

    if( p->idTipObj != p->pObj[found]->id )
    {
        p->idTipObj  = p->pObj[found]->id;
        p->cTipTicks = 0;
        HideTip( p );
    }

    p->cTipTicks++;
    if( !p->bTipOn &&
        (LONG)p->cTipTicks * TBM_TIPPOLL >= p->lTipDelay )
        ShowTip( hwnd, p, p->pObj[found] );
}

/* ------------------------------------------------------------------ */
/* Internal child-window procedures                                   */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY TitleProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                   MPARAM mp2 )
{
    XPMTITLE *t = TitleData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            t = (XPMTITLE *)malloc( sizeof( XPMTITLE ) );
            if( !t )
                return (MRESULT)TRUE;

            memset( t, 0, sizeof( XPMTITLE ) );
            t->hwndToolBar = WinQueryWindow( hwnd, QW_PARENT );
            WinSetWindowULong( hwnd, QWL_USER, (ULONG)t );
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( t )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( t );
            }
            return (MRESULT)FALSE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawTitle( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_TIMER:
            if( t )
            {
                POINTL ptl;
                RECTL  rcl;

                WinQueryPointerPos( HWND_DESKTOP, &ptl );
                WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1 );
                WinQueryWindowRect( hwnd, &rcl );

                if( ptl.x < rcl.xLeft || ptl.x >= rcl.xRight ||
                    ptl.y < rcl.yBottom || ptl.y >= rcl.yTop )
                {
                    WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                                  TBM_TIMER_HILITE );
                    if( t->bHilite )
                    {
                        t->bHilite = FALSE;
                        WinInvalidateRect( hwnd, NULL, FALSE );
                    }
                }
            }
            return (MRESULT)FALSE;

        case WM_MOUSEMOVE:
            if( t )
            {
                if( t->bDown && !t->bDragging )
                {
                    XPMTOOLDATA *p = ToolData( t->hwndToolBar );
                    POINTL       ptl;
                    LONG         dx, dy;

                    if( p && CanDrag( p ) )
                    {
                        WinQueryPointerPos( HWND_DESKTOP, &ptl );
                        dx = ptl.x - p->ptlTrack.x;
                        dy = ptl.y - p->ptlTrack.y;
                        if( dx < 0 ) dx = -dx;
                        if( dy < 0 ) dy = -dy;
                        if( dx > 3 || dy > 3 )
                        {
                            t->bDragging = TRUE;
                            TrackStart( t->hwndToolBar, p );
                        }
                    }
                }
                else if( t->bDown && t->bDragging )
                {
                    XPMTOOLDATA *p = ToolData( t->hwndToolBar );

                    if( p )
                        TrackMove( t->hwndToolBar, p );
                }
                else if( !t->bHilite )
                {
                    t->bHilite = TRUE;
                    WinInvalidateRect( hwnd, NULL, FALSE );
                    WinStartTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                                   TBM_TIMER_HILITE, 250 );
                }
            }
            return (MRESULT)FALSE;

        case WM_BUTTON1DOWN:
            if( t )
            {
                XPMTOOLDATA *p = ToolData( t->hwndToolBar );

                t->bDown     = TRUE;
                t->bDragging = FALSE;
                if( p )
                    WinQueryPointerPos( HWND_DESKTOP, &p->ptlTrack );
                WinSetCapture( HWND_DESKTOP, hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)FALSE;

        case WM_BUTTON1UP:
            if( t )
            {
                XPMTOOLDATA *p = ToolData( t->hwndToolBar );
                BOOL         bDrag = t->bDragging;

                t->bDown     = FALSE;
                t->bDragging = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );
                WinInvalidateRect( hwnd, NULL, FALSE );

                if( p && bDrag )
                {
                    TrackEnd( t->hwndToolBar, p );
                }
                else if( p )
                {
                    RECTL rcl;
                    POINTL ptl;
                    BOOL  bFix = FALSE;

                    WinQueryWindowRect( hwnd, &rcl );
                    WinQueryPointerPos( HWND_DESKTOP, &ptl );
                    WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1 );

                    if( p->flStyle & XpmTBS_FIXBUTTON )
                    {
                        if( IsVertical( p ) )
                            bFix = ( ptl.x < rcl.xLeft + TBM_TITLEBTN + 4 );
                        else
                            bFix = ( ptl.y > rcl.yTop - TBM_TITLEBTN - 4 );

                        if( bFix )
                        {
                            if( p->flState & ( XpmTST_FIXED | XpmTST_MINIMIZED ) )
                            {
                                if( IsMinimized( p ) )
                                    DoUnminimize( t->hwndToolBar, p );
                                DoFloat( t->hwndToolBar, p );
                            }
                            else
                            {
                                DoDock( t->hwndToolBar, p );
                            }
                        }
                    }

                    if( !bFix && ( p->flStyle & XpmTBS_ENABLEMINIMIZE ) )
                    {
                        if( IsMinimized( p ) )
                            DoUnminimize( t->hwndToolBar, p );
                        else
                            DoMinimize( t->hwndToolBar, p );
                    }
                }
            }
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

static MRESULT EXPENTRY RotateProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    XPMROTATE *r = RotateData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            r = (XPMROTATE *)malloc( sizeof( XPMROTATE ) );
            if( !r )
                return (MRESULT)TRUE;

            memset( r, 0, sizeof( XPMROTATE ) );
            r->hwndToolBar = WinQueryWindow( hwnd, QW_PARENT );
            WinSetWindowULong( hwnd, QWL_USER, (ULONG)r );
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
            if( r )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( r );
            }
            return (MRESULT)FALSE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawRotate( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_MOUSEMOVE:
            if( r && !r->bHilite )
            {
                r->bHilite = TRUE;
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)FALSE;

        case WM_BUTTON1DOWN:
            return (MRESULT)FALSE;

        case WM_BUTTON1UP:
            if( r )
            {
                r->bHilite = FALSE;
                WinInvalidateRect( hwnd, NULL, FALSE );
                WinSendMsg( r->hwndToolBar, XpmTM_ROTATE,
                            MPFROMLONG( XpmTBR_ROTATE ), 0 );
                WinSendMsg( r->hwndToolBar, XpmTM_AUTOSIZE,
                            MPFROMLONG( 0 ), NULL );
            }
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

static MRESULT EXPENTRY TipProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                 MPARAM mp2 )
{
    switch( msg )
    {
        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawTip( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */
/* Window procedure                                                   */
/* ------------------------------------------------------------------ */

static void CreateControls( HWND hwnd, XPMTOOLDATA *p )
{
    if( HasTitle( p ) && !p->hwndTitle )
    {
        p->hwndTitle = WinCreateWindow( hwnd, XpmToolBarTitleClass, "",
                                        WS_VISIBLE, 0, 0, TBM_TITLE, TBM_TITLE,
                                        hwnd, HWND_TOP,
                                        (ULONG)XpmTCID_TITLEBAR, NULL, NULL );
    }

    if( HasRotate( p ) && !p->hwndRotate )
    {
        p->hwndRotate = WinCreateWindow( hwnd, XpmToolBarRotateClass, "",
                                         WS_VISIBLE, 0, 0, TBM_ROTATE,
                                         TBM_ROTATE, hwnd, HWND_TOP,
                                         (ULONG)XpmTCID_ROTATE, NULL, NULL );
    }

    if( !p->hwndTip )
    {
        /* A top-level (desktop-parented) window owned by the toolbar:
         * PM has no WS_POPUP style. */
        p->hwndTip = WinCreateWindow( HWND_DESKTOP, XpmToolBarTipClass, "",
                                      WS_CLIPSIBLINGS,
                                      0, 0, 10, 10, hwnd, HWND_TOP, 0,
                                      NULL, NULL );
    }
}

static MRESULT EXPENTRY ToolBarProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                     MPARAM mp2 )
{
    XPMTOOLDATA *p = ToolData( hwnd );

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)PVOIDFROMMP( mp2 );

            p = (XPMTOOLDATA *)malloc( sizeof( XPMTOOLDATA ) );
            if( !p )
                return (MRESULT)TRUE;

            memset( p, 0, sizeof( XPMTOOLDATA ) );

            p->flStyle = pcs ? pcs->flStyle
                             : WinQueryWindowULong( hwnd, QWL_STYLE );

            if( pcs && pcs->pCtlData )
            {
                PXpmTOOLINFO pti = (PXpmTOOLINFO)pcs->pCtlData;

                p->flToolAttrs = pti->flToolAttrs;
                p->usMaxLength = pti->usMaxLength;
            }

            p->flState = XpmTST_FIXED;
            if( p->flToolAttrs & XpmTBA_VERTICAL )
                p->flState |= XpmTST_ROTATED;

            p->lTipDelay = 1000;

            if( pcs && pcs->pPresParams )
                XpmStorePresParams( hwnd, pcs->pPresParams, NULL, 0 );

            WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );

            /* Keep child objects from being painted over. */
            WinSetWindowBits( hwnd, QWL_STYLE, WS_CLIPCHILDREN,
                              WS_CLIPCHILDREN );

            p->flToolAttrs = (USHORT)( p->flToolAttrs );

            CreateControls( hwnd, p );
            p->lTipDelay = (LONG)QueryULongPP( hwnd, XpmPP_TOOLTIPDELAY, 1000 );

            p->bReady = TRUE;
            return (MRESULT)FALSE;
        }

        case WM_DESTROY:
        {
            LONG i;

            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );

                if( p->idTipTimer )
                    WinStopTimer( WinQueryAnchorBlock( hwnd ), hwnd,
                                  TBM_TIMER_TIP );
                if( p->hwndTip )
                    WinDestroyWindow( p->hwndTip );

                for( i = 0; i < p->cObj; i++ )
                    free( p->pObj[i] );
                if( p->pObj )
                    free( p->pObj );
                free( p );
            }
            return (MRESULT)FALSE;
        }

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawToolBar( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_ERASEBACKGROUND:
            return (MRESULT)TRUE;

        case WM_SIZE:
            LayoutToolBar( hwnd );
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_SYSCOLORCHANGE:
        case WM_PRESPARAMCHANGED:
            WinInvalidateRect( hwnd, NULL, FALSE );
            if( p && p->hwndTitle )
                WinInvalidateRect( p->hwndTitle, NULL, FALSE );
            if( p && p->hwndRotate )
                WinInvalidateRect( p->hwndRotate, NULL, FALSE );
            LayoutToolBar( hwnd );
            return (MRESULT)FALSE;

        case WM_TIMER:
            if( p && (ULONG)SHORT1FROMMP( mp1 ) == TBM_TIMER_TIP )
            {
                ToolTipTick( hwnd, p );
                return (MRESULT)FALSE;
            }
            break;

        case WM_ADJUSTWINDOWPOS:
        {
            PSWP pswp = (PSWP)PVOIDFROMMP( mp1 );

            if( p && pswp )
            {
                if( pswp->fl & SWP_MINIMIZE )
                {
                    pswp->fl = 0;
                    DoMinimize( hwnd, p );
                }
                else if( pswp->fl & SWP_MAXIMIZE )
                {
                    pswp->fl = 0;
                    DoDock( hwnd, p );
                }
                else if( pswp->fl & SWP_RESTORE )
                {
                    pswp->fl = 0;
                    if( IsMinimized( p ) )
                        DoUnminimize( hwnd, p );
                    else
                        DoFloat( hwnd, p );
                }
            }
            return (MRESULT)FALSE;
        }

        case XpmTM_QUERYSTATE:
            return (MRESULT)( p ? p->flState : XpmTST_FIXED );

        case XpmTM_QUERYOBJECTCOUNT:
            return (MRESULT)( p ? p->cObj : 0 );

        case XpmTM_QUERYHEIGHT:
        {
            ULONG opt    = (ULONG)LONGFROMMP( mp1 );
            LONG  maxlen = (LONG)LONGFROMMP( mp2 );
            LONG  h;

            if( !p )
                return (MRESULT)0L;

            if( opt == XpmTDQ_MINIMIZED ||
                ( opt == XpmTDQ_CURRENT && IsMinimized( p ) ) )
                h = TBM_TITLE + 2 * BorderW( p );
            else
                h = IsVertical( p ) ? NatLength( p, TRUE, FALSE )
                                    : NatCross( p, FALSE, FALSE );

            if( maxlen > 0 && h > maxlen )
                h = maxlen;
            return (MRESULT)h;
        }

        case XpmTM_QUERYWIDTH:
        {
            ULONG opt    = (ULONG)LONGFROMMP( mp1 );
            LONG  maxlen = (LONG)LONGFROMMP( mp2 );
            LONG  w;

            if( !p )
                return (MRESULT)0L;

            if( opt == XpmTDQ_MINIMIZED ||
                ( opt == XpmTDQ_CURRENT && IsMinimized( p ) ) )
                w = TBM_TITLE + 2 * BorderW( p );
            else
                w = IsVertical( p ) ? NatCross( p, TRUE, FALSE )
                                    : NatLength( p, FALSE, FALSE );

            if( maxlen > 0 && w > maxlen )
                w = maxlen;
            return (MRESULT)w;
        }

        case XpmTM_AUTOSIZE:
            return (MRESULT)( AutoSizeToolBar( hwnd, p, (LONG)LONGFROMMP( mp1 ) )
                              ? TRUE : FALSE );

        case XpmTM_CALCNEWRECT:
            if( p && mp1 )
            {
                PXpmTOOLADJ padj = (PXpmTOOLADJ)PVOIDFROMMP( mp1 );

                CalcNewRect( hwnd, p, padj );
            }
            return (MRESULT)0L;

        case XpmTM_ENABLEROTATE:
            if( p )
            {
                p->bRotateEnabled = mp1 ? TRUE : FALSE;
                if( p->bRotateEnabled && !p->hwndRotate )
                    CreateControls( hwnd, p );
                else if( !p->bRotateEnabled && p->hwndRotate &&
                         !( p->flStyle & XpmTBS_ROTATEBUTTON ) )
                {
                    WinDestroyWindow( p->hwndRotate );
                    p->hwndRotate = NULLHANDLE;
                }
                LayoutToolBar( hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
                if( p->hwndRotate )
                    WinInvalidateRect( p->hwndRotate, NULL, FALSE );
            }
            return (MRESULT)0L;

        case XpmTM_ROTATE:
        {
            ULONG opt;
            BOOL  vertical;

            if( !p )
                return (MRESULT)0L;

            opt      = (ULONG)LONGFROMMP( mp1 );
            vertical = IsVertical( p );

            if( opt == XpmTBR_HORIZONTAL )
                vertical = FALSE;
            else if( opt == XpmTBR_VERTICAL )
                vertical = TRUE;
            else
                vertical = !vertical;

            if( vertical )
                p->flState |= XpmTST_ROTATED;
            else
                p->flState &= ~XpmTST_ROTATED;

            LayoutToolBar( hwnd );
            WinInvalidateRect( hwnd, NULL, FALSE );
            NotifyOwner( hwnd, XpmTN_ROTATE, p->flState );
            return (MRESULT)0L;
        }

        case XpmTM_INSERTOBJECT:
            if( !p || !mp1 )
                return (MRESULT)FALSE;
            return (MRESULT)( InsertObject( hwnd, p,
                                            (PXpmTOOLOBJ)PVOIDFROMMP( mp1 ),
                                            (PXpmTOOLWIN)PVOIDFROMMP( mp2 ) )
                              ? TRUE : FALSE );

        case XpmTM_DELETEOBJECT:
            if( !p )
                return (MRESULT)FALSE;
            return (MRESULT)( DeleteObject( hwnd, p,
                                            (ULONG)LONGFROMMP( mp1 ) )
                              ? TRUE : FALSE );

        case XpmTM_MOVEOBJECT:
            if( !p )
                return (MRESULT)FALSE;
            return (MRESULT)( MoveObject( hwnd, p,
                                          (ULONG)SHORT1FROMMP( mp1 ),
                                          (ULONG)SHORT2FROMMP( mp1 ),
                                          (ULONG)LONGFROMMP( mp2 ) )
                              ? TRUE : FALSE );

        case XpmTM_QUERYOBJECT:
        {
            PXpmTOOLOBJ po = (PXpmTOOLOBJ)PVOIDFROMMP( mp2 );
            LONG        idx;
            XPMTOBJ    *o;

            if( !p || !po )
                return (MRESULT)FALSE;

            idx = FindById( p, (ULONG)LONGFROMMP( mp1 ) );
            if( idx < 0 )
                return (MRESULT)FALSE;

            o = p->pObj[idx];
            po->iPosition   = o->iPosition;
            po->ulGroup     = o->ulGroup;
            po->flAttribute = o->flAttribute;
            po->cx          = o->orgCx;
            po->cy          = o->orgCy;
            po->id          = o->id;
            po->ulUser      = o->ulUser;
            return (MRESULT)TRUE;
        }

        case XpmTM_QUERYHANDLE:
        {
            LONG idx = p ? FindById( p, (ULONG)LONGFROMMP( mp1 ) ) : -1;

            if( idx < 0 )
                return (MRESULT)NULLHANDLE;
            return (MRESULT)( p->pObj[idx]->hwnd );
        }

        case XpmTM_POSITIONFROMID:
        {
            LONG idx = p ? FindById( p, (ULONG)LONGFROMMP( mp1 ) ) : -1;

            return (MRESULT)( idx < 0 ? -1L : (LONG)idx );
        }

        case XpmTM_IDFROMPOSITION:
        {
            ULONG pos = (ULONG)LONGFROMMP( mp1 );

            if( !p || p->cObj <= 0 )
                return (MRESULT)0L;
            if( pos == XpmTB_END || pos >= (ULONG)p->cObj )
                pos = (ULONG)( p->cObj - 1 );
            return (MRESULT)( p->pObj[pos]->id );
        }

        case XpmTM_SETOBJECT:
        {
            ULONG       id   = (ULONG)SHORT1FROMMP( mp1 );
            ULONG       opts = (ULONG)SHORT2FROMMP( mp1 );
            PXpmTOOLOBJ po   = (PXpmTOOLOBJ)PVOIDFROMMP( mp2 );
            LONG        idx;
            XPMTOBJ    *o;

            if( !p || !po )
                return (MRESULT)FALSE;

            idx = FindById( p, id );
            if( idx < 0 )
                return (MRESULT)FALSE;

            o = p->pObj[idx];

            if( opts & XpmTSO_GROUP )
                o->ulGroup = po->ulGroup;

            if( opts & XpmTSO_SIZE )
            {
                ULONG basic = o->flAttribute & XpmTBO_BASICTYPES;

                if( basic == XpmTBO_STDWINDOW || basic == XpmTBO_STDBUTTON )
                {
                    o->orgCx = po->cx;
                    o->orgCy = po->cy;
                }
            }

            if( opts & XpmTSO_USER )
                o->ulUser = po->ulUser;

            LayoutToolBar( hwnd );
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)TRUE;
        }

        case XpmTM_SHOWOBJECT:
        {
            ULONG    id  = (ULONG)LONGFROMMP( mp1 );
            BOOL     vis = mp2 ? TRUE : FALSE;
            LONG     idx;
            XPMTOBJ *o;

            if( !p )
                return (MRESULT)FALSE;

            idx = FindById( p, id );
            if( idx < 0 )
                return (MRESULT)FALSE;

            o = p->pObj[idx];
            if( !o->hwnd )
                return (MRESULT)FALSE;

            if( vis )
                o->flAttribute &= ~XpmTBO_HIDE;
            else
                o->flAttribute |= XpmTBO_HIDE;

            if( !IsMinimized( p ) )
                WinShowWindow( o->hwnd, vis );
            return (MRESULT)TRUE;
        }

        case XpmTM_INSERTTOOLTIP:
        {
            ULONG    id  = (ULONG)LONGFROMMP( mp1 );
            PSZ      psz = (PSZ)PVOIDFROMMP( mp2 );
            LONG     idx;
            XPMTOBJ *o;

            if( !p )
                return (MRESULT)FALSE;

            idx = FindById( p, id );
            if( idx < 0 )
                return (MRESULT)FALSE;

            o = p->pObj[idx];
            if( !psz )
            {
                o->szTip[0] = '\0';
            }
            else
            {
                strncpy( o->szTip, psz, TBM_TIPMAX - 1 );
                o->szTip[TBM_TIPMAX - 1] = '\0';
            }

            UpdateTipTimer( hwnd, p );
            return (MRESULT)TRUE;
        }

        case XpmTM_QUERYTIP:
        {
            ULONG    id      = (ULONG)SHORT1FROMMP( mp1 );
            ULONG    bufsize = (ULONG)SHORT2FROMMP( mp1 );
            PSZ      psz     = (PSZ)PVOIDFROMMP( mp2 );
            LONG     idx;
            XPMTOBJ *o;
            ULONG    len;

            if( !p )
                return (MRESULT)0L;

            idx = FindById( p, id );
            if( idx < 0 )
                return (MRESULT)0L;

            o   = p->pObj[idx];
            len = (ULONG)strlen( o->szTip );

            if( psz && bufsize > 0 )
            {
                ULONG n = ( len < bufsize - 1 ) ? len : bufsize - 1;

                memcpy( psz, o->szTip, (size_t)n );
                psz[n] = '\0';
            }
            return (MRESULT)len;
        }

        case XpmTM_OBJECTCONTROL:
            return (MRESULT)0L;

        case WM_QUERYWINDOWPARAMS:
        {
            PWNDPARAMS pwp = (PWNDPARAMS)PVOIDFROMMP( mp1 );

            if( pwp && p && ( pwp->fsStatus & WPM_CBCTLDATA ) != 0 )
                pwp->cbCtlData = (ULONG)sizeof( XpmTOOLINFO );

            if( pwp && p && ( pwp->fsStatus & WPM_CTLDATA ) != 0 &&
                pwp->pCtlData )
            {
                PXpmTOOLINFO pti = (PXpmTOOLINFO)pwp->pCtlData;

                pti->cb          = (USHORT)sizeof( XpmTOOLINFO );
                pti->flToolAttrs = p->flToolAttrs;
                pti->usMaxLength = p->usMaxLength;
            }
            return (MRESULT)TRUE;
        }

        case WM_SETWINDOWPARAMS:
        {
            PWNDPARAMS pwp = (PWNDPARAMS)PVOIDFROMMP( mp1 );

            if( pwp && p && ( pwp->fsStatus & WPM_CTLDATA ) != 0 &&
                pwp->pCtlData )
            {
                PXpmTOOLINFO pti = (PXpmTOOLINFO)pwp->pCtlData;

                p->flToolAttrs = pti->flToolAttrs;
                p->usMaxLength = pti->usMaxLength;

                if( p->flToolAttrs & XpmTBA_VERTICAL )
                    p->flState |= XpmTST_ROTATED;
                else
                    p->flState &= ~XpmTST_ROTATED;

                LayoutToolBar( hwnd );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)TRUE;
        }
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */

LONG EXPENTRY XpmToolBarInit( HAB hab )
{
    if( g_bToolRegistered )
        return 0L;

    if( !WinRegisterClass( hab, XpmToolBarClass, ToolBarProc,
                           CS_SIZEREDRAW, sizeof( XPMTOOLDATA ) ) )
        return (LONG)XpmERR_PNL_CLASSREG;

    if( !WinRegisterClass( hab, XpmToolBarTitleClass, TitleProc,
                           CS_SIZEREDRAW, 0 ) )
        return (LONG)XpmERR_PNL_CLASSREG;

    if( !WinRegisterClass( hab, XpmToolBarRotateClass, RotateProc,
                           CS_SIZEREDRAW, 0 ) )
        return (LONG)XpmERR_PNL_CLASSREG;

    if( !WinRegisterClass( hab, XpmToolBarTipClass, TipProc,
                           CS_SIZEREDRAW, 0 ) )
        return (LONG)XpmERR_PNL_CLASSREG;

    g_bToolRegistered = TRUE;
    return 0L;
}

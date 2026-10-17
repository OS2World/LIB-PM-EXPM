/*
** Module   :CTLPLUS.C
** Abstract :Control Plus -- supplementary PM controls.
**
** Clean-room reimplementation for the EXPM project (2026) of the
** closed-source "Controls+" 1.40 library (CTLPLUS.DLL).  Written in C
** from the published Controls+ API (ctlplus.h) and the Controls+
 * manual.  Window classes, style, message and notification constants
** keep their upstream names; the Ctl* entry points are renamed to the
** Presentation Manager Win* style.  EXPM is LGPLv3 (see LICENSE); this
** file carries no third-party source.
*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "expm.h"

/*****************************************************************************
** Per-window control data.  The style field is first in every structure
** so that the general CTLM_QUERYSTYLE / CTLM_SETSTYLE messages can be
** handled generically.
*/

typedef struct
{
    ULONG style;          /* MTS_* */
    LONG  fgColor;
    LONG  lower;          /* inclusive */
    LONG  upper;          /* exclusive */
    LONG  value;
    BOOL  bCapture;
} METERDATA;

typedef struct
{
    ULONG style;          /* SWS_* */
    LONG  fgColor;
    BOOL  value;          /* TRUE = up ("on") */
    CHAR  upText[SW_MAXTEXT];
    CHAR  downText[SW_MAXTEXT];
} SWITCHDATA;

typedef struct
{
    ULONG style;          /* RCS_* */
    LONG  fgColor;
    LONG  cxMin;
    LONG  cyMin;
    LONG  cxMax;
    LONG  cyMax;
    LONG  handle;         /* opaque owner handle */
    RECTL rcl;            /* current rect, client coordinates */
    BOOL  bInit;
    int   dragMode;       /* 0 none, 1 move, 2 size */
    BOOL  bDragLeft;
    BOOL  bDragRight;
    BOOL  bDragTop;
    BOOL  bDragBottom;
    POINTL ptlStart;
    RECTL  rclStart;
} RECTDATA;

typedef struct
{
    ULONG style;          /* CTS_* */
    LONG  fgColor;
    LONG  bgColor;
    HWND  hwndNext;
    LONG  value;
    LONG  radix;
    LONG  maxValue;
} COUNTERDATA;

#define HANDLE_SIZE 6

/* ------------------------------------------------------------------ */

static METERDATA   *MeterData( HWND hwnd )
{
    return (METERDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static SWITCHDATA  *SwitchData( HWND hwnd )
{
    return (SWITCHDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static RECTDATA    *RectData( HWND hwnd )
{
    return (RECTDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static COUNTERDATA *CounterData( HWND hwnd )
{
    return (COUNTERDATA *)WinQueryWindowULong( hwnd, QWL_USER );
}

static void NotifyOwner( HWND hwnd, USHORT code, MPARAM mp )
{
    HWND hwndOwner = WinQueryWindow( hwnd, QW_OWNER );

    if( !hwndOwner )
        hwndOwner = WinQueryWindow( hwnd, QW_PARENT );

    if( hwndOwner )
        WinSendMsg( hwndOwner, WM_CONTROL,
                    MPFROM2SHORT( WinQueryWindowUShort( hwnd, QWS_ID ), code ),
                    mp );
}

/* Generic CTLM_QUERYSTYLE / CTLM_SETSTYLE helper.  Returns TRUE if the
 * message was handled (and sets *pmrResult). */
static BOOL GeneralMsg( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2,
                        MRESULT *pmrResult )
{
    ULONG *pulStyle = (ULONG *)WinQueryWindowULong( hwnd, QWL_USER );

    (void)mp2;

    if( !pulStyle )
        return FALSE;

    switch( msg )
    {
        case CTLM_QUERYSTYLE:
            *pmrResult = (MRESULT)*pulStyle;
            return TRUE;
        case CTLM_SETSTYLE:
            *pulStyle = (ULONG)mp1;
            *pmrResult = (MRESULT)FALSE;
            return TRUE;
    }
    return FALSE;
}

/* ------------------------------------------------------------------ */
/* Shared drawing helpers                                             */
/* ------------------------------------------------------------------ */

/* Set up a solid, single-colour fill for the area / box primitives. */
static void SetFillColor( HPS hps, LONG color )
{
    GpiSetColor( hps, color );
    GpiSetBackColor( hps, color );
    GpiSetPattern( hps, PATSYM_SOLID );
}

static void FillTriangle( HPS hps, LONG x1, LONG y1, LONG x2, LONG y2,
                          LONG x3, LONG y3, LONG color )
{
    POINTL aptl[3];

    aptl[0].x = x1; aptl[0].y = y1;
    aptl[1].x = x2; aptl[1].y = y2;
    aptl[2].x = x3; aptl[2].y = y3;

    SetFillColor( hps, color );
    GpiBeginArea( hps, BA_NOBOUNDARY );
    GpiMove( hps, &aptl[0] );
    GpiPolyLine( hps, 2, &aptl[1] );
    GpiEndArea( hps );
}

/* A white (labels) band, then the "Up"/"Down" captions.  Used by the
 * switch controls, drawn the way the Controls+ demo shows them. */
static void DrawSwitchLabels( HPS hps, PRECTL prcl, SWITCHDATA *p )
{
    RECTL rLab;

    rLab = *prcl;
    rLab.yBottom = prcl->yTop - ( prcl->yTop - prcl->yBottom ) / 4;
    WinDrawText( hps, -1, p->upText, &rLab, CLR_BLACK, CLR_WHITE,
                 DT_CENTER | DT_VCENTER | DT_ERASERECT );

    rLab = *prcl;
    rLab.yTop = prcl->yBottom + ( prcl->yTop - prcl->yBottom ) / 4;
    WinDrawText( hps, -1, p->downText, &rLab, CLR_BLACK, CLR_WHITE,
                 DT_CENTER | DT_VCENTER | DT_ERASERECT );
}

/* ================================================================== */
/* Meter                                                              */
/* ================================================================== */

static void DrawMeter( HWND hwnd, HPS hps )
{
    METERDATA *p = MeterData( hwnd );
    RECTL rcl, rFill;
    POINTL ptl, ptl2;
    LONG  lW, lH, lTubeX, lTubeW, lTop, lBot, lRange, lVal, lFillH;
    int   i, div;

    if( !p )
        return;

    WinQueryWindowRect( hwnd, &rcl );
    WinFillRect( hps, &rcl, CLR_WHITE );

    lW = rcl.xRight - rcl.xLeft;
    lH = rcl.yTop - rcl.yBottom;
    if( lW < 16 || lH < 24 )
        return;

    lTubeW = ( lW - 24 ) / 3;
    if( lTubeW < 8 )
        lTubeW = 8;
    lTubeX = rcl.xLeft + 12;
    lTop   = rcl.yTop - 10;
    lBot   = rcl.yBottom + 10;

    lRange = p->upper - p->lower;
    if( lRange <= 0 )
        lRange = 1;
    lVal = p->value - p->lower;
    if( lVal < 0 )
        lVal = 0;
    if( lVal > lRange )
        lVal = lRange;
    lFillH = ( lVal * ( lTop - lBot ) ) / lRange;

    /* mercury column: black outline */
    GpiSetColor( hps, CLR_BLACK );
    ptl.x  = lTubeX;
    ptl.y  = lBot;
    GpiMove( hps, &ptl );
    ptl2.x = lTubeX + lTubeW;
    ptl2.y = lTop;
    GpiBox( hps, DRO_OUTLINE, &ptl2, 0, 0 );

    /* fill from the bottom */
    if( lFillH > 2 )
    {
        rFill.xLeft   = lTubeX + 1;
        rFill.xRight  = lTubeX + lTubeW;
        rFill.yBottom = lBot + 1;
        rFill.yTop    = lBot + lFillH;
        WinFillRect( hps, &rFill, p->fgColor );
    }

    /* tick marks to the right of the tube */
    div = ( p->style & MTS_QUARTERTICK ) ? 4 :
          ( p->style & MTS_HALFTICK )    ? 2 : 0;
    if( div > 0 )
    {
        GpiSetColor( hps, CLR_BLACK );
        for( i = 0; i <= div; i++ )
        {
            LONG   y   = lBot + ( i * ( lTop - lBot ) ) / div;
            LONG   len = ( i == 0 || i == div ) ? 12 : 7;
            POINTL a, b;

            a.x = lTubeX + lTubeW + 1;
            a.y = y;
            b.x = a.x + len;
            b.y = y;
            GpiMove( hps, &a );
            GpiLine( hps, &b );
        }
    }
}

static void MeterSetValue( HWND hwnd, LONG lValue )
{
    METERDATA *p = MeterData( hwnd );

    if( lValue < p->lower )
        lValue = p->lower;
    if( lValue >= p->upper )
        lValue = p->upper - 1;

    if( lValue != p->value )
    {
        p->value = lValue;
        WinInvalidateRect( hwnd, NULL, FALSE );
        if( p->style & MTS_NOTIFYOWNER )
            NotifyOwner( hwnd, MTN_CHGVALUE, MPFROMSHORT( (USHORT)p->value ) );
    }
}

static void MeterFromPointer( HWND hwnd, LONG lY )
{
    METERDATA *p = MeterData( hwnd );
    RECTL rcl;
    LONG  lH, lRange, lVal;

    WinQueryWindowRect( hwnd, &rcl );
    lH = ( rcl.yTop - 8 ) - ( rcl.yBottom + 4 );
    if( lH <= 0 )
        return;
    lRange = p->upper - p->lower;
    if( lRange <= 0 )
        return;

    lVal = p->lower + ( ( lY - ( rcl.yBottom + 4 ) ) * lRange ) / lH;
    MeterSetValue( hwnd, lVal );
}

static MRESULT EXPENTRY MeterProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    METERDATA *p = MeterData( hwnd );
    MRESULT   mr;

    if( GeneralMsg( hwnd, msg, mp1, mp2, &mr ) )
        return mr;

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)mp2;
            PMTCREATE pmc = pcs ? (PMTCREATE)pcs->pCtlData : NULL;

            p = (METERDATA *)malloc( sizeof( METERDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            p->style    = pmc ? pmc->mtStyle : MTS_THERM;
            p->fgColor  = ( pmc && pmc->mtFgColor ) ? (LONG)pmc->mtFgColor : CLR_BLUE;
            p->lower    = 0;
            p->upper    = 100;
            p->value    = 0;
            p->bCapture = FALSE;
            WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );
            return (MRESULT)FALSE;
        }

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawMeter( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_BUTTON1DOWN:
            if( p && !( p->style & MTS_NOINPUT ) )
            {
                p->bCapture = TRUE;
                WinSetCapture( HWND_DESKTOP, hwnd );
                MeterFromPointer( hwnd, SHORT2FROMMP( mp1 ) );
            }
            return (MRESULT)FALSE;

        case WM_MOUSEMOVE:
            if( p && p->bCapture )
                MeterFromPointer( hwnd, SHORT2FROMMP( mp1 ) );
            return (MRESULT)FALSE;

        case WM_BUTTON1UP:
            if( p && p->bCapture )
            {
                p->bCapture = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );
            }
            return (MRESULT)FALSE;

        case MTM_SETRANGE:
            if( p )
            {
                LONG lLower = (LONG)SHORT1FROMMP( mp1 );
                LONG lUpper = (LONG)SHORT2FROMMP( mp1 );

                if( lUpper <= lLower )
                    lUpper = lLower + 1;
                p->lower = lLower;
                p->upper = lUpper;
                if( p->value < p->lower )
                    p->value = p->lower;
                if( p->value >= p->upper )
                    p->value = p->upper - 1;
                WinInvalidateRect( hwnd, NULL, FALSE );
                if( p->style & MTS_NOTIFYOWNER )
                    NotifyOwner( hwnd, MTN_CHGRANGE,
                                 MPFROM2SHORT( (USHORT)p->lower, (USHORT)p->upper ) );
            }
            return (MRESULT)FALSE;

        case MTM_QUERYRANGE:
            if( p )
                return MPFROM2SHORT( (USHORT)p->lower, (USHORT)p->upper );
            return (MRESULT)FALSE;

        case MTM_SETVALUE:
            if( p )
                MeterSetValue( hwnd, (LONG)SHORT1FROMMP( mp1 ) );
            return (MRESULT)FALSE;

        case MTM_QUERYVALUE:
            return p ? (MRESULT)p->value : (MRESULT)0;

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ================================================================== */
/* Switch                                                             */
/* ================================================================== */

static void DrawSwitch( HWND hwnd, HPS hps )
{
    SWITCHDATA *p = SwitchData( hwnd );
    RECTL  rcl;
    POINTL ptl, ptl2;
    LONG   lW, lH;

    if( !p )
        return;

    WinQueryWindowRect( hwnd, &rcl );
    WinFillRect( hps, &rcl, CLR_WHITE );
    lW = rcl.xRight - rcl.xLeft;
    lH = rcl.yTop - rcl.yBottom;

    DrawSwitchLabels( hps, &rcl, p );

    if( p->style & SWS_THROW )
    {
        /* Blue rounded body with a white lever that slides up / down. */
        LONG bw = lW / 2;
        LONG bh = lH / 2;
        LONG bx = rcl.xLeft + ( lW - bw ) / 2;
        LONG by = rcl.yBottom + ( lH - bh ) / 2;
        LONG lv, lt, ly;

        SetFillColor( hps, p->fgColor );
        ptl.x  = bx;
        ptl.y  = by;
        GpiMove( hps, &ptl );
        ptl2.x = bx + bw;
        ptl2.y = by + bh;
        GpiBox( hps, DRO_FILL, &ptl2, bw / 6, bh / 6 );

        lv = bw - 8;
        if( lv < 6 )
            lv = 6;
        lt = bh / 5;
        if( lt < 3 )
            lt = 3;
        ly = p->value ? ( by + bh - 4 - lt ) : ( by + 4 );

        SetFillColor( hps, CLR_WHITE );
        ptl.x  = bx + 4;
        ptl.y  = ly;
        GpiMove( hps, &ptl );
        ptl2.x = bx + 4 + lv;
        ptl2.y = ly + lt;
        GpiBox( hps, DRO_FILL, &ptl2, lt / 2, lt / 2 );
    }
    else
    {
        /* A round push button, greyed when off, filled with the fg
         * colour when on. */
        LONG   dia = ( lW < ( lH * 2 ) / 3 ) ? lW : ( lH * 2 ) / 3;
        LONG   cx  = rcl.xLeft + lW / 2;
        LONG   cy  = rcl.yBottom + lH / 2;
        LONG   r;
        POINTL aptl[2];

        dia -= 12;
        if( dia < 16 )
            dia = 16;
        r = dia / 2;

        aptl[0].x = cx - r;
        aptl[0].y = cy - r;
        aptl[1].x = cx + r;
        aptl[1].y = cy + r;

        SetFillColor( hps, p->value ? p->fgColor : CLR_PALEGRAY );
        GpiOval( hps, aptl, 0L, 360L * 65536L, DRO_FILL );

        GpiSetColor( hps, CLR_BLACK );
        GpiOval( hps, aptl, 0L, 360L * 65536L, DRO_OUTLINE );
    }
}

static void SwitchSetValue( HWND hwnd, BOOL bValue )
{
    SWITCHDATA *p = SwitchData( hwnd );

    bValue = bValue ? TRUE : FALSE;
    if( bValue != p->value )
    {
        p->value = bValue;
        WinInvalidateRect( hwnd, NULL, FALSE );
        if( p->style & SWS_NOTIFYOWNER )
            NotifyOwner( hwnd, SWN_CHGVALUE, MPFROMSHORT( (USHORT)p->value ) );
    }
}

static MRESULT EXPENTRY SwitchProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    SWITCHDATA *p = SwitchData( hwnd );
    MRESULT     mr;

    if( GeneralMsg( hwnd, msg, mp1, mp2, &mr ) )
        return mr;

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)mp2;
            PSWCREATE psw = pcs ? (PSWCREATE)pcs->pCtlData : NULL;

            p = (SWITCHDATA *)malloc( sizeof( SWITCHDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            p->style   = psw ? psw->swStyle : SWS_BUTTON;
            p->fgColor = ( psw && psw->swFgColor ) ? (LONG)psw->swFgColor : CLR_BLUE;
            p->value   = FALSE;
            p->upText[0]   = '\0';
            p->downText[0] = '\0';
            if( psw )
            {
                strncpy( p->upText, psw->swUpText, SW_MAXTEXT - 1 );
                strncpy( p->downText, psw->swDownText, SW_MAXTEXT - 1 );
                p->upText[SW_MAXTEXT-1]   = '\0';
                p->downText[SW_MAXTEXT-1] = '\0';
            }
            WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );
            return (MRESULT)FALSE;
        }

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawSwitch( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_BUTTON1UP:
            if( p )
                SwitchSetValue( hwnd, !p->value );
            return (MRESULT)FALSE;

        case SWM_SETVALUE:
            if( p )
                SwitchSetValue( hwnd, (BOOL)SHORT1FROMMP( mp1 ) );
            return (MRESULT)FALSE;

        case SWM_QUERYVALUE:
            return p ? (MRESULT)p->value : (MRESULT)FALSE;

        case SWM_SETUPTEXT:
            if( p && mp1 )
            {
                strncpy( p->upText, (PSZ)mp1, SW_MAXTEXT - 1 );
                p->upText[SW_MAXTEXT-1] = '\0';
                WinInvalidateRect( hwnd, NULL, FALSE );
                if( p->style & SWS_NOTIFYOWNER )
                    NotifyOwner( hwnd, SWN_CHGUPTEXT, MPFROMP( p->upText ) );
            }
            return (MRESULT)FALSE;

        case SWM_SETDOWNTEXT:
            if( p && mp1 )
            {
                strncpy( p->downText, (PSZ)mp1, SW_MAXTEXT - 1 );
                p->downText[SW_MAXTEXT-1] = '\0';
                WinInvalidateRect( hwnd, NULL, FALSE );
                if( p->style & SWS_NOTIFYOWNER )
                    NotifyOwner( hwnd, SWN_CHGDOWNTEXT, MPFROMP( p->downText ) );
            }
            return (MRESULT)FALSE;

        case SWM_QUERYUPTEXT:
            if( p && mp1 && mp2 )
            {
                PSZ psz = (PSZ)mp2;
                LONG cch = (LONG)SHORT1FROMMP( mp1 );

                if( cch > 0 )
                {
                    strncpy( psz, p->upText, (size_t)cch - 1 );
                    psz[cch-1] = '\0';
                }
            }
            return (MRESULT)FALSE;

        case SWM_QUERYDOWNTEXT:
            if( p && mp1 && mp2 )
            {
                PSZ psz = (PSZ)mp2;
                LONG cch = (LONG)SHORT1FROMMP( mp1 );

                if( cch > 0 )
                {
                    strncpy( psz, p->downText, (size_t)cch - 1 );
                    psz[cch-1] = '\0';
                }
            }
            return (MRESULT)FALSE;

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ================================================================== */
/* Rect                                                               */
/* ================================================================== */

static void RectClamp( RECTDATA *p, LONG lW, LONG lH )
{
    if( p->rcl.xLeft < 0 )
    {
        p->rcl.xRight -= p->rcl.xLeft;
        p->rcl.xLeft = 0;
    }
    if( p->rcl.yBottom < 0 )
    {
        p->rcl.yTop -= p->rcl.yBottom;
        p->rcl.yBottom = 0;
    }
    if( p->rcl.xRight > lW )
    {
        LONG d = p->rcl.xRight - lW;
        p->rcl.xLeft -= d;
        p->rcl.xRight = lW;
        if( p->rcl.xLeft < 0 )
            p->rcl.xLeft = 0;
    }
    if( p->rcl.yTop > lH )
    {
        LONG d = p->rcl.yTop - lH;
        p->rcl.yBottom -= d;
        p->rcl.yTop = lH;
        if( p->rcl.yBottom < 0 )
            p->rcl.yBottom = 0;
    }
}

static void DrawRect( HWND hwnd, HPS hps )
{
    RECTDATA *p = RectData( hwnd );
    RECTL rcl, rIn;
    POINTL ptl, ptl2;
    LONG  lW, lH;
    int   i;
    static const LONG aHx[8] = { 0, 1, 2, 0, 2, 0, 1, 2 };
    static const LONG aHy[8] = { 0, 0, 0, 1, 1, 2, 2, 2 };

    if( !p )
        return;

    WinQueryWindowRect( hwnd, &rcl );
    WinFillRect( hps, &rcl, CLR_WHITE );

    lW = rcl.xRight - rcl.xLeft;
    lH = rcl.yTop - rcl.yBottom;
    if( !p->bInit )
    {
        p->rcl.xLeft   = lW / 4;
        p->rcl.yBottom = lH / 4;
        p->rcl.xRight  = lW - lW / 4;
        p->rcl.yTop    = lH - lH / 4;
        p->bInit = TRUE;
    }

    /* dotted rubber-band outline */
    GpiSetColor( hps, CLR_BLACK );
    GpiSetLineType( hps, LINETYPE_DOT );
    ptl.x  = p->rcl.xLeft;
    ptl.y  = p->rcl.yBottom;
    GpiMove( hps, &ptl );
    ptl2.x = p->rcl.xRight;
    ptl2.y = p->rcl.yTop;
    GpiBox( hps, DRO_OUTLINE, &ptl2, 0, 0 );
    GpiSetLineType( hps, LINETYPE_SOLID );

    /* eight solid handles: 4 corners + 4 edge midpoints */
    if( p->style & ( RCS_VERTSIZE | RCS_HORZSIZE ) )
    {
        LONG ax[3], ay[3];

        ax[0] = p->rcl.xLeft;
        ax[1] = ( p->rcl.xLeft + p->rcl.xRight ) / 2;
        ax[2] = p->rcl.xRight;
        ay[0] = p->rcl.yBottom;
        ay[1] = ( p->rcl.yBottom + p->rcl.yTop ) / 2;
        ay[2] = p->rcl.yTop;

        for( i = 0; i < 8; i++ )
        {
            RECTL rKnob;

            rKnob.xLeft   = ax[aHx[i]] - HANDLE_SIZE / 2;
            rKnob.yBottom = ay[aHy[i]] - HANDLE_SIZE / 2;
            rKnob.xRight  = ax[aHx[i]] + HANDLE_SIZE / 2;
            rKnob.yTop    = ay[aHy[i]] + HANDLE_SIZE / 2;
            WinFillRect( hps, &rKnob, CLR_BLACK );
        }
    }

    /* Notify owner with the interior rect */
    if( p->style & RCS_NOTIFYOWNER )
    {
        rIn = p->rcl;
        rIn.xLeft   += 1;
        rIn.yBottom += 1;
        rIn.xRight  -= 1;
        rIn.yTop    -= 1;
        NotifyOwner( hwnd, RCN_PAINT, MPFROMP( &rIn ) );
    }
}

static void RectSizeToRect( RECTL *prcl, LONG cx, LONG cy )
{
    prcl->xLeft   = 0;
    prcl->yBottom = 0;
    prcl->xRight  = cx;
    prcl->yTop    = cy;
}

static MRESULT EXPENTRY RectProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    RECTDATA *p = RectData( hwnd );
    MRESULT   mr;

    if( GeneralMsg( hwnd, msg, mp1, mp2, &mr ) )
        return mr;

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)mp2;
            PRCCREATE prc = pcs ? (PRCCREATE)pcs->pCtlData : NULL;

            p = (RECTDATA *)malloc( sizeof( RECTDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            p->style   = prc ? prc->rcStyle : ( RCS_VERTHORZSIZE | RCS_KSQUARE );
            p->fgColor = ( prc && prc->rcFgColor ) ? (LONG)prc->rcFgColor : CLR_BLUE;
            p->cxMin = 16;
            p->cyMin = 16;
            p->cxMax = 32767;
            p->cyMax = 32767;
            if( prc )
            {
                if( prc->rcMinSize.cx > 0 ) p->cxMin = prc->rcMinSize.cx;
                if( prc->rcMinSize.cy > 0 ) p->cyMin = prc->rcMinSize.cy;
                if( prc->rcMaxSize.cx > 0 ) p->cxMax = prc->rcMaxSize.cx;
                if( prc->rcMaxSize.cy > 0 ) p->cyMax = prc->rcMaxSize.cy;
            }
            p->handle = 0;
            p->bInit  = FALSE;
            p->dragMode = 0;
            WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );
            return (MRESULT)FALSE;
        }

        case WM_SIZE:
            if( p )
            {
                if( !p->bInit )
                    p->bInit = TRUE;
                RectClamp( p, (LONG)SHORT1FROMMP( mp2 ), (LONG)SHORT2FROMMP( mp2 ) );
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)FALSE;

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawRect( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_BUTTON1DOWN:
        {
            POINTL ptl;
            LONG   dl, dr, db, dt;
            BOOL   bSize = ( p->style & ( RCS_VERTSIZE | RCS_HORZSIZE ) ) != 0;

            ptl.x = (LONG)SHORT1FROMMP( mp1 );
            ptl.y = (LONG)SHORT2FROMMP( mp1 );

            dl = ptl.x - p->rcl.xLeft;
            dr = p->rcl.xRight - ptl.x;
            db = ptl.y - p->rcl.yBottom;
            dt = p->rcl.yTop - ptl.y;

            p->ptlStart = ptl;
            p->rclStart = p->rcl;
            p->dragMode = 0;
            p->bDragLeft = p->bDragRight = p->bDragTop = p->bDragBottom = FALSE;

            if( bSize && ( dl < HANDLE_SIZE || dr < HANDLE_SIZE ||
                           db < HANDLE_SIZE || dt < HANDLE_SIZE ) &&
                dl > -HANDLE_SIZE && dr > -HANDLE_SIZE &&
                db > -HANDLE_SIZE && dt > -HANDLE_SIZE )
            {
                p->dragMode = 2;
                p->bDragLeft   = ( dl < HANDLE_SIZE );
                p->bDragRight  = ( dr < HANDLE_SIZE );
                p->bDragBottom = ( db < HANDLE_SIZE );
                p->bDragTop    = ( dt < HANDLE_SIZE );
                if( p->style & RCS_NOTIFYOWNER )
                    NotifyOwner( hwnd, RCN_SIZE, MPFROMSHORT( TRUE ) );
            }
            else if( !( p->style & RCS_NOMOVE ) &&
                     ptl.x >= p->rcl.xLeft && ptl.x <= p->rcl.xRight &&
                     ptl.y >= p->rcl.yBottom && ptl.y <= p->rcl.yTop )
            {
                p->dragMode = 1;
                if( p->style & RCS_NOTIFYOWNER )
                    NotifyOwner( hwnd, RCN_DRAG, MPFROMSHORT( TRUE ) );
            }

            if( p->dragMode )
                WinSetCapture( HWND_DESKTOP, hwnd );
            return (MRESULT)FALSE;
        }

        case WM_MOUSEMOVE:
            if( p && p->dragMode )
            {
                RECTL  rcl;
                LONG   lW, lH, dx, dy;

                WinQueryWindowRect( hwnd, &rcl );
                lW = rcl.xRight;
                lH = rcl.yTop;

                dx = (LONG)SHORT1FROMMP( mp1 ) - p->ptlStart.x;
                dy = (LONG)SHORT2FROMMP( mp1 ) - p->ptlStart.y;

                p->rcl = p->rclStart;
                if( p->dragMode == 1 )
                {
                    p->rcl.xLeft   += dx;
                    p->rcl.xRight  += dx;
                    p->rcl.yBottom += dy;
                    p->rcl.yTop    += dy;
                    RectClamp( p, lW, lH );
                }
                else
                {
                    if( p->bDragLeft )   p->rcl.xLeft   += dx;
                    if( p->bDragRight )  p->rcl.xRight  += dx;
                    if( p->bDragBottom ) p->rcl.yBottom += dy;
                    if( p->bDragTop )    p->rcl.yTop    += dy;

                    if( p->rcl.xRight - p->rcl.xLeft < p->cxMin )
                    {
                        if( p->bDragLeft ) p->rcl.xLeft = p->rcl.xRight - p->cxMin;
                        else               p->rcl.xRight = p->rcl.xLeft + p->cxMin;
                    }
                    if( p->rcl.yTop - p->rcl.yBottom < p->cyMin )
                    {
                        if( p->bDragBottom ) p->rcl.yBottom = p->rcl.yTop - p->cyMin;
                        else                 p->rcl.yTop = p->rcl.yBottom + p->cyMin;
                    }
                    if( p->rcl.xRight - p->rcl.xLeft > p->cxMax )
                    {
                        if( p->bDragLeft ) p->rcl.xLeft = p->rcl.xRight - p->cxMax;
                        else               p->rcl.xRight = p->rcl.xLeft + p->cxMax;
                    }
                    if( p->rcl.yTop - p->rcl.yBottom > p->cyMax )
                    {
                        if( p->bDragBottom ) p->rcl.yBottom = p->rcl.yTop - p->cyMax;
                        else                 p->rcl.yTop = p->rcl.yBottom + p->cyMax;
                    }

                    if( p->rcl.xLeft < 0 ) { p->rcl.xLeft = 0; }
                    if( p->rcl.yBottom < 0 ) { p->rcl.yBottom = 0; }
                    if( p->rcl.xRight > lW ) { p->rcl.xRight = lW; }
                    if( p->rcl.yTop > lH ) { p->rcl.yTop = lH; }
                }
                WinInvalidateRect( hwnd, NULL, FALSE );
            }
            return (MRESULT)FALSE;

        case WM_BUTTON1UP:
            if( p && p->dragMode )
            {
                USHORT code = ( p->dragMode == 1 ) ? RCN_DRAG : RCN_SIZE;

                p->dragMode = 0;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );
                if( p->style & RCS_NOTIFYOWNER )
                    NotifyOwner( hwnd, code, MPFROMSHORT( FALSE ) );
            }
            return (MRESULT)FALSE;

        case RCM_SETHANDLE:
            if( p )
                p->handle = (LONG)mp1;
            return (MRESULT)FALSE;

        case RCM_QUERYHANDLE:
            return p ? (MRESULT)p->handle : (MRESULT)0;

        case RCM_SETMINSIZE:
            if( p && mp1 && ( p->style & RCS_MINSIZE ) )
            {
                PRECTL prcl = (PRECTL)mp1;

                if( prcl->xRight > 0 )
                    p->cxMin = prcl->xRight;
                if( prcl->yTop > 0 )
                    p->cyMin = prcl->yTop;
            }
            return (MRESULT)FALSE;

        case RCM_SETMAXSIZE:
            if( p && mp1 && ( p->style & RCS_MAXSIZE ) )
            {
                PRECTL prcl = (PRECTL)mp1;

                if( prcl->xRight > 0 )
                    p->cxMax = prcl->xRight;
                if( prcl->yTop > 0 )
                    p->cyMax = prcl->yTop;
            }
            return (MRESULT)FALSE;

        case RCM_QUERYMINSIZE:
            if( p && mp1 )
                RectSizeToRect( (PRECTL)mp1, p->cxMin, p->cyMin );
            return (MRESULT)FALSE;

        case RCM_QUERYMAXSIZE:
            if( p && mp1 )
                RectSizeToRect( (PRECTL)mp1, p->cxMax, p->cyMax );
            return (MRESULT)FALSE;

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ================================================================== */
/* Counter                                                            */
/* ================================================================== */

static USHORT SegmentMask( LONG value )
{
    static USHORT ausMask[16] =
    {
        0x3F, 0x06, 0x5B, 0x4F,   /* 0 1 2 3 */
        0x66, 0x6D, 0x7D, 0x07,   /* 4 5 6 7 */
        0x7F, 0x6F, 0x77, 0x7C,   /* 8 9 A b */
        0x39, 0x5E, 0x79, 0x71    /* C d E F */
    };

    return ausMask[value & 0x0F];
}

static void FillSeg( HPS hps, LONG x1, LONG y1, LONG x2, LONG y2, LONG color )
{
    RECTL r;

    if( x2 <= x1 || y2 <= y1 )
        return;
    r.xLeft   = x1;
    r.yBottom = y1;
    r.xRight  = x2;
    r.yTop    = y2;
    WinFillRect( hps, &r, color );
}

static void DrawCounter( HWND hwnd, HPS hps )
{
    COUNTERDATA *p = CounterData( hwnd );
    RECTL rcl;
    LONG  lW, lH, t, left, right, top, bot, mid, half, aw;
    USHORT mask;

    if( !p )
        return;

    WinQueryWindowRect( hwnd, &rcl );
    WinFillRect( hps, &rcl, p->bgColor );

    lW = rcl.xRight - rcl.xLeft;
    lH = rcl.yTop - rcl.yBottom;
    if( lW < 6 || lH < 8 )
        return;

    left  = rcl.xLeft + 2;
    right = rcl.xRight - 2;
    top   = rcl.yTop - 2;
    bot   = rcl.yBottom + 2;
    mid   = ( top + bot ) / 2;

    /* Reserve a narrow column on the right for the scroll chevrons. */
    aw = lW / 3;
    if( aw < 6 )
        aw = 6;
    if( right - left > aw + 6 )
        right -= aw;
    else
        aw = 0;

    t = ( right - left ) / 5;
    if( t < 2 )
        t = 2;
    if( t > ( top - bot ) / 5 )
        t = ( top - bot ) / 5;
    if( t < 1 )
        t = 1;
    half = t / 2;
    if( half < 1 )
        half = 1;

    mask = SegmentMask( p->value );

    /* 7-segment digit: bit0=a(0x01) .. bit6=g(0x40) */
    if( mask & 0x01 )   /* a : top */
        FillSeg( hps, left + t, top - t, right - t, top, p->fgColor );
    if( mask & 0x02 )   /* b : upper right */
        FillSeg( hps, right - t, mid + half, right, top - half, p->fgColor );
    if( mask & 0x04 )   /* c : lower right */
        FillSeg( hps, right - t, bot + half, right, mid - half, p->fgColor );
    if( mask & 0x08 )   /* d : bottom */
        FillSeg( hps, left + t, bot, right - t, bot + t, p->fgColor );
    if( mask & 0x10 )   /* e : lower left */
        FillSeg( hps, left, bot + half, left + t, mid - half, p->fgColor );
    if( mask & 0x20 )   /* f : upper left */
        FillSeg( hps, left, mid + half, left + t, top - half, p->fgColor );
    if( mask & 0x40 )   /* g : middle */
        FillSeg( hps, left + t, mid - half, right - t, mid + half, p->fgColor );

    /* Scroll chevrons: up near the top, down near the bottom. */
    if( aw > 0 )
    {
        LONG ax0 = right + 2;
        LONG aw2 = ( rcl.xRight - 2 ) - ax0;
        LONG acx = ax0 + aw2 / 2;
        LONG ath = ( top - bot ) / 4;

        if( ath < 3 )
            ath = 3;
        FillTriangle( hps, acx, top - 2, ax0, top - 2 - ath,
                      rcl.xRight - 2, top - 2 - ath, p->fgColor );
        FillTriangle( hps, acx, bot + 2, ax0, bot + 2 + ath,
                      rcl.xRight - 2, bot + 2 + ath, p->fgColor );
    }
}

static void CounterSetValue( HWND hwnd, LONG lValue )
{
    COUNTERDATA *p = CounterData( hwnd );

    if( !p || lValue == p->value )
        return;

    p->value = lValue;
    WinInvalidateRect( hwnd, NULL, FALSE );

    if( p->style & CTS_NOTIFYOWNER )
        NotifyOwner( hwnd, CTN_CHGVALUE,
                     MPFROM2SHORT( (USHORT)p->value, (USHORT)p->maxValue ) );
}

/* Increment.  On overflow the digit wraps to zero and either rolls into
 * the next significant digit or, when there is no next digit / NOROLL is
 * set, notifies CTN_OVERFLOW. */
static void CounterInc( HWND hwnd, BOOL bNoRoll )
{
    COUNTERDATA *p = CounterData( hwnd );

    if( !p )
        return;

    if( p->value < p->maxValue )
    {
        CounterSetValue( hwnd, p->value + 1 );
        return;
    }

    CounterSetValue( hwnd, 0 );
    if( !bNoRoll && p->hwndNext && !( p->style & CTS_NOROLL ) )
        WinSendMsg( p->hwndNext, CTM_INC, 0, 0 );
    else if( p->style & CTS_NOTIFYOWNER )
        NotifyOwner( hwnd, CTN_OVERFLOW, MPFROMSHORT( (USHORT)p->maxValue ) );
}

/* Decrement.  On underflow the digit wraps to max and either rolls into
 * the next significant digit or notifies CTN_UNDERFLOW. */
static void CounterDec( HWND hwnd, BOOL bNoRoll )
{
    COUNTERDATA *p = CounterData( hwnd );

    if( !p )
        return;

    if( p->value > 0 )
    {
        CounterSetValue( hwnd, p->value - 1 );
        return;
    }

    CounterSetValue( hwnd, p->maxValue );
    if( !bNoRoll && p->hwndNext && !( p->style & CTS_NOROLL ) )
        WinSendMsg( p->hwndNext, CTM_DEC, 0, 0 );
    else if( p->style & CTS_NOTIFYOWNER )
        NotifyOwner( hwnd, CTN_UNDERFLOW, MPFROMSHORT( (USHORT)p->maxValue ) );
}

static MRESULT EXPENTRY CounterProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    COUNTERDATA *p = CounterData( hwnd );
    MRESULT   mr;

    if( GeneralMsg( hwnd, msg, mp1, mp2, &mr ) )
        return mr;

    switch( msg )
    {
        case WM_CREATE:
        {
            PCREATESTRUCT pcs = (PCREATESTRUCT)mp2;
            PCNTCREATE pcc = pcs ? (PCNTCREATE)pcs->pCtlData : NULL;

            p = (COUNTERDATA *)malloc( sizeof( COUNTERDATA ) );
            if( !p )
                return (MRESULT)TRUE;
            p->style    = pcc ? pcc->cntStyle : CTS_DEC;
            p->fgColor  = ( pcc && pcc->cntFgColor ) ? (LONG)pcc->cntFgColor : CLR_RED;
            p->bgColor  = ( pcc && pcc->cntBgColor ) ? (LONG)pcc->cntBgColor : CLR_BLACK;
            p->hwndNext = pcc ? pcc->cntNext : NULLHANDLE;
            p->value    = 0;
            if( p->style & CTS_BIN )
                p->radix = 2;
            else if( p->style & CTS_HEX )
                p->radix = 16;
            else
                p->radix = 10;
            p->maxValue = p->radix - 1;
            WinSetWindowULong( hwnd, QWL_USER, (ULONG)p );
            return (MRESULT)FALSE;
        }

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

            DrawCounter( hwnd, hps );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case WM_BUTTON1DOWN:
            if( p && !( p->style & CTS_NOSCROLL ) )
            {
                RECTL rcl;

                WinQueryWindowRect( hwnd, &rcl );
                if( (LONG)SHORT2FROMMP( mp1 ) >= ( rcl.yTop + rcl.yBottom ) / 2 )
                    CounterInc( hwnd, FALSE );
                else
                    CounterDec( hwnd, FALSE );
            }
            return (MRESULT)FALSE;

        case CTM_SETVALUE:
            if( p )
            {
                LONG lValue = (LONG)SHORT1FROMMP( mp1 );

                if( lValue >= 0 && lValue <= p->maxValue )
                    CounterSetValue( hwnd, lValue );
            }
            return (MRESULT)FALSE;

        case CTM_QUERYVALUE:
            return p ? (MRESULT)p->value : (MRESULT)0;

        case CTM_SETNEXT:
            if( p )
                p->hwndNext = (HWND)mp1;
            return (MRESULT)FALSE;

        case CTM_QUERYNEXT:
            return p ? (MRESULT)p->hwndNext : (MRESULT)NULLHANDLE;

        case CTM_INC:
            CounterInc( hwnd, FALSE );
            return (MRESULT)FALSE;

        case CTM_DEC:
            CounterDec( hwnd, FALSE );
            return (MRESULT)FALSE;

        case CTM_INCNOROLL:
            CounterInc( hwnd, TRUE );
            return (MRESULT)FALSE;

        case CTM_DECNOROLL:
            CounterDec( hwnd, TRUE );
            return (MRESULT)FALSE;

        case WM_DESTROY:
            if( p )
            {
                WinSetWindowULong( hwnd, QWL_USER, 0 );
                free( p );
            }
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ================================================================== */
/* Public entry points                                                */
/* ================================================================== */

USHORT EXPENTRY XpmCtlInit( HAB hab )
{
    if( !WinRegisterClass( hab, WC_METER,   MeterProc,   CS_SIZEREDRAW,
                           sizeof( ULONG ) ) )
        return FALSE;
    if( !WinRegisterClass( hab, WC_SWITCH,  SwitchProc,  CS_SIZEREDRAW,
                           sizeof( ULONG ) ) )
        return FALSE;
    if( !WinRegisterClass( hab, WC_RECT,    RectProc,    CS_SIZEREDRAW,
                           sizeof( ULONG ) ) )
        return FALSE;
    if( !WinRegisterClass( hab, WC_COUNTER, CounterProc, CS_SIZEREDRAW,
                           sizeof( ULONG ) ) )
        return FALSE;
    return TRUE;
}

USHORT EXPENTRY XpmCtlTerminate( VOID )
{
    return TRUE;
}

MRESULT EXPENTRY XpmCtlVersion( VOID )
{
    return MPFROM2SHORT( 1, 40 );
}

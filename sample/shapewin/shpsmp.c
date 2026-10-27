/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/shapewin/shpsmp.c -- Shaped ("transparent") windows sample.
 *
 * Creates a shaped window whose outline follows a circular mask, so the
 * desktop shows through the transparent parts.  Left-dragging the circle
 * moves it; right-clicking cycles its colour.  Demonstrates the
 * XpmShapeWin window class.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define  INCL_WINDIALOGS
#define  INCL_WINENTRYFIELDS
#define  INCL_WINSYS
#define  INCL_WINWINDOWMGR
#define  INCL_WINMENUS
#define  INCL_WINMESSAGEMGR
#define  INCL_WININPUT
#define  INCL_WINPOINTERS
#define  INCL_WINBUTTONS
#define  INCL_GPI
#include <os2.h>
#include <string.h>


#include "shpsmp.h"
#include "expm.h"

#define SHAPE_SIZE      96

static HAB  hab;
static HWND hwndShape = NULLHANDLE;
static HPS  hpsDraw = NULLHANDLE;
static HPS  hpsMask = NULLHANDLE;
static HDC  hdcMem  = NULLHANDLE;
static HBITMAP hbm  = NULLHANDLE;

static LONG lColorIndex = 0;
static const LONG aColors[] = { 0x000000FFL, 0x0000FF00L, 0x00FF0000L,
                                0x0000FFFFL, 0x00FF00FFL, 0x00FFFF00L };
#define N_COLORS (sizeof(aColors)/sizeof(aColors[0]))

static BOOL  bDragging = FALSE;
static POINTL ptDragStart;
static SHORT  sShapeX = 200, sShapeY = 200;

static VOID BuildShape( HWND hwndOwner, LONG idx )
{
    SIZEL   siz;
    RECTL   rcl;
    POINTL  ptl;
    LONG    c;


    /* destroy previous shape window */
    if( hwndShape != NULLHANDLE )
    {
        WinDestroyWindow( hwndShape );
        hwndShape = NULLHANDLE;
    }

    /* (re)create the bitmap + PS if needed */
    if( hpsDraw == NULLHANDLE )
    {
        hdcMem = DevOpenDC( hab, OD_MEMORY, "*", 0L, NULL, NULLHANDLE );
        siz.cx = SHAPE_SIZE; siz.cy = SHAPE_SIZE;
        hpsDraw = GpiCreatePS( hab, hdcMem, &siz,
                    PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );
        hpsMask = hpsDraw;
    }

    /* create a fresh bitmap */
    if( hbm != NULLHANDLE )
    {
        GpiSetBitmap( hpsDraw, NULLHANDLE );
        GpiDeleteBitmap( hbm );
        hbm = NULLHANDLE;
    }

    {
        BITMAPINFOHEADER2 bmi;
        memset( &bmi, 0, sizeof(bmi) );
        bmi.cbFix          = sizeof(BITMAPINFOHEADER2);
        bmi.cx             = SHAPE_SIZE;
        bmi.cy             = SHAPE_SIZE;
        bmi.cPlanes        = 1;
        bmi.cBitCount      = 24;
        hbm = GpiCreateBitmap( hpsDraw, &bmi, 0L, NULL, NULL );
    }
    GpiSetBitmap( hpsDraw, hbm );

    /* fill with transparent (magenta) key colour */
    GpiCreateLogColorTable( hpsDraw, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL );
    rcl.xLeft = 0; rcl.yBottom = 0;
    rcl.xRight = SHAPE_SIZE; rcl.yTop = SHAPE_SIZE;
    WinFillRect( hpsDraw, &rcl, 0x00FF00FFL );

    /* draw a filled circle in the chosen colour */
    c = aColors[idx % N_COLORS];
    GpiSetColor( hpsDraw, c );
    ptl.x = SHAPE_SIZE / 2; ptl.y = SHAPE_SIZE / 2;
    GpiMove( hpsDraw, &ptl );
    {
        /* use a simple filled ellipse via GpiFullArc with ELLIPSE */
        ARCPARAMS ap;
        ap.lP = SHAPE_SIZE / 2 - 4;
        ap.lQ = SHAPE_SIZE / 2 - 4;
        ap.lR = 0; ap.lS = 0;
        GpiSetArcParams( hpsDraw, &ap );
        GpiFullArc( hpsDraw, DRO_FILL, MAKEFIXED(1,0) );
    }

    /* create the shaped window on the desktop */
    {
        XpmShapeData data;
        memset( &data, 0, sizeof(data) );
        data.cbSize     = sizeof(data);
        data.hpsDraw    = hpsDraw;
        data.hpsMask    = hpsDraw;
        data.cx         = SHAPE_SIZE;
        data.cy         = SHAPE_SIZE;
        data.hwndOwner  = hwndOwner;
        hwndShape = WinCreateWindow( HWND_DESKTOP, WC_XpmSHAPEWIN, NULL,
                     0, sShapeX, sShapeY, SHAPE_SIZE, SHAPE_SIZE,
                     hwndOwner, HWND_TOP, 1, &data, NULL );
    }
}

static VOID DestroyShape( VOID )
{
    if( hwndShape != NULLHANDLE )
    {
        WinDestroyWindow( hwndShape );
        hwndShape = NULLHANDLE;
    }
}

MRESULT EXPENTRY MainWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
            XpmShapeInit();
            BuildShape( hwnd, 0 );
            return (MRESULT)FALSE;

        case WM_PAINT:
        {
            HPS   hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );
            RECTL rcl;
            if( hps != NULLHANDLE )
            {
                WinQueryWindowRect( hwnd, &rcl );
                WinFillRect( hps, &rcl, SYSCLR_DIALOGBACKGROUND );
                WinEndPaint( hps );
            }
            return (MRESULT)0;
        }

        case WM_COMMAND:
            if( SHORT1FROMMP( mp1 ) == IDM_MOVE )
            {
                sShapeX = (SHORT)( sShapeX + 80 );
                sShapeY = (SHORT)( sShapeY + 60 );
                if( sShapeX > 900 ) sShapeX = 200;
                if( sShapeY > 600 ) sShapeY = 200;
                if( hwndShape != NULLHANDLE )
                    WinSetWindowPos( hwndShape, HWND_TOP, sShapeX, sShapeY,
                                     0, 0, SWP_MOVE );
                /* repaint the client where the circle was */
                WinInvalidateRect( hwnd, NULL, TRUE );
            }
            else if( SHORT1FROMMP( mp1 ) == IDM_EXIT )
            {
                WinPostMsg( hwnd, WM_CLOSE, 0, 0 );
            }
            break;

        case WM_BUTTON1DOWN:
            /* start dragging the shape */
            bDragging = TRUE;
            WinQueryPointerPos( HWND_DESKTOP, &ptDragStart );
            WinSetCapture( HWND_DESKTOP, hwnd );
            break;

        case WM_MOUSEMOVE:
            if( bDragging )
            {
                POINTL pt;
                WinQueryPointerPos( HWND_DESKTOP, &pt );
                sShapeX = (SHORT)(sShapeX + pt.x - ptDragStart.x);
                sShapeY = (SHORT)(sShapeY + pt.y - ptDragStart.y);
                ptDragStart = pt;
                if( hwndShape != NULLHANDLE )
                    WinSetWindowPos( hwndShape, HWND_TOP, sShapeX, sShapeY,
                                     0, 0, SWP_MOVE );
            }
            break;

        case WM_BUTTON2DOWN:
            /* right-click: cycle colour */
            lColorIndex = (lColorIndex + 1) % N_COLORS;
            BuildShape( hwnd, lColorIndex );
            break;

        case WM_BUTTON1UP:
            if( bDragging )
            {
                bDragging = FALSE;
                WinSetCapture( HWND_DESKTOP, NULLHANDLE );
                /* repaint the client where the circle was */
                WinInvalidateRect( hwnd, NULL, TRUE );
            }
            break;

        case WM_CLOSE:
            DestroyShape();
            if( hbm != NULLHANDLE )
            {
                if( hpsDraw ) GpiSetBitmap( hpsDraw, NULLHANDLE );
                GpiDeleteBitmap( hbm );
                hbm = NULLHANDLE;
            }
            if( hpsDraw != NULLHANDLE && hpsDraw != hpsMask )
            { /* not the case here */ }
            if( hdcMem != NULLHANDLE ) { DevCloseDC( hdcMem ); hdcMem = NULLHANDLE; }
            if( hpsDraw != NULLHANDLE ) { GpiDestroyPS( hpsDraw ); hpsDraw = NULLHANDLE; hpsMask = NULLHANDLE; }
            WinDestroyWindow( hwnd );
            break;

        default:
            return WinDefWindowProc( hwnd, msg, mp1, mp2 );
    }
    return 0L;
}

int main( VOID )
{
    HMQ     hmq;
    QMSG    qmsg;
    HWND    hwndFrame;
    HWND    hwndClient;
    ULONG   flCreate;
    LONG    cyFrame;




    hab = WinInitialize( 0 );
    hmq = WinCreateMsgQueue( hab, 0 );
    WinRegisterClass( hab, "XpmShapeSmp", MainWndProc, 0L, 0 );

    flCreate = FCF_STANDARD & ~FCF_ICON & ~FCF_ACCELTABLE & ~FCF_MENU | FCF_NOBYTEALIGN;

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0L, &flCreate,
        "XpmShapeSmp", "EXPM Shaped Window Sample", 0L, 0, IDM_MENU, &hwndClient );

    cyFrame = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR )
            + (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYSIZEBORDER );

    WinSetWindowPos( hwndFrame, HWND_TOP, 100, 100,
        260, 140 + cyFrame, SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ZORDER );



    while( WinGetMsg( hab, &qmsg, 0, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}

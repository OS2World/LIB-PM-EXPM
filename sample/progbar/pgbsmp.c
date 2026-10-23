/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/progbar/pgbsmp.c -- XpmProgressBarClass progress bar sample.
 *
 * Demonstrates the AsuaCtl progress bar reimplemented on EXPM: smooth
 * and segmented indicators, a text (percentage) indicator, a vertical
 * indicator, the raised/depressed frames and the colour messages.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This sample is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WIN
#define INCL_WINMENUS
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <string.h>
#include "expm.h"

#define IDM_MENU        600
#define IDM_VIEW        601
#define IDM_DARK        602

#define ID_BAR_SMOOTH   100
#define ID_BAR_SEGMENT  101
#define ID_BAR_TEXT     102
#define ID_BAR_VERT     103

#define ID_BTN_STEP     200
#define ID_BTN_RESET    201
#define ID_BTN_RANGE    202
#define ID_BTN_COLORS   203
#define ID_BTN_ANIMATE  204

#define TIMER_ANIMATE   1

static HAB  hab;
static BOOL bAnimating = FALSE;
static BOOL bRecolored = FALSE;
static BOOL bDark = FALSE;
static HWND hwndThemeMenu = NULLHANDLE;

static HWND aBars[4];

static void SetRgb( HWND hwnd, ULONG idMode, ULONG ulColor )
{
    RGB2 rgb;

    rgb.fcOptions = 0;
    rgb.bBlue  = (BYTE)(ulColor & 0xFF);
    rgb.bGreen = (BYTE)((ulColor >> 8) & 0xFF);
    rgb.bRed   = (BYTE)((ulColor >> 16) & 0xFF);
    WinSetPresParam( hwnd, idMode, sizeof(rgb), &rgb );
}

static void ApplyPgbTheme( HWND hwnd )
{
    static const ULONG aBtn[5] =
        { ID_BTN_STEP, ID_BTN_RESET, ID_BTN_RANGE, ID_BTN_COLORS,
          ID_BTN_ANIMATE };
    static const ULONG aBar[4] =
        { ID_BAR_SMOOTH, ID_BAR_SEGMENT, ID_BAR_TEXT, ID_BAR_VERT };
    PXpmTheme pth = XpmQueryTheme();
    HWND      hCtl;
    int       i;

    for( i = 0; i < 5; i++ )
    {
        hCtl = WinWindowFromID( hwnd, aBtn[i] );
        if( hCtl == NULLHANDLE )
            continue;
        SetRgb( hCtl, PP_BACKGROUNDCOLOR, pth->clrCanvas );
        SetRgb( hCtl, PP_FOREGROUNDCOLOR, pth->clrText );
    }

    for( i = 0; i < 4; i++ )
    {
        hCtl = WinWindowFromID( hwnd, aBar[i] );
        if( hCtl == NULLHANDLE )
            continue;
        SetRgb( hCtl, PP_BACKGROUNDCOLOR, pth->clrSurface );
        SetRgb( hCtl, PP_FOREGROUNDCOLOR, pth->clrText );
    }
}

/* ------------------------------------------------------------------ */

static void CreateBar( HWND hwndClient, ULONG id, ULONG flStyle,
                       LONG x, LONG y, LONG cx, LONG cy )
{
    WinCreateWindow( hwndClient, XpmProgressBarClass, NULL,
                     WS_VISIBLE | flStyle,
                     x, y, cx, cy,
                     hwndClient, HWND_TOP, id, NULL, NULL );
}

static void CreateButton( HWND hwndClient, ULONG id, PSZ pszText,
                          LONG x, LONG y, LONG cx, LONG cy )
{
    WinCreateWindow( hwndClient, WC_BUTTON, pszText,
                     WS_VISIBLE | BS_PUSHBUTTON,
                     x, y, cx, cy,
                     hwndClient, HWND_TOP, id, NULL, NULL );
}

/* ------------------------------------------------------------------ */

static void StepAll( HWND hwndClient )
{
    int i;

    for( i = 0; i < 4; i++ )
        if( aBars[i] )
            WinSendMsg( aBars[i], XpmPBM_STEPIT, NULL, NULL );
}

static void ResetAll( HWND hwndClient )
{
    int i;

    for( i = 0; i < 4; i++ )
        if( aBars[i] )
            WinSendMsg( aBars[i], XpmPBM_SETPOS, MPFROMSHORT( 0 ), NULL );
}

static void SetRangeAll( HWND hwndClient )
{
    int i;

    for( i = 0; i < 4; i++ )
        if( aBars[i] )
            WinSendMsg( aBars[i], XpmPBM_SETRANGE,
                        MPFROMSHORT( 0 ), MPFROMSHORT( 50 ) );
}

static void Recolor( HWND hwndClient )
{
    int i;

    bRecolored = !bRecolored;

    for( i = 0; i < 4; i++ )
        if( aBars[i] )
        {
            WinSendMsg( aBars[i], XpmPBM_SETBARCOLOR,
                        MPFROMLONG( bRecolored ? 0x0000B000UL  /* green  */
                                               : 0x000080C0UL ), NULL );
            WinSendMsg( aBars[i], XpmPBM_SETBKCOLOR,
                        MPFROMLONG( bRecolored ? 0x00FFFFE0UL  /* cream  */
                                               : 0x00FFFFFFUL ), NULL );
        }
}

/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY ClientProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
            hwndThemeMenu = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ),
                                             FID_MENU);
            CreateBar( hwnd, ID_BAR_SMOOTH,
                       XpmPBS_SMOOTH | XpmPBS_RAISED, 75, 330, 495, 26 );
            CreateBar( hwnd, ID_BAR_SEGMENT,
                       XpmPBS_DEPRESSED, 75, 290, 495, 26 );
            CreateBar( hwnd, ID_BAR_TEXT,
                       XpmPBS_SMOOTH | XpmPBS_TEXT, 75, 250, 495, 26 );
            CreateBar( hwnd, ID_BAR_VERT,
                       XpmPBS_SMOOTH | XpmPBS_VERTICAL | XpmPBS_RAISED,
                       15, 80, 44, 150 );

            CreateButton( hwnd, ID_BTN_STEP,    "Step",        15, 15,  95, 34 );
            CreateButton( hwnd, ID_BTN_RESET,   "Reset",      120, 15,  95, 34 );
            CreateButton( hwnd, ID_BTN_RANGE,   "Range 0-50",
                          225, 15, 130, 34 );
            CreateButton( hwnd, ID_BTN_COLORS,  "Colours",    365, 15,  95, 34 );
            CreateButton( hwnd, ID_BTN_ANIMATE, "Animate",    470, 15,  95, 34 );

            aBars[0] = WinWindowFromID( hwnd, ID_BAR_SMOOTH );
            aBars[1] = WinWindowFromID( hwnd, ID_BAR_SEGMENT );
            aBars[2] = WinWindowFromID( hwnd, ID_BAR_TEXT );
            aBars[3] = WinWindowFromID( hwnd, ID_BAR_VERT );

            WinSendMsg( aBars[0], XpmPBM_SETPOS, MPFROMSHORT( 35 ), NULL );
            WinSendMsg( aBars[1], XpmPBM_SETPOS, MPFROMSHORT( 60 ), NULL );
            WinSendMsg( aBars[2], XpmPBM_SETPOS, MPFROMSHORT( 45 ), NULL );
            WinSendMsg( aBars[3], XpmPBM_SETPOS, MPFROMSHORT( 70 ), NULL );
            ApplyPgbTheme( hwnd );
            return (MRESULT)FALSE;

        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 ) )
            {
                case ID_BTN_STEP:
                    StepAll( hwnd );
                    return (MRESULT)FALSE;

                case ID_BTN_RESET:
                    ResetAll( hwnd );
                    return (MRESULT)FALSE;

                case ID_BTN_RANGE:
                    SetRangeAll( hwnd );
                    return (MRESULT)FALSE;

                case ID_BTN_COLORS:
                    Recolor( hwnd );
                    return (MRESULT)FALSE;

                case ID_BTN_ANIMATE:
                    bAnimating = !bAnimating;
                    if( bAnimating )
                        WinStartTimer( hab, hwnd, TIMER_ANIMATE, 100 );
                    else
                        WinStopTimer( hab, hwnd, TIMER_ANIMATE );
                    return (MRESULT)FALSE;

                case IDM_DARK:
                {
                    XpmTheme th;

                    bDark = !bDark;
                    if( bDark )
                        XpmThemeDark( &th );
                    else
                        XpmThemeDefault( &th );
                    XpmSetTheme( &th );
                    WinCheckMenuItem( hwndThemeMenu, IDM_DARK, bDark );
                    ApplyPgbTheme( hwnd );
                    WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
                    return (MRESULT)FALSE;
                }
            }
            break;

        case WM_TIMER:
            StepAll( hwnd );
            return (MRESULT)FALSE;

        case WM_ERASEBACKGROUND:
        {
            HPS   hps = (HPS)mp1;
            RECTL rcl;

            GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL );
            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, (LONG)XpmQueryTheme()->clrCanvas );
            return (MRESULT)TRUE;
        }

        case WM_PAINT:
        {
            HPS   hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );
            RECTL rcl;

            GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL );
            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, (LONG)XpmQueryTheme()->clrCanvas );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case XpmWM_THEMECHANGED:
            ApplyPgbTheme( hwnd );
            WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
            return (MRESULT)FALSE;

        case WM_CLOSE:
            WinPostMsg( hwnd, WM_QUIT, NULL, NULL );
            return (MRESULT)FALSE;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/* ------------------------------------------------------------------ */

int main( VOID )
{
    HMQ   hmq;
    HWND  hwndFrame;
    HWND  hwndClient;
    QMSG  qmsg;
    ULONG flCreate = FCF_SYSMENU | FCF_TITLEBAR | FCF_SIZEBORDER |
                     FCF_MINMAX | FCF_TASKLIST | FCF_MENU;

    hab = WinInitialize( 0 );
    hmq = WinCreateMsgQueue( hab, 0 );

    if( XpmControlsInit( hab ) != 0 )
    {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                       "Could not register the EXPM control classes.",
                       "EXPM progress bar sample", 0, MB_OK | MB_ERROR );
        WinDestroyMsgQueue( hmq );
        WinTerminate( hab );
        return 1;
    }

    WinRegisterClass( hab, "XpmPgbClient", ClientProc,
                      CS_SIZEREDRAW, 0 );

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flCreate,
                                    "XpmPgbClient",
                                    "EXPM progress bar sample",
                                    0, NULLHANDLE, IDM_MENU, &hwndClient );

    WinSetWindowPos( hwndFrame, HWND_TOP, 60, 110, 590, 440,
                     SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW );

    while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}

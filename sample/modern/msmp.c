/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/modern/msmp.c -- modern controls sample.
 *
 * Demonstrates the EXPM 0.3 modern controls: toggle switches, a
 * segmented control, a ring progress indicator and a spinner, chips and
 * icon buttons, plus the light/dark theme engine.  Owner notifications
 * are shown in the sample's status line.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This sample is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WIN
#define INCL_GPI
#define INCL_GPILOGCOLORTABLE
#define INCL_DOS

#include <os2.h>
#include <string.h>
#include <stdio.h>
#include "expm.h"

#define ID_TOG_WIFI     100
#define ID_TOG_BT       101
#define ID_SEG          110
#define ID_RING         120
#define ID_SPIN         121
#define ID_CHIP_FILTER  130
#define ID_CHIP_SORT    131
#define ID_CHIP_REMOVE  132
#define ID_IB_ADD       140
#define ID_IB_MORE      141
#define ID_IB_BOLD      142

#define ID_BTN_THEME    200
#define ID_BTN_TOGGLE   201

static HAB  hab;
static BOOL bDark = FALSE;
static char szStatus[128];

/* ------------------------------------------------------------------ */

static HWND CreateChild( HWND hwndClient, PCSZ pszClass, PCSZ pszText,
                         ULONG id, ULONG flStyle,
                         LONG x, LONG y, LONG cx, LONG cy, PVOID pCtl )
{
    return WinCreateWindow( hwndClient, pszClass, pszText,
                            WS_VISIBLE | flStyle,
                            x, y, cx, cy,
                            hwndClient, HWND_TOP, id, pCtl, NULL );
}

/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY ClientProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
        {
            static PCSZ apszSeg[] = { "Day", "Week", "Month", NULL };

            CreateChild( hwnd, XpmToggleSwitchClass, "Wi-Fi", ID_TOG_WIFI,
                         XpmTOG_AUTOSIZE | XpmTOG_CHECKED, 30, 360, 150, 24,
                         NULL );
            CreateChild( hwnd, XpmToggleSwitchClass, "Bluetooth",
                         ID_TOG_BT, XpmTOG_AUTOSIZE, 30, 328, 150, 24,
                         NULL );

            CreateChild( hwnd, XpmSegmentedClass, NULL, ID_SEG,
                         XpmSEG_EQUALWIDTH, 220, 356, 210, 28, NULL );
            WinSendMsg( WinWindowFromID( hwnd, ID_SEG ), XpmSEGM_SETITEMS,
                        MPFROMP( apszSeg ), NULL );

            CreateChild( hwnd, XpmRingProgressClass, NULL, ID_RING,
                         XpmRNG_TEXT, 30, 220, 100, 100, NULL );
            WinSendMsg( WinWindowFromID( hwnd, ID_RING ), XpmRNGM_SETRANGE,
                        MPFROMLONG( 0 ), MPFROMLONG( 100 ) );
            WinSendMsg( WinWindowFromID( hwnd, ID_RING ), XpmRNGM_SETPOS,
                        MPFROMLONG( 65 ), NULL );

            CreateChild( hwnd, XpmRingProgressClass, NULL, ID_SPIN,
                         XpmRNG_INDETERMINATE | XpmRNG_THICK,
                         150, 245, 50, 50, NULL );

            CreateChild( hwnd, XpmChipClass, "Filter", ID_CHIP_FILTER,
                         XpmCHIP_AUTOSIZE | XpmCHIP_SELECTABLE
                         | XpmCHIP_SELECTED, 30, 180, 90, 28, NULL );
            CreateChild( hwnd, XpmChipClass, "Sort", ID_CHIP_SORT,
                         XpmCHIP_AUTOSIZE | XpmCHIP_SELECTABLE,
                         130, 180, 90, 28, NULL );
            CreateChild( hwnd, XpmChipClass, "Remove", ID_CHIP_REMOVE,
                         XpmCHIP_AUTOSIZE | XpmCHIP_CLOSE,
                         230, 180, 100, 28, NULL );

            CreateChild( hwnd, XpmIconButtonClass, "+", ID_IB_ADD,
                         XpmIB_ROUND, 260, 245, 40, 40, NULL );
            CreateChild( hwnd, XpmIconButtonClass, "...", ID_IB_MORE,
                         XpmIB_FLAT, 316, 245, 52, 40, NULL );
            CreateChild( hwnd, XpmIconButtonClass, "B", ID_IB_BOLD,
                         XpmIB_ROUND | XpmIB_TOGGLE, 384, 245, 40, 40,
                         NULL );

            WinCreateWindow( hwnd, WC_BUTTON, "Dark theme", WS_VISIBLE,
                             30, 30, 120, 34, hwnd, HWND_TOP, ID_BTN_THEME,
                             NULL, NULL );
            WinCreateWindow( hwnd, WC_BUTTON, "Toggle Wi-Fi", WS_VISIBLE,
                             160, 30, 120, 34, hwnd, HWND_TOP, ID_BTN_TOGGLE,
                             NULL, NULL );

            strcpy( szStatus, "Ready." );
            return (MRESULT)FALSE;
        }

        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 ) )
            {
                case ID_BTN_THEME:
                {
                    XpmTheme th;
                    bDark = !bDark;
                    if( bDark )
                        XpmThemeDark( &th );
                    else
                        XpmThemeDefault( &th );
                    XpmSetTheme( &th );
                    strcpy( szStatus, bDark ? "Theme: dark"
                                            : "Theme: light" );
                    WinInvalidateRect( hwnd, NULL, FALSE );
                    return (MRESULT)FALSE;
                }

                case ID_BTN_TOGGLE:
                {
                    HWND hwndTog = WinWindowFromID( hwnd, ID_TOG_WIFI );
                    WinSendMsg( hwndTog, XpmTOGM_TOGGLE, NULL, NULL );
                    return (MRESULT)FALSE;
                }
            }
            break;

        case WM_CONTROL:
        {
            USHORT id   = SHORT1FROMMP( mp1 );
            USHORT code = SHORT2FROMMP( mp1 );
            LONG   val  = LONGFROMMP( mp2 );
            char   buf[96];

            switch( code )
            {
                case XpmTOGN_CHANGED:
                    sprintf( buf, "Toggle %u -> %s", (unsigned)id,
                             val ? "on" : "off" );
                    break;
                case XpmSEGN_SELECT:
                    sprintf( buf, "Segment selected: %ld", val );
                    break;
                case XpmCHIPN_SEL:
                    sprintf( buf, "Chip %u -> %s", (unsigned)id,
                             val ? "selected" : "cleared" );
                    break;
                case XpmCHIPN_CLOSE:
                    sprintf( buf, "Chip %u close clicked", (unsigned)id );
                    break;
                case XpmIBN_CLICK:
                    sprintf( buf, "Icon button %u clicked (%s)",
                             (unsigned)id, val ? "on" : "off" );
                    break;
                default:
                    return (MRESULT)FALSE;
            }

            strncpy( szStatus, buf, sizeof( szStatus ) - 1 );
            szStatus[sizeof( szStatus ) - 1] = '\0';
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;
        }

        case WM_PAINT:
        {
            HPS          hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );
            PXpmTheme    pth = XpmQueryTheme();
            RECTL        rcl, rclText;

            GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL );
            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, (LONG)pth->clrCanvas );

            rclText.xLeft   = 20;
            rclText.xRight  = rcl.xRight - 20;
            rclText.yBottom = 405;
            rclText.yTop    = rcl.yTop - 10;
            WinDrawText( hps, (LONG)strlen( "EXPM modern controls" ),
                         "EXPM modern controls", &rclText,
                         (LONG)pth->clrText, (LONG)pth->clrCanvas,
                         DT_LEFT | DT_VCENTER );

            rclText.xLeft   = 20;
            rclText.xRight  = rcl.xRight - 20;
            rclText.yBottom = 72;
            rclText.yTop    = 100;
            WinDrawText( hps, (LONG)strlen( szStatus ), szStatus, &rclText,
                         (LONG)pth->clrTextDim, (LONG)pth->clrCanvas,
                         DT_LEFT | DT_VCENTER );

            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

        case XpmWM_THEMECHANGED:
            WinInvalidateRect( hwnd, NULL, FALSE );
            return (MRESULT)FALSE;

        case WM_SIZE:
            WinInvalidateRect( hwnd, NULL, FALSE );
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
                     FCF_MINMAX | FCF_TASKLIST;

    hab = WinInitialize( 0 );
    hmq = WinCreateMsgQueue( hab, 0 );

    if( XpmModernInit( hab ) != 0 )
    {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                       "Could not register the EXPM modern control classes.",
                       "EXPM modern sample", 0, MB_OK | MB_ERROR );
        WinDestroyMsgQueue( hmq );
        WinTerminate( hab );
        return 1;
    }

    WinRegisterClass( hab, "XpmModernSmp", ClientProc,
                      CS_SIZEREDRAW, 0 );

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flCreate,
                                    "XpmModernSmp",
                                    "EXPM modern controls sample",
                                    0, NULLHANDLE, 0, &hwndClient );

    /* Keep the client's own paint from covering its child controls. */
    WinSetWindowBits( hwndClient, QWL_STYLE, WS_CLIPCHILDREN,
                      WS_CLIPCHILDREN );

    WinSetWindowPos( hwndFrame, HWND_TOP, 60, 110, 600, 470,
                     SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW );

    while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}

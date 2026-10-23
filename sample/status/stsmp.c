/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/status/stsmp.c -- XpmStatusClass status control sample.
 *
 * Demonstrates the AsuaCtl status control reimplemented on EXPM:
 * borders (standard and thin), horizontal alignment, automatic
 * scaling, the window grip, enable/disable and owner-drawn background.
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
#include <stdio.h>
#include "expm.h"

#define IDM_MENU            700
#define IDM_VIEW            701
#define IDM_DARK            702

#define ID_STATUS_LEFT      100
#define ID_STATUS_CENTER    101
#define ID_STATUS_RIGHT     102
#define ID_STATUS_OWNER     103
#define ID_STATUS_AUTO      104

#define ID_BTN_TOGGLE       200
#define ID_BTN_LONG         201
#define ID_BTN_QUERY        202

static HAB   hab;
static BOOL  bEnabled = TRUE;
static BOOL  bDark = FALSE;
static HWND  hwndThemeMenu = NULLHANDLE;

/* ------------------------------------------------------------------ */

static VOID SetRgb( HWND hwnd, ULONG idMode, ULONG ulColor )
{
    RGB2 rgb;

    rgb.fcOptions = 0;
    rgb.bBlue  = (BYTE)(ulColor & 0xFF);
    rgb.bGreen = (BYTE)((ulColor >> 8) & 0xFF);
    rgb.bRed   = (BYTE)((ulColor >> 16) & 0xFF);
    WinSetPresParam( hwnd, idMode, sizeof(rgb), &rgb );
}

static VOID ApplyStsTheme( HWND hwnd )
{
    static const ULONG aStatus[5] =
        { ID_STATUS_LEFT, ID_STATUS_CENTER, ID_STATUS_RIGHT,
          ID_STATUS_OWNER, ID_STATUS_AUTO };
    static const ULONG aBtn[3] =
        { ID_BTN_TOGGLE, ID_BTN_LONG, ID_BTN_QUERY };
    PXpmTheme pth = XpmQueryTheme();
    HWND      hCtl;
    int       i;

    for( i = 0; i < 5; i++ )
    {
        hCtl = WinWindowFromID( hwnd, aStatus[i] );
        if( hCtl == NULLHANDLE )
            continue;
        SetRgb( hCtl, PP_BACKGROUNDCOLOR, pth->clrSurface );
        SetRgb( hCtl, PP_FOREGROUNDCOLOR, pth->clrText );
    }

    for( i = 0; i < 3; i++ )
    {
        hCtl = WinWindowFromID( hwnd, aBtn[i] );
        if( hCtl == NULLHANDLE )
            continue;
        SetRgb( hCtl, PP_BACKGROUNDCOLOR, pth->clrCanvas );
        SetRgb( hCtl, PP_FOREGROUNDCOLOR, pth->clrText );
    }
}

/* ------------------------------------------------------------------ */

static VOID CreateStatus( HWND hwndClient, ULONG id, PSZ pszText,
                          ULONG flStyle, LONG x, LONG y, LONG cx, LONG cy )
{
    WinCreateWindow( hwndClient, XpmStatusClass, pszText,
                     WS_VISIBLE | flStyle,
                     x, y, cx, cy,
                     hwndClient, HWND_TOP, id, NULL, NULL );
}

static VOID CreateButton( HWND hwndClient, ULONG id, PSZ pszText,
                          LONG x, LONG y, LONG cx, LONG cy )
{
    WinCreateWindow( hwndClient, WC_BUTTON, pszText,
                     WS_VISIBLE | BS_PUSHBUTTON,
                     x, y, cx, cy,
                     hwndClient, HWND_TOP, id, NULL, NULL );
}

/* ------------------------------------------------------------------ */

static VOID ToggleEnabled( HWND hwndClient )
{
    ULONG aId[5];
    int   i;

    bEnabled = !bEnabled;

    aId[0] = ID_STATUS_LEFT;
    aId[1] = ID_STATUS_CENTER;
    aId[2] = ID_STATUS_RIGHT;
    aId[3] = ID_STATUS_OWNER;
    aId[4] = ID_STATUS_AUTO;

    for( i = 0; i < 5; i++ )
    {
        HWND hwnd = WinWindowFromID( hwndClient, aId[i] );

        if( hwnd )
            WinSendMsg( hwnd, XpmSM_SETHILITE, MPFROMLONG( bEnabled ), NULL );
    }
}

static VOID SetLongText( HWND hwndClient )
{
    HWND hwnd = WinWindowFromID( hwndClient, ID_STATUS_AUTO );

    if( hwnd )
        WinSetWindowText( hwnd,
                          "This text is deliberately far too long to fit "
                          "and is right-aligned by XpmSS_AUTOSHIFT." );
}

static VOID QuerySizes( HWND hwndClient )
{
    HWND  hwnd = WinWindowFromID( hwndClient, ID_STATUS_LEFT );
    CHAR  szBuf[256];
    LONG  lWidth;
    LONG  lHeight;
    LONG  lTextWidth;
    LONG  lTextHeight;
    LONG  lCharInc;

    if( !hwnd )
        return;

    lWidth      = (LONG)WinSendMsg( hwnd, XpmSM_QUERYWIDTH, NULL, NULL );
    lHeight     = (LONG)WinSendMsg( hwnd, XpmSM_QUERYHEIGHT, NULL, NULL );
    lTextWidth  = (LONG)WinSendMsg( hwnd, XpmSM_QUERYTEXTWIDTH, NULL, NULL );
    lTextHeight = (LONG)WinSendMsg( hwnd, XpmSM_QUERYTEXTHEIGHT, NULL, NULL );
    lCharInc    = (LONG)WinSendMsg( hwnd, XpmSM_QUERYMAXCHARINC, NULL, NULL );

    sprintf( szBuf,
             "Ideal width : %ld\n"
             "Ideal height: %ld\n"
             "Text width  : %ld\n"
             "Text height : %ld\n"
             "Max char inc: %ld",
             lWidth, lHeight, lTextWidth, lTextHeight, lCharInc );

    WinMessageBox( HWND_DESKTOP, hwndClient, szBuf,
                   "XpmStatus metrics", 0, MB_OK | MB_INFORMATION );
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
            CreateStatus( hwnd, ID_STATUS_LEFT, "Left aligned text",
                          XpmSS_BORDER | XpmSS_LEFT, 15, 250, 580, 26 );
            CreateStatus( hwnd, ID_STATUS_CENTER, "Centered text",
                          XpmSS_BORDER | XpmSS_CENTER, 15, 218, 580, 26 );
            CreateStatus( hwnd, ID_STATUS_RIGHT, "Right aligned text",
                          XpmSS_BORDER | XpmSS_RIGHT | XpmSS_THINFRAME,
                          15, 186, 580, 26 );
            CreateStatus( hwnd, ID_STATUS_OWNER,
                          "Owner-drawn background (XpmSS_OWNERDRAW)",
                          XpmSS_BORDER | XpmSS_LEFT | XpmSS_OWNERDRAW,
                          15, 154, 580, 26 );
            CreateStatus( hwnd, ID_STATUS_AUTO, "Auto-shift text",
                          XpmSS_BORDER | XpmSS_AUTOSHIFT, 15, 122, 580, 26 );

            CreateButton( hwnd, ID_BTN_TOGGLE, "Enable / Disable",
                          15, 15, 170, 34 );
            CreateButton( hwnd, ID_BTN_LONG, "Long text", 195, 15, 170, 34 );
            CreateButton( hwnd, ID_BTN_QUERY, "Query sizes",
                          375, 15, 170, 34 );
            ApplyStsTheme( hwnd );
            return (MRESULT)FALSE;

        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 ) )
            {
                case ID_BTN_TOGGLE:
                    ToggleEnabled( hwnd );
                    return (MRESULT)FALSE;

                case ID_BTN_LONG:
                    SetLongText( hwnd );
                    return (MRESULT)FALSE;

                case ID_BTN_QUERY:
                    QuerySizes( hwnd );
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
                    ApplyStsTheme( hwnd );
                    WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
                    return (MRESULT)FALSE;
                }
            }
            break;

        case WM_DRAWITEM:
        {
            ULONG         id  = (ULONG)SHORT1FROMMP( mp1 );
            PXpmOwnerBack pob = (PXpmOwnerBack)PVOIDFROMMP( mp2 );

            if( id == ID_STATUS_OWNER && pob )
            {
                WinFillRect( pob->hps, &pob->bound,
                             (LONG)XpmQueryTheme()->clrSurfaceDim );
                return (MRESULT)TRUE;
            }
            return (MRESULT)FALSE;
        }

        case WM_CONTROL:
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
            ApplyStsTheme( hwnd );
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
                       "EXPM status sample", 0, MB_OK | MB_ERROR );
        WinDestroyMsgQueue( hmq );
        WinTerminate( hab );
        return 1;
    }

    WinRegisterClass( hab, "XpmStsClient", ClientProc,
                      CS_SIZEREDRAW, 0 );

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flCreate,
                                    "XpmStsClient",
                                    "EXPM status control sample",
                                    0, NULLHANDLE, IDM_MENU, &hwndClient );

    WinSetWindowPos( hwndFrame, HWND_TOP, 60, 110, 620, 370,
                     SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW );

    while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}

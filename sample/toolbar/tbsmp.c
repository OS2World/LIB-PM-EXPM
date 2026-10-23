/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/toolbar/tbsmp.c -- XpmToolBarClass toolbar sample.
 *
 * Demonstrates the AsuaCtl toolbar reimplemented on EXPM: a toolbar
 * holding auto-sized window buttons, a standard push button, static
 * text and separators, with begin / centre / end alignment.  The toolbar
 * has a title bar (drag it to float the toolbar, click it to minimise,
 * click its small triangle to dock/undock) and a rotate button, and
 * shows tool-tips for the objects.  Clicks on the objects arrive as
 * WM_COMMAND; the toolbar's notifications arrive as WM_CONTROL.
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
#define INCL_GPILOGCOLORTABLE
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "expm.h"

#define IDM_MENU        800
#define IDM_VIEW        801
#define IDM_DARK        802

#define ID_TOOLBAR      1000
#define ID_STATUS       1100

#define IDB_NEW         2001
#define IDB_OPEN        2002
#define IDB_SAVE        2003
#define SEP_1           2004
#define IDB_STD         2005
#define SEP_2           2006
#define IDL_LABEL       2007
#define IDB_ROTATE      2008
#define IDB_EXIT        2009

static HAB  hab;
static HWND hwndStatus;
static BOOL bDark = FALSE;
static HWND hwndThemeMenu = NULLHANDLE;

static void SetRgb( HWND hwnd, ULONG idMode, ULONG ulColor )
{
    RGB2 rgb;

    rgb.fcOptions = 0;
    rgb.bBlue  = (BYTE)(ulColor & 0xFF);
    rgb.bGreen = (BYTE)((ulColor >> 8) & 0xFF);
    rgb.bRed   = (BYTE)((ulColor >> 16) & 0xFF);
    WinSetPresParam( hwnd, idMode, sizeof(rgb), &rgb );
}

static void ApplyTbTheme( HWND hwnd )
{
    static const ULONG aObj[7] =
        { IDB_NEW, IDB_OPEN, IDB_SAVE, IDB_STD, IDL_LABEL,
          IDB_ROTATE, IDB_EXIT };
    PXpmTheme pth = XpmQueryTheme();
    HWND      hwndTb;
    HWND      hCtl;
    int       i;

    if( hwndStatus )
    {
        SetRgb( hwndStatus, PP_BACKGROUNDCOLOR, pth->clrCanvas );
        SetRgb( hwndStatus, PP_FOREGROUNDCOLOR, pth->clrText );
    }

    hwndTb = WinWindowFromID( hwnd, ID_TOOLBAR );
    if( hwndTb == NULLHANDLE )
        return;

    SetRgb( hwndTb, PP_BACKGROUNDCOLOR, pth->clrCanvas );
    SetRgb( hwndTb, PP_FOREGROUNDCOLOR, pth->clrText );

    for( i = 0; i < 7; i++ )
    {
        hCtl = WinWindowFromID( hwndTb, aObj[i] );
        if( hCtl == NULLHANDLE )
            continue;
        SetRgb( hCtl, PP_BACKGROUNDCOLOR, pth->clrCanvas );
        SetRgb( hCtl, PP_FOREGROUNDCOLOR, pth->clrText );
    }
}

static void ShowStatus( PSZ pszText, ULONG id )
{
    CHAR szBuf[128];

    sprintf( szBuf, "%s (id=%lu)", pszText, (unsigned long)id );

    if( hwndStatus )
        WinSetWindowText( hwndStatus, szBuf );
}

/* Create one object on a toolbar. */
static BOOL InsertObject( HWND hwndToolBar, ULONG id, ULONG flAttr,
                          PSZ pszClass, PSZ pszName, ULONG flStyle,
                          ULONG cx, ULONG cy )
{
    XpmTOOLOBJ to;
    XpmTOOLWIN tw;

    memset( &to, 0, sizeof( to ) );
    memset( &tw, 0, sizeof( tw ) );

    to.iPosition   = XpmTB_END;
    to.ulGroup     = 1;
    to.flAttribute = flAttr;
    to.cx          = cx;
    to.cy          = cy;
    to.id          = id;

    tw.pszClass = pszClass;
    tw.pszName  = pszName;
    tw.flStyle  = flStyle;

    return (BOOL)WinSendMsg( hwndToolBar, XpmTM_INSERTOBJECT,
                             MPFROMP( &to ),
                             MPFROMP( pszClass ? &tw : NULL ) );
}

/* Attach a tool-tip string to a toolbar object. */
static void SetTip( HWND hwndToolBar, ULONG id, PSZ pszTip )
{
    WinSendMsg( hwndToolBar, XpmTM_INSERTTOOLTIP,
                MPFROMLONG( id ), MPFROMP( pszTip ) );
}

/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY ClientProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
        {
            XpmTOOLINFO ti;
            HWND        hwndTb;

            hwndThemeMenu = WinWindowFromID( WinQueryWindow( hwnd, QW_PARENT ),
                                             FID_MENU);

            hwndStatus = WinCreateWindow( hwnd, WC_STATIC, "Ready.",
                                          WS_VISIBLE | SS_TEXT,
                                          15, 8, 640, 26,
                                          hwnd, HWND_TOP, ID_STATUS,
                                          NULL, NULL );

            /* Docked horizontal toolbar at the top of the client area.
             * The title bar (move / minimise / dock), the rotate button
             * and the object tool-tips are enabled by the style.  The
             * owner (the client window) receives the object's
             * WM_COMMAND messages. */
            ti.cb          = (USHORT)sizeof( XpmTOOLINFO );
            ti.flToolAttrs = XpmTBA_FRAME | XpmTBA_FIXEDTOP;
            ti.usMaxLength = 0;

            hwndTb = WinCreateWindow( hwnd, XpmToolBarClass, "",
                                      WS_VISIBLE |
                                      XpmTBS_MOVEBUTTON |
                                      XpmTBS_ENABLEMINIMIZE |
                                      XpmTBS_FIXBUTTON |
                                      XpmTBS_ROTATEBUTTON,
                                      0, 0, 0, 0,
                                      hwnd, HWND_TOP, ID_TOOLBAR,
                                      &ti, NULL );

            InsertObject( hwndTb, IDB_NEW, XpmTBO_BUTTON | XpmTBO_BEGINALIGN,
                          XpmWinButtonClass, "New", 0, 0, 0 );
            InsertObject( hwndTb, IDB_OPEN, XpmTBO_BUTTON | XpmTBO_BEGINALIGN,
                          XpmWinButtonClass, "Open", 0, 0, 0 );
            InsertObject( hwndTb, IDB_SAVE, XpmTBO_BUTTON | XpmTBO_BEGINALIGN,
                          XpmWinButtonClass, "Save", 0, 0, 0 );
            InsertObject( hwndTb, SEP_1,
                          XpmTBO_SEPARATOR | XpmTBO_BEGINALIGN | XpmSPS_CONCAVE,
                          NULL, NULL, 0, 5, 5 );
            InsertObject( hwndTb, IDB_STD,
                          XpmTBO_STDBUTTON | XpmTBO_BEGINALIGN,
                          WC_BUTTON, "Standard", BS_PUSHBUTTON, 110, 28 );
            InsertObject( hwndTb, SEP_2,
                          XpmTBO_SEPARATOR | XpmTBO_CENTERALIGN | XpmSPS_LINE,
                          NULL, NULL, 0, 5, 5 );
            InsertObject( hwndTb, IDL_LABEL,
                          XpmTBO_STDWINDOW | XpmTBO_CENTERALIGN,
                          WC_STATIC, "Label", SS_TEXT, 70, 28 );
            InsertObject( hwndTb, IDB_ROTATE, XpmTBO_BUTTON | XpmTBO_ENDALIGN,
                          XpmWinButtonClass, "Rotate", 0, 0, 0 );
            InsertObject( hwndTb, IDB_EXIT, XpmTBO_BUTTON | XpmTBO_ENDALIGN,
                          XpmWinButtonClass, "Exit", 0, 0, 0 );

            /* Tool-tips shown after the pointer rests over an object. */
            SetTip( hwndTb, IDB_NEW,    "Create a new document" );
            SetTip( hwndTb, IDB_OPEN,   "Open an existing document" );
            SetTip( hwndTb, IDB_SAVE,   "Save the document" );
            SetTip( hwndTb, IDB_ROTATE, "Rotate the toolbar" );
            SetTip( hwndTb, IDB_EXIT,   "Close the sample" );

            /* Blend the label and the objects into the toolbar
             * background using the current theme.  A WC_STATIC with
             * SS_TEXT draws in the system foreground/background, so set
             * presentation colours (RGB, not colour indices). */
            ApplyTbTheme( hwnd );

            WinSendMsg( hwndTb, XpmTM_AUTOSIZE, MPFROMLONG( 0 ), NULL );
            return (MRESULT)FALSE;
        }

        case WM_COMMAND:
        {
            ULONG id   = SHORT1FROMMP( mp1 );
            ULONG code = SHORT2FROMMP( mp1 );

            if( id == IDM_DARK )
            {
                XpmTheme th;

                bDark = !bDark;
                if( bDark )
                    XpmThemeDark( &th );
                else
                    XpmThemeDefault( &th );
                XpmSetTheme( &th );
                WinCheckMenuItem( hwndThemeMenu, IDM_DARK, bDark );
                ApplyTbTheme( hwnd );
                WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
            }
            else if( id == IDB_EXIT )
            {
                WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
            }
            else if( id == IDB_ROTATE )
            {
                HWND  hwndTb = WinWindowFromID( hwnd, ID_TOOLBAR );
                ULONG st     = (ULONG)WinSendMsg( hwndTb, XpmTM_QUERYSTATE,
                                                  0, 0 );

                WinSendMsg( hwndTb, XpmTM_ROTATE,
                            MPFROMLONG( ( st & XpmTST_ROTATED )
                                        ? XpmTBR_HORIZONTAL
                                        : XpmTBR_VERTICAL ), 0 );
                WinSendMsg( hwndTb, XpmTM_AUTOSIZE, MPFROMLONG( 0 ), NULL );
                ShowStatus( ( st & XpmTST_ROTATED ) ? "Horizontal"
                                                    : "Vertical", id );
            }
            else if( code == XpmBN_CLICKED )
            {
                ShowStatus( "Object clicked", id );
            }
            return (MRESULT)FALSE;
        }

        case WM_CONTROL:
        {
            ULONG id   = SHORT1FROMMP( mp1 );
            ULONG code = SHORT2FROMMP( mp1 );
            PSZ   pszN = NULL;

            /* Only the toolbar itself sends XpmTN_* notifications; the
             * object controls send their own WM_CONTROL codes (which
             * overlap numerically), so ignore anything but id 1000. */
            if( id != ID_TOOLBAR )
                return (MRESULT)FALSE;

            switch( code )
            {
                case XpmTN_AUTOSIZE:  pszN = "TN_AUTOSIZE";  break;
                case XpmTN_FIXTOOL:   pszN = "TN_FIXTOOL";   break;
                case XpmTN_MINIMIZE:  pszN = "TN_MINIMIZE";  break;
                case XpmTN_RESTORE:   pszN = "TN_RESTORE";   break;
                case XpmTN_BEGINTRACK:pszN = "TN_BEGINTRACK";break;
                case XpmTN_TRACKING:  pszN = "TN_TRACKING";  break;
                case XpmTN_ENDTRACK:  pszN = "TN_ENDTRACK";  break;
                case XpmTN_ROTATE:    pszN = "TN_ROTATE";    break;
            }

            if( pszN )
                ShowStatus( pszN, id );
            return (MRESULT)FALSE;
        }

        case WM_SIZE:
            /* Re-dock the toolbar to the new client width. */
            WinSendMsg( WinWindowFromID( hwnd, ID_TOOLBAR ),
                        XpmTM_AUTOSIZE, MPFROMLONG( 0 ), NULL );
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
            ApplyTbTheme( hwnd );
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
                       "EXPM toolbar sample", 0, MB_OK | MB_ERROR );
        WinDestroyMsgQueue( hmq );
        WinTerminate( hab );
        return 1;
    }

    WinRegisterClass( hab, "XpmTbClient", ClientProc,
                      CS_SIZEREDRAW, 0 );

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flCreate,
                                    "XpmTbClient",
                                    "EXPM toolbar sample",
                                    0, NULLHANDLE, IDM_MENU, &hwndClient );

    /* Keep the client's own paint from covering its child controls. */
    WinSetWindowBits( hwndClient, QWL_STYLE, WS_CLIPCHILDREN,
                      WS_CLIPCHILDREN );

    WinSetWindowPos( hwndFrame, HWND_TOP, 60, 120, 680, 380,
                     SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW );

    while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}

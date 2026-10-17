/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/winbutt/wbsmp.c -- XpmWinButtonClass window button sample.
 *
 * Demonstrates the AsuaCtl window button reimplemented on EXPM: normal,
 * default, thin-frame, disabled and flat buttons, the four text
 * placements, the XpmBS_AUTOTEXT ellipsis, XpmBS_AUTOSIZE, an
 * owner-drawn background and the mouse enter/leave notifications.
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
#include <stdio.h>
#include <string.h>
#include "expm.h"

#define IDB_OK          100
#define IDB_CANCEL      101
#define IDB_DEFAULT     102
#define IDB_THIN        103
#define IDB_DISABLED    104
#define IDB_LEFT        110
#define IDB_RIGHT       111
#define IDB_TOP         112
#define IDB_BOTTOM      113
#define IDB_FLAT        120
#define IDB_FLAT2       121
#define IDB_AUTOTEXT    130
#define IDB_AUTOSIZE    131
#define IDB_OWNDRAW     140

#define ID_STATUS       200

static HAB  hab;
static HWND hwndStatus;

/* ------------------------------------------------------------------ */

static void CreateButton( HWND hwndClient, ULONG id, PSZ pszText,
                          ULONG flStyle, LONG x, LONG y, LONG cx, LONG cy )
{
    WinCreateWindow( hwndClient, XpmWinButtonClass, pszText,
                     WS_VISIBLE | flStyle,
                     x, y, cx, cy,
                     hwndClient, HWND_TOP, id, NULL, NULL );
}

static void ShowStatus( PSZ pszPrefix, ULONG id, ULONG code )
{
    CHAR szBuf[128];

    sprintf( szBuf, "%s: id=%lu code=%lu", pszPrefix,
             (unsigned long)id, (unsigned long)code );

    if( hwndStatus )
        WinSetWindowText( hwndStatus, szBuf );
}

/* Owner-draw background for IDB_OWNDRAW. */
static BOOL PaintOwnerItem( PXpmOwnerBack pob )
{
    GpiCreateLogColorTable( pob->hps, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );
    WinFillRect( pob->hps, &pob->bound, 0x00FFFFE0UL );   /* cream */
    return TRUE;
}

/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY ClientProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
            CreateButton( hwnd, IDB_OK, "OK", 0,
                          10, 250, 90, 30 );
            CreateButton( hwnd, IDB_CANCEL, "Cancel", 0,
                          110, 250, 90, 30 );
            CreateButton( hwnd, IDB_DEFAULT, "Default",
                          XpmBS_DEFAULT, 210, 250, 90, 30 );
            CreateButton( hwnd, IDB_THIN, "Thin",
                          XpmBS_THINFRAME, 310, 250, 90, 30 );
            CreateButton( hwnd, IDB_DISABLED, "Disabled", 0,
                          410, 250, 90, 30 );
            WinEnableWindow( WinWindowFromID( hwnd, IDB_DISABLED ), FALSE );

            CreateButton( hwnd, IDB_LEFT, "Left",
                          XpmBS_ALIGNLEFT, 10, 210, 110, 28 );
            CreateButton( hwnd, IDB_RIGHT, "Right",
                          XpmBS_ALIGNRIGHT, 130, 210, 110, 28 );
            CreateButton( hwnd, IDB_TOP, "Top",
                          XpmBS_ALIGNTOP, 250, 210, 110, 28 );
            CreateButton( hwnd, IDB_BOTTOM, "Bottom",
                          XpmBS_ALIGNBOTTOM, 370, 210, 110, 28 );

            CreateButton( hwnd, IDB_FLAT, "Flat",
                          XpmBS_FLATBUTTON, 10, 160, 150, 30 );
            CreateButton( hwnd, IDB_FLAT2, "Flat button",
                          XpmBS_FLATBUTTON, 170, 160, 150, 30 );

            CreateButton( hwnd, IDB_AUTOTEXT,
                          "A very long button label",
                          XpmBS_AUTOTEXT, 10, 110, 130, 30 );
            CreateButton( hwnd, IDB_AUTOSIZE, "Autosize",
                          XpmBS_AUTOSIZE, 10, 60, 10, 10 );

            CreateButton( hwnd, IDB_OWNDRAW, "OwnerDraw",
                          XpmBS_OWNERDRAW, 330, 160, 130, 30 );

            hwndStatus = WinCreateWindow( hwnd, WC_STATIC,
                                          "Ready.",
                                          WS_VISIBLE,
                                          10, 10, 490, 24,
                                          hwnd, HWND_TOP, ID_STATUS,
                                          NULL, NULL );
            return (MRESULT)FALSE;

        case WM_DRAWITEM:
        {
            ULONG id = SHORT1FROMMP( mp1 );

            if( id == IDB_OWNDRAW )
                return (MRESULT)PaintOwnerItem( (PXpmOwnerBack)PVOIDFROMMP( mp2 ) );
            break;
        }

        case WM_COMMAND:
        {
            ULONG id   = SHORT1FROMMP( mp1 );
            ULONG code = SHORT2FROMMP( mp1 );

            if( code == XpmBN_CLICKED )
                ShowStatus( "Click", id, code );
            else if( code == XpmBN_BUTTON2CLICKED )
                ShowStatus( "Right click", id, code );
            else if( code == XpmBN_DBLCLICKED )
                ShowStatus( "Double click", id, code );
            return (MRESULT)FALSE;
        }

        case WM_CONTROL:
        {
            ULONG id   = SHORT1FROMMP( mp1 );
            ULONG code = SHORT2FROMMP( mp1 );

            if( code == XpmBN_MOUSEENTER )
                ShowStatus( "Mouse enter", id, code );
            else if( code == XpmBN_MOUSELEAVE )
                ShowStatus( "Mouse leave", id, code );
            else if( code == XpmBN_PRESPARAMCHANGED )
                ShowStatus( "Presparam", id, code );
            return (MRESULT)FALSE;
        }

        case WM_PAINT:
        {
            HPS   hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );
            RECTL rcl;

            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, CLR_PALEGRAY );
            WinEndPaint( hps );
            return (MRESULT)FALSE;
        }

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

    if( XpmControlsInit( hab ) != 0 )
    {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                       "Could not register the EXPM control classes.",
                       "EXPM window button sample", 0, MB_OK | MB_ERROR );
        WinDestroyMsgQueue( hmq );
        WinTerminate( hab );
        return 1;
    }

    WinRegisterClass( hab, "XpmWbtClient", ClientProc,
                      CS_SIZEREDRAW, 0 );

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flCreate,
                                    "XpmWbtClient",
                                    "EXPM window button sample",
                                    0, NULLHANDLE, 0, &hwndClient );

    WinSetWindowPos( hwndFrame, HWND_TOP, 60, 120, 520, 340,
                     SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW );

    while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}

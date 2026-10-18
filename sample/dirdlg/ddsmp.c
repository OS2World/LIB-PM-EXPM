/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/dirdlg/ddsmp.c -- XpmDirDlg directory dialog sample.
 *
 * Demonstrates the AsuaCtl directory dialog reimplemented on EXPM: a
 * plain OK/Cancel dialog, the XpmWinButton variant, and the OK-only
 * variant.  The chosen directory (or the fact that the user cancelled)
 * is shown in a status field.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This sample is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#define INCL_WIN
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "expm.h"

#define IDB_PLAIN       100
#define IDB_WINBUTTON   101
#define IDB_OKONLY      102

#define IDL_RESULT      200

static HAB  hab;
static HWND hwndFrame;
static HWND hwndResult;

/* ------------------------------------------------------------------ */

static void ShowResult( PXpmDIRDLG pdd )
{
    CHAR szBuf[CCHMAXPATH + 64];

    if( pdd->lReturn == DID_OK )
        sprintf( szBuf, "OK: %s", pdd->szFullDirectoryName );
    else
        sprintf( szBuf, "Cancelled (lReturn=%ld).", (long)pdd->lReturn );

    if( hwndResult )
        WinSetWindowText( hwndResult, szBuf );
}

static void ChooseDirectory( HWND hwnd, LONG flDialogType, BOOL fWinButton )
{
    XpmDIRDLG dd;

    memset( &dd, 0, sizeof(dd) );
    dd.pszTitle        = "Choose a directory";
    dd.pszOKButton     = NULL;          /* keep the default labels        */
    dd.pszCancelButton = NULL;
    dd.pszInitialDir   = "C:\\";
    dd.flDialogType    = flDialogType;
    dd.flUseWinButton  = fWinButton;

    XpmDirDlg( HWND_DESKTOP, hwndFrame, &dd );

    ShowResult( &dd );
}

/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY ClientProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
            WinCreateWindow( hwnd, WC_BUTTON, "Choose directory...",
                             WS_VISIBLE | BS_PUSHBUTTON,
                             15, 250, 200, 34,
                             hwnd, HWND_TOP, IDB_PLAIN, NULL, NULL );

            WinCreateWindow( hwnd, WC_BUTTON, "Choose (XpmWinButton)",
                             WS_VISIBLE | BS_PUSHBUTTON,
                             230, 250, 230, 34,
                             hwnd, HWND_TOP, IDB_WINBUTTON, NULL, NULL );

            WinCreateWindow( hwnd, WC_BUTTON, "Choose (OK only)",
                             WS_VISIBLE | BS_PUSHBUTTON,
                             475, 250, 190, 34,
                             hwnd, HWND_TOP, IDB_OKONLY, NULL, NULL );

            hwndResult = WinCreateWindow( hwnd, WC_STATIC,
                                          "Ready.",
                                          WS_VISIBLE | SS_TEXT,
                                          15, 195, 650, 44,
                                          hwnd, HWND_TOP, IDL_RESULT,
                                          NULL, NULL );
            return (MRESULT)FALSE;

        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 ) )
            {
                case IDB_PLAIN:
                    ChooseDirectory( hwnd, XpmDIRDLG_OKCANCEL, FALSE );
                    break;
                case IDB_WINBUTTON:
                    ChooseDirectory( hwnd, XpmDIRDLG_OKCANCEL, TRUE );
                    break;
                case IDB_OKONLY:
                    ChooseDirectory( hwnd, XpmDIRDLG_OK, FALSE );
                    break;
            }
            return (MRESULT)FALSE;

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
                       "EXPM directory dialog sample", 0, MB_OK | MB_ERROR );
        WinDestroyMsgQueue( hmq );
        WinTerminate( hab );
        return 1;
    }

    WinRegisterClass( hab, "XpmDdClient", ClientProc, CS_SIZEREDRAW, 0 );

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flCreate,
                                    "XpmDdClient",
                                    "EXPM directory dialog sample",
                                    0, NULLHANDLE, 0, &hwndClient );

    WinSetWindowPos( hwndFrame, HWND_TOP, 60, 110, 700, 360,
                     SWP_MOVE | SWP_SIZE | SWP_ACTIVATE | SWP_SHOW );

    while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}

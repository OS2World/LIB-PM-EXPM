/*
 * sample/combo/combosmp.c -- EXPM COMBO sample application.
 *
 * Creates a standard window with two combo boxes:
 *   - combo 1 populated from a resource STRINGTABLE via
 *     XpmInitComboBoxFromRes
 *   - combo 2 populated from a hard-coded array via XpmInitComboBox
 *
 * Adapted from the COMBOSMP sample of LIB-PM-ComboBoxSupport 1.1
 * (public domain, by Dave Briccetti).
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#define  INCL_WINDIALOGS
#define  INCL_WINENTRYFIELDS
#define  INCL_WINLISTBOXES
#define  INCL_WINSYS
#define  INCL_WINWINDOWMGR
#define  INCL_WINMENUS
#define  INCL_GPI
#include <os2.h>

#include "combosmp.h"
#include "expm.h"

#define COMBOBOXWIDTH    100
#define COMBOBOXHEIGHT   100
#define COMBOBOXSPACING  10

/* Combo box notification codes (from os2win.h / pmwin.h) */
#define CBN_SELCHANGE        1
#define CBN_CLOSEUP          8
#define CBN_EFCHANGE         1

MRESULT EXPENTRY ComboSampWindowProc( HWND hwnd, ULONG msg,
    MPARAM mp1, MPARAM mp2 );

static HAB  hab;
static BOOL bDark;
static HWND hwndThemeMenu;
static HWND hwndCombo1;
static HWND hwndCombo2;

static void ApplyComboTheme( void )
{
    PXpmTheme pth = XpmQueryTheme();
    RGB2      rgb;
    LONG      c;

    c = (LONG)pth->clrCanvas;
    rgb.fcOptions = 0;
    rgb.bBlue  = (BYTE)(c & 0xFF);
    rgb.bGreen = (BYTE)((c >> 8) & 0xFF);
    rgb.bRed   = (BYTE)((c >> 16) & 0xFF);

    if( hwndCombo1 )
    {
        WinSetPresParam( hwndCombo1, PP_BACKGROUNDCOLOR, sizeof(rgb), &rgb );
        WinSetPresParam( hwndCombo1, PP_FOREGROUNDCOLOR, sizeof(rgb), &rgb );
    }
    if( hwndCombo2 )
    {
        WinSetPresParam( hwndCombo2, PP_BACKGROUNDCOLOR, sizeof(rgb), &rgb );
        WinSetPresParam( hwndCombo2, PP_FOREGROUNDCOLOR, sizeof(rgb), &rgb );
    }
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

    WinRegisterClass( hab, "EXPMComboSmp", ComboSampWindowProc, 0L, 0 );

    flCreate = FCF_STANDARD
             & ~FCF_ICON & ~FCF_ACCELTABLE
             | FCF_NOBYTEALIGN;

    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0L,
        &flCreate, "EXPMComboSmp", "EXPM Combo Sample", 0L,
        0, IDM_MENU, &hwndClient );

    hwndThemeMenu = WinWindowFromID( hwndFrame, FID_MENU );

    cyFrame = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR )
            + (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYSIZEBORDER );

    WinSetWindowPos( hwndFrame, HWND_TOP, 0, 0,
        2 * COMBOBOXWIDTH + 3 * COMBOBOXSPACING,
        COMBOBOXHEIGHT + 2 * COMBOBOXSPACING + cyFrame,
        SWP_SHOW | SWP_SIZE | SWP_ZORDER );

    while( WinGetMsg( hab, &qmsg, 0, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );

    return 0;
}


MRESULT EXPENTRY ComboSampWindowProc( HWND hwnd, ULONG msg,
    MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
        {
            static  LONG    aCombo1Init[] =
                { IDS_ITEM1, IDS_ITEM2, IDS_ITEM3, IDS_ITEM4 };
            static  PCSZ    apszCombo2Init[] =
                { "Item 1", "Item 2", "Item 3", "Item 4", 0 };

            hwndCombo1 = WinCreateWindow( hwnd, WC_COMBOBOX, "",
                CBS_DROPDOWNLIST, 0, 0, 0, 0,
                hwnd, HWND_TOP, ID_COMBO1, 0, 0 );

            hwndCombo2 = WinCreateWindow( hwnd, WC_COMBOBOX, "",
                CBS_DROPDOWNLIST, 0, 0, 0, 0,
                hwnd, HWND_TOP, ID_COMBO2, 0, 0 );

            XpmInitComboBoxFromRes( hab, 0, hwnd, ID_COMBO1,
                aCombo1Init,
                sizeof aCombo1Init / sizeof aCombo1Init[0],
                40, LIT_NONE );

            XpmInitComboBox( hwnd, ID_COMBO2, apszCombo2Init, 1 );

            ApplyComboTheme();
            break;
        }

        case WM_FOCUSCHANGE:
            break;

        case WM_SIZE:
            WinSetWindowPos( hwndCombo1, 0,
                COMBOBOXSPACING, COMBOBOXSPACING,
                COMBOBOXWIDTH, COMBOBOXHEIGHT,
                SWP_MOVE | SWP_SIZE | SWP_SHOW );
            WinSetWindowPos( hwndCombo2, 0,
                COMBOBOXWIDTH + 2 * COMBOBOXSPACING, COMBOBOXSPACING,
                COMBOBOXWIDTH, COMBOBOXHEIGHT,
                SWP_MOVE | SWP_SIZE | SWP_SHOW );
            break;

        case WM_ERASEBACKGROUND:
        {
            HPS         hps = (HPS)mp1;
            PXpmTheme   pth = XpmQueryTheme();
            RECTL       rcl;

            GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL );
            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, (LONG)pth->clrCanvas );
            return (MRESULT)TRUE;
        }

        case WM_PAINT:
        {
            HPS         hps = WinBeginPaint( hwnd, 0, 0 );
            PXpmTheme   pth = XpmQueryTheme();
            RECTL       rcl;

            GpiCreateLogColorTable( hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL );
            WinQueryWindowRect( hwnd, &rcl );
            WinFillRect( hps, &rcl, (LONG)pth->clrCanvas );
            WinEndPaint( hps );
            break;
        }

        case WM_COMMAND:
            if( SHORT1FROMMP( mp1 ) == IDM_DARK )
            {
                XpmTheme th;

                bDark = !bDark;
                if( bDark )
                    XpmThemeDark( &th );
                else
                    XpmThemeDefault( &th );
                XpmSetTheme( &th );
                WinCheckMenuItem( hwndThemeMenu, IDM_DARK, bDark );
                ApplyComboTheme();
                WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
                return (MRESULT)FALSE;
            }
            else if( SHORT1FROMMP( mp1 ) == ID_COMBO1 || SHORT1FROMMP( mp1 ) == ID_COMBO2 )
            {
                /* Handle combo box selection change - just invalidate to refresh */
                WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
            }
            break;

        case WM_CONTROL:
        {
            USHORT id   = SHORT1FROMMP( mp1 );
            USHORT code = SHORT2FROMMP( mp1 );

            if( id == ID_COMBO1 || id == ID_COMBO2 )
            {
                if( code == CBN_SELCHANGE )
                {
                    /* Combo box selection changed: repaint the client area. */
                    WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
                }
            }
            break;
        }

        case XpmWM_THEMECHANGED:
            ApplyComboTheme();
            WinInvalidateRect( hwnd, NULLHANDLE, TRUE );
            break;

        default:
            return WinDefWindowProc( hwnd, msg, mp1, mp2 );
    }

    return 0L;
}

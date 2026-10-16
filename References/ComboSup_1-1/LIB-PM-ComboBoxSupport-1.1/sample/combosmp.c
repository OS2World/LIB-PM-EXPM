/*
    COMBOSMP.C - Sample program to illustrate use of combo box support
                 routines

    Written by Dave Briccetti, CIS 74475,1072, 28-Jan-1990
    This code may be used for any purpose

 */

#define  INCL_WINDIALOGS
#define  INCL_WINENTRYFIELDS
#define  INCL_WINLISTBOXES
#define  INCL_WINSYS
#define  INCL_WINWINDOWMGR
#include <os2.h>

#include "combosmp.h"
#include "combosup.h"

#define ID_COMBO1   1
#define ID_COMBO2   2

#define COMBOBOXWIDTH   100
#define COMBOBOXHEIGHT  100
#define COMBOBOXSPACING 10

int main( VOID );
MRESULT EXPENTRY ComboSampWindowProc( HWND hwnd, ULONG msg,
    MPARAM mp1, MPARAM mp2 );

static  HAB     hab;


int main( VOID )
{
    HMQ     hmq;
    QMSG    qmsg;
    HWND    hwndFrame;
    HWND    hwndClient;
    ULONG   flCreate;
    LONG   cyFrame;

    hab = WinInitialize( 0 );           // Initialize
    hmq = WinCreateMsgQueue( hab, 0 );  // Create message queue

    WinRegisterClass( hab, "COMBOSMP",  // Register window class
        ComboSampWindowProc, 0L, 0 );

    flCreate = FCF_STANDARD             // Set frame creation flags
        & ~FCF_ICON & ~FCF_MENU & ~FCF_ACCELTABLE
        | FCF_NOBYTEALIGN;

    hwndFrame = WinCreateStdWindow(     // Create main window
        HWND_DESKTOP, 0L,
        &flCreate, "COMBOSMP", "", 0L,
        0, 1, &hwndClient );

    cyFrame = (LONG) WinQuerySysValue( // Find space required for title, border
        HWND_DESKTOP, SV_CYTITLEBAR ) +
        (LONG) WinQuerySysValue( HWND_DESKTOP,
        SV_CYSIZEBORDER );

    WinSetWindowPos( hwndFrame,         // Size main window
        HWND_TOP,
        0, 0, 2 * COMBOBOXWIDTH + 3 * COMBOBOXSPACING,
        COMBOBOXHEIGHT + 2 * COMBOBOXSPACING + cyFrame,
        SWP_SHOW | SWP_SIZE | SWP_ZORDER );

    while( WinGetMsg( hab, &qmsg,       // Process messages
        0, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyMsgQueue( hmq );          // Destroy message queue
    WinTerminate( hab );                // Terminate

    return 0;
}



MRESULT EXPENTRY ComboSampWindowProc( HWND hwnd, ULONG msg,
    MPARAM mp1, MPARAM mp2 )
{
    static  HWND    hwndCombo1;
    static  HWND    hwndCombo2;

    switch( msg )
    {
        case WM_CREATE:
        {
            static  SHORT   aCombo1Init[] =
                { IDS_ITEM1, IDS_ITEM2, IDS_ITEM3, IDS_ITEM4 };
            static  PSZ apszCombo2Init[] =
                { "Item 1", "Item 2", "Item 3", "Item 4", 0 };


            hwndCombo1 = WinCreateWindow( hwnd, WC_COMBOBOX, "",
                CBS_DROPDOWNLIST, 0, 0, 0, 0,
                hwnd, HWND_TOP, ID_COMBO1, 0, 0 );

            hwndCombo2 = WinCreateWindow( hwnd, WC_COMBOBOX, "",
                CBS_DROPDOWNLIST, 0, 0, 0, 0,
                hwnd, HWND_TOP, ID_COMBO2, 0, 0 );


            // Initialize combo box 1 with dynamically loaded values
            // from resource string table

            InitComboBoxFromRcStrings( hab, hwnd, ID_COMBO1,
                sizeof aCombo1Init / sizeof aCombo1Init[ 0 ],
                aCombo1Init, 100, 0, 0 );


            // Initialize combo box 2 with hard-coded values from array
            //  of string pointers

            InitComboBox( hwnd, ID_COMBO2, apszCombo2Init, 1 );


            break;
        }

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

        case WM_PAINT:
        {
            HPS hps = WinBeginPaint( hwnd, 0, 0 );
            GpiErase( hps );
            WinEndPaint( hps );
            break;
        }

        default:
            return WinDefWindowProc( hwnd, msg, mp1, mp2 );
    }

    return 0L;
}

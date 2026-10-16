#include <malloc.h>
#include <string.h>

#define  INCL_WINDIALOGS
#define  INCL_WINLISTBOXES
#define  INCL_WINENTRYFIELDS
#define  INCL_WINWINDOWMGR
#include <os2.h>

#include "combosup.h"



/* ------------------------------------------------------------------ */

USHORT EXPENTRY InitComboBox
(
    HWND hwnd,              // Dialog box or owner window handle
    SHORT idCombo,          // Combo box identifier
    PSZ *apszInit,          // Array of pointers to listbox strings
    SHORT InitialSelection  // Array index of initially selected
                            //  string, or value ICB_NOSELECTION
)
{
    SHORT   cItems = 0;
    PSZ *   apszInitItem = apszInit;

    while( *apszInitItem )              // Loop through init array
    {
        ++cItems;                       // Count number of items
        WinSendDlgItemMsg( hwnd,        // Insert item into combo listbox
            idCombo, LM_INSERTITEM,
            MPFROMSHORT( LIT_END ),
            *apszInitItem++ );
    }

    if( (InitialSelection != ICB_NOSELECTION)   // If a valid initial
    &&  (InitialSelection >= 0)             //  selection index is provided,
    &&  (InitialSelection < cItems) )       //  select that item
        WinSetDlgItemText( hwnd, idCombo,
            apszInit[ InitialSelection ] );

    return 0;
}



/* ------------------------------------------------------------------ */

USHORT  EXPENTRY InitComboBoxFromRcStrings
(
    HAB hab,                // Anchor block handle
    HWND hwnd,              // Dialog box or owner window handle
    SHORT idCombo,          // Combo box identifier
    SHORT cStrings,         // Count of strings to insert in combo box
    SHORT aidStrings[],     // Array of resource string identifiers
    SHORT cbStringMax,      // Maximum length of resource string
    HMODULE hmodResource,   // Resource module handle, or null
    SHORT InitialSelection  // Array index of initially selected
                            //  string, or value ICB_NOSELECTION
)
{
    PSZ     *apszInit;      // Combo box initialization structure
    PSZ     pszString;      // Buffer for loading individual strings
    SHORT   cbString;       // String length returned from WinLoadString
    SHORT   i;              // Array index for looping

    pszString = malloc(                  // Allocate WinLoadString buffer
        cbStringMax + 1 );
    apszInit = malloc(                  // Allocate array of pointers to
        sizeof (PSZ) * (cStrings + 1) );//  initialization strings
    apszInit[ cStrings ] = 0;           // Set last element to NULL

    for( i = 0; i < cStrings; ++i )     // Load all strings
    {
        cbString = WinLoadString( hab,  // Load string from resource file
            hmodResource, aidStrings[ i ],
            cbStringMax, pszString );
        apszInit[ i ] = malloc(         // Allocate buffer for this string
            cbString + 1 );             //  and point array element to it
        strcpy( apszInit[ i ], pszString ); // Copy string into buffer
    }

    InitComboBox( hwnd, idCombo,        // Initialize combo box
        apszInit, InitialSelection );

    for( i = 1; i <= cStrings; ++i )    // Free all string buffers
        free( apszInit[ i ] );

    free( apszInit );                   // Free array of string pointers
    free( pszString );                  // Free WinLoadString buffer

    return 0;
}

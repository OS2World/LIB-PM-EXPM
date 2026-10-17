/*
 * EXPM - Extended Presentation Manager library
 *
 * src/combosup/combosup.c -- combo box support module.
 *
 * Merged from LIB-PM-ComboBoxSupport 1.1 (public domain), originally
 * written by Dave Briccetti in 1990 as a 16-bit OS/2 PM library, then
 * ported to 32-bit OS/2.  The two entry points were renamed to the PM
 * naming style:
 *     InitComboBox             -> XpmInitComboBox
 *     InitComboBoxFromRcStrings -> XpmInitComboBoxFromRes
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#include <malloc.h>
#include <string.h>

#define INCL_WINDIALOGS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINWINDOWMGR
#include <os2.h>

#include "expm.h"


/* ------------------------------------------------------------------ */

USHORT EXPENTRY XpmInitComboBox( HWND  hwndOwner,
                                 ULONG idCombo,
                                 PCSZ *apszInit,
                                 LONG  iInitialSelection )
{
    PCSZ *apszInitItem = apszInit;
    LONG  cItems = 0;

    while( *apszInitItem )
    {
        cItems++;
        WinSendDlgItemMsg( hwndOwner, idCombo, LM_INSERTITEM,
            MPFROMSHORT( LIT_END ), MPFROMP( *apszInitItem++ ) );
    }

    if( (iInitialSelection != (LONG)LIT_NONE )
    &&  (iInitialSelection >= 0)
    &&  (iInitialSelection < cItems) )
        WinSetDlgItemText( hwndOwner, idCombo, (PSZ)apszInit[iInitialSelection] );

    return 0;
}


/* ------------------------------------------------------------------ */

USHORT EXPENTRY XpmInitComboBoxFromRes( HAB     hab,
                                        HMODULE hmodRes,
                                        HWND    hwndOwner,
                                        ULONG   idCombo,
                                        PLONG   alId,
                                        LONG    cIds,
                                        ULONG   cbStringMax,
                                        LONG    iInitialSelection )
{
    PCSZ *apszInit;
    PSZ   pszString;
    LONG  cbString;
    LONG  i;

    apszInit = (PCSZ *)malloc( sizeof(PCSZ) * ((size_t)cIds + 1) );
    if( apszInit == NULL )
        return 1;
    apszInit[ cIds ] = 0;

    pszString = (PSZ)malloc( (size_t)cbStringMax + 1 );
    if( pszString == NULL )
    {
        free( apszInit );
        return 1;
    }

    for( i = 0; i < cIds; i++ )
    {
        cbString = WinLoadString( hab, hmodRes, (ULONG)alId[i],
                                  (LONG)cbStringMax, pszString );
        apszInit[ i ] = (PCSZ)malloc( (size_t)cbString + 1 );
        if( apszInit[ i ] == NULL )
        {
            LONG j;
            for( j = 0; j < i; j++ )
                free( (PSZ)apszInit[j] );
            free( pszString );
            free( apszInit );
            return 1;
        }
        if( cbString <= 0 )
            *((PSZ)apszInit[i]) = 0;
        else
            strcpy( (PSZ)apszInit[i], pszString );
    }

    XpmInitComboBox( hwndOwner, idCombo, apszInit, iInitialSelection );

    for( i = 0; i < cIds; i++ )
        free( (PSZ)apszInit[i] );
    free( pszString );
    free( apszInit );

    return 0;
}

/*
 * EXPM - Extended Presentation Manager library
 *
 * src/ctlcommon/ctlcommon.c -- AsuaCtl shared helpers and version API.
 *
 * Clean-room reimplementation for the EXPM project (2026) of the
 * general-purpose routines of AsuaCtl 1.0 (asuactl.dll: pparams.c,
 * graphics.c and asuactl.c).  Written in C from the published AsuaCtl
 * headers and the IPF manual (doc/src/common/doc/commfunc.inc); no
 * AsuaCtl source was consulted.  AsuaCtl is GPLv2, EXPM is LGPLv3 (see
 * doc\ctlcommon.txt and doc\status.txt for the mapping of names).
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
 */

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <string.h>
#include "expm.h"

/* AsuaCtl 1.0 is the baseline this port tracks. */
#define XPM_CONTROLS_MAJOR  1
#define XPM_CONTROLS_MINOR  0

/* Anchor block captured by XpmControlsInit, used by
 * XpmGetComponentVersion to test whether a class is registered. */
static HAB g_habAnchor = NULLHANDLE;

/* ================================================================== */
/* Presentation parameter helpers                                     */
/* ================================================================== */

/* ------------------------------------------------------------------
 * XpmQueryPresColors -- build a colour table from a window's
 * presentation parameters.
 *
 * For every entry in ppt the function first tries WinQueryPresParam,
 * using the entry's QPF_* flags.  If that fails it falls back to the
 * SYSCLR_* system colour, and if none is given to the hard-coded RGB
 * value.  The result is written to lColTable.
 * ------------------------------------------------------------------ */
LONG EXPENTRY XpmQueryPresColors( HWND hwnd, LONG nColors,
                                  XpmPPTemplate *ppt, LONG *lColTable )
{
    LONG  i;
    ULONG ulColor;

    if( ppt == NULL || lColTable == NULL )
        return 0L;

    for( i = 0; i < nColors; i++ )
    {
        ulColor = 0;

        if( WinQueryPresParam( hwnd, ppt[i].ulPresParam, 0, NULL,
                               sizeof( ULONG ), (PVOID)&ulColor,
                               ppt[i].ulPresFlags ) )
        {
            lColTable[i] = (LONG)ulColor;
        }
        else if( ppt[i].ulSysColor != 0 )
        {
            lColTable[i] = WinQuerySysColor( HWND_DESKTOP,
                                             (LONG)ppt[i].ulSysColor, 0L );
        }
        else
        {
            lColTable[i] = (LONG)ppt[i].ulRgbColor;
        }
    }

    return 0L;
}

/* ------------------------------------------------------------------
 * XpmStorePresParams -- copy presentation parameters from a PRESPARAMS
 * list into a window.
 *
 * Font-size parameters (PP_FONTNAMESIZE) are always copied.  Colour
 * parameters are copied either unconditionally (ppt == NULL) or only
 * when present in the ppt filter table.  For each copied parameter the
 * matching WinSetPresParam is issued.
 * ------------------------------------------------------------------ */
LONG EXPENTRY XpmStorePresParams( HWND hwnd, PRESPARAMS *ppm,
                                  XpmPPTemplate *ppt, LONG nColors )
{
    PBYTE p;
    PBYTE pEnd;
    LONG  j;

    if( ppm == NULL )
        return 0L;

    p    = (PBYTE)&ppm->aparam[0];
    pEnd = (PBYTE)ppm + ppm->cb;

    while( p < pEnd )
    {
        PPARAM pp = (PPARAM)p;

        if( pp->id == PP_FONTNAMESIZE )
        {
            WinSetPresParam( hwnd, pp->id, pp->cb, (PVOID)pp->ab );
        }
        else if( ppt == NULL )
        {
            WinSetPresParam( hwnd, pp->id, pp->cb, (PVOID)pp->ab );
        }
        else
        {
            for( j = 0; j < nColors; j++ )
            {
                if( ppt[j].ulPresParam == pp->id )
                {
                    WinSetPresParam( hwnd, pp->id, pp->cb, (PVOID)pp->ab );
                    break;
                }
            }
        }

        p += sizeof( ULONG ) * 2 + pp->cb;
    }

    return 0L;
}

/* ================================================================== */
/* Graphics helper                                                    */
/* ================================================================== */

/* ------------------------------------------------------------------
 * XpmDrawBitmap -- draw a colour bitmap with an optional monochrome
 * transparency mask.  The mask is drawn first (its black bits set the
 * background) and the colour bitmap is then overlaid leaving the
 * background untouched, which yields a transparent bitmap.
 * ------------------------------------------------------------------ */
LONG EXPENTRY XpmDrawBitmap( HPS hps, HBITMAP hbmColor, HBITMAP hbmMono,
                             POINTL *pos, BOOL flPattern )
{
    POINTL ptl;

    if( pos )
        ptl = *pos;
    else
    {
        ptl.x = 0;
        ptl.y = 0;
    }

    GpiSetBackColor( hps, CLR_WHITE );
    GpiSetColor( hps, CLR_BLACK );
    GpiSetBackMix( hps, flPattern ? BM_OVERPAINT : BM_LEAVEALONE );

    if( hbmMono )
        WinDrawBitmap( hps, hbmMono, NULL, &ptl, CLR_BLACK, CLR_WHITE,
                       DBM_NORMAL );

    GpiSetBackMix( hps, BM_LEAVEALONE );
    WinDrawBitmap( hps, hbmColor, NULL, &ptl, CLR_BLACK, CLR_WHITE,
                   DBM_NORMAL );

    return 0L;
}

/* ================================================================== */
/* Version / initialisation API                                       */
/* ================================================================== */

LONG EXPENTRY XpmControlsInit( HAB hab )
{
    LONG rc = 0;

    g_habAnchor = hab;

    if( XpmStatusInit( hab ) != 0 )
        rc = XpmERR_PNL_CLASSREG;

    return rc;
}

LONG EXPENTRY XpmGetVersion( VOID )
{
    return ((LONG)XPM_CONTROLS_MAJOR << 16) | (LONG)XPM_CONTROLS_MINOR;
}

LONG EXPENTRY XpmGetComponentVersion( PSZ pszClass )
{
    CLASSINFO clsi;

    if( pszClass == (PSZ)NULL || g_habAnchor == NULLHANDLE )
        return 0L;

    if( !WinQueryClassInfo( g_habAnchor, pszClass, &clsi ) )
        return 0L;

    if( strcmp( pszClass, XpmStatusClass ) == 0 )
        return ((LONG)XPM_CONTROLS_MAJOR << 16) | (LONG)XPM_CONTROLS_MINOR;

    return 0L;
}

/*
 * EXPM - Extended Presentation Manager library
 *
 * src/fonts/fonts.c -- create a logical font bound to an image (raster)
 * font and report the metrics of that font.
 *
 * Clean-room reimplementation for the EXPM project (2026) of the font
 * helper of AsuaCtl 1.0 (asuactl.dll: fonts.c).  Written in C from the
 * published AsuaCtl header (include/fonts.h) and the OS/2 GPI font API
 * (GpiQueryFonts / GpiCreateLogFont) only; no AsuaCtl source was
 * consulted.  AsuaCtl is GPLv2, EXPM is LGPLv3.
 *
 * The upstream entry point FntCreateRasterFont is renamed to the PM
 * Win* style:
 *     FntCreateRasterFont  ->  XpmCreateRasterFont
 * and the upstream ERR_FNT_* codes become XpmERR_FNT_*.
 * See doc/fonts.txt for the API reference.
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

#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include "expm.h"


/* Largest point size accepted while parsing pszNameSize, so a
 * malformed string cannot overflow the decipoint conversion. */
#define FNT_MAXPOINTS   999

/* Highest usable local id (GpiCreateLogFont reserves 0 and LCID_ALL). */
#define FNT_MAXLCID     254


/* ------------------------------------------------------------------
 * FntSplitName -- split "[pointsize.]facename" into its two parts.
 *
 * The point size is an optional run of decimal digits followed by a
 * period; everything from the period on is the face name.  When no
 * point size is present *plPoints is set to 0 and the whole string is
 * taken as the face name.  The face name is copied to szFace and
 * truncated to FACESIZE-1 characters.
 * ------------------------------------------------------------------ */
static VOID FntSplitName( PSZ pszNameSize, PLONG plPoints, PSZ szFace )
{
    PSZ  p = pszNameSize;
    LONG lPoints = 0;

    while( *p == ' ' )
        p++;

    if( *p >= '0' && *p <= '9' )
    {
        PSZ pDigits = p;

        while( *p >= '0' && *p <= '9' )
        {
            lPoints = lPoints * 10 + (LONG)( *p - '0' );
            if( lPoints > FNT_MAXPOINTS )
                lPoints = FNT_MAXPOINTS;
            p++;
        }

        if( *p == '.' )
        {
            p++;                        /* skip the size/name separator */
        }
        else
        {
            lPoints = 0;                /* digits without a period */
            p = pDigits;                /* treat the whole string as a */
        }                               /* face name                    */
    }

    strncpy( szFace, p, FACESIZE - 1 );
    szFace[FACESIZE - 1] = '\0';

    *plPoints = lPoints;
}


/* ------------------------------------------------------------------
 * XpmCreateRasterFont -- create a logical font bound to lLcid, pinned
 * to the image font of the requested face whose nominal point size is
 * closest to the requested one, and report that font's metrics.
 *
 * Returns 0 on success or an XpmERR_FNT_* code.  The logical font is
 * created but not selected; the caller selects it with GpiSetCharSet.
 * ------------------------------------------------------------------ */
LONG EXPENTRY XpmCreateRasterFont( HPS hps, PSZ pszNameSize,
                                   LONG lLcid, FONTMETRICS *pfm )
{
    HDC          hdc;
    PFONTMETRICS pMetrics;
    LONG         lTotal;
    LONG         lCount;
    LONG         lGot;
    LONG         i;
    LONG         lPoints;
    LONG         lWant;
    LONG         lDiff;
    LONG         lBest;
    LONG         lBestDiff;
    CHAR         szFace[FACESIZE];
    FATTRS       fat;
    LONG         rc;

    if( hps == NULLHANDLE || pszNameSize == (PSZ)NULL ||
        *pszNameSize == '\0' || pfm == (FONTMETRICS *)NULL ||
        lLcid < 1 || lLcid > FNT_MAXLCID )
    {
        return XpmERR_FNT_INVPARAM;
    }

    /* The font list comes from the device behind the presentation
     * space; fail cleanly when it cannot be read. */
    hdc = GpiQueryDevice( hps );
    if( hdc == NULLHANDLE || hdc == HDC_ERROR )
        return XpmERR_FNT_HDC;

    FntSplitName( pszNameSize, &lPoints, szFace );
    lWant = lPoints * 10;               /* points -> decipoints */

    /* First call (empty buffer) reports how many public fonts match. */
    lCount = 0;
    lTotal = GpiQueryFonts( hps, QF_PUBLIC, szFace, &lCount, 0, NULL );
    if( lTotal <= 0 )
        return XpmERR_FNT_CREATE;       /* no such public face */

    pMetrics = (PFONTMETRICS)malloc( (size_t)lTotal * sizeof( FONTMETRICS ) );
    if( pMetrics == (PFONTMETRICS)NULL )
        return XpmERR_FNT_OUTMEM;

    /* Second call fills the array.  The number of structures actually
     * written is returned in lCount; the function's own return value is
     * the number of fonts left to enumerate (0 when all fitted), so it
     * must not be used as the count. */
    lCount = lTotal;
    GpiQueryFonts( hps, QF_PUBLIC, szFace, &lCount,
                   sizeof( FONTMETRICS ), pMetrics );
    lGot = lCount;
    if( lGot <= 0 )
    {
        free( pMetrics );
        return XpmERR_FNT_CREATE;
    }

    /* Choose an image font: skip outline fonts and pick the nominal
     * point size nearest to the requested one (the first image font
     * when no size was given). */
    lBest     = -1;
    lBestDiff = 0;

    for( i = 0; i < lGot; i++ )
    {
        if( pMetrics[i].fsDefn & FM_DEFN_OUTLINE )
            continue;

        if( lPoints == 0 )
        {
            lBest = i;
            break;
        }

        lDiff = (LONG)pMetrics[i].sNominalPointSize - lWant;
        if( lDiff < 0 )
            lDiff = -lDiff;

        if( lBest < 0 || lDiff < lBestDiff )
        {
            lBest     = i;
            lBestDiff = lDiff;
        }
    }

    if( lBest < 0 )
    {
        free( pMetrics );
        return XpmERR_FNT_CREATE;       /* only outline fonts available */
    }

    /* Pin the chosen physical font in a logical-font attribute block. */
    memset( &fat, 0, sizeof( fat ) );
    fat.usRecordLength  = (USHORT)sizeof( FATTRS );
    fat.fsSelection     = 0;
    fat.lMatch          = pMetrics[lBest].lMatch;
    strncpy( fat.szFacename, pMetrics[lBest].szFacename, FACESIZE - 1 );
    fat.szFacename[FACESIZE - 1] = '\0';
    fat.idRegistry      = pMetrics[lBest].idRegistry;
    fat.usCodePage      = pMetrics[lBest].usCodePage;
    fat.lMaxBaselineExt = pMetrics[lBest].lMaxBaselineExt;
    fat.lAveCharWidth   = pMetrics[lBest].lAveCharWidth;
    fat.fsType          = 0;
    fat.fsFontUse       = 0;            /* image font */

    *pfm = pMetrics[lBest];

    free( pMetrics );

    rc = GpiCreateLogFont( hps, NULL, lLcid, &fat );
    if( rc == GPI_ERROR )
        return XpmERR_FNT_CREATE;

    return 0;
}

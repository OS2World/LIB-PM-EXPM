/*
 * EXPM - Extended Presentation Manager library
 *
 * src/owndraw/owndraw.c -- owner-drawn listbox column alignment.
 *
 * Implements two helpers that extend the PM listbox control to align
 * text in fixed columns when a proportional-spaced font is in use.  A
 * listbox with the LS_OWNERDRAW style sends WM_MEASUREITEM once per item
 * and WM_DRAWITEM each time an item must be painted; the two helpers
 * answer those messages.  Column widths are expressed as character-count
 * offsets from the left edge (a caller-supplied tab-stop array such as
 * {10, 20, 35, 0}); the library translates those character positions to
 * pixel offsets using the font metric obtained at measure time.
 *
 * Merged from DEV-SAMPLES-C-LIB-PM-Owner_Drawn_Listbox, which is
 * distributed under the BSD 3-Clause License.  The upstream entry points
 * are renamed to the EXPM Xpm* style:
 *
 *     ODInitLBWidthHeight  ->  XpmInitLBWidthHeight
 *     ODDrawLBItems        ->  XpmDrawLBItems
 *
 * See doc/owndraw.txt for the API reference.
 *
 * Copyright (c) 1992 ASH Software, Inc.
 * Updates (c) 2026 Martin Iturbide / OS2World
 *
 * Distributed under the BSD 3-Clause License:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <string.h>
#include "expm.h"

/*=========================================================================
 * XpmInitLBWidthHeight - initialise column width/height for an
 * owner-draw listbox.
 *
 * Call from the WM_MEASUREITEM handler.  Queries the current font
 * metrics from the listbox presentation space, stores the per-character
 * width for later use by XpmDrawLBItems, and returns the item dimensions
 * that PM needs to allocate space for each row.
 *
 *   hListBox    handle to the owner-draw listbox window
 *   sfCharWidth AVERAGE_WIDTH: use lAveCharWidth (tighter packing)
 *               MAXIMUM_WIDTH: use lMaxCharInc (guaranteed no overlap)
 *   sCharMax    maximum number of characters in any item string; used to
 *               compute the total item width
 *   lCharWidth  OUT: receives the pixel width chosen by sfCharWidth;
 *               pass this value to every XpmDrawLBItems call
 *
 * Returns the item height (lMaxBaselineExt) as an MRESULT.  Return this
 * value directly from the WM_MEASUREITEM handler.  sCharMax is retained
 * for source compatibility with the original ASH Software interface.
 *=========================================================================*/
MRESULT EXPENTRY XpmInitLBWidthHeight(HWND  hListBox,
                                      SHORT sfCharWidth,
                                      SHORT sCharMax,
                                      LONG *lCharWidth)
{
    SHORT       sCharWidth;
    FONTMETRICS fm;
    HPS         hps;

    hps = WinGetPS(hListBox);
    GpiQueryFontMetrics(hps, (LONG)sizeof(FONTMETRICS), &fm);
    WinReleasePS(hps);

    if (sfCharWidth == AVERAGE_WIDTH)
        sCharWidth = (SHORT)fm.lAveCharWidth;
    else
        sCharWidth = (SHORT)fm.lMaxCharInc;

    *lCharWidth = (LONG)sCharWidth;

    /* WM_MEASUREITEM must return the item height (all items in a list
     * box share one height).  The width is not communicated here; the
     * list box is as wide as its window.  The per-character width needed
     * for column alignment is returned via *lCharWidth instead. */
    return (MRESULT)fm.lMaxBaselineExt;
}

/*=========================================================================
 * XpmDrawLBItems - draw one owner-draw listbox item with column
 * alignment.
 *
 * Call from the WM_DRAWITEM handler.  Retrieves the item text, expands
 * embedded tab characters into spaces, then renders each column segment
 * at its correct pixel offset so that columns align regardless of the
 * proportional font in use.
 *
 *   pOwnerItem  pointer to the OWNERITEM structure from mp2
 *   lCharWidth  per-character pixel width from XpmInitLBWidthHeight
 *   psTabStops  1-based column array, e.g. {10,20,35,0}; terminated by
 *               0.  Column 1 is the leftmost character position.
 *
 * Returns (MRESULT)TRUE - the item text has been drawn; default
 * highlighting is performed by the system after this function returns.
 *
 * Notes:
 *   - Only redraws when pOwnerItem->fsState == pOwnerItem->fsStateOld
 *     (i.e. the item content, not just the selection highlight, changed).
 *   - The tab-stop array must remain valid for the lifetime of the
 *     listbox; declare it static or at file scope.
 *   - Item strings longer than MAX_TEXT_CHARS characters are truncated.
 *=========================================================================*/
MRESULT EXPENTRY XpmDrawLBItems(POWNERITEM pOwnerItem,
                                LONG       lCharWidth,
                                SHORT     *psTabStops)
{
    SHORT sTab;
    SHORT sLoop;
    SHORT sNumberOfChars;
    SHORT sTabCharPos;
    SHORT sNextCharPos;
    SHORT sPrevCharPos;
    LONG  lSaveXLeft;
    CHAR  cChar;
    CHAR  cTempReadString[MAX_TEXT_CHARS];
    CHAR  cTempString[MAX_TEXT_CHARS];

    /* Only repaint when the item content (not just highlight) changes.
     * The system handles highlight transitions on its own. */
    if (pOwnerItem->fsState != pOwnerItem->fsStateOld)
        return (MRESULT)TRUE;


    /* Fetch the item text from the listbox. */
    WinSendMsg(pOwnerItem->hwnd,
               LM_QUERYITEMTEXT,
               MPFROM2SHORT(pOwnerItem->idItem, MAX_TEXT_CHARS),
               MPFROMP(cTempReadString));

    lSaveXLeft = pOwnerItem->rclItem.xLeft;

    /* --- Pass 1: expand embedded tab characters into spaces ---
     *
     * Walk cTempReadString character by character.  When a '\t' is
     * encountered, advance sNextCharPos to the next tab stop boundary by
     * inserting spaces.  Non-tab characters are copied as-is.  The
     * result is stored in cTempString, ready for column drawing. */
    sNextCharPos = 0;
    for (sLoop = 0; sLoop <= (SHORT)strlen(cTempReadString); sLoop++)
    {
        cChar = cTempReadString[sLoop];
        if (cChar == '\t')
        {
            /* Find the first tab stop beyond the current position. */
            sTab = -1;
            do {
                sTab++;
                sTabCharPos = psTabStops[sTab];
            } while ((sTabCharPos < sNextCharPos) && (sTabCharPos != 0));

            /* Convert to 0-based and pad with spaces up to that column. */
            sTabCharPos--;
            for (; sNextCharPos < sTabCharPos; sNextCharPos++)
                cTempString[sNextCharPos] = ' ';
        }
        else
        {
            cTempString[sNextCharPos] = cChar;
            sNextCharPos++;
        }
    }

    /* --- Pass 2: draw each column segment at its pixel offset ---
     *
     * Iterate through the tab-stop array.  For each column, advance
     * xLeft by the character offset of the column start, then call
     * WinDrawText for the characters belonging to that column.  The
     * final column (tab stop == 0) gets sNumberOfChars = 0x7FFF so
     * WinDrawText draws the remaining text to the right edge. */
    sTab         = -1;
    sNextCharPos =  1;   /* 1-based; the first column starts at position 1 */
    do {
        sTab++;
        sPrevCharPos = sNextCharPos;
        sNextCharPos = psTabStops[sTab];

        if (sNextCharPos > 0)
            sNumberOfChars = sNextCharPos - sPrevCharPos;
        else
            /* Last column: draw the remaining characters of the expanded
             * string.  WinDrawText uses cchText as an exact count, so a
             * "large" value would make it scan past the buffer. */
            sNumberOfChars = (SHORT)((SHORT)strlen(cTempString) -
                                     (sPrevCharPos - 1));

        /* Position each column at an absolute pixel offset from the
         * saved left edge.  sPrevCharPos is a 1-based character column,
         * so column 1 lands at the left edge (offset 0) and column n at
         * (n-1) character widths.  Setting (rather than accumulating)
         * xLeft keeps the columns from drifting apart. */
        pOwnerItem->rclItem.xLeft = lSaveXLeft +
                                    (LONG)(sPrevCharPos - 1) * lCharWidth;

        WinDrawText(pOwnerItem->hps,
                    sNumberOfChars,
                    (CHAR *)&cTempString[sPrevCharPos - 1],
                    &pOwnerItem->rclItem,
                    0, 0,
                    DT_LEFT | DT_VCENTER | DT_ERASERECT | DT_TEXTATTRS);

    } while (psTabStops[sTab] > 0);

    /* Restore xLeft so the system can apply default highlighting. */
    pOwnerItem->rclItem.xLeft = lSaveXLeft;

    /* Clear the old-state flag so highlighting is applied next time. */
    if (pOwnerItem->fsState)
        pOwnerItem->fsStateOld = FALSE;

    return (MRESULT)TRUE;
}

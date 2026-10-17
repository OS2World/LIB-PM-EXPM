/*
 * EXPM - Extended Presentation Manager library
 *
 * h/arrowhead.h -- ArrowHead module public interface.
 *
 * Clean-room reimplementation of the ArrowHead API by David Kopp,
 * written from its published header and readme (see
 * src/arrowhead/arrowhead.c).  The single entry point was renamed to
 * the PM style:
 *
 *     ArrowHead  ->  XpmDrawArrowHead
 *
 * This header contains the prototype only; it is included by h/expm.h,
 * so include that instead of this file in applications.
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

#ifndef EXPM_ARROWHEAD_H
#define EXPM_ARROWHEAD_H

/* Make the header self-contained: include os2.h with the GPI subset if
 * the caller has not already done so.  The OS2_INCLUDED guard is set by
 * os2.h itself, so this block is skipped when the caller already
 * included it (h/expm.h does). */
#ifndef OS2_INCLUDED
  #define INCL_GPI
  #include <os2.h>
#endif

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

/* XpmDrawArrowHead -- draw a two-line arrowhead at the tip of a line.
 *
 * Draws two lines that form the "V" of an arrowhead pointing from
 * ptStart toward ptEnd.  The arrowhead is positioned at ptEnd and drawn
 * using the current line attributes (color, width, type) of the
 * presentation space.
 *
 *   hps      presentation space to draw into
 *   ptStart  starting point of the line (tail end; not drawn here)
 *   ptEnd    ending point of the line (tip of the arrowhead)
 *   lLength  length of each arrowhead line, in world-coordinate units
 *   dAngle   half-angle of the arrowhead in degrees; each arrowhead line
 *            is offset by this angle from the main line direction.
 *            A value of 30 gives a narrow head; 45 gives a wide one.
 *   bFill    TRUE to fill the arrowhead triangle as a closed area using
 *            GpiBeginArea/GpiEndArea; FALSE to draw the two open lines
 *            only.
 *
 * Notes:
 *   - When ptStart == ptEnd the function draws nothing (degenerate line).
 *   - The fill uses GPI area attributes (pattern, color) set on the hps.
 *
 * (Upstream name: ArrowHead) */
VOID EXPENTRY XpmDrawArrowHead( HPS    hps,
                                POINTL ptStart,
                                POINTL ptEnd,
                                LONG   lLength,
                                double dAngle,
                                BOOL   bFill );

#ifdef __cplusplus
}
#endif

#endif /* RC_INVOKED */

#endif /* EXPM_ARROWHEAD_H */

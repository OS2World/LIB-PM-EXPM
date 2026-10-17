/*
 * EXPM - Extended Presentation Manager library
 *
 * src/arrowhead/arrowhead.c -- draw a two-line arrowhead at the tip of a
 * GPI line.
 *
 * Clean-room reimplementation of the ArrowHead API by David Kopp,
 * written from the published header (arrow.h) and the readme of the
 * shareware ArrowHead 1.1 package only; no third-party source is used
 * (see doc/arrowhead.txt).  The entry point follows the EXPM PM-style
 * naming:
 *
 *     ArrowHead  ->  XpmDrawArrowHead
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

#define INCL_GPI
#include <os2.h>

#include <math.h>

#include "arrowhead.h"


static const double dPi = 3.14159265358979;


/*=========================================================================
 * XpmDrawArrowHead - Draw a two-line arrowhead at the tip of a line.
 * (See arrowhead.h for the full parameter description.)
 * (Upstream name: ArrowHead)
 *=========================================================================*/
VOID EXPENTRY XpmDrawArrowHead(HPS hps, POINTL ptStart, POINTL ptEnd,
                               LONG lLength, double dAngle, BOOL bFill)
{
   double dx, dy;      /* vector from the tip back to the tail   */
   double len;         /* length of that vector                  */
   double ux, uy;      /* unit vector along that direction       */
   double half;        /* half-angle converted to radians        */
   double ca, sa;      /* cosine / sine of the half-angle        */
   POINTL pt1, pt2;    /* endpoints of the two arrowhead lines   */

   /* The arrowhead barbs point back from the tip toward the tail, so the
    * reference direction is ptStart - ptEnd (not ptEnd - ptStart). */
   dx = (double)ptStart.x - (double)ptEnd.x;
   dy = (double)ptStart.y - (double)ptEnd.y;

   len = sqrt(dx * dx + dy * dy);
   if (len == 0.0)
      return;          /* ptStart == ptEnd: degenerate line, draw nothing */

   ux = dx / len;
   uy = dy / len;

   half = dAngle / 180.0 * dPi;
   ca   = cos(half);
   sa   = sin(half);

   /* The two arrowhead lines are the backward direction rotated by +half
    * and -half, of length lLength, meeting at the tip (ptEnd).  Rotating
    * the unit vector (ux,uy) by an angle t gives (ux*cos t - uy*sin t,
    * ux*sin t + uy*cos t). */
   pt1.x = ptEnd.x + (LONG)(lLength * (ux * ca - uy * sa));
   pt1.y = ptEnd.y + (LONG)(lLength * (ux * sa + uy * ca));

   pt2.x = ptEnd.x + (LONG)(lLength * (ux * ca + uy * sa));
   pt2.y = ptEnd.y + (LONG)(lLength * (uy * ca - ux * sa));

   if (bFill)
      GpiBeginArea(hps, 0UL);

   GpiMove(hps, &pt1);
   GpiLine(hps, &ptEnd);
   GpiLine(hps, &pt2);

   if (bFill)
      GpiEndArea(hps);

}  /* XpmDrawArrowHead */

/*
 * EXPM - Extended Presentation Manager library
 *
 * h/shapewin.h -- Shaped ("transparent") windows module.
 *
 * A shaped window is a window whose outline follows an arbitrary mask
 * bitmap instead of being a plain rectangle.  OS/2 PM has no per-pixel
 * alpha compositing, so transparency is achieved the classic way: the
 * mask is parsed into rectangular spans and one child window is created
 * for each span, each showing the corresponding chunk of the image.  The
 * child windows are clipped to the mask's opaque pixels, so the
 * background shows through the transparent (key-coloured) parts.
 *
 * Usage:
 *
 *   1. Call XpmShapeInit() once, after WinCreateMsgQueue().
 *   2. Fill an XpmShapeData with two same-sized HPSs: hpsDraw (the
 *      image to display) and hpsMask (the mask; the colour of its
 *      bottom-left pixel is treated as transparent), plus cx/cy and an
 *      optional owner window.
 *   3. Create the window with WinCreateWindow(parent, WC_XpmSHAPEWIN, ...,
 *      0, x, y, cx, cy, hwndOwner, HWND_TOP, id, &data, NULL).  The data
 *      pointer is passed as the control data (mp1 of WM_CREATE).
 *
 * The owner receives the forwarded mouse/keyboard messages.  The caller
 * keeps hpsDraw/hpsMask alive for the lifetime of the window.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#ifndef EXPM_SHAPEWIN_H
#define EXPM_SHAPEWIN_H

#define WC_XpmSHAPEWIN          "XpmShapeWin"

#ifndef RC_INVOKED

typedef struct _XPMSHAPEDATA
{
    ULONG   cbSize;         /* sizeof(XpmShapeData)                    */
    HPS     hpsDraw;        /* image to display                         */
    HPS     hpsMask;        /* mask; bottom-left pixel colour = key     */
    LONG    cx;             /* width in pels                           */
    LONG    cy;             /* height in pels                          */
    HWND    hwndOwner;      /* owner for forwarded messages (optional)  */
} XpmShapeData, *PXpmShapeData;

/* Register the XpmShapeWin (and its helper) window classes.
 * Call once, after WinCreateMsgQueue and before creating any
 * XpmShapeWin window.  Returns TRUE on success. */
BOOL  EXPENTRY XpmShapeInit( VOID );

#endif /* RC_INVOKED */

#endif /* EXPM_SHAPEWIN_H */

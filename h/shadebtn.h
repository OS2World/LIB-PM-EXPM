/*
 * EXPM - Extended Presentation Manager library
 *
 * h/shadebtn.h -- Shaded Buttons module public interface.
 *
 * Merged from the LIB-PM-ShadedButtons 1.50 library (Shaded Buttons
 * 1.50) by Hubert Chan; see src/shadebtn/shadebtn.c for the copyright
 * notice and licence.  The entry points were renamed to the PM style:
 *
 *     shBtnInit          -> XpmShadeBtnInit
 *     shBtnQueryVersion  -> XpmShadeBtnQueryVersion
 *
 * The window class name, window styles, messages and presentation
 * parameters keep their upstream names so that code written against
 * the original ShadeBtn.dll compiles against EXPM unchanged.
 *
 * This header contains macros and prototypes; it is included by
 * h/expm.h, so include that instead of this file in applications.
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

#ifndef EXPM_SHADEBTN_H
#define EXPM_SHADEBTN_H

/* Window class name.  Register it with XpmShadeBtnInit, then use
 * WC_shBUTTON anywhere WC_BUTTON would be used. */
#define WC_shBUTTON             "shButton"

#ifndef RC_INVOKED

/* XpmShadeBtnInit -- register the shaded-button window class.
 * Call once, after WinCreateMsgQueue and before creating any
 * WC_shBUTTON window.  Returns TRUE on success.
 * (Upstream name: shBtnInit) */
BOOL  EXPENTRY XpmShadeBtnInit( VOID );

/* XpmShadeBtnQueryVersion -- return the library version times 100
 * (for example 1.50 reports 150).
 * (Upstream name: shBtnQueryVersion) */
ULONG EXPENTRY XpmShadeBtnQueryVersion( VOID );

#endif /* RC_INVOKED */

/* Window messages */
#define shBM_SETCOLOR           (WM_USER + 0)

/* mp2 values for shBM_SETCOLOR (mp1 = new RGB colour) */
#define shBSC_BUTTONFACE        0
#define shBSC_BACKGROUND        1

/* Presentation parameters */
#define shBPP_BUTTONCOLOR       PP_BACKGROUNDCOLOR
#define shBPP_BUTTONCOLORINDEX  PP_BACKGROUNDCOLORINDEX
#define shBPP_BUTTONBGCOLOR     (PP_USER + 0)
#define shBPP_BUTTONBGCOLORINDEX (PP_USER + 1)

/* Window styles.  The button type is one of the BS_* subset; the
 * remaining bits are shading / shape modifiers. */
#define shBS_PUSHBUTTON         BS_PUSHBUTTON
#define shBS_CHECKBOX           BS_CHECKBOX
#define shBS_AUTOCHECKBOX       BS_AUTOCHECKBOX
#define shBS_RADIOBUTTON        BS_RADIOBUTTON
#define shBS_AUTORADIOBUTTON    BS_AUTORADIOBUTTON
#define shBS_3STATE             BS_3STATE
#define shBS_AUTO3STATE         BS_AUTO3STATE
#define shBS_USERBUTTON         BS_USERBUTTON

#define shBS_TEXT               BS_TEXT        /* 0x0010 */
#define shBS_MINIICON           BS_MINIICON    /* 0x0020 */
#define shBS_BITMAP             BS_BITMAP      /* 0x0040 */
#define shBS_ICON               BS_ICON        /* 0x0080 */
#define shBS_HELP               BS_HELP        /* 0x0100 */
#define shBS_SYSCOMMAND         BS_SYSCOMMAND  /* 0x0200 */
#define shBS_DEFAULT            BS_DEFAULT     /* 0x0400 */

/* Horizontal shapes */
#define shBS_NOLEFTLINE         0x0008
#define shBS_LEFTROUND          0x0800
#define shBS_NORIGHTLINE        0x1000
#define shBS_RIGHTROUND         0x2000

/* Vertical shapes (used together with shBS_VERTICAL) */
#define shBS_NOTOPLINE          0x0008
#define shBS_TOPROUND           0x0800
#define shBS_NOBOTTOMLINE       0x1000
#define shBS_BOTTOMROUND        0x2000

/* Vertical gradient (left to right) */
#define shBS_VERTICAL           0x4000

/* Non-pushable (for status bars) and borderless */
#define shBS_NOPUSH             0x8000
#define shBS_NOBORDER           0x1008

/* Bits used by the (optional) PMCX integration; accepted for source
 * compatibility, unused by EXPM. */
#define shBS_BT0                0x0001
#define shBS_BT1                0x0002
#define shBS_BT2                0x0004

#endif /* EXPM_SHADEBTN_H */

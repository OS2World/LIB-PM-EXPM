/*
 * EXPM - Extended Presentation Manager library
 *
 * h/colorwheel.h -- ColorWheel (color picker dialog) module.
 *
 * Merged from the LIB-PM-ColorWheel 1.2 dialog library.
 *   Original code by Alessandro Cantatore,
 *   from the original color wheel concept by Paul Ratcliffe.
 *
 * This header contains macros only, so it is safe to include both from
 * C sources and from the resource script (src/colorwheel/colorwheel.rc);
 * that keeps the dialog template and the implementation in sync on the
 * control IDs, the localizable strings and the message numbers.
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

#ifndef COLORWHEEL_H
#define COLORWHEEL_H


/* Window class name of the OS/2 color wheel control.  The class lives in
 * WPCONFIG.DLL and is registered as a side effect of loading that DLL.
 * Used internally by the dialog template; applications do not need it. */
#define WC_COLORWHEEL               "ColorSelectClass"


/* Color wheel messages (undocumented OS/2 PM messages).
 *
 * WM_COLORWHEELCHANGED
 *   Sent by the color wheel control to its owner when the user changes
 *   the color by dragging the wheel.  Also forwarded by WinEditColorDlg
 *   to hwndOwner while the dialog is open.
 *   mp1 = (MPARAM)(LONG) new RGB color value, mp2 = MPVOID.
 *
 * CWM_COLORWHEELSETVAL
 *   Sent TO the color wheel control to set its color programmatically.
 *   mp1 = (MPARAM)(LONG) RGB color value to set, mp2 = MPVOID.
 *
 * Warp 4 (QSV_VERSION_MINOR > 30) uses 0x0601 / 0x0602; Warp 3 uses
 * different values.  WinEditColorDlg selects the right pair at runtime. */

#define WM_COLORWHEELCHANGED        0x0601
#define CWM_COLORWHEELSETVAL        0x0602

#define WM_COLORWHEELCHANGED3       0x130C
#define CWM_COLORWHEELSETVAL3       0x1384


/* Maximum length of the dialog title string, including the NUL. */
#define MAXTITLESIZE                128

/* Name of the module that owns the dialog template.  EXPM.DLL looks up
 * its own module handle under this name and passes it to WinDlgBox. */
#define SZ_MODULE                   "EXPM.DLL"


/* Localizable UI strings (edit to translate the dialog). */
#define SZ_DEFTITLE                 "Edit Color"  /* default dialog title   */
#define SZ_RED                      "~Red:"       /* Red   label (mnemonic) */
#define SZ_GREEN                    "~Green:"     /* Green label (mnemonic) */
#define SZ_BLUE                     "~Blue:"      /* Blue  label (mnemonic) */
#define SZ_OK                       "~OK"         /* OK button  (mnemonic)  */
#define SZ_UNDO                     "~Undo"       /* Undo button (mnemonic) */
#define SZ_CANCEL                   "~Cancel"     /* Cancel button (mnemonic)*/


/* Dialog and control resource IDs. */
#define DLG_CWHEEL                  100           /* dialog template ID     */
#define CWHEEL                      101           /* color wheel control    */
#define TXT_RED                     102           /* "Red:"   static text   */
#define SPN_RED                     103           /* Red   spin button      */
#define TXT_GREEN                   104           /* "Green:" static text   */
#define SPN_GREEN                   105           /* Green spin button      */
#define TXT_BLUE                    106           /* "Blue:"  static text   */
#define SPN_BLUE                    107           /* Blue  spin button      */
#define BTN_OK                      108           /* OK push button         */
#define BTN_UNDO                    109           /* Undo push button       */
#define BTN_CANCEL                  110           /* Cancel push button     */

#endif /* COLORWHEEL_H */

/*
 * EXPM - Extended Presentation Manager library
 *
 * h/expm.h -- public declarations for applications using EXPM.
 *
 * Function names use the EXPM "Xpm" prefix (for the window-manager
 * helpers), keeping clear of IBM's reserved Win, Gpi and Dos namespaces.
 * See doc/expm.txt for the API reference and the mapping of merged
 * third-party routines.
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

#ifndef EXPM_H
#define EXPM_H

#define INCL_WINDIALOGS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINWINDOWMGR
#define INCL_WINBUTTONS
#define INCL_WINSYS
#define INCL_GPI
#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ------------------------------------------------------------------ */
/* Library version                                                    */
/* ------------------------------------------------------------------ */

/* EXPM release version, as (major << 16) | minor.  This is the version
 * of the EXPM library as a whole and is independent of the version of
 * the third-party library each module tracks (the AsuaCtl controls, for
 * example, report their own baseline through XpmGetVersion). */
#define XPM_VERSION_MAJOR   0
#define XPM_VERSION_MINOR   2
#define XPM_VERSION_STRING  "0.2"


/* ------------------------------------------------------------------ */
/* Combo box support (merged from LIB-PM-ComboBoxSupport 1.1)         */
/* ------------------------------------------------------------------ */

/* XpmInitComboBox -- populate a combo box list from a NULL-terminated
 * array of strings, then select one item.
 *
 *   hwndOwner          owner window (dialog or client window)
 *   idCombo            combo box control identifier
 *   apszInit           NULL-terminated array of string pointers
 *   iInitialSelection  index of the item to preselect, or LIT_NONE
 *
 * Returns 0 on success. */
USHORT EXPENTRY XpmInitComboBox( HWND  hwndOwner,
                                 ULONG idCombo,
                                 PCSZ *apszInit,
                                 LONG  iInitialSelection );

/* XpmInitComboBoxFromRes -- like XpmInitComboBox, but the strings are
 * loaded from a resource STRINGTABLE.
 *
 *   hab                anchor block handle
 *   hmodRes            resource module handle (0 = current EXE)
 *   hwndOwner          owner window (dialog or client window)
 *   idCombo            combo box control identifier
 *   alId               array of string-table IDs to load
 *   cIds               number of IDs in alId
 *   cbStringMax        maximum length of a loaded string (incl. NUL)
 *   iInitialSelection  index of the item to preselect, or LIT_NONE
 *
 * Returns 0 on success, or 1 if memory could not be allocated. */
USHORT EXPENTRY XpmInitComboBoxFromRes( HAB    hab,
                                        HMODULE hmodRes,
                                        HWND   hwndOwner,
                                        ULONG  idCombo,
                                        PLONG  alId,
                                        LONG   cIds,
                                        ULONG  cbStringMax,
                                        LONG   iInitialSelection );


/* ------------------------------------------------------------------ */
/* Split views and toolbars (Cell Toolkit)                             */
/* Merged from the Cell Toolkit 0.7b by Sergey I. Yevtushenko,         */
/* redistributed under the BSD-2 licence (see src/cell/cell.c for      */
/* the copyright notice).                                              */
/* ------------------------------------------------------------------ */

/* Cell type flags */
#define CELL_WINDOW     0x0000  /* cell is an ordinary window        */
#define CELL_VSPLIT     0x0001  /* cell is a vertically split view   */
#define CELL_HSPLIT     0x0002  /* cell is a horizontally split view */
#define CELL_SPLITBAR   0x0004  /* cell has a split bar              */
#define CELL_FIXED      0x0008  /* views cannot be sized             */
#define CELL_SIZE1      0x0010  /* panel 1 has a fixed size          */
#define CELL_SIZE2      0x0020  /* panel 2 has a fixed size          */
#define CELL_HIDE_1     0x0040  /* panel 1 is hidden                 */
#define CELL_HIDE_2     0x0080  /* panel 2 is hidden                 */
#define CELL_HIDE       0x00C0  /* either panel is hidden            */
#define CELL_SPLIT_MASK 0x003F  /* split-related flag mask           */
#define CELL_SWAP       0x1000  /* panels are swapped                */

/* Initial panel size ratios */
#define CELL_SPLIT10x90 0x0100  /* panels related as 10% / 90%       */
#define CELL_SPLIT20x80 0x0200  /* panels related as 20% / 80%       */
#define CELL_SPLIT30x70 0x0300  /* panels related as 30% / 70%       */
#define CELL_SPLIT40x60 0x0400  /* panels related as 40% / 60%       */
#define CELL_SPLIT50x50 0x0500  /* panels related as 50% / 50%       */
#define CELL_SPLIT60x40 0x0600  /* panels related as 60% / 40%       */
#define CELL_SPLIT70x30 0x0700  /* panels related as 70% / 30%       */
#define CELL_SPLIT80x20 0x0800  /* panels related as 80% / 20%       */
#define CELL_SPLIT90x10 0x0900  /* panels related as 90% / 10%       */
#define CELL_SPLIT_REL  0x0F00  /* ratios are relative, not pixels   */

/* Toolbar flags */
#define TB_BUBBLE       0x0001  /* toolbar shows bubble help         */
#define TB_VERTICAL     0x0002  /* default toolbar view is vertical  */
#define TB_FLOATING     0x0004  /* toolbar is not attached           */
#define TB_ATTACHED_LT  0x0010  /* toolbar attached to the left      */
#define TB_ATTACHED_TP  0x0020  /* toolbar attached to the top       */
#define TB_ATTACHED_RT  0x0040  /* toolbar attached to the right     */
#define TB_ATTACHED_BT  0x0080  /* toolbar attached to the bottom    */
#define TB_ALLOWED      0x00FF  /* attachment flag mask              */
#define TB_SEPARATOR    0x7001  /* toolbar separator item ID         */
#define TB_BUBBLEID     0x7002  /* bubble help window ID             */

/* Limits */
#define SPLITBAR_WIDTH      2   /* split bar width                   */
#define HAND_SIZE           8   /* toolbar drag 'hand' size          */
#define TB_SEP_SIZE         7   /* toolbar separator width           */
#define TB_BUBBLE_SIZE     32   /* bubble help item size             */
#define CELL_TOP_LIMIT     98   /* maximal space occupied by one cell (%%) */
#define CELL_BOTTOM_LIMIT   2   /* minimal space occupied by one cell (%%) */

/* CellDef: one node of the split-view tree.
 *
 *   lType     type flags (CELL_*)
 *   pszClass  window class name when lType == CELL_WINDOW
 *   pszName   caption
 *   ulStyle   window (CELL_WINDOW) or frame creation (FCF_*) style
 *   ulID      window ID used with XpmCellWindowFromID etc.
 *   pPanel1   left/top panel child
 *   pPanel2   right/bottom panel child
 *   pClassProc      optional replacement class window proc for panels
 *   pClientClassProc reserved
 *   lSize     fixed panel size, meaningful when CELL_FIXED plus
 *             CELL_SIZE1/CELL_SIZE2 is set */
typedef struct stCellDef
{
    LONG  lType;
    PSZ   pszClass;
    PSZ   pszName;
    ULONG ulStyle;
    ULONG ulID;
    struct stCellDef* pPanel1;
    struct stCellDef* pPanel2;
    PFNWP pClassProc;
    PFNWP pClientClassProc;
    LONG  lSize;
} CellDef;

/* TbDef: toolbar description.
 *
 *   lType    toolbar flags (TB_*)
 *   ulID     toolbar window ID
 *   tbItems  NULL-terminated array of toolbar item IDs; TB_SEPARATOR
 *            entries insert a separator between groups */
typedef struct
{
    LONG   lType;
    ULONG  ulID;
    ULONG  *tbItems;
} TbDef;

/* WindowCellCtlData: per-window cell state, useful for windows that
 * are managed by a cell.  The first word (qWord == 0) is the old
 * window proc; it must be freed by a user subclass proc. */
typedef struct
{
    PFNWP pOldProc;
} WindowCellCtlData;

/* XpmCellInit -- register the window classes used by the Cell Toolkit.
 * Must be called once with the process anchor block before any other
 * Cell Toolkit routine. */
void  EXPENTRY XpmCellInit( HAB hab );

/* XpmCreateCell -- create a cell window from a CellDef tree. */
HWND  EXPENTRY XpmCreateCell( CellDef* pCell, HWND hWndParent, HWND hWndOwner );

/* XpmCellWindowFromID -- find the direct or indirect child window of a
 * cell with the given window ID. */
HWND  EXPENTRY XpmCellWindowFromID( HWND hwndCell, ULONG ulID );

/* XpmCellParentWindowFromID -- find the cell that directly manages a
 * window with the given window ID. */
HWND  EXPENTRY XpmCellParentWindowFromID( HWND hwndCell, ULONG ulID );

/* XpmCreateToolbar -- attach a toolbar described by pTb to a cell
 * window.  Bubble help is installed when TB_BUBBLE is set; place the
 * help strings in a resource STRINGTABLE numbered from the toolbar's
 * ulID. */
void  EXPENTRY XpmCreateToolbar( HWND hwndCell, TbDef* pTb );

/* XpmGenResIDStr -- format ulID as the '#nnnn' string that bitmap
 * buttons use to reference a resource. */
void  EXPENTRY XpmGenResIDStr( CHAR *buff, ULONG ulID );

/* XpmQueryCellSplit -- return the split ratio (in %) of a split cell. */
LONG  EXPENTRY XpmQueryCellSplit( HWND hwnd, LONG lID );

/* XpmSetCellSplit -- set the split ratio (in %) of a split cell,
 * subject to the CELL_TOP/CELL_BOTTOM limits.  Returns the effective
 * ratio. */
LONG  EXPENTRY XpmSetCellSplit( HWND hwnd, LONG lID, LONG lNewSplit );

/* XpmQueryCellSplitType -- return the CELL_VSPLIT/CELL_HSPLIT/CELL_SWAP
 * flags of a split cell. */
LONG  EXPENTRY XpmQueryCellSplitType( HWND hwnd, LONG lID );

/* XpmSetCellSplitType -- change the split direction of a cell,
 * optionally swapping its panels when CELL_SWAP is set. */
void  EXPENTRY XpmSetCellSplitType( HWND hwnd, LONG lID, LONG lNewType );

/* XpmShowCell -- show (bAction = TRUE) or hide (bAction = FALSE) the
 * panel with the given window ID. */
void  EXPENTRY XpmShowCell( HWND hwnd, LONG lID, BOOL bAction );


/* ------------------------------------------------------------------ */
/* Control Plus (clone of the closed-source Controls+ 1.40 controls)   */
/*                                                                     */
/* Reimplemented in C from the published Controls+ API and manual.     */
/* Window class names, style, message and notification constants keep  */
/* their upstream names so that applications written against Controls+ */
/* can link against EXPM unchanged.  The Ctl* entry points are         */
/* renamed to the PM Win* style (see doc/ctlplus.txt).                 */
/* ------------------------------------------------------------------ */

/* Window classes */
#define WC_METER                "meterCtl"    /* meter / thermometer  */
#define WC_SWITCH               "switchCtl"   /* switch               */
#define WC_RECT                 "rectCtl"     /* draggable rectangle  */
#define WC_COUNTER              "counterCtl"  /* chained digit        */

/* Base of the Control Plus private message range.  Upstream calls this
 * plain MSG_BASE; a CTL_ prefix is used here to avoid collisions. */
#define CTL_MSG_BASE            (WM_USER+0x800)

/* General Control Plus messages (recognised by every control) */
#define CTLM_SETSTYLE           (CTL_MSG_BASE+0x01)
#define CTLM_QUERYSTYLE         (CTL_MSG_BASE+0x02)

/* Creation data structures */
typedef struct
{
    ULONG mtStyle;
    ULONG mtFgColor;
} MTCREATE, *PMTCREATE;

#define SW_MAXTEXT              32

typedef struct
{
    ULONG swStyle;
    ULONG swFgColor;
    CHAR  swUpText[SW_MAXTEXT];
    CHAR  swDownText[SW_MAXTEXT];
} SWCREATE, *PSWCREATE;

typedef struct
{
    ULONG rcStyle;
    ULONG rcFgColor;
    SIZEL rcMaxSize;
    SIZEL rcMinSize;
} RCCREATE, *PRCCREATE;

typedef struct
{
    ULONG cntStyle;
    ULONG cntFgColor;
    ULONG cntBgColor;
    HWND  cntNext;
} CNTCREATE, *PCNTCREATE;

/* ---- Meter ------------------------------------------------------- */

/* Thermometer / analog meter styles */
#define MTS_THERM               0x00010000L
#define MTS_ANALOG              0x00020000L

#define MTS_NOTIFYOWNER         0x00000001L
#define MTS_NOINPUT             0x00000002L
#define MTS_HALFTICK            0x00000100L
#define MTS_QUARTERTICK         0x00000200L

/* Meter notification codes (via WM_CONTROL) */
#define MTN_CHGRANGE            1
#define MTN_CHGVALUE            2

/* Meter messages */
#define MTM_SETRANGE            (CTL_MSG_BASE+0x11)
#define MTM_QUERYRANGE          (CTL_MSG_BASE+0x12)
#define MTM_SETVALUE            (CTL_MSG_BASE+0x13)
#define MTM_QUERYVALUE          (CTL_MSG_BASE+0x14)

/* ---- Switch ------------------------------------------------------ */

/* Switch type styles */
#define SWS_BUTTON              0x00010000L
#define SWS_THROW               0x00020000L

#define SWS_NOTIFYOWNER         0x00000001L

/* Switch notification codes */
#define SWN_CHGVALUE            1
#define SWN_CHGUPTEXT           2
#define SWN_CHGDOWNTEXT         3

/* Switch messages */
#define SWM_SETVALUE            (CTL_MSG_BASE+0x15)
#define SWM_QUERYVALUE          (CTL_MSG_BASE+0x16)
#define SWM_SETUPTEXT           (CTL_MSG_BASE+0x17)
#define SWM_QUERYUPTEXT         (CTL_MSG_BASE+0x18)
#define SWM_SETDOWNTEXT         (CTL_MSG_BASE+0x19)
#define SWM_QUERYDOWNTEXT       (CTL_MSG_BASE+0x1A)

/* ---- Rect -------------------------------------------------------- */

#define RCS_NOTIFYOWNER         0x00000001L
#define RCS_VERTSIZE            0x00000002L
#define RCS_HORZSIZE            0x00000004L
#define RCS_VERTHORZSIZE        0x00000006L
#define RCS_NOMOVE              0x00000008L
#define RCS_MINSIZE             0x00000010L
#define RCS_MAXSIZE             0x00000020L
#define RCS_KSQUARE             0x00000040L
#define RCS_KCIRCLE             0x00000080L

/* Rect notification codes */
#define RCN_SIZE                1
#define RCN_DRAG                2
#define RCN_PAINT               3

/* Rect messages */
#define RCM_SETHANDLE           (CTL_MSG_BASE+0x1B)
#define RCM_QUERYHANDLE         (CTL_MSG_BASE+0x1C)
#define RCM_SETMINSIZE          (CTL_MSG_BASE+0x1D)
#define RCM_QUERYMINSIZE        (CTL_MSG_BASE+0x1E)
#define RCM_SETMAXSIZE          (CTL_MSG_BASE+0x1F)
#define RCM_QUERYMAXSIZE        (CTL_MSG_BASE+0x20)

/* ---- Counter ----------------------------------------------------- */

#define CTS_NOTIFYOWNER         0x00000001L
#define CTS_BIN                 0x00000002L
#define CTS_DEC                 0x00000004L
#define CTS_HEX                 0x00000008L
#define CTS_NOROLL              0x00000010L
#define CTS_NOSCROLL            0x00000020L

/* Counter notification codes */
#define CTN_CHGVALUE            1
#define CTN_UNDERFLOW           2
#define CTN_OVERFLOW            3

/* Counter messages */
#define CTM_SETVALUE            (CTL_MSG_BASE+0x21)
#define CTM_QUERYVALUE          (CTL_MSG_BASE+0x22)
#define CTM_SETNEXT             (CTL_MSG_BASE+0x23)
#define CTM_QUERYNEXT           (CTL_MSG_BASE+0x24)
#define CTM_INC                 (CTL_MSG_BASE+0x25)
#define CTM_DEC                 (CTL_MSG_BASE+0x26)
#define CTM_INCNOROLL           (CTL_MSG_BASE+0x27)
#define CTM_DECNOROLL           (CTL_MSG_BASE+0x28)

/* XpmCtlInit -- register the Control Plus window classes.  Call once,
 * after WinCreateMsgQueue and before creating any Control Plus window.
 * Returns TRUE on success.
 * (Controls+ name: CtlInitialize) */
USHORT  EXPENTRY XpmCtlInit( HAB hab );

/* XpmCtlTerminate -- release Control Plus resources.  Call after the
 * application windows are destroyed and before WinDestroyMsgQueue.
 * Returns TRUE on success.
 * (Controls+ name: CtlTerminate) */
USHORT  EXPENTRY XpmCtlTerminate( VOID );

/* XpmCtlVersion -- return the Control Plus version as MPFROM2SHORT
 * (major, minor).  (Controls+ name: CtlVersion) */
MRESULT EXPENTRY XpmCtlVersion( VOID );


/* ------------------------------------------------------------------ */
/* ColorWheel (modal color-picker dialog)                              */
/*                                                                     */
/* Merged from LIB-PM-ColorWheel 1.2 by Alessandro Cantatore, based on */
/* the original color wheel concept by Paul Ratcliffe (see             */
/* src/colorwheel/colorwheel.c for the copyright notice and licence).  */
/* The dialog wraps the undocumented "ColorSelectClass" control that   */
/* WPCONFIG.DLL registers.  h/colorwheel.h holds the message numbers,  */
/* the window class name and the resource IDs (it is shared with the   */
/* dialog template, hence kept as macros).                             */
/* ------------------------------------------------------------------ */

#include "colorwheel.h"

/* CLR - RGB color union: access a 32-bit OS/2 RGB color (LONG, format
 * 0x00RRGGBB) either as a whole or byte by byte.
 *
 * The rgb struct is ordered blue / green / red / pad, matching the
 * little-endian byte layout of a LONG 0x00RRGGBB in memory:
 *   byte 0 = blue (least significant), byte 1 = green,
 *   byte 2 = red,  byte 3 = 0x00 (unused high byte). */
#pragma pack(1)

typedef union {
   LONG  lClr;          /* full 32-bit OS/2 RGB color (0x00RRGGBB) */
   struct {
      UCHAR blu;        /* blue  component (bits  0-7)  */
      UCHAR grn;        /* green component (bits  8-15) */
      UCHAR red;        /* red   component (bits 16-23) */
      UCHAR x;          /* unused high byte (always 0)  */
   } rgb;
} CLR, *PCLR;

#pragma pack()

/* XpmEditColorDlg -- display a modal "Edit Color" dialog.
 *
 *   hwndParent  parent window; the dialog centers itself over it
 *   hwndOwner   owner window; receives WM_COLORWHEELCHANGED while the
 *               dialog is open (mp1 = current LONG RGB color)
 *   color       initial RGB color shown when the dialog opens
 *   pszTitle    dialog title, or NULL for the default "Edit Color"
 *
 * Returns the selected LONG RGB color on OK, the original color on
 * Cancel / Close, or CLR_ERROR if initialization failed. */
LONG EXPENTRY XpmEditColorDlg( HWND  hwndParent,
                               HWND  hwndOwner,
                               COLOR color,
                               PSZ   pszTitle );


/* ------------------------------------------------------------------ */
/* AniButtn (animated two-state graphical push buttons)                */
/*                                                                     */
/* Clean-room reimplementation of the AniButtn 1.02 library by Greg    */
/* Ratajik (1993), written from its published header and manual (see   */
/* src/anibtn/anibtn.c for the copyright notice and licence).          */
/* The upstream entry points AniCreateButton / AniDeleteButton are     */
/* renamed to the PM Win* style (see doc/anibtn.txt).                  */
/* ------------------------------------------------------------------ */

/* XpmCreateAniButton -- turn an existing WC_BUTTON control of style
 * BS_USERBUTTON into an animated two-state graphical push button.
 *
 *   hwndDlg    dialog (or other window) that owns the button
 *   id         control identifier of the button
 *   idUp       bitmap resource id drawn while the button is up
 *   idDown     bitmap resource id drawn while the button is pressed
 *   cx, cy     bitmap dimensions in pels
 *   pszStatus  status text shown while the pointer is over the button,
 *              or NULL for none
 *   idStatus   identifier of the static-text field that displays
 *              pszStatus, or 0 for none
 *
 * The two bitmaps are loaded from the application's executable, sized
 * cx by cy pels.  The control keeps the behaviour of a normal push
 * button: it sends WM_COMMAND / BN_CLICKED to its owner.  A second call
 * for the same button has no effect. */
VOID EXPENTRY XpmCreateAniButton( HWND   hwndDlg,
                                  SHORT  id,
                                  USHORT idUp,
                                  USHORT idDown,
                                  USHORT cx,
                                  USHORT cy,
                                  PSZ    pszStatus,
                                  USHORT idStatus );

/* XpmDeleteAniButton -- release an animated button created with
 * XpmCreateAniButton.  Call it when the owning dialog is closed, before
 * the dialog window is destroyed.  Deleting the last animated button of
 * a dialog also restores the dialog's original window procedure and the
 * status fields' original text. */
VOID EXPENTRY XpmDeleteAniButton( HWND hwndDlg, USHORT id );


/* ------------------------------------------------------------------ */
/* Shaded Buttons (gradient-shaded button window class)                */
/*                                                                     */
/* Merged from LIB-PM-ShadedButtons 1.50 (Shaded Buttons 1.50) by      */
/* Hubert Chan; see src/shadebtn/shadebtn.c for the copyright notice   */
/* and licence.  Registers WC_shBUTTON, a WC_BUTTON replacement that   */
/* paints a gradient face and adds check box, radio, three-state,      */
/* rounded, vertical and icon variants.  The upstream entry points     */
/* shBtnInit / shBtnQueryVersion are renamed to XpmShadeBtnInit /      */
/* XpmShadeBtnQueryVersion (see doc/shadebtn.txt).                     */
/* ------------------------------------------------------------------ */

#include "shadebtn.h"


/* ------------------------------------------------------------------ */
/* ArrowHead (GPI arrowhead drawing helper)                            */
/*                                                                     */
/* Merged from the ArrowHead 1.1 library by David Kopp; see            */
/* src/arrowhead/arrowhead.c for the copyright notice and licence.     */
/* The upstream entry point ArrowHead is renamed to the PM Win* style  */
/* (see doc/arrowhead.txt).                                            */
/* ------------------------------------------------------------------ */

#include "arrowhead.h"


/* ------------------------------------------------------------------ */
/* Owner-Draw Listbox (column alignment for LS_OWNERDRAW listboxes)    */
/*                                                                     */
/* Merged from DEV-SAMPLES-C-LIB-PM-Owner_Drawn_Listbox, distributed   */
/* under the BSD 3-Clause License (original by ASH Software, Inc.,     */
/* 1992; updates by Martin Iturbide / OS2World, 2026).  See            */
/* src/owndraw/owndraw.c for the copyright notice.  The upstream entry */
/* points ODInitLBWidthHeight / ODDrawLBItems are renamed to the EXPM  */
/* Xpm* style (see doc/owndraw.txt).                                   */
/* ------------------------------------------------------------------ */

/* sfCharWidth values for XpmInitLBWidthHeight */
#define AVERAGE_WIDTH   0   /* use lAveCharWidth from FONTMETRICS */
#define MAXIMUM_WIDTH   1   /* use lMaxCharInc from FONTMETRICS   */

/* Maximum length (in characters) of a single listbox item string.
 * Longer strings are silently truncated during drawing. */
#define MAX_TEXT_CHARS  255

/* PMWIN.H declares OWNERITEM / POWNERITEM only when INCL_WINMENUS is
 * in effect.  Forward-declaring the tag here keeps the prototype below
 * independent of the caller's include order (POWNERITEM is exactly
 * "struct _OWNERITEM *"). */
struct _OWNERITEM;

/* XpmInitLBWidthHeight -- initialise column width/height for an
 * owner-draw listbox.  Call from the WM_MEASUREITEM handler and return
 * the value it returns.
 *
 *   hListBox           handle to the listbox window
 *   sfCharWidth        AVERAGE_WIDTH or MAXIMUM_WIDTH
 *   sMaxNumberOfChars  longest string (in characters) that will be
 *                      inserted
 *   lCharWidth         receives the per-character pixel width used for
 *                      column calculations; pass this to XpmDrawLBItems
 *
 * Returns MRESULT: HIUSHORT = maximum baseline extent (row height),
 *                  LOUSHORT = total item width in pixels. */
MRESULT EXPENTRY XpmInitLBWidthHeight( HWND   hListBox,
                                       SHORT  sfCharWidth,
                                       SHORT  sMaxNumberOfChars,
                                       LONG  *lCharWidth );

/* XpmDrawLBItems -- draw one owner-draw listbox item with column
 * alignment.  Call from the WM_DRAWITEM handler and return the value it
 * returns.
 *
 *   pOwnerItem  pointer to the OWNERITEM structure passed in mp2
 *   lCharWidth  per-character pixel width from XpmInitLBWidthHeight
 *   psTabStops  array of 1-based character-column numbers defining each
 *               column boundary; the array must be terminated with 0
 *
 * Returns (MRESULT)TRUE to indicate the item has been drawn. */
MRESULT EXPENTRY XpmDrawLBItems( struct _OWNERITEM *pOwnerItem,
                                 LONG               lCharWidth,
                                 SHORT             *psTabStops );


/* ------------------------------------------------------------------ */
/* AsuaCtl controls (clean-room reimplementation of AsuaCtl 1.0)       */
/*                                                                     */
/* Reimplemented in C from the published AsuaCtl headers and IPF       */
/* manual only; no AsuaCtl source was consulted.  AsuaCtl is GPLv2, so */
/* a clean-room rewrite keeps EXPM LGPLv3.  Every public name (window  */
/* classes, styles, messages, notifications and structures) is renamed */
/* with the EXPM "Xpm" prefix so it never clashes with the original    */
/* asuactl.dll.  See doc\ctlcommon.txt and doc\status.txt.             */
/*                                                                     */
/* This release covers the shared helpers, the version/initialisation  */
/* API, the status control (WC_STATUS) and the progress bar            */
/* (WC_PROGRESSBAR).                                                   */
/* ------------------------------------------------------------------ */

/* Window class names */
#define XpmStatusClass          "XpmStatus"

/* Status control message range.  AsuaCtl used WM_USER+5..+11 for the
 * status window; the same numeric range is kept, but every macro is
 * renamed with the Xpm prefix. */
#define XpmSTAT_FIRST_MSG       (WM_USER+5)
#define XpmSTAT_LAST_MSG        (XpmSTAT_FIRST_MSG+7)

/* Status control messages.  All take reserved (0) mp1/mp2 except
 * XpmSM_SETHILITE, whose mp1 is the new enable state. */
#define XpmSM_QUERYMAXCHARINC   (XpmSTAT_FIRST_MSG+0)
#define XpmSM_QUERYTEXTHEIGHT   (XpmSTAT_FIRST_MSG+1)
#define XpmSM_QUERYTEXTWIDTH    (XpmSTAT_FIRST_MSG+2)
#define XpmSM_QUERYHEIGHT       (XpmSTAT_FIRST_MSG+3)
#define XpmSM_QUERYHILITE       (XpmSTAT_FIRST_MSG+4)
#define XpmSM_QUERYWIDTH        (XpmSTAT_FIRST_MSG+5)
#define XpmSM_SETHILITE         (XpmSTAT_FIRST_MSG+6)

/* Notification codes delivered in WM_CONTROL */
#define XpmSMN_PRESPARAMCHANGED 0x0400

/* Initialisation error codes */
#define XpmERR_PNL_CLASSREG     0x0001

/* Status control styles */
/* Border drawing */
#define XpmSS_BORDER            0x000F  /* complete frame, all four edges */
#define XpmSS_THINFRAME         0x0080  /* thin (1 pixel) black frame    */
#define XpmSS_LEFTFRAME         0x0001  /* draw left edge                */
#define XpmSS_RIGHTFRAME        0x0002  /* draw right edge               */
#define XpmSS_TOPFRAME          0x0004  /* draw top edge                 */
#define XpmSS_BOTTOMFRAME       0x0008  /* draw bottom edge              */
/* Horizontal text alignment */
#define XpmSS_LEFT              0x0000
#define XpmSS_CENTER            0x0020
#define XpmSS_RIGHT             0x0040
#define XpmSS_AUTOSHIFT         0x0060
/* Automatic scaling */
#define XpmSS_XAUTOSCALE        0x0100
#define XpmSS_YAUTOSCALE        0x0200
/* Additional options */
#define XpmSS_OWNERDRAW         0x0010
#define XpmSS_MOUSETRANSPARENT  0x0400
#define XpmSS_WINSTYLE          0x0800

/* XpmOwnerBack -- passed to the owner of an owner-draw control in
 * WM_DRAWITEM so it can repaint the background.  The owner should draw
 * into hps within bound (client coordinates) and return TRUE; returning
 * FALSE tells the control to paint the background itself.
 *
 *   hwnd      window being repainted
 *   hps       presentation space to draw with
 *   bound     rectangle to repaint (client coordinates)
 *   refpoint  coordinates of bound.xLeft / bound.yBottom
 */
#pragma pack(1)
typedef struct _XPMOWNERBACK
{
    HWND   hwnd;
    HPS    hps;
    RECTL  bound;
    POINTL refpoint;
} XpmOwnerBack, *PXpmOwnerBack;

/* XpmPPTemplate -- one entry of the configuration table used by
 * XpmQueryPresColors / XpmStorePresParams.
 *
 *   ulPresParam  presentation parameter to read (PP_*)
 *   ulPresFlags  QPF_* flags passed to WinQueryPresParam
 *   ulSysColor   SYSCLR_* fallback used when the parameter is absent
 *   ulRgbColor   hard-coded RGB fallback used when ulSysColor is 0
 */
typedef struct
{
    ULONG ulPresParam;
    ULONG ulPresFlags;
    ULONG ulSysColor;
    ULONG ulRgbColor;
} XpmPPTemplate, *PXpmPPTemplate;
#pragma pack()

/* Shared helpers (from AsuaCtl's pparams / graphics modules) */

/* XpmQueryPresColors -- build a colour table from a window's
 * presentation parameters.  (AsuaCtl name: PpmQueryPresColors)
 *
 *   hwnd       window to query
 *   nColors    number of entries in ppt and lColTable
 *   ppt        table of presentation parameters to read
 *   lColTable  receives one RGB colour per entry
 *
 * Returns 0 (reserved). */
LONG EXPENTRY XpmQueryPresColors( HWND hwnd, LONG nColors,
                                  XpmPPTemplate *ppt, LONG *lColTable );

/* XpmStorePresParams -- copy presentation parameters from a PRESPARAMS
 * list (for example CREATESTRUCT.pPresParams) into a window.
 * (AsuaCtl name: PpmStorePresParams)
 *
 *   hwnd     window to store into
 *   ppm      PRESPARAMS list to copy
 *   ppt      optional filter: when non-NULL only parameters listed in
 *            ppt are copied; when NULL all non-font parameters are
 *   nColors  number of entries in ppt
 *
 * Returns 0 (reserved). */
/* The PRESPARAMS tag is forward-declared so this prototype does not
 * depend on the caller having enabled INCL_WINSYS. */
struct _PRESPARAMS;
LONG EXPENTRY XpmStorePresParams( HWND hwnd, struct _PRESPARAMS *ppm,
                                  XpmPPTemplate *ppt, LONG nColors );

/* XpmDrawBitmap -- draw a colour bitmap together with an optional
 * monochrome transparency mask.  (AsuaCtl name: GrfDrawBitmap)
 *
 *   hps         presentation space
 *   hbmColor    colour bitmap to draw
 *   hbmMono     monochrome mask (1 bits are transparent), or NULL
 *   pos         destination point, or NULL for the current position
 *   flPattern   TRUE: paint the bitmap background (BM_OVERPAINT);
 *               FALSE: leave the background untouched
 *
 * Returns 0 (reserved). */
LONG EXPENTRY XpmDrawBitmap( HPS hps, HBITMAP hbmColor, HBITMAP hbmMono,
                             POINTL *pos, BOOL flPattern );

/* ================================================================== */
/* Font helper (from AsuaCtl's fonts module)                          */
/* ================================================================== */

/* XpmCreateRasterFont error codes (AsuaCtl ERR_FNT_* values, renamed
 * to the Xpm namespace).  The function returns 0 on success. */
#define XpmERR_FNT_INVPARAM     1       /* bad argument                */
#define XpmERR_FNT_OUTMEM       2       /* out of memory               */
#define XpmERR_FNT_HDC          3       /* device context read error   */
#define XpmERR_FNT_CREATE       4       /* font could not be created   */

/* XpmCreateRasterFont -- create a logical font bound to a local id and
 * report the metrics of the image (raster) font it was pinned to.
 * (AsuaCtl name: FntCreateRasterFont)
 *
 *   hps          presentation space whose device supplies the fonts
 *   pszNameSize  "[pointsize.]facename", for example "12.Helv" or
 *                "10.System Monospaced"; when the point size is omitted
 *                the first available image font of that face is used
 *   lLcid        local id (1..254) the logical font is bound to
 *   pfm          receives the metrics of the physical font selected
 *
 * Only raster (image) fonts are used: the face's image fonts are
 * enumerated with GpiQueryFonts and the one whose nominal point size is
 * closest to the requested size is pinned (FATTRS.lMatch) and created
 * with GpiCreateLogFont.  If the face has no image font, the
 * presentation space or its device cannot be read, or the font cannot
 * be created, the call fails rather than silently substituting an
 * outline font.
 *
 * Returns 0 on success or one of the XpmERR_FNT_* codes.  On success
 * *pfm describes the physical font that was pinned (its
 * sNominalPointSize is the real image size, which may differ from the
 * requested one).  The logical font is created but NOT selected into
 * hps; select it with GpiSetCharSet(hps, lLcid) and release it with
 * GpiDeleteSetId(hps, lLcid). */
LONG EXPENTRY XpmCreateRasterFont( HPS hps, PSZ pszNameSize,
                                   LONG lLcid, FONTMETRICS *pfm );

/* Version / initialisation API (from AsuaCtl's asuactl module) */

/* XpmControlsInit -- register every AsuaCtl-derived window class.
 * Returns 0 on success, or XpmERR_* if any registration failed; the
 * remaining classes are still attempted.
 * (AsuaCtl name: AsuCtlInitialize) */
LONG EXPENTRY XpmControlsInit( HAB hab );

/* XpmGetVersion -- return the library version as (major << 16) | minor.
 * (AsuaCtl name: AsuGetVersion) */
LONG EXPENTRY XpmGetVersion( VOID );

/* XpmGetComponentVersion -- return the version of the control class
 * pszClass as (major << 16) | minor, or 0 if the class is not
 * registered.  (AsuaCtl name: AsuGetComponentVersion) */
LONG EXPENTRY XpmGetComponentVersion( PSZ pszClass );

/* Status control (from AsuaCtl's status module) */

/* XpmStatusInit -- register the XpmStatusClass window class.  Called by
 * XpmControlsInit; call it directly only when the status control is
 * used on its own.  Returns 0 on success or XpmERR_PNL_CLASSREG.
 * (AsuaCtl name: CtlStatusInitialize) */
LONG EXPENTRY XpmStatusInit( HAB hab );

/* ------------------------------------------------------------------ */
/* Progress bar control (clean-room reimplementation of the AsuaCtl     */
/* progress bar).  AsuaCtl is GPLv2, EXPM is LGPLv3; every public name  */
/* carries the Xpm prefix.  See doc\progbar.txt.                       */
/* ------------------------------------------------------------------ */

/* Window class name */
#define XpmProgressBarClass     "XpmProgressBar"

/* Message range.  AsuaCtl used WM_USER+15..+24; the numeric range is
 * kept, every macro is renamed with the Xpm prefix. */
#define XpmPROG_FIRST_MSG       (WM_USER+15)
#define XpmPROG_LAST_MSG        (XpmPROG_FIRST_MSG+9)

#define XpmPBM_SETRANGE         (XpmPROG_FIRST_MSG+0)  /* mp1=low, mp2=high  */
#define XpmPBM_SETPOS           (XpmPROG_FIRST_MSG+1)  /* mp1=position       */
#define XpmPBM_DELTAPOS         (XpmPROG_FIRST_MSG+2)  /* mp1=delta          */
#define XpmPBM_SETSTEP          (XpmPROG_FIRST_MSG+3)  /* mp1=step           */
#define XpmPBM_STEPIT           (XpmPROG_FIRST_MSG+4)  /* advance by step    */
#define XpmPBM_GETRANGE         (XpmPROG_FIRST_MSG+5)  /* mp1=PXpmPBRange    */
#define XpmPBM_GETPOS           (XpmPROG_FIRST_MSG+6)  /* return position    */
#define XpmPBM_SETBARCOLOR      (XpmPROG_FIRST_MSG+7)  /* mp1=RGB bar colour */
#define XpmPBM_SETBKCOLOR       (XpmPROG_FIRST_MSG+8)  /* mp1=RGB background */

/* Styles */
#define XpmPBS_SMOOTH           0x0001  /* continuous indicator           */
#define XpmPBS_TEXT             0x0002  /* draw the percentage as text    */
#define XpmPBS_VERTICAL         0x0004  /* vertical indicator             */
#define XpmPBS_RAISED           0x0008  /* raised  (out) border           */
#define XpmPBS_DEPRESSED        0x0010  /* sunken  (in)  border           */

/* Range structure used by XpmPBM_SETRANGE / XpmPBM_GETRANGE */
#pragma pack(1)
typedef struct _XPMPBRANGE
{
    SHORT sLow;
    SHORT sHigh;
} XpmPBRange, *PXpmPBRange;
#pragma pack()

/* XpmProgressBarInit -- register the XpmProgressBarClass window class.
 * Called by XpmControlsInit; call it directly only when the progress
 * bar is used on its own.  Returns 0 on success or XpmERR_PNL_CLASSREG.
 * (AsuaCtl name: ProgbarRegister) */
LONG EXPENTRY XpmProgressBarInit( HAB hab );

/* ------------------------------------------------------------------ */
/* Window button control (clean-room reimplementation of the AsuaCtl    */
/* window button, WC_WINBUTTON).  AsuaCtl is GPLv2, EXPM is LGPLv3;     */
/* every public name carries the Xpm prefix.  See doc\winbutt.txt.      */
/* ------------------------------------------------------------------ */

/* Window class name */
#define XpmWinButtonClass       "XpmWinButton"

/* Styles.  AsuaCtl reused the WC_BUTTON style bit-space with its own
 * meaning, so these values intentionally overlap PM's BS_* bits; they
 * are only meaningful to XpmWinButtonClass.  The AsuaCtl numeric
 * values are kept, except XpmBS_DEFAULT which AsuaCtl left colliding
 * with XpmBS_AUTOTEXT and which EXPM renumbers (see doc\winbutt.txt). */
#define XpmBS_FLATBUTTON        0x0001  /* flat: frames on mouse-over only  */
#define XpmBS_OWNERDRAW         0x0008  /* owner paints the background      */
#define XpmBS_ALIGNLEFT         0x0000  /* text placement (see note below)  */
#define XpmBS_ALIGNRIGHT        0x0010
#define XpmBS_ALIGNTOP          0x0020
#define XpmBS_ALIGNBOTTOM       0x0030
#define XpmBS_ALIGNMASK         0x0030
#define XpmBS_THINFRAME         0x0080  /* one-pixel frame                  */
#define XpmBS_AUTOTEXT          0x0400  /* ellipsis when text does not fit  */
#define XpmBS_BITMAP            0x0040  /* a bitmap is used for the face    */
#define XpmBS_HELP              0x0100  /* notify with WM_HELP              */
#define XpmBS_SYSCOMMAND        0x0200  /* notify with WM_SYSCOMMAND        */
#define XpmBS_DEFAULT           0x1000  /* thick default-button frame       */
#define XpmBS_AUTOSIZE          0x4000  /* resize to fit text and/or bitmap */

/* The alignment field (XpmBS_ALIGNMASK) selects where the text sits:
 * XpmBS_ALIGNLEFT (left, vertically centred), XpmBS_ALIGNRIGHT (right,
 * vertically centred), XpmBS_ALIGNTOP (top, horizontally centred) or
 * XpmBS_ALIGNBOTTOM (bottom, horizontally centred). */

/* Notification codes sent to the owner.  Clicks (and the other buttons)
 * arrive as WM_COMMAND, the mouse/presentation events as WM_CONTROL. */
#define XpmBN_CLICKED           1
#define XpmBN_DBLCLICKED        2
#define XpmBN_PAINT             3
#define XpmBN_MOUSEENTER        4
#define XpmBN_MOUSELEAVE        5
#define XpmBN_MOUSECHANGE       6
#define XpmBN_PRESPARAMCHANGED  7
#define XpmBN_BUTTON2CLICKED    8

/* Control messages */
#define XpmBM_QUERYHEIGHT       (WM_USER+25)  /* return minimum height */
#define XpmBM_QUERYWIDTH        (WM_USER+26)  /* return minimum width  */

/* Identifiers of the bitmaps AsuaCtl embedded in its DLL resources.
 * EXPM does not ship those images; the identifiers are defined so that
 * existing code compiles, but XpmWinButtonInit does not load them
 * (pass the bitmaps through XpmWBTNCDATA instead). */
#define XpmBBMP_OK              1
#define XpmBBMP_CANCEL          2
#define XpmBBMP_HELP            3

/* XpmWBTNCDATA -- control data (CTLDATA) accepted at creation, and the
 * layout of the per-window data used by WM_DRAWITEM notifications.
 * Only the bitmap handles are honoured by EXPM; idStdBitmap is ignored
 * (the embedded images are not shipped) and fsHiliteState requests an
 * initially pressed appearance. */
#pragma pack(1)
typedef struct _XPMWBTNCDATA
{
    USHORT  cb;                 /* sizeof(XpmWBTNCDATA)                 */
    USHORT  idStdBitmap;        /* embedded bitmap id (EXPM: ignored)   */
    USHORT  fsHiliteState;      /* TRUE: start pressed                  */
    USHORT  usPad;
    LHANDLE hColorImage;        /* normal    colour bitmap, or NULL     */
    LHANDLE hColorDisImage;     /* disabled  colour bitmap, or NULL     */
    LHANDLE hColorPressImage;   /* pressed   colour bitmap, or NULL     */
    LHANDLE hColorHiliteImage;  /* mouse-over colour bitmap, or NULL    */
    LHANDLE hImage;             /* normal    monochrome mask, or NULL   */
    LHANDLE hImageDis;          /* disabled  monochrome mask, or NULL   */
    LHANDLE hImagePress;        /* pressed   monochrome mask, or NULL   */
    LHANDLE hImageHilite;       /* mouse-over monochrome mask, or NULL  */
} XpmWBTNCDATA, *PXpmWBTNCDATA;
#pragma pack()

/* XpmWinButtonInit -- register the XpmWinButtonClass window class.
 * Called by XpmControlsInit; call it directly only when the window
 * button is used on its own.  Returns 0 on success or
 * XpmERR_PNL_CLASSREG.  (AsuaCtl name: BtnRegister) */
LONG EXPENTRY XpmWinButtonInit( HAB hab );

/* ------------------------------------------------------------------ */
/* Toolbar control (clean-room reimplementation of the AsuaCtl toolbar, */
/* WC_TOOLBAR).  AsuaCtl is GPLv2, EXPM is LGPLv3; every public name    */
/* carries the Xpm prefix.  See doc\toolbar.txt.                       */
/*                                                                     */
/* This release implements the full toolbar: a toolbar holding control  */
/* windows and separators, with begin / centre / end alignment,         */
/* horizontal or vertical orientation, object insert / delete / move /  */
/* query, the state and size queries, the automatic sizing message and  */
/* its TN_AUTOSIZE notification, the 3-D frame, floating / docking /    */
/* minimising with drag tracking (the title bar and rotate button), and */
/* per-object tool-tips.  Scrolling and multi-line layout are the only   */
/* original features left out (see doc\toolbar.txt).                    */
/* ------------------------------------------------------------------ */

/* Window class name */
#define XpmToolBarClass         "XpmToolBar"

/* Reserved internal child-window identifiers.  EXPM parents objects
 * directly to the toolbar window and paints separators itself, so no
 * XpmTCID_OBJECTPAD window is created; the title bar, rotate button and
 * tool-tip windows use the other identifiers. */
#define XpmTCID_TITLEBAR        0xFFFF
#define XpmTCID_TOOLTIP         0xFFFE
#define XpmTCID_ROTATE          0xFFFC
#define XpmTCID_OBJECTPAD       0xFFF9

/* Message range.  AsuaCtl used WM_USER+60..+79; the numeric range is
 * kept, every macro is renamed with the Xpm prefix.  A caller may
 * define XpmTOOL_FIRST_MSG before including this header to relocate the
 * range (it must stay relative to WM_USER). */
#ifndef XpmTOOL_FIRST_MSG
#define XpmTOOL_FIRST_MSG       (WM_USER+60)
#endif
#define XpmTOOL_LAST_MSG        (XpmTOOL_FIRST_MSG+19)

/* Toolbar control messages (details in doc\toolbar.txt) */
#define XpmTM_AUTOSIZE          (XpmTOOL_FIRST_MSG+0)
#define XpmTM_CALCNEWRECT       (XpmTOOL_FIRST_MSG+1)
#define XpmTM_ENABLEROTATE      (XpmTOOL_FIRST_MSG+2)
#define XpmTM_QUERYHEIGHT       (XpmTOOL_FIRST_MSG+3)
#define XpmTM_QUERYWIDTH        (XpmTOOL_FIRST_MSG+4)
#define XpmTM_QUERYSTATE        (XpmTOOL_FIRST_MSG+5)
#define XpmTM_ROTATE            (XpmTOOL_FIRST_MSG+6)
#define XpmTM_DELETEOBJECT      (XpmTOOL_FIRST_MSG+7)
#define XpmTM_IDFROMPOSITION    (XpmTOOL_FIRST_MSG+8)
#define XpmTM_INSERTOBJECT      (XpmTOOL_FIRST_MSG+9)
#define XpmTM_INSERTTOOLTIP     (XpmTOOL_FIRST_MSG+10)
#define XpmTM_MOVEOBJECT        (XpmTOOL_FIRST_MSG+11)
#define XpmTM_OBJECTCONTROL     (XpmTOOL_FIRST_MSG+12)
#define XpmTM_POSITIONFROMID    (XpmTOOL_FIRST_MSG+13)
#define XpmTM_QUERYHANDLE       (XpmTOOL_FIRST_MSG+14)
#define XpmTM_QUERYOBJECT       (XpmTOOL_FIRST_MSG+15)
#define XpmTM_QUERYOBJECTCOUNT  (XpmTOOL_FIRST_MSG+16)
#define XpmTM_QUERYTIP          (XpmTOOL_FIRST_MSG+17)
#define XpmTM_SETOBJECT         (XpmTOOL_FIRST_MSG+18)
#define XpmTM_SHOWOBJECT        (XpmTOOL_FIRST_MSG+19)

/* Window styles */
#define XpmTBS_STANDARD         0x0000  /* clip objects that do not fit   */
#define XpmTBS_SCROLLABLE       0x0001  /* scroll buttons (not impl.)     */
#define XpmTBS_MULTILINE        0x0002  /* wrap objects (not impl.)       */
#define XpmTBS_CUTTYPES         0x0007
#define XpmTBS_MOVEWITHOWNER    0x0800  /* track owner moves (not impl.)  */
#define XpmTBS_ENABLEMINIMIZE   0x1000  /* click title to minimise        */
#define XpmTBS_MOVEBUTTON       0x2000  /* title bar moves / floats       */
#define XpmTBS_FIXBUTTON        0x4000  /* title bar dock toggle          */
#define XpmTBS_ROTATEBUTTON     0x8000  /* rotate button                  */

/* Toolbar states (XpmTM_QUERYSTATE result, XpmTOOLADJ.flState) */
#define XpmTST_FIXED            0x0001  /* docked to the parent           */
#define XpmTST_MINIMIZED        0x0002  /* minimized                      */
#define XpmTST_ROTATED          0x0004  /* vertical orientation           */

/* Toolbar attributes (XpmTOOLINFO.flToolAttrs, XpmTOOLADJ.flAttrs) */
#define XpmTBA_FRAME            0x000F  /* complete 3-D frame             */
#define XpmTBA_LEFTFRAME        0x0001
#define XpmTBA_BOTTOMFRAME      0x0002
#define XpmTBA_RIGHTFRAME       0x0004
#define XpmTBA_TOPFRAME         0x0008
#define XpmTBA_FIXEDTOP         0x0000  /* horizontal: dock at top        */
#define XpmTBA_FIXEDBOTTOM      0x0010  /* horizontal: dock at bottom     */
#define XpmTBA_FIXEDLEFT        0x0000  /* vertical:   dock at left       */
#define XpmTBA_FIXEDRIGHT       0x0020  /* vertical:   dock at right      */
#define XpmTBA_HORIZONTAL       0x0000
#define XpmTBA_VERTICAL         0x0040

/* XpmTM_QUERYHEIGHT / XpmTM_QUERYWIDTH options */
#define XpmTDQ_RESTORED         0x0000  /* floating size                  */
#define XpmTDQ_MINIMIZED        0x0001  /* minimized size                 */
#define XpmTDQ_FIXED            0x0002  /* docked size                    */
#define XpmTDQ_CURRENT          0x0003  /* current size                   */

/* XpmTM_ROTATE options */
#define XpmTBR_ROTATE           0
#define XpmTBR_HORIZONTAL       1
#define XpmTBR_VERTICAL         2

/* XpmTM_SETOBJECT options */
#define XpmTSO_GROUP            0x0001
#define XpmTSO_SIZE             0x0002
#define XpmTSO_USER             0x0004

/* Object attributes (XpmTOOLOBJ.flAttribute) */
#define XpmTBO_STDWINDOW        0x0000  /* ordinary control window        */
#define XpmTBO_BUTTON           0x0001  /* auto-sized button              */
#define XpmTBO_STDBUTTON        0x0002  /* supported, not auto-sized      */
#define XpmTBO_SEPARATOR        0x0003  /* separator (not a window)       */
#define XpmTBO_BASICTYPES       0x000F
#define XpmTBO_BEGINALIGN       0x0000
#define XpmTBO_ENDALIGN         0x0010
#define XpmTBO_CENTERALIGN      0x0020
#define XpmTBO_JUSTTYPES        0x0030
#define XpmTBO_PUSHBUTTON       0x0040
#define XpmTBO_CHECKBOX         0x0080
#define XpmTBO_RADIOBUTTON      0x00C0
#define XpmTBO_HIDE             0x0100  /* object is hidden               */
#define XpmTBO_DISABLE          0x0200  /* object is disabled             */

/* Separator styles (bits of XpmTOOLOBJ.flAttribute) */
#define XpmSPS_FLAT             0x0000
#define XpmSPS_LINE             0x2000
#define XpmSPS_CONCAVE          0x4000
#define XpmSPS_CONVEX           0x6000
#define XpmSPS_SEPARATORS       0xE000

/* Position that appends an object to the toolbar */
#define XpmTB_END               0xFFFFFFFFL

/* Notification codes delivered in WM_CONTROL */
#define XpmTN_AUTOSIZE          1
#define XpmTN_BEGINTRACK        2
#define XpmTN_TRACKING          3
#define XpmTN_ENDTRACK          4
#define XpmTN_FIXTOOL           5
#define XpmTN_MINIMIZE          6
#define XpmTN_RESTORE           7
#define XpmTN_ROTATE            8

/* Tool-tip presentation parameters.  XpmPP_TOOLTIPBACKGROUND etc. may
 * be defined by the caller before including this header. */
#ifndef XpmTOOL_FIRST_PP
#define XpmTOOL_FIRST_PP        (PP_USER+0)
#endif
#define XpmPP_TOOLTIPBACKGROUND (XpmTOOL_FIRST_PP+0)
#define XpmPP_TOOLTIPTEXT       (XpmTOOL_FIRST_PP+1)
#define XpmPP_TOOLTIPFRAME      (XpmTOOL_FIRST_PP+2)
#define XpmPP_TOOLTIPFONT       (XpmTOOL_FIRST_PP+3)
#define XpmPP_TOOLTIPDELAY      (XpmTOOL_FIRST_PP+4)

/* XpmTOOLINFO -- toolbar control data (WinCreateWindow pCtlData, and
 * WM_SETWINDOWPARAMS).  Packed to 2 bytes like the AsuaCtl original. */
#pragma pack(2)
typedef struct _XPMTOOLINFO
{
    USHORT cb;            /* total size of the structure in bytes        */
    USHORT flToolAttrs;   /* XpmTBA_* attributes                         */
    USHORT usMaxLength;   /* maximum floating length in pixels (0 = none) */
} XpmTOOLINFO, *PXpmTOOLINFO;
#pragma pack()

/* XpmTOOLADJ -- passed with the XpmTN_AUTOSIZE notification.  Packed
 * to 4 bytes like the AsuaCtl original. */
#pragma pack(4)
typedef struct _XPMTOOLADJ
{
    HWND  hwndToolBar;    /* toolbar window handle                       */
    HWND  hwndParent;     /* suggested parent window handle              */
    RECTL rclSizePos;     /* suggested position and size (parent coords) */
    ULONG flAttrs;        /* suggested XpmTBA_* attributes               */
    ULONG flState;        /* state (XpmTST_*); input for CALCNEWRECT    */
} XpmTOOLADJ, *PXpmTOOLADJ;

/* XpmTOOLOBJ -- object information (insert / query / set). */
typedef struct _XPMTOOLOBJ
{
    ULONG iPosition;      /* ordinal position from the toolbar start     */
    ULONG ulGroup;        /* button group (buttons only)                 */
    ULONG flAttribute;    /* XpmTBO_* / XpmSPS_* attributes              */
    ULONG cx;             /* initial object width                        */
    ULONG cy;             /* initial object height                       */
    ULONG id;             /* object identifier                           */
    ULONG ulUser;         /* available for the application               */
} XpmTOOLOBJ, *PXpmTOOLOBJ;

/* XpmTOOLWIN -- supplementary data used to create the object window. */
typedef struct _XPMTOOLWIN
{
    PSZ   pszClass;       /* window class of the object                  */
    PSZ   pszName;        /* window text                                 */
    ULONG flStyle;        /* window style flags                          */
    PVOID pCtlData;       /* class-specific data                         */
    PVOID pPresParams;    /* presentation parameters                     */
} XpmTOOLWIN, *PXpmTOOLWIN;

/* XpmTOOLTRACK -- passed with the drag-tracking notifications
 * (XpmTN_BEGINTRACK / XpmTN_TRACKING / XpmTN_ENDTRACK). */
typedef struct _XPMTOOLTRACK
{
    HWND  hwndToolBar;
    HWND  hwndParent;
    RECTL rclSizePos;
    RECTL rclBoundary;
    ULONG flAttrs;
    ULONG flState;
} XpmTOOLTRACK, *PXpmTOOLTRACK;
#pragma pack()

/* XpmToolBarInit -- register the XpmToolBarClass window class.  Called
 * by XpmControlsInit; call it directly only when the toolbar is used on
 * its own.  Returns 0 on success or XpmERR_PNL_CLASSREG.
 * (AsuaCtl name: CtlToolbarInitialize) */
LONG EXPENTRY XpmToolBarInit( HAB hab );


/* ------------------------------------------------------------------ */
/* Directory dialog (AsuaCtl AsuDirDlg)                               */
/* ------------------------------------------------------------------ */

/* Dialog template identifiers.  The templates live in EXPM.DLL
 * (src/dirdlg/dirdlg.rc) and are loaded by XpmDirDlg; the caller only
 * chooses a variant through XpmDIRDLG.flDialogType.  The "_W" variants
 * use XpmWinButtonClass for the OK / Cancel buttons. */
#define XpmDIRDLG_OKCANCEL      2000    /* OK + Cancel buttons         */
#define XpmDIRDLG_OK            2001    /* OK button only              */
#define XpmDIRDLG_OKCANCEL_W    2100    /* OK + Cancel, XpmWinButton   */
#define XpmDIRDLG_OK_W          2101    /* OK only, XpmWinButton       */

/* Container (directory list) identifier inside the dialog. */
#define XpmCNR_TREE             257

/* XpmDIRDLG -- describes a directory dialog and receives its result.
 * Fill in the fields before calling XpmDirDlg; lReturn and
 * szFullDirectoryName are set on return.  The AsuaCtl layout is kept
 * so existing source maps across unchanged. */
typedef struct _XpmDIRDLG
{
    PSZ  pszTitle;                          /* title (NULL = default)      */
    PSZ  pszOKButton;                       /* OK text (NULL = default)    */
    PSZ  pszCancelButton;                   /* Cancel text (NULL = default)*/
    PSZ  pszInitialDir;                     /* directory to start in       */
    CHAR szFullDirectoryName[CCHMAXPATH];   /* chosen directory (output)   */
    LONG lReturn;                           /* DID_OK or DID_CANCEL        */
    LONG flDialogType;                      /* XpmDIRDLG_* variant         */
    BOOL flUseWinButton;                    /* use XpmWinButtonClass       */
} XpmDIRDLG, *PXpmDIRDLG;

/* XpmDirDlg -- display a modal "choose directory" dialog and return the
 * chosen directory in pdd->szFullDirectoryName.  Returns TRUE when the
 * user pressed OK (pdd->lReturn == DID_OK), FALSE on Cancel / Close.
 * The user navigates the directory tree (double-click to open a drive
 * or folder, ".." to go up); OK returns the highlighted entry, or the
 * directory currently on display when nothing is highlighted.
 * (AsuaCtl name: AsuDirDlg) */
BOOL EXPENTRY XpmDirDlg( HWND hwndParent, HWND hwndOwner, PXpmDIRDLG pdd );


#ifdef __cplusplus
}
#endif

#endif /* EXPM_H */

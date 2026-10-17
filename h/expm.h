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
/* API and the status control (WC_STATUS).                             */
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


#ifdef __cplusplus
}
#endif

#endif /* EXPM_H */

/*=========================================================================
 * editcol.h - Public header for the EDITCOL.DLL color-wheel dialog.
 *
 * Wraps the undocumented OS/2 Workplace Shell "ColorSelectClass" window
 * class (which lives inside WPCONFIG.DLL) into a ready-to-use modal dialog
 * with Red / Green / Blue spin buttons and OK / Undo / Cancel buttons.
 *
 * Based on an idea by Paul Ratcliffe.
 * Coded by Alessandro Cantatore.
 * Version 1.2 - Open Watcom 2.0 port.
 *=========================================================================*/

#ifndef EDITCOL_H
#define EDITCOL_H


/* Window class name for the OS/2 color wheel control.
 * This class is registered by WPCONFIG.DLL when it is loaded.
 * It is used internally by the DLL; applications do not need it directly. */
#define WC_COLORWHEEL               "ColorSelectClass"


/*-------------------------------------------------------------------------
 * Color wheel messages (undocumented OS/2 PM messages).
 *
 * WM_COLORWHEELCHANGED
 *   Sent by the color wheel control to its owner when the user changes
 *   the color by dragging the wheel.
 *   Also forwarded by WinEditColorDlg to hwndOwner while the dialog is open.
 *   mp1 = (MPARAM)(LONG) new RGB color value
 *   mp2 = MPVOID
 *
 * CWM_COLORWHEELSETVAL
 *   Sent TO the color wheel control to programmatically set its color.
 *   mp1 = (MPARAM)(LONG) RGB color value to set
 *   mp2 = MPVOID
 *
 * Warp 4 (minor version > 30) uses 0x0601 / 0x0602.
 * Warp 3 uses different values; WinEditColorDlg selects the correct
 * pair at runtime by querying QSV_VERSION_MINOR.
 *-------------------------------------------------------------------------*/

/* Warp 4 message numbers */
#define WM_COLORWHEELCHANGED        0x0601
#define CWM_COLORWHEELSETVAL        0x0602

/* Warp 3 message numbers */
#define WM_COLORWHEELCHANGED3       0x130C
#define CWM_COLORWHEELSETVAL3       0x1384


/*-------------------------------------------------------------------------
 * Constants
 *-------------------------------------------------------------------------*/

/* Maximum length of the dialog title string, including the null terminator. */
#define MAXTITLESIZE           128

/* Name of this DLL, used to obtain its own module handle for loading
 * the dialog template from its own resources. */
#define SZ_MODULE     "EDITCOL.DLL"


/*-------------------------------------------------------------------------
 * CLR - RGB color union.
 *
 * Allows a 32-bit OS/2 RGB color (LONG, format 0x00RRGGBB) to be accessed
 * either as a whole LONG or as individual byte components.
 *
 * Field order in the rgb struct is blue / green / red / padding, matching
 * the little-endian byte layout of a LONG value 0x00RRGGBB in memory:
 *   byte 0 = blue  (least significant)
 *   byte 1 = green
 *   byte 2 = red
 *   byte 3 = 0x00  (padding / unused high byte)
 *-------------------------------------------------------------------------*/
#pragma pack(1)

typedef union {
   LONG lClr;           /* full 32-bit OS/2 RGB color (0x00RRGGBB) */
   struct {
      UCHAR blu;        /* blue  component (bits  0-7)  */
      UCHAR grn;        /* green component (bits  8-15) */
      UCHAR red;        /* red   component (bits 16-23) */
      UCHAR x;          /* unused high byte (always 0)  */
   } rgb;
} CLR, * PCLR;

#pragma pack()


/*-------------------------------------------------------------------------
 * APPDATA - Per-dialog instance data, allocated on the heap by
 * WinEditColorDlg and freed during WM_DESTROY.
 * A pointer to this structure is stored in the dialog's window words
 * at offset 0 (QWL_USER).
 *-------------------------------------------------------------------------*/
#pragma pack(2)

typedef struct {
   USHORT  cbSize;                   /* size of this structure in bytes     */
   USHORT  res;                      /* reserved / alignment padding        */
   HAB     hab;                      /* anchor block for PM calls           */
   HLIB    hlib;                     /* handle of loaded WPCONFIG.DLL       */
   HMODULE hDlgMod;                  /* module handle of EDITCOL.DLL itself,
                                        used to load the dialog resource    */
   HWND    hwndParent;               /* parent window (dialog centers over) */
   HWND    hwndOwner;                /* owner window (receives notifications)*/
   ULONG   ulVer;                    /* OS/2 minor version (QSV_VERSION_MINOR):
                                        > 30 = Warp 4, else Warp 3         */
   ULONG   cwChangedMsg;             /* WM_COLORWHEELCHANGED or ..CHANGED3  */
   ULONG   cwSetMsg;                 /* CWM_COLORWHEELSETVAL or ..SETVAL3   */
   CHAR    achDlgTitle[MAXTITLESIZE];/* dialog title bar text               */
   BOOL    flUpd;                    /* TRUE = spin-button changes come from
                                        the user and should update the wheel;
                                        FALSE = changes are programmatic
                                        (init / wheel-driven) and must be
                                        ignored to prevent feedback loops   */
   CLR     clrUndo;                  /* color when dialog was opened (Undo) */
   CLR     clrCurr;                  /* color currently shown in the dialog */
   RECTL   rclPar;                   /* bounding rectangle of parent window */
   RECTL   rcl;                      /* bounding rectangle of the dialog    */
} APPDATA, * PAPPDATA;

#pragma pack()


/*-------------------------------------------------------------------------
 * Public API
 *-------------------------------------------------------------------------*/

/* WinEditColorDlg - Display a modal "Edit Color" dialog.
 *
 * hwndParent : parent window handle; the dialog centers itself over this.
 * hwndOwner  : owner window handle; receives WM_COLORWHEELCHANGED while
 *              the dialog is open (mp1 = current LONG RGB color).
 * color      : initial RGB color shown when the dialog opens.
 * pszTitle   : dialog title; pass NULL to use the default "Edit Color".
 *
 * Returns the selected LONG RGB color on OK, or the original color on
 * Cancel / Close, or CLR_ERROR if initialization failed. */
LONG APIENTRY WinEditColorDlg(HWND hwndParent,
                               HWND hwndOwner,
                               COLOR color,
                               PSZ pszTitle);

/* Internal dialog procedure — not called directly by applications. */
MRESULT EXPENTRY EditColorDlgProc(HWND hwnd, ULONG msg,
                                  MPARAM mp1, MPARAM mp2);

/* Internal helper — sets all three spin buttons to the values in clr. */
VOID SetSpins(HWND hwnd, CLR clr);


/*-------------------------------------------------------------------------
 * Localizable UI strings (edit to translate the dialog to another language)
 *-------------------------------------------------------------------------*/
#define SZ_DEFTITLE    "Edit Color"   /* default dialog title               */
#define SZ_RED         "~Red:"        /* Red spin button label   (mnemonic) */
#define SZ_GREEN       "~Green:"      /* Green spin button label (mnemonic) */
#define SZ_BLUE        "~Blue:"       /* Blue spin button label  (mnemonic) */
#define SZ_OK          "~OK"          /* OK push button          (mnemonic) */
#define SZ_UNDO        "~Undo"        /* Undo push button        (mnemonic) */
#define SZ_CANCEL      "~Cancel"      /* Cancel push button      (mnemonic) */


/*-------------------------------------------------------------------------
 * Dialog and control resource IDs
 *-------------------------------------------------------------------------*/
#define DLG_CWHEEL     100            /* dialog template ID                 */
#define CWHEEL         101            /* color wheel control                */
#define TXT_RED        102            /* "Red:"   static text label         */
#define SPN_RED        103            /* Red   spin button                  */
#define TXT_GREEN      104            /* "Green:" static text label         */
#define SPN_GREEN      105            /* Green spin button                  */
#define TXT_BLUE       106            /* "Blue:"  static text label         */
#define SPN_BLUE       107            /* Blue  spin button                  */
#define BTN_OK         108            /* OK push button                     */
#define BTN_UNDO       109            /* Undo push button                   */
#define BTN_CANCEL     110            /* Cancel push button                 */

#endif /* EDITCOL_H */

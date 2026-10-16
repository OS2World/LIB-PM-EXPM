/*=========================================================================
 * editcol.c - Color wheel dialog implementation.
 *
 * Implements WinEditColorDlg(), which displays a modal PM dialog wrapping
 * the undocumented OS/2 "ColorSelectClass" color wheel control that lives
 * inside WPCONFIG.DLL (the Workplace Shell configuration DLL).
 *
 * Version 1.2 - Open Watcom 2.0 port.
 *=========================================================================*/

#define INCL_WIN
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#include <os2.h>
#include <editcol.h>


/*=========================================================================
 * WinEditColorDlg() - Display the "Edit Color" modal dialog.
 *
 * Allocates per-dialog state (APPDATA), loads WPCONFIG.DLL to register
 * the ColorSelectClass window class, then runs the dialog modally via
 * WinDlgBox.  The APPDATA block is freed during WM_DESTROY.
 *
 * Parameters:
 *   hwndParent : parent window handle; the dialog centers itself over this.
 *   hwndOwner  : owner window handle; receives WM_COLORWHEELCHANGED
 *                notifications while the dialog is open.
 *   color      : initial RGB color (OS/2 LONG format 0x00RRGGBB).
 *   pszTitle   : dialog title string, or NULL for the default "Edit Color".
 *
 * Returns:
 *   Selected LONG RGB color on OK, original color on Cancel / Close,
 *   or CLR_ERROR if initialization failed.
 *=========================================================================*/
LONG APIENTRY WinEditColorDlg(HWND hwndParent,
                               HWND hwndOwner,
                               COLOR color,
                               PSZ pszTitle) {
   PAPPDATA pad;
   ULONG i;

   /* Allocate per-dialog instance data on the heap.
    * It will be stored in the dialog's QWL_USER word and freed in WM_DESTROY. */
   if (DosAllocMem((PPVOID)&pad, sizeof(APPDATA),
                   PAG_READ | PAG_WRITE | PAG_COMMIT))
      return CLR_ERROR;

   pad->cbSize    = sizeof(APPDATA);
   pad->hab       = WinQueryAnchorBlock(hwndOwner);
   pad->hwndParent = hwndParent;
   pad->hwndOwner  = hwndOwner;

   /* Query the OS/2 minor version to select the correct (undocumented)
    * color wheel message numbers: Warp 4 (minor > 30) uses 0x0601/0x0602;
    * Warp 3 uses 0x130C/0x1384. */
   DosQuerySysInfo(QSV_VERSION_MINOR, QSV_VERSION_MINOR,
                   &pad->ulVer, sizeof(ULONG));
   if (pad->ulVer > 30) {
      pad->cwChangedMsg = WM_COLORWHEELCHANGED;
      pad->cwSetMsg     = CWM_COLORWHEELSETVAL;
   } else {
      pad->cwChangedMsg = WM_COLORWHEELCHANGED3;
      pad->cwSetMsg     = CWM_COLORWHEELSETVAL3;
   }

   /* Load WPCONFIG.DLL.  This registers the "ColorSelectClass" window class
    * as a side effect, making the color wheel control available to the dialog.
    * The handle is stored so WinDeleteLibrary can unload it in WM_DESTROY. */
   if (NULLHANDLE == (pad->hlib = WinLoadLibrary(pad->hab, "WPCONFIG.DLL")))
      return CLR_ERROR;

   /* Obtain this DLL's own module handle so the dialog template (embedded
    * in EDITCOL.DLL's resources) can be loaded by WinDlgBox. */
   if (DosQueryModuleHandle(SZ_MODULE, &pad->hDlgMod))
      return CLR_ERROR;

   /* Store the initial color as both the current and the undo color.
    * The undo color is returned when the user clicks Cancel or Close. */
   pad->clrCurr.lClr = pad->clrUndo.lClr = color;

   /* Copy the dialog title, truncating at MAXTITLESIZE-1 characters.
    * Written as a compact loop to avoid pulling in strcpy/strncpy. */
   if (!pszTitle) pszTitle = SZ_DEFTITLE;
   for (i = 0;
        pad->achDlgTitle[i++] = *pszTitle++;
        i == MAXTITLESIZE - 1 ? pad->achDlgTitle[i] = 0, pszTitle = "" : 0);

   /* Run the dialog modally.  WinDlgBox returns the value passed to
    * WinDismissDlg, which is the chosen LONG RGB color. */
   return (LONG)WinDlgBox(hwndParent,
                           hwndOwner,
                           EditColorDlgProc,
                           pad->hDlgMod,
                           DLG_CWHEEL,
                           pad);
}


/*=========================================================================
 * EditColorDlgProc() - Dialog procedure for the "Edit Color" dialog.
 *
 * Handles the following messages:
 *   WM_INITDLG            - initialize controls, center dialog over parent
 *   WM_COLORWHEELCHANGED  - color wheel changed: update spin buttons
 *   WM_COLORWHEELCHANGED3 - same, Warp 3 message number
 *   WM_CONTROL            - spin button changed: update color wheel
 *   WM_COMMAND            - OK / Undo / Cancel button clicked
 *   WM_CLOSE              - system-menu Close or title-bar close button
 *   WM_DESTROY            - free WPCONFIG.DLL and APPDATA
 *=========================================================================*/
MRESULT EXPENTRY EditColorDlgProc(HWND hwnd,
                                   ULONG msg,
                                   MPARAM mp1,
                                   MPARAM mp2) {
   switch (msg) {

      /* Initialize the dialog: store APPDATA pointer, set title, initialize
       * the spin buttons and the color wheel, then position the dialog in
       * the center of the parent window. */
      case WM_INITDLG: {
         PAPPDATA pad = (PAPPDATA)mp2;
         WinSetWindowPtr(hwnd, 0L, (PVOID)mp2);
         WinSetWindowText(hwnd, pad->achDlgTitle);

         /* Set spin buttons first, then the color wheel.  flUpd is still
          * FALSE here, so the SPBN_CHANGE notifications produced by
          * SetSpins will be ignored by WM_CONTROL. */
         SetSpins(hwnd, pad->clrUndo);
         WinSendDlgItemMsg(hwnd, CWHEEL, pad->cwSetMsg,
                           (MPARAM)pad->clrUndo.lClr, MPVOID);

         /* Allow user-driven spin-button changes to update the color wheel. */
         pad->flUpd = TRUE;

         /* Center the dialog over its parent window. */
         WinQueryWindowRect(pad->hwndParent, &pad->rclPar);
         WinQueryWindowRect(hwnd, &pad->rcl);
         WinSetWindowPos(hwnd, NULLHANDLE,
                         (pad->rclPar.xRight - pad->rcl.xRight) / 2,
                         (pad->rclPar.yTop   - pad->rcl.yTop)   / 2,
                         0, 0, SWP_MOVE | SWP_SHOW);
         break;
      }

      /* The color wheel control notified us of a color change (user dragged
       * the wheel).  Update the spin buttons to match the new color, then
       * forward the notification to the owner window.
       * flUpd is cleared while SetSpins runs so that the resulting
       * SPBN_CHANGE events do not feed back into WM_CONTROL. */
      case WM_COLORWHEELCHANGED3:
      case WM_COLORWHEELCHANGED: {
         PAPPDATA pad = WinQueryWindowPtr(hwnd, 0L);
         pad->clrCurr.lClr = (LONG)mp1;
         pad->flUpd = FALSE;
         SetSpins(hwnd, pad->clrCurr);
         pad->flUpd = TRUE;
         WinSendMsg(pad->hwndOwner, msg, mp1, mp2);
         break;
      }

      /* One of the R/G/B spin buttons changed value (user typed or spun).
       * Read the new value, update the matching color component, push the
       * updated color to the wheel, and notify the owner.
       * Ignored when flUpd is FALSE (programmatic update in progress). */
      case WM_CONTROL:
         if ((SHORT1FROMMP(mp1) == SPN_RED   ||
              SHORT1FROMMP(mp1) == SPN_GREEN ||
              SHORT1FROMMP(mp1) == SPN_BLUE) &&
             (SHORT2FROMMP(mp1) == SPBN_CHANGE ||
              SHORT2FROMMP(mp1) == SPBN_ENDSPIN)) {
            PAPPDATA pad = WinQueryWindowPtr(hwnd, 0L);
            ULONG ulVal;
            if (!pad->flUpd) break;
            WinSendMsg(HWNDFROMMP(mp2), SPBM_QUERYVALUE,
                       (MPARAM)&ulVal, MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
            switch (SHORT1FROMMP(mp1)) {
               case SPN_RED:   pad->clrCurr.rgb.red = (BYTE)ulVal; break;
               case SPN_GREEN: pad->clrCurr.rgb.grn = (BYTE)ulVal; break;
               default:        pad->clrCurr.rgb.blu = (BYTE)ulVal; break;
            }
            WinSendDlgItemMsg(hwnd, CWHEEL, pad->cwSetMsg,
                              (MPARAM)pad->clrCurr.lClr, MPVOID);
            WinSendMsg(pad->hwndOwner, pad->cwChangedMsg,
                       (MPARAM)pad->clrCurr.lClr, MPVOID);
         }
         break;

      /* Push button clicked.
       * OK     : dismiss and return the current (newly chosen) color.
       * Undo   : restore the spin buttons and wheel to the original color.
       * Cancel : dismiss and return the original (undo) color. */
      case WM_COMMAND: {
         PAPPDATA pad = WinQueryWindowPtr(hwnd, 0L);
         switch ((USHORT)mp1) {
            case BTN_OK:
               WinDismissDlg(hwnd, pad->clrCurr.lClr);
               break;
            case BTN_UNDO:
               SetSpins(hwnd, pad->clrUndo);
               WinSendDlgItemMsg(hwnd, CWHEEL, pad->cwSetMsg,
                                 (MPARAM)pad->clrUndo.lClr, MPVOID);
               break;
            default: /* BTN_CANCEL */
               WinDismissDlg(hwnd, pad->clrUndo.lClr);
               break;
         }
         break;
      }

      /* System-menu Close or title-bar close button: treat as Cancel. */
      case WM_CLOSE: {
         PAPPDATA pad = WinQueryWindowPtr(hwnd, 0L);
         WinDismissDlg(hwnd, pad->clrUndo.lClr);
         break;
      }

      /* Dialog is being destroyed: unload WPCONFIG.DLL and free the
       * APPDATA block.  WinDeleteLibrary must be called before DosFreeMem
       * because it still references pad->hab and pad->hlib. */
      case WM_DESTROY: {
         PAPPDATA pad = WinQueryWindowPtr(hwnd, 0L);
         WinDeleteLibrary(pad->hab, pad->hlib);
         DosFreeMem(pad);
         break;
      }

      default:
         return WinDefDlgProc(hwnd, msg, mp1, mp2);
   }
   return (MRESULT)FALSE;
}


/*=========================================================================
 * SetSpins() - Push a CLR value into the three R/G/B spin buttons.
 *
 * Called on WM_INITDLG and whenever the color wheel reports a new color.
 * The caller must clear flUpd before calling this function and restore it
 * afterwards to prevent the resulting SPBN_CHANGE events from feeding back
 * into WM_CONTROL and triggering a redundant color-wheel update.
 *=========================================================================*/
VOID SetSpins(HWND hwnd, CLR clr) {
   WinSendDlgItemMsg(hwnd, SPN_RED,   SPBM_SETCURRENTVALUE,
                     (MPARAM)clr.rgb.red, MPVOID);
   WinSendDlgItemMsg(hwnd, SPN_GREEN, SPBM_SETCURRENTVALUE,
                     (MPARAM)clr.rgb.grn, MPVOID);
   WinSendDlgItemMsg(hwnd, SPN_BLUE,  SPBM_SETCURRENTVALUE,
                     (MPARAM)clr.rgb.blu, MPVOID);
}

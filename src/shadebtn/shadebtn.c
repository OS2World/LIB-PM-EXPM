/*
 * EXPM - Extended Presentation Manager library
 *
 * src/shadebtn/shadebtn.c -- gradient-shaded button window class.
 *
 * Merged from the LIB-PM-ShadedButtons 1.50 library (Shaded Buttons
 * 1.50).  Registers the window class WC_shBUTTON, a drop-in replacement
 * for WC_BUTTON that paints the button face as a two-tone gradient and
 * can additionally draw check boxes, radio buttons and three-state
 * buttons, rounded or open sides, vertical orientation and icon / mini
 * icon buttons.
 *
 * Original code by Hubert Chan (1997-1998); the package is distributed
 * as free software and is used here under its original permissive
 * terms.  The original file header is retained below.
 *
 * Changes for EXPM (2026):
 *   - ported from C++ (shadebtn.cpp) to conservative C;
 *   - entry points renamed to the Presentation Manager style:
 *       shBtnInit          -> XpmShadeBtnInit
 *       shBtnQueryVersion  -> XpmShadeBtnQueryVersion
 *   - the ten pointer resources were merged into EXPM.DLL and are now
 *     loaded lazily by XpmShadeBtnInit from EXPM.DLL's own module
 *     handle (the original loaded them from _DLL_InitTerm);
 *   - the PMCX wrapper (ShBtn.dll) is not part of EXPM.
 *
 * ---- original file header ------------------------------------------
 *   Shaded Button v. 1.15
 *   Hubert Chan, 1998
 *   A replacement window class for normal OS/2 buttons.
 * ---- end original file header --------------------------------------
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

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shadebtn.h"
#include "shaderes.h"


/* Class data reservation and the class data word that holds the
 * per-window GBTN pointer.  The original library took these from its
 * private pmcx.h; EXPM defines them here (QWL_USER is 0). */
#define USER_CWINDOWWORDS    8UL
#define QWW_CDATA            (QWL_USER + 4UL)

#define TYPE         7
#define HAVECAPTURE  8
#define HAVEFOCUS   16
#define INSIDERECT  32
#define SPACEDOWN   64
#define SELECTED   128
#define STATE3     256

/* Local RGB colour union (the public rgb components are blue, green,
 * red; the module keeps its own copy so it does not depend on the
 * ColorWheel CLR type). */
typedef union {
   ULONG Index;
   struct {
      UCHAR blue, green, red;
      UCHAR dummy;
   } rgb;
} CLR;

typedef struct {
   USHORT    Flags;
   char      Mnemonic;
   USHORT    MnemPos;
   HPOINTER  Bitmap;
   HPOINTER  DisBitmap;
   HPOINTER  ActBitmap;
   CLR       Colour;
   CLR       BackgroundColour;
   PSZ       pszLabel;
   PSZ       pszText;
   ULONG     ClickMsg;
   POINTL    ptlText;
   POINTL    ptlGraphic;
   POINTL    ptlTextSize;
   POINTL    ptlFocusCnr;
} GBTN, *PGBTN;


/* Check box / radio / three-state indicators, loaded from EXPM.DLL. */
static HPOINTER RadioUp      = NULLHANDLE;
static HPOINTER RadioUpSel   = NULLHANDLE;
static HPOINTER RadioDown    = NULLHANDLE;
static HPOINTER RadioDownSel = NULLHANDLE;
static HPOINTER CheckUp      = NULLHANDLE;
static HPOINTER CheckUpSel   = NULLHANDLE;
static HPOINTER CheckDown    = NULLHANDLE;
static HPOINTER CheckDownSel = NULLHANDLE;
static HPOINTER State3Up     = NULLHANDLE;
static HPOINTER State3Down   = NULLHANDLE;

static BOOL     fPointersLoaded = FALSE;


static MRESULT EXPENTRY ButtonClassProc(HWND, ULONG, MPARAM, MPARAM);
static VOID     LoadPointers(VOID);
static VOID     GBTN_CalcTextPos(HWND, PGBTN);
static VOID     GBTN_DrawText(HWND, HPS, PGBTN, ULONG);
static VOID     GBTN_DrawGradient(HPS, RECTL, CLR, CLR, int);
static VOID     GBTN_DrawBorder(HPS, RECTL, ULONG, PGBTN, int, int);
static VOID     GBTN_DrawBtn(HWND, HPS, RECTL, HPOINTER);
static MRESULT  GBTN_PaintPush(HWND, PGBTN);
static MRESULT  GBTN_PaintCheck(HWND, PGBTN);
static MRESULT  GBTN_PaintRadio(HWND, PGBTN);


/*=========================================================================
 * LoadPointers() - load the check / radio / three-state indicator
 * graphics from EXPM.DLL's own resources.  Called once, on the first
 * XpmShadeBtnInit.
 *=========================================================================*/
static VOID LoadPointers(VOID) {
   HMODULE hmod;

   if (fPointersLoaded)
      return;
   fPointersLoaded = TRUE;

   if (DosQueryModuleHandle("EXPM.DLL", &hmod))
      return;

   RadioUp      = WinLoadPointer(HWND_DESKTOP, hmod, IDP_RADIOUP);
   RadioUpSel   = WinLoadPointer(HWND_DESKTOP, hmod, IDP_RADIOUPSEL);
   RadioDown    = WinLoadPointer(HWND_DESKTOP, hmod, IDP_RADIODOWN);
   RadioDownSel = WinLoadPointer(HWND_DESKTOP, hmod, IDP_RADIODOWNSEL);
   CheckUp      = WinLoadPointer(HWND_DESKTOP, hmod, IDP_CHECKUP);
   CheckUpSel   = WinLoadPointer(HWND_DESKTOP, hmod, IDP_CHECKUPSEL);
   CheckDown    = WinLoadPointer(HWND_DESKTOP, hmod, IDP_CHECKDOWN);
   CheckDownSel = WinLoadPointer(HWND_DESKTOP, hmod, IDP_CHECKDOWNSEL);
   State3Up     = WinLoadPointer(HWND_DESKTOP, hmod, IDP_3STATEUP);
   State3Down   = WinLoadPointer(HWND_DESKTOP, hmod, IDP_3STATEDOWN);
}


/*=========================================================================
 * XpmShadeBtnInit() - register the WC_shBUTTON window class.
 * (Upstream name: shBtnInit)
 *=========================================================================*/
BOOL EXPENTRY XpmShadeBtnInit(VOID) {
   LoadPointers();
   return WinRegisterClass((HAB)0, WC_shBUTTON, ButtonClassProc,
                           CS_SIZEREDRAW, USER_CWINDOWWORDS + sizeof(PGBTN));
}


/*=========================================================================
 * XpmShadeBtnQueryVersion() - return the version times 100.
 * (Upstream name: shBtnQueryVersion)
 *=========================================================================*/
ULONG EXPENTRY XpmShadeBtnQueryVersion(VOID) {
   return 150L;
}


/*=========================================================================
 * ButtonClassProc() - window procedure of the shaded-button class.
 *=========================================================================*/
static MRESULT EXPENTRY ButtonClassProc(HWND hwnd, ULONG msg,
                                        MPARAM mp1, MPARAM mp2) {
   POINTL     ptl;
   PGBTN      pGradBtn;
   PWNDPARAMS pwprm;
   RECTL      rcl;

   pGradBtn = (PGBTN)WinQueryWindowPtr(hwnd, QWW_CDATA);

   switch (msg) {
   case WM_CREATE: {
      PCREATESTRUCT pcrst;
      ULONG         WindowStyle = WinQueryWindowULong(hwnd, QWL_STYLE);

      pGradBtn = (PGBTN)malloc(sizeof(GBTN));
      pGradBtn->Flags = WindowStyle & 7;
      pcrst = (PCREATESTRUCT)PVOIDFROMMP(mp2);
      pGradBtn->pszText = pGradBtn->pszLabel = (PSZ)malloc(1 + strlen(pcrst->pszText));
      strcpy(pGradBtn->pszLabel, pcrst->pszText);

      if (WindowStyle & (shBS_ICON | shBS_MINIICON)) {
         int resno = 0;
         if (pGradBtn->pszText[0] == '#') {
            while (isdigit(*(++pGradBtn->pszText)))
               resno = resno * 10 + (*(pGradBtn->pszText) - '0');
            pGradBtn->Bitmap = WinLoadPointer(HWND_DESKTOP, 0UL, resno);
            if (*(pGradBtn->pszText) == ',') {
               resno = 0;
               while (isdigit(*(++pGradBtn->pszText)))
                  resno = resno * 10 + (*(pGradBtn->pszText) - '0');
               pGradBtn->DisBitmap = WinLoadPointer(HWND_DESKTOP, 0UL, resno);
               if (*(pGradBtn->pszText) == ',') {
                  resno = 0;
                  while (isdigit(*(++pGradBtn->pszText)))
                     resno = resno * 10 + (*(pGradBtn->pszText) - '0');
                  pGradBtn->ActBitmap = WinLoadPointer(HWND_DESKTOP, 0UL, resno);
               } else {
                  pGradBtn->ActBitmap = NULLHANDLE;
               }
            } else {
               pGradBtn->ActBitmap = NULLHANDLE;
               pGradBtn->DisBitmap = NULLHANDLE;
            }
         } else {
            pGradBtn->Bitmap = NULLHANDLE;
            pGradBtn->DisBitmap = NULLHANDLE;
            pGradBtn->ActBitmap = NULLHANDLE;
         }
      }

      if ((WindowStyle & 7) && ((~WindowStyle) & 7))
         pGradBtn->ClickMsg = WM_CONTROL;
      else
         switch (WindowStyle & 0x0300) {
         case 0x0100: pGradBtn->ClickMsg = WM_HELP;       break;
         case 0x0200: pGradBtn->ClickMsg = WM_SYSCOMMAND; break;
         default:     pGradBtn->ClickMsg = WM_COMMAND;    break;
         }
   }
   WinSetWindowPtr(hwnd, QWW_CDATA, pGradBtn);

   if (!WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, 4UL, (PVOID)&(pGradBtn->Colour.Index), QPF_ID2COLORINDEX))
      pGradBtn->Colour.Index = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0L);
   if (!WinQueryPresParam(hwnd, shBPP_BUTTONBGCOLOR, shBPP_BUTTONBGCOLORINDEX, NULL, 4UL, (PVOID)&(pGradBtn->BackgroundColour.Index), QPF_ID2COLORINDEX))
      pGradBtn->BackgroundColour.Index = 0x00CCCCCC;

   GBTN_CalcTextPos(hwnd, pGradBtn);

   return 0;

   case shBM_SETCOLOR:
      switch (LONGFROMMP(mp2)) {
      case shBSC_BUTTONFACE:
         pGradBtn->Colour.Index = LONGFROMMP(mp1);
         WinInvalidateRect(hwnd, NULL, FALSE);
         return MRFROMSHORT(1);
      case shBSC_BACKGROUND:
         pGradBtn->BackgroundColour.Index = LONGFROMMP(mp1);
         WinInvalidateRect(hwnd, NULL, FALSE);
         return MRFROMSHORT(1);
      }
      break;

   case WM_SETWINDOWPARAMS:
      pwprm = (PWNDPARAMS)PVOIDFROMMP(mp1);
      if (pwprm->fsStatus & WPM_TEXT) {
         if (pGradBtn->Bitmap)
            WinDestroyPointer(pGradBtn->Bitmap);
         if (pGradBtn->DisBitmap)
            WinDestroyPointer(pGradBtn->DisBitmap);
         if (pGradBtn->ActBitmap)
            WinDestroyPointer(pGradBtn->ActBitmap);
         pGradBtn->ActBitmap = pGradBtn->DisBitmap = pGradBtn->Bitmap = NULLHANDLE;
         free(pGradBtn->pszLabel);
         pGradBtn->pszText = pGradBtn->pszLabel = (PSZ)malloc(1 + pwprm->cchText);
         strcpy(pGradBtn->pszLabel, pwprm->pszText);

         if (WinQueryWindowULong(hwnd, QWL_STYLE) & (shBS_ICON | shBS_MINIICON)) {
            int resno = 0;
            if (pGradBtn->pszText[0] == '#') {
               while (isdigit(*(++pGradBtn->pszText)))
                  resno = resno * 10 + (*(pGradBtn->pszText) - '0');
               pGradBtn->Bitmap = WinLoadPointer(HWND_DESKTOP, 0UL, resno);
               if (*(pGradBtn->pszText) == ',') {
                  resno = 0;
                  while (isdigit(*(++pGradBtn->pszText)))
                     resno = resno * 10 + (*(pGradBtn->pszText) - '0');
                  pGradBtn->DisBitmap = WinLoadPointer(HWND_DESKTOP, 0UL, resno);
                  if (*(pGradBtn->pszText) == ',') {
                     resno = 0;
                     while (isdigit(*(++pGradBtn->pszText)))
                        resno = resno * 10 + (*(pGradBtn->pszText) - '0');
                     pGradBtn->DisBitmap = WinLoadPointer(HWND_DESKTOP, 0UL, resno);
                  } else {
                     pGradBtn->ActBitmap = NULLHANDLE;
                  }
               } else {
                  pGradBtn->ActBitmap = NULLHANDLE;
                  pGradBtn->DisBitmap = NULLHANDLE;
               }
            } else {
               pGradBtn->ActBitmap = NULLHANDLE;
               pGradBtn->Bitmap = NULLHANDLE;
               pGradBtn->DisBitmap = NULLHANDLE;
            }
         }

         GBTN_CalcTextPos(hwnd, pGradBtn);
      } else
         break;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return MRFROMSHORT(1);

   case WM_QUERYWINDOWPARAMS:
      pwprm = (PWNDPARAMS)PVOIDFROMMP(mp1);
      if (pwprm->fsStatus & WPM_CCHTEXT)
         pwprm->cchText = strlen(pGradBtn->pszLabel);
      if (pwprm->fsStatus & WPM_TEXT)
         strcpy(pwprm->pszText, pGradBtn->pszLabel);
      if (pwprm->fsStatus & WPM_CBCTLDATA)
         pwprm->cbCtlData = 0;
      if (pwprm->fsStatus & WPM_CTLDATA)
         pwprm->pCtlData = 0;
      if (pwprm->fsStatus & (WPM_CCHTEXT | WPM_TEXT | WPM_CBCTLDATA | WPM_CTLDATA))
         return MRFROMSHORT(1);
      break;

   case WM_QUERYDLGCODE:
      switch (pGradBtn->Flags & 7) {
      case BS_PUSHBUTTON:
      case BS_USERBUTTON:
         return MRFROMLONG(DLGC_BUTTON || DLGC_PUSHBUTTON);
      case BS_RADIOBUTTON:
      case BS_AUTORADIOBUTTON:
         return MRFROMLONG(DLGC_BUTTON || DLGC_RADIOBUTTON);
      case BS_CHECKBOX:
      case BS_AUTOCHECKBOX:
      case BS_3STATE:
      case BS_AUTO3STATE:
         return MRFROMLONG(DLGC_BUTTON || DLGC_CHECKBOX);
      }
      break;

   case WM_QUERYCONVERTPOS:
      return QCP_NOCONVERT;

   case WM_PRESPARAMCHANGED:
      switch (LONGFROMMP(mp1)) {
      case 0:
         if (!WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, 4UL, (PVOID)&(pGradBtn->Colour.Index), QPF_ID2COLORINDEX))
            pGradBtn->Colour.Index = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0L);
         if (!WinQueryPresParam(hwnd, shBPP_BUTTONCOLOR, shBPP_BUTTONCOLORINDEX, NULL, 4UL, (PVOID)&(pGradBtn->BackgroundColour.Index), QPF_ID2COLORINDEX))
            pGradBtn->BackgroundColour.Index = 0x00CCCCCC;
         WinInvalidateRect(hwnd, NULL, FALSE);
         return MRFROMSHORT(1);
      case PP_BACKGROUNDCOLOR:
      case PP_BACKGROUNDCOLORINDEX:
         if (!WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, 4UL, (PVOID)&(pGradBtn->Colour.Index), QPF_ID2COLORINDEX))
            pGradBtn->Colour.Index = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0L);
         WinInvalidateRect(hwnd, NULL, FALSE);
         return MRFROMSHORT(1);
      case shBPP_BUTTONBGCOLOR:
      case shBPP_BUTTONBGCOLORINDEX:
         if (!WinQueryPresParam(hwnd, shBPP_BUTTONCOLOR, shBPP_BUTTONCOLORINDEX, NULL, 4UL, (PVOID)&(pGradBtn->BackgroundColour.Index), QPF_ID2COLORINDEX))
            pGradBtn->BackgroundColour.Index = 0x00CCCCCC;
         WinInvalidateRect(hwnd, NULL, FALSE);
         return MRFROMSHORT(1);
      case PP_FONTNAMESIZE: {
         MRESULT retval = WinDefWindowProc(hwnd, msg, mp1, mp2);
         GBTN_CalcTextPos(hwnd, pGradBtn);
         WinInvalidateRect(hwnd, NULL, FALSE);
         return retval;
      }
      }
      break;

   case WM_SYSCOLORCHANGE:
      if (WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, 4UL, (PVOID)&(pGradBtn->Colour.Index), QPF_ID2COLORINDEX))
         pGradBtn->Colour.Index = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0L);
      WinInvalidateRect(hwnd, NULL, FALSE);
      break;

   case WM_MATCHMNEMONIC:
      return FALSE;

   case BM_SETCHECK:
      pGradBtn->Flags = (pGradBtn->Flags & ~(SELECTED | STATE3)) | (SHORT1FROMMP(mp1) << 7);
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;

   case BM_QUERYCHECK:
      return MRFROMLONG((pGradBtn->Flags >> 7) & 3);

   case WM_BUTTON1DOWN:
      if (WinQueryWindowULong(hwnd, QWL_STYLE) & shBS_NOPUSH)
         return 0;
      WinSetCapture(HWND_DESKTOP, hwnd);
      pGradBtn->Flags |= HAVECAPTURE | INSIDERECT;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;

   case WM_MOUSEMOVE: {
      BOOL fTestInsideRect;

      if ((pGradBtn->Flags & HAVECAPTURE) == 0)
         return 0;
      WinQueryWindowRect(hwnd, &rcl);
      ptl.x = MOUSEMSG(&msg)->x;
      ptl.y = MOUSEMSG(&msg)->y;
      fTestInsideRect = WinPtInRect(WinQueryAnchorBlock(hwnd), &rcl, &ptl);
      if (((pGradBtn->Flags & INSIDERECT) != 0) != fTestInsideRect) {
         pGradBtn->Flags ^= INSIDERECT;
         WinInvalidateRect(hwnd, NULL, FALSE);
      }
   }
   break;

   case WM_BUTTON1UP:
      if (((pGradBtn->Flags & HAVECAPTURE) == 0) || !WinIsWindowEnabled(hwnd))
         return 0;
      WinSetCapture(HWND_DESKTOP, NULLHANDLE);
      pGradBtn->Flags &= ~(HAVECAPTURE | INSIDERECT);
      WinQueryWindowRect(hwnd, &rcl);
      ptl.x = MOUSEMSG(&msg)->x;
      ptl.y = MOUSEMSG(&msg)->y;
      if (WinPtInRect(WinQueryAnchorBlock(hwnd), &rcl, &ptl)) {
         WinPostMsg(WinQueryWindow(hwnd, QW_OWNER), pGradBtn->ClickMsg,
                    MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), BN_CLICKED),
                    MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));
         WinSetFocus(HWND_DESKTOP, hwnd);
         if (!(pGradBtn->Flags & 1)) {
            if ((pGradBtn->Flags & 7) == BS_AUTO3STATE) {
               if (pGradBtn->Flags & STATE3)
                  pGradBtn->Flags = (pGradBtn->Flags & ~(STATE3 | SELECTED));
               else if (pGradBtn->Flags & SELECTED)
                  pGradBtn->Flags = (pGradBtn->Flags & ~SELECTED) | STATE3;
               else
                  pGradBtn->Flags |= SELECTED;
            } else
               pGradBtn->Flags ^= SELECTED;
         }
      }
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;

   case WM_ENABLE:
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;

   case WM_SETFOCUS:
      if (SHORT1FROMMP(mp2))
         pGradBtn->Flags |= HAVEFOCUS;
      else
         pGradBtn->Flags &= ~HAVEFOCUS;
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;

   case WM_CHAR:
      if (!(CHARMSG(&msg)->fs & KC_VIRTUALKEY) ||
          CHARMSG(&msg)->fs & KC_PREVDOWN    ||
          !WinIsWindowEnabled(hwnd))
         break;
      if ((CHARMSG(&msg)->fs & KC_KEYUP)) {
         switch (CHARMSG(&msg)->vkey) {
         case VK_ENTER:
         case VK_SPACE:
            pGradBtn->Flags &= ~SPACEDOWN;
            WinPostMsg(WinQueryWindow(hwnd, QW_OWNER), pGradBtn->ClickMsg,
                       MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), BN_CLICKED),
                       MPFROM2SHORT(CMDSRC_PUSHBUTTON, FALSE));
            break;
         default:
            return WinDefWindowProc(hwnd, msg, mp1, mp2);
         }
      } else {
         switch (CHARMSG(&msg)->vkey) {
         case VK_ENTER:
         case VK_SPACE:
            pGradBtn->Flags |= SPACEDOWN;
            break;
         case VK_TAB:
            WinSetFocus(HWND_DESKTOP, WinEnumDlgItem(WinQueryWindow(hwnd, QW_OWNER),
                                                     hwnd, EDI_NEXTTABITEM));
            break;
         case VK_RIGHT:
         case VK_DOWN:
            WinSetFocus(HWND_DESKTOP, WinEnumDlgItem(WinQueryWindow(hwnd, QW_OWNER),
                                                     hwnd, EDI_NEXTGROUPITEM));
            break;
         case VK_LEFT:
         case VK_UP:
            WinSetFocus(HWND_DESKTOP, WinEnumDlgItem(WinQueryWindow(hwnd, QW_OWNER),
                                                     hwnd, EDI_PREVGROUPITEM));
            break;
         case VK_BACKTAB:
            WinSetFocus(HWND_DESKTOP, WinEnumDlgItem(WinQueryWindow(hwnd, QW_OWNER),
                                                     hwnd, EDI_PREVTABITEM));
            break;
         default:
            return WinDefWindowProc(hwnd, msg, mp1, mp2);
         }
      }
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;

   case WM_SIZE:
      GBTN_CalcTextPos(hwnd, pGradBtn);
      WinInvalidateRect(hwnd, NULL, FALSE);
      return 0;

   case WM_PAINT:
      switch (pGradBtn->Flags & TYPE) {
      case shBS_PUSHBUTTON:
         return GBTN_PaintPush(hwnd, pGradBtn);
      case shBS_CHECKBOX:
      case shBS_AUTOCHECKBOX:
      case shBS_3STATE:
      case shBS_AUTO3STATE:
         return GBTN_PaintCheck(hwnd, pGradBtn);
      case shBS_RADIOBUTTON:
      case shBS_AUTORADIOBUTTON:
         return GBTN_PaintRadio(hwnd, pGradBtn);
      case shBS_USERBUTTON:
         WinPostMsg(WinQueryWindow(hwnd, QW_OWNER), pGradBtn->ClickMsg,
                    MPFROM2SHORT(WinQueryWindowUShort(hwnd, QWS_ID), BN_PAINT),
                    0);
         return 0;
      }
      return 0;

   case WM_DESTROY:
      if (pGradBtn->Bitmap)
         WinDestroyPointer(pGradBtn->Bitmap);
      if (pGradBtn->DisBitmap)
         WinDestroyPointer(pGradBtn->DisBitmap);
      if (pGradBtn->ActBitmap)
         WinDestroyPointer(pGradBtn->ActBitmap);
      free(pGradBtn->pszLabel);
      free(pGradBtn);
      return 0;
   }
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


static VOID GBTN_CalcTextPos(HWND hwnd, PGBTN pGradBtn) {
   POINTL  aptl[TXTBOX_COUNT];
   HPS     hps;
   RECTL   rcl;
   ULONG   WindowStyle;
   LONG    PWidth, PHeight;

   hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);

   GpiQueryTextBox(hps, strlen(pGradBtn->pszText), pGradBtn->pszText, TXTBOX_COUNT, aptl);

   WinEndPaint(hps);

   pGradBtn->ptlTextSize.x = aptl[TXTBOX_CONCAT].x;
   pGradBtn->ptlTextSize.y = aptl[TXTBOX_TOPLEFT].y + aptl[TXTBOX_BOTTOMLEFT].y;

   WinQueryWindowRect(hwnd, &rcl);
   WindowStyle = WinQueryWindowULong(hwnd, QWL_STYLE);

   PWidth = WinQuerySysValue(HWND_DESKTOP, SV_CXPOINTER);
   PHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYPOINTER);

   /* Centre vertically */
   pGradBtn->ptlText.y = rcl.yTop - pGradBtn->ptlTextSize.y;
   if (WindowStyle & shBS_VERTICAL) {
      if (WindowStyle & shBS_BOTTOMROUND)
         pGradBtn->ptlText.y += rcl.yTop >> 1;
      if (WindowStyle & shBS_TOPROUND)
         pGradBtn->ptlText.y -= rcl.yTop >> 1;
   }
   pGradBtn->ptlText.y >>= 1;

   if ((pGradBtn->Flags & TYPE) == shBS_PUSHBUTTON) { /* If button, centre horiz */
      pGradBtn->ptlText.x = rcl.xRight - pGradBtn->ptlTextSize.x;
      if (!(WindowStyle & shBS_VERTICAL)) {
         if (WindowStyle & shBS_LEFTROUND)
            pGradBtn->ptlGraphic.x += rcl.xRight >> 1;
         if (WindowStyle & shBS_RIGHTROUND)
            pGradBtn->ptlGraphic.x -= rcl.xRight >> 1;
      }
      pGradBtn->ptlText.x >>= 1;
   } else {
      pGradBtn->ptlText.x = PWidth >> 1;
   }

   if (WindowStyle & (shBS_MINIICON | shBS_ICON)) {
      if (WindowStyle & shBS_MINIICON) {
         PWidth >>= 1;
         PHeight >>= 1;
      }
      pGradBtn->ptlTextSize.x = PWidth;
      pGradBtn->ptlTextSize.y = PHeight;

      /* Centre vertically */
      pGradBtn->ptlGraphic.y = rcl.yTop - PHeight;
      if (WindowStyle & shBS_VERTICAL) {
         if (WindowStyle & shBS_BOTTOMROUND)
            pGradBtn->ptlGraphic.y += rcl.yTop >> 1;
         if (WindowStyle & shBS_TOPROUND)
            pGradBtn->ptlGraphic.y -= rcl.yTop >> 1;
      }
      pGradBtn->ptlGraphic.y >>= 1;

      if ((pGradBtn->Flags & TYPE) == shBS_PUSHBUTTON) { /* If button, centre horiz */
         pGradBtn->ptlGraphic.x = rcl.xRight - PWidth;
         if (!(WindowStyle & shBS_VERTICAL)) {
            if (WindowStyle & shBS_LEFTROUND)
               pGradBtn->ptlGraphic.x += rcl.xRight >> 1;
            if (WindowStyle & shBS_RIGHTROUND)
               pGradBtn->ptlGraphic.x -= rcl.xRight >> 1;
         }
         pGradBtn->ptlGraphic.x >>= 1;
         pGradBtn->ptlText.y -= PHeight >> 1; /* text goes under icon */
      } else {
         pGradBtn->ptlGraphic.x = (WindowStyle & shBS_MINIICON) ? PWidth : PWidth >> 1;
         pGradBtn->ptlText.x += PWidth; /* text goes beside icon */
      }
   }
}


static VOID GBTN_DrawText(HWND hwnd, HPS hps, PGBTN pGradBtn, ULONG WindowStyle) {
/* Draws text or icon.  The text/icon and the location is given in the
 * pGradBtn parameter. */
   POINTL ptl;
   POINTL aptl[4];
   ULONG  PtrS;

   aptl[0].x = aptl[3].x = pGradBtn->ptlText.x - 2;
   aptl[0].y = aptl[1].y = pGradBtn->ptlText.y - 4;
   aptl[1].x = aptl[2].x = aptl[0].x + pGradBtn->ptlTextSize.x + 4;
   aptl[2].y = aptl[3].y = aptl[0].y + pGradBtn->ptlTextSize.y + 8;

   if (pGradBtn->Bitmap) {
      aptl[0].x = aptl[3].x = pGradBtn->ptlGraphic.x;
      aptl[0].y = aptl[1].y = pGradBtn->ptlGraphic.y;
      aptl[1].x = aptl[2].x = aptl[0].x + pGradBtn->ptlTextSize.x;
      aptl[2].y = aptl[3].y = aptl[0].y + pGradBtn->ptlTextSize.y;
      ptl.x = pGradBtn->ptlGraphic.x;
      ptl.y = pGradBtn->ptlGraphic.y;
      if (pGradBtn->Flags & (INSIDERECT | SPACEDOWN) && !(pGradBtn->Flags & TYPE) && !pGradBtn->ActBitmap) {
         ptl.x++;
         ptl.y--;
      }
      if (WindowStyle & shBS_MINIICON)
         PtrS = DP_MINI;
      else
         PtrS = 0;
      if (WinIsWindowEnabled(hwnd)) {
         if (pGradBtn->Flags & (INSIDERECT | SPACEDOWN) && pGradBtn->ActBitmap)
            WinDrawPointer(hps, ptl.x, ptl.y, pGradBtn->ActBitmap, DP_NORMAL | PtrS);
         else
            WinDrawPointer(hps, ptl.x, ptl.y, pGradBtn->Bitmap, DP_NORMAL | PtrS);
      } else {
         if (pGradBtn->DisBitmap)
            WinDrawPointer(hps, ptl.x, ptl.y, pGradBtn->DisBitmap, DP_NORMAL | PtrS);
         else
            WinDrawPointer(hps, ptl.x, ptl.y, pGradBtn->Bitmap, DP_HALFTONED | PtrS);
      }
   }

   ptl.x = pGradBtn->ptlText.x;
   ptl.y = pGradBtn->ptlText.y;
   if (pGradBtn->Flags & (INSIDERECT | SPACEDOWN) && !(pGradBtn->Flags & TYPE)) {
      ptl.x++;
      ptl.y--;
   }
   if (WinIsWindowEnabled(hwnd)) {
      GpiSetColor(hps, 0x00000000);
      GpiCharStringAt(hps, &ptl, strlen(pGradBtn->pszText), pGradBtn->pszText);
   } else {
      GpiSetColor(hps, 0x00FFFFFF);
      GpiCharStringAt(hps, &ptl, strlen(pGradBtn->pszText), pGradBtn->pszText);
      ptl.x--; ptl.y++;
      GpiSetColor(hps, 0x005F5F5F);
      GpiCharStringAt(hps, &ptl, strlen(pGradBtn->pszText), pGradBtn->pszText);
   }

   if (pGradBtn->Flags & HAVEFOCUS) {
      GpiSetColor(hps, 0x00000000);
      GpiSetLineType(hps, LINETYPE_DOT);
      GpiMove(hps, aptl + 3);
      GpiPolyLine(hps, 4L, aptl);
   }
}


static VOID GBTN_DrawGradient(HPS hps, RECTL rcl, CLR Start, CLR End, int Dir) {
   /* Dir indicates the direction - 0 for horizontal, 1 for vertical */
   POINTL left, right;
   int    height;
   CLR    d_colr;
   int    Red = 0, Green = 0, Blue = 0;
   int    delRed, delGreen, delBlue;
   int    maxRed, maxGreen, maxBlue;

   height = Dir ? rcl.xRight - rcl.xLeft : rcl.yTop - rcl.yBottom;

   d_colr.Index = 0;
   maxRed = End.rgb.red - Start.rgb.red;
   maxGreen = End.rgb.green - Start.rgb.green;
   maxBlue = End.rgb.blue - Start.rgb.blue;
   delRed = maxRed % height;
   delGreen = maxGreen % height;
   delBlue = maxBlue % height;
   d_colr.rgb.red = maxRed / height;
   d_colr.rgb.green = maxGreen / height;
   d_colr.rgb.blue = maxBlue / height;
   if (Dir) {
      left.y = rcl.yBottom;
      right.y = rcl.yTop;
      for (left.x = right.x = height - 1; right.x > 0; right.x--, left.x--) {
         Start.Index += d_colr.Index;
         GpiSetColor(hps, Start.Index);
         GpiMove(hps, &left);
         GpiLine(hps, &right);
         Red += delRed;
         if (Red > height) {
            Start.rgb.red++;
            Red -= height;
         }
         Green += delGreen;
         if (Green > height) {
            Start.rgb.green++;
            Green -= height;
         }
         Blue += delBlue;
         if (Blue > height) {
            Start.rgb.blue++;
            Blue -= height;
         }
      }
   } else {
      left.x = rcl.xLeft;
      right.x = rcl.xRight;
      for (left.y = right.y = 1; right.y < height; right.y++, left.y++) {
         Start.Index += d_colr.Index;
         GpiSetColor(hps, Start.Index);
         GpiMove(hps, &left);
         GpiLine(hps, &right);
         Red += delRed;
         if (Red > height) {
            Start.rgb.red++;
            Red -= height;
         }
         Green += delGreen;
         if (Green > height) {
            Start.rgb.green++;
            Green -= height;
         }
         Blue += delBlue;
         if (Blue > height) {
            Start.rgb.blue++;
            Blue -= height;
         }
      }
   }
}


static VOID GBTN_DrawBorder(HPS hps, RECTL rcl, ULONG WindowStyle, PGBTN pGradBtn, int Dir, int Clip) {
   POINTL ptl, right;
   long   height;

   height = Dir ? rcl.xRight - rcl.xLeft : rcl.yTop - rcl.yBottom;

   rcl.xRight--;
   rcl.yTop--;

   if (Dir) {

      /* Vertical */
      /* Move to lower right */
      ptl.x = rcl.xRight;
      ptl.y = (WindowStyle & shBS_BOTTOMROUND) ? rcl.yBottom + height / 2 : rcl.yBottom;
      GpiMove(hps, &ptl);
      /* Move to the upper right */
      ptl.y = (WindowStyle & shBS_TOPROUND) ? rcl.yTop - height / 2 : rcl.yTop;
      GpiSetColor(hps, 0x00888888);
      GpiLine(hps, &ptl);
      /* Move to the upper left */
      ptl.x = rcl.xLeft;
      if (WindowStyle & shBS_TOPROUND) {
         ARCPARAMS arcparams;
         arcparams.lP = arcparams.lQ = height / 2;
         arcparams.lR = arcparams.lS = 0L;
         GpiSetArcParams(hps, &arcparams);
         right.y = rcl.yTop - height / 2;
         right.x = height / 2;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(0,0), MAKEFIXED(45,0));
         if (!(pGradBtn->Flags & (INSIDERECT | SPACEDOWN)))
            GpiSetColor(hps, 0x00FFFFFF);
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(45,0), MAKEFIXED(45,0));
         if (!(height % 2))
            right.x++;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(90,0), MAKEFIXED(90,0));
         GpiLine(hps, &ptl);
      }
      if (!(pGradBtn->Flags & (INSIDERECT | SPACEDOWN)))
         GpiSetColor(hps, 0x00FFFFFF);
      if (!(WindowStyle & shBS_NOTOPLINE) || Clip)
         GpiLine(hps, &ptl);
      else
         GpiMove(hps, &ptl);
      /* Move to the lower left */
      ptl.y = (WindowStyle & shBS_BOTTOMROUND) ? rcl.yBottom + height / 2 : rcl.yBottom;
      GpiLine(hps, &ptl);
      /* Move to the lower right */
      ptl.x = rcl.xRight;
      if (WindowStyle & shBS_BOTTOMROUND) {
         ARCPARAMS arcparams;
         arcparams.lP = arcparams.lQ = height / 2;
         arcparams.lR = arcparams.lS = 0L;
         GpiSetArcParams(hps, &arcparams);
         right.y = right.x = height / 2;
         if (!(height % 2))
            right.x++;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(180,0), MAKEFIXED(45,0));
         GpiSetColor(hps, 0x00888888);
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(215,0), MAKEFIXED(45,0));
         if (!(height % 2))
            right.x--;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(270,0), MAKEFIXED(90,0));
      }
      if (!(pGradBtn->Flags & (INSIDERECT | SPACEDOWN)))
         GpiSetColor(hps, 0x00888888);
      if (!(WindowStyle & shBS_NOBOTTOMLINE) || Clip)
         GpiLine(hps, &ptl);

   } else {

      /* Horizontal */
      ptl.y = rcl.yBottom;
      ptl.x = (WindowStyle & shBS_LEFTROUND) ? rcl.xLeft + height / 2 : rcl.xLeft;
      GpiMove(hps, &ptl);
      ptl.x = (WindowStyle & shBS_RIGHTROUND) ? rcl.xRight - height / 2 : rcl.xRight;
      GpiSetColor(hps, 0x00888888);
      GpiLine(hps, &ptl);
      ptl.y = rcl.yTop;
      if (WindowStyle & shBS_RIGHTROUND) {
         ARCPARAMS arcparams;
         arcparams.lP = arcparams.lQ = height / 2;
         arcparams.lR = arcparams.lS = 0L;
         GpiSetArcParams(hps, &arcparams);
         right.x = rcl.xRight - height / 2;
         right.y = height / 2;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(270,0), MAKEFIXED(90,0));
         if (!(height % 2))
            right.y--;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(0,0), MAKEFIXED(45,0));
         GpiSetColor(hps, (pGradBtn->Flags & (INSIDERECT | SPACEDOWN) ? 0x00888888 : 0x00FFFFFF));
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(45,0), MAKEFIXED(45,0));
      }
      if (!(WindowStyle & shBS_NORIGHTLINE) || Clip)
         GpiLine(hps, &ptl);
      else
         GpiMove(hps, &ptl);
      GpiSetColor(hps, (pGradBtn->Flags & (INSIDERECT | SPACEDOWN) ? 0x00888888 : 0x00FFFFFF));
      ptl.x = (WindowStyle & shBS_LEFTROUND) ? rcl.xLeft + height / 2 : rcl.xLeft;
      GpiLine(hps, &ptl);
      ptl.y = rcl.yBottom;
      if (WindowStyle & shBS_LEFTROUND) {
         ARCPARAMS arcparams;
         arcparams.lP = arcparams.lQ = height / 2;
         arcparams.lR = arcparams.lS = 0L;
         GpiSetArcParams(hps, &arcparams);
         right.y = right.x = height / 2;
         if (!(height % 2))
            right.y--;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(90,0), MAKEFIXED(90,0));
         if (!(height % 2))
            right.y++;
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(180,0), MAKEFIXED(45,0));
         GpiSetColor(hps, 0x00888888);
         GpiPartialArc(hps, &right, MAKEFIXED(1,0), MAKEFIXED(215,0), MAKEFIXED(45,0));
      }
      if (!(WindowStyle & shBS_NOLEFTLINE) || Clip)
         GpiLine(hps, &ptl);
   }
}


static MRESULT GBTN_PaintPush(HWND hwnd, PGBTN pGradBtn) {
   RECTL rcl;
   HPS   hps;
   CLR   colr, d_colr;
   ULONG WindowStyle;

   /* Initialize */
   WinQueryWindowRect(hwnd, &rcl);
   WindowStyle = WinQueryWindowULong(hwnd, QWL_STYLE);
   hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
   GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
   WinFillRect(hps, &rcl, pGradBtn->BackgroundColour.Index);

   /* Draw clipping region */
   GpiBeginPath(hps, 1);
   GBTN_DrawBorder(hps, rcl, WindowStyle, pGradBtn, WindowStyle & shBS_VERTICAL, 1);
   GpiEndPath(hps);
   GpiSetClipPath(hps, 1, SCP_AND | SCP_ALTERNATE);

   /* Draw gradient pattern */
   colr.Index = 0UL;
   d_colr.Index = 0UL;
   colr.rgb.red   = pGradBtn->Colour.rgb.red   / 2;
   colr.rgb.green = pGradBtn->Colour.rgb.green / 2;
   colr.rgb.blue  = pGradBtn->Colour.rgb.blue  / 2;
   if (pGradBtn->Flags & (INSIDERECT | SPACEDOWN)) {
      d_colr.rgb.red   = pGradBtn->Colour.rgb.red   * 5 / 6;
      d_colr.rgb.green = pGradBtn->Colour.rgb.green * 5 / 6;
      d_colr.rgb.blue  = pGradBtn->Colour.rgb.blue  * 5 / 6;
   } else {
      d_colr.rgb.red   = pGradBtn->Colour.rgb.red;
      d_colr.rgb.green = pGradBtn->Colour.rgb.green;
      d_colr.rgb.blue  = pGradBtn->Colour.rgb.blue;
   }
   GBTN_DrawGradient(hps, rcl, colr, d_colr, WindowStyle & shBS_VERTICAL);

   /* Draw border */
   GBTN_DrawBorder(hps, rcl, WindowStyle, pGradBtn, WindowStyle & shBS_VERTICAL, 0);

   GBTN_DrawText(hwnd, hps, pGradBtn, WindowStyle);

   WinEndPaint(hps);
   return 0;
}


static VOID GBTN_DrawBtn(HWND hwnd, HPS hps, RECTL rcl, HPOINTER icon) {
   LONG PHeight;

   PHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYPOINTER) >> 1;
   if (WinIsWindowEnabled(hwnd))
      WinDrawPointer(hps, 0, (rcl.yTop - PHeight) >> 1, icon, DP_NORMAL | DP_MINI);
   else
      WinDrawPointer(hps, 0, (rcl.yTop - PHeight) >> 1, icon, DP_HALFTONED | DP_MINI);
}


static MRESULT GBTN_PaintCheck(HWND hwnd, PGBTN pGradBtn) {
   RECTL rcl;
   HPS   hps;
   ULONG WindowStyle;

   /* Initialize */
   WinQueryWindowRect(hwnd, &rcl);
   WindowStyle = WinQueryWindowULong(hwnd, QWL_STYLE);
   hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
   GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
   WinFillRect(hps, &rcl, pGradBtn->Colour.Index);

   if (pGradBtn->Flags & (INSIDERECT | SPACEDOWN)) {
      if (pGradBtn->Flags & STATE3)
         GBTN_DrawBtn(hwnd, hps, rcl, State3Down);
      else if (pGradBtn->Flags & SELECTED)
         GBTN_DrawBtn(hwnd, hps, rcl, CheckDownSel);
      else
         GBTN_DrawBtn(hwnd, hps, rcl, CheckDown);
   } else {
      if (pGradBtn->Flags & STATE3)
         GBTN_DrawBtn(hwnd, hps, rcl, State3Up);
      else if (pGradBtn->Flags & SELECTED)
         GBTN_DrawBtn(hwnd, hps, rcl, CheckUpSel);
      else
         GBTN_DrawBtn(hwnd, hps, rcl, CheckUp);
   }

   /* Draw Text */
   GBTN_DrawText(hwnd, hps, pGradBtn, WindowStyle);

   WinEndPaint(hps);
   return 0;
}


static MRESULT GBTN_PaintRadio(HWND hwnd, PGBTN pGradBtn) {
   RECTL rcl;
   HPS   hps;
   ULONG WindowStyle;

   /* Initialize */
   WinQueryWindowRect(hwnd, &rcl);
   WindowStyle = WinQueryWindowULong(hwnd, QWL_STYLE);
   hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
   GpiCreateLogColorTable(hps, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
   WinFillRect(hps, &rcl, pGradBtn->Colour.Index);

   if (pGradBtn->Flags & (INSIDERECT | SPACEDOWN)) {
      if (pGradBtn->Flags & SELECTED)
         GBTN_DrawBtn(hwnd, hps, rcl, RadioDownSel);
      else
         GBTN_DrawBtn(hwnd, hps, rcl, RadioDown);
   } else {
      if (pGradBtn->Flags & SELECTED)
         GBTN_DrawBtn(hwnd, hps, rcl, RadioUpSel);
      else
         GBTN_DrawBtn(hwnd, hps, rcl, RadioUp);
   }

   /* Draw Text */
   GBTN_DrawText(hwnd, hps, pGradBtn, WindowStyle);

   WinEndPaint(hps);
   return 0;
}

/*
 * EXPM - Extended Presentation Manager library
 *
 * src/anibtn/anibtn.c -- animated two-state graphical push buttons.
 *
 * Clean-room reimplementation of the AniButtn 1.02 library by Greg
 * Ratajik (1993), written from its published header (ANIBUTTN.H) and
 * manual (ANIBUTTN.DOC) only; no original source was used.
 *
 * XpmCreateAniButton() turns an ordinary WC_BUTTON / BS_USERBUTTON
 * control into a graphical push button that shows one bitmap while it
 * is up and another while it is pressed, and (optionally) displays a
 * help string in a static-text field while the pointer is over it.  The
 * button keeps behaving like a normal push button: it sends WM_COMMAND
 * / BN_CLICKED to its owner.
 *
 * How it works: a BS_USERBUTTON does not draw itself; instead it asks
 * its owner to draw it by sending WM_CONTROL with code BN_PAINT, whose
 * mp2 is a PUSERBUTTON carrying the presentation space and the current
 * draw state.  The library subclasses the owning dialog to answer that
 * request, choosing the up bitmap or the down bitmap from the
 * BDS_HILITED state.  A second subclass on the button itself watches
 * WM_MOUSEMOVE to update the status-text field, and the dialog's
 * WM_MOUSEMOVE restores the field to its original text.
 *
 * The upstream entry points AniCreateButton / AniDeleteButton are
 * renamed to the PM Win* style:
 *     AniCreateButton  ->  XpmCreateAniButton
 *     AniDeleteButton  ->  XpmDeleteAniButton
 * See doc/anibtn.txt for the API reference.
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
#include "expm.h"


/* One ANIBTN node per animated button.  The list is process-wide, as in
 * the original library; EXPM.DLL keeps the head in its static data. */
typedef struct _ANIBTN {
   HWND           hwndDlg;      /* dialog owning the button            */
   HWND           hwndBtn;      /* the BS_USERBUTTON control itself    */
   SHORT          id;           /* control identifier                  */
   USHORT         idUp;         /* bitmap resource id, up state        */
   USHORT         idDown;       /* bitmap resource id, down state      */
   USHORT         cx;           /* bitmap width in pels                */
   USHORT         cy;           /* bitmap height in pels               */
   USHORT         idStatus;     /* status-text field id (0 = none)     */
   PSZ            pszStatus;    /* status text (heap copy or NULL)     */
   HBITMAP        hbmUp;        /* loaded up bitmap                    */
   HBITMAP        hbmDown;      /* loaded down bitmap                  */
   PFNWP          pOldBtnProc;  /* button proc before subclassing      */
   struct _ANIBTN *pNext;
} ANIBTN, *PANIBTN;

/* One ANISTAT node per status-text field, remembering the text the field
 * held before the library ever changed it, so it can be restored when
 * the pointer leaves a button. */
typedef struct _ANISTAT {
   HWND            hwndDlg;     /* dialog owning the field             */
   USHORT          id;          /* field identifier                    */
   PSZ             pszOrig;     /* original field text (heap copy)     */
   struct _ANISTAT *pNext;
} ANISTAT, *PANISTAT;

/* One ANIDLG node per subclassed dialog, remembering the dialog proc we
 * replaced. */
typedef struct _ANIDLG {
   HWND           hwndDlg;      /* the subclassed dialog               */
   PFNWP          pOldDlgProc;  /* dialog proc before subclassing      */
   struct _ANIDLG *pNext;
} ANIDLG, *PANIDLG;


static PANIBTN  pBtnList  = NULL;    /* all animated buttons in this process */
static PANISTAT pStatList = NULL;    /* all managed status fields            */
static PANIDLG  pDlgList  = NULL;    /* all subclassed dialogs               */


static MRESULT EXPENTRY AniBtnProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY AniDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static PANIBTN  FindBtn (HWND hwndDlg, SHORT id);
static PANISTAT FindStat(HWND hwndDlg, USHORT id);
static PANIDLG  FindDlg (HWND hwndDlg);
static VOID     AddStat (HWND hwndDlg, USHORT id);
static VOID     RestoreStatus(HWND hwndDlg);
static VOID     DrawButton(PANIBTN p, HPS hps, ULONG fsState);
static VOID     FreeDialog(HWND hwndDlg);


/*=========================================================================
 * XpmCreateAniButton() - make an existing user button animated.
 *=========================================================================*/
VOID EXPENTRY XpmCreateAniButton(HWND hwndDlg,
                                 SHORT id,
                                 USHORT idUp,
                                 USHORT idDown,
                                 USHORT cx,
                                 USHORT cy,
                                 PSZ pszStatus,
                                 USHORT idStatus) {
   PANIBTN p;
   PANIDLG pD;
   HWND    hwndBtn;
   HPS     hps;

   if (!hwndDlg)
      return;

   /* Ignore a second call for the same button. */
   if (FindBtn(hwndDlg, id))
      return;

   hwndBtn = WinWindowFromID(hwndDlg, id);
   if (!hwndBtn)
      return;

   p = (PANIBTN)malloc(sizeof(ANIBTN));
   if (!p)
      return;
   memset(p, 0, sizeof(ANIBTN));

   p->hwndDlg  = hwndDlg;
   p->hwndBtn  = hwndBtn;
   p->id       = id;
   p->idUp     = idUp;
   p->idDown   = idDown;
   p->cx       = cx;
   p->cy       = cy;
   p->idStatus = idStatus;

   /* Keep a private copy of the status string. */
   if (pszStatus) {
      p->pszStatus = (PSZ)malloc(strlen(pszStatus) + 1);
      if (p->pszStatus)
         strcpy(p->pszStatus, pszStatus);
   }

   /* Load the two bitmap resources from the application executable. */
   hps = WinGetPS(hwndBtn);
   if (hps) {
      p->hbmUp   = GpiLoadBitmap(hps, NULLHANDLE, idUp, cx, cy);
      p->hbmDown = GpiLoadBitmap(hps, NULLHANDLE, idDown, cx, cy);
      WinReleasePS(hps);
   }

   /* Subclass the button to catch WM_MOUSEMOVE (status help). */
   p->pOldBtnProc = WinSubclassWindow(hwndBtn, AniBtnProc);
   WinSetWindowPtr(hwndBtn, QWL_USER, (PVOID)p);

   /* Remember the status field's original text the first time we see it. */
   if (idStatus && !FindStat(hwndDlg, idStatus))
      AddStat(hwndDlg, idStatus);

   /* Subclass the dialog once, to answer BN_PAINT. */
   if (!FindDlg(hwndDlg)) {
      pD = (PANIDLG)malloc(sizeof(ANIDLG));
      if (pD) {
         pD->hwndDlg     = hwndDlg;
         pD->pOldDlgProc = WinSubclassWindow(hwndDlg, AniDlgProc);
         pD->pNext       = pDlgList;
         pDlgList        = pD;
      }
   }

   p->pNext = pBtnList;
   pBtnList = p;

   /* Ask for a first BN_PAINT so the up bitmap appears immediately. */
   WinInvalidateRect(hwndBtn, NULL, FALSE);
}


/*=========================================================================
 * XpmDeleteAniButton() - release an animated button.
 *=========================================================================*/
VOID EXPENTRY XpmDeleteAniButton(HWND hwndDlg, USHORT id) {
   PANIBTN p, *pp;
   BOOL    fMore;

   pp = &pBtnList;
   while (*pp && ((*pp)->hwndDlg != hwndDlg || (*pp)->id != (SHORT)id))
      pp = &(*pp)->pNext;
   if (!*pp)
      return;
   p = *pp;
   *pp = p->pNext;

   /* Restore the button's original window procedure. */
   if (WinIsWindow(NULLHANDLE, p->hwndBtn)) {
      WinSetWindowPtr(p->hwndBtn, QWL_USER, NULL);
      if (p->pOldBtnProc)
         WinSubclassWindow(p->hwndBtn, p->pOldBtnProc);
   }

   if (p->pszStatus)
      free(p->pszStatus);
   if (p->hbmUp)
      GpiDeleteBitmap(p->hbmUp);
   if (p->hbmDown)
      GpiDeleteBitmap(p->hbmDown);
   free(p);

   /* If that was the dialog's last animated button, undo the rest. */
   fMore = FALSE;
   for (p = pBtnList; p; p = p->pNext)
      if (p->hwndDlg == hwndDlg) {
         fMore = TRUE;
         break;
      }
   if (!fMore)
      FreeDialog(hwndDlg);
}


/*=========================================================================
 * FindBtn() - locate the node for a button of a dialog (NULL if none).
 *=========================================================================*/
static PANIBTN FindBtn(HWND hwndDlg, SHORT id) {
   PANIBTN p;

   for (p = pBtnList; p; p = p->pNext)
      if (p->hwndDlg == hwndDlg && p->id == id)
         return p;
   return NULL;
}


/*=========================================================================
 * FindStat() - locate the status-field record (NULL if none).
 *=========================================================================*/
static PANISTAT FindStat(HWND hwndDlg, USHORT id) {
   PANISTAT s;

   for (s = pStatList; s; s = s->pNext)
      if (s->hwndDlg == hwndDlg && s->id == id)
         return s;
   return NULL;
}


/*=========================================================================
 * FindDlg() - locate the dialog record (NULL if not subclassed).
 *=========================================================================*/
static PANIDLG FindDlg(HWND hwndDlg) {
   PANIDLG d;

   for (d = pDlgList; d; d = d->pNext)
      if (d->hwndDlg == hwndDlg)
         return d;
   return NULL;
}


/*=========================================================================
 * AddStat() - begin managing a status field, saving its current text.
 *=========================================================================*/
static VOID AddStat(HWND hwndDlg, USHORT id) {
   PANISTAT s;
   HWND     hwnd;
   LONG     cch;

   s = (PANISTAT)malloc(sizeof(ANISTAT));
   if (!s)
      return;
   memset(s, 0, sizeof(ANISTAT));
   s->hwndDlg = hwndDlg;
   s->id      = id;

   hwnd = WinWindowFromID(hwndDlg, id);
   if (hwnd) {
      cch = WinQueryWindowTextLength(hwnd);
      s->pszOrig = (PSZ)malloc(cch + 1);
      if (s->pszOrig) {
         s->pszOrig[0] = '\0';
         WinQueryWindowText(hwnd, (LONG)(cch + 1), s->pszOrig);
      }
   }

   s->pNext  = pStatList;
   pStatList = s;
}


/*=========================================================================
 * RestoreStatus() - put every managed field of a dialog back to the text
 * it held before the library first touched it.
 *=========================================================================*/
static VOID RestoreStatus(HWND hwndDlg) {
   PANISTAT s;
   HWND     hwnd;

   for (s = pStatList; s; s = s->pNext) {
      if (s->hwndDlg != hwndDlg)
         continue;
      hwnd = WinWindowFromID(hwndDlg, s->id);
      if (hwnd)
         WinSetWindowText(hwnd, s->pszOrig ? s->pszOrig : "");
   }
}


/*=========================================================================
 * DrawButton() - blit the up or down bitmap into the button's PS.
 *
 * The down bitmap is selected while BDS_HILITED is set, which is how a
 * BS_USERBUTTON reports that it is being pressed.
 *=========================================================================*/
static VOID DrawButton(PANIBTN p, HPS hps, ULONG fsState) {
   POINTL  aptl[4];
   HBITMAP hbm;

   if (!hps)
      return;

   hbm = (fsState & BDS_HILITED) ? p->hbmDown : p->hbmUp;
   if (!hbm || hbm == (HBITMAP)-1)
      return;

   aptl[0].x = 0;           aptl[0].y = 0;
   aptl[1].x = (LONG)p->cx; aptl[1].y = (LONG)p->cy;
   aptl[2].x = 0;           aptl[2].y = 0;
   aptl[3].x = (LONG)p->cx; aptl[3].y = (LONG)p->cy;
   GpiWCBitBlt(hps, hbm, 4, aptl, ROP_SRCCOPY, BBO_IGNORE);
}


/*=========================================================================
 * FreeDialog() - drop all per-dialog state (procs, fields, buttons) and
 * restore the dialog's original window procedure.
 *
 * Used both by XpmDeleteAniButton (when the last button goes) and by the
 * WM_DESTROY safety net.
 *=========================================================================*/
static VOID FreeDialog(HWND hwndDlg) {
   PANIDLG  d, *dp;
   PANISTAT s, *sp;
   PANIBTN  b, *bp;

   /* Restore the dialog procedure. */
   dp = &pDlgList;
   while (*dp && (*dp)->hwndDlg != hwndDlg)
      dp = &(*dp)->pNext;
   if (*dp) {
      d = *dp;
      *dp = d->pNext;
      if (d->pOldDlgProc && WinIsWindow(NULLHANDLE, hwndDlg))
         WinSubclassWindow(hwndDlg, d->pOldDlgProc);
      free(d);
   }

   /* Release the status fields. */
   sp = &pStatList;
   while (*sp) {
      s = *sp;
      if (s->hwndDlg == hwndDlg) {
         *sp = s->pNext;
         if (s->pszOrig)
            free(s->pszOrig);
         free(s);
      } else
         sp = &(*sp)->pNext;
   }

   /* Release any buttons still attached to the dialog. */
   bp = &pBtnList;
   while (*bp) {
      b = *bp;
      if (b->hwndDlg == hwndDlg) {
         *bp = b->pNext;
         if (b->pszStatus)
            free(b->pszStatus);
         if (b->hbmUp)
            GpiDeleteBitmap(b->hbmUp);
         if (b->hbmDown)
            GpiDeleteBitmap(b->hbmDown);
         free(b);
      } else
         bp = &(*bp)->pNext;
   }
}


/*=========================================================================
 * AniBtnProc() - subclass proc for an animated button.
 *
 * Only WM_MOUSEMOVE is intercepted, to show the status help; every other
 * message goes to the original button procedure.
 *=========================================================================*/
static MRESULT EXPENTRY AniBtnProc(HWND hwnd, ULONG msg,
                                   MPARAM mp1, MPARAM mp2) {
   PANIBTN p = (PANIBTN)WinQueryWindowPtr(hwnd, QWL_USER);

   if (msg == WM_MOUSEMOVE && p && p->pszStatus && p->idStatus)
      WinSetWindowText(WinWindowFromID(p->hwndDlg, p->idStatus),
                       p->pszStatus);

   if (p && p->pOldBtnProc)
      return p->pOldBtnProc(hwnd, msg, mp1, mp2);
   return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


/*=========================================================================
 * AniDlgProc() - subclass proc for a dialog owning animated buttons.
 *
 *   WM_CONTROL / BN_PAINT : draw the up or down bitmap.
 *   WM_MOUSEMOVE          : restore the status fields.
 *   WM_DESTROY            : safety net; free anything the application
 *                           forgot to delete with XpmDeleteAniButton.
 *=========================================================================*/
static MRESULT EXPENTRY AniDlgProc(HWND hwnd, ULONG msg,
                                   MPARAM mp1, MPARAM mp2) {
   PANIDLG pD  = FindDlg(hwnd);
   PFNWP   old = pD ? pD->pOldDlgProc : NULLHANDLE;

   if (msg == WM_CONTROL) {
      if (SHORT2FROMMP(mp1) == BN_PAINT) {
         PANIBTN p = FindBtn(hwnd, SHORT1FROMMP(mp1));
         if (p) {
            PUSERBUTTON pub = (PUSERBUTTON)mp2;
            DrawButton(p, pub->hps, pub->fsState);
            return (MRESULT)TRUE;
         }
      }
   } else if (msg == WM_MOUSEMOVE) {
      RestoreStatus(hwnd);
   } else if (msg == WM_DESTROY) {
      FreeDialog(hwnd);
   }

   if (old)
      return old(hwnd, msg, mp1, mp2);
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

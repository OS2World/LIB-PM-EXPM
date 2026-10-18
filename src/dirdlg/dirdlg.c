/*
 * EXPM - Extended Presentation Manager library
 *
 * src/dirdlg/dirdlg.c -- "Choose directory" modal dialog.
 *
 * Implements XpmDirDlg(), a ready-to-use modal directory chooser.  It is
 * a clean-room reimplementation for the EXPM project (2026) of the
 * AsuaCtl 1.0 directory dialog (AsuDirDlg).  It was written in C from the
 * published AsuaCtl header (include/asuactl.h, the XpmDIRDLG structure
 * and the AsuDirDlg prototype) and the published dialog resource layout
 * only; no AsuaCtl source was consulted.  AsuaCtl is GPLv2, EXPM is
 * LGPLv3.
 *
 * The dialog presents the drives (at the root) or the subdirectories of
 * the directory on show in a container; the user opens a drive or folder
 * by double-clicking it (or pressing Enter) and moves up with "..".
 * OK returns the highlighted entry, or the directory on display when
 * nothing is highlighted.  AsuaCtl shipped ready-made drive / folder
 * icons; EXPM draws no icons (its resources do not include the AsuaCtl
 * bitmaps), so entries are plain names.
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
#define INCL_DOS
#define INCL_DOSMODULEMGR
#define INCL_WINSTDCNR

#include "expm.h"
#include <stdio.h>
#include <string.h>


/* Per-dialog instance data.  Allocated on the heap by XpmDirDlg and
 * freed during WM_DESTROY; a pointer to it is kept in QWL_USER. */
typedef struct {
    HAB         hab;                    /* anchor block                  */
    HWND        hwndParent;             /* parent (dialog is centred on) */
    HWND        hwndOwner;              /* owner                         */
    HWND        hwndCnr;                /* directory container           */
    PXpmDIRDLG  pdd;                    /* caller's description / result */
    PRECORDCORE pSel;                   /* highlighted record, or NULL   */
    CHAR        szCurDir[CCHMAXPATH];   /* "" = drive list (root)        */
    CHAR        szNames[8192];          /* backing store for the record
                                           name strings (NUL separated)  */
    ULONG       cNames;                 /* bytes used in szNames         */
} DIRDATA, *PDIRDATA;


static MRESULT EXPENTRY DirDlgProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 );
static VOID  NormalizeDir( PCSZ pszIn, PSZ pszOut );
static VOID  ParentDir( PCSZ pszCur, PSZ pszOut );
static VOID  InsertRecord( PDIRDATA pad, PCSZ pszName );
static VOID  PopulateDrives( PDIRDATA pad );
static VOID  PopulateSubdirs( PDIRDATA pad );
static VOID  Populate( PDIRDATA pad );
static VOID  Descend( PDIRDATA pad, PRECORDCORE prc );
static VOID  ComputePath( PDIRDATA pad, PSZ pszOut );


/* ------------------------------------------------------------------ */
/* Path helpers                                                        */
/* ------------------------------------------------------------------ */

/* Normalise a caller-supplied directory: copy it (bounded) and make sure
 * it ends with a backslash.  The result is not tested for existence. */
static VOID NormalizeDir( PCSZ pszIn, PSZ pszOut )
{
    ULONG cch;

    pszOut[0] = '\0';
    if( pszIn == NULL || pszIn[0] == '\0' )
        return;

    strncpy( pszOut, pszIn, CCHMAXPATH - 2 );
    pszOut[CCHMAXPATH - 2] = '\0';

    cch = (ULONG)strlen( pszOut );
    if( cch > 0 && pszOut[cch - 1] != '\\' ) {
        pszOut[cch]     = '\\';
        pszOut[cch + 1] = '\0';
    }
}

/* Compute the parent of a directory.  "C:\" (or any "X:\") has the drive
 * list as its parent, reported as the empty string. */
static VOID ParentDir( PCSZ pszCur, PSZ pszOut )
{
    LONG n;
    PSZ  p;

    pszOut[0] = '\0';
    if( pszCur == NULL || pszCur[0] == '\0' )
        return;

    n = (LONG)strlen( pszCur );
    if( n <= 3 ) {                      /* "X:\" -> drive list           */
        return;
    }

    strcpy( pszOut, pszCur );
    pszOut[n - 1] = '\0';               /* drop the trailing backslash   */
    p = strrchr( pszOut, '\\' );
    if( p != NULL )
        p[1] = '\0';                    /* keep up to and including it   */
    else
        pszOut[0] = '\0';
}

/* The directory to report for OK: the highlighted entry, or the directory
 * on display, or the current drive root. */
static VOID ComputePath( PDIRDATA pad, PSZ pszOut )
{
    PRECORDCORE prc;
    PSZ         pszName;
    ULONG       ulDisk;

    pszOut[0] = '\0';

    prc = pad->pSel;
    if( prc != NULL && (pszName = prc->pszName) != NULL ) {
        if( pad->szCurDir[0] == '\0' ) {            /* a drive "X:"      */
            if( pszName[0] != '\0' ) {
                pszOut[0] = pszName[0];
                pszOut[1] = ':';
                pszOut[2] = '\\';
                pszOut[3] = '\0';
            }
            return;
        }
        if( strcmp( pszName, ".." ) == 0 ) {
            ParentDir( pad->szCurDir, pszOut );
            if( pszOut[0] == '\0' )
                strcpy( pszOut, pad->szCurDir );    /* ".." at a drive root */
            return;
        }
        sprintf( pszOut, "%s%s", pad->szCurDir, pszName );
        return;
    }

    if( pad->szCurDir[0] != '\0' ) {
        strcpy( pszOut, pad->szCurDir );
        return;
    }

    /* Root with nothing highlighted: report the current drive root. */
    ulDisk = 0;
    DosQueryCurrentDisk( &ulDisk, NULL );
    if( ulDisk >= 1 && ulDisk <= 26 )
        sprintf( pszOut, "%c:\\", (char)('A' + ulDisk - 1) );
    else
        strcpy( pszOut, "C:\\" );
}


/* ------------------------------------------------------------------ */
/* Container population                                                */
/* ------------------------------------------------------------------ */

/* Add one entry with the given display name. */
static VOID InsertRecord( PDIRDATA pad, PCSZ pszName )
{
    PRECORDCORE  prc;
    RECORDINSERT ri;
    PSZ          psz;
    ULONG        cch;

    cch = (ULONG)strlen( pszName );
    if( pad->cNames + cch + 1 > sizeof(pad->szNames) )
        return;                                  /* backing store full     */

    prc = (PRECORDCORE)WinSendMsg( pad->hwndCnr, CM_ALLOCRECORD,
                                   MPFROMLONG(0), MPFROMLONG(1) );
    if( prc == NULL )
        return;

    psz = &pad->szNames[pad->cNames];
    strcpy( psz, pszName );
    pad->cNames += cch + 1;

    prc->cb             = sizeof(RECORDCORE);
    prc->flRecordAttr   = 0;
    prc->pszIcon        = psz;
    prc->pszName        = psz;
    prc->pszText        = psz;
    prc->hptrIcon       = NULLHANDLE;
    prc->hptrMiniIcon   = NULLHANDLE;
    prc->hbmBitmap      = NULLHANDLE;
    prc->hbmMiniBitmap  = NULLHANDLE;
    prc->pTreeItemDesc  = NULL;

    ri.cb                = sizeof(RECORDINSERT);
    ri.pRecordOrder      = (PRECORDCORE)CMA_END;
    ri.pRecordParent     = NULL;
    ri.fInvalidateRecord = TRUE;
    ri.zOrder            = (ULONG)CMA_TOP;
    ri.cRecordsInsert    = 1;
    WinSendMsg( pad->hwndCnr, CM_INSERTRECORD, MPFROMP(prc), MPFROMP(&ri) );
}

/* Root view: one entry per available drive. */
static VOID PopulateDrives( PDIRDATA pad )
{
    ULONG  ulDisk;
    FSINFO fsi;
    CHAR   szName[8];

    for( ulDisk = 1; ulDisk <= 26; ulDisk++ ) {
        if( DosQueryFSInfo( ulDisk, FSIL_VOLSER, &fsi, sizeof(fsi) ) != 0 )
            continue;
        sprintf( szName, "%c:", (char)('A' + ulDisk - 1) );
        InsertRecord( pad, szName );
    }
}

/* One entry per subdirectory of the directory on display. */
static VOID PopulateSubdirs( PDIRDATA pad )
{
    CHAR         szSpec[CCHMAXPATH];
    HDIR         hdir;
    FILEFINDBUF3 ffb;
    ULONG        count;
    APIRET       rc;

    if( strlen( pad->szCurDir ) + 2 > sizeof(szSpec) )
        return;
    strcpy( szSpec, pad->szCurDir );
    strcat( szSpec, "*" );

    hdir  = (HDIR)HDIR_CREATE;
    count = 1;
    rc = DosFindFirst( szSpec, &hdir, FILE_DIRECTORY,
                       &ffb, sizeof(ffb), &count, FIL_STANDARD );
    while( rc == 0 && count == 1 ) {
        ffb.achName[ffb.cchName] = '\0';
        if( (ffb.attrFile & FILE_DIRECTORY) &&
            ffb.achName[0] != '.' )          /* skip "." and ".."          */
            InsertRecord( pad, ffb.achName );
        count = 1;
        rc = DosFindNext( hdir, &ffb, sizeof(ffb), &count );
    }
    if( hdir != (HDIR)HDIR_CREATE )
        DosFindClose( hdir );
}

/* Rebuild the container for the directory on display. */
static VOID Populate( PDIRDATA pad )
{
    WinSendMsg( pad->hwndCnr, CM_REMOVERECORD, NULL,
                MPFROM2SHORT(0, CMA_FREE) );

    pad->cNames = 0;
    pad->pSel   = NULL;

    if( pad->szCurDir[0] == '\0' ) {
        PopulateDrives( pad );
    } else {
        InsertRecord( pad, ".." );
        PopulateSubdirs( pad );
    }
}

/* Open the highlighted entry (drive, folder or ".."). */
static VOID Descend( PDIRDATA pad, PRECORDCORE prc )
{
    PSZ  pszName;
    CHAR szParent[CCHMAXPATH];

    if( prc == NULL )
        return;
    pszName = prc->pszName;
    if( pszName == NULL || pszName[0] == '\0' )
        return;

    if( strcmp( pszName, ".." ) == 0 ) {
        if( pad->szCurDir[0] == '\0' )
            return;
        ParentDir( pad->szCurDir, szParent );
        strcpy( pad->szCurDir, szParent );
    } else if( pad->szCurDir[0] == '\0' ) {
        if( strlen( pszName ) < 2 || pszName[1] != ':' )
            return;
        pad->szCurDir[0] = pszName[0];
        pad->szCurDir[1] = ':';
        pad->szCurDir[2] = '\\';
        pad->szCurDir[3] = '\0';
    } else {
        if( strlen( pad->szCurDir ) + strlen( pszName ) + 2 >
            sizeof(pad->szCurDir) )
            return;
        strcat( pad->szCurDir, pszName );
        strcat( pad->szCurDir, "\\" );
    }

    Populate( pad );
}


/* ------------------------------------------------------------------ */
/* Dialog procedure                                                    */
/* ------------------------------------------------------------------ */

static MRESULT EXPENTRY DirDlgProc( HWND hwnd, ULONG msg, MPARAM mp1,
                                    MPARAM mp2 )
{
    switch( msg ) {

        case WM_INITDLG: {
            PDIRDATA pad = (PDIRDATA)mp2;
            CNRINFO  ci;
            RECTL    rcl;
            RECTL    rclPar;

            WinSetWindowPtr( hwnd, QWL_USER, (PVOID)pad );

            pad->hwndCnr = WinWindowFromID( hwnd, XpmCNR_TREE );

            if( pad->pdd->pszTitle != NULL )
                WinSetWindowText( hwnd, pad->pdd->pszTitle );
            if( pad->pdd->pszOKButton != NULL )
                WinSetDlgItemText( hwnd, DID_OK, pad->pdd->pszOKButton );
            if( pad->pdd->pszCancelButton != NULL )
                WinSetDlgItemText( hwnd, DID_CANCEL, pad->pdd->pszCancelButton );

            if( pad->hwndCnr != NULLHANDLE ) {
                memset( &ci, 0, sizeof(ci) );
                ci.cb           = sizeof(CNRINFO);
                ci.flWindowAttr = CV_NAME;      /* one name per line      */
                WinSendMsg( pad->hwndCnr, CM_SETCNRINFO,
                            MPFROMP(&ci), MPFROMLONG(CMA_FLWINDOWATTR) );
                Populate( pad );
            }

            /* Centre the dialog over its parent. */
            WinQueryWindowRect( hwnd, &rcl );
            WinQueryWindowRect( pad->hwndParent, &rclPar );
            WinSetWindowPos( hwnd, NULLHANDLE,
                             (rclPar.xRight - rcl.xRight) / 2,
                             (rclPar.yTop   - rcl.yTop)   / 2,
                             0, 0, SWP_MOVE | SWP_SHOW );
            break;
        }

        case WM_CONTROL:
            if( SHORT1FROMMP( mp1 ) == XpmCNR_TREE ) {
                PDIRDATA pad = WinQueryWindowPtr( hwnd, QWL_USER );

                switch( SHORT2FROMMP( mp1 ) ) {
                    case CN_ENTER:              /* double-click or Enter  */
                        if( mp2 != NULL )
                            Descend( pad,
                                     ((PNOTIFYRECORDENTER)mp2)->pRecord );
                        break;

                    case CN_EMPHASIS:           /* highlight changed      */
                        if( mp2 != NULL ) {
                            PNOTIFYRECORDEMPHASIS pe =
                                (PNOTIFYRECORDEMPHASIS)mp2;

                            if( pe->fEmphasisMask & CRA_SELECTED ) {
                                if( pe->pRecord->flRecordAttr & CRA_SELECTED )
                                    pad->pSel = pe->pRecord;
                                else if( pad->pSel == pe->pRecord )
                                    pad->pSel = NULL;
                            }
                        }
                        break;
                }
            }
            break;

        case WM_COMMAND: {
            PDIRDATA pad = WinQueryWindowPtr( hwnd, QWL_USER );

            switch( SHORT1FROMMP( mp1 ) ) {
                case DID_OK: {
                    CHAR szPath[CCHMAXPATH];

                    ComputePath( pad, szPath );
                    strcpy( pad->pdd->szFullDirectoryName, szPath );
                    pad->pdd->lReturn = DID_OK;
                    WinDismissDlg( hwnd, DID_OK );
                    break;
                }
                case DID_CANCEL:
                    pad->pdd->lReturn = DID_CANCEL;
                    WinDismissDlg( hwnd, DID_CANCEL );
                    break;
            }
            break;
        }

        case WM_CLOSE: {
            PDIRDATA pad = WinQueryWindowPtr( hwnd, QWL_USER );

            if( pad != NULL )
                pad->pdd->lReturn = DID_CANCEL;
            WinDismissDlg( hwnd, DID_CANCEL );
            break;
        }

        case WM_DESTROY: {
            PDIRDATA pad = WinQueryWindowPtr( hwnd, QWL_USER );
            HWND     hwndCnr;

            hwndCnr = WinWindowFromID( hwnd, XpmCNR_TREE );
            if( hwndCnr != NULLHANDLE )
                WinSendMsg( hwndCnr, CM_REMOVERECORD, NULL,
                            MPFROM2SHORT(0, CMA_FREE) );
            if( pad != NULL )
                DosFreeMem( pad );
            break;
        }

        default:
            return WinDefDlgProc( hwnd, msg, mp1, mp2 );
    }

    return (MRESULT)FALSE;
}


/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

BOOL EXPENTRY XpmDirDlg( HWND hwndParent, HWND hwndOwner, PXpmDIRDLG pdd )
{
    PDIRDATA    pad;
    HMODULE     hmod;
    ULONG       idDlg;
    HAB         hab;
    FILESTATUS3 fst;

    if( pdd == NULL )
        return FALSE;

    hab = WinQueryAnchorBlock( hwndOwner != NULLHANDLE ? hwndOwner
                                                       : hwndParent );
    if( hab == NULLHANDLE )
        return FALSE;

    /* The "_W" templates use XpmWinButtonClass; make sure it is
     * registered (XpmControlsInit also does this). */
    XpmWinButtonInit( hab );

    if( DosQueryModuleHandle( "EXPM.DLL", &hmod ) != 0 )
        return FALSE;

    if( DosAllocMem( (PPVOID)&pad, (ULONG)sizeof(DIRDATA),
                     PAG_READ | PAG_WRITE | PAG_COMMIT ) != 0 )
        return FALSE;

    pad->hab         = hab;
    pad->hwndParent  = hwndParent;
    pad->hwndOwner   = hwndOwner;
    pad->hwndCnr     = NULLHANDLE;
    pad->pdd         = pdd;
    pad->pSel        = NULL;
    pad->cNames      = 0;
    pad->szCurDir[0] = '\0';

    /* Start in the requested directory, or at the drive list. */
    if( pdd->pszInitialDir != NULL && pdd->pszInitialDir[0] != '\0' ) {
        NormalizeDir( pdd->pszInitialDir, pad->szCurDir );
        if( DosQueryPathInfo( pad->szCurDir, FIL_STANDARD,
                              &fst, sizeof(fst) ) != 0 )
            pad->szCurDir[0] = '\0';
    }

    pdd->szFullDirectoryName[0] = '\0';
    pdd->lReturn = DID_CANCEL;

    /* Pick the template variant. */
    if( pdd->flDialogType == XpmDIRDLG_OK ||
        pdd->flDialogType == XpmDIRDLG_OK_W ) {
        idDlg = pdd->flUseWinButton ? XpmDIRDLG_OK_W : XpmDIRDLG_OK;
    } else {
        idDlg = pdd->flUseWinButton ? XpmDIRDLG_OKCANCEL_W
                                    : XpmDIRDLG_OKCANCEL;
    }

    WinDlgBox( hwndParent, hwndOwner, DirDlgProc, hmod, idDlg, (PVOID)pad );

    return (BOOL)(pdd->lReturn == DID_OK);
}

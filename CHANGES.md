# Changelog

All notable changes to EXPM (Extended Presentation Manager library) are
recorded here.  The version refers to the EXPM library as a whole (see
`XPM_VERSION_MAJOR` / `XPM_VERSION_MINOR` in `h/expm.h`); it is
independent of the version of the third-party library each module
tracks.

## [0.2] - 2026-10-18

First release of EXPM, an LGPLv3 library that extends the OS/2
Presentation Manager with helper routines and window classes whose entry
points use EXPM's own `Xpm` prefix (IBM reserves `Win*`, `Gpi*` and
`Dos*`).

### Added

- **ComboSup** (`src/combosup`) - combo box initialization helpers,
  merged from the public-domain ComboSup 1.1.
- **Cell** (`src/cell`) - split views and bitmap toolbars, merged from
  Cell Toolkit 0.7b (BSD-2).
- **Control Plus** (`src/ctlplus`) - clean-room clone of Controls+ 1.40:
  meter, switch, draggable rectangle and chained digit controls.
- **ColorWheel** (`src/colorwheel`) - modal "Edit Color" dialog wrapping
  the system color-wheel control, merged from ColorWheel 1.2.
- **AniButtn** (`src/anibtn`) - clean-room clone of AniButtn 1.02:
  two-state graphical push buttons.
- **Shaded Buttons** (`src/shadebtn`) - `WC_shBUTTON` gradient-shaded
  button class, merged from Shaded Buttons 1.50.
- **ArrowHead** (`src/arrowhead`) - clean-room clone of ArrowHead 1.1:
  `XpmDrawArrowHead` GPI helper.
- **OwnerDraw** (`src/owndraw`) - column alignment for owner-draw
  listboxes, merged from DEV-SAMPLES-C-LIB-PM-Owner_Drawn_Listbox
  (BSD-3).
- **AsuaCtl** (`src/ctlcommon`, `src/status`, `src/progbar`,
  `src/winbutt`, `src/toolbar`, `src/dirdlg`, `src/fonts`) - clean-room
  clone of AsuaCtl 1.0 (GPLv2):
  - shared helpers and the version / initialisation API
    (`XpmControlsInit`, `XpmGetVersion`, `XpmGetComponentVersion`,
    `XpmQueryPresColors`, `XpmStorePresParams`, `XpmDrawBitmap`);
  - the status text control (`XpmStatusClass`);
  - the progress bar (`XpmProgressBarClass`);
  - the window button (`XpmWinButtonClass`);
  - the toolbar (`XpmToolBarClass`) with the full floating behaviour of
    the original - title bar that moves / docks and a rotate button,
    minimise / restore, drag tracking, and per-object tool-tips;
  - the directory dialog (`XpmDirDlg`);
  - the raster font helper (`XpmCreateRasterFont`).
- Samples for every module under `sample/`, plus an API reference for
  each under `doc/`.

### Notes

- Built with Open Watcom 2.0.1 (`wmake -f Makefile.wat`, or
  `compile_wat.cmd`); produces `dll/expm.dll` and `lib/expm.lib`.
- All 14 sample applications were built and exercised on ArcaOS 5.1.1.
- Clean-room clones were written from the published headers and manuals
  of the original libraries only; no third-party source is used.  See
  the "Licences of merged modules" section of `README.md`.

# Changelog

All notable changes to EXPM (Extended Presentation Manager library) are
recorded here.  The version refers to the EXPM library as a whole (see
`XPM_VERSION_MAJOR` / `XPM_VERSION_MINOR` in `h/expm.h`); it is
independent of the version of the third-party library each module
tracks.

## [0.9] - 2026-10-26

### Fixed

- **Shaped Windows** (`src/shapewin`) - the span child windows
  (`"XpmShapeRegion"`) were created without `WS_VISIBLE`, so a shaped
  window painted nothing at all; what looked like transparency was the
  unpainted client window of the application behind it.  The spans are
  now created visible (and with `WS_SAVEBITS`).
- **Shaped Windows repaint** - moving a shape now invalidates the
  rectangle it vacated on the desktop and forces that repaint, so the
  old position is cleaned up instead of leaving the shape smeared
  across the screen.

### Added

- Module documentation in `doc/shapewin.txt`; the README module table,
  sample list, directory layout and roadmap now mention the module.

### Changed

- **Shaped window sample** - left-drag moves the circle and
  right-click cycles its colour; the temporary push buttons were
  replaced by "~Move shape" and "E~xit" menu entries.

## [0.8] - 2026-10-25

### Fixed

- **Shaped Windows repaint** - the owner window is invisible, so PM
  never repainted the desktop area that a moved shape vacated; the
  module now invalidates that area itself.

## [0.7] - 2026-10-25

### Added

- **Shaped Windows** (`src/shapewin`) - original, clean-room
  implementation of non-rectangular ("transparent") PM windows: the
  mask is parsed into horizontal spans and one child window per span
  paints its slice, so the desktop shows through the rest.  New class
  `WC_XpmSHAPEWIN`, create data `XpmShapeData`, entry point
  `XpmShapeInit`; exported through `src/expm.def`.
- **Shaped window sample** (`sample/shapewin/shpsmp.c`) - a circle
  drawn on the desktop.

## [0.6] - 2026-10-24

### Fixed

- **Combo sample** - opening the drop-down and then moving focus to the
  other combo box terminated the process: the sample was linked with an
  8 KB stack and deeply nested message handling overflowed it.  All
  samples now use a 32 KB stack.

## [0.3] - 2026-10-19

### Added

- **Modern controls** (`src/modern`) - an original EXPM control pack in
  the "Material" / iOS visual style, drawn with the ordinary PM/GPI
  primitives.  Five new classes plus a shared theme engine:
  - `XpmToggleSwitchClass` - animated on/off switch.
  - `XpmSegmentedClass` - segmented (tab-like) selector.
  - `XpmRingProgressClass` - circular progress and indeterminate
    spinner, with an optional centre label.
  - `XpmChipClass` - compact tag / filter chip with optional close
    button.
  - `XpmIconButtonClass` - round or flat icon button with optional
    toggle state.
  - Theme API (`XpmThemeDefault`, `XpmThemeDark`, `XpmSetTheme`,
    `XpmQueryTheme`, `XpmModernInit`) with light and dark palettes;
    `XpmSetTheme` broadcasts `XpmWM_THEMECHANGED` so live controls
    restyle themselves.  See `doc/modern.txt`.
- **Modern sample** (`sample/modern/msmp.c`) - exercises every modern
  control and the light/dark theme switch.

### Fixed

- **Combo sample** (`sample/combo/combosmp.c`) - choosing an item and
  then moving focus to the other combo box terminated the process.
  Forwarding `WM_FOCUSCHANGE` to `WinDefWindowProc` recursed without
  bound and overflowed the stack; the client window now consumes the
  message itself.

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

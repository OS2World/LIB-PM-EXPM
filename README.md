# EXPM - Extended Presentation Manager library

Extended Presentation Manager library for OS/2 (ArcaOS / Warp 4.52).
Extends PM GUI elements with helper routines whose entry points use
EXPM's own `Xpm` prefix.  PM reserves the `Win*`, `Gpi*` and `Dos*`
prefixes for the system APIs, so a third-party library must not claim
them; the helpers otherwise follow PM calling conventions (`EXPENTRY`,
`MPARAM` / `MRESULT`, untyped handles).  See
https://www.edm2.com/index.php/Presentation_Manager_Functions.

- Version: 0.3 (see `CHANGES.md`)
- License: LGPLv3
- Language: C
- Toolchain: Open Watcom 2.0.1 (wcc386 / wlink / wrc / wmake)

## Modules

| Module     | Source                     | Description                              |
|------------|----------------------------|------------------------------------------|
| ComboSup   | src/combosup/combosup.c    | Combo box initialization helpers         |
| Cell       | src/cell/cell.c            | Split views and toolbars (Cell Toolkit)  |
| Control Plus | src/ctlplus/ctlplus.c   | Meter, switch, rect and counter controls |
| ColorWheel | src/colorwheel/colorwheel.c | Modal color-picker dialog (color wheel) |
| AniButtn   | src/anibtn/anibtn.c        | Two-state graphical push buttons         |
| ShadedBtn  | src/shadebtn/shadebtn.c    | Gradient-shaded button window class      |
| ArrowHead  | src/arrowhead/arrowhead.c  | GPI arrowhead drawing helper             |
| OwnerDraw  | src/owndraw/owndraw.c      | Column alignment for owner-draw listboxes |
| CtlCommon  | src/ctlcommon/ctlcommon.c  | AsuaCtl shared helpers + version API     |
| Status     | src/status/status.c        | Status text control (clone of AsuaCtl)   |
| ProgressBar | src/progbar/progbar.c     | Progress bar control (clone of AsuaCtl)  |
| WinButton  | src/winbutt/winbutt.c      | Window button control (clone of AsuaCtl) |
| ToolBar    | src/toolbar/toolbar.c      | Toolbar control (clone of AsuaCtl)       |
| DirDlg     | src/dirdlg/dirdlg.c        | Modal "choose directory" dialog (AsuaCtl)|
| Fonts      | src/fonts/fonts.c          | Raster font helper (clone of AsuaCtl)    |
| Modern     | src/modern/modern.c        | Modern controls + theme engine (original) |

### ComboSup (merged from LIB-PM-ComboBoxSupport 1.1)

The public-domain ComboSup 1.1 library (Dave Briccetti, 1990) was merged
into EXPM. Its functions were renamed to EXPM's `Xpm` prefix:

| Original (16-bit)         | EXPM name                  |
|---------------------------|----------------------------|
| `InitComboBox`            | `XpmInitComboBox`          |
| `InitComboBoxFromRcStrings` | `XpmInitComboBoxFromRes`  |
| `ICB_NOSELECTION` (-1)    | `LIT_NONE` (PM, pmwin.h)   |

That source is 16-bit OS/2 1.x Microsoft C code. This build is the
32-bit OpenWatcom port: `halloc`/`hfree` replaced with `malloc`/`free`,
`SHORT` results widened to `LONG`/`ULONG` to match the 32-bit PM
signatures (`WinLoadString`, `WinSendDlgItemMsg`), and PM handles/types
used untyped per the PM ABI (see doc/c-guide notes in the toolkit).

### Cell (merged from Cell Toolkit 0.7b)

The Cell Toolkit 0.7b (Sergey I. Yevtushenko, 1998-2000, BSD-2) was
merged into EXPM. It provides split views (recursively splittable frame
panels with a draggable split bar) and toolbars (bitmap-button bars with
bubble help). Its functions were renamed to EXPM's `Xpm` prefix;
constants and structures keep their upstream names:

| Original (Cell Toolkit) | EXPM name                  |
|--------------------------|----------------------------|
| `ToolkitInit`            | `XpmCellInit`              |
| `CreateCell`             | `XpmCreateCell`            |
| `CellWindowFromID`       | `XpmCellWindowFromID`      |
| `CellParentWindowFromID` | `XpmCellParentWindowFromID`|
| `CreateToolbar`          | `XpmCreateToolbar`         |
| `GenResIDStr`            | `XpmGenResIDStr`           |
| `GetSplit`               | `XpmQueryCellSplit`        |
| `SetSplit`               | `XpmSetCellSplit`          |
| `GetSplitType`           | `XpmQueryCellSplitType`    |
| `SetSplitType`           | `XpmSetCellSplitType`      |
| `ShowCell`               | `XpmShowCell`              |

The source is plain C despite its upstream .CPP name and builds under
wcc386 unchanged apart from the renames, `static` internal helpers, and
`/* */` comments.

### Control Plus (clean-room clone of Controls+ 1.40)

EXPM provides a clean-room reimplementation of the closed-source
"Controls+" 1.40 library (CTLPLUS.DLL) in `src/ctlplus/ctlplus.c`.
It adds four supplementary PM window classes: a meter (thermometer /
analog), a switch (button / throw), a draggable rectangle and a chained
decimal digit (counter).  It was written in C from the published
Controls+ API and manual; no third-party source is used.  The window
classes, style bits, message and notification codes keep their upstream
names, so existing Controls+ code links against EXPM with only the three
entry-point names changed:

| Original (Controls+) | EXPM name        |
|----------------------|------------------|
| `CtlInitialize`      | `XpmCtlInit`     |
| `CtlTerminate`       | `XpmCtlTerminate`|
| `CtlVersion`         | `XpmCtlVersion`  |

### ColorWheel (merged from LIB-PM-ColorWheel 1.2)

The ColorWheel 1.2 dialog library (Alessandro Cantatore, from the
original color wheel concept by Paul Ratcliffe) was merged into EXPM. It
provides a ready-to-use modal "Edit Color" dialog wrapping the
undocumented OS/2 `ColorSelectClass` color wheel control from
`WPCONFIG.DLL`, with R/G/B spin buttons and OK / Undo / Cancel. The
single entry point is already in EXPM's `Xpm` style:

| Original (ColorWheel) | EXPM name          |
|-----------------------|--------------------|
| `XpmEditColorDlg`     | `XpmEditColorDlg`  |

The dialog resource was merged into `expm.dll` and the module-handle
lookup repointed from `EDITCOL.DLL` to `EXPM.DLL`; the two internal
helpers became file-static. The message numbers, window class name and
resource IDs are shared with the dialog template through
`h/colorwheel.h`.

### AniButtn (clean-room clone of AniButtn 1.02)

EXPM provides a clean-room reimplementation of the closed-source AniButtn
1.02 library (Greg Ratajik, 1993) in `src/anibtn/anibtn.c`. It turns a
`BS_USERBUTTON` into a graphical two-state push button (one bitmap up,
another while pressed) that can show a help string in a status field
while the pointer is over it, while still sending `WM_COMMAND` /
`BN_CLICKED` like a normal button. It was written in C from the published
header and manual only; no third-party source is used. The two entry
points were renamed to EXPM's `Xpm` prefix:

| Original (AniButtn) | EXPM name            |
|---------------------|----------------------|
| `AniCreateButton`   | `XpmCreateAniButton` |
| `AniDeleteButton`   | `XpmDeleteAniButton` |

The library subclasses the owning dialog to answer the `BN_PAINT`
request that a `BS_USERBUTTON` sends, choosing the up or down bitmap from
the `BDS_HILITED` state.

### Shaded Buttons (merged from LIB-PM-ShadedButtons 1.50)

The Shaded Buttons 1.50 library (Hubert Chan, 1997-1998, freeware) was
merged into EXPM. It registers the window class `WC_shBUTTON`, a drop-in
replacement for `WC_BUTTON` that paints the button face as a two-tone
gradient and can additionally draw check boxes, radio buttons and
three-state buttons, rounded or open sides, vertical orientation and
icon / mini-icon / bitmap content. The window class name, style bits,
message and parameter codes keep their upstream names; only the two
entry points were renamed to EXPM's `Xpm` prefix:

| Original (Shaded Buttons) | EXPM name                 |
|---------------------------|---------------------------|
| `shBtnInit`               | `XpmShadeBtnInit`         |
| `shBtnQueryVersion`       | `XpmShadeBtnQueryVersion` |

The class was ported from C++ to conservative C. The ten check / radio /
three-state indicator graphics were extracted from the original
`ShadeBtn.dll` and merged into `expm.dll`; `XpmShadeBtnInit` loads them
from `EXPM.DLL`'s own module handle. Because `wrc` binds only one
resource file to the DLL, the module resource scripts are assembled by
`src/expm.rc`. The upstream PMCX wrapper (`ShBtn.dll`) is not part of
EXPM.

### ArrowHead (clean-room clone of ArrowHead 1.1)

EXPM provides a clean-room reimplementation of the shareware ArrowHead
1.1 library (David Kopp) in `src/arrowhead/arrowhead.c`. It adds a
single helper, `XpmDrawArrowHead`, that draws a two-line arrowhead at
the tip of a line in a GPI presentation space, optionally filled, using
the current GPI attributes. It was written from the published header and
readme only; no third-party source is used. There is no window class and
no resource; only the entry point was renamed to EXPM's `Xpm` prefix:

| Original (ArrowHead) | EXPM name          |
|----------------------|--------------------|
| `ArrowHead`          | `XpmDrawArrowHead` |

The function is declared with `EXPENTRY` (`_System`) so that the
`POINTL`-by-value and `double` parameters pass correctly across the DLL
boundary.

### OwnerDraw (merged from DEV-SAMPLES-C-LIB-PM-Owner_Drawn_Listbox)

The DEV-SAMPLES-C-LIB-PM-Owner_Drawn_Listbox library (BSD 3-Clause;
original by ASH Software, Inc., 1992; updates by Martin Iturbide /
OS2World, 2026) was merged into EXPM. It provides two helpers that make
an `LS_OWNERDRAW` listbox align its text in fixed columns under a
proportional-spaced font: `XpmInitLBWidthHeight` (called from
`WM_MEASUREITEM`) and `XpmDrawLBItems` (called from `WM_DRAWITEM`). The
upstream entry points were renamed to EXPM's `Xpm` prefix; the constants
keep their upstream names:

| Original (Owner-Drawn Listbox) | EXPM name              |
|--------------------------------|------------------------|
| `ODInitLBWidthHeight`          | `XpmInitLBWidthHeight` |
| `ODDrawLBItems`                | `XpmDrawLBItems`       |

Columns are described by a zero-terminated array of 1-based character
positions (e.g. `{24, 36, 0}`); items may embed `'\t'` characters to
advance to the next column. The BSD 3-Clause notice is retained in
`src/owndraw/owndraw.c`.

### AsuaCtl (clean-room clone of AsuaCtl 1.0)

EXPM provides a clean-room reimplementation of AsuaCtl 1.0 (asuactl.dll,
GPLv2) split across `src/ctlcommon/ctlcommon.c` (shared helpers and the
version / initialisation API), `src/status/status.c` (the status text
control, `WC_STATUS`), `src/progbar/progbar.c` (the progress bar,
`WC_PROGRESSBAR`), `src/winbutt/winbutt.c` (the window button,
`WC_WINBUTTON`) and `src/toolbar/toolbar.c` (the toolbar,
`WC_TOOLBAR`); the directory dialog, `AsuDirDlg`, is a standalone entry
point in `src/dirdlg/dirdlg.c`, and the raster-font helper
`FntCreateRasterFont` is a standalone entry point in `src/fonts/fonts.c`.
It was written in C from the published
AsuaCtl
headers and IPF manuals only; no AsuaCtl source is used, which keeps EXPM
LGPLv3. Every public name - window classes, styles, messages,
notifications and structures - carries the EXPM `Xpm` prefix so it never
clashes with the original:

| Original (AsuaCtl)     | EXPM name                |
|------------------------|--------------------------|
| `AsuCtlInitialize`     | `XpmControlsInit`        |
| `AsuGetVersion`        | `XpmGetVersion`          |
| `AsuGetComponentVersion` | `XpmGetComponentVersion` |
| `PpmQueryPresColors`   | `XpmQueryPresColors`     |
| `PpmStorePresParams`   | `XpmStorePresParams`     |
| `GrfDrawBitmap`        | `XpmDrawBitmap`          |
| `CtlStatusInitialize`  | `XpmStatusInit`          |
| `ProgbarRegister`      | `XpmProgressBarInit`     |
| `BtnRegister`          | `XpmWinButtonInit`       |
| `ToolBarRegister`      | `XpmToolBarInit`         |
| `AsuDirDlg`            | `XpmDirDlg`              |
| `FntCreateRasterFont`  | `XpmCreateRasterFont`    |
| `ERR_FNT_*`            | `XpmERR_FNT_*`           |
| `WC_STATUS` / `WC_PROGRESSBAR` / `WC_WINBUTTON` / `WC_TOOLBAR` | `XpmStatusClass` / `XpmProgressBarClass` / `XpmWinButtonClass` / `XpmToolBarClass` |
| `SS_*` / `SM_*`        | `XpmSS_*` / `XpmSM_*`    |
| `PBS_*` / `PBM_*`      | `XpmPBS_*` / `XpmPBM_*`  |
| `BS_*` / `BM_*` / `BN_*` | `XpmBS_*` / `XpmBM_*` / `XpmBN_*` |
| `TBS_*` / `TM_*` / `TN_*` | `XpmTBS_*` / `XpmTM_*` / `XpmTN_*` |
| `TBA_*` / `TBO_*` / `TST_*` | `XpmTBA_*` / `XpmTBO_*` / `XpmTST_*` |
| `SPS_*` / `TDQ_*` / `TBR_*` / `TSO_*` | `XpmSPS_*` / `XpmTDQ_*` / `XpmTBR_*` / `XpmTSO_*` |

`XpmControlsInit` registers the AsuaCtl-derived classes; the status
control is created with the class `XpmStatusClass` and styled with the
`XpmSS_*` bits (borders, alignment, automatic scaling, owner-drawn
background, sizing grip), the progress bar with `XpmProgressBarClass`
and the `XpmPBS_*` bits (smooth/segmented, text, vertical, raised/
depressed), the window button with `XpmWinButtonClass` and the
`XpmBS_*` bits (flat, thin frame, text placement, autotext, autosize,
owner-drawn background, default), and the toolbar with `XpmToolBarClass`
and the `XpmTBA_*` attributes (frame, docking edge, orientation), holding
objects added with `XpmTM_INSERTOBJECT` and styled with the `XpmTBO_*` /
`XpmSPS_*` bits. The toolbar implements the full floating behaviour of
the original: a title bar that moves / docks the toolbar and a rotate
button, minimise / restore and drag tracking (with the `XpmTN_BEGINTRACK`
/ `TRACKING` / `ENDTRACK` / `FIXTOOL` / `MINIMIZE` / `RESTORE` / `ROTATE`
notifications), plus per-object tool-tips. The directory dialog is opened
with `XpmDirDlg`, which
returns the chosen directory in an `XpmDIRDLG` structure, and
`XpmCreateRasterFont` creates a logical font bound to a local id, pinned to
an image (bit-map) font of a requested face and size, and reports that
font's metrics. This release
covers the shared helpers, the version / initialisation API, the status
control, the progress bar, the window button, the toolbar, the
directory dialog and the raster font helper; further AsuaCtl controls are
planned. See
`doc/ctlcommon.txt`, `doc/status.txt`, `doc/progbar.txt`,
`doc/winbutt.txt`, `doc/toolbar.txt`, `doc/dirdlg.txt` and `doc/fonts.txt`.


## Directory layout

```
Makefile.wat        Open Watcom 2.0 build file (run from project root)
compile_wat.cmd     Command line script using wmake; generates compile.log
README.md           This file
CHANGES.md          Release notes / changelog
LICENSE             License terms (LGPLv3)
dll\                Rebuilt library DLL (EXPM.DLL)
lib\                Import library file (EXPM.LIB)
h\                  Public headers (expm.h, colorwheel.h, shadebtn.h, arrowhead.h)
src\                Source code of the library (one subdir per module)
sample\             Application samples for using the library
doc\                Documentation and information about the library
obj\                Intermediate object files (build output)
```

## Building

On ArcaOS with OpenWatcom 2.0.1 installed, from the project root:

```
compile_wat.cmd
```

Outputs:

| File                 | Description                        |
|----------------------|------------------------------------|
| `dll\expm.dll`       | EXPM 32-bit PM library DLL         |
| `lib\expm.lib`       | EXPM import library                |
| `sample\combo\combosmp.exe` | Combo box sample application |
| `sample\cell\celmp.exe`     | Split views + toolbar sample  |
| `sample\ctlplus\ctlpsmp.exe` | Control Plus controls sample  |
| `sample\colorwheel\cwsmp.exe` | ColorWheel dialog sample     |
| `sample\anibtn\anismp.exe`    | AniButtn animated-button sample |
| `sample\shadebtn\shbsmp.exe`  | Shaded Buttons sample         |
| `sample\arrowhead\arsmp.exe`  | ArrowHead drawing sample      |
| `sample\owndraw\ownsmpl.exe`  | Owner-Draw Listbox sample     |
| `sample\status\stsmp.exe`     | Status control sample         |
| `sample\progbar\pgbsmp.exe`   | Progress bar sample           |
| `sample\winbutt\wbsmp.exe`    | Window button sample          |
| `sample\toolbar\tbsmp.exe`    | Toolbar sample                |
| `sample\dirdlg\ddsmp.exe`     | Directory dialog sample       |
| `sample\fonts\fntsmp.exe`     | Raster font helper sample     |

API reference (one file per module, under `doc\`): `expm.txt` (ComboSup),
`cell.txt` (Cell), `ctlplus.txt` (Control Plus), `colorwheel.txt`
(ColorWheel), `anibtn.txt` (AniButtn), `shadebtn.txt` (Shaded Buttons),
`arrowhead.txt` (ArrowHead), `owndraw.txt` (Owner-Draw Listbox),
`ctlcommon.txt` (AsuaCtl shared helpers), `status.txt` (Status control),
`progbar.txt` (Progress bar), `winbutt.txt` (Window button),
`toolbar.txt` (Toolbar), `dirdlg.txt` (Directory dialog) and `fonts.txt`
(Raster font helper).

## Runtime

`expm.dll` must be reachable on `LIBPATH` (or sit in the same directory
as the executable) for any program linked against `expm.lib`.

## Roadmap

All components listed in `Idea.txt` have been merged or cloned:

- ComboSup (combo box initialization helpers)
- Cell Toolkit (split views and toolbars)
- ColorWheel (modal color-picker dialog)
- Control Plus (meter, switch, rect and counter controls)
- AniButtn (animated buttons)
- Shaded Buttons (gradient-shaded button window class)
- ArrowHead (GPI arrowhead drawing helper)
- Owner-Draw Listbox (column alignment for LS_OWNERDRAW listboxes)
- AsuaCtl (shared helpers, version API, status control, progress bar, window button, toolbar, directory dialog and raster font helper; more controls planned)

## Licences of merged modules

The Cell Toolkit is used under its BSD-2 licence; the copyright notice
is retained in `src/cell/cell.c`. The ComboSup library is public domain.
The Control Plus module is a clean-room reimplementation of the
Controls+ API and manual and carries no third-party source. The
ColorWheel module is used under its original terms (free to use, modify
and redistribute, but not for sale); its copyright notice is retained in
`src/colorwheel/colorwheel.c`. The AniButtn module is a clean-room
reimplementation of the AniButtn 1.02 API and manual and carries no
third-party source. The Shaded Buttons module is used under its original
freeware terms (original code by Hubert Chan); its copyright notice is
retained in `src/shadebtn/shadebtn.c`. The ArrowHead module is a
clean-room reimplementation of the shareware ArrowHead API by David Kopp
and carries no third-party source. The Owner-Draw Listbox module is used
under its BSD 3-Clause licence (original by ASH Software, Inc., 1992;
updates by Martin Iturbide / OS2World); its copyright notice is retained
in `src/owndraw/owndraw.c`. The AsuaCtl module (shared helpers, version
API, status control, progress bar, window button, toolbar, directory
dialog and raster font helper) is a
clean-room reimplementation of the AsuaCtl 1.0 API and manuals, written
without consulting the GPLv2 AsuaCtl source, and carries no third-party
source; see `doc/ctlcommon.txt`, `doc/status.txt`, `doc/progbar.txt`,
`doc/winbutt.txt`, `doc/toolbar.txt`, `doc/dirdlg.txt` and
`doc/fonts.txt`. EXPM itself is LGPLv3 (see
`LICENSE`).

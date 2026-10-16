# EXPM - Extended Presentation Manager library

Extended Presentation Manager library for OS/2 (ArcaOS / Warp 4.52).
Extends PM GUI elements with helper routines that follow the natural
naming style of Presentation Manager (see
https://www.edm2.com/index.php/Presentation_Manager_Functions).

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

### ComboSup (merged from LIB-PM-ComboBoxSupport 1.1)

The public-domain ComboSup 1.1 library (Dave Briccetti, 1990) was merged
into EXPM. Its functions were renamed to the PM style:

| Original (16-bit)         | EXPM name                  |
|---------------------------|----------------------------|
| `InitComboBox`            | `WinInitComboBox`          |
| `InitComboBoxFromRcStrings` | `WinInitComboBoxFromRes`  |
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
bubble help). Its functions were renamed to the PM style; constants and
structures keep their upstream names:

| Original (Cell Toolkit) | EXPM name                  |
|--------------------------|----------------------------|
| `ToolkitInit`            | `WinCellInit`              |
| `CreateCell`             | `WinCreateCell`            |
| `CellWindowFromID`       | `WinCellWindowFromID`      |
| `CellParentWindowFromID` | `WinCellParentWindowFromID`|
| `CreateToolbar`          | `WinCreateToolbar`         |
| `GenResIDStr`            | `WinGenResIDStr`           |
| `GetSplit`               | `WinQueryCellSplit`        |
| `SetSplit`               | `WinSetCellSplit`          |
| `GetSplitType`           | `WinQueryCellSplitType`    |
| `SetSplitType`           | `WinSetCellSplitType`      |
| `ShowCell`               | `WinShowCell`              |

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
| `CtlInitialize`      | `WinCtlInit`     |
| `CtlTerminate`       | `WinCtlTerminate`|
| `CtlVersion`         | `WinCtlVersion`  |

### ColorWheel (merged from LIB-PM-ColorWheel 1.2)

The ColorWheel 1.2 dialog library (Alessandro Cantatore, from the
original color wheel concept by Paul Ratcliffe) was merged into EXPM. It
provides a ready-to-use modal "Edit Color" dialog wrapping the
undocumented OS/2 `ColorSelectClass` color wheel control from
`WPCONFIG.DLL`, with R/G/B spin buttons and OK / Undo / Cancel. The
single entry point keeps its PM-style name:

| Original (ColorWheel) | EXPM name          |
|-----------------------|--------------------|
| `WinEditColorDlg`     | `WinEditColorDlg`  |

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
points were renamed to the PM style:

| Original (AniButtn) | EXPM name            |
|---------------------|----------------------|
| `AniCreateButton`   | `WinCreateAniButton` |
| `AniDeleteButton`   | `WinDeleteAniButton` |

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
entry points were renamed to the PM style:

| Original (Shaded Buttons) | EXPM name                 |
|---------------------------|---------------------------|
| `shBtnInit`               | `WinShadeBtnInit`         |
| `shBtnQueryVersion`       | `WinShadeBtnQueryVersion` |

The class was ported from C++ to conservative C. The ten check / radio /
three-state indicator graphics were extracted from the original
`ShadeBtn.dll` and merged into `expm.dll`; `WinShadeBtnInit` loads them
from `EXPM.DLL`'s own module handle. Because `wrc` binds only one
resource file to the DLL, the module resource scripts are assembled by
`src/expm.rc`. The upstream PMCX wrapper (`ShBtn.dll`) is not part of
EXPM.


## Directory layout

```
Makefile.wat        Open Watcom 2.0 build file (run from project root)
compile_wat.cmd     Command line script using wmake; generates compile.log
README.md           This file
LICENSE             License terms (LGPLv3)
dll\                Rebuilt library DLL (EXPM.DLL)
lib\                Import library file (EXPM.LIB)
h\                  Public headers (expm.h, colorwheel.h, shadebtn.h)
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

API reference: `doc\cell.txt` (Cell module), `doc\expm.txt` (ComboSup
module), `doc\ctlplus.txt` (Control Plus module), `doc\colorwheel.txt`
(ColorWheel module), `doc\anibtn.txt` (AniButtn module) and
`doc\shadebtn.txt` (Shaded Buttons module).

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
retained in `src/shadebtn/shadebtn.c`. EXPM itself is LGPLv3 (see
`LICENSE`).

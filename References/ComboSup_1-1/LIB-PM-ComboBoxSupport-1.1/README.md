# LIB-PM-ComboBoxSupport

Combo Box Support library for OS/2 Presentation Manager. Provides two helper
routines that simplify initializing PM combo boxes: populating the list and
setting an initial selection.

Originally written by Dave Briccetti in January 1990 as a 16-bit OS/2 PM
library. Ported to 32-bit OS/2 (ArcaOS / Warp 4) using OpenWatcom 2.0.1.

## API

```c
USHORT EXPENTRY InitComboBox(
    HWND    hwnd,              // Owner window handle
    SHORT   idCombo,           // Combo box control ID
    PSZ    *apszInit,          // NULL-terminated array of string pointers
    SHORT   InitialSelection   // Index to pre-select, or ICB_NOSELECTION
);

USHORT EXPENTRY InitComboBoxFromRcStrings(
    HAB     hab,               // Anchor block handle
    HWND    hwnd,              // Owner window handle
    SHORT   idCombo,           // Combo box control ID
    SHORT   cStrings,          // Number of resource strings to load
    SHORT   aidStrings[],      // Array of resource string IDs
    SHORT   cbStringMax,       // Maximum string length
    HMODULE hmodResource,      // Resource module handle, or 0 for current EXE
    SHORT   InitialSelection   // Index to pre-select, or ICB_NOSELECTION
);
```

`ICB_NOSELECTION` (defined in `h/combosup.h`) leaves the combo with no initial
selection.

## Directory Layout

```
src/            DLL source (combosup.c)
h/              Public header (combosup.h)
sample/         Sample PM application (combosmp.c, combosmp.h, combosmp.rc)
dll/            Built DLL and map file (build output)
lib/            Import library combosup.lib (generated during build)
obj/            Intermediate object files (build output)
doc/            Documentation
makefile.wat    OpenWatcom wmake build file
compile.cmd     Build script
```

## Building

Requires OpenWatcom 2.0.1 with the environment configured (`WATCOM` variable
set). Run from the project root on ArcaOS:

```
compile.cmd
```

Outputs:

| File | Description |
|---|---|
| `dll\combosup.dll` | 32-bit PM combo box support DLL |
| `lib\combosup.lib` | Import library for linking |
| `sample\combosmp.exe` | Sample PM application |

## Runtime

`combosup.dll` must be on the `LIBPATH` (or in the same directory as the EXE)
when running any application linked against `combosup.lib`.

## 32-bit Port Notes

- `halloc`/`hfree` (16-bit huge-model allocators) replaced with `malloc`/`free`.
- `EXPENTRY` expands to `_System` in 32-bit `os2.h`; export names are
  mixed-case (`InitComboBox`, `InitComboBoxFromRcStrings`).
- Window procedure `msg` parameter corrected from `USHORT` to `ULONG`, as
  required by the 32-bit `PFNWP` signature.
- `WinQuerySysValue` return values stored as `LONG` instead of `SHORT` to
  avoid truncation on large displays.
- Build system replaced: Microsoft C 6 / LINK / RC  →
  OpenWatcom `wcc386` / `wlink` / `wrc`.
- `option implib=` in the `wlink` step auto-generates `combosup.lib` as a
  side effect of linking the DLL (no separate librarian step needed).

## License

Public Domain. Original code by Dave Briccetti (1990).

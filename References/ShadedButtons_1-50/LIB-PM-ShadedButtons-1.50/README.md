# Shaded Buttons 1.50

A custom OS/2 window class that replaces the standard push button with a
gradient-shaded alternative.  Supports push buttons, check boxes, radio
buttons, rounded sides, icon/mini-icon display, vertical (left-to-right)
gradient, and PMCX integration for use in URE and Prominare Designer.

![ShadedButtons ScreenShot](/doc/ShadedButtons.png)

Originally written by Hubert Chan (1997–1998).  Version 1.50 adds an
OpenWatcom 2.0.1 build system so the DLLs and sample programs can be
compiled from source on ArcaOS / OS/2 Warp 4.

---

## Files

```
h\shadebtn.h        Public header — include this in your application
h\ShBtn.H           PMCX wrapper header
h\pmcx.h            PMCX SDK definitions (IBM/Prominare)

src\shadebtn.cpp    ShadeBtn.dll source
src\ShadeBtn.rc     ShadeBtn.dll resources (pointer icons)
src\ShadeRes.h      Resource IDs for the pointer icons
src\shadebtn.def    Linker definition file for ShadeBtn.dll
src\*.ptr           Extracted OS/2 pointer resources (10 files)

src\shbtn.cpp       ShBtn.dll source (PMCX wrapper)
src\ShBtn.rc        ShBtn.dll dialog resources
src\ShBtn.def       Linker definition file for ShBtn.dll

lib\ShadeBtn.lib    OpenWatcom import library (built from source)

dll\ShadeBtn.dll    Main shaded button window class DLL
dll\ShBtn.dll       PMCX wrapper DLL

sample\Test1\       C++ PM demo — colour-swatch controls
sample\Test2\       C++ PM demo — resize window with icon buttons
sample\Test3\       C   PM demo — rotating cube icon button

doc\HTML_Docs\      Original HTML documentation
doc\FILE_ID.DIZ     Package description

makefile.wat        OpenWatcom wmake build file
```

---

## Building from Source

Requires **OpenWatcom 2.0.1** (or compatible) installed and on the PATH.

```
wmake -f makefile.wat
```

This builds in order:

1. `dll\ShadeBtn.dll` and `lib\ShadeBtn.lib`
2. `dll\ShBtn.dll`
3. `sample\Test1\Test1.exe`, `sample\Test2\Test2.exe`, `sample\Test3\Test3.exe`

To clean all generated files:

```
wmake -f makefile.wat clean
```

The PMCX SDK header (`pmcx.h`) is included under `h\`.  No external SDK
installation is required.

---

## Using Shaded Buttons in Your Application

1. Add `h\` to your compiler include path.
2. Include `h\shadebtn.h` in your source file.
3. Call `shBtnInit()` to register the window class (`WC_shBUTTON`).
4. Use `WC_shBUTTON` wherever you would use `WC_BUTTON`.
5. Link against `lib\ShadeBtn.lib`.
6. Place `ShadeBtn.dll` in a directory on your `LIBPATH`.

For PMCX support, register `ShBtn.dll` with URE or Prominare Designer.
`ShadeBtn.dll` must be on the `LIBPATH` as well.

### Changing button colour at run time

Send `shBM_SETCOLOR` to the button window:

- `mp1` — RGB colour value
- `mp2` — `shBSC_BUTTONFACE` for the button face, or `shBSC_BACKGROUND`
  for the window background

Colour can also be changed by dragging from the Mixed Color Palette.

---

## Button Styles

| Style | Description |
|---|---|
| `shBS_PUSHBUTTON` | Push button (default) |
| `shBS_CHECKBOX` | Check box |
| `shBS_AUTOCHECKBOX` | Auto check box |
| `shBS_RADIOBUTTON` | Radio button |
| `shBS_AUTORADIOBUTTON` | Auto radio button |
| `shBS_3STATE` | Three-state button |
| `shBS_AUTO3STATE` | Auto three-state button |
| `shBS_ICON` | Display icon instead of text |
| `shBS_MINIICON` | Display mini-icon instead of text |
| `shBS_BITMAP` | Display bitmap instead of text |
| `shBS_HELP` | Sends `WM_HELP` when clicked |
| `shBS_SYSCOMMAND` | Sends `WM_SYSCOMMAND` when clicked |
| `shBS_NOLEFTLINE` | Remove left border |
| `shBS_LEFTROUND` | Round left side |
| `shBS_NORIGHTLINE` | Remove right border |
| `shBS_RIGHTROUND` | Round right side |
| `shBS_VERTICAL` | Vertical gradient (left-to-right) |
| `shBS_NOTOPLINE` | Remove top border (vertical only) |
| `shBS_TOPROUND` | Round top side (vertical only) |
| `shBS_NOBOTTOMLINE` | Remove bottom border (vertical only) |
| `shBS_BOTTOMROUND` | Round bottom side (vertical only) |
| `shBS_NOPUSH` | Non-pushable (e.g. status bars) |
| `shBS_NOBORDER` | Remove all edge borders |

---

## History

**v. 1.50 (2026)**
- Ported build system to OpenWatcom 2.0.1 (`makefile.wat`).
- Guarded EMX-specific runtime calls (`_CRT_init`, `__ctordtorInit`, etc.)
  with `#ifdef __EMX__` so the source compiles cleanly under OpenWatcom.
- Extracted the ten OS/2 pointer resources from the original `ShadeBtn.dll`
  binary (LX format) and added them as `src\*.ptr` files with accompanying
  `src\ShadeBtn.rc` and `src\ShadeRes.h`.
- Fixed `Test3.Rc` hard-coded absolute icon paths; converted to relative names.
- Removed redundant root-level redirect header; samples now include
  `h\shadebtn.h` directly.
- Fixed `PFN` callback casts in `shbtn.cpp` for OpenWatcom strict C++ rules.
- Cleaned up all compiler warnings (unused variables, unreachable code).

**v. 1.15 (1998)**
- Added radio buttons and check boxes; integrated styles into PMCX dialog.
- Icon and text can now be displayed simultaneously.
- Focus indicator (dotted box) drawn around button text.
- Button gains focus on mouse-button release, matching system behaviour.
- Documentation moved to HTML.

**v. 1.10 (1998)**
- Gradient pattern corrected (Bresenham-style integer math).
- Became a PMCX control (URE / Prominare Designer compatible).
- Auto-redraws on font presentation-parameter change.

**v. 1.06 (unreleased)**
- Added second demo program; restructured directory layout.
- `shBS_MINIICON` style implemented.

**v. 1.05**
- All symbols renamed with `shB` prefix to avoid scroll-bar class conflicts.
- Button styles aligned with OS/2 standard values.
- Disabled-button icon support added.
- Vertical buttons working.

**v. 1.01**
- Background colour fix for Warp 4 (RGB vs. indexed colour default).
- Disabled buttons rendered with chiselled look.

**v. 1.00**
- First DLL release.
- `shBtnInit()` and `shBtnQueryVersion()` API established.
- Rounded-side drawing and background-colour support added.

**v. 0.20**
- `shBM_SETCOLOR` message added.
- Rounded sides implemented.
- Freeware licence declared.

**v. 0.10**
- Initial release.

---

## Licence

Free.  Original code by Hubert Chan.  Modifications for OpenWatcom
compatibility contributed 2026.

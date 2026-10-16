ColorWheel Control V. 1.2 - DLL and sample executable (source code enclosed)
-----------------------------------------------------------------

 Disclaimer
============

This code was elaborated by:
Alessandro Cantatore (alexcant@tin.it)
from the original source code by:
Paul Ratcliffe
(E-mail: paulr@orac.clara.co.uk, paul.ratcliffe@bbc.co.uk)

Under no circumstances the author is liable for any loss or damage.
The use of the program implies acceptance of the conditions stated above.

You are free to use and modify the code and the enclosed executables
as you like. You can distribute it on any media. You are not allowed
to sell the code or executables, but can charge a reasonable amount to
cover the cost of the distribution media.


 Sample program
================

Click with any mouse button on the client window to make the "Edit control"
dialog pop up.
The client window color is updated according to the selected color and
the color values are displayed in the test.exe titlebar.
The "OK" button dismiss the dialog returning the last selected color.
The "Undo" button restores the color the "Edit color" dialog was started
with.
The "Cancel" button dismiss the dialog returning the color the dialog was
started with. The Close button on the titlebar or the "Close" menuitem of
the system menu work like the "Cancel" button.


 Usage
=======

This program makes the undocumented OS/2 color wheel control
available to any application. To display an "Edit color" dialog
you must use editcol.lib in your .DEF file, call the API
WinEditColorDlg() and include editcol.h.

-- [syntax] ----------------------------------------------------

LONG APIENTRY WinEditColorDlg(HWND hwndParent,  // parent handle
                              HWND hwndOwner,   // owner handle
                              COLOR color,      // RGB color (LONG)
                              PSZ pszTitle);    // dialog title

-- [Parameters] ------------------------------------------------

HWND hwndParent : parent window handle (the dialog is showed in the
                  middle of its parent window)
HWND hwndOwner  : owner window handle (it receives the WM_COLORWHEELCHANGED
                  messages from the "Edit color" dialog whenever a new
                  color has been selected
COLOR color     : is LONG RGB value
PSZ pszTitle    : it is the dialog title. If it is NULL the default
                  title ("Edit color") will be used. The maximum title
                  length is 127 bytes (plus the termination null character).

-- [Returned value] --------------------------------------------

LONG color      : is an RGB color


 Notes
=======

You can find more informations in the header file and the source code.

The enclosed executables are compiled as subsystem to make them smaller
and have been packed. I do not own an OS/2 2.x version so I do not know
if it provides a WPCONFIG.DLL and a color wheel control. Anyway, if you
want to try it with OS/2 2.x you must recompile the executables since
they have been packed in the warp 3.x format.

You can easily edit the "strings" section of editcol.h to translate
the executable to a different language.


 File list
===========
file_id.diz     : BBS description
readme.txt      : this file
README.md       : project overview and build instructions
Makefile.wat    : Open Watcom 2.0 build file

dll\
 +- editcol.dll : the "edit color" dialog DLL

lib\
 +- editcol.lib : import library for EDITCOL.DLL

h\
 +- editcol.h   : public header file (WinEditColorDlg, CLR, constants)

src\
 +- editcol.c   : DLL source file
 +- editcol.rc  : "Edit color" dialog template
 +- editcol.def : DLL definition file (reference)
 +- editcol.mak : original IBM VisualAge C++ makefile (reference)

sample\
 +- test.c      : sample executable source file
 +- test.ico    : sample executable icon
 +- test.rc     : sample executable resource script
 +- test.mak    : original IBM VisualAge C++ makefile (reference)


 Version 1.2 changes (2026)
============================

Ported to Open Watcom 2.0 on ArcaOS / OS/2 Warp:
- Added Makefile.wat for Open Watcom 2.0 (wmake).
- Removed IBM VisualAge C++-specific keywords: _Export,
  #pragma strings(readonly).
- Moved editcol.h to h\ subdirectory (public header location).
- Renamed dllsrc\ to src\ and exesrc\ to sample\.
- Removed unused variable clrRet from sample\test.c.
- Build produces zero errors and zero warnings.


========================================================================

Original author:
Alessandro Cantatore
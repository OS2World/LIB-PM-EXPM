# Makefile.wat -- EXPM build (Open Watcom 2.0.1 / wmake).
#
# Builds:
#   dll\expm.dll               -- EXPM 32-bit PM library DLL
#   lib\expm.lib               -- Import library (OPTION IMPLIB side effect)
#   sample\combo\combosmp.exe  -- Combo box sample application
#   sample\cell\celmp.exe      -- Split views + toolbar sample application
#   sample\ctlplus\ctlpsmp.exe -- Control Plus controls sample application
#   sample\colorwheel\cwsmp.exe -- ColorWheel dialog sample application
#   sample\anibtn\anismp.exe   -- AniButtn animated buttons sample application
#   sample\shadebtn\shbsmp.exe -- Shaded Buttons sample application
#
# Usage (run from the project root, where this file lives):
#   wmake -f Makefile.wat           # build all
#   wmake -f Makefile.wat clean     # remove generated files

CC = wcc386

# -bd      build as DLL (defines __SW_BD, enables DLL runtime entry)
# -bt=os2  target OS/2 32-bit protected mode
# -zp1     1-byte struct packing
# -w3      warning level 3
# -d__OS2__ explicit OS/2 guard macro
# -i=h     project public headers
CFLAGS_DLL = -bt=os2 -zp1 -w3 -bd -d__OS2__ -i=h
CFLAGS_EXE = -bt=os2 -zp1 -w3     -d__OS2__ -i=h

OBJ = obj
DLL = dll

.BEFORE
    -mkdir $(OBJ) 2>NUL
    -mkdir $(DLL) 2>NUL

all: $(DLL)\expm.dll sample\combo\combosmp.exe sample\cell\celmp.exe sample\ctlplus\ctlpsmp.exe sample\colorwheel\cwsmp.exe sample\anibtn\anismp.exe sample\shadebtn\shbsmp.exe .SYMBOLIC

# -----------------------------------------------------------------------
# Library DLL: dll\expm.dll (also produces lib\expm.lib via OPTION IMPLIB)
# -----------------------------------------------------------------------

$(OBJ)\combosup.obj: src\combosup\combosup.c h\expm.h
    $(CC) $(CFLAGS_DLL) -fo=$@ src\combosup\combosup.c

$(OBJ)\cell.obj: src\cell\cell.c h\expm.h
    $(CC) $(CFLAGS_DLL) -fo=$@ src\cell\cell.c

$(OBJ)\ctlplus.obj: src\ctlplus\ctlplus.c h\expm.h
    $(CC) $(CFLAGS_DLL) -fo=$@ src\ctlplus\ctlplus.c

$(OBJ)\colorwheel.obj: src\colorwheel\colorwheel.c h\expm.h h\colorwheel.h
    $(CC) $(CFLAGS_DLL) -fo=$@ src\colorwheel\colorwheel.c

# The DLL's resources are assembled from the module resource scripts in
# src\expm.rc; wrc can bind only a single resource file to the DLL.
$(OBJ)\expm.res: src\expm.rc src\colorwheel\colorwheel.rc src\shadebtn\shadebtn.rc h\colorwheel.h src\shadebtn\shaderes.h
    wrc -q -r -bt=os2 -i=h -i=src\colorwheel -i=src\shadebtn -fo=$@ src\expm.rc

$(OBJ)\anibtn.obj: src\anibtn\anibtn.c h\expm.h
    $(CC) $(CFLAGS_DLL) -fo=$@ src\anibtn\anibtn.c

$(OBJ)\shadebtn.obj: src\shadebtn\shadebtn.c h\expm.h h\shadebtn.h src\shadebtn\shaderes.h
    $(CC) $(CFLAGS_DLL) -i=src\shadebtn -fo=$@ src\shadebtn\shadebtn.c

$(DLL)\expm.dll: $(OBJ)\combosup.obj $(OBJ)\cell.obj $(OBJ)\ctlplus.obj $(OBJ)\colorwheel.obj $(OBJ)\anibtn.obj $(OBJ)\shadebtn.obj $(OBJ)\expm.res
    wlink &
        system os2v2 dll initinstance terminstance &
        name $(DLL)\expm.dll &
        file $(OBJ)\combosup.obj &
        file $(OBJ)\cell.obj &
        file $(OBJ)\ctlplus.obj &
        file $(OBJ)\colorwheel.obj &
        file $(OBJ)\anibtn.obj &
        file $(OBJ)\shadebtn.obj &
        export WinInitComboBox &
        export WinInitComboBoxFromRes &
        export WinCellInit &
        export WinCreateCell &
        export WinCellWindowFromID &
        export WinCellParentWindowFromID &
        export WinCreateToolbar &
        export WinGenResIDStr &
        export WinQueryCellSplit &
        export WinSetCellSplit &
        export WinQueryCellSplitType &
        export WinSetCellSplitType &
        export WinShowCell &
        export WinCtlInit &
        export WinCtlTerminate &
        export WinCtlVersion &
        export WinEditColorDlg &
        export WinCreateAniButton &
        export WinDeleteAniButton &
        export WinShadeBtnInit &
        export WinShadeBtnQueryVersion &
        option implib=lib\expm.lib &
        option map=$(DLL)\expm.map
    wrc -q -bt=os2 $(OBJ)\expm.res $(DLL)\expm.dll

# -----------------------------------------------------------------------
# Sample: COMBO
# -----------------------------------------------------------------------

$(OBJ)\combosmp.obj: sample\combo\combosmp.c sample\combo\combosmp.h h\expm.h
    $(CC) $(CFLAGS_EXE) -i=sample\combo -fo=$@ sample\combo\combosmp.c

$(OBJ)\combosmp.res: sample\combo\combosmp.rc sample\combo\combosmp.h
    wrc -q -r -bt=os2 -i=sample\combo sample\combo\combosmp.rc -fo=$@

sample\combo\combosmp.exe: $(OBJ)\combosmp.obj $(OBJ)\combosmp.res $(DLL)\expm.dll
    wlink &
        system os2v2_pm &
        name sample\combo\combosmp.exe &
        file $(OBJ)\combosmp.obj &
        option stack=8192 &
        option heap=8192 &
        library lib\expm.lib &
        option map=sample\combo\combosmp.map
    wrc -q -bt=os2 $(OBJ)\combosmp.res sample\combo\combosmp.exe

# -----------------------------------------------------------------------
# Sample: CELL
# -----------------------------------------------------------------------

$(OBJ)\celmp.obj: sample\cell\celmp.c sample\cell\celmp.h h\expm.h
    $(CC) $(CFLAGS_EXE) -i=sample\cell -fo=$@ sample\cell\celmp.c

$(OBJ)\celmp.res: sample\cell\celmp.rc sample\cell\celmp.h
    wrc -q -r -bt=os2 -i=sample\cell sample\cell\celmp.rc -fo=$@

sample\cell\celmp.exe: $(OBJ)\celmp.obj $(OBJ)\celmp.res $(DLL)\expm.dll
    wlink &
        system os2v2_pm &
        name sample\cell\celmp.exe &
        file $(OBJ)\celmp.obj &
        option stack=8192 &
        option heap=8192 &
        library lib\expm.lib &
        option map=sample\cell\celmp.map
    wrc -q -bt=os2 $(OBJ)\celmp.res sample\cell\celmp.exe

# -----------------------------------------------------------------------
# Sample: CTLPLUS
# -----------------------------------------------------------------------

$(OBJ)\ctlpsmp.obj: sample\ctlplus\ctlpsmp.c sample\ctlplus\ctlpsmp.h h\expm.h
    $(CC) $(CFLAGS_EXE) -i=sample\ctlplus -fo=$@ sample\ctlplus\ctlpsmp.c

$(OBJ)\ctlpsmp.res: sample\ctlplus\ctlpsmp.rc sample\ctlplus\ctlpsmp.h
    wrc -q -r -bt=os2 -i=sample\ctlplus sample\ctlplus\ctlpsmp.rc -fo=$@

sample\ctlplus\ctlpsmp.exe: $(OBJ)\ctlpsmp.obj $(OBJ)\ctlpsmp.res $(DLL)\expm.dll
    wlink &
        system os2v2_pm &
        name sample\ctlplus\ctlpsmp.exe &
        file $(OBJ)\ctlpsmp.obj &
        option stack=8192 &
        option heap=8192 &
        library lib\expm.lib &
        option map=sample\ctlplus\ctlpsmp.map
    wrc -q -bt=os2 $(OBJ)\ctlpsmp.res sample\ctlplus\ctlpsmp.exe

# -----------------------------------------------------------------------
# Sample: COLORWHEEL
# -----------------------------------------------------------------------

$(OBJ)\cwsmp.obj: sample\colorwheel\cwsmp.c h\expm.h h\colorwheel.h
    $(CC) $(CFLAGS_EXE) -fo=$@ sample\colorwheel\cwsmp.c

$(OBJ)\cwsmp.res: sample\colorwheel\cwsmp.rc sample\colorwheel\cwsmp.ico
    wrc -q -r -bt=os2 -i=sample\colorwheel sample\colorwheel\cwsmp.rc -fo=$@

sample\colorwheel\cwsmp.exe: $(OBJ)\cwsmp.obj $(OBJ)\cwsmp.res $(DLL)\expm.dll
    wlink &
        system os2v2_pm &
        name sample\colorwheel\cwsmp.exe &
        file $(OBJ)\cwsmp.obj &
        option stack=8192 &
        option heap=8192 &
        library lib\expm.lib &
        option map=sample\colorwheel\cwsmp.map
    wrc -q -bt=os2 $(OBJ)\cwsmp.res sample\colorwheel\cwsmp.exe

# -----------------------------------------------------------------------
# Sample: ANIBTN
# -----------------------------------------------------------------------

$(OBJ)\anismp.obj: sample\anibtn\anismp.c sample\anibtn\anismp.h h\expm.h
    $(CC) $(CFLAGS_EXE) -i=sample\anibtn -fo=$@ sample\anibtn\anismp.c

$(OBJ)\anismp.res: sample\anibtn\anismp.rc sample\anibtn\anismp.h
    wrc -q -r -bt=os2 -i=sample\anibtn sample\anibtn\anismp.rc -fo=$@

sample\anibtn\anismp.exe: $(OBJ)\anismp.obj $(OBJ)\anismp.res $(DLL)\expm.dll
    wlink &
        system os2v2_pm &
        name sample\anibtn\anismp.exe &
        file $(OBJ)\anismp.obj &
        option stack=8192 &
        option heap=8192 &
        library lib\expm.lib &
        option map=sample\anibtn\anismp.map
    wrc -q -bt=os2 $(OBJ)\anismp.res sample\anibtn\anismp.exe

# -----------------------------------------------------------------------
# Sample: SHADEBTN
# -----------------------------------------------------------------------

$(OBJ)\shbsmp.obj: sample\shadebtn\shbsmp.c sample\shadebtn\shbsmp.h h\expm.h
    $(CC) $(CFLAGS_EXE) -i=sample\shadebtn -fo=$@ sample\shadebtn\shbsmp.c

$(OBJ)\shbsmp.res: sample\shadebtn\shbsmp.rc sample\shadebtn\shbsmp.h h\shadebtn.h
    wrc -q -r -bt=os2 -i=sample\shadebtn -i=h sample\shadebtn\shbsmp.rc -fo=$@

sample\shadebtn\shbsmp.exe: $(OBJ)\shbsmp.obj $(OBJ)\shbsmp.res $(DLL)\expm.dll
    wlink &
        system os2v2_pm &
        name sample\shadebtn\shbsmp.exe &
        file $(OBJ)\shbsmp.obj &
        option stack=8192 &
        option heap=8192 &
        library lib\expm.lib &
        option map=sample\shadebtn\shbsmp.map
    wrc -q -bt=os2 $(OBJ)\shbsmp.res sample\shadebtn\shbsmp.exe

# -----------------------------------------------------------------------
# Clean
# -----------------------------------------------------------------------

clean: .SYMBOLIC
    -rm -f $(OBJ)\*.obj $(OBJ)\*.res
    -rm -f $(DLL)\expm.dll $(DLL)\expm.map $(DLL)\expm.sym
    -rm -f lib\expm.lib
    -rm -f sample\combo\combosmp.exe sample\combo\combosmp.map sample\combo\combosmp.sym
    -rm -f sample\cell\celmp.exe sample\cell\celmp.map sample\cell\celmp.sym
    -rm -f sample\ctlplus\ctlpsmp.exe sample\ctlplus\ctlpsmp.map sample\ctlplus\ctlpsmp.sym
    -rm -f sample\colorwheel\cwsmp.exe sample\colorwheel\cwsmp.map sample\colorwheel\cwsmp.sym
    -rm -f sample\anibtn\anismp.exe sample\anibtn\anismp.map sample\anibtn\anismp.sym
    -rm -f sample\shadebtn\shbsmp.exe sample\shadebtn\shbsmp.map sample\shadebtn\shbsmp.sym

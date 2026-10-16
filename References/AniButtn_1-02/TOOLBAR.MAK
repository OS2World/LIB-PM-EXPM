#debugc=/Ti+
#debugl=/CO

ALL: ToolBar.EXE

ToolBar.res : toolbar.rc ToolBar.rch
     rc -r toolbar.rc

ToolBar.obj : ToolBar.c ToolBar.rch
     ICC.EXE /C+ /Sa /Ss $(DEBUGC) /G4 .\$*.c

ToolBar.EXE : $(filename) ToolBar.def ToolBar.obj
     link386 $(DEBUGL) /PACKD /PM:PM ToolBar.obj, ToolBar.exe, ,..\AniButtn.lib,ToolBar.def
     rc toolbar ToolBar.exe

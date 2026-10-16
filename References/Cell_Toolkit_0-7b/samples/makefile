#
# Cell Library samples makefile
#

#--------------------

CC=icc
CFLAGS=-O+ -Ss+ -Gs+ -G5 -I..\libcell
LDFLAGS= $(CFLAGS) -B/ST:327680 -B/NOE -B/PMTYPE:PM
RC=rc -i .

#--------------------

LIBOBJ=cell.obj

#--------------------

.SUFFIXES:
.SUFFIXES: .obj .cpp .rc

.cpp.obj:
    $(CC) $(CFLAGS) -c $<

.rc.res:
	$(RC) -r $<

.obj.exe:
    $(CC) $(LDFLAGS) $< cell.obj
    $(RC) $*.res $*.exe

#--------------------
# Samples to build

all: entry.exe multibar.exe nbsample.exe simple.exe status.exe

clean:
    @del *.obj *.exe *.res

entry.exe   : $(LIBOBJ) entry.obj    entry.res
multibar.exe: $(LIBOBJ) multibar.obj multibar.res
nbsample.exe: $(LIBOBJ) nbsample.obj nbsample.res
simple.exe  : $(LIBOBJ) simple.obj   simple.res
status.exe  : $(LIBOBJ) status.obj   status.res
cell.obj    : ..\libcell\cell.cpp
    $(CC) $(CFLAGS) -c $?


# Makefile for Shaded Buttons Sample Program #2
# Hubert Chan, 1997

CPP = gcc

.SUFFIXES: .cpp .obj

.cpp.obj:
	$(CPP) -Zomf -o $*.obj -c $*.cpp

Test2.exe: Test2.obj Test2.res
	$(CPP) -Zomf -Zsys -o Test2.exe Test2.obj ..\\..\ShadeBtn.lib Test2.def
	rc Test2.res
	copy Test2.exe ..\..

Test2.res: Test2.rc Test2.H
	rc -r Test2

Test2.obj: ..\..\ShadeBtn.h Test2.cpp Test2.h

# Makefile for Shaded Buttons Sample Program #1
# Hubert Chan, 1997

CPP = gcc

.SUFFIXES: .cpp .obj

.cpp.obj:
	$(CPP) -Zomf -o $*.obj -c $*.cpp

Test1.exe: Test1.obj Test1.res
	$(CPP) -Zomf -Zsys -o Test1.exe Test1.obj ..\\..\\ShadeBtn.lib Test1.def
	rc Test1.res
	cp Test1.exe ..\\..

Test1.res: Test1.rc Test1.H
	rc -r Test1

Test1.obj: ..\..\ShadeBtn.h Test1.cpp Test1.h

# Makefile for Shaded Buttons Sample Program #2
# Hubert Chan, 1997

CPP = gcc

.SUFFIXES: .c .obj

.c.obj:
	$(CPP) -Zomf -o $*.obj -c $*.c

Test3.exe: Test3.obj Test3.res
	$(CPP) -Zomf -Zsys -o Test3.exe Test3.obj ../../ShadeBtn.lib Test3.def
	rc Test3.res
	cp Test3.exe ../..

Test3.res: Test3.rc Test3.H
	rc -r Test3

Test3.obj: ../../ShadeBtn.h Test3.c Test3.h

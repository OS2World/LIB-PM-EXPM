CLOPTS=-c -Alfu -G2s -W3 -I..
CLDBG=-Zipde -Od
LNKOPTS=/A:16 /NOD /NOI
LNKDBG=/CO

CTLDEMO.EXE:                    CTLDEMO.OBJ \
                                CTLDEMO.RES \
                                CTLDEMO.DEF
   LINK $(LNKOPTS) $(LNKDBG) CTLDEMO, \
        CTLDEMO, \
        NUL, \
        OS2+LLIBCE+..\CTLPLUS.LIB, \
        CTLDEMO
   RC CTLDEMO.RES CTLDEMO.EXE
   COPY CTLDEMO.EXE ..
   BEEP 440 50 660 50 880 50 440 50 660 50 880 50

CTLDEMO.RES:                    CTLDEMO.RC \
                                CTLDEMO.H
   RC -r CTLDEMO.RC CTLDEMO.RES

CTLDEMO.OBJ:                    CTLDEMO.C \
                                CTLDEMO.H
   CL $(CLOPTS) $(CLDBG) CTLDEMO.C

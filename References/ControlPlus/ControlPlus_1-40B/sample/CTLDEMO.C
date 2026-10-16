#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMENUS
#define INCL_WINPOINTERS
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctlplus.h>
#include "ctldemo.h"

#define CHILD_SWBUTTON          1024
#define CHILD_SWTHROW           1025
#define CHILD_MTTHERM           1026
#define CHILD_MTANALOG          1027
#define CHILD_RECT              1028
#define CHILD_DIGITS            1029
#define CHILD_DIGIT0            1030
#define CHILD_DIGIT1            1031
#define CHILD_DIGIT2            1032

HWND hwndFrame;
HWND hwndClient;
HWND hwndMenu;
HPOINTER hArrow;
HPOINTER hWait;

USHORT cdecl wprintf(PCHAR title,PCHAR fmt,...)
{
   va_list vargs;
   CHAR string[1024];

   va_start(vargs,fmt);
   vsprintf(string,fmt,vargs);
   va_end(vargs);

   return (WinMessageBox(HWND_DESKTOP,
                         HWND_DESKTOP,
                         string,
                         title,
                         NULL,
                         MB_OK|MB_ICONASTERISK|MB_APPLMODAL));
}

BOOL toggleCheckMark(USHORT mId)
{
   USHORT attr;

   attr=SHORT1FROMMR(WinSendMsg(hwndMenu,
                                MM_QUERYITEMATTR,
                                MPFROM2SHORT(mId,TRUE),
                                MPFROMSHORT(MIA_CHECKED)));
   attr=(~attr) & MIA_CHECKED;
   WinSendMsg(hwndMenu,
              MM_SETITEMATTR,
              MPFROM2SHORT(mId,TRUE),
              MPFROM2SHORT(MIA_CHECKED,attr));

   return (BOOL)(attr!=0);
}

VOID hwndFromID(HWND hwnd,USHORT id,PHWND pFrame,PHWND pClient)
{
   (*pFrame)=WinWindowFromID(hwnd,id);
   (*pClient)=WinWindowFromID((*pFrame),FID_CLIENT);
}

VOID createChild(HWND hwndParent,USHORT id,PCHAR titleText)
{
   ULONG fCreate=FCF_SIZEBORDER|FCF_TITLEBAR|FCF_MINBUTTON;
   USHORT fSWP=SWP_ACTIVATE|SWP_SHOW|SWP_MOVE|SWP_SIZE;
   HWND hwndChildFrame;
   HWND hwndChildClient;
   HPOINTER hIcon;

   hwndChildFrame=WinCreateStdWindow(hwndParent,
                                     0L,
                                     &fCreate,
                                     "ctlDemoChild",
                                     titleText,
                                     0L,
                                     NULL,
                                     id,
                                     &hwndChildClient);
   if (hwndChildFrame==NULL) {
      toggleCheckMark(MI_SWTHROW);
      return;
   } /* endif */

   hIcon=WinLoadPointer(HWND_DESKTOP,NULL,IC_CHILD);
   if (hIcon==NULL) {
      hIcon=WinQuerySysPointer(HWND_DESKTOP,SPTR_APPICON,FALSE);
   } /* endif */

   WinSendMsg(hwndChildFrame,WM_SETICON,MPFROMP(hIcon),0L);

   WinSetWindowPos(hwndChildFrame,
                   NULL,
                   10+(id-CHILD_SWBUTTON)*20,
                   10+(id-CHILD_SWBUTTON)*5,
                   200,
                   300,
                   fSWP);
}

VOID createChildControl(HWND hwndParent)
{
   HWND hwndCtl;
   USHORT ctlId;

   ctlId=WinQueryWindowUShort(WinQueryWindow(hwndParent,QW_PARENT,FALSE),
                              QWS_ID);

   switch (ctlId) {
   case CHILD_SWBUTTON:
      {
         SWCREATE swc;

         swc.swStyle=SWS_BUTTON;
         swc.swFgColor=CLR_BLUE;
         strcpy(swc.swUpText,"Up");
         strcpy(swc.swDownText,"Down");

         hwndCtl=WinCreateWindow(hwndParent,
                                 WC_SWITCH,
                                 "",
                                 WS_VISIBLE,
                                 0,
                                 0,
                                 0,
                                 0,
                                 hwndParent,
                                 HWND_TOP,
                                 CHILD_SWBUTTON,
                                 &swc,
                                 NULL);
         WinSetWindowPos(hwndCtl,
                         NULL,
                         25,
                         25,
                         150,
                         250,
                         SWP_MOVE|SWP_SIZE);
      }
      break;
   case CHILD_SWTHROW:
      {
         SWCREATE swc;

         swc.swStyle=SWS_THROW;
         swc.swFgColor=CLR_BLUE;
         strcpy(swc.swUpText,"Up");
         strcpy(swc.swDownText,"Down");

         hwndCtl=WinCreateWindow(hwndParent,
                                 WC_SWITCH,
                                 "",
                                 WS_VISIBLE,
                                 0,
                                 0,
                                 0,
                                 0,
                                 hwndParent,
                                 HWND_TOP,
                                 CHILD_SWTHROW,
                                 &swc,
                                 NULL);
         WinSetWindowPos(hwndCtl,
                         NULL,
                         25,
                         25,
                         150,
                         250,
                         SWP_MOVE|SWP_SIZE);
      }
      break;
   case CHILD_MTTHERM:
      {
         MTCREATE mtc;

         mtc.mtStyle=MTS_THERM|MTS_HALFTICK|MTS_QUARTERTICK;
         mtc.mtFgColor=CLR_BLUE;

         hwndCtl=WinCreateWindow(hwndParent,
                                 WC_METER,
                                 "",
                                 WS_VISIBLE,
                                 0,
                                 0,
                                 0,
                                 0,
                                 hwndParent,
                                 HWND_TOP,
                                 CHILD_MTTHERM,
                                 &mtc,
                                 NULL);
         WinSetWindowPos(hwndCtl,
                         NULL,
                         25,
                         25,
                         150,
                         250,
                         SWP_MOVE|SWP_SIZE);
      }
      break;
   case CHILD_MTANALOG:
      break;
   case CHILD_RECT:
      {
         RCCREATE rcc;

         rcc.rcStyle=RCS_VERTHORZSIZE|RCS_KSQUARE;
         rcc.rcFgColor=CLR_BLUE;

         hwndCtl=WinCreateWindow(hwndParent,
                                 WC_RECT,
                                 "",
                                 WS_VISIBLE,
                                 0,
                                 0,
                                 0,
                                 0,
                                 hwndParent,
                                 HWND_TOP,
                                 CHILD_RECT,
                                 &rcc,
                                 NULL);
         WinSetWindowPos(hwndCtl,
                         NULL,
                         25,
                         25,
                         150,
                         250,
                         SWP_MOVE|SWP_SIZE);
      }
      break;
   case CHILD_DIGITS:
      {
         CNTCREATE cnc;
         USHORT index;

         cnc.cntStyle=CTS_DEC;
         cnc.cntFgColor=CLR_RED;
         cnc.cntBgColor=CLR_BLACK;

         for (index=0; index<3; index++) {
            hwndCtl=WinCreateWindow(hwndParent,
                                    WC_COUNTER,
                                    "",
                                    WS_VISIBLE,
                                    10+index*20,
                                    10,
                                    20,
                                    80,
                                    hwndParent,
                                    HWND_TOP,
                                    CHILD_DIGIT0+index,
                                    &cnc,
                                    NULL);
         } /* endfor */
         for (index=1; index<3; index++) {
            WinSendMsg(WinWindowFromID(hwndParent,CHILD_DIGIT0+index),
                       CTM_SETNEXT,
                       MPFROMP(WinWindowFromID(hwndParent,CHILD_DIGIT0+index-1)),
                       0L);
         } /* endfor */
      }
      break;
   default:
      WinAlarm(HWND_DESKTOP,WA_ERROR);
      break;
   } /* endswitch */

}

MRESULT EXPENTRY childWinProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2)
{
   switch (msg) {
   case WM_CREATE:
      createChildControl(hwnd);
      break;
   case WM_PAINT:
      {
         HPS hps;

         hps=WinBeginPaint(hwnd,NULL,NULL);
         GpiErase(hps);
         WinEndPaint(hps);
      }
      break;
   case WM_CLOSE:
      break;
   default:
      return WinDefWindowProc(hwnd,msg,mp1,mp2);
   } /* endswitch */

   return (MRESULT)FALSE;
}

MRESULT EXPENTRY mainWinProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2)
{
   HWND hwndChildFrame;
   HWND hwndChildClient;

   switch (msg) {
   case WM_CREATE:
      {
         HWND hwndParent;

         hwndParent=WinQueryWindow(hwnd,QW_PARENT,FALSE);
         hwndMenu=WinWindowFromID(hwndParent,FID_MENU);
      }
      break;
   case WM_COMMAND:
      switch (SHORT1FROMMP(mp1)) {
      case MI_SWBUTTON:
         if (toggleCheckMark(MI_SWBUTTON)) {
            DosBeep(440,100);
            DosBeep(880,100);

            createChild(hwnd,CHILD_SWBUTTON,"Button Switch");
         } else {
            hwndFromID(hwnd,CHILD_SWBUTTON,&hwndChildFrame,&hwndChildClient);
            WinDestroyWindow(hwndChildFrame);

            DosBeep(880,100);
            DosBeep(440,100);
         } /* endif */
         break;
      case MI_SWTHROW:
         if (toggleCheckMark(MI_SWTHROW)) {
            DosBeep(440,100);
            DosBeep(880,100);

            createChild(hwnd,CHILD_SWTHROW,"Throw Switch");
         } else {
            hwndFromID(hwnd,CHILD_SWTHROW,&hwndChildFrame,&hwndChildClient);
            WinDestroyWindow(hwndChildFrame);

            DosBeep(880,100);
            DosBeep(440,100);
         } /* endif */
         break;
      case MI_MTTHERM:
         if (toggleCheckMark(MI_MTTHERM)) {
            DosBeep(440,100);
            DosBeep(880,100);

            createChild(hwnd,CHILD_MTTHERM,"Therm Meter");
         } else {
            hwndFromID(hwnd,CHILD_MTTHERM,&hwndChildFrame,&hwndChildClient);
            WinDestroyWindow(hwndChildFrame);

            DosBeep(880,100);
            DosBeep(440,100);
         } /* endif */
         break;
      case MI_MTANALOG:
         WinAlarm(HWND_DESKTOP,WA_ERROR);
         break;
      case MI_RECT:
         if (toggleCheckMark(MI_RECT)) {
            DosBeep(440,100);
            DosBeep(880,100);

            createChild(hwnd,CHILD_RECT,"Sizing Rect");
         } else {
            hwndFromID(hwnd,CHILD_RECT,&hwndChildFrame,&hwndChildClient);
            WinDestroyWindow(hwndChildFrame);

            DosBeep(880,100);
            DosBeep(440,100);
         } /* endif */
         break;
      case MI_COUNTERS:
         if (toggleCheckMark(MI_COUNTERS)) {
            DosBeep(440,100);
            DosBeep(880,100);

            createChild(hwnd,CHILD_DIGITS,"Digit Counters");
         } else {
            hwndFromID(hwnd,CHILD_DIGITS,&hwndChildFrame,&hwndChildClient);
            WinDestroyWindow(hwndChildFrame);

            DosBeep(880,100);
            DosBeep(440,100);
         } /* endif */
         break;
      case MI_EXIT:
         WinPostMsg(hwnd,WM_CLOSE,0L,0L);
         break;
      case MI_RESUME:
         break;
      default:
         return WinDefWindowProc(hwnd,msg,mp1,mp2);
      } /* endswitch */
      break;
   case WM_PAINT:
      {
         HPS hps;
         RECTL rectl;

         hps=WinBeginPaint(hwnd,NULL,&rectl);
         WinFillRect(hps,&rectl,CLR_BLUE);
         WinEndPaint(hps);
      }
      break;
   default:
      return WinDefWindowProc(hwnd,msg,mp1,mp2);
   } /* endswitch */

   return (MRESULT)FALSE;
}

VOID cdecl main()
{
   HAB hab;
   HMQ hmq;
   QMSG qmsg;
   ULONG fCreate=FCF_SYSMENU|FCF_TITLEBAR|FCF_MENU|
                 FCF_MINMAX|FCF_SHELLPOSITION|FCF_ICON|
                 FCF_SIZEBORDER|FCF_ACCELTABLE;
   USHORT fSWP=SWP_ACTIVATE|SWP_SHOW;
   CHAR titleText[64];
   MRESULT ctlPlusVer;
   USHORT majVer;
   USHORT minVer;
   SWCNTRL swcntrl;

   hab=WinInitialize(NULL);
   if (hab==NULL) {
      goto NO_HAB;
   } /* endif */

   hmq=WinCreateMsgQueue(hab,0);
   if (hmq==NULL) {
      goto NO_HMQ;
   } /* endif */

   CtlInitialize(hab);

   ctlPlusVer=CtlVersion();
   majVer=SHORT1FROMMR(ctlPlusVer);
   minVer=SHORT2FROMMR(ctlPlusVer);

   sprintf(titleText,"Control+ Version %d.%02d Demo",majVer,minVer);

   hArrow=WinQuerySysPointer(HWND_DESKTOP,SPTR_ARROW,FALSE);
   hWait=WinQuerySysPointer(HWND_DESKTOP,SPTR_WAIT,FALSE);

   WinSetPointer(HWND_DESKTOP,hWait);

   WinRegisterClass(hab,"ctlDemo",mainWinProc,CS_SIZEREDRAW,0);
   WinRegisterClass(hab,"ctlDemoChild",childWinProc,CS_SIZEREDRAW,0);

   hwndFrame=WinCreateStdWindow(HWND_DESKTOP,
                                0L,
                                &fCreate,
                                "ctlDemo",
                                titleText,
                                0L,
                                NULL,
                                WND_MAIN,
                                &hwndClient);
   if (hwndFrame==NULL) {
      goto NO_WND;
   } /* endif */

   WinSetWindowPos(hwndFrame,NULL,0,0,0,0,fSWP);

   swcntrl.hwnd=hwndFrame;
   sprintf(swcntrl.szSwtitle,titleText);
   WinAddSwitchEntry(&swcntrl);

   while (WinGetMsg(hab,&qmsg,NULL,0,0)) {
      WinDispatchMsg(hab,&qmsg);
   } /* endwhile */

   WinDestroyWindow(hwndFrame);
   NO_WND:
   CtlTerminate();
   WinDestroyMsgQueue(hmq);
   NO_HMQ:
   WinTerminate(hab);
   NO_HAB:
   return;
}

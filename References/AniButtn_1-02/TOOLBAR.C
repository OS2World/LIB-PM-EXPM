/*______________________________________________________________________
| ToolBar    05-15-93 Greg Ratajik                                      |
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
| This program shows how to use the AniButtn DLL calls to use           |
| animated pushbuttons.  This version is for SVGS mode.                 |
| See ANIBUTTN.DOC for more information. This is the OS/2 2.0 Version   |
|                                                                       |
|_____________________________________________________________________*/
#define INCL_PM
#define INCL_WINSWITCHLIST

/*______________________________________________________________________
|                                                                       |
|                         System Includes                               |
|_____________________________________________________________________*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os2.h>

/*______________________________________________________________________
|                                                                       |
|             Application Specific Includes & Defines                   |
|_____________________________________________________________________*/
#include "toolbar.rch"
#include "AniButtn.h"

/*______________________________________________________________________
|                                                                       |
|  Gobal Local Variables						|
|_____________________________________________________________________*/
HSWITCH  hSwitch;          /* Task switch                          */
SWCNTRL  Swctl;            /* Task switch                          */
CHAR     szAppName[20];    /* Window creation                      */
HAB      hAB;              /* Handle to anchor block               */
HMQ      hMQ;              /* Handle to message Queue              */
HWND     hWndFrame;        /* Handle to main shell's frame         */
HWND     hWndClient;       /* Handle to main shell's client area   */

/*____________________________________________________________________
|								      |
|  Function Prototypes. 					      |
|____________________________________________________________________*/
MRESULT EXPENTRY WndProc        ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY ToolBar        ( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY SENDChatter    ( HWND, ULONG, MPARAM, MPARAM );

HWND CreateWindow(HWND,ULONG,PCH,PCH,USHORT,INT,INT,INT,INT,PHWND,ULONG,USHORT );

/*___________________________________________________________________________
|                                                                            |
|    Function: Main							     |
|                                                                            |
|      Return:	SHORT							     |
|                                                                            |
| -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  |
|			     C H A N G E    L O G			     |
| -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  |
|                                                                            |
|  INT.     DATE      DESCRIPTION                                            |
|  ----  ----------   ---------------------------------------------------    |
|  GWR   05-15-1993   Initial Developement                                   |
|___________________________________________________________________________*/
main(int argc, char *argv[])
{
static CHAR   szTitle[13];
       QMSG   qmsg;
       ULONG  flCreate = FCF_TITLEBAR     |
                         FCF_SYSMENU      ;
       SEL    sel;
       TID    tid;

 argc = argc;
 argv = argv;

 if((hAB = WinInitialize(0)) == 0)
    return(FALSE);
  
 if((hMQ = WinCreateMsgQueue(hAB, 0)) == 0)
    return(FALSE);

 if(!RegisterClass())
    return(FALSE);

 WinLoadString(hAB, 0, IDS_TITLE, 13, szTitle);
 hWndFrame = WinCreateStdWindow(HWND_DESKTOP,
                                WS_VISIBLE | WS_MAXIMIZED,
                                &flCreate,
                                "ToolBar",
                                "ToolBar",
                                WS_MAXIMIZED,
                                (HMODULE)NULL,
                                ID_CHATTER,
                                &hWndClient);

 /*___________________________________
 |				      |
 | Provide switch info so	      |
 | ChangeTaskList will work	      |
 |___________________________________*/
 {
  PSWCNTRL psw;
  PHSWITCH phsw;
  PID	   pid;
  SWCNTRL  swctlSwitchList[10];
  HSWITCH  hswSwitch;
  CHAR     szSwitchTitle[50];

  psw =  &swctlSwitchList[0];
  phsw = &hswSwitch;

  WinQueryWindowProcess(hWndFrame, &pid, NULL);

  strcpy(szSwitchTitle, "ToolBar");
  strcpy(swctlSwitchList[0].szSwtitle, szSwitchTitle);

 WinQueryWindowProcess(hWndFrame, &pid, &tid);
 Swctl.hwnd = hWndFrame;                         /* Frame window handle    */
 Swctl.idProcess = pid;                          /* Process identifier     */
 Swctl.uchVisibility = SWL_VISIBLE;              /* visibility             */
 Swctl.fbJump = SWL_JUMPABLE;                    /* Jump indicator         */
 strcpy(Swctl.szSwtitle, szTitle);               /* Frame window title     */
 hSwitch = WinAddSwitchEntry(&Swctl);

 hswSwitch = WinAddSwitchEntry(&swctlSwitchList[0]);
 }

 /* The following is the message loop for the application.                 */
 while(WinGetMsg(hAB, (PQMSG)&qmsg, 0, 0, 0))
       WinDispatchMsg(hAB,(PQMSG)&qmsg);

 WinDestroyWindow(hWndFrame); /* Destroy the frame window                  */

 WinDestroyMsgQueue(hMQ);     /* Destroy this application's message queue  */
 WinTerminate(hAB);           /* Terminate this application's use of the   */
                              /* Presentation Manager resources            */
} /* end of main */

/*___________________________________________________________________________
|                                                                            |
|    Function: WndProc                                                       |
|                                                                            |
| Description: Main window proc for Toolbar Example                          |
|									     |
|      Return:	SHORT							     |
|                                                                            |
| -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  |
|			     C H A N G E    L O G			     |
| -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  |
|                                                                            |
|  INT.     DATE      DESCRIPTION                                            |
|  ----  ----------   ---------------------------------------------------    |
|  GWR   05-15-1993   Initial Developement                                   |
|___________________________________________________________________________*/
MRESULT EXPENTRY WndProc(HWND hWnd, ULONG message, MPARAM mp1, MPARAM mp2)
{
   HPS    hPS;          /* Handle for the Presentation Space              */
   RECTL  rClient;      /* Handle to rectangle formed by client area      */
   USHORT rc;

   switch(message)
     {
      case WM_COMMAND:
           switch(SHORT1FROMMP(mp1))
            {
            }
           break; /* End of WM_COMMAND */

      case WM_CREATE:
           /*_____________________________________
           |                                      |
           | Start the main dialog box off.       |
           |                                      |
           |_____________________________________*/
           rc = (USHORT) WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, (PFNWP)ToolBar,
                                    0, DLG_TOOLBAR, NULL);

           WinPostMsg(hWndClient, WM_QUIT,0L,0L);
           return(0);

           break; /* End of WM_CREATE */

      case WM_MOUSEMOVE:
           return(WinDefWindowProc(hWnd, message, mp1, mp2));
           break;

      case WM_PAINT:    /* code for the window's client area               */
           /* Obtain a handle to a cache presentation space                */
           hPS = WinBeginPaint(hWnd, 0, 0);

           /* Determine the size of the client area                        */
           WinQueryWindowRect(hWnd, &rClient);

           /* Fill the background with the default background color        */
           WinFillRect(hPS, &rClient, CLR_BACKGROUND);

           /* return presentation space to state before WinBeginPaint      */
           WinEndPaint(hPS);
           break; /* End of WM_PAINT */


      case WM_CLOSE:  /* close the window                                  */
           WinPostMsg(hWndClient, WM_QUIT,0L,0L);
           if(hWnd != hWndClient)
             break;
           return(WinDefWindowProc(hWnd, message, mp1, mp2));
   }
   return(0L);
} /* End of WndProc */

/*___________________________________________________________________________
|                                                                            |
|    Function: ToolBar                                                       |
|                                                                            |
| Description: Toolbar DLG Message proc.                                     |
|									     |
|									     |
|      Return: VOID							     |
|                                                                            |
| -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  |
|			     C H A N G E    L O G			     |
| -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  |
|                                                                            |
|  INT.     DATE      DESCRIPTION                                            |
|  ----  ----------   ---------------------------------------------------    |
|  GWR   11-02-1993   Initial Developement                                   |
|___________________________________________________________________________*/
MRESULT EXPENTRY ToolBar(HWND hWndDlg,
                         ULONG message,
                         MPARAM mp1,
                         MPARAM mp2)
{
 static HWND   hWndParent;

 switch(message)
   {
    case WM_INITDLG:
         hWndParent = (HWND)mp2;

         /*___________________________________
         |                                    |
         | Create the Animated pushbuttons.   |
         |                                    |
         |___________________________________*/
         AniCreateButton(hWndDlg,                   /* Handle of Dialog    */
                         TBB_FILE,                  /* ID of control       */
                         ID_INFO1,                  /* ID of UP bitmap     */
                         ID_INFO2,                  /* ID of DOWN bitmap   */
                         68,                        /* X Size of bitmaps   */
                         31,                        /* Y Size of bitmaps   */
                         "This is the ABOUT Button",/* Status bar text     */
                         TX_TBB_HELP);              /* ID of status bar    */

         AniCreateButton(hWndDlg, TBB_NEXT, ID_NEXT1, ID_NEXT2,  68, 31, "This is the NEXT Button.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_LAST, ID_LAST1, ID_LAST2,  68, 31, "This is the LAST Button.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NEW , ID_EYE1 , ID_EYE2 ,  68, 31, "This is the INFORMATION Button.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_EXIT, ID_EXIT1, ID_EXIT2,  68, 31, "This is the EXIT Button.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NA1,  ID_NA1,   ID_NA2 ,   68, 31, "This button is not Currently used.  Try clicking on one of the other buttons.  Click the EXIT button to stop the test.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NA2,  ID_NA1,   ID_NA2 ,   68, 31, "This button is not Currently used.  Try clicking on one of the other buttons.  Click the EXIT button to stop the test.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NA3,  ID_NA1,   ID_NA2 ,   68, 31, "This button is not Currently used.  Try clicking on one of the other buttons.  Click the EXIT button to stop the test.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NA4,  ID_NA1,   ID_NA2 ,   68, 31, "This button is not Currently used.  Try clicking on one of the other buttons.  Click the EXIT button to stop the test.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NA5,  ID_NA1,   ID_NA2 ,   68, 31, "This button is not Currently used.  Try clicking on one of the other buttons.  Click the EXIT button to stop the test.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NA6,  ID_NA1,   ID_NA2 ,   68, 31, "This button is not Currently used.  Try clicking on one of the other buttons.  Click the EXIT button to stop the test.", TX_TBB_HELP);
         AniCreateButton(hWndDlg, TBB_NA7,  ID_NA1,   ID_NA2 ,   68, 31, "This button is not Currently used.  Try clicking on one of the other buttons.  Click the EXIT button to stop the test.", TX_TBB_HELP);

         break; /* End of WM_INITDLG */
   
    case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
           {
           }
         break; /* End of WM_CONTROL */

    case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
           {
            /*___________________________________
            |                                    |
            | The following messages are just    |
            | so we can verify the button is     |
            | working correctly.                 |
            |___________________________________*/
            case TBB_FILE:
                  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                                   "ToolBar Button Pressed.",
                                   "This is Greg Ratajiks nifty Toolbar test! (10/8/93)",
                                   0, MB_NOICON | MB_OK );

                  break;

            case TBB_NEXT:
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NEXT Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_LAST:
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the LAST Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_NEW :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the INFORMATION Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_EXIT:
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed", "You Have pressed the EXIT Toolbutton", 0, MB_NOICON | MB_OK );
                  WinDismissDlg(hWndDlg, FALSE);
                  break;

            case TBB_NA1 :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NA1  Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_NA2 :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NA2  Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_NA3 :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NA3  Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_NA4 :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NA4  Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_NA5 :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NA5  Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_NA6 :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NA6  Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

            case TBB_NA7 :
                  WinMessageBox(HWND_DESKTOP, hWndDlg, "ToolBar Button Pressed.", "You Have pressed the NA7  Toolbutton", 0, MB_NOICON | MB_OK );
                  break;

           }
         break; /* End of WM_COMMAND */

    case WM_CLOSE:
         /*___________________________________
         |                                    |
         | AniButtn uses a linked list to keep|
         | track of everything.  This API will|
         | delete the node for this button.   |
         |___________________________________*/
         AniDeleteButton(hWndDlg, TBB_NEXT);
         AniDeleteButton(hWndDlg, TBB_LAST);
         AniDeleteButton(hWndDlg, TBB_NEW );
         AniDeleteButton(hWndDlg, TBB_EXIT);
         AniDeleteButton(hWndDlg, TBB_NA1 );
         AniDeleteButton(hWndDlg, TBB_NA2 );
         AniDeleteButton(hWndDlg, TBB_NA3 );
         AniDeleteButton(hWndDlg, TBB_NA4 );
         AniDeleteButton(hWndDlg, TBB_NA5 );
         AniDeleteButton(hWndDlg, TBB_NA6 );
         AniDeleteButton(hWndDlg, TBB_NA7 );

         WinDismissDlg(hWndDlg, FALSE);
         break; /* End of WM_CLOSE */
           
    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
         break;
   }
 return FALSE;
} /* End of CHATTERBox */


/*___________________________________________________________________________
|                                                                            |
|    Function: CreateWindow                                                  |
|___________________________________________________________________________*/
HWND CreateWindow(HWND   hWndParent,
                  ULONG  ctldata,
                  PCH    appname,
                  PCH    title,
                  USHORT ResID,
                  INT    x,
                  INT    y,
                  INT    cx,
                  INT    cy,
                  PHWND  hWndClient,
                  ULONG  lfStyle,
                  USHORT uSizeStyle)

{
   HWND   hWndFrame;

   hWndFrame = WinCreateStdWindow(hWndParent,
                                  lfStyle,
                                  &ctldata,
                                  appname,
                                  title,
                                  WS_MAXIMIZED,
                                  0,
                                  ResID,
                                  (HWND FAR *)hWndClient);

   if(hWndFrame == 0)
     {
      WinMessageBox(HWND_DESKTOP, hWndParent, "Error Creating Window!",
                    0, 0, MB_OK|MB_ICONEXCLAMATION);
      return((HWND)0);
     }

   return(hWndFrame);
}  /* End of CreateWindow */

/*___________________________________________________________________________
|                                                                            |
|    Function: RegisterClass                                                 |
|___________________________________________________________________________*/
INT RegisterClass(VOID)
{
 INT rc;
 
 WinLoadString(hAB, 0, IDS_APP_NAME, 80, szAppName);
 rc = WinRegisterClass(hAB,
		      (PCH)"ToolBar",
                      (PFNWP)WndProc,
                      CS_SIZEREDRAW ,
                      0);
 if (rc == FALSE)
   return(FALSE);


 return(TRUE);
} /* End of RegisterClass */

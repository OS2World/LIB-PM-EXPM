#ifndef __SHADEDBUTTONS_
#define __SHADEDBUTTONS_

#include <os2.h>

#define WC_shBUTTON   "shButton"

#ifdef __cplusplus
extern "C" {
#endif
BOOL EXPENTRY  shBtnInit();
ULONG EXPENTRY shBtnQueryVersion();
#ifdef __cplusplus
}
#endif

/* Window messages */
#define shBM_SETCOLOR    (WM_USER + 0)

/* Parameters for SBM_SETCOLOR */
#define shBSC_BUTTONFACE  0
#define shBSC_BACKGROUND  1

/* Presentation parameters */
#define shBPP_BUTTONCOLOR  PP_BACKGROUNDCOLOR
#define shBPP_BUTTONCOLORINDEX PP_BACKGROUNDCOLORINDEX
#define shBPP_BUTTONBGCOLOR (PP_USER + 0)
#define shBPP_BUTTONBGCOLORINDEX (PP_USER + 1)

/* Window styles */
#define shBS_PUSHBUTTON       BS_PUSHBUTTON
/* These aren't implemented yet */
#define shBS_CHECKBOX         BS_CHECKBOX
#define shBS_AUTOCHECKBOX     BS_AUTOCHECKBOX
#define shBS_RADIOBUTTON      BS_RADIOBUTTON
#define shBS_AUTORADIOBUTTON  BS_AUTORADIOBUTTON
#define shBS_3STATE           BS_3STATE
#define shBS_AUTO3STATE       BS_AUTO3STATE
#define shBS_USERBUTTON       BS_USERBUTTON

#define shBS_TEXT             BS_TEXT       /*0x0010*/
#define shBS_MINIICON         BS_MINIICON   /*0x0020*/
#define shBS_BITMAP           BS_BITMAP     /*0x0040*/
#define shBS_ICON             BS_ICON       /*0x0080*/
#define shBS_HELP             BS_HELP       /*0x0100*/
#define shBS_SYSCOMMAND       BS_SYSCOMMAND /*0x0200*/
#define shBS_DEFAULT          BS_DEFAULT    /*0x0400*/
#define shBS_NOLEFTLINE       0x0008
#define shBS_LEFTROUND        0x0800
#define shBS_NORIGHTLINE      0x1000
#define shBS_RIGHTROUND       0x2000
#define shBS_VERTICAL         0x4000
#define shBS_NOTOPLINE        0x0008
#define shBS_TOPROUND         0x0800
#define shBS_NOBOTTOMLINE     0x1000
#define shBS_BOTTOMROUND      0x2000
#define shBS_NOPUSH           0x8000
#define shBS_NOBORDER         0x1008

/* stuff needed for PMCX */
#define shBS_BT0              0x0001
#define shBS_BT1              0x0002
#define shBS_BT2              0x0004

#endif

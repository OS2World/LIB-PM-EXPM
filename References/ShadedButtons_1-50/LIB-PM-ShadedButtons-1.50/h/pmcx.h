
/* pmcx.h	   Created:    1993-12-14  Revised:    1997-08-20	*/

/* PM Control Extensions (PMCX)						*/
/* Definitions Header							*/

/* Consult User's Guide for complete description of PM Control          */
/* Extensions								*/

/* Licensed Materials - Property of IBM										*/
/* (C) Copyright IBM Corp. 1991, 1997   All Rights Reserved					*/
/* Copyright ¸ 1989-1997  Prominare Inc.  All Rights Reserved.				*/

/* --------------------------------------------------------------------	*/

/* Function Prototypes (32-bit definition)				*/
/* -------------------							*/
/*									*/
/*     Control Initialization:						*/
/*									*/
/*	   BOOL	EXPENTRY XxxxRegister(HAB hAB);				*/
/*									*/
/*     Styles Dialogue Procedures:					*/
/*									*/
/*	   MRESULT EXPENTRY XxxxStyles(HWND hWnd, ULONG	msg,		*/
/*				       MPARAM mp1, MPARAM mp2);		*/
/*									*/
/*     Window Procedure:						*/
/*									*/
/*	   MRESULT EXPENTRY XxxxWndProc(HWND hWnd, ULONG msg,		*/
/*					MPARAM mp1, MPARAM mp2);	*/
/*									*/
/*     Control Information Procedure:					*/
/*									*/
/*	   BOOL	EXPENTRY XxxxQuery(PUSERINFO pUserInfo);		*/
/*									*/
/*     Set Page	Procedure:						*/
/*									*/
/*	   BOOL	EXPENTRY XxxxSetPage(HWND hwndControl, HWND hwndPage);	*/

/* --- Constant	Definitions -------------------------------------------	*/

#define	CTYPES		    1	   /* Maximum Number of	User Types	*/
#define	CCHCLASS	   32	   /* Maximum Classname	Length		*/
#define	CCHNAME		   32	   /* Maximum Name Length		*/
#define	CCHAUTHOR	   64	   /* Maximum Author Name Length	*/
#define	CCHDESC		   32	   /* Maximum Type Description Length	*/
#define	CCHID		   32	   /* Maximum ID Symbol	Length		*/
#define	CCHTEXTMAX	  512	   /* Maximum Control Text Length	*/

#define	UTYPE_PRIVATE  0x0002	   /* Type:  Private			*/
#define	UTYPE_PUBLIC   0x0004	   /* Type:  Public			*/

#define	USER_CWINDOWWORDS   8UL	   /* Control Reserved Memory Size	*/

#define	QWW_USER  (QWL_USER + 0UL) /* Pointer to User Data		*/
#define	QWW_CDATA (QWL_USER + 4UL) /* Pointer to Private Data Pointer	*/

#define	CUACHK_MNEMONIC	   0	   /* CUA Check:  Mnemonic		*/
#define	CUACHK_CAPS	   1	   /* CUA Check:  Capitalization	*/
#define	CUACHK_ELLIPSIS	   2	   /* CUA Check:  Ellipsis		*/

#define	STYLETYPE_BITFLAGS 0x0001  /* Style Type:  Bit Flags		*/
#define	STYLETYPE_SEQUENCE 0x0002  /* Style Type:  Sequential		*/

#define	PMCXOPT_NONE	  0x00000000UL
#define	PMCXOPT_REFRESH	  0x00000001UL
#define	PMCXOPT_VARICDATA 0x00000002UL
#define	PMCXOPT_STYLECHG  0x00000004UL
#define	PMCXOPT_TABABLE	  0x00000008UL
#define	PMCXOPT_HELP	  0x00000010UL
#define	PMCXOPT_PAGE	  0x00000020UL
#define	PMCXOPT_WINNT95	  0x00000040UL

#define	PMCXWS_VISIBLE	  0x80000000UL
#define	PMCXWS_DISABLED	  0x40000000UL
#define	PMCXWS_GROUP	  0x00010000UL
#define	PMCXWS_TABSTOP	  0x00020000UL

/************************************************************************/
/************************************************************************/
/*									*/
/*     PMCX 32-Bit Structure Definitions				*/
/*									*/
/************************************************************************/
/************************************************************************/

/* --- User Control Styles Structure ----------------------------------	*/

typedef	struct _STYLE		   /* st */
   {				   /* Size:    8 bytes			*/
   ULONG    flStyleMask;	   /* Style Flag Mask			*/
   ULONG    idStyle;		   /* Resource String ID		*/
   } STYLE ;

/* --- User Control Type Structure ------------------------------------	*/

typedef	struct _USERTYPE	   /* ut */
   {				   /* Size:  348 bytes			*/
   LONG	    cx;			   /* Suggested	Width of Control	*/
   LONG	    cy;			   /* Suggested	Height of Control	*/
   ULONG    flStyle;		   /* Initial Style Flags		*/
   ULONG    flOptions;		   /* Options Flag			*/
   ULONG    cMaxText;		   /* Maximum Text Required		*/
   ULONG    idDlg;		   /* Dialog ID				*/
   ULONG    idReserved;		   /* Reserved				*/
   ULONG    flStyleType;	   /* Style Type			*/
   ULONG    cCtlData;		   /* Control Data Count		*/
   ULONG    ulType;		   /* Control Type			*/
   ULONG    cMasks;		   /* Style Masks Count			*/
   CHAR	    szDescription[CCHDESC];/* Control Name			*/
   STYLE    stMasks[32];	   /* Style Masks Array			*/
   ULONG    aulReserved[4];	   /* Reserved				*/
   } USERTYPE ;

/* --- User Control Information	Structure -----------------------------	*/

typedef	struct _USERINFO	   /* ui */
   {				   /* Size:  512 bytes			*/
   ULONG     ulMajor;		   /* Control Version Number:  Major	*/
   ULONG     ulMinor;		   /* Control Version Number:  Minor	*/
   CHAR	     szAuthor[CCHAUTHOR];  /* Control Author			*/
   CHAR	     szClassname[CCHCLASS];/* Classname				*/
   CHAR	     szName[CCHNAME];	   /* Library Name			*/
   ULONG     aulReserved[6];	   /* Reserved				*/
   ULONG     cTypes;		   /* Number of	Control	Types Supported	*/
   USERTYPE  utDefined[CTYPES];	   /* Define User Types	Array		*/
   } USERINFO ;

typedef	USERINFO *PUSERINFO;

/* --- User Control Style Structure -----------------------------------	*/

typedef	struct _USERSTYLE	   /* ust */
   {				   /* Size:  112 bytes			*/
   PFN	    pfnSetSymbolID;	   /* Symbol/ID	Set Function Address	*/
   PFN	    pfnGetSymbolID;	   /* Symbol/ID	Get Function Address	*/
   PFN	    pfnGetFontClr;	   /* Font/Colours Function Address	*/
   PFN	    pfnCUACheck;	   /* CUA Compliance Function Address	*/
   PFN	    pfnRealloc;		   /* Reallocation Function Address	*/
   ULONG    flStyle;		   /* Style				*/
   ULONG    id;			   /* ID Value				*/
   CHAR	    szid[CCHID];	   /* ID Symbol				*/
   PSZ	    pszText;		   /* Text Pointer			*/
   ULONG    cText;		   /* Text Count (Including NULL)	*/
   PFN	    pfnGetBidi;		   /* Bidi Params Function Address	*/
   ULONG    aulReserved[8];	   /* Reserved				*/
   PBYTE    pbCtlData;		   /* Variable Control Data Pointer	*/
   ULONG    cbCtlData;		   /* Control Data Size			*/
   BYTE	    abCtlData[1];	   /* Control Data			*/
   } USERSTYLE ;

typedef	USERSTYLE *PUSERSTYLE;

/* --- Macro Helpers for Setting or Retrieving User Style Pointer ------ */

#define	PDATATODLG(hwndClient) ((WinSetWindowPtr(hwndClient, QWL_USER, (PVOID)mp2)))

#define	PDATAFROMDLG(hwndClient) (((PUSERSTYLE)WinQueryWindowPtr(hwndClient, QWL_USER)))

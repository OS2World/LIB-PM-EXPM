/*
** Module   :CELL.H
** Abstract :Cell Toolkit header file
**
** Copyright (C) Sergey I. Yevtushenko
** Log: Sun  08/02/98   Created
**      Wed  25/10/2000 Updated to version 0.7b
*/

#ifndef __CELL_H
#define __CELL_H

/* Constants */

#define TK_VERSION      "0.7b"  /* Toolkit version */
#define CELL_WINDOW     0x0000   /* Cell is window */
#define CELL_VSPLIT     0x0001   /* Cell is vertically splitted view */
#define CELL_HSPLIT     0x0002   /* Cell is horizontally splitted view */
#define CELL_SPLITBAR   0x0004   /* Call has a splitbar */
#define CELL_FIXED      0x0008   /* Views can't be sized */
#define CELL_SIZE1      0x0010   /* */
#define CELL_SIZE2      0x0020
#define CELL_HIDE_1     0x0040   /* Cell 1 is hidden */
#define CELL_HIDE_2     0x0080   /* Cell 2 is hidden */
#define CELL_HIDE       0x00C0   /* Cell 1 or cell 2 is hidden */
#define CELL_SPLIT_MASK 0x003F
#define CELL_SWAP       0x1000   /* Cells are swapped */
#define CELL_SPLIT10x90 0x0100   /* Sizes of panels related as 10% and 90% */
#define CELL_SPLIT20x80 0x0200   /* Sizes of panels related as 20% and 80% */
#define CELL_SPLIT30x70 0x0300   /* Sizes of panels related as 30% and 70% */
#define CELL_SPLIT40x60 0x0400   /* Sizes of panels related as 40% and 60% */
#define CELL_SPLIT50x50 0x0500   /* Sizes of panels related as 50% and 50% */
#define CELL_SPLIT60x40 0x0600   /* Sizes of panels related as 60% and 40% */
#define CELL_SPLIT70x30 0x0700   /* Sizes of panels related as 70% and 30% */
#define CELL_SPLIT80x20 0x0800   /* Sizes of panels related as 80% and 20% */
#define CELL_SPLIT90x10 0x0900   /* Sizes of panels related as 90% and 10% */
#define CELL_SPLIT_REL  0x0F00

#define TB_BUBBLE       0x0001  /* Toolbar has bubble help */
#define TB_VERTICAL     0x0002  /* Default toolbar view is vertical */
#define TB_FLOATING     0x0004  /* Toolbar not attached */
#define TB_ATTACHED_LT  0x0010  /* Toolbar attached to left side  */
#define TB_ATTACHED_TP  0x0020  /* Toolbar attached to right side */
#define TB_ATTACHED_RT  0x0040  /* Toolbar attached to top side   */
#define TB_ATTACHED_BT  0x0080  /* Toolbar attached to bottom     */
#define TB_ALLOWED      0x00FF  /* */

#define TB_SEPARATOR    0x7001  /* Separator Item ID */
#define TB_BUBBLEID     0x7002  /* Bubble help window ID */

/* Limits */

#define SPLITBAR_WIDTH       2      /* wodth of split bar between cells */
#define HAND_SIZE            8      /* toolbar drag 'hand' */
#define TB_SEP_SIZE          7      /* width of toolbar separator item */
#define TB_BUBBLE_SIZE      32      /* bubble help item size */
#define CELL_TOP_LIMIT      98      /* maximal space occupied by one cell (%%) */
#define CELL_BOTTOM_LIMIT    2      /* minimal space occupied by one cell (%%) */

/* Window classes */

typedef struct stCellDef
{
    LONG lType;    //Cell type flags
    PSZ  pszClass; //if flag CELL_WINDOW is set, this is a Window Class
    PSZ  pszName;  // Caption
    ULONG ulStyle; //if flag CELL_WINDOW is not set, this a Frame creation flags
    ULONG ulID;    // Cell window ID
    struct stCellDef* pPanel1;
    struct stCellDef* pPanel2;
    PFNWP pClassProc;
    PFNWP pClientClassProc;
    LONG  lSize; //Meaningful only if both CELL_SIZE(1|2) and CELL_FIXED is set
} CellDef;

typedef struct
{
    LONG  lType;   // Toolbar flags
    ULONG ulID;    // Toolbar window ID
    ULONG *tbItems;
} TbDef;

/*
** Internal cell data, used by ordinary windows.
** May be useful for user-defined windows
*/

typedef struct
{
    PFNWP pOldProc;
} WindowCellCtlData;

/* Prototypes */

void ToolkitInit(HAB hab);

HWND CreateCell(CellDef* pCell, HWND hWndParent, HWND hWndOwner);
HWND CellWindowFromID(HWND hwndCell, ULONG ulID);
HWND CellParentWindowFromID(HWND hwndCell, ULONG ulID);
void CreateToolbar(HWND hwndCell, TbDef* pTb);
void GenResIDStr(CHAR *buff, ULONG ulID);

/* Some useful additions */

LONG GetSplit(HWND, LONG lID);
LONG SetSplit(HWND, LONG lID, LONG lProp);
void SetSplitType(HWND, LONG lID, LONG lType);
LONG GetSplitType(HWND, LONG lID);
void ShowCell(HWND hwnd, LONG lID, BOOL bAction);

#endif  /* __CELL_H */


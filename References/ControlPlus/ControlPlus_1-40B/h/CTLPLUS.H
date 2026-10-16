#define WC_METER                "meterCtl"
#define WC_SWITCH               "switchCtl"
#define WC_RECT                 "rectCtl"
#define WC_COUNTER              "counterCtl"

typedef struct {
   ULONG mtStyle;
   ULONG mtFgColor;
} MTCREATE, *PMTCREATE;

#define SW_MAXTEXT         32

typedef struct {
   ULONG swStyle;
   ULONG swFgColor;
   CHAR swUpText[SW_MAXTEXT];
   CHAR swDownText[SW_MAXTEXT];
} SWCREATE, *PSWCREATE;

typedef struct {
   ULONG rcStyle;
   ULONG rcFgColor;
   SIZEL rcMaxSize;
   SIZEL rcMinSize;
} RCCREATE, *PRCCREATE;

typedef struct {
   ULONG cntStyle;
   ULONG cntFgColor;
   ULONG cntBgColor;
   HWND cntNext;
} CNTCREATE, *PCNTCREATE;

#define MSG_BASE                WM_USER+0x800

/* General Control+ Messages */
#define CTLM_SETSTYLE           (MSG_BASE+0x01)
#define CTLM_QUERYSTYLE         (MSG_BASE+0x02)

/* Therm Styles */
#define MTS_THERM               0x00010000L

#define MTS_NOTIFYOWNER         0x00000001L
#define MTS_NOINPUT             0x00000002L
#define MTS_HALFTICK            0x00000100L
#define MTS_QUARTERTICK         0x00000200L

/* Therm Notification Codes */
#define MTN_CHGRANGE            1
#define MTN_CHGVALUE            2

/* Therm Messages */
#define MTM_SETRANGE            (MSG_BASE+0x11)
#define MTM_QUERYRANGE          (MSG_BASE+0x12)
#define MTM_SETVALUE            (MSG_BASE+0x13)
#define MTM_QUERYVALUE          (MSG_BASE+0x14)

/* Switch Styles */
#define SWS_BUTTON              0x00010000L
#define SWS_THROW               0x00020000L

#define SWS_NOTIFYOWNER         0x00000001L

/* Switch Notification Codes */
#define SWN_CHGVALUE            1
#define SWN_CHGUPTEXT           2
#define SWN_CHGDOWNTEXT         3

/* Switch Messages */
#define SWM_SETVALUE            (MSG_BASE+0x15)
#define SWM_QUERYVALUE          (MSG_BASE+0x16)
#define SWM_SETUPTEXT           (MSG_BASE+0x17)
#define SWM_QUERYUPTEXT         (MSG_BASE+0x18)
#define SWM_SETDOWNTEXT         (MSG_BASE+0x19)
#define SWM_QUERYDOWNTEXT       (MSG_BASE+0x1A)

/* Rect Styles */
#define RCS_NOTIFYOWNER         0x00000001L
#define RCS_VERTSIZE            0x00000002L
#define RCS_HORZSIZE            0x00000004L
#define RCS_VERTHORZSIZE        0x00000006L
#define RCS_NOMOVE              0x00000008L
#define RCS_MINSIZE             0x00000010L
#define RCS_MAXSIZE             0x00000020L
#define RCS_KSQUARE             0x00000040L
#define RCS_KCIRCLE             0x00000080L

/* Rect Notification Codes */
#define RCN_SIZE                1
#define RCN_DRAG                2
#define RCN_PAINT               3

/* Rect Messages */
#define RCM_SETHANDLE           (MSG_BASE+0x1B)
#define RCM_QUERYHANDLE         (MSG_BASE+0x1C)
#define RCM_SETMINSIZE          (MSG_BASE+0x1D)
#define RCM_QUERYMINSIZE        (MSG_BASE+0x1E)
#define RCM_SETMAXSIZE          (MSG_BASE+0x1F)
#define RCM_QUERYMAXSIZE        (MSG_BASE+0x20)

/* Counter Styles */
#define CTS_NOTIFYOWNER         0x00000001L
#define CTS_BIN                 0x00000002L
#define CTS_DEC                 0x00000004L
#define CTS_HEX                 0x00000008L
#define CTS_NOROLL              0x00000010L
#define CTS_NOSCROLL            0x00000020L

/* Counter Notification Codes */
#define CTN_CHGVALUE            1
#define CTN_UNDERFLOW           2
#define CTN_OVERFLOW            3

/* Counter Messages */
#define CTM_SETVALUE            (MSG_BASE+0x21)
#define CTM_QUERYVALUE          (MSG_BASE+0x22)
#define CTM_SETNEXT             (MSG_BASE+0x23)
#define CTM_QUERYNEXT           (MSG_BASE+0x24)
#define CTM_INC                 (MSG_BASE+0x25)
#define CTM_DEC                 (MSG_BASE+0x26)
#define CTM_INCNOROLL           (MSG_BASE+0x27)
#define CTM_DECNOROLL           (MSG_BASE+0x28)

USHORT EXPENTRY CtlInitialize(HAB hab);
USHORT EXPENTRY CtlTerminate(VOID);
MRESULT EXPENTRY CtlVersion(VOID);

; ============================================================================
; ScreenCapture Pro x64 - Constants and Design Color Tokens
; ============================================================================

; Window & UI Dimensions
MAIN_WIDTH              = 560
MAIN_HEIGHT             = 650

; Tab Indices
TAB_RECORD              = 0
TAB_GALLERY             = 1
TAB_SETTINGS            = 2

; Recording States
STATE_IDLE              = 0
STATE_RECORDING         = 1
STATE_PAUSED            = 2

; Global Hotkey Identifiers
ID_HOTKEY_START         = 101
ID_HOTKEY_PAUSE         = 102
ID_HOTKEY_STOP          = 103
ID_HOTKEY_SCREENSHOT    = 104

; Virtual Key Codes
VK_F5                   = 0x74
VK_F6                   = 0x75
VK_F10                  = 0x79
VK_F11                  = 0x7A
VK_ESCAPE               = 0x1B

; Custom Window Messages
WM_USER_TELEMETRY       = 0x0400 + 100
WM_USER_REC_FINISHED    = 0x0400 + 101

; Top Tab Bar Buttons
IDC_TAB_BTN_RECORD      = 1001
IDC_TAB_BTN_GALLERY     = 1002
IDC_TAB_BTN_SETTINGS    = 1003
IDC_STATUSBAR           = 1004

; Tab 0 (Record View) Control IDs
IDC_LBL_STATUS_BADGE    = 1101
IDC_LBL_REGION_INFO     = 1102
IDC_LBL_TIMER           = 1103
IDC_LBL_FRAMES          = 1104
IDC_LBL_FPS             = 1105
IDC_LBL_SIZE            = 1106

IDC_COMBO_MONITOR       = 1110
IDC_BTN_SELECT_REGION   = 1111
IDC_BTN_FULLSCREEN      = 1112

IDC_CHK_CURSOR          = 1121
IDC_CHK_HIGHLIGHT       = 1122
IDC_CHK_COUNTDOWN       = 1123
IDC_CHK_MINIBAR         = 1124
IDC_CHK_AUDIO_SYS       = 1125
IDC_CHK_AUDIO_MIC       = 1126

IDC_BTN_START           = 1131
IDC_BTN_PAUSE           = 1132
IDC_BTN_STOP            = 1133
IDC_BTN_SCREENSHOT      = 1134

; Tab 1 (Gallery View) Control IDs
IDC_LIST_GALLERY        = 1200
IDC_BTN_PLAY            = 1201
IDC_BTN_OPEN_FOLDER     = 1202
IDC_BTN_REFRESH         = 1203
IDC_BTN_DELETE          = 1204
IDC_BTN_OPEN_REC_DIR    = 1205
IDC_FILTER_ALL          = 1210
IDC_FILTER_VIDEO        = 1211
IDC_FILTER_SHOTS        = 1212

; Tab 2 (Settings View) Control IDs
IDC_EDIT_REC_DIR        = 1301
IDC_BTN_BROWSE_REC      = 1302
IDC_EDIT_SHOT_DIR       = 1303
IDC_BTN_BROWSE_SHOT     = 1304
IDC_COMBO_FPS           = 1305
IDC_COMBO_FORMAT        = 1306
IDC_COMBO_CODEC         = 1307
IDC_COMBO_AUDIO_BITRATE = 1308
IDC_CHK_AUDIO_SYS2      = 1309
IDC_CHK_AUDIO_MIC2      = 1310

; Floating Mini-Toolbar Control IDs
IDC_MINI_PAUSE          = 1401
IDC_MINI_STOP           = 1402
IDC_MINI_SHOT           = 1403
IDC_MINI_RESTORE        = 1404
IDC_MINI_TIMER          = 1405

; ----------------------------------------------------------------------------
; Dark Design Color Palette Tokens (BGR format for GDI)
; ----------------------------------------------------------------------------
COLOR_BG                = 0x1E1E1E ; Dark Slate / Charcoal Background (#1E1E1E)
COLOR_CARD_BG           = 0x262525 ; Dark Card Surface (#252526)
COLOR_BORDER            = 0x473E38 ; Subtle Card Border (#383E47)
COLOR_BORDER_LIGHT      = 0x584E46 ; Slightly lighter border
COLOR_TEXT_MAIN         = 0xF0E8E2 ; High-contrast light text (#E2E8F0)
COLOR_TEXT_MUTED        = 0xA89A8E ; Muted secondary text (#8E9AA8)
COLOR_STATUS_GREEN      = 0x87D200 ; Emerald status green (#00D287)
COLOR_STATUS_RED        = 0x4444EF ; Recording status red (#EF4444)
COLOR_STATUS_AMBER      = 0x0B9EF5 ; Paused status amber (#F59E0B)
COLOR_TIMER             = 0xECF0DC ; Large digital clock font (#DCF0EC)

; Button Colors
COLOR_BTN_GREEN         = 0x7DBC00 ; Emerald action green (#00BC7D)
COLOR_BTN_GREEN_HOVER   = 0x699E00 ; Emerald hover (#009E69)
COLOR_BTN_BLUE          = 0xC78402 ; Ocean blue (#0284C7)
COLOR_BTN_BLUE_HOVER    = 0xA16903 ; Ocean blue hover (#0369A1)
COLOR_BTN_DARK          = 0x3E3733 ; Dark button surface (#33373E)
COLOR_BTN_DARK_HOVER    = 0x4D443E ; Dark button hover (#3E444D)
COLOR_BTN_DISABLED      = 0x2A2624 ; Disabled dark button (#24262A)
COLOR_TEXT_DISABLED     = 0x55504D ; Disabled button text (#4D5055)
COLOR_BTN_RED_BORDER    = 0x3030C5 ; Delete red outline (#C53030)

; Tab Bar Colors
COLOR_TAB_ACTIVE        = 0x4E382B ; Active tab dark blue/slate (#2B384E)
COLOR_TAB_ACTIVE_BORDER = 0x7DBC00 ; Active tab green accent border (#00BC7D)
COLOR_TAB_INACTIVE      = 0x262525 ; Inactive tab (#252526)

; Toggle Switch Colors (BGR)
COLOR_TOGGLE_ON_BG      = 0x7E4C2B ; Dark blue track (#2B4C7E)
COLOR_TOGGLE_ON_BORDER  = 0xA06030 ; Active track border
COLOR_TOGGLE_ON_KNOB    = 0xFFFFFF ; White knob
COLOR_TOGGLE_OFF_BG     = 0x473F3A ; Dark grey track (#3A3F47)
COLOR_TOGGLE_OFF_BORDER = 0x584E46 ; Inactive track border
COLOR_TOGGLE_OFF_KNOB   = 0x968071 ; Grey knob (#718096)

; ----------------------------------------------------------------------------
; Win32 Constants
; ----------------------------------------------------------------------------
WS_OVERLAPPEDWINDOW     = 0x00CF0000
WS_VISIBLE              = 0x10000000
WS_CHILD                = 0x40000000
WS_TABSTOP              = 0x00010000
WS_DISABLED             = 0x08000000
WS_CLIPCHILDREN         = 0x02000000
WS_CLIPSIBLINGS         = 0x04000000
WS_POPUP                = 0x80000000
WS_EX_TOPMOST           = 0x00000008
WS_EX_TOOLWINDOW        = 0x00000080
WS_EX_CLIENTEDGE        = 0x00000200
WS_EX_APPWINDOW         = 0x00040000

WM_CREATE               = 0x0001
WM_DESTROY              = 0x0002
WM_SIZE                 = 0x0005
WM_PAINT                = 0x000F
WM_ERASEBKGND           = 0x0014
WM_DRAWITEM             = 0x002B
WM_SETFONT              = 0x0030
WM_NOTIFY               = 0x004E
WM_KEYDOWN              = 0x0100
WM_KEYUP                = 0x0101
WM_COMMAND              = 0x0111
WM_HOTKEY               = 0x0312
WM_CTLCOLORMSGBOX       = 0x0132
WM_CTLCOLOREDIT         = 0x0133
WM_CTLCOLORLISTBOX      = 0x0134
WM_CTLCOLORBTN          = 0x0135
WM_CTLCOLORDLG          = 0x0136
WM_CTLCOLORSCROLLBAR    = 0x0137
WM_CTLCOLORSTATIC       = 0x0138
WM_MOUSEMOVE            = 0x0200
WM_LBUTTONDOWN          = 0x0201
WM_LBUTTONUP            = 0x0202
WM_RBUTTONUP            = 0x0205

BS_OWNERDRAW            = 0x0000000B
BS_AUTOCHECKBOX         = 0x00000003
BS_AUTORADIOBUTTON      = 0x00000009
CBS_DROPDOWNLIST        = 0x00000003
ES_AUTOHSCROLL          = 0x00000080

SS_CENTER               = 0x00000001
SS_RIGHT                = 0x00000002
SS_NOPREFIX             = 0x00000080

DT_CENTER               = 0x00000001
DT_VCENTER              = 0x00000004
DT_SINGLELINE           = 0x00000020
DT_NOPREFIX             = 0x00000800

ODT_BUTTON              = 4
ODS_SELECTED            = 0x0001
ODS_DISABLED            = 0x0004

LVS_REPORT              = 0x0001
LVS_SINGLESEL           = 0x0004
LVS_SHOWSELALWAYS       = 0x0008
LVS_EX_FULLROWSELECT    = 0x00000020
LVS_EX_GRIDLINES        = 0x00000001
LVS_EX_DOUBLEBUFFER     = 0x00010000

LVM_FIRST               = 0x1000
LVM_SETBKCOLOR          = LVM_FIRST + 1
LVM_SETTEXTCOLOR        = LVM_FIRST + 36
LVM_SETTEXTBKCOLOR      = LVM_FIRST + 38
LVM_GETITEMCOUNT        = LVM_FIRST + 4
LVM_INSERTITEMW         = LVM_FIRST + 77
LVM_SETITEMW            = LVM_FIRST + 76
LVM_DELETEALLITEMS      = LVM_FIRST + 9
LVM_GETNEXTITEM         = LVM_FIRST + 12
LVM_INSERTCOLUMNW       = LVM_FIRST + 97
LVM_GETITEMTEXTW        = LVM_FIRST + 115
LVM_SETEXTENDEDLISTVIEWSTYLE = LVM_FIRST + 54

LVCF_FMT                = 0x0001
LVCF_WIDTH              = 0x0002
LVCF_TEXT               = 0x0004
LVCF_SUBITEM            = 0x0008
LVIF_TEXT               = 0x0001
LVNI_SELECTED           = 0x0002
NM_DBLCLK               = -3

CB_ADDSTRING            = 0x0143
CB_SETCURSEL            = 0x014E
CB_GETCURSEL            = 0x0147

BM_SETCHECK             = 0x00F1
BM_GETCHECK             = 0x00F0
BST_UNCHECKED           = 0
BST_CHECKED             = 1

CURSOR_SHOWING          = 0x00000001
DI_NORMAL               = 0x0003
SRCCOPY                 = 0x00CC0020
CAPTUREBLT              = 0x40000000
BI_RGB                  = 0
DIB_RGB_COLORS          = 0

FOURCC_VIDS             = 0x73646976
OF_CREATE               = 0x00001000
OF_WRITE                = 0x00000001
AVIIF_KEYFRAME          = 0x00000010

; ----------------------------------------------------------------------------
; Custom x64 Structure Definitions
; ----------------------------------------------------------------------------

struct LVCOLUMNW
  mask        dd ?
  fmt         dd ?
  cx          dd ?
  _pad        dd ?
  pszText     dq ?
  cchTextMax  dd ?
  iSubItem    dd ?
  iImage      dd ?
  iOrder      dd ?
  cxMin       dd ?
  cxDefault   dd ?
  cxIdeal     dd ?
ends

struct LVITEMW
  mask        dd ?
  iItem       dd ?
  iSubItem    dd ?
  state       dd ?
  stateMask   dd ?
  _pad        dd ?
  pszText     dq ?
  cchTextMax  dd ?
  iImage      dd ?
  lParam      dq ?
  iIndent     dd ?
  iGroupId    dd ?
  cColumns    dd ?
  _pad2       dd ?
  puColumns   dq ?
  piColFmt    dq ?
  iGroup      dd ?
  _pad3       dd ?
ends

HANDLE_FLAG_INHERIT         = 0x00000001
STARTF_USESTDHANDLES        = 0x00000100
MOVEFILE_REPLACE_EXISTING   = 0x00000001

struct SECURITY_ATTRIBUTES
  nLength              dd ?
  _pad                 dd ?
  lpSecurityDescriptor dq ?
  bInheritHandle       dd ?
  _pad2                dd ?
ends

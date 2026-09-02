; ============================================================================
; ScreenCapture Pro x64 - Dark Modern UI Engine & Custom Controls
; ============================================================================

; ----------------------------------------------------------------------------
; CreateFontsAndBrushes - Allocates Segoe UI typography and Dark Theme GDI tools
; ----------------------------------------------------------------------------
proc CreateFontsAndBrushes uses rbx
    ; 1. Typography Fonts
    invoke CreateFontW, 16, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, addr szFontSegoe
    mov [hFontRegular], rax

    invoke CreateFontW, 16, 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 0, 0, addr szFontSegoe
    mov [hFontBold], rax

    invoke CreateFontW, 19, 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 0, 0, addr szFontSegoe
    mov [hFontTitle], rax

    invoke CreateFontW, 17, 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 0, 0, addr szFontSegoe
    mov [hFontStatusBadge], rax

    invoke CreateFontW, 46, 0, 0, 0, 700, 0, 0, 0, 1, 0, 0, 0, 0, addr szFontConsolas
    mov [hFontTimer], rax

    invoke CreateFontW, 13, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, addr szFontSegoe
    mov [hFontCaption], rax

    ; 2. Dark Theme Surfaces and Accents
    invoke CreateSolidBrush, COLOR_BG
    mov [hBrushBg], rax

    invoke CreateSolidBrush, COLOR_CARD_BG
    mov [hBrushCard], rax

    invoke CreatePen, 0, 1, COLOR_BORDER
    mov [hPenBorder], rax

    invoke CreatePen, 0, 1, COLOR_BORDER_LIGHT
    mov [hPenBorderLight], rax

    invoke CreateSolidBrush, COLOR_TAB_ACTIVE
    mov [hBrushTabActive], rax

    invoke CreateSolidBrush, COLOR_TAB_INACTIVE
    mov [hBrushTabInactive], rax

    invoke CreateSolidBrush, COLOR_BTN_GREEN
    mov [hBrushBtnGreen], rax

    invoke CreateSolidBrush, COLOR_BTN_GREEN_HOVER
    mov [hBrushBtnGreenHover], rax

    invoke CreateSolidBrush, COLOR_BTN_BLUE
    mov [hBrushBtnBlue], rax

    invoke CreateSolidBrush, COLOR_BTN_BLUE_HOVER
    mov [hBrushBtnBlueHover], rax

    invoke CreateSolidBrush, COLOR_BTN_DARK
    mov [hBrushBtnDark], rax

    invoke CreateSolidBrush, COLOR_BTN_DARK_HOVER
    mov [hBrushBtnDarkHover], rax

    invoke CreateSolidBrush, COLOR_BTN_DISABLED
    mov [hBrushBtnDisabled], rax

    invoke CreatePen, 0, 1, COLOR_BTN_RED_BORDER
    mov [hPenBtnRed], rax

    invoke CreateSolidBrush, COLOR_TOGGLE_ON_BG
    mov [hBrushToggleOn], rax
    invoke CreateSolidBrush, COLOR_TOGGLE_OFF_BG
    mov [hBrushToggleOff], rax
    invoke CreateSolidBrush, COLOR_TOGGLE_ON_KNOB
    mov [hBrushKnobOn], rax
    invoke CreateSolidBrush, COLOR_TOGGLE_OFF_KNOB
    mov [hBrushKnobOff], rax

    invoke CreatePen, 0, 1, COLOR_TOGGLE_ON_BORDER
    mov [hPenToggleOn], rax
    invoke CreatePen, 0, 1, COLOR_TOGGLE_OFF_BORDER
    mov [hPenToggleOff], rax

    invoke CreateSolidBrush, 0x00FFFFFF ; Pure White
    mov [hBrushWhite], rax
    invoke CreatePen, 0, 1, 0x00FFFFFF
    mov [hPenWhite], rax

    invoke CreateSolidBrush, COLOR_STATUS_AMBER
    mov [hBrushAmber], rax
    invoke CreatePen, 0, 1, COLOR_STATUS_AMBER
    mov [hPenAmber], rax

    invoke CreateSolidBrush, COLOR_STATUS_RED
    mov [hBrushRed], rax

    invoke CreateSolidBrush, 0x00D8B400 ; Sky Blue / Cyan
    mov [hBrushCyan], rax
    invoke CreatePen, 0, 2, 0x00D8B400
    mov [hPenCyan], rax

    invoke CreatePen, 0, 1, COLOR_STATUS_GREEN
    mov [hPenBtnGreen], rax

    ret
endp


; ----------------------------------------------------------------------------
; CreateMainControls - Top Tab Bar and Bottom Status Bar
; ----------------------------------------------------------------------------
proc CreateMainControls
    ; Top Tab Buttons (Modern Dark Slate Segmented Tabs)
    invoke CreateWindowExW, 0, addr szButtonClass, addr szTabRecord,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW,\
           16, 12, 104, 34, [hWndMain], IDC_TAB_BTN_RECORD, [hInst], NULL
    mov [hTabBtnRecord], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szTabGallery,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW,\
           126, 12, 104, 34, [hWndMain], IDC_TAB_BTN_GALLERY, [hInst], NULL
    mov [hTabBtnGallery], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szTabSettings,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW,\
           236, 12, 114, 34, [hWndMain], IDC_TAB_BTN_SETTINGS, [hInst], NULL
    mov [hTabBtnSettings], rax

    ; Bottom Status Bar (Modern Dark Theme Static Label)
    invoke CreateWindowExW, 0, addr szStaticClass, addr szStatusReady,\
           WS_CHILD or WS_VISIBLE,\
           16, 502, 510, 22, [hWndMain], IDC_STATUSBAR, [hInst], NULL
    mov [hStatusBar], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontCaption], 1

    ret
endp


; ----------------------------------------------------------------------------
; CreateRecordTabControls - Controls for Tab 0 (Запись)
; ----------------------------------------------------------------------------
proc CreateRecordTabControls
    ; Card 1: Status Badge & Region Info
    invoke CreateWindowExW, 0, addr szStaticClass, addr szStatusIdle,\
           WS_CHILD or WS_VISIBLE,\
           30, 70, 220, 20, [hWndMain], IDC_LBL_STATUS_BADGE, [hInst], NULL
    mov [hLblStatusBadge], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontStatusBadge], 1

    invoke CreateWindowExW, 0, addr szStaticClass, addr szTargetFull,\
           WS_CHILD or WS_VISIBLE or SS_RIGHT,\
           380, 70, 124, 20, [hWndMain], IDC_LBL_REGION_INFO, [hInst], NULL
    mov [hLblRegionInfo], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontCaption], 1

    ; Large Digital Clock Timer
    invoke CreateWindowExW, 0, addr szStaticClass, addr szDefTimer,\
           WS_CHILD or WS_VISIBLE or SS_CENTER,\
           30, 94, 474, 52, [hWndMain], IDC_LBL_TIMER, [hInst], NULL
    mov [hLblTimer], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontTimer], 1

    ; Telemetry Sub-stats
    invoke CreateWindowExW, 0, addr szStaticClass, addr szDefFrames,\
           WS_CHILD or WS_VISIBLE,\
           45, 154, 110, 20, [hWndMain], IDC_LBL_FRAMES, [hInst], NULL
    mov [hLblFrames], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontCaption], 1

    invoke CreateWindowExW, 0, addr szStaticClass, addr szDefFps,\
           WS_CHILD or WS_VISIBLE or SS_CENTER,\
           215, 154, 110, 20, [hWndMain], IDC_LBL_FPS, [hInst], NULL
    mov [hLblFps], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontCaption], 1

    invoke CreateWindowExW, 0, addr szStaticClass, addr szDefSize,\
           WS_CHILD or WS_VISIBLE or SS_RIGHT,\
           370, 154, 120, 20, [hWndMain], IDC_LBL_SIZE, [hInst], NULL
    mov [hLblSize], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontCaption], 1

    ; Card 2: Capture Target Controls
    invoke CreateWindowExW, 0, addr szComboClass, NULL,\
           WS_CHILD or WS_VISIBLE or CBS_DROPDOWNLIST or WS_TABSTOP,\
           150, 222, 354, 200, [hWndMain], IDC_COMBO_MONITOR, [hInst], NULL
    mov [hComboMonitor], rax
    invoke SendMessageW, [hComboMonitor], WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hComboMonitor], addr szThemeCfd, NULL
    invoke SendMessageW, [hComboMonitor], CB_ADDSTRING, 0, addr szMonAll
    invoke SendMessageW, [hComboMonitor], CB_ADDSTRING, 0, addr szMonPrimary
    invoke SendMessageW, [hComboMonitor], CB_SETCURSEL, 1, 0

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnSelectReg,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           30, 256, 210, 32, [hWndMain], IDC_BTN_SELECT_REGION, [hInst], NULL
    mov [hBtnSelectRegion], rax

    ; Card 3: Quick Options (Modern Toggle Switches with BS_OWNERDRAW)
    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkCursor,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           32, 332, 230, 26, [hWndMain], IDC_CHK_CURSOR, [hInst], NULL
    mov [hChkCursor], rax
    mov [includeCursor], 1

    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkCountdown,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           32, 360, 230, 26, [hWndMain], IDC_CHK_COUNTDOWN, [hInst], NULL
    mov [hChkCountdown], rax
    mov [showCountdown], 1

    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkAudioSys,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           32, 388, 236, 26, [hWndMain], IDC_CHK_AUDIO_SYS, [hInst], NULL
    mov [hChkAudioSys], rax
    mov [audioSysEnabled], 1

    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkHighlight,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           276, 332, 230, 26, [hWndMain], IDC_CHK_HIGHLIGHT, [hInst], NULL
    mov [hChkHighlight], rax
    mov [highlightCursor], 1

    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkMinibar,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           276, 360, 230, 26, [hWndMain], IDC_CHK_MINIBAR, [hInst], NULL
    mov [hChkMinibar], rax
    mov [showMinibar], 1

    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkAudioMic,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           276, 388, 230, 26, [hWndMain], IDC_CHK_AUDIO_MIC, [hInst], NULL
    mov [hChkAudioMic], rax
    mov [audioMicEnabled], 0

    ; Primary Action Buttons Bar
    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnStart,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           16, 440, 122, 46, [hWndMain], IDC_BTN_START, [hInst], NULL
    mov [hBtnStart], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnPause,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP or WS_DISABLED,\
           144, 440, 114, 46, [hWndMain], IDC_BTN_PAUSE, [hInst], NULL
    mov [hBtnPause], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnStop,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP or WS_DISABLED,\
           264, 440, 114, 46, [hWndMain], IDC_BTN_STOP, [hInst], NULL
    mov [hBtnStop], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnShot,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW or WS_TABSTOP,\
           384, 440, 134, 46, [hWndMain], IDC_BTN_SCREENSHOT, [hInst], NULL
    mov [hBtnScreenshot], rax

    ret
endp


; ----------------------------------------------------------------------------
; CreateGalleryTabControls - Controls for Tab 1 (Галерея)
; ----------------------------------------------------------------------------
proc CreateGalleryTabControls
    ; Header Buttons
    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnOpenRecDir,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           280, 54, 130, 28, [hWndMain], IDC_BTN_OPEN_REC_DIR, [hInst], NULL
    mov [hBtnOpenRecDir], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnRefresh,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           418, 54, 100, 28, [hWndMain], IDC_BTN_REFRESH, [hInst], NULL
    mov [hBtnRefresh], rax

    ; Filter Segment Bar
    invoke CreateWindowExW, 0, addr szButtonClass, addr szFilterAll,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           16, 90, 104, 28, [hWndMain], IDC_FILTER_ALL, [hInst], NULL
    mov [hBtnFilterAll], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szFilterVideo,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           126, 90, 120, 28, [hWndMain], IDC_FILTER_VIDEO, [hInst], NULL
    mov [hBtnFilterVideo], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szFilterShots,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           252, 90, 126, 28, [hWndMain], IDC_FILTER_SHOTS, [hInst], NULL
    mov [hBtnFilterShots], rax

    ; Dark Modern Report ListView (Borderless, flat dark)
    invoke CreateWindowExW, 0, addr szListViewClass, NULL,\
           WS_CHILD or LVS_REPORT or LVS_NOCOLUMNHEADER or LVS_SINGLESEL or LVS_SHOWSELALWAYS or WS_TABSTOP,\
           16, 154, 502, 270, [hWndMain], IDC_LIST_GALLERY, [hInst], NULL
    mov [hListGallery], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hListGallery], addr szThemeExplorer, NULL

    ; Extended styles & dark palette (Clean row select without harsh gridlines)
    invoke SendMessageW, [hListGallery], LVM_SETEXTENDEDLISTVIEWSTYLE, 0,\
           LVS_EX_FULLROWSELECT or LVS_EX_DOUBLEBUFFER
    invoke SendMessageW, [hListGallery], LVM_SETBKCOLOR, 0, COLOR_BG
    invoke SendMessageW, [hListGallery], LVM_SETTEXTBKCOLOR, 0, COLOR_BG
    invoke SendMessageW, [hListGallery], LVM_SETTEXTCOLOR, 0, 0x00E0E0E0

    ; Columns
    mov dword [lvColW.mask], LVCF_TEXT or LVCF_WIDTH or LVCF_SUBITEM
    mov dword [lvColW.cx], 75
    lea rax, [szColType]
    mov qword [lvColW.pszText], rax
    mov dword [lvColW.iSubItem], 0
    invoke SendMessageW, [hListGallery], LVM_INSERTCOLUMNW, 0, addr lvColW

    mov dword [lvColW.cx], 220
    lea rax, [szColName]
    mov qword [lvColW.pszText], rax
    mov dword [lvColW.iSubItem], 1
    invoke SendMessageW, [hListGallery], LVM_INSERTCOLUMNW, 1, addr lvColW

    mov dword [lvColW.cx], 85
    lea rax, [szColSize]
    mov qword [lvColW.pszText], rax
    mov dword [lvColW.iSubItem], 2
    invoke SendMessageW, [hListGallery], LVM_INSERTCOLUMNW, 2, addr lvColW

    mov dword [lvColW.cx], 110
    lea rax, [szColRes]
    mov qword [lvColW.pszText], rax
    mov dword [lvColW.iSubItem], 3
    invoke SendMessageW, [hListGallery], LVM_INSERTCOLUMNW, 3, addr lvColW

    ; Action Buttons below ListView
    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnPlay,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           16, 438, 140, 44, [hWndMain], IDC_BTN_PLAY, [hInst], NULL
    mov [hBtnPlay], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnShowInFolder,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           164, 438, 160, 44, [hWndMain], IDC_BTN_OPEN_FOLDER, [hInst], NULL
    mov [hBtnShowInFolder], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnDelete,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           408, 438, 110, 44, [hWndMain], IDC_BTN_DELETE, [hInst], NULL
    mov [hBtnDelete], rax

    ret
endp


; ----------------------------------------------------------------------------
; CreateSettingsTabControls - Controls for Tab 2 (Настройки)
; ----------------------------------------------------------------------------
proc CreateSettingsTabControls
    ; Card 1: Folders
    invoke CreateWindowExW, 0, addr szEditClass, addr szDefRecDirFull,\
           WS_CHILD or ES_AUTOHSCROLL or WS_TABSTOP,\
           30, 96, 380, 24, [hWndMain], IDC_EDIT_REC_DIR, [hInst], NULL
    mov [hEditRecDir], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hEditRecDir], addr szThemeExplorer, NULL

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnBrowse,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           418, 95, 84, 26, [hWndMain], IDC_BTN_BROWSE_REC, [hInst], NULL
    mov [hBtnBrowseRec], rax

    invoke CreateWindowExW, 0, addr szEditClass, addr szDefShotDirFull,\
           WS_CHILD or ES_AUTOHSCROLL or WS_TABSTOP,\
           30, 144, 380, 24, [hWndMain], IDC_EDIT_SHOT_DIR, [hInst], NULL
    mov [hEditShotDir], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hEditShotDir], addr szThemeExplorer, NULL

    invoke CreateWindowExW, 0, addr szButtonClass, addr szBtnBrowse,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           418, 143, 84, 26, [hWndMain], IDC_BTN_BROWSE_SHOT, [hInst], NULL
    mov [hBtnBrowseShot], rax

    ; Card 2: Video Settings Combos
    invoke CreateWindowExW, 0, addr szComboClass, NULL,\
           WS_CHILD or CBS_DROPDOWNLIST or WS_TABSTOP,\
           220, 192, 130, 120, [hWndMain], IDC_COMBO_FPS, [hInst], NULL
    mov [hComboFps], rax
    invoke SendMessageW, [hComboFps], WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hComboFps], addr szThemeCfd, NULL
    invoke SendMessageW, [hComboFps], CB_ADDSTRING, 0, addr szFps15
    invoke SendMessageW, [hComboFps], CB_ADDSTRING, 0, addr szFps30
    invoke SendMessageW, [hComboFps], CB_ADDSTRING, 0, addr szFps60
    invoke SendMessageW, [hComboFps], CB_SETCURSEL, 1, 0 ; select 30 fps

    invoke CreateWindowExW, 0, addr szComboClass, NULL,\
           WS_CHILD or CBS_DROPDOWNLIST or WS_TABSTOP,\
           220, 220, 130, 120, [hWndMain], IDC_COMBO_FORMAT, [hInst], NULL
    mov [hComboFormat], rax
    invoke SendMessageW, [hComboFormat], WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hComboFormat], addr szThemeCfd, NULL
    invoke SendMessageW, [hComboFormat], CB_ADDSTRING, 0, addr szFmtAvi
    invoke SendMessageW, [hComboFormat], CB_SETCURSEL, 0, 0

    invoke CreateWindowExW, 0, addr szComboClass, NULL,\
           WS_CHILD or CBS_DROPDOWNLIST or WS_TABSTOP,\
           220, 248, 130, 120, [hWndMain], IDC_COMBO_CODEC, [hInst], NULL
    mov [hComboCodec], rax
    invoke SendMessageW, [hComboCodec], WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hComboCodec], addr szThemeCfd, NULL
    invoke SendMessageW, [hComboCodec], CB_ADDSTRING, 0, addr szCodecDib
    invoke SendMessageW, [hComboCodec], CB_SETCURSEL, 0, 0

    ; Card 3: Audio Settings (Modern Pill Toggles)
    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkAudioSys2,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           30, 306, 440, 24, [hWndMain], IDC_CHK_AUDIO_SYS2, [hInst], NULL
    mov [hChkAudioSys2], rax

    invoke CreateWindowExW, 0, addr szButtonClass, addr szChkAudioMic2,\
           WS_CHILD or BS_OWNERDRAW or WS_TABSTOP,\
           30, 332, 440, 24, [hWndMain], IDC_CHK_AUDIO_MIC2, [hInst], NULL
    mov [hChkAudioMic2], rax

    invoke CreateWindowExW, 0, addr szComboClass, NULL,\
           WS_CHILD or CBS_DROPDOWNLIST or WS_TABSTOP,\
           160, 356, 120, 100, [hWndMain], IDC_COMBO_AUDIO_BITRATE, [hInst], NULL
    mov [hComboAudioBitrate], rax
    invoke SendMessageW, [hComboAudioBitrate], WM_SETFONT, [hFontRegular], 1
    invoke SetWindowTheme, [hComboAudioBitrate], addr szThemeCfd, NULL
    invoke SendMessageW, [hComboAudioBitrate], CB_ADDSTRING, 0, addr szBitrate192
    invoke SendMessageW, [hComboAudioBitrate], CB_SETCURSEL, 0, 0

    ret
endp


proc DrawAutoText uses rbx r12 r13 r14 r15, hDC, posX, posY, lpString
    mov r12, rcx
    mov r13d, edx
    mov r14d, r8d
    mov r15, r9

    invoke lstrlenW, r15
    invoke TextOutW, r12, r13d, r14d, r15, eax
    ret
endp

; ----------------------------------------------------------------------------
; PaintCustomBackgroundAndCards - Paints dark background and rounded cards
; ----------------------------------------------------------------------------
proc PaintCustomBackgroundAndCards uses rbx rsi rdi
    invoke BeginPaint, [hWndMain], addr ps
    mov rbx, rax ; rbx = hdc

    ; 1. Draw Tab 0 Cards
    cmp [currentTab], TAB_RECORD
    jne .check_tab1

    invoke SelectObject, rbx, [hBrushCard]
    invoke SelectObject, rbx, [hPenBorder]

    ; Card 1: Текущее состояние
    invoke RoundRect, rbx, 16, 54, 518, 190, 8, 8
    ; Card 2: Источник захвата
    invoke RoundRect, rbx, 16, 200, 518, 296, 8, 8
    ; Card 3: Быстрые опции
    invoke RoundRect, rbx, 16, 306, 518, 426, 8, 8

    ; Text Labels on Cards
    invoke SetBkMode, rbx, 1 ; TRANSPARENT
    invoke SelectObject, rbx, [hFontCaption]
    invoke SetTextColor, rbx, COLOR_TEXT_MUTED

    fastcall DrawAutoText, rbx, 28, 52, addr szCardHeroTitle
    fastcall DrawAutoText, rbx, 28, 198, addr szCardTargetTitle
    fastcall DrawAutoText, rbx, 28, 304, addr szCardOptsTitle
    fastcall DrawAutoText, rbx, 30, 226, addr szLblMonitor
    jmp .finish_paint

.check_tab1:
    cmp [currentTab], TAB_GALLERY
    jne .check_tab2

    ; Header title on Gallery
    invoke SetBkMode, rbx, 1
    invoke SelectObject, rbx, [hFontTitle]
    invoke SetTextColor, rbx, 0x00FFFFFF
    fastcall DrawAutoText, rbx, 18, 58, addr szGalleryTitle

    ; 1px Border framing the Entire Gallery Table
    invoke SelectObject, rbx, [hBrushBg]
    invoke SelectObject, rbx, [hPenBorder]
    invoke RoundRect, rbx, 15, 126, 519, 426, 6, 6

    ; Dark Header Bar Background (y=127..152)
    invoke SelectObject, rbx, [hBrushCard]
    invoke SelectObject, rbx, [hPenBorder]
    invoke RoundRect, rbx, 15, 126, 519, 153, 6, 6

    ; Header Column Labels (matching Python ttkbootstrap)
    invoke SetBkMode, rbx, 1
    invoke SelectObject, rbx, [hFontBold]
    invoke SetTextColor, rbx, COLOR_TEXT_MAIN
    fastcall DrawAutoText, rbx, 30, 132, addr szColType
    fastcall DrawAutoText, rbx, 100, 132, addr szColName
    fastcall DrawAutoText, rbx, 335, 132, addr szColSize
    fastcall DrawAutoText, rbx, 415, 132, addr szColRes
    jmp .finish_paint

.check_tab2:
    cmp [currentTab], TAB_SETTINGS
    jne .finish_paint

    invoke SelectObject, rbx, [hBrushCard]
    invoke SelectObject, rbx, [hPenBorder]

    ; Card 1: Папки
    invoke RoundRect, rbx, 16, 54, 518, 164, 8, 8
    ; Card 2: Видео и FPS
    invoke RoundRect, rbx, 16, 174, 518, 280, 8, 8
    ; Card 3: Звук
    invoke RoundRect, rbx, 16, 290, 518, 378, 8, 8
    ; Card 4: Горячие клавиши
    invoke RoundRect, rbx, 16, 388, 518, 484, 8, 8

    ; 1px subtle borders around edit boxes
    invoke SelectObject, rbx, [hPenBorderLight]
    invoke RoundRect, rbx, 28, 94, 412, 122, 4, 4
    invoke RoundRect, rbx, 28, 142, 412, 170, 4, 4

    invoke SetBkMode, rbx, 1
    invoke SelectObject, rbx, [hFontCaption]
    invoke SetTextColor, rbx, COLOR_TEXT_MUTED

    fastcall DrawAutoText, rbx, 28, 52, addr szCardFoldersTitle
    fastcall DrawAutoText, rbx, 30, 74, addr szLblRecFolder
    fastcall DrawAutoText, rbx, 30, 124, addr szLblShotFolder

    fastcall DrawAutoText, rbx, 28, 172, addr szCardVideoTitle
    fastcall DrawAutoText, rbx, 30, 194, addr szLblFps
    fastcall DrawAutoText, rbx, 30, 222, addr szLblContainer
    fastcall DrawAutoText, rbx, 30, 250, addr szLblCodec

    fastcall DrawAutoText, rbx, 28, 288, addr szCardAudioTitle
    fastcall DrawAutoText, rbx, 30, 358, addr szLblBitrate

    fastcall DrawAutoText, rbx, 28, 386, addr szCardHotkeysTitle
    fastcall DrawAutoText, rbx, 30, 404, addr szLblHkStart
    fastcall DrawAutoText, rbx, 30, 424, addr szLblHkPause
    fastcall DrawAutoText, rbx, 30, 444, addr szLblHkStop
    fastcall DrawAutoText, rbx, 30, 464, addr szLblHkShot

    ; Hotkey key badges
    invoke SelectObject, rbx, [hBrushBtnDark]
    invoke SelectObject, rbx, [hPenBorderLight]
    invoke RoundRect, rbx, 160, 400, 220, 418, 4, 4
    invoke RoundRect, rbx, 160, 420, 220, 438, 4, 4
    invoke RoundRect, rbx, 160, 440, 220, 458, 4, 4
    invoke RoundRect, rbx, 160, 460, 220, 478, 4, 4

    invoke SetTextColor, rbx, 0x00FFFFFF
    invoke SelectObject, rbx, [hFontBold]
    fastcall DrawAutoText, rbx, 183, 401, addr szKeyF5
    fastcall DrawAutoText, rbx, 183, 421, addr szKeyF6
    fastcall DrawAutoText, rbx, 179, 441, addr szKeyF10
    fastcall DrawAutoText, rbx, 179, 461, addr szKeyF11

.finish_paint:
    invoke EndPaint, [hWndMain], addr ps
    ret
endp


; ----------------------------------------------------------------------------
; DrawCustomButton - Renders modern rounded colored buttons via WM_DRAWITEM
; ----------------------------------------------------------------------------
proc DrawCustomButton uses rbx rsi rdi r12 r13 r14 r15, lpDrawItem
    mov rdi, rcx ; first argument lpDrawItem is in rcx!
    mov rsi, [rdi+32] ; hDC
    mov ebx, [rdi+4]  ; CtlID

    mov r12d, dword [rdi+40] ; left
    mov r13d, dword [rdi+44] ; top
    mov r14d, dword [rdi+48] ; right
    mov r15d, dword [rdi+52] ; bottom

    ; ------------------------------------------------------------------------
    ; Top Tab Buttons
    ; ------------------------------------------------------------------------
    cmp ebx, IDC_TAB_BTN_RECORD
    jb .check_toggles
    cmp ebx, IDC_TAB_BTN_SETTINGS
    ja .check_toggles

    ; Determine if this tab is currently active
    mov edx, [currentTab]
    add edx, IDC_TAB_BTN_RECORD
    cmp ebx, edx
    jne .inactive_tab

    ; Active Tab: Blue-Slate Surface with Green Accent Underline
    invoke SelectObject, rsi, [hBrushTabActive]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6

    ; Draw Green Accent Bottom Line
    invoke CreatePen, 0, 2, COLOR_TAB_ACTIVE_BORDER
    push rax
    invoke SelectObject, rsi, rax
    mov eax, r15d
    sub eax, 2
    invoke MoveToEx, rsi, r12d, eax, NULL
    mov eax, r15d
    sub eax, 2
    invoke LineTo, rsi, r14d, eax
    pop rax
    invoke DeleteObject, rax

    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_TEXT_MAIN
    invoke SelectObject, rsi, [hFontBold]
    jmp .draw_tab_text

.inactive_tab:
    invoke SelectObject, rsi, [hBrushTabInactive]
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6

    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_TEXT_MUTED
    invoke SelectObject, rsi, [hFontRegular]

.draw_tab_text:
    cmp ebx, IDC_TAB_BTN_RECORD
    jne .draw_tab_gal
    lea rax, [szTabRecord]
    jmp .exec_draw_text
.draw_tab_gal:
    cmp ebx, IDC_TAB_BTN_GALLERY
    jne .draw_tab_set
    lea rax, [szTabGallery]
    jmp .exec_draw_text
.draw_tab_set:
    lea rax, [szTabSettings]
    jmp .exec_draw_text

; ----------------------------------------------------------------------------
; Quick Options Modern Switch Toggles
; ----------------------------------------------------------------------------
.check_toggles:
    cmp ebx, IDC_CHK_CURSOR
    je .toggle_cursor
    cmp ebx, IDC_CHK_COUNTDOWN
    je .toggle_countdown
    cmp ebx, IDC_CHK_AUDIO_SYS
    je .toggle_audiosys
    cmp ebx, IDC_CHK_AUDIO_SYS2
    je .toggle_audiosys2
    cmp ebx, IDC_CHK_HIGHLIGHT
    je .toggle_highlight
    cmp ebx, IDC_CHK_MINIBAR
    je .toggle_minibar
    cmp ebx, IDC_CHK_AUDIO_MIC
    je .toggle_audiomic
    cmp ebx, IDC_CHK_AUDIO_MIC2
    je .toggle_audiomic2
    jmp .action_buttons

.toggle_cursor:
    mov edx, [includeCursor]
    lea rax, [szChkCursor]
    jmp .draw_toggle_pill

.toggle_countdown:
    mov edx, [showCountdown]
    lea rax, [szChkCountdown]
    jmp .draw_toggle_pill

.toggle_audiosys:
    mov edx, [audioSysEnabled]
    lea rax, [szChkAudioSys]
    jmp .draw_toggle_pill

.toggle_audiosys2:
    mov edx, [audioSysEnabled]
    lea rax, [szChkAudioSys2]
    jmp .draw_toggle_pill

.toggle_highlight:
    mov edx, [highlightCursor]
    lea rax, [szChkHighlight]
    jmp .draw_toggle_pill

.toggle_minibar:
    mov edx, [showMinibar]
    lea rax, [szChkMinibar]
    jmp .draw_toggle_pill

.toggle_audiomic:
    mov edx, [audioMicEnabled]
    lea rax, [szChkAudioMic]
    jmp .draw_toggle_pill

.toggle_audiomic2:
    mov edx, [audioMicEnabled]
    lea rax, [szChkAudioMic2]
    jmp .draw_toggle_pill

.draw_toggle_pill:
    mov r13d, edx ; r13d = state (0 or 1)
    mov r14, rax  ; r14 = string pointer (szChkCursor, etc.)

    ; 1. Clear control background with Card Background
    lea rdx, [rdi+40] ; rcItem
    invoke FillRect, rsi, rdx, [hBrushCard]

    ; 2. Draw Pill track: width 38, height 20
    ; Pill bounds: left=2, top=3, right=40, bottom=23
    test r13d, r13d
    jz .pill_off

.pill_on:
    invoke SelectObject, rsi, [hBrushToggleOn]
    invoke SelectObject, rsi, [hPenToggleOn]
    invoke RoundRect, rsi, 2, 3, 40, 23, 18, 18

    ; Active Knob (White circle on right: left=22, top=4, right=38, bottom=20)
    invoke SelectObject, rsi, [hBrushKnobOn]
    invoke SelectObject, rsi, [hPenToggleOn]
    invoke Ellipse, rsi, 22, 4, 38, 20
    jmp .pill_draw_text

.pill_off:
    invoke SelectObject, rsi, [hBrushToggleOff]
    invoke SelectObject, rsi, [hPenToggleOff]
    invoke RoundRect, rsi, 2, 3, 40, 23, 18, 18

    ; Inactive Knob (Grey circle on left: left=4, top=4, right=20, bottom=20)
    invoke SelectObject, rsi, [hBrushKnobOff]
    invoke SelectObject, rsi, [hPenToggleOff]
    invoke Ellipse, rsi, 4, 4, 20, 20

.pill_draw_text:
    ; Draw label text to the right of the switch at x=48, y=3
    invoke SetBkMode, rsi, 1 ; TRANSPARENT
    invoke SetTextColor, rsi, 0x00FFFFFF ; pure white
    invoke SelectObject, rsi, [hFontRegular]
    fastcall DrawAutoText, rsi, 48, 3, r14
    ret

; ----------------------------------------------------------------------------
; Primary Record Actions Bar
; ----------------------------------------------------------------------------
.action_buttons:
    cmp ebx, IDC_BTN_START
    jne .check_btn_pause

    ; Start Button (Emerald Green #00BC7D)
    test dword [rdi+16], ODS_SELECTED
    jnz .start_pressed
    invoke SelectObject, rsi, [hBrushBtnGreen]
    jmp .draw_start_rect
.start_pressed:
    invoke SelectObject, rsi, [hBrushBtnGreenHover]
.draw_start_rect:
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6

    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00FFFFFF ; Pure White
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnStart]
    jmp .exec_draw_text

.check_btn_pause:
    cmp ebx, IDC_BTN_PAUSE
    jne .check_btn_stop

    ; Pause Button
    test dword [rdi+16], ODS_DISABLED
    jnz .draw_disabled_btn

    cmp [recState], STATE_PAUSED
    je .pause_amber

    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_TEXT_MAIN
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnPause]
    jmp .exec_draw_text

.pause_amber:
    invoke CreateSolidBrush, COLOR_STATUS_AMBER
    push rax
    invoke SelectObject, rsi, rax
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6
    pop rax
    invoke DeleteObject, rax
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00FFFFFF
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnPause]
    jmp .exec_draw_text

.check_btn_stop:
    cmp ebx, IDC_BTN_STOP
    jne .check_btn_shot

    ; Stop Button
    test dword [rdi+16], ODS_DISABLED
    jnz .draw_disabled_btn

    invoke CreateSolidBrush, COLOR_STATUS_RED
    push rax
    invoke SelectObject, rsi, rax
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6
    pop rax
    invoke DeleteObject, rax

    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00FFFFFF
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnStop]
    jmp .exec_draw_text

.check_btn_shot:
    cmp ebx, IDC_BTN_SCREENSHOT
    jne .check_btn_play

    ; Screenshot Button (Dark with Cyan outline bootstyle="info-outline")
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenCyan]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6

    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00D8B400 ; Info Cyan text
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnShot]
    jmp .exec_draw_text

; ----------------------------------------------------------------------------
; Gallery Buttons
; ----------------------------------------------------------------------------
.check_btn_play:
    cmp ebx, IDC_BTN_PLAY
    jne .check_btn_show_folder

    invoke SelectObject, rsi, [hBrushBtnGreen]
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00FFFFFF
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnPlay]
    jmp .exec_draw_text

.check_btn_show_folder:
    cmp ebx, IDC_BTN_OPEN_FOLDER
    jne .check_btn_delete

    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_TEXT_MAIN
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnShowInFolder]
    jmp .exec_draw_text

.check_btn_delete:
    cmp ebx, IDC_BTN_DELETE
    jne .check_filter_btns

    invoke SelectObject, rsi, [hBrushCard]
    invoke SelectObject, rsi, [hPenBtnRed]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_STATUS_RED
    invoke SelectObject, rsi, [hFontBold]
    lea rax, [szBtnDelete]
    jmp .exec_draw_text

; ----------------------------------------------------------------------------
; Filter Segment Buttons (Все файлы, Только видео, Только снимки)
; ----------------------------------------------------------------------------
.check_filter_btns:
    cmp ebx, IDC_FILTER_ALL
    jb .check_open_rec_dir
    cmp ebx, IDC_FILTER_SHOTS
    ja .check_open_rec_dir

    cmp ebx, [gal_filterMode]
    jne .filter_inactive

    ; Active filter button: Solid Blue Primary with pure white bold text
    invoke SelectObject, rsi, [hBrushTabActive]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 4, 4
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00FFFFFF
    invoke SelectObject, rsi, [hFontBold]
    jmp .get_filter_text

.filter_inactive:
    ; Inactive filter button: Dark button with muted text
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 4, 4
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_TEXT_MUTED
    invoke SelectObject, rsi, [hFontRegular]

.get_filter_text:
    cmp ebx, IDC_FILTER_ALL
    jne .f_vid
    lea rax, [szFilterAll]
    jmp .exec_draw_text
.f_vid:
    cmp ebx, IDC_FILTER_VIDEO
    jne .f_shot
    lea rax, [szFilterVideo]
    jmp .exec_draw_text
.f_shot:
    lea rax, [szFilterShots]
    jmp .exec_draw_text

; ----------------------------------------------------------------------------
; Info Outline Buttons (Папка записей)
; ----------------------------------------------------------------------------
.check_open_rec_dir:
    cmp ebx, IDC_BTN_OPEN_REC_DIR
    jne .check_generic_dark

    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenCyan]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 4, 4
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00D8B400 ; Cyan text
    invoke SelectObject, rsi, [hFontRegular]
    lea rax, [szBtnOpenRecDir]
    jmp .exec_draw_text

; ----------------------------------------------------------------------------
; Generic Secondary Buttons (Выбрать область, Сбросить, Обновить, Обзор...)
; ----------------------------------------------------------------------------
.check_generic_dark:
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 4, 4
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_TEXT_MAIN
    invoke SelectObject, rsi, [hFontRegular]

    cmp ebx, IDC_BTN_SELECT_REGION
    jne .g1
    lea rax, [szBtnSelectReg]
    jmp .exec_draw_text
.g1:
    cmp ebx, IDC_BTN_FULLSCREEN
    jne .g2
    lea rax, [szBtnResetFull]
    jmp .exec_draw_text
.g2:
    cmp ebx, IDC_BTN_REFRESH
    jne .g3
    lea rax, [szBtnRefresh]
    jmp .exec_draw_text
.g3:
    lea rax, [szBtnBrowse]
    jmp .exec_draw_text

.draw_disabled_btn:
    invoke SelectObject, rsi, [hBrushBtnDisabled]
    invoke SelectObject, rsi, [hPenBorder]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 6, 6
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, COLOR_TEXT_DISABLED
    invoke SelectObject, rsi, [hFontBold]

    cmp ebx, IDC_BTN_PAUSE
    jne .d_stop
    lea rax, [szBtnPause]
    jmp .exec_draw_text
.d_stop:
    lea rax, [szBtnStop]

.exec_draw_text:
    mov r10, rax
    lea r11, [rdi+40] ; rcItem
    invoke DrawTextW, rsi, r10, -1, r11, DT_CENTER or DT_VCENTER or DT_SINGLELINE or DT_NOPREFIX
    ret
endp


; ----------------------------------------------------------------------------
; SwitchTab - Displays controls for the selected tab and hides the others
; ----------------------------------------------------------------------------
proc SwitchTab uses rbx, nTab
    mov eax, ecx ; nTab is in ecx!
    mov [currentTab], eax

    ; Invalidate Top Tab Buttons
    invoke InvalidateRect, [hTabBtnRecord], NULL, TRUE
    invoke InvalidateRect, [hTabBtnGallery], NULL, TRUE
    invoke InvalidateRect, [hTabBtnSettings], NULL, TRUE

    ; Tab 0 Controls Visibility
    xor ebx, ebx
    cmp [currentTab], TAB_RECORD
    jne .t0_hide
    mov ebx, 1 ; SW_SHOWNORMAL
.t0_hide:
    invoke ShowWindow, [hLblStatusBadge], ebx
    invoke ShowWindow, [hLblRegionInfo], ebx
    invoke ShowWindow, [hLblTimer], ebx
    invoke ShowWindow, [hLblFrames], ebx
    invoke ShowWindow, [hLblFps], ebx
    invoke ShowWindow, [hLblSize], ebx
    invoke ShowWindow, [hComboMonitor], ebx
    invoke ShowWindow, [hBtnSelectRegion], ebx
    invoke ShowWindow, [hBtnFullscreen], ebx
    invoke ShowWindow, [hChkCursor], ebx
    invoke ShowWindow, [hChkCountdown], ebx
    invoke ShowWindow, [hChkAudioSys], ebx
    invoke ShowWindow, [hChkHighlight], ebx
    invoke ShowWindow, [hChkMinibar], ebx
    invoke ShowWindow, [hChkAudioMic], ebx
    invoke ShowWindow, [hBtnStart], ebx
    invoke ShowWindow, [hBtnPause], ebx
    invoke ShowWindow, [hBtnStop], ebx
    invoke ShowWindow, [hBtnScreenshot], ebx

    ; Tab 1 Controls Visibility
    xor ebx, ebx
    cmp [currentTab], TAB_GALLERY
    jne .t1_hide
    mov ebx, 1
.t1_hide:
    invoke ShowWindow, [hBtnOpenRecDir], ebx
    invoke ShowWindow, [hBtnRefresh], ebx
    invoke ShowWindow, [hBtnFilterAll], ebx
    invoke ShowWindow, [hBtnFilterVideo], ebx
    invoke ShowWindow, [hBtnFilterShots], ebx
    invoke ShowWindow, [hListGallery], ebx
    invoke ShowWindow, [hBtnPlay], ebx
    invoke ShowWindow, [hBtnShowInFolder], ebx
    invoke ShowWindow, [hBtnDelete], ebx

    ; Tab 2 Controls Visibility
    xor ebx, ebx
    cmp [currentTab], TAB_SETTINGS
    jne .t2_hide
    mov ebx, 1
.t2_hide:
    invoke ShowWindow, [hEditRecDir], ebx
    invoke ShowWindow, [hBtnBrowseRec], ebx
    invoke ShowWindow, [hEditShotDir], ebx
    invoke ShowWindow, [hBtnBrowseShot], ebx
    invoke ShowWindow, [hComboFps], ebx
    invoke ShowWindow, [hComboFormat], ebx
    invoke ShowWindow, [hComboCodec], ebx
    invoke ShowWindow, [hChkAudioSys2], ebx
    invoke ShowWindow, [hChkAudioMic2], ebx
    invoke ShowWindow, [hComboAudioBitrate], ebx

    ; If switching to gallery, refresh list
    cmp [currentTab], TAB_GALLERY
    jne .done
    call RefreshGalleryList

.done:
    invoke InvalidateRect, [hWndMain], NULL, TRUE
    ret
endp


; ----------------------------------------------------------------------------
; UpdateUIState - Updates button states and status texts
; ----------------------------------------------------------------------------
proc UpdateUIState
    cmp [recState], STATE_IDLE
    je .state_idle
    cmp [recState], STATE_RECORDING
    je .state_rec
    cmp [recState], STATE_PAUSED
    je .state_pause
    ret

.state_idle:
    invoke EnableWindow, [hBtnStart], TRUE
    invoke EnableWindow, [hBtnPause], FALSE
    invoke EnableWindow, [hBtnStop], FALSE
    invoke EnableWindow, [hBtnSelectRegion], TRUE
    invoke EnableWindow, [hBtnFullscreen], TRUE
    invoke SetWindowTextW, [hLblStatusBadge], addr szStatusIdle
    invoke InvalidateRect, [hBtnStart], NULL, TRUE
    invoke InvalidateRect, [hBtnPause], NULL, TRUE
    invoke InvalidateRect, [hBtnStop], NULL, TRUE
    ret

.state_rec:
    invoke EnableWindow, [hBtnStart], FALSE
    invoke EnableWindow, [hBtnPause], TRUE
    invoke EnableWindow, [hBtnStop], TRUE
    invoke EnableWindow, [hBtnSelectRegion], FALSE
    invoke EnableWindow, [hBtnFullscreen], FALSE
    invoke SetWindowTextW, [hLblStatusBadge], addr szStatusRecording
    invoke InvalidateRect, [hBtnStart], NULL, TRUE
    invoke InvalidateRect, [hBtnPause], NULL, TRUE
    invoke InvalidateRect, [hBtnStop], NULL, TRUE
    ret

.state_pause:
    invoke EnableWindow, [hBtnStart], FALSE
    invoke EnableWindow, [hBtnPause], TRUE
    invoke EnableWindow, [hBtnStop], TRUE
    invoke SetWindowTextW, [hLblStatusBadge], addr szStatusPaused
    invoke InvalidateRect, [hBtnPause], NULL, TRUE
    ret
endp

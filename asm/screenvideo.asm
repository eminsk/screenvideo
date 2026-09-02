; ============================================================================
; ScreenCapture Pro v2.0 (x64 Native Assembly Edition)
; High-Performance Desktop Recording & Instant Screenshot Engine
; Modern Dark Theme UI matching ScreenCapture Pro Python Edition
; Pure 64-bit Flat Assembler (FASM x64) - Zero CPU Overhead
; ============================================================================

format PE64 GUI 6.0
entry start

include 'INCLUDE\win64a.inc'
include 'INCLUDE\ENCODING\UTF8.INC'
include 'const.inc'
include 'data.inc'
include 'bss.inc'

section '.text' code readable executable

include 'ui.inc'
include 'capture.inc'
include 'region.inc'
include 'gallery.inc'
include 'minibar.inc'

; ----------------------------------------------------------------------------
; Entry Point - Application Initialization & Windows 10/11 Dark Mode Setup
; ----------------------------------------------------------------------------
start:
    sub rsp, 28h

    ; Initialize Common Controls 6.0
    lea rcx, [icc]
    call [InitCommonControlsEx]

    ; Dynamic Path Resolution & Embedded Audio Auto-Extraction
    call InitializeAppDirectories

    ; Retrieve Application Instance
    invoke GetModuleHandleW, NULL
    mov [hInst], rax

    ; Register Main Window Class
    mov [wc.cbSize], sizeof.WNDCLASSEX
    mov [wc.style], 3 ; CS_HREDRAW | CS_VREDRAW
    lea rax, [MainWndProc]
    mov [wc.lpfnWndProc], rax
    mov [wc.cbClsExtra], 0
    mov [wc.cbWndExtra], 0
    mov rax, [hInst]
    mov [wc.hInstance], rax

    ; Load embedded application icon (ID 1)
    invoke LoadImageW, [hInst], 1, 1, 32, 32, 0 ; IMAGE_ICON
    mov [wc.hIcon], rax
    mov [wc.hIconSm], rax

    invoke LoadCursorW, 0, 32512 ; IDC_ARROW
    mov [wc.hCursor], rax
    invoke CreateSolidBrush, COLOR_BG
    mov [wc.hbrBackground], rax
    mov [wc.lpszMenuName], 0
    lea rax, [szMainClass]
    mov [wc.lpszClassName], rax
    lea rcx, [wc]
    call [RegisterClassExW]

    ; Calculate Screen Center Coordinates
    invoke GetSystemMetrics, 0 ; SM_CXSCREEN
    sub eax, MAIN_WIDTH
    shr eax, 1
    mov r12d, eax ; X

    invoke GetSystemMetrics, 1 ; SM_CYSCREEN
    sub eax, MAIN_HEIGHT
    shr eax, 1
    mov r13d, eax ; Y

    ; Create Centered Main Window (Fixed modern geometry)
    sub rsp, 60h
    mov ecx, WS_EX_APPWINDOW
    lea rdx, [szMainClass]
    lea r8, [szAppTitle]
    mov r9d, WS_OVERLAPPEDWINDOW and not (0x00040000 or 0x00010000) ; no thickframe, no maximize
    mov dword [rsp+20h], r12d          ; X
    mov dword [rsp+28h], r13d          ; Y
    mov dword [rsp+30h], MAIN_WIDTH    ; Width
    mov dword [rsp+38h], MAIN_HEIGHT   ; Height
    mov qword [rsp+40h], 0             ; hWndParent
    mov qword [rsp+48h], 0             ; hMenu
    mov rax, [hInst]
    mov qword [rsp+50h], rax           ; hInstance
    mov qword [rsp+58h], 0             ; lpParam
    call [CreateWindowExW]
    add rsp, 60h
    mov [hWndMain], rax

    ; Enable Windows 10/11 Immersive Dark Titlebar (DWMWA_USE_IMMERSIVE_DARK_MODE = 20 & 19)
    mov dword [dwDarkVal], 1
    invoke DwmSetWindowAttribute, [hWndMain], 20, addr dwDarkVal, 4
    invoke DwmSetWindowAttribute, [hWndMain], 19, addr dwDarkVal, 4

    ; Display and Refresh Window
    invoke ShowWindow, [hWndMain], 1 ; SW_SHOWNORMAL
    invoke UpdateWindow, [hWndMain]

; ----------------------------------------------------------------------------
; Message Pump Loop
; ----------------------------------------------------------------------------
.msg_loop:
    invoke GetMessageW, addr msg, NULL, 0, 0
    test eax, eax
    jle .exit_app

    invoke TranslateMessage, addr msg
    invoke DispatchMessageW, addr msg
    jmp .msg_loop

.exit_app:
    invoke ExitProcess, [msg.wParam]


; ----------------------------------------------------------------------------
; MainWndProc - Main Window Event Dispatcher
; ----------------------------------------------------------------------------
proc MainWndProc uses rbx rsi rdi, hwnd, wmsg, wparam, lparam
    mov [hwnd], rcx
    mov [wmsg], rdx
    mov [wparam], r8
    mov [lparam], r9

    cmp edx, WM_CREATE
    je .on_create
    cmp edx, WM_ERASEBKGND
    je .on_erasebkgnd
    cmp edx, WM_PAINT
    je .on_paint
    cmp edx, WM_DRAWITEM
    je .on_drawitem
    cmp edx, WM_CTLCOLORSTATIC
    je .on_ctlcolorstatic
    cmp edx, WM_CTLCOLORBTN
    je .on_ctlcolorbtn
    cmp edx, WM_CTLCOLOREDIT
    je .on_ctlcoloredit
    cmp edx, WM_CTLCOLORLISTBOX
    je .on_ctlcoloredit
    cmp edx, WM_COMMAND
    je .on_command
    cmp edx, WM_NOTIFY
    je .on_notify
    cmp edx, WM_HOTKEY
    je .on_hotkey
    cmp edx, WM_USER_TELEMETRY
    je .on_telemetry
    cmp edx, WM_USER_REC_FINISHED
    je .on_rec_finished
    cmp edx, WM_DESTROY
    je .on_destroy

    invoke DefWindowProcW, [hwnd], [wmsg], [wparam], [lparam]
    ret

; ----------------------------------------------------------------------------
; WM_CREATE
; ----------------------------------------------------------------------------
.on_create:
    mov [hWndMain], rcx

    ; Set initial states
    mov [currentTab], TAB_RECORD
    mov [recState], STATE_IDLE
    mov [targetFps], 30
    mov [includeCursor], 1
    mov [showCountdown], 1
    mov [audioSysEnabled], 1
    mov [highlightCursor], 1
    mov [showMinibar], 1
    mov [audioMicEnabled], 0
    mov [isCustomRegion], 0
    mov [gal_filterMode], IDC_FILTER_ALL

    ; Build Dark Modern UI Subsystems
    call CreateFontsAndBrushes
    call CreateMainControls
    call CreateRecordTabControls
    call CreateGalleryTabControls
    call CreateSettingsTabControls
    call InitMiniBar

    ; Activate initial Record Tab
    fastcall SwitchTab, TAB_RECORD
    call UpdateUIState
    call RefreshGalleryList

    ; Register Global System Hotkeys (F5, F6, F10, F11)
    invoke RegisterHotKey, [hWndMain], ID_HOTKEY_START, 0, VK_F5
    invoke RegisterHotKey, [hWndMain], ID_HOTKEY_PAUSE, 0, VK_F6
    invoke RegisterHotKey, [hWndMain], ID_HOTKEY_STOP, 0, VK_F10
    invoke RegisterHotKey, [hWndMain], ID_HOTKEY_SCREENSHOT, 0, VK_F11

    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; WM_ERASEBKGND - Dark Background Paint
; ----------------------------------------------------------------------------
.on_erasebkgnd:
    invoke GetClientRect, [hwnd], addr rcMainClient
    invoke FillRect, [wparam], addr rcMainClient, [hBrushBg]
    mov eax, 1
    ret

; ----------------------------------------------------------------------------
; WM_PAINT - Paints Dark Rounded Cards & Headers
; ----------------------------------------------------------------------------
.on_paint:
    call PaintCustomBackgroundAndCards
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; WM_DRAWITEM - Renders Owner-Draw Modern Colored Buttons
; ----------------------------------------------------------------------------
.on_drawitem:
    fastcall DrawCustomButton, r9
    mov eax, 1
    ret

; ----------------------------------------------------------------------------
; WM_CTLCOLORSTATIC - Dark Label Styling
; ----------------------------------------------------------------------------
.on_ctlcolorstatic:
    mov rsi, [wparam] ; hdc
    invoke SetBkMode, rsi, 1 ; TRANSPARENT

    ; Digital Clock Timer: Light Blue-Grey
    mov rax, [lparam]
    cmp rax, [hLblTimer]
    jne .check_badge
    invoke SetTextColor, rsi, COLOR_TIMER
    mov rax, [hBrushCard]
    ret

.check_badge:
    mov rax, [lparam]
    cmp rax, [hLblStatusBadge]
    jne .check_stats

    cmp [recState], STATE_RECORDING
    je .badge_rec
    cmp [recState], STATE_PAUSED
    je .badge_pause

    invoke SetTextColor, rsi, COLOR_STATUS_GREEN
    mov rax, [hBrushCard]
    ret
.badge_rec:
    invoke SetTextColor, rsi, COLOR_STATUS_RED
    mov rax, [hBrushCard]
    ret
.badge_pause:
    invoke SetTextColor, rsi, COLOR_STATUS_AMBER
    mov rax, [hBrushCard]
    ret

.check_stats:
    mov rax, [lparam]
    cmp rax, [hLblFrames]
    je .set_muted
    cmp rax, [hLblFps]
    je .set_muted
    cmp rax, [hLblSize]
    je .set_muted
    cmp rax, [hLblRegionInfo]
    je .set_muted
    jmp .check_card_labels

.set_muted:
    invoke SetTextColor, rsi, COLOR_TEXT_MUTED
    mov rax, [hBrushCard]
    ret

.check_card_labels:
    mov rax, [lparam]
    cmp rax, [hStatusBar]
    jne .not_status
    invoke SetTextColor, rsi, COLOR_TEXT_MUTED
    mov rax, [hBrushBg]
    ret
.not_status:
    invoke SetTextColor, rsi, 0x00FFFFFF ; crisp bright white
    mov rax, [hBrushCard]
    ret

; ----------------------------------------------------------------------------
; WM_CTLCOLORBTN & WM_CTLCOLOREDIT
; ----------------------------------------------------------------------------
.on_ctlcolorbtn:
    mov rsi, [wparam]
    invoke SetBkMode, rsi, 1
    invoke SetTextColor, rsi, 0x00FFFFFF
    mov rax, [hBrushCard]
    ret

.on_ctlcoloredit:
    mov rsi, [wparam]
    invoke SetBkMode, rsi, 2 ; OPAQUE
    invoke SetBkColor, rsi, 0x00262525
    invoke SetTextColor, rsi, 0x00FFFFFF
    mov rax, [hBrushCard]
    ret

; ----------------------------------------------------------------------------
; WM_COMMAND - Button & Control Dispatcher
; ----------------------------------------------------------------------------
.on_command:
    mov eax, r8d
    and eax, 0xFFFF ; LOWORD(wParam)

    ; Top Tab Buttons
    cmp eax, IDC_TAB_BTN_RECORD
    je .cmd_tab_record
    cmp eax, IDC_TAB_BTN_GALLERY
    je .cmd_tab_gallery
    cmp eax, IDC_TAB_BTN_SETTINGS
    je .cmd_tab_settings

    ; Record Actions
    cmp eax, IDC_BTN_START
    je .cmd_start
    cmp eax, IDC_BTN_PAUSE
    je .cmd_pause
    cmp eax, IDC_BTN_STOP
    je .cmd_stop
    cmp eax, IDC_BTN_SCREENSHOT
    je .cmd_screenshot

    ; Target Selection
    cmp eax, IDC_BTN_SELECT_REGION
    je .cmd_select_region
    cmp eax, IDC_BTN_FULLSCREEN
    je .cmd_reset_fullscreen

    ; Options
    cmp eax, IDC_CHK_CURSOR
    je .cmd_chk_cursor
    cmp eax, IDC_CHK_COUNTDOWN
    je .cmd_chk_countdown
    cmp eax, IDC_CHK_AUDIO_SYS
    je .cmd_chk_audiosys
    cmp eax, IDC_CHK_AUDIO_SYS2
    je .cmd_chk_audiosys
    cmp eax, IDC_CHK_HIGHLIGHT
    je .cmd_chk_highlight
    cmp eax, IDC_CHK_MINIBAR
    je .cmd_chk_minibar
    cmp eax, IDC_CHK_AUDIO_MIC
    je .cmd_chk_audiomic
    cmp eax, IDC_CHK_AUDIO_MIC2
    je .cmd_chk_audiomic

    ; Gallery Actions
    cmp eax, IDC_BTN_PLAY
    je .cmd_play
    cmp eax, IDC_BTN_OPEN_FOLDER
    je .cmd_open_folder
    cmp eax, IDC_BTN_OPEN_REC_DIR
    je .cmd_open_rec_dir
    cmp eax, IDC_BTN_REFRESH
    je .cmd_refresh
    cmp eax, IDC_BTN_DELETE
    je .cmd_delete

    ; Gallery Filters
    cmp eax, IDC_FILTER_ALL
    je .cmd_filter_all
    cmp eax, IDC_FILTER_VIDEO
    je .cmd_filter_video
    cmp eax, IDC_FILTER_SHOTS
    je .cmd_filter_shots

    ; Settings
    cmp eax, IDC_COMBO_FPS
    je .cmd_combo_fps

    xor eax, eax
    ret

.cmd_tab_record:
    fastcall SwitchTab, TAB_RECORD
    xor eax, eax
    ret

.cmd_tab_gallery:
    fastcall SwitchTab, TAB_GALLERY
    call RefreshGalleryList
    xor eax, eax
    ret

.cmd_tab_settings:
    fastcall SwitchTab, TAB_SETTINGS
    xor eax, eax
    ret

.cmd_start:
    call StartScreenRecording
    xor eax, eax
    ret

.cmd_pause:
    call PauseScreenRecording
    xor eax, eax
    ret

.cmd_stop:
    call StopScreenRecording
    xor eax, eax
    ret

.cmd_screenshot:
    call TakeInstantScreenshot
    xor eax, eax
    ret

.cmd_select_region:
    call ShowRegionSelector
    xor eax, eax
    ret

.cmd_reset_fullscreen:
    mov [isCustomRegion], 0
    invoke SetWindowTextW, [hLblRegionInfo], addr szTargetFull
    xor eax, eax
    ret

.cmd_chk_cursor:
    xor [includeCursor], 1
    invoke InvalidateRect, [hChkCursor], NULL, TRUE
    xor eax, eax
    ret

.cmd_chk_countdown:
    xor [showCountdown], 1
    invoke InvalidateRect, [hChkCountdown], NULL, TRUE
    xor eax, eax
    ret

.cmd_chk_audiosys:
    xor [audioSysEnabled], 1
    invoke InvalidateRect, [hChkAudioSys], NULL, TRUE
    cmp [hChkAudioSys2], 0
    je .done_audiosys
    invoke InvalidateRect, [hChkAudioSys2], NULL, TRUE
.done_audiosys:
    xor eax, eax
    ret

.cmd_chk_highlight:
    xor [highlightCursor], 1
    invoke InvalidateRect, [hChkHighlight], NULL, TRUE
    xor eax, eax
    ret

.cmd_chk_minibar:
    xor [showMinibar], 1
    invoke InvalidateRect, [hChkMinibar], NULL, TRUE
    xor eax, eax
    ret

.cmd_chk_audiomic:
    xor [audioMicEnabled], 1
    invoke InvalidateRect, [hChkAudioMic], NULL, TRUE
    cmp [hChkAudioMic2], 0
    je .done_audiomic
    invoke InvalidateRect, [hChkAudioMic2], NULL, TRUE
.done_audiomic:
    xor eax, eax
    ret

.cmd_play:
    call OpenSelectedGalleryItem
    xor eax, eax
    ret

.cmd_open_folder:
    call OpenSelectedGalleryFolder
    xor eax, eax
    ret

.cmd_open_rec_dir:
    invoke ShellExecuteW, [hWndMain], addr szShellOpen, addr szRecDir, NULL, NULL, 1
    xor eax, eax
    ret

.cmd_refresh:
    call RefreshGalleryList
    xor eax, eax
    ret

.cmd_delete:
    call DeleteSelectedGalleryItem
    xor eax, eax
    ret

.cmd_filter_all:
    mov [gal_filterMode], IDC_FILTER_ALL
    call RefreshGalleryList
    xor eax, eax
    ret

.cmd_filter_video:
    mov [gal_filterMode], IDC_FILTER_VIDEO
    call RefreshGalleryList
    xor eax, eax
    ret

.cmd_filter_shots:
    mov [gal_filterMode], IDC_FILTER_SHOTS
    call RefreshGalleryList
    xor eax, eax
    ret

.cmd_combo_fps:
    mov eax, r8d
    shr eax, 16 ; HIWORD(wParam) = notification
    cmp eax, 1  ; CBN_SELCHANGE
    jne .cmd_done
    invoke SendMessageW, [hComboFps], CB_GETCURSEL, 0, 0
    cmp eax, 0
    je .fps_15
    cmp eax, 1
    je .fps_30
    cmp eax, 2
    je .fps_60
    jmp .cmd_done
.fps_15:
    mov [targetFps], 15
    jmp .cmd_done
.fps_30:
    mov [targetFps], 30
    jmp .cmd_done
.fps_60:
    mov [targetFps], 60
.cmd_done:
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; WM_NOTIFY - Double-click on Gallery item opens file
; ----------------------------------------------------------------------------
.on_notify:
    mov rax, r9 ; NMHDR*
    test rax, rax
    jz .notify_done
    cmp dword [rax+16], NM_DBLCLK
    jne .notify_done
    mov rcx, [rax]
    cmp rcx, [hListGallery]
    jne .notify_done
    call OpenSelectedGalleryItem
.notify_done:
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; WM_HOTKEY - Global System Keyboard Shortcuts
; ----------------------------------------------------------------------------
.on_hotkey:
    cmp r8d, ID_HOTKEY_START
    je .hk_start
    cmp r8d, ID_HOTKEY_PAUSE
    je .hk_pause
    cmp r8d, ID_HOTKEY_STOP
    je .hk_stop
    cmp r8d, ID_HOTKEY_SCREENSHOT
    je .hk_shot
    xor eax, eax
    ret

.hk_start:
    call StartScreenRecording
    xor eax, eax
    ret

.hk_pause:
    call PauseScreenRecording
    xor eax, eax
    ret

.hk_stop:
    call StopScreenRecording
    xor eax, eax
    ret

.hk_shot:
    call TakeInstantScreenshot
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; Telemetry & Background Thread Events
; ----------------------------------------------------------------------------
.on_telemetry:
    call UpdateTelemetryUI
    call UpdateMiniBarTimer
    xor eax, eax
    ret

.on_rec_finished:
    call UpdateUIState
    call HideMiniBar
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; WM_DESTROY - Clean Shutdown
; ----------------------------------------------------------------------------
.on_destroy:
    invoke UnregisterHotKey, [hWndMain], ID_HOTKEY_START
    invoke UnregisterHotKey, [hWndMain], ID_HOTKEY_PAUSE
    invoke UnregisterHotKey, [hWndMain], ID_HOTKEY_STOP
    invoke UnregisterHotKey, [hWndMain], ID_HOTKEY_SCREENSHOT

    ; Terminate recording if still running
    cmp [recState], STATE_IDLE
    je .clean_exit
    call StopScreenRecording

.clean_exit:
    invoke PostQuitMessage, 0
    xor eax, eax
    ret
endp


; ----------------------------------------------------------------------------
; PE64 Dynamic Link Library Imports
; ----------------------------------------------------------------------------
section '.idata' import data readable writeable

  library kernel32, 'KERNEL32.DLL',\
          user32,   'USER32.DLL',\
          gdi32,    'GDI32.DLL',\
          comctl32, 'COMCTL32.DLL',\
          shell32,  'SHELL32.DLL',\
          avifil32, 'AVIFIL32.DLL',\
          dwmapi,   'DWMAPI.DLL',\
          uxtheme,  'UXTHEME.DLL'

  include 'INCLUDE\API\KERNEL32.INC'
  include 'INCLUDE\API\USER32.INC'
  include 'INCLUDE\API\GDI32.INC'
  include 'INCLUDE\API\COMCTL32.INC'
  include 'INCLUDE\API\SHELL32.INC'

  import avifil32,\
         AVIFileInit,         'AVIFileInit',\
         AVIFileOpenW,        'AVIFileOpenW',\
         AVIFileCreateStreamW,'AVIFileCreateStreamW',\
         AVIStreamSetFormat,  'AVIStreamSetFormat',\
         AVIStreamWrite,      'AVIStreamWrite',\
         AVIStreamRelease,    'AVIStreamRelease',\
         AVIFileRelease,      'AVIFileRelease',\
         AVIFileExit,         'AVIFileExit'

  import dwmapi,\
         DwmSetWindowAttribute, 'DwmSetWindowAttribute'

  import uxtheme,\
         SetWindowTheme,        'SetWindowTheme'



; ----------------------------------------------------------------------------
; PE64 Embedded Resource Directory (.rsrc)
; Icon ID 1 and Application Manifest ID 1 (RT_MANIFEST = 24)
; ----------------------------------------------------------------------------
section '.rsrc' resource data readable

  directory RT_ICON, icons, \
            RT_GROUP_ICON, group_icons, \
            RT_MANIFEST, manifests

  resource icons, \
           1, LANG_NEUTRAL, icon_data

  resource group_icons, \
           1, LANG_NEUTRAL, main_icon

  resource manifests, \
           1, LANG_NEUTRAL, manifest_data

  icon main_icon, icon_data, 'icon.ico'

  resdata manifest_data
    file 'manifest.xml'
  endres

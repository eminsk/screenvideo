; ============================================================================
; ScreenCapture Pro x64 - Floating Mini-Toolbar (MiniBar) Subsystem
; ============================================================================

proc InitMiniBar uses rbx rsi rdi
    ; Register MiniBar Window Class
    mov [wcMiniBar.cbSize], sizeof.WNDCLASSEX
    mov [wcMiniBar.style], CS_HREDRAW or CS_VREDRAW
    lea rax, [MiniBarWndProc]
    mov [wcMiniBar.lpfnWndProc], rax
    mov [wcMiniBar.cbClsExtra], 0
    mov [wcMiniBar.cbWndExtra], 0
    mov rax, [hInst]
    mov [wcMiniBar.hInstance], rax
    mov [wcMiniBar.hIcon], 0
    invoke LoadCursorW, NULL, 32512    ; IDC_ARROW
    mov [wcMiniBar.hCursor], rax
    invoke CreateSolidBrush, COLOR_CARD_BG
    mov [hBrushMiniBar], rax
    mov [wcMiniBar.hbrBackground], rax
    mov [wcMiniBar.lpszMenuName], 0
    lea rax, [szMiniBarClass]
    mov [wcMiniBar.lpszClassName], rax
    mov [wcMiniBar.hIconSm], 0

    lea rcx, [wcMiniBar]
    call [RegisterClassExW]

    ; Position at top-right of primary display
    invoke GetSystemMetrics, 0 ; SM_CXSCREEN
    sub eax, 360
    mov ebx, eax ; x pos

    ; Create Frameless Draggable Topmost Popup (Parent is NULL so hiding hWndMain doesn't hide it)
    invoke CreateWindowExW, WS_EX_TOPMOST or WS_EX_TOOLWINDOW,\
           addr szMiniBarClass, addr szAppTitle,\
           WS_POPUP,\
           ebx, 30, 316, 44,\
           NULL, NULL, [hInst], NULL
    mov [hWndMiniBar], rax

    ; 1. Digital Monospace Timer Label
    invoke CreateWindowExW, 0, addr szStaticClass, addr szDefMiniTimer,\
           WS_CHILD or WS_VISIBLE or SS_LEFT,\
           32, 11, 88, 22, [hWndMiniBar], IDC_MINI_TIMER, [hInst], NULL
    mov [hMiniTimer], rax
    invoke SendMessageW, rax, WM_SETFONT, [hFontBold], 1

    ; 2. Control Buttons (BS_OWNERDRAW)
    ; Pause Button (x = 124, y = 6, w = 36, h = 32)
    invoke CreateWindowExW, 0, addr szButtonClass, addr szMiniBtnPause,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW,\
           124, 6, 36, 32, [hWndMiniBar], IDC_MINI_PAUSE, [hInst], NULL
    mov [hBtnMiniPause], rax

    ; Stop Button (x = 166, y = 6, w = 36, h = 32)
    invoke CreateWindowExW, 0, addr szButtonClass, addr szMiniBtnStop,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW,\
           166, 6, 36, 32, [hWndMiniBar], IDC_MINI_STOP, [hInst], NULL
    mov [hBtnMiniStop], rax

    ; Screenshot Button (x = 208, y = 6, w = 36, h = 32)
    invoke CreateWindowExW, 0, addr szButtonClass, addr szMiniBtnShot,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW,\
           208, 6, 36, 32, [hWndMiniBar], IDC_MINI_SHOT, [hInst], NULL
    mov [hBtnMiniShot], rax

    ; Restore Main Window Button (x = 264, y = 6, w = 36, h = 32)
    invoke CreateWindowExW, 0, addr szButtonClass, addr szMiniBtnRestore,\
           WS_CHILD or WS_VISIBLE or BS_OWNERDRAW,\
           264, 6, 36, 32, [hWndMiniBar], IDC_MINI_RESTORE, [hInst], NULL
    mov [hBtnMiniRestore], rax

    ret
endp


; ----------------------------------------------------------------------------
; ShowMiniBar - Shows floating toolbar and hides main recording panel
; ----------------------------------------------------------------------------
proc ShowMiniBar uses rbx rsi rdi
    cmp [showMinibar], 1
    jne .skip

    invoke ShowWindow, [hWndMiniBar], 4 ; SW_SHOWNOACTIVATE
    invoke UpdateWindow, [hWndMiniBar]

    ; Hide Main Window so it does not block the screen
    invoke ShowWindow, [hWndMain], 0 ; SW_HIDE

.skip:
    ret
endp


; ----------------------------------------------------------------------------
; HideMiniBar - Hides floating toolbar and restores main window
; ----------------------------------------------------------------------------
proc HideMiniBar uses rbx rsi rdi
    invoke ShowWindow, [hWndMiniBar], 0 ; SW_HIDE
    invoke ShowWindow, [hWndMain], 9 ; SW_RESTORE
    invoke SetForegroundWindow, [hWndMain]
    ret
endp


; ----------------------------------------------------------------------------
; UpdateMiniBarTimer - Updates digital timer on floating toolbar
; ----------------------------------------------------------------------------
proc UpdateMiniBarTimer uses rbx rsi rdi
    cmp [hWndMiniBar], 0
    je .done

    ; Update text with current timeBuf
    invoke SetWindowTextW, [hMiniTimer], addr timeBuf

.done:
    ret
endp


; ----------------------------------------------------------------------------
; MiniBarWndProc - Window Procedure for Floating Toolbar
; ----------------------------------------------------------------------------
proc MiniBarWndProc uses rbx rsi rdi, hwnd, wmsg, wparam, lparam
    mov [hwnd], rcx
    mov [wmsg], rdx
    mov [wparam], r8
    mov [lparam], r9

    mov rdx, [wmsg]

    cmp edx, WM_PAINT
    je .on_paint
    cmp edx, WM_LBUTTONDOWN
    je .on_lbuttondown
    cmp edx, WM_CTLCOLORSTATIC
    je .on_ctlcolorstatic
    cmp edx, WM_DRAWITEM
    je .on_drawitem
    cmp edx, WM_COMMAND
    je .on_command

    invoke DefWindowProcW, [hwnd], [wmsg], [wparam], [lparam]
    ret

; ----------------------------------------------------------------------------
; WM_PAINT - Paints Pill Shape & Pulsing Recording Indicator
; ----------------------------------------------------------------------------
.on_paint:
    invoke BeginPaint, [hwnd], addr ps
    mov rsi, rax ; rsi = hDC

    ; 1. Draw Rounded Dark Pill Background
    invoke SelectObject, rsi, [hBrushCard]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, 0, 0, 316, 44, 20, 20

    ; 2. Draw Recording Red Circle Indicator
    invoke CreateSolidBrush, COLOR_STATUS_RED
    push rax
    invoke SelectObject, rsi, rax
    invoke CreatePen, 0, 1, COLOR_STATUS_RED
    push rax
    invoke SelectObject, rsi, rax

    ; Red Circle at x = 12 to 24, y = 16 to 28
    invoke Ellipse, rsi, 12, 16, 24, 28

    pop rax
    invoke DeleteObject, rax
    pop rax
    invoke DeleteObject, rax

    invoke EndPaint, [hwnd], addr ps
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; WM_LBUTTONDOWN - Drag the MiniBar Anywhere on Screen
; ----------------------------------------------------------------------------
.on_lbuttondown:
    invoke ReleaseCapture
    invoke SendMessageW, [hwnd], WM_NCLBUTTONDOWN, 2, 0 ; HTCAPTION = 2
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; WM_CTLCOLORSTATIC - Transparent Background with Pure White Text
; ----------------------------------------------------------------------------
.on_ctlcolorstatic:
    mov rsi, [wparam]
    invoke SetBkMode, rsi, 1 ; TRANSPARENT
    invoke SetTextColor, rsi, 0x00FFFFFF ; Pure White
    mov rax, [hBrushCard]
    ret

; ----------------------------------------------------------------------------
; WM_DRAWITEM - Owner-Drawn Mini Action Buttons
; ----------------------------------------------------------------------------
.on_drawitem:
    mov rdi, [lparam]
    mov rsi, [rdi+32] ; hDC
    mov ebx, [rdi+4]  ; CtlID

    ; 1. Cleanly erase entire button rectangle with smooth dark card background
    lea rdx, [rdi+40]
    invoke FillRect, rsi, rdx, [hBrushCard]

    mov r12d, dword [rdi+40] ; left (0)
    mov r13d, dword [rdi+44] ; top (0)
    mov r14d, dword [rdi+48] ; right (36)
    mov r15d, dword [rdi+52] ; bottom (32)

    cmp ebx, IDC_MINI_PAUSE
    je .draw_mini_pause
    cmp ebx, IDC_MINI_STOP
    je .draw_mini_stop
    cmp ebx, IDC_MINI_SHOT
    je .draw_mini_shot
    cmp ebx, IDC_MINI_RESTORE
    je .draw_mini_restore
    mov eax, 1
    ret

.draw_mini_pause:
    cmp [recState], STATE_PAUSED
    je .pause_is_amber

    ; Normal recording: dark button with two crisp amber bars
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 8, 8

    invoke SelectObject, rsi, [hBrushAmber]
    invoke SelectObject, rsi, [hPenAmber]
    invoke RoundRect, rsi, 13, 10, 16, 22, 2, 2
    invoke RoundRect, rsi, 20, 10, 23, 22, 2, 2
    mov eax, 1
    ret

.pause_is_amber:
    ; Paused: dark button with emerald green play triangle
    invoke SelectObject, rsi, [hBrushBtnDarkHover]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 8, 8

    mov [ptResume1.x], 14
    mov [ptResume1.y], 9
    mov [ptResume2.x], 14
    mov [ptResume2.y], 23
    mov [ptResume3.x], 25
    mov [ptResume3.y], 16
    invoke SelectObject, rsi, [hBrushBtnGreen]
    invoke SelectObject, rsi, [hPenBtnGreen]
    invoke Polygon, rsi, addr ptResume1, 3
    mov eax, 1
    ret

.draw_mini_stop:
    ; Stop Button: Vibrant Crimson Red danger button (like Python ttkbootstrap)
    invoke SelectObject, rsi, [hBrushRed]
    invoke SelectObject, rsi, [hPenBtnRed]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 8, 8

    ; White filled stop square in center
    invoke SelectObject, rsi, [hBrushWhite]
    invoke SelectObject, rsi, [hPenWhite]
    invoke RoundRect, rsi, 13, 11, 23, 21, 3, 3
    mov eax, 1
    ret

.draw_mini_shot:
    ; Screenshot Button: Dark button with Cyan camera icon
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 8, 8

    ; Camera flash/bump
    invoke SelectObject, rsi, [hBrushCyan]
    invoke SelectObject, rsi, [hPenCyan]
    invoke RoundRect, rsi, 13, 9, 17, 12, 1, 1

    ; Camera body
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenCyan]
    invoke RoundRect, rsi, 10, 12, 26, 23, 3, 3

    ; Camera lens
    invoke Ellipse, rsi, 15, 14, 21, 20
    mov eax, 1
    ret

.draw_mini_restore:
    ; Restore Button: Dark button with clean maximize window glyph
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenBorderLight]
    invoke RoundRect, rsi, r12d, r13d, r14d, r15d, 8, 8

    ; Outer window frame
    invoke SelectObject, rsi, [hBrushBtnDark]
    invoke SelectObject, rsi, [hPenWhite]
    invoke RoundRect, rsi, 11, 10, 25, 22, 2, 2

    ; Title bar separator line
    invoke MoveToEx, rsi, 11, 13, NULL
    invoke LineTo, rsi, 25, 13
    mov eax, 1
    ret

; ----------------------------------------------------------------------------
; WM_COMMAND - MiniBar Actions Dispatcher
; ----------------------------------------------------------------------------
.on_command:
    mov rax, [wparam]
    and eax, 0xFFFF

    cmp eax, IDC_MINI_PAUSE
    je .cmd_mini_pause
    cmp eax, IDC_MINI_STOP
    je .cmd_mini_stop
    cmp eax, IDC_MINI_SHOT
    je .cmd_mini_shot
    cmp eax, IDC_MINI_RESTORE
    je .cmd_mini_restore
    xor eax, eax
    ret

.cmd_mini_pause:
    call PauseScreenRecording
    invoke InvalidateRect, [hBtnMiniPause], NULL, TRUE
    xor eax, eax
    ret

.cmd_mini_stop:
    call StopScreenRecording
    call HideMiniBar
    xor eax, eax
    ret

.cmd_mini_shot:
    call TakeInstantScreenshot
    xor eax, eax
    ret

.cmd_mini_restore:
    call HideMiniBar
    xor eax, eax
    ret
endp

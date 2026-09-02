; ============================================================================
; ScreenCapture Pro x64 - Wide Unicode Interactive Region Selector
; Ultra-smooth freeze-screen sniper overlay (Double-Buffered, 0% Flicker, SSE2)
; ============================================================================

; ----------------------------------------------------------------------------
; UpdateVirtualScreenGeometry - Queries multi-monitor virtual screen bounds
; ----------------------------------------------------------------------------
proc UpdateVirtualScreenGeometry uses rbx
    invoke GetSystemMetrics, 76 ; SM_XVIRTUALSCREEN
    mov [vScreenX], eax
    invoke GetSystemMetrics, 77 ; SM_YVIRTUALSCREEN
    mov [vScreenY], eax
    invoke GetSystemMetrics, 78 ; SM_CXVIRTUALSCREEN
    mov [vScreenW], eax
    invoke GetSystemMetrics, 79 ; SM_CYVIRTUALSCREEN
    mov [vScreenH], eax

    cmp [vScreenW], 0
    jg .done
    ; Fallback to single primary monitor metrics
    mov [vScreenX], 0
    mov [vScreenY], 0
    invoke GetSystemMetrics, 0 ; SM_CXSCREEN
    mov [vScreenW], eax
    invoke GetSystemMetrics, 1 ; SM_CYSCREEN
    mov [vScreenH], eax

.done:
    ret
endp


; ----------------------------------------------------------------------------
; CaptureFullScreenBitmap - Captures 100% clean freeze-frame of desktop
; ----------------------------------------------------------------------------
proc CaptureFullScreenBitmap uses rbx rsi rdi
    local hScreenDC:QWORD
    local hMemDC:QWORD
    local hBitmap:QWORD
    local hOldBmp:QWORD

    call UpdateVirtualScreenGeometry

    invoke GetDC, 0
    mov [hScreenDC], rax
    test rax, rax
    jz .fail

    invoke CreateCompatibleDC, [hScreenDC]
    mov [hMemDC], rax
    test rax, rax
    jz .cleanup_screen

    invoke CreateCompatibleBitmap, [hScreenDC], [vScreenW], [vScreenH]
    mov [hBitmap], rax
    test rax, rax
    jz .cleanup_mem

    invoke SelectObject, [hMemDC], [hBitmap]
    mov [hOldBmp], rax

    ; Blit screen contents
    invoke BitBlt, [hMemDC], 0, 0, [vScreenW], [vScreenH], [hScreenDC], [vScreenX], [vScreenY], 0x00CC0020 ; SRCCOPY

    invoke SelectObject, [hMemDC], [hOldBmp]
    invoke DeleteDC, [hMemDC]
    invoke ReleaseDC, 0, [hScreenDC]

    mov rax, [hBitmap]
    ret

.cleanup_mem:
    invoke DeleteDC, [hMemDC]
.cleanup_screen:
    invoke ReleaseDC, 0, [hScreenDC]
.fail:
    xor eax, eax
    ret
endp


; ----------------------------------------------------------------------------
; CreateDimmedDesktopBitmap - Pre-dims desktop using high-performance SSE2
; ----------------------------------------------------------------------------
proc CreateDimmedDesktopBitmap uses rbx rsi rdi, hSourceBmp:QWORD
    local hScreenDC:QWORD
    local hSrcDC:QWORD
    local hDstDC:QWORD
    local hDimmedBmp:QWORD
    local hOldSrc:QWORD
    local hOldDst:QWORD
    local pBits:QWORD
    local numBytes:QWORD
    local bi:BITMAPINFOHEADER

    mov [hSourceBmp], rcx

    ; Prepare 32-bit BITMAPINFO
    mov [bi.biSize], sizeof.BITMAPINFOHEADER
    mov eax, [vScreenW]
    mov [bi.biWidth], eax
    mov eax, [vScreenH]
    neg eax                               ; Top-down DIB
    mov [bi.biHeight], eax
    mov [bi.biPlanes], 1
    mov [bi.biBitCount], 32
    mov [bi.biCompression], 0             ; BI_RGB
    mov [bi.biSizeImage], 0
    mov [bi.biXPelsPerMeter], 0
    mov [bi.biYPelsPerMeter], 0
    mov [bi.biClrUsed], 0
    mov [bi.biClrImportant], 0

    ; Total bytes = vScreenW * vScreenH * 4
    mov eax, [vScreenW]
    mul [vScreenH]
    shl rax, 2
    mov [numBytes], rax

    invoke VirtualAlloc, 0, [numBytes], 0x1000 or 0x2000, 4 ; MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE
    mov [pBits], rax
    test rax, rax
    jz .fail

    invoke GetDC, 0
    mov [hScreenDC], rax

    invoke CreateCompatibleDC, [hScreenDC]
    mov [hSrcDC], rax
    invoke SelectObject, [hSrcDC], [hSourceBmp]
    mov [hOldSrc], rax

    ; Read pixels into pBits
    invoke GetDIBits, [hSrcDC], [hSourceBmp], 0, [vScreenH], [pBits], addr bi, 0

    ; Dim every 32-bit pixel using SSE2 (Independent RGB channels, 0% color bleed)
    ; Multiplier: 115 (115 / 256 = ~45.0% brightness)
    mov rdi, [pBits]
    mov rcx, [numBytes]
    shr rcx, 4                            ; number of 16-byte (4-pixel) SIMD blocks

    pxor xmm7, xmm7                       ; zero register for unpacking
    mov eax, 00730073h                    ; 115 in 16-bit words (0x0073 = 115)
    movd xmm6, eax
    pshufd xmm6, xmm6, 0                  ; xmm6 = [115, 115, 115, 115, 115, 115, 115, 115]

.dim_simd_loop:
    movdqu xmm0, [rdi]                    ; load 4 pixels (16 bytes)
    movdqa xmm1, xmm0
    punpcklbw xmm0, xmm7                  ; unpack lower 2 pixels to 16-bit words
    punpckhbw xmm1, xmm7                  ; unpack upper 2 pixels to 16-bit words

    pmullw xmm0, xmm6                     ; multiply each channel independently by 115
    pmullw xmm1, xmm6

    psrlw xmm0, 8                         ; divide by 256
    psrlw xmm1, 8

    packuswb xmm0, xmm1                   ; pack back to 8-bit unsigned saturated bytes
    movdqu [rdi], xmm0                    ; write back 4 dimmed pixels

    add rdi, 16
    dec rcx
    jnz .dim_simd_loop

    ; Create compatible bitmap and copy dimmed pixels into it
    invoke CreateCompatibleBitmap, [hScreenDC], [vScreenW], [vScreenH]
    mov [hDimmedBmp], rax

    invoke CreateCompatibleDC, [hScreenDC]
    mov [hDstDC], rax
    invoke SelectObject, [hDstDC], [hDimmedBmp]
    mov [hOldDst], rax

    invoke SetDIBits, [hDstDC], [hDimmedBmp], 0, [vScreenH], [pBits], addr bi, 0

    invoke SelectObject, [hDstDC], [hOldDst]
    invoke DeleteDC, [hDstDC]

    invoke SelectObject, [hSrcDC], [hOldSrc]
    invoke DeleteDC, [hSrcDC]
    invoke ReleaseDC, 0, [hScreenDC]

    invoke VirtualFree, [pBits], 0, 0x8000 ; MEM_RELEASE

    mov rax, [hDimmedBmp]
    ret

.fail:
    xor eax, eax
    ret
endp


; ----------------------------------------------------------------------------
; CleanupOverlayResources - Frees overlay DCs and Bitmaps safely
; ----------------------------------------------------------------------------
proc CleanupOverlayResources uses rbx
    cmp [hOverlayDimDC], 0
    je @f
    invoke SelectObject, [hOverlayDimDC], [hOverlayOldDim]
    invoke DeleteDC, [hOverlayDimDC]
    mov [hOverlayDimDC], 0
@@:
    cmp [hOverlayBrightDC], 0
    je @f
    invoke SelectObject, [hOverlayBrightDC], [hOverlayOldBright]
    invoke DeleteDC, [hOverlayBrightDC]
    mov [hOverlayBrightDC], 0
@@:
    cmp [hOverlayMemDC], 0
    je @f
    invoke SelectObject, [hOverlayMemDC], [hOverlayOldBuf]
    invoke DeleteObject, [hOverlayBufferBmp]
    invoke DeleteDC, [hOverlayMemDC]
    mov [hOverlayMemDC], 0
    mov [hOverlayBufferBmp], 0
@@:
    cmp [hFrozenBitmap], 0
    je @f
    invoke DeleteObject, [hFrozenBitmap]
    mov [hFrozenBitmap], 0
@@:
    cmp [hDimmedBitmap], 0
    je @f
    invoke DeleteObject, [hDimmedBitmap]
    mov [hDimmedBitmap], 0
@@:
    ret
endp


; ----------------------------------------------------------------------------
; ShowRegionSelector - Initializes freeze-screen overlay
; ----------------------------------------------------------------------------
proc ShowRegionSelector uses rbx rsi rdi
    local hScreenDC:QWORD

    ; 1. Smoothly hide main window
    invoke ShowWindow, [hWndMain], 0 ; SW_HIDE
    invoke Sleep, 120

    ; 2. Freeze entire virtual screen to memory bitmap (100% clean desktop!)
    call CaptureFullScreenBitmap
    mov [hFrozenBitmap], rax
    test rax, rax
    jz .fail_restore

    ; 3. Create pre-dimmed copy of desktop bitmap (45% brightness)
    fastcall CreateDimmedDesktopBitmap, [hFrozenBitmap]
    mov [hDimmedBitmap], rax

    ; 4. Prepare Persistent High-Performance Rendering DCs (0ms Latency)
    invoke GetDC, 0
    mov [hScreenDC], rax

    invoke CreateCompatibleDC, [hScreenDC]
    mov [hOverlayMemDC], rax
    invoke CreateCompatibleBitmap, [hScreenDC], [vScreenW], [vScreenH]
    mov [hOverlayBufferBmp], rax
    invoke SelectObject, [hOverlayMemDC], [hOverlayBufferBmp]
    mov [hOverlayOldBuf], rax

    invoke CreateCompatibleDC, [hScreenDC]
    mov [hOverlayDimDC], rax
    invoke SelectObject, [hOverlayDimDC], [hDimmedBitmap]
    mov [hOverlayOldDim], rax

    invoke CreateCompatibleDC, [hScreenDC]
    mov [hOverlayBrightDC], rax
    invoke SelectObject, [hOverlayBrightDC], [hFrozenBitmap]
    mov [hOverlayOldBright], rax

    invoke ReleaseDC, 0, [hScreenDC]

    ; 5. Register overlay window class
    mov [wcOverlay.cbSize], sizeof.WNDCLASSEX
    mov [wcOverlay.style], 0
    lea rax, [RegionOverlayWndProc]
    mov [wcOverlay.lpfnWndProc], rax
    mov [wcOverlay.cbClsExtra], 0
    mov [wcOverlay.cbWndExtra], 0
    mov rax, [hInst]
    mov [wcOverlay.hInstance], rax
    mov [wcOverlay.hIcon], 0
    invoke LoadCursorW, 0, 32515 ; IDC_CROSS
    mov [wcOverlay.hCursor], rax
    mov [wcOverlay.hbrBackground], 0
    mov [wcOverlay.lpszMenuName], 0
    lea rax, [szOverlayClass]
    mov [wcOverlay.lpszClassName], rax
    lea rcx, [wcOverlay]
    call [RegisterClassExW]

    ; 6. Create Full-Screen Popup Window
    invoke CreateWindowExW, 0x00000008 or 0x00000080, addr szOverlayClass, NULL,\
           0x90000000, [vScreenX], [vScreenY], [vScreenW], [vScreenH],\
           NULL, NULL, [hInst], NULL
    mov [hWndOverlay], rax
    test rax, rax
    jz .cleanup_bmp

    ; 7. Focus & Show Overlay Window
    mov [isOverlayDragging], 0
    invoke SetForegroundWindow, [hWndOverlay]
    invoke SetFocus, [hWndOverlay]
    invoke ShowWindow, [hWndOverlay], 5 ; SW_SHOW
    invoke UpdateWindow, [hWndOverlay]
    mov eax, 1
    ret

.cleanup_bmp:
    call CleanupOverlayResources
.fail_restore:
    invoke ShowWindow, [hWndMain], 1 ; SW_SHOWNORMAL
    invoke SetForegroundWindow, [hWndMain]
    xor eax, eax
    ret
endp


; ----------------------------------------------------------------------------
; RegionOverlayWndProc - Overlay Window Procedure
; ----------------------------------------------------------------------------
proc RegionOverlayWndProc uses rbx rsi rdi r12 r13 r14 r15, hwnd, wmsg, wparam, lparam
    local hDC:QWORD
    local leftX:DWORD
    local topY:DWORD
    local rightX:DWORD
    local bottomY:DWORD
    local rectW:DWORD
    local rectH:DWORD
    local hPenBorder:QWORD
    local hOldPen:QWORD
    local hOldBrush:QWORD
    local badgeX:DWORD
    local badgeY:DWORD
    local badgeRight:DWORD
    local badgeBottom:DWORD
    local badgeTextX:DWORD
    local badgeTextY:DWORD
    local textLen:DWORD

    mov [hwnd], rcx
    mov [wmsg], rdx
    mov [wparam], r8
    mov [lparam], r9

    cmp edx, WM_ERASEBKGND
    je .on_erasebkgnd
    cmp edx, WM_SETCURSOR
    je .on_setcursor
    cmp edx, WM_LBUTTONDOWN
    je .on_lbutton_down
    cmp edx, WM_MOUSEMOVE
    je .on_mouse_move
    cmp edx, WM_LBUTTONUP
    je .on_lbutton_up
    cmp edx, WM_RBUTTONUP
    je .on_cancel
    cmp edx, WM_KEYDOWN
    je .on_keydown
    cmp edx, WM_PAINT
    je .on_paint
    cmp edx, WM_DESTROY
    je .on_destroy

    invoke DefWindowProcW, [hwnd], [wmsg], [wparam], [lparam]
    ret

.on_erasebkgnd:
    mov eax, 1
    ret

.on_setcursor:
    invoke LoadCursorW, 0, 32515 ; IDC_CROSS
    invoke SetCursor, rax
    mov eax, 1
    ret

; ----------------------------------------------------------------------------
; Left Mouse Down: Begin sniper drag
; ----------------------------------------------------------------------------
.on_lbutton_down:
    mov [isOverlayDragging], 1
    mov rax, [lparam]
    movsx ecx, ax                         ; mouse X (LOWORD)
    sar eax, 16                           ; mouse Y (HIWORD)
    mov [dragStartX], ecx
    mov [dragStartY], eax
    mov [dragCurrentX], ecx
    mov [dragCurrentY], eax

    invoke SetCapture, [hwnd]
    invoke InvalidateRect, [hwnd], NULL, FALSE
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; Mouse Move: Update selection rectangle
; ----------------------------------------------------------------------------
.on_mouse_move:
    mov rax, [lparam]
    movsx ecx, ax
    sar eax, 16
    mov [hoverX], ecx
    mov [hoverY], eax

    cmp [isOverlayDragging], 1
    jne .hover_only

    mov [dragCurrentX], ecx
    mov [dragCurrentY], eax
    invoke InvalidateRect, [hwnd], NULL, FALSE
    xor eax, eax
    ret

.hover_only:
    invoke InvalidateRect, [hwnd], NULL, FALSE
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; Left Mouse Up: Finalize selection
; ----------------------------------------------------------------------------
.on_lbutton_up:
    cmp [isOverlayDragging], 1
    jne .lup_done
    mov [isOverlayDragging], 0
    invoke ReleaseCapture

    ; Calculate normalized bounds (min X, min Y, width, height)
    mov eax, [dragStartX]
    mov ecx, [dragCurrentX]
    cmp eax, ecx
    jle @f
    xchg eax, ecx
@@:
    mov [leftX], eax
    sub ecx, eax
    mov [rectW], ecx

    mov eax, [dragStartY]
    mov ecx, [dragCurrentY]
    cmp eax, ecx
    jle @f
    xchg eax, ecx
@@:
    mov [topY], eax
    sub ecx, eax
    mov [rectH], ecx

    ; Minimum valid size: at least 16x16 pixels
    cmp [rectW], 16
    jl .on_cancel
    cmp [rectH], 16
    jl .on_cancel

    ; Make width and height even numbers for video encoders
    mov eax, [rectW]
    and eax, not 1
    mov [capW], eax

    mov eax, [rectH]
    and eax, not 1
    mov [capH], eax

    ; Store global screen coordinates (offset by virtual screen origin)
    mov eax, [leftX]
    add eax, [vScreenX]
    test eax, eax
    jns @f
    xor eax, eax
@@:
    mov [capX], eax

    mov eax, [topY]
    add eax, [vScreenY]
    test eax, eax
    jns @f
    xor eax, eax
@@:
    mov [capY], eax

    mov [isCustomRegion], 1

    ; Format summary string: W×H
    movsxd r8, [capW]
    movsxd r9, [capH]
    invoke wsprintfW, addr targetDescBuf, addr fmtRegionSummary, r8, r9
    invoke SetWindowTextW, [hLblRegionInfo], addr targetDescBuf

    ; Close overlay and restore main window
    invoke DestroyWindow, [hwnd]
    xor eax, eax
    ret

.lup_done:
    xor eax, eax
    ret

; ----------------------------------------------------------------------------
; ESC or Right-click: Cancel region selection
; ----------------------------------------------------------------------------
.on_keydown:
    cmp r8d, VK_ESCAPE
    jne .kd_done
.on_cancel:
    cmp [isOverlayDragging], 1
    jne @f
    invoke ReleaseCapture
    mov [isOverlayDragging], 0
@@:
    invoke DestroyWindow, [hwnd]
.kd_done:
    xor eax, eax
    ret

.on_destroy:
    mov [hWndOverlay], 0
    call CleanupOverlayResources
    invoke ShowWindow, [hWndMain], 1 ; SW_SHOWNORMAL
    invoke SetForegroundWindow, [hWndMain]
    invoke InvalidateRect, [hWndMain], NULL, TRUE
    xor eax, eax
    ret

; ============================================================================
; WM_PAINT - Zero-Flicker High-Speed Double-Buffered Compositor
; ============================================================================
.on_paint:
    invoke BeginPaint, [hwnd], addr ps
    mov [hDC], rax

    ; 1. Instant Blit of full pre-dimmed desktop to backbuffer
    invoke BitBlt, [hOverlayMemDC], 0, 0, [vScreenW], [vScreenH], [hOverlayDimDC], 0, 0, 0x00CC0020 ; SRCCOPY

    ; 2. If dragging, draw 100% bright crystal-clear cutout from original desktop
    cmp [isOverlayDragging], 1
    jne .draw_hover_crosshairs

    ; Calculate normalized drag bounds
    mov eax, [dragStartX]
    mov ecx, [dragCurrentX]
    cmp eax, ecx
    jle @f
    xchg eax, ecx
@@:
    mov [leftX], eax
    sub ecx, eax
    mov [rectW], ecx

    mov eax, [dragStartY]
    mov ecx, [dragCurrentY]
    cmp eax, ecx
    jle @f
    xchg eax, ecx
@@:
    mov [topY], eax
    sub ecx, eax
    mov [rectH], ecx

    ; Blit bright original desktop cutout
    invoke BitBlt, [hOverlayMemDC], [leftX], [topY], [rectW], [rectH], [hOverlayBrightDC], [leftX], [topY], 0x00CC0020 ; SRCCOPY

    ; Calculate right and bottom
    mov eax, [leftX]
    add eax, [rectW]
    mov [rightX], eax

    mov eax, [topY]
    add eax, [rectH]
    mov [bottomY], eax

    ; Draw glowing Neon Cyan 2px border around selection
    invoke CreatePen, 0, 2, 0x00E6B400 ; Sky Cyan (#00B4E6)
    mov [hPenBorder], rax
    invoke SelectObject, [hOverlayMemDC], [hPenBorder]
    mov [hOldPen], rax
    invoke GetStockObject, 5 ; NULL_BRUSH
    invoke SelectObject, [hOverlayMemDC], rax
    mov [hOldBrush], rax

    invoke Rectangle, [hOverlayMemDC], [leftX], [topY], [rightX], [bottomY]

    invoke SelectObject, [hOverlayMemDC], [hOldPen]
    invoke SelectObject, [hOverlayMemDC], [hOldBrush]
    invoke DeleteObject, [hPenBorder]

    ; Draw floating dimension badge: "1280×720"
    movsxd r8, [rectW]
    movsxd r9, [rectH]
    invoke wsprintfW, addr szBadgeDimBuf, addr fmtRegionSummary, r8, r9

    ; Position badge below selection or above if near bottom edge
    mov eax, [leftX]
    mov [badgeX], eax

    mov eax, [bottomY]
    add eax, 6
    mov [badgeY], eax
    add eax, 24
    cmp eax, [vScreenH]
    jle @f
    mov eax, [topY]
    sub eax, 28
    mov [badgeY], eax
@@:
    mov eax, [badgeX]
    add eax, 100
    mov [badgeRight], eax

    mov eax, [badgeY]
    add eax, 22
    mov [badgeBottom], eax

    ; Draw Dark Pill Badge Background
    invoke SelectObject, [hOverlayMemDC], [hBrushCard]
    invoke SelectObject, [hOverlayMemDC], [hPenBorder]
    invoke RoundRect, [hOverlayMemDC], [badgeX], [badgeY], [badgeRight], [badgeBottom], 6, 6

    ; Draw dimension text inside badge
    lea rax, [szBadgeDimBuf]
    invoke lstrlenW, rax
    mov [textLen], eax

    mov eax, [badgeX]
    add eax, 10
    mov [badgeTextX], eax

    mov eax, [badgeY]
    add eax, 3
    mov [badgeTextY], eax

    invoke SetBkMode, [hOverlayMemDC], 1 ; TRANSPARENT
    invoke SetTextColor, [hOverlayMemDC], 0x00FFFFFF ; pure white
    invoke SelectObject, [hOverlayMemDC], [hFontBold]
    invoke TextOutW, [hOverlayMemDC], [badgeTextX], [badgeTextY], addr szBadgeDimBuf, [textLen]
    jmp .present_screen

.draw_hover_crosshairs:
    ; Full-screen subtle guide lines at cursor position
    invoke CreatePen, 0, 1, 0x00504030 ; subtle dark slate
    mov [hPenBorder], rax
    invoke SelectObject, [hOverlayMemDC], [hPenBorder]
    mov [hOldPen], rax

    invoke MoveToEx, [hOverlayMemDC], 0, [hoverY], NULL
    invoke LineTo, [hOverlayMemDC], [vScreenW], [hoverY]
    invoke MoveToEx, [hOverlayMemDC], [hoverX], 0, NULL
    invoke LineTo, [hOverlayMemDC], [hoverX], [vScreenH]

    invoke SelectObject, [hOverlayMemDC], [hOldPen]
    invoke DeleteObject, [hPenBorder]

.present_screen:
    ; 3. Atomic 1-call flip to screen (Zero flicker)
    invoke BitBlt, [hDC], 0, 0, [vScreenW], [vScreenH], [hOverlayMemDC], 0, 0, 0x00CC0020 ; SRCCOPY

    invoke EndPaint, [hwnd], addr ps
    xor eax, eax
    ret
endp

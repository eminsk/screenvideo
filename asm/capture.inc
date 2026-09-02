; ============================================================================
; ScreenCapture Pro x64 - Wide Unicode Capture and Direct Video Recording Engine
; ============================================================================

; ----------------------------------------------------------------------------
; SaveBitmapToFile - Writes 24-bit uncompressed BMP to disk
; ----------------------------------------------------------------------------
proc SaveBitmapToFile uses rbx rsi rdi
    ; Calculate 4-byte aligned stride: ((nWidth * 3 + 3) & ~3)
    mov eax, [shot_cW]
    lea eax, [eax*2 + eax]
    add eax, 3
    and eax, not 3
    mov [sb_stride], eax

    ; imageSize = stride * nHeight
    mov ecx, [shot_cH]
    imul ecx
    mov [sb_imageSize], eax

    ; Setup BITMAPINFOHEADER (40 bytes)
    lea rdi, [sb_bih]
    xor eax, eax
    mov ecx, 10
    rep stosd

    lea rdi, [sb_bih]
    mov dword [rdi], 40                ; biSize
    mov eax, [shot_cW]
    mov dword [rdi+4], eax             ; biWidth
    mov eax, [shot_cH]
    mov dword [rdi+8], eax             ; biHeight
    mov word [rdi+12], 1               ; biPlanes
    mov word [rdi+14], 24              ; biBitCount (24-bit TrueColor)
    mov dword [rdi+16], BI_RGB         ; biCompression = 0
    mov eax, [sb_imageSize]
    mov dword [rdi+20], eax            ; biSizeImage

    ; Setup BITMAPFILEHEADER (14 bytes)
    lea rdi, [sb_bfh]
    mov word [rdi], 0x4D42             ; bfType = 'BM'
    mov eax, [sb_imageSize]
    add eax, 54                        ; 14 + 40
    mov dword [rdi+2], eax             ; bfSize
    mov word [rdi+6], 0                ; bfReserved1
    mov word [rdi+8], 0                ; bfReserved2
    mov dword [rdi+10], 54             ; bfOffBits = 54

    ; Allocate pixel memory
    invoke VirtualAlloc, NULL, [sb_imageSize], 0x1000 or 0x2000, 4
    test rax, rax
    jz .fail
    mov [sb_pBits], rax

    ; Extract raw DIB bits from HBITMAP
    invoke GetDIBits, [shot_hMemDC], [shot_hBmp], 0, [shot_cH], [sb_pBits], addr sb_bih, DIB_RGB_COLORS

    ; Create Output File (Wide API)
    invoke CreateFileW, addr szCurrentShotFile, GENERIC_WRITE, 0, NULL,\
           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    cmp rax, -1
    je .free_mem
    mov [sb_hFile], rax

    ; Write BITMAPFILEHEADER (14 bytes)
    invoke WriteFile, [sb_hFile], addr sb_bfh, 14, addr sb_bytesWritten, NULL

    ; Write BITMAPINFOHEADER (40 bytes)
    invoke WriteFile, [sb_hFile], addr sb_bih, 40, addr sb_bytesWritten, NULL

    ; Write Pixel Data (imageSize bytes)
    invoke WriteFile, [sb_hFile], [sb_pBits], [sb_imageSize], addr sb_bytesWritten, NULL

    invoke CloseHandle, [sb_hFile]

.free_mem:
    invoke VirtualFree, [sb_pBits], 0, 0x8000 ; MEM_RELEASE
    mov eax, 1
    ret

.fail:
    xor eax, eax
    ret
endp


; ----------------------------------------------------------------------------
; TakeInstantScreenshot - High-speed instant screenshot capture
; ----------------------------------------------------------------------------
proc TakeInstantScreenshot uses rbx rsi rdi
    ; Determine Capture Bounds
    cmp [isCustomRegion], 1
    je .use_custom_reg

    ; Full screen
    mov [shot_cX], 0
    mov [shot_cY], 0
    invoke GetSystemMetrics, 0 ; SM_CXSCREEN
    mov [shot_cW], eax
    invoke GetSystemMetrics, 1 ; SM_CYSCREEN
    mov [shot_cH], eax
    jmp .got_bounds

.use_custom_reg:
    mov eax, [capX]
    mov [shot_cX], eax
    mov eax, [capY]
    mov [shot_cY], eax
    mov eax, [capW]
    mov [shot_cW], eax
    mov eax, [capH]
    mov [shot_cH], eax

.got_bounds:
    ; Format Unique Screenshot Filename with Timestamp (Wide)
    invoke GetLocalTime, addr stLocal
    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    movzx r15, [stLocal.wSecond]
    invoke wsprintfW, addr szCurrentShotFile, addr fmtShotFilename, r10, r11, r12, r13, r14, r15

    ; Create Screen Capture DC & Bitmap
    invoke GetWindowDC, 0
    mov [shot_hScreenDC], rax
    invoke CreateCompatibleDC, [shot_hScreenDC]
    mov [shot_hMemDC], rax
    invoke CreateCompatibleBitmap, [shot_hScreenDC], [shot_cW], [shot_cH]
    mov [shot_hBmp], rax
    invoke SelectObject, [shot_hMemDC], [shot_hBmp]
    mov [shot_hOldBmp], rax

    ; BitBlt Screen to Memory DC (with layered window support)
    invoke BitBlt, [shot_hMemDC], 0, 0, [shot_cW], [shot_cH],\
           [shot_hScreenDC], [shot_cX], [shot_cY], SRCCOPY or CAPTUREBLT
    invoke GdiFlush

    ; Cursor Capture (if enabled)
    cmp [includeCursor], 1
    jne .skip_cursor

    lea rdi, [shot_ci]
    mov dword [rdi], 24 ; cbSize
    invoke GetCursorInfo, addr shot_ci
    test eax, eax
    jz .skip_cursor

    lea rdi, [shot_ci]
    test dword [rdi+4], CURSOR_SHOWING
    jz .skip_cursor

    ; Compute relative cursor position
    mov eax, dword [rdi+16]            ; ptScreenPos.x
    sub eax, [shot_cX]
    mov ebx, eax                       ; cx
    mov eax, dword [rdi+20]            ; ptScreenPos.y
    sub eax, [shot_cY]
    mov esi, eax                       ; cy

    invoke DrawIconEx, [shot_hMemDC], ebx, esi, qword [rdi+8], 0, 0, 0, NULL, DI_NORMAL

.skip_cursor:
    ; Save to BMP File
    call SaveBitmapToFile

    ; Clean up GDI handles
    invoke SelectObject, [shot_hMemDC], [shot_hOldBmp]
    invoke DeleteObject, [shot_hBmp]
    invoke DeleteDC, [shot_hMemDC]
    invoke ReleaseDC, 0, [shot_hScreenDC]

    ; Sound feedback
    invoke MessageBeep, 0x00000040 ; MB_ICONASTERISK

    ; Update Status Bar
    invoke wsprintfW, addr statusMsgBuf, addr szStatusShotSavedMsg, addr szCurrentShotFile
    invoke SetWindowTextW, [hStatusBar], addr statusMsgBuf

    ; Auto-refresh gallery if on gallery tab
    cmp [currentTab], TAB_GALLERY
    jne .done
    call RefreshGalleryList

.done:
    ret
endp


; ----------------------------------------------------------------------------
; StartScreenRecording - Prepares state and launches background recording thread
; ----------------------------------------------------------------------------
proc StartScreenRecording uses rbx rsi rdi
    cmp [recState], STATE_IDLE
    jne .already_running

    ; Determine Capture Bounds
    cmp [isCustomRegion], 1
    je .use_custom_reg

    mov [capX], 0
    mov [capY], 0
    invoke GetSystemMetrics, 0 ; SM_CXSCREEN
    mov [capW], eax
    invoke GetSystemMetrics, 1 ; SM_CYSCREEN
    mov [capH], eax
    jmp .align_dims

.use_custom_reg:
    ; Use capX, capY, capW, capH set by region selector

.align_dims:
    and [capW], not 3                  ; multiple of 4
    and [capH], not 1                  ; multiple of 2

    ; Format Video Output Filename with Timestamp (Wide)
    invoke GetLocalTime, addr stLocal
    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    movzx r15, [stLocal.wSecond]
    invoke wsprintfW, addr szCurrentRecFile, addr fmtRecFilename, r10, r11, r12, r13, r14, r15

    ; Initialize Recording Telemetry
    mov [recFrameCount], 0
    mov [recBytesWritten], 0
    mov [recTotalPausedTime], 0
    mov [recThreadPause], 0
    mov [recThreadRunning], 1
    mov [recState], STATE_RECORDING

    ; Sound feedback
    invoke MessageBeep, 0x00000040

    ; Update UI Controls
    call UpdateUIState
    invoke SetWindowTextW, [hStatusBar], addr szStatusRecordingMsg

    ; Launch High-Performance Recording Thread
    invoke CreateThread, NULL, 0, RecordingThreadProc, NULL, 0, NULL
    mov [hRecThread], rax

    ; Check and Launch Audio Capture if enabled
    mov [audioCaptureActive], 0
    cmp [audioSysEnabled], 1
    je .launch_audio
    cmp [audioMicEnabled], 1
    jne .no_audio_launch

.launch_audio:
    ; Reset Audio Stop Event
    invoke CreateEventA, NULL, TRUE, FALSE, addr szAudioStopEvent
    push rax
    invoke ResetEvent, rax
    pop rax
    invoke CloseHandle, rax

    invoke DeleteFileW, addr szAudioTempWav

    lea rdi, [siAudio]
    xor eax, eax
    mov ecx, sizeof.STARTUPINFO / 4
    rep stosd
    mov [siAudio.cb], sizeof.STARTUPINFO
    mov [siAudio.dwFlags], 1 ; STARTF_USESHOWWINDOW
    mov [siAudio.wShowWindow], 0 ; SW_HIDE

    invoke CreateProcessW, NULL, addr szAudioCapExe, NULL, NULL, FALSE,\
           0x08000000, NULL, NULL, addr siAudio, addr piAudio
    test eax, eax
    jz .no_audio_launch
    mov [audioCaptureActive], 1

.no_audio_launch:
    ; Show floating toolbar and hide main window
    call ShowMiniBar

.already_running:
    ret
endp


; ----------------------------------------------------------------------------
; RecordingThreadProc - Background Real-Time Screen Capture Loop
; ----------------------------------------------------------------------------
proc RecordingThreadProc uses rbx rsi rdi
    ; Calculate 32-bit stride and image size (always 4-byte aligned)
    mov eax, [capW]
    shl eax, 2
    mov [rec_stride], eax

    mov ecx, [capH]
    imul ecx
    mov [rec_imageSize], eax

    ; 1. Initialize Video for Windows
    invoke AVIFileInit

    ; 2. Create AVI Output File (Wide API)
    invoke AVIFileOpenW, addr rec_pAviFile, addr szCurrentRecFile, OF_CREATE or OF_WRITE, NULL
    test eax, eax
    jnz .exit_thread

    ; 3. Setup Stream Header (AVISTREAMINFOW)
    lea rdi, [rec_siStream]
    xor eax, eax
    mov ecx, 64 ; 256 / 4
    rep stosd

    lea rdi, [rec_siStream]
    mov dword [rdi], FOURCC_VIDS       ; fccType = 'vids'
    mov dword [rdi+4], 0               ; fccHandler = 0 (DIB)
    mov dword [rdi+20], 1              ; dwScale = 1
    mov eax, [targetFps]
    test eax, eax
    jnz .has_fps
    mov eax, 30
.has_fps:
    mov dword [rdi+24], eax            ; dwRate = targetFps
    mov eax, [capW]
    mov dword [rdi+60], eax            ; rcFrame.right
    mov eax, [capH]
    mov dword [rdi+64], eax            ; rcFrame.bottom

    ; Copy Stream Name (Wide Unicode)
    lea rsi, [szAppTitle]
    lea rdi, [rec_siStream+76]
    mov ecx, 34
    rep movsw

    ; Create Stream in File (Wide API)
    invoke AVIFileCreateStreamW, [rec_pAviFile], addr rec_pAviStream, addr rec_siStream
    test eax, eax
    jnz .close_avi_file

    ; 4. Setup BITMAPINFOHEADER (32-bit TrueColor Top-Down DIB)
    lea rdi, [rec_bih]
    xor eax, eax
    mov ecx, 10
    rep stosd

    lea rdi, [rec_bih]
    mov dword [rdi], 40                ; biSize
    mov eax, [capW]
    mov dword [rdi+4], eax             ; biWidth
    mov eax, [capH]
    neg eax                            ; biHeight = -capH (top-down DIB)
    mov dword [rdi+8], eax
    mov word [rdi+12], 1               ; biPlanes
    mov word [rdi+14], 32              ; biBitCount = 32 (BGRA/BGR0)
    mov dword [rdi+16], BI_RGB
    mov eax, [rec_imageSize]
    mov dword [rdi+20], eax            ; biSizeImage

    ; Set Stream Format
    invoke AVIStreamSetFormat, [rec_pAviStream], 0, addr rec_bih, 40
    test eax, eax
    jnz .release_stream

    ; 5. Create Capture Device Context & Direct DIB Section
    invoke GetWindowDC, 0
    mov [rec_hScreenDC], rax
    invoke CreateCompatibleDC, [rec_hScreenDC]
    mov [rec_hMemDC], rax

    invoke CreateDIBSection, [rec_hMemDC], addr rec_bih, DIB_RGB_COLORS, addr rec_pDIBits, NULL, 0
    mov [rec_hDIB], rax
    invoke SelectObject, [rec_hMemDC], [rec_hDIB]
    mov [rec_hOldDIB], rax

    ; Frame Delay: 1000 / targetFps
    xor edx, edx
    mov eax, 1000
    mov ecx, [targetFps]
    test ecx, ecx
    jnz .fps_ok
    mov ecx, 30
.fps_ok:
    div ecx
    mov [rec_frameDelay], eax

    ; Record Start Timestamp
    invoke GetTickCount64
    mov [recStartTime], rax
    mov [rec_sampleIndex], 0

; ----------------------------------------------------------------------------
; Core Capture Loop
; ----------------------------------------------------------------------------
.capture_loop:
    cmp [recThreadRunning], 1
    jne .finish_capture

    ; Check Pause State
    cmp [recThreadPause], 1
    jne .capture_frame
    invoke Sleep, 40
    jmp .capture_loop

.capture_frame:
    invoke GetTickCount64
    mov [rec_tickBefore], rax

    ; Direct BitBlt into DIB Section
    invoke BitBlt, [rec_hMemDC], 0, 0, [capW], [capH],\
           [rec_hScreenDC], [capX], [capY], SRCCOPY or CAPTUREBLT
    invoke GdiFlush

    ; Mouse Cursor Rendering
    cmp [includeCursor], 1
    jne .write_frame

    lea rdi, [rec_ci]
    mov dword [rdi], 24
    invoke GetCursorInfo, addr rec_ci
    test eax, eax
    jz .write_frame

    lea rdi, [rec_ci]
    test dword [rdi+4], CURSOR_SHOWING
    jz .write_frame

    ; Relative cursor pos
    mov eax, dword [rdi+16]
    sub eax, [capX]
    mov ebx, eax
    mov eax, dword [rdi+20]
    sub eax, [capY]
    mov esi, eax

    invoke DrawIconEx, [rec_hMemDC], ebx, esi, qword [rdi+8], 0, 0, 0, NULL, DI_NORMAL

.write_frame:
    ; Calculate elapsed wall-clock time since start (excluding pause)
    invoke GetTickCount64
    sub rax, [recStartTime]
    sub rax, [recTotalPausedTime]

    ; Target frame number based on real-time clock: (elapsedMs * targetFps) / 1000
    mov ecx, [targetFps]
    test ecx, ecx
    jnz .fps_pacing_ok
    mov ecx, 30
.fps_pacing_ok:
    imul rax, rcx
    xor edx, edx
    mov ecx, 1000
    div rcx
    ; eax = expectedFrameIndex
    mov ebx, eax

    ; Write frame(s) to stream so stream frame count precisely matches clock
.write_stream_catchup:
    invoke AVIStreamWrite, [rec_pAviStream], [recFrameCount], 1,\
           [rec_pDIBits], [rec_imageSize], AVIIF_KEYFRAME, NULL, NULL

    inc [recFrameCount]
    mov edx, [rec_imageSize]
    add [recBytesWritten], rdx

    cmp [recFrameCount], ebx
    jbe .write_stream_catchup

    ; Send Telemetry Update to Main GUI
    invoke PostMessageW, [hWndMain], WM_USER_TELEMETRY, 0, 0

    ; Calculate Frame Pacing Sleep
    invoke GetTickCount64
    mov [rec_tickAfter], rax
    sub rax, [rec_tickBefore]
    mov [rec_elapsed], rax

    mov ecx, [rec_frameDelay]
    cmp rax, rcx
    jge .capture_loop

    sub ecx, eax
    invoke Sleep, ecx
    jmp .capture_loop

; ----------------------------------------------------------------------------
; Teardown and Cleanup
; ----------------------------------------------------------------------------
.finish_capture:
    ; Final catch-up on stop so total video duration matches real-time clock to the millisecond
    cmp [rec_pDIBits], 0
    je .do_cleanup
    cmp [rec_pAviStream], 0
    je .do_cleanup

    invoke GetTickCount64
    sub rax, [recStartTime]
    sub rax, [recTotalPausedTime]
    mov ecx, [targetFps]
    test ecx, ecx
    jnz .fin_fps_ok
    mov ecx, 30
.fin_fps_ok:
    imul rax, rcx
    xor edx, edx
    mov ecx, 1000
    div rcx
    mov ebx, eax

.fin_catchup_loop:
    cmp [recFrameCount], ebx
    jae .do_cleanup
    invoke AVIStreamWrite, [rec_pAviStream], [recFrameCount], 1,\
           [rec_pDIBits], [rec_imageSize], AVIIF_KEYFRAME, NULL, NULL
    inc [recFrameCount]
    mov edx, [rec_imageSize]
    add [recBytesWritten], rdx
    jmp .fin_catchup_loop

.do_cleanup:
    invoke SelectObject, [rec_hMemDC], [rec_hOldDIB]
    invoke DeleteObject, [rec_hDIB]
    invoke DeleteDC, [rec_hMemDC]
    invoke ReleaseDC, 0, [rec_hScreenDC]

.release_stream:
    invoke AVIStreamRelease, [rec_pAviStream]

.close_avi_file:
    invoke AVIFileRelease, [rec_pAviFile]
    invoke AVIFileExit

.exit_thread:
    invoke PostMessageW, [hWndMain], WM_USER_REC_FINISHED, 0, 0
    xor eax, eax
    ret
endp


; ----------------------------------------------------------------------------
; PauseScreenRecording - Toggles Pause / Resume state
; ----------------------------------------------------------------------------
proc PauseScreenRecording uses rbx rsi rdi
    cmp [recState], STATE_RECORDING
    je .do_pause
    cmp [recState], STATE_PAUSED
    je .do_resume
    ret

.do_pause:
    mov [recState], STATE_PAUSED
    mov [recThreadPause], 1
    invoke GetTickCount64
    mov [recPauseStartTime], rax
    invoke MessageBeep, 0x00000030 ; MB_ICONEXCLAMATION
    call UpdateUIState
    invoke SetWindowTextW, [hStatusBar], addr szStatusPausedMsg
    ret

.do_resume:
    mov [recState], STATE_RECORDING
    mov [recThreadPause], 0
    invoke GetTickCount64
    sub rax, [recPauseStartTime]
    add [recTotalPausedTime], rax
    invoke MessageBeep, 0x00000040 ; MB_ICONASTERISK
    call UpdateUIState
    invoke SetWindowTextW, [hStatusBar], addr szStatusRecordingMsg
    ret
endp


; ----------------------------------------------------------------------------
; StopScreenRecording - Finalizes AVI file and updates GUI
; ----------------------------------------------------------------------------
proc StopScreenRecording uses rbx rsi rdi
    cmp [recState], STATE_IDLE
    je .done

    ; Signal Thread to Stop
    mov [recThreadRunning], 0
    mov [recThreadPause], 0

    ; Wait for thread completion (up to 3 seconds)
    cmp [hRecThread], 0
    je .thread_done
    invoke WaitForSingleObject, [hRecThread], 3000
    invoke CloseHandle, [hRecThread]
    mov [hRecThread], 0

.thread_done:
    mov [recState], STATE_IDLE
    invoke MessageBeep, 0x00000000 ; MB_OK

    ; Stop Audio Capture if active
    cmp [audioCaptureActive], 1
    jne .skip_audio_finalize

    invoke OpenEventA, 0x0002, FALSE, addr szAudioStopEvent
    test rax, rax
    jz .no_event_open
    push rax
    invoke SetEvent, rax
    pop rax
    invoke CloseHandle, rax

.no_event_open:
    cmp [piAudio.hProcess], 0
    je .check_mux
    invoke WaitForSingleObject, [piAudio.hProcess], 2500
    invoke CloseHandle, [piAudio.hProcess]
    invoke CloseHandle, [piAudio.hThread]
    mov [piAudio.hProcess], 0
    mov [audioCaptureActive], 0

.check_mux:
    ; Check if temp_audio.wav exists
    invoke GetFileAttributesW, addr szAudioTempWav
    cmp eax, -1
    je .skip_audio_finalize

    ; Copy szCurrentRecFile to szFinalMp4File and change .avi to .mp4
    lea rsi, [szCurrentRecFile]
    lea rdi, [szFinalMp4File]
    mov ecx, 260
    rep movsw

    lea rdi, [szFinalMp4File]
.find_ext:
    cmp word [rdi], 0
    je .got_ext
    add rdi, 2
    jmp .find_ext
.got_ext:
    sub rdi, 8
    mov word [rdi], '.'
    mov word [rdi+2], 'm'
    mov word [rdi+4], 'p'
    mov word [rdi+6], '4'
    mov word [rdi+8], 0

    ; Format Mux Command: ffmpeg.exe -y -i "%s" -i "recordings\temp_audio.wav" -c:v copy -c:a aac -b:a 192k "%s"
    invoke wsprintfW, addr cmdMuxBuf, addr fmtFfmpegMux, addr szCurrentRecFile, addr szFinalMp4File

    ; Launch FFmpeg in background
    lea rdi, [siFfmpeg]
    xor eax, eax
    mov ecx, sizeof.STARTUPINFO / 4
    rep stosd
    mov [siFfmpeg.cb], sizeof.STARTUPINFO
    mov [siFfmpeg.dwFlags], 1
    mov [siFfmpeg.wShowWindow], 0 ; SW_HIDE

    invoke CreateProcessW, NULL, addr cmdMuxBuf, NULL, NULL, FALSE,\
           0x08000000, NULL, NULL, addr siFfmpeg, addr piFfmpeg
    test eax, eax
    jz .skip_audio_finalize

    invoke WaitForSingleObject, [piFfmpeg.hProcess], 10000
    invoke CloseHandle, [piFfmpeg.hProcess]
    invoke CloseHandle, [piFfmpeg.hThread]

    ; Verify output MP4 exists
    invoke GetFileAttributesW, addr szFinalMp4File
    cmp eax, -1
    je .skip_audio_finalize

    ; Delete intermediate uncompressed AVI and temporary WAV
    invoke DeleteFileW, addr szCurrentRecFile
    invoke DeleteFileW, addr szAudioTempWav

    ; Update szCurrentRecFile to the final MP4
    lea rsi, [szFinalMp4File]
    lea rdi, [szCurrentRecFile]
    mov ecx, 260
    rep movsw

.skip_audio_finalize:
    call UpdateUIState
    call HideMiniBar

    ; Format Completion Message for Status Bar
    mov rax, [recBytesWritten]
    shr rax, 10 ; KB
    invoke wsprintfW, addr statusMsgBuf, addr szStatusRecSavedMsg, addr szCurrentRecFile, eax, [recFrameCount]
    invoke SetWindowTextW, [hStatusBar], addr statusMsgBuf

    ; Auto-refresh gallery
    call RefreshGalleryList

.done:
    ret
endp


; ----------------------------------------------------------------------------
; UpdateTelemetryUI - Calculates live recording duration, FPS, and size
; ----------------------------------------------------------------------------
proc UpdateTelemetryUI uses rbx rsi rdi
    cmp [recState], STATE_IDLE
    je .done

    ; Calculate Elapsed Time (excluding paused duration)
    invoke GetTickCount64
    sub rax, [recStartTime]
    sub rax, [recTotalPausedTime]

    cmp [recState], STATE_PAUSED
    jne .calc_time

    ; If paused, subtract current pause interval
    invoke GetTickCount64
    sub rax, [recPauseStartTime]
    sub [tel_elapsedMs], rax

.calc_time:
    xor edx, edx
    mov ecx, 1000
    div ecx ; eax = elapsed seconds
    mov [tel_elapsedSec], eax

    ; Hours
    xor edx, edx
    mov ecx, 3600
    div ecx
    mov [tel_hh], eax

    ; Minutes & Seconds
    mov eax, edx
    xor edx, edx
    mov ecx, 60
    div ecx
    mov [tel_mm], eax
    mov [tel_ss], edx

    ; Format Big Timer (HH : MM : SS)
    invoke wsprintfW, addr timeBuf, addr fmtTimer, [tel_hh], [tel_mm], [tel_ss]
    invoke SetWindowTextW, [hLblTimer], addr timeBuf

    ; Format Frames
    invoke wsprintfW, addr framesBuf, addr fmtFrames, [recFrameCount]
    invoke SetWindowTextW, [hLblFrames], addr framesBuf

    ; Format FPS
    mov eax, [targetFps]
    test eax, eax
    jnz .fps_disp_ok
    mov eax, 30
.fps_disp_ok:
    invoke wsprintfW, addr fpsBuf, addr fmtFps, eax
    invoke SetWindowTextW, [hLblFps], addr fpsBuf

    ; Format Size
    fastcall FormatFileSize, [recBytesWritten], addr gal_sizeStr
    invoke wsprintfW, addr sizeBuf, addr fmtTelemetrySize, addr gal_sizeStr
    invoke SetWindowTextW, [hLblSize], addr sizeBuf

.done:
    ret
endp

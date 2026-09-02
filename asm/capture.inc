; ============================================================================
; ScreenCapture Pro x64 - Wide Unicode Capture and Real-Time Video Engine
; Direct Real-Time H.264 MP4 Streaming Pipe & Standalone Portable WASAPI Audio
; ============================================================================

; ----------------------------------------------------------------------------
; InitializeAppDirectories - Resolves application directory, creates folders,
; and auto-extracts embedded audiocap.exe if not present on disk.
; ----------------------------------------------------------------------------
proc InitializeAppDirectories uses rbx rsi rdi
    ; 1. Get full executable path: e.g. "C:\MyApp\screenvideo.exe"
    invoke GetModuleFileNameW, NULL, addr szAppDir, 260
    lea rdi, [szAppDir]
    invoke lstrlenW, rdi
    lea rdi, [szAppDir + rax*2]
.find_slash:
    cmp rdi, szAppDir
    jbe .slash_found
    cmp word [rdi], '\'
    je .slash_found
    sub rdi, 2
    jmp .find_slash
.slash_found:
    mov word [rdi+2], 0 ; Truncate after '\' to get base directory (e.g. "C:\MyApp\")

    ; 2. Build absolute szRecDir: "<szAppDir>recordings"
    invoke lstrcpyW, addr szRecDir, addr szAppDir
    invoke lstrcatW, addr szRecDir, addr szRecSubDir
    invoke CreateDirectoryW, addr szRecDir, NULL

    ; 3. Build absolute szShotDir: "<szAppDir>screenshots"
    invoke lstrcpyW, addr szShotDir, addr szAppDir
    invoke lstrcatW, addr szShotDir, addr szShotSubDir
    invoke CreateDirectoryW, addr szShotDir, NULL

    ; 4. Build absolute szAudioCapExe: "<szAppDir>audiocap.exe"
    invoke lstrcpyW, addr szAudioCapExe, addr szAppDir
    invoke lstrcatW, addr szAudioCapExe, addr szAudioCapName

    ; 5. Check if audiocap.exe exists; if not, AUTO-EXTRACT from embedded binary!
    invoke GetFileAttributesW, addr szAudioCapExe
    cmp eax, -1
    jne .audiocap_ready

    invoke CreateFileW, addr szAudioCapExe, GENERIC_WRITE, 0, NULL,\
           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    cmp rax, -1
    je .audiocap_ready
    mov rbx, rax
    invoke WriteFile, rbx, addr audiocap_bin_start, audiocap_bin_size, addr sb_bytesWritten, NULL
    invoke CloseHandle, rbx

.audiocap_ready:
    ; 6. Check for FFmpeg (App dir or PATH):
    invoke lstrcpyW, addr szFfmpegExe, addr szAppDir
    invoke lstrcatW, addr szFfmpegExe, addr szFfmpegName
    invoke GetFileAttributesW, addr szFfmpegExe
    cmp eax, -1
    jne .ffmpeg_found

    ; Search PATH via SearchPathW
    invoke SearchPathW, NULL, addr szFfmpegName, NULL, 260, addr szFfmpegExe, NULL
    test eax, eax
    jz .ffmpeg_not_found

.ffmpeg_found:
    mov [hasFfmpeg], 1
    jmp .ffmpeg_ready

.ffmpeg_not_found:
    mov [hasFfmpeg], 0

.ffmpeg_ready:
    ; 7. Build search masks for gallery
    invoke lstrcpyW, addr szMaskMp4, addr szRecDir
    invoke lstrcatW, addr szMaskMp4, addr szWildcardMp4

    invoke lstrcpyW, addr szMaskAvi, addr szRecDir
    invoke lstrcatW, addr szMaskAvi, addr szWildcardAvi

    invoke lstrcpyW, addr szMaskBmp, addr szShotDir
    invoke lstrcatW, addr szMaskBmp, addr szWildcardBmp

    ; 8. Build temp audio WAV path: "<szRecDir>\temp_audio.wav"
    invoke lstrcpyW, addr szAudioTempWav, addr szRecDir
    invoke lstrcatW, addr szAudioTempWav, addr szBackslash
    invoke lstrcatW, addr szAudioTempWav, addr szAudioTempWavName

    ; 9. Build temp video MP4 path: "<szRecDir>\temp_video.mp4"
    invoke lstrcpyW, addr szTempVideoMp4, addr szRecDir
    invoke lstrcatW, addr szTempVideoMp4, addr szBackslash
    invoke lstrcatW, addr szTempVideoMp4, addr szTempVideoMp4Name

    ret
endp


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
    ; Format Unique Screenshot Filename with Timestamp
    invoke GetLocalTime, addr stLocal
    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    movzx r15, [stLocal.wSecond]
    invoke wsprintfW, addr tempBuf, addr fmtShotFilenameRel, r10, r11, r12, r13, r14, r15
    invoke lstrcpyW, addr szCurrentShotFile, addr szShotDir
    invoke lstrcatW, addr szCurrentShotFile, addr tempBuf

    ; Create Screen Capture DC & Bitmap
    invoke GetWindowDC, 0
    mov [shot_hScreenDC], rax
    invoke CreateCompatibleDC, [shot_hScreenDC]
    mov [shot_hMemDC], rax
    invoke CreateCompatibleBitmap, [shot_hScreenDC], [shot_cW], [shot_cH]
    mov [shot_hBmp], rax
    invoke SelectObject, [shot_hMemDC], [shot_hBmp]
    mov [shot_hOldBmp], rax

    ; BitBlt Screen to Memory DC
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

    mov eax, dword [rdi+16]            ; ptScreenPos.x
    sub eax, [shot_cX]
    mov ebx, eax
    mov eax, dword [rdi+20]            ; ptScreenPos.y
    sub eax, [shot_cY]
    mov esi, eax

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
; StartScreenRecording - Sets up real-time FFmpeg pipe and launches capture thread
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

    ; Format Video Output Filename with Timestamp (.mp4)
    invoke GetLocalTime, addr stLocal
    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    movzx r15, [stLocal.wSecond]
    invoke wsprintfW, addr tempBuf, addr fmtRecFilenameRel, r10, r11, r12, r13, r14, r15
    invoke lstrcpyW, addr szFinalMp4File, addr szRecDir
    invoke lstrcatW, addr szFinalMp4File, addr tempBuf
    invoke lstrcpyW, addr szCurrentRecFile, addr szFinalMp4File

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

    ; 1. Create Anonymous Pipe for Video (with handle inheritance)
    mov [pipeSecAttr.nLength], sizeof.SECURITY_ATTRIBUTES
    mov [pipeSecAttr.lpSecurityDescriptor], 0
    mov [pipeSecAttr.bInheritHandle], 1
    mov [pipeSecAttr._pad], 0
    mov [pipeSecAttr._pad2], 0

    invoke CreatePipe, addr hPipeRead, addr hPipeWrite, addr pipeSecAttr, 0
    test eax, eax
    jz .fail_pipe

    ; Do NOT inherit write handle into child process
    invoke SetHandleInformation, [hPipeWrite], HANDLE_FLAG_INHERIT, 0

    ; 2. Format FFmpeg Real-Time Encoding Pipe Command
    ; If audio is enabled, stream to szTempVideoMp4, then mux on stop.
    ; If audio is disabled, stream directly to szFinalMp4File!
    cmp [audioSysEnabled], 1
    je .use_temp_target
    cmp [audioMicEnabled], 1
    je .use_temp_target

    lea rbx, [szFinalMp4File]
    jmp .format_ffmpeg_cmd

.use_temp_target:
    lea rbx, [szTempVideoMp4]

.format_ffmpeg_cmd:
    mov eax, [targetFps]
    test eax, eax
    jnz @f
    mov eax, 30
@@:
    mov [rec_fpsVal], eax
    invoke wsprintfW, addr szFfmpegPipeCmd, addr fmtFfmpegPipe, addr szFfmpegExe, [capW], [capH], [rec_fpsVal], rbx

    ; 3. Launch FFmpeg Process with Stdin redirected to Read Pipe
    lea rdi, [siFfmpegPipe]
    xor eax, eax
    mov ecx, sizeof.STARTUPINFO / 4
    rep stosd
    mov [siFfmpegPipe.cb], sizeof.STARTUPINFO
    mov [siFfmpegPipe.dwFlags], STARTF_USESTDHANDLES or 1 ; STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW
    mov [siFfmpegPipe.wShowWindow], 0 ; SW_HIDE
    mov rax, [hPipeRead]
    mov [siFfmpegPipe.hStdInput], rax

    invoke CreateProcessW, NULL, addr szFfmpegPipeCmd, NULL, NULL, TRUE,\
           0x08000000, NULL, NULL, addr siFfmpegPipe, addr piFfmpegPipe
    test eax, eax
    jz .fail_ffmpeg

    ; Parent does not need read handle anymore
    invoke CloseHandle, [hPipeRead]
    mov [hPipeRead], 0

    ; 4. Check and Launch Audio Capture if enabled
    mov [audioCaptureActive], 0
    cmp [audioSysEnabled], 1
    je .launch_audio
    cmp [audioMicEnabled], 1
    jne .launch_video_thread

.launch_audio:
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
    mov [siAudio.dwFlags], 1
    mov [siAudio.wShowWindow], 0

    invoke wsprintfW, addr cmdMuxBuf, addr fmtAudioCapCmd, addr szAudioCapExe, addr szAudioTempWav

    invoke CreateProcessW, NULL, addr cmdMuxBuf, NULL, NULL, FALSE,\
           0x08000000, NULL, NULL, addr siAudio, addr piAudio
    test eax, eax
    jz .launch_video_thread
    mov [audioCaptureActive], 1

.launch_video_thread:
    ; 5. Launch High-Performance Video Recording Thread
    invoke CreateThread, NULL, 0, RecordingThreadProc, NULL, 0, NULL
    mov [hRecThread], rax

    ; Show floating toolbar and hide main window
    call ShowMiniBar
    ret

.fail_ffmpeg:
    invoke CloseHandle, [hPipeRead]
    invoke CloseHandle, [hPipeWrite]
    mov [hPipeRead], 0
    mov [hPipeWrite], 0
.fail_pipe:
    mov [recState], STATE_IDLE
    call UpdateUIState
.already_running:
    ret
endp


; ----------------------------------------------------------------------------
; RecordingThreadProc - Real-Time Screen Capture & Direct Stdin Pipe Streaming
; ----------------------------------------------------------------------------
proc RecordingThreadProc uses rbx rsi rdi
    ; Calculate 32-bit stride and image size (always 4-byte aligned)
    mov eax, [capW]
    shl eax, 2
    mov [rec_stride], eax

    mov ecx, [capH]
    imul ecx
    mov [rec_imageSize], eax

    ; Setup BITMAPINFOHEADER (32-bit TrueColor Top-Down DIB)
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

    ; Create Capture Device Context & Direct DIB Section
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

; ----------------------------------------------------------------------------
; Core Capture Loop (Direct Real-Time Pipe Streaming)
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

    ; Mouse Cursor Rendering (if enabled)
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
    mov ebx, eax

.write_pipe_catchup:
    ; Write raw BGR0 frame directly into FFmpeg stdin pipe!
    invoke WriteFile, [hPipeWrite], [rec_pDIBits], [rec_imageSize], addr sb_bytesWritten, NULL
    inc [recFrameCount]
    ; Approximate compressed bytes for UI telemetry (average ~35 KB per H.264 frame instead of 8 MB uncompressed)
    add [recBytesWritten], 35000

    cmp [recFrameCount], ebx
    jbe .write_pipe_catchup

    ; Send Telemetry Update to Main GUI
    invoke PostMessageW, [hWndMain], WM_USER_TELEMETRY, 0, 0

    ; Frame Pacing Sleep
    invoke GetTickCount64
    mov [rec_tickAfter], rax
    sub rax, [rec_tickBefore]

    mov ecx, [rec_frameDelay]
    cmp rax, rcx
    jge .capture_loop

    sub ecx, eax
    invoke Sleep, ecx
    jmp .capture_loop

.finish_capture:
    ; Clean up GDI handles
    invoke SelectObject, [rec_hMemDC], [rec_hOldDIB]
    invoke DeleteObject, [rec_hDIB]
    invoke DeleteDC, [rec_hMemDC]
    invoke ReleaseDC, 0, [rec_hScreenDC]

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
; StopScreenRecording - Finalizes MP4 recording, merges audio, and updates GUI
; ----------------------------------------------------------------------------
proc StopScreenRecording uses rbx rsi rdi
    cmp [recState], STATE_IDLE
    je .done

    ; 1. Signal Recording Thread to Stop
    mov [recThreadRunning], 0
    mov [recThreadPause], 0

    ; Wait for thread completion (up to 3 seconds)
    cmp [hRecThread], 0
    je .thread_done
    invoke WaitForSingleObject, [hRecThread], 3000
    invoke CloseHandle, [hRecThread]
    mov [hRecThread], 0

.thread_done:
    ; 2. Close Write Pipe to send EOF to FFmpeg
    cmp [hPipeWrite], 0
    je .pipe_closed
    invoke CloseHandle, [hPipeWrite]
    mov [hPipeWrite], 0

.pipe_closed:
    ; Wait for FFmpeg real-time pipe process to finalize MP4 file (up to 5 seconds)
    cmp [piFfmpegPipe.hProcess], 0
    je .ffmpeg_done
    invoke WaitForSingleObject, [piFfmpegPipe.hProcess], 5000
    invoke CloseHandle, [piFfmpegPipe.hProcess]
    invoke CloseHandle, [piFfmpegPipe.hThread]
    mov [piFfmpegPipe.hProcess], 0

.ffmpeg_done:
    mov [recState], STATE_IDLE
    invoke MessageBeep, 0x00000000 ; MB_OK

    ; 3. Stop Audio Capture if active
    cmp [audioCaptureActive], 1
    jne .check_final_file

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
    je .no_audio_wav

    ; Mux temp_video.mp4 + temp_audio.wav -> szFinalMp4File
    invoke wsprintfW, addr cmdMuxBuf, addr fmtFfmpegMuxFinal, addr szFfmpegExe,\
           addr szTempVideoMp4, addr szAudioTempWav, addr szFinalMp4File

    lea rdi, [siFfmpeg]
    xor eax, eax
    mov ecx, sizeof.STARTUPINFO / 4
    rep stosd
    mov [siFfmpeg.cb], sizeof.STARTUPINFO
    mov [siFfmpeg.dwFlags], 1
    mov [siFfmpeg.wShowWindow], 0

    invoke CreateProcessW, NULL, addr cmdMuxBuf, NULL, NULL, FALSE,\
           0x08000000, NULL, NULL, addr siFfmpeg, addr piFfmpeg
    test eax, eax
    jz .no_audio_wav

    invoke WaitForSingleObject, [piFfmpeg.hProcess], 10000
    invoke CloseHandle, [piFfmpeg.hProcess]
    invoke CloseHandle, [piFfmpeg.hThread]

    ; Verify output exists
    invoke GetFileAttributesW, addr szFinalMp4File
    cmp eax, -1
    je .no_audio_wav

    ; Cleanup temp files
    invoke DeleteFileW, addr szTempVideoMp4
    invoke DeleteFileW, addr szAudioTempWav
    jmp .finalize_ui

.no_audio_wav:
    ; If temp_video.mp4 exists, rename it to szFinalMp4File
    invoke MoveFileExW, addr szTempVideoMp4, addr szFinalMp4File, MOVEFILE_REPLACE_EXISTING

.check_final_file:
.finalize_ui:
    call UpdateUIState
    call HideMiniBar

    ; Get accurate final file size from disk
    invoke CreateFileW, addr szFinalMp4File, GENERIC_READ, FILE_SHARE_READ, NULL,\
           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    cmp rax, -1
    je .skip_file_size
    mov rbx, rax
    invoke GetFileSize, rbx, NULL
    mov [recBytesWritten], rax
    invoke CloseHandle, rbx

.skip_file_size:
    ; Format Completion Message for Status Bar
    mov rax, [recBytesWritten]
    shr rax, 10 ; KB
    invoke wsprintfW, addr statusMsgBuf, addr szStatusRecSavedMsg, addr szFinalMp4File, eax, [recFrameCount]
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

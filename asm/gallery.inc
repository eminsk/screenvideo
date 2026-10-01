; ============================================================================
; ScreenCapture Pro x64 - Wide Unicode Gallery Manager (SysListView32)
; ============================================================================

; ----------------------------------------------------------------------------
; FormatFileSize - Formats bytes into clean KB or MB Wide string
; ----------------------------------------------------------------------------
proc FormatFileSize uses rbx rsi, nBytes, lpBuffer
    mov eax, ecx ; nBytes is in ecx
    mov rsi, rdx ; lpBuffer is in rdx
    cmp eax, 1048576
    jb .format_kb

    ; Format MB: (size / 1048576) . ((size % 1048576) * 100 / 1048576)
    xor edx, edx
    mov ecx, 1048576
    div ecx ; eax = MB, edx = remainder
    mov ebx, eax

    mov eax, edx
    mov ecx, 100
    mul ecx
    mov ecx, 1048576
    div ecx

    invoke wsprintfW, rsi, addr fmtSizeMb, ebx, eax
    ret

.format_kb:
    xor edx, edx
    mov ecx, 1024
    div ecx
    test eax, eax
    jnz .nonzero_kb
    mov eax, 1
.nonzero_kb:
    invoke wsprintfW, rsi, addr fmtSizeKb, eax
    ret
endp


; ----------------------------------------------------------------------------
; RefreshGalleryList - Scans recordings and screenshots folders
; ----------------------------------------------------------------------------
proc RefreshGalleryList uses rbx rsi rdi
    invoke SendMessageW, [hListGallery], LVM_DELETEALLITEMS, 0, 0
    mov [gal_itemIndex], 0

    ; Filter check: if only screenshots requested, skip AVI
    cmp [gal_filterMode], IDC_FILTER_SHOTS
    je .scan_bmp

    ; 1. SCAN RECORDINGS (*.mp4)
    invoke FindFirstFileW, addr szMaskMp4, addr findDataW
    cmp rax, -1
    je .scan_avi
    mov [gal_hFind], rax

.loop_mp4:
    test [findDataW.dwFileAttributes], 16
    jnz .next_mp4

    fastcall FormatFileSize, [findDataW.nFileSizeLow], addr gal_sizeStr

    lea rcx, [findDataW.ftLastWriteTime]
    invoke FileTimeToLocalFileTime, rcx, addr ftLocal
    invoke FileTimeToSystemTime, addr ftLocal, addr stLocal

    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    invoke wsprintfW, addr gal_resStr, addr fmtDateTime, r10, r11, r12, r13, r14

    ; Col 0: Type ("Видео")
    mov dword [lvItemW.mask], LVIF_TEXT
    mov eax, [gal_itemIndex]
    mov [lvItemW.iItem], eax
    mov [lvItemW.iSubItem], 0
    lea rax, [szTypeVideo]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_INSERTITEMW, 0, addr lvItemW

    ; Col 1: Filename
    mov dword [lvItemW.iSubItem], 1
    lea rax, [findDataW.cFileName]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    ; Col 2: Size
    mov dword [lvItemW.iSubItem], 2
    lea rax, [gal_sizeStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    ; Col 3: Date / Res
    mov dword [lvItemW.iSubItem], 3
    lea rax, [gal_resStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    inc [gal_itemIndex]

.next_mp4:
    invoke FindNextFileW, [gal_hFind], addr findDataW
    test eax, eax
    jnz .loop_mp4

    invoke FindClose, [gal_hFind]

.scan_avi:
    ; 2. SCAN RECORDINGS (*.avi)
    invoke FindFirstFileW, addr szMaskAvi, addr findDataW
    cmp rax, -1 ; INVALID_HANDLE_VALUE
    je .scan_bmp
    mov [gal_hFind], rax

.loop_avi:
    test [findDataW.dwFileAttributes], 16 ; FILE_ATTRIBUTE_DIRECTORY
    jnz .next_avi

    ; Format Size
    fastcall FormatFileSize, [findDataW.nFileSizeLow], addr gal_sizeStr

    ; Format Date
    lea rcx, [findDataW.ftLastWriteTime]
    invoke FileTimeToLocalFileTime, rcx, addr ftLocal
    invoke FileTimeToSystemTime, addr ftLocal, addr stLocal

    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    invoke wsprintfW, addr gal_resStr, addr fmtDateTime, r10, r11, r12, r13, r14

    ; Col 0: Type ("Видео")
    mov dword [lvItemW.mask], LVIF_TEXT
    mov eax, [gal_itemIndex]
    mov [lvItemW.iItem], eax
    mov [lvItemW.iSubItem], 0
    lea rax, [szTypeVideo]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_INSERTITEMW, 0, addr lvItemW

    ; Col 1: Filename
    mov dword [lvItemW.iSubItem], 1
    lea rax, [findDataW.cFileName]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    ; Col 2: Size
    mov dword [lvItemW.iSubItem], 2
    lea rax, [gal_sizeStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    ; Col 3: Date / Res
    mov dword [lvItemW.iSubItem], 3
    lea rax, [gal_resStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    inc [gal_itemIndex]

.next_avi:
    invoke FindNextFileW, [gal_hFind], addr findDataW
    test eax, eax
    jnz .loop_avi

    invoke FindClose, [gal_hFind]

.scan_bmp:
    ; Filter check: if only videos requested, finish
    cmp [gal_filterMode], IDC_FILTER_VIDEO
    je .done

    ; 2. SCAN SCREENSHOTS (*.bmp)
    invoke FindFirstFileW, addr szMaskBmp, addr findDataW
    cmp rax, -1
    je .done
    mov [gal_hFind], rax

.loop_bmp:
    test [findDataW.dwFileAttributes], 16
    jnz .next_bmp

    fastcall FormatFileSize, [findDataW.nFileSizeLow], addr gal_sizeStr

    lea rcx, [findDataW.ftLastWriteTime]
    invoke FileTimeToLocalFileTime, rcx, addr ftLocal
    invoke FileTimeToSystemTime, addr ftLocal, addr stLocal

    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    invoke wsprintfW, addr gal_resStr, addr fmtDateTime, r10, r11, r12, r13, r14

    mov dword [lvItemW.mask], LVIF_TEXT
    mov eax, [gal_itemIndex]
    mov [lvItemW.iItem], eax
    mov [lvItemW.iSubItem], 0
    lea rax, [szTypeShot]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_INSERTITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 1
    lea rax, [findDataW.cFileName]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 2
    lea rax, [gal_sizeStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 3
    lea rax, [gal_resStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    inc [gal_itemIndex]

.next_bmp:
    invoke FindNextFileW, [gal_hFind], addr findDataW
    test eax, eax
    jnz .loop_bmp

    invoke FindClose, [gal_hFind]

.scan_png:
    ; 3. SCAN SCREENSHOTS (*.png)
    invoke FindFirstFileW, addr szMaskPng, addr findDataW
    cmp rax, -1
    je .scan_jpg
    mov [gal_hFind], rax

.loop_png:
    test [findDataW.dwFileAttributes], 16
    jnz .next_png

    fastcall FormatFileSize, [findDataW.nFileSizeLow], addr gal_sizeStr

    lea rcx, [findDataW.ftLastWriteTime]
    invoke FileTimeToLocalFileTime, rcx, addr ftLocal
    invoke FileTimeToSystemTime, addr ftLocal, addr stLocal

    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    invoke wsprintfW, addr gal_resStr, addr fmtDateTime, r10, r11, r12, r13, r14

    mov dword [lvItemW.mask], LVIF_TEXT
    mov eax, [gal_itemIndex]
    mov [lvItemW.iItem], eax
    mov [lvItemW.iSubItem], 0
    lea rax, [szTypeShot]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_INSERTITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 1
    lea rax, [findDataW.cFileName]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 2
    lea rax, [gal_sizeStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 3
    lea rax, [gal_resStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    inc [gal_itemIndex]

.next_png:
    invoke FindNextFileW, [gal_hFind], addr findDataW
    test eax, eax
    jnz .loop_png

    invoke FindClose, [gal_hFind]

.scan_jpg:
    ; 4. SCAN SCREENSHOTS (*.jpg)
    invoke FindFirstFileW, addr szMaskJpg, addr findDataW
    cmp rax, -1
    je .done
    mov [gal_hFind], rax

.loop_jpg:
    test [findDataW.dwFileAttributes], 16
    jnz .next_jpg

    fastcall FormatFileSize, [findDataW.nFileSizeLow], addr gal_sizeStr

    lea rcx, [findDataW.ftLastWriteTime]
    invoke FileTimeToLocalFileTime, rcx, addr ftLocal
    invoke FileTimeToSystemTime, addr ftLocal, addr stLocal

    movzx r10, [stLocal.wYear]
    movzx r11, [stLocal.wMonth]
    movzx r12, [stLocal.wDay]
    movzx r13, [stLocal.wHour]
    movzx r14, [stLocal.wMinute]
    invoke wsprintfW, addr gal_resStr, addr fmtDateTime, r10, r11, r12, r13, r14

    mov dword [lvItemW.mask], LVIF_TEXT
    mov eax, [gal_itemIndex]
    mov [lvItemW.iItem], eax
    mov [lvItemW.iSubItem], 0
    lea rax, [szTypeShot]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_INSERTITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 1
    lea rax, [findDataW.cFileName]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 2
    lea rax, [gal_sizeStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    mov dword [lvItemW.iSubItem], 3
    lea rax, [gal_resStr]
    mov [lvItemW.pszText], rax
    invoke SendMessageW, [hListGallery], LVM_SETITEMW, 0, addr lvItemW

    inc [gal_itemIndex]

.next_jpg:
    invoke FindNextFileW, [gal_hFind], addr findDataW
    test eax, eax
    jnz .loop_jpg

    invoke FindClose, [gal_hFind]

.done:
    ret
endp


; ----------------------------------------------------------------------------
; GetSelectedGalleryFilePath - Retrieves full Wide path of selected item
; Returns 1 in RAX if found, 0 if nothing selected
; ----------------------------------------------------------------------------
proc GetSelectedGalleryFilePath uses rbx rsi rdi
    ; Get selected index
    mov r8, -1
    mov r9, LVNI_SELECTED
    invoke SendMessageW, [hListGallery], LVM_GETNEXTITEM, r8, r9
    cmp eax, -1
    je .no_selection
    mov [gal_selIdx], eax

    ; Get Col 1 (Filename)
    mov dword [lvItemW.mask], LVIF_TEXT
    mov eax, [gal_selIdx]
    mov [lvItemW.iItem], eax
    mov dword [lvItemW.iSubItem], 1
    lea rax, [szSelectedGalleryFile]
    mov [lvItemW.pszText], rax
    mov dword [lvItemW.cchTextMax], 256
    mov dword [lvItemW.state], 0
    mov dword [lvItemW.stateMask], 0
    mov dword [lvItemW._pad], 0
    movsxd r8, [gal_selIdx]
    lea r9, [lvItemW]
    invoke SendMessageW, [hListGallery], LVM_GETITEMTEXTW, r8, r9

    ; 1. First check in recordings\ folder
    invoke lstrcpyW, addr tempBuf, addr szRecDir
    lea rax, [tempBuf]
    invoke lstrlenW, rax
    mov word [tempBuf + rax*2], '\'
    mov word [tempBuf + rax*2 + 2], 0
    invoke lstrcatW, addr tempBuf, addr szSelectedGalleryFile

    invoke GetFileAttributesW, addr tempBuf
    cmp eax, -1 ; INVALID_FILE_ATTRIBUTES
    jne .found

    ; 2. If not found in recordings, check screenshots\ folder
    invoke lstrcpyW, addr tempBuf, addr szShotDir
    lea rax, [tempBuf]
    invoke lstrlenW, rax
    mov word [tempBuf + rax*2], '\'
    mov word [tempBuf + rax*2 + 2], 0
    invoke lstrcatW, addr tempBuf, addr szSelectedGalleryFile

.found:
    mov eax, 1
    ret

.no_selection:
    xor eax, eax
    ret
endp


; ----------------------------------------------------------------------------
; OpenSelectedGalleryItem - Launches file in default media player / viewer
; ----------------------------------------------------------------------------
proc OpenSelectedGalleryItem
    call GetSelectedGalleryFilePath
    test eax, eax
    jz .no_sel

    invoke ShellExecuteW, [hWndMain], addr szShellOpen, addr tempBuf, NULL, NULL, 1 ; SW_SHOWNORMAL
    ret

.no_sel:
    invoke SetWindowTextW, [hStatusBar], addr szStatusNoFileSelMsg
    ret
endp


; ----------------------------------------------------------------------------
; OpenSelectedGalleryFolder - Opens folder in Explorer and highlights file
; ----------------------------------------------------------------------------
proc OpenSelectedGalleryFolder
    call GetSelectedGalleryFilePath
    test eax, eax
    jz .open_default_dir

    ; Build /select,"path" command
    invoke wsprintfW, addr statusMsgBuf, addr fmtSelectExplorer, addr tempBuf
    invoke ShellExecuteW, [hWndMain], addr szShellOpen, addr szExplorer, addr statusMsgBuf, NULL, 1
    ret

.open_default_dir:
    invoke ShellExecuteW, [hWndMain], addr szShellOpen, addr szRecDir, NULL, NULL, 1
    ret
endp


; ----------------------------------------------------------------------------
; DeleteSelectedGalleryItem - Prompts user and deletes file
; ----------------------------------------------------------------------------
proc DeleteSelectedGalleryItem
    call GetSelectedGalleryFilePath
    test eax, eax
    jz .no_sel

    invoke MessageBoxW, [hWndMain], addr szConfirmDelMsg, addr szConfirmDelTitle,\
           0x00000004 or 0x00000020 ; MB_YESNO | MB_ICONQUESTION
    cmp eax, 6 ; IDYES
    jne .cancelled

    invoke DeleteFileW, addr tempBuf
    test eax, eax
    jz .fail

    invoke wsprintfW, addr statusMsgBuf, addr szStatusDeletedMsg, addr szSelectedGalleryFile
    invoke SetWindowTextW, [hStatusBar], addr statusMsgBuf
    call RefreshGalleryList
    ret

.fail:
    invoke MessageBeep, 0x00000010 ; MB_ICONERROR
    ret

.cancelled:
    ret

.no_sel:
    invoke SetWindowTextW, [hStatusBar], addr szStatusNoFileSelMsg
    ret
endp


; ----------------------------------------------------------------------------
; CopySelectedGalleryItemToClipboard - Copies selected screenshot BMP to clipboard
; ----------------------------------------------------------------------------
proc CopySelectedGalleryItemToClipboard uses rbx rsi rdi r12 r13
    local gpImg:QWORD
    local hBmp:QWORD
    local hFile:QWORD
    local dwFileSize:DWORD
    local dwDibSize:DWORD
    local dwBytesRead:DWORD
    local bmpMagic:WORD

    mov [hFile], 0

    call GetSelectedGalleryFilePath
    test eax, eax
    jz .no_sel

    ; 1. Try GDI+ loader (opens PNG, JPG, BMP seamlessly)
    lea rax, [gpImg]
    invoke GdipLoadImageFromFile, addr tempBuf, rax
    test eax, eax
    jnz .fallback_bmp_reader

    lea rax, [hBmp]
    invoke GdipCreateHBITMAPFromBitmap, [gpImg], rax, 0
    test eax, eax
    jnz .dispose_gp_and_fallback

    invoke OpenClipboard, [hWndMain]
    test eax, eax
    jz .delete_hbmp

    invoke EmptyClipboard
    invoke SetClipboardData, 2, [hBmp] ; CF_BITMAP = 2 (Windows auto-synthesizes CF_DIB)
    invoke CloseClipboard

    invoke GdipDisposeImage, [gpImg]

    ; Sound and status feedback
    invoke MessageBeep, 0x00000040 ; MB_ICONASTERISK
    invoke SetWindowTextW, [hStatusBar], addr szStatusCopiedToClipboard
    mov eax, 1
    ret

.delete_hbmp:
    invoke DeleteObject, [hBmp]
.dispose_gp_and_fallback:
    invoke GdipDisposeImage, [gpImg]

.fallback_bmp_reader:
    ; 2. Fallback: Direct BMP file reader
    invoke CreateFileW, addr tempBuf, 0x80000000, 1, NULL, 3, 0x80, NULL ; GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING
    cmp rax, -1
    je .file_error
    mov [hFile], rax

    ; Check file size
    invoke GetFileSize, [hFile], NULL
    mov [dwFileSize], eax
    cmp eax, 54 ; At least 14 (BFH) + 40 (BIH) bytes
    jbe .close_and_error

    ; Verify BMP magic 'BM' (0x4D42)
    invoke ReadFile, [hFile], addr bmpMagic, 2, addr dwBytesRead, NULL
    cmp [bmpMagic], 0x4D42
    jne .close_and_error

    ; DIB size = fileSize - 14
    mov eax, [dwFileSize]
    sub eax, 14
    mov [dwDibSize], eax

    ; Allocate moveable global memory for CF_DIB
    invoke GlobalAlloc, 2, [dwDibSize] ; GMEM_MOVEABLE = 2
    test rax, rax
    jz .close_and_error
    mov r12, rax ; r12 = hGlobal

    invoke GlobalLock, r12
    test rax, rax
    jz .free_hglobal
    mov r13, rax ; r13 = pLocked

    ; Seek past 14-byte BITMAPFILEHEADER
    invoke SetFilePointer, [hFile], 14, NULL, 0 ; FILE_BEGIN = 0

    ; Read DIB payload directly into global memory
    invoke ReadFile, [hFile], r13, [dwDibSize], addr dwBytesRead, NULL
    invoke CloseHandle, [hFile]
    mov [hFile], 0

    invoke GlobalUnlock, r12

    ; Set clipboard data
    invoke OpenClipboard, [hWndMain]
    test eax, eax
    jz .free_hglobal

    invoke EmptyClipboard
    invoke SetClipboardData, 8, r12 ; CF_DIB = 8
    invoke CloseClipboard

    ; Sound and status feedback
    invoke MessageBeep, 0x00000040 ; MB_ICONASTERISK
    invoke SetWindowTextW, [hStatusBar], addr szStatusCopiedToClipboard
    mov eax, 1
    ret

.free_hglobal:
    invoke GlobalFree, r12
.close_and_error:
    cmp [hFile], 0
    je .err_msg
    invoke CloseHandle, [hFile]
.err_msg:
    invoke MessageBeep, 0x00000010 ; MB_ICONERROR
    invoke SetWindowTextW, [hStatusBar], addr szStatusCopyError
    xor eax, eax
    ret

.file_error:
    invoke MessageBeep, 0x00000010
    invoke SetWindowTextW, [hStatusBar], addr szStatusCopyError
    xor eax, eax
    ret

.no_sel:
    invoke SetWindowTextW, [hStatusBar], addr szStatusNoFileSelMsg
    xor eax, eax
    ret
endp

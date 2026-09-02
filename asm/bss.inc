; ============================================================================
; ScreenCapture Pro x64 - Uninitialized Global Buffers & Application State
; ============================================================================

section '.bss' readable writeable

  ; Module & Window Handles
  hInst                 dq ?
  hWndMain              dq ?
  hWndOverlay           dq ?
  hStatusBar            dq ?

  ; Top Tab Navigation Buttons
  hTabBtnRecord         dq ?
  hTabBtnGallery        dq ?
  hTabBtnSettings       dq ?

  ; Tab 0: Запись Handles
  hLblStatusBadge       dq ?
  hLblRegionInfo        dq ?
  hLblTimer             dq ?
  hLblFrames            dq ?
  hLblFps               dq ?
  hLblSize              dq ?
  hComboMonitor         dq ?
  hBtnSelectRegion      dq ?
  hBtnFullscreen        dq ?
  hChkCursor            dq ?
  hChkHighlight         dq ?
  hChkCountdown         dq ?
  hChkMinibar           dq ?
  hChkAudioSys          dq ?
  hChkAudioMic          dq ?
  hBtnStart             dq ?
  hBtnPause             dq ?
  hBtnStop              dq ?
  hBtnScreenshot        dq ?

  ; Tab 1: Галерея Handles
  hLblGalleryHeader     dq ?
  hBtnOpenRecDir        dq ?
  hBtnRefresh           dq ?
  hBtnFilterAll         dq ?
  hBtnFilterVideo       dq ?
  hBtnFilterShots       dq ?
  hListGallery          dq ?
  hBtnPlay              dq ?
  hBtnShowInFolder      dq ?
  hBtnDelete            dq ?

  ; Tab 2: Настройки Handles
  hEditRecDir           dq ?
  hBtnBrowseRec         dq ?
  hEditShotDir          dq ?
  hBtnBrowseShot        dq ?
  hComboFps             dq ?
  hComboFormat          dq ?
  hComboCodec           dq ?
  hChkAudioSys2         dq ?
  hChkAudioMic2         dq ?
  hComboAudioBitrate    dq ?
  hLblHkStart           dq ?
  hLblHkPause           dq ?
  hLblHkStop            dq ?
  hLblHkShot            dq ?

  ; Application State
  currentTab            dd ?
  recState              dd ?
  targetFps             dd ?
  includeCursor         dd ?
  highlightCursor       dd ?
  showCountdown         dd ?
  showMinibar           dd ?
  audioSysEnabled       dd ?
  audioMicEnabled       dd ?
  audioBitrate          dd ?
  gal_filterMode        dd ?

  ; Screen Capture Bounds
  isCustomRegion        dd ?
  capX                  dd ?
  capY                  dd ?
  capW                  dd ?
  capH                  dd ?

  ; Recording Thread & Telemetry
  hRecThread            dq ?
  recStartTime          dq ?
  recPauseStartTime     dq ?
  recTotalPausedTime    dq ?
  recFrameCount         dd ?
  recBytesWritten       dq ?
  recThreadRunning      dd ?
  recThreadPause        dd ?

  ; GDI Fonts
  hFontRegular          dq ?
  hFontBold             dq ?
  hFontTitle            dq ?
  hFontStatusBadge      dq ?
  hFontTimer            dq ?
  hFontCaption          dq ?

  ; GDI Brushes & Pens
  hBrushBg              dq ?
  hBrushCard            dq ?
  hPenBorder            dq ?
  hPenBorderLight       dq ?
  hBrushTabActive       dq ?
  hBrushTabInactive     dq ?
  hBrushBtnGreen        dq ?
  hBrushBtnGreenHover   dq ?
  hBrushBtnBlue         dq ?
  hBrushBtnBlueHover    dq ?
  hBrushBtnDark         dq ?
  hBrushBtnDarkHover    dq ?
  hBrushBtnDisabled     dq ?
  hPenBtnRed            dq ?
  hBrushToggleOn        dq ?
  hBrushToggleOff       dq ?
  hBrushKnobOn          dq ?
  hBrushKnobOff         dq ?
  hPenToggleOn          dq ?
  hPenToggleOff         dq ?
  hBrushWhite           dq ?
  hPenWhite             dq ?
  hBrushAmber           dq ?
  hPenAmber             dq ?
  hBrushRed             dq ?
  hBrushCyan            dq ?
  hPenCyan              dq ?
  hPenBtnGreen          dq ?
  ptResume1             POINT
  ptResume2             POINT
  ptResume3             POINT

  ; Region Overlay Selector variables (Freeze-Screen Double-Buffered Engine)
  vScreenX              dd ?
  vScreenY              dd ?
  vScreenW              dd ?
  vScreenH              dd ?
  hFrozenBitmap         dq ?
  hDimmedBitmap         dq ?
  hOverlayMemDC         dq ?
  hOverlayBufferBmp     dq ?
  hOverlayOldBuf        dq ?
  hOverlayDimDC         dq ?
  hOverlayOldDim        dq ?
  hOverlayBrightDC      dq ?
  hOverlayOldBright     dq ?
  isOverlayDragging     dd ?
  dragStartX            dd ?
  dragStartY            dd ?
  dragCurrentX          dd ?
  dragCurrentY          dd ?
  hoverX                dd ?
  hoverY                dd ?
  szBadgeDimBuf         du 64 dup(?)
  rcMainClient          RECT

  ; Floating MiniBar
  hWndMiniBar           dq ?
  hMiniTimer            dq ?
  hBtnMiniPause         dq ?
  hBtnMiniStop          dq ?
  hBtnMiniShot          dq ?
  hBtnMiniRestore       dq ?
  hBrushMiniBar         dq ?
  hFontMiniTimer        dq ?
  wcMiniBar             WNDCLASSEX
  miniTimerBuf          du 32 dup(?)

  ; Dynamic Application Paths (Absolute, relocatable anywhere)
  szAppDir              du 260 dup(?)
  szRecDir              du 260 dup(?)
  szShotDir             du 260 dup(?)
  szAudioCapExe         du 260 dup(?)
  szFfmpegExe           du 260 dup(?)
  szAudioTempWav        du 260 dup(?)
  szTempVideoMp4        du 260 dup(?)
  szMaskMp4             du 260 dup(?)
  szMaskAvi             du 260 dup(?)
  szMaskBmp             du 260 dup(?)
  szFfmpegPipeCmd       du 1024 dup(?)

  ; Real-Time Video Pipe Handles & Structures
  hPipeRead             dq ?
  hPipeWrite            dq ?
  pipeSecAttr           SECURITY_ATTRIBUTES
  piFfmpegPipe          PROCESS_INFORMATION
  siFfmpegPipe          STARTUPINFO
  hasFfmpeg             dd ?

  ; Wide String Buffers
  szCurrentRecFile      du 260 dup(?)
  szCurrentShotFile     du 260 dup(?)
  szSelectedGalleryFile du 260 dup(?)
  szSelectedGalleryType du 64 dup(?)
  tempBuf               du 512 dup(?)
  timeBuf               du 64 dup(?)
  framesBuf             du 64 dup(?)
  fpsBuf                du 64 dup(?)
  sizeBuf               du 64 dup(?)
  targetDescBuf         du 128 dup(?)
  statusMsgBuf          du 256 dup(?)
  dwDarkVal             dd ?

  ; API Structures
  wc                    WNDCLASSEX
  wcOverlay             WNDCLASSEX
  msg                   MSG
  ps                    PAINTSTRUCT
  dis                   DRAWITEMSTRUCT
  findDataW             WIN32_FIND_DATAW
  lvColW                LVCOLUMNW
  lvItemW               LVITEMW
  stLocal               SYSTEMTIME
  ftLocal               FILETIME

  ; Audio & FFmpeg Process variables
  piAudio               PROCESS_INFORMATION
  siAudio               STARTUPINFO
  piFfmpeg              PROCESS_INFORMATION
  siFfmpeg              STARTUPINFO
  szFinalMp4File        du 260 dup(?)
  cmdMuxBuf             du 1024 dup(?)
  audioCaptureActive    dd ?

  ; SaveBitmapToFile work variables
  sb_bfh                db 14 dup(?)
  sb_bih                db 40 dup(?)
  sb_hFile              dq ?
  sb_pBits              dq ?
  sb_bytesWritten       dd ?
  sb_stride             dd ?
  sb_imageSize          dd ?

  ; TakeInstantScreenshot work variables
  shot_hScreenDC        dq ?
  shot_hMemDC           dq ?
  shot_hBmp             dq ?
  shot_hOldBmp          dq ?
  shot_ci               db 24 dup(?)
  shot_cW               dd ?
  shot_cH               dd ?
  shot_cX               dd ?
  shot_cY               dd ?

  ; RecordingThreadProc work variables
  rec_pAviFile          dq ?
  rec_pAviStream        dq ?
  rec_hScreenDC         dq ?
  rec_hMemDC            dq ?
  rec_hDIB              dq ?
  rec_hOldDIB           dq ?
  rec_pDIBits           dq ?
  rec_stride            dd ?
  rec_imageSize         dd ?
  rec_bih               db 40 dup(?)
  rec_siStream          db 256 dup(?)
  rec_ci                db 24 dup(?)
  rec_frameDelay        dd ?
  rec_fpsVal            dd ?
  rec_tickBefore        dq ?
  rec_tickAfter         dq ?
  rec_elapsed           dq ?
  rec_sampleIndex       dd ?
  rec_frameSamples      dd ?

  ; UpdateTelemetryUI work variables
  tel_elapsedMs         dq ?
  tel_elapsedSec        dd ?
  tel_hh                dd ?
  tel_mm                dd ?
  tel_ss                dd ?

  ; Gallery work variables
  gal_hFind             dq ?
  gal_itemIndex         dd ?
  gal_sizeStr           du 32 dup(?)
  gal_resStr            du 32 dup(?)
  gal_selIdx            dd ?

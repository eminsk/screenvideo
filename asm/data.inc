; ============================================================================
; ScreenCapture Pro x64 - Wide Unicode Data Strings & Formats
; ============================================================================

section '.data' data readable writeable

  ; Window Classes and Title
  szMainClass           du 'ScreenVideoMainClass', 0
  szOverlayClass        du 'ScreenVideoRegionOverlayClass', 0
  szAppTitle            du 'ScreenCapture Pro', 0

  ; Typography Font Families
  szFontSegoe           du 'Segoe UI', 0
  szFontConsolas        du 'Consolas', 0

  ; System Control Class Names
  szButtonClass         du 'BUTTON', 0
  szStaticClass         du 'STATIC', 0
  szEditClass           du 'EDIT', 0
  szComboClass          du 'COMBOBOX', 0
  szListViewClass       du 'SysListView32', 0
  szStatusBarClass      du 'msctls_statusbar32', 0
  szThemeExplorer       du 'DarkMode_Explorer', 0
  szThemeCfd            du 'DarkMode_CFD', 0
  szMiniBarClass        du 'ScreenVideoMiniBarClass', 0
  szMiniDot             du '●', 0
  szMiniBtnPause        du '⏸', 0
  szMiniBtnResume       du '▶', 0
  szMiniBtnStop         du '⏹', 0
  szMiniBtnShot         du '📸', 0
  szMiniBtnRestore      du '🗖', 0
  szDefMiniTimer        du '00:00', 0

  ; Top Tab Navigation Buttons
  szTabRecord           du '📹 Запись', 0
  szTabGallery          du '📁 Галерея', 0
  szTabSettings         du '⚙ Настройки', 0

  ; --------------------------------------------------------------------------
  ; Tab 0: Запись (Record View)
  ; --------------------------------------------------------------------------
  szCardHeroTitle       du 'Текущее состояние', 0
  szStatusIdle          du 'ГОТОВ К ЗАПИСИ', 0
  szStatusRecording     du 'ИДЁТ ЗАПИСЬ...', 0
  szStatusPaused        du 'ЗАПИСЬ НА ПАУЗЕ', 0
  szTargetFull          du 'Весь экран', 0
  szDefTimer            du '00 : 00 : 00', 0
  szDefFrames           du 'Кадров: 0', 0
  szDefFps              du 'FPS: 30.0', 0
  szDefSize             du 'Размер: 0 KB', 0

  szCardTargetTitle     du 'Источник захвата', 0
  szLblMonitor          du 'Экран / Монитор:', 0
  szMonAll              du '🖥 Все мониторы (Виртуальный экран): 1920×1080', 0
  szMonPrimary          du '🖥 Монитор 1: 1920×1080 (Основной)', 0
  szBtnSelectReg        du '✂ Выбрать область', 0
  szBtnResetFull        du '🖥 Сбросить на весь экран', 0

  szCardOptsTitle       du 'Быстрые опции', 0
  szChkCursor           du 'Захват курсора', 0
  szChkCountdown        du 'Таймер 3..2..1..', 0
  szChkAudioSys         du '🔊 Звук системы (динамики)', 0
  szChkHighlight        du 'Подсветка кликов/курсора', 0
  szChkMinibar          du 'Мини-панель записи', 0
  szChkAudioMic         du '🎤 Запись с микрофона', 0

  szBtnStart            du '▶ СТАРТ (F5)', 0
  szBtnPause            du '⏸ ПАУЗА (F6)', 0
  szBtnStop             du '⏹ СТОП (F10)', 0
  szBtnShot             du '📸 СНИМОК (F11)', 0

  ; --------------------------------------------------------------------------
  ; Tab 1: Галерея (Gallery View)
  ; --------------------------------------------------------------------------
  szGalleryTitle        du '📁 Сохранённые записи и снимки', 0
  szBtnOpenRecDir       du '📁 Папка записей', 0
  szBtnRefresh          du '🔄 Обновить', 0
  szFilterAll           du 'Все файлы', 0
  szFilterVideo         du '🎬 Только видео', 0
  szFilterShots         du '📸 Только снимки', 0

  szColType             du 'Тип', 0
  szColName             du 'Имя файла', 0
  szColSize             du 'Размер', 0
  szColRes              du 'Длит. / Разреш.', 0

  szTypeVideo           du '🎬 Видео', 0
  szTypeShot            du '📸 Снимок', 0

  szBtnPlay             du '▶ Воспроизвести', 0
  szBtnShowInFolder     du '📁 Показать в папке', 0
  szBtnDelete           du '🗑 Удалить', 0

  ; --------------------------------------------------------------------------
  ; Tab 2: Настройки (Settings View)
  ; --------------------------------------------------------------------------
  szCardFoldersTitle    du 'Папки сохранения', 0
  szLblRecFolder        du 'Папка для видеозаписей:', 0
  szDefRecDirFull       du 'C:\proekts\screenvideo\recordings', 0
  szLblShotFolder       du 'Папка для снимков экрана:', 0
  szDefShotDirFull      du 'C:\proekts\screenvideo\screenshots', 0
  szBtnBrowse           du 'Обзор...', 0

  szCardVideoTitle      du 'Настройки видео и частоты кадров', 0
  szLblFps              du 'Частота кадров (FPS):', 0
  szFps15               du '15', 0
  szFps30               du '30', 0
  szFps60               du '60', 0
  szLblContainer        du 'Формат контейнера:', 0
  szFmtAvi              du 'avi', 0
  szLblCodec            du 'Видеокодек FourCC:', 0
  szCodecDib            du 'DIB (несжатый)', 0

  szCardAudioTitle      du 'Настройки звука (Аудио)', 0
  szChkAudioSys2        du 'Записывать системный звук (динамики / наушники)', 0
  szChkAudioMic2        du 'Записывать звук с микрофона', 0
  szLblBitrate          du 'Битрейт аудио:', 0
  szBitrate192          du '192k', 0

  szCardHotkeysTitle    du 'Горячие клавиши', 0
  szLblHkStart          du 'Старт записи:', 0
  szLblHkPause          du 'Пауза записи:', 0
  szLblHkStop           du 'Стоп записи:', 0
  szLblHkShot           du 'Снимок экрана:', 0
  szKeyF5               du 'F5', 0
  szKeyF6               du 'F6', 0
  szKeyF10              du 'F10', 0
  szKeyF11              du 'F11', 0

  ; --------------------------------------------------------------------------
  ; Formats & Output Paths
  ; --------------------------------------------------------------------------
  fmtTimer              du '%02u : %02u : %02u', 0
  fmtFrames             du 'Кадров: %u', 0
  fmtFps                du 'FPS: %u.0', 0
  fmtSizeKb             du '%u KB', 0
  fmtSizeMb             du '%u.%02u MB', 0
  fmtTelemetrySize      du 'Размер: %s', 0
  fmtRegionSummary      du '%u × %u', 0
  fmtDateTime           du '%04u-%02u-%02u %02u:%02u', 0

  szRecSubDir           du 'recordings', 0
  szShotSubDir          du 'screenshots', 0
  szAudioCapName        du 'audiocap.exe', 0
  szFfmpegName          du 'ffmpeg.exe', 0
  szAudioTempWavName    du 'temp_audio.wav', 0
  szTempVideoMp4Name    du 'temp_video.mp4', 0
  szWildcardMp4         du '\*.mp4', 0
  szWildcardAvi         du '\*.avi', 0
  szWildcardBmp         du '\*.bmp', 0
  szBackslash           du '\', 0

  szAudioStopEvent      db 'ScreenVideoAudioStopEvent', 0
  fmtAudioCapCmd        du '"%s" "%s"', 0
  fmtFfmpegPipe         du '"%s" -y -f rawvideo -vcodec rawvideo -s %ux%u -pix_fmt bgr0 -r %u -i - -c:v libx264 -preset ultrafast -pix_fmt yuv420p -crf 23 "%s"', 0
  fmtFfmpegMuxFinal     du '"%s" -y -i "%s" -i "%s" -c:v copy -c:a aac -b:a 192k -shortest "%s"', 0
  fmtRecFilenameRel     du '\recording_%04u%02u%02u_%02u%02u%02u.mp4', 0
  fmtShotFilenameRel    du '\shot_%04u%02u%02u_%02u%02u%02u.bmp', 0

  szShellOpen           du 'open', 0
  szExplorer            du 'explorer.exe', 0
  fmtSelectExplorer     du '/select,"%s"', 0
  szConfirmDelTitle     du 'Подтверждение удаления', 0
  szConfirmDelMsg       du 'Вы действительно хотите безвозвратно удалить выбранный файл?', 0

  szStatusReady         du 'Готов к работе', 0
  szStatusRecordingMsg  du 'Идёт запись экрана...', 0
  szStatusPausedMsg     du 'Запись на паузе', 0
  szStatusRecSavedMsg   du 'Запись сохранена: %s (%u KB, %u кадров)', 0
  szStatusShotSavedMsg  du 'Снимок экрана сохранён: %s (%u KB)', 0
  szStatusDeletedMsg    du 'Файл удалён: %s', 0
  szStatusNoFileSelMsg  du 'Выберите файл из списка для открытия или удаления', 0

  ; Common Controls Configuration
  align 8
  icc INITCOMMONCONTROLSEX sizeof.INITCOMMONCONTROLSEX, 0x0000FFFF

  ; Embedded Standalone Audio Engine Binary
  align 16
  audiocap_bin_start:
    file 'audiocap.exe'
  audiocap_bin_end:
  audiocap_bin_size = audiocap_bin_end - audiocap_bin_start

# ⚡ ScreenCapture Pro v2.0 (x64 Native Assembly Edition)

[![Architecture: x64](https://img.shields.io/badge/Architecture-x86__64-3776AB.svg)](https://en.wikipedia.org/wiki/X86-64)
[![Assembler: FASM](https://img.shields.io/badge/Assembler-Flat%20Assembler%201.73-E91E63.svg)](https://flatassembler.net/)
[![Binary Size](https://img.shields.io/badge/Binary%20Size-~37%20KB-4CAF50.svg)](#)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%2F%2011-0078D6.svg?logo=windows&logoColor=white)](https://microsoft.com/windows)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](../LICENSE)

A professional, high-performance screen recorder, interactive region selector, and instant screenshot suite engineered with near-zero CPU and memory overhead, written in **pure 64-bit Flat Assembler (FASM x64)** for Windows 10 and 11.

Designed as a bare-metal native counterpart to the Python edition of `ScreenCapture Pro`, achieving microsecond responsiveness and complete independence from heavy runtimes.

---

## ⚡ Key Highlights

- **Ultra-Lightweight Footprint**: Standalone executable size of only **~37 KB** (37,888 bytes). Zero external dependencies, no Python runtime, no Electron bloat, and no third-party runtime DLLs.
- **Pure 64-Bit x64 ABI**: Direct low-level integration with Windows system APIs (`KERNEL32`, `USER32`, `GDI32`, `COMCTL32`, `SHELL32`, `AVIFIL32`, `DWMAPI`, `UXTHEME`).
- **Custom Owner-Draw Modern Dark UI**:
  - Pixel-perfect match with the Python edition: `#1E1E1E` background, `#252526` rounded cards with `#383E47` borders (`RoundRect`).
  - Native Windows 10/11 dark titlebar integration via DWM API (`DwmSetWindowAttribute`).
  - Custom colored owner-draw rounded buttons: Start (Emerald `#00BC7D`), Snapshot (Ocean Blue `#0284C7`), Pause (Amber `#F59E0B`), Stop (Crimson `#EF4444`).
  - Dark segmented tab switcher with active neon indicator.
  - High-precision digital timer rendered in Consolas 32pt.
- **100% Native Wide Unicode (UTF-16)**: Full international character support via `INCLUDE\ENCODING\UTF8.INC` and Win32 `W` APIs (`CreateWindowExW`, `DrawTextW`, `SetWindowTextW`, `FindFirstFileW`, `CreateFileW`).
- **Interactive Snipping Tool Overlay**:
  - Fullscreen translucent sniper overlay with crosshair cursor (`IDC_CROSS`).
  - Smooth rectangular region focus framing (`DrawFocusRect`).
  - Instant cancellation with `ESC` or right mouse click.
- **Global Low-Level Hotkeys**:
  - `F5` — Start screen recording.
  - `F6` — Pause / Resume recording.
  - `F10` — Stop recording and finalize file headers.
  - `F11` — Instant fullscreen or region screenshot.
- **Built-in Media Gallery & Catalog**:
  - Native list view table (`SysListView32`) with sorting, file types, human-readable sizes (KB / MB), and creation timestamps.
  - Actions: Open File, Reveal in Explorer with selection (`/select`), Refresh, Delete.
- **Hardware Cursor Capture**: High-precision cursor position tracking and icon blitting (`GetCursorInfo`, `DrawIconEx`) on screenshots and video stream frames.
- **Embedded Resources**: Hi-DPI application icon (ID 1) and Common Controls 6.0 XML manifest compiled directly into the PE `.rsrc` section.

---

## 📂 Architecture & File Structure

```
screenvideo/asm/
├── screenvideo.asm          # Entry point (start), window class, message loop,
│                            # hotkey dispatch, MainWndProc, imports & .rsrc section
├── const.inc                # Control IDs, color tokens, hotkey constants,
│                            # window messages and struct layouts (TCITEM, LVCOLUMN, LVITEM)
├── data.inc                 # Initialized string literals, formats, paths, UI classes
├── bss.inc                  # Uninitialized handles (HWND, HFONT, HDC), telemetry, buffers
├── ui.inc                   # Owner-draw UI rendering, fonts, brushes, tab controller
├── capture.inc              # Capture engine: background worker thread (RecordingThreadProc),
│                            # AVI stream creation via AVIFIL32, 24-bit BMP saver, cursor blitting
├── region.inc               # Interactive region snipping tool overlay & geometry math
├── gallery.inc              # Directory scanning for recordings/screenshots, SysListView32 binding
├── manifest.xml             # Common Controls v6.0 & Per-Monitor V2 DPI awareness manifest
├── icon.ico                 # High-resolution application icon
├── FASM.EXE                 # Flat Assembler compiler v1.73.35 (x64)
├── build.bat                # One-click automated build & run script
└── INCLUDE/                 # Self-contained 64-bit FASM header library (API, EQUATES, MACRO)
```

---

## 🛠️ Build & Compilation

### One-Click Build Script:
Run `build.bat` from Command Prompt:
```cmd
cd asm
build.bat
```

### Direct Compilation with FASM:
```cmd
cd asm
FASM.EXE screenvideo.asm screenvideo.exe
```

Compilation finishes in **~0.5 seconds** across 5 passes, producing an ultra-optimized `screenvideo.exe` of ~37 KB.

---

## ⌨️ Controls & Hotkeys

| Key / Control | Action |
|---|---|
| **`F5`** | Start screen recording |
| **`F6`** | Pause / Resume recording |
| **`F10`** | Stop recording and finalize AVI file |
| **`F11`** | Instant screenshot (BMP) |
| **`ESC`** | Cancel active region selection |
| **Double-Click (Gallery)** | Open recorded video or screenshot in default Windows app |

### Output Storage Directories:
- **Recordings:** `recordings\rec_YYYYMMDD_HHMMSS.avi`
- **Screenshots:** `screenshots\shot_YYYYMMDD_HHMMSS.bmp`

---

## 📄 License

This assembly edition is distributed under the **MIT License** as part of the ScreenCapture Pro project.

---

<details>
<summary><b>📖 Описание на русском языке (Нажмите, чтобы развернуть)</b></summary>

### Преимущества FASM x64 редакции:
* **Экстремальный размер**: Исполняемый файл всего **~37 КБ** без Python, Electron и сторонних зависимостей.
* **Чистый x64 Win32 API**: Прямая работа с `GDI32`, `USER32`, `AVIFIL32`, `DWMAPI`.
* **Тёмный современный интерфейс**: Кастомная отрисовка скругленных элементов, кнопок и карточек в тёмном стиле.
* **Интерактивный селектор**: Выделение произвольной области экрана для записи или скриншота.
* **Горячие клавиши**: `F5` — старт, `F6` — пауза, `F10` — стоп, `F11` — снимок, `ESC` — отмена.
* **Встроенная галерея**: Менеджер записанных файлов и скриншотов с открытием в один клик.

</details>

# 🎬 ScreenCapture Pro

[![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB.svg?logo=python&logoColor=white)](https://www.python.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-4CAF50.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%2F%2011-0078D6.svg?logo=windows&logoColor=white)](https://microsoft.com/windows)
[![Build: Nuitka](https://img.shields.io/badge/Build-Nuitka%20Standalone-FF9800.svg)](https://nuitka.net/)
[![Code style: ruff](https://img.shields.io/badge/Code%20style-Ruff-000000.svg?logo=ruff&logoColor=white)](https://github.com/astral-sh/ruff)

**ScreenCapture Pro** is a modern, high-performance desktop screen recording and screenshot suite for Windows. Engineered with a zero-memory streaming architecture, low-latency WASAPI loopback audio, native Win32/x64 assembly optimizations, customizable hotkeys, interactive region snipping, and a sleek dark/light theme GUI.

![Screen Recorder](icon.png)

---

## ✨ Key Features

- 🖥️ **Full-Screen & Multi-Monitor Support**: Seamless recording across single displays, multi-monitor setups, or virtual desktop workspaces.
- ✂️ **Interactive Region Selector (Snipping Tool)**: Real-time visual region selection with darkened overlay, pixel dimensions, and aspect ratio guides (16:9, 4:3, 1:1).
- 🔊 **System Audio Capture (WASAPI Loopback)**: Crystal-clear internal sound recording capturing speakers and headphones without external virtual cables.
- 🎤 **Microphone & Multi-Source Audio**: Record voice commentary independently or mixed synchronously with system audio.
- 💾 **Zero-Memory Direct-to-Disk Streaming**: Frames are encoded directly to disk via OpenCV/native pipes without ballooning RAM usage.
- ⏸️ **Synchronous Pause & Resume**: Instant pause and continuation keeping audio and video streams in lockstep.
- 🖱️ **Hardware Cursor Rendering & Halo Highlight**: Fast mouse pointer tracking with an optional translucent glowing halo for presentations and tutorials.
- 📸 **Instant High-Res Screenshots**: Capture fullscreen or region PNG snapshots in one keystroke (`F11`).
- 🎛️ **Floating Mini-Toolbar Widget**: Compact, draggable on-screen controller during active recording with live duration timer and quick-action buttons.
- ⏱️ **Animated Countdown (3.. 2.. 1..)**: Clean pre-recording visual countdown overlay.
- 📁 **Integrated Media Gallery & Manager**: In-app viewer for recorded videos and screenshots, instant playback in default media player, and folder reveal.
- 🎨 **Modern Themed Interface**: Customizable dark and light palettes powered by `ttkbootstrap` (Darkly, Superhero, Solar, Cyborg, Cosmo, Flatly, Minty).
- ⚙️ **Configurable Encoding**: Adjustable framerates (15, 24, 30, 60 FPS), audio bitrates (128k–320k), container formats (MP4, AVI, MKV), and custom hotkeys.

---

## ⌨️ Default Hotkeys

| Hotkey | Action | Description |
| :--- | :--- | :--- |
| **`F5`** | **Start Recording** | Begins screen capture (with animated 3..2..1 countdown) |
| **`F6`** | **Pause / Resume** | Instantly toggles recording state without dropping sync |
| **`F10`** | **Stop & Save** | Concludes capture, finalizes file headers, and opens gallery |
| **`F11`** | **Screenshot** | Captures active display/region to PNG immediately |
| **`ESC`** | **Cancel** | Exits region selection overlay |

*(All hotkeys are rebindable in the Settings tab)*

---

## 📁 Clean Architecture

```
screenvideo/
├── src/
│   ├── core/                  # Core engine independent of UI
│   │   ├── config.py          # Persistent JSON settings & state
│   │   ├── cursor.py          # Fast pointer capture & halo renderer
│   │   ├── history.py         # Media catalog & metadata storage
│   │   ├── hotkeys.py         # Safe global low-level keyboard listener
│   │   ├── monitors.py        # Multi-monitor enumeration & geometry
│   │   ├── recorder.py        # Multi-threaded FPS-locked video writer
│   │   └── screenshot.py      # High-performance PNG snapshot engine
│   ├── ui/                    # Presentation layer (ttkbootstrap)
│   │   ├── views/             # Functional views
│   │   │   ├── record_view.py   # Main capture & telemetry control
│   │   │   ├── history_view.py  # Gallery of saved captures
│   │   │   └── settings_view.py # Encoder & audio preferences
│   │   ├── app.py             # Main application coordinator
│   │   ├── countdown.py       # Transparent overlay countdown
│   │   ├── floating_bar.py    # Draggable mini control widget
│   │   ├── region_selector.py # Visual interactive snipping overlay
│   │   └── theme.py           # Typography, palette & styling tokens
│   └── utils/                 # System helpers & formatting
│       ├── formatting.py      # Human-readable time, bitrate & sizes
│       └── system.py          # High-DPI scaling & explorer integration
├── asm/                       # Native x64 assembly performance routines
├── tests/                     # Automated unit and integration tests
│   └── test_core.py
├── main.py                    # Application bootstrap entry point
├── pyproject.toml             # Modern package metadata & ruff config
└── LICENSE                    # Official MIT License
```

---

## 🚀 Quick Start

### Prerequisites
- **Windows 10 / 11 (64-bit)**
- **Python 3.10+** (Python 3.12+ recommended)
- Package manager: [`uv`](https://github.com/astral-sh/uv) (recommended) or `pip`

### Running from Source

```bash
# Clone the repository
git clone https://github.com/eminsk/screenvideo.git
cd screenvideo

# Synchronize dependencies with uv
uv sync

# Launch ScreenCapture Pro
uv run python main.py
```

### Running Test Suite

```bash
uv run python -m unittest discover tests
```

---

## 🛠️ Standalone Binary Build (Nuitka)

To compile ScreenCapture Pro into a self-contained, standalone Windows `.exe` without requiring a local Python installation:

```bash
uv run python -m nuitka \
    --onefile \
    --windows-console-mode=disable \
    --windows-icon-from-ico=icon.ico \
    --enable-plugin=tk-inter \
    --include-data-file=icon.ico=icon.ico \
    --include-data-file=icon.png=icon.png \
    --output-filename=ScreenCapturePro.exe \
    main.py
```

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

<details>
<summary><b>📖 Описание на русском языке (Нажмите, чтобы развернуть)</b></summary>

### Основные возможности
* **Запись всего экрана или нескольких мониторов**: поддержка мультимониторных систем (выбор конкретного монитора или виртуального рабочего стола).
* **Интерактивный селектор области (Snipping Tool)**: визуальное выделение области экрана с затемнением, подсказками и отображением точных размеров.
* **Запись системного звука (динамики / наушники)**: кристально чистый захват через Windows WASAPI Loopback.
* **Прямой стриминг на диск (Zero-Memory)**: видеокадры записываются напрямую в файл без утечек оперативной памяти.
* **Пауза и Возобновление (Pause & Resume)**: мгновенная синхронная пауза видео и звука.
* **Захват и подсветка курсора**: высокоскоростной рендеринг указателя мыши и мягкого ореола (Halo).
* **Горячие клавиши по умолчанию**: `F5` — старт, `F6` — пауза, `F10` — стоп, `F11` — скриншот, `ESC` — отмена.
* **Лицензия**: MIT.

</details>

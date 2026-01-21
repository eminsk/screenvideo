"""
Screen Recorder Application.

Professional screen recording tool with hotkey controls.
F5 - Start recording, F10 - Stop recording.
Zero-memory streaming: writes directly to disk.
"""

from __future__ import annotations

import threading
import time
import tkinter as tk
from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum, auto
from pathlib import Path

import cv2
import keyboard
import mss
import numpy as np
import ttkbootstrap as ttk
from ttkbootstrap.constants import BOTH, DISABLED, NORMAL, X


class RecordingState(Enum):
    """Recording state enumeration."""

    IDLE = auto()
    RECORDING = auto()


@dataclass
class Region:
    """Screen region for capture."""

    x: int = 0
    y: int = 0
    width: int = 0
    height: int = 0

    @property
    def is_valid(self) -> bool:
        return self.width > 0 and self.height > 0

    def to_mss_monitor(self) -> dict:
        return {"left": self.x, "top": self.y, "width": self.width, "height": self.height}


@dataclass
class RecordingConfig:
    """Configuration for screen recording."""

    fps: int = 20
    codec: str = "mp4v"
    output_dir: Path = field(default_factory=lambda: Path.cwd() / "recordings")

    def __post_init__(self) -> None:
        self.output_dir.mkdir(parents=True, exist_ok=True)


class ScreenRecorder:
    """Zero-memory screen recorder. Streams directly to disk."""

    __slots__ = (
        "_config", "_state", "_lock", "_recording_thread",
        "_start_time", "_frame_count", "_writer", "_current_file",
        "_region", "_sct"
    )

    def __init__(self, config: RecordingConfig | None = None) -> None:
        self._config = config or RecordingConfig()
        self._state = RecordingState.IDLE
        self._lock = threading.Lock()
        self._recording_thread: threading.Thread | None = None
        self._start_time: float = 0.0
        self._frame_count: int = 0
        self._writer: cv2.VideoWriter | None = None
        self._current_file: Path | None = None
        self._region: Region | None = None
        self._sct: mss.mss | None = None

    @property
    def state(self) -> RecordingState:
        return self._state

    @property
    def frame_count(self) -> int:
        with self._lock:
            return self._frame_count

    @property
    def duration(self) -> float:
        return time.time() - self._start_time if self._state == RecordingState.RECORDING else 0.0

    @property
    def current_file(self) -> Path | None:
        return self._current_file

    def set_region(self, region: Region | None) -> None:
        """Set capture region. None = full screen."""
        self._region = region

    def start(self) -> bool:
        """Start screen recording with streaming to disk."""
        if self._state == RecordingState.RECORDING:
            return False

        self._current_file = self._generate_filename()
        self._frame_count = 0
        self._state = RecordingState.RECORDING
        self._start_time = time.time()
        self._recording_thread = threading.Thread(target=self._capture_loop, daemon=True)
        self._recording_thread.start()
        return True

    def stop(self) -> Path | None:
        """Stop screen recording and finalize file."""
        if self._state != RecordingState.RECORDING:
            return None

        self._state = RecordingState.IDLE
        if self._recording_thread:
            self._recording_thread.join(timeout=3.0)
        return self._current_file

    def _capture_loop(self) -> None:
        """Main capture loop - streams frames directly to disk."""
        interval = 1.0 / self._config.fps
        self._sct = mss.mss()

        # Get capture region
        if self._region and self._region.is_valid:
            monitor = self._region.to_mss_monitor()
            width, height = self._region.width, self._region.height
        else:
            monitor = self._sct.monitors[1]  # Primary monitor
            width, height = monitor["width"], monitor["height"]

        # Initialize video writer
        fourcc = cv2.VideoWriter_fourcc(*self._config.codec)
        self._writer = cv2.VideoWriter(
            str(self._current_file), fourcc, self._config.fps, (width, height)
        )

        try:
            while self._state == RecordingState.RECORDING:
                start = time.perf_counter()

                # Capture and write directly - no storage
                img = self._sct.grab(monitor)
                frame = np.array(img, dtype=np.uint8)[:, :, :3]  # Remove alpha, keep BGR
                self._writer.write(frame)

                with self._lock:
                    self._frame_count += 1

                elapsed = time.perf_counter() - start
                sleep_time = interval - elapsed
                if sleep_time > 0:
                    time.sleep(sleep_time)
        finally:
            if self._writer:
                self._writer.release()
                self._writer = None
            if self._sct:
                self._sct.close()
                self._sct = None

    def _generate_filename(self) -> Path:
        """Generate unique filename with timestamp."""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        return self._config.output_dir / f"recording_{timestamp}.mp4"


class RegionSelector:
    """Transparent overlay for selecting screen region."""

    def __init__(self, callback):
        self._callback = callback
        self._start_x = 0
        self._start_y = 0
        self._rect_id = None

    def show(self) -> None:
        """Show fullscreen selection overlay."""
        self._root = tk.Tk()
        self._root.attributes("-fullscreen", True)
        self._root.attributes("-alpha", 0.3)
        self._root.attributes("-topmost", True)
        self._root.configure(bg="black")

        self._canvas = tk.Canvas(
            self._root, highlightthickness=0, bg="black", cursor="cross"
        )
        self._canvas.pack(fill="both", expand=True)

        # Instructions
        self._canvas.create_text(
            self._root.winfo_screenwidth() // 2,
            50,
            text="Выберите область для записи (ESC - отмена, ПКМ - весь экран)",
            fill="white",
            font=("Segoe UI", 16)
        )

        self._canvas.bind("<Button-1>", self._on_press)
        self._canvas.bind("<B1-Motion>", self._on_drag)
        self._canvas.bind("<ButtonRelease-1>", self._on_release)
        self._canvas.bind("<Button-3>", self._on_fullscreen)
        self._root.bind("<Escape>", self._on_cancel)

        self._root.mainloop()

    def _on_press(self, event) -> None:
        self._start_x = event.x
        self._start_y = event.y
        if self._rect_id:
            self._canvas.delete(self._rect_id)

    def _on_drag(self, event) -> None:
        if self._rect_id:
            self._canvas.delete(self._rect_id)
        self._rect_id = self._canvas.create_rectangle(
            self._start_x, self._start_y, event.x, event.y,
            outline="red", width=2
        )

    def _on_release(self, event) -> None:
        x1, y1 = self._start_x, self._start_y
        x2, y2 = event.x, event.y

        # Normalize coordinates
        left = min(x1, x2)
        top = min(y1, y2)
        width = abs(x2 - x1)
        height = abs(y2 - y1)

        self._root.destroy()

        if width > 10 and height > 10:
            region = Region(left, top, width, height)
            self._callback(region)
        else:
            self._callback(None)

    def _on_fullscreen(self, event) -> None:
        self._root.destroy()
        self._callback(None)

    def _on_cancel(self, event) -> None:
        self._root.destroy()
        self._callback(None)


class RecorderApp:
    """Modern screen recorder GUI application."""

    THEME = "darkly"
    WINDOW_SIZE = "400x420"
    HOTKEY_START = "f5"
    HOTKEY_STOP = "f10"

    def __init__(self) -> None:
        self._recorder = ScreenRecorder()
        self._root = ttk.Window(themename=self.THEME)
        self._status_var = ttk.StringVar(value="Ready")
        self._timer_var = ttk.StringVar(value="00:00:00")
        self._frame_var = ttk.StringVar(value="Frames: 0")
        self._region_var = ttk.StringVar(value="Region: Full Screen")
        self._timer_id: str | None = None
        self._selected_region: Region | None = None
        self._last_file: Path | None = None

        self._setup_window()
        self._create_widgets()
        self._bind_hotkeys()

    def _setup_window(self) -> None:
        """Configure main window properties."""
        self._root.title("Screen Recorder")
        self._root.geometry(self.WINDOW_SIZE)
        self._root.resizable(False, False)
        self._root.attributes("-topmost", True)
        
        # Set window icon
        import sys
        icon_path = Path(getattr(sys, '_MEIPASS', Path(__file__).parent)) / "icon.ico"
        if icon_path.exists():
            self._root.iconbitmap(str(icon_path))

    def _create_widgets(self) -> None:
        """Build UI components."""
        main_frame = ttk.Frame(self._root, padding=20)
        main_frame.pack(fill=BOTH, expand=True)

        # Title
        ttk.Label(
            main_frame,
            text="Screen Recorder",
            font=("Segoe UI", 18, "bold"),
        ).pack(pady=(0, 15))

        # Status display
        status_frame = ttk.Frame(main_frame)
        status_frame.pack(fill=X, pady=5)

        ttk.Label(status_frame, textvariable=self._status_var, font=("Segoe UI", 12)).pack()
        ttk.Label(status_frame, textvariable=self._timer_var, font=("Consolas", 24)).pack(pady=5)
        ttk.Label(status_frame, textvariable=self._frame_var, font=("Segoe UI", 10)).pack()
        ttk.Label(status_frame, textvariable=self._region_var, font=("Segoe UI", 9), bootstyle="info").pack(pady=5)

        # Region selection button
        self._btn_region = ttk.Button(
            main_frame,
            text="Select Region",
            bootstyle="secondary",
            command=self._on_select_region,
            width=32,
        )
        self._btn_region.pack(pady=5)

        # Buttons
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=X, pady=15)

        self._btn_start = ttk.Button(
            btn_frame,
            text="Start (F5)",
            bootstyle="success",
            command=self._on_start,
            width=15,
        )
        self._btn_start.pack(side="left", padx=5, expand=True)

        self._btn_stop = ttk.Button(
            btn_frame,
            text="Stop (F10)",
            bootstyle="danger",
            command=self._on_stop,
            width=15,
            state=DISABLED,
        )
        self._btn_stop.pack(side="left", padx=5, expand=True)

        # Open folder button
        self._btn_folder = ttk.Button(
            main_frame,
            text="Open Recordings Folder",
            bootstyle="info-outline",
            command=self._on_open_folder,
            width=32,
        )
        self._btn_folder.pack(pady=5)

        # Hotkey hints
        ttk.Label(
            main_frame,
            text="F5: Start | F10: Stop | Saves automatically",
            font=("Segoe UI", 9),
            bootstyle="secondary",
        ).pack(side="bottom")

    def _bind_hotkeys(self) -> None:
        """Register global hotkeys."""
        keyboard.add_hotkey(self.HOTKEY_START, lambda: self._root.after(0, self._on_start))
        keyboard.add_hotkey(self.HOTKEY_STOP, lambda: self._root.after(0, self._on_stop))
        self._root.protocol("WM_DELETE_WINDOW", self._on_close)

    def _on_select_region(self) -> None:
        """Open region selection overlay."""
        self._root.withdraw()
        self._root.after(200, self._show_region_selector)

    def _show_region_selector(self) -> None:
        """Show region selector in separate thread."""
        selector = RegionSelector(self._on_region_selected)
        selector.show()

    def _on_region_selected(self, region: Region | None) -> None:
        """Handle region selection result."""
        self._selected_region = region
        self._recorder.set_region(region)

        if region and region.is_valid:
            self._region_var.set(f"Region: {region.width}x{region.height}")
        else:
            self._region_var.set("Region: Full Screen")

        self._root.deiconify()

    def _on_start(self) -> None:
        """Handle start recording action."""
        if self._recorder.state == RecordingState.RECORDING:
            return

        self._root.withdraw()
        self._root.after(300, self._start_recording)

    def _start_recording(self) -> None:
        """Start recording after window is hidden."""
        self._recorder.start()
        self._update_ui_state(recording=True)
        self._update_timer()

    def _on_stop(self) -> None:
        """Handle stop recording action."""
        if self._recorder.state != RecordingState.RECORDING:
            return

        self._last_file = self._recorder.stop()
        if self._timer_id:
            self._root.after_cancel(self._timer_id)
        self._update_ui_state(recording=False)
        self._root.deiconify()

        if self._last_file and self._last_file.exists():
            self._status_var.set(f"Saved: {self._last_file.name}")

    def _on_open_folder(self) -> None:
        """Open recordings folder in explorer."""
        import os
        folder = RecordingConfig().output_dir
        os.startfile(str(folder))

    def _on_close(self) -> None:
        """Handle window close event."""
        if self._recorder.state == RecordingState.RECORDING:
            self._recorder.stop()
        keyboard.unhook_all()
        self._root.destroy()

    def _update_ui_state(self, *, recording: bool) -> None:
        """Update UI elements based on recording state."""
        self._btn_start.configure(state=DISABLED if recording else NORMAL)
        self._btn_stop.configure(state=NORMAL if recording else DISABLED)
        self._btn_region.configure(state=DISABLED if recording else NORMAL)
        self._status_var.set("🔴 Recording..." if recording else "⏹ Stopped")

    def _update_timer(self) -> None:
        """Update timer display during recording."""
        if self._recorder.state != RecordingState.RECORDING:
            return

        duration = int(self._recorder.duration)
        hours, remainder = divmod(duration, 3600)
        minutes, seconds = divmod(remainder, 60)

        self._timer_var.set(f"{hours:02d}:{minutes:02d}:{seconds:02d}")
        self._frame_var.set(f"Frames: {self._recorder.frame_count}")
        self._timer_id = self._root.after(100, self._update_timer)

    def _reset_display(self) -> None:
        """Reset display to initial state."""
        self._status_var.set("Ready")
        self._timer_var.set("00:00:00")
        self._frame_var.set("Frames: 0")

    def run(self) -> None:
        """Start the application main loop."""
        self._root.mainloop()


def main() -> None:
    """Application entry point."""
    app = RecorderApp()
    app.run()


if __name__ == "__main__":
    main()

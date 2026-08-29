"""Main application window and state coordinator for ScreenCapture Pro."""

from __future__ import annotations

import sys
from pathlib import Path

import ttkbootstrap as ttk
from ttkbootstrap.constants import BOTH

from src.core.config import AppConfig
from src.core.hotkeys import HotkeyManager
from src.core.monitors import Region
from src.core.recorder import RecordingState, ScreenRecorder
from src.core.screenshot import capture_screenshot
from src.ui.countdown import CountdownOverlay
from src.ui.floating_bar import FloatingBar
from src.ui.region_selector import RegionSelector
from src.ui.views.history_view import HistoryView
from src.ui.views.record_view import RecordView
from src.ui.views.settings_view import SettingsView
from src.utils.formatting import format_bytes, format_duration
from src.utils.system import enable_high_dpi, play_sound_feedback


class ScreenCaptureApp:
    """Main application window integrating recording engine, UI views, hotkeys, and overlays."""

    def __init__(self) -> None:
        enable_high_dpi()

        # 1. Load configuration & initialize recorder
        self.config = AppConfig.load()
        self.recorder = ScreenRecorder(config=self.config)

        # 2. Setup main Tk window
        self.root = ttk.Window(
            title="ScreenCapture Pro",
            themename=self.config.theme,
            minsize=(520, 580),
        )
        self.root.geometry("540x600")
        self.root.resizable(True, True)

        self._set_app_icon()

        # 3. Floating bar & hotkeys
        self._floating_bar = FloatingBar(
            parent=self.root,
            on_pause_toggle=self.toggle_pause,
            on_stop=self.stop_recording,
            on_screenshot=self.take_instant_screenshot,
            on_restore=self.restore_window,
        )

        self._hotkeys = HotkeyManager(dispatch_fn=lambda fn: self.root.after(0, fn))

        # 4. Telemetry timer ID
        self._timer_after_id: str | None = None

        # 5. Build Tabbed Interface
        self._notebook = ttk.Notebook(self.root, bootstyle="primary")
        self._notebook.pack(fill=BOTH, expand=True, padx=8, pady=(8, 4))

        self.record_view = RecordView(self._notebook, self)
        self.history_view = HistoryView(self._notebook, self)
        self.settings_view = SettingsView(self._notebook, self)

        self._notebook.add(self.record_view, text=" 🎬 Запись ")
        self._notebook.add(self.history_view, text=" 📁 Галерея ")
        self._notebook.add(self.settings_view, text=" ⚙ Настройки ")

        # 6. Status Bar at bottom
        self._status_var = ttk.StringVar(value="Готов к работе")
        lbl_status_bar = ttk.Label(
            self.root,
            textvariable=self._status_var,
            font=("Segoe UI", 9),
            bootstyle="secondary",
            padding=(10, 2),
        )
        lbl_status_bar.pack(fill="x", side="bottom")

        # 7. Bindings & Events
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        self._register_all_hotkeys()

    def _set_app_icon(self) -> None:
        """Locate and set window icon."""
        base_dir = Path(getattr(sys, "_MEIPASS", Path(__file__).resolve().parent.parent.parent))
        icon_ico = base_dir / "icon.ico"
        icon_png = base_dir / "icon.png"

        if icon_ico.exists() and sys.platform == "win32":
            try:
                self.root.iconbitmap(str(icon_ico))
                return
            except Exception:
                pass

        if icon_png.exists():
            try:
                from PIL import Image, ImageTk

                img = ImageTk.PhotoImage(Image.open(icon_png))
                self.root.iconphoto(True, img)
            except Exception:
                pass

    def _register_all_hotkeys(self) -> None:
        """Register configured global hotkeys."""
        self._hotkeys.unregister_all()
        self._hotkeys.register("start", self.config.hotkey_start, self.start_recording)
        self._hotkeys.register("pause", self.config.hotkey_pause, self.toggle_pause)
        self._hotkeys.register("stop", self.config.hotkey_stop, self.stop_recording)
        self._hotkeys.register(
            "screenshot", self.config.hotkey_screenshot, self.take_instant_screenshot
        )

    def apply_theme(self, theme_name: str) -> None:
        """Apply ttkbootstrap theme in real time."""
        try:
            self.root.style.theme_use(theme_name)
        except Exception:
            pass

    def on_settings_saved(self) -> None:
        """Called when settings are updated."""
        self.recorder.config = self.config
        self._register_all_hotkeys()
        self.record_view.update_hotkey_labels()
        self.record_view.refresh_monitors()
        self.history_view.refresh_list()
        self._status_var.set("Настройки успешно обновлены")

    def on_open_region_selector(self) -> None:
        """Launch interactive region selection overlay."""
        self.root.withdraw()
        self.root.after(150, self._show_region_selector_overlay)

    def _show_region_selector_overlay(self) -> None:
        def _on_done(region: Region | None) -> None:
            self.record_view.set_custom_region(region)
            self.root.deiconify()
            self.root.lift()

        selector = RegionSelector(self.root, _on_done)
        selector.show()

    def start_recording(self) -> None:
        """Initiate recording sequence."""
        if self.recorder.state != RecordingState.IDLE:
            return

        if self.config.show_countdown:
            self.root.withdraw()
            countdown = CountdownOverlay(
                self.root,
                seconds=self.config.countdown_seconds,
                on_finish=self._actual_start_recording,
            )
            countdown.start()
        else:
            self._actual_start_recording()

    def _actual_start_recording(self) -> None:
        """Start recording engine and update UI state."""
        if self.config.sound_effects:
            play_sound_feedback("start")

        success = self.recorder.start()
        if not success:
            self.root.deiconify()
            self._status_var.set("Не удалось запустить запись")
            return

        self.record_view.update_recording_state(RecordingState.RECORDING)
        filename = self.recorder.current_file.name if self.recorder.current_file else ""
        self._status_var.set(f"Запись началась: {filename}")

        if self.config.minimize_on_record:
            self.root.withdraw()

        if self.config.show_floating_bar:
            self._floating_bar.show()

        self._start_telemetry_loop()

    def toggle_pause(self) -> None:
        """Toggle recording pause state."""
        if self.recorder.state == RecordingState.IDLE:
            return

        is_now_paused = self.recorder.state == RecordingState.RECORDING
        self.recorder.toggle_pause()

        if self.config.sound_effects:
            play_sound_feedback("pause")

        new_state = self.recorder.state
        self.record_view.update_recording_state(new_state)

        dur_str = format_duration(self.recorder.duration)
        self._floating_bar.update_state(dur_str, is_paused=is_now_paused)
        self._status_var.set("Запись приостановлена" if is_now_paused else "Запись возобновлена")

    def stop_recording(self) -> None:
        """Stop active recording session and finalize file."""
        if self.recorder.state == RecordingState.IDLE:
            return

        if self.config.sound_effects:
            play_sound_feedback("stop")

        saved_file = self.recorder.stop()

        if self._timer_after_id:
            self.root.after_cancel(self._timer_after_id)
            self._timer_after_id = None

        self._floating_bar.hide()
        self.record_view.update_recording_state(RecordingState.IDLE)
        self.restore_window()

        self.history_view.refresh_list()

        if saved_file and saved_file.exists():
            file_sz = format_bytes(saved_file.stat().st_size)
            self._status_var.set(f"Видео сохранено: {saved_file.name} ({file_sz})")
        else:
            self._status_var.set("Запись остановлена")

    def take_instant_screenshot(self) -> None:
        """Capture screenshot immediately."""
        if self.config.sound_effects:
            play_sound_feedback("screenshot")

        region = self.record_view._selected_region
        mon_idx = self.config.monitor_index

        try:
            saved_path = capture_screenshot(
                region=region,
                output_dir=self.config.screenshots_dir,
                monitor_index=mon_idx,
                include_cursor=self.config.record_cursor,
                highlight_cursor=self.config.highlight_cursor,
            )
            self.history_view.refresh_list()
            self._status_var.set(f"Снимок сохранён: {saved_path.name}")
        except Exception as e:
            self._status_var.set(f"Ошибка сохранения снимка: {e}")

    def restore_window(self) -> None:
        """Restore and focus main window."""
        self.root.deiconify()
        self.root.lift()
        self.root.focus_force()

    def _start_telemetry_loop(self) -> None:
        """Periodic loop updating UI and mini-bar timer while recording."""
        if self.recorder.state == RecordingState.IDLE:
            return

        dur_secs = self.recorder.duration
        dur_str = format_duration(dur_secs)
        frames = self.recorder.frame_count
        fps = self.recorder.fps_actual
        sz_str = format_bytes(self.recorder.current_file_size)

        self.record_view.update_telemetry(dur_str, frames, fps, sz_str)

        is_paused = self.recorder.state == RecordingState.PAUSED
        self._floating_bar.update_state(dur_str, is_paused=is_paused)

        self._timer_after_id = self.root.after(100, self._start_telemetry_loop)

    def on_close(self) -> None:
        """Clean shutdown handler."""
        if self.recorder.state != RecordingState.IDLE:
            self.recorder.stop()

        self._floating_bar.hide()
        self._hotkeys.unregister_all()
        self.root.destroy()

    def run(self) -> None:
        """Start GUI event loop."""
        self.root.mainloop()

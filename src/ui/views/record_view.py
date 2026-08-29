"""Primary recording dashboard view."""

from __future__ import annotations

from typing import TYPE_CHECKING

import ttkbootstrap as ttk
from ttkbootstrap.constants import DISABLED, LEFT, NORMAL, RIGHT, X

from src.core.monitors import MonitorInfo, Region, get_available_monitors
from src.core.recorder import RecordingState
from src.ui.theme import Fonts

if TYPE_CHECKING:
    from src.ui.app import ScreenCaptureApp


class RecordView(ttk.Frame):
    """Main view for screen recording controls, target selection, and live telemetry."""

    def __init__(self, parent: ttk.Notebook, app: ScreenCaptureApp) -> None:
        super().__init__(parent, padding=16)
        self._app = app
        self._monitors: list[MonitorInfo] = []
        self._selected_region: Region | None = None

        self._create_widgets()
        self.refresh_monitors()

    def _create_widgets(self) -> None:
        """Construct recording UI."""
        # 1. Hero Telemetry Card
        self._card_hero = ttk.Labelframe(
            self, text=" Текущее состояние ", padding=14, bootstyle="primary"
        )
        self._card_hero.pack(fill=X, pady=(0, 10))

        # Status badge & pulse dot
        top_hero = ttk.Frame(self._card_hero)
        top_hero.pack(fill=X, pady=(0, 5))

        self._lbl_status = ttk.Label(
            top_hero,
            text="ГОТОВ К ЗАПИСИ",
            font=Fonts.TITLE_SMALL,
            bootstyle="success",
        )
        self._lbl_status.pack(side=LEFT)

        self._lbl_region_summary = ttk.Label(
            top_hero,
            text="Весь экран",
            font=Fonts.CAPTION,
            bootstyle="secondary",
        )
        self._lbl_region_summary.pack(side=RIGHT)

        # Big Timer
        self._lbl_timer = ttk.Label(
            self._card_hero,
            text="00:00:00",
            font=Fonts.HERO_TIMER,
            anchor="center",
            bootstyle="light",
        )
        self._lbl_timer.pack(fill=X, pady=4)

        # Telemetry info line
        stats_frame = ttk.Frame(self._card_hero)
        stats_frame.pack(fill=X, pady=(2, 0))

        self._lbl_frames = ttk.Label(
            stats_frame, text="Кадров: 0", font=Fonts.CAPTION, bootstyle="secondary"
        )
        self._lbl_frames.pack(side=LEFT, expand=True)

        self._lbl_fps = ttk.Label(
            stats_frame, text="FPS: 30.0", font=Fonts.CAPTION, bootstyle="secondary"
        )
        self._lbl_fps.pack(side=LEFT, expand=True)

        self._lbl_size = ttk.Label(
            stats_frame, text="Размер: 0 KB", font=Fonts.CAPTION, bootstyle="secondary"
        )
        self._lbl_size.pack(side=LEFT, expand=True)

        # 2. Capture Target Selection Card
        card_target = ttk.Labelframe(self, text=" Источник захвата ", padding=12)
        card_target.pack(fill=X, pady=(0, 10))

        mon_row = ttk.Frame(card_target)
        mon_row.pack(fill=X, pady=(0, 6))

        ttk.Label(mon_row, text="Экран / Монитор:", font=Fonts.BODY).pack(side=LEFT, padx=(0, 8))
        self._combo_monitors = ttk.Combobox(mon_row, state="readonly", width=32)
        self._combo_monitors.pack(side=LEFT, fill=X, expand=True)
        self._combo_monitors.bind("<<ComboboxSelected>>", self._on_monitor_selected)

        # Region selection buttons
        reg_row = ttk.Frame(card_target)
        reg_row.pack(fill=X, pady=2)

        self._btn_select_region = ttk.Button(
            reg_row,
            text="✂ Выбрать область",
            bootstyle="secondary",
            command=self._app.on_open_region_selector,
        )
        self._btn_select_region.pack(side=LEFT, padx=(0, 6))

        self._btn_reset_fullscreen = ttk.Button(
            reg_row,
            text="🖥 Сбросить на весь экран",
            bootstyle="secondary-outline",
            command=self._on_reset_to_fullscreen,
        )
        self._btn_reset_fullscreen.pack(side=LEFT, padx=(0, 6))

        # 3. Quick Options Card
        card_opts = ttk.Labelframe(self, text=" Быстрые опции ", padding=10)
        card_opts.pack(fill=X, pady=(0, 10))

        opts_grid = ttk.Frame(card_opts)
        opts_grid.pack(fill=X)

        self._var_cursor = ttk.BooleanVar(value=self._app.config.record_cursor)
        chk_cursor = ttk.Checkbutton(
            opts_grid,
            text="Захват курсора",
            variable=self._var_cursor,
            command=self._on_opt_changed,
            bootstyle="round-toggle",
        )
        chk_cursor.grid(row=0, column=0, sticky="w", padx=8, pady=2)

        self._var_highlight = ttk.BooleanVar(value=self._app.config.highlight_cursor)
        chk_highlight = ttk.Checkbutton(
            opts_grid,
            text="Подсветка кликов/курсора",
            variable=self._var_highlight,
            command=self._on_opt_changed,
            bootstyle="round-toggle",
        )
        chk_highlight.grid(row=0, column=1, sticky="w", padx=8, pady=2)

        self._var_countdown = ttk.BooleanVar(value=self._app.config.show_countdown)
        chk_countdown = ttk.Checkbutton(
            opts_grid,
            text="Таймер 3..2..1..",
            variable=self._var_countdown,
            command=self._on_opt_changed,
            bootstyle="round-toggle",
        )
        chk_countdown.grid(row=1, column=0, sticky="w", padx=8, pady=2)

        self._var_floating = ttk.BooleanVar(value=self._app.config.show_floating_bar)
        chk_floating = ttk.Checkbutton(
            opts_grid,
            text="Мини-панель записи",
            variable=self._var_floating,
            command=self._on_opt_changed,
            bootstyle="round-toggle",
        )
        chk_floating.grid(row=1, column=1, sticky="w", padx=8, pady=2)

        # 4. Primary Action Controls Bar
        actions_row = ttk.Frame(self)
        actions_row.pack(fill=X, pady=(4, 6))

        self._btn_start = ttk.Button(
            actions_row,
            text=f"▶ СТАРТ ({self._app.config.hotkey_start.upper()})",
            bootstyle="success",
            command=self._app.start_recording,
        )
        self._btn_start.pack(side=LEFT, fill=X, expand=True, padx=2)

        self._btn_pause = ttk.Button(
            actions_row,
            text=f"⏸ ПАУЗА ({self._app.config.hotkey_pause.upper()})",
            bootstyle="warning",
            command=self._app.toggle_pause,
            state=DISABLED,
        )
        self._btn_pause.pack(side=LEFT, fill=X, expand=True, padx=2)

        self._btn_stop = ttk.Button(
            actions_row,
            text=f"⏹ СТОП ({self._app.config.hotkey_stop.upper()})",
            bootstyle="danger",
            command=self._app.stop_recording,
            state=DISABLED,
        )
        self._btn_stop.pack(side=LEFT, fill=X, expand=True, padx=2)

        self._btn_screenshot = ttk.Button(
            actions_row,
            text=f"📸 СНИМОК ({self._app.config.hotkey_screenshot.upper()})",
            bootstyle="info-outline",
            command=self._app.take_instant_screenshot,
        )
        self._btn_screenshot.pack(side=LEFT, padx=2)

    def refresh_monitors(self) -> None:
        """Detect monitors and populate dropdown."""
        self._monitors = get_available_monitors()
        values = [m.display_text for m in self._monitors]
        self._combo_monitors["values"] = values

        # Select saved monitor
        saved_idx = self._app.config.monitor_index
        sel = 0
        for i, m in enumerate(self._monitors):
            if m.index == saved_idx:
                sel = i
                break
        if values:
            self._combo_monitors.current(sel)

    def _on_monitor_selected(self, event: object = None) -> None:
        idx = self._combo_monitors.current()
        if 0 <= idx < len(self._monitors):
            mon = self._monitors[idx]
            self._app.config.monitor_index = mon.index
            self._app.config.save()
            self._app.recorder.set_monitor_index(mon.index)
            self._selected_region = None
            self._app.recorder.set_region(None)
            self._lbl_region_summary.configure(text=f"{mon.width}×{mon.height}")

    def _on_reset_to_fullscreen(self) -> None:
        self._selected_region = None
        self._app.recorder.set_region(None)
        self._on_monitor_selected()

    def set_custom_region(self, region: Region | None) -> None:
        """Update display when custom region is chosen."""
        self._selected_region = region
        self._app.recorder.set_region(region)
        if region and region.is_valid:
            self._lbl_region_summary.configure(text=f"Область: {region.label}")
        else:
            self._on_reset_to_fullscreen()

    def _on_opt_changed(self) -> None:
        """Sync checkbutton toggles with config."""
        self._app.config.record_cursor = self._var_cursor.get()
        self._app.config.highlight_cursor = self._var_highlight.get()
        self._app.config.show_countdown = self._var_countdown.get()
        self._app.config.show_floating_bar = self._var_floating.get()
        self._app.config.save()

    def update_recording_state(self, state: RecordingState) -> None:
        """Update button states and status badges."""
        if state == RecordingState.RECORDING:
            self._lbl_status.configure(text="🔴 ИДЁТ ЗАПИСЬ...", bootstyle="danger")
            self._btn_start.configure(state=DISABLED)
            self._btn_pause.configure(
                state=NORMAL,
                text=f"⏸ ПАУЗА ({self._app.config.hotkey_pause.upper()})",
                bootstyle="warning",
            )
            self._btn_stop.configure(state=NORMAL)
            self._btn_select_region.configure(state=DISABLED)
            self._btn_reset_fullscreen.configure(state=DISABLED)
            self._combo_monitors.configure(state=DISABLED)
        elif state == RecordingState.PAUSED:
            self._lbl_status.configure(text="🟡 НА ПАУЗЕ", bootstyle="warning")
            self._btn_pause.configure(
                state=NORMAL,
                text=f"▶ ПРОДОЛЖИТЬ ({self._app.config.hotkey_pause.upper()})",
                bootstyle="success",
            )
        else:  # IDLE
            self._lbl_status.configure(text="ГОТОВ К ЗАПИСИ", bootstyle="success")
            self._btn_start.configure(state=NORMAL)
            self._btn_pause.configure(
                state=DISABLED, text=f"⏸ ПАУЗА ({self._app.config.hotkey_pause.upper()})"
            )
            self._btn_stop.configure(state=DISABLED)
            self._btn_select_region.configure(state=NORMAL)
            self._btn_reset_fullscreen.configure(state=NORMAL)
            self._combo_monitors.configure(state="readonly")

    def update_telemetry(self, duration_str: str, frames: int, fps: float, size_str: str) -> None:
        """Update live telemetry counters."""
        self._lbl_timer.configure(text=duration_str)
        self._lbl_frames.configure(text=f"Кадров: {frames}")
        self._lbl_fps.configure(text=f"FPS: {fps:.1f}")
        self._lbl_size.configure(text=f"Размер: {size_str}")

    def update_hotkey_labels(self) -> None:
        """Refresh button hotkey hints."""
        self._btn_start.configure(text=f"▶ СТАРТ ({self._app.config.hotkey_start.upper()})")
        self._btn_pause.configure(text=f"⏸ ПАУЗА ({self._app.config.hotkey_pause.upper()})")
        self._btn_stop.configure(text=f"⏹ СТОП ({self._app.config.hotkey_stop.upper()})")
        self._btn_screenshot.configure(
            text=f"📸 СНИМОК ({self._app.config.hotkey_screenshot.upper()})"
        )

"""Application settings and configuration view."""

from __future__ import annotations

import tkinter.filedialog as filedialog
import tkinter.messagebox as msgbox
from pathlib import Path
from typing import TYPE_CHECKING

import ttkbootstrap as ttk
from ttkbootstrap.constants import BOTH, LEFT, RIGHT, X

from src.core.config import AppConfig
from src.ui.theme import AVAILABLE_THEMES, Fonts

if TYPE_CHECKING:
    from src.ui.app import ScreenCaptureApp


class SettingsView(ttk.Frame):
    """Configuration interface for video parameters, storage paths, hotkeys, and appearance."""

    def __init__(self, parent: ttk.Notebook, app: ScreenCaptureApp) -> None:
        super().__init__(parent, padding=16)
        self._app = app

        self._create_widgets()
        self.load_from_config()

    def _create_widgets(self) -> None:
        """Build settings form controls."""
        canvas = ttk.Canvas(self, highlightthickness=0)
        scrollbar = ttk.Scrollbar(self, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)

        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all")),
        )
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side=LEFT, fill=BOTH, expand=True)
        scrollbar.pack(side=RIGHT, fill="y")

        # 1. Folders Card
        card_paths = ttk.Labelframe(scrollable_frame, text=" Папки сохранения ", padding=12)
        card_paths.pack(fill=X, pady=(0, 10))

        # Recordings folder
        ttk.Label(card_paths, text="Папка для видеозаписей:", font=Fonts.BODY).pack(
            anchor="w", pady=(0, 2)
        )
        row_rec = ttk.Frame(card_paths)
        row_rec.pack(fill=X, pady=(0, 8))

        self._var_rec_path = ttk.StringVar()
        ttk.Entry(row_rec, textvariable=self._var_rec_path).pack(
            side=LEFT, fill=X, expand=True, padx=(0, 6)
        )
        ttk.Button(
            row_rec, text="Обзор...", bootstyle="secondary", command=self._browse_rec_dir
        ).pack(side=RIGHT)

        # Screenshots folder
        ttk.Label(card_paths, text="Папка для снимков экрана:", font=Fonts.BODY).pack(
            anchor="w", pady=(0, 2)
        )
        row_snap = ttk.Frame(card_paths)
        row_snap.pack(fill=X)

        self._var_snap_path = ttk.StringVar()
        ttk.Entry(row_snap, textvariable=self._var_snap_path).pack(
            side=LEFT, fill=X, expand=True, padx=(0, 6)
        )
        ttk.Button(
            row_snap, text="Обзор...", bootstyle="secondary", command=self._browse_snap_dir
        ).pack(side=RIGHT)

        # 2. Video Encoding Card
        card_video = ttk.Labelframe(
            scrollable_frame, text=" Настройки видео и частоты кадров ", padding=12
        )
        card_video.pack(fill=X, pady=(0, 10))

        grid_v = ttk.Frame(card_video)
        grid_v.pack(fill=X)

        # FPS
        ttk.Label(grid_v, text="Частота кадров (FPS):", font=Fonts.BODY).grid(
            row=0, column=0, sticky="w", pady=4
        )
        self._var_fps = ttk.StringVar(value="30")
        self._combo_fps = ttk.Combobox(
            grid_v,
            textvariable=self._var_fps,
            values=["15", "24", "30", "60"],
            state="readonly",
            width=12,
        )
        self._combo_fps.grid(row=0, column=1, sticky="w", padx=10, pady=4)

        # Format
        ttk.Label(grid_v, text="Формат контейнера:", font=Fonts.BODY).grid(
            row=1, column=0, sticky="w", pady=4
        )
        self._var_format = ttk.StringVar(value="mp4")
        self._combo_format = ttk.Combobox(
            grid_v,
            textvariable=self._var_format,
            values=["mp4", "avi", "mkv"],
            state="readonly",
            width=12,
        )
        self._combo_format.grid(row=1, column=1, sticky="w", padx=10, pady=4)

        # Codec
        ttk.Label(grid_v, text="Видеокодек FourCC:", font=Fonts.BODY).grid(
            row=2, column=0, sticky="w", pady=4
        )
        self._var_codec = ttk.StringVar(value="mp4v")
        self._combo_codec = ttk.Combobox(
            grid_v,
            textvariable=self._var_codec,
            values=["mp4v", "avc1", "XVID"],
            state="readonly",
            width=12,
        )
        self._combo_codec.grid(row=2, column=1, sticky="w", padx=10, pady=4)

        # 3. Audio Recording Card
        card_audio = ttk.Labelframe(
            scrollable_frame, text=" Настройки звука (Аудио) ", padding=12
        )
        card_audio.pack(fill=X, pady=(0, 10))

        self._var_rec_sys_audio = ttk.BooleanVar(value=True)
        ttk.Checkbutton(
            card_audio,
            text="Записывать системный звук (динамики / наушники)",
            variable=self._var_rec_sys_audio,
            bootstyle="round-toggle",
        ).pack(anchor="w", pady=2)

        self._var_rec_mic = ttk.BooleanVar(value=False)
        ttk.Checkbutton(
            card_audio,
            text="Записывать звук с микрофона",
            variable=self._var_rec_mic,
            bootstyle="round-toggle",
        ).pack(anchor="w", pady=2)

        row_bitrate = ttk.Frame(card_audio)
        row_bitrate.pack(fill=X, pady=(6, 2))
        ttk.Label(row_bitrate, text="Битрейт аудио:", font=Fonts.BODY).pack(side=LEFT, padx=(0, 8))
        self._var_audio_bitrate = ttk.StringVar(value="192k")
        self._combo_audio_bitrate = ttk.Combobox(
            row_bitrate,
            textvariable=self._var_audio_bitrate,
            values=["128k", "192k", "256k", "320k"],
            state="readonly",
            width=10,
        )
        self._combo_audio_bitrate.pack(side=LEFT)

        # 4. Hotkeys Card
        card_keys = ttk.Labelframe(scrollable_frame, text=" Горячие клавиши ", padding=12)
        card_keys.pack(fill=X, pady=(0, 10))

        grid_k = ttk.Frame(card_keys)
        grid_k.pack(fill=X)

        ttk.Label(grid_k, text="Старт записи:", font=Fonts.BODY).grid(
            row=0, column=0, sticky="w", pady=3
        )
        self._var_k_start = ttk.StringVar()
        ttk.Entry(grid_k, textvariable=self._var_k_start, width=12).grid(
            row=0, column=1, sticky="w", padx=10, pady=3
        )

        ttk.Label(grid_k, text="Пауза / Продолжить:", font=Fonts.BODY).grid(
            row=1, column=0, sticky="w", pady=3
        )
        self._var_k_pause = ttk.StringVar()
        ttk.Entry(grid_k, textvariable=self._var_k_pause, width=12).grid(
            row=1, column=1, sticky="w", padx=10, pady=3
        )

        ttk.Label(grid_k, text="Стоп и сохранение:", font=Fonts.BODY).grid(
            row=2, column=0, sticky="w", pady=3
        )
        self._var_k_stop = ttk.StringVar()
        ttk.Entry(grid_k, textvariable=self._var_k_stop, width=12).grid(
            row=2, column=1, sticky="w", padx=10, pady=3
        )

        ttk.Label(grid_k, text="Снимок экрана:", font=Fonts.BODY).grid(
            row=3, column=0, sticky="w", pady=3
        )
        self._var_k_snap = ttk.StringVar()
        ttk.Entry(grid_k, textvariable=self._var_k_snap, width=12).grid(
            row=3, column=1, sticky="w", padx=10, pady=3
        )

        # 4. Appearance & Behavior Card
        card_ui = ttk.Labelframe(scrollable_frame, text=" Оформление и поведение ", padding=12)
        card_ui.pack(fill=X, pady=(0, 10))

        row_th = ttk.Frame(card_ui)
        row_th.pack(fill=X, pady=(0, 6))

        ttk.Label(row_th, text="Тема оформления:", font=Fonts.BODY).pack(side=LEFT, padx=(0, 8))
        self._combo_theme = ttk.Combobox(
            row_th,
            values=[name for _, name in AVAILABLE_THEMES],
            state="readonly",
            width=26,
        )
        self._combo_theme.pack(side=LEFT)
        self._combo_theme.bind("<<ComboboxSelected>>", self._on_theme_preview)

        self._var_minimize = ttk.BooleanVar(value=True)
        ttk.Checkbutton(
            card_ui,
            text="Сворачивать главное окно при начале записи",
            variable=self._var_minimize,
            bootstyle="round-toggle",
        ).pack(anchor="w", pady=2)

        self._var_sound = ttk.BooleanVar(value=True)
        ttk.Checkbutton(
            card_ui,
            text="Звуковые сигналы при старте / стопе / снимке",
            variable=self._var_sound,
            bootstyle="round-toggle",
        ).pack(anchor="w", pady=2)

        # Bottom Buttons
        btn_row = ttk.Frame(scrollable_frame)
        btn_row.pack(fill=X, pady=(8, 12))

        btn_save = ttk.Button(
            btn_row,
            text="💾 Сохранить настройки",
            bootstyle="success",
            command=self.save_settings,
        )
        btn_save.pack(side=LEFT, padx=(0, 6))

        btn_reset = ttk.Button(
            btn_row,
            text="🔄 По умолчанию",
            bootstyle="secondary-outline",
            command=self._reset_defaults,
        )
        btn_reset.pack(side=LEFT)

    def load_from_config(self) -> None:
        """Populate inputs from active configuration."""
        cfg = self._app.config
        self._var_rec_path.set(str(cfg.recordings_dir))
        self._var_snap_path.set(str(cfg.screenshots_dir))
        self._var_fps.set(str(cfg.fps))
        self._var_format.set(cfg.format)
        self._var_codec.set(cfg.codec)
        self._var_rec_sys_audio.set(cfg.record_system_audio)
        self._var_rec_mic.set(cfg.record_microphone)
        self._var_audio_bitrate.set(cfg.audio_bitrate)
        self._var_k_start.set(cfg.hotkey_start)
        self._var_k_pause.set(cfg.hotkey_pause)
        self._var_k_stop.set(cfg.hotkey_stop)
        self._var_k_snap.set(cfg.hotkey_screenshot)
        self._var_minimize.set(cfg.minimize_on_record)
        self._var_sound.set(cfg.sound_effects)

        # Match theme combo
        for key, display in AVAILABLE_THEMES:
            if key == cfg.theme:
                self._combo_theme.set(display)
                break

    def _browse_rec_dir(self) -> None:
        chosen = filedialog.askdirectory(initialdir=self._var_rec_path.get(), parent=self)
        if chosen:
            self._var_rec_path.set(chosen)

    def _browse_snap_dir(self) -> None:
        chosen = filedialog.askdirectory(initialdir=self._var_snap_path.get(), parent=self)
        if chosen:
            self._var_snap_path.set(chosen)

    def _on_theme_preview(self, event: object = None) -> None:
        selected_name = self._combo_theme.get()
        for key, display in AVAILABLE_THEMES:
            if display == selected_name:
                self._app.apply_theme(key)
                break

    def save_settings(self) -> None:
        """Validate and apply settings."""
        cfg = self._app.config

        cfg.recordings_dir = Path(self._var_rec_path.get())
        cfg.screenshots_dir = Path(self._var_snap_path.get())

        try:
            cfg.fps = int(self._var_fps.get())
        except ValueError:
            cfg.fps = 30

        cfg.format = self._var_format.get()
        cfg.codec = self._var_codec.get()

        cfg.record_system_audio = self._var_rec_sys_audio.get()
        cfg.record_microphone = self._var_rec_mic.get()
        cfg.audio_bitrate = self._var_audio_bitrate.get()

        cfg.hotkey_start = self._var_k_start.get().strip().lower()
        cfg.hotkey_pause = self._var_k_pause.get().strip().lower()
        cfg.hotkey_stop = self._var_k_stop.get().strip().lower()
        cfg.hotkey_screenshot = self._var_k_snap.get().strip().lower()

        cfg.minimize_on_record = self._var_minimize.get()
        cfg.sound_effects = self._var_sound.get()

        selected_theme = self._combo_theme.get()
        for key, display in AVAILABLE_THEMES:
            if display == selected_theme:
                cfg.theme = key
                break

        cfg.save()
        self._app.on_settings_saved()

        msgbox.showinfo(
            "Настройки сохранены", "Все параметры успешно применены и сохранены.", parent=self
        )

    def _reset_defaults(self) -> None:
        if msgbox.askyesno(
            "Сброс настроек", "Сбросить все параметры к значениям по умолчанию?", parent=self
        ):
            default_cfg = AppConfig()
            self._app.config = default_cfg
            default_cfg.save()
            self.load_from_config()
            self._app.apply_theme(default_cfg.theme)
            self._app.on_settings_saved()

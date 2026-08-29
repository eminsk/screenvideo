"""Floating mini-toolbar for recording controls and timer display."""

from __future__ import annotations

import tkinter as tk
from typing import Callable

import ttkbootstrap as ttk

from src.ui.theme import Fonts


class FloatingBar:
    """Sleek, draggable floating pill widget displaying recording status and quick controls."""

    def __init__(
        self,
        parent: tk.Tk,
        on_pause_toggle: Callable[[], None],
        on_stop: Callable[[], None],
        on_screenshot: Callable[[], None],
        on_restore: Callable[[], None],
    ) -> None:
        self._parent = parent
        self._on_pause_toggle = on_pause_toggle
        self._on_stop = on_stop
        self._on_screenshot = on_screenshot
        self._on_restore = on_restore

        self._top: tk.Toplevel | None = None
        self._timer_var = tk.StringVar(value="00:00")
        self._btn_pause: ttk.Button | None = None
        self._dot_label: ttk.Label | None = None
        self._pulse_state = False

        # Drag coordinates
        self._drag_x = 0
        self._drag_y = 0

    def show(self) -> None:
        """Create and show floating toolbar."""
        if self._top is not None:
            return

        self._top = tk.Toplevel(self._parent)
        self._top.overrideredirect(True)
        self._top.attributes("-topmost", True)
        self._top.configure(bg="#1a1a1a")

        # Position at top right of primary screen
        sw = self._parent.winfo_screenwidth()
        self._top.geometry(f"320x46+{sw - 360}+30")

        # Outer card container
        card = ttk.Frame(self._top, padding=(10, 4), bootstyle="dark")
        card.pack(fill="both", expand=True)

        # 1. Pulsing recording dot
        self._dot_label = ttk.Label(card, text="●", font=("Segoe UI", 14), foreground="#e74c3c")
        self._dot_label.pack(side="left", padx=(2, 6))

        # 2. Monospace Timer
        lbl_timer = ttk.Label(
            card,
            textvariable=self._timer_var,
            font=Fonts.FLOATING_TIMER,
            foreground="#ffffff",
        )
        lbl_timer.pack(side="left", padx=(0, 10))

        # 3. Control buttons
        self._btn_pause = ttk.Button(
            card,
            text="⏸",
            bootstyle="warning-outline",
            width=3,
            command=self._on_pause_toggle,
        )
        self._btn_pause.pack(side="left", padx=2)

        btn_stop = ttk.Button(
            card,
            text="⏹",
            bootstyle="danger",
            width=3,
            command=self._on_stop,
        )
        btn_stop.pack(side="left", padx=2)

        btn_snap = ttk.Button(
            card,
            text="📸",
            bootstyle="info-outline",
            width=3,
            command=self._on_screenshot,
        )
        btn_snap.pack(side="left", padx=2)

        btn_expand = ttk.Button(
            card,
            text="🗖",
            bootstyle="secondary-outline",
            width=3,
            command=self._on_restore,
        )
        btn_expand.pack(side="left", padx=(6, 2))

        # Draggable bindings
        for widget in (self._top, card, self._dot_label, lbl_timer):
            widget.bind("<ButtonPress-1>", self._start_drag)
            widget.bind("<B1-Motion>", self._on_drag)

    def _start_drag(self, event: tk.Event) -> None:
        self._drag_x = event.x
        self._drag_y = event.y

    def _on_drag(self, event: tk.Event) -> None:
        if not self._top:
            return
        x = self._top.winfo_x() + (event.x - self._drag_x)
        y = self._top.winfo_y() + (event.y - self._drag_y)
        self._top.geometry(f"+{x}+{y}")

    def update_state(self, duration_str: str, is_paused: bool) -> None:
        """Update live timer and pause/recording appearance."""
        if not self._top:
            return

        self._timer_var.set(duration_str)

        if self._btn_pause:
            self._btn_pause.configure(
                text="▶" if is_paused else "⏸",
                bootstyle="success-outline" if is_paused else "warning-outline",
            )

        if self._dot_label:
            if is_paused:
                self._dot_label.configure(foreground="#f39c12", text="⏸")
            else:
                self._pulse_state = not self._pulse_state
                color = "#e74c3c" if self._pulse_state else "#882222"
                self._dot_label.configure(foreground=color, text="●")

    def hide(self) -> None:
        """Close and destroy floating toolbar."""
        if self._top:
            try:
                self._top.destroy()
            except Exception:
                pass
            self._top = None

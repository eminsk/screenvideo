"""Animated 3-2-1 countdown overlay prior to recording."""

from __future__ import annotations

import tkinter as tk
from typing import Callable

from src.ui.theme import Colors, Fonts


class CountdownOverlay:
    """Displays a large, animated countdown overlay (3.. 2.. 1..) before recording starts."""

    def __init__(
        self, parent: tk.Tk, seconds: int = 3, on_finish: Callable[[], None] | None = None
    ) -> None:
        self._parent = parent
        self._total_seconds = max(1, seconds)
        self._current_second = self._total_seconds
        self._on_finish = on_finish
        self._top: tk.Toplevel | None = None
        self._label: tk.Label | None = None

    def start(self) -> None:
        """Show overlay and start countdown timer."""
        self._top = tk.Toplevel(self._parent)
        self._top.attributes("-fullscreen", True)
        self._top.attributes("-topmost", True)
        self._top.attributes("-alpha", 0.7)
        self._top.configure(bg="#000000")

        self._label = tk.Label(
            self._top,
            text=str(self._current_second),
            font=Fonts.COUNTDOWN,
            fg=Colors.SUCCESS,
            bg="#000000",
        )
        self._label.pack(expand=True)

        self._tick()

    def _tick(self) -> None:
        if self._current_second > 0:
            if self._label:
                self._label.configure(text=str(self._current_second))
            self._current_second -= 1
            self._parent.after(800, self._tick)
        else:
            if self._label:
                self._label.configure(text="🎬 ЗАПИСЬ!", fg="#ffffff")
            self._parent.after(400, self._finish)

    def _finish(self) -> None:
        if self._top:
            try:
                self._top.destroy()
            except Exception:
                pass
            self._top = None

        if self._on_finish:
            self._on_finish()

"""Animated 3-2-1 countdown overlay prior to recording."""

from __future__ import annotations

import tkinter as tk
from typing import Callable

from src.ui.theme import Colors, Fonts


class CountdownOverlay:
    """Displays a large, animated countdown overlay (3.. 2.. 1..) before recording starts."""

    def __init__(
        self,
        parent: tk.Tk,
        seconds: int = 3,
        on_finish: Callable[[], None] | None = None,
        on_cancel: Callable[[], None] | None = None,
    ) -> None:
        self._parent = parent
        self._total_seconds = max(1, seconds)
        self._current_second = self._total_seconds
        self._on_finish = on_finish
        self._on_cancel = on_cancel
        self._top: tk.Toplevel | None = None
        self._label: tk.Label | None = None
        self._after_id: str | None = None
        self._cancelled: bool = False

    def start(self) -> None:
        """Show overlay and start countdown timer."""
        self._cancelled = False
        self._top = tk.Toplevel(self._parent)
        self._top.attributes("-fullscreen", True)
        self._top.attributes("-topmost", True)
        self._top.attributes("-alpha", 0.7)
        self._top.configure(bg="#000000")

        self._top.bind("<Escape>", lambda e: self.cancel())

        self._label = tk.Label(
            self._top,
            text=str(self._current_second),
            font=Fonts.COUNTDOWN,
            fg=Colors.SUCCESS,
            bg="#000000",
        )
        self._label.pack(expand=True)

        hint = tk.Label(
            self._top,
            text="Нажмите ESC для отмены",
            font=Fonts.CAPTION,
            fg="#888888",
            bg="#000000",
        )
        hint.pack(side="bottom", pady=20)

        self._tick()

    def _tick(self) -> None:
        if self._cancelled:
            return

        if self._current_second > 0:
            if self._label:
                self._label.configure(text=str(self._current_second))
            self._current_second -= 1
            self._after_id = self._parent.after(800, self._tick)
        else:
            if self._label:
                self._label.configure(text="🎬 ЗАПИСЬ!", fg="#ffffff")
            self._after_id = self._parent.after(400, self._finish)

    def cancel(self) -> None:
        """Cancel active countdown and restore state."""
        if self._cancelled:
            return
        self._cancelled = True

        if self._after_id:
            try:
                self._parent.after_cancel(self._after_id)
            except Exception:
                pass
            self._after_id = None

        if self._top:
            try:
                self._top.destroy()
            except Exception:
                pass
            self._top = None

        if self._on_cancel:
            self._on_cancel()

    def _finish(self) -> None:
        if self._cancelled:
            return

        if self._top:
            try:
                self._top.destroy()
            except Exception:
                pass
            self._top = None

        if self._on_finish:
            self._on_finish()

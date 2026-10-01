"""Interactive screen region selector with dimmed overlay, live HUD, and preset ratios."""

from __future__ import annotations

import tkinter as tk
from typing import Callable

import mss

from src.core.monitors import Region
from src.ui.theme import Colors, Fonts


class RegionSelector:
    """Interactive snipping tool overlay for selecting a capture region."""

    def __init__(
        self,
        parent: tk.Tk,
        on_selected: Callable[[Region | None], None],
        mode: str = "record",  # "record" or "snip"
    ) -> None:
        self._parent = parent
        self._callback = on_selected
        self._mode = mode

        self._start_x = 0
        self._start_y = 0
        self._curr_x = 0
        self._curr_y = 0
        self._is_dragging = False

        self._top: tk.Toplevel | None = None
        self._canvas: tk.Canvas | None = None

        # Virtual desktop geometry
        with mss.mss() as sct:
            v_mon = sct.monitors[0]
            self._screen_x = v_mon["left"]
            self._screen_y = v_mon["top"]
            self._screen_w = v_mon["width"]
            self._screen_h = v_mon["height"]

    def show(self) -> None:
        """Create and display fullscreen semi-transparent overlay."""
        self._top = tk.Toplevel(self._parent)
        self._top.attributes("-fullscreen", True)
        self._top.attributes("-topmost", True)
        self._top.attributes("-alpha", 0.75)
        self._top.configure(bg="#111111")

        # Geometry covering all monitors
        self._top.geometry(f"{self._screen_w}x{self._screen_h}+{self._screen_x}+{self._screen_y}")

        self._canvas = tk.Canvas(
            self._top,
            bg="#111111",
            highlightthickness=0,
            cursor="crosshair",
        )
        self._canvas.pack(fill="both", expand=True)

        self._draw_initial_ui()

        # Bindings
        self._canvas.bind("<ButtonPress-1>", self._on_press)
        self._canvas.bind("<B1-Motion>", self._on_drag)
        self._canvas.bind("<ButtonRelease-1>", self._on_release)
        self._canvas.bind("<Button-3>", self._on_right_click)
        self._top.bind("<Escape>", lambda e: self._cancel())

        # Grab focus
        self._top.focus_set()

    def _draw_initial_ui(self) -> None:
        """Draw initial helper text and instructions banner."""
        if not self._canvas:
            return

        cx = self._screen_w // 2

        if self._mode == "snip":
            title_text = "✂ Выделите область экрана для моментального снимка"
            sub_text = "ЛКМ + Потянуть: Выбор области | ПКМ: Весь экран в буфер | ESC: Отмена"
            accent_color = Colors.INFO
        else:
            title_text = "🎬 Выделите область экрана для записи видео"
            sub_text = "ЛКМ + Потянуть: Выбор области | ПКМ: Весь экран | ESC: Отмена"
            accent_color = "#444444"

        # Header instructions banner
        self._canvas.create_rectangle(
            cx - 390, 20, cx + 390, 82, fill="#1c1c1c", outline=accent_color, width=2
        )
        self._canvas.create_text(
            cx,
            41,
            text=title_text,
            fill="#ffffff",
            font=Fonts.TITLE_SMALL,
        )
        self._canvas.create_text(
            cx, 63, text=sub_text, fill="#b0b0b0", font=Fonts.CAPTION
        )

    def _on_right_click(self, event: tk.Event) -> None:
        """Right click triggers fullscreen selection or snip."""
        if self._mode == "snip":
            # In snip mode, right-click captures the entire screen
            fullscreen_region = Region(
                x=self._screen_x, y=self._screen_y, width=self._screen_w, height=self._screen_h
            )
            self._finish(fullscreen_region)
        else:
            self._finish(None)

    def _on_press(self, event: tk.Event) -> None:
        self._start_x = event.x
        self._start_y = event.y
        self._curr_x = event.x
        self._curr_y = event.y
        self._is_dragging = True
        self._redraw_selection()

    def _on_drag(self, event: tk.Event) -> None:
        if not self._is_dragging:
            return
        self._curr_x = event.x
        self._curr_y = event.y
        self._redraw_selection()

    def _redraw_selection(self) -> None:
        """Redraw the dimmed mask, selection rectangle and live dimension HUD."""
        if not self._canvas:
            return

        self._canvas.delete("selection_elements")

        x1 = min(self._start_x, self._curr_x)
        y1 = min(self._start_y, self._curr_y)
        x2 = max(self._start_x, self._curr_x)
        y2 = max(self._start_y, self._curr_y)

        width = x2 - x1
        height = y2 - y1

        # Accent color depending on mode
        accent = Colors.INFO if self._mode == "snip" else Colors.SUCCESS

        # 1. Clear selection box area (highlight with accent border)
        self._canvas.create_rectangle(
            x1,
            y1,
            x2,
            y2,
            outline=accent,
            width=2,
            fill="#000000",
            stipple="gray25",
            tags="selection_elements",
        )

        # 2. Corner indicators
        for cx, cy in [(x1, y1), (x2, y1), (x1, y2), (x2, y2)]:
            self._canvas.create_rectangle(
                cx - 3,
                cy - 3,
                cx + 3,
                cy + 3,
                fill=accent,
                outline="#ffffff",
                tags="selection_elements",
            )

        # 3. Live dimensions HUD badge
        if width > 30 and height > 20:
            hud_text = f"✂ {width} × {height} px (в буфер)" if self._mode == "snip" else f"🎬 {width} × {height} px"
            hud_x = x1 + 10
            hud_y = y1 - 25 if y1 > 35 else y1 + 15

            badge_w = len(hud_text) * 8 + 12
            self._canvas.create_rectangle(
                hud_x - 5,
                hud_y - 12,
                hud_x + badge_w,
                hud_y + 12,
                fill="#181818",
                outline=accent,
                tags="selection_elements",
            )
            self._canvas.create_text(
                hud_x + (badge_w // 2) - 4,
                hud_y,
                text=hud_text,
                fill="#ffffff",
                font=Fonts.BODY_BOLD,
                tags="selection_elements",
            )

    def _on_release(self, event: tk.Event) -> None:
        self._is_dragging = False
        x1 = min(self._start_x, event.x)
        y1 = min(self._start_y, event.y)
        x2 = max(self._start_x, event.x)
        y2 = max(self._start_y, event.y)

        width = x2 - x1
        height = y2 - y1

        if width >= 20 and height >= 20:
            # Add screen offset if multi-monitor
            global_x = self._screen_x + x1
            global_y = self._screen_y + y1
            selected_region = Region(
                x=global_x, y=global_y, width=width, height=height
            ).normalized()
            self._finish(selected_region)
        else:
            self._finish(None)

    def _cancel(self) -> None:
        self._finish(None)

    def _finish(self, region: Region | None) -> None:
        """Close overlay and trigger callback."""
        if self._top:
            try:
                self._top.destroy()
            except Exception:
                pass
            self._top = None
        self._callback(region)

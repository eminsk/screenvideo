"""High-performance Windows cursor capture and overlay renderer with click visualizer."""

from __future__ import annotations

import ctypes
import sys
import time
from ctypes import wintypes
from dataclasses import dataclass

import cv2
import numpy as np


def hex_to_bgr(hex_str: str, default: tuple[int, int, int] = (0, 215, 255)) -> tuple[int, int, int]:
    """Convert hex color string (#RRGGBB) to OpenCV BGR tuple."""
    s = hex_str.lstrip("#")
    if len(s) == 6:
        try:
            r = int(s[0:2], 16)
            g = int(s[2:4], 16)
            b = int(s[4:6], 16)
            return (b, g, r)
        except ValueError:
            pass
    return default


@dataclass
class ClickRipple:
    """Active click ripple animation state."""

    x: int
    y: int
    start_time: float
    color_bgr: tuple[int, int, int]
    max_radius: int = 32
    duration: float = 0.35


class ClickRippleManager:
    """Tracks mouse button presses and maintains expanding ripple animations."""

    def __init__(self) -> None:
        self.ripples: list[ClickRipple] = []
        self._left_down = False
        self._right_down = False

    def check_mouse_events(
        self,
        cursor_pos: tuple[int, int] | None,
        color_left: tuple[int, int, int],
        color_right: tuple[int, int, int],
    ) -> None:
        """Poll Win32 mouse button state and trigger ripples on press events."""
        if sys.platform != "win32" or cursor_pos is None:
            return

        now = time.perf_counter()

        try:
            # 0x01: VK_LBUTTON, 0x02: VK_RBUTTON
            # Most significant bit indicates button is currently down
            l_state = bool(ctypes.windll.user32.GetAsyncKeyState(0x01) & 0x8000)
            r_state = bool(ctypes.windll.user32.GetAsyncKeyState(0x02) & 0x8000)

            if l_state and not self._left_down:
                self.ripples.append(
                    ClickRipple(
                        x=cursor_pos[0],
                        y=cursor_pos[1],
                        start_time=now,
                        color_bgr=color_left,
                    )
                )
            if r_state and not self._right_down:
                self.ripples.append(
                    ClickRipple(
                        x=cursor_pos[0],
                        y=cursor_pos[1],
                        start_time=now,
                        color_bgr=color_right,
                    )
                )

            self._left_down = l_state
            self._right_down = r_state
        except Exception:
            pass

    def prune_expired(self, now: float) -> None:
        """Remove finished ripple animations."""
        self.ripples = [r for r in self.ripples if now - r.start_time < r.duration]

    def render(self, frame: np.ndarray, offset_x: int, offset_y: int) -> None:
        """Render all active ripples onto captured frame in-place."""
        now = time.perf_counter()
        self.prune_expired(now)

        h, w = frame.shape[:2]
        for ripple in self.ripples:
            elapsed = now - ripple.start_time
            t = min(1.0, max(0.0, elapsed / ripple.duration))

            # Expanding radius from 6px to max_radius
            current_radius = int(6 + (ripple.max_radius - 6) * t)
            # Fading ring thickness
            thickness = max(1, int(3 * (1.0 - t * 0.7)))

            rx = ripple.x - offset_x
            ry = ripple.y - offset_y

            # Only draw if within visible frame boundary
            in_x = -current_radius <= rx < w + current_radius
            in_y = -current_radius <= ry < h + current_radius
            if in_x and in_y:
                cv2.circle(
                    frame,
                    (rx, ry),
                    current_radius,
                    ripple.color_bgr,
                    thickness=thickness,
                    lineType=cv2.LINE_AA,
                )


class CursorRenderer:
    """Renders mouse cursor, glowing highlight, and click ripples directly onto frame."""

    # Arrow polygon vertices relative to cursor hotspot (0, 0)
    _ARROW_POINTS = np.array(
        [[0, 0], [0, 16], [4, 12], [8, 20], [11, 19], [7, 11], [13, 11]],
        dtype=np.int32,
    )

    def __init__(self) -> None:
        self._pt = wintypes.POINT() if sys.platform == "win32" else None
        self.ripple_manager = ClickRippleManager()

    def get_cursor_pos(self) -> tuple[int, int] | None:
        """Get global screen coordinates of mouse cursor."""
        if sys.platform != "win32" or self._pt is None:
            return None
        try:
            if ctypes.windll.user32.GetCursorPos(ctypes.byref(self._pt)):
                return int(self._pt.x), int(self._pt.y)
        except Exception:
            pass
        return None

    def render(
        self,
        frame: np.ndarray,
        offset_x: int,
        offset_y: int,
        *,
        highlight: bool = True,
        highlight_color_bgr: tuple[int, int, int] = (0, 215, 255),
        visualize_clicks: bool = True,
        click_color_left: tuple[int, int, int] = (255, 229, 0),
        click_color_right: tuple[int, int, int] = (82, 82, 255),
    ) -> None:
        """
        Draw cursor arrow, translucent highlight ring, and click ripples onto frame in-place.
        offset_x, offset_y: The top-left coordinates of the captured region on screen.
        """
        pos = self.get_cursor_pos()

        # 1. Update and render click ripples
        if visualize_clicks:
            self.ripple_manager.check_mouse_events(pos, click_color_left, click_color_right)
            self.ripple_manager.render(frame, offset_x, offset_y)

        if pos is None:
            return

        cx = pos[0] - offset_x
        cy = pos[1] - offset_y

        h, w = frame.shape[:2]

        # Check if cursor is inside captured frame
        if cx < -25 or cy < -25 or cx >= w + 25 or cy >= h + 25:
            return

        # 2. Draw glowing highlight halo
        if highlight and 0 <= cx < w and 0 <= cy < h:
            radius = 18
            x1 = max(0, cx - radius)
            y1 = max(0, cy - radius)
            x2 = min(w, cx + radius + 1)
            y2 = min(h, cy + radius + 1)

            if x2 > x1 and y2 > y1:
                roi = frame[y1:y2, x1:x2]
                overlay = roi.copy()
                cv2.circle(
                    overlay,
                    (cx - x1, cy - y1),
                    radius,
                    highlight_color_bgr,
                    thickness=-1,
                    lineType=cv2.LINE_AA,
                )
                # Alpha blend halo
                blended = cv2.addWeighted(overlay, 0.35, roi, 0.65, 0)
                frame[y1:y2, x1:x2] = blended
                # Outer ring border
                cv2.circle(
                    frame,
                    (cx, cy),
                    radius,
                    highlight_color_bgr,
                    thickness=1,
                    lineType=cv2.LINE_AA,
                )

        # 3. Draw crisp cursor arrow
        if 0 <= cx < w and 0 <= cy < h:
            arrow = self._ARROW_POINTS + [cx, cy]
            # White body
            cv2.fillPoly(frame, [arrow], color=(255, 255, 255), lineType=cv2.LINE_AA)
            # Black crisp border
            cv2.polylines(
                frame,
                [arrow],
                isClosed=True,
                color=(0, 0, 0),
                thickness=1,
                lineType=cv2.LINE_AA,
            )

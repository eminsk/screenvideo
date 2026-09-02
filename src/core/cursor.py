"""High-performance Windows cursor capture and overlay renderer."""

from __future__ import annotations

import ctypes
import sys
from ctypes import wintypes

import cv2
import numpy as np


class CursorRenderer:
    """Renders mouse cursor and glowing highlight directly onto captured frame."""

    # Arrow polygon vertices relative to cursor hotspot (0, 0)
    _ARROW_POINTS = np.array(
        [[0, 0], [0, 16], [4, 12], [8, 20], [11, 19], [7, 11], [13, 11]],
        dtype=np.int32,
    )

    def __init__(self) -> None:
        self._pt = wintypes.POINT() if sys.platform == "win32" else None

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
    ) -> None:
        """
        Draw cursor arrow and optional translucent highlight ring onto frame in-place.
        offset_x, offset_y: The top-left coordinates of the captured region on screen.
        """
        pos = self.get_cursor_pos()
        if pos is None:
            return

        cx = pos[0] - offset_x
        cy = pos[1] - offset_y

        h, w = frame.shape[:2]

        # Check if cursor is inside captured frame
        if cx < -25 or cy < -25 or cx >= w + 25 or cy >= h + 25:
            return

        # 1. Draw glowing highlight halo
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

        # 2. Draw crisp cursor arrow
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

"""High-quality instant screenshot capture engine."""

from __future__ import annotations

from datetime import datetime
from pathlib import Path

import cv2
import mss
import numpy as np

from src.core.cursor import CursorRenderer
from src.core.monitors import Region


def capture_screenshot(
    region: Region | None,
    output_dir: Path,
    *,
    monitor_index: int = 1,
    include_cursor: bool = True,
    highlight_cursor: bool = False,
) -> Path:
    """
    Capture instant screenshot to PNG file.
    Returns path to saved image file.
    """
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    file_path = output_dir / f"screenshot_{timestamp}.png"

    with mss.mss() as sct:
        if region and region.is_valid:
            norm_region = region.normalized()
            monitor = norm_region.to_mss_monitor()
            offset_x = norm_region.x
            offset_y = norm_region.y
        else:
            mon_idx = min(monitor_index, len(sct.monitors) - 1)
            monitor = sct.monitors[mon_idx]
            offset_x = monitor["left"]
            offset_y = monitor["top"]

        raw_img = sct.grab(monitor)
        # Convert BGRA to BGR numpy array
        frame = cv2.cvtColor(np.asarray(raw_img, dtype=np.uint8), cv2.COLOR_BGRA2BGR)

        if include_cursor:
            renderer = CursorRenderer()
            renderer.render(
                frame,
                offset_x=offset_x,
                offset_y=offset_y,
                highlight=highlight_cursor,
            )

        cv2.imwrite(str(file_path), frame, [cv2.IMWRITE_PNG_COMPRESSION, 4])

    return file_path

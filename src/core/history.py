"""History and file management for recordings and screenshots."""

from __future__ import annotations

import os
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

import cv2

from src.utils.formatting import format_bytes, format_duration


@dataclass
class MediaItem:
    """Represents a recorded video or captured screenshot file."""

    path: Path
    filename: str
    file_type: str  # "video" or "image"
    size_bytes: int
    created_at: datetime
    duration_secs: float = 0.0
    resolution: str = ""

    @property
    def formatted_size(self) -> str:
        return format_bytes(self.size_bytes)

    @property
    def formatted_date(self) -> str:
        return self.created_at.strftime("%d.%m.%Y %H:%M")

    @property
    def formatted_duration(self) -> str:
        if self.file_type == "image":
            return "Снимок"
        return format_duration(self.duration_secs)


class HistoryManager:
    """Manages files in recordings and screenshots folders."""

    VIDEO_EXTENSIONS = {".mp4", ".avi", ".mkv", ".mov"}
    IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".bmp"}

    def __init__(self, recordings_dir: Path, screenshots_dir: Path) -> None:
        self._recordings_dir = Path(recordings_dir)
        self._screenshots_dir = Path(screenshots_dir)

    def scan_items(self) -> list[MediaItem]:
        """Scan directory and return sorted list of media items (newest first)."""
        items: list[MediaItem] = []
        paths_to_scan = [self._recordings_dir, self._screenshots_dir]

        for folder in paths_to_scan:
            if not folder.exists():
                continue
            for file in folder.iterdir():
                if not file.is_file():
                    continue

                ext = file.suffix.lower()
                if ext in self.VIDEO_EXTENSIONS:
                    item = self._create_video_item(file)
                    if item:
                        items.append(item)
                elif ext in self.IMAGE_EXTENSIONS:
                    item = self._create_image_item(file)
                    if item:
                        items.append(item)

        items.sort(key=lambda x: x.created_at, reverse=True)
        return items

    def _create_video_item(self, file_path: Path) -> MediaItem | None:
        """Extract video metadata."""
        try:
            stat = file_path.stat()
            created_at = datetime.fromtimestamp(stat.st_mtime)
            size = stat.st_size

            duration = 0.0
            resolution = ""

            # Try to read fast video metadata with OpenCV
            cap = cv2.VideoCapture(str(file_path))
            if cap.isOpened():
                fps = cap.get(cv2.CAP_PROP_FPS) or 30.0
                frame_count = cap.get(cv2.CAP_PROP_FRAME_COUNT)
                width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
                height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
                if frame_count > 0 and fps > 0:
                    duration = frame_count / fps
                if width > 0 and height > 0:
                    resolution = f"{width}×{height}"
                cap.release()

            return MediaItem(
                path=file_path,
                filename=file_path.name,
                file_type="video",
                size_bytes=size,
                created_at=created_at,
                duration_secs=duration,
                resolution=resolution,
            )
        except Exception:
            return None

    def _create_image_item(self, file_path: Path) -> MediaItem | None:
        """Extract image metadata."""
        try:
            stat = file_path.stat()
            created_at = datetime.fromtimestamp(stat.st_mtime)
            size = stat.st_size
            return MediaItem(
                path=file_path,
                filename=file_path.name,
                file_type="image",
                size_bytes=size,
                created_at=created_at,
            )
        except Exception:
            return None

    def delete_item(self, path: Path) -> bool:
        """Delete media file from disk."""
        try:
            if path.exists():
                os.remove(path)
                return True
        except Exception:
            pass
        return False

"""Formatting helper functions for human-readable numbers, times, and sizes."""

from __future__ import annotations

from datetime import datetime


def format_duration(seconds: float) -> str:
    """Format duration in seconds to HH:MM:SS format."""
    total_seconds = max(0, int(seconds))
    hours, remainder = divmod(total_seconds, 3600)
    minutes, secs = divmod(remainder, 60)
    if hours > 0:
        return f"{hours:02d}:{minutes:02d}:{secs:02d}"
    return f"{minutes:02d}:{secs:02d}"


def format_duration_full(seconds: float) -> str:
    """Format duration in seconds to HH:MM:SS.ms format."""
    total_secs = max(0.0, seconds)
    hours = int(total_secs // 3600)
    minutes = int((total_secs % 3600) // 60)
    secs = int(total_secs % 60)
    ms = int((total_secs - int(total_secs)) * 10)
    return f"{hours:02d}:{minutes:02d}:{secs:02d}.{ms}"


def format_bytes(bytes_count: int | float) -> str:
    """Format byte size to human readable string (KB, MB, GB)."""
    count = float(max(0, bytes_count))
    units = ["B", "KB", "MB", "GB", "TB"]
    unit_index = 0
    while count >= 1024.0 and unit_index < len(units) - 1:
        count /= 1024.0
        unit_index += 1
    if unit_index == 0:
        return f"{int(count)} {units[unit_index]}"
    return f"{count:.1f} {units[unit_index]}"


def format_timestamp(dt: datetime | None = None) -> str:
    """Format datetime for user display."""
    if dt is None:
        dt = datetime.now()
    return dt.strftime("%d.%m.%Y %H:%M:%S")

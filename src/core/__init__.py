"""Core application logic, recorder engine, and configuration."""

from src.core.config import AppConfig
from src.core.cursor import CursorRenderer
from src.core.history import HistoryManager, MediaItem
from src.core.hotkeys import HotkeyManager
from src.core.monitors import MonitorInfo, Region, get_available_monitors
from src.core.recorder import RecordingState, ScreenRecorder
from src.core.screenshot import capture_screenshot

__all__ = [
    "AppConfig",
    "CursorRenderer",
    "HistoryManager",
    "HotkeyManager",
    "MediaItem",
    "MonitorInfo",
    "Region",
    "RecordingState",
    "ScreenRecorder",
    "capture_screenshot",
    "get_available_monitors",
]

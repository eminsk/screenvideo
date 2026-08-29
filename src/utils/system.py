"""System-level integrations and Windows utilities."""

from __future__ import annotations

import ctypes
import os
import subprocess
import sys
from pathlib import Path


def enable_high_dpi() -> None:
    """Enable Windows Per-Monitor DPI Awareness for crisp text and UI rendering."""
    if sys.platform == "win32":
        try:
            # PROCESS_PER_MONITOR_DPI_AWARE = 2
            ctypes.windll.shcore.SetProcessDpiAwareness(2)
        except Exception:
            try:
                ctypes.windll.user32.SetProcessDPIAware()
            except Exception:
                pass


def open_in_default_app(file_path: Path | str) -> bool:
    """Open file in default system application."""
    path = Path(file_path).resolve()
    if not path.exists():
        return False

    try:
        if sys.platform == "win32":
            os.startfile(str(path))
            return True
        elif sys.platform == "darwin":
            subprocess.run(["open", str(path)], check=False)
            return True
        else:
            subprocess.run(["xdg-open", str(path)], check=False)
            return True
    except Exception:
        return False


def reveal_in_explorer(file_path: Path | str) -> bool:
    """Open folder in Windows Explorer and highlight the selected file."""
    path = Path(file_path).resolve()
    if not path.exists():
        return False

    try:
        if sys.platform == "win32":
            subprocess.run(["explorer", f"/select,{path}"], check=False)
            return True
        else:
            return open_in_default_app(path.parent)
    except Exception:
        return False


def play_sound_feedback(kind: str = "start") -> None:
    """Play brief system sound feedback for start/stop/screenshot."""
    if sys.platform == "win32":
        try:
            import winsound

            if kind == "start":
                winsound.Beep(880, 80)
            elif kind == "stop":
                winsound.Beep(587, 120)
            elif kind == "pause":
                winsound.Beep(659, 70)
            elif kind == "screenshot":
                winsound.Beep(1175, 90)
        except Exception:
            pass

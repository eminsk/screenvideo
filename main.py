"""ScreenCapture Pro - Professional Screen Recorder for Windows.

High-performance zero-memory screen recording with custom regions,
multi-monitor support, pause/resume, cursor capture, instant screenshots, and global hotkeys.
"""

from __future__ import annotations

import sys

from src.ui.app import ScreenCaptureApp


def main() -> None:
    """Application entry point."""
    try:
        app = ScreenCaptureApp()
        app.run()
    except KeyboardInterrupt:
        sys.exit(0)


if __name__ == "__main__":
    main()

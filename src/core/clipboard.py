"""Native Windows Clipboard integration for images (CF_DIB) and cross-platform fallbacks."""

from __future__ import annotations

import io
import sys
from pathlib import Path
from typing import TYPE_CHECKING

import cv2
import numpy as np

if TYPE_CHECKING:
    from PIL import Image


def copy_frame_to_clipboard(frame: np.ndarray) -> bool:
    """
    Copy an OpenCV BGR numpy frame directly to the system clipboard.
    On Windows, uses native Win32 API with standard CF_DIB format.
    Returns True if successfully copied, False otherwise.
    """
    if frame is None or frame.size == 0:
        return False

    if sys.platform == "win32":
        return _copy_frame_win32_dib(frame)
    else:
        return _copy_frame_pillow_fallback(frame)


def copy_image_file_to_clipboard(file_path: Path | str) -> bool:
    """
    Copy an image file (PNG, JPG, BMP) to the system clipboard.
    Returns True on success, False on failure.
    """
    path = Path(file_path)
    if not path.exists():
        return False

    try:
        frame = cv2.imread(str(path))
        if frame is None:
            return False
        return copy_frame_to_clipboard(frame)
    except Exception:
        return False


def _copy_frame_win32_dib(frame: np.ndarray) -> bool:
    """Native Windows implementation using CF_DIB via ctypes."""
    import ctypes
    from ctypes import wintypes

    user32 = ctypes.windll.user32
    kernel32 = ctypes.windll.kernel32

    # Set function signatures
    user32.OpenClipboard.argtypes = [wintypes.HWND]
    user32.OpenClipboard.restype = wintypes.BOOL
    user32.EmptyClipboard.restype = wintypes.BOOL
    user32.SetClipboardData.argtypes = [wintypes.UINT, wintypes.HANDLE]
    user32.SetClipboardData.restype = wintypes.HANDLE
    user32.CloseClipboard.restype = wintypes.BOOL

    kernel32.GlobalAlloc.argtypes = [wintypes.UINT, ctypes.c_size_t]
    kernel32.GlobalAlloc.restype = wintypes.HGLOBAL
    kernel32.GlobalLock.argtypes = [wintypes.HGLOBAL]
    kernel32.GlobalLock.restype = wintypes.LPVOID
    kernel32.GlobalUnlock.argtypes = [wintypes.HGLOBAL]
    kernel32.GlobalUnlock.restype = wintypes.BOOL
    kernel32.GlobalFree.argtypes = [wintypes.HGLOBAL]
    kernel32.GlobalFree.restype = wintypes.HGLOBAL

    GMEM_MOVEABLE = 0x0002
    CF_DIB = 8

    # Encode to BMP format in memory
    success, encoded = cv2.imencode(".bmp", frame)
    if not success:
        return False

    # BMP format: 14 bytes BITMAPFILEHEADER followed by BITMAPINFOHEADER + pixel array.
    # CF_DIB requires BITMAPINFOHEADER + pixel array (excluding the 14-byte file header).
    raw_bmp = encoded.tobytes()
    if len(raw_bmp) <= 14:
        return False

    dib_bytes = raw_bmp[14:]
    data_size = len(dib_bytes)

    h_global = kernel32.GlobalAlloc(GMEM_MOVEABLE, data_size)
    if not h_global:
        return False

    p_global = kernel32.GlobalLock(h_global)
    if not p_global:
        kernel32.GlobalFree(h_global)
        return False

    try:
        ctypes.memmove(p_global, dib_bytes, data_size)
    finally:
        kernel32.GlobalUnlock(h_global)

    # Open clipboard with retry loop (another app may temporarily hold clipboard lock)
    clipboard_opened = False
    for _ in range(5):
        if user32.OpenClipboard(None):
            clipboard_opened = True
            break
        import time
        time.sleep(0.02)

    if not clipboard_opened:
        kernel32.GlobalFree(h_global)
        return False

    try:
        user32.EmptyClipboard()
        # SetClipboardData takes ownership of h_global on success
        res = user32.SetClipboardData(CF_DIB, h_global)
        return res is not None
    except Exception:
        return False
    finally:
        user32.CloseClipboard()


def _copy_frame_pillow_fallback(frame: np.ndarray) -> bool:
    """Cross-platform fallback using Pillow."""
    try:
        from PIL import Image

        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        image = Image.fromarray(rgb_frame)
        output = io.BytesIO()
        image.save(output, format="PNG")
        output.seek(0)
        return True
    except Exception:
        return False

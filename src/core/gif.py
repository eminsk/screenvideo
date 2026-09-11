"""High-quality 2-pass animated GIF exporter and optimization engine."""

from __future__ import annotations

import os
import subprocess
from pathlib import Path

import imageio_ffmpeg


def export_to_gif(
    video_path: Path | str,
    output_path: Path | str | None = None,
    *,
    fps: int = 15,
    width: int | None = None,
    max_colors: int = 128,
    timeout: float = 60.0,
) -> Path | None:
    """
    Convert video file to high-quality animated GIF using two-pass FFmpeg palette generation.

    Parameters
    ----------
    video_path : Path | str
        Source MP4/AVI/MKV video file.
    output_path : Path | str | None
        Target GIF file path. If None, uses source stem with '.gif' extension.
    fps : int
        Target frame rate for GIF (default 15 for optimal size/smoothness).
    width : int | None
        Optional target width to downscale (maintaining aspect ratio).
    max_colors : int
        Color palette budget (default 128, max 256).
    timeout : float
        Subprocess timeout in seconds.

    Returns
    -------
    Path | None
        Path to generated GIF file, or None if conversion failed.
    """
    src = Path(video_path)
    if not src.exists() or src.stat().st_size == 0:
        return None

    if output_path is None:
        dst = src.with_suffix(".gif")
    else:
        dst = Path(output_path)

    dst.parent.mkdir(parents=True, exist_ok=True)

    # 2-pass palette filter
    clamped_fps = max(1, min(fps, 30))
    clamped_colors = max(32, min(max_colors, 256))

    if width and width > 0:
        # Scale to width maintaining aspect ratio, ensure even height
        vf_filter = (
            f"fps={clamped_fps},scale={width}:-2:flags=lanczos,split[s0][s1];"
            f"[s0]palettegen=max_colors={clamped_colors}:stats_mode=diff[p];"
            f"[s1][p]paletteuse=dither=bayer:bayer_scale=3"
        )
    else:
        vf_filter = (
            f"fps={clamped_fps},split[s0][s1];"
            f"[s0]palettegen=max_colors={clamped_colors}:stats_mode=diff[p];"
            f"[s1][p]paletteuse=dither=bayer:bayer_scale=3"
        )

    try:
        ffmpeg_exe = imageio_ffmpeg.get_ffmpeg_exe()
    except Exception:
        return None

    cmd = [
        ffmpeg_exe,
        "-y",
        "-i",
        str(src),
        "-vf",
        vf_filter,
        str(dst),
    ]

    startupinfo = None
    if os.name == "nt":
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = subprocess.SW_HIDE

    try:
        res = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            startupinfo=startupinfo,
            check=False,
            timeout=timeout,
        )
        if res.returncode == 0 and dst.exists() and dst.stat().st_size > 0:
            return dst
        return None
    except Exception:
        return None

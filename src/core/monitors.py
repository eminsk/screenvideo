"""Monitor detection and capture region management."""

from __future__ import annotations

from dataclasses import dataclass

import mss


@dataclass
class Region:
    """Screen region for recording."""

    x: int = 0
    y: int = 0
    width: int = 0
    height: int = 0

    @property
    def is_valid(self) -> bool:
        """Check if region has non-zero positive dimensions."""
        return self.width >= 16 and self.height >= 16

    def normalized(self) -> Region:
        """
        Return region with even dimensions (width & height divisible by 2).
        Most video codecs (H264, MP4V, XVID) require even dimensions.
        """
        w = self.width - (self.width % 2)
        h = self.height - (self.height % 2)
        return Region(x=self.x, y=self.y, width=max(16, w), height=max(16, h))

    def to_mss_monitor(self) -> dict[str, int]:
        """Convert to mss monitor dict."""
        norm = self.normalized()
        return {
            "left": int(norm.x),
            "top": int(norm.y),
            "width": int(norm.width),
            "height": int(norm.height),
        }

    @property
    def label(self) -> str:
        """Human-readable size and aspect ratio."""
        if not self.is_valid:
            return "Не выбрано"
        ratio = self._get_aspect_ratio_str()
        return f"{self.width} × {self.height} ({ratio})"

    def _get_aspect_ratio_str(self) -> str:
        """Calculate closest common aspect ratio."""
        if self.height == 0:
            return "0:0"
        ratio = self.width / self.height
        if abs(ratio - 16 / 9) < 0.05:
            return "16:9"
        elif abs(ratio - 4 / 3) < 0.05:
            return "4:3"
        elif abs(ratio - 1.0) < 0.05:
            return "1:1"
        elif abs(ratio - 21 / 9) < 0.05:
            return "21:9"
        elif abs(ratio - 9 / 16) < 0.05:
            return "9:16"
        return f"{self.width}:{self.height}"


@dataclass
class MonitorInfo:
    """Information about a physical or virtual display monitor."""

    index: int
    name: str
    left: int
    top: int
    width: int
    height: int
    is_primary: bool

    @property
    def display_text(self) -> str:
        """Formatted text for dropdowns."""
        primary_tag = " (Основной)" if self.is_primary else ""
        return f"{self.name}: {self.width}×{self.height}{primary_tag}"


def get_available_monitors() -> list[MonitorInfo]:
    """Get list of all active monitors on system."""
    monitors_list: list[MonitorInfo] = []
    try:
        with mss.mss() as sct:
            for idx, mon in enumerate(sct.monitors):
                if idx == 0:
                    monitors_list.append(
                        MonitorInfo(
                            index=0,
                            name="🖥 Все мониторы (Виртуальный экран)",
                            left=mon["left"],
                            top=mon["top"],
                            width=mon["width"],
                            height=mon["height"],
                            is_primary=False,
                        )
                    )
                else:
                    monitors_list.append(
                        MonitorInfo(
                            index=idx,
                            name=f"🖥 Монитор {idx}",
                            left=mon["left"],
                            top=mon["top"],
                            width=mon["width"],
                            height=mon["height"],
                            is_primary=(idx == 1),
                        )
                    )
    except Exception:
        # Fallback to single primary monitor
        monitors_list.append(
            MonitorInfo(
                index=1,
                name="🖥 Монитор 1",
                left=0,
                top=0,
                width=1920,
                height=1080,
                is_primary=True,
            )
        )
    return monitors_list

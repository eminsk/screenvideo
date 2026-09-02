"""Application configuration management with JSON persistence."""

from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
from pathlib import Path


@dataclass
class AppConfig:
    """Application settings configuration."""

    # Video settings
    fps: int = 30
    codec: str = "mp4v"
    format: str = "mp4"
    quality_preset: str = "High"  # Normal, High, Ultra

    # Storage paths
    recordings_dir: Path = field(default_factory=lambda: Path.cwd() / "recordings")
    screenshots_dir: Path = field(default_factory=lambda: Path.cwd() / "screenshots")

    # Cursor & visual options
    record_cursor: bool = True
    highlight_cursor: bool = True
    cursor_color: str = "#FFD700"  # Gold yellow halo

    # Countdown & mini overlay
    show_countdown: bool = True
    countdown_seconds: int = 3
    show_floating_bar: bool = True
    minimize_on_record: bool = True
    sound_effects: bool = True

    # Hotkeys
    hotkey_start: str = "f5"
    hotkey_pause: str = "f6"
    hotkey_stop: str = "f10"
    hotkey_screenshot: str = "f11"

    # UI Theme
    theme: str = "darkly"

    # Audio recording
    record_system_audio: bool = True
    record_microphone: bool = False
    audio_bitrate: str = "192k"

    # Selected monitor index (1: Primary, 2..N: Secondary, 0: All)
    monitor_index: int = 1

    def __post_init__(self) -> None:
        if isinstance(self.recordings_dir, str):
            self.recordings_dir = Path(self.recordings_dir)
        if isinstance(self.screenshots_dir, str):
            self.screenshots_dir = Path(self.screenshots_dir)

        self.recordings_dir.mkdir(parents=True, exist_ok=True)
        self.screenshots_dir.mkdir(parents=True, exist_ok=True)

    @classmethod
    def get_config_file_path(cls) -> Path:
        """Get standard config file location."""
        return Path.cwd() / "config.json"

    @classmethod
    def load(cls) -> AppConfig:
        """Load configuration from config.json or return defaults."""
        config_path = cls.get_config_file_path()
        if not config_path.exists():
            return cls()

        try:
            with open(config_path, encoding="utf-8") as f:
                data = json.load(f)

            if "recordings_dir" in data:
                data["recordings_dir"] = Path(data["recordings_dir"])
            if "screenshots_dir" in data:
                data["screenshots_dir"] = Path(data["screenshots_dir"])

            valid_keys = {f.name for f in cls.__dataclass_fields__.values()}
            filtered_data = {k: v for k, v in data.items() if k in valid_keys}
            return cls(**filtered_data)
        except Exception:
            return cls()

    def save(self) -> bool:
        """Save configuration to config.json."""
        config_path = self.get_config_file_path()
        try:
            data = asdict(self)
            data["recordings_dir"] = str(self.recordings_dir)
            data["screenshots_dir"] = str(self.screenshots_dir)
            with open(config_path, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
            return True
        except Exception:
            return False

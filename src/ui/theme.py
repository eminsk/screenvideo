"""UI Theme, styling constants, typography, and color tokens."""

from __future__ import annotations


class Fonts:
    """Modern UI typography definitions."""

    HERO_TIMER = ("Consolas", 32, "bold")
    FLOATING_TIMER = ("Consolas", 15, "bold")
    COUNTDOWN = ("Segoe UI", 80, "bold")

    TITLE_LARGE = ("Segoe UI", 18, "bold")
    TITLE_MEDIUM = ("Segoe UI", 14, "bold")
    TITLE_SMALL = ("Segoe UI", 11, "bold")

    BODY = ("Segoe UI", 10)
    BODY_BOLD = ("Segoe UI", 10, "bold")
    CAPTION = ("Segoe UI", 9)
    MONO_SMALL = ("Consolas", 9)


class Colors:
    """Design color palette tokens."""

    PRIMARY = "#375a7f"
    SUCCESS = "#00bc8c"
    DANGER = "#e74c3c"
    WARNING = "#f39c12"
    INFO = "#3498db"

    BG_DARK = "#222222"
    CARD_BG_DARK = "#2b2b2b"
    BORDER_DARK = "#444444"
    TEXT_MUTED = "#888888"
    ACCENT_GOLD = "#ffd700"


AVAILABLE_THEMES = [
    ("darkly", "🌙 Darkly (Тёмная)"),
    ("superhero", "🦸 Superhero (Синяя тёмная)"),
    ("solar", "☀️ Solar (Тёмно-янтарная)"),
    ("cyborg", "🤖 Cyborg (Контрастная тёмная)"),
    ("flatly", "📄 Flatly (Светлая чистая)"),
    ("cosmo", "🚀 Cosmo (Светлая современная)"),
    ("minty", "🌿 Minty (Светло-мятная)"),
    ("litera", "📖 Litera (Классическая светлая)"),
]

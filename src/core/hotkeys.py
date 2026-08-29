"""Global hotkey manager for application shortcuts."""

from __future__ import annotations

from typing import Callable

import keyboard


class HotkeyManager:
    """Safely registers and unregisters global keyboard shortcuts."""

    def __init__(self, dispatch_fn: Callable[[Callable[[], None]], None]) -> None:
        """
        dispatch_fn: Thread-safe callback dispatcher (e.g. root.after(0, fn))
        """
        self._dispatch = dispatch_fn
        self._active_hotkeys: dict[str, str] = {}

    def register(
        self, hotkey_name: str, key_combination: str, callback: Callable[[], None]
    ) -> bool:
        """Register a global hotkey."""
        key = key_combination.strip().lower()
        if not key:
            return False

        # Remove existing hotkey for this name if present
        self.unregister(hotkey_name)

        try:

            def _handler() -> None:
                self._dispatch(callback)

            keyboard.add_hotkey(key, _handler)
            self._active_hotkeys[hotkey_name] = key
            return True
        except Exception as e:
            print(f"[HotkeyManager] Could not register hotkey '{key}' for {hotkey_name}: {e}")
            return False

    def unregister(self, hotkey_name: str) -> None:
        """Unregister a specific hotkey by its logical name."""
        if hotkey_name in self._active_hotkeys:
            key = self._active_hotkeys.pop(hotkey_name)
            try:
                keyboard.remove_hotkey(key)
            except Exception:
                pass

    def unregister_all(self) -> None:
        """Unregister all hotkeys."""
        self._active_hotkeys.clear()
        try:
            keyboard.unhook_all()
        except Exception:
            pass

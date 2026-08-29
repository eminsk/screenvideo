"""Recordings and screenshots history manager view."""

from __future__ import annotations

import tkinter.messagebox as msgbox
from typing import TYPE_CHECKING

import ttkbootstrap as ttk
from ttkbootstrap.constants import BOTH, LEFT, RIGHT, X

from src.core.history import HistoryManager, MediaItem
from src.ui.theme import Fonts
from src.utils.system import open_in_default_app, reveal_in_explorer

if TYPE_CHECKING:
    from src.ui.app import ScreenCaptureApp


class HistoryView(ttk.Frame):
    """View displaying gallery/table of past recordings and screenshots."""

    def __init__(self, parent: ttk.Notebook, app: ScreenCaptureApp) -> None:
        super().__init__(parent, padding=16)
        self._app = app
        self._history_mgr = HistoryManager(app.config.recordings_dir, app.config.screenshots_dir)
        self._items: list[MediaItem] = []
        self._filter_type = "all"  # "all", "video", "image"

        self._create_widgets()
        self.refresh_list()

    def _create_widgets(self) -> None:
        """Create history UI components."""
        # Top toolbar
        toolbar = ttk.Frame(self)
        toolbar.pack(fill=X, pady=(0, 10))

        lbl_title = ttk.Label(
            toolbar, text="📁 Сохранённые записи и снимки", font=Fonts.TITLE_SMALL
        )
        lbl_title.pack(side=LEFT)

        btn_refresh = ttk.Button(
            toolbar,
            text="🔄 Обновить",
            bootstyle="secondary-outline",
            command=self.refresh_list,
        )
        btn_refresh.pack(side=RIGHT, padx=2)

        btn_open_folder = ttk.Button(
            toolbar,
            text="📂 Папка записей",
            bootstyle="info-outline",
            command=lambda: open_in_default_app(self._app.config.recordings_dir),
        )
        btn_open_folder.pack(side=RIGHT, padx=4)

        # Filter buttons
        filter_frame = ttk.Frame(self)
        filter_frame.pack(fill=X, pady=(0, 8))

        self._filter_var = ttk.StringVar(value="all")
        ttk.Radiobutton(
            filter_frame,
            text="Все файлы",
            variable=self._filter_var,
            value="all",
            command=self._apply_filter,
            bootstyle="primary-toolbutton",
        ).pack(side=LEFT, padx=2)

        ttk.Radiobutton(
            filter_frame,
            text="🎬 Только видео",
            variable=self._filter_var,
            value="video",
            command=self._apply_filter,
            bootstyle="primary-toolbutton",
        ).pack(side=LEFT, padx=2)

        ttk.Radiobutton(
            filter_frame,
            text="📸 Только снимки",
            variable=self._filter_var,
            value="image",
            command=self._apply_filter,
            bootstyle="primary-toolbutton",
        ).pack(side=LEFT, padx=2)

        # Treeview table
        tree_container = ttk.Frame(self)
        tree_container.pack(fill=BOTH, expand=True)

        columns = ("type", "name", "size", "duration", "date")
        self._tree = ttk.Treeview(
            tree_container,
            columns=columns,
            show="headings",
            selectmode="browse",
            bootstyle="dark",
        )

        self._tree.heading("type", text="Тип", anchor="center")
        self._tree.heading("name", text="Имя файла", anchor="w")
        self._tree.heading("size", text="Размер", anchor="center")
        self._tree.heading("duration", text="Длит. / Разреш.", anchor="center")
        self._tree.heading("date", text="Дата создания", anchor="center")

        self._tree.column("type", width=70, anchor="center")
        self._tree.column("name", width=220, anchor="w")
        self._tree.column("size", width=85, anchor="center")
        self._tree.column("duration", width=110, anchor="center")
        self._tree.column("date", width=130, anchor="center")

        scrollbar = ttk.Scrollbar(tree_container, orient="vertical", command=self._tree.yview)
        self._tree.configure(yscrollcommand=scrollbar.set)

        self._tree.pack(side=LEFT, fill=BOTH, expand=True)
        scrollbar.pack(side=RIGHT, fill="y")

        self._tree.bind("<Double-1>", lambda e: self._on_play_selected())

        # Bottom Action Buttons
        actions_bar = ttk.Frame(self)
        actions_bar.pack(fill=X, pady=(10, 0))

        btn_play = ttk.Button(
            actions_bar,
            text="▶ Воспроизвести",
            bootstyle="success",
            command=self._on_play_selected,
        )
        btn_play.pack(side=LEFT, padx=3)

        btn_reveal = ttk.Button(
            actions_bar,
            text="📁 Показать в папке",
            bootstyle="secondary",
            command=self._on_reveal_selected,
        )
        btn_reveal.pack(side=LEFT, padx=3)

        btn_delete = ttk.Button(
            actions_bar,
            text="🗑 Удалить",
            bootstyle="danger-outline",
            command=self._on_delete_selected,
        )
        btn_delete.pack(side=RIGHT, padx=3)

    def refresh_list(self) -> None:
        """Rescan media files and refresh table."""
        self._history_mgr = HistoryManager(
            self._app.config.recordings_dir, self._app.config.screenshots_dir
        )
        self._items = self._history_mgr.scan_items()
        self._apply_filter()

    def _apply_filter(self) -> None:
        """Filter table items by active radio filter."""
        filter_val = self._filter_var.get()
        for row in self._tree.get_children():
            self._tree.delete(row)

        for item in self._items:
            if filter_val == "video" and item.file_type != "video":
                continue
            if filter_val == "image" and item.file_type != "image":
                continue

            icon = "🎬 Видео" if item.file_type == "video" else "📸 Снимок"
            extra = (
                item.resolution
                if item.file_type == "video" and item.resolution
                else item.formatted_duration
            )

            self._tree.insert(
                "",
                "end",
                iid=str(item.path),
                values=(
                    icon,
                    item.filename,
                    item.formatted_size,
                    extra,
                    item.formatted_date,
                ),
            )

    def _get_selected_item(self) -> MediaItem | None:
        """Return currently selected MediaItem."""
        selection = self._tree.selection()
        if not selection:
            return None
        selected_path_str = selection[0]
        for item in self._items:
            if str(item.path) == selected_path_str:
                return item
        return None

    def _on_play_selected(self) -> None:
        item = self._get_selected_item()
        if item:
            open_in_default_app(item.path)

    def _on_reveal_selected(self) -> None:
        item = self._get_selected_item()
        if item:
            reveal_in_explorer(item.path)

    def _on_delete_selected(self) -> None:
        item = self._get_selected_item()
        if not item:
            return

        confirm = msgbox.askyesno(
            "Удаление файла",
            f"Вы уверены, что хотите удалить файл:\n{item.filename}?",
            parent=self,
        )
        if confirm:
            if self._history_mgr.delete_item(item.path):
                self.refresh_list()

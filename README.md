# ScreenCapturePro

🎬 **Professional Screen Recorder** — простое и мощное приложение для записи экрана на Windows.

![Screen Recorder](icon.png)

## ✨ Возможности

- 🖥️ **Запись всего экрана** или выбранной области
- ⌨️ **Горячие клавиши**: F5 — старт, F10 — стоп
- 💾 **Zero-memory streaming** — запись напрямую на диск без нагрузки на RAM
- 🎯 **Выбор области** — записывайте только нужную часть экрана
- ⏱️ **Таймер и счётчик кадров** в реальном времени
- 📁 **Автосохранение** с уникальными именами файлов

## 🚀 Установка

### Скачать готовый exe
Скачайте последнюю версию из [Releases](https://github.com/eminsk/screenvideo/releases)

### Запуск из исходников
```bash
# Требуется Python 3.12+ и uv
uv sync
uv run python main.py
```

## 🎮 Использование

1. Запустите `ScreenCapturePro.exe`
2. Нажмите **Select Region** чтобы выбрать область (или пропустите для записи всего экрана)
3. Нажмите **F5** или кнопку **Start** для начала записи
4. Нажмите **F10** или кнопку **Stop** для остановки
5. Видео сохраняется в папку `recordings/`

## 🛠️ Сборка

```bash
uv add nuitka
uv run python -m nuitka --onefile --windows-console-mode=disable --windows-icon-from-ico=icon.ico --enable-plugin=tk-inter --include-data-file=icon.ico=icon.ico --output-filename=ScreenCapturePro.exe main.py
```

## 📋 Технологии

- Python 3.12
- ttkbootstrap (UI)
- OpenCV (видео)
- mss (захват экрана)
- keyboard (горячие клавиши)

## 📄 Лицензия

MIT License

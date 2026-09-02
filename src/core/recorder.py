"""High-performance, zero-memory screen recorder engine with pause/resume and cursor capture."""

from __future__ import annotations

import os
import threading
import time
from datetime import datetime
from enum import Enum, auto
from pathlib import Path

import cv2
import mss
import numpy as np

from src.core.audio import AudioRecorder, merge_video_audio
from src.core.config import AppConfig
from src.core.cursor import CursorRenderer
from src.core.monitors import Region


class RecordingState(Enum):
    """Recording state enumeration."""

    IDLE = auto()
    RECORDING = auto()
    PAUSED = auto()


class ScreenRecorder:
    """
    Multi-threaded, zero-memory screen recorder.
    Streams video frames directly to disk with precise timestamp synchronization.
    """

    def __init__(self, config: AppConfig | None = None) -> None:
        self._config = config or AppConfig()
        self._state = RecordingState.IDLE
        self._lock = threading.Lock()
        self._thread: threading.Thread | None = None

        self._start_time: float = 0.0
        self._pause_start_time: float = 0.0
        self._total_paused_duration: float = 0.0
        self._frame_count: int = 0
        self._fps_actual: float = 0.0

        self._writer: cv2.VideoWriter | None = None
        self._current_file: Path | None = None
        self._temp_video_file: Path | None = None
        self._audio_recorder: AudioRecorder | None = None
        self._region: Region | None = None
        self._monitor_index: int = self._config.monitor_index
        self._sct: mss.mss | None = None
        self._cursor_renderer = CursorRenderer()

    @property
    def config(self) -> AppConfig:
        return self._config

    @config.setter
    def config(self, value: AppConfig) -> None:
        self._config = value

    @property
    def state(self) -> RecordingState:
        with self._lock:
            return self._state

    @property
    def frame_count(self) -> int:
        with self._lock:
            return self._frame_count

    @property
    def duration(self) -> float:
        """Effective recorded duration in seconds (excluding pause time)."""
        with self._lock:
            if self._state == RecordingState.IDLE or self._start_time == 0.0:
                return 0.0
            if self._state == RecordingState.PAUSED:
                return self._pause_start_time - self._start_time - self._total_paused_duration
            return time.time() - self._start_time - self._total_paused_duration

    @property
    def fps_actual(self) -> float:
        with self._lock:
            return self._fps_actual

    @property
    def current_file(self) -> Path | None:
        with self._lock:
            return self._current_file

    @property
    def current_file_size(self) -> int:
        """Current file size on disk in bytes."""
        with self._lock:
            target = self._temp_video_file or self._current_file
            if target and target.exists():
                try:
                    return os.path.getsize(target)
                except OSError:
                    pass
        return 0

    def set_region(self, region: Region | None) -> None:
        """Set capture region. None represents full screen/monitor."""
        with self._lock:
            self._region = region

    def set_monitor_index(self, index: int) -> None:
        """Set target monitor index."""
        with self._lock:
            self._monitor_index = index

    def start(self) -> bool:
        """Start recording session."""
        with self._lock:
            if self._state != RecordingState.IDLE:
                return False

            self._current_file = self._generate_output_path()
            timestamp_ms = int(time.time() * 1000)

            # Audio setup
            has_audio = (
                self._config.record_system_audio or self._config.record_microphone
            )
            if has_audio:
                self._temp_video_file = (
                    self._config.recordings_dir / f"temp_vid_{timestamp_ms}.mp4"
                )
                self._audio_recorder = AudioRecorder(
                    self._config.recordings_dir,
                    record_system_audio=self._config.record_system_audio,
                    record_microphone=self._config.record_microphone,
                )
                self._audio_recorder.start()
            else:
                self._temp_video_file = self._current_file
                self._audio_recorder = None

            self._frame_count = 0
            self._fps_actual = float(self._config.fps)
            self._total_paused_duration = 0.0
            self._state = RecordingState.RECORDING
            self._start_time = time.time()

        self._thread = threading.Thread(
            target=self._capture_loop, daemon=True, name="RecorderThread"
        )
        self._thread.start()
        return True

    def pause(self) -> bool:
        """Pause active recording."""
        with self._lock:
            if self._state != RecordingState.RECORDING:
                return False
            self._state = RecordingState.PAUSED
            self._pause_start_time = time.time()
            if self._audio_recorder:
                self._audio_recorder.pause()
            return True

    def resume(self) -> bool:
        """Resume paused recording."""
        with self._lock:
            if self._state != RecordingState.PAUSED:
                return False
            paused_delta = time.time() - self._pause_start_time
            self._total_paused_duration += paused_delta
            self._state = RecordingState.RECORDING
            if self._audio_recorder:
                self._audio_recorder.resume()
            return True

    def toggle_pause(self) -> bool:
        """Toggle between paused and recording states."""
        with self._lock:
            current_state = self._state

        if current_state == RecordingState.RECORDING:
            return self.pause()
        elif current_state == RecordingState.PAUSED:
            return self.resume()
        return False

    def stop(self) -> Path | None:
        """Stop recording, finalize video and audio files and return path."""
        with self._lock:
            if self._state == RecordingState.IDLE:
                return None
            self._state = RecordingState.IDLE
            audio_rec = self._audio_recorder

        # Stop audio recording and get temp WAV file
        audio_file = audio_rec.stop() if audio_rec else None

        if self._thread and self._thread.is_alive():
            self._thread.join(timeout=3.5)

        with self._lock:
            temp_vid = self._temp_video_file
            target_file = self._current_file
            frames = self._frame_count

            # Clean up empty or corrupted video files (0 frames or tiny header-only files)
            if temp_vid and temp_vid.exists():
                try:
                    file_sz = os.path.getsize(temp_vid)
                    if frames == 0 or file_sz <= 1024:
                        temp_vid.unlink(missing_ok=True)
                        if audio_file and audio_file.exists():
                            audio_file.unlink(missing_ok=True)
                        self._current_file = None
                        self._temp_video_file = None
                        return None
                except OSError:
                    pass

            # Mux video and audio into final container
            if temp_vid and temp_vid.exists() and target_file:
                if audio_file and audio_file.exists():
                    merge_video_audio(
                        temp_vid,
                        audio_file,
                        target_file,
                        audio_bitrate=self._config.audio_bitrate,
                    )
                elif temp_vid != target_file:
                    try:
                        if target_file.exists():
                            target_file.unlink()
                        temp_vid.replace(target_file)
                    except Exception:
                        pass

            self._temp_video_file = None
            self._audio_recorder = None

            if target_file and target_file.exists() and target_file.stat().st_size > 1024:
                return target_file

            return None

    def _capture_loop(self) -> None:
        """Core streaming loop with timestamp pacing."""
        target_fps = max(1, self._config.fps)
        frame_interval = 1.0 / target_fps

        self._sct = mss.mss()

        # Determine monitor and geometry
        with self._lock:
            region = self._region
            monitor_index = self._monitor_index
            file_path = self._temp_video_file or self._current_file
            codec = self._config.codec

        if region and region.is_valid:
            norm_region = region.normalized()
            monitor = norm_region.to_mss_monitor()
            offset_x = norm_region.x
            offset_y = norm_region.y
            width, height = norm_region.width, norm_region.height
        else:
            mon_idx = min(monitor_index, len(self._sct.monitors) - 1)
            monitor = self._sct.monitors[mon_idx]
            offset_x = monitor["left"]
            offset_y = monitor["top"]
            width = monitor["width"] - (monitor["width"] % 2)
            height = monitor["height"] - (monitor["height"] % 2)
            monitor = {
                "left": offset_x,
                "top": offset_y,
                "width": width,
                "height": height,
            }

        # Initialize VideoWriter
        fourcc = cv2.VideoWriter_fourcc(*codec)
        self._writer = cv2.VideoWriter(
            str(file_path),
            fourcc,
            target_fps,
            (width, height),
        )

        # Fallback to mp4v if writer initialization failed
        if not self._writer.isOpened() and codec != "mp4v":
            fourcc = cv2.VideoWriter_fourcc(*"mp4v")
            self._writer = cv2.VideoWriter(
                str(file_path),
                fourcc,
                target_fps,
                (width, height),
            )

        fps_calc_time = time.perf_counter()
        fps_calc_frames = 0
        written_frames = 0
        loop_start_time = time.perf_counter()

        # Capture initial frame immediately to ensure the file contains valid video data
        try:
            with self._lock:
                should_capture = self._state == RecordingState.RECORDING

            if should_capture:
                raw_img = self._sct.grab(monitor)
                frame = cv2.cvtColor(np.asarray(raw_img, dtype=np.uint8), cv2.COLOR_BGRA2BGR)
                if self._config.record_cursor:
                    self._cursor_renderer.render(
                        frame,
                        offset_x=offset_x,
                        offset_y=offset_y,
                        highlight=self._config.highlight_cursor,
                    )
                if self._writer and self._writer.isOpened():
                    self._writer.write(frame)
                    written_frames += 1
                    with self._lock:
                        self._frame_count = written_frames
        except Exception as e:
            print(f"[ScreenRecorder] Error writing initial frame: {e}")

        try:
            while True:
                with self._lock:
                    current_state = self._state

                if current_state == RecordingState.IDLE:
                    break

                if current_state == RecordingState.PAUSED:
                    time.sleep(0.04)
                    # Shift loop start time so pacing remains continuous on resume
                    loop_start_time += 0.04
                    continue

                # Target timestamp for next frame
                target_time = loop_start_time + (written_frames * frame_interval)
                now = time.perf_counter()
                wait_time = target_time - now

                if wait_time > 0.001:
                    time.sleep(wait_time)

                with self._lock:
                    if self._state != RecordingState.RECORDING:
                        continue

                # Capture raw frame
                raw_img = self._sct.grab(monitor)
                frame = cv2.cvtColor(np.asarray(raw_img, dtype=np.uint8), cv2.COLOR_BGRA2BGR)

                # Draw mouse cursor if enabled
                if self._config.record_cursor:
                    self._cursor_renderer.render(
                        frame,
                        offset_x=offset_x,
                        offset_y=offset_y,
                        highlight=self._config.highlight_cursor,
                    )

                if self._writer and self._writer.isOpened():
                    self._writer.write(frame)
                    written_frames += 1

                with self._lock:
                    self._frame_count = written_frames

                # FPS Measurement calculation
                fps_calc_frames += 1
                fps_elapsed = time.perf_counter() - fps_calc_time
                if fps_elapsed >= 1.0:
                    with self._lock:
                        self._fps_actual = round(fps_calc_frames / fps_elapsed, 1)
                    fps_calc_frames = 0
                    fps_calc_time = time.perf_counter()

        except Exception as e:
            print(f"[ScreenRecorder] Error during capture: {e}")
        finally:
            if self._writer:
                self._writer.release()
                self._writer = None
            if self._sct:
                self._sct.close()
                self._sct = None

    def _generate_output_path(self) -> Path:
        """Generate unique timestamped output filepath."""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        ext = self._config.format.lower()
        if not ext.startswith("."):
            ext = f".{ext}"
        return self._config.recordings_dir / f"recording_{timestamp}{ext}"

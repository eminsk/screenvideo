"""High-performance audio capture engine (WASAPI Loopback & Microphone) and MP4 muxer."""

from __future__ import annotations

import os
import subprocess
import threading
import time
from pathlib import Path
from typing import List

import imageio_ffmpeg
import numpy as np
import soundcard as sc
import soundfile as sf


class AudioRecorder:
    """
    Multi-threaded audio recorder capturing Windows system audio (WASAPI Loopback)
    and/or microphone, saving synchronized PCM WAV for subsequent muxing.
    """

    SAMPLE_RATE = 44100
    CHANNELS = 2
    CHUNK_SIZE = 2048

    def __init__(
        self,
        output_dir: Path,
        *,
        record_system_audio: bool = True,
        record_microphone: bool = False,
    ) -> None:
        self._output_dir = Path(output_dir)
        self._record_system = record_system_audio
        self._record_mic = record_microphone

        self._is_recording = False
        self._is_paused = False
        self._thread: threading.Thread | None = None
        self._lock = threading.Lock()

        self._audio_chunks: List[np.ndarray] = []
        self._temp_file: Path | None = None

    @property
    def is_recording(self) -> bool:
        with self._lock:
            return self._is_recording

    def start(self) -> bool:
        """Start audio capture thread."""
        if not self._record_system and not self._record_mic:
            return False

        with self._lock:
            if self._is_recording:
                return False
            self._is_recording = True
            self._is_paused = False
            self._audio_chunks = []
            timestamp = int(time.time() * 1000)
            self._temp_file = self._output_dir / f"temp_audio_{timestamp}.wav"

        self._thread = threading.Thread(
            target=self._capture_loop, daemon=True, name="AudioRecorderThread"
        )
        self._thread.start()
        return True

    def pause(self) -> None:
        """Pause audio capture."""
        with self._lock:
            self._is_paused = True

    def resume(self) -> None:
        """Resume audio capture."""
        with self._lock:
            self._is_paused = False

    def stop(self) -> Path | None:
        """Stop audio recording, flush to WAV file, and return filepath."""
        with self._lock:
            if not self._is_recording:
                return None
            self._is_recording = False
            self._is_paused = False

        if self._thread and self._thread.is_alive():
            self._thread.join(timeout=2.0)

        with self._lock:
            chunks = self._audio_chunks
            temp_path = self._temp_file

        if not chunks or temp_path is None:
            return None

        try:
            full_audio = np.concatenate(chunks, axis=0)
            # Ensure float32 in [-1.0, 1.0] range
            full_audio = np.clip(full_audio, -1.0, 1.0)
            sf.write(str(temp_path), full_audio, self.SAMPLE_RATE, subtype="PCM_16")
            if temp_path.exists() and temp_path.stat().st_size > 44:
                return temp_path
        except Exception as e:
            print(f"[AudioRecorder] Failed to save WAV file: {e}")

        return None

    def _capture_loop(self) -> None:
        """Main audio capture streaming loop."""
        loopback_mic = None
        standard_mic = None

        try:
            if self._record_system:
                try:
                    default_speaker = sc.default_speaker()
                    if default_speaker:
                        loopback_mic = sc.get_microphone(
                            id=str(default_speaker.name), include_loopback=True
                        )
                except Exception as e:
                    print(f"[AudioRecorder] Could not access default speaker loopback: {e}")

            if self._record_mic:
                try:
                    standard_mic = sc.default_microphone()
                except Exception as e:
                    print(f"[AudioRecorder] Could not access default microphone: {e}")

            if loopback_mic is None and standard_mic is None:
                print("[AudioRecorder] No audio recording devices available.")
                return

            # Open recorders
            rec_loop = (
                loopback_mic.recorder(samplerate=self.SAMPLE_RATE, channels=self.CHANNELS)
                if loopback_mic
                else None
            )
            rec_mic = (
                standard_mic.recorder(samplerate=self.SAMPLE_RATE, channels=self.CHANNELS)
                if standard_mic
                else None
            )

            with rec_loop if rec_loop else _DummyContext():
                with rec_mic if rec_mic else _DummyContext():
                    while True:
                        with self._lock:
                            if not self._is_recording:
                                break
                            is_paused = self._is_paused

                        if is_paused:
                            time.sleep(0.05)
                            continue

                        chunk = None

                        if rec_loop is not None:
                            try:
                                chunk = rec_loop.record(numframes=self.CHUNK_SIZE)
                            except Exception:
                                chunk = None

                        if rec_mic is not None:
                            try:
                                mic_chunk = rec_mic.record(numframes=self.CHUNK_SIZE)
                                if chunk is not None and chunk.shape == mic_chunk.shape:
                                    chunk = (chunk * 0.7) + (mic_chunk * 0.7)
                                elif chunk is None:
                                    chunk = mic_chunk
                            except Exception:
                                pass

                        if chunk is not None and len(chunk) > 0:
                            with self._lock:
                                self._audio_chunks.append(chunk.astype(np.float32))
                        else:
                            time.sleep(0.01)

        except Exception as e:
            print(f"[AudioRecorder] Error during audio capture: {e}")


class _DummyContext:
    """No-op context manager for optional recorders."""

    def __enter__(self) -> None:
        pass

    def __exit__(self, exc_type: object, exc_val: object, exc_tb: object) -> None:
        pass


def merge_video_audio(
    video_path: Path,
    audio_path: Path | None,
    output_path: Path,
    *,
    audio_bitrate: str = "192k",
) -> bool:
    """
    Merge video stream with audio WAV stream into final MP4 using bundled FFmpeg.
    """
    video_path = Path(video_path)
    output_path = Path(output_path)

    # If no audio or audio missing, simply ensure video is at output_path
    if audio_path is None or not Path(audio_path).exists():
        if video_path != output_path:
            try:
                if output_path.exists():
                    output_path.unlink()
                video_path.replace(output_path)
                return True
            except Exception as e:
                print(f"[Muxer] Failed to move video to output: {e}")
                return False
        return True

    audio_path = Path(audio_path)
    ffmpeg_exe = imageio_ffmpeg.get_ffmpeg_exe()

    # Construct ffmpeg command
    cmd = [
        ffmpeg_exe,
        "-y",
        "-i",
        str(video_path),
        "-i",
        str(audio_path),
        "-c:v",
        "copy",
        "-c:a",
        "aac",
        "-b:a",
        audio_bitrate,
        "-shortest",
        str(output_path),
    ]

    try:
        # Hide console window on Windows
        startupinfo = None
        if os.name == "nt":
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            startupinfo.wShowWindow = subprocess.SW_HIDE

        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            startupinfo=startupinfo,
            check=False,
            timeout=30.0,
        )

        # Cleanup temp intermediate files
        if video_path != output_path and video_path.exists():
            video_path.unlink(missing_ok=True)
        if audio_path.exists():
            audio_path.unlink(missing_ok=True)

        if result.returncode == 0 and output_path.exists() and output_path.stat().st_size > 1024:
            return True
        else:
            print(f"[Muxer] FFmpeg error: {result.stderr.decode('utf-8', errors='ignore')}")
            # Fallback: if output_path wasn't created, try keeping video_path
            if video_path.exists() and not output_path.exists():
                video_path.replace(output_path)
            return False

    except Exception as e:
        print(f"[Muxer] Failed to mux video and audio: {e}")
        if video_path.exists() and not output_path.exists():
            video_path.replace(output_path)
        if audio_path.exists():
            audio_path.unlink(missing_ok=True)
        return False

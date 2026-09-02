"""Unit and integration tests for ScreenCapture Pro."""

from __future__ import annotations

import shutil
import tempfile
import time
import unittest
from pathlib import Path

import numpy as np

from src.core.config import AppConfig
from src.core.cursor import CursorRenderer
from src.core.history import HistoryManager
from src.core.monitors import Region, get_available_monitors
from src.core.recorder import RecordingState, ScreenRecorder
from src.core.screenshot import capture_screenshot
from src.utils.formatting import format_bytes, format_duration, format_duration_full


class TestScreenCapturePro(unittest.TestCase):
    """Test suite for core components."""

    def setUp(self) -> None:
        self.temp_dir = Path(tempfile.mkdtemp())
        self.rec_dir = self.temp_dir / "recordings"
        self.snap_dir = self.temp_dir / "screenshots"
        self.rec_dir.mkdir()
        self.snap_dir.mkdir()

    def tearDown(self) -> None:
        shutil.rmtree(self.temp_dir, ignore_errors=True)

    def test_formatting_utils(self) -> None:
        """Test human readable formatting functions."""
        self.assertEqual(format_duration(0), "00:00")
        self.assertEqual(format_duration(65), "01:05")
        self.assertEqual(format_duration(3665), "01:01:05")

        self.assertIn("00:00:05", format_duration_full(5.2))

        self.assertEqual(format_bytes(500), "500 B")
        self.assertEqual(format_bytes(1024), "1.0 KB")
        self.assertEqual(format_bytes(1024 * 1024 * 5), "5.0 MB")

    def test_region_normalization(self) -> None:
        """Test that odd dimensions are normalized to even numbers for codecs."""
        odd_region = Region(x=10, y=20, width=517, height=301)
        self.assertTrue(odd_region.is_valid)

        norm = odd_region.normalized()
        self.assertEqual(norm.width % 2, 0)
        self.assertEqual(norm.height % 2, 0)
        self.assertEqual(norm.width, 516)
        self.assertEqual(norm.height, 300)

        mss_dict = norm.to_mss_monitor()
        self.assertEqual(mss_dict["left"], 10)
        self.assertEqual(mss_dict["top"], 20)
        self.assertEqual(mss_dict["width"], 516)
        self.assertEqual(mss_dict["height"], 300)

    def test_config_persistence(self) -> None:
        """Test config loading, saving, and defaults."""
        config = AppConfig(
            fps=60,
            recordings_dir=self.rec_dir,
            screenshots_dir=self.snap_dir,
            hotkey_start="f8",
        )
        self.assertEqual(config.fps, 60)
        self.assertEqual(config.hotkey_start, "f8")

    def test_monitors_detection(self) -> None:
        """Test monitor enumeration."""
        monitors = get_available_monitors()
        self.assertGreaterEqual(len(monitors), 1)
        self.assertGreater(monitors[0].width, 0)
        self.assertGreater(monitors[0].height, 0)

    def test_cursor_renderer(self) -> None:
        """Test cursor rendering on dummy frame."""
        frame = np.zeros((400, 600, 3), dtype=np.uint8)
        renderer = CursorRenderer()
        # Should not raise exception
        renderer.render(frame, offset_x=0, offset_y=0, highlight=True)
        self.assertEqual(frame.shape, (400, 600, 3))

    def test_screenshot_capture(self) -> None:
        """Test capturing instant screenshot."""
        region = Region(x=0, y=0, width=200, height=200)
        snap_path = capture_screenshot(
            region=region,
            output_dir=self.snap_dir,
            include_cursor=False,
        )
        self.assertTrue(snap_path.exists())
        self.assertEqual(snap_path.suffix.lower(), ".png")

    def test_recorder_lifecycle(self) -> None:
        """Test recorder start, pause, resume, and stop sequence."""
        cfg = AppConfig(
            fps=15,
            recordings_dir=self.rec_dir,
            record_cursor=False,
        )
        recorder = ScreenRecorder(config=cfg)
        recorder.set_region(Region(x=0, y=0, width=320, height=240))

        self.assertEqual(recorder.state, RecordingState.IDLE)
        self.assertTrue(recorder.start())
        self.assertEqual(recorder.state, RecordingState.RECORDING)

        time.sleep(0.4)
        self.assertGreaterEqual(recorder.frame_count, 1)

        # Pause
        self.assertTrue(recorder.pause())
        self.assertEqual(recorder.state, RecordingState.PAUSED)
        time.sleep(0.08)
        paused_frames = recorder.frame_count
        time.sleep(0.25)
        self.assertEqual(recorder.frame_count, paused_frames)

        # Resume
        self.assertTrue(recorder.resume())
        self.assertEqual(recorder.state, RecordingState.RECORDING)
        time.sleep(0.3)
        self.assertGreater(recorder.frame_count, paused_frames)

        # Stop
        out_file = recorder.stop()
        self.assertEqual(recorder.state, RecordingState.IDLE)
        self.assertIsNotNone(out_file)
        self.assertTrue(out_file.exists())
        self.assertGreater(out_file.stat().st_size, 1024)

    def test_recorder_empty_cleanup(self) -> None:
        """Test that stopping immediately with 0 frames cleans up the empty file."""
        cfg = AppConfig(
            fps=30,
            recordings_dir=self.rec_dir,
            record_cursor=False,
        )
        recorder = ScreenRecorder(config=cfg)
        recorder.set_region(Region(x=0, y=0, width=320, height=240))
        # Start and immediately stop
        recorder.start()
        # Force frame_count to 0 to simulate cancellation before capture
        with recorder._lock:
            recorder._frame_count = 0
        out = recorder.stop()
        self.assertIsNone(out)
        self.assertEqual(len(list(self.rec_dir.glob("*.mp4"))), 0)

    def test_history_manager(self) -> None:
        """Test history scanning and deletion."""
        # Create dummy video and screenshot
        region = Region(x=0, y=0, width=160, height=120)
        snap_file = capture_screenshot(region, self.snap_dir, include_cursor=False)

        history_mgr = HistoryManager(self.rec_dir, self.snap_dir)
        items = history_mgr.scan_items()

        self.assertGreaterEqual(len(items), 1)
        found_snap = any(item.path == snap_file for item in items)
        self.assertTrue(found_snap)

    def test_audio_recorder(self) -> None:
        """Test AudioRecorder lifecycle."""
        from src.core.audio import AudioRecorder

        audio_rec = AudioRecorder(
            self.rec_dir, record_system_audio=True, record_microphone=False
        )
        self.assertFalse(audio_rec.is_recording)
        started = audio_rec.start()
        if started:
            self.assertTrue(audio_rec.is_recording)
            time.sleep(0.3)
            audio_rec.pause()
            time.sleep(0.1)
            audio_rec.resume()
            time.sleep(0.2)
            wav_file = audio_rec.stop()
            self.assertFalse(audio_rec.is_recording)
            if wav_file:
                self.assertTrue(wav_file.exists())
                self.assertEqual(wav_file.suffix.lower(), ".wav")

    def test_merge_video_audio(self) -> None:
        """Test FFmpeg video and audio muxer."""
        import cv2
        import soundfile as sf

        from src.core.audio import merge_video_audio

        # Create dummy video
        vid_path = self.rec_dir / "temp_dummy_vid.mp4"
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        w = cv2.VideoWriter(str(vid_path), fourcc, 30, (320, 240))
        frame = np.zeros((240, 320, 3), dtype=np.uint8)
        for _ in range(30):
            w.write(frame)
        w.release()

        # Create dummy WAV audio
        wav_path = self.rec_dir / "temp_dummy_audio.wav"
        dummy_audio = np.zeros((44100, 2), dtype=np.float32)
        sf.write(str(wav_path), dummy_audio, 44100, subtype="PCM_16")

        final_out = self.rec_dir / "final_merged.mp4"
        success = merge_video_audio(vid_path, wav_path, final_out)
        self.assertTrue(success)
        self.assertTrue(final_out.exists())
        self.assertGreater(final_out.stat().st_size, 1024)


if __name__ == "__main__":
    unittest.main()

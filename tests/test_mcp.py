"""
Unit Tests for screenvideo Native MCP Server (JSON-RPC 2.0 stdio)
"""

import json

import mss
import numpy as np
import pytest

from src.mcp_server import (
    LATEST_PROTOCOL_VERSION,
    ScreenVideoMCPServer,
)


class MockScreenShot:
    """Mock ScreenShot for headless or display-restricted environments."""

    def __init__(self, width: int = 320, height: int = 240) -> None:
        self.width = max(1, width)
        self.height = max(1, height)
        self._data = np.zeros((self.height, self.width, 4), dtype=np.uint8)
        self.raw = self._data.tobytes()

    def __array__(self, dtype: object = None) -> np.ndarray:
        return self._data


@pytest.fixture(autouse=True)
def mock_mss_grab(monkeypatch):
    """Ensure mss.MSS.grab works cleanly in headless/CI test environments."""

    def _mock_grab(self, *args, **kwargs):
        monitor = args[0] if args and isinstance(args[0], dict) else kwargs.get("monitor", {})
        w = monitor.get("width", 320) if isinstance(monitor, dict) else 320
        h = monitor.get("height", 240) if isinstance(monitor, dict) else 240
        return MockScreenShot(w, h)

    monkeypatch.setattr(mss.MSS, "grab", _mock_grab)


def test_mcp_initialize_negotiation():
    server = ScreenVideoMCPServer()

    # 1. Request latest supported version
    req = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "initialize",
        "params": {"protocolVersion": "2025-11-25"},
    }
    resp = server.handle_request(req)
    assert resp["id"] == 1
    assert resp["result"]["protocolVersion"] == "2025-11-25"
    assert resp["result"]["serverInfo"]["name"] == "screenvideo-mcp"
    assert "tools" in resp["result"]["capabilities"]

    # 2. Request older supported version
    req_old = {
        "jsonrpc": "2.0",
        "id": 2,
        "method": "initialize",
        "params": {"protocolVersion": "2024-11-05"},
    }
    resp_old = server.handle_request(req_old)
    assert resp_old["result"]["protocolVersion"] == "2024-11-05"

    # 3. Request unknown version -> fallback to latest supported
    req_unknown = {
        "jsonrpc": "2.0",
        "id": 3,
        "method": "initialize",
        "params": {"protocolVersion": "9999-99-99"},
    }
    resp_unknown = server.handle_request(req_unknown)
    assert resp_unknown["result"]["protocolVersion"] == LATEST_PROTOCOL_VERSION


def test_mcp_ping():
    server = ScreenVideoMCPServer()
    resp = server.handle_request({"jsonrpc": "2.0", "id": 5, "method": "ping"})
    assert resp["id"] == 5
    assert resp["result"] == {}


def test_mcp_tools_list_schema():
    server = ScreenVideoMCPServer()
    resp = server.handle_request({"jsonrpc": "2.0", "id": 10, "method": "tools/list"})
    assert resp["id"] == 10
    tools = resp["result"]["tools"]
    tool_names = [t["name"] for t in tools]
    assert "screen_list_monitors" in tool_names
    assert "screen_capture" in tool_names
    assert "screen_list_audio_devices" in tool_names
    assert "screen_copy_to_clipboard" in tool_names

    for tool in tools:
        assert "name" in tool
        assert "title" in tool
        assert "description" in tool
        assert "annotations" in tool
        assert "inputSchema" in tool
        assert "outputSchema" in tool


def test_mcp_screen_list_monitors():
    server = ScreenVideoMCPServer()
    req = {
        "jsonrpc": "2.0",
        "id": 20,
        "method": "tools/call",
        "params": {"name": "screen_list_monitors", "arguments": {}},
    }
    resp = server.handle_request(req)
    assert resp["id"] == 20
    assert resp["result"]["isError"] is False
    content = json.loads(resp["result"]["content"][0]["text"])
    assert "monitors" in content
    assert content["count"] > 0
    assert content["monitors"][0]["width"] > 0


def test_mcp_screen_capture(tmp_path):
    server = ScreenVideoMCPServer(output_dir=str(tmp_path))
    req = {
        "jsonrpc": "2.0",
        "id": 30,
        "method": "tools/call",
        "params": {
            "name": "screen_capture",
            "arguments": {
                "x": 0,
                "y": 0,
                "width": 100,
                "height": 100,
                "save_dir": str(tmp_path),
                "return_base64": True,
            },
        },
    }
    resp = server.handle_request(req)
    assert resp["id"] == 30
    assert resp["result"]["isError"] is False
    content = json.loads(resp["result"]["content"][0]["text"])
    assert "file_path" in content
    assert "base64_data" in content
    assert len(content["base64_data"]) > 0


def test_mcp_screen_list_audio_devices():
    server = ScreenVideoMCPServer()
    req = {
        "jsonrpc": "2.0",
        "id": 40,
        "method": "tools/call",
        "params": {"name": "screen_list_audio_devices", "arguments": {}},
    }
    resp = server.handle_request(req)
    assert resp["id"] == 40
    assert resp["result"]["isError"] is False
    content = json.loads(resp["result"]["content"][0]["text"])
    assert "speakers" in content
    assert "microphones" in content

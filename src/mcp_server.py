"""
Native Model Context Protocol (MCP) Server for screenvideo / ScreenCapture Pro.
Exposes high-performance desktop screen capture, multi-monitor topology discovery,
WASAPI loopback audio device enumeration, and clipboard image integration to AI agents
(Claude Desktop, Cursor, Antigravity, OpenManus) over JSON-RPC 2.0 stdio.
"""

from __future__ import annotations

import base64
import contextlib
import io
import json
import os
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

SUPPORTED_PROTOCOL_VERSIONS = (
    "2025-11-25",
    "2025-06-18",
    "2024-11-05",
)
LATEST_PROTOCOL_VERSION = SUPPORTED_PROTOCOL_VERSIONS[0]

MCP_TOOLS_SCHEMA: List[Dict[str, Any]] = [
    {
        "name": "screen_list_monitors",
        "title": "List Display Monitors",
        "description": "List all active physical and virtual displays with geometry, resolutions, and primary screen flags.",
        "annotations": {
            "readOnlyHint": True,
            "openWorldHint": False,
        },
        "inputSchema": {
            "type": "object",
            "properties": {},
        },
        "outputSchema": {
            "type": "object",
            "properties": {
                "monitors": {"type": "array"},
                "count": {"type": "integer"},
            },
        },
    },
    {
        "name": "screen_capture",
        "title": "Capture Screen or Region",
        "description": "Capture an instant high-resolution screenshot of a monitor, entire virtual screen, or bounding region (x, y, width, height).",
        "annotations": {
            "readOnlyHint": False,
            "openWorldHint": False,
        },
        "inputSchema": {
            "type": "object",
            "properties": {
                "monitor_index": {
                    "type": "integer",
                    "description": "Monitor index (0 for virtual combined desktop, 1 for primary display, 2+ for secondary). Default is 1.",
                    "default": 1,
                },
                "x": {"type": "integer", "description": "Optional bounding box X coordinate."},
                "y": {"type": "integer", "description": "Optional bounding box Y coordinate."},
                "width": {"type": "integer", "description": "Optional bounding box width (min 16)."},
                "height": {"type": "integer", "description": "Optional bounding box height (min 16)."},
                "include_cursor": {
                    "type": "boolean",
                    "description": "Include mouse cursor in the capture (default: True).",
                    "default": True,
                },
                "highlight_cursor": {
                    "type": "boolean",
                    "description": "Draw halo highlight around mouse cursor (default: False).",
                    "default": False,
                },
                "format": {
                    "type": "string",
                    "enum": ["png", "jpg", "bmp"],
                    "description": "Image format to save (default: 'png').",
                    "default": "png",
                },
                "save_dir": {
                    "type": "string",
                    "description": "Directory to save the screenshot file (default: 'screenshots').",
                    "default": "screenshots",
                },
                "return_base64": {
                    "type": "boolean",
                    "description": "Include base64-encoded image string in output (default: False).",
                    "default": False,
                },
                "copy_to_clipboard": {
                    "type": "boolean",
                    "description": "Copy screenshot to system clipboard (default: False).",
                    "default": False,
                },
            },
        },
        "outputSchema": {
            "type": "object",
            "properties": {
                "file_path": {"type": "string"},
                "width": {"type": "integer"},
                "height": {"type": "integer"},
                "format": {"type": "string"},
                "base64_data": {"type": "string"},
            },
        },
    },
    {
        "name": "screen_list_audio_devices",
        "title": "List Audio Devices",
        "description": "Enumerate available system output speakers (supporting WASAPI loopback audio recording) and input microphones.",
        "annotations": {
            "readOnlyHint": True,
            "openWorldHint": False,
        },
        "inputSchema": {
            "type": "object",
            "properties": {},
        },
        "outputSchema": {
            "type": "object",
            "properties": {
                "speakers": {"type": "array"},
                "microphones": {"type": "array"},
            },
        },
    },
    {
        "name": "screen_copy_to_clipboard",
        "title": "Copy Image to Clipboard",
        "description": "Copy an existing image file (PNG, JPG, BMP) directly to the system clipboard.",
        "annotations": {
            "readOnlyHint": False,
            "openWorldHint": False,
        },
        "inputSchema": {
            "type": "object",
            "properties": {
                "file_path": {
                    "type": "string",
                    "description": "Absolute or relative path to image file.",
                },
            },
            "required": ["file_path"],
        },
        "outputSchema": {
            "type": "object",
            "properties": {
                "success": {"type": "boolean"},
                "file_path": {"type": "string"},
            },
        },
    },
]


class ScreenVideoMCPServer:
    """Zero-dependency Model Context Protocol (MCP) JSON-RPC 2.0 stdio server for screenvideo."""

    def __init__(self, output_dir: str = "screenshots") -> None:
        self.output_dir = Path(output_dir)

    def handle_request(self, request: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Process a single JSON-RPC 2.0 request."""
        method = request.get("method", "")
        req_id = request.get("id")
        params = request.get("params") or {}

        if req_id is None and method.startswith("notifications/"):
            return None

        try:
            if method == "initialize":
                pkg_ver = getattr(sys.modules.get("src", None), "__version__", "1.0.0")
                client_version = params.get("protocolVersion")
                negotiated_version = (
                    client_version
                    if client_version in SUPPORTED_PROTOCOL_VERSIONS
                    else LATEST_PROTOCOL_VERSION
                )
                return {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {
                        "protocolVersion": negotiated_version,
                        "capabilities": {
                            "tools": {
                                "listChanged": False,
                            }
                        },
                        "serverInfo": {
                            "name": "screenvideo-mcp",
                            "version": "1.0.0",
                        },
                    },
                }

            if method == "ping":
                return {"jsonrpc": "2.0", "id": req_id, "result": {}}

            if method == "tools/list":
                return {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {"tools": MCP_TOOLS_SCHEMA},
                }

            if method == "tools/call":
                tool_name = params.get("name", "")
                arguments = params.get("arguments") or {}
                result_payload = self._call_tool(tool_name, arguments)
                return {
                    "jsonrpc": "2.0",
                    "id": req_id,
                    "result": {
                        "content": [
                            {
                                "type": "text",
                                "text": json.dumps(result_payload, ensure_ascii=False, indent=2),
                            }
                        ],
                        "isError": False,
                    },
                }

            return {
                "jsonrpc": "2.0",
                "id": req_id,
                "error": {
                    "code": -32601,
                    "message": f"Method not found: {method}",
                },
            }

        except Exception as exc:
            return {
                "jsonrpc": "2.0",
                "id": req_id,
                "result": {
                    "content": [{"type": "text", "text": f"Error: {exc}"}],
                    "isError": True,
                },
            }

    def _call_tool(self, name: str, args: Dict[str, Any]) -> Any:
        with contextlib.redirect_stdout(sys.stderr):
            if name == "screen_list_monitors":
                from src.core.monitors import get_available_monitors
                monitors = get_available_monitors()
                return {
                    "monitors": [
                        {
                            "index": m.index,
                            "name": m.name,
                            "left": m.left,
                            "top": m.top,
                            "width": m.width,
                            "height": m.height,
                            "is_primary": m.is_primary,
                            "display_text": m.display_text,
                        }
                        for m in monitors
                    ],
                    "count": len(monitors),
                }

            elif name == "screen_capture":
                from src.core.monitors import Region
                from src.core.screenshot import capture_screenshot

                mon_idx = int(args.get("monitor_index", 1))
                x = args.get("x")
                y = args.get("y")
                w = args.get("width")
                h = args.get("height")

                region = None
                if all(v is not None for v in (x, y, w, h)):
                    region = Region(x=int(x), y=int(y), width=int(w), height=int(h))

                fmt = str(args.get("format", "png"))
                out_dir = Path(args.get("save_dir", self.output_dir))
                include_cursor = bool(args.get("include_cursor", True))
                highlight_cursor = bool(args.get("highlight_cursor", False))
                copy_clip = bool(args.get("copy_to_clipboard", False))
                return_b64 = bool(args.get("return_base64", False))

                saved_path = capture_screenshot(
                    region=region,
                    output_dir=out_dir,
                    monitor_index=mon_idx,
                    include_cursor=include_cursor,
                    highlight_cursor=highlight_cursor,
                    copy_to_clipboard=copy_clip,
                    format=fmt,
                )

                result: Dict[str, Any] = {
                    "file_path": str(saved_path.resolve()),
                    "format": fmt,
                    "size_bytes": saved_path.stat().st_size if saved_path.exists() else 0,
                }

                if return_b64 and saved_path.exists():
                    raw_data = saved_path.read_bytes()
                    result["base64_data"] = base64.b64encode(raw_data).decode("ascii")

                return result

            elif name == "screen_list_audio_devices":
                speakers_list = []
                mics_list = []
                try:
                    import soundcard as sc
                    speakers = sc.all_speakers()
                    for s in speakers:
                        speakers_list.append({"name": s.name, "id": str(s.id)})
                    mics = sc.all_microphones(include_loopback=True)
                    for m in mics:
                        mics_list.append({
                            "name": m.name,
                            "id": str(m.id),
                            "is_loopback": getattr(m, "isloopback", False),
                        })
                except Exception as exc:
                    return {"speakers": [], "microphones": [], "error": str(exc)}

                return {
                    "speakers": speakers_list,
                    "microphones": mics_list,
                    "loopback_supported": any(m.get("is_loopback") for m in mics_list),
                }

            elif name == "screen_copy_to_clipboard":
                from src.core.clipboard import copy_image_file_to_clipboard

                target = Path(args["file_path"])
                if not target.exists():
                    raise FileNotFoundError(f"Image file not found: {target}")

                success = copy_image_file_to_clipboard(target)
                return {
                    "success": success,
                    "file_path": str(target.resolve()),
                }

            else:
                raise ValueError(f"Unknown tool: {name}")

    def run_stdio(self) -> None:
        """Run JSON-RPC 2.0 stdio loop until EOF."""
        for line in sys.stdin:
            line = line.strip()
            if not line:
                continue
            try:
                req = json.loads(line)
            except Exception:
                continue

            resp = self.handle_request(req)
            if resp is not None:
                sys.stdout.write(json.dumps(resp, ensure_ascii=False) + "\n")
                sys.stdout.flush()


def main_mcp() -> int:
    """CLI entry point for screenvideo-mcp."""
    server = ScreenVideoMCPServer()
    server.run_stdio()
    return 0


if __name__ == "__main__":
    sys.exit(main_mcp())

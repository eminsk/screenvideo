import sys
import os
import time
from pathlib import Path

# Configure utf-8 stdout/stderr
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

# Target project is aiagent
aiagent_dir = Path(r"C:\proekts\aiagent")
sys.path.insert(0, str(aiagent_dir / "src"))

import g4f
import g4f.Provider as P
from g4f.client import Client
from aiagent.model_inspector import ModelInspector
from aiagent.agent import G4FAgentEngine
from aiagent.config import ConfigManager

print("=== 1. G4F VERSION & METADATA ===")
print("g4f file:", g4f.__file__)
ver_info = ModelInspector.get_g4f_version_info(force_refresh=True)
for k, v in ver_info.items():
    if k != "verification":
        print(f"  {k}: {v}")
print("  verification status:", ver_info.get("verification"))

print("\n=== 2. COMMITS IN G4F ===")
commits = ModelInspector.get_g4f_recent_commits(limit=10, force_refresh=True)
for c in commits:
    print(f"  [{c['sha']}] {c['date']} | {c['author']}: {c['title']}")

print("\n=== 3. PROVIDERS INSPECTION ===")
print(f"Total raw providers in P.__providers__: {len(P.__providers__)}")
providers_map = G4FAgentEngine.get_providers_map()
print(f"Providers recognized by G4FAgentEngine: {len(providers_map)}")
img_map = G4FAgentEngine.get_image_providers_map()
print(f"Image providers recognized by G4FAgentEngine: {len(img_map)}")

print("\n=== 4. TESTING CHAT COMPLETIONS (ONLINE TEST) ===")
client = Client()
test_models = ["gpt-4o-mini", "llama-3.3-70b", "deepseek-v3"]
for m in test_models:
    t0 = time.time()
    try:
        resp = client.chat.completions.create(
            model=m,
            messages=[{"role": "user", "content": "Ping! Reply with 'OK' and your model name in 5 words."}],
            timeout=15
        )
        content = resp.choices[0].message.content if (resp and resp.choices and resp.choices[0].message) else "EMPTY"
        print(f"  [OK] Model '{m}' ({time.time()-t0:.2f}s): {content.strip()[:100]}")
    except Exception as exc:
        print(f"  [FAIL] Model '{m}' ({time.time()-t0:.2f}s): {type(exc).__name__}: {exc}")

print("\n=== 5. TESTING STREAMING CHAT (ONLINE TEST) ===")
agent = G4FAgentEngine(model_name="gpt-4o-mini")
t0 = time.time()
try:
    accum = []
    for chunk in agent.stream_task("Reply with exactly: STREAMING_WORKS"):
        accum.append(chunk)
        if len(accum) > 30:
            break
    full_text = "".join(accum).strip()
    print(f"  Stream elapsed: {time.time()-t0:.2f}s | Received: {full_text[:120]}")
except Exception as exc:
    print(f"  Stream error ({time.time()-t0:.2f}s): {type(exc).__name__}: {exc}")

print("\n=== 6. VERIFYING CONFIG & BASELINE ===")
cfg = ConfigManager(workspace_dir=aiagent_dir)
print("Config path:", cfg.config_path)
print("Current baseline in ConfigManager:", cfg.get_verified_g4f_version())
print("Default verified in code:", cfg.DEFAULT_VERIFIED_G4F)

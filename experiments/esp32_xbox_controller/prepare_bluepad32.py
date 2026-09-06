"""Fetch pinned upstream sources and integrate BTstack locally before CMake runs."""
import os
from pathlib import Path
import subprocess
import sys

Import("env")

REVISION = "e9b755faabc240585da42e6d26164bb2cdd064d3"  # Bluepad32 4.2.0
root = Path(env.subst("$PROJECT_DIR"))
deps = root / ".pio" / "bluepad32"

def run(*args, **kwargs):
    subprocess.run(args, check=True, **kwargs)

if not (deps / ".git").exists():
    run("git", "clone", "--no-checkout",
        "https://github.com/ricardoquesada/bluepad32.git", str(deps))
if (not (deps / "src/components/bluepad32/CMakeLists.txt").exists()
        or subprocess.check_output(["git", "-C", str(deps), "rev-parse", "HEAD"], text=True).strip() != REVISION):
    run("git", "-C", str(deps), "checkout", "--detach", REVISION)
if not (deps / "src/components/btstack/CMakeLists.txt").exists():
    run("git", "-C", str(deps), "submodule", "update", "--init", "--recursive")
    child_env = dict(os.environ, IDF_PATH=str(deps / "src"))
    run(sys.executable, "integrate_btstack.py",
        cwd=deps / "external/btstack/port/esp32", env=child_env)

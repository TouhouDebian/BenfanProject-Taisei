#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
# Host requirements: Python 3, Meson >=1.8, Ninja, CMake, pkg-config,
# msgfmt (gettext), glslc (shaderc), spirv-cross, Git.
git submodule update --init --recursive
if [[ ! -x .tools/emsdk/emsdk ]]; then
  git clone --depth 1 https://github.com/emscripten-core/emsdk.git .tools/emsdk
fi
.tools/emsdk/emsdk install 4.0.10
.tools/emsdk/emsdk activate 4.0.10
source .tools/emsdk/emsdk_env.sh
python3 - <<'PY'
from pathlib import Path
p=Path('misc/ci/benfan-emscripten.ini')
s=Path('misc/ci/benfan-emscripten.template.ini').read_text()
p.write_text(s.replace('@EMSCRIPTEN@', str(Path('.tools/emsdk/upstream/emscripten').resolve())))
PY
meson setup build-web --cross-file misc/ci/emscripten-build.ini --cross-file misc/ci/benfan-emscripten.ini --prefix="$PWD/dist-web" "$@"
meson compile -C build-web
meson install -C build-web
python3 scripts/benfan/package_web.py

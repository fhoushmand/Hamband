#!/bin/bash
set -e

rm -rf build
mkdir -p build
pushd build >/dev/null

# --- Build-time linker stub for libibverbs (do NOT change LD_LIBRARY_PATH) ---
STUB_DIR="/scratch/user/u.js213354/ibverbs-linkstub"
IBV_SO=""

for c in /usr/lib64/libibverbs.so.1 /usr/lib/x86_64-linux-gnu/libibverbs.so.1; do
  [[ -f "$c" ]] && { IBV_SO="$c"; break; }
done

if [[ -n "$IBV_SO" ]]; then
  mkdir -p "$STUB_DIR"
  ln -sf "$IBV_SO" "$STUB_DIR/libibverbs.so"
  export LIBRARY_PATH="$STUB_DIR:${LIBRARY_PATH-}"
  echo "[build.sh] Using ibverbs stub -> $IBV_SO"
else
  echo "[build.sh] WARNING: libibverbs.so.1 not found on this node; link may fail." >&2
fi
# ---------------------------------------------------------------------------

# 1) Conan deps (also tells conan we want C++17)
conan install .. --build missing -s compiler.cppstd=17

# 2) Force C++17 in CMake configure (THIS is what fixes make_unique + structured bindings)
GEN=""
command -v ninja >/dev/null 2>&1 && GEN="-G Ninja"

cmake .. ${GEN} \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_CXX_EXTENSIONS=ON \
  -DCMAKE_CXX_FLAGS="-std=gnu++17"

# 3) Build
cmake --build . -- -j"$(nproc)"

popd >/dev/null

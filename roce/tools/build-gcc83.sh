#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
profile="$root/tools/gcc83-release.profile"
toolchain="${HAMBAND_GCC83_ROOT:-$HOME/opt/gcc-8.3.0}"
conan_home="${HAMBAND_GCC83_CONAN_HOME:-$HOME/.conan-hamband-gcc83}"
build_dir="${HAMBAND_GCC83_BUILD_DIR:-$root/wellcoordination/build-gcc83}"
runtime_dir=${HAMBAND_GCC83_RUNTIME_DIR:-}
runtime_link_flags=()
runtime_linker_flags=

export PATH="$HOME/.local/bin:$PATH"
export CC="$toolchain/bin/gcc"
export CXX="$toolchain/bin/g++"
export LD_LIBRARY_PATH="$toolchain/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
export CONAN_DEFAULT_PROFILE_PATH="$profile"
export CONAN_USER_HOME="$conan_home"

if [[ -n "$runtime_dir" ]]; then
  test -f "$runtime_dir/libstdc++.so.6.0.25"
  test -f "$runtime_dir/libgcc_s.so.1"
  test "$(sha256sum "$runtime_dir/libstdc++.so.6.0.25" | awk '{print $1}')" = \
    cd83a7033636810b5b9be5f3b4702d79a8adb9fd7d42ddf64e9988635a86798a
  test "$(sha256sum "$runtime_dir/libgcc_s.so.1" | awk '{print $1}')" = \
    44d951ea7184dfc82128451e23f8b15a9d9aae4376e1a39ce2373f6e2cb40299
  ln -sfn libstdc++.so.6.0.25 "$runtime_dir/libstdc++.so.6"
  ln -sfn libstdc++.so.6.0.25 "$runtime_dir/libstdc++.so"
  export LIBRARY_PATH="$runtime_dir${LIBRARY_PATH:+:$LIBRARY_PATH}"
  runtime_linker_flags="-L$runtime_dir -Wl,-rpath,$runtime_dir"
  export LDFLAGS="$runtime_linker_flags ${LDFLAGS:-}"
  runtime_link_flags=(-L "$runtime_dir" -Wl,-rpath,"$runtime_dir")
fi

"$root/tools/install-gcc83.sh"

if [[ ! -f "$conan_home/.seeded-from-default-cache" ]]; then
  mkdir -p "$conan_home/.conan"
  cp -a "$HOME/.conan/." "$conan_home/.conan/"
  touch "$conan_home/.seeded-from-default-cache"
fi

if [[ ! -f "$root/junction/CMakeLists.txt" ]]; then
  unzip -q -o "$root/junction.zip" -d "$root"
fi
if [[ ! -f "$root/turf/CMakeLists.txt" ]]; then
  unzip -q -o "$root/turf.zip" -d "$root"
fi

conan export "$root/conan/exports/compiler-options" dory/stable

packages=(extern shared memstore ctrl conn crypto mem log crash-consensus)
for package in "${packages[@]}"; do
  conan create "$root/$package" --profile "$profile" --build=missing \
    --test-folder=None
done

mkdir -p "$build_dir"
pushd "$build_dir" >/dev/null
conan install .. --profile "$profile" --build=missing
conan build ..
if [[ -n "$runtime_dir" ]]; then
  # Conan's CMake helper clears CMAKE_EXE_LINKER_FLAGS in this project. Reconfigure
  # only the executable link setting so GCC 8.3 objects cannot bind to a newer ABI.
  cmake -S "$root/wellcoordination/src" -B "$build_dir" \
    -DCMAKE_EXE_LINKER_FLAGS="$runtime_linker_flags"
  cmake --build "$build_dir" --target band band-crdt
fi
popd >/dev/null

benchmarks=(
  account counter courseware gset movie orset pnset project register rubis shop
  smallbank twopset kvstore
)
for benchmark in "${benchmarks[@]}"; do
  "$CXX" -std=c++17 -O3 -pthread \
    "$root/wellcoordination/benchmark/$benchmark-benchmark.cpp" \
    -o "$build_dir/bin/$benchmark-benchmark" \
    "${runtime_link_flags[@]}" \
    -lstdc++fs
done

"$CXX" -std=c++17 -O3 -pthread \
  "$root/wellcoordination/benchmark/register-crdt-benchmark.cpp" \
  -o "$build_dir/bin/register-crdt-benchmark" \
  "${runtime_link_flags[@]}" \
  -lstdc++fs

test -x "$build_dir/bin/band"
test -x "$build_dir/bin/band-crdt"
if [[ -n "$runtime_dir" ]]; then
  while IFS= read -r binary; do
    ldd_output="$(LD_LIBRARY_PATH="$runtime_dir" ldd "$binary" 2>&1)"
    grep -Fq "$runtime_dir/libstdc++.so.6" <<<"$ldd_output"
    if grep -Fq "not found" <<<"$ldd_output"; then
      echo "GCC 8.3 binary has an unresolved dependency: $binary" >&2
      exit 1
    fi
    symbol_table="$(objdump -T "$binary")"
    if grep -Fq GLIBCXX_3.4.30 <<<"$symbol_table"; then
      echo "GCC 8.3 binary unexpectedly requires GLIBCXX_3.4.30: $binary" >&2
      exit 1
    fi
  done < <(find "$build_dir/bin" -maxdepth 1 -type f -executable | sort)
fi
"$CXX" --version | head -1
sha256sum \
  "$build_dir/bin/band" \
  "$build_dir/bin/band-crdt"

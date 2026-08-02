#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
profile="$root/tools/gcc11-release.profile"

export PATH="$HOME/.local/bin:$PATH"
export CONAN_DEFAULT_PROFILE_PATH="$profile"

if [[ ! -f "$root/junction/CMakeLists.txt" ]]; then
  unzip -q -o "$root/junction.zip" -d "$root"
fi
if [[ ! -f "$root/turf/CMakeLists.txt" ]]; then
  unzip -q -o "$root/turf.zip" -d "$root"
fi

conan export "$root/conan/exports/compiler-options" dory/stable

packages=(extern shared memstore ctrl conn crypto mem log crash-consensus)
for package in "${packages[@]}"; do
  conan create "$root/$package" --profile "$profile" --build=outdated \
    --test-folder=None
done

mkdir -p "$root/wellcoordination/build"
pushd "$root/wellcoordination/build" >/dev/null
conan install .. --profile "$profile" --build=missing
conan build ..
popd >/dev/null

g++ -std=c++17 -O3 -pthread \
  "$root/wellcoordination/benchmark/account-benchmark.cpp" \
  -o "$root/wellcoordination/build/bin/account-benchmark"

#!/bin/bash

set -e

rm -rf build
mkdir build
pushd build

# --- Build-time linker stub for libibverbs (do NOT change LD_LIBRARY_PATH) ---

STUB_DIR="/scratch/user/u.js213354/ibverbs-linkstub"

IBV_SO=""



# common locations on RHEL/Ubuntu-style nodes

for c in /usr/lib64/libibverbs.so.1 /usr/lib/x86_64-linux-gnu/libibverbs.so.1; do

  [[ -f "$c" ]] && { IBV_SO="$c"; break; }

done



if [[ -n "$IBV_SO" ]]; then

  mkdir -p "$STUB_DIR"

  ln -sf "$IBV_SO" "$STUB_DIR/libibverbs.so"

  # point the linker at our stub (runtime stays on system .so.1)

  export LIBRARY_PATH="$STUB_DIR:${LIBRARY_PATH-}"

  echo "[build.sh] Using ibverbs stub -> $IBV_SO"

else

  echo "[build.sh] WARNING: libibverbs.so.1 not found on this node; link may fail." >&2

fi

# ---------------------------------------------------------------------------

conan install .. --build missing 
#--profile /rhome/fhous001/farzin/FastChain/dory/conan/profiles/gcc-debug.profile
conan build ..

wrdt_benchmarks=(
  account counter courseware gset movie orset pnset project register rubis shop
  smallbank twopset kvstore
)
for benchmark in "${wrdt_benchmarks[@]}"; do
  g++ -std=c++17 -O3 -pthread "../benchmark/$benchmark-benchmark.cpp" \
    -o "bin/$benchmark-benchmark"
done

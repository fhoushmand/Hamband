#!/bin/bash
#SBATCH --account=158421658697
#SBATCH --partition=cpu
#SBATCH --job-name=hamband_ib_build
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=24G
#SBATCH --time=02:00:00

set -euo pipefail

ROOT=${HAMBAND_ROOT:-/scratch/user/u.js213354/Hamband/infiniband}
mkdir -p "$ROOT/results"
exec > >(tee "$ROOT/results/hamband_ib_build.${SLURM_JOB_ID}.log") 2>&1

cd "$ROOT"
module purge
module load GCC/8.3.0
export PATH="/scratch/user/u.js213354/bin:/scratch/user/u.js213354/python-local/bin:$PATH"
export CONAN_CPU_COUNT="${SLURM_CPUS_PER_TASK:-8}"

stub_dir="/scratch/user/u.js213354/ibverbs-linkstub"
ibverbs_so=""
for candidate in /usr/lib64/libibverbs.so.1 /usr/lib/x86_64-linux-gnu/libibverbs.so.1; do
  if [[ -f "$candidate" ]]; then
    ibverbs_so="$candidate"
    break
  fi
done
if [[ -z "$ibverbs_so" ]]; then
  echo "ERROR: libibverbs.so.1 is unavailable" >&2
  exit 1
fi
mkdir -p "$stub_dir"
ln -sf "$ibverbs_so" "$stub_dir/libibverbs.so"
memcached_lib_dir="/scratch/user/u.js213354/libmemcached/build/lib"
test -f "$memcached_lib_dir/libmemcached.so.11.0.0"
export LIBRARY_PATH="$stub_dir:$memcached_lib_dir:${LIBRARY_PATH:-}"
export CMAKE_LIBRARY_PATH="$stub_dir:$memcached_lib_dir:${CMAKE_LIBRARY_PATH:-}"
export LD_LIBRARY_PATH="$memcached_lib_dir:${LD_LIBRARY_PATH:-}"

conan export conan/exports/compiler-options dory/stable
while read -r directory package; do
  conan create "$directory" \
    --profile default \
    --build="$package" \
    --test-folder=None
done <<'PACKAGES'
extern dory-external
shared dory-shared
memstore dory-memstore
ctrl dory-ctrl
conn dory-connection
mem dory-memory
log dory-log
crash-consensus dory-crash-consensus
PACKAGES

cd wellcoordination
bash build.sh

test -x build/bin/band
test -x build/bin/band-crdt
sha256sum build/bin/band build/bin/band-crdt

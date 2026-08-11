#!/bin/bash
#SBATCH --account=158421658697
#SBATCH --partition=cpu
#SBATCH --job-name=hamband_ib_failure_build
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=24G
#SBATCH --time=02:00:00

set -euo pipefail

ROOT=${HAMBAND_ROOT:-/scratch/user/u.js213354/Hamband/infiniband}
RESULTS="$ROOT/results"
mkdir -p "$RESULTS"
exec > >(tee "$RESULTS/hamband_ib_failure_build.${SLURM_JOB_ID}.log") 2>&1

module purge
module load GCC/8.3.0
export PATH="/scratch/user/u.js213354/bin:/scratch/user/u.js213354/python-local/bin:$PATH"
export CONAN_CPU_COUNT="${SLURM_CPUS_PER_TASK:-8}"
export LD_LIBRARY_PATH="/scratch/user/u.js213354/libmemcached/build/lib:${LD_LIBRARY_PATH:-}"

ordinary_backup="$RESULTS/ordinary-binary-backup.${SLURM_JOB_ID}"
rm -rf "$ordinary_backup"
mkdir -p "$ordinary_backup"
restore_ordinary() {
  if [[ -f "$ordinary_backup/band" && -f "$ordinary_backup/band-crdt" ]]; then
    mkdir -p "$ROOT/wellcoordination/build/bin"
    cp -p "$ordinary_backup/band" "$ROOT/wellcoordination/build/bin/band"
    cp -p "$ordinary_backup/band-crdt" "$ROOT/wellcoordination/build/bin/band-crdt"
  fi
}
cleanup() {
  restore_ordinary
  rm -rf "$ordinary_backup"
}
trap cleanup EXIT
for binary in band band-crdt; do
  test -x "$ROOT/wellcoordination/build/bin/$binary"
  cp -p "$ROOT/wellcoordination/build/bin/$binary" "$ordinary_backup/$binary"
done
sha256sum "$ordinary_backup/band" "$ordinary_backup/band-crdt"

cd "$ROOT/wellcoordination"
bash build.sh

test -x build/bin/band-failure
test -x build/bin/band-crdt-failure
sha256sum build/bin/band-failure build/bin/band-crdt-failure

# The failure targets are additive. Keep the published no-failure executables
# byte-identical to the binaries that produced the 312 baseline rows.
restore_ordinary
cmp -s "$ordinary_backup/band" build/bin/band
cmp -s "$ordinary_backup/band-crdt" build/bin/band-crdt
sha256sum build/bin/band build/bin/band-crdt

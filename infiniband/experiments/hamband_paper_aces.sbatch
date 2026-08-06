#!/bin/bash

set -euo pipefail

ROOT=${HAMBAND_ROOT:-/scratch/user/u.js213354/Hamband/infiniband}
RESULTS="$ROOT/results"
RUNNER="$ROOT/experiments/run_hamband_paper_aces.py"

cd "$ROOT"
mkdir -p "$RESULTS"

module purge
module load GCCcore/10.2.0 Python/3.8.6
export LD_LIBRARY_PATH="/scratch/user/u.js213354/libmemcached/build/lib:${LD_LIBRARY_PATH:-}"

test -x wellcoordination/build/bin/band
test -x wellcoordination/build/bin/band-crdt
test -x "$RUNNER"

printf 'job_id=%s\nnode_list=%s\nstarted=%s\n' \
  "$SLURM_JOB_ID" "$SLURM_JOB_NODELIST" "$(date --iso-8601=seconds)"
scontrol show hostnames "$SLURM_JOB_NODELIST"

if [[ "${HAMBAND_SKIP_SMOKE:-0}" != "1" ]]; then
  python3 "$RUNNER" \
    --operations 40000 \
    --workload account counter \
    --replicas 3 4 8 \
    --percentage 25 \
    --attempts 1 \
    --timeout 900 \
    --output "$RESULTS/hamband_infiniband_aces_smoke_40k.csv" \
    --log-dir "$RESULTS/hamband_infiniband_aces_smoke_40k_logs"
fi

python3 "$RUNNER" \
  --operations 4000000 \
  --attempts 2 \
  --timeout 3600 \
  --output "$RESULTS/hamband_infiniband_aces_4m.csv" \
  --log-dir "$RESULTS/hamband_infiniband_aces_4m_logs"

printf 'completed=%s\n' "$(date --iso-8601=seconds)"

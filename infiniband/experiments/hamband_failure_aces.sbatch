#!/bin/bash

set -euo pipefail

ROOT=${HAMBAND_ROOT:-/scratch/user/u.js213354/Hamband/infiniband}
RESULTS="$ROOT/results"
RUNNER="$ROOT/experiments/run_hamband_failure_aces.py"
STATE="$RESULTS/hamband_infiniband_aces_4m_failure.${SLURM_JOB_ID}.state"

cd "$ROOT"
mkdir -p "$RESULTS" "$STATE"

module purge
module load GCCcore/10.2.0 Python/3.8.6
export LD_LIBRARY_PATH="/scratch/user/u.js213354/libmemcached/build/lib:${LD_LIBRARY_PATH:-}"

test -x wellcoordination/build/bin/band-failure
test -x wellcoordination/build/bin/band-crdt-failure
test -x "$RUNNER"

printf 'job_id=%s\nnode_list=%s\nstarted=%s\n' \
  "$SLURM_JOB_ID" "$SLURM_JOB_NODELIST" "$(date --iso-8601=seconds)"
scontrol show hostnames "$SLURM_JOB_NODELIST"

attempt=0
while true; do
  attempt=$((attempt + 1))
  rm -f "$STATE/failed" "$STATE/retry"
  printf 'attempt=%s\nstarted=%s\n' "$attempt" "$(date --iso-8601=seconds)" \
    >"$STATE/running"
  if python3 "$RUNNER" --attempts 2 --timeout 1800; then
    printf 'attempt=%s\ncompleted=%s\n' "$attempt" "$(date --iso-8601=seconds)" \
      >"$STATE/completed"
    exit 0
  else
    status=$?
  fi

  printf 'attempt=%s\nexit_status=%s\nfailed=%s\n' \
    "$attempt" "$status" "$(date --iso-8601=seconds)" >"$STATE/failed"
  echo "Failure matrix attempt failed; preserving the allocation" >&2
  while [[ ! -e "$STATE/retry" && ! -e "$STATE/stop" ]]; do
    sleep 30
  done
  if [[ -e "$STATE/stop" ]]; then
    exit 1
  fi
done

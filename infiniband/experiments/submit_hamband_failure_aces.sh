#!/bin/bash
# Submit one shared five-node failure matrix from the same ACES NUMA2 pool.

set -euo pipefail

ROOT=${HAMBAND_ROOT:-/scratch/user/u.js213354/Hamband/infiniband}
REFERENCE_ROOT=${NUMA2_REFERENCE_ROOT:-/scratch/user/u.js213354/Coordination-Geo-replicated-hybrid-system/nonblocking}
POOL_FILE=${NUMA2_NODE_POOL_FILE:-$REFERENCE_ROOT/config/numa2_nodes.txt}
ACCOUNT=${HAMBAND_SLURM_ACCOUNT:-158421658697}
PARTITION=${HAMBAND_SLURM_PARTITION:-cpu}
JOB_SCRIPT="$ROOT/experiments/hamband_failure_aces.sbatch"
RESULTS="$ROOT/results"

test -f "$POOL_FILE"
test -f "$JOB_SCRIPT"
mapfile -t pool < <(awk '!/^#/ && NF {print $1}' "$POOL_FILE" | sort -u)
if (( ${#pool[@]} < 5 )); then
  echo "ERROR: NUMA2 pool has only ${#pool[@]} nodes" >&2
  exit 1
fi
declare -A allowed=()
for node in "${pool[@]}"; do allowed[$node]=1; done

mapfile -t cpu_nodes < <(sinfo -p "$PARTITION" -N -h -o '%N' | sort -u)
excluded=()
for node in "${cpu_nodes[@]}"; do
  [[ -n "${allowed[$node]:-}" ]] || excluded+=("$node")
done
if (( ${#excluded[@]} == 0 )); then
  echo "ERROR: could not derive non-NUMA2 exclusions" >&2
  exit 1
fi

mkdir -p "$RESULTS"
job_id=$(sbatch --parsable \
  --account="$ACCOUNT" --partition="$PARTITION" \
  --job-name=hamband_ib_failure_4m \
  --nodes=5 --ntasks=5 --ntasks-per-node=1 --cpus-per-task=4 \
  --mem=16G --time=04:00:00 --chdir="$ROOT" \
  --exclude="$(IFS=,; echo "${excluded[*]}")" \
  --output="$RESULTS/hamband_infiniband_aces_4m_failure.%j.log" \
  --export=ALL,HAMBAND_ROOT="$ROOT" \
  "$JOB_SCRIPT")
echo "submitted_job_id=$job_id"
echo "Slurm will retain one registry and four workers for all nine rows."

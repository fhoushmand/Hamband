#!/bin/bash
# Submit one fixed nine-node Hamband allocation from the verified NUMA2 pool.

set -euo pipefail

ROOT=${HAMBAND_ROOT:-/scratch/user/u.js213354/Hamband/infiniband}
REFERENCE_ROOT=${NUMA2_REFERENCE_ROOT:-/scratch/user/u.js213354/Coordination-Geo-replicated-hybrid-system/nonblocking}
POOL_FILE=${NUMA2_NODE_POOL_FILE:-$REFERENCE_ROOT/config/numa2_nodes.txt}
ACCOUNT=${HAMBAND_SLURM_ACCOUNT:-158421658697}
PARTITION=${HAMBAND_SLURM_PARTITION:-cpu}
JOB_SCRIPT="$ROOT/experiments/hamband_paper_aces.sbatch"
RESULTS="$ROOT/results"

if [[ ! -f "$POOL_FILE" ]]; then
  echo "ERROR: missing verified NUMA2 pool: $POOL_FILE" >&2
  exit 1
fi
if [[ ! -f "$JOB_SCRIPT" ]]; then
  echo "ERROR: missing experiment job script: $JOB_SCRIPT" >&2
  exit 1
fi

mapfile -t pool < <(awk '!/^#/ && NF {print $1}' "$POOL_FILE" | sort -u)
if (( ${#pool[@]} < 9 )); then
  echo "ERROR: NUMA2 pool has only ${#pool[@]} nodes" >&2
  exit 1
fi

declare -A is_pool=()
for node in "${pool[@]}"; do
  is_pool[$node]=1
done

mkdir -p "$RESULTS"
sbatch_args=(
  --parsable
  --account="$ACCOUNT"
  --partition="$PARTITION"
  --job-name=hamband_ib_4m
  --nodes=9
  --ntasks=9
  --ntasks-per-node=1
  --cpus-per-task=4
  --mem=16G
  --time=24:00:00
  --chdir="$ROOT"
  --output="$RESULTS/hamband_infiniband_aces_4m.%j.log"
  --export=ALL,HAMBAND_ROOT="$ROOT"
)

if [[ -n "${HAMBAND_FIXED_NODES:-}" ]]; then
  IFS=',' read -r -a fixed_nodes <<< "$HAMBAND_FIXED_NODES"
  if (( ${#fixed_nodes[@]} != 9 )); then
    echo "ERROR: HAMBAND_FIXED_NODES must contain exactly nine nodes" >&2
    exit 1
  fi
  for node in "${fixed_nodes[@]}"; do
    if [[ -z "${is_pool[$node]:-}" ]]; then
      echo "ERROR: $node is not in the verified NUMA2 pool" >&2
      exit 1
    fi
  done
  sbatch_args+=(--nodelist="$(IFS=,; echo "${fixed_nodes[*]}")")
else
  mapfile -t cpu_nodes < <(sinfo -p "$PARTITION" -N -h -o '%N' | sort -u)
  excluded=()
  for node in "${cpu_nodes[@]}"; do
    if [[ -z "${is_pool[$node]:-}" ]]; then
      excluded+=("$node")
    fi
  done
  if (( ${#excluded[@]} == 0 )); then
    echo "ERROR: could not derive non-NUMA2 exclusions for $PARTITION" >&2
    exit 1
  fi
  sbatch_args+=(--exclude="$(IFS=,; echo "${excluded[*]}")")
fi

job_id=$(sbatch "${sbatch_args[@]}" "$JOB_SCRIPT")
echo "submitted_job_id=$job_id"
echo "Slurm will atomically select and retain one registry plus eight NUMA2 workers."
